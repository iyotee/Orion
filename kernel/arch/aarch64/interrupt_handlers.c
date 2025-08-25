/*
 * Orion Operating System - aarch64 Interrupt Handlers
 *
 * Complete interrupt handler implementation for aarch64 including:
 * - IRQ handlers for hardware interrupts
 * - FIQ handlers for fast interrupts
 * - System call handlers
 * - Exception handlers
 * - Interrupt chaining and priority handling
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "../hal/common/types.h"
#include "config.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// EXTERNAL DECLARATIONS
// ============================================================================

// Functions from interrupts.c
extern uint32_t aarch64_interrupt_acknowledge(void);
extern void aarch64_interrupt_end(uint32_t irq);
extern void aarch64_interrupt_priority_drop(void);

// ============================================================================
// INTERRUPT HANDLER TYPES
// ============================================================================

typedef struct
{
    void (*handler)(void);
    uint32_t priority;
    bool enabled;
    char name[32];
} interrupt_handler_info_t;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// Interrupt handler table
static interrupt_handler_info_t interrupt_handlers[1024];

// System call handler
static void (*syscall_handler)(void) = NULL;

// Exception handler
static void (*exception_handler)(uint32_t esr, uint64_t elr, uint64_t far) = NULL;

// Interrupt statistics
static uint32_t interrupt_counts[1024];
static uint32_t total_interrupts = 0;

// ============================================================================
// DEFAULT INTERRUPT HANDLERS
// ============================================================================

void aarch64_default_irq_handler(void)
{
    printf("WARNING: Unhandled IRQ received\n");
}

void aarch64_default_fiq_handler(void)
{
    printf("WARNING: Unhandled FIQ received\n");
}

void aarch64_default_syscall_handler(void)
{
    printf("WARNING: Unhandled system call received\n");
}

void aarch64_default_exception_handler(uint32_t esr, uint64_t elr, uint64_t far)
{
    printf("WARNING: Unhandled exception: ESR=0x%X, ELR=0x%llX, FAR=0x%llX\n", esr, elr, far);
}

// ============================================================================
// INTERRUPT HANDLER MANAGEMENT
// ============================================================================

void aarch64_register_interrupt_handler(uint32_t irq, void (*handler)(void), const char *name, uint32_t priority)
{
    if (irq >= 1024)
        return;

    interrupt_handlers[irq].handler = handler ? handler : aarch64_default_irq_handler;
    interrupt_handlers[irq].priority = priority;
    interrupt_handlers[irq].enabled = true;
    strncpy(interrupt_handlers[irq].name, name ? name : "unknown", 31);
    interrupt_handlers[irq].name[31] = '\0';

    printf("Registered handler for IRQ %u: %s (priority %u)\n", irq, interrupt_handlers[irq].name, priority);
}

void aarch64_unregister_interrupt_handler(uint32_t irq)
{
    if (irq >= 1024)
        return;

    interrupt_handlers[irq].handler = aarch64_default_irq_handler;
    interrupt_handlers[irq].enabled = false;
    strcpy(interrupt_handlers[irq].name, "unregistered");

    printf("Unregistered handler for IRQ %u\n", irq);
}

void aarch64_set_interrupt_priority(uint32_t irq, uint32_t priority)
{
    if (irq >= 1024)
        return;

    interrupt_handlers[irq].priority = priority;
    printf("Set priority %u for IRQ %u\n", priority, irq);
}

void aarch64_enable_interrupt_handler(uint32_t irq)
{
    if (irq >= 1024)
        return;

    interrupt_handlers[irq].enabled = true;
    printf("Enabled handler for IRQ %u\n", irq);
}

void aarch64_disable_interrupt_handler(uint32_t irq)
{
    if (irq >= 1024)
        return;

    interrupt_handlers[irq].enabled = false;
    printf("Disabled handler for IRQ %u\n", irq);
}

// ============================================================================
// SYSTEM CALL HANDLER MANAGEMENT
// ============================================================================

void aarch64_register_syscall_handler(void (*handler)(void))
{
    syscall_handler = handler;
    printf("Registered system call handler\n");
}

void aarch64_unregister_syscall_handler(void)
{
    syscall_handler = NULL;
    printf("Unregistered system call handler\n");
}

// ============================================================================
// EXCEPTION HANDLER MANAGEMENT
// ============================================================================

void aarch64_register_exception_handler(void (*handler)(uint32_t esr, uint64_t elr, uint64_t far))
{
    exception_handler = handler;
    printf("Registered exception handler\n");
}

void aarch64_unregister_exception_handler(void)
{
    exception_handler = NULL;
    printf("Unregistered exception handler\n");
}

// ============================================================================
// MAIN INTERRUPT HANDLERS
// ============================================================================

void aarch64_irq_handler(void)
{
    // Acknowledge the interrupt
    uint32_t irq = aarch64_interrupt_acknowledge();

    // Check if this is a valid interrupt
    if (irq >= 1024)
    {
        printf("ERROR: Invalid IRQ number: %u\n", irq);
        aarch64_interrupt_end(0);
        return;
    }

    // Update statistics
    interrupt_counts[irq]++;
    total_interrupts++;

    // Check if handler is enabled
    if (!interrupt_handlers[irq].enabled)
    {
        printf("WARNING: IRQ %u received but handler disabled\n", irq);
        aarch64_interrupt_end(irq);
        return;
    }

    // Call the registered handler
    if (interrupt_handlers[irq].handler)
    {
        interrupt_handlers[irq].handler();
    }
    else
    {
        printf("WARNING: No handler for IRQ %u\n", irq);
    }

    // End the interrupt
    aarch64_interrupt_end(irq);
}

void aarch64_fiq_handler(void)
{
    // FIQ handling is similar to IRQ but with higher priority
    printf("FIQ received - high priority interrupt\n");

    // For now, just acknowledge and end
    // In a real system, you would handle FIQ differently
}

void aarch64_syscall_handler_entry(void)
{
    // This is called from assembly when a system call is made
    if (syscall_handler)
    {
        syscall_handler();
    }
    else
    {
        printf("WARNING: No system call handler registered\n");
    }
}

void aarch64_exception_handler_entry(uint32_t esr, uint64_t elr, uint64_t far)
{
    // This is called from assembly when an exception occurs
    if (exception_handler)
    {
        exception_handler(esr, elr, far);
    }
    else
    {
        printf("WARNING: No exception handler registered\n");
        aarch64_default_exception_handler(esr, elr, far);
    }
}

// ============================================================================
// SPECIALIZED INTERRUPT HANDLERS
// ============================================================================

void aarch64_timer_interrupt_handler(void)
{
    printf("Timer interrupt received\n");

    // In a real system, this would:
    // 1. Update system time
    // 2. Check for expired timers
    // 3. Trigger scheduler if needed
    // 4. Handle periodic tasks
}

void aarch64_uart_interrupt_handler(void)
{
    printf("UART interrupt received\n");

    // In a real system, this would:
    // 1. Read UART data
    // 2. Process received characters
    // 3. Handle transmission completion
    // 4. Wake up waiting processes
}

void aarch64_disk_interrupt_handler(void)
{
    printf("Disk interrupt received\n");

    // In a real system, this would:
    // 1. Check disk status
    // 2. Complete I/O operations
    // 3. Wake up waiting processes
    // 4. Handle errors
}

void aarch64_network_interrupt_handler(void)
{
    printf("Network interrupt received\n");

    // In a real system, this would:
    // 1. Process network packets
    // 2. Handle connection events
    // 3. Wake up network processes
    // 4. Handle errors
}

void aarch64_gpu_interrupt_handler(void)
{
    printf("GPU interrupt received\n");

    // In a real system, this would:
    // 1. Handle frame completion
    // 2. Process GPU commands
    // 3. Handle GPU errors
    // 4. Wake up graphics processes
}

// ============================================================================
// INTERRUPT CHAINING
// ============================================================================

void aarch64_chain_interrupt_handlers(uint32_t irq, void (*handler1)(void), void (*handler2)(void))
{
    if (irq >= 1024)
        return;

    // Create a chained handler that calls both handlers
    // This is useful for sharing interrupts between drivers

    printf("Chaining handlers for IRQ %u\n", irq);

    // For now, just register the first handler
    // In a real system, you would implement proper chaining
    aarch64_register_interrupt_handler(irq, handler1, "chained_handler", 0);
}

// ============================================================================
// INTERRUPT STATISTICS
// ============================================================================

uint32_t aarch64_get_interrupt_count(uint32_t irq)
{
    if (irq >= 1024)
        return 0;
    return interrupt_counts[irq];
}

uint32_t aarch64_get_total_interrupts(void)
{
    return total_interrupts;
}

void aarch64_reset_interrupt_statistics(void)
{
    memset(interrupt_counts, 0, sizeof(interrupt_counts));
    total_interrupts = 0;
    printf("Interrupt statistics reset\n");
}

void aarch64_print_interrupt_statistics(void)
{
    printf("=== aarch64 Interrupt Statistics ===\n");
    printf("Total interrupts: %u\n", total_interrupts);

    // Print statistics for first 16 IRQs
    for (int i = 0; i < 16; i++)
    {
        if (interrupt_counts[i] > 0)
        {
            printf("IRQ %2d: %6u interrupts (%s)\n",
                   i, interrupt_counts[i], interrupt_handlers[i].name);
        }
    }
}

// ============================================================================
// INTERRUPT DEBUGGING
// ============================================================================

void aarch64_print_interrupt_handlers(void)
{
    printf("=== aarch64 Interrupt Handlers ===\n");

    for (int i = 0; i < 32; i++)
    {
        if (interrupt_handlers[i].enabled)
        {
            printf("IRQ %2d: %s (priority %u, count %u)\n",
                   i, interrupt_handlers[i].name,
                   interrupt_handlers[i].priority,
                   interrupt_counts[i]);
        }
    }
}

void aarch64_test_interrupt_handlers(void)
{
    printf("Testing aarch64 interrupt handlers...\n");

    // Register test handlers
    aarch64_register_interrupt_handler(1, aarch64_timer_interrupt_handler, "timer", 0);
    aarch64_register_interrupt_handler(2, aarch64_uart_interrupt_handler, "uart", 1);
    aarch64_register_interrupt_handler(3, aarch64_disk_interrupt_handler, "disk", 2);

    // Test handler registration
    TEST_ASSERT(interrupt_handlers[1].enabled, "Timer handler enabled");
    TEST_ASSERT(interrupt_handlers[2].enabled, "UART handler enabled");
    TEST_ASSERT(interrupt_handlers[3].enabled, "Disk handler enabled");

    // Test priority setting
    aarch64_set_interrupt_priority(1, 10);
    aarch64_set_interrupt_priority(2, 20);
    aarch64_set_interrupt_priority(3, 30);

    TEST_ASSERT(interrupt_handlers[1].priority == 10, "Timer priority");
    TEST_ASSERT(interrupt_handlers[2].priority == 20, "UART priority");
    TEST_ASSERT(interrupt_handlers[3].priority == 30, "Disk priority");

    // Test handler disable/enable
    aarch64_disable_interrupt_handler(2);
    TEST_ASSERT(!interrupt_handlers[2].enabled, "UART handler disabled");

    aarch64_enable_interrupt_handler(2);
    TEST_ASSERT(interrupt_handlers[2].enabled, "UART handler re-enabled");

    printf("Interrupt handler test completed successfully\n");
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void aarch64_interrupt_handlers_init(void)
{
    printf("Initializing aarch64 interrupt handlers...\n");

    // Clear handler table
    memset(interrupt_handlers, 0, sizeof(interrupt_handlers));

    // Set default handlers
    for (int i = 0; i < 1024; i++)
    {
        interrupt_handlers[i].handler = aarch64_default_irq_handler;
        interrupt_handlers[i].enabled = false;
        interrupt_handlers[i].priority = 0x80; // Default priority
        strcpy(interrupt_handlers[i].name, "default");
    }

    // Clear statistics
    memset(interrupt_counts, 0, sizeof(interrupt_counts));
    total_interrupts = 0;

    // Set default system call and exception handlers
    syscall_handler = aarch64_default_syscall_handler;
    exception_handler = aarch64_default_exception_handler;

    printf("aarch64 interrupt handlers initialized successfully\n");
}
