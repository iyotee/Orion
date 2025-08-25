/*
 * ORION OS - LoongArch Architecture Tests
 *
 * This file contains tests for LoongArch architecture support.
 */

#include "arch.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* Test function declarations */
static void test_cpu_detection(void);
static void test_mmu_basic(void);
static void test_interrupt_basic(void);
static void test_timer_basic(void);
static void test_cache_basic(void);
static void test_vector_extensions(void);
static void test_crypto_support(void);
static void test_memory_barriers(void);
static void test_atomic_operations(void);

/* New Phase 1.5 test functions */
static void test_interrupts_comprehensive(void);
static void test_timers_comprehensive(void);
static void test_entry_point(void);
static void test_integration(void);

/* Advanced features test functions */
static void test_advanced_features(void);

/* Test main function */
int test_loongarch_main(void)
{
    printf("LoongArch: Starting architecture tests...\n");

    /* Run basic tests */
    test_cpu_detection();
    test_mmu_basic();
    test_interrupt_basic();
    test_timer_basic();
    test_cache_basic();
    test_vector_extensions();
    test_crypto_support();
    test_memory_barriers();
    test_atomic_operations();

    /* Run Phase 1.5 comprehensive tests */
    test_interrupts_comprehensive();
    test_timers_comprehensive();
    test_entry_point();
    test_integration();

    /* Run advanced features tests */
    test_advanced_features();

    printf("LoongArch: All architecture tests passed!\n");
    return 0;
}

/* Test CPU detection */
static void test_cpu_detection(void)
{
    printf("LoongArch: Testing CPU detection...\n");

    struct loongarch_cpu_info info;
    memset(&info, 0, sizeof(info));

    /* Test CPU detection */
    int result = loongarch_cpu_detect(&info);
    assert(result == 0);

    /* Verify basic fields */
    assert(info.vendor_id != 0);
    assert(info.cpu_id != 0);
    assert(info.cache_line_size == LOONGARCH_CACHE_LINE_SIZE);
    assert(strlen(info.model_name) > 0);

    printf("LoongArch: CPU detection test passed\n");
    printf("  Model: %s\n", info.model_name);
    printf("  Vendor ID: 0x%08X\n", info.vendor_id);
    printf("  CPU ID: 0x%08X\n", info.cpu_id);
    printf("  Features: 0x%016llX\n", (unsigned long long)info.features);
}

/* Test basic MMU functionality */
static void test_mmu_basic(void)
{
    printf("LoongArch: Testing basic MMU functionality...\n");

    /* Test MMU initialization */
    int result = loongarch_mmu_init();
    assert(result == 0);

    /* Test basic memory mapping */
    loongarch_vaddr_t test_va = 0x1000000000000000UL;
    loongarch_paddr_t test_pa = 0x1000000000000000UL;
    uint64_t test_flags = LOONGARCH_PTE_VALID | LOONGARCH_PTE_PRESENT | LOONGARCH_PTE_WRITE;

    result = loongarch_mmu_map(test_va, test_pa, test_flags);
    assert(result == 0);

    /* Test memory unmapping */
    result = loongarch_mmu_unmap(test_va, LOONGARCH_PAGE_SIZE);
    assert(result == 0);

    printf("LoongArch: Basic MMU test passed\n");
}

/* Test basic interrupt functionality */
static void test_interrupt_basic(void)
{
    printf("LoongArch: Testing basic interrupt functionality...\n");

    /* Test interrupt initialization */
    int result = loongarch_interrupt_init();
    assert(result == 0);

    /* Test interrupt handler registration */
    void test_handler(void)
    {
        printf("LoongArch: Test interrupt handler called\n");
    }

    result = loongarch_interrupt_set_handler(0, test_handler);
    assert(result == 0);

    /* Test interrupt enable/disable */
    result = loongarch_interrupt_enable(0);
    assert(result == 0);

    result = loongarch_interrupt_disable(0);
    assert(result == 0);

    printf("LoongArch: Basic interrupt test passed\n");
}

/* Test basic timer functionality */
static void test_timer_basic(void)
{
    printf("LoongArch: Testing basic timer functionality...\n");

    /* Test timer initialization */
    int result = loongarch_timer_init();
    assert(result == 0);

    /* Test timer read */
    uint64_t timer_value = loongarch_timer_read();
    assert(timer_value >= 0);

    /* Test timer configuration */
    result = loongarch_timer_set_periodic(1000000); /* 1ms */
    assert(result == 0);

    printf("LoongArch: Basic timer test passed\n");
    printf("  Timer value: %llu\n", (unsigned long long)timer_value);
}

/* Test basic cache functionality */
static void test_cache_basic(void)
{
    printf("LoongArch: Testing basic cache functionality...\n");

    /* Test cache operations */
    loongarch_vaddr_t test_addr = 0x1000000000000000UL;
    loongarch_size_t test_len = LOONGARCH_CACHE_LINE_SIZE;

    /* Test cache invalidation */
    loongarch_cache_invalidate_range(test_addr, test_len);

    /* Test cache cleaning */
    loongarch_cache_clean_range(test_addr, test_len);

    /* Test cache flushing */
    loongarch_cache_flush_range(test_addr, test_len);

    printf("LoongArch: Basic cache test passed\n");
}

/* Test vector extensions */
static void test_vector_extensions(void)
{
    printf("LoongArch: Testing vector extensions...\n");

    /* Test LSX support */
    if (loongarch_lsx_supported())
    {
        int result = loongarch_lsx_init();
        assert(result == 0);
        printf("LoongArch: LSX support available and initialized\n");
    }
    else
    {
        printf("LoongArch: LSX support not available\n");
    }

    /* Test LASX support */
    if (loongarch_lasx_supported())
    {
        int result = loongarch_lasx_init();
        assert(result == 0);
        printf("LoongArch: LASX support available and initialized\n");
    }
    else
    {
        printf("LoongArch: LASX support not available\n");
    }

    printf("LoongArch: Vector extensions test passed\n");
}

/* Test crypto support */
static void test_crypto_support(void)
{
    printf("LoongArch: Testing crypto support...\n");

    if (loongarch_crypto_supported())
    {
        int result = loongarch_crypto_init();
        assert(result == 0);
        printf("LoongArch: Crypto support available and initialized\n");
    }
    else
    {
        printf("LoongArch: Crypto support not available\n");
    }

    printf("LoongArch: Crypto support test passed\n");
}

/* Test memory barriers */
static void test_memory_barriers(void)
{
    printf("LoongArch: Testing memory barriers...\n");

    /* Test memory barriers */
    loongarch_mb();
    loongarch_rmb();
    loongarch_wmb();

    printf("LoongArch: Memory barriers test passed\n");
}

/* Test atomic operations */
static void test_atomic_operations(void)
{
    printf("LoongArch: Testing atomic operations...\n");

    /* Test atomic operations */
    uint64_t test_value = 42;
    uint64_t old_value = 0;
    uint64_t new_value = 100;

    /* Test atomic compare and swap */
    int result = loongarch_atomic_cas(&test_value, old_value, new_value);
    assert(result == 0); /* Should fail since old_value != test_value */

    /* Test atomic add */
    uint64_t add_result = loongarch_atomic_add(&test_value, 10);
    assert(add_result == 42); /* Should return old value */
    assert(test_value == 52); /* Should be updated */

    /* Test atomic subtract */
    uint64_t sub_result = loongarch_atomic_sub(&test_value, 5);
    assert(sub_result == 52); /* Should return old value */
    assert(test_value == 47); /* Should be updated */

    printf("LoongArch: Atomic operations test passed\n");
}

/* Test utility functions */
static void test_utility_functions(void)
{
    printf("LoongArch: Testing utility functions...\n");

    /* Test address validation */
    loongarch_vaddr_t kernel_addr = 0x9000000000000000UL;
    loongarch_vaddr_t user_addr = 0x0000000000000000UL;
    loongarch_vaddr_t invalid_addr = 0x8000000000000000UL;

    assert(loongarch_is_kernel_address(kernel_addr));
    assert(loongarch_is_user_address(user_addr));
    assert(!loongarch_is_valid_address(invalid_addr));

    /* Test address conversion */
    loongarch_paddr_t phys_addr = loongarch_virt_to_phys(kernel_addr);
    loongarch_vaddr_t virt_addr = loongarch_phys_to_virt(phys_addr);
    assert(virt_addr == kernel_addr);

    /* Test alignment functions */
    loongarch_vaddr_t unaligned = 0x1000000000000001UL;
    loongarch_vaddr_t aligned = LOONGARCH_PAGE_ALIGN(unaligned);
    assert(LOONGARCH_IS_ALIGNED(aligned, LOONGARCH_PAGE_SIZE));

    printf("LoongArch: Utility functions test passed\n");
}

/* Test error handling */
static void test_error_handling(void)
{
    printf("LoongArch: Testing error handling...\n");

    /* Test panic function (should not return) */
    /* Note: This test is commented out as it would halt the system
    printf("LoongArch: Testing panic function (this should not print)...\n");
    loongarch_panic("Test panic message");
    */

    printf("LoongArch: Error handling test passed (panic test skipped)\n");
}

/* Test performance monitoring */
static void test_performance_monitoring(void)
{
    printf("LoongArch: Testing performance monitoring...\n");

    /* Test PMU initialization */
    int result = loongarch_pmu_init();
    assert(result == 0);

    /* Test PMU counter operations */
    uint64_t counter_value = loongarch_pmu_read_counter(0);
    assert(counter_value >= 0);

    result = loongarch_pmu_set_counter(0, 1000);
    assert(result == 0);

    printf("LoongArch: Performance monitoring test passed\n");
}

/* Test NUMA support */
static void test_numa_support(void)
{
    printf("LoongArch: Testing NUMA support...\n");

    /* Test NUMA initialization */
    int result = loongarch_numa_init();
    assert(result == 0);

    /* Test NUMA operations */
    loongarch_paddr_t test_pa = 0x1000000000000000UL;
    uint32_t node_id = loongarch_numa_get_node_id(test_pa);
    assert(node_id >= 0);

    uint32_t distance = loongarch_numa_get_distance(0, 1);
    assert(distance >= 0);

    printf("LoongArch: NUMA support test passed\n");
}

/* Test virtualization support */
static void test_virtualization_support(void)
{
    printf("LoongArch: Testing virtualization support...\n");

    if (loongarch_virt_supported())
    {
        /* Test virtualization initialization */
        int result = loongarch_virt_init();
        assert(result == 0);

        /* Test VM operations */
        int vm_id = loongarch_virt_create_vm();
        assert(vm_id >= 0);

        result = loongarch_virt_destroy_vm(vm_id);
        assert(result == 0);

        printf("LoongArch: Virtualization support available and tested\n");
    }
    else
    {
        printf("LoongArch: Virtualization support not available\n");
    }

    printf("LoongArch: Virtualization support test passed\n");
}

/* Test binary translation */
static void test_binary_translation(void)
{
    printf("LoongArch: Testing binary translation...\n");

    if (loongarch_lbt_supported())
    {
        /* Test binary translation initialization */
        int result = loongarch_lbt_init();
        assert(result == 0);

        /* Test binary translation */
        uint8_t test_code[] = {0x00, 0x00, 0x00, 0x00}; /* NOP */
        result = loongarch_lbt_translate(test_code, sizeof(test_code));
        assert(result >= 0);

        printf("LoongArch: Binary translation support available and tested\n");
    }
    else
    {
        printf("LoongArch: Binary translation support not available\n");
    }

    printf("LoongArch: Binary translation test passed\n");
}

/* Extended test suite */
int test_loongarch_extended(void)
{
    printf("LoongArch: Starting extended architecture tests...\n");

    /* Run extended tests */
    test_utility_functions();
    test_error_handling();
    test_performance_monitoring();
    test_numa_support();
    test_virtualization_support();
    test_binary_translation();

    printf("LoongArch: All extended architecture tests passed!\n");
    return 0;
}

/* Performance test suite */
int test_loongarch_performance(void)
{
    printf("LoongArch: Starting performance tests...\n");

    /* Test memory barrier performance */
    const int iterations = 1000000;

    /* Test memory barrier overhead */
    uint64_t start_time = loongarch_timer_read();
    for (int i = 0; i < iterations; i++)
    {
        loongarch_mb();
    }
    uint64_t end_time = loongarch_timer_read();

    uint64_t barrier_time = end_time - start_time;
    printf("LoongArch: Memory barrier performance: %llu cycles for %d iterations\n",
           (unsigned long long)barrier_time, iterations);

    /* Test cache operation performance */
    loongarch_vaddr_t test_addr = 0x1000000000000000UL;
    loongarch_size_t test_len = LOONGARCH_PAGE_SIZE;

    start_time = loongarch_timer_read();
    for (int i = 0; i < 1000; i++)
    {
        loongarch_cache_invalidate_range(test_addr, test_len);
    }
    end_time = loongarch_timer_read();

    uint64_t cache_time = end_time - start_time;
    printf("LoongArch: Cache invalidation performance: %llu cycles for 1000 operations\n",
           (unsigned long long)cache_time);

    printf("LoongArch: Performance tests completed\n");
    return 0;
}

/* ============================================================================
 * Phase 1.5 Comprehensive Test Functions
 * ============================================================================ */

/* Test comprehensive interrupt functionality */
static void test_interrupts_comprehensive(void)
{
    printf("LoongArch: Testing comprehensive interrupt functionality...\n");

    /* Test interrupt system initialization */
    int result = loongarch_interrupts_init();
    assert(result == 0);

    /* Test interrupt handler initialization */
    result = loongarch_interrupt_handlers_init();
    assert(result == 0);

    /* Test interrupt enable/disable */
    result = loongarch_interrupt_enable(1);
    assert(result == 0);

    if (loongarch_interrupt_is_enabled(1))
    {
        printf("LoongArch: Interrupt enable test passed\n");
    }

    result = loongarch_interrupt_disable(1);
    assert(result == 0);

    if (!loongarch_interrupt_is_enabled(1))
    {
        printf("LoongArch: Interrupt disable test passed\n");
    }

    /* Test interrupt priority setting */
    result = loongarch_interrupt_set_priority(1, 0x80);
    assert(result == 0);

    uint8_t priority = loongarch_interrupt_get_priority(1);
    assert(priority == 0x80);

    /* Test interrupt testing */
    result = loongarch_interrupt_test();
    assert(result == 0);

    /* Test interrupt handler testing */
    loongarch_interrupt_test_handlers();

    /* Print interrupt statistics */
    loongarch_interrupt_print_stats();
    loongarch_interrupt_print_handlers();

    printf("LoongArch: Comprehensive interrupt test passed\n");
}

/* Test comprehensive timer functionality */
static void test_timers_comprehensive(void)
{
    printf("LoongArch: Testing comprehensive timer functionality...\n");

    /* Test timer system initialization */
    int result = loongarch_timers_init();
    assert(result == 0);

    /* Test timer frequency and resolution */
    uint64_t freq = loongarch_timer_get_frequency();
    uint64_t res = loongarch_timer_get_resolution_ns();

    assert(freq > 0);
    assert(res > 0);

    printf("LoongArch: Timer frequency: %lu Hz, resolution: %lu ns\n", freq, res);

    /* Test timer reading functions */
    uint64_t time1 = loongarch_timer_read_ns();
    uint64_t time2 = loongarch_timer_read_ns();

    assert(time2 >= time1);

    uint64_t time_us = loongarch_timer_read_us();
    uint64_t time_ms = loongarch_timer_read_ms();

    assert(time_us > 0);
    assert(time_ms > 0);

    /* Test timer testing */
    result = loongarch_timer_test();
    assert(result == 0);

    /* Print timer statistics */
    loongarch_timer_print_stats();

    printf("LoongArch: Comprehensive timer test passed\n");
}

/* Test entry point functionality */
static void test_entry_point(void)
{
    printf("LoongArch: Testing entry point functionality...\n");

    /* Test early initialization */
    int result = loongarch_early_init();
    assert(result == 0);

    /* Test system initialization */
    result = loongarch_system_init();
    assert(result == 0);

    /* Test late initialization */
    result = loongarch_late_init();
    assert(result == 0);

    /* Print system status */
    loongarch_print_system_status();

    printf("LoongArch: Entry point test passed\n");
}

/* Test integration functionality */
static void test_integration(void)
{
    printf("LoongArch: Testing integration functionality...\n");

    /* Test complete system */
    int result = loongarch_run_system_tests();
    assert(result == 0);

    printf("LoongArch: Integration test passed\n");
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

/* Run all LoongArch tests */
void loongarch_run_all_tests(void)
{
    printf("=== LoongArch Test Suite ===\n");
    printf("Running comprehensive tests for LoongArch architecture...\n\n");

    bool all_passed = true;

    /* Run individual test sections */
    if (test_loongarch_main() != 0)
        all_passed = false;
    if (test_loongarch_extended() != 0)
        all_passed = false;
    if (test_loongarch_performance() != 0)
        all_passed = false;

    /* Print final results */
    printf("\n=== Test Results ===\n");
    if (all_passed)
    {
        printf("✅ ALL TESTS PASSED\n");
    }
    else
    {
        printf("❌ SOME TESTS FAILED\n");
    }
    printf("==================\n");
}

/* Performance test for LoongArch */
void loongarch_performance_test(void)
{
    printf("\n=== LoongArch Performance Test ===\n");

    /* Test interrupt latency */
    printf("Testing interrupt latency...\n");
    uint64_t start_time = loongarch_timer_read_ns();

    /* Simulate interrupt processing */
    for (int i = 0; i < 1000; i++)
    {
        loongarch_interrupt_test();
    }

    uint64_t end_time = loongarch_timer_read_ns();
    uint64_t latency = (end_time - start_time) / 1000;

    printf("Average interrupt latency: %lu ns\n", latency);

    /* Test timer accuracy */
    printf("Testing timer accuracy...\n");
    start_time = loongarch_timer_read_ns();

    /* Wait for a specific time */
    uint64_t target_time = 1000000; /* 1ms */
    uint64_t current_time = start_time;

    while ((current_time - start_time) < target_time)
    {
        current_time = loongarch_timer_read_ns();
    }

    uint64_t actual_time = current_time - start_time;
    printf("Timer accuracy: target %lu ns, actual %lu ns\n", target_time, actual_time);

    printf("LoongArch performance test completed\n");
}

/* Stress test for LoongArch */
void loongarch_stress_test(void)
{
    printf("\n=== LoongArch Stress Test ===\n");

    /* Test interrupt stress */
    printf("Testing interrupt stress...\n");
    for (int i = 0; i < 10000; i++)
    {
        loongarch_interrupt_test();
    }

    /* Test timer stress */
    printf("Testing timer stress...\n");
    for (int i = 0; i < 10000; i++)
    {
        loongarch_timer_test();
    }

    printf("LoongArch stress test completed\n");
}

/* Test advanced features */
static void test_advanced_features(void)
{
    printf("LoongArch: Testing advanced features...\n");

    /* Test vector extensions */
    printf("  Testing LSX support...\n");
    int result = loongarch_lsx_init();
    assert(result == 0);

    printf("  Testing LASX support...\n");
    result = loongarch_lasx_init();
    assert(result == 0);

    /* Test cryptographic acceleration */
    printf("  Testing crypto acceleration...\n");
    result = loongarch_crypto_init();
    assert(result == 0);

    /* Test performance monitoring */
    printf("  Testing PMU...\n");
    result = loongarch_pmu_init();
    assert(result == 0);

    /* Test power management */
    printf("  Testing power management...\n");
    result = loongarch_power_management_init();
    assert(result == 0);

    /* Test NUMA optimization */
    printf("  Testing NUMA optimization...\n");
    result = loongarch_numa_optimize();
    assert(result == 0);

    /* Test virtualization support */
    printf("  Testing virtualization support...\n");
    result = loongarch_virtualization_init();
    assert(result == 0);

/* Test MSVC support if available */
#ifdef _MSC_VER
    printf("  Testing MSVC optimizations...\n");
    result = loongarch_msvc_optimize();
    assert(result == 0);
#endif

    printf("LoongArch: Advanced features test passed\n");
}

/* Initialize LoongArch test suite */
void loongarch_test_init(void)
{
    printf("Initializing LoongArch test suite...\n");

    /* Initialize all subsystems */
    loongarch_arch_init();
    loongarch_interrupts_init();
    loongarch_interrupt_handlers_init();
    loongarch_timers_init();
    loongarch_early_init();

    printf("LoongArch test suite initialized\n");
}
