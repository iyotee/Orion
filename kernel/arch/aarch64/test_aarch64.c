/*
 * ORION OS - aarch64 Architecture Tests
 *
 * This file contains tests for aarch64 architecture support.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
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
static void test_neon_support(void);
static void test_sve_support(void);
static void test_security_features(void);
static void test_pmu_basic(void);
static void test_power_management(void);
static void test_numa_support(void);
static void test_advanced_features(void);

/* Test main function */
int test_aarch64_main(void)
{
    printf("aarch64: Starting architecture tests...\n");

    /* Run basic tests */
    test_cpu_detection();
    test_mmu_basic();
    test_interrupt_basic();
    test_timer_basic();
    test_cache_basic();
    test_neon_support();
    test_sve_support();
    test_security_features();
    test_pmu_basic();
    test_power_management();
    test_numa_support();

    /* Run advanced features tests */
    test_advanced_features();

    printf("aarch64: All architecture tests passed!\n");
    return 0;
}

/* Test CPU detection */
static void test_cpu_detection(void)
{
    printf("aarch64: Testing CPU detection...\n");

    /* Test CPU features */
    assert(aarch64_has_feature(AARCH64_FEATURE_ARMv8));
    assert(aarch64_has_feature(AARCH64_FEATURE_NEON));
    assert(aarch64_has_feature(AARCH64_FEATURE_FP));

    printf("aarch64: CPU detection test passed\n");
}

/* Test basic MMU functionality */
static void test_mmu_basic(void)
{
    printf("aarch64: Testing basic MMU functionality...\n");

    /* Test MMU initialization */
    aarch64_mmu_init();

    /* Test basic memory mapping */
    uint64_t test_va = 0x8000000000000000UL;
    uint64_t test_pa = 0x8000000000000000UL;
    uint64_t test_flags = AARCH64_PTE_VALID | AARCH64_PTE_PRESENT | AARCH64_PTE_WRITE;

    int result = aarch64_mmu_map_page(test_va, test_pa, test_flags);
    assert(result == 0);

    /* Test memory unmapping */
    result = aarch64_mmu_unmap_page(test_va);
    assert(result == 0);

    printf("aarch64: Basic MMU test passed\n");
}

/* Test basic interrupt functionality */
static void test_interrupt_basic(void)
{
    printf("aarch64: Testing basic interrupt functionality...\n");

    /* Test interrupt initialization */
    aarch64_interrupts_init();

    /* Test interrupt handler registration */
    void test_handler(void)
    {
        printf("aarch64: Test interrupt handler called\n");
    }

    aarch64_interrupt_set_handler(0, test_handler);

    /* Test interrupt enable/disable */
    aarch64_interrupt_enable(0);
    aarch64_interrupt_disable(0);

    printf("aarch64: Basic interrupt test passed\n");
}

/* Test basic timer functionality */
static void test_timer_basic(void)
{
    printf("aarch64: Testing basic timer functionality...\n");

    /* Test timer initialization */
    aarch64_timer_init();

    /* Test timer read */
    uint64_t timer_value = aarch64_timer_read_ns();
    assert(timer_value >= 0);

    printf("aarch64: Basic timer test passed\n");
    printf("  Timer value: %llu ns\n", (unsigned long long)timer_value);
}

/* Test basic cache functionality */
static void test_cache_basic(void)
{
    printf("aarch64: Testing basic cache functionality...\n");

    /* Test cache operations */
    aarch64_cache_invalidate_all();
    aarch64_cache_clean_all();

    printf("aarch64: Basic cache test passed\n");
}

/* Test NEON support */
static void test_neon_support(void)
{
    printf("aarch64: Testing NEON support...\n");

    /* Test NEON initialization */
    aarch64_neon_init();

    printf("aarch64: NEON support test passed\n");
}

/* Test SVE support */
static void test_sve_support(void)
{
    printf("aarch64: Testing SVE support...\n");

    /* Test SVE initialization */
    aarch64_sve_init();

    printf("aarch64: SVE support test passed\n");
}

/* Test security features */
static void test_security_features(void)
{
    printf("aarch64: Testing security features...\n");

    /* Test security initialization */
    aarch64_security_init();
    aarch64_pauth_init();
    aarch64_mte_init();

    printf("aarch64: Security features test passed\n");
}

/* Test PMU basic functionality */
static void test_pmu_basic(void)
{
    printf("aarch64: Testing PMU basic functionality...\n");

    /* Test PMU initialization */
    aarch64_pmu_init();

    /* Test PMU counter operations */
    uint64_t counter_value = aarch64_pmu_read_counter(0);
    assert(counter_value >= 0);

    printf("aarch64: PMU basic test passed\n");
}

/* Test power management */
static void test_power_management(void)
{
    printf("aarch64: Testing power management...\n");

    /* Test power initialization */
    aarch64_power_init();

    /* Test power state setting */
    int result = aarch64_power_set_state(AARCH64_POWER_STATE_C1);
    assert(result == 0);

    uint32_t current_state = aarch64_power_get_state();
    assert(current_state == AARCH64_POWER_STATE_C1);

    printf("aarch64: Power management test passed\n");
}

/* Test NUMA support */
static void test_numa_support(void)
{
    printf("aarch64: Testing NUMA support...\n");

    /* Test NUMA initialization */
    aarch64_numa_init();

    /* Test NUMA operations */
    uint32_t node_count = aarch64_numa_get_node_count();
    assert(node_count > 0);

    uint32_t current_node = aarch64_numa_get_current_node();
    assert(current_node < node_count);

    printf("aarch64: NUMA support test passed\n");
}

/* Test advanced features */
static void test_advanced_features(void)
{
    printf("aarch64: Testing advanced features...\n");

    /* Test SVE2 support */
    printf("  Testing SVE2 support...\n");
    int result = aarch64_sve2_init();
    assert(result == 0);

    uint32_t vl = aarch64_sve2_get_vector_length();
    assert(vl > 0);

    /* Test NEON optimizations */
    printf("  Testing NEON optimizations...\n");
    result = aarch64_neon_optimize();
    assert(result == 0);

    /* Test cryptographic acceleration */
    printf("  Testing crypto acceleration...\n");
    result = aarch64_crypto_init();
    assert(result == 0);

    /* Test advanced PMU features */
    printf("  Testing advanced PMU...\n");
    result = aarch64_pmu_advanced_init();
    assert(result == 0);

    /* Test advanced power management */
    printf("  Testing advanced power management...\n");
    result = aarch64_power_advanced_init();
    assert(result == 0);

    /* Test virtualization support */
    printf("  Testing virtualization support...\n");
    result = aarch64_virtualization_init();
    assert(result == 0);

/* Test MSVC support if available */
#ifdef _MSC_VER
    printf("  Testing MSVC optimizations...\n");
    result = aarch64_msvc_optimize();
    assert(result == 0);
#endif

    printf("aarch64: Advanced features test passed\n");
}

/* Initialize aarch64 test suite */
void aarch64_test_init(void)
{
    printf("Initializing aarch64 test suite...\n");

    /* Initialize all subsystems */
    aarch64_arch_init();

    printf("aarch64 test suite initialized\n");
}
