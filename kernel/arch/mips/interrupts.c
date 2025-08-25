/*
 * ORION OS - MIPS Architecture Interrupt Management
 *
 * Interrupt management for MIPS architecture (MIPS32, MIPS64)
 * Implements interrupt controller and handler management
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// Interrupt controller context
static mips_interrupt_controller_t mips_interrupt_controller = {0};

// Interrupt handler table
static struct {
    void (*handler)(void);
    void *data;
    uint32_t priority;
    bool enabled;
} mips_irq_handlers[MIPS_MAX_IRQS] = {0};

// Exception handler table
static struct {
    void (*handler)(void);
    void *data;
} mips_exception_handlers[MIPS_MAX_EXCEPTIONS] = {0};

// Interrupt statistics
static struct {
    uint64_t irq_count[MIPS_MAX_IRQS];
    uint64_t exception_count[MIPS_MAX_EXCEPTIONS];
    uint64_t total_irqs;
    uint64_t total_exceptions;
} mips_interrupt_stats = {0};

// ============================================================================
// INTERRUPT CONTROLLER FUNCTIONS
// ============================================================================

void mips_interrupt_init(void)
{
    printf("MIPS: Initializing interrupt controller\n");

    // Initialize interrupt controller context
    memset(&mips_interrupt_controller, 0, sizeof(mips_interrupt_controller));
    mips_interrupt_controller.vector_base = MIPS_VECTOR_BASE;
    mips_interrupt_controller.vector_count = MIPS_MAX_IRQS;
    mips_interrupt_controller.priority_bits = MIPS_PRIORITY_BITS;
    mips_interrupt_controller.external_enabled = false;
    mips_interrupt_controller.timer_enabled = false;

    // Clear interrupt handler tables
    memset(mips_irq_handlers, 0, sizeof(mips_irq_handlers));
    memset(mips_exception_handlers, 0, sizeof(mips_exception_handlers));

    // Clear interrupt statistics
    memset(&mips_interrupt_stats, 0, sizeof(mips_interrupt_stats));

    printf("MIPS: Interrupt controller initialization complete\n");
}

void mips_interrupt_enable_irq(uint32_t irq_number)
{
    if (irq_number < MIPS_MAX_IRQS)
    {
        // Enable IRQ in CP0 Status register
        uint64_t status = mips_cpu_read_cp0_status();
        status |= (1 << (irq_number + 8)); // Set corresponding IP bit
        mips_cpu_write_cp0_status(status);

        // Mark IRQ as enabled in handler table
        mips_irq_handlers[irq_number].enabled = true;

        printf("MIPS: IRQ %u enabled\n", irq_number);
    }
}

void mips_interrupt_disable_irq(uint32_t irq_number)
{
    if (irq_number < MIPS_MAX_IRQS)
    {
        // Disable IRQ in CP0 Status register
        uint64_t status = mips_cpu_read_cp0_status();
        status &= ~(1 << (irq_number + 8)); // Clear corresponding IP bit
        mips_cpu_write_cp0_status(status);

        // Mark IRQ as disabled in handler table
        mips_irq_handlers[irq_number].enabled = false;

        printf("MIPS: IRQ %u disabled\n", irq_number);
    }
}

bool mips_interrupt_is_irq_enabled(uint32_t irq_number)
{
    if (irq_number < MIPS_MAX_IRQS)
    {
        uint64_t status = mips_cpu_read_cp0_status();
        return (status & (1 << (irq_number + 8))) != 0;
    }
    return false;
}

void mips_interrupt_register_handler(uint32_t irq_number, void (*handler)(void), void *data)
{
    if (irq_number < MIPS_MAX_IRQS && handler)
    {
        mips_irq_handlers[irq_number].handler = handler;
        mips_irq_handlers[irq_number].data = data;
        mips_irq_handlers[irq_number].priority = 0; // Default priority
        mips_irq_handlers[irq_number].enabled = false; // Disabled by default

        printf("MIPS: IRQ handler registered for IRQ %u\n", irq_number);
    }
}

void mips_interrupt_unregister_handler(uint32_t irq_number)
{
    if (irq_number < MIPS_MAX_IRQS)
    {
        mips_irq_handlers[irq_number].handler = NULL;
        mips_irq_handlers[irq_number].data = NULL;
        mips_irq_handlers[irq_number].enabled = false;

        printf("MIPS: IRQ handler unregistered for IRQ %u\n", irq_number);
    }
}

void mips_interrupt_set_priority(uint32_t irq_number, uint32_t priority)
{
    if (irq_number < MIPS_MAX_IRQS && priority < (1 << MIPS_PRIORITY_BITS))
    {
        mips_irq_handlers[irq_number].priority = priority;
        printf("MIPS: IRQ %u priority set to %u\n", irq_number, priority);
    }
}

// ============================================================================
// EXTERNAL INTERRUPT CONTROL
// ============================================================================

void mips_interrupt_enable_external(void)
{
    mips_interrupt_controller.external_enabled = true;
    mips_cpu_enable_interrupts();
    printf("MIPS: External interrupts enabled\n");
}

void mips_interrupt_disable_external(void)
{
    mips_interrupt_controller.external_enabled = false;
    mips_cpu_disable_interrupts();
    printf("MIPS: External interrupts disabled\n");
}

bool mips_is_external_enabled(void)
{
    return mips_interrupt_controller.external_enabled;
}

// ============================================================================
// TIMER INTERRUPT CONTROL
// ============================================================================

void mips_interrupt_enable_timer(void)
{
    mips_interrupt_controller.timer_enabled = true;
    
    // Enable timer interrupt in CP0 Status
    uint64_t status = mips_cpu_read_cp0_status();
    status |= MIPS_STATUS_IP7; // Timer interrupt (IP7)
    mips_cpu_write_cp0_status(status);

    printf("MIPS: Timer interrupts enabled\n");
}

void mips_interrupt_disable_timer(void)
{
    mips_interrupt_controller.timer_enabled = false;
    
    // Disable timer interrupt in CP0 Status
    uint64_t status = mips_cpu_read_cp0_status();
    status &= ~MIPS_STATUS_IP7; // Timer interrupt (IP7)
    mips_cpu_write_cp0_status(status);

    printf("MIPS: Timer interrupts disabled\n");
}

bool mips_is_timer_enabled(void)
{
    return mips_interrupt_controller.timer_enabled;
}

// ============================================================================
// EXCEPTION HANDLER MANAGEMENT
// ============================================================================

int mips_register_exception_handler(uint32_t exception_type, void (*handler)(void), void *data)
{
    if (exception_type < MIPS_MAX_EXCEPTIONS && handler)
    {
        mips_exception_handlers[exception_type].handler = handler;
        mips_exception_handlers[exception_type].data = data;
        
        printf("MIPS: Exception handler registered for type %u\n", exception_type);
        return 0; // Success
    }
    return -1; // Error
}

void mips_unregister_exception_handler(uint32_t exception_type)
{
    if (exception_type < MIPS_MAX_EXCEPTIONS)
    {
        mips_exception_handlers[exception_type].handler = NULL;
        mips_exception_handlers[exception_type].data = NULL;
        
        printf("MIPS: Exception handler unregistered for type %u\n", exception_type);
    }
}

void mips_exception_init(void)
{
    printf("MIPS: Initializing exception handling\n");
    
    // Clear exception handler table
    memset(mips_exception_handlers, 0, sizeof(mips_exception_handlers));
    
    printf("MIPS: Exception handling initialization complete\n");
}

void mips_exception_register_handler(uint32_t exception_type, void (*handler)(void))
{
    mips_register_exception_handler(exception_type, handler, NULL);
}

void mips_exception_unregister_handler(uint32_t exception_type)
{
    mips_unregister_exception_handler(exception_type);
}

// ============================================================================
// INTERRUPT DISPATCHING
// ============================================================================

void mips_dispatch_irq(uint32_t irq_number)
{
    if (irq_number < MIPS_MAX_IRQS && mips_irq_handlers[irq_number].handler)
    {
        // Update statistics
        mips_interrupt_stats.irq_count[irq_number]++;
        mips_interrupt_stats.total_irqs++;

        // Call handler
        mips_irq_handlers[irq_number].handler();
    }
}

void mips_dispatch_exception(uint32_t exception_type)
{
    if (exception_type < MIPS_MAX_EXCEPTIONS && mips_exception_handlers[exception_type].handler)
    {
        // Update statistics
        mips_interrupt_stats.exception_count[exception_type]++;
        mips_interrupt_stats.total_exceptions++;

        // Call handler
        mips_exception_handlers[exception_type].handler();
    }
}

// ============================================================================
// INTERRUPT ACKNOWLEDGMENT
// ============================================================================

void mips_interrupt_acknowledge(uint32_t irq_number)
{
    if (irq_number < MIPS_MAX_IRQS)
    {
        // For MIPS, acknowledgment is typically done by the specific device
        // This is a placeholder for device-specific acknowledgment
        
        printf("MIPS: IRQ %u acknowledged\n", irq_number);
    }
}

void mips_timer_interrupt_acknowledge(void)
{
    // Clear timer interrupt by writing to CP0 Compare
    uint64_t current_count = mips_cpu_read_cp0_count();
    uint64_t compare_value = current_count + MIPS_CP0_COUNT_FREQ / 1000; // 1ms
    mips_cpu_write_cp0_compare(compare_value);
}

// ============================================================================
// INTERRUPT STATISTICS
// ============================================================================

uint64_t mips_get_irq_count(uint32_t irq_number)
{
    if (irq_number < MIPS_MAX_IRQS)
    {
        return mips_interrupt_stats.irq_count[irq_number];
    }
    return 0;
}

uint64_t mips_get_exception_count(uint32_t exception_type)
{
    if (exception_type < MIPS_MAX_EXCEPTIONS)
    {
        return mips_interrupt_stats.exception_count[exception_type];
    }
    return 0;
}

uint64_t mips_get_total_irqs(void)
{
    return mips_interrupt_stats.total_irqs;
}

uint64_t mips_get_total_exceptions(void)
{
    return mips_interrupt_stats.total_exceptions;
}

void mips_reset_interrupt_statistics(void)
{
    memset(&mips_interrupt_stats, 0, sizeof(mips_interrupt_stats));
    printf("MIPS: Interrupt statistics reset\n");
}

// ============================================================================
// INTERRUPT PRIORITY MANAGEMENT
// ============================================================================

uint32_t mips_get_highest_priority_irq(void)
{
    uint32_t highest_priority = 0;
    uint32_t highest_priority_irq = MIPS_MAX_IRQS;

    for (uint32_t i = 0; i < MIPS_MAX_IRQS; i++)
    {
        if (mips_irq_handlers[i].enabled && 
            mips_irq_handlers[i].priority > highest_priority)
        {
            highest_priority = mips_irq_handlers[i].priority;
            highest_priority_irq = i;
        }
    }

    return highest_priority_irq;
}

void mips_set_global_priority_threshold(uint32_t threshold)
{
    // Set global priority threshold for interrupt handling
    // This would be implemented based on the specific MIPS processor
    printf("MIPS: Global priority threshold set to %u\n", threshold);
}

// ============================================================================
// INTERRUPT CONTROLLER STATUS
// ============================================================================

mips_interrupt_controller_t *mips_get_interrupt_controller(void)
{
    return &mips_interrupt_controller;
}

bool mips_is_interrupt_pending(void)
{
    // Check if any interrupts are pending
    uint64_t cause = mips_cpu_read_cp0_cause();
    uint64_t status = mips_cpu_read_cp0_status();
    
    return (cause & status & MIPS_STATUS_IP) != 0;
}

uint32_t mips_get_pending_interrupts(void)
{
    // Get mask of pending interrupts
    uint64_t cause = mips_cpu_read_cp0_cause();
    uint64_t status = mips_cpu_read_cp0_status();
    
    return (uint32_t)(cause & status & MIPS_STATUS_IP);
}
