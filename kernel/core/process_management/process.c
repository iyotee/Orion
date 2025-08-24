/*
 * Orion Operating System - Process Management
 *
 * Complete process creation, management, and lifecycle implementation.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/kernel.h>
#include <orion/types.h>
#include <orion/structures.h>
#include <orion/mm.h>
#include <orion/security.h>

// ========================================
// CONSTANTS AND DEFINITIONS
// ========================================

#define MAX_PROCESSES 1024
#define MAX_THREADS_PER_PROCESS 64
#define PROCESS_STACK_SIZE (64 * 1024)          // 64KB default stack
#define PROCESS_HEAP_INITIAL_SIZE (1024 * 1024) // 1MB initial heap

// Process states
#define PROCESS_STATE_NEW 0
#define PROCESS_STATE_READY 1
#define PROCESS_STATE_RUNNING 2
#define PROCESS_STATE_BLOCKED 3
#define PROCESS_STATE_TERMINATED 4
#define PROCESS_STATE_ZOMBIE 5

// Process flags
#define PROCESS_FLAG_SYSTEM 0x00000001
#define PROCESS_FLAG_USER 0x00000002
#define PROCESS_FLAG_PRIVILEGED 0x00000004
#define PROCESS_FLAG_SANDBOXED 0x00000008

// Process priorities
#define PROCESS_PRIORITY_LOW 0
#define PROCESS_PRIORITY_NORMAL 1
#define PROCESS_PRIORITY_HIGH 2
#define PROCESS_PRIORITY_CRITICAL 3

// ========================================
// GLOBAL VARIABLES
// ========================================

static process_t *g_processes[MAX_PROCESSES];
static atomic64_t g_next_pid = ATOMIC_VAR_INIT(1);
static spinlock_t g_process_table_lock = SPINLOCK_INIT;

// ========================================
// PROCESS CREATION AND MANAGEMENT
// ========================================

/**
 * Create a new process
 *
 * @param name Process name
 * @param entry_point Entry point address
 * @param stack_pointer Initial stack pointer
 * @return Pointer to created process or NULL on failure
 */
process_t *process_create(const char *name, uint64_t entry_point, uint64_t stack_pointer)
{
    if (!name)
    {
        kerror("process_create: Invalid name");
        return NULL;
    }

    spinlock_lock(&g_process_table_lock);

    // Find a free process slot
    uint32_t free_slot = 0;
    bool slot_found = false;

    for (uint32_t i = 0; i < MAX_PROCESSES; i++)
    {
        if (!g_processes[i])
        {
            free_slot = i;
            slot_found = true;
            break;
        }
    }

    if (!slot_found)
    {
        spinlock_unlock(&g_process_table_lock);
        kerror("process_create: No free process slots");
        return NULL;
    }

    // Allocate process structure
    process_t *process = (process_t *)kmalloc(sizeof(process_t));
    if (!process)
    {
        spinlock_unlock(&g_process_table_lock);
        kerror("process_create: Failed to allocate process structure");
        return NULL;
    }

    // Initialize process structure
    memset(process, 0, sizeof(process_t));

    // Assign PID
    process->pid = atomic_fetch_add(&g_next_pid, 1);
    process->state = PROCESS_STATE_NEW;
    process->exit_code = 0;

    // Copy process name
    size_t name_len = strlen(name);
    if (name_len > 31)
        name_len = 31;
    memcpy(process->name, name, name_len);
    process->name[name_len] = '\0';

    // Set entry point and stack
    process->entry_point = entry_point;
    process->stack_top = stack_pointer;
    process->stack_base = stack_pointer - PROCESS_STACK_SIZE;
    process->stack_size = PROCESS_STACK_SIZE;

    // Initialize memory layout
    process->code_base = entry_point;
    process->code_size = 0; // Will be set when ELF is loaded
    process->data_base = 0;
    process->data_size = 0;
    process->heap_start = 0;
    // process->brk = 0; // TODO: Add brk field to process struct

    // Initialize virtual memory space
    process->vm_space = vmm_create_space();
    if (!process->vm_space)
    {
        kfree(process);
        spinlock_unlock(&g_process_table_lock);
        kerror("process_create: Failed to create VM space");
        return NULL;
    }

    // Initialize thread list
    process->main_thread = NULL;
    process->threads = NULL;
    process->thread_count = 0;

    // Initialize hierarchy
    process->parent = NULL;
    process->children = NULL;
    process->next_sibling = NULL;

    // Initialize handles
    memset(process->handles, 0, sizeof(process->handles));

    // Initialize arguments and environment
    process->argv = NULL;
    process->envp = NULL;

    // Initialize signal handling
    process->pending_signals = 0;

    // Initialize timing
    process->creation_time = arch_get_timestamp();
    process->cpu_time_total = 0;

    // Add to process table
    g_processes[free_slot] = process;

    // Add to scheduler
    scheduler_add_process(process);

    spinlock_unlock(&g_process_table_lock);

    kinfo("Created process '%s' with PID %llu", process->name, (unsigned long long)process->pid);

    return process;
}

/**
 * Start a process
 *
 * @param process Process to start
 * @return 0 on success, negative error code on failure
 */
int process_start(process_t *process)
{
    if (!process)
    {
        return -OR_EINVAL;
    }

    if (process->state != PROCESS_STATE_NEW && process->state != PROCESS_STATE_READY)
    {
        kerror("process_start: Process %llu not in valid state (%d)",
               (unsigned long long)process->pid, process->state);
        return -OR_EINVAL;
    }

    // Set process state to ready
    process->state = PROCESS_STATE_READY;

    // If process has a main thread, start it
    if (process->main_thread)
    {
        // Add thread to scheduler
        scheduler_add_thread_to_rq(process->main_thread);
        kinfo("Started process '%s' (PID %llu) with main thread",
              process->name, (unsigned long long)process->pid);
    }
    else
    {
        kinfo("Started process '%s' (PID %llu) without main thread",
              process->name, (unsigned long long)process->pid);
    }

    return OR_OK;
}

/**
 * Destroy a process
 *
 * @param process Process to destroy
 */
void process_destroy(process_t *process)
{
    if (!process)
        return;

    kinfo("Destroying process '%s' (PID %llu)", process->name, (unsigned long long)process->pid);

    // Stop all threads
    thread_t *thread = process->threads;
    while (thread)
    {
        thread_t *next = thread->next;
        thread_destroy(thread);
        thread = next;
    }

    // Free virtual memory space
    if (process->vm_space)
    {
        vmm_destroy_space(process->vm_space);
    }

    // Free arguments and environment
    if (process->argv)
    {
        for (int i = 0; process->argv[i]; i++)
        {
            kfree(process->argv[i]);
        }
        kfree(process->argv);
    }

    if (process->envp)
    {
        for (int i = 0; process->envp[i]; i++)
        {
            kfree(process->envp[i]);
        }
        kfree(process->envp);
    }

    // Remove from process table
    spinlock_lock(&g_process_table_lock);
    for (uint32_t i = 0; i < MAX_PROCESSES; i++)
    {
        if (g_processes[i] == process)
        {
            g_processes[i] = NULL;
            break;
        }
    }
    spinlock_unlock(&g_process_table_lock);

    // Free process structure
    kfree(process);
}

/**
 * Find process by PID
 *
 * @param pid Process ID to find
 * @return Pointer to process or NULL if not found
 */
process_t *process_find(uint64_t pid)
{
    if (pid == 0)
        return NULL;

    spinlock_lock(&g_process_table_lock);

    for (uint32_t i = 0; i < MAX_PROCESSES; i++)
    {
        if (g_processes[i] && g_processes[i]->pid == pid)
        {
            spinlock_unlock(&g_process_table_lock);
            return g_processes[i];
        }
    }

    spinlock_unlock(&g_process_table_lock);
    return NULL;
}

/**
 * Get current process
 *
 * @return Pointer to current process or NULL
 */
process_t *process_get_current(void)
{
    thread_t *current_thread = scheduler_get_current_thread();
    if (!current_thread)
        return NULL;

    return current_thread->parent_process;
}

/**
 * Set process arguments
 *
 * @param process Process to set arguments for
 * @param argv Argument vector
 * @param envp Environment vector
 * @return 0 on success, negative error code on failure
 */
int process_set_args(process_t *process, char *const argv[], char *const envp[])
{
    if (!process)
        return -OR_EINVAL;

    // Free existing arguments
    if (process->argv)
    {
        for (int i = 0; process->argv[i]; i++)
        {
            kfree(process->argv[i]);
        }
        kfree(process->argv);
        process->argv = NULL;
    }

    if (process->envp)
    {
        for (int i = 0; process->envp[i]; i++)
        {
            kfree(process->envp[i]);
        }
        kfree(process->envp);
        process->envp = NULL;
    }

    // Count arguments
    int argc = 0;
    if (argv)
    {
        while (argv[argc])
            argc++;
    }

    int envc = 0;
    if (envp)
    {
        while (envp[envc])
            envc++;
    }

    // Allocate and copy arguments
    if (argc > 0)
    {
        process->argv = (char **)kmalloc((argc + 1) * sizeof(char *));
        if (!process->argv)
            return -OR_ENOMEM;

        for (int i = 0; i < argc; i++)
        {
            size_t len = strlen(argv[i]) + 1;
            process->argv[i] = (char *)kmalloc(len);
            if (!process->argv[i])
            {
                // Cleanup on failure
                for (int j = 0; j < i; j++)
                {
                    kfree(process->argv[j]);
                }
                kfree(process->argv);
                process->argv = NULL;
                return -OR_ENOMEM;
            }
            memcpy(process->argv[i], argv[i], len);
        }
        process->argv[argc] = NULL;
    }

    // Allocate and copy environment
    if (envc > 0)
    {
        process->envp = (char **)kmalloc((envc + 1) * sizeof(char *));
        if (!process->envp)
        {
            // Cleanup on failure
            if (process->argv)
            {
                for (int i = 0; process->argv[i]; i++)
                {
                    kfree(process->argv[i]);
                }
                kfree(process->argv);
                process->argv = NULL;
            }
            return -OR_ENOMEM;
        }

        for (int i = 0; i < envc; i++)
        {
            size_t len = strlen(envp[i]) + 1;
            process->envp[i] = (char *)kmalloc(len);
            if (!process->envp[i])
            {
                // Cleanup on failure
                for (int j = 0; j < i; j++)
                {
                    kfree(process->envp[j]);
                }
                kfree(process->envp);
                process->envp = NULL;
                return -OR_ENOMEM;
            }
            memcpy(process->envp[i], envp[i], len);
        }
        process->envp[envc] = NULL;
    }

    kinfo("Set %d arguments and %d environment variables for process %llu",
          argc, envc, (unsigned long long)process->pid);

    return OR_OK;
}

/**
 * Get process count
 *
 * @return Number of active processes
 */
uint32_t process_get_count(void)
{
    uint32_t count = 0;

    spinlock_lock(&g_process_table_lock);
    for (uint32_t i = 0; i < MAX_PROCESSES; i++)
    {
        if (g_processes[i])
            count++;
    }
    spinlock_unlock(&g_process_table_lock);

    return count;
}

/**
 * Initialize process management system
 */
void process_init(void)
{
    kinfo("Initializing process management system");

    // Initialize process table
    memset(g_processes, 0, sizeof(g_processes));

    // Initialize locks
    spinlock_init(&g_process_table_lock);

    // Reset PID counter
    atomic_store(&g_next_pid, 1);

    kinfo("Process management system initialized (max processes: %d)", MAX_PROCESSES);
}
