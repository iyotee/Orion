/*
 * ORION OS - POWER Architecture Timer Management
 *
 * Timer implementation for POWER architecture (decrementer, time base)
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
// TIMER CONFIGURATION
// ============================================================================

#define POWER_TIMER_MAX_FREQ 1000000000ULL  // 1GHz max
#define POWER_TIMER_MIN_FREQ 1000ULL        // 1kHz min
#define POWER_TIMER_DEFAULT_FREQ 1000000ULL // 1MHz default

typedef struct {
    uint64_t frequency;        // Timer frequency in Hz
    uint64_t period_ns;        // Timer period in nanoseconds
    uint64_t decrementer_value; // Current decrementer value
    uint64_t auto_reload;      // Auto-reload value
    bool running;              // Timer running state
    bool auto_reload_enabled;  // Auto-reload enabled
    bool oneshot;              // One-shot mode
    uint64_t callback;         // Timer callback function
    void *callback_data;       // Timer callback data
} power_timer_context_t;

static power_timer_context_t power_timer;

// ============================================================================
// TIMER INITIALIZATION
// ============================================================================

int power_timer_init(void)
{
    memset(&power_timer, 0, sizeof(power_timer));
    
    // Set default values
    power_timer.frequency = POWER_TIMER_DEFAULT_FREQ;
    power_timer.period_ns = 1000000000ULL / power_timer.frequency;
    power_timer.decrementer_value = 0;
    power_timer.auto_reload = 0;
    power_timer.running = false;
    power_timer.auto_reload_enabled = false;
    power_timer.oneshot = false;
    power_timer.callback = 0;
    power_timer.callback_data = NULL;
    
    // Set up time base frequency
    // This would typically involve reading from device tree or firmware
    
    // Set up decrementer
    __asm__ volatile("mtspr 22, %0" : : "r"(0ULL));
    
    // Set up time base
    __asm__ volatile("mtspr 268, %0" : : "r"(0ULL)); // TBU
    __asm__ volatile("mtspr 269, %0" : : "r"(0ULL)); // TBL
    
    return 0;
}

// ============================================================================
// TIMER FREQUENCY MANAGEMENT
// ============================================================================

int power_timer_set_frequency(uint64_t freq_hz)
{
    if (freq_hz < POWER_TIMER_MIN_FREQ || freq_hz > POWER_TIMER_MAX_FREQ) {
        return -1;
    }
    
    power_timer.frequency = freq_hz;
    power_timer.period_ns = 1000000000ULL / freq_hz;
    
    // Update decrementer if timer is running
    if (power_timer.running) {
        power_timer.decrementer_value = power_timer.frequency / 1000; // 1ms default
        __asm__ volatile("mtspr 22, %0" : : "r"(power_timer.decrementer_value));
    }
    
    return 0;
}

int power_timer_get_frequency(uint64_t *freq_hz)
{
    if (!freq_hz) {
        return -1;
    }
    
    *freq_hz = power_timer.frequency;
    return 0;
}

// ============================================================================
// TIMER PERIOD MANAGEMENT
// ============================================================================

int power_timer_set_period(uint64_t period_ns)
{
    if (period_ns == 0) {
        return -1;
    }
    
    power_timer.period_ns = period_ns;
    power_timer.frequency = 1000000000ULL / period_ns;
    
    // Update decrementer value
    power_timer.decrementer_value = (period_ns * power_timer.frequency) / 1000000000ULL;
    
    // Update decrementer register if timer is running
    if (power_timer.running) {
        __asm__ volatile("mtspr 22, %0" : : "r"(power_timer.decrementer_value));
    }
    
    return 0;
}

int power_timer_get_period(uint64_t *period_ns)
{
    if (!period_ns) {
        return -1;
    }
    
    *period_ns = power_timer.period_ns;
    return 0;
}

// ============================================================================
// TIMER CONTROL
// ============================================================================

int power_timer_start(void)
{
    if (power_timer.running) {
        return 0; // Already running
    }
    
    // Set decrementer value
    if (power_timer.decrementer_value == 0) {
        power_timer.decrementer_value = power_timer.frequency / 1000; // 1ms default
    }
    
    __asm__ volatile("mtspr 22, %0" : : "r"(power_timer.decrementer_value));
    
    power_timer.running = true;
    
    return 0;
}

int power_timer_stop(void)
{
    if (!power_timer.running) {
        return 0; // Already stopped
    }
    
    // Stop timer by setting decrementer to 0
    __asm__ volatile("mtspr 22, %0" : : "r"(0ULL));
    
    power_timer.running = false;
    
    return 0;
}

int power_timer_reset(void)
{
    // Stop timer
    power_timer_stop();
    
    // Reset values
    power_timer.decrementer_value = 0;
    power_timer.auto_reload = 0;
    power_timer.auto_reload_enabled = false;
    power_timer.oneshot = false;
    
    return 0;
}

// ============================================================================
// TIMER READING
// ============================================================================

uint64_t power_timer_read(void)
{
    uint64_t dec_value;
    __asm__ volatile("mfspr %0, 22" : "=r"(dec_value));
    return dec_value;
}

uint64_t power_timer_read_tb(void)
{
    uint64_t tbu, tbl;
    
    // Read time base (64-bit value split into two 32-bit registers)
    __asm__ volatile("mfspr %0, 268" : "=r"(tbu)); // TBU
    __asm__ volatile("mfspr %1, 269" : "=r"(tbl)); // TBL
    
    // Combine into 64-bit value
    return (tbu << 32) | tbl;
}

uint64_t power_timer_read_cycles(void)
{
    // Read cycle counter from time base
    return power_timer_read_tb();
}

// ============================================================================
// TIMER MODES
// ============================================================================

int power_timer_set_oneshot(uint64_t period_ns)
{
    // Set one-shot mode
    power_timer.oneshot = true;
    power_timer.auto_reload_enabled = false;
    
    // Set period
    return power_timer_set_period(period_ns);
}

int power_timer_set_periodic(uint64_t period_ns)
{
    // Set periodic mode
    power_timer.oneshot = false;
    power_timer.auto_reload_enabled = true;
    power_timer.auto_reload = period_ns;
    
    // Set period
    return power_timer_set_period(period_ns);
}

int power_timer_set_auto_reload(uint64_t period_ns)
{
    // Set auto-reload mode
    power_timer.oneshot = false;
    power_timer.auto_reload_enabled = true;
    power_timer.auto_reload = period_ns;
    
    // Set period
    return power_timer_set_period(period_ns);
}

// ============================================================================
// TIMER CALLBACKS
// ============================================================================

int power_timer_set_callback(uint64_t callback, void *data)
{
    power_timer.callback = callback;
    power_timer.callback_data = data;
    
    return 0;
}

int power_timer_clear_callback(void)
{
    power_timer.callback = 0;
    power_timer.callback_data = NULL;
    
    return 0;
}

// ============================================================================
// ADVANCED TIMER FEATURES
// ============================================================================

int power_timer_set_precision(uint64_t precision_ns)
{
    // Set timer precision
    // This would typically involve adjusting timer hardware settings
    
    if (precision_ns > power_timer.period_ns) {
        return -1; // Precision cannot be larger than period
    }
    
    // Adjust frequency to achieve desired precision
    uint64_t new_freq = 1000000000ULL / precision_ns;
    return power_timer_set_frequency(new_freq);
}

int power_timer_get_precision(uint64_t *precision_ns)
{
    if (!precision_ns) {
        return -1;
    }
    
    // Calculate precision based on current frequency
    *precision_ns = 1000000000ULL / power_timer.frequency;
    
    return 0;
}

int power_timer_set_deadline(uint64_t deadline_ns)
{
    // Set timer to expire at specific deadline
    uint64_t current_time = power_timer_read_tb();
    uint64_t target_time = current_time + deadline_ns;
    
    // Calculate decrementer value
    uint64_t dec_value = (deadline_ns * power_timer.frequency) / 1000000000ULL;
    
    power_timer.decrementer_value = dec_value;
    
    // Update decrementer register if timer is running
    if (power_timer.running) {
        __asm__ volatile("mtspr 22, %0" : : "r"(power_timer.decrementer_value));
    }
    
    return 0;
}

// ============================================================================
// TIMER CALIBRATION
// ============================================================================

int power_timer_calibrate(void)
{
    // Calibrate timer against a known reference
    // This would typically involve comparing with an external clock source
    
    uint64_t start_tb = power_timer_read_tb();
    
    // Wait for a known duration (e.g., 1 second)
    // This would involve using a reliable external timer
    
    uint64_t end_tb = power_timer_read_tb();
    uint64_t measured_ticks = end_tb - start_tb;
    
    // Adjust frequency based on measured vs expected
    uint64_t expected_ticks = power_timer.frequency;
    uint64_t correction_factor = (expected_ticks * 1000000ULL) / measured_ticks;
    
    // Apply correction
    uint64_t corrected_freq = (power_timer.frequency * correction_factor) / 1000000ULL;
    
    return power_timer_set_frequency(corrected_freq);
}

// ============================================================================
// TIMER STATISTICS
// ============================================================================

int power_timer_get_stats(power_timer_stats_t *stats)
{
    if (!stats) {
        return -1;
    }
    
    stats->frequency = power_timer.frequency;
    stats->period_ns = power_timer.period_ns;
    stats->decrementer_value = power_timer.decrementer_value;
    stats->auto_reload = power_timer.auto_reload;
    stats->running = power_timer.running;
    stats->auto_reload_enabled = power_timer.auto_reload_enabled;
    stats->oneshot = power_timer.oneshot;
    stats->current_tb = power_timer_read_tb();
    stats->current_dec = power_timer_read();
    
    return 0;
}

// ============================================================================
// TIMER INTERRUPT HANDLING
// ============================================================================

void power_timer_interrupt_handler(void)
{
    // Handle timer interrupt
    if (power_timer.callback) {
        // Call user callback
        // This would involve calling the callback function
        // power_timer.callback(power_timer.callback_data);
    }
    
    // Handle auto-reload
    if (power_timer.auto_reload_enabled && !power_timer.oneshot) {
        // Reload decrementer
        __asm__ volatile("mtspr 22, %0" : : "r"(power_timer.decrementer_value));
    } else if (power_timer.oneshot) {
        // Stop timer in one-shot mode
        power_timer.running = false;
        __asm__ volatile("mtspr 22, %0" : : "r"(0ULL));
    }
}

// ============================================================================
// TIMER CLEANUP
// ============================================================================

void power_timer_cleanup(void)
{
    // Stop timer
    power_timer_stop();
    
    // Clear callback
    power_timer_clear_callback();
    
    // Reset timer state
    memset(&power_timer, 0, sizeof(power_timer));
}
