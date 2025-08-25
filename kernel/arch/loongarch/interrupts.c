/*
 * ORION OS - LoongArch Interrupt Management
 *
 * This file implements comprehensive interrupt management for LoongArch architecture.
 * It handles the LoongArch interrupt controller, exception vectors, and interrupt
 * routing for all supported LoongArch platforms.
 */

#include "arch.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ============================================================================
 * LoongArch Interrupt Controller Definitions
 * ============================================================================ */

/* LoongArch CSR (Control and Status Register) definitions for interrupts */
#define LOONGARCH_CSR_ESTAT 0x5
#define LOONGARCH_CSR_ECFG 0x4
#define LOONGARCH_CSR_TCFG 0x41
#define LOONGARCH_CSR_TVAL 0x42
#define LOONGARCH_CSR_CRMD 0x1
#define LOONGARCH_CSR_PRMD 0x6
#define LOONGARCH_CSR_EUEN 0x10
#define LOONGARCH_CSR_ECFG_VS 0x0
#define LOONGARCH_CSR_ECFG_IM 0x1

/* Interrupt vector offsets */
#define LOONGARCH_IRQ_OFFSET 16
#define LOONGARCH_IRQ_MASK 0xFF
#define LOONGARCH_EXC_OFFSET 0
#define LOONGARCH_EXC_MASK 0x1F

/* Exception types */
#define LOONGARCH_EXC_INT 0x0
#define LOONGARCH_EXC_PIL 0x1
#define LOONGARCH_EXC_PIS 0x2
#define LOONGARCH_EXC_PIF 0x3
#define LOONGARCH_EXC_PME 0x4
#define LOONGARCH_EXC_PNR 0x5
#define LOONGARCH_EXC_PNX 0x6
#define LOONGARCH_EXC_PPI 0x7
#define LOONGARCH_EXC_ADEM 0x8
#define LOONGARCH_EXC_ADEF 0x9
#define LOONGARCH_EXC_ALE 0xA
#define LOONGARCH_EXC_BCE 0xB
#define LOONGARCH_EXC_SYS 0xC
#define LOONGARCH_EXC_BRK 0xD
#define LOONGARCH_EXC_INE 0xE
#define LOONGARCH_EXC_IPE 0xF
#define LOONGARCH_EXC_FPD 0x10
#define LOONGARCH_EXC_SXD 0x11
#define LOONGARCH_EXC_ASXD 0x12
#define LOONGARCH_EXC_FPE 0x13
#define LOONGARCH_EXC_TBR 0x14

/* Timer interrupt */
#define LOONGARCH_TIMER_IRQ 7

/* Global interrupt state */
static bool g_interrupts_initialized = false;
static uint32_t g_interrupt_enabled_mask = 0;
static uint32_t g_interrupt_priority_mask = 0;

/* Interrupt statistics */
static struct
{
    uint64_t total_interrupts;
    uint64_t handled_interrupts;
    uint64_t unhandled_interrupts;
    uint64_t timer_interrupts;
    uint64_t external_interrupts;
} g_interrupt_stats;

/* ============================================================================
 * CSR Access Functions
 * ============================================================================ */

static inline uint64_t loongarch_csr_read(uint32_t csr_num)
{
    uint64_t value;
    __asm__ volatile("csrrd %0, %1" : "=r"(value) : "i"(csr_num));
    return value;
}

static inline void loongarch_csr_write(uint32_t csr_num, uint64_t value)
{
    __asm__ volatile("csrwr %0, %1" : : "r"(value), "i"(csr_num));
}

static inline uint64_t loongarch_csr_xchg(uint32_t csr_num, uint64_t value)
{
    uint64_t old_value;
    __asm__ volatile("csrxchg %0, %1, %2" : "=r"(old_value) : "r"(value), "i"(csr_num));
    return old_value;
}

/* ============================================================================
 * Interrupt Controller Management
 * ============================================================================ */

int loongarch_interrupts_init(void)
{
    if (g_interrupts_initialized)
    {
        return 0;
    }

    printf("LoongArch: Initializing interrupt system...\n");

    /* Clear interrupt statistics */
    memset(&g_interrupt_stats, 0, sizeof(g_interrupt_stats));

    /* Initialize interrupt controller */
    if (loongarch_interrupt_controller_init() != 0)
    {
        printf("LoongArch: Failed to initialize interrupt controller\n");
        return -1;
    }

    /* Set up exception vectors */
    if (loongarch_exception_vectors_init() != 0)
    {
        printf("LoongArch: Failed to initialize exception vectors\n");
        return -1;
    }

    /* Configure default interrupt priorities */
    if (loongarch_interrupt_priorities_init() != 0)
    {
        printf("LoongArch: Failed to initialize interrupt priorities\n");
        return -1;
    }

    /* Enable global interrupts */
    loongarch_interrupts_enable_global();

    g_interrupts_initialized = true;
    printf("LoongArch: Interrupt system initialized successfully\n");

    return 0;
}

int loongarch_interrupt_controller_init(void)
{
    printf("LoongArch: Initializing interrupt controller...\n");

    /* Clear all interrupt enables */
    g_interrupt_enabled_mask = 0;
    g_interrupt_priority_mask = 0;

    /* Configure ECFG (Exception Configuration) */
    uint64_t ecfg = loongarch_csr_read(LOONGARCH_CSR_ECFG);
    ecfg &= ~(LOONGARCH_IRQ_MASK << LOONGARCH_IRQ_OFFSET);
    ecfg |= (0x1 << LOONGARCH_IRQ_OFFSET); /* Enable IRQ 0 */
    loongarch_csr_write(LOONGARCH_CSR_ECFG, ecfg);

    /* Configure ESTAT (Exception Status) */
    uint64_t estat = loongarch_csr_read(LOONGARCH_CSR_ESTAT);
    estat &= ~(LOONGARCH_IRQ_MASK << LOONGARCH_IRQ_OFFSET);
    loongarch_csr_write(LOONGARCH_CSR_ESTAT, estat);

    printf("LoongArch: Interrupt controller initialized\n");
    return 0;
}

int loongarch_exception_vectors_init(void)
{
    printf("LoongArch: Setting up exception vectors...\n");

    /* Exception vectors are set up in boot.S and arch_asm.S */
    /* This function handles any additional runtime configuration */

    printf("LoongArch: Exception vectors initialized\n");
    return 0;
}

int loongarch_interrupt_priorities_init(void)
{
    printf("LoongArch: Initializing interrupt priorities...\n");

    /* Set default priorities for common interrupts */
    loongarch_interrupt_set_priority(LOONGARCH_TIMER_IRQ, 0x80); /* High priority for timer */

    /* Set priorities for external interrupts */
    for (int i = 0; i < 8; i++)
    {
        if (i != LOONGARCH_TIMER_IRQ)
        {
            loongarch_interrupt_set_priority(i, 0x40); /* Medium priority */
        }
    }

    printf("LoongArch: Interrupt priorities initialized\n");
    return 0;
}

/* ============================================================================
 * Interrupt Control Functions
 * ============================================================================ */

void loongarch_interrupts_enable_global(void)
{
    uint64_t crmd = loongarch_csr_read(LOONGARCH_CSR_CRMD);
    crmd |= (1 << 2); /* Enable interrupts (IE bit) */
    loongarch_csr_write(LOONGARCH_CSR_CRMD, crmd);
    printf("LoongArch: Global interrupts enabled\n");
}

void loongarch_interrupts_disable_global(void)
{
    uint64_t crmd = loongarch_csr_read(LOONGARCH_CSR_CRMD);
    crmd &= ~(1 << 2); /* Disable interrupts (IE bit) */
    loongarch_csr_write(LOONGARCH_CSR_CRMD, crmd);
    printf("LoongArch: Global interrupts disabled\n");
}

int loongarch_interrupt_enable(uint32_t irq)
{
    if (irq >= LOONGARCH_MAX_IRQS)
    {
        return -1;
    }

    printf("LoongArch: Enabling interrupt %d\n", irq);

    /* Update local mask */
    g_interrupt_enabled_mask |= (1 << irq);

    /* Update CSR */
    uint64_t ecfg = loongarch_csr_read(LOONGARCH_CSR_ECFG);
    ecfg |= (1 << (irq + LOONGARCH_IRQ_OFFSET));
    loongarch_csr_write(LOONGARCH_CSR_ECFG, ecfg);

    return 0;
}

int loongarch_interrupt_disable(uint32_t irq)
{
    if (irq >= LOONGARCH_MAX_IRQS)
    {
        return -1;
    }

    printf("LoongArch: Disabling interrupt %d\n", irq);

    /* Update local mask */
    g_interrupt_enabled_mask &= ~(1 << irq);

    /* Update CSR */
    uint64_t ecfg = loongarch_csr_read(LOONGARCH_CSR_ECFG);
    ecfg &= ~(1 << (irq + LOONGARCH_IRQ_OFFSET));
    loongarch_csr_write(LOONGARCH_CSR_ECFG, ecfg);

    return 0;
}

bool loongarch_interrupt_is_enabled(uint32_t irq)
{
    if (irq >= LOONGARCH_MAX_IRQS)
    {
        return false;
    }

    return (g_interrupt_enabled_mask & (1 << irq)) != 0;
}

int loongarch_interrupt_set_priority(uint32_t irq, uint8_t priority)
{
    if (irq >= LOONGARCH_MAX_IRQS)
    {
        return -1;
    }

    printf("LoongArch: Setting priority %d for interrupt %d\n", priority, irq);

    /* Update local priority mask */
    g_interrupt_priority_mask = (g_interrupt_priority_mask & ~(0xFF << (irq * 8))) |
                                ((uint32_t)priority << (irq * 8));

    /* For LoongArch, priorities are typically handled by the interrupt controller */
    /* This is a simplified implementation */

    return 0;
}

uint8_t loongarch_interrupt_get_priority(uint32_t irq)
{
    if (irq >= LOONGARCH_MAX_IRQS)
    {
        return 0;
    }

    return (g_interrupt_priority_mask >> (irq * 8)) & 0xFF;
}

/* ============================================================================
 * Interrupt Handling Functions
 * ============================================================================ */

int loongarch_interrupt_acknowledge(uint32_t irq)
{
    if (irq >= LOONGARCH_MAX_IRQS)
    {
        return -1;
    }

    /* Clear the interrupt in ESTAT */
    uint64_t estat = loongarch_csr_read(LOONGARCH_CSR_ESTAT);
    estat &= ~(1 << (irq + LOONGARCH_IRQ_OFFSET));
    loongarch_csr_write(LOONGARCH_CSR_ESTAT, estat);

    /* Update statistics */
    g_interrupt_stats.total_interrupts++;
    g_interrupt_stats.handled_interrupts++;

    return 0;
}

int loongarch_interrupt_end(uint32_t irq)
{
    if (irq >= LOONGARCH_MAX_IRQS)
    {
        return -1;
    }

    /* For LoongArch, this typically involves clearing any pending flags */
    /* and ensuring the interrupt can be triggered again */

    return 0;
}

int loongarch_interrupt_route_to_cpu(uint32_t irq, uint32_t cpu_id)
{
    if (irq >= LOONGARCH_MAX_IRQS)
    {
        return -1;
    }

    printf("LoongArch: Routing interrupt %d to CPU %d\n", irq, cpu_id);

    /* LoongArch interrupt routing is typically handled by the platform */
    /* This is a placeholder for future implementation */

    return 0;
}

/* ============================================================================
 * System Call Handler Registration
 * ============================================================================ */

int loongarch_syscall_handler_register(void (*handler)(struct loongarch_interrupt_context *ctx))
{
    if (!handler)
    {
        return -1;
    }

    printf("LoongArch: Registering system call handler\n");

    /* System call handler is set up in arch_asm.S */
    /* This function can be used for runtime registration if needed */

    return 0;
}

/* ============================================================================
 * Interrupt Testing and Debugging
 * ============================================================================ */

int loongarch_interrupt_test(void)
{
    printf("LoongArch: Testing interrupt system...\n");

    /* Test interrupt enable/disable */
    if (loongarch_interrupt_enable(1) != 0)
    {
        printf("LoongArch: Failed to enable test interrupt\n");
        return -1;
    }

    if (!loongarch_interrupt_is_enabled(1))
    {
        printf("LoongArch: Interrupt enable test failed\n");
        return -1;
    }

    if (loongarch_interrupt_disable(1) != 0)
    {
        printf("LoongArch: Failed to disable test interrupt\n");
        return -1;
    }

    if (loongarch_interrupt_is_enabled(1))
    {
        printf("LoongArch: Interrupt disable test failed\n");
        return -1;
    }

    /* Test priority setting */
    if (loongarch_interrupt_set_priority(1, 0x80) != 0)
    {
        printf("LoongArch: Failed to set interrupt priority\n");
        return -1;
    }

    if (loongarch_interrupt_get_priority(1) != 0x80)
    {
        printf("LoongArch: Interrupt priority test failed\n");
        return -1;
    }

    printf("LoongArch: Interrupt system test passed\n");
    return 0;
}

void loongarch_interrupt_print_stats(void)
{
    printf("\n=== LoongArch Interrupt Statistics ===\n");
    printf("Total interrupts: %lu\n", g_interrupt_stats.total_interrupts);
    printf("Handled interrupts: %lu\n", g_interrupt_stats.handled_interrupts);
    printf("Unhandled interrupts: %lu\n", g_interrupt_stats.unhandled_interrupts);
    printf("Timer interrupts: %lu\n", g_interrupt_stats.timer_interrupts);
    printf("External interrupts: %lu\n", g_interrupt_stats.external_interrupts);
    printf("=====================================\n");
}

/* ============================================================================
 * Cleanup Functions
 * ============================================================================ */

void loongarch_interrupts_cleanup(void)
{
    if (!g_interrupts_initialized)
    {
        return;
    }

    printf("LoongArch: Cleaning up interrupt system...\n");

    /* Disable all interrupts */
    for (int i = 0; i < LOONGARCH_MAX_IRQS; i++)
    {
        if (loongarch_interrupt_is_enabled(i))
        {
            loongarch_interrupt_disable(i);
        }
    }

    /* Disable global interrupts */
    loongarch_interrupts_disable_global();

    g_interrupts_initialized = false;
    printf("LoongArch: Interrupt system cleaned up\n");
}
