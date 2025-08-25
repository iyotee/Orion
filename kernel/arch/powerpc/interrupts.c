/*
 * ORION OS - POWER Architecture Interrupt Management
 *
 * Interrupt handling and vector table management for POWER architecture
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

// ============================================================================
// INTERRUPT VECTOR TABLE
// ============================================================================

#define POWER_MAX_IRQS 256
#define POWER_MAX_VECTORS 64

typedef struct {
    void (*handler)(void *);
    void *data;
    uint64_t priority;
    bool enabled;
    bool edge_triggered;
    uint64_t affinity;
} power_irq_info_t;

typedef struct {
    uint64_t vector_base;
    uint64_t vector_count;
    power_irq_info_t irqs[POWER_MAX_IRQS];
    uint64_t active_irqs;
    uint64_t pending_irqs;
    uint64_t masked_irqs;
} power_interrupt_controller_t;

static power_interrupt_controller_t power_ic;

// ============================================================================
// INTERRUPT INITIALIZATION
// ============================================================================

int power_interrupt_init(void)
{
    memset(&power_ic, 0, sizeof(power_ic));
    
    // Set up vector table base
    power_ic.vector_base = (uint64_t)&power_interrupt_vector;
    power_ic.vector_count = POWER_MAX_VECTORS;
    
    // Initialize all IRQs
    for (int i = 0; i < POWER_MAX_IRQS; i++) {
        power_ic.irqs[i].handler = NULL;
        power_ic.irqs[i].data = NULL;
        power_ic.irqs[i].priority = 0;
        power_ic.irqs[i].enabled = false;
        power_ic.irqs[i].edge_triggered = false;
        power_ic.irqs[i].affinity = 0;
    }
    
    // Set up XIVE (eXternal Interrupt Virtualization Engine) if available
    if (power_cpu_features & POWER_FEATURE_XIVE) {
        return power_xive_init();
    }
    
    // Set up traditional interrupt controller
    return power_traditional_ic_init();
}

int power_xive_init(void)
{
    // Initialize XIVE interrupt controller
    // This would typically involve setting up XIVE registers and tables
    
    // Set up XIVE base address
    uint64_t xive_base = 0x4000000000ULL; // Example base address
    
    // Set up XIVE configuration
    // This would involve writing to XIVE configuration registers
    
    return 0;
}

int power_traditional_ic_init(void)
{
    // Initialize traditional interrupt controller
    // This would typically involve setting up interrupt controller registers
    
    // Set up interrupt vector base
    __asm__ volatile("mtspr 63, %0" : : "r"(power_ic.vector_base));
    
    // Set up exception vector base
    __asm__ volatile("mtspr 62, %0" : : "r"((uint64_t)&power_exception_vector));
    
    // Enable external interrupts in MSR
    uint64_t msr;
    __asm__ volatile("mfmsr %0" : "=r"(msr));
    msr |= POWER_MSR_EE;
    __asm__ volatile("mtmsr %0" : : "r"(msr));
    
    return 0;
}

// ============================================================================
// INTERRUPT REGISTRATION
// ============================================================================

int power_interrupt_register(uint64_t irq, void (*handler)(void *), void *data)
{
    if (irq >= POWER_MAX_IRQS || !handler) {
        return -1;
    }
    
    // Check if IRQ is already registered
    if (power_ic.irqs[irq].handler) {
        return -1;
    }
    
    // Register the interrupt
    power_ic.irqs[irq].handler = handler;
    power_ic.irqs[irq].data = data;
    power_ic.irqs[irq].enabled = false;
    power_ic.irqs[irq].priority = 0;
    power_ic.irqs[irq].edge_triggered = false;
    power_ic.irqs[irq].affinity = 0;
    
    power_ic.active_irqs++;
    
    return 0;
}

int power_interrupt_unregister(uint64_t irq)
{
    if (irq >= POWER_MAX_IRQS) {
        return -1;
    }
    
    // Check if IRQ is registered
    if (!power_ic.irqs[irq].handler) {
        return -1;
    }
    
    // Disable the interrupt first
    power_interrupt_disable(irq);
    
    // Clear the registration
    power_ic.irqs[irq].handler = NULL;
    power_ic.irqs[irq].data = NULL;
    power_ic.irqs[irq].enabled = false;
    power_ic.irqs[irq].priority = 0;
    power_ic.irqs[irq].edge_triggered = false;
    power_ic.irqs[irq].affinity = 0;
    
    power_ic.active_irqs--;
    
    return 0;
}

// ============================================================================
// INTERRUPT ENABLE/DISABLE
// ============================================================================

int power_interrupt_enable(uint64_t irq)
{
    if (irq >= POWER_MAX_IRQS) {
        return -1;
    }
    
    // Check if IRQ is registered
    if (!power_ic.irqs[irq].handler) {
        return -1;
    }
    
    // Enable the interrupt
    power_ic.irqs[irq].enabled = true;
    
    // If using XIVE, enable in XIVE
    if (power_cpu_features & POWER_FEATURE_XIVE) {
        return power_xive_enable_irq(irq);
    }
    
    // For traditional controller, enable in MSR
    uint64_t msr;
    __asm__ volatile("mfmsr %0" : "=r"(msr));
    msr |= POWER_MSR_EE;
    __asm__ volatile("mtmsr %0" : : "r"(msr));
    
    return 0;
}

int power_interrupt_disable(uint64_t irq)
{
    if (irq >= POWER_MAX_IRQS) {
        return -1;
    }
    
    // Check if IRQ is registered
    if (!power_ic.irqs[irq].handler) {
        return -1;
    }
    
    // Disable the interrupt
    power_ic.irqs[irq].enabled = false;
    
    // If using XIVE, disable in XIVE
    if (power_cpu_features & POWER_FEATURE_XIVE) {
        return power_xive_disable_irq(irq);
    }
    
    // For traditional controller, check if any other IRQs are enabled
    bool any_enabled = false;
    for (int i = 0; i < POWER_MAX_IRQS; i++) {
        if (power_ic.irqs[i].enabled) {
            any_enabled = true;
            break;
        }
    }
    
    // If no IRQs are enabled, disable external interrupts in MSR
    if (!any_enabled) {
        uint64_t msr;
        __asm__ volatile("mfmsr %0" : "=r"(msr));
        msr &= ~POWER_MSR_EE;
        __asm__ volatile("mtmsr %0" : : "r"(msr));
    }
    
    return 0;
}

// ============================================================================
// INTERRUPT ACKNOWLEDGMENT
// ============================================================================

int power_interrupt_ack(uint64_t irq)
{
    if (irq >= POWER_MAX_IRQS) {
        return -1;
    }
    
    // Check if IRQ is registered
    if (!power_ic.irqs[irq].handler) {
        return -1;
    }
    
    // If using XIVE, acknowledge in XIVE
    if (power_cpu_features & POWER_FEATURE_XIVE) {
        return power_xive_ack_irq(irq);
    }
    
    // For traditional controller, clear pending bit
    // This would typically involve writing to interrupt controller registers
    
    return 0;
}

// ============================================================================
// INTERRUPT PRIORITY MANAGEMENT
// ============================================================================

int power_interrupt_set_priority(uint64_t irq, uint64_t priority)
{
    if (irq >= POWER_MAX_IRQS) {
        return -1;
    }
    
    // Check if IRQ is registered
    if (!power_ic.irqs[irq].handler) {
        return -1;
    }
    
    // Set the priority
    power_ic.irqs[irq].priority = priority;
    
    // If using XIVE, set priority in XIVE
    if (power_cpu_features & POWER_FEATURE_XIVE) {
        return power_xive_set_priority(irq, priority);
    }
    
    return 0;
}

int power_interrupt_get_priority(uint64_t irq, uint64_t *priority)
{
    if (irq >= POWER_MAX_IRQS || !priority) {
        return -1;
    }
    
    // Check if IRQ is registered
    if (!power_ic.irqs[irq].handler) {
        return -1;
    }
    
    *priority = power_ic.irqs[irq].priority;
    return 0;
}

// ============================================================================
// INTERRUPT AFFINITY MANAGEMENT
// ============================================================================

int power_interrupt_set_affinity(uint64_t irq, uint64_t cpu_mask)
{
    if (irq >= POWER_MAX_IRQS) {
        return -1;
    }
    
    // Check if IRQ is registered
    if (!power_ic.irqs[irq].handler) {
        return -1;
    }
    
    // Set the affinity
    power_ic.irqs[irq].affinity = cpu_mask;
    
    // If using XIVE, set affinity in XIVE
    if (power_cpu_features & POWER_FEATURE_XIVE) {
        return power_xive_set_affinity(irq, cpu_mask);
    }
    
    return 0;
}

int power_interrupt_get_affinity(uint64_t irq, uint64_t *cpu_mask)
{
    if (irq >= POWER_MAX_IRQS || !cpu_mask) {
        return -1;
    }
    
    // Check if IRQ is registered
    if (!power_ic.irqs[irq].handler) {
        return -1;
    }
    
    *cpu_mask = power_ic.irqs[irq].affinity;
    return 0;
}

// ============================================================================
// INTERRUPT TRIGGER MODE
// ============================================================================

int power_interrupt_set_edge_triggered(uint64_t irq)
{
    if (irq >= POWER_MAX_IRQS) {
        return -1;
    }
    
    // Check if IRQ is registered
    if (!power_ic.irqs[irq].handler) {
        return -1;
    }
    
    // Set edge-triggered mode
    power_ic.irqs[irq].edge_triggered = true;
    
    // If using XIVE, set trigger mode in XIVE
    if (power_cpu_features & POWER_FEATURE_XIVE) {
        return power_xive_set_edge_triggered(irq);
    }
    
    return 0;
}

int power_interrupt_set_level_triggered(uint64_t irq)
{
    if (irq >= POWER_MAX_IRQS) {
        return -1;
    }
    
    // Check if IRQ is registered
    if (!power_ic.irqs[irq].handler) {
        return -1;
    }
    
    // Set level-triggered mode
    power_ic.irqs[irq].edge_triggered = false;
    
    // If using XIVE, set trigger mode in XIVE
    if (power_cpu_features & POWER_FEATURE_XIVE) {
        return power_xive_set_level_triggered(irq);
    }
    
    return 0;
}

// ============================================================================
// INTERRUPT HANDLING
// ============================================================================

void power_interrupt_handler(uint64_t irq, power_cpu_context_t *context)
{
    if (irq >= POWER_MAX_IRQS || !context) {
        return;
    }
    
    // Check if IRQ is registered and enabled
    if (!power_ic.irqs[irq].handler || !power_ic.irqs[irq].enabled) {
        return;
    }
    
    // Mark IRQ as pending
    power_ic.pending_irqs |= (1ULL << irq);
    
    // Call the interrupt handler
    power_ic.irqs[irq].handler(power_ic.irqs[irq].data);
    
    // Clear pending bit
    power_ic.pending_irqs &= ~(1ULL << irq);
    
    // Acknowledge the interrupt
    power_interrupt_ack(irq);
}

// ============================================================================
// INTERRUPT STATISTICS
// ============================================================================

int power_interrupt_get_stats(uint64_t irq, power_interrupt_stats_t *stats)
{
    if (irq >= POWER_MAX_IRQS || !stats) {
        return -1;
    }
    
    // Check if IRQ is registered
    if (!power_ic.irqs[irq].handler) {
        return -1;
    }
    
    // Get statistics
    stats->irq = irq;
    stats->enabled = power_ic.irqs[irq].enabled;
    stats->priority = power_ic.irqs[irq].priority;
    stats->edge_triggered = power_ic.irqs[irq].edge_triggered;
    stats->affinity = power_ic.irqs[irq].affinity;
    stats->pending = (power_ic.pending_irqs & (1ULL << irq)) != 0;
    
    return 0;
}

int power_interrupt_get_controller_info(power_ic_info_t *info)
{
    if (!info) {
        return -1;
    }
    
    info->vector_base = power_ic.vector_base;
    info->vector_count = power_ic.vector_count;
    info->active_irqs = power_ic.active_irqs;
    info->pending_irqs = power_ic.pending_irqs;
    info->masked_irqs = power_ic.masked_irqs;
    info->xive_supported = (power_cpu_features & POWER_FEATURE_XIVE) != 0;
    
    return 0;
}

// ============================================================================
// XIVE-SPECIFIC FUNCTIONS
// ============================================================================

#ifdef POWER_FEATURE_XIVE

int power_xive_enable_irq(uint64_t irq)
{
    // Enable IRQ in XIVE
    // This would involve writing to XIVE enable registers
    return 0;
}

int power_xive_disable_irq(uint64_t irq)
{
    // Disable IRQ in XIVE
    // This would involve writing to XIVE disable registers
    return 0;
}

int power_xive_ack_irq(uint64_t irq)
{
    // Acknowledge IRQ in XIVE
    // This would involve writing to XIVE acknowledge registers
    return 0;
}

int power_xive_set_priority(uint64_t irq, uint64_t priority)
{
    // Set IRQ priority in XIVE
    // This would involve writing to XIVE priority registers
    return 0;
}

int power_xive_set_affinity(uint64_t irq, uint64_t cpu_mask)
{
    // Set IRQ affinity in XIVE
    // This would involve writing to XIVE affinity registers
    return 0;
}

int power_xive_set_edge_triggered(uint64_t irq)
{
    // Set IRQ to edge-triggered mode in XIVE
    // This would involve writing to XIVE trigger mode registers
    return 0;
}

int power_xive_set_level_triggered(uint64_t irq)
{
    // Set IRQ to level-triggered mode in XIVE
    // This would involve writing to XIVE trigger mode registers
    return 0;
}

#endif // POWER_FEATURE_XIVE

// ============================================================================
// INTERRUPT CLEANUP
// ============================================================================

void power_interrupt_cleanup(void)
{
    // Disable all interrupts
    for (int i = 0; i < POWER_MAX_IRQS; i++) {
        if (power_ic.irqs[i].enabled) {
            power_interrupt_disable(i);
        }
    }
    
    // Clear interrupt vector table
    memset(&power_ic, 0, sizeof(power_ic));
    
    // Disable external interrupts in MSR
    uint64_t msr;
    __asm__ volatile("mfmsr %0" : "=r"(msr));
    msr &= ~POWER_MSR_EE;
    __asm__ volatile("mtmsr %0" : : "r"(msr));
}
