/*
 * ORION OS - MIPS System Management
 *
 * System management for MIPS architecture
 * Handles system calls, process management, memory management, and utilities
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// SYSTEM CONTEXT MANAGEMENT
// ============================================================================

static mips_system_context_t mips_system_context = {0};

void mips_system_init(void)
{
    memset(&mips_system_context, 0, sizeof(mips_system_context));

    // Initialize system context
    mips_system_context.system_initialized = false;
    mips_system_context.process_count = 0;
    mips_system_context.memory_allocated = 0;
    mips_system_context.system_calls_enabled = true;
    mips_system_context.process_switching_enabled = true;

    // Initialize system call table
    for (int i = 0; i < MIPS_MAX_SYSCALLS; i++)
    {
        mips_system_context.syscall_table[i] = NULL;
    }

    printf("MIPS: System management initialized\n");
}

mips_system_context_t *mips_system_get_context(void)
{
    return &mips_system_context;
}

// ============================================================================
// SYSTEM CALL MANAGEMENT
// ============================================================================

void mips_system_register_syscall(uint32_t number, mips_syscall_handler_t handler)
{
    if (number >= MIPS_MAX_SYSCALLS)
    {
        printf("MIPS: Invalid syscall number %u\n", number);
        return;
    }

    mips_system_context.syscall_table[number] = handler;
    printf("MIPS: System call %u registered\n", number);
}

mips_syscall_handler_t mips_system_get_syscall(uint32_t number)
{
    if (number >= MIPS_MAX_SYSCALLS)
    {
        return NULL;
    }

    return mips_system_context.syscall_table[number];
}

void mips_system_dispatch_syscall(uint32_t number, uint64_t *args)
{
    if (!mips_system_context.system_calls_enabled)
    {
        printf("MIPS: System calls disabled\n");
        return;
    }

    if (number >= MIPS_MAX_SYSCALLS)
    {
        printf("MIPS: Invalid syscall number %u\n", number);
        return;
    }

    mips_syscall_handler_t handler = mips_system_context.syscall_table[number];
    if (!handler)
    {
        printf("MIPS: No handler for syscall %u\n", number);
        return;
    }

    // Execute system call
    handler(args);
}

void mips_system_enable_syscalls(void)
{
    mips_system_context.system_calls_enabled = true;
    printf("MIPS: System calls enabled\n");
}

void mips_system_disable_syscalls(void)
{
    mips_system_context.system_calls_enabled = false;
    printf("MIPS: System calls disabled\n");
}

// ============================================================================
// PROCESS MANAGEMENT
// ============================================================================

mips_process_id_t mips_system_create_process(const char *name, mips_addr_t entry_point)
{
    if (mips_system_context.process_count >= MIPS_MAX_PROCESSES)
    {
        printf("MIPS: Maximum process count reached\n");
        return MIPS_INVALID_PROCESS_ID;
    }

    // Find free process slot
    mips_process_id_t pid = MIPS_INVALID_PROCESS_ID;
    for (int i = 0; i < MIPS_MAX_PROCESSES; i++)
    {
        if (!mips_system_context.processes[i].active)
        {
            pid = i;
            break;
        }
    }

    if (pid == MIPS_INVALID_PROCESS_ID)
    {
        printf("MIPS: No free process slots\n");
        return MIPS_INVALID_PROCESS_ID;
    }

    // Initialize process
    mips_process_t *process = &mips_system_context.processes[pid];
    memset(process, 0, sizeof(mips_process_t));

    strncpy(process->name, name, MIPS_MAX_PROCESS_NAME - 1);
    process->name[MIPS_MAX_PROCESS_NAME - 1] = '\0';
    process->pid = pid;
    process->entry_point = entry_point;
    process->active = true;
    process->state = MIPS_PROCESS_STATE_READY;
    process->priority = MIPS_PROCESS_PRIORITY_NORMAL;
    process->created_time = mips_timer_get_system_time();

    mips_system_context.process_count++;

    printf("MIPS: Process '%s' created with PID %u\n", name, pid);
    return pid;
}

void mips_system_destroy_process(mips_process_id_t pid)
{
    if (pid >= MIPS_MAX_PROCESSES)
    {
        printf("MIPS: Invalid process ID %u\n", pid);
        return;
    }

    mips_process_t *process = &mips_system_context.processes[pid];
    if (!process->active)
    {
        printf("MIPS: Process %u not active\n", pid);
        return;
    }

    // Clean up process resources
    if (process->memory_regions)
    {
        free(process->memory_regions);
        process->memory_regions = NULL;
    }

    // Mark process as inactive
    process->active = false;
    process->state = MIPS_PROCESS_STATE_TERMINATED;

    mips_system_context.process_count--;

    printf("MIPS: Process %u destroyed\n", pid);
}

mips_process_t *mips_system_get_process(mips_process_id_t pid)
{
    if (pid >= MIPS_MAX_PROCESSES)
    {
        return NULL;
    }

    mips_process_t *process = &mips_system_context.processes[pid];
    return process->active ? process : NULL;
}

void mips_system_set_process_state(mips_process_id_t pid, mips_process_state_t state)
{
    mips_process_t *process = mips_system_get_process(pid);
    if (!process)
    {
        return;
    }

    process->state = state;
    printf("MIPS: Process %u state changed to %u\n", pid, state);
}

void mips_system_set_process_priority(mips_process_id_t pid, mips_process_priority_t priority)
{
    mips_process_t *process = mips_system_get_process(pid);
    if (!process)
    {
        return;
    }

    process->priority = priority;
    printf("MIPS: Process %u priority changed to %u\n", pid, priority);
}

mips_process_id_t mips_system_get_current_process(void)
{
    return mips_system_context.current_process;
}

void mips_system_set_current_process(mips_process_id_t pid)
{
    if (pid >= MIPS_MAX_PROCESSES)
    {
        printf("MIPS: Invalid process ID %u\n", pid);
        return;
    }

    mips_system_context.current_process = pid;
    printf("MIPS: Current process set to %u\n", pid);
}

// ============================================================================
// MEMORY MANAGEMENT
// ============================================================================

mips_addr_t mips_system_allocate_memory(size_t size, mips_memory_flags_t flags)
{
    // Simple memory allocation
    mips_addr_t address = mips_memory_allocate_region(size, flags);

    if (address != MIPS_INVALID_ADDRESS)
    {
        mips_system_context.memory_allocated += size;
        printf("MIPS: Allocated %zu bytes at 0x%llx\n", size, address);
    }

    return address;
}

void mips_system_free_memory(mips_addr_t address)
{
    size_t size = mips_memory_get_region_size(address);

    if (size > 0)
    {
        mips_memory_free_region(address);
        mips_system_context.memory_allocated -= size;
        printf("MIPS: Freed %zu bytes at 0x%llx\n", size, address);
    }
}

mips_addr_t mips_system_map_memory(mips_addr_t physical, size_t size, mips_memory_flags_t flags)
{
    mips_addr_t virtual_addr = mips_memory_map_region(physical, size, flags);

    if (virtual_addr != MIPS_INVALID_ADDRESS)
    {
        printf("MIPS: Mapped physical 0x%llx to virtual 0x%llx (%zu bytes)\n",
               physical, virtual_addr, size);
    }

    return virtual_addr;
}

void mips_system_unmap_memory(mips_addr_t virtual_addr)
{
    size_t size = mips_memory_get_region_size(virtual_addr);

    if (size > 0)
    {
        mips_memory_unmap_region(virtual_addr);
        printf("MIPS: Unmapped virtual address 0x%llx (%zu bytes)\n", virtual_addr, size);
    }
}

mips_addr_t mips_system_virt_to_phys(mips_addr_t virtual_addr)
{
    return mips_memory_virt_to_phys(virtual_addr);
}

mips_addr_t mips_system_phys_to_virt(mips_addr_t physical_addr)
{
    return mips_memory_phys_to_virt(physical_addr);
}

// ============================================================================
// CONTEXT SWITCHING
// ============================================================================

void mips_system_switch_context(mips_process_id_t from_pid, mips_process_id_t to_pid)
{
    if (!mips_system_context.process_switching_enabled)
    {
        printf("MIPS: Process switching disabled\n");
        return;
    }

    printf("MIPS: Switching context from process %u to %u\n", from_pid, to_pid);

    // Save current process context
    if (from_pid != MIPS_INVALID_PROCESS_ID)
    {
        mips_process_t *from_process = mips_system_get_process(from_pid);
        if (from_process)
        {
            mips_cpu_save_context(&from_process->context);
            from_process->state = MIPS_PROCESS_STATE_READY;
        }
    }

    // Restore target process context
    if (to_pid != MIPS_INVALID_PROCESS_ID)
    {
        mips_process_t *to_process = mips_system_get_process(to_pid);
        if (to_process)
        {
            mips_cpu_restore_context(&to_process->context);
            to_process->state = MIPS_PROCESS_STATE_RUNNING;
            mips_system_context.current_process = to_pid;
        }
    }

    printf("MIPS: Context switch completed\n");
}

void mips_system_enable_context_switching(void)
{
    mips_system_context.process_switching_enabled = true;
    printf("MIPS: Context switching enabled\n");
}

void mips_system_disable_context_switching(void)
{
    mips_system_context.process_switching_enabled = false;
    printf("MIPS: Context switching disabled\n");
}

// ============================================================================
// SYSTEM UTILITIES
// ============================================================================

void mips_system_print_status(void)
{
    printf("MIPS: System Status:\n");
    printf("  System Initialized: %s\n", mips_system_context.system_initialized ? "yes" : "no");
    printf("  Process Count: %u\n", mips_system_context.process_count);
    printf("  Current Process: %u\n", mips_system_context.current_process);
    printf("  Memory Allocated: %zu bytes\n", mips_system_context.memory_allocated);
    printf("  System Calls: %s\n", mips_system_context.system_calls_enabled ? "enabled" : "disabled");
    printf("  Context Switching: %s\n", mips_system_context.process_switching_enabled ? "enabled" : "disabled");

    printf("  Active Processes:\n");
    for (int i = 0; i < MIPS_MAX_PROCESSES; i++)
    {
        mips_process_t *process = &mips_system_context.processes[i];
        if (process->active)
        {
            printf("    PID %u: %s (state: %u, priority: %u)\n",
                   process->pid, process->name, process->state, process->priority);
        }
    }
}

void mips_system_print_process_info(mips_process_id_t pid)
{
    mips_process_t *process = mips_system_get_process(pid);
    if (!process)
    {
        printf("MIPS: Process %u not found\n", pid);
        return;
    }

    printf("MIPS: Process %u Information:\n", pid);
    printf("  Name: %s\n", process->name);
    printf("  Entry Point: 0x%llx\n", process->entry_point);
    printf("  State: %u\n", process->state);
    printf("  Priority: %u\n", process->priority);
    printf("  Created: %llu\n", process->created_time);
    printf("  Active: %s\n", process->active ? "yes" : "no");
}

void mips_system_print_memory_status(void)
{
    printf("MIPS: Memory Status:\n");
    printf("  Total Allocated: %zu bytes\n", mips_system_context.memory_allocated);

    // Print memory regions
    mips_memory_print_regions();
}

void mips_system_cleanup(void)
{
    printf("MIPS: Cleaning up system\n");

    // Destroy all processes
    for (int i = 0; i < MIPS_MAX_PROCESSES; i++)
    {
        if (mips_system_context.processes[i].active)
        {
            mips_system_destroy_process(i);
        }
    }

    // Free allocated memory
    mips_system_context.memory_allocated = 0;

    // Clear system call table
    for (int i = 0; i < MIPS_MAX_SYSCALLS; i++)
    {
        mips_system_context.syscall_table[i] = NULL;
    }

    mips_system_context.system_initialized = false;
    mips_system_context.process_count = 0;
    mips_system_context.current_process = MIPS_INVALID_PROCESS_ID;

    printf("MIPS: System cleanup completed\n");
}

// ============================================================================
// SYSTEM CALL IMPLEMENTATIONS
// ============================================================================

void mips_system_syscall_exit(uint64_t *args)
{
    uint32_t exit_code = (uint32_t)args[0];
    mips_process_id_t current_pid = mips_system_get_current_process();

    printf("MIPS: Process %u exiting with code %u\n", current_pid, exit_code);
    mips_system_destroy_process(current_pid);
}

void mips_system_syscall_fork(uint64_t *args)
{
    mips_process_id_t parent_pid = mips_system_get_current_process();
    mips_process_t *parent = mips_system_get_process(parent_pid);

    if (!parent)
    {
        printf("MIPS: Cannot fork - parent process not found\n");
        return;
    }

    // Create child process
    mips_process_id_t child_pid = mips_system_create_process(parent->name, parent->entry_point);

    if (child_pid != MIPS_INVALID_PROCESS_ID)
    {
        mips_process_t *child = mips_system_get_process(child_pid);

        // Copy parent context
        memcpy(&child->context, &parent->context, sizeof(mips_cpu_context_t));
        child->priority = parent->priority;

        printf("MIPS: Process %u forked to %u\n", parent_pid, child_pid);
    }
}

void mips_system_syscall_getpid(uint64_t *args)
{
    mips_process_id_t current_pid = mips_system_get_current_process();
    printf("MIPS: Current process ID: %u\n", current_pid);
}

void mips_system_syscall_sleep(uint64_t *args)
{
    uint32_t milliseconds = (uint32_t)args[0];
    printf("MIPS: Process sleeping for %u milliseconds\n", milliseconds);

    // Simple sleep implementation
    uint64_t start_time = mips_timer_get_system_time();
    while ((mips_timer_get_system_time() - start_time) < milliseconds)
    {
        // Wait
    }
}

void mips_system_syscall_yield(uint64_t *args)
{
    mips_process_id_t current_pid = mips_system_get_current_process();
    printf("MIPS: Process %u yielding\n", current_pid);

    // Set process to ready state
    mips_system_set_process_state(current_pid, MIPS_PROCESS_STATE_READY);
}

// ============================================================================
// SYSTEM INITIALIZATION
// ============================================================================

void mips_system_setup_default_syscalls(void)
{
    printf("MIPS: Setting up default system calls\n");

    // Register basic system calls
    mips_system_register_syscall(MIPS_SYSCALL_EXIT, mips_system_syscall_exit);
    mips_system_register_syscall(MIPS_SYSCALL_FORK, mips_system_syscall_fork);
    mips_system_register_syscall(MIPS_SYSCALL_GETPID, mips_system_syscall_getpid);
    mips_system_register_syscall(MIPS_SYSCALL_SLEEP, mips_system_syscall_sleep);
    mips_system_register_syscall(MIPS_SYSCALL_YIELD, mips_system_syscall_yield);

    printf("MIPS: Default system calls registered\n");
}

void mips_system_finalize_initialization(void)
{
    mips_system_context.system_initialized = true;
    printf("MIPS: System initialization completed\n");
}
