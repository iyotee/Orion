/*
 * ORION OS - LoongArch Interrupt Handlers
 *
 * This file implements the C-level interrupt and exception handlers for LoongArch.
 * It provides the interface between the assembly exception vectors and the kernel
 * interrupt management system.
 */

#include "arch.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ============================================================================
 * Interrupt Handler Information Structure
 * ============================================================================ */

typedef struct
{
    void (*handler)(void);
    void *data;
    uint8_t priority;
    bool active;
    char name[32];
} interrupt_handler_info_t;

/* Global interrupt handlers table */
static interrupt_handler_info_t g_interrupt_handlers[LOONGARCH_MAX_IRQS];

/* Exception handler function pointer */
static void (*g_exception_handler)(uint32_t exc_type, struct loongarch_interrupt_context *ctx);

/* System call handler function pointer */
static void (*g_syscall_handler)(struct loongarch_interrupt_context *ctx);

/* ============================================================================
 * Default Interrupt Handlers
 * ============================================================================ */

void loongarch_default_irq_handler(void)
{
    printf("LoongArch: Unhandled IRQ received\n");
}

void loongarch_default_fiq_handler(void)
{
    printf("LoongArch: Unhandled FIQ received\n");
}

void loongarch_default_syscall_handler(struct loongarch_interrupt_context *ctx)
{
    printf("LoongArch: Unhandled system call received\n");
    if (ctx)
    {
        printf("LoongArch: System call number: %lu\n", ctx->a7);
    }
}

void loongarch_default_exception_handler(uint32_t exc_type, struct loongarch_interrupt_context *ctx)
{
    printf("LoongArch: Unhandled exception type %d\n", exc_type);
    if (ctx)
    {
        printf("LoongArch: Exception context available\n");
    }
}

/* ============================================================================
 * Device-Specific Interrupt Handlers
 * ============================================================================ */

void loongarch_timer_interrupt_handler(void)
{
    printf("LoongArch: Timer interrupt received\n");

    /* Acknowledge the timer interrupt */
    loongarch_interrupt_acknowledge(LOONGARCH_TIMER_IRQ);

    /* Update timer statistics */
    /* This would typically involve updating the kernel's timekeeping */

    /* End the interrupt */
    loongarch_interrupt_end(LOONGARCH_TIMER_IRQ);
}

void loongarch_uart_interrupt_handler(void)
{
    printf("LoongArch: UART interrupt received\n");

    /* Handle UART interrupt */
    /* This would typically involve reading/writing UART data */

    /* Acknowledge and end the interrupt */
    loongarch_interrupt_acknowledge(1); /* Assuming UART is IRQ 1 */
    loongarch_interrupt_end(1);
}

void loongarch_disk_interrupt_handler(void)
{
    printf("LoongArch: Disk interrupt received\n");

    /* Handle disk interrupt */
    /* This would typically involve processing I/O completion */

    /* Acknowledge and end the interrupt */
    loongarch_interrupt_acknowledge(2); /* Assuming disk is IRQ 2 */
    loongarch_interrupt_end(2);
}

void loongarch_network_interrupt_handler(void)
{
    printf("LoongArch: Network interrupt received\n");

    /* Handle network interrupt */
    /* This would typically involve processing network packets */

    /* Acknowledge and end the interrupt */
    loongarch_interrupt_acknowledge(3); /* Assuming network is IRQ 3 */
    loongarch_interrupt_end(3);
}

void loongarch_gpu_interrupt_handler(void)
{
    printf("LoongArch: GPU interrupt received\n");

    /* Handle GPU interrupt */
    /* This would typically involve processing graphics commands */

    /* Acknowledge and end the interrupt */
    loongarch_interrupt_acknowledge(4); /* Assuming GPU is IRQ 4 */
    loongarch_interrupt_end(4);
}

/* ============================================================================
 * Interrupt Handler Management
 * ============================================================================ */

int loongarch_interrupt_handlers_init(void)
{
    printf("LoongArch: Initializing interrupt handlers...\n");

    /* Clear all interrupt handlers */
    memset(g_interrupt_handlers, 0, sizeof(g_interrupt_handlers));

    /* Set default handlers */
    for (int i = 0; i < LOONGARCH_MAX_IRQS; i++)
    {
        g_interrupt_handlers[i].handler = loongarch_default_irq_handler;
        g_interrupt_handlers[i].priority = 0x40; /* Medium priority */
        g_interrupt_handlers[i].active = false;
        snprintf(g_interrupt_handlers[i].name, sizeof(g_interrupt_handlers[i].name), "irq_%d", i);
    }

    /* Set default exception handler */
    g_exception_handler = loongarch_default_exception_handler;

    /* Set default system call handler */
    g_syscall_handler = loongarch_default_syscall_handler;

    /* Register common device handlers */
    loongarch_register_timer_handler();
    loongarch_register_uart_handler();
    loongarch_register_disk_handler();
    loongarch_register_network_handler();
    loongarch_register_gpu_handler();

    printf("LoongArch: Interrupt handlers initialized\n");
    return 0;
}

int loongarch_register_interrupt_handler(uint32_t irq, void (*handler)(void), void *data, uint8_t priority, const char *name)
{
    if (irq >= LOONGARCH_MAX_IRQS || !handler)
    {
        return -1;
    }

    printf("LoongArch: Registering handler for IRQ %d\n", irq);

    g_interrupt_handlers[irq].handler = handler;
    g_interrupt_handlers[irq].data = data;
    g_interrupt_handlers[irq].priority = priority;
    g_interrupt_handlers[irq].active = true;

    if (name)
    {
        strncpy(g_interrupt_handlers[irq].name, name, sizeof(g_interrupt_handlers[irq].name) - 1);
        g_interrupt_handlers[irq].name[sizeof(g_interrupt_handlers[irq].name) - 1] = '\0';
    }

    return 0;
}

int loongarch_unregister_interrupt_handler(uint32_t irq)
{
    if (irq >= LOONGARCH_MAX_IRQS)
    {
        return -1;
    }

    printf("LoongArch: Unregistering handler for IRQ %d\n", irq);

    g_interrupt_handlers[irq].handler = loongarch_default_irq_handler;
    g_interrupt_handlers[irq].data = NULL;
    g_interrupt_handlers[irq].priority = 0x40;
    g_interrupt_handlers[irq].active = false;
    snprintf(g_interrupt_handlers[irq].name, sizeof(g_interrupt_handlers[irq].name), "irq_%d", irq);

    return 0;
}

int loongarch_set_interrupt_priority(uint32_t irq, uint8_t priority)
{
    if (irq >= LOONGARCH_MAX_IRQS)
    {
        return -1;
    }

    printf("LoongArch: Setting priority %d for IRQ %d\n", priority, irq);

    g_interrupt_handlers[irq].priority = priority;

    /* Update the hardware priority if supported */
    return loongarch_interrupt_set_priority(irq, priority);
}

uint8_t loongarch_get_interrupt_priority(uint32_t irq)
{
    if (irq >= LOONGARCH_MAX_IRQS)
    {
        return 0;
    }

    return g_interrupt_handlers[irq].priority;
}

/* ============================================================================
 * Device Handler Registration
 * ============================================================================ */

int loongarch_register_timer_handler(void)
{
    return loongarch_register_interrupt_handler(
        LOONGARCH_TIMER_IRQ,
        loongarch_timer_interrupt_handler,
        NULL,
        0x80, /* High priority */
        "timer");
}

int loongarch_register_uart_handler(void)
{
    return loongarch_register_interrupt_handler(
        1, /* UART IRQ */
        loongarch_uart_interrupt_handler,
        NULL,
        0x60, /* Medium-high priority */
        "uart");
}

int loongarch_register_disk_handler(void)
{
    return loongarch_register_interrupt_handler(
        2, /* Disk IRQ */
        loongarch_disk_interrupt_handler,
        NULL,
        0x50, /* Medium priority */
        "disk");
}

int loongarch_register_network_handler(void)
{
    return loongarch_register_interrupt_handler(
        3, /* Network IRQ */
        loongarch_network_interrupt_handler,
        NULL,
        0x50, /* Medium priority */
        "network");
}

int loongarch_register_gpu_handler(void)
{
    return loongarch_register_interrupt_handler(
        4, /* GPU IRQ */
        loongarch_gpu_interrupt_handler,
        NULL,
        0x40, /* Medium priority */
        "gpu");
}

/* ============================================================================
 * Main Interrupt Handler
 * ============================================================================ */

void loongarch_irq_handler(struct loongarch_interrupt_context *ctx)
{
    if (!ctx)
    {
        printf("LoongArch: Invalid interrupt context\n");
        return;
    }

    /* Extract interrupt number from ESTAT CSR */
    uint32_t irq = (ctx->csr_estat >> LOONGARCH_IRQ_OFFSET) & LOONGARCH_IRQ_MASK;

    if (irq >= LOONGARCH_MAX_IRQS)
    {
        printf("LoongArch: Invalid IRQ number %d\n", irq);
        return;
    }

    /* Call the registered handler */
    if (g_interrupt_handlers[irq].handler)
    {
        g_interrupt_handlers[irq].handler();
    }
    else
    {
        printf("LoongArch: No handler registered for IRQ %d\n", irq);
        loongarch_default_irq_handler();
    }
}

/* ============================================================================
 * Exception Handler
 * ============================================================================ */

void loongarch_exception_handler(uint32_t exc_type, struct loongarch_interrupt_context *ctx)
{
    if (!ctx)
    {
        printf("LoongArch: Invalid exception context\n");
        return;
    }

    printf("LoongArch: Exception type %d received\n", exc_type);

    /* Call the registered exception handler */
    if (g_exception_handler)
    {
        g_exception_handler(exc_type, ctx);
    }
    else
    {
        loongarch_default_exception_handler(exc_type, ctx);
    }
}

int loongarch_register_exception_handler(void (*handler)(uint32_t exc_type, struct loongarch_interrupt_context *ctx))
{
    if (!handler)
    {
        return -1;
    }

    printf("LoongArch: Registering exception handler\n");
    g_exception_handler = handler;
    return 0;
}

/* ============================================================================
 * System Call Handler
 * ============================================================================ */

void loongarch_syscall_handler_entry(struct loongarch_interrupt_context *ctx)
{
    if (!ctx)
    {
        printf("LoongArch: Invalid system call context\n");
        return;
    }

    printf("LoongArch: System call %lu received\n", ctx->a7);

    /* Call the registered system call handler */
    if (g_syscall_handler)
    {
        g_syscall_handler(ctx);
    }
    else
    {
        loongarch_default_syscall_handler(ctx);
    }
}

int loongarch_register_syscall_handler(void (*handler)(struct loongarch_interrupt_context *ctx))
{
    if (!handler)
    {
        return -1;
    }

    printf("LoongArch: Registering system call handler\n");
    g_syscall_handler = handler;
    return 0;
}

/* ============================================================================
 * Interrupt Statistics and Debugging
 * ============================================================================ */

void loongarch_interrupt_print_handlers(void)
{
    printf("\n=== LoongArch Interrupt Handlers ===\n");
    for (int i = 0; i < LOONGARCH_MAX_IRQS; i++)
    {
        if (g_interrupt_handlers[i].active)
        {
            printf("IRQ %d: %s (priority: %d, active: %s)\n",
                   i,
                   g_interrupt_handlers[i].name,
                   g_interrupt_handlers[i].priority,
                   g_interrupt_handlers[i].active ? "yes" : "no");
        }
    }
    printf("====================================\n");
}

void loongarch_interrupt_test_handlers(void)
{
    printf("LoongArch: Testing interrupt handlers...\n");

    /* Test handler registration */
    if (loongarch_register_interrupt_handler(5, loongarch_default_irq_handler, NULL, 0x30, "test") != 0)
    {
        printf("LoongArch: Failed to register test handler\n");
        return;
    }

    /* Test priority setting */
    if (loongarch_set_interrupt_priority(5, 0x90) != 0)
    {
        printf("LoongArch: Failed to set test handler priority\n");
        return;
    }

    if (loongarch_get_interrupt_priority(5) != 0x90)
    {
        printf("LoongArch: Priority setting test failed\n");
        return;
    }

    /* Test handler unregistration */
    if (loongarch_unregister_interrupt_handler(5) != 0)
    {
        printf("LoongArch: Failed to unregister test handler\n");
        return;
    }

    printf("LoongArch: Interrupt handler tests passed\n");
}

/* ============================================================================
 * Cleanup Functions
 * ============================================================================ */

void loongarch_interrupt_handlers_cleanup(void)
{
    printf("LoongArch: Cleaning up interrupt handlers...\n");

    /* Unregister all custom handlers */
    for (int i = 0; i < LOONGARCH_MAX_IRQS; i++)
    {
        if (g_interrupt_handlers[i].active && g_interrupt_handlers[i].handler != loongarch_default_irq_handler)
        {
            loongarch_unregister_interrupt_handler(i);
        }
    }

    /* Reset default handlers */
    g_exception_handler = loongarch_default_exception_handler;
    g_syscall_handler = loongarch_default_syscall_handler;

    printf("LoongArch: Interrupt handlers cleaned up\n");
}
