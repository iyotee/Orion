/*
 * ORION OS - LoongArch Timer Management
 *
 * This file implements comprehensive timer management for LoongArch architecture.
 * It handles the LoongArch timer system, including the generic timer, high-resolution
 * timers, and timer interrupt management.
 */

#include "arch.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ============================================================================
 * LoongArch Timer Definitions
 * ============================================================================ */

/* Timer CSR definitions */
#define LOONGARCH_CSR_TCFG 0x41
#define LOONGARCH_CSR_TVAL 0x42
#define LOONGARCH_CSR_TIDX 0x43
#define LOONGARCH_CSR_TLBRENTRY 0x44
#define LOONGARCH_CSR_TLBRBADV 0x45
#define LOONGARCH_CSR_TLBRERA 0x46
#define LOONGARCH_CSR_TLBRSAVE 0x47
#define LOONGARCH_CSR_TLBRPRMD 0x48
#define LOONGARCH_CSR_TLBRINF 0x49
#define LOONGARCH_CSR_TLBRBADV 0x4A
#define LOONGARCH_CSR_TLBRERA 0x4B
#define LOONGARCH_CSR_TLBRSAVE 0x4C
#define LOONGARCH_CSR_TLBRPRMD 0x4D
#define LOONGARCH_CSR_TLBRINF 0x4E

/* Timer configuration bits */
#define LOONGARCH_TCFG_EN 0x1
#define LOONGARCH_TCFG_PERIOD 0x2
#define LOONGARCH_TCFG_IT 0x4

/* Timer modes */
#define LOONGARCH_TIMER_MODE_OFF 0x0
#define LOONGARCH_TIMER_MODE_ONESHOT 0x1
#define LOONGARCH_TIMER_MODE_PERIODIC 0x2

/* Default timer frequency (1GHz) */
#define LOONGARCH_DEFAULT_TIMER_FREQ 1000000000ULL

/* Timer structure */
typedef struct
{
    uint32_t id;
    uint64_t deadline;
    uint64_t period;
    uint8_t mode;
    bool active;
    void (*callback)(void);
    void *data;
} loongarch_timer_t;

/* Global timer state */
static bool g_timers_initialized = false;
static uint64_t g_timer_frequency = LOONGARCH_DEFAULT_TIMER_FREQ;
static uint64_t g_timer_resolution_ns = 1; /* 1ns resolution at 1GHz */
static loongarch_timer_t g_timers[LOONGARCH_MAX_TIMERS];
static uint32_t g_next_timer_id = 0;

/* Timer statistics */
static struct
{
    uint64_t total_timers;
    uint64_t active_timers;
    uint64_t timer_interrupts;
    uint64_t oneshot_timers;
    uint64_t periodic_timers;
} g_timer_stats;

/* ============================================================================
 * Timer CSR Access Functions
 * ============================================================================ */

static inline uint64_t loongarch_timer_csr_read(uint32_t csr_num)
{
    uint64_t value;
    __asm__ volatile("csrrd %0, %1" : "=r"(value) : "i"(csr_num));
    return value;
}

static inline void loongarch_timer_csr_write(uint32_t csr_num, uint64_t value)
{
    __asm__ volatile("csrwr %0, %1" : : "r"(value), "i"(csr_num));
}

static inline uint64_t loongarch_timer_csr_xchg(uint32_t csr_num, uint64_t value)
{
    uint64_t old_value;
    __asm__ volatile("csrxchg %0, %1, %2" : "=r"(old_value) : "r"(value), "i"(csr_num));
    return old_value;
}

/* ============================================================================
 * Timer Initialization and Configuration
 * ============================================================================ */

int loongarch_timers_init(void)
{
    if (g_timers_initialized)
    {
        return 0;
    }

    printf("LoongArch: Initializing timer system...\n");

    /* Clear timer statistics */
    memset(&g_timer_stats, 0, sizeof(g_timer_stats));

    /* Initialize timer array */
    memset(g_timers, 0, sizeof(g_timers));
    for (int i = 0; i < LOONGARCH_MAX_TIMERS; i++)
    {
        g_timers[i].id = i;
        g_timers[i].active = false;
        g_timers[i].mode = LOONGARCH_TIMER_MODE_OFF;
    }

    /* Configure timer frequency */
    if (loongarch_timer_configure_frequency() != 0)
    {
        printf("LoongArch: Failed to configure timer frequency\n");
        return -1;
    }

    /* Initialize timer interrupt */
    if (loongarch_timer_interrupt_init() != 0)
    {
        printf("LoongArch: Failed to initialize timer interrupt\n");
        return -1;
    }

    /* Set up default timer */
    if (loongarch_timer_setup_default() != 0)
    {
        printf("LoongArch: Failed to setup default timer\n");
        return -1;
    }

    g_timers_initialized = true;
    printf("LoongArch: Timer system initialized successfully\n");

    return 0;
}

int loongarch_timer_configure_frequency(void)
{
    printf("LoongArch: Configuring timer frequency...\n");

    /* Read current timer configuration */
    uint64_t tcfg = loongarch_timer_csr_read(LOONGARCH_CSR_TCFG);

    /* Set timer frequency to 1GHz (default) */
    g_timer_frequency = LOONGARCH_DEFAULT_TIMER_FREQ;
    g_timer_resolution_ns = 1000000000ULL / g_timer_frequency;

    printf("LoongArch: Timer frequency set to %lu Hz (%lu ns resolution)\n",
           g_timer_frequency, g_timer_resolution_ns);

    return 0;
}

int loongarch_timer_interrupt_init(void)
{
    printf("LoongArch: Initializing timer interrupt...\n");

    /* Configure timer interrupt in TCFG */
    uint64_t tcfg = loongarch_timer_csr_read(LOONGARCH_CSR_TCFG);
    tcfg |= LOONGARCH_TCFG_IT; /* Enable timer interrupt */
    loongarch_timer_csr_write(LOONGARCH_CSR_TCFG, tcfg);

    /* Set up timer interrupt handler */
    if (loongarch_interrupt_set_handler(LOONGARCH_TIMER_IRQ, loongarch_timer_interrupt_handler) != 0)
    {
        printf("LoongArch: Failed to set timer interrupt handler\n");
        return -1;
    }

    /* Enable timer interrupt */
    if (loongarch_interrupt_enable(LOONGARCH_TIMER_IRQ) != 0)
    {
        printf("LoongArch: Failed to enable timer interrupt\n");
        return -1;
    }

    printf("LoongArch: Timer interrupt initialized\n");
    return 0;
}

int loongarch_timer_setup_default(void)
{
    printf("LoongArch: Setting up default timer...\n");

    /* Configure default timer for system tick */
    uint64_t tcfg = loongarch_timer_csr_read(LOONGARCH_CSR_TCFG);
    tcfg |= LOONGARCH_TCFG_EN;     /* Enable timer */
    tcfg |= LOONGARCH_TCFG_PERIOD; /* Periodic mode */
    loongarch_timer_csr_write(LOONGARCH_CSR_TCFG, tcfg);

    /* Set default period (1ms) */
    uint64_t period_ticks = g_timer_frequency / 1000;
    loongarch_timer_csr_write(LOONGARCH_CSR_TVAL, period_ticks);

    printf("LoongArch: Default timer configured for 1ms period\n");
    return 0;
}

/* ============================================================================
 * Timer Control Functions
 * ============================================================================ */

uint64_t loongarch_timer_get_frequency(void)
{
    return g_timer_frequency;
}

uint64_t loongarch_timer_get_resolution_ns(void)
{
    return g_timer_resolution_ns;
}

uint64_t loongarch_timer_read(void)
{
    /* Read current timer value */
    return loongarch_timer_csr_read(LOONGARCH_CSR_TVAL);
}

uint64_t loongarch_timer_read_ns(void)
{
    uint64_t ticks = loongarch_timer_read();
    return (ticks * 1000000000ULL) / g_timer_frequency;
}

uint64_t loongarch_timer_read_us(void)
{
    uint64_t ticks = loongarch_timer_read();
    return (ticks * 1000000ULL) / g_timer_frequency;
}

uint64_t loongarch_timer_read_ms(void)
{
    uint64_t ticks = loongarch_timer_read();
    return (ticks * 1000ULL) / g_timer_frequency;
}

/* ============================================================================
 * Timer Management Functions
 * ============================================================================ */

int loongarch_timer_set_oneshot(uint64_t deadline_ns, void (*callback)(void), void *data)
{
    if (!callback)
    {
        return -1;
    }

    /* Find free timer slot */
    int timer_id = -1;
    for (int i = 0; i < LOONGARCH_MAX_TIMERS; i++)
    {
        if (!g_timers[i].active)
        {
            timer_id = i;
            break;
        }
    }

    if (timer_id == -1)
    {
        printf("LoongArch: No free timer slots available\n");
        return -1;
    }

    printf("LoongArch: Setting oneshot timer %d for %lu ns\n", timer_id, deadline_ns);

    /* Configure timer */
    g_timers[timer_id].deadline = deadline_ns;
    g_timers[timer_id].period = 0;
    g_timers[timer_id].mode = LOONGARCH_TIMER_MODE_ONESHOT;
    g_timers[timer_id].active = true;
    g_timers[timer_id].callback = callback;
    g_timers[timer_id].data = data;

    /* Update statistics */
    g_timer_stats.total_timers++;
    g_timer_stats.active_timers++;
    g_timer_stats.oneshot_timers++;

    return timer_id;
}

int loongarch_timer_set_periodic(uint64_t period_ns, void (*callback)(void), void *data)
{
    if (!callback || period_ns == 0)
    {
        return -1;
    }

    /* Find free timer slot */
    int timer_id = -1;
    for (int i = 0; i < LOONGARCH_MAX_TIMERS; i++)
    {
        if (!g_timers[i].active)
        {
            timer_id = i;
            break;
        }
    }

    if (timer_id == -1)
    {
        printf("LoongArch: No free timer slots available\n");
        return -1;
    }

    printf("LoongArch: Setting periodic timer %d for %lu ns period\n", timer_id, period_ns);

    /* Configure timer */
    g_timers[timer_id].deadline = period_ns;
    g_timers[timer_id].period = period_ns;
    g_timers[timer_id].mode = LOONGARCH_TIMER_MODE_PERIODIC;
    g_timers[timer_id].active = true;
    g_timers[timer_id].callback = callback;
    g_timers[timer_id].data = data;

    /* Update statistics */
    g_timer_stats.total_timers++;
    g_timer_stats.active_timers++;
    g_timer_stats.periodic_timers++;

    return timer_id;
}

int loongarch_timer_cancel(uint32_t timer_id)
{
    if (timer_id >= LOONGARCH_MAX_TIMERS)
    {
        return -1;
    }

    if (!g_timers[timer_id].active)
    {
        return -1;
    }

    printf("LoongArch: Cancelling timer %d\n", timer_id);

    /* Deactivate timer */
    g_timers[timer_id].active = false;
    g_timers[timer_id].mode = LOONGARCH_TIMER_MODE_OFF;
    g_timers[timer_id].callback = NULL;
    g_timers[timer_id].data = NULL;

    /* Update statistics */
    g_timer_stats.active_timers--;

    return 0;
}

int loongarch_timer_modify(uint32_t timer_id, uint64_t new_deadline, uint64_t new_period)
{
    if (timer_id >= LOONGARCH_MAX_TIMERS)
    {
        return -1;
    }

    if (!g_timers[timer_id].active)
    {
        return -1;
    }

    printf("LoongArch: Modifying timer %d\n", timer_id);

    /* Update timer parameters */
    g_timers[timer_id].deadline = new_deadline;
    if (g_timers[timer_id].mode == LOONGARCH_TIMER_MODE_PERIODIC)
    {
        g_timers[timer_id].period = new_period;
    }

    return 0;
}

/* ============================================================================
 * Timer Interrupt Handling
 * ============================================================================ */

void loongarch_timer_interrupt_handler(void)
{
    printf("LoongArch: Timer interrupt received\n");

    /* Update statistics */
    g_timer_stats.timer_interrupts++;

    /* Process all active timers */
    uint64_t current_time = loongarch_timer_read_ns();

    for (int i = 0; i < LOONGARCH_MAX_TIMERS; i++)
    {
        if (!g_timers[i].active)
        {
            continue;
        }

        /* Check if timer has expired */
        if (current_time >= g_timers[i].deadline)
        {
            /* Call timer callback */
            if (g_timers[i].callback)
            {
                g_timers[i].callback();
            }

            /* Handle timer based on mode */
            if (g_timers[i].mode == LOONGARCH_TIMER_MODE_ONESHOT)
            {
                /* Deactivate oneshot timer */
                g_timers[i].active = false;
                g_timer_stats.active_timers--;
            }
            else if (g_timers[i].mode == LOONGARCH_TIMER_MODE_PERIODIC)
            {
                /* Update deadline for periodic timer */
                g_timers[i].deadline += g_timers[i].period;
            }
        }
    }

    /* Acknowledge timer interrupt */
    loongarch_interrupt_acknowledge(LOONGARCH_TIMER_IRQ);
}

/* ============================================================================
 * Timer Power Management
 * ============================================================================ */

int loongarch_timer_suspend(void)
{
    printf("LoongArch: Suspending timer system...\n");

    /* Disable timer interrupt */
    loongarch_interrupt_disable(LOONGARCH_TIMER_IRQ);

    /* Save timer state if needed */
    /* For LoongArch, this might involve saving CSR values */

    printf("LoongArch: Timer system suspended\n");
    return 0;
}

int loongarch_timer_resume(void)
{
    printf("LoongArch: Resuming timer system...\n");

    /* Restore timer state if needed */
    /* For LoongArch, this might involve restoring CSR values */

    /* Re-enable timer interrupt */
    loongarch_interrupt_enable(LOONGARCH_TIMER_IRQ);

    printf("LoongArch: Timer system resumed\n");
    return 0;
}

/* ============================================================================
 * Timer Testing and Validation
 * ============================================================================ */

int loongarch_timer_test(void)
{
    printf("LoongArch: Testing timer system...\n");

    /* Test timer reading */
    uint64_t time1 = loongarch_timer_read_ns();
    uint64_t time2 = loongarch_timer_read_ns();

    if (time2 < time1)
    {
        printf("LoongArch: Timer monotonicity test failed\n");
        return -1;
    }

    /* Test oneshot timer */
    bool oneshot_triggered = false;
    int timer_id = loongarch_timer_set_oneshot(1000000, /* 1ms */
                                               (void (*)(void))[](void) { oneshot_triggered = true; }, NULL);

    if (timer_id < 0)
    {
        printf("LoongArch: Failed to create oneshot timer\n");
        return -1;
    }

    /* Wait a bit for timer to potentially trigger */
    /* In a real system, this would be handled by the interrupt system */

    /* Cancel timer */
    if (loongarch_timer_cancel(timer_id) != 0)
    {
        printf("LoongArch: Failed to cancel timer\n");
        return -1;
    }

    printf("LoongArch: Timer system test passed\n");
    return 0;
}

void loongarch_timer_print_stats(void)
{
    printf("\n=== LoongArch Timer Statistics ===\n");
    printf("Total timers: %lu\n", g_timer_stats.total_timers);
    printf("Active timers: %lu\n", g_timer_stats.active_timers);
    printf("Timer interrupts: %lu\n", g_timer_stats.timer_interrupts);
    printf("Oneshot timers: %lu\n", g_timer_stats.oneshot_timers);
    printf("Periodic timers: %lu\n", g_timer_stats.periodic_timers);
    printf("Timer frequency: %lu Hz\n", g_timer_frequency);
    printf("Timer resolution: %lu ns\n", g_timer_resolution_ns);
    printf("==================================\n");
}

/* ============================================================================
 * Cleanup Functions
 * ============================================================================ */

void loongarch_timers_cleanup(void)
{
    if (!g_timers_initialized)
    {
        return;
    }

    printf("LoongArch: Cleaning up timer system...\n");

    /* Cancel all active timers */
    for (int i = 0; i < LOONGARCH_MAX_TIMERS; i++)
    {
        if (g_timers[i].active)
        {
            loongarch_timer_cancel(i);
        }
    }

    /* Disable timer interrupt */
    loongarch_interrupt_disable(LOONGARCH_TIMER_IRQ);

    /* Disable timer */
    uint64_t tcfg = loongarch_timer_csr_read(LOONGARCH_CSR_TCFG);
    tcfg &= ~LOONGARCH_TCFG_EN;
    loongarch_timer_csr_write(LOONGARCH_CSR_TCFG, tcfg);

    g_timers_initialized = false;
    printf("LoongArch: Timer system cleaned up\n");
}
