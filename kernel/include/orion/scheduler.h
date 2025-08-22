/*
 * Orion Operating System - Scheduler Header
 *
 * Complete Fair Scheduler (CFS) definitions and interface.
 * This header provides the public API for process and thread management.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_SCHEDULER_H
#define ORION_SCHEDULER_H

#include <orion/types.h>
#include <orion/mm.h>
#include <orion/forward_decls.h>
#include <orion/spinlock.h>

// ========================================
// FORWARD DECLARATIONS
// ========================================

// ========================================
// ENUMS AND CONSTANTS
// ========================================

// Process and thread states are defined in process.h and thread.h respectively

// Handle types
typedef enum
{
    HANDLE_TYPE_NONE = 0,
    HANDLE_TYPE_PROCESS,
    HANDLE_TYPE_THREAD,
    HANDLE_TYPE_IPC_PORT,
    HANDLE_TYPE_MEMORY,
    HANDLE_TYPE_TIMER
} handle_type_t;

// ========================================
// STRUCTURE DEFINITIONS
// ========================================

// Handle structure
typedef struct
{
    handle_type_t type;
    uint64_t object_id;
    uint32_t permissions;
    uint32_t ref_count;
} handle_t;

// Per-CPU runqueue for SMP
typedef struct cpu_runqueue
{
    thread_t *rb_root;     // Red-black tree root
    thread_t *current;     // Currently running thread
    uint64_t min_vruntime; // Minimum virtual runtime
    uint32_t nr_running;   // Number of ready threads
    spinlock_t lock;       // Lock for concurrent access
    uint64_t load_weight;  // Total load weight
    uint64_t last_update;  // Last update time
} cpu_runqueue_t;

// ========================================
// SCHEDULER API
// ========================================

// Initialization
void scheduler_init(void);

// Process management
process_t *scheduler_create_process(void);
void scheduler_destroy_process(process_t *process);
void scheduler_add_process(process_t *process);
process_t *scheduler_find_process(uint64_t pid);
process_t *scheduler_get_current_process(void);

// Thread management
thread_t *scheduler_create_thread(process_t *process, uint64_t entry_point, uint64_t stack_pointer, uint64_t arg);
thread_t *scheduler_get_current_thread(void);

// Scheduling control
void sched_yield(void);
void scheduler_block_current_process(void);
void scheduler_wakeup_process(process_t *process);
void scheduler_sleep_ns(uint64_t nanoseconds);

// Statistics
uint64_t scheduler_get_process_count(void);
uint64_t scheduler_get_thread_count(void);

// Utilities
void handle_cleanup(handle_t *handle);
int elf_load_process(process_t *process, const char *path);
int process_setup_args(process_t *process, char *const argv[], char *const envp[]);
int signal_send(process_t *target, uint32_t signal);

// Compatibility
void thread_exit(int exit_code);

// ========================================
// ARCHITECTURE-SPECIFIC FUNCTIONS
// ========================================

// These functions must be implemented by the architecture
uint32_t arch_get_current_cpu(void);
uint32_t arch_get_cpu_count(void);
uint64_t arch_get_timestamp(void);
uint64_t arch_get_boot_time(void);
void arch_context_switch(thread_t *prev, thread_t *next);
void arch_cpu_idle(void);
void arch_setup_syscall_interface(void);

#endif // ORION_SCHEDULER_H
