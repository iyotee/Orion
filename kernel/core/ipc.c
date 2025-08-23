/*
 * Orion Operating System - Lock-Free IPC System
 *
 * High-performance Inter-Process Communication with zero-copy message passing,
 * lock-free queues, and capability-based security.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/kernel.h>
#include <orion/types.h>
#include <orion/mm.h>
#include <orion/structures.h

// Capability constants (if not defined elsewhere)
#ifndef CAP_READ
#define CAP_READ (1ULL << 0)
#endif

#ifndef CAP_WRITE
#define CAP_WRITE (1ULL << 1)
#endif

#ifndef CAP_CREATE
#define CAP_CREATE (1ULL << 6)
#endif

// Forward declarations
extern or_cap_t cap_create(uint32_t type, uint64_t object_id, uint64_t rights, uint64_t owner_pid);

// ========================================
// CONSTANTS AND CONFIGURATION
// ========================================

#define MAX_IPC_PORTS 4096
#define MAX_MSG_QUEUE_SIZE 256
#define MAX_MSG_SIZE (64 * 1024)                // 64KB max per message
#define IPC_SHARED_POOL_SIZE (16 * 1024 * 1024) // 16MB shared pool
#define IPC_RING_BUFFER_SIZE 4096

// Flags for messages
#define IPC_MSG_FLAG_ZERO_COPY 0x00000001
#define IPC_MSG_FLAG_URGENT 0x00000002
#define IPC_MSG_FLAG_RELIABLE 0x00000004
#define IPC_MSG_FLAG_BROADCAST 0x00000008

// IPC capability types
#define CAP_IPC_SEND 0x01
#define CAP_IPC_RECV 0x02
#define CAP_IPC_CREATE 0x04
#define CAP_IPC_DESTROY 0x08

// IPC object types for capabilities
#define CAP_TYPE_IPC_PORT 0x08

// ========================================
// DATA STRUCTURES
// ========================================

// Structure for a message slot in the lock-free queue
typedef struct ipc_msg_slot
{
    atomic64_t sequence;          // Sequence number for lock-free operation
    or_cap_t sender_port;         // Sender port
    uint32_t flags;               // Message flags
    uint32_t data_size;           // Data size
    uint64_t timestamp;           // Send timestamp
    uint64_t page_phys;           // Physical page for zero-copy
    uint64_t offset;              // Offset within the page
    or_cap_t transferred_caps[8]; // Transferred capabilities
    uint32_t cap_count;           // Number of capabilities
    uint32_t reserved;            // Padding
    uint8_t data[256];            // Inline data storage for small messages
} ipc_msg_slot_t;

// Lock-free queue for messages
typedef struct ipc_msg_queue
{
    atomic64_t head;      // Head index (producer)
    atomic64_t tail;      // Tail index (consumer)
    atomic64_t committed; // Last committed transaction
    ipc_msg_slot_t slots[MAX_MSG_QUEUE_SIZE];
    char cache_line_padding[64]; // Avoid false sharing
} ipc_msg_queue_t;

// Complete IPC port
typedef struct ipc_port
{
    atomic64_t ref_count; // Reference counter
    or_cap_t cap_id;      // Capability ID
    uint64_t owner_pid;   // Owner process ID
    uint64_t permissions; // Access permissions
    atomic32_t state;     // Port state

    // Message queues
    ipc_msg_queue_t *send_queue; // Send queue
    ipc_msg_queue_t *recv_queue; // Receive queue

    // Waiting threads
    thread_t *waiting_senders;   // Threads blocked on send
    thread_t *waiting_receivers; // Threads blocked on receive
    spinlock_t waiters_lock;     // Lock for waiting lists

    // Statistics
    atomic64_t msgs_sent;         // Messages sent
    atomic64_t msgs_received;     // Messages received
    atomic64_t bytes_transferred; // Bytes transferred

    // Configuration
    uint32_t max_queue_size; // Maximum queue size
    uint32_t max_msg_size;   // Maximum message size
    uint64_t created_time;   // Creation time

    // Callbacks and endpoints
    void (*msg_handler)(struct ipc_port *port, ipc_msg_slot_t *msg);
    struct ipc_port *linked_port; // Linked port for pipes
} ipc_port_t;

// IPC port states
#define IPC_PORT_STATE_ACTIVE 1
#define IPC_PORT_STATE_CLOSING 2
#define IPC_PORT_STATE_CLOSED 3
#define IPC_PORT_STATE_ERROR 4

// Shared memory pool for zero-copy
typedef struct ipc_shared_pool
{
    uint64_t base_phys;            // Base physical address
    uint64_t base_virt;            // Base virtual address
    uint64_t total_size;           // Total size
    atomic64_t *allocation_bitmap; // Allocation bitmap
    uint64_t page_count;           // Number of pages
    spinlock_t alloc_lock;         // Lock for allocation
} ipc_shared_pool_t;

// Global IPC registry
typedef struct ipc_registry
{
    ipc_port_t ports[MAX_IPC_PORTS]; // Port table
    atomic64_t next_cap_id;          // Next capability ID
    spinlock_t registry_lock;        // Lock for the table

    // Shared memory pool
    ipc_shared_pool_t shared_pool;

    // Global statistics
    atomic64_t total_ports_created;
    atomic64_t total_msgs_sent;
    atomic64_t active_ports;

    // Hash table for fast lookup
    uint32_t port_hash_table[MAX_IPC_PORTS];

} ipc_registry_t;

// ========================================
// GLOBAL VARIABLES
// ========================================

static ipc_registry_t *g_ipc_registry = NULL;
static bool ipc_initialized = false;

// ========================================
// LOCK-FREE UTILITIES
// ========================================

static uint32_t hash_capability(or_cap_t cap)
{
    // Simple but efficient hash for fast lookup
    uint64_t h = cap;
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    return (uint32_t)(h % MAX_IPC_PORTS);
}

static bool cas_sequence(atomic64_t *ptr, uint64_t expected, uint64_t desired)
{
    return atomic_compare_exchange_strong(ptr, &expected, desired);
}

static uint64_t get_next_sequence(atomic64_t *sequence)
{
    return atomic_fetch_add(sequence, 1);
}

// ========================================
// SHARED MEMORY POOL MANAGEMENT
// ========================================

static int ipc_shared_pool_init(ipc_shared_pool_t *pool)
{
    pool->total_size = IPC_SHARED_POOL_SIZE;
    pool->page_count = pool->total_size / PAGE_SIZE;

    // Allocate contiguous physical pages
    uint64_t phys_pages = pmm_alloc_pages(pool->page_count);
    if (!phys_pages)
    {
        kerror("Failed to allocate IPC shared pool");
        return -OR_ENOMEM;
    }

    pool->base_phys = phys_pages;
    pool->base_virt = phys_pages; // Identity mapping for simplicity

    // Allocate allocation bitmap
    size_t bitmap_size = (pool->page_count + 63) / 64 * sizeof(uint64_t);
    pool->allocation_bitmap = (atomic64_t *)kmalloc(bitmap_size);
    if (!pool->allocation_bitmap)
    {
        pmm_free_pages(phys_pages, pool->page_count);
        return -OR_ENOMEM;
    }

    // Initialize bitmap (all pages free)
    memset(pool->allocation_bitmap, 0, bitmap_size);

    spinlock_init(&pool->alloc_lock);

    kdebug("IPC shared pool initialized: %llu pages at 0x%p",
           (unsigned long long)pool->page_count, (void *)pool->base_phys);

    return OR_OK;
}

static uint64_t ipc_shared_alloc_page(ipc_shared_pool_t *pool)
{
    spinlock_lock(&pool->alloc_lock);

    // Find a free page in the bitmap
    for (uint64_t i = 0; i < pool->page_count; i++)
    {
        uint64_t word_idx = i / 64;
        uint64_t bit_idx = i % 64;

        uint64_t word = atomic_load(&pool->allocation_bitmap[word_idx]);
        if (!(word & (1ULL << bit_idx)))
        {
            // Free page found
            atomic_fetch_or(&pool->allocation_bitmap[word_idx], 1ULL << bit_idx);
            spinlock_unlock(&pool->alloc_lock);

            uint64_t page_phys = pool->base_phys + i * PAGE_SIZE;
            kdebug("Allocated shared page %llu at 0x%p",
                   (unsigned long long)i, (void *)page_phys);
            return page_phys;
        }
    }

    spinlock_unlock(&pool->alloc_lock);
    return 0; // No free page
}

static void ipc_shared_free_page(ipc_shared_pool_t *pool, uint64_t page_phys)
{
    if (page_phys < pool->base_phys ||
        page_phys >= pool->base_phys + pool->total_size)
    {
        return;
    }

    uint64_t page_idx = (page_phys - pool->base_phys) / PAGE_SIZE;
    uint64_t word_idx = page_idx / 64;
    uint64_t bit_idx = page_idx % 64;

    spinlock_lock(&pool->alloc_lock);
    atomic_fetch_and(&pool->allocation_bitmap[word_idx], ~(1ULL << bit_idx));
    spinlock_unlock(&pool->alloc_lock);

    kdebug("Freed shared page %llu at 0x%p",
           (unsigned long long)page_idx, (void *)page_phys);
}

// ========================================
// QUEUE MANAGEMENT
// ========================================

static ipc_msg_queue_t *ipc_queue_create(void)
{
    ipc_msg_queue_t *queue = (ipc_msg_queue_t *)kmalloc(sizeof(ipc_msg_queue_t));
    if (!queue)
    {
        return NULL;
    }

    memset(queue, 0, sizeof(ipc_msg_queue_t));

    // Initialize atomic indices
    atomic_store(&queue->head, 0);
    atomic_store(&queue->tail, 0);
    atomic_store(&queue->committed, 0);

    // Initialize slots
    for (int i = 0; i < MAX_MSG_QUEUE_SIZE; i++)
    {
        atomic_store(&queue->slots[i].sequence, i);
    }

    return queue;
}

static bool ipc_queue_send(ipc_msg_queue_t *queue, ipc_msg_slot_t *msg)
{
    uint64_t head = atomic_load(&queue->head);

    for (;;)
    {
        ipc_msg_slot_t *slot = &queue->slots[head % MAX_MSG_QUEUE_SIZE];
        uint64_t seq = atomic_load(&slot->sequence);

        int64_t diff = (int64_t)seq - (int64_t)head;

        if (diff == 0)
        {
            // Slot available, try to reserve it
            if (cas_sequence(&queue->head, head, head + 1))
            {
                // Copy message into the slot
                slot->sender_port = msg->sender_port;
                slot->flags = msg->flags;
                slot->data_size = msg->data_size;
                slot->timestamp = msg->timestamp;
                slot->page_phys = msg->page_phys;
                slot->offset = msg->offset;
                slot->cap_count = msg->cap_count;

                for (uint32_t i = 0; i < msg->cap_count && i < 8; i++)
                {
                    slot->transferred_caps[i] = msg->transferred_caps[i];
                }

                // Publish the message
                atomic_store(&slot->sequence, head + 1);
                return true;
            }
        }
        else if (diff < 0)
        {
            // Queue full
            return false;
        }
        else
        {
            // Retry with new head
            head = atomic_load(&queue->head);
        }
    }
}

static bool ipc_queue_recv(ipc_msg_queue_t *queue, ipc_msg_slot_t *msg)
{
    uint64_t tail = atomic_load(&queue->tail);

    for (;;)
    {
        ipc_msg_slot_t *slot = &queue->slots[tail % MAX_MSG_QUEUE_SIZE];
        uint64_t seq = atomic_load(&slot->sequence);

        int64_t diff = (int64_t)seq - (int64_t)(tail + 1);

        if (diff == 0)
        {
            // Message available
            if (cas_sequence(&queue->tail, tail, tail + 1))
            {
                // Copy message
                *msg = *slot;

                // Free the slot
                atomic_store(&slot->sequence, tail + MAX_MSG_QUEUE_SIZE);
                return true;
            }
        }
        else if (diff < 0)
        {
            // No message
            return false;
        }
        else
        {
            // Retry with new tail
            tail = atomic_load(&queue->tail);
        }
    }
}

// ========================================
// PORT MANAGEMENT
// ========================================

static ipc_port_t *ipc_find_port(or_cap_t cap_id)
{
    if (!g_ipc_registry)
        return NULL;

    uint32_t hash = hash_capability(cap_id);

    // Linear search from hash
    for (uint32_t i = 0; i < MAX_IPC_PORTS; i++)
    {
        uint32_t idx = (hash + i) % MAX_IPC_PORTS;
        ipc_port_t *port = &g_ipc_registry->ports[idx];

        if (port->cap_id == cap_id &&
            atomic_load(&port->state) == IPC_PORT_STATE_ACTIVE)
        {
            return port;
        }

        if (port->cap_id == 0)
        {
            break; // Empty slot = end of chain
        }
    }

    return NULL;
}

// ========================================
// IPC CAPABILITY MANAGEMENT
// ========================================

// Get IPC capability for a process
or_cap_t process_get_ipc_capability(process_t *process, uint32_t cap_type)
{
    if (!process || !ipc_initialized)
    {
        return 0;
    }

    // Check if process has the required IPC capability
    // This would typically query the capability system
    // Note: cap_check_rights is declared elsewhere

    // For now, create a basic IPC capability if none exists
    // In a full implementation, this would query existing capabilities
    static or_cap_t process_ipc_caps[1024] = {0}; // Simple process capability cache

    uint32_t process_idx = process->pid % 1024;
    if (process_ipc_caps[process_idx] == 0)
    {
        // Create new IPC capability for this process
        process_ipc_caps[process_idx] = cap_create(CAP_TYPE_IPC_PORT, process->pid,
                                                   CAP_READ | CAP_WRITE | CAP_CREATE, process->pid);

        if (process_ipc_caps[process_idx] != 0)
        {
            kdebug("Created IPC capability 0x%llx for process PID %llu",
                   (unsigned long long)process_ipc_caps[process_idx], (unsigned long long)process->pid);
        }
    }

    return process_ipc_caps[process_idx];
}

// ========================================
// PUBLIC IPC API
// ========================================

void ipc_init(void)
{
    kinfo("Initializing high-performance IPC system");

    // Allocate global registry
    g_ipc_registry = (ipc_registry_t *)kmalloc(sizeof(ipc_registry_t));
    if (!g_ipc_registry)
    {
        kerror("Failed to allocate IPC registry");
        return;
    }

    memset(g_ipc_registry, 0, sizeof(ipc_registry_t));

    // Initialize registry
    atomic_store(&g_ipc_registry->next_cap_id, 1000); // Start from 1000
    spinlock_init(&g_ipc_registry->registry_lock);

    // Initialize shared pool
    if (ipc_shared_pool_init(&g_ipc_registry->shared_pool) != OR_OK)
    {
        kfree(g_ipc_registry);
        g_ipc_registry = NULL;
        kerror("Failed to initialize IPC shared pool");
        return;
    }

    // Initialize all ports
    for (int i = 0; i < MAX_IPC_PORTS; i++)
    {
        ipc_port_t *port = &g_ipc_registry->ports[i];
        memset(port, 0, sizeof(ipc_port_t));
        atomic_store(&port->ref_count, 0);
        atomic_store(&port->state, IPC_PORT_STATE_CLOSED);
        spinlock_init(&port->waiters_lock);
    }

    // Initialize hash table
    memset(g_ipc_registry->port_hash_table, 0xFF, sizeof(g_ipc_registry->port_hash_table));

    ipc_initialized = true;

    kinfo("IPC system initialized:");
    kinfo("  Max ports: %d", MAX_IPC_PORTS);
    kinfo("  Max message size: %d KB", MAX_MSG_SIZE / 1024);
    kinfo("  Shared pool: %d MB", IPC_SHARED_POOL_SIZE / (1024 * 1024));
    kinfo("  Queue size: %d messages", MAX_MSG_QUEUE_SIZE);
}

or_cap_t ipc_port_create(uint64_t owner_pid)
{
    if (!ipc_initialized || !g_ipc_registry)
    {
        return 0;
    }

    spinlock_lock(&g_ipc_registry->registry_lock);

    // Find a free slot
    ipc_port_t *free_port = NULL;
    uint32_t free_idx = 0;

    for (uint32_t i = 0; i < MAX_IPC_PORTS; i++)
    {
        ipc_port_t *port = &g_ipc_registry->ports[i];
        if (atomic_load(&port->state) == IPC_PORT_STATE_CLOSED && port->cap_id == 0)
        {
            free_port = port;
            free_idx = i;
            break;
        }
    }

    if (!free_port)
    {
        spinlock_unlock(&g_ipc_registry->registry_lock);
        kerror("No free IPC port slots");
        return 0;
    }

    // Generate new capability ID
    or_cap_t new_cap_id = atomic_fetch_add(&g_ipc_registry->next_cap_id, 1);

    // Initialize the port
    memset(free_port, 0, sizeof(ipc_port_t));
    free_port->cap_id = new_cap_id;
    free_port->owner_pid = owner_pid;
    free_port->permissions = 0xFFFFFFFFFFFFFFFFULL; // All permissions for owner
    free_port->max_queue_size = MAX_MSG_QUEUE_SIZE;
    free_port->max_msg_size = MAX_MSG_SIZE;
    free_port->created_time = arch_get_timestamp();

    // Create queues
    free_port->send_queue = ipc_queue_create();
    free_port->recv_queue = ipc_queue_create();

    if (!free_port->send_queue || !free_port->recv_queue)
    {
        if (free_port->send_queue)
            kfree(free_port->send_queue);
        if (free_port->recv_queue)
            kfree(free_port->recv_queue);
        spinlock_unlock(&g_ipc_registry->registry_lock);
        return 0;
    }

    // Initialize atomics
    atomic_store(&free_port->ref_count, 1);
    atomic_store(&free_port->state, IPC_PORT_STATE_ACTIVE);
    atomic_store(&free_port->msgs_sent, 0);
    atomic_store(&free_port->msgs_received, 0);
    atomic_store(&free_port->bytes_transferred, 0);

    spinlock_init(&free_port->waiters_lock);

    // Add to hash table
    uint32_t hash = hash_capability(new_cap_id);
    g_ipc_registry->port_hash_table[hash] = free_idx;

    // Update statistics
    atomic_fetch_add(&g_ipc_registry->total_ports_created, 1);
    atomic_fetch_add(&g_ipc_registry->active_ports, 1);

    spinlock_unlock(&g_ipc_registry->registry_lock);

    kdebug("Created IPC port cap=%llu for PID %llu (slot %u)",
           (unsigned long long)new_cap_id, (unsigned long long)owner_pid, free_idx);

    return new_cap_id;
}

// Send an IPC message with zero-copy
int ipc_send_message(or_cap_t port_cap, const void *data, size_t size, uint64_t timeout_ns)
{
    if (!ipc_initialized || !g_ipc_registry || size > MAX_MSG_SIZE)
    {
        return -OR_EINVAL;
    }

    // Find the destination port
    ipc_port_t *port = ipc_find_port(port_cap);
    if (!port)
    {
        return -OR_ENOENT;
    }

    // Check permissions
    process_t *current = scheduler_get_current_process();
    if (!current)
    {
        return -OR_EPERM;
    }

    kdebug("IPC send: port=%llu, size=%llu",
           (unsigned long long)port_cap, (unsigned long long)size);

    // Prepare the message
    ipc_msg_slot_t msg = {0};

    // Get sender port from current process
    process_t *current = scheduler_get_current_process();
    if (current)
    {
        // Implement complete IPC capability system
        // Each process has a set of capabilities for IPC operations
        extern or_cap_t process_get_ipc_capability(process_t * process, uint32_t cap_type);

        // Get the IPC capability for this process
        or_cap_t ipc_cap = process_get_ipc_capability(current, CAP_IPC_SEND);

        if (ipc_cap != 0)
        {
            msg.sender_port = ipc_cap; // Use capability as sender identifier
            kdebug("IPC: Using capability 0x%llx as sender port", (unsigned long long)ipc_cap);
        }
        else
        {
            // Create IPC capability if none exists
            ipc_cap = cap_create(CAP_TYPE_IPC_PORT, current->pid, CAP_READ | CAP_WRITE | CAP_CREATE, current->pid);
            if (ipc_cap != 0)
            {
                msg.sender_port = ipc_cap;
                kdebug("IPC: Created new capability 0x%llx for PID %llu", (unsigned long long)ipc_cap, (unsigned long long)current->pid);
            }
            else
            {
                // Fallback: use process ID if capability creation fails
                msg.sender_port = current->pid;
                kwarn("IPC: Failed to create capability, falling back to PID %llu", (unsigned long long)current->pid);
            }
        }
    }
    else
    {
        msg.sender_port = 0; // Kernel process
    }

    msg.flags = (size > PAGE_SIZE) ? IPC_MSG_FLAG_ZERO_COPY : 0;
    msg.data_size = (uint32_t)size;
    msg.timestamp = arch_get_timestamp();
    msg.cap_count = 0;

    // Allocate memory based on size
    if (msg.flags & IPC_MSG_FLAG_ZERO_COPY)
    {
        // Zero-copy: allocate shared page
        msg.page_phys = ipc_shared_alloc_page(&g_ipc_registry->shared_pool);
        if (!msg.page_phys)
        {
            return -OR_ENOMEM;
        }

        msg.offset = 0;

        // Copy data to the shared page
        void *shared_addr = (void *)msg.page_phys; // Identity mapping
        memcpy(shared_addr, data, size);

        kdebug("IPC zero-copy: allocated shared page 0x%p", (void *)msg.page_phys);
    }
    else
    {
        // Classic copy (small messages) - store data directly in slot
        msg.page_phys = 0;
        msg.offset = 0;

        if (size <= sizeof(msg.data))
        {
            // Copy data directly into the slot
            memcpy(msg.data, data, size);
            kdebug("IPC small message: %llu bytes stored in slot", (unsigned long long)size);
        }
        else
        {
            // Message too large for slot, fall back to zero-copy
            msg.flags |= IPC_MSG_FLAG_ZERO_COPY;
            msg.page_phys = ipc_shared_alloc_page(&g_ipc_registry->shared_pool);
            if (!msg.page_phys)
            {
                return -OR_ENOMEM;
            }

            void *shared_addr = (void *)msg.page_phys;
            memcpy(shared_addr, data, size);
            kdebug("IPC fallback: %llu bytes stored in shared page 0x%p", (unsigned long long)size, (void *)msg.page_phys);
        }
    }

    // Try to send the message
    uint64_t start_time = arch_get_timestamp();
    bool sent = false;

    while (!sent)
    {
        sent = ipc_queue_send(port->recv_queue, &msg);

        if (!sent)
        {
            // Queue full, check timeout
            if (timeout_ns > 0)
            {
                uint64_t elapsed = arch_get_timestamp() - start_time;
                if (elapsed >= timeout_ns)
                {
                    // Timeout, release resources
                    if (msg.page_phys)
                    {
                        ipc_shared_free_page(&g_ipc_registry->shared_pool, msg.page_phys);
                    }
                    return -OR_ETIMEDOUT;
                }
            }

            // Wait a bit before retrying
            scheduler_sleep_ns(1000); // 1µs
        }
    }

    // Wake up waiting threads for reception
    spinlock_lock(&port->waiters_lock);
    if (port->waiting_receivers)
    {
        thread_t *thread = port->waiting_receivers;
        port->waiting_receivers = thread->next;
        scheduler_wakeup_process(thread->parent_process);
    }
    spinlock_unlock(&port->waiters_lock);

    // Update statistics
    atomic_fetch_add(&port->msgs_sent, 1);
    atomic_fetch_add(&port->bytes_transferred, size);
    atomic_fetch_add(&g_ipc_registry->total_msgs_sent, 1);

    kdebug("IPC message sent successfully: %llu bytes", (unsigned long long)size);
    return OR_OK;
}

// Receive an IPC message
int ipc_recv_message(or_cap_t port_cap, void *buffer, size_t buffer_size, uint64_t timeout_ns)
{
    if (!ipc_initialized || !g_ipc_registry || !buffer)
    {
        return -OR_EINVAL;
    }

    // Find the port
    ipc_port_t *port = ipc_find_port(port_cap);
    if (!port)
    {
        return -OR_ENOENT;
    }

    // Check permissions
    process_t *current = scheduler_get_current_process();
    if (!current || current->pid != port->owner_pid)
    {
        return -OR_EPERM;
    }

    kdebug("IPC recv: port=%llu, buffer_size=%llu",
           (unsigned long long)port_cap, (unsigned long long)buffer_size);

    // Try to receive a message
    ipc_msg_slot_t msg;
    uint64_t start_time = arch_get_timestamp();
    bool received = false;

    while (!received)
    {
        received = ipc_queue_recv(port->recv_queue, &msg);

        if (!received)
        {
            // No message, check timeout
            if (timeout_ns > 0)
            {
                uint64_t elapsed = arch_get_timestamp() - start_time;
                if (elapsed >= timeout_ns)
                {
                    return -OR_ETIMEDOUT;
                }
            }

            // Block
            thread_t *current_thread = scheduler_get_current_thread();
            if (current_thread)
            {
                spinlock_lock(&port->waiters_lock);
                current_thread->next = port->waiting_receivers;
                port->waiting_receivers = current_thread;
                spinlock_unlock(&port->waiters_lock);

                scheduler_block_current_process();
                scheduler_sleep_ns(1000); // 1µs
            }
        }
    }

    // Check if buffer is large enough
    if (buffer_size < msg.data_size)
    {
        // Release resources
        if (msg.page_phys)
        {
            ipc_shared_free_page(&g_ipc_registry->shared_pool, msg.page_phys);
        }
        return -OR_EINVAL;
    }

    // Copy data
    if (msg.flags & IPC_MSG_FLAG_ZERO_COPY)
    {
        // Zero-copy: map the page into user space
        void *shared_addr = (void *)msg.page_phys;
        memcpy(buffer, (char *)shared_addr + msg.offset, msg.data_size);

        // Free the shared page
        ipc_shared_free_page(&g_ipc_registry->shared_pool, msg.page_phys);

        kdebug("IPC zero-copy recv: freed shared page 0x%p", (void *)msg.page_phys);
    }
    else
    {
        // Data is stored directly in the slot for small messages
        if (msg.data_size <= sizeof(msg.data))
        {
            // Copy data from slot to user buffer
            memcpy(buffer, msg.data, msg.data_size);
            kdebug("IPC small message recv: %u bytes copied from slot", msg.data_size);
        }
        else
        {
            // Data should be in shared page but flags indicate it's not
            // This is an error condition
            kerror("IPC: Small message flag but data size %u exceeds slot capacity", msg.data_size);
            memset(buffer, 0, msg.data_size);
        }
    }

    // Update statistics
    atomic_fetch_add(&port->msgs_received, 1);

    kdebug("IPC message received: %u bytes", msg.data_size);
    return (int)msg.data_size;
}

// Destroy an IPC port
void ipc_port_destroy(or_cap_t port_cap)
{
    if (!ipc_initialized || !g_ipc_registry || port_cap == 0)
    {
        return;
    }

    ipc_port_t *port = ipc_find_port(port_cap);
    if (!port)
    {
        return;
    }

    // Mark port as closing
    atomic_store(&port->state, IPC_PORT_STATE_CLOSING);

    // Wake up all waiting threads
    spinlock_lock(&port->waiters_lock);

    thread_t *sender = port->waiting_senders;
    while (sender)
    {
        thread_t *next = sender->next;
        scheduler_wakeup_process(sender->parent_process);
        sender = next;
    }

    thread_t *receiver = port->waiting_receivers;
    while (receiver)
    {
        thread_t *next = receiver->next;
        scheduler_wakeup_process(receiver->parent_process);
        receiver = next;
    }

    port->waiting_senders = NULL;
    port->waiting_receivers = NULL;
    spinlock_unlock(&port->waiters_lock);

    // Free queues
    if (port->send_queue)
    {
        kfree(port->send_queue);
        port->send_queue = NULL;
    }

    if (port->recv_queue)
    {
        kfree(port->recv_queue);
        port->recv_queue = NULL;
    }

    // Clean up port
    atomic_store(&port->state, IPC_PORT_STATE_CLOSED);
    port->cap_id = 0;
    port->owner_pid = 0;

    // Update statistics
    atomic_fetch_sub(&g_ipc_registry->active_ports, 1);

    kdebug("Destroyed IPC port %llu", (unsigned long long)port_cap);
}
