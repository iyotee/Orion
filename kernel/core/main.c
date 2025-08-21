/*
 * Orion Operating System - Kernel Main Entry Point
 * 
 * This is the main entry point for the Orion kernel, called after the
 * architecture-specific boot code has set up the basic runtime environment.
 * 
 * Developed by Jérémy Noverraz (1988-2025)
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
    kprintf("Kernel: Starting Orion OS...\n");

    // Initialize boot information processing
    result = orion_boot_init(boot_info);
    if (result != 0) {
        kprintf("PANIC: Failed to initialize boot information: %d\n", result);
        kernel_panic("Boot initialization failed");
    }

    // Debug print boot information
    orion_boot_debug_print(boot_info);

    // Early kernel initialization
    kprintf("Kernel: Early initialization...\n");
    kernel_early_init();

    // Initialize kernel subsystems
    kprintf("Kernel: Subsystem initialization...\n");
    kernel_subsystem_init();

    kprintf("Kernel: Orion OS initialization complete\n");
    kprintf("Kernel: Starting user space...\n");

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

    // Initialize memory management
    kprintf("  - Initializing memory management...\n");
    mm_init();

    // Initialize interrupt handling
    kprintf("  - Initializing interrupt handling...\n");
    arch_interrupt_init();

    // Initialize timer subsystem
    kprintf("  - Initializing timer subsystem...\n");
    arch_timer_init();

    kprintf("  - Early initialization complete\n");
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
    kprintf("  - Initializing process scheduler...\n");
    scheduler_init();

    // Initialize IPC subsystem
    kprintf("  - Initializing IPC subsystem...\n");
    ipc_init();

    // Initialize security subsystem (capabilities, hardening, etc.)
    kprintf("  - Initializing security subsystem...\n");
    capabilities_init();  // Initialize capability system
    security_init();      // Initialize hardware security features
    kprintf("    Security subsystem initialized successfully\n");

    // Initialize system call interface
    kprintf("  - Initializing system call interface...\n");
    syscalls_init();

    kprintf("  - Subsystem initialization complete\n");
    return 0;
}

/**
 * Start user space
 * 
 * Launch the initial user space processes and system servers.
 */
static void kernel_start_userspace(void)
{
    kprintf("  - Starting init process...\n");
    
    // TODO: Create and start init process
    // This will eventually load the user space init program
    
    kprintf("  - Starting system servers...\n");
    
    // TODO: Start core system servers
    // - File system server
    // - Network server  
    // - Device manager
    
    kprintf("  - User space startup complete\n");
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
    
    while (1) {
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
    while (1) {
        arch_halt();
    }
}
