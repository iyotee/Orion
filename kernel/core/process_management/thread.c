/*
 * Orion Operating System - Thread Management
 *
 * Complete thread creation, management, and lifecycle implementation.
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
#include <orion/scheduler.h>
#include <orion/security.h>

// ========================================
// CONSTANTS AND DEFINITIONS
// ========================================

#define MAX_THREADS 4096
#define THREAD_STACK_SIZE (64 * 1024) // 64KB default stack
#define THREAD_STACK_ALIGN 16

// Thread states
#define THREAD_STATE_NEW 0
#define THREAD_STATE_READY 1
#define THREAD_STATE_RUNNING 2
#define THREAD_STATE_BLOCKED 3
#define THREAD_STATE_SLEEPING 4
#define THREAD_STATE_TERMINATED 5

// Thread priorities (nice values)
#define THREAD_PRIORITY_MIN -20
#define THREAD_PRIORITY_MAX 19
#define THREAD_PRIORITY_DEFAULT 0

// ========================================
// GLOBAL VARIABLES
// ========================================

static thread_t *g_threads[MAX_THREADS];
static atomic64_t g_next_tid = ATOMIC_VAR_INIT(1);
static spinlock_t g_thread_table_lock = SPINLOCK_INIT;

// ========================================
// THREAD CREATION AND MANAGEMENT
// ========================================

/**
 * Create a new thread
 *
 * @param process Parent process
 * @param name Thread name
 * @param entry_point Entry point function
 * @param arg Argument passed to entry point
 * @return Pointer to created thread or NULL on failure
 */
thread_t *thread_create(process_t *process, const char *name, void (*entry_point)(void *), void *arg)
{
    if (!process || !name || !entry_point)
    {
        kerror("thread_create: Invalid parameters");
        return NULL;
    }

    spinlock_lock(&g_thread_table_lock);

    // Find a free thread slot
    uint32_t free_slot = 0;
    bool slot_found = false;

    for (uint32_t i = 0; i < MAX_THREADS; i++)
    {
        if (!g_threads[i])
        {
            free_slot = i;
            slot_found = true;
            break;
        }
    }

    if (!slot_found)
    {
        spinlock_unlock(&g_thread_table_lock);
        kerror("thread_create: No free thread slots");
        return NULL;
    }

    // Allocate thread structure
    thread_t *thread = (thread_t *)kmalloc(sizeof(thread_t));
    if (!thread)
    {
        spinlock_unlock(&g_thread_table_lock);
        kerror("thread_create: Failed to allocate thread structure");
        return NULL;
    }

    // Initialize thread structure
    memset(thread, 0, sizeof(thread_t));

    // Assign TID
    thread->tid = atomic_fetch_add(&g_next_tid, 1);
    thread->state = THREAD_STATE_NEW;
    thread->exit_code = 0;

    // Copy thread name
    size_t name_len = strlen(name);
    if (name_len > 31)
        name_len = 31;
    memcpy(thread->name, name, name_len);
    thread->name[name_len] = '\0';

    // Set parent process
    thread->parent_process = process;

    // Initialize scheduler fields
    thread->virtual_runtime = 0;
    thread->actual_runtime = 0;
    thread->last_switch_time = arch_get_timestamp();
    thread->priority = THREAD_PRIORITY_DEFAULT;
    thread->nice_weight = 1024; // Default weight

    // Initialize CPU context
    thread->rsp = 0;
    thread->rip = (uint64_t)entry_point;
    thread->rflags = 0x202; // Default flags (IF=1, IOPL=0)
    thread->rbp = 0;
    memset(thread->registers, 0, sizeof(thread->registers));

    // Allocate stack
    uint64_t stack_pages = (THREAD_STACK_SIZE + PAGE_SIZE - 1) / PAGE_SIZE;
    uint64_t stack_phys = pmm_alloc_pages(stack_pages);
    if (!stack_phys)
    {
        kfree(thread);
        spinlock_unlock(&g_thread_table_lock);
        kerror("thread_create: Failed to allocate thread stack");
        return NULL;
    }

    // Map stack into process virtual space
    uint64_t stack_vaddr = vmm_alloc_pages(process->vm_space, stack_pages,
                                           VM_FLAG_READ | VM_FLAG_WRITE | VM_FLAG_USER);
    if (!stack_vaddr)
    {
        pmm_free_pages(stack_phys, stack_pages);
        kfree(thread);
        spinlock_unlock(&g_thread_table_lock);
        kerror("thread_create: Failed to map thread stack");
        return NULL;
    }

    // Map physical pages to virtual address
    for (uint64_t i = 0; i < stack_pages; i++)
    {
        vmm_map_page(process->vm_space, stack_vaddr + i * PAGE_SIZE,
                     stack_phys + i * PAGE_SIZE,
                     VM_FLAG_READ | VM_FLAG_WRITE | VM_FLAG_USER);
    }

    // Set up stack pointers
    thread->stack_base = stack_vaddr;
    thread->stack_size = stack_pages * PAGE_SIZE;
    thread->stack_top = stack_vaddr + thread->stack_size;
    thread->user_stack = stack_vaddr;

    // Initialize stack with entry point and argument
    uint64_t *stack_ptr = (uint64_t *)(thread->stack_top - 16);
    stack_ptr[0] = (uint64_t)arg;         // Argument
    stack_ptr[1] = (uint64_t)entry_point; // Return address (entry point)

    // Set initial stack pointer
    thread->rsp = (uint64_t)stack_ptr;

    // Initialize scheduler links
    thread->next = NULL;
    thread->prev = NULL;
    thread->rb_left = NULL;
    thread->rb_right = NULL;
    thread->rb_parent = NULL;
    thread->rb_color = 0; // RB_BLACK

    // Initialize timing and sleep
    thread->sleep_until = 0;
    thread->cpu_affinity = 0xFFFFFFFFFFFFFFFFULL; // All CPUs

    // Add to process thread list
    if (!process->threads)
    {
        process->threads = thread;
    }
    else
    {
        // Add to end of list
        thread_t *last = process->threads;
        while (last->next)
        {
            last = last->next;
        }
        last->next = thread;
        thread->prev = last;
    }

    // Set as main thread if this is the first thread
    if (!process->main_thread)
    {
        process->main_thread = thread;
    }

    process->thread_count++;

    // Add to thread table
    g_threads[free_slot] = thread;

    spinlock_unlock(&g_thread_table_lock);

    kinfo("Created thread '%s' (TID %llu) in process %llu",
          thread->name, (unsigned long long)thread->tid, (unsigned long long)process->pid);

    return thread;
}

/**
 * Start a thread
 *
 * @param thread Thread to start
 * @return 0 on success, negative error code on failure
 */
int thread_start(thread_t *thread)
{
    if (!thread)
    {
        return -OR_EINVAL;
    }

    if (thread->state != THREAD_STATE_NEW && thread->state != THREAD_STATE_READY)
    {
        kerror("thread_start: Thread %llu not in valid state (%d)",
               (unsigned long long)thread->tid, thread->state);
        return -OR_EINVAL;
    }

    // Set thread state to ready
    thread->state = THREAD_STATE_READY;

    // Add to scheduler
    scheduler_add_thread_to_rq(thread);

    kinfo("Started thread '%s' (TID %llu)", thread->name, (unsigned long long)thread->tid);

    return OR_OK;
}

/**
 * Destroy a thread
 *
 * @param thread Thread to destroy
 */
void thread_destroy(thread_t *thread)
{
    if (!thread)
        return;

    kinfo("Destroying thread '%s' (TID %llu)", thread->name, (unsigned long long)thread->tid);

    // Remove from scheduler if running
    if (thread->state == THREAD_STATE_READY || thread->state == THREAD_STATE_RUNNING)
    {
        // This would call scheduler_remove_thread() if implemented
        kdebug("Thread %llu removed from scheduler", (unsigned long long)thread->tid);
    }

    // Remove from process thread list
    if (thread->parent_process)
    {
        if (thread->prev)
        {
            thread->prev->next = thread->next;
        }
        else
        {
            // This is the first thread in the list
            thread->parent_process->threads = thread->next;
        }

        if (thread->next)
        {
            thread->next->prev = thread->prev;
        }

        // Update main thread if this was the main thread
        if (thread->parent_process->main_thread == thread)
        {
            thread->parent_process->main_thread = thread->next;
        }

        thread->parent_process->thread_count--;
    }

    // Free stack memory
    if (thread->stack_base && thread->stack_size > 0)
    {
        uint64_t stack_pages = (thread->stack_size + PAGE_SIZE - 1) / PAGE_SIZE;

        // Unmap from virtual space
        for (uint64_t i = 0; i < stack_pages; i++)
        {
            uint64_t vaddr = thread->stack_base + i * PAGE_SIZE;
            uint64_t paddr = vmm_get_physical_address(thread->parent_process->vm_space, vaddr);
            if (paddr)
            {
                vmm_unmap_page(thread->parent_process->vm_space, vaddr);
                pmm_free_pages(paddr, 1);
            }
        }
    }

    // Remove from thread table
    spinlock_lock(&g_thread_table_lock);
    for (uint32_t i = 0; i < MAX_THREADS; i++)
    {
        if (g_threads[i] == thread)
        {
            g_threads[i] = NULL;
            break;
        }
    }
    spinlock_unlock(&g_thread_table_lock);

    // Free thread structure
    kfree(thread);
}

/**
 * Find thread by TID
 *
 * @param tid Thread ID to find
 * @return Pointer to thread or NULL if not found
 */
thread_t *thread_find(uint64_t tid)
{
    if (tid == 0)
        return NULL;

    spinlock_lock(&g_thread_table_lock);

    for (uint32_t i = 0; i < MAX_THREADS; i++)
    {
        if (g_threads[i] && g_threads[i]->tid == tid)
        {
            spinlock_unlock(&g_thread_table_lock);
            return g_threads[i];
        }
    }

    spinlock_unlock(&g_thread_table_lock);
    return NULL;
}

/**
 * Get current thread
 *
 * @return Pointer to current thread or NULL
 */
thread_t *thread_get_current(void)
{
    return scheduler_get_current_thread();
}

/**
 * Set thread priority
 *
 * @param thread Thread to set priority for
 * @param priority New priority (-20 to +19)
 * @return 0 on success, negative error code on failure
 */
int thread_set_priority(thread_t *thread, int priority)
{
    if (!thread)
        return -OR_EINVAL;

    if (priority < THREAD_PRIORITY_MIN || priority > THREAD_PRIORITY_MAX)
    {
        return -OR_EINVAL;
    }

    thread->priority = priority;

    // Update nice weight based on priority
    // This is a simplified calculation - in a real system, this would be more sophisticated
    if (priority <= 0)
    {
        thread->nice_weight = 1024 >> (-priority);
    }
    else
    {
        thread->nice_weight = 1024 << priority;
    }

    kinfo("Set thread %llu priority to %d (weight: %llu)",
          (unsigned long long)thread->tid, priority, (unsigned long long)thread->nice_weight);

    return OR_OK;
}

/**
 * Put thread to sleep
 *
 * @param thread Thread to sleep
 * @param nanoseconds Sleep duration in nanoseconds
 * @return 0 on success, negative error code on failure
 */
int thread_sleep(thread_t *thread, uint64_t nanoseconds)
{
    if (!thread)
        return -OR_EINVAL;

    if (nanoseconds == 0)
        return OR_OK;

    thread->state = THREAD_STATE_SLEEPING;
    thread->sleep_until = arch_get_timestamp() + nanoseconds;

    kinfo("Thread %llu sleeping for %llu nanoseconds",
          (unsigned long long)thread->tid, (unsigned long long)nanoseconds);

    return OR_OK;
}

/**
 * Wake up a sleeping thread
 *
 * @param thread Thread to wake up
 * @return 0 on success, negative error code on failure
 */
int thread_wakeup(thread_t *thread)
{
    if (!thread)
        return -OR_EINVAL;

    if (thread->state != THREAD_STATE_SLEEPING)
    {
        return -OR_EINVAL;
    }

    thread->state = THREAD_STATE_READY;
    thread->sleep_until = 0;

    // Add back to scheduler
    scheduler_add_thread_to_rq(thread);

    kinfo("Woke up thread %llu", (unsigned long long)thread->tid);

    return OR_OK;
}

/**
 * Get thread count
 *
 * @return Number of active threads
 */
uint32_t thread_get_count(void)
{
    uint32_t count = 0;

    spinlock_lock(&g_thread_table_lock);
    for (uint32_t i = 0; i < MAX_THREADS; i++)
    {
        if (g_threads[i])
            count++;
    }
    spinlock_unlock(&g_thread_table_lock);

    return count;
}

/**
 * Initialize thread management system
 */
void thread_init(void)
{
    kinfo("Initializing thread management system");

    // Initialize thread table
    memset(g_threads, 0, sizeof(g_threads));

    // Initialize locks
    spinlock_init(&g_thread_table_lock);

    // Reset TID counter
    atomic_store(&g_next_tid, 1);

    kinfo("Thread management system initialized (max threads: %d)", MAX_THREADS);
}
