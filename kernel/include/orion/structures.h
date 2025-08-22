/*
 * Orion Operating System - Centralized Structures
 *
 * This header contains all major system structures to avoid duplication
 * and ensure consistency across the kernel.
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_STRUCTURES_H
#define ORION_STRUCTURES_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <orion/types.h>
#include <orion/forward_decls.h>

    // ========================================
    // PROCESS STRUCTURES
    // ========================================

    /**
     * Process execution states
     */
    typedef enum
    {
        PROCESS_STATE_NEW = 0,    // Process created but not yet initialized
        PROCESS_STATE_READY,      // Ready to run
        PROCESS_STATE_RUNNING,    // Currently executing
        PROCESS_STATE_BLOCKED,    // Waiting for I/O or event
        PROCESS_STATE_SLEEPING,   // Sleeping for specified time
        PROCESS_STATE_WAITING,    // Waiting for child process
        PROCESS_STATE_ZOMBIE,     // Process terminated, waiting for cleanup
        PROCESS_STATE_TERMINATED, // Process fully cleaned up
        PROCESS_STATE_STOPPED,    // Process stopped by signal
        PROCESS_STATE_SUSPENDED   // Process suspended
    } process_execution_state_t;

    /**
     * Main process structure
     */
    struct process
    {
        // Basic identification
        uint64_t pid;  // Process ID
        char name[64]; // Process name

        // State and execution
        process_execution_state_t state; // Current execution state
        int exit_code;                   // Exit code when terminated
        uint64_t creation_time;          // Creation timestamp
        uint64_t last_run_time;          // Last execution timestamp

        // Memory management
        vm_space_t *vm_space; // Virtual memory space
        uint64_t heap_start;  // Heap start address
        uint64_t heap_size;   // Heap size
        uint64_t stack_base;  // Stack base address
        uint64_t stack_size;  // Stack size
        uint64_t stack_top;   // Stack top address
        uint64_t code_base;   // Code segment base
        uint64_t code_size;   // Code segment size
        uint64_t data_base;   // Data segment base
        uint64_t data_size;   // Data segment size
        uint64_t entry_point; // Entry point address

        // Thread management
        thread_t *main_thread; // Main thread
        thread_t *threads;     // Thread list
        uint32_t thread_count; // Number of threads

        // Process hierarchy
        process_t *parent;       // Parent process
        process_t *children;     // Children processes
        process_t *next_sibling; // Next sibling process

        // Resource management
        handle_t handles[MAX_HANDLES]; // Handle table
        uint32_t handle_count;         // Number of open handles

        // Security and capabilities
        uint64_t capabilities;        // Capability bitmap
        security_context_t *security; // Security context

        // IPC and communication
        ipc_port_t *ipc_ports;   // IPC ports
        uint32_t ipc_port_count; // Number of IPC ports

        // File system
        vfs_mount_t *mounts; // Mount points
        vfs_inode_t *cwd;    // Current working directory

        // Signals
        uint64_t pending_signals;     // Pending signal bitmap
        uint64_t signal_handlers[32]; // Signal handler addresses

        // Statistics
        uint64_t cpu_time;         // Total CPU time used
        uint64_t memory_usage;     // Memory usage in bytes
        uint32_t context_switches; // Number of context switches

        // Next process in global list
        process_t *next_sibling; // Next process in global list
    };

    // ========================================
    // THREAD STRUCTURES
    // ========================================

    /**
     * Thread execution states
     */
    typedef enum
    {
        THREAD_STATE_NEW = 0,    // Thread created but not yet initialized
        THREAD_STATE_READY,      // Ready to run
        THREAD_STATE_RUNNING,    // Currently executing
        THREAD_STATE_BLOCKED,    // Waiting for I/O or event
        THREAD_STATE_SLEEPING,   // Sleeping for specified time
        THREAD_STATE_TERMINATED, // Thread terminated
        THREAD_STATE_SUSPENDED   // Thread suspended
    } thread_execution_state_t;

    /**
     * Main thread structure
     */
    struct thread
    {
        // Basic identification
        uint64_t tid;  // Thread ID
        char name[32]; // Thread name

        // State and execution
        thread_execution_state_t state; // Current execution state
        uint64_t creation_time;         // Creation timestamp
        uint64_t last_switch_time;      // Last context switch time
        uint64_t sleep_until;           // Sleep until timestamp

        // Process association
        process_t *parent_process; // Owning process

        // Scheduling
        int priority;             // Priority (-20 to +19)
        uint64_t nice_weight;     // Nice weight for CFS
        uint64_t virtual_runtime; // Virtual runtime for CFS
        uint64_t actual_runtime;  // Actual runtime
        uint64_t cpu_affinity;    // CPU affinity mask

        // Memory and stack
        uint64_t stack_base; // Kernel stack base
        uint64_t stack_size; // Kernel stack size
        uint64_t user_stack; // User stack pointer

        // Register context
        uint64_t rsp;           // Stack pointer
        uint64_t rip;           // Instruction pointer
        uint64_t rflags;        // Flags register
        uint64_t registers[16]; // General purpose registers

        // Red-black tree for scheduler
        thread_t *rb_left;   // Left child
        thread_t *rb_right;  // Right child
        thread_t *rb_parent; // Parent node
        int rb_color;        // Node color (0=red, 1=black)

        // Queue management
        thread_t *next; // Next thread in queue
        thread_t *prev; // Previous thread in queue

        // Statistics
        uint64_t cpu_time;         // CPU time used
        uint32_t context_switches; // Number of context switches
    };

    // ========================================
    // VIRTUAL MEMORY STRUCTURES
    // ========================================

    /**
     * Virtual memory space structure
     */
    struct vm_space
    {
        uint64_t *pml4;      // Page map level 4
        uint64_t pml4_phys;  // Physical address of PML4
        bool is_kernel;      // True if kernel space
        uint64_t start_addr; // Start address
        uint64_t end_addr;   // End address
        uint64_t ref_count;  // Reference count
        spinlock_t lock;     // Protection lock
    };

    // ========================================
    // SCHEDULER STRUCTURES
    // ========================================

    /**
     * CPU runqueue structure
     */
    struct cpu_runqueue
    {
        spinlock_t lock;       // Protection lock
        thread_t *current;     // Currently running thread
        thread_t *rb_root;     // Red-black tree root
        uint32_t nr_running;   // Number of runnable threads
        uint64_t min_vruntime; // Minimum virtual runtime
        uint64_t load_weight;  // Load weight
        uint64_t last_update;  // Last update timestamp
    };

    // ========================================
    // HANDLE STRUCTURES
    // ========================================

    /**
     * Handle types
     */
    typedef enum
    {
        HANDLE_TYPE_NONE = 0, // No handle
        HANDLE_TYPE_PROCESS,  // Process handle
        HANDLE_TYPE_THREAD,   // Thread handle
        HANDLE_TYPE_IPC_PORT, // IPC port handle
        HANDLE_TYPE_MEMORY,   // Memory region handle
        HANDLE_TYPE_FILE,     // File handle
        HANDLE_TYPE_TIMER,    // Timer handle
        HANDLE_TYPE_DEVICE    // Device handle
    } handle_type_t;

    /**
     * Handle structure
     */
    struct handle
    {
        handle_type_t type;     // Handle type
        uint64_t object_id;     // Object identifier
        uint32_t ref_count;     // Reference count
        uint64_t rights;        // Access rights
        uint64_t creation_time; // Creation timestamp
    };

#ifdef __cplusplus
}
#endif

#endif // ORION_STRUCTURES_H
