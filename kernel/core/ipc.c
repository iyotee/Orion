/*
 * Orion Operating System - Lock-Free IPC System
 *
 * High-performance Inter-Process Communication with zero-copy message passing,
 * lock-free queues, and capability-based security.
 *
 * Developed by Jérémy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/kernel.h>
#include <orion/types.h>
#include <orion/mm.h>
#include <orion/scheduler.h>

// ========================================
// CONSTANTES ET CONFIGURATION
// ========================================

#define MAX_IPC_PORTS           4096
#define MAX_MSG_QUEUE_SIZE      256
#define MAX_MSG_SIZE            (64 * 1024)    // 64KB max par message
#define IPC_SHARED_POOL_SIZE    (16 * 1024 * 1024) // 16MB pool partagé
#define IPC_RING_BUFFER_SIZE    4096

// Flags pour les messages
#define IPC_MSG_FLAG_ZERO_COPY  0x00000001
#define IPC_MSG_FLAG_URGENT     0x00000002
#define IPC_MSG_FLAG_RELIABLE   0x00000004
#define IPC_MSG_FLAG_BROADCAST  0x00000008

// ========================================
// STRUCTURES DE DONNÉES
// ========================================

// Structure pour un slot de message dans la queue lock-free
typedef struct ipc_msg_slot {
    atomic64_t sequence;             // Numéro de séquence pour lock-free
    or_cap_t sender_port;            // Port expéditeur
    uint32_t flags;                  // Flags du message
    uint32_t data_size;              // Taille des données
    uint64_t timestamp;              // Timestamp d'envoi
    uint64_t page_phys;              // Page physique pour zero-copy
    uint64_t offset;                 // Offset dans la page
    or_cap_t transferred_caps[8];    // Capabilities transférées
    uint32_t cap_count;              // Nombre de capabilities
    uint32_t reserved;               // Padding
} ipc_msg_slot_t;

// Queue lock-free pour messages
typedef struct ipc_msg_queue {
    atomic64_t head;                 // Index head (producteur)
    atomic64_t tail;                 // Index tail (consommateur)
    atomic64_t committed;            // Dernière transaction commitée
    ipc_msg_slot_t slots[MAX_MSG_QUEUE_SIZE];
    char cache_line_padding[64];     // Éviter false sharing
} ipc_msg_queue_t;

// Port IPC complet
typedef struct ipc_port {
    atomic64_t ref_count;            // Compteur de références
    or_cap_t cap_id;                 // Capability ID
    uint64_t owner_pid;              // PID propriétaire
    uint64_t permissions;            // Permissions d'accès
    atomic32_t state;                // État du port
    
    // Queues de messages
    ipc_msg_queue_t* send_queue;     // Queue d'envoi
    ipc_msg_queue_t* recv_queue;     // Queue de réception
    
    // Threads en attente
    thread_t* waiting_senders;       // Threads bloqués en envoi
    thread_t* waiting_receivers;     // Threads bloqués en réception
    spinlock_t waiters_lock;         // Verrou pour les listes d'attente
    
    // Statistiques
    atomic64_t msgs_sent;            // Messages envoyés
    atomic64_t msgs_received;        // Messages reçus
    atomic64_t bytes_transferred;    // Bytes transférés
    
    // Configuration
    uint32_t max_queue_size;         // Taille max de queue
    uint32_t max_msg_size;           // Taille max de message
    uint64_t created_time;           // Temps de création
    
    // Callbacks et endpoints
    void (*msg_handler)(struct ipc_port* port, ipc_msg_slot_t* msg);
    struct ipc_port* linked_port;    // Port lié pour pipes
} ipc_port_t;

// États d'un port IPC
#define IPC_PORT_STATE_ACTIVE    1
#define IPC_PORT_STATE_CLOSING   2
#define IPC_PORT_STATE_CLOSED    3
#define IPC_PORT_STATE_ERROR     4

// Pool mémoire partagé pour zero-copy
typedef struct ipc_shared_pool {
    uint64_t base_phys;              // Adresse physique de base
    uint64_t base_virt;              // Adresse virtuelle de base
    uint64_t total_size;             // Taille totale
    atomic64_t* allocation_bitmap;   // Bitmap d'allocation
    uint64_t page_count;             // Nombre de pages
    spinlock_t alloc_lock;           // Verrou pour allocation
} ipc_shared_pool_t;

// Registre global IPC
typedef struct ipc_registry {
    ipc_port_t ports[MAX_IPC_PORTS]; // Table des ports
    atomic64_t next_cap_id;          // Prochain capability ID
    spinlock_t registry_lock;        // Verrou pour la table
    
    // Pool mémoire partagé
    ipc_shared_pool_t shared_pool;
    
    // Statistiques globales
    atomic64_t total_ports_created;
    atomic64_t total_msgs_sent;
    atomic64_t active_ports;
    
    // Hash table pour lookup rapide
    uint32_t port_hash_table[MAX_IPC_PORTS];
    
} ipc_registry_t;

// ========================================
// VARIABLES GLOBALES
// ========================================

static ipc_registry_t* g_ipc_registry = NULL;
static bool ipc_initialized = false;

// ========================================
// UTILITAIRES LOCK-FREE
// ========================================

static uint32_t hash_capability(or_cap_t cap) {
    // Hash simple mais efficace pour lookup rapide
    uint64_t h = cap;
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    return (uint32_t)(h % MAX_IPC_PORTS);
}

static bool cas_sequence(atomic64_t* ptr, uint64_t expected, uint64_t desired) {
    return atomic_compare_exchange_strong(ptr, &expected, desired);
}

static uint64_t get_next_sequence(atomic64_t* sequence) {
    return atomic_fetch_add(sequence, 1);
}

// ========================================
// GESTION POOL MÉMOIRE PARTAGÉ
// ========================================

static int ipc_shared_pool_init(ipc_shared_pool_t* pool) {
    pool->total_size = IPC_SHARED_POOL_SIZE;
    pool->page_count = pool->total_size / PAGE_SIZE;
    
    // Allouer pages physiques contiguës
    uint64_t phys_pages = pmm_alloc_pages(pool->page_count);
    if (!phys_pages) {
        kerror("Failed to allocate IPC shared pool");
        return -OR_ENOMEM;
    }
    
    pool->base_phys = phys_pages;
    pool->base_virt = phys_pages; // Identity mapping pour simplifier
    
    // Allouer bitmap d'allocation
    size_t bitmap_size = (pool->page_count + 63) / 64 * sizeof(uint64_t);
    pool->allocation_bitmap = (atomic64_t*)kmalloc(bitmap_size);
    if (!pool->allocation_bitmap) {
        pmm_free_pages(phys_pages, pool->page_count);
        return -OR_ENOMEM;
    }
    
    // Initialiser bitmap (toutes les pages libres)
    memset(pool->allocation_bitmap, 0, bitmap_size);
    
    spinlock_init(&pool->alloc_lock);
    
    kdebug("IPC shared pool initialized: %llu pages at 0x%p", 
           (unsigned long long)pool->page_count, (void*)pool->base_phys);
    
    return OR_OK;
}

static uint64_t ipc_shared_alloc_page(ipc_shared_pool_t* pool) {
    spinlock_lock(&pool->alloc_lock);
    
    // Chercher une page libre dans le bitmap
    for (uint64_t i = 0; i < pool->page_count; i++) {
        uint64_t word_idx = i / 64;
        uint64_t bit_idx = i % 64;
        
        uint64_t word = atomic_load(&pool->allocation_bitmap[word_idx]);
        if (!(word & (1ULL << bit_idx))) {
            // Page libre trouvée
            atomic_fetch_or(&pool->allocation_bitmap[word_idx], 1ULL << bit_idx);
            spinlock_unlock(&pool->alloc_lock);
            
            uint64_t page_phys = pool->base_phys + i * PAGE_SIZE;
            kdebug("Allocated shared page %llu at 0x%p", 
                   (unsigned long long)i, (void*)page_phys);
            return page_phys;
        }
    }
    
    spinlock_unlock(&pool->alloc_lock);
    return 0; // Pas de page libre
}

static void ipc_shared_free_page(ipc_shared_pool_t* pool, uint64_t page_phys) {
    if (page_phys < pool->base_phys || 
        page_phys >= pool->base_phys + pool->total_size) {
        return;
    }
    
    uint64_t page_idx = (page_phys - pool->base_phys) / PAGE_SIZE;
    uint64_t word_idx = page_idx / 64;
    uint64_t bit_idx = page_idx % 64;
    
    spinlock_lock(&pool->alloc_lock);
    atomic_fetch_and(&pool->allocation_bitmap[word_idx], ~(1ULL << bit_idx));
    spinlock_unlock(&pool->alloc_lock);
    
    kdebug("Freed shared page %llu at 0x%p", 
           (unsigned long long)page_idx, (void*)page_phys);
}

// ========================================
// GESTION QUEUES LOCK-FREE
// ========================================

static ipc_msg_queue_t* ipc_queue_create(void) {
    ipc_msg_queue_t* queue = (ipc_msg_queue_t*)kmalloc(sizeof(ipc_msg_queue_t));
    if (!queue) {
        return NULL;
    }
    
    memset(queue, 0, sizeof(ipc_msg_queue_t));
    
    // Initialiser les index atomiques
    atomic_store(&queue->head, 0);
    atomic_store(&queue->tail, 0);
    atomic_store(&queue->committed, 0);
    
    // Initialiser les slots
    for (int i = 0; i < MAX_MSG_QUEUE_SIZE; i++) {
        atomic_store(&queue->slots[i].sequence, i);
    }
    
    return queue;
}

static bool ipc_queue_send(ipc_msg_queue_t* queue, ipc_msg_slot_t* msg) {
    uint64_t head = atomic_load(&queue->head);
    
    for (;;) {
        ipc_msg_slot_t* slot = &queue->slots[head % MAX_MSG_QUEUE_SIZE];
        uint64_t seq = atomic_load(&slot->sequence);
        
        int64_t diff = (int64_t)seq - (int64_t)head;
        
        if (diff == 0) {
            // Slot disponible, essayer de le réserver
            if (cas_sequence(&queue->head, head, head + 1)) {
                // Copier le message dans le slot
                slot->sender_port = msg->sender_port;
                slot->flags = msg->flags;
                slot->data_size = msg->data_size;
                slot->timestamp = msg->timestamp;
                slot->page_phys = msg->page_phys;
                slot->offset = msg->offset;
                slot->cap_count = msg->cap_count;
                
                for (uint32_t i = 0; i < msg->cap_count && i < 8; i++) {
                    slot->transferred_caps[i] = msg->transferred_caps[i];
                }
                
                // Publier le message
                atomic_store(&slot->sequence, head + 1);
                return true;
            }
        } else if (diff < 0) {
            // Queue pleine
            return false;
        } else {
            // Retry avec nouveau head
            head = atomic_load(&queue->head);
        }
    }
}

static bool ipc_queue_recv(ipc_msg_queue_t* queue, ipc_msg_slot_t* msg) {
    uint64_t tail = atomic_load(&queue->tail);
    
    for (;;) {
        ipc_msg_slot_t* slot = &queue->slots[tail % MAX_MSG_QUEUE_SIZE];
        uint64_t seq = atomic_load(&slot->sequence);
        
        int64_t diff = (int64_t)seq - (int64_t)(tail + 1);
        
        if (diff == 0) {
            // Message disponible
            if (cas_sequence(&queue->tail, tail, tail + 1)) {
                // Copier le message
                *msg = *slot;
                
                // Libérer le slot
                atomic_store(&slot->sequence, tail + MAX_MSG_QUEUE_SIZE);
                return true;
            }
        } else if (diff < 0) {
            // Pas de message
            return false;
        } else {
            // Retry avec nouveau tail
            tail = atomic_load(&queue->tail);
        }
    }
}

// ========================================
// GESTION PORTS IPC
// ========================================

static ipc_port_t* ipc_find_port(or_cap_t cap_id) {
    if (!g_ipc_registry) return NULL;
    
    uint32_t hash = hash_capability(cap_id);
    
    // Recherche linéaire à partir du hash
    for (uint32_t i = 0; i < MAX_IPC_PORTS; i++) {
        uint32_t idx = (hash + i) % MAX_IPC_PORTS;
        ipc_port_t* port = &g_ipc_registry->ports[idx];
        
        if (port->cap_id == cap_id && 
            atomic_load(&port->state) == IPC_PORT_STATE_ACTIVE) {
            return port;
        }
        
        if (port->cap_id == 0) {
            break; // Slot vide = fin de la chaîne
        }
    }
    
    return NULL;
}

// ========================================
// API PUBLIQUE IPC
// ========================================

void ipc_init(void) {
    kinfo("Initializing high-performance IPC system");
    
    // Allouer le registre global
    g_ipc_registry = (ipc_registry_t*)kmalloc(sizeof(ipc_registry_t));
    if (!g_ipc_registry) {
        kerror("Failed to allocate IPC registry");
        return;
    }
    
    memset(g_ipc_registry, 0, sizeof(ipc_registry_t));
    
    // Initialiser le registre
    atomic_store(&g_ipc_registry->next_cap_id, 1000); // Commencer à 1000
    spinlock_init(&g_ipc_registry->registry_lock);
    
    // Initialiser le pool partagé
    if (ipc_shared_pool_init(&g_ipc_registry->shared_pool) != OR_OK) {
        kfree(g_ipc_registry);
        g_ipc_registry = NULL;
        kerror("Failed to initialize IPC shared pool");
        return;
    }
    
    // Initialiser tous les ports
    for (int i = 0; i < MAX_IPC_PORTS; i++) {
        ipc_port_t* port = &g_ipc_registry->ports[i];
        memset(port, 0, sizeof(ipc_port_t));
        atomic_store(&port->ref_count, 0);
        atomic_store(&port->state, IPC_PORT_STATE_CLOSED);
        spinlock_init(&port->waiters_lock);
    }
    
    // Initialiser la hash table
    memset(g_ipc_registry->port_hash_table, 0xFF, sizeof(g_ipc_registry->port_hash_table));
    
    ipc_initialized = true;
    
    kinfo("IPC system initialized:");
    kinfo("  Max ports: %d", MAX_IPC_PORTS);
    kinfo("  Max message size: %d KB", MAX_MSG_SIZE / 1024);
    kinfo("  Shared pool: %d MB", IPC_SHARED_POOL_SIZE / (1024 * 1024));
    kinfo("  Queue size: %d messages", MAX_MSG_QUEUE_SIZE);
}

or_cap_t ipc_port_create(uint64_t owner_pid) {
    if (!ipc_initialized || !g_ipc_registry) {
        return 0;
    }
    
    spinlock_lock(&g_ipc_registry->registry_lock);
    
    // Chercher un slot libre
    ipc_port_t* free_port = NULL;
    uint32_t free_idx = 0;
    
    for (uint32_t i = 0; i < MAX_IPC_PORTS; i++) {
        ipc_port_t* port = &g_ipc_registry->ports[i];
        if (atomic_load(&port->state) == IPC_PORT_STATE_CLOSED && port->cap_id == 0) {
            free_port = port;
            free_idx = i;
            break;
        }
    }
    
    if (!free_port) {
        spinlock_unlock(&g_ipc_registry->registry_lock);
        kerror("No free IPC port slots");
        return 0;
    }
    
    // Générer nouveau capability ID
    or_cap_t new_cap_id = atomic_fetch_add(&g_ipc_registry->next_cap_id, 1);
    
    // Initialiser le port
    memset(free_port, 0, sizeof(ipc_port_t));
    free_port->cap_id = new_cap_id;
    free_port->owner_pid = owner_pid;
    free_port->permissions = 0xFFFFFFFFFFFFFFFFULL; // Toutes permissions pour le propriétaire
    free_port->max_queue_size = MAX_MSG_QUEUE_SIZE;
    free_port->max_msg_size = MAX_MSG_SIZE;
    free_port->created_time = arch_get_timestamp();
    
    // Créer les queues
    free_port->send_queue = ipc_queue_create();
    free_port->recv_queue = ipc_queue_create();
    
    if (!free_port->send_queue || !free_port->recv_queue) {
        if (free_port->send_queue) kfree(free_port->send_queue);
        if (free_port->recv_queue) kfree(free_port->recv_queue);
        spinlock_unlock(&g_ipc_registry->registry_lock);
        return 0;
    }
    
    // Initialiser atomiques
    atomic_store(&free_port->ref_count, 1);
    atomic_store(&free_port->state, IPC_PORT_STATE_ACTIVE);
    atomic_store(&free_port->msgs_sent, 0);
    atomic_store(&free_port->msgs_received, 0);
    atomic_store(&free_port->bytes_transferred, 0);
    
    spinlock_init(&free_port->waiters_lock);
    
    // Ajouter à la hash table
    uint32_t hash = hash_capability(new_cap_id);
    g_ipc_registry->port_hash_table[hash] = free_idx;
    
    // Mettre à jour les statistiques
    atomic_fetch_add(&g_ipc_registry->total_ports_created, 1);
    atomic_fetch_add(&g_ipc_registry->active_ports, 1);
    
    spinlock_unlock(&g_ipc_registry->registry_lock);
    
    kdebug("Created IPC port cap=%llu for PID %llu (slot %u)", 
           (unsigned long long)new_cap_id, (unsigned long long)owner_pid, free_idx);
    
    return new_cap_id;
}

// Envoyer un message IPC avec zero-copy
int ipc_send_message(or_cap_t port_cap, const void* data, size_t size, uint64_t timeout_ns) {
    if (!ipc_initialized || !g_ipc_registry || size > MAX_MSG_SIZE) {
        return -OR_EINVAL;
    }
    
    // Trouver le port de destination
    ipc_port_t* port = ipc_find_port(port_cap);
    if (!port) {
        return -OR_ENOENT;
    }
    
    // Vérifier permissions
    process_t* current = scheduler_get_current_process();
    if (!current) {
        return -OR_EPERM;
    }
    
    kdebug("IPC send: port=%llu, size=%llu", 
           (unsigned long long)port_cap, (unsigned long long)size);
    
    // Préparer le message
    ipc_msg_slot_t msg = {0};
    msg.sender_port = 0; // TODO: Port expéditeur
    msg.flags = (size > PAGE_SIZE) ? IPC_MSG_FLAG_ZERO_COPY : 0;
    msg.data_size = (uint32_t)size;
    msg.timestamp = arch_get_timestamp();
    msg.cap_count = 0;
    
    // Allocation mémoire selon la taille
    if (msg.flags & IPC_MSG_FLAG_ZERO_COPY) {
        // Zero-copy: allouer page partagée
        msg.page_phys = ipc_shared_alloc_page(&g_ipc_registry->shared_pool);
        if (!msg.page_phys) {
            return -OR_ENOMEM;
        }
        
        msg.offset = 0;
        
        // Copier données vers la page partagée
        void* shared_addr = (void*)msg.page_phys; // Identity mapping
        memcpy(shared_addr, data, size);
        
        kdebug("IPC zero-copy: allocated shared page 0x%p", (void*)msg.page_phys);
    } else {
        // Copie classique (petits messages)
        msg.page_phys = 0;
        msg.offset = 0;
        // Pour les petits messages, on copie dans le slot directement
    }
    
    // Essayer d'envoyer le message
    uint64_t start_time = arch_get_timestamp();
    bool sent = false;
    
    while (!sent) {
        sent = ipc_queue_send(port->recv_queue, &msg);
        
        if (!sent) {
            // Queue pleine, vérifier timeout
            if (timeout_ns > 0) {
                uint64_t elapsed = arch_get_timestamp() - start_time;
                if (elapsed >= timeout_ns) {
                    // Timeout, libérer les ressources
                    if (msg.page_phys) {
                        ipc_shared_free_page(&g_ipc_registry->shared_pool, msg.page_phys);
                    }
                    return -OR_ETIMEDOUT;
                }
            }
            
            // Attendre un peu avant retry
            scheduler_sleep_ns(1000); // 1µs
        }
    }
    
    // Réveiller les threads en attente de réception
    spinlock_lock(&port->waiters_lock);
    if (port->waiting_receivers) {
        thread_t* thread = port->waiting_receivers;
        port->waiting_receivers = thread->next;
        scheduler_wakeup_process(thread->parent_process);
    }
    spinlock_unlock(&port->waiters_lock);
    
    // Mettre à jour les statistiques
    atomic_fetch_add(&port->msgs_sent, 1);
    atomic_fetch_add(&port->bytes_transferred, size);
    atomic_fetch_add(&g_ipc_registry->total_msgs_sent, 1);
    
    kdebug("IPC message sent successfully: %llu bytes", (unsigned long long)size);
    return OR_OK;
}

// Recevoir un message IPC
int ipc_recv_message(or_cap_t port_cap, void* buffer, size_t buffer_size, uint64_t timeout_ns) {
    if (!ipc_initialized || !g_ipc_registry || !buffer) {
        return -OR_EINVAL;
    }
    
    // Trouver le port
    ipc_port_t* port = ipc_find_port(port_cap);
    if (!port) {
        return -OR_ENOENT;
    }
    
    // Vérifier permissions
    process_t* current = scheduler_get_current_process();
    if (!current || current->pid != port->owner_pid) {
        return -OR_EPERM;
    }
    
    kdebug("IPC recv: port=%llu, buffer_size=%llu", 
           (unsigned long long)port_cap, (unsigned long long)buffer_size);
    
    // Essayer de recevoir un message
    ipc_msg_slot_t msg;
    uint64_t start_time = arch_get_timestamp();
    bool received = false;
    
    while (!received) {
        received = ipc_queue_recv(port->recv_queue, &msg);
        
        if (!received) {
            // Pas de message, vérifier timeout
            if (timeout_ns > 0) {
                uint64_t elapsed = arch_get_timestamp() - start_time;
                if (elapsed >= timeout_ns) {
                    return -OR_ETIMEDOUT;
                }
            }
            
            // Se mettre en attente
            thread_t* current_thread = scheduler_get_current_thread();
            if (current_thread) {
                spinlock_lock(&port->waiters_lock);
                current_thread->next = port->waiting_receivers;
                port->waiting_receivers = current_thread;
                spinlock_unlock(&port->waiters_lock);
                
                scheduler_block_current_process();
                scheduler_sleep_ns(1000); // 1µs
            }
        }
    }
    
    // Vérifier que le buffer est assez grand
    if (buffer_size < msg.data_size) {
        // Libérer les ressources
        if (msg.page_phys) {
            ipc_shared_free_page(&g_ipc_registry->shared_pool, msg.page_phys);
        }
        return -OR_EINVAL;
    }
    
    // Copier les données
    if (msg.flags & IPC_MSG_FLAG_ZERO_COPY) {
        // Zero-copy: mapper la page dans l'espace utilisateur
        void* shared_addr = (void*)msg.page_phys;
        memcpy(buffer, (char*)shared_addr + msg.offset, msg.data_size);
        
        // Libérer la page partagée
        ipc_shared_free_page(&g_ipc_registry->shared_pool, msg.page_phys);
        
        kdebug("IPC zero-copy recv: freed shared page 0x%p", (void*)msg.page_phys);
    } else {
        // Les données sont déjà dans le slot (petits messages)
        // TODO: Implémenter copie depuis slot
        memset(buffer, 0, msg.data_size); // Placeholder
    }
    
    // Mettre à jour les statistiques
    atomic_fetch_add(&port->msgs_received, 1);
    
    kdebug("IPC message received: %u bytes", msg.data_size);
    return (int)msg.data_size;
}

// Détruire un port IPC
void ipc_port_destroy(or_cap_t port_cap) {
    if (!ipc_initialized || !g_ipc_registry || port_cap == 0) {
        return;
    }
    
    ipc_port_t* port = ipc_find_port(port_cap);
    if (!port) {
        return;
    }
    
    // Marquer le port comme fermé
    atomic_store(&port->state, IPC_PORT_STATE_CLOSING);
    
    // Réveiller tous les threads en attente
    spinlock_lock(&port->waiters_lock);
    
    thread_t* sender = port->waiting_senders;
    while (sender) {
        thread_t* next = sender->next;
        scheduler_wakeup_process(sender->parent_process);
        sender = next;
    }
    
    thread_t* receiver = port->waiting_receivers;
    while (receiver) {
        thread_t* next = receiver->next;
        scheduler_wakeup_process(receiver->parent_process);
        receiver = next;
    }
    
    port->waiting_senders = NULL;
    port->waiting_receivers = NULL;
    spinlock_unlock(&port->waiters_lock);
    
    // Libérer les queues
    if (port->send_queue) {
        kfree(port->send_queue);
        port->send_queue = NULL;
    }
    
    if (port->recv_queue) {
        kfree(port->recv_queue);
        port->recv_queue = NULL;
    }
    
    // Nettoyer le port
    atomic_store(&port->state, IPC_PORT_STATE_CLOSED);
    port->cap_id = 0;
    port->owner_pid = 0;
    
    // Mettre à jour les statistiques
    atomic_fetch_sub(&g_ipc_registry->active_ports, 1);
    
    kdebug("Destroyed IPC port %llu", (unsigned long long)port_cap);
}
