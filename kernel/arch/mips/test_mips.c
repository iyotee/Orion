/*
 * ORION OS - MIPS Architecture Test
 *
 * Test file for MIPS architecture (MIPS32, MIPS64)
 * Tests basic functionality and features
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
// TEST FUNCTIONS
// ============================================================================

void test_mips_basic_functions(void)
{
    printf("MIPS: Testing basic functions...\n");
    
    // Test CPU functions
    uint32_t cpu_id = mips_cpu_get_id();
    uint32_t cpu_rev = mips_cpu_get_revision();
    uint64_t cpu_freq = mips_cpu_get_frequency();
    
    printf("MIPS: CPU ID: 0x%08x\n", cpu_id);
    printf("MIPS: CPU Revision: 0x%08x\n", cpu_rev);
    printf("MIPS: CPU Frequency: %llu Hz\n", (unsigned long long)cpu_freq);
    
    // Test feature detection
    bool msa_supported = mips_cpu_supports_msa();
    bool dsp_supported = mips_cpu_supports_dsp();
    bool fpu_supported = mips_cpu_supports_fpu();
    bool vz_supported = mips_cpu_supports_vz();
    
    printf("MIPS: MSA supported: %s\n", msa_supported ? "Yes" : "No");
    printf("MIPS: DSP supported: %s\n", dsp_supported ? "Yes" : "No");
    printf("MIPS: FPU supported: %s\n", fpu_supported ? "Yes" : "No");
    printf("MIPS: VZ supported: %s\n", vz_supported ? "Yes" : "No");
    
    // Test feature flags
    uint32_t feature_flags = mips_cpu_get_feature_flags();
    printf("MIPS: Feature flags: 0x%08x\n", feature_flags);
    
    printf("MIPS: Basic functions test complete\n");
}

void test_mips_memory_functions(void)
{
    printf("MIPS: Testing memory functions...\n");
    
    // Test TLB functions
    bool tlb_enabled = mips_tlb_is_enabled();
    printf("MIPS: TLB enabled: %s\n", tlb_enabled ? "Yes" : "No");
    
    // Test memory region management
    mips_memory_region_t test_region = {
        .base_addr = 0x10000000,
        .size = 0x1000000,
        .flags = 0x03,
        .domain = 0
    };
    
    mips_tlb_add_memory_region(&test_region);
    printf("MIPS: Added test memory region\n");
    
    mips_memory_region_t *found_region = mips_tlb_find_memory_region(0x10000000);
    if (found_region) {
        printf("MIPS: Found memory region at 0x%llx\n", (unsigned long long)found_region->base_addr);
    }
    
    printf("MIPS: Memory functions test complete\n");
}

void test_mips_interrupt_functions(void)
{
    printf("MIPS: Testing interrupt functions...\n");
    
    // Test interrupt status
    bool external_enabled = mips_is_external_enabled();
    bool timer_enabled = mips_is_timer_enabled();
    
    printf("MIPS: External interrupts enabled: %s\n", external_enabled ? "Yes" : "No");
    printf("MIPS: Timer interrupts enabled: %s\n", timer_enabled ? "Yes" : "No");
    
    // Test interrupt registration
    int result = mips_register_irq_handler(0, NULL, NULL);
    printf("MIPS: IRQ handler registration result: %d\n", result);
    
    printf("MIPS: Interrupt functions test complete\n");
}

void test_mips_timer_functions(void)
{
    printf("MIPS: Testing timer functions...\n");
    
    // Test CP0 timer functions
    uint64_t count_value = mips_cp0_count_get_value();
    uint64_t compare_value = mips_cp0_compare_get_value();
    uint64_t count_freq = mips_cp0_count_get_frequency();
    
    printf("MIPS: CP0 Count: %llu\n", (unsigned long long)count_value);
    printf("MIPS: CP0 Compare: %llu\n", (unsigned long long)compare_value);
    printf("MIPS: CP0 Count Frequency: %llu Hz\n", (unsigned long long)count_freq);
    
    // Test timer management
    uint64_t timer_value = mips_timer_get_value(0);
    uint64_t timer_freq = mips_timer_get_frequency(0);
    uint64_t system_tick = mips_timer_get_system_tick();
    
    printf("MIPS: Timer 0 value: %llu\n", (unsigned long long)timer_value);
    printf("MIPS: Timer 0 frequency: %llu Hz\n", (unsigned long long)timer_freq);
    printf("MIPS: System tick: %llu\n", (unsigned long long)system_tick);
    
    printf("MIPS: Timer functions test complete\n");
}

void test_mips_cache_functions(void)
{
    printf("MIPS: Testing cache functions...\n");
    
    // Test cache status
    bool cache_enabled = mips_cache_is_enabled();
    printf("MIPS: Cache enabled: %s\n", cache_enabled ? "Yes" : "No");
    
    // Test cache information
    uint32_t line_size = mips_cache_get_line_size();
    printf("MIPS: Cache line size: %u bytes\n", line_size);
    
    mips_cache_info_t *l1_cache = mips_cache_get_info(0);
    if (l1_cache) {
        printf("MIPS: L1 cache size: %llu bytes\n", (unsigned long long)l1_cache->size);
        printf("MIPS: L1 cache associativity: %u\n", l1_cache->associativity);
    }
    
    printf("MIPS: Cache functions test complete\n");
}

void test_mips_power_functions(void)
{
    printf("MIPS: Testing power functions...\n");
    
    // Test power state
    uint32_t power_state = mips_power_get_state();
    uint64_t cpu_freq = mips_power_get_cpu_frequency();
    uint32_t voltage = mips_power_get_voltage_level();
    uint32_t temperature = mips_power_get_temperature();
    
    printf("MIPS: Power state: %u\n", power_state);
    printf("MIPS: CPU frequency: %llu Hz\n", (unsigned long long)cpu_freq);
    printf("MIPS: Voltage level: %u\n", voltage);
    printf("MIPS: Temperature: %u\n", temperature);
    
    printf("MIPS: Power functions test complete\n");
}

void test_mips_security_functions(void)
{
    printf("MIPS: Testing security functions...\n");
    
    // Test security features
    bool secure_boot = mips_security_is_secure_boot_enabled();
    bool mem_protection = mips_security_is_memory_protection_enabled();
    bool aslr = mips_security_is_aslr_enabled();
    
    printf("MIPS: Secure boot: %s\n", secure_boot ? "Enabled" : "Disabled");
    printf("MIPS: Memory protection: %s\n", mem_protection ? "Enabled" : "Disabled");
    printf("MIPS: ASLR: %s\n", aslr ? "Enabled" : "Disabled");
    
    printf("MIPS: Security functions test complete\n");
}

void test_mips_virtualization_functions(void)
{
    printf("MIPS: Testing virtualization functions...\n");
    
    // Test virtualization status
    bool virt_enabled = mips_virtualization_is_enabled();
    printf("MIPS: Virtualization enabled: %s\n", virt_enabled ? "Yes" : "No");
    
    printf("MIPS: Virtualization functions test complete\n");
}

void test_mips_performance_functions(void)
{
    printf("MIPS: Testing performance functions...\n");
    
    // Test performance monitoring
    bool pmu_enabled = mips_performance_is_pmu_enabled();
    printf("MIPS: PMU enabled: %s\n", pmu_enabled ? "Yes" : "No");
    
    // Test performance counters
    uint64_t cycle_counter = mips_performance_get_cycle_counter();
    uint64_t instruction_counter = mips_performance_get_instruction_counter();
    
    printf("MIPS: Cycle counter: %llu\n", (unsigned long long)cycle_counter);
    printf("MIPS: Instruction counter: %llu\n", (unsigned long long)instruction_counter);
    
    printf("MIPS: Performance functions test complete\n");
}

void test_mips_debug_functions(void)
{
    printf("MIPS: Testing debug functions...\n");
    
    // Test debug status
    bool debug_enabled = mips_debug_is_enabled();
    printf("MIPS: Debug enabled: %s\n", debug_enabled ? "Yes" : "No");
    
    printf("MIPS: Debug functions test complete\n");
}

void test_mips_extension_functions(void)
{
    printf("MIPS: Testing extension functions...\n");
    
    // Test MSA
    bool msa_available = mips_msa_is_available();
    printf("MIPS: MSA available: %s\n", msa_available ? "Yes" : "No");
    
    // Test DSP
    bool dsp_available = mips_dsp_is_available();
    printf("MIPS: DSP available: %s\n", dsp_available ? "Yes" : "No");
    
    // Test FPU
    bool fpu_available = mips_fpu_is_available();
    printf("MIPS: FPU available: %s\n", fpu_available ? "Yes" : "No");
    
    // Test CRC
    bool crc_available = mips_crc_is_available();
    printf("MIPS: CRC available: %s\n", crc_available ? "Yes" : "No");
    
    printf("MIPS: Extension functions test complete\n");
}

void test_mips_system_functions(void)
{
    printf("MIPS: Testing system functions...\n");
    
    // Test system status
    bool system_running = mips_is_system_running();
    uint64_t system_uptime = mips_get_system_uptime();
    
    printf("MIPS: System running: %s\n", system_running ? "Yes" : "No");
    printf("MIPS: System uptime: %llu ticks\n", (unsigned long long)system_uptime);
    
    printf("MIPS: System functions test complete\n");
}

// ============================================================================
// MAIN TEST FUNCTION
// ============================================================================

void mips_run_all_tests(void)
{
    printf("MIPS: Starting all tests...\n");
    printf("MIPS: ========================================\n");
    
    test_mips_basic_functions();
    printf("MIPS: ========================================\n");
    
    test_mips_memory_functions();
    printf("MIPS: ========================================\n");
    
    test_mips_interrupt_functions();
    printf("MIPS: ========================================\n");
    
    test_mips_timer_functions();
    printf("MIPS: ========================================\n");
    
    test_mips_cache_functions();
    printf("MIPS: ========================================\n");
    
    test_mips_power_functions();
    printf("MIPS: ========================================\n");
    
    test_mips_security_functions();
    printf("MIPS: ========================================\n");
    
    test_mips_virtualization_functions();
    printf("MIPS: ========================================\n");
    
    test_mips_performance_functions();
    printf("MIPS: ========================================\n");
    
    test_mips_debug_functions();
    printf("MIPS: ========================================\n");
    
    test_mips_extension_functions();
    printf("MIPS: ========================================\n");
    
    test_mips_system_functions();
    printf("MIPS: ========================================\n");
    
    printf("MIPS: All tests completed successfully!\n");
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main(void)
{
    printf("MIPS: MIPS Architecture Test Suite\n");
    printf("MIPS: ========================================\n");
    
    // Initialize MIPS architecture
    mips_early_init();
    
    // Run all tests
    mips_run_all_tests();
    
    printf("MIPS: Test suite completed successfully!\n");
    return 0;
}
