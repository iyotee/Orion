/*
 * ORION OS - POWER Architecture Test Suite
 *
 * Comprehensive test suite for POWER architecture support
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
#include <assert.h>

// ============================================================================
// TEST FUNCTIONS
// ============================================================================

static void test_cpu_detection(void)
{
    printf("POWER: Testing CPU detection...\n");
    
    int ret = power_cpu_detect_features();
    assert(ret == 0);
    
    printf("POWER: CPU detection test passed\n");
}

static void test_mmu_basic(void)
{
    printf("POWER: Testing MMU basic functionality...\n");
    
    int ret = power_mmu_init();
    assert(ret == 0);
    
    // Test page mapping
    power_vaddr_t vaddr = 0x1000;
    power_paddr_t paddr = 0x2000;
    uint64_t flags = 0x3; // Read/Write
    
    ret = power_mmu_map_page(vaddr, paddr, flags);
    assert(ret == 0);
    
    // Test page retrieval
    power_paddr_t retrieved_paddr;
    uint64_t retrieved_flags;
    ret = power_mmu_get_page(vaddr, &retrieved_paddr, &retrieved_flags);
    assert(ret == 0);
    assert(retrieved_paddr == paddr);
    assert(retrieved_flags == flags);
    
    // Test page unmapping
    ret = power_mmu_unmap_page(vaddr);
    assert(ret == 0);
    
    printf("POWER: MMU basic test passed\n");
}

static void test_interrupt_basic(void)
{
    printf("POWER: Testing interrupt basic functionality...\n");
    
    int ret = power_interrupt_init();
    assert(ret == 0);
    
    // Test interrupt enable/disable
    ret = power_interrupt_enable(0);
    assert(ret == 0);
    
    ret = power_interrupt_disable(0);
    assert(ret == 0);
    
    printf("POWER: Interrupt basic test passed\n");
}

static void test_timer_basic(void)
{
    printf("POWER: Testing timer basic functionality...\n");
    
    int ret = power_timer_init();
    assert(ret == 0);
    
    // Test timer frequency setting
    ret = power_timer_set_frequency(1000000); // 1MHz
    assert(ret == 0);
    
    uint64_t freq;
    ret = power_timer_get_frequency(&freq);
    assert(ret == 0);
    assert(freq == 1000000);
    
    // Test timer period setting
    ret = power_timer_set_period(1000000); // 1ms
    assert(ret == 0);
    
    uint64_t period;
    ret = power_timer_get_period(&period);
    assert(ret == 0);
    assert(period == 1000000);
    
    printf("POWER: Timer basic test passed\n");
}

static void test_cache_basic(void)
{
    printf("POWER: Testing cache basic functionality...\n");
    
    int ret = power_cache_init();
    assert(ret == 0);
    
    // Test cache flush operations
    ret = power_cache_flush_l1i();
    assert(ret == 0);
    
    ret = power_cache_flush_l1d();
    assert(ret == 0);
    
    ret = power_cache_flush_all();
    assert(ret == 0);
    
    printf("POWER: Cache basic test passed\n");
}

static void test_vector_support(void)
{
    printf("POWER: Testing vector support...\n");
    
    int ret = power_vector_init();
    assert(ret == 0);
    
    // Test VSX initialization
    ret = power_vsx_init();
    assert(ret == 0);
    
    // Test AltiVec initialization
    ret = power_altivec_init();
    assert(ret == 0);
    
    printf("POWER: Vector support test passed\n");
}

static void test_security_features(void)
{
    printf("POWER: Testing security features...\n");
    
    int ret = power_security_init();
    assert(ret == 0);
    
    // Test secure boot verification
    ret = power_secure_boot_verify();
    assert(ret == 0);
    
    printf("POWER: Security features test passed\n");
}

static void test_pmu_basic(void)
{
    printf("POWER: Testing PMU basic functionality...\n");
    
    int ret = power_pmu_init();
    assert(ret == 0);
    
    // Test PMU counter operations
    ret = power_pmu_start_counter(0, 1);
    assert(ret == 0);
    
    uint64_t value;
    ret = power_pmu_read_counter(0, &value);
    assert(ret == 0);
    
    ret = power_pmu_stop_counter(0);
    assert(ret == 0);
    
    ret = power_pmu_reset_counter(0);
    assert(ret == 0);
    
    printf("POWER: PMU basic test passed\n");
}

static void test_power_management(void)
{
    printf("POWER: Testing power management...\n");
    
    int ret = power_power_init();
    assert(ret == 0);
    
    // Test power mode operations
    ret = power_power_set_mode(1);
    assert(ret == 0);
    
    uint64_t mode;
    ret = power_power_get_mode(&mode);
    assert(ret == 0);
    assert(mode == 1);
    
    printf("POWER: Power management test passed\n");
}

static void test_numa_support(void)
{
    printf("POWER: Testing NUMA support...\n");
    
    int ret = power_numa_init();
    assert(ret == 0);
    
    // Test NUMA operations
    uint64_t node_id;
    ret = power_numa_get_node_id(&node_id);
    assert(ret == 0);
    
    uint64_t local_memory;
    ret = power_numa_get_local_memory(&local_memory);
    assert(ret == 0);
    
    printf("POWER: NUMA support test passed\n");
}

static void test_advanced_features(void)
{
    printf("POWER: Testing advanced features...\n");
    
    int ret = power_advanced_features_test();
    assert(ret == 0);
    
    printf("POWER: Advanced features test passed\n");
}

// ============================================================================
// MAIN TEST FUNCTION
// ============================================================================

int power_run_all_tests(void)
{
    printf("POWER: Starting comprehensive test suite\n");
    
    // Run all tests
    test_cpu_detection();
    test_mmu_basic();
    test_interrupt_basic();
    test_timer_basic();
    test_cache_basic();
    test_vector_support();
    test_security_features();
    test_pmu_basic();
    test_power_management();
    test_numa_support();
    test_advanced_features();
    
    printf("POWER: All tests passed successfully!\n");
    return 0;
}

// ============================================================================
// INDIVIDUAL TEST RUNNERS
// ============================================================================

int power_test_cpu(void)
{
    printf("POWER: Running CPU tests...\n");
    test_cpu_detection();
    return 0;
}

int power_test_mmu(void)
{
    printf("POWER: Running MMU tests...\n");
    test_mmu_basic();
    return 0;
}

int power_test_interrupt(void)
{
    printf("POWER: Running interrupt tests...\n");
    test_interrupt_basic();
    return 0;
}

int power_test_timer(void)
{
    printf("POWER: Running timer tests...\n");
    test_timer_basic();
    return 0;
}

int power_test_cache(void)
{
    printf("POWER: Running cache tests...\n");
    test_cache_basic();
    return 0;
}

int power_test_vector(void)
{
    printf("POWER: Running vector tests...\n");
    test_vector_support();
    return 0;
}

int power_test_security(void)
{
    printf("POWER: Running security tests...\n");
    test_security_features();
    return 0;
}

int power_test_pmu(void)
{
    printf("POWER: Running PMU tests...\n");
    test_pmu_basic();
    return 0;
}

int power_test_power(void)
{
    printf("POWER: Running power management tests...\n");
    test_power_management();
    return 0;
}

int power_test_numa(void)
{
    printf("POWER: Running NUMA tests...\n");
    test_numa_support();
    return 0;
}

int power_test_advanced(void)
{
    printf("POWER: Running advanced features tests...\n");
    test_advanced_features();
    return 0;
}

// ============================================================================
// BENCHMARK TESTS
// ============================================================================

int power_benchmark_vector_operations(void)
{
    printf("POWER: Benchmarking vector operations...\n");
    
    // Test data
    uint64_t a[2] = {1, 2};
    uint64_t b[2] = {3, 4};
    uint64_t result[2];
    
    // Benchmark VSX operations
    const int iterations = 1000000;
    
    for (int i = 0; i < iterations; i++) {
        power_vsx_vector_add_128(a, b, result);
        power_vsx_vector_mul_128(a, b, result);
    }
    
    printf("POWER: Vector operations benchmark completed (%d iterations)\n", iterations);
    return 0;
}

int power_benchmark_cache_operations(void)
{
    printf("POWER: Benchmarking cache operations...\n");
    
    const int iterations = 100000;
    
    for (int i = 0; i < iterations; i++) {
        power_cache_flush_l1d();
        power_cache_invalidate_l1d();
    }
    
    printf("POWER: Cache operations benchmark completed (%d iterations)\n", iterations);
    return 0;
}

int power_benchmark_timer_operations(void)
{
    printf("POWER: Benchmarking timer operations...\n");
    
    const int iterations = 100000;
    
    for (int i = 0; i < iterations; i++) {
        power_timer_read();
        power_timer_read_tb();
    }
    
    printf("POWER: Timer operations benchmark completed (%d iterations)\n", iterations);
    return 0;
}

// ============================================================================
// STRESS TESTS
// ============================================================================

int power_stress_test_mmu(void)
{
    printf("POWER: Running MMU stress test...\n");
    
    const int iterations = 10000;
    
    for (int i = 0; i < iterations; i++) {
        power_vaddr_t vaddr = 0x1000 + (i * 0x1000);
        power_paddr_t paddr = 0x2000 + (i * 0x1000);
        uint64_t flags = 0x3;
        
        int ret = power_mmu_map_page(vaddr, paddr, flags);
        if (ret != 0) {
            printf("POWER: MMU stress test failed at iteration %d\n", i);
            return ret;
        }
        
        ret = power_mmu_unmap_page(vaddr);
        if (ret != 0) {
            printf("POWER: MMU stress test failed at iteration %d\n", i);
            return ret;
        }
    }
    
    printf("POWER: MMU stress test completed successfully (%d iterations)\n", iterations);
    return 0;
}

int power_stress_test_interrupts(void)
{
    printf("POWER: Running interrupt stress test...\n");
    
    const int iterations = 10000;
    
    for (int i = 0; i < iterations; i++) {
        int ret = power_interrupt_enable(i % 10);
        if (ret != 0) {
            printf("POWER: Interrupt stress test failed at iteration %d\n", i);
            return ret;
        }
        
        ret = power_interrupt_disable(i % 10);
        if (ret != 0) {
            printf("POWER: Interrupt stress test failed at iteration %d\n", i);
            return ret;
        }
    }
    
    printf("POWER: Interrupt stress test completed successfully (%d iterations)\n", iterations);
    return 0;
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main(void)
{
    printf("POWER: POWER Architecture Test Suite\n");
    printf("POWER: =============================\n");
    
    // Run comprehensive test suite
    int ret = power_run_all_tests();
    if (ret != 0) {
        printf("POWER: Test suite failed with error %d\n", ret);
        return ret;
    }
    
    // Run benchmark tests
    power_benchmark_vector_operations();
    power_benchmark_cache_operations();
    power_benchmark_timer_operations();
    
    // Run stress tests
    power_stress_test_mmu();
    power_stress_test_interrupts();
    
    printf("POWER: All tests and benchmarks completed successfully!\n");
    return 0;
}
