/*
 * Orion Operating System - Scheduler Header
 *
 * Complete Fair Scheduler (CFS) definitions and interface.
 * This header provides the public API for process and thread management.
 *
 * Developed by Jérémy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_SCHEDULER_H
#define ORION_SCHEDULER_H

#include <orion/types.h>
#include <orion/mm.h>

// ========================================
// FORWARD DECLARATIONS
// ========================================

typedef struct vm_space vm_space_t;
typedef struct handle handle_t;

// ========================================
// ENUMS AND CONSTANTS
// ========================================

// États des processus
typedef enum
{
    PROC_STATE_READY,
    PROC_STATE_RUNNING,
    PROC_STATE_BLOCKED,
    PROC_STATE_ZOMBIE,
    PROC_STATE_TERMINATED
} process_state_t;

// États des threads
typedef enum
{
    THREAD_STATE_READY,
    THREAD_STATE_RUNNING,
    THREAD_STATE_BLOCKED,
    THREAD_STATE_ZOMBIE,
    THREAD_STATE_TERMINATED
} thread_state_t;

// Types de handles
typedef enum
{
    HANDLE_TYPE_NONE = 0,
    HANDLE_TYPE_PROCESS,
    HANDLE_TYPE_THREAD,
    HANDLE_TYPE_IPC_PORT,
    HANDLE_TYPE_MEMORY,
    HANDLE_TYPE_TIMER
} handle_type_t;

#define MAX_HANDLES 256

// ========================================
// STRUCTURE DEFINITIONS
// ========================================

// Structure handle
typedef struct
{
    handle_type_t type;
    uint64_t object_id;
    uint32_t permissions;
    uint32_t ref_count;
} handle_t;

// Maximum number of CPUs
#define MAX_CPUS 64

// Structure thread complète
typedef struct thread
{
    uint64_t tid;              // Thread ID
    thread_state_t state;      // Thread state
    uint64_t virtual_runtime;  // Virtual runtime for CFS
    uint64_t actual_runtime;   // Actual runtime
    uint64_t last_switch_time; // Last context switch time
    int priority;              // Priority (-20 to +19)
    uint64_t nice_weight;      // Weight for CFS

    // CPU context (saved during context switches)
    uint64_t rsp;           // Stack pointer
    uint64_t rip;           // Instruction pointer
    uint64_t rflags;        // Flags register
    uint64_t rbp;           // Base pointer
    uint64_t registers[16]; // General purpose registers

    // Stack and memory
    uint64_t stack_base; // Stack base address
    uint64_t stack_size; // Stack size
    uint64_t user_stack; // Userland stack

    // Scheduler links
    struct thread *next;      // Queue list
    struct thread *prev;      // Queue list
    struct thread *rb_left;   // Red-black tree
    struct thread *rb_right;  // Red-black tree
    struct thread *rb_parent; // Red-black tree
    int rb_color;             // Red-black tree color

    // Parent process
    struct process *parent_process; // Owning process

    // Timing and sleep
    uint64_t sleep_until;  // Wake up time if sleeping
    uint64_t cpu_affinity; // CPU affinity (bitmap)
} thread_t;

// Structure processus complète
typedef struct process
{
    uint64_t pid;          // Process ID
    process_state_t state; // Process state
    int exit_code;         // Exit code

    // Memory management
    vm_space_t *vm_space; // Virtual address space

    // Threads
    thread_t *main_thread; // Main thread
    thread_t *threads;     // Thread list
    uint32_t thread_count; // Number of threads

    // Hierarchy
    struct process *parent;       // Parent process
    struct process *children;     // Children
    struct process *next_sibling; // Next sibling

    // Handles and resources
    handle_t handles[MAX_HANDLES]; // Handle table

    // ELF information
    uint64_t entry_point; // Entry point
    uint64_t brk;         // End of heap

    // Memory layout
    uint64_t code_base;  // Code segment base
    uint64_t code_size;  // Code segment size
    uint64_t data_base;  // Data segment base
    uint64_t data_size;  // Data segment size
    uint64_t stack_base; // Stack base
    uint64_t stack_size; // Stack size
    uint64_t heap_start; // Heap start
    uint64_t stack_top;  // Stack top

    // Process arguments
    char **argv; // Command line arguments
    char **envp; // Environment variables

    // Signal handling
    uint64_t pending_signals; // Pending signals bitmap

    // Timing
    uint64_t creation_time;  // Creation time
    uint64_t cpu_time_total; // Total CPU time
} process_t;

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

// Initialisation
void scheduler_init(void);

// Gestion des processus
process_t *scheduler_create_process(void);
void scheduler_destroy_process(process_t *process);
void scheduler_add_process(process_t *process);
process_t *scheduler_find_process(uint64_t pid);
process_t *scheduler_get_current_process(void);

// Gestion des threads
thread_t *scheduler_create_thread(process_t *process, uint64_t entry_point, uint64_t stack_pointer, uint64_t arg);
thread_t *scheduler_get_current_thread(void);

// Contrôle du scheduling
void sched_yield(void);
void scheduler_block_current_process(void);
void scheduler_wakeup_process(process_t *process);
void scheduler_sleep_ns(uint64_t nanoseconds);

// Statistiques
uint64_t scheduler_get_process_count(void);
uint64_t scheduler_get_thread_count(void);

// Utilitaires
void handle_cleanup(handle_t *handle);
int elf_load_process(process_t *process, const char *path);
int process_setup_args(process_t *process, char *const argv[], char *const envp[]);
int signal_send(process_t *target, uint32_t signal);

// Compatibilité
void thread_exit(int exit_code);

// ========================================
// ARCHITECTURE-SPECIFIC FUNCTIONS
// ========================================

// Ces fonctions doivent être implémentées par l'architecture
uint32_t arch_get_current_cpu(void);
uint32_t arch_get_cpu_count(void);
uint64_t arch_get_timestamp(void);
uint64_t arch_get_boot_time(void);
void arch_context_switch(thread_t *prev, thread_t *next);
void arch_cpu_idle(void);
void arch_setup_syscall_interface(void);

#endif // ORION_SCHEDULER_H
