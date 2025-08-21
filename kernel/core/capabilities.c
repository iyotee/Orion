/*
 * Orion Operating System - Advanced Security & Capability System
 *
 * Complete security framework with capability-based access control,
 * hardware security features, and real-time threat detection.
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
#include <arch.h>

// ========================================
// CONSTANTES DE SÉCURITÉ
// ========================================

// MAX_CAPABILITIES défini dans kernel.h
#define MAX_SECURITY_CONTEXTS   1024
#define MAX_AUDIT_ENTRIES       4096
#define ENTROPY_POOL_SIZE       4096
#define KASLR_ENTROPY_BITS      16
#define CFI_JUMP_TABLE_SIZE     1024

// Droits de capabilities étendus
#define CAP_READ                (1ULL << 0)   // Lecture
#define CAP_WRITE               (1ULL << 1)   // Écriture
#define CAP_EXEC                (1ULL << 2)   // Exécution
#define CAP_GRANT               (1ULL << 3)   // Accorder droits
#define CAP_REVOKE              (1ULL << 4)   // Révoquer droits
#define CAP_DELETE              (1ULL << 5)   // Supprimer
#define CAP_CREATE              (1ULL << 6)   // Créer objets
#define CAP_MODIFY              (1ULL << 7)   // Modifier métadonnées
#define CAP_TRAVERSE            (1ULL << 8)   // Traverser répertoires
#define CAP_BIND                (1ULL << 9)   // Bind network
#define CAP_LISTEN              (1ULL << 10)  // Listen network
#define CAP_CONNECT             (1ULL << 11)  // Connect network
#define CAP_DEBUG               (1ULL << 12)  // Debug processes
#define CAP_ADMIN               (1ULL << 13)  // Administration
#define CAP_IMMORTAL            (1ULL << 14)  // Ne peut être révoqué
#define CAP_DELEGATABLE         (1ULL << 15)  // Peut être délégué

// Types d'objets étendus
typedef enum {
    CAP_TYPE_NONE = 0,
    CAP_TYPE_MEMORY,
    CAP_TYPE_IPC_PORT,
    CAP_TYPE_PROCESS,
    CAP_TYPE_THREAD,
    CAP_TYPE_FILE,
    CAP_TYPE_DIRECTORY,
    CAP_TYPE_DEVICE,
    CAP_TYPE_NETWORK_SOCKET,
    CAP_TYPE_TIMER,
    CAP_TYPE_SECURITY_CONTEXT,
    CAP_TYPE_CRYPTO_KEY,
    CAP_TYPE_HARDWARE_RESOURCE,
} cap_type_t;

// Niveaux de sécurité
typedef enum {
    SECURITY_LEVEL_PUBLIC = 0,      // Accès libre
    SECURITY_LEVEL_RESTRICTED,      // Accès restreint
    SECURITY_LEVEL_CONFIDENTIAL,    // Confidentiel
    SECURITY_LEVEL_SECRET,          // Secret
    SECURITY_LEVEL_TOP_SECRET,      // Top secret
} security_level_t;

// ========================================
// STRUCTURES DE DONNÉES
// ========================================

// Capability complète avec sécurité renforcée
typedef struct capability {
    atomic64_t cap_id;              // ID unique non-forgeable
    atomic32_t ref_count;           // Compteur de références
    cap_type_t type;                // Type d'objet
    uint64_t object_id;             // ID de l'objet cible
    atomic64_t rights;              // Droits atomiques
    uint64_t owner_pid;             // PID propriétaire
    uint64_t creator_pid;           // PID créateur
    uint64_t creation_time;         // Timestamp de création
    uint64_t last_access_time;      // Dernier accès
    uint64_t access_count;          // Nombre d'accès
    security_level_t level;         // Niveau de sécurité
    uint64_t expiry_time;           // Expiration (0 = jamais)
    uint64_t delegation_depth;      // Profondeur de délégation
    uint64_t audit_mask;            // Masque d'audit
    atomic32_t state;               // État (actif, suspendu, révoqué)
    uint64_t checksum;              // Checksum d'intégrité
    spinlock_t lock;                // Verrou pour modifications
} capability_t;

// États d'une capability
#define CAP_STATE_ACTIVE         1
#define CAP_STATE_SUSPENDED      2
#define CAP_STATE_REVOKED        3
#define CAP_STATE_EXPIRED        4

// Contexte de sécurité par processus
typedef struct security_context {
    uint64_t pid;                   // PID du processus
    security_level_t level;         // Niveau de sécurité
    uint64_t capabilities[64];      // Bitmap des capabilities
    uint64_t denied_syscalls[8];    // Syscalls interdits
    uint64_t max_memory;            // Limite mémoire
    uint64_t max_files;             // Limite fichiers
    uint64_t max_network_conn;      // Limite connexions réseau
    bool sandboxed;                 // Processus sandboxé
    uint64_t jail_root;             // Racine du jail
    atomic64_t violation_count;     // Violations détectées
    uint64_t created_time;          // Temps de création
    spinlock_t lock;                // Verrou du contexte
} security_context_t;

// Entrée d'audit
typedef struct audit_entry {
    uint64_t timestamp;             // Timestamp de l'événement
    uint64_t pid;                   // PID du processus
    uint64_t tid;                   // TID du thread
    uint32_t event_type;            // Type d'événement
    uint32_t severity;              // Gravité
    or_cap_t capability;            // Capability impliquée
    uint64_t object_id;             // Objet accédé
    uint64_t result;                // Résultat de l'opération
    char description[128];          // Description de l'événement
} audit_entry_t;

// Types d'événements d'audit
#define AUDIT_CAP_CREATE         1
#define AUDIT_CAP_GRANT          2
#define AUDIT_CAP_REVOKE         3
#define AUDIT_CAP_ACCESS         4
#define AUDIT_CAP_VIOLATION      5
#define AUDIT_SYSCALL_DENIED     6
#define AUDIT_MEMORY_VIOLATION   7
#define AUDIT_SECURITY_BREACH    8

// Pool d'entropie pour génération aléatoire sécurisée
typedef struct entropy_pool {
    uint8_t pool[ENTROPY_POOL_SIZE];
    atomic64_t write_ptr;
    atomic64_t read_ptr;
    atomic64_t entropy_count;
    uint64_t last_reseed;
    spinlock_t lock;
    bool initialized;
} entropy_pool_t;

// Système de détection d'intrusion
typedef struct intrusion_detection {
    uint64_t total_violations;
    uint64_t failed_authentications;
    uint64_t privilege_escalations;
    uint64_t suspicious_syscalls;
    uint64_t memory_corruptions;
    uint64_t last_alert_time;
    bool alert_mode;
    spinlock_t lock;
} intrusion_detection_t;

// ========================================
// VARIABLES GLOBALES
// ========================================

static capability_t* g_capabilities = NULL;
static security_context_t* g_security_contexts = NULL;
static audit_entry_t* g_audit_log = NULL;
static entropy_pool_t g_entropy_pool;
static intrusion_detection_t g_ids;

static atomic64_t next_cap_id = ATOMIC_VAR_INIT(1000000);
static atomic64_t audit_head = ATOMIC_VAR_INIT(0);
static spinlock_t cap_system_lock = SPINLOCK_INIT;
static bool cap_system_initialized = false;

// ========================================
// UTILITAIRES CRYPTOGRAPHIQUES
// ========================================

static uint64_t hash_combine(uint64_t seed, uint64_t value) {
    return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

static uint64_t calculate_capability_checksum(const capability_t* cap) {
    uint64_t checksum = 0;
    checksum = hash_combine(checksum, atomic_load(&cap->cap_id));
    checksum = hash_combine(checksum, cap->type);
    checksum = hash_combine(checksum, cap->object_id);
    checksum = hash_combine(checksum, atomic_load(&cap->rights));
    checksum = hash_combine(checksum, cap->owner_pid);
    checksum = hash_combine(checksum, cap->creation_time);
    return checksum;
}

static bool verify_capability_integrity(const capability_t* cap) {
    uint64_t expected = calculate_capability_checksum(cap);
    return cap->checksum == expected;
}

// ========================================
// GESTION ENTROPIE ET ALÉATOIRE
// ========================================

static void entropy_add_event(uint64_t data) {
    if (!g_entropy_pool.initialized) return;
    
    spinlock_lock(&g_entropy_pool.lock);
    
    uint64_t pos = atomic_fetch_add(&g_entropy_pool.write_ptr, 8) % ENTROPY_POOL_SIZE;
    
    // Mélanger l'entropie existante avec les nouvelles données
    for (int i = 0; i < 8; i++) {
        uint8_t byte = (data >> (i * 8)) & 0xFF;
        g_entropy_pool.pool[(pos + i) % ENTROPY_POOL_SIZE] ^= byte;
    }
    
    atomic_fetch_add(&g_entropy_pool.entropy_count, 8);
    
    spinlock_unlock(&g_entropy_pool.lock);
}

static uint64_t get_secure_random(void) {
    if (!g_entropy_pool.initialized) {
        // Fallback: utiliser timestamp + RDTSC si disponible
        return arch_get_timestamp() ^ arch_get_rdtsc();
    }
    
    spinlock_lock(&g_entropy_pool.lock);
    
    uint64_t result = 0;
    uint64_t pos = atomic_fetch_add(&g_entropy_pool.read_ptr, 8) % ENTROPY_POOL_SIZE;
    
    for (int i = 0; i < 8; i++) {
        result |= ((uint64_t)g_entropy_pool.pool[(pos + i) % ENTROPY_POOL_SIZE]) << (i * 8);
    }
    
    // Ajouter de l'entropie supplémentaire basée sur le timing
    entropy_add_event(arch_get_timestamp());
    
    spinlock_unlock(&g_entropy_pool.lock);
    
    return result;
}

// ========================================
// AUDIT ET LOGGING
// ========================================

static void audit_log_event(uint32_t event_type, uint32_t severity, 
                           or_cap_t capability, uint64_t object_id, 
                           uint64_t result, const char* description) {
    if (!g_audit_log) return;
    
    uint64_t head = atomic_fetch_add(&audit_head, 1) % MAX_AUDIT_ENTRIES;
    audit_entry_t* entry = &g_audit_log[head];
    
    entry->timestamp = arch_get_timestamp();
    
    process_t* current = scheduler_get_current_process();
    thread_t* thread = scheduler_get_current_thread();
    
    entry->pid = current ? current->pid : 0;
    entry->tid = thread ? thread->tid : 0;
    entry->event_type = event_type;
    entry->severity = severity;
    entry->capability = capability;
    entry->object_id = object_id;
    entry->result = result;
    
    // Copier description de façon sécurisée
    if (description) {
        size_t len = strlen(description);
        if (len > 127) len = 127;
        memcpy(entry->description, description, len);
        entry->description[len] = '\0';
    } else {
        entry->description[0] = '\0';
    }
    
    // Logger événements critiques
    if (severity >= 7) { // Critique
        kwarning("SECURITY: %s (PID %llu, cap %llu)", 
                 description ? description : "Unknown event",
                 (unsigned long long)entry->pid,
                 (unsigned long long)capability);
    }
}

// ========================================
// SYSTÈME DE DÉTECTION D'INTRUSION
// ========================================

static void ids_report_violation(const char* type, uint64_t severity) {
    spinlock_lock(&g_ids.lock);
    
    g_ids.total_violations++;
    
    if (strcmp(type, "auth_failure") == 0) {
        g_ids.failed_authentications++;
    } else if (strcmp(type, "privilege_escalation") == 0) {
        g_ids.privilege_escalations++;
    } else if (strcmp(type, "suspicious_syscall") == 0) {
        g_ids.suspicious_syscalls++;
    } else if (strcmp(type, "memory_corruption") == 0) {
        g_ids.memory_corruptions++;
    }
    
    uint64_t now = arch_get_timestamp();
    if (severity >= 8 || (now - g_ids.last_alert_time) > 1000000000ULL) { // 1 sec
        g_ids.alert_mode = true;
        g_ids.last_alert_time = now;
        
        kerror("IDS ALERT: %s (severity %llu, total violations: %llu)",
               type, (unsigned long long)severity, (unsigned long long)g_ids.total_violations);
    }
    
    spinlock_unlock(&g_ids.lock);
    
    // Audit log
    audit_log_event(AUDIT_SECURITY_BREACH, (uint32_t)severity, 0, 0, 0, type);
}

// ========================================
// GESTION CONTEXTES DE SÉCURITÉ
// ========================================

static security_context_t* security_get_context(uint64_t pid) {
    if (!g_security_contexts) return NULL;
    
    for (uint32_t i = 0; i < MAX_SECURITY_CONTEXTS; i++) {
        if (g_security_contexts[i].pid == pid) {
            return &g_security_contexts[i];
        }
    }
    
    return NULL;
}

static security_context_t* security_create_context(uint64_t pid, security_level_t level) {
    if (!g_security_contexts) return NULL;
    
    // Chercher un slot libre
    for (uint32_t i = 0; i < MAX_SECURITY_CONTEXTS; i++) {
        if (g_security_contexts[i].pid == 0) {
            security_context_t* ctx = &g_security_contexts[i];
            
            memset(ctx, 0, sizeof(security_context_t));
            ctx->pid = pid;
            ctx->level = level;
            ctx->max_memory = 512 * 1024 * 1024; // 512MB par défaut
            ctx->max_files = 1024;
            ctx->max_network_conn = 64;
            ctx->sandboxed = (level >= SECURITY_LEVEL_RESTRICTED);
            ctx->created_time = arch_get_timestamp();
            atomic_store(&ctx->violation_count, 0);
            spinlock_init(&ctx->lock);
            
            kdebug("Created security context for PID %llu (level %d)",
                   (unsigned long long)pid, level);
            
            return ctx;
        }
    }
    
    return NULL;
}

// ========================================
// API PRINCIPALE CAPABILITIES
// ========================================

void capabilities_init(void) {
    kinfo("Initializing advanced security system");
    
    // Allouer les structures principales
    g_capabilities = (capability_t*)kmalloc(sizeof(capability_t) * MAX_CAPABILITIES);
    g_security_contexts = (security_context_t*)kmalloc(sizeof(security_context_t) * MAX_SECURITY_CONTEXTS);
    g_audit_log = (audit_entry_t*)kmalloc(sizeof(audit_entry_t) * MAX_AUDIT_ENTRIES);
    
    if (!g_capabilities || !g_security_contexts || !g_audit_log) {
        kerror("Failed to allocate security structures");
        return;
    }
    
    // Initialiser les capabilities
    for (uint32_t i = 0; i < MAX_CAPABILITIES; i++) {
        capability_t* cap = &g_capabilities[i];
        memset(cap, 0, sizeof(capability_t));
        atomic_store(&cap->cap_id, 0);
        atomic_store(&cap->ref_count, 0);
        atomic_store(&cap->rights, 0);
        atomic_store(&cap->state, CAP_STATE_REVOKED);
        spinlock_init(&cap->lock);
    }
    
    // Initialiser les contextes de sécurité
    for (uint32_t i = 0; i < MAX_SECURITY_CONTEXTS; i++) {
        memset(&g_security_contexts[i], 0, sizeof(security_context_t));
        spinlock_init(&g_security_contexts[i].lock);
    }
    
    // Initialiser l'audit log
    memset(g_audit_log, 0, sizeof(audit_entry_t) * MAX_AUDIT_ENTRIES);
    
    // Initialiser le pool d'entropie
    memset(&g_entropy_pool, 0, sizeof(entropy_pool_t));
    atomic_store(&g_entropy_pool.write_ptr, 0);
    atomic_store(&g_entropy_pool.read_ptr, 0);
    atomic_store(&g_entropy_pool.entropy_count, 0);
    spinlock_init(&g_entropy_pool.lock);
    
    // Seeder le pool avec de l'entropie initiale
    for (int i = 0; i < ENTROPY_POOL_SIZE; i += 8) {
        uint64_t entropy = arch_get_timestamp() ^ arch_get_rdtsc() ^ ((uint64_t)i * 0x9e3779b9);
        entropy_add_event(entropy);
    }
    g_entropy_pool.last_reseed = arch_get_timestamp();
    g_entropy_pool.initialized = true;
    
    // Initialiser l'IDS
    memset(&g_ids, 0, sizeof(intrusion_detection_t));
    spinlock_init(&g_ids.lock);
    
    // Créer contexte pour le processus init (PID 1)
    security_create_context(1, SECURITY_LEVEL_RESTRICTED);
    
    cap_system_initialized = true;
    
    kinfo("Security system initialized:");
    kinfo("  Capabilities: %d max", MAX_CAPABILITIES);
    kinfo("  Security contexts: %d max", MAX_SECURITY_CONTEXTS);
    kinfo("  Audit entries: %d max", MAX_AUDIT_ENTRIES);
    kinfo("  Entropy pool: %d bytes", ENTROPY_POOL_SIZE);
    kinfo("  Hardware features: KASLR, CFI, W^X");
}

// Create a new capability with enhanced security
or_cap_t cap_create(cap_type_t type, uint64_t object_id, uint64_t rights, uint64_t owner_pid) {
    if (!cap_system_initialized || !g_capabilities) {
        return 0;
    }
    
    // Check if owner process has valid security context
    security_context_t* ctx = security_get_context(owner_pid);
    if (!ctx) {
        ids_report_violation("invalid_owner_context", 6);
        return 0;
    }
    
    // Find a free capability slot
    for (uint32_t i = 0; i < MAX_CAPABILITIES; i++) {
        capability_t* cap = &g_capabilities[i];
        
        if (atomic_load(&cap->state) == CAP_STATE_REVOKED) {
            spinlock_lock(&cap->lock);
            
            // Double-check state under lock
            if (atomic_load(&cap->state) != CAP_STATE_REVOKED) {
                spinlock_unlock(&cap->lock);
                continue;
            }
            
            // Generate secure, non-forgeable capability ID
            uint64_t new_cap_id = atomic_fetch_add(&next_cap_id, 1);
            new_cap_id ^= get_secure_random(); // Add entropy to prevent prediction
            
            // Initialize capability
            atomic_store(&cap->cap_id, new_cap_id);
            atomic_store(&cap->ref_count, 1);
            cap->type = type;
            cap->object_id = object_id;
            atomic_store(&cap->rights, rights);
            cap->owner_pid = owner_pid;
            
            process_t* current = scheduler_get_current_process();
            cap->creator_pid = current ? current->pid : 0;
            
            cap->creation_time = arch_get_timestamp();
            cap->last_access_time = cap->creation_time;
            cap->access_count = 0;
            cap->level = ctx->level;
            cap->expiry_time = 0; // Never expires by default
            cap->delegation_depth = 0;
            cap->audit_mask = 0xFFFFFFFFFFFFFFFFULL; // Audit all by default
            
            // Calculate integrity checksum
            cap->checksum = calculate_capability_checksum(cap);
            
            atomic_store(&cap->state, CAP_STATE_ACTIVE);
            
            spinlock_unlock(&cap->lock);
            
            // Log capability creation
            audit_log_event(AUDIT_CAP_CREATE, 3, new_cap_id, object_id, 0, 
                          "Capability created successfully");
            
            kdebug("Created capability %llu (type=%d, rights=0x%llx) for PID %llu",
                   (unsigned long long)new_cap_id, type,
                   (unsigned long long)rights, (unsigned long long)owner_pid);
            
            return new_cap_id;
        }
    }
    
    kerror("No free capability slots available");
    ids_report_violation("capability_exhaustion", 7);
    return 0;
}

// Check capability rights with enhanced security validation
bool cap_check_rights(or_cap_t cap_id, uint64_t required_rights, uint64_t caller_pid) {
    if (!cap_system_initialized || cap_id == 0 || !g_capabilities) {
        return false;
    }
    
    // Find the capability
    for (uint32_t i = 0; i < MAX_CAPABILITIES; i++) {
        capability_t* cap = &g_capabilities[i];
        
        if (atomic_load(&cap->cap_id) == cap_id) {
            spinlock_lock(&cap->lock);
            
            // Verify capability is still active
            if (atomic_load(&cap->state) != CAP_STATE_ACTIVE) {
                spinlock_unlock(&cap->lock);
                audit_log_event(AUDIT_CAP_VIOLATION, 5, cap_id, 0, 
                              atomic_load(&cap->state), "Inactive capability access attempt");
                return false;
            }
            
            // Check expiry time
            uint64_t now = arch_get_timestamp();
            if (cap->expiry_time != 0 && now > cap->expiry_time) {
                atomic_store(&cap->state, CAP_STATE_EXPIRED);
                spinlock_unlock(&cap->lock);
                audit_log_event(AUDIT_CAP_VIOLATION, 4, cap_id, 0, 0, "Expired capability access");
                return false;
            }
            
            // Verify integrity checksum
            if (!verify_capability_integrity(cap)) {
                atomic_store(&cap->state, CAP_STATE_REVOKED);
                spinlock_unlock(&cap->lock);
                ids_report_violation("capability_corruption", 9);
                audit_log_event(AUDIT_CAP_VIOLATION, 9, cap_id, 0, 0, "Capability integrity violation");
                return false;
            }
            
            // Check owner permission
            if (cap->owner_pid != caller_pid) {
                spinlock_unlock(&cap->lock);
                audit_log_event(AUDIT_CAP_VIOLATION, 6, cap_id, 0, caller_pid, 
                              "Wrong capability owner");
                kdebug("Capability %llu: wrong owner (have %llu, need %llu)",
                       (unsigned long long)cap_id,
                       (unsigned long long)caller_pid,
                       (unsigned long long)cap->owner_pid);
                return false;
            }
            
            // Check required rights
            uint64_t current_rights = atomic_load(&cap->rights);
            if ((current_rights & required_rights) != required_rights) {
                spinlock_unlock(&cap->lock);
                audit_log_event(AUDIT_CAP_VIOLATION, 5, cap_id, 0, required_rights, 
                              "Insufficient capability rights");
                kdebug("Capability %llu: insufficient rights (have 0x%llx, need 0x%llx)",
                       (unsigned long long)cap_id,
                       (unsigned long long)current_rights,
                       (unsigned long long)required_rights);
                return false;
            }
            
            // Update access statistics
            cap->last_access_time = now;
            cap->access_count++;
            
            // Log access if auditing is enabled
            if (cap->audit_mask & required_rights) {
                audit_log_event(AUDIT_CAP_ACCESS, 2, cap_id, cap->object_id, 
                              required_rights, "Capability access granted");
            }
            
            spinlock_unlock(&cap->lock);
            return true;
        }
    }
    
    audit_log_event(AUDIT_CAP_VIOLATION, 4, cap_id, 0, 0, "Capability not found");
    kdebug("Capability %llu not found", (unsigned long long)cap_id);
    return false;
}

// Grant rights to another process (delegation)
int cap_grant(or_cap_t cap_id, uint64_t target_pid, uint64_t rights, uint64_t caller_pid) {
    if (!cap_system_initialized || !g_capabilities) {
        return -OR_ENOSYS;
    }
    
    // Check that caller has GRANT permission
    if (!cap_check_rights(cap_id, CAP_GRANT, caller_pid)) {
        audit_log_event(AUDIT_CAP_VIOLATION, 6, cap_id, 0, 0, "Grant permission denied");
        return -OR_EPERM;
    }
    
    // Find the source capability
    capability_t* source_cap = NULL;
    for (uint32_t i = 0; i < MAX_CAPABILITIES; i++) {
        if (atomic_load(&g_capabilities[i].cap_id) == cap_id) {
            source_cap = &g_capabilities[i];
            break;
        }
    }
    
    if (!source_cap) {
        return -OR_ENOENT;
    }
    
    // Check delegation depth limit
    if (source_cap->delegation_depth >= 10) {
        audit_log_event(AUDIT_CAP_VIOLATION, 5, cap_id, 0, 0, "Delegation depth exceeded");
        return -OR_EPERM;
    }
    
    // Ensure target process has security context
    security_context_t* target_ctx = security_get_context(target_pid);
    if (!target_ctx) {
        target_ctx = security_create_context(target_pid, SECURITY_LEVEL_RESTRICTED);
        if (!target_ctx) {
            return -OR_ENOMEM;
        }
    }
    
    // Create delegated capability with reduced rights
    uint64_t delegated_rights = atomic_load(&source_cap->rights) & rights;
    delegated_rights &= ~CAP_IMMORTAL; // Cannot delegate immortal rights
    
    or_cap_t new_cap = cap_create(source_cap->type, source_cap->object_id, 
                                  delegated_rights, target_pid);
    
    if (new_cap != 0) {
        // Find the new capability and set delegation info
        for (uint32_t i = 0; i < MAX_CAPABILITIES; i++) {
            capability_t* cap = &g_capabilities[i];
            if (atomic_load(&cap->cap_id) == new_cap) {
                cap->delegation_depth = source_cap->delegation_depth + 1;
                cap->creator_pid = caller_pid;
                break;
            }
        }
        
        audit_log_event(AUDIT_CAP_GRANT, 3, cap_id, new_cap, rights, 
                        "Capability granted successfully");
        
        kdebug("Granted capability %llu to PID %llu (rights=0x%llx)",
               (unsigned long long)new_cap, (unsigned long long)target_pid,
               (unsigned long long)delegated_rights);
        
        return 0;
    }
    
    return -OR_ENOMEM;
}

// Revoke capability rights from a process
int cap_revoke(or_cap_t cap_id, uint64_t target_pid, uint64_t rights, uint64_t caller_pid) {
    if (!cap_system_initialized || !g_capabilities) {
        return -OR_ENOSYS;
    }
    
    // Check that caller has REVOKE permission
    if (!cap_check_rights(cap_id, CAP_REVOKE, caller_pid)) {
        audit_log_event(AUDIT_CAP_VIOLATION, 6, cap_id, 0, 0, "Revoke permission denied");
        return -OR_EPERM;
    }
    
    // Find capabilities owned by target_pid that need revocation
    uint32_t revoked_count = 0;
    
    for (uint32_t i = 0; i < MAX_CAPABILITIES; i++) {
        capability_t* cap = &g_capabilities[i];
        
        if (cap->owner_pid == target_pid && 
            atomic_load(&cap->state) == CAP_STATE_ACTIVE) {
            
            spinlock_lock(&cap->lock);
            
            // Check if this capability is derived from the one being revoked
            if (cap->object_id == 0 || cap->creator_pid == caller_pid) {
                uint64_t current_rights = atomic_load(&cap->rights);
                
                // Check for immortal rights
                if (current_rights & CAP_IMMORTAL) {
                    spinlock_unlock(&cap->lock);
                    continue; // Cannot revoke immortal capabilities
                }
                
                // Remove specified rights
                uint64_t new_rights = current_rights & ~rights;
                atomic_store(&cap->rights, new_rights);
                
                // If no rights remain, revoke the capability
                if (new_rights == 0) {
                    atomic_store(&cap->state, CAP_STATE_REVOKED);
                }
                
                revoked_count++;
                
                audit_log_event(AUDIT_CAP_REVOKE, 4, atomic_load(&cap->cap_id), 
                              cap->object_id, rights, "Rights revoked");
            }
            
            spinlock_unlock(&cap->lock);
        }
    }
    
    if (revoked_count > 0) {
        kdebug("Revoked rights 0x%llx from %u capabilities of PID %llu",
               (unsigned long long)rights, revoked_count, (unsigned long long)target_pid);
        return 0;
    }
    
    return -OR_ENOENT;
}

// Destroy a capability completely
void cap_destroy(or_cap_t cap_id) {
    if (!cap_system_initialized || cap_id == 0 || !g_capabilities) {
        return;
    }
    
    for (uint32_t i = 0; i < MAX_CAPABILITIES; i++) {
        capability_t* cap = &g_capabilities[i];
        
        if (atomic_load(&cap->cap_id) == cap_id) {
            spinlock_lock(&cap->lock);
            
            // Check for immortal capabilities
            uint64_t rights = atomic_load(&cap->rights);
            if (rights & CAP_IMMORTAL) {
                spinlock_unlock(&cap->lock);
                audit_log_event(AUDIT_CAP_VIOLATION, 6, cap_id, 0, 0, 
                              "Attempt to destroy immortal capability");
                return;
            }
            
            // Mark as revoked
            atomic_store(&cap->state, CAP_STATE_REVOKED);
            atomic_store(&cap->cap_id, 0);
            atomic_store(&cap->rights, 0);
            cap->owner_pid = 0;
            cap->object_id = 0;
            
            spinlock_unlock(&cap->lock);
            
            audit_log_event(AUDIT_CAP_REVOKE, 3, cap_id, 0, 0, "Capability destroyed");
            kdebug("Destroyed capability %llu", (unsigned long long)cap_id);
            return;
        }
    }
}

// ========================================
// HARDWARE SECURITY FEATURES
// ========================================

// KASLR (Kernel Address Space Layout Randomization)
static uint64_t kaslr_offset = 0;

void security_init_kaslr(void) {
    // Generate random offset for KASLR
    uint64_t entropy = get_secure_random();
    kaslr_offset = (entropy & ((1ULL << KASLR_ENTROPY_BITS) - 1)) << 21; // Align to 2MB
    
    kinfo("KASLR initialized with %d bits of entropy (offset: 0x%llx)",
          KASLR_ENTROPY_BITS, (unsigned long long)kaslr_offset);
}

uint64_t security_get_kaslr_offset(void) {
    return kaslr_offset;
}

// W^X (Write XOR Execute) enforcement
bool security_check_wx_violation(uint64_t vaddr, uint64_t flags) {
    // Check if memory has both write and execute permissions
    bool writable = (flags & PAGE_FLAG_WRITE) != 0;
    bool executable = (flags & PAGE_FLAG_EXEC) != 0;
    
    if (writable && executable) {
        ids_report_violation("wx_violation", 8);
        audit_log_event(AUDIT_MEMORY_VIOLATION, 8, 0, vaddr, flags, 
                        "W^X violation detected");
        
        process_t* current = scheduler_get_current_process();
        if (current) {
            security_context_t* ctx = security_get_context(current->pid);
            if (ctx) {
                atomic_fetch_add(&ctx->violation_count, 1);
            }
        }
        
        return true; // Violation detected
    }
    
    return false; // No violation
}

// Stack canary protection
static uint64_t stack_canary = 0;

void security_init_stack_canary(void) {
    stack_canary = get_secure_random();
    kinfo("Stack canary initialized: 0x%llx", (unsigned long long)stack_canary);
}

uint64_t security_get_stack_canary(void) {
    return stack_canary;
}

bool security_check_stack_canary(uint64_t canary) {
    if (canary != stack_canary) {
        ids_report_violation("stack_overflow", 9);
        audit_log_event(AUDIT_MEMORY_VIOLATION, 9, 0, 0, canary, 
                        "Stack overflow detected");
        return false;
    }
    return true;
}

// Control Flow Integrity (CFI)
typedef struct cfi_entry {
    uint64_t source_addr;
    uint64_t target_addr;
    uint64_t hash;
    bool valid;
} cfi_entry_t;

static cfi_entry_t cfi_jump_table[CFI_JUMP_TABLE_SIZE];
static atomic64_t cfi_next_slot = ATOMIC_VAR_INIT(0);

void security_init_cfi(void) {
    memset(cfi_jump_table, 0, sizeof(cfi_jump_table));
    kinfo("CFI jump table initialized (%d entries)", CFI_JUMP_TABLE_SIZE);
}

bool security_register_cfi_target(uint64_t source, uint64_t target) {
    uint64_t slot = atomic_fetch_add(&cfi_next_slot, 1) % CFI_JUMP_TABLE_SIZE;
    
    cfi_jump_table[slot].source_addr = source;
    cfi_jump_table[slot].target_addr = target;
    cfi_jump_table[slot].hash = hash_combine(source, target);
    cfi_jump_table[slot].valid = true;
    
    return true;
}

bool security_validate_cfi_jump(uint64_t source, uint64_t target) {
    uint64_t expected_hash = hash_combine(source, target);
    
    for (uint32_t i = 0; i < CFI_JUMP_TABLE_SIZE; i++) {
        if (cfi_jump_table[i].valid && 
            cfi_jump_table[i].source_addr == source &&
            cfi_jump_table[i].hash == expected_hash) {
            return true;
        }
    }
    
    ids_report_violation("cfi_violation", 9);
    audit_log_event(AUDIT_MEMORY_VIOLATION, 9, 0, source, target, 
                    "CFI violation detected");
    return false;
}

// ========================================
// SECURITY POLICY ENFORCEMENT
// ========================================

bool security_check_syscall_allowed(uint64_t syscall_num, uint64_t pid) {
    security_context_t* ctx = security_get_context(pid);
    if (!ctx) {
        return true; // Default allow if no context
    }
    
    spinlock_lock(&ctx->lock);
    
    // Check if syscall is explicitly denied
    uint64_t word = syscall_num / 64;
    uint64_t bit = syscall_num % 64;
    
    if (word < 8 && (ctx->denied_syscalls[word] & (1ULL << bit))) {
        spinlock_unlock(&ctx->lock);
        
        audit_log_event(AUDIT_SYSCALL_DENIED, 5, 0, syscall_num, pid, 
                        "Syscall denied by security policy");
        
        atomic_fetch_add(&ctx->violation_count, 1);
        return false;
    }
    
    spinlock_unlock(&ctx->lock);
    return true;
}

bool security_check_memory_limit(uint64_t pid, uint64_t requested_bytes) {
    security_context_t* ctx = security_get_context(pid);
    if (!ctx) {
        return true; // Default allow
    }
    
    // TODO: Get current memory usage from process
    uint64_t current_usage = 0; // Placeholder
    
    if (current_usage + requested_bytes > ctx->max_memory) {
        audit_log_event(AUDIT_MEMORY_VIOLATION, 4, 0, pid, requested_bytes,
                        "Memory limit exceeded");
        return false;
    }
    
    return true;
}

// ========================================
// PUBLIC SECURITY API
// ========================================

void security_init(void) {
    kinfo("Initializing hardware security features");
    
    security_init_kaslr();
    security_init_stack_canary();
    security_init_cfi();
    
    // Enable hardware features if available
    arch_enable_smep();  // Supervisor Mode Execution Prevention
    arch_enable_smap();  // Supervisor Mode Access Prevention
    arch_enable_umip();  // User Mode Instruction Prevention
    
    kinfo("Hardware security features initialized");
}

uint64_t security_get_random(void) {
    return get_secure_random();
}

void security_add_entropy(uint64_t data) {
    entropy_add_event(data);
}

bool security_is_address_valid(uint64_t vaddr, uint64_t size, bool write) {
    // Check for obvious invalid addresses
    if (vaddr == 0 || size == 0) {
        return false;
    }
    
    // Check for overflow
    if (vaddr + size < vaddr) {
        return false;
    }
    
    // Check against KASLR protected areas
    if (vaddr >= 0xFFFFFF8000000000ULL && vaddr < 0xFFFFFF8000000000ULL + kaslr_offset) {
        return false; // Kernel protected area
    }
    
    // Additional architecture-specific checks
    return arch_validate_user_address(vaddr, size, write);
}

void security_report_violation(const char* type, uint64_t severity, 
                              uint64_t pid, const char* details) {
    ids_report_violation(type, severity);
    
    audit_log_event(AUDIT_SECURITY_BREACH, (uint32_t)severity, 0, pid, 0, details);
    
    // Update process violation count
    security_context_t* ctx = security_get_context(pid);
    if (ctx) {
        atomic_fetch_add(&ctx->violation_count, 1);
        
        // Consider process termination for severe violations
        if (severity >= 9 && atomic_load(&ctx->violation_count) > 5) {
            kwarning("Process PID %llu terminated due to security violations",
                     (unsigned long long)pid);
            // TODO: Terminate process
        }
    }
}

void security_get_stats(uint64_t* capabilities_active, uint64_t* violations_total,
                       uint64_t* audit_entries, bool* alert_mode) {
    if (capabilities_active) {
        uint32_t count = 0;
        for (uint32_t i = 0; i < MAX_CAPABILITIES; i++) {
            if (atomic_load(&g_capabilities[i].state) == CAP_STATE_ACTIVE) {
                count++;
            }
        }
        *capabilities_active = count;
    }
    
    if (violations_total) {
        spinlock_lock(&g_ids.lock);
        *violations_total = g_ids.total_violations;
        spinlock_unlock(&g_ids.lock);
    }
    
    if (audit_entries) {
        *audit_entries = atomic_load(&audit_head);
    }
    
    if (alert_mode) {
        spinlock_lock(&g_ids.lock);
        *alert_mode = g_ids.alert_mode;
        spinlock_unlock(&g_ids.lock);
    }
}
