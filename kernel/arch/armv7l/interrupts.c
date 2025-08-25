/*
 * ORION OS - ARMv7l Interrupt Management
 * 
 * This file implements the interrupt management system for ARMv7l architecture,
 * including IRQ and FIQ handling, interrupt controllers, and exception vectors.
 */

#include "arch.h"
#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

static arm_interrupt_context_t g_irq_context;
static arm_interrupt_context_t g_fiq_context;
static arm_interrupt_context_t g_abort_context;
static arm_interrupt_context_t g_undefined_context;

static bool g_interrupts_initialized = false;
static bool g_irq_enabled = false;
static bool g_fiq_enabled = false;

// ============================================================================
// INTERRUPT INITIALIZATION
// ============================================================================

int arm_interrupt_init(void)
{
    if (g_interrupts_initialized) {
        return 0;
    }

    // Initialize interrupt contexts
    memset(&g_irq_context, 0, sizeof(arm_interrupt_context_t));
    memset(&g_fiq_context, 0, sizeof(arm_fiq_context_t));
    memset(&g_abort_context, 0, sizeof(arm_interrupt_context_t));
    memset(&g_undefined_context, 0, sizeof(arm_interrupt_context_t));

    // Set up exception vectors
    arm_setup_exception_vectors();

    // Initialize interrupt controller
    arm_interrupt_controller_init();

    // Enable IRQ and FIQ
    arm_enable_irq();
    arm_enable_fiq();

    g_interrupts_initialized = true;
    return 0;
}

// ============================================================================
// INTERRUPT CONTROL
// ============================================================================

void arm_enable_irq(void)
{
    if (!g_interrupts_initialized) {
        return;
    }

    __asm__ volatile("cpsie i");
    g_irq_enabled = true;
}

void arm_disable_irq(void)
{
    if (!g_interrupts_initialized) {
        return;
    }

    __asm__ volatile("cpsid i");
    g_irq_enabled = false;
}

bool arm_is_irq_enabled(void)
{
    return g_irq_enabled && g_interrupts_initialized;
}

void arm_enable_fiq(void)
{
    if (!g_interrupts_initialized) {
        return;
    }

    __asm__ volatile("cpsie f");
    g_fiq_enabled = true;
}

void arm_disable_fiq(void)
{
    if (!g_interrupts_initialized) {
        return;
    }

    __asm__ volatile("cpsid f");
    g_fiq_enabled = false;
}

bool arm_is_fiq_enabled(void)
{
    return g_fiq_enabled && g_interrupts_initialized;
}

// ============================================================================
// INTERRUPT HANDLER REGISTRATION
// ============================================================================

int arm_register_irq_handler(uint32_t irq_number, arm_irq_handler_t handler, void *data)
{
    if (!g_interrupts_initialized || !handler) {
        return -1;
    }

    if (irq_number >= ARM_MAX_IRQS) {
        return -1;
    }

    // Register handler in interrupt controller
    return arm_interrupt_controller_register_handler(irq_number, handler, data);
}

int arm_unregister_irq_handler(uint32_t irq_number)
{
    if (!g_interrupts_initialized) {
        return -1;
    }

    if (irq_number >= ARM_MAX_IRQS) {
        return -1;
    }

    // Unregister handler from interrupt controller
    return arm_interrupt_controller_unregister_handler(irq_number);
}

int arm_register_fiq_handler(arm_fiq_handler_t handler, void *data)
{
    if (!g_interrupts_initialized || !handler) {
        return -1;
    }

    // FIQ can only have one handler
    g_fiq_context.handler = handler;
    g_fiq_context.data = data;
    return 0;
}

int arm_unregister_fiq_handler(void)
{
    if (!g_interrupts_initialized) {
        return -1;
    }

    g_fiq_context.handler = NULL;
    g_fiq_context.data = NULL;
    return 0;
}

// ============================================================================
// INTERRUPT PRIORITY
// ============================================================================

int arm_set_irq_priority(uint32_t irq_number, uint32_t priority)
{
    if (!g_interrupts_initialized) {
        return -1;
    }

    if (irq_number >= ARM_MAX_IRQS) {
        return -1;
    }

    // Set priority in interrupt controller
    return arm_interrupt_controller_set_priority(irq_number, priority);
}

uint32_t arm_get_irq_priority(uint32_t irq_number)
{
    if (!g_interrupts_initialized) {
        return 0;
    }

    if (irq_number >= ARM_MAX_IRQS) {
        return 0;
    }

    // Get priority from interrupt controller
    return arm_interrupt_controller_get_priority(irq_number);
}

// ============================================================================
// INTERRUPT CONTROLLER INTERFACE
// ============================================================================

int arm_interrupt_controller_init(void)
{
    // Initialize the specific interrupt controller (GIC, etc.)
    // This is architecture-specific and should be implemented
    // based on the actual hardware being used
    return 0;
}

int arm_interrupt_controller_register_handler(uint32_t irq_number, arm_irq_handler_t handler, void *data)
{
    // Register handler in the interrupt controller
    // This is architecture-specific
    (void)irq_number;
    (void)handler;
    (void)data;
    return 0;
}

int arm_interrupt_controller_unregister_handler(uint32_t irq_number)
{
    // Unregister handler from the interrupt controller
    // This is architecture-specific
    (void)irq_number;
    return 0;
}

int arm_interrupt_controller_set_priority(uint32_t irq_number, uint32_t priority)
{
    // Set priority in the interrupt controller
    // This is architecture-specific
    (void)irq_number;
    (void)priority;
    return 0;
}

uint32_t arm_interrupt_controller_get_priority(uint32_t irq_number)
{
    // Get priority from the interrupt controller
    // This is architecture-specific
    (void)irq_number;
    return 0;
}

// ============================================================================
// EXCEPTION VECTOR SETUP
// ============================================================================

void arm_setup_exception_vectors(void)
{
    // Set up the exception vector table
    // This should point to the assembly exception handlers
    uint32_t vbar = (uint32_t)&arm_exception_vector_table;
    __asm__ volatile("mcr p15, 0, %0, c12, c0, 0" : : "r"(vbar));
}

// ============================================================================
// INTERRUPT ACKNOWLEDGMENT
// ============================================================================

void arm_irq_acknowledge(uint32_t irq_number)
{
    if (!g_interrupts_initialized) {
        return;
    }

    if (irq_number >= ARM_MAX_IRQS) {
        return;
    }

    // Acknowledge the interrupt in the controller
    arm_interrupt_controller_acknowledge(irq_number);
}

void arm_fiq_acknowledge(void)
{
    if (!g_interrupts_initialized) {
        return;
    }

    // FIQ acknowledgment is typically automatic
    // but can be implemented if needed
}

// ============================================================================
// INTERRUPT CONTROLLER ACKNOWLEDGMENT
// ============================================================================

void arm_interrupt_controller_acknowledge(uint32_t irq_number)
{
    // Acknowledge interrupt in the controller
    // This is architecture-specific
    (void)irq_number;
}

// ============================================================================
// INTERRUPT STATE MANAGEMENT
// ============================================================================

void arm_save_interrupt_state(arm_interrupt_state_t *state)
{
    if (!state) {
        return;
    }

    state->cpsr = arm_cpu_read_cpsr();
    state->irq_enabled = g_irq_enabled;
    state->fiq_enabled = g_fiq_enabled;
}

void arm_restore_interrupt_state(const arm_interrupt_state_t *state)
{
    if (!state) {
        return;
    }

    if (state->irq_enabled) {
        arm_enable_irq();
    } else {
        arm_disable_irq();
    }

    if (state->fiq_enabled) {
        arm_enable_fiq();
    } else {
        arm_disable_fiq();
    }

    arm_cpu_write_cpsr(state->cpsr);
}

// ============================================================================
// INTERRUPT STATISTICS
// ============================================================================

uint32_t arm_get_irq_count(void)
{
    // Return the number of IRQs handled
    // This can be implemented for debugging/monitoring
    return 0;
}

uint32_t arm_get_fiq_count(void)
{
    // Return the number of FIQs handled
    // This can be implemented for debugging/monitoring
    return 0;
}

// ============================================================================
// INTERRUPT DEBUGGING
// ============================================================================

void arm_dump_interrupt_state(void)
{
    // Dump current interrupt state for debugging
    // This can be implemented for debugging purposes
}

void arm_interrupt_test(void)
{
    // Test interrupt functionality
    // This can be implemented for testing purposes
}
