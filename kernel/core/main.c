/*
 * Orion Operating System - Kernel Main Entry Point
 *
 * This is the main entry point for the Orion kernel, called after the
 * architecture-specific boot code has set up the basic runtime environment.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/kernel.h>
#include "orion-boot-protocol.h"
#include <orion/security.h>
#include <orion/mm.h>
#include <orion/types.h>
#include <orion/constants.h>
#include <orion/process.h>
#include <orion/thread.h>
#include <orion/servers.h>
#include <orion/klog.h>

// Kernel banner
static const char *orion_banner =
    "\n"
    "Orion Operating System v" ORION_VERSION_STR "\n"
    "Copyright (c) 2025 Orion OS Project\n"
    "Academic Research Operating System\n"
    "========================================\n";

// Forward declarations
static int kernel_early_init(void);
static int kernel_subsystem_init(void);
static void kernel_start_userspace(void);
static void kernel_idle_loop(void);

/**
 * Main kernel entry point
 *
 * This function is called by the architecture-specific boot code after
 * basic CPU and memory setup is complete. It receives boot information
 * from the Orion bootloader via the Orion Boot Protocol.
 *
 * @param boot_info Pointer to boot information from bootloader
 */
void kernel_main(struct orion_boot_info *boot_info)
{
    int result;

    // Display kernel banner
    kprintf("%s", orion_banner);
    klog_info(KLOG_CAT_KERNEL, "Starting Orion OS...");

    // Initialize boot information processing
    result = orion_boot_init(boot_info);
    if (result != 0)
    {
        kprintf("PANIC: Failed to initialize boot information: %d\n", result);
        kernel_panic("Boot initialization failed");
    }

    // Debug print boot information
    orion_boot_debug_print(boot_info);

    // Early kernel initialization
    klog_info(KLOG_CAT_KERNEL, "Early initialization...");
    kernel_early_init();

    // Initialize kernel subsystems
    klog_info(KLOG_CAT_KERNEL, "Subsystem initialization...");
    kernel_subsystem_init();

    klog_info(KLOG_CAT_KERNEL, "Orion OS initialization complete");
    klog_info(KLOG_CAT_KERNEL, "Starting user space...");

    // Start user space
    kernel_start_userspace();

    // Enter idle loop (should never return)
    kernel_idle_loop();
}

/**
 * Early kernel initialization
 *
 * Initialize critical kernel components that are needed before
 * other subsystems can be started.
 */
static int kernel_early_init(void)
{
    int result;

    // Initialize kernel logging system
    klog_info(KLOG_CAT_KERNEL, "Initializing kernel logging system...");
    klog_init();

    // Initialize memory management
    klog_info(KLOG_CAT_KERNEL, "Initializing memory management...");
    mm_init();

    // Initialize interrupt handling
    klog_info(KLOG_CAT_KERNEL, "Initializing interrupt handling...");
    arch_interrupt_init();

    // Initialize timer subsystem
    klog_info(KLOG_CAT_KERNEL, "Initializing timer subsystem...");
    arch_timer_init();

    klog_info(KLOG_CAT_KERNEL, "Early initialization complete");
    return 0;
}

/**
 * Kernel subsystem initialization
 *
 * Initialize the remaining kernel subsystems in the correct order.
 */
static int kernel_subsystem_init(void)
{
    int result;

    // Initialize process scheduler
    klog_info(KLOG_CAT_KERNEL, "Initializing process scheduler...");
    scheduler_init();

    // Initialize IPC subsystem
    klog_info(KLOG_CAT_KERNEL, "Initializing IPC subsystem...");
    ipc_init();

    // Initialize security subsystem (capabilities, hardening, etc.)
    klog_info(KLOG_CAT_KERNEL, "Initializing security subsystem...");
    capabilities_init(); // Initialize capability system
    security_init();     // Initialize hardware security features
    klog_info(KLOG_CAT_SECURITY, "Security subsystem initialized successfully");

    // Initialize system call interface
    klog_info(KLOG_CAT_KERNEL, "Initializing system call interface...");
    syscalls_init();

    klog_info(KLOG_CAT_KERNEL, "Subsystem initialization complete");
    return 0;
}

/**
 * Start user space
 *
 * Launch the initial user space processes and system servers.
 */
static void kernel_start_userspace(void)
{
    klog_info(KLOG_CAT_PROCESS, "Starting init process...");

    // Create and start init process
    process_t *init_proc = process_create("init", 0, 0);
    if (!init_proc)
    {
        klog_err(KLOG_CAT_PROCESS, "Failed to create init process");
        return;
    }

    // Set init process as system process
    init_proc->state = PROCESS_STATE_READY;
    // Note: priority field not available in process struct

    // Create main thread for init process
    thread_t *init_thread = thread_create(init_proc, "init_main", init_process_entry, NULL);
    if (!init_thread)
    {
        klog_err(KLOG_CAT_PROCESS, "Failed to create init process thread");
        process_destroy(init_proc);
        return;
    }

    // Start the init process
    if (process_start(init_proc) != OR_OK)
    {
        klog_err(KLOG_CAT_PROCESS, "Failed to start init process");
        thread_destroy(init_thread);
        process_destroy(init_proc);
        return;
    }

    klog_info(KLOG_CAT_PROCESS, "Init process started successfully (PID: %llu)", (unsigned long long)init_proc->pid);

    klog_info(KLOG_CAT_KERNEL, "Starting system servers...");

    // Start core system servers
    if (start_system_servers() != OR_OK)
    {
        klog_warning(KLOG_CAT_KERNEL, "Some system servers failed to start");
    }

    klog_info(KLOG_CAT_KERNEL, "User space startup complete");
}

/**
 * Kernel idle loop
 *
 * This function runs when the kernel has nothing else to do.
 * It should never return.
 */
static void kernel_idle_loop(void)
{
    kprintf("Kernel: Entering idle loop\n");
    kprintf("System ready - waiting for user input...\n");

    // Simple shell-like interface for now
    kprintf("\nOrion Shell v1.0 - Academic Demo\n");
    kprintf("Type 'help' for available commands\n");
    kprintf("orion$ ");

    while (1)
    {
        // Wait for interrupts
        arch_halt();

        // Handle any pending work
        sched_yield();
    }
}

/**
 * Kernel panic handler
 *
 * This function is called when an unrecoverable error occurs.
 * It prints diagnostic information and halts the system.
 *
 * @param message Panic message
 */
void kernel_panic(const char *message)
{
    kprintf("\n*** KERNEL PANIC ***\n");
    kprintf("Message: %s\n", message);
    kprintf("System halted.\n");

    // Disable interrupts and halt
    arch_disable_interrupts();
    while (1)
    {
        arch_halt();
    }
}

/**
 * Start core system servers
 *
 * Launches essential system servers for file system, network, and device management.
 *
 * @return 0 on success, negative error code on failure
 */
static int start_system_servers(void)
{
    int result = OR_OK;

    // Start file system server
    kprintf("    - Starting file system server...\n");
    if (start_fs_server() != OR_OK)
    {
        kerror("Failed to start file system server");
        result = -1; // Generic error code
    }
    else
    {
        kinfo("File system server started");
    }

    // Start network server
    kprintf("    - Starting network server...\n");
    if (start_network_server() != OR_OK)
    {
        kerror("Failed to start network server");
        result = -1; // Generic error code
    }
    else
    {
        kinfo("Network server started");
    }

    // Start device manager
    kprintf("    - Starting device manager...\n");
    if (start_device_manager() != OR_OK)
    {
        kerror("Failed to start device manager");
        result = -1; // Generic error code
    }
    else
    {
        kinfo("Device manager started");
    }

    // Start IPC server
    kprintf("    - Starting IPC server...\n");
    if (start_ipc_server() != OR_OK)
    {
        kerror("Failed to start IPC server");
        result = -1; // Generic error code
    }
    else
    {
        kinfo("IPC server started");
    }

    return result;
}
