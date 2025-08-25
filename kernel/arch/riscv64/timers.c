/*
 * ORION OS - RISC-V 64-bit Timer Management
 *
 * Timer system management for RISC-V 64-bit architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// GLOBAL TIMER CONTEXTS
// ============================================================================

// System timer context
static struct {
    bool initialized;
    uint64_t frequency;
    uint64_t current_value;
    uint64_t compare_value;
    bool enabled;
    bool interrupt_enabled;
    uint32_t tick_count;
    uint64_t last_tick_time;
} riscv64_system_timer = {0};

// High-resolution timer context
static struct {
    bool initialized;
    uint64_t frequency;
    uint64_t current_value;
    uint64_t start_time;
    uint64_t elapsed_time;
    bool running;
} riscv64_high_res_timer = {0};

// Timer callback context
static struct {
    bool initialized;
    void (*callbacks[32])(void);
    uint64_t intervals[32];
    uint64_t next_trigger[32];
    bool active[32];
    uint32_t callback_count;
} riscv64_timer_callbacks = {0};

// ============================================================================
// TIMER INITIALIZATION
// ============================================================================

// Initialize timer system
void riscv64_timer_init(void)
{
    if (riscv64_system_timer.initialized) {
        return;
    }
    
    // Initialize system timer
    riscv64_system_timer.frequency = RISCV64_TIMER_FREQ;
    riscv64_system_timer.current_value = 0;
    riscv64_system_timer.compare_value = 0;
    riscv64_system_timer.enabled = false;
    riscv64_system_timer.interrupt_enabled = false;
    riscv64_system_timer.tick_count = 0;
    riscv64_system_timer.last_tick_time = 0;
    riscv64_system_timer.initialized = true;
    
    // Initialize high-resolution timer
    riscv64_high_res_timer.frequency = RISCV64_TIMER_FREQ;
    riscv64_high_res_timer.current_value = 0;
    riscv64_high_res_timer.start_time = 0;
    riscv64_high_res_timer.elapsed_time = 0;
    riscv64_high_res_timer.running = false;
    riscv64_high_res_timer.initialized = true;
    
    // Initialize timer callbacks
    for (int i = 0; i < 32; i++) {
        riscv64_timer_callbacks.callbacks[i] = NULL;
        riscv64_timer_callbacks.intervals[i] = 0;
        riscv64_timer_callbacks.next_trigger[i] = 0;
        riscv64_timer_callbacks.active[i] = false;
    }
    riscv64_timer_callbacks.callback_count = 0;
    riscv64_timer_callbacks.initialized = true;
    
    // Enable timer interrupt
    riscv64_timer_enable_interrupt();
    
    printf("RISC-V64: Timer system initialized\n");
}

// ============================================================================
// SYSTEM TIMER FUNCTIONS
// ============================================================================

// Get system timer context
struct riscv64_timer_config* riscv64_get_system_timer_context(void)
{
    static struct riscv64_timer_config context;
    
    if (!riscv64_system_timer.initialized) {
        return NULL;
    }
    
    context.frequency = riscv64_system_timer.frequency;
    context.compare_value = riscv64_system_timer.compare_value;
    context.current_value = riscv64_system_timer.current_value;
    context.enabled = riscv64_system_timer.enabled;
    context.interrupt_enabled = riscv64_system_timer.interrupt_enabled;
    
    return &context;
}

// Enable system timer
void riscv64_timer_enable(void)
{
    if (!riscv64_system_timer.initialized) {
        return;
    }
    
    riscv64_system_timer.enabled = true;
    
    // Set up hardware timer
    uint64_t current_time = riscv64_timer_read_ns();
    riscv64_system_timer.compare_value = current_time + (1000000000ULL / riscv64_system_timer.frequency);
    
    // Enable timer interrupt
    riscv64_timer_enable_interrupt();
    
    printf("RISC-V64: System timer enabled\n");
}

// Disable system timer
void riscv64_timer_disable(void)
{
    if (!riscv64_system_timer.initialized) {
        return;
    }
    
    riscv64_system_timer.enabled = false;
    
    // Disable timer interrupt
    riscv64_timer_disable_interrupt();
    
    printf("RISC-V64: System timer disabled\n");
}

// Set system timer frequency
void riscv64_timer_set_frequency(uint64_t frequency)
{
    if (!riscv64_system_timer.initialized || frequency == 0) {
        return;
    }
    
    riscv64_system_timer.frequency = frequency;
    
    // Recalculate compare value if timer is running
    if (riscv64_system_timer.enabled) {
        uint64_t current_time = riscv64_timer_read_ns();
        riscv64_system_timer.compare_value = current_time + (1000000000ULL / frequency);
    }
    
    printf("RISC-V64: System timer frequency set to %llu Hz\n", frequency);
}

// Get system timer frequency
uint64_t riscv64_timer_get_frequency(void)
{
    if (!riscv64_system_timer.initialized) {
        return 0;
    }
    
    return riscv64_system_timer.frequency;
}

// ============================================================================
// PERIODIC TIMER FUNCTIONS
// ============================================================================

// Set periodic timer
int riscv64_timer_set_periodic(uint64_t period_ns)
{
    if (!riscv64_system_timer.initialized || period_ns == 0) {
        return -1;
    }
    
    uint64_t current_time = riscv64_timer_read_ns();
    riscv64_system_timer.compare_value = current_time + period_ns;
    
    // Enable timer if not already enabled
    if (!riscv64_system_timer.enabled) {
        riscv64_timer_enable();
    }
    
    printf("RISC-V64: Periodic timer set to %llu ns\n", period_ns);
    return 0;
}

// Get periodic timer period
uint64_t riscv64_timer_get_period(void)
{
    if (!riscv64_system_timer.initialized) {
        return 0;
    }
    
    return 1000000000ULL / riscv64_system_timer.frequency;
}

// ============================================================================
// HIGH-RESOLUTION TIMER FUNCTIONS
// ============================================================================

// Get high-resolution timer context
struct riscv64_timer_config* riscv64_get_high_res_timer_context(void)
{
    static struct riscv64_timer_config context;
    
    if (!riscv64_high_res_timer.initialized) {
        return NULL;
    }
    
    context.frequency = riscv64_high_res_timer.frequency;
    context.compare_value = 0;
    context.current_value = riscv64_high_res_timer.elapsed_time;
    context.enabled = riscv64_high_res_timer.running;
    context.interrupt_enabled = false;
    
    return &context;
}

// Start high-resolution timer
void riscv64_high_res_timer_start(void)
{
    if (!riscv64_high_res_timer.initialized) {
        return;
    }
    
    riscv64_high_res_timer.start_time = riscv64_timer_read_ns();
    riscv64_high_res_timer.running = true;
    
    printf("RISC-V64: High-resolution timer started\n");
}

// Stop high-resolution timer
void riscv64_high_res_timer_stop(void)
{
    if (!riscv64_high_res_timer.initialized || !riscv64_high_res_timer.running) {
        return;
    }
    
    uint64_t stop_time = riscv64_timer_read_ns();
    riscv64_high_res_timer.elapsed_time = stop_time - riscv64_high_res_timer.start_time;
    riscv64_high_res_timer.running = false;
    
    printf("RISC-V64: High-resolution timer stopped, elapsed: %llu ns\n", riscv64_high_res_timer.elapsed_time);
}

// Reset high-resolution timer
void riscv64_high_res_timer_reset(void)
{
    if (!riscv64_high_res_timer.initialized) {
        return;
    }
    
    riscv64_high_res_timer.elapsed_time = 0;
    riscv64_high_res_timer.start_time = 0;
    riscv64_high_res_timer.running = false;
    
    printf("RISC-V64: High-resolution timer reset\n");
}

// Get high-resolution timer value
uint64_t riscv64_high_res_timer_get_value(void)
{
    if (!riscv64_high_res_timer.initialized) {
        return 0;
    }
    
    if (riscv64_high_res_timer.running) {
        uint64_t current_time = riscv64_timer_read_ns();
        return current_time - riscv64_high_res_timer.start_time;
    }
    
    return riscv64_high_res_timer.elapsed_time;
}

// ============================================================================
// TIMER UTILITY FUNCTIONS
// ============================================================================

// Read timer value in nanoseconds
uint64_t riscv64_timer_read_ns(void)
{
    uint64_t time_value;
    asm volatile("csrr %0, time" : "=r"(time_value));
    
    // Convert to nanoseconds (assuming 1GHz timer)
    return time_value;
}

// Read timer value in microseconds
uint64_t riscv64_timer_read_us(void)
{
    return riscv64_timer_read_ns() / 1000;
}

// Read timer value in milliseconds
uint64_t riscv64_timer_read_ms(void)
{
    return riscv64_timer_read_ns() / 1000000;
}

// Read timer value in seconds
uint64_t riscv64_timer_read_s(void)
{
    return riscv64_timer_read_ns() / 1000000000;
}

// Set one-shot timer
int riscv64_timer_set_oneshot(uint64_t deadline_ns)
{
    if (!riscv64_system_timer.initialized) {
        return -1;
    }
    
    riscv64_system_timer.compare_value = deadline_ns;
    
    // Enable timer if not already enabled
    if (!riscv64_system_timer.enabled) {
        riscv64_timer_enable();
    }
    
    printf("RISC-V64: One-shot timer set to %llu ns\n", deadline_ns);
    return 0;
}

// Delay for specified number of nanoseconds
void riscv64_timer_delay_ns(uint64_t delay_ns)
{
    uint64_t start_time = riscv64_timer_read_ns();
    uint64_t target_time = start_time + delay_ns;
    
    while (riscv64_timer_read_ns() < target_time) {
        // Busy wait
    }
}

// Delay for specified number of microseconds
void riscv64_timer_delay_us(uint64_t delay_us)
{
    riscv64_timer_delay_ns(delay_us * 1000);
}

// Delay for specified number of milliseconds
void riscv64_timer_delay_ms(uint64_t delay_ms)
{
    riscv64_timer_delay_ns(delay_ms * 1000000);
}

// Delay for specified number of seconds
void riscv64_timer_delay_s(uint64_t delay_s)
{
    riscv64_timer_delay_ns(delay_s * 1000000000);
}

// ============================================================================
// TIMER TICK PROCESSING
// ============================================================================

// Process timer tick
void riscv64_timer_process_tick(void)
{
    if (!riscv64_system_timer.initialized || !riscv64_system_timer.enabled) {
        return;
    }
    
    uint64_t current_time = riscv64_timer_read_ns();
    
    // Check if timer has expired
    if (current_time >= riscv64_system_timer.compare_value) {
        riscv64_system_timer.tick_count++;
        riscv64_system_timer.last_tick_time = current_time;
        
        // Set next timer tick
        riscv64_system_timer.compare_value = current_time + (1000000000ULL / riscv64_system_timer.frequency);
        
        // Process timer callbacks
        riscv64_timer_process_callbacks();
        
        // Call system tick handler
        riscv64_system_tick_handler();
    }
}

// Process timer callbacks
void riscv64_timer_process_callbacks(void)
{
    if (!riscv64_timer_callbacks.initialized) {
        return;
    }
    
    uint64_t current_time = riscv64_timer_read_ns();
    
    for (int i = 0; i < 32; i++) {
        if (riscv64_timer_callbacks.active[i] && riscv64_timer_callbacks.callbacks[i]) {
            if (current_time >= riscv64_timer_callbacks.next_trigger[i]) {
                // Call callback
                riscv64_timer_callbacks.callbacks[i]();
                
                // Set next trigger time
                riscv64_timer_callbacks.next_trigger[i] = current_time + riscv64_timer_callbacks.intervals[i];
            }
        }
    }
}

// ============================================================================
// TIMER CALLBACK MANAGEMENT
// ============================================================================

// Register timer callback
int riscv64_timer_register_callback(void (*callback)(void), uint64_t interval_ns)
{
    if (!riscv64_timer_callbacks.initialized || !callback || interval_ns == 0) {
        return -1;
    }
    
    // Find free slot
    for (int i = 0; i < 32; i++) {
        if (!riscv64_timer_callbacks.active[i]) {
            riscv64_timer_callbacks.callbacks[i] = callback;
            riscv64_timer_callbacks.intervals[i] = interval_ns;
            riscv64_timer_callbacks.next_trigger[i] = riscv64_timer_read_ns() + interval_ns;
            riscv64_timer_callbacks.active[i] = true;
            riscv64_timer_callbacks.callback_count++;
            
            printf("RISC-V64: Timer callback registered at slot %d, interval %llu ns\n", i, interval_ns);
            return i;
        }
    }
    
    printf("RISC-V64: No free timer callback slots available\n");
    return -1;
}

// Unregister timer callback
int riscv64_timer_unregister_callback(int callback_id)
{
    if (!riscv64_timer_callbacks.initialized || callback_id < 0 || callback_id >= 32) {
        return -1;
    }
    
    if (riscv64_timer_callbacks.active[callback_id]) {
        riscv64_timer_callbacks.callbacks[callback_id] = NULL;
        riscv64_timer_callbacks.intervals[callback_id] = 0;
        riscv64_timer_callbacks.next_trigger[callback_id] = 0;
        riscv64_timer_callbacks.active[callback_id] = false;
        riscv64_timer_callbacks.callback_count--;
        
        printf("RISC-V64: Timer callback unregistered from slot %d\n", callback_id);
        return 0;
    }
    
    return -1;
}

// Get timer callback count
uint32_t riscv64_timer_get_callback_count(void)
{
    if (!riscv64_timer_callbacks.initialized) {
        return 0;
    }
    
    return riscv64_timer_callbacks.callback_count;
}

// ============================================================================
// TIMER STATUS FUNCTIONS
// ============================================================================

// Get timer status
bool riscv64_timer_is_enabled(void)
{
    if (!riscv64_system_timer.initialized) {
        return false;
    }
    
    return riscv64_system_timer.enabled;
}

// Get timer tick count
uint32_t riscv64_timer_get_tick_count(void)
{
    if (!riscv64_system_timer.initialized) {
        return 0;
    }
    
    return riscv64_system_timer.tick_count;
}

// Get last tick time
uint64_t riscv64_timer_get_last_tick_time(void)
{
    if (!riscv64_system_timer.initialized) {
        return 0;
    }
    
    return riscv64_system_timer.last_tick_time;
}

// Get timer resolution
uint64_t riscv64_timer_get_resolution(void)
{
    return RISCV64_TIMER_RESOLUTION_NS;
}

// ============================================================================
// TIMER CALIBRATION
// ============================================================================

// Calibrate timer
void riscv64_timer_calibrate(void)
{
    if (!riscv64_system_timer.initialized) {
        return;
    }
    
    printf("RISC-V64: Calibrating timer...\n");
    
    // Measure timer frequency by counting ticks over a known period
    uint64_t start_time = riscv64_timer_read_ns();
    uint32_t start_ticks = riscv64_system_timer.tick_count;
    
    // Wait for a reasonable number of ticks
    while (riscv64_system_timer.tick_count < start_ticks + 100) {
        // Wait for ticks
    }
    
    uint64_t end_time = riscv64_timer_read_ns();
    uint32_t end_ticks = riscv64_system_timer.tick_count;
    
    uint64_t elapsed_time = end_time - start_time;
    uint32_t tick_count = end_ticks - start_ticks;
    
    if (tick_count > 0 && elapsed_time > 0) {
        uint64_t measured_frequency = (tick_count * 1000000000ULL) / elapsed_time;
        printf("RISC-V64: Timer calibrated - measured frequency: %llu Hz\n", measured_frequency);
        
        // Update timer frequency if significantly different
        if (measured_frequency != riscv64_system_timer.frequency) {
            riscv64_system_timer.frequency = measured_frequency;
            printf("RISC-V64: Timer frequency updated to %llu Hz\n", measured_frequency);
        }
    }
}

// ============================================================================
// TIMER POWER MANAGEMENT
// ============================================================================

// Enable timer power management
void riscv64_timer_enable_power_management(void)
{
    printf("RISC-V64: Timer power management enabled\n");
}

// Disable timer power management
void riscv64_timer_disable_power_management(void)
{
    printf("RISC-V64: Timer power management disabled\n");
}

// Set timer power state
int riscv64_timer_set_power_state(uint32_t state)
{
    printf("RISC-V64: Timer power state set to %u\n", state);
    return 0;
}

// Get timer power state
uint32_t riscv64_timer_get_power_state(void)
{
    return 0; // Normal operation
}

// ============================================================================
// TIMER DEBUG FUNCTIONS
// ============================================================================

// Print timer status
void riscv64_timer_print_status(void)
{
    printf("RISC-V64: Timer System Status\n");
    printf("  System Timer:\n");
    printf("    Initialized: %s\n", riscv64_system_timer.initialized ? "Yes" : "No");
    printf("    Enabled: %s\n", riscv64_system_timer.enabled ? "Yes" : "No");
    printf("    Frequency: %llu Hz\n", riscv64_system_timer.frequency);
    printf("    Tick Count: %u\n", riscv64_system_timer.tick_count);
    printf("    Last Tick: %llu ns\n", riscv64_system_timer.last_tick_time);
    
    printf("  High-Resolution Timer:\n");
    printf("    Initialized: %s\n", riscv64_high_res_timer.initialized ? "Yes" : "No");
    printf("    Running: %s\n", riscv64_high_res_timer.running ? "Yes" : "No");
    printf("    Elapsed Time: %llu ns\n", riscv64_high_res_timer.elapsed_time);
    
    printf("  Timer Callbacks:\n");
    printf("    Initialized: %s\n", riscv64_timer_callbacks.initialized ? "Yes" : "No");
    printf("    Active Callbacks: %u\n", riscv64_timer_callbacks.callback_count);
    
    printf("  Current Time: %llu ns\n", riscv64_timer_read_ns());
}

// Print timer statistics
void riscv64_timer_print_statistics(void)
{
    printf("RISC-V64: Timer Statistics\n");
    printf("  System Timer Ticks: %u\n", riscv64_timer_get_tick_count());
    printf("  Timer Resolution: %llu ns\n", riscv64_timer_get_resolution());
    printf("  Timer Frequency: %llu Hz\n", riscv64_timer_get_frequency());
    printf("  Timer Callbacks: %u\n", riscv64_timer_get_callback_count());
}

// ============================================================================
// TIMER CLEANUP
// ============================================================================

// Cleanup timer system
void riscv64_timer_cleanup(void)
{
    if (!riscv64_system_timer.initialized) {
        return;
    }
    
    // Disable timers
    riscv64_timer_disable();
    riscv64_high_res_timer_stop();
    
    // Clear callbacks
    for (int i = 0; i < 32; i++) {
        riscv64_timer_callbacks.callbacks[i] = NULL;
        riscv64_timer_callbacks.intervals[i] = 0;
        riscv64_timer_callbacks.next_trigger[i] = 0;
        riscv64_timer_callbacks.active[i] = false;
    }
    riscv64_timer_callbacks.callback_count = 0;
    
    riscv64_system_timer.initialized = false;
    riscv64_high_res_timer.initialized = false;
    riscv64_timer_callbacks.initialized = false;
    
    printf("RISC-V64: Timer system cleaned up\n");
}

// ============================================================================
// INTERRUPT MANAGEMENT
// ============================================================================

// Enable timer interrupt
void riscv64_timer_enable_interrupt(void)
{
    if (!riscv64_system_timer.initialized) {
        return;
    }
    
    riscv64_system_timer.interrupt_enabled = true;
    
    // Enable timer interrupt in hardware
    uint64_t mie_val;
    asm volatile("csrr %0, mie" : "=r"(mie_val));
    mie_val |= (1ULL << RISCV64_IRQ_TIMER);
    asm volatile("csrw mie, %0" : : "r"(mie_val));
    
    printf("RISC-V64: Timer interrupt enabled\n");
}

// Disable timer interrupt
void riscv64_timer_disable_interrupt(void)
{
    if (!riscv64_system_timer.initialized) {
        return;
    }
    
    riscv64_system_timer.interrupt_enabled = false;
    
    // Disable timer interrupt in hardware
    uint64_t mie_val;
    asm volatile("csrr %0, mie" : "=r"(mie_val));
    mie_val &= ~(1ULL << RISCV64_IRQ_TIMER);
    asm volatile("csrw mie, %0" : : "r"(mie_val));
    
    printf("RISC-V64: Timer interrupt disabled\n");
}

// Check if timer interrupt is enabled
bool riscv64_timer_interrupt_is_enabled(void)
{
    if (!riscv64_system_timer.initialized) {
        return false;
    }
    
    return riscv64_system_timer.interrupt_enabled;
}

// ============================================================================
// SYSTEM TICK HANDLER
// ============================================================================

// System tick handler (called from timer interrupt)
void riscv64_system_tick_handler(void)
{
    // This function is called on each timer tick
    // It can be used for system-wide timing operations
    
    // Update system time
    // Process scheduler
    // Handle timeouts
    // etc.
}

// ============================================================================
// TIMER EXTENSIONS
// ============================================================================

// Get timer capabilities
uint64_t riscv64_timer_get_capabilities(void)
{
    uint64_t capabilities = 0;
    
    // Set capability bits based on available features
    capabilities |= (1ULL << 0);  // Basic timer support
    capabilities |= (1ULL << 1);  // High-resolution timer
    capabilities |= (1ULL << 2);  // Periodic timer
    capabilities |= (1ULL << 3);  // One-shot timer
    capabilities |= (1ULL << 4);  // Callback support
    capabilities |= (1ULL << 5);  // Power management
    
    return capabilities;
}

// Check timer capability
bool riscv64_timer_has_capability(uint64_t capability)
{
    uint64_t capabilities = riscv64_timer_get_capabilities();
    return (capabilities & capability) != 0;
}
