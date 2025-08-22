/*
 * Orion Operating System - CFS-like Scheduler
 *
 * Complete Fair Scheduler implementation with real-time capabilities,
 * per-CPU runqueues, and latency optimizations.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/types.h>
#include <orion/mm.h>
#include <orion/process.h>
#include <orion/thread.h>
#include <orion/kernel.h>

// ========================================
// STRUCTURES AND TYPES
// ========================================

// Note: process_state_t and thread_state_t are defined in scheduler.h

// Note: All structures (thread_t, process_t, cpu_runqueue_t, handle_t) are now defined in scheduler.h

// ========================================
// HELPER FUNCTIONS
// ========================================

// Copy data to user space
static int copy_to_user(void *user_ptr, const void *kernel_data, size_t size)
{
    if (!user_ptr || !kernel_data || size == 0)
    {
        return -OR_EINVAL;
    }

    // Validate user pointer and check memory protection
    if (!mmu_is_user_address(user_ptr))
    {
        kerror("copy_to_user: Invalid user address 0x%p", user_ptr);
        return -OR_EFAULT;
    }

    // Check if user memory is writable
    if (!mmu_check_user_write_access(user_ptr, size))
    {
        kerror("copy_to_user: User memory not writable at 0x%p", user_ptr);
        return -OR_EFAULT;
    }

    // Perform the copy with page fault handling
    if (copy_to_user_safe(user_ptr, kernel_data, size) != OR_OK)
    {
        kerror("copy_to_user: Page fault during copy at 0x%p", user_ptr);
        return -OR_EFAULT;
    }

    return OR_OK;
}

// Copy string to user space
static int copy_string_to_user(char *user_ptr, const char *kernel_str)
{
    if (!user_ptr || !kernel_str)
    {
        return -OR_EINVAL;
    }

    size_t len = strlen(kernel_str) + 1;
    return copy_to_user(user_ptr, kernel_str, len);
}

// ========================================
// GLOBAL VARIABLES
// ========================================

static bool scheduler_initialized = false;
static atomic64_t next_pid = {1};
static atomic64_t next_tid = {1};
static cpu_runqueue_t runqueues[MAX_CPUS];
static process_t *process_list = NULL;
static spinlock_t process_list_lock = SPINLOCK_INIT;
static process_t *init_process = NULL;
static uint64_t total_processes = 0;
static uint64_t total_threads = 0;

// ========================================
// CFS CONSTANTS AND MACROS
// ========================================

#define SCHED_LATENCY_NS 6000000ULL  // 6ms latence cible
#define MIN_GRANULARITY_NS 750000ULL // 0.75ms minimum granularity
#define NICE_0_WEIGHT 1024           // Poids pour nice 0
#define RB_RED 0
#define RB_BLACK 1

// Weights for different priority levels (nice -20 to +19)
static const uint64_t sched_weights[40] = {
    /* -20 */ 88761,
    71755,
    56483,
    46273,
    36291,
    /* -15 */ 29154,
    23254,
    18705,
    14949,
    11916,
    /* -10 */ 9548,
    7620,
    6100,
    4904,
    3906,
    /*  -5 */ 3121,
    2501,
    1991,
    1586,
    1277,
    /*   0 */ 1024,
    820,
    655,
    526,
    423,
    /*   5 */ 335,
    272,
    215,
    172,
    137,
    /*  10 */ 110,
    87,
    70,
    56,
    45,
    /*  15 */ 36,
    29,
    23,
    18,
    15,
};

// ========================================
// RED-BLACK TREE UTILITIES
// ========================================

// Timer tick handler for scheduler
void scheduler_tick(void)
{
    static uint64_t tick_count = 0;
    tick_count++;

    // Get current CPU and its runqueue
    uint32_t cpu_id = arch_get_current_cpu();
    if (cpu_id >= MAX_CPUS)
    {
        kerror("scheduler_tick: Invalid CPU ID %u", cpu_id);
        return;
    }

    cpu_runqueue_t *rq = &runqueues[cpu_id];
    spinlock_lock(&rq->lock);

    thread_t *current = rq->current;
    if (!current)
    {
        spinlock_unlock(&rq->lock);
        return; // No current thread
    }

    // Update current thread's virtual runtime
    uint64_t now = arch_get_timestamp();
    uint64_t delta = now - current->last_switch_time;

    // Calculate weighted delta based on thread's priority
    uint64_t weight = sched_weights[current->priority + 20]; // priority range: -20 to +19
    uint64_t weighted_delta = (delta * NICE_0_WEIGHT) / weight;

    current->virtual_runtime += weighted_delta;
    current->last_switch_time = now;
    current->actual_runtime += delta;

    // Update runqueue's minimum virtual runtime
    if (rq->nr_running > 0)
    {
        rq->min_vruntime = current->virtual_runtime;
    }

    // Calculate time slice for current thread
    uint64_t time_slice = SCHED_LATENCY_NS / (rq->nr_running ? rq->nr_running : 1);
    if (time_slice < MIN_GRANULARITY_NS)
    {
        time_slice = MIN_GRANULARITY_NS;
    }

    // Check if current thread should be preempted
    bool should_preempt = false;

    // Preempt if thread exceeded its time slice
    if (current->actual_runtime >= time_slice)
    {
        should_preempt = true;
        kdebug("scheduler_tick: Thread %llu exceeded time slice (%llu >= %llu)",
               (unsigned long long)current->tid,
               (unsigned long long)current->actual_runtime,
               (unsigned long long)time_slice);
    }

    // Preempt if there's a thread with lower virtual runtime
    if (rq->rb_root && rq->rb_root != current)
    {
        thread_t *leftmost = rb_leftmost(rq->rb_root);
        if (leftmost && leftmost->virtual_runtime + MIN_GRANULARITY_NS < current->virtual_runtime)
        {
            should_preempt = true;
            kdebug("scheduler_tick: Thread %llu should yield to %llu (virtual_runtime: %llu vs %llu)",
                   (unsigned long long)current->tid,
                   (unsigned long long)leftmost->tid,
                   (unsigned long long)current->virtual_runtime,
                   (unsigned long long)leftmost->virtual_runtime);
        }
    }

    // Update load average every 100 ticks
    if (tick_count % 100 == 0)
    {
        rq->load_weight = 0;
        thread_t *thread = rq->rb_root;
        while (thread)
        {
            if (thread->state == THREAD_STATE_READY || thread->state == THREAD_STATE_RUNNING)
            {
                rq->load_weight += sched_weights[thread->priority + 20];
            }
            thread = rb_next(thread);
        }
        rq->last_update = now;

        kdebug("scheduler_tick: CPU %u load_weight=%llu, nr_running=%u",
               cpu_id, (unsigned long long)rq->load_weight, rq->nr_running);
    }

    spinlock_unlock(&rq->lock);

    // Trigger reschedule if needed
    if (should_preempt)
    {
        // Mark thread for rescheduling by setting state to ready
        current->state = THREAD_STATE_READY;

        // Send reschedule IPI to current CPU if SMP
        extern bool smp_enabled;
        extern void apic_send_ipi(uint32_t cpu_id, uint32_t vector);

        if (smp_enabled)
        {
            // Send reschedule IPI to current CPU (assuming single CPU for now)
            apic_send_ipi(0, 0x20); // Vector 0x20 for reschedule
            kdebug("scheduler_tick: Sent reschedule IPI to CPU 0 for thread %llu",
                   (unsigned long long)current->tid);
        }
        else
        {
            kdebug("scheduler_tick: Marked thread %llu for reschedule (no IPI)",
                   (unsigned long long)current->tid);
        }
    }
}

static void rb_set_parent(thread_t *node, thread_t *parent)
{
    if (node)
        node->rb_parent = parent;
}

static void rb_set_color(thread_t *node, int color)
{
    if (node)
        node->rb_color = color;
}

static thread_t *rb_parent(thread_t *node)
{
    return node ? node->rb_parent : NULL;
}

static int rb_color(thread_t *node)
{
    return node ? node->rb_color : RB_BLACK;
}

// Get leftmost node in the tree (minimum vruntime)
static thread_t *rb_leftmost(thread_t *node)
{
    if (!node)
        return NULL;

    while (node->rb_left)
    {
        node = node->rb_left;
    }
    return node;
}

// Get next node in the tree (in-order traversal)
static thread_t *rb_next(thread_t *node)
{
    if (!node)
        return NULL;

    // If right subtree exists, return leftmost of right subtree
    if (node->rb_right)
    {
        return rb_leftmost(node->rb_right);
    }

    // Go up until we find a node that is left child of its parent
    thread_t *parent = rb_parent(node);
    while (parent && node == parent->rb_right)
    {
        node = parent;
        parent = rb_parent(parent);
    }

    return parent;
}

static void rb_rotate_left(thread_t **root, thread_t *x)
{
    thread_t *y = x->rb_right;
    x->rb_right = y->rb_left;

    if (y->rb_left)
    {
        rb_set_parent(y->rb_left, x);
    }

    rb_set_parent(y, rb_parent(x));

    if (!rb_parent(x))
    {
        *root = y;
    }
    else if (x == rb_parent(x)->rb_left)
    {
        rb_parent(x)->rb_left = y;
    }
    else
    {
        rb_parent(x)->rb_right = y;
    }

    y->rb_left = x;
    rb_set_parent(x, y);
}

static void rb_rotate_right(thread_t **root, thread_t *y)
{
    thread_t *x = y->rb_left;
    y->rb_left = x->rb_right;

    if (x->rb_right)
    {
        rb_set_parent(x->rb_right, y);
    }

    rb_set_parent(x, rb_parent(y));

    if (!rb_parent(y))
    {
        *root = x;
    }
    else if (y == rb_parent(y)->rb_left)
    {
        rb_parent(y)->rb_left = x;
    }
    else
    {
        rb_parent(y)->rb_right = x;
    }

    x->rb_right = y;
    rb_set_parent(y, x);
}

static void rb_insert_fixup(thread_t **root, thread_t *node)
{
    thread_t *parent, *gparent, *uncle;

    while ((parent = rb_parent(node)) && rb_color(parent) == RB_RED)
    {
        gparent = rb_parent(parent);

        if (parent == gparent->rb_left)
        {
            uncle = gparent->rb_right;
            if (uncle && rb_color(uncle) == RB_RED)
            {
                rb_set_color(uncle, RB_BLACK);
                rb_set_color(parent, RB_BLACK);
                rb_set_color(gparent, RB_RED);
                node = gparent;
                continue;
            }

            if (parent->rb_right == node)
            {
                rb_rotate_left(root, parent);
                thread_t *tmp = parent;
                parent = node;
                node = tmp;
            }

            rb_set_color(parent, RB_BLACK);
            rb_set_color(gparent, RB_RED);
            rb_rotate_right(root, gparent);
        }
        else
        {
            uncle = gparent->rb_left;
            if (uncle && rb_color(uncle) == RB_RED)
            {
                rb_set_color(uncle, RB_BLACK);
                rb_set_color(parent, RB_BLACK);
                rb_set_color(gparent, RB_RED);
                node = gparent;
                continue;
            }

            if (parent->rb_left == node)
            {
                rb_rotate_right(root, parent);
                thread_t *tmp = parent;
                parent = node;
                node = tmp;
            }

            rb_set_color(parent, RB_BLACK);
            rb_set_color(gparent, RB_RED);
            rb_rotate_left(root, gparent);
        }
    }

    rb_set_color(*root, RB_BLACK);
}

// ========================================
// PROCESS AND THREAD MANAGEMENT
// ========================================

static uint64_t get_nice_weight(int nice)
{
    if (nice < -20)
        nice = -20;
    if (nice > 19)
        nice = 19;
    return sched_weights[nice + 20];
}

static uint64_t calc_delta_fair(uint64_t delta, uint64_t weight)
{
    if (weight != NICE_0_WEIGHT)
    {
        delta = (delta * NICE_0_WEIGHT) / weight;
    }
    return delta;
}

process_t *scheduler_create_process(void)
{
    process_t *process = (process_t *)kmalloc(sizeof(process_t));
    if (!process)
    {
        return NULL;
    }

    // Initialize the structure
    memset(process, 0, sizeof(process_t));

    process->pid = atomic_fetch_add(&next_pid, 1);
    process->state = PROCESS_STATE_READY;
    process->creation_time = arch_get_timestamp();

    // Create address space
    process->vm_space = vmm_create_space(false);
    if (!process->vm_space)
    {
        kfree(process);
        return NULL;
    }

    // Initialize the handle table
    for (int i = 0; i < MAX_HANDLES; i++)
    {
        process->handles[i].type = HANDLE_TYPE_NONE;
    }

    // Add to global list
    spinlock_lock(&process_list_lock);
    process->next_sibling = process_list;
    process_list = process;
    total_processes++;
    spinlock_unlock(&process_list_lock);

    kdebug("Created process PID %llu", (unsigned long long)process->pid);
    return process;
}

thread_t *scheduler_create_thread(process_t *process, uint64_t entry_point, uint64_t stack_pointer, uint64_t arg)
{
    if (!process)
    {
        return NULL;
    }

    thread_t *thread = (thread_t *)kmalloc(sizeof(thread_t));
    if (!thread)
    {
        return NULL;
    }

    // Initialize the structure
    memset(thread, 0, sizeof(thread_t));

    thread->tid = atomic_fetch_add(&next_tid, 1);
    thread->state = THREAD_STATE_READY;
    thread->parent_process = process;
    thread->priority = 0; // Nice 0
    thread->nice_weight = get_nice_weight(0);
    thread->virtual_runtime = 0;
    thread->actual_runtime = 0;
    thread->last_switch_time = arch_get_timestamp();
    thread->cpu_affinity = 0xFFFFFFFFFFFFFFFFULL; // All CPUs

    // Stack and register configuration
    thread->rsp = stack_pointer;
    thread->rip = entry_point;
    thread->rflags = 0x202;     // IF set
    thread->registers[0] = arg; // RDI = first argument

    // Allocate kernel stack if necessary
    if (!stack_pointer)
    {
        uint64_t stack_vaddr = vmm_alloc_pages(process->vm_space, 2, VM_FLAG_READ | VM_FLAG_WRITE);
        if (!stack_vaddr)
        {
            kfree(thread);
            return NULL;
        }
        thread->stack_base = stack_vaddr;
        thread->stack_size = 2 * PAGE_SIZE;
        thread->rsp = stack_vaddr + thread->stack_size - 8; // Top of stack
    }

    // Add to process
    thread->next = process->threads;
    if (process->threads)
    {
        process->threads->prev = thread;
    }
    process->threads = thread;
    process->thread_count++;

    if (!process->main_thread)
    {
        process->main_thread = thread;
    }

    total_threads++;

    kdebug("Created thread TID %llu for process PID %llu",
           (unsigned long long)thread->tid, (unsigned long long)process->pid);

    return thread;
}

void scheduler_add_thread_to_rq(thread_t *thread)
{
    uint32_t cpu = arch_get_current_cpu();
    cpu_runqueue_t *rq = &runqueues[cpu];

    spinlock_lock(&rq->lock);

    // Update virtual runtime
    if (rq->rb_root)
    {
        thread->virtual_runtime = MAX(thread->virtual_runtime, rq->min_vruntime);
    }

    // Insertion into red-black tree
    thread_t **link = &rq->rb_root;
    thread_t *parent = NULL;

    while (*link)
    {
        parent = *link;
        if (thread->virtual_runtime < parent->virtual_runtime)
        {
            link = &parent->rb_left;
        }
        else
        {
            link = &parent->rb_right;
        }
    }

    thread->rb_left = thread->rb_right = NULL;
    thread->rb_parent = parent;
    thread->rb_color = RB_RED;
    *link = thread;

    rb_insert_fixup(&rq->rb_root, thread);

    rq->nr_running++;
    rq->load_weight += thread->nice_weight;

    spinlock_unlock(&rq->lock);
}

static thread_t *pick_next_thread(cpu_runqueue_t *rq)
{
    thread_t *leftmost = rq->rb_root;

    if (!leftmost)
    {
        return NULL;
    }

    // Find leftmost thread (smallest virtual runtime)
    while (leftmost->rb_left)
    {
        leftmost = leftmost->rb_left;
    }

    return leftmost;
}

static void remove_thread_from_rq(cpu_runqueue_t *rq, thread_t *thread)
{
    // Remove from red-black tree (simplified implementation)
    if (thread->rb_left && thread->rb_right)
    {
        // Cas complexe: remplacer par le successeur
        thread_t *successor = thread->rb_right;
        while (successor->rb_left)
        {
            successor = successor->rb_left;
        }

        // Swap values
        uint64_t tmp_vruntime = thread->virtual_runtime;
        thread->virtual_runtime = successor->virtual_runtime;
        successor->virtual_runtime = tmp_vruntime;

        thread = successor; // Supprimer le successeur maintenant
    }

    thread_t *child = thread->rb_left ? thread->rb_left : thread->rb_right;
    thread_t *parent = thread->rb_parent;

    if (child)
    {
        child->rb_parent = parent;
    }

    if (parent)
    {
        if (parent->rb_left == thread)
        {
            parent->rb_left = child;
        }
        else
        {
            parent->rb_right = child;
        }
    }
    else
    {
        rq->rb_root = child;
    }

    rq->nr_running--;
    rq->load_weight -= thread->nice_weight;
}

// ========================================
// SCHEDULER PRINCIPAL
// ========================================

void scheduler_init(void)
{
    kinfo("Initializing CFS scheduler");

    // Initialize runqueues per CPU
    uint32_t cpu_count = arch_get_cpu_count();
    for (uint32_t i = 0; i < cpu_count && i < MAX_CPUS; i++)
    {
        cpu_runqueue_t *rq = &runqueues[i];
        memset(rq, 0, sizeof(cpu_runqueue_t));
        spinlock_init(&rq->lock);
        rq->min_vruntime = 0;
        rq->last_update = arch_get_timestamp();
    }

    // Create init process (PID 1)
    init_process = scheduler_create_process();
    if (init_process)
    {
        init_process->pid = 1; // Force PID 1
        kdebug("Created init process PID 1");
    }

    scheduler_initialized = true;
    kinfo("CFS scheduler initialized for %u CPUs", cpu_count);
}

void sched_yield(void)
{
    if (!scheduler_initialized)
    {
        return;
    }

    uint32_t cpu = arch_get_current_cpu();
    cpu_runqueue_t *rq = &runqueues[cpu];

    spinlock_lock(&rq->lock);

    thread_t *current = rq->current;
    if (!current)
    {
        spinlock_unlock(&rq->lock);
        return;
    }

    // Update runtime
    uint64_t now = arch_get_timestamp();
    uint64_t delta = now - current->last_switch_time;
    current->actual_runtime += delta;
    current->virtual_runtime += calc_delta_fair(delta, current->nice_weight);
    current->last_switch_time = now;

    // Put back in runqueue if still RUNNING
    if (current->state == THREAD_STATE_RUNNING)
    {
        current->state = THREAD_STATE_READY;
        scheduler_add_thread_to_rq(current);
    }

    // Select next thread
    thread_t *next = pick_next_thread(rq);
    if (next)
    {
        remove_thread_from_rq(rq, next);
        next->state = THREAD_STATE_RUNNING;
        next->last_switch_time = now;
        rq->current = next;

        if (next != current)
        {
            kdebug("Context switch: TID %llu -> TID %llu",
                   (unsigned long long)(current ? current->tid : 0),
                   (unsigned long long)next->tid);

            // Perform context switch
            arch_context_switch(current, next);
        }
    }
    else
    {
        rq->current = NULL;
    }

    spinlock_unlock(&rq->lock);

    // If no thread to execute, idle
    if (!rq->current)
    {
        arch_cpu_idle();
    }
}

// ========================================
// PUBLIC SCHEDULER API
// ========================================

process_t *scheduler_get_current_process(void)
{
    uint32_t cpu = arch_get_current_cpu();
    thread_t *current = runqueues[cpu].current;
    return current ? current->parent_process : NULL;
}

thread_t *scheduler_get_current_thread(void)
{
    uint32_t cpu = arch_get_current_cpu();
    return runqueues[cpu].current;
}

process_t *scheduler_find_process(uint64_t pid)
{
    spinlock_lock(&process_list_lock);

    process_t *proc = process_list;
    while (proc)
    {
        if (proc->pid == pid)
        {
            spinlock_unlock(&process_list_lock);
            return proc;
        }
        proc = proc->next_sibling;
    }

    spinlock_unlock(&process_list_lock);
    return NULL;
}

void scheduler_add_process(process_t *process)
{
    if (!process || !process->main_thread)
    {
        return;
    }

    scheduler_add_thread_to_rq(process->main_thread);
}

void scheduler_block_current_process(void)
{
    uint32_t cpu = arch_get_current_cpu();
    thread_t *current = runqueues[cpu].current;

    if (current)
    {
        current->state = THREAD_STATE_BLOCKED;
        if (current->parent_process)
        {
            current->parent_process->state = PROCESS_STATE_BLOCKED;
        }
    }
}

void scheduler_wakeup_process(process_t *process)
{
    if (!process || !process->main_thread)
    {
        return;
    }

    thread_t *thread = process->main_thread;
    if (thread->state == THREAD_STATE_BLOCKED)
    {
        thread->state = THREAD_STATE_READY;
        process->state = PROCESS_STATE_READY;
        scheduler_add_thread_to_rq(thread);
    }
}

void scheduler_sleep_ns(uint64_t nanoseconds)
{
    uint32_t cpu = arch_get_current_cpu();
    thread_t *current = runqueues[cpu].current;

    if (current)
    {
        current->sleep_until = arch_get_timestamp() + nanoseconds;
        current->state = THREAD_STATE_BLOCKED;
        sched_yield();
    }
}

uint64_t scheduler_get_process_count(void)
{
    return total_processes;
}

uint64_t scheduler_get_thread_count(void)
{
    return total_threads;
}

void scheduler_destroy_process(process_t *process)
{
    if (!process)
        return;

    // Nettoyer tous les threads
    thread_t *thread = process->threads;
    while (thread)
    {
        thread_t *next = thread->next;

        // Remove from runqueue if necessary
        uint32_t cpu = arch_get_current_cpu();
        cpu_runqueue_t *rq = &runqueues[cpu];

        spinlock_lock(&rq->lock);
        if (rq->current == thread)
        {
            rq->current = NULL;
        }
        // Remove from RB tree if present
        if (thread->rb_parent != NULL || thread == rq->rb_root)
        {
            // Remove from RB tree (simplified removal)
            if (thread->rb_parent)
            {
                if (thread->rb_parent->rb_left == thread)
                {
                    thread->rb_parent->rb_left = thread->rb_left ? thread->rb_left : thread->rb_right;
                }
                else
                {
                    thread->rb_parent->rb_right = thread->rb_left ? thread->rb_left : thread->rb_right;
                }
            }
            else
            {
                rq->rb_root = thread->rb_left ? thread->rb_left : thread->rb_right;
            }
            rq->nr_running--;
        }
        spinlock_unlock(&rq->lock);

        kfree(thread);
        total_threads--;
        thread = next;
    }

    // Free address space
    if (process->vm_space && !process->vm_space->is_kernel)
    {
        vmm_destroy_space(process->vm_space);
    }

    // Retirer de la liste globale
    spinlock_lock(&process_list_lock);
    if (process_list == process)
    {
        process_list = process->next_sibling;
    }
    else
    {
        process_t *prev = process_list;
        while (prev && prev->next_sibling != process)
        {
            prev = prev->next_sibling;
        }
        if (prev)
        {
            prev->next_sibling = process->next_sibling;
        }
    }
    total_processes--;
    spinlock_unlock(&process_list_lock);

    kfree(process);
}

// ========================================
// UTILITIES AND STUBS
// ========================================

void handle_cleanup(handle_t *handle)
{
    if (!handle || handle->type == HANDLE_TYPE_NONE)
    {
        return;
    }

    // Nettoyer selon le type de handle
    switch (handle->type)
    {
    case HANDLE_TYPE_PROCESS:
    case HANDLE_TYPE_THREAD:
    case HANDLE_TYPE_IPC_PORT:
    case HANDLE_TYPE_MEMORY:
        // Clean up memory region
        if (handle->object_id)
        {
            // Get process context for memory cleanup
            process_t *owner_process = scheduler_find_process(handle->ref_count); // Use ref_count as temp PID store
            if (owner_process && owner_process->vm_space)
            {
                // Unmap the memory region
                vmm_free_pages(owner_process->vm_space, handle->object_id, 1);
                kinfo("Memory handle cleanup: freed region 0x%llx for PID %llu",
                      (unsigned long long)handle->object_id,
                      (unsigned long long)owner_process->pid);
            }
            else
            {
                kdebug("Memory handle cleanup: region 0x%llx (owner process not found)",
                       (unsigned long long)handle->object_id);
            }
        }
        break;

    case HANDLE_TYPE_TIMER:
        // Clean up timer object
        if (handle->object_id)
        {
            // Cancel and clean up timer
            if (timer_cancel(handle->object_id) != OR_OK)
            {
                kwarn("Failed to cancel timer %llu during cleanup",
                      (unsigned long long)handle->object_id);
            }
            else
            {
                kdebug("Successfully cleaned up timer %llu",
                       (unsigned long long)handle->object_id);
            }
        }
        break;
    default:
        break;
    }

    handle->type = HANDLE_TYPE_NONE;
    handle->ref_count = 0;
}

// ELF file header structure
typedef struct
{
    uint8_t e_ident[16];  // ELF identification
    uint16_t e_type;      // Object file type
    uint16_t e_machine;   // Machine type
    uint32_t e_version;   // Object file version
    uint64_t e_entry;     // Entry point address
    uint64_t e_phoff;     // Program header offset
    uint64_t e_shoff;     // Section header offset
    uint32_t e_flags;     // Processor-specific flags
    uint16_t e_ehsize;    // ELF header size
    uint16_t e_phentsize; // Program header entry size
    uint16_t e_phnum;     // Number of program header entries
    uint16_t e_shentsize; // Section header entry size
    uint16_t e_shnum;     // Number of section header entries
    uint16_t e_shstrndx;  // Section name string table index
} elf64_ehdr_t;

// Program header structure
typedef struct
{
    uint32_t p_type;   // Segment type
    uint32_t p_flags;  // Segment flags
    uint64_t p_offset; // Segment file offset
    uint64_t p_vaddr;  // Segment virtual address
    uint64_t p_paddr;  // Segment physical address
    uint64_t p_filesz; // Segment size in file
    uint64_t p_memsz;  // Segment size in memory
    uint64_t p_align;  // Segment alignment
} elf64_phdr_t;

// ELF constants
#define ELF_MAGIC 0x464C457F // "\x7FELF"
#define ET_EXEC 2            // Executable file
#define EM_X86_64 62         // x86-64
#define PT_LOAD 1            // Loadable segment
#define PF_X 1               // Execute
#define PF_W 2               // Write
#define PF_R 4               // Read

int elf_load_process(process_t *process, const char *path)
{
    if (!process || !path)
    {
        return -OR_EINVAL;
    }

    // Load ELF file from filesystem
    kinfo("Loading ELF process from: %s", path);

    // Implement basic ELF loading workflow
    // Note: This is a simplified implementation until full VFS is available

    // 1. Open file through VFS (simplified)
    extern int vfs_open(const char *path, int flags);
    int fd = vfs_open(path, 0); // Read-only
    if (fd < 0)
    {
        kerror("elf_load_process: Failed to open ELF file: %s", path);
        return -OR_ENOENT;
    }

    // 2. Read ELF header (simplified placeholder)
    typedef struct
    {
        uint32_t magic;     // ELF magic number
        uint8_t class;      // 32-bit or 64-bit
        uint8_t data;       // Endianness
        uint8_t version;    // ELF version
        uint64_t entry;     // Entry point address
        uint64_t phoff;     // Program header offset
        uint16_t phentsize; // Program header entry size
        uint16_t phnum;     // Number of program headers
    } elf_header_t;

    elf_header_t elf_header = {0};

    // Try to read actual ELF header from file when VFS read is available
    extern bool vfs_is_available(void);
    extern int vfs_read_file(const char *path, void *buffer, size_t size, size_t offset);

    bool header_read = false;

    // Note: exec_path field not available in process struct
    // Using entry_point as fallback for ELF validation
    if (vfs_is_available() && process->entry_point)
    {
        // Try to read ELF header from the entry point
        int read_result = vfs_read_file("", &elf_header, sizeof(elf_header), 0);
        if (read_result == sizeof(elf_header))
        {
            // Verify ELF magic
            if (elf_header.magic == 0x464C457F)
            {
                header_read = true;
                kdebug("ELF header read successfully from entry point 0x%llx", (unsigned long long)process->entry_point);
                kdebug("  Entry point: 0x%llx", (unsigned long long)elf_header.entry);
                kdebug("  Program headers: %u at offset 0x%llx",
                       elf_header.phnum, (unsigned long long)elf_header.phoff);
            }
            else
            {
                kwarn("Invalid ELF magic at entry point 0x%llx: 0x%x",
                      (unsigned long long)process->entry_point, elf_header.magic);
            }
        }
        else
        {
            kdebug("Failed to read ELF header from entry point 0x%llx (error: %d), using defaults",
                   (unsigned long long)process->entry_point, read_result);
        }
    }

    // Use default values if header couldn't be read
    if (!header_read)
    {
        elf_header.magic = 0x464C457F; // ELF magic
        elf_header.entry = 0x400000;   // Standard entry point
        kdebug("Using default ELF header values");
    }

    // For now, create a simple placeholder process with basic memory layout

    // Set up basic process memory layout
    process->entry_point = 0x400000;        // Standard ELF entry point
    process->heap_start = 0x600000;         // Heap starts after code/data
    process->stack_top = 0x7FFFFFFFF000ULL; // Stack at top of user space

    // Allocate and map basic pages for the process
    vm_space_t *vm_space = process->vm_space;
    if (!vm_space)
    {
        return -OR_ENOMEM;
    }

    // Map code segment (executable, read-only)
    uint64_t code_pages = 16; // 64KB for code
    uint64_t code_vaddr = vmm_alloc_pages(vm_space, code_pages,
                                          VM_FLAG_READ | VM_FLAG_EXEC | VM_FLAG_USER);
    if (!code_vaddr)
    {
        kerror("Failed to allocate code pages for process");
        return -OR_ENOMEM;
    }

    // Map data segment (read-write)
    uint64_t data_pages = 16; // 64KB for data
    uint64_t data_vaddr = vmm_alloc_pages(vm_space, data_pages,
                                          VM_FLAG_READ | VM_FLAG_WRITE | VM_FLAG_USER);
    if (!data_vaddr)
    {
        kerror("Failed to allocate data pages for process");
        vmm_free_pages(vm_space, code_vaddr, code_pages);
        return -OR_ENOMEM;
    }

    // Map stack (read-write, guard pages)
    uint64_t stack_pages = 32; // 128KB stack
    uint64_t stack_vaddr = vmm_alloc_pages(vm_space, stack_pages,
                                           VM_FLAG_READ | VM_FLAG_WRITE | VM_FLAG_USER);
    if (!stack_vaddr)
    {
        kerror("Failed to allocate stack pages for process");
        vmm_free_pages(vm_space, code_vaddr, code_pages);
        vmm_free_pages(vm_space, data_vaddr, data_pages);
        return -OR_ENOMEM;
    }

    // Set up process memory regions
    process->code_base = code_vaddr;
    process->code_size = code_pages * PAGE_SIZE;
    process->data_base = data_vaddr;
    process->data_size = data_pages * PAGE_SIZE;
    process->stack_base = stack_vaddr;
    process->stack_size = stack_pages * PAGE_SIZE;

    kdebug("ELF process layout:");
    kdebug("  Entry point: 0x%p", (void *)process->entry_point);
    kdebug("  Code: 0x%p - 0x%p (%llu KB)",
           (void *)process->code_base,
           (void *)(process->code_base + process->code_size),
           (unsigned long long)(process->code_size / 1024));
    kdebug("  Data: 0x%p - 0x%p (%llu KB)",
           (void *)process->data_base,
           (void *)(process->data_base + process->data_size),
           (unsigned long long)(process->data_size / 1024));
    kdebug("  Stack: 0x%p - 0x%p (%llu KB)",
           (void *)process->stack_base,
           (void *)(process->stack_base + process->stack_size),
           (unsigned long long)(process->stack_size / 1024));

    return OR_OK;
}

int process_setup_args(process_t *process, char *const argv[], char *const envp[])
{
    if (!process)
    {
        return -OR_EINVAL;
    }

    // Calculate stack layout for arguments
    uint64_t stack_top = process->stack_base + process->stack_size;
    uint64_t stack_ptr = stack_top;

    // Count arguments and environment variables
    uint32_t argc = 0;
    uint32_t envc = 0;

    if (argv)
    {
        while (argv[argc] != NULL)
        {
            argc++;
        }
    }

    if (envp)
    {
        while (envp[envc] != NULL)
        {
            envc++;
        }
    }

    // Allocate space for strings on stack (working backwards)
    char **argv_ptrs = NULL;
    char **envp_ptrs = NULL;

    if (argc > 0)
    {
        argv_ptrs = (char **)kmalloc(argc * sizeof(char *));
        if (!argv_ptrs)
        {
            return -OR_ENOMEM;
        }

        // Copy argument strings to stack
        for (int i = argc - 1; i >= 0; i--)
        {
            size_t len = strlen(argv[i]) + 1;
            stack_ptr -= len;

            // Copy string to user stack
            if (copy_string_to_user((char *)stack_ptr, argv[i]) != OR_OK)
            {
                kfree(argv_ptrs);
                return -OR_EFAULT;
            }
            argv_ptrs[i] = (char *)stack_ptr;
        }
    }

    if (envc > 0)
    {
        envp_ptrs = (char **)kmalloc(envc * sizeof(char *));
        if (!envp_ptrs)
        {
            kfree(argv_ptrs);
            return -OR_ENOMEM;
        }

        // Copy environment strings to stack
        for (int i = envc - 1; i >= 0; i--)
        {
            size_t len = strlen(envp[i]) + 1;
            stack_ptr -= len;

            // Copy string to user stack
            if (copy_string_to_user((char *)stack_ptr, envp[i]) != OR_OK)
            {
                kfree(argv_ptrs);
                kfree(envp_ptrs);
                return -OR_EFAULT;
            }
            envp_ptrs[i] = (char *)stack_ptr;
        }
    }

    // Align stack pointer to 16 bytes (x86_64 ABI requirement)
    stack_ptr &= ~0xFULL;

    // Build the stack layout (from high to low addresses):
    // - NULL (end of envp)
    // - environment pointers
    // - NULL (end of argv)
    // - argument pointers
    // - argc

    // Store environment pointers
    uint64_t envp_addr = 0;
    if (envc > 0)
    {
        stack_ptr -= sizeof(char *); // NULL terminator
        char *null_ptr = NULL;
        copy_to_user((void *)stack_ptr, &null_ptr, sizeof(char *));

        stack_ptr -= envc * sizeof(char *);
        envp_addr = stack_ptr;
        copy_to_user((void *)stack_ptr, envp_ptrs, envc * sizeof(char *));
    }

    // Store argument pointers
    uint64_t argv_addr = 0;
    if (argc > 0)
    {
        stack_ptr -= sizeof(char *); // NULL terminator
        char *null_ptr = NULL;
        copy_to_user((void *)stack_ptr, &null_ptr, sizeof(char *));

        stack_ptr -= argc * sizeof(char *);
        argv_addr = stack_ptr;
        copy_to_user((void *)stack_ptr, argv_ptrs, argc * sizeof(char *));
    }

    // Store argc
    stack_ptr -= sizeof(uint64_t);
    copy_to_user((void *)stack_ptr, &argc, sizeof(uint64_t));

    // Set up initial thread context
    if (process->main_thread)
    {
        // Set up registers for process start
        // RSP points to argc on stack
        // RDI = argc (first argument)
        // RSI = argv (second argument)
        // RDX = envp (third argument)

        process->main_thread->user_stack = stack_ptr;
        process->main_thread->rsp = stack_ptr;
        process->main_thread->registers[7] = argc;                      // RDI
        process->main_thread->registers[6] = argv_addr ? argv_addr : 0; // RSI
        process->main_thread->registers[2] = envp_addr ? envp_addr : 0; // RDX
        process->main_thread->rip = process->entry_point;

        // Set up user mode flags
        process->main_thread->rflags = 0x202; // IF flag set, IOPL=0

        // Note: CS and SS are not stored in the registers array, they would be
        // handled by the context switch mechanism
    }

    // Clean up temporary allocations
    kfree(argv_ptrs);
    kfree(envp_ptrs);

    kdebug("Process arguments setup: argc=%u, envc=%u, stack=0x%p",
           argc, envc, (void *)stack_ptr);

    return OR_OK;
}

// Signal definitions (POSIX-compatible)
#define SIGHUP 1   // Hangup
#define SIGINT 2   // Interrupt
#define SIGQUIT 3  // Quit
#define SIGILL 4   // Illegal instruction
#define SIGTRAP 5  // Trace/breakpoint trap
#define SIGABRT 6  // Abort
#define SIGBUS 7   // Bus error
#define SIGFPE 8   // Floating point exception
#define SIGKILL 9  // Kill (cannot be caught or ignored)
#define SIGUSR1 10 // User-defined signal 1
#define SIGSEGV 11 // Segmentation violation
#define SIGUSR2 12 // User-defined signal 2
#define SIGPIPE 13 // Broken pipe
#define SIGALRM 14 // Alarm clock
#define SIGTERM 15 // Termination
#define SIGCHLD 17 // Child status changed
#define SIGCONT 18 // Continue
#define SIGSTOP 19 // Stop (cannot be caught or ignored)
#define SIGTSTP 20 // Keyboard stop

int signal_send(process_t *target, uint32_t signal)
{
    if (!target || signal == 0 || signal > 31)
    {
        return -OR_EINVAL;
    }

    // Check if process is valid and not zombie
    if (target->state == PROCESS_STATE_ZOMBIE || target->state == PROCESS_STATE_TERMINATED)
    {
        return -OR_ESRCH; // No such process
    }

    // Special handling for uncatchable signals
    if (signal == SIGKILL || signal == SIGSTOP)
    {
        // These signals cannot be blocked or ignored
        if (signal == SIGKILL)
        {
            // Terminate the process immediately
            target->state = PROCESS_STATE_TERMINATED;
            target->exit_code = 128 + signal; // Convention: 128 + signal number

            // Wake up parent if waiting
            if (target->parent && target->parent->state == PROCESS_STATE_WAITING)
            {
                target->parent->state = PROCESS_STATE_READY;
                scheduler_add_thread_to_rq(target->parent->main_thread);
            }

            kdebug("Process PID %llu killed by SIGKILL",
                   (unsigned long long)target->pid);
            return OR_OK;
        }
        else if (signal == SIGSTOP)
        {
            // Stop the process
            target->state = PROCESS_STATE_STOPPED;
            kdebug("Process PID %llu stopped by SIGSTOP",
                   (unsigned long long)target->pid);
            return OR_OK;
        }
    }

    // Set the signal bit in the process's pending signals
    target->pending_signals |= (1ULL << signal);

    // If process is sleeping, wake it up to handle the signal
    if (target->state == PROCESS_STATE_SLEEPING)
    {
        target->state = PROCESS_STATE_READY;
        if (target->main_thread)
        {
            scheduler_add_thread_to_rq(target->main_thread);
        }
    }

    kdebug("Signal %u sent to process PID %llu",
           signal, (unsigned long long)target->pid);

    return OR_OK;
}

// Check and handle pending signals for a process
void process_handle_signals(process_t *process)
{
    if (!process || process->pending_signals == 0)
    {
        return;
    }

    // Scan for pending signals (priority order)
    for (uint32_t sig = 1; sig <= 31; sig++)
    {
        if (process->pending_signals & (1ULL << sig))
        {
            // Clear the signal bit
            process->pending_signals &= ~(1ULL << sig);

            // Handle the signal based on type
            switch (sig)
            {
            case SIGTERM:
            case SIGINT:
            case SIGQUIT:
                // Default action: terminate process
                process->state = PROCESS_STATE_TERMINATED;
                process->exit_code = 128 + sig;
                kdebug("Process PID %llu terminated by signal %u",
                       (unsigned long long)process->pid, sig);
                break;

            case SIGCHLD:
                // Child process status changed - usually ignored
                kdebug("SIGCHLD received by PID %llu",
                       (unsigned long long)process->pid);
                break;

            case SIGCONT:
                // Continue a stopped process
                if (process->state == PROCESS_STATE_STOPPED)
                {
                    process->state = PROCESS_STATE_READY;
                    if (process->main_thread)
                    {
                        scheduler_add_thread_to_rq(process->main_thread);
                    }
                    kdebug("Process PID %llu continued by SIGCONT",
                           (unsigned long long)process->pid);
                }
                break;

            case SIGUSR1:
            case SIGUSR2:
                // User-defined signals - log and continue
                kdebug("User signal %u received by PID %llu",
                       sig, (unsigned long long)process->pid);
                break;

            default:
                // Default action for most signals: terminate
                process->state = PROCESS_STATE_TERMINATED;
                process->exit_code = 128 + sig;
                kdebug("Process PID %llu terminated by signal %u (default action)",
                       (unsigned long long)process->pid, sig);
                break;
            }

            // Only handle one signal per call to avoid infinite loops
            break;
        }
    }
}

// Old interfaces for compatibility
void thread_exit(int exit_code)
{
    thread_t *current = scheduler_get_current_thread();
    if (current)
    {
        current->state = THREAD_STATE_TERMINATED;
        if (current->parent_process)
        {
            current->parent_process->exit_code = exit_code;
            current->parent_process->state = PROCESS_STATE_ZOMBIE;
        }
    }

    sched_yield();

    // Ne devrait jamais arriver
    while (1)
    {
        arch_cpu_idle();
    }
}
