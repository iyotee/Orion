/*
 * ORION OS - MIPS Architecture Entry Point
 *
 * Main entry point for MIPS architecture (MIPS32, MIPS64)
 * Supports all MIPS processors: R3000, R4000, R5000, R10000, etc.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include <stdio.h>

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// Stack for different CPU modes
static uint8_t mips_kernel_stack[16384] __attribute__((aligned(16)));
static uint8_t mips_exception_stack[4096] __attribute__((aligned(16)));
static uint8_t mips_interrupt_stack[4096] __attribute__((aligned(16)));

// System state
static volatile bool mips_system_running = false;
static volatile uint64_t mips_system_uptime = 0;

// ============================================================================
// ENTRY POINT FUNCTIONS
// ============================================================================

void mips_entry_point(void)
{
    printf("MIPS: Entry point reached\n");
    
    // Set up initial stack pointer
    __asm__ volatile("move $sp, %0" : : "r"(mips_kernel_stack + sizeof(mips_kernel_stack)) : "sp");
    
    // Clear BSS section
    extern uint8_t __bss_start, __bss_end;
    memset(&__bss_start, 0, &__bss_end - &__bss_start);
    
    // Initialize basic hardware
    mips_early_init();
    
    // Jump to main
    mips_main();
}

void mips_exception_entry(void)
{
    printf("MIPS: Exception entry point\n");
    
    // Save current context
    mips_context_t context;
    mips_cpu_save_context(&context);
    
    // Handle exception based on cause
    uint64_t cause = mips_cpu_read_cp0_cause();
    uint64_t epc = mips_cpu_read_cp0_epc();
    
    printf("MIPS: Exception at EPC 0x%llx, Cause: 0x%llx\n", (unsigned long long)epc, (unsigned long long)cause);
    
    // Restore context and return
    mips_cpu_restore_context(&context);
}

void mips_interrupt_entry(void)
{
    // Save current context
    mips_context_t context;
    mips_cpu_save_context(&context);
    
    // Handle interrupt
    uint64_t cause = mips_cpu_read_cp0_cause();
    uint64_t status = mips_cpu_read_cp0_status();
    
    // Check which interrupt is pending
    uint64_t pending = cause & status & MIPS_STATUS_IP;
    
    if (pending & MIPS_STATUS_IP7) {
        // Timer interrupt
        mips_timer_context.system_tick++;
        mips_system_uptime++;
        
        // Clear timer interrupt
        mips_cpu_write_cp0_compare(mips_cpu_read_cp0_compare() + MIPS_CP0_COUNT_FREQ / 1000);
    }
    
    // Restore context and return
    mips_cpu_restore_context(&context);
}

void mips_syscall_entry(void)
{
    printf("MIPS: System call entry point\n");
    
    // Save current context
    mips_context_t context;
    mips_cpu_save_context(&context);
    
    // Handle system call
    uint64_t syscall_number = context.v0;
    
    printf("MIPS: System call %llu\n", (unsigned long long)syscall_number);
    
    // Restore context and return
    mips_cpu_restore_context(&context);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void mips_setup_stacks(void)
{
    printf("MIPS: Setting up stacks\n");
    
    // Set up exception stack
    __asm__ volatile("move $k0, %0" : : "r"(mips_exception_stack + sizeof(mips_exception_stack)) : "k0");
    
    // Set up interrupt stack
    __asm__ volatile("move $k1, %0" : : "r"(mips_interrupt_stack + sizeof(mips_interrupt_stack)) : "k1");
    
    printf("MIPS: Stacks setup complete\n");
}

void mips_setup_exception_vectors(void)
{
    printf("MIPS: Setting up exception vectors\n");
    
    // Set up exception vector base
    uint64_t vector_base = 0x80000000;
    
    // Write vector base to CP0
    mips_cpu_write_cp0_register(MIPS_CP0_CONTEXT, vector_base);
    
    printf("MIPS: Exception vectors setup complete\n");
}

void mips_setup_interrupts(void)
{
    printf("MIPS: Setting up interrupts\n");
    
    // Enable interrupts in CP0 Status
    uint64_t status = mips_cpu_read_cp0_status();
    status |= MIPS_STATUS_IE; // Enable interrupts
    status |= MIPS_STATUS_IP7; // Enable timer interrupt
    mips_cpu_write_cp0_status(status);
    
    // Set up timer interrupt
    uint64_t current_count = mips_cpu_read_cp0_count();
    uint64_t compare_value = current_count + MIPS_CP0_COUNT_FREQ / 1000; // 1ms
    mips_cpu_write_cp0_compare(compare_value);
    
    printf("MIPS: Interrupts setup complete\n");
}

void mips_setup_memory(void)
{
    printf("MIPS: Setting up memory\n");
    
    // Add basic memory regions
    mips_memory_region_t kernel_region = {
        .base_addr = MIPS_KERNEL_BASE,
        .size = MIPS_KERNEL_SIZE,
        .flags = 0x03, // Read/Write/Execute
        .domain = 0
    };
    
    mips_memory_region_t user_region = {
        .base_addr = MIPS_USER_BASE,
        .size = MIPS_USER_SIZE,
        .flags = 0x01, // Read/Write
        .domain = 1
    };
    
    mips_tlb_add_memory_region(&kernel_region);
    mips_tlb_add_memory_region(&user_region);
    
    printf("MIPS: Memory setup complete\n");
}

void mips_setup_timers(void)
{
    printf("MIPS: Setting up timers\n");
    
    // Initialize system timer
    mips_timer_context.timers[0].timer_id = 0;
    mips_timer_context.timers[0].frequency = MIPS_TIMER_FREQ;
    mips_timer_context.timers[0].mode = 0; // Periodic
    mips_timer_context.timers[0].value = 0;
    mips_timer_context.timers[0].reload_value = MIPS_TIMER_FREQ / 1000; // 1ms
    mips_timer_context.timers[0].callback = NULL;
    
    // Initialize periodic timer
    mips_timer_context.timers[1].timer_id = 1;
    mips_timer_context.timers[1].frequency = 1000; // 1kHz
    mips_timer_context.timers[1].mode = 1; // Periodic
    mips_timer_context.timers[1].value = 0;
    mips_timer_context.timers[1].reload_value = 1000;
    mips_timer_context.timers[1].callback = NULL;
    
    printf("MIPS: Timers setup complete\n");
}

void mips_setup_cache(void)
{
    printf("MIPS: Setting up cache\n");
    
    // Initialize cache information
    mips_cache_context.caches[0].cache_level = 0;
    mips_cache_context.caches[0].size = MIPS_L1I_CACHE_SIZE;
    mips_cache_context.caches[0].line_size = MIPS_L1_CACHE_LINE_SIZE;
    mips_cache_context.caches[0].associativity = 4;
    mips_cache_context.caches[0].write_policy = 0; // Write-through
    
    mips_cache_context.caches[1].cache_level = 1;
    mips_cache_context.caches[1].size = MIPS_L1D_CACHE_SIZE;
    mips_cache_context.caches[1].line_size = MIPS_L1_CACHE_LINE_SIZE;
    mips_cache_context.caches[1].associativity = 4;
    mips_cache_context.caches[1].write_policy = 1; // Write-back
    
    mips_cache_context.caches[2].cache_level = 2;
    mips_cache_context.caches[2].size = MIPS_L2_CACHE_SIZE;
    mips_cache_context.caches[2].line_size = MIPS_L2_CACHE_LINE_SIZE;
    mips_cache_context.caches[2].associativity = 8;
    mips_cache_context.caches[2].write_policy = 1; // Write-back
    
    printf("MIPS: Cache setup complete\n");
}

void mips_setup_security(void)
{
    printf("MIPS: Setting up security\n");
    
    // Enable secure boot if supported
    if (MIPS_SECURE_BOOT) {
        mips_security_enable_secure_boot();
    }
    
    // Enable memory protection
    if (MIPS_MEMORY_PROTECTION) {
        mips_security_enable_memory_protection();
    }
    
    // Enable ASLR
    if (MIPS_ASLR) {
        mips_security_enable_aslr();
    }
    
    printf("MIPS: Security setup complete\n");
}

void mips_setup_performance(void)
{
    printf("MIPS: Setting up performance monitoring\n");
    
    // Enable PMU if supported
    if (mips_performance_is_pmu_enabled()) {
        mips_performance_enable_pmu();
    }
    
    // Reset performance counters
    mips_performance_reset_counters();
    
    printf("MIPS: Performance monitoring setup complete\n");
}

void mips_setup_debug(void)
{
    printf("MIPS: Setting up debug support\n");
    
    // Enable debug if supported
    if (MIPS_DEBUG_MAX_BREAKPOINTS > 0) {
        mips_debug_enable();
    }
    
    printf("MIPS: Debug support setup complete\n");
}

void mips_setup_extensions(void)
{
    printf("MIPS: Setting up extensions\n");
    
    // Enable MSA if supported
    if (MIPS_ENABLE_MSA && mips_cpu_supports_msa()) {
        mips_msa_enable();
    }
    
    // Enable DSP if supported
    if (MIPS_ENABLE_DSP && mips_cpu_supports_dsp()) {
        mips_dsp_enable();
    }
    
    // Enable FPU if supported
    if (MIPS_ENABLE_FPU && mips_cpu_supports_fpu()) {
        mips_fpu_enable();
    }
    
    // Enable CRC if supported
    if (MIPS_ENABLE_CRC && mips_cpu_supports_crc()) {
        mips_crc_enable();
    }
    
    printf("MIPS: Extensions setup complete\n");
}

// ============================================================================
// SYSTEM CONTROL FUNCTIONS
// ============================================================================

void mips_start_system(void)
{
    printf("MIPS: Starting system\n");
    
    mips_system_running = true;
    mips_system_uptime = 0;
    
    // Start timers
    mips_timer_start(0); // System timer
    mips_timer_start(1); // Periodic timer
    
    printf("MIPS: System started\n");
}

void mips_stop_system(void)
{
    printf("MIPS: Stopping system\n");
    
    mips_system_running = false;
    
    // Stop timers
    mips_timer_stop(0);
    mips_timer_stop(1);
    
    printf("MIPS: System stopped\n");
}

bool mips_is_system_running(void)
{
    return mips_system_running;
}

uint64_t mips_get_system_uptime(void)
{
    return mips_system_uptime;
}

void mips_system_tick(void)
{
    if (mips_system_running) {
        mips_system_uptime++;
        
        // Perform periodic system tasks
        if (mips_system_uptime % 1000 == 0) {
            // Every second
            mips_system_monitor();
        }
    }
}

// ============================================================================
// ERROR HANDLING FUNCTIONS
// ============================================================================

void mips_handle_error(uint32_t error_code, const char *error_message)
{
    printf("MIPS: ERROR %u: %s\n", error_code, error_message);
    
    // Log error
    // Send error to error handler
    // Potentially restart system
    
    if (error_code >= 0x1000) {
        // Critical error, restart system
        printf("MIPS: Critical error detected, restarting system\n");
        mips_reset();
    }
}

void mips_handle_panic(const char *panic_message)
{
    printf("MIPS: PANIC: %s\n", panic_message);
    
    // Disable interrupts
    mips_cpu_disable_interrupts();
    
    // Dump system state
    printf("MIPS: System state dump:\n");
    printf("  CPU ID: %u\n", mips_cpu_get_id());
    printf("  CPU Revision: %u\n", mips_cpu_get_revision());
    printf("  CPU Frequency: %llu Hz\n", (unsigned long long)mips_cpu_get_frequency());
    printf("  System Uptime: %llu ticks\n", (unsigned long long)mips_system_uptime);
    
    // Halt system
    printf("MIPS: System halted due to panic\n");
    mips_cpu_halt();
}

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

void mips_initialize_hardware(void)
{
    printf("MIPS: Initializing hardware\n");
    
    // Set up stacks
    mips_setup_stacks();
    
    // Set up exception vectors
    mips_setup_exception_vectors();
    
    // Set up interrupts
    mips_setup_interrupts();
    
    // Set up memory
    mips_setup_memory();
    
    // Set up timers
    mips_setup_timers();
    
    // Set up cache
    mips_setup_cache();
    
    // Set up security
    mips_setup_security();
    
    // Set up performance monitoring
    mips_setup_performance();
    
    // Set up debug support
    mips_setup_debug();
    
    // Set up extensions
    mips_setup_extensions();
    
    printf("MIPS: Hardware initialization complete\n");
}

void mips_initialize_software(void)
{
    printf("MIPS: Initializing software\n");
    
    // Initialize device drivers
    mips_device_drivers_init();
    
    // Initialize file system
    mips_file_system_init();
    
    // Initialize network stack
    mips_network_stack_init();
    
    // Initialize user space
    mips_user_space_init();
    
    printf("MIPS: Software initialization complete\n");
}

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================

int main(void)
{
    printf("MIPS: Main entry point\n");
    
    // Initialize hardware
    mips_initialize_hardware();
    
    // Initialize software
    mips_initialize_software();
    
    // Start system
    mips_start_system();
    
    // Run system tests
    mips_system_test();
    
    // Enter main loop
    mips_main_loop();
    
    // Should never reach here
    return 0;
}
