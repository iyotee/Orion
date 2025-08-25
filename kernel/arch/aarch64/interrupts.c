/*
 * Orion Operating System - aarch64 Interrupt Management
 *
 * Complete interrupt management system for aarch64 including:
 * - GIC (Generic Interrupt Controller) support
 * - Interrupt routing and priority management
 * - Interrupt vector table management
 * - System call handling
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
// GIC REGISTER DEFINITIONS
// ============================================================================

// GIC Distributor registers
#define GICD_CTLR 0x0000
#define GICD_TYPER 0x0004
#define GICD_IIDR 0x0008
#define GICD_IGROUPR 0x0080
#define GICD_ISENABLER 0x0100
#define GICD_ICENABLER 0x0180
#define GICD_ISPENDR 0x0200
#define GICD_ICPENDR 0x0280
#define GICD_ISACTIVER 0x0300
#define GICD_ICACTIVER 0x0380
#define GICD_IPRIORITYR 0x0400
#define GICD_ITARGETSR 0x0800
#define GICD_ICFGR 0x0C00
#define GICD_IGRPMODR 0x0D00
#define GICD_NSACR 0x0E00
#define GICD_SGIR 0x0F00
#define GICD_CPENDSGIR 0x0F10
#define GICD_SPENDSGIR 0x0F20

// GIC CPU Interface registers
#define GICC_CTLR 0x0000
#define GICC_PMR 0x0004
#define GICC_BPR 0x0008
#define GICC_IAR 0x000C
#define GICC_EOIR 0x0010
#define GICC_RPR 0x0014
#define GICC_HPPIR 0x0018
#define GICC_ABPR 0x001C
#define GICC_AIAR 0x0020
#define GICC_AEOIR 0x0024
#define GICC_AHPPIR 0x0028
#define GICC_APR 0x00D0
#define GICC_NSAPR 0x00E0
#define GICC_IIDR 0x00FC
#define GICC_DIR 0x1000

// GIC Redistributor registers
#define GICR_CTLR 0x0000
#define GICR_IIDR 0x0004
#define GICR_TYPER 0x0008
#define GICR_STATUSR 0x0010
#define GICR_WAKER 0x0014
#define GICR_SETLPIR 0x0040
#define GICR_CLRLPIR 0x0048
#define GICR_PROPBASER 0x0070
#define GICR_PENDBASER 0x0078
#define GICR_INVLPIR 0x00A0
#define GICR_INVALLR 0x00B0
#define GICR_SYNCR 0x00C0

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// GIC base addresses
static volatile uint32_t *gic_distributor = NULL;
static volatile uint32_t *gic_cpu_interface = NULL;
static volatile uint32_t *gic_redistributor = NULL;

// Interrupt handler table
static void (*interrupt_handlers[1024])(void);

// Interrupt priorities
static uint8_t interrupt_priorities[1024];

// Interrupt enable/disable state
static uint32_t interrupt_enabled[32]; // 1024 interrupts / 32 bits per word

// System call handler
static void (*syscall_handler)(void) = NULL;

// ============================================================================
// GIC ACCESS FUNCTIONS
// ============================================================================

static inline uint32_t gicd_read(uint32_t reg)
{
    return gic_distributor[reg / 4];
}

static inline void gicd_write(uint32_t reg, uint32_t value)
{
    gic_distributor[reg / 4] = value;
}

static inline uint32_t gicc_read(uint32_t reg)
{
    return gic_cpu_interface[reg / 4];
}

static inline void gicc_write(uint32_t reg, uint32_t value)
{
    gic_cpu_interface[reg / 4] = value;
}

static inline uint32_t gicr_read(uint32_t reg)
{
    return gic_redistributor[reg / 4];
}

static inline void gicr_write(uint32_t reg, uint32_t value)
{
    gic_redistributor[reg / 4] = value;
}

// ============================================================================
// GIC INITIALIZATION
// ============================================================================

void aarch64_gic_init(void)
{
    printf("Initializing aarch64 GIC...\n");

    // For now, use fixed addresses (should be discovered from devicetree)
    // These are typical addresses for QEMU and many ARM boards
    gic_distributor = (volatile uint32_t *)0x08000000;
    gic_cpu_interface = (volatile uint32_t *)0x08010000;
    gic_redistributor = (volatile uint32_t *)0x080A0000;

    // Check if GIC is accessible
    if (!gic_distributor || !gic_cpu_interface)
    {
        printf("ERROR: GIC not accessible at expected addresses\n");
        return;
    }

    // Read GIC type
    uint32_t gic_type = gicd_read(GICD_TYPER);
    uint32_t gic_version = (gic_type >> 19) & 0x3F;
    uint32_t cpu_count = ((gic_type >> 5) & 0x7) + 1;

    printf("GIC Version: %u, CPU Count: %u\n", gic_version, cpu_count);

    // Initialize GIC Distributor
    // Disable all interrupts
    for (int i = 0; i < 32; i++)
    {
        gicd_write(GICD_ICENABLER + i * 4, 0xFFFFFFFF);
        gicd_write(GICD_ICPENDR + i * 4, 0xFFFFFFFF);
    }

    // Set all interrupts to group 0 (secure)
    for (int i = 0; i < 32; i++)
    {
        gicd_write(GICD_IGROUPR + i * 4, 0x00000000);
    }

    // Set all interrupts to lowest priority
    for (int i = 0; i < 256; i++)
    {
        gicd_write(GICD_IPRIORITYR + i * 4, 0x80808080);
    }

    // Set all interrupts to target CPU 0
    for (int i = 0; i < 64; i++)
    {
        gicd_write(GICD_ITARGETSR + i * 4, 0x01010101);
    }

    // Enable GIC Distributor
    gicd_write(GICD_CTLR, 0x01);

    // Initialize GIC CPU Interface
    // Set priority mask to allow all priorities
    gicc_write(GICC_PMR, 0xFF);

    // Set binary point register
    gicc_write(GICC_BPR, 0x00);

    // Enable GIC CPU Interface
    gicc_write(GICC_CTLR, 0x01);

    // Initialize GIC Redistributor
    if (gic_redistributor)
    {
        // Wake up redistributor
        gicr_write(GICR_WAKER, 0x00000000);

        // Wait for wake up
        while (gicr_read(GICR_WAKER) & 0x00000004)
        {
            // Wait for processor sleep bit to clear
        }

        // Enable redistributor
        gicr_write(GICR_CTLR, 0x01);
    }

    printf("GIC initialized successfully\n");
}

// ============================================================================
// INTERRUPT MANAGEMENT
// ============================================================================

void aarch64_interrupt_enable(uint32_t irq)
{
    if (irq >= 1024)
        return;

    uint32_t word = irq / 32;
    uint32_t bit = irq % 32;

    // Enable interrupt in distributor
    gicd_write(GICD_ISENABLER + word * 4, 1 << bit);

    // Mark as enabled locally
    interrupt_enabled[word] |= (1 << bit);

    printf("Enabled IRQ %u\n", irq);
}

void aarch64_interrupt_disable(uint32_t irq)
{
    if (irq >= 1024)
        return;

    uint32_t word = irq / 32;
    uint32_t bit = irq % 32;

    // Disable interrupt in distributor
    gicd_write(GICD_ICENABLER + word * 4, 1 << bit);

    // Mark as disabled locally
    interrupt_enabled[word] &= ~(1 << bit);

    printf("Disabled IRQ %u\n", irq);
}

void aarch64_interrupt_set_handler(uint32_t irq, void (*handler)(void))
{
    if (irq >= 1024)
        return;

    interrupt_handlers[irq] = handler;
    printf("Set handler for IRQ %u\n", irq);
}

void aarch64_interrupt_set_priority(uint32_t irq, uint8_t priority)
{
    if (irq >= 1024)
        return;

    uint32_t reg_offset = GICD_IPRIORITYR + (irq / 4) * 4;
    uint32_t shift = (irq % 4) * 8;
    uint32_t mask = 0xFF << shift;

    uint32_t current = gicd_read(reg_offset);
    current = (current & ~mask) | (priority << shift);
    gicd_write(reg_offset, current);

    interrupt_priorities[irq] = priority;
    printf("Set priority %u for IRQ %u\n", priority, irq);
}

// ============================================================================
// INTERRUPT HANDLING
// ============================================================================

uint32_t aarch64_interrupt_acknowledge(void)
{
    uint32_t irq = gicc_read(GICC_IAR);
    return irq & 0x3FF; // Clear CPU ID bits
}

void aarch64_interrupt_end(uint32_t irq)
{
    gicc_write(GICC_EOIR, irq);
}

void aarch64_interrupt_priority_drop(void)
{
    gicc_write(GICC_EOIR, 0);
}

// ============================================================================
// SYSTEM CALL HANDLING
// ============================================================================

void aarch64_syscall_set_handler(void (*handler)(void))
{
    syscall_handler = handler;
    printf("System call handler set\n");
}

void aarch64_syscall_trigger(void)
{
    if (syscall_handler)
    {
        syscall_handler();
    }
    else
    {
        printf("WARNING: No system call handler registered\n");
    }
}

// ============================================================================
// INTERRUPT ROUTING
// ============================================================================

void aarch64_interrupt_route_to_cpu(uint32_t irq, uint32_t cpu)
{
    if (irq >= 1024 || cpu >= 8)
        return;

    uint32_t reg_offset = GICD_ITARGETSR + (irq / 4) * 4;
    uint32_t shift = (irq % 4) * 8;
    uint32_t mask = 0xFF << shift;
    uint32_t cpu_mask = 1 << cpu;

    uint32_t current = gicd_read(reg_offset);
    current = (current & ~mask) | (cpu_mask << shift);
    gicd_write(reg_offset, current);

    printf("Routed IRQ %u to CPU %u\n", irq, cpu);
}

void aarch64_interrupt_set_type(uint32_t irq, bool is_level_triggered)
{
    if (irq >= 1024)
        return;

    uint32_t reg_offset = GICD_ICFGR + (irq / 16) * 4;
    uint32_t shift = (irq % 16) * 2;
    uint32_t mask = 0x3 << shift;
    uint32_t value = is_level_triggered ? 0x2 : 0x0; // 10 = level, 00 = edge

    uint32_t current = gicd_read(reg_offset);
    current = (current & ~mask) | (value << shift);
    gicd_write(reg_offset, current);

    printf("Set IRQ %u to %s triggered\n", irq, is_level_triggered ? "level" : "edge");
}

// ============================================================================
// INTERRUPT STATISTICS
// ============================================================================

uint32_t aarch64_interrupt_get_count(void)
{
    uint32_t count = 0;
    for (int i = 0; i < 32; i++)
    {
        count += __builtin_popcount(interrupt_enabled[i]);
    }
    return count;
}

bool aarch64_interrupt_is_enabled(uint32_t irq)
{
    if (irq >= 1024)
        return false;

    uint32_t word = irq / 32;
    uint32_t bit = irq % 32;

    return (interrupt_enabled[word] & (1 << bit)) != 0;
}

uint8_t aarch64_interrupt_get_priority(uint32_t irq)
{
    if (irq >= 1024)
        return 0;

    return interrupt_priorities[irq];
}

// ============================================================================
// INTERRUPT DEBUGGING
// ============================================================================

void aarch64_interrupt_print_status(void)
{
    printf("=== aarch64 Interrupt Status ===\n");
    printf("GIC Distributor: 0x%p\n", (void *)gic_distributor);
    printf("GIC CPU Interface: 0x%p\n", (void *)gic_cpu_interface);
    printf("GIC Redistributor: 0x%p\n", (void *)gic_redistributor);
    printf("Enabled Interrupts: %u\n", aarch64_interrupt_get_count());

    // Print first 32 enabled interrupts
    printf("First 32 enabled IRQs: ");
    for (int i = 0; i < 32; i++)
    {
        if (aarch64_interrupt_is_enabled(i))
        {
            printf("%d ", i);
        }
    }
    printf("\n");
}

void aarch64_interrupt_test(void)
{
    printf("Testing aarch64 interrupt system...\n");

    // Test interrupt enable/disable
    aarch64_interrupt_enable(1);
    aarch64_interrupt_enable(2);
    aarch64_interrupt_enable(3);

    TEST_ASSERT(aarch64_interrupt_is_enabled(1), "IRQ 1 enabled");
    TEST_ASSERT(aarch64_interrupt_is_enabled(2), "IRQ 2 enabled");
    TEST_ASSERT(aarch64_interrupt_is_enabled(3), "IRQ 3 enabled");

    // Test priority setting
    aarch64_interrupt_set_priority(1, 0x40);
    aarch64_interrupt_set_priority(2, 0x80);
    aarch64_interrupt_set_priority(3, 0xC0);

    TEST_ASSERT(aarch64_interrupt_get_priority(1) == 0x40, "IRQ 1 priority");
    TEST_ASSERT(aarch64_interrupt_get_priority(2) == 0x80, "IRQ 2 priority");
    TEST_ASSERT(aarch64_interrupt_get_priority(3) == 0xC0, "IRQ 3 priority");

    // Test interrupt disable
    aarch64_interrupt_disable(2);
    TEST_ASSERT(!aarch64_interrupt_is_enabled(2), "IRQ 2 disabled");

    printf("Interrupt system test completed successfully\n");
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void aarch64_interrupts_init(void)
{
    printf("Initializing aarch64 interrupt system...\n");

    // Initialize GIC
    aarch64_gic_init();

    // Clear interrupt handler table
    memset(interrupt_handlers, 0, sizeof(interrupt_handlers));

    // Clear interrupt priorities
    memset(interrupt_priorities, 0x80, sizeof(interrupt_priorities)); // Default priority

    // Clear interrupt enabled state
    memset(interrupt_enabled, 0, sizeof(interrupt_enabled));

    // Set up default handlers for critical interrupts
    // These will be replaced by proper handlers later

    printf("aarch64 interrupt system initialized successfully\n");
}
