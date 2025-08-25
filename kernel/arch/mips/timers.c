/*
 * ORION OS - MIPS Architecture Timer Management
 *
 * Timer management for MIPS architecture (MIPS32, MIPS64)
 * Implements system timers, periodic timers, and high-resolution timers
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

// Timer contexts
static mips_timer_context_t mips_timer_context = {0};

// System timer state
static volatile uint64_t mips_system_tick = 0;
static volatile uint64_t mips_system_uptime_ms = 0;

// High-resolution timer state
static volatile uint64_t mips_high_res_timer_start = 0;
static volatile uint64_t mips_high_res_timer_frequency = 0;

// Timer callback functions
static struct {
    void (*callback)(void);
    void *data;
    uint64_t period;
    uint64_t next_trigger;
    bool active;
} mips_timer_callbacks[MIPS_MAX_TIMERS] = {0};

// ============================================================================
// TIMER INITIALIZATION
// ============================================================================

void mips_timer_init(void)
{
    printf("MIPS: Initializing timers\n");

    // Initialize timer context
    memset(&mips_timer_context, 0, sizeof(mips_timer_context));
    mips_timer_context.timer_enabled = false;
    mips_timer_context.timer_count = 0;
    mips_timer_context.system_tick = 0;

    // Initialize timer callbacks
    memset(mips_timer_callbacks, 0, sizeof(mips_timer_callbacks));

    // Initialize system timer
    mips_system_tick = 0;
    mips_system_uptime_ms = 0;

    // Get CP0 timer frequency
    mips_high_res_timer_frequency = mips_cp0_count_get_frequency();

    printf("MIPS: Timer initialization complete\n");
    printf("MIPS: CP0 timer frequency: %llu Hz\n", (unsigned long long)mips_high_res_timer_frequency);
}

// ============================================================================
// SYSTEM TIMER FUNCTIONS
// ============================================================================

void mips_system_timer_start(void)
{
    printf("MIPS: Starting system timer\n");
    
    // Enable timer interrupt
    mips_interrupt_enable_timer();
    
    // Set initial compare value
    uint64_t current_count = mips_cpu_read_cp0_count();
    uint64_t compare_value = current_count + mips_high_res_timer_frequency / 1000; // 1ms
    mips_cpu_write_cp0_compare(compare_value);
    
    mips_timer_context.timer_enabled = true;
}

void mips_system_timer_stop(void)
{
    printf("MIPS: Stopping system timer\n");
    
    // Disable timer interrupt
    mips_interrupt_disable_timer();
    
    mips_timer_context.timer_enabled = false;
}

void mips_system_timer_reset(void)
{
    printf("MIPS: Resetting system timer\n");
    
    mips_system_tick = 0;
    mips_system_uptime_ms = 0;
    mips_timer_context.system_tick = 0;
}

uint64_t mips_system_timer_get_tick(void)
{
    return mips_system_tick;
}

uint64_t mips_system_timer_get_uptime_ms(void)
{
    return mips_system_uptime_ms;
}

// ============================================================================
// PERIODIC TIMER FUNCTIONS
// ============================================================================

void mips_periodic_timer_start(uint32_t timer_id)
{
    if (timer_id < MIPS_MAX_TIMERS)
    {
        printf("MIPS: Starting periodic timer %u\n", timer_id);
        
        mips_timer_context.timers[timer_id].timer_id = timer_id;
        mips_timer_context.timers[timer_id].mode = MIPS_TIMER_MODE_PERIODIC;
        mips_timer_context.timers[timer_id].value = 0;
        
        // Calculate next trigger time
        uint64_t current_tick = mips_system_timer_get_tick();
        mips_timer_context.timers[timer_id].next_trigger = current_tick + 
            mips_timer_context.timers[timer_id].reload_value;
        
        mips_timer_context.timers[timer_id].active = true;
        mips_timer_context.timer_count++;
    }
}

void mips_periodic_timer_stop(uint32_t timer_id)
{
    if (timer_id < MIPS_MAX_TIMERS)
    {
        printf("MIPS: Stopping periodic timer %u\n", timer_id);
        
        mips_timer_context.timers[timer_id].active = false;
        mips_timer_context.timer_count--;
    }
}

void mips_periodic_timer_reset(uint32_t timer_id)
{
    if (timer_id < MIPS_MAX_TIMERS)
    {
        printf("MIPS: Resetting periodic timer %u\n", timer_id);
        
        mips_timer_context.timers[timer_id].value = 0;
        mips_timer_context.timers[timer_id].next_trigger = mips_system_timer_get_tick() + 
            mips_timer_context.timers[timer_id].reload_value;
    }
}

void mips_periodic_timer_set_frequency(uint32_t timer_id, uint64_t frequency)
{
    if (timer_id < MIPS_MAX_TIMERS && frequency > 0)
    {
        mips_timer_context.timers[timer_id].frequency = frequency;
        mips_timer_context.timers[timer_id].reload_value = mips_high_res_timer_frequency / frequency;
        
        printf("MIPS: Periodic timer %u frequency set to %llu Hz\n", 
               timer_id, (unsigned long long)frequency);
    }
}

void mips_periodic_timer_set_callback(uint32_t timer_id, void (*callback)(void))
{
    if (timer_id < MIPS_MAX_TIMERS)
    {
        mips_timer_context.timers[timer_id].callback = callback;
        printf("MIPS: Periodic timer %u callback set\n", timer_id);
    }
}

// ============================================================================
// HIGH-RESOLUTION TIMER FUNCTIONS
// ============================================================================

uint64_t mips_high_res_timer_get_cycles(void)
{
    return mips_cpu_read_cp0_count();
}

uint64_t mips_high_res_timer_get_frequency(void)
{
    return mips_high_res_timer_frequency;
}

uint64_t mips_high_res_timer_get_microseconds(void)
{
    uint64_t cycles = mips_cpu_read_cp0_count();
    return cycles / (mips_high_res_timer_frequency / 1000000);
}

uint64_t mips_high_res_timer_get_milliseconds(void)
{
    uint64_t cycles = mips_cpu_read_cp0_count();
    return cycles / (mips_high_res_timer_frequency / 1000);
}

void mips_high_res_timer_start_measurement(void)
{
    mips_high_res_timer_start = mips_cpu_read_cp0_count();
}

uint64_t mips_high_res_timer_end_measurement(void)
{
    uint64_t end_cycles = mips_cpu_read_cp0_count();
    return end_cycles - mips_high_res_timer_start;
}

uint64_t mips_high_res_timer_measure_microseconds(void)
{
    uint64_t cycles = mips_high_res_timer_end_measurement();
    return cycles / (mips_high_res_timer_frequency / 1000000);
}

uint64_t mips_high_res_timer_measure_milliseconds(void)
{
    uint64_t cycles = mips_high_res_timer_end_measurement();
    return cycles / (mips_high_res_timer_frequency / 1000);
}

// ============================================================================
// TIMER UTILITY FUNCTIONS
// ============================================================================

void mips_delay_cycles(uint64_t cycles)
{
    uint64_t start = mips_cpu_read_cp0_count();
    uint64_t target = start + cycles;
    
    while (mips_cpu_read_cp0_count() < target)
    {
        // Wait for target cycle count
    }
}

void mips_delay_microseconds(uint64_t microseconds)
{
    uint64_t cycles = microseconds * (mips_high_res_timer_frequency / 1000000);
    mips_delay_cycles(cycles);
}

void mips_delay_milliseconds(uint64_t milliseconds)
{
    uint64_t cycles = milliseconds * (mips_high_res_timer_frequency / 1000);
    mips_delay_cycles(cycles);
}

// ============================================================================
// TIMER TICK PROCESSING
// ============================================================================

void mips_timer_tick(void)
{
    // Update system timer
    mips_system_tick++;
    mips_timer_context.system_tick++;
    
    // Update system uptime every 1000 ticks (1 second)
    if (mips_system_tick % 1000 == 0)
    {
        mips_system_uptime_ms += 1000;
    }
    
    // Process periodic timers
    for (uint32_t i = 0; i < MIPS_MAX_TIMERS; i++)
    {
        if (mips_timer_context.timers[i].active && 
            mips_system_tick >= mips_timer_context.timers[i].next_trigger)
        {
            // Timer expired, execute callback
            if (mips_timer_context.timers[i].callback)
            {
                mips_timer_context.timers[i].callback();
            }
            
            // Update next trigger time
            mips_timer_context.timers[i].next_trigger = mips_system_tick + 
                mips_timer_context.timers[i].reload_value;
        }
    }
}

// ============================================================================
// TIMER STATUS AND CONTROL
// ============================================================================

bool mips_timer_is_enabled(uint32_t timer_id)
{
    if (timer_id < MIPS_MAX_TIMERS)
    {
        return mips_timer_context.timers[timer_id].active;
    }
    return false;
}

uint64_t mips_timer_get_value(uint32_t timer_id)
{
    if (timer_id < MIPS_MAX_TIMERS)
    {
        return mips_timer_context.timers[timer_id].value;
    }
    return 0;
}

uint64_t mips_timer_get_frequency(uint32_t timer_id)
{
    if (timer_id < MIPS_MAX_TIMERS)
    {
        return mips_timer_context.timers[timer_id].frequency;
    }
    return 0;
}

uint64_t mips_timer_get_system_tick(void)
{
    return mips_timer_context.system_tick;
}

uint32_t mips_timer_get_active_count(void)
{
    return mips_timer_context.timer_count;
}

// ============================================================================
// CP0 TIMER FUNCTIONS
// ============================================================================

uint64_t mips_cp0_count_get_value(void)
{
    return mips_cpu_read_cp0_count();
}

uint64_t mips_cp0_compare_get_value(void)
{
    return mips_cpu_read_cp0_compare();
}

void mips_cp0_compare_set_value(uint64_t value)
{
    mips_cpu_write_cp0_compare(value);
}

uint64_t mips_cp0_count_get_frequency(void)
{
    return mips_high_res_timer_frequency;
}

// ============================================================================
// TIMER CALIBRATION
// ============================================================================

void mips_timer_calibrate(void)
{
    printf("MIPS: Calibrating timer\n");
    
    // Measure timer frequency by counting cycles over a known period
    uint64_t start_cycles = mips_cpu_read_cp0_count();
    
    // Wait for approximately 1 second (using a simple delay)
    mips_delay_milliseconds(1000);
    
    uint64_t end_cycles = mips_cpu_read_cp0_count();
    uint64_t measured_frequency = end_cycles - start_cycles;
    
    printf("MIPS: Measured timer frequency: %llu Hz\n", (unsigned long long)measured_frequency);
    
    // Update frequency if significantly different
    if (measured_frequency > 0 && 
        (measured_frequency < mips_high_res_timer_frequency * 0.9 || 
         measured_frequency > mips_high_res_timer_frequency * 1.1))
    {
        mips_high_res_timer_frequency = measured_frequency;
        printf("MIPS: Timer frequency updated to: %llu Hz\n", (unsigned long long)measured_frequency);
    }
}

// ============================================================================
// TIMER POWER MANAGEMENT
// ============================================================================

void mips_timer_enter_low_power_mode(void)
{
    printf("MIPS: Entering timer low power mode\n");
    
    // Disable high-frequency timers
    // Keep only essential timers running
    
    // Reduce timer interrupt frequency
    uint64_t current_count = mips_cpu_read_cp0_count();
    uint64_t compare_value = current_count + mips_high_res_timer_frequency / 100; // 10ms
    mips_cpu_write_cp0_compare(compare_value);
}

void mips_timer_exit_low_power_mode(void)
{
    printf("MIPS: Exiting timer low power mode\n");
    
    // Restore normal timer operation
    uint64_t current_count = mips_cpu_read_cp0_count();
    uint64_t compare_value = current_count + mips_high_res_timer_frequency / 1000; // 1ms
    mips_cpu_write_cp0_compare(compare_value);
}

// ============================================================================
// TIMER DEBUG AND DIAGNOSTICS
// ============================================================================

void mips_timer_print_status(void)
{
    printf("MIPS: Timer Status:\n");
    printf("  System Timer: %s\n", mips_timer_context.timer_enabled ? "Enabled" : "Disabled");
    printf("  System Tick: %llu\n", (unsigned long long)mips_system_tick);
    printf("  System Uptime: %llu ms\n", (unsigned long long)mips_system_uptime_ms);
    printf("  Active Timers: %u\n", mips_timer_context.timer_count);
    printf("  CP0 Frequency: %llu Hz\n", (unsigned long long)mips_high_res_timer_frequency);
    
    for (uint32_t i = 0; i < MIPS_MAX_TIMERS; i++)
    {
        if (mips_timer_context.timers[i].active)
        {
            printf("  Timer %u: Freq=%llu Hz, Active=%s\n", 
                   i, 
                   (unsigned long long)mips_timer_context.timers[i].frequency,
                   mips_timer_context.timers[i].active ? "Yes" : "No");
        }
    }
}

void mips_timer_reset_all(void)
{
    printf("MIPS: Resetting all timers\n");
    
    // Stop all timers
    for (uint32_t i = 0; i < MIPS_MAX_TIMERS; i++)
    {
        mips_timer_context.timers[i].active = false;
    }
    
    // Reset system timer
    mips_system_timer_reset();
    
    // Reset timer context
    mips_timer_context.timer_count = 0;
    
    printf("MIPS: All timers reset\n");
}
