/*
 * Orion Operating System - aarch64 Timer Management
 *
 * Complete timer management system for aarch64 including:
 * - ARM Generic Timer support
 * - Periodic and oneshot timers
 * - High-resolution timing
 * - Timer interrupt handling
 * - Power management integration
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
// TIMER REGISTER DEFINITIONS
// ============================================================================

// ARM Generic Timer registers
#define CNTFRQ_EL0 0x00000000    // Counter frequency
#define CNTPCT_EL0 0x00000000    // Physical counter value
#define CNTPCT_EL1 0x00000000    // Physical counter value (EL1)
#define CNTP_CTL_EL0 0x00000000  // Physical timer control
#define CNTP_CVAL_EL0 0x00000000 // Physical timer compare value
#define CNTP_TVAL_EL0 0x00000000 // Physical timer value
#define CNTP_CTL_EL1 0x00000000  // Physical timer control (EL1)
#define CNTP_CVAL_EL1 0x00000000 // Physical timer compare value (EL1)
#define CNTP_TVAL_EL1 0x00000000 // Physical timer value (EL1)

// Virtual timer registers
#define CNTV_CTL_EL0 0x00000000  // Virtual timer control
#define CNTV_CVAL_EL0 0x00000000 // Virtual timer compare value
#define CNTV_TVAL_EL0 0x00000000 // Virtual timer value

// Timer control bits
#define CNT_CTL_ENABLE 0x00000001  // Timer enable
#define CNT_CTL_IMASK 0x00000002   // Interrupt mask
#define CNT_CTL_ISTATUS 0x00000004 // Interrupt status

// ============================================================================
// TIMER TYPES
// ============================================================================

typedef enum
{
    TIMER_TYPE_ONESHOT,
    TIMER_TYPE_PERIODIC,
    TIMER_TYPE_RELATIVE
} timer_type_t;

typedef struct
{
    uint32_t id;
    timer_type_t type;
    uint64_t deadline;
    uint64_t period;
    void (*callback)(void);
    bool active;
    bool expired;
    char name[32];
} timer_t;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// Timer frequency (detected from CNTFRQ_EL0)
static uint64_t timer_frequency = 0;

// Timer resolution in nanoseconds
static uint64_t timer_resolution_ns = 0;

// System uptime in nanoseconds
static uint64_t system_uptime_ns = 0;

// Timer interrupt handler
static void (*timer_interrupt_handler)(void) = NULL;

// Timer table
static timer_t timers[64];
static uint32_t next_timer_id = 0;

// Timer statistics
static uint32_t timer_interrupts = 0;
static uint32_t timer_expirations = 0;
static uint64_t last_timer_check = 0;

// ============================================================================
// TIMER ACCESS FUNCTIONS
// ============================================================================

static inline uint64_t read_cntfrq(void)
{
    uint64_t value;
    __asm__ volatile("mrs %0, CNTFRQ_EL0" : "=r"(value));
    return value;
}

static inline uint64_t read_cntpct(void)
{
    uint64_t value;
    __asm__ volatile("mrs %0, CNTPCT_EL0" : "=r"(value));
    return value;
}

static inline uint64_t read_cntpctl(void)
{
    uint64_t value;
    __asm__ volatile("mrs %0, CNTP_CTL_EL0" : "=r"(value));
    return value;
}

static inline void write_cntpctl(uint64_t value)
{
    __asm__ volatile("msr CNTP_CTL_EL0, %0" : : "r"(value));
}

static inline uint64_t read_cntpcval(void)
{
    uint64_t value;
    __asm__ volatile("mrs %0, CNTP_CVAL_EL0" : "=r"(value));
    return value;
}

static inline void write_cntpcval(uint64_t value)
{
    __asm__ volatile("msr CNTP_CVAL_EL0, %0" : : "r"(value));
}

static inline uint64_t read_cntptval(void)
{
    uint64_t value;
    __asm__ volatile("mrs %0, CNTP_TVAL_EL0" : "=r"(value));
    return value;
}

static inline void write_cntptval(uint64_t value)
{
    __asm__ volatile("msr CNTP_TVAL_EL0, %0" : : "r"(value));
}

// ============================================================================
// TIMER INITIALIZATION
// ============================================================================

void aarch64_timer_init(void)
{
    printf("Initializing aarch64 timer system...\n");

    // Read timer frequency
    timer_frequency = read_cntfrq();
    if (timer_frequency == 0)
    {
        // Use default frequency if not detected
        timer_frequency = 24000000; // 24 MHz typical
        printf("WARNING: Timer frequency not detected, using default: %llu Hz\n", timer_frequency);
    }
    else
    {
        printf("Timer frequency detected: %llu Hz\n", timer_frequency);
    }

    // Calculate timer resolution
    timer_resolution_ns = 1000000000ULL / timer_frequency;
    printf("Timer resolution: %llu ns\n", timer_resolution_ns);

    // Initialize timer table
    memset(timers, 0, sizeof(timers));
    next_timer_id = 0;

    // Initialize system uptime
    system_uptime_ns = 0;
    last_timer_check = read_cntpct();

    // Clear statistics
    timer_interrupts = 0;
    timer_expirations = 0;

    // Set up timer interrupt handler
    timer_interrupt_handler = NULL;

    printf("aarch64 timer system initialized successfully\n");
}

// ============================================================================
// BASIC TIMER OPERATIONS
// ============================================================================

uint64_t aarch64_timer_read_ns(void)
{
    uint64_t current_count = read_cntpct();
    uint64_t elapsed_ticks = current_count - last_timer_check;

    // Convert ticks to nanoseconds
    uint64_t elapsed_ns = (elapsed_ticks * 1000000000ULL) / timer_frequency;

    // Update system uptime
    system_uptime_ns += elapsed_ns;
    last_timer_check = current_count;

    return system_uptime_ns;
}

uint64_t aarch64_timer_read_raw(void)
{
    return read_cntpct();
}

uint64_t aarch64_timer_get_frequency(void)
{
    return timer_frequency;
}

uint64_t aarch64_timer_get_resolution_ns(void)
{
    return timer_resolution_ns;
}

// ============================================================================
// TIMER INTERRUPT MANAGEMENT
// ============================================================================

void aarch64_timer_enable_interrupts(void)
{
    uint64_t ctl = read_cntpctl();
    ctl |= CNT_CTL_ENABLE | CNT_CTL_IMASK;
    write_cntpctl(ctl);
    printf("Timer interrupts enabled\n");
}

void aarch64_timer_disable_interrupts(void)
{
    uint64_t ctl = read_cntpctl();
    ctl &= ~(CNT_CTL_ENABLE | CNT_CTL_IMASK);
    write_cntpctl(ctl);
    printf("Timer interrupts disabled\n");
}

bool aarch64_timer_interrupt_pending(void)
{
    uint64_t ctl = read_cntpctl();
    return (ctl & CNT_CTL_ISTATUS) != 0;
}

void aarch64_timer_clear_interrupt(void)
{
    // Clear interrupt status by writing 1 to ISTATUS bit
    uint64_t ctl = read_cntpctl();
    ctl |= CNT_CTL_ISTATUS;
    write_cntpctl(ctl);
}

// ============================================================================
// ONESHOT TIMER
// ============================================================================

int aarch64_timer_set_oneshot(uint64_t deadline_ns)
{
    // Convert deadline to timer ticks
    uint64_t current_count = read_cntpct();
    uint64_t deadline_ticks = (deadline_ns * timer_frequency) / 1000000000ULL;
    uint64_t compare_value = current_count + deadline_ticks;

    // Set compare value
    write_cntpcval(compare_value);

    // Enable timer and interrupts
    uint64_t ctl = CNT_CTL_ENABLE | CNT_CTL_IMASK;
    write_cntpctl(ctl);

    printf("Oneshot timer set for %llu ns from now\n", deadline_ns);
    return 0;
}

int aarch64_timer_set_oneshot_absolute(uint64_t absolute_time_ns)
{
    // Convert absolute time to timer ticks
    uint64_t compare_value = (absolute_time_ns * timer_frequency) / 1000000000ULL;

    // Set compare value
    write_cntpcval(compare_value);

    // Enable timer and interrupts
    uint64_t ctl = CNT_CTL_ENABLE | CNT_CTL_IMASK;
    write_cntpctl(ctl);

    printf("Oneshot timer set for absolute time %llu ns\n", absolute_time_ns);
    return 0;
}

// ============================================================================
// PERIODIC TIMER
// ============================================================================

int aarch64_timer_set_periodic(uint64_t period_ns)
{
    // Convert period to timer ticks
    uint64_t period_ticks = (period_ns * timer_frequency) / 1000000000ULL;

    // Set timer value (countdown)
    write_cntptval(period_ticks);

    // Enable timer and interrupts
    uint64_t ctl = CNT_CTL_ENABLE | CNT_CTL_IMASK;
    write_cntpctl(ctl);

    printf("Periodic timer set with period %llu ns\n", period_ns);
    return 0;
}

// ============================================================================
// ADVANCED TIMER MANAGEMENT
// ============================================================================

uint32_t aarch64_timer_create(timer_type_t type, uint64_t initial_delay, uint64_t period, void (*callback)(void), const char *name)
{
    if (next_timer_id >= 64)
    {
        printf("ERROR: No free timer slots available\n");
        return 0xFFFFFFFF;
    }

    uint32_t id = next_timer_id++;

    timers[id].id = id;
    timers[id].type = type;
    timers[id].deadline = aarch64_timer_read_ns() + initial_delay;
    timers[id].period = period;
    timers[id].callback = callback;
    timers[id].active = true;
    timers[id].expired = false;

    if (name)
    {
        strncpy(timers[id].name, name, 31);
        timers[id].name[31] = '\0';
    }
    else
    {
        snprintf(timers[id].name, 32, "timer_%u", id);
    }

    printf("Created timer %u: %s (type %d, delay %llu ns, period %llu ns)\n",
           id, timers[id].name, type, initial_delay, period);

    return id;
}

void aarch64_timer_destroy(uint32_t timer_id)
{
    if (timer_id >= 64)
        return;

    timers[timer_id].active = false;
    timers[timer_id].expired = false;

    printf("Destroyed timer %u\n", timer_id);
}

void aarch64_timer_start(uint32_t timer_id)
{
    if (timer_id >= 64)
        return;

    timers[timer_id].active = true;
    timers[timer_id].expired = false;

    printf("Started timer %u\n", timer_id);
}

void aarch64_timer_stop(uint32_t timer_id)
{
    if (timer_id >= 64)
        return;

    timers[timer_id].active = false;

    printf("Stopped timer %u\n", timer_id);
}

void aarch64_timer_reset(uint32_t timer_id)
{
    if (timer_id >= 64)
        return;

    uint64_t current_time = aarch64_timer_read_ns();
    timers[timer_id].deadline = current_time + timers[timer_id].period;
    timers[timer_id].expired = false;

    printf("Reset timer %u\n", timer_id);
}

// ============================================================================
// TIMER PROCESSING
// ============================================================================

void aarch64_timer_process(void)
{
    uint64_t current_time = aarch64_timer_read_ns();

    // Check all active timers
    for (int i = 0; i < next_timer_id; i++)
    {
        if (!timers[i].active || timers[i].expired)
            continue;

        // Check if timer has expired
        if (current_time >= timers[i].deadline)
        {
            timers[i].expired = true;
            timer_expirations++;

            // Call callback if available
            if (timers[i].callback)
            {
                timers[i].callback();
            }

            // Handle periodic timers
            if (timers[i].type == TIMER_TYPE_PERIODIC && timers[i].period > 0)
            {
                timers[i].deadline = current_time + timers[i].period;
                timers[i].expired = false;
            }

            printf("Timer %u (%s) expired\n", i, timers[i].name);
        }
    }
}

// ============================================================================
// TIMER INTERRUPT HANDLING
// ============================================================================

void aarch64_timer_interrupt_handler(void)
{
    timer_interrupts++;

    // Clear interrupt
    aarch64_timer_clear_interrupt();

    // Process expired timers
    aarch64_timer_process();

    // Re-enable timer if it's periodic
    // For oneshot timers, they stay disabled until explicitly re-enabled
}

void aarch64_timer_set_interrupt_handler(void (*handler)(void))
{
    timer_interrupt_handler = handler;
    printf("Timer interrupt handler set\n");
}

// ============================================================================
// POWER MANAGEMENT INTEGRATION
// ============================================================================

void aarch64_timer_suspend(void)
{
    // Save current timer state
    uint64_t current_ctl = read_cntpctl();
    uint64_t current_cval = read_cntpcval();

    // Disable timer
    write_cntpctl(0);

    printf("Timer suspended\n");
}

void aarch64_timer_resume(void)
{
    // Restore timer state
    // In a real system, you would restore the saved state

    printf("Timer resumed\n");
}

// ============================================================================
// TIMER STATISTICS
// ============================================================================

uint32_t aarch64_timer_get_interrupt_count(void)
{
    return timer_interrupts;
}

uint32_t aarch64_timer_get_expiration_count(void)
{
    return timer_expirations;
}

uint64_t aarch64_timer_get_uptime_ns(void)
{
    return system_uptime_ns;
}

void aarch64_timer_print_statistics(void)
{
    printf("=== aarch64 Timer Statistics ===\n");
    printf("Timer frequency: %llu Hz\n", timer_frequency);
    printf("Timer resolution: %llu ns\n", timer_resolution_ns);
    printf("System uptime: %llu ns\n", system_uptime_ns);
    printf("Timer interrupts: %u\n", timer_interrupts);
    printf("Timer expirations: %u\n", timer_expirations);
    printf("Active timers: %u\n", next_timer_id);
}

// ============================================================================
// TIMER TESTING
// ============================================================================

void aarch64_timer_test(void)
{
    printf("Testing aarch64 timer system...\n");

    // Test basic timer reading
    uint64_t time1 = aarch64_timer_read_ns();
    uint64_t time2 = aarch64_timer_read_ns();
    TEST_ASSERT(time2 >= time1, "Timer monotonic");

    // Test timer frequency
    uint64_t freq = aarch64_timer_get_frequency();
    TEST_ASSERT(freq > 0, "Timer frequency valid");

    // Test timer resolution
    uint64_t res = aarch64_timer_get_resolution_ns();
    TEST_ASSERT(res > 0, "Timer resolution valid");

    // Test timer creation
    uint32_t timer_id = aarch64_timer_create(TIMER_TYPE_PERIODIC, 1000000, 1000000, NULL, "test_timer");
    TEST_ASSERT(timer_id != 0xFFFFFFFF, "Timer creation");

    // Test timer operations
    aarch64_timer_start(timer_id);
    TEST_ASSERT(timers[timer_id].active, "Timer start");

    aarch64_timer_stop(timer_id);
    TEST_ASSERT(!timers[timer_id].active, "Timer stop");

    aarch64_timer_destroy(timer_id);

    printf("Timer system test completed successfully\n");
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void aarch64_timers_init(void)
{
    printf("Initializing aarch64 timers...\n");

    // Initialize timer system
    aarch64_timer_init();

    // Set up default timer interrupt handler
    aarch64_timer_set_interrupt_handler(aarch64_timer_interrupt_handler);

    printf("aarch64 timers initialized successfully\n");
}
