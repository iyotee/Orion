/*
 * Orion Operating System - x86_64 Timer Management
 *
 * Complete timer subsystem with PIT, HPET, and APIC timer support
 * for precise timing and scheduling operations.
 *
 * Developed by Jérémy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/kernel.h>
#include <orion/types.h>
#include <orion/scheduler.h>

// External function declarations
extern void scheduler_tick(void);

// ========================================
// CONSTANTS AND DEFINITIONS
// ========================================

#define PIT_COMMAND           0x43
#define PIT_CHANNEL0          0x40
#define PIT_FREQUENCY         1193180  // Hz
#define PIT_DIVISOR           1193     // For 1ms intervals
#define PIT_MODE3             0x36     // Square wave mode

#define HPET_BASE             0xFED00000
#define HPET_CAPABILITIES     0x00
#define HPET_CONFIG           0x10
#define HPET_MAIN_COUNTER    0xF0
#define HPET_TIMER0_CONFIG   0x100
#define HPET_TIMER0_COMPARATOR 0x108

#define APIC_TIMER_DIV_1     0x0
#define APIC_TIMER_DIV_2     0x1
#define APIC_TIMER_DIV_4     0x2
#define APIC_TIMER_DIV_8     0x3
#define APIC_TIMER_DIV_16    0x4
#define APIC_TIMER_DIV_32    0x5
#define APIC_TIMER_DIV_64    0x6
#define APIC_TIMER_DIV_128   0x7

#define APIC_TIMER_MODE_PERIODIC 0x20000
#define APIC_TIMER_MODE_ONE_SHOT 0x40000

// ========================================
// GLOBAL VARIABLES
// ========================================

static volatile uint64_t system_ticks = 0;
static volatile uint64_t last_tick_time = 0;
static uint64_t tick_frequency = 1000;  // Default 1kHz
static bool pit_initialized = false;
static bool hpet_available = false;
static bool apic_timer_available = false;

// ========================================
// PIT (PROGRAMMABLE INTERVAL TIMER)
// ========================================

// Initialize PIT
static void pit_init(void) {
    extern void pic_send_command(uint8_t command, uint8_t port);
    extern void pic_send_data(uint8_t data, uint8_t port);
    
    // Send command to PIT
    pic_send_command(PIT_MODE3, PIT_COMMAND);
    
    // Set divisor for 1ms intervals
    pic_send_data(PIT_DIVISOR & 0xFF, PIT_CHANNEL0);
    pic_send_data((PIT_DIVISOR >> 8) & 0xFF, PIT_CHANNEL0);
    
    pit_initialized = true;
    kinfo("PIT initialized at %d Hz", PIT_FREQUENCY / PIT_DIVISOR);
}

// PIT interrupt handler
void pit_interrupt_handler(void) {
    system_ticks++;
    
    // Update scheduler tick
    scheduler_tick();
    
    // Update last tick time
    last_tick_time = arch_get_timestamp();
}

// ========================================
// HPET (HIGH PRECISION EVENT TIMER)
// ========================================

// Check HPET availability
static bool hpet_check_availability(void) {
    // Check if HPET is available via ACPI
    // For now, assume it's not available
    return false;
}

// Initialize HPET
static void hpet_init(void) {
    if (!hpet_check_availability()) {
        kinfo("HPET not available");
        return;
    }
    
    // HPET initialization would go here
    // - Map HPET registers
    // - Configure main counter
    // - Set up timer 0
    
    hpet_available = true;
    kinfo("HPET initialized");
}

// ========================================
// APIC TIMER
// ========================================

// Check APIC timer availability
static bool apic_timer_check_availability(void) {
    extern uint32_t cpuid_get_feature_info(void);
    
    // Check if APIC is available
    uint32_t features = cpuid_get_feature_info();
    return (features & (1 << 9)) != 0;  // APIC bit
}

// Initialize APIC timer
static void apic_timer_init(void) {
    if (!apic_timer_check_availability()) {
        kinfo("APIC timer not available");
        return;
    }
    
    extern uint64_t msr_read(uint32_t msr);
    
    // Get APIC base
    uint64_t apic_base = msr_read(0x1B);
    apic_base &= 0xFFFFF000;
    
    uint32_t* apic_regs = (uint32_t*)apic_base;
    
    // Configure timer divider
    apic_regs[0x3E0 / 4] = APIC_TIMER_DIV_16;  // Divide by 16
    
    // Set timer mode to one-shot
    apic_regs[0x320 / 4] = 32 | APIC_TIMER_MODE_ONE_SHOT;  // IRQ 0, one-shot
    
    // Set initial count (for 1ms at 16MHz = 16000 ticks)
    apic_regs[0x380 / 4] = 16000;
    
    apic_timer_available = true;
    kinfo("APIC timer initialized");
}

// Set APIC timer interval
void apic_timer_set_interval(uint64_t microseconds) {
    if (!apic_timer_available) {
        return;
    }
    
    extern uint64_t msr_read(uint32_t msr);
    uint64_t apic_base = msr_read(0x1B);
    apic_base &= 0xFFFFF000;
    
    uint32_t* apic_regs = (uint32_t*)apic_base;
    
    // Calculate ticks (assuming 16MHz base frequency)
    uint32_t ticks = (microseconds * 16) / 1000;
    
    // Set count
    apic_regs[0x380 / 4] = ticks;
}

// ========================================
// TIMER MANAGEMENT
// ========================================

// Initialize timer subsystem
void arch_timer_init(void) {
    kinfo("x86_64 timer subsystem initialization started");
    
    // Initialize PIT first (fallback)
    pit_init();
    
    // Try to initialize HPET
    hpet_init();
    
    // Try to initialize APIC timer
    apic_timer_init();
    
    // Set tick frequency based on available timers
    if (apic_timer_available) {
        tick_frequency = 1000;  // 1kHz with APIC
        kinfo("Using APIC timer at %d Hz", (int)tick_frequency);
    } else if (hpet_available) {
        tick_frequency = 1000;  // 1kHz with HPET
        kinfo("Using HPET at %d Hz", (int)tick_frequency);
    } else {
        tick_frequency = 1000;  // 1kHz with PIT
        kinfo("Using PIT at %d Hz", (int)tick_frequency);
    }
    
    kinfo("Timer subsystem fully initialized");
}

// Get system ticks
uint64_t arch_get_system_ticks(void) {
    return system_ticks;
}

// Get tick frequency
uint64_t arch_get_tick_frequency(void) {
    return tick_frequency;
}

// Get time since boot in milliseconds
uint64_t arch_get_uptime_ms(void) {
    return system_ticks;
}

// Get time since boot in microseconds
uint64_t arch_get_uptime_us(void) {
    return system_ticks * 1000;  // Assuming 1ms ticks
}

// Get time since boot in nanoseconds
uint64_t arch_get_uptime_ns(void) {
    return system_ticks * 1000000;  // Assuming 1ms ticks
}

// Sleep for specified milliseconds
void arch_sleep_ms(uint64_t milliseconds) {
    uint64_t target_ticks = system_ticks + milliseconds;
    while (system_ticks < target_ticks) {
        extern void cpu_halt(void);
        cpu_halt();
    }
}

// Sleep for specified microseconds
void arch_sleep_us(uint64_t microseconds) {
    // For microsecond precision, we'd need a higher frequency timer
    // For now, just sleep for the minimum tick interval
    arch_sleep_ms(1);
}

// Get timestamp (high-resolution)
uint64_t arch_get_timestamp(void) {
    extern uint64_t read_tsc(void);
    return read_tsc();
}

// Get timestamp frequency
uint64_t arch_get_timestamp_frequency(void) {
    // This would require calibration, but typical values are:
    // - Modern CPUs: 2.4 GHz to 5.0 GHz
    // - For now, return a reasonable default
    return 2400000000ULL;  // 2.4 GHz
}

// Calibrate timestamp frequency
void arch_calibrate_timestamp(void) {
    // TODO: Implement TSC frequency calibration
    // This would involve:
    // 1. Reading PIT/HPET at start
    // 2. Reading TSC at start
    // 3. Waiting for a known interval
    // 4. Reading TSC at end
    // 5. Calculating frequency
    
    kinfo("TSC frequency calibration not yet implemented");
}

// ========================================
// TIMER INTERRUPTS
// ========================================

// Handle timer interrupt
void timer_interrupt_handler(void) {
    // Update system ticks
    system_ticks++;
    
    // Call scheduler tick
    scheduler_tick();
    
    // Update last tick time
    last_tick_time = arch_get_timestamp();
    
    // Send EOI if using APIC
    if (apic_timer_available) {
        extern uint64_t msr_read(uint32_t msr);
        uint64_t apic_base = msr_read(0x1B);
        apic_base &= 0xFFFFF000;
        
        uint32_t* apic_regs = (uint32_t*)apic_base;
        apic_regs[0xB0 / 4] = 0;  // APIC_EOI
    } else {
        // Send EOI to PIC
        extern void pic_send_data(uint8_t data, uint8_t port);
        pic_send_data(0x20, 0x20);
    }
}

// ========================================
// PRECISION TIMING
// ========================================

// Get high-resolution time
uint64_t arch_get_high_res_time(void) {
    return arch_get_timestamp();
}

// Get time difference in nanoseconds
uint64_t arch_get_time_diff_ns(uint64_t start, uint64_t end) {
    uint64_t diff = end - start;
    uint64_t frequency = arch_get_timestamp_frequency();
    
    // Convert to nanoseconds
    return (diff * 1000000000ULL) / frequency;
}

// Wait for precise interval
void arch_wait_precise_ns(uint64_t nanoseconds) {
    uint64_t start = arch_get_timestamp();
    uint64_t target_cycles = (nanoseconds * arch_get_timestamp_frequency()) / 1000000000ULL;
    
    while ((arch_get_timestamp() - start) < target_cycles) {
        extern void cpu_pause(void);
        cpu_pause();
    }
}
