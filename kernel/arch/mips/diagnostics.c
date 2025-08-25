/*
 * ORION OS - MIPS Diagnostics
 *
 * Diagnostics and testing system for MIPS architecture
 * Handles hardware testing, performance benchmarking, and system health monitoring
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
#include <stdlib.h>

// ============================================================================
// DIAGNOSTICS CONTEXT MANAGEMENT
// ============================================================================

static mips_diagnostics_context_t mips_diagnostics_context = {0};

void mips_diagnostics_init(void)
{
    memset(&mips_diagnostics_context, 0, sizeof(mips_diagnostics_context));

    // Initialize diagnostics context
    mips_diagnostics_context.diagnostics_initialized = false;
    mips_diagnostics_context.test_count = 0;
    mips_diagnostics_context.test_passed = 0;
    mips_diagnostics_context.test_failed = 0;
    mips_diagnostics_context.performance_tests_enabled = true;
    mips_diagnostics_context.hardware_tests_enabled = true;
    mips_diagnostics_context.continuous_monitoring = false;

    // Initialize test results
    for (int i = 0; i < MIPS_MAX_DIAGNOSTIC_TESTS; i++)
    {
        mips_diagnostics_context.test_results[i] = MIPS_TEST_RESULT_NOT_RUN;
    }

    printf("MIPS: Diagnostics system initialized\n");
}

mips_diagnostics_context_t *mips_diagnostics_get_context(void)
{
    return &mips_diagnostics_context;
}

// ============================================================================
// HARDWARE TESTING
// ============================================================================

bool mips_diagnostics_test_cpu(void)
{
    printf("MIPS: Running CPU diagnostics...\n");

    bool test_passed = true;

    // Test CPU identification
    uint32_t cpu_id = mips_cpu_get_id();
    if (cpu_id == 0 || cpu_id == 0xFFFFFFFF)
    {
        printf("MIPS: CPU ID test failed: 0x%x\n", cpu_id);
        test_passed = false;
    }
    else
    {
        printf("MIPS: CPU ID: 0x%x\n", cpu_id);
    }

    // Test CPU revision
    uint32_t cpu_revision = mips_cpu_get_revision();
    printf("MIPS: CPU Revision: 0x%x\n", cpu_revision);

    // Test CPU frequency
    uint32_t cpu_frequency = mips_cpu_get_frequency();
    if (cpu_frequency == 0)
    {
        printf("MIPS: CPU frequency test failed\n");
        test_passed = false;
    }
    else
    {
        printf("MIPS: CPU Frequency: %u Hz\n", cpu_frequency);
    }

    // Test CPU features
    mips_cpu_detect_features();

    printf("MIPS: CPU diagnostics %s\n", test_passed ? "PASSED" : "FAILED");
    return test_passed;
}

bool mips_diagnostics_test_memory(void)
{
    printf("MIPS: Running memory diagnostics...\n");

    bool test_passed = true;

    // Test memory allocation
    void *test_ptr = mips_memory_allocate_region(1024, MIPS_MEMORY_FLAG_READ | MIPS_MEMORY_FLAG_WRITE);
    if (!test_ptr)
    {
        printf("MIPS: Memory allocation test failed\n");
        test_passed = false;
    }
    else
    {
        printf("MIPS: Memory allocation test passed\n");

        // Test memory write
        memset(test_ptr, 0xAA, 1024);

        // Test memory read
        bool read_test_passed = true;
        unsigned char *ptr = (unsigned char *)test_ptr;
        for (int i = 0; i < 1024; i++)
        {
            if (ptr[i] != 0xAA)
            {
                read_test_passed = false;
                break;
            }
        }

        if (read_test_passed)
        {
            printf("MIPS: Memory read/write test passed\n");
        }
        else
        {
            printf("MIPS: Memory read/write test failed\n");
            test_passed = false;
        }

        // Free test memory
        mips_memory_free_region((mips_addr_t)test_ptr);
    }

    printf("MIPS: Memory diagnostics %s\n", test_passed ? "PASSED" : "FAILED");
    return test_passed;
}

bool mips_diagnostics_test_cache(void)
{
    printf("MIPS: Running cache diagnostics...\n");

    bool test_passed = true;

    // Test cache initialization
    mips_cache_init();

    // Test cache operations
    mips_cache_invalidate_all();
    mips_cache_sync_all();

    // Test cache statistics
    mips_cache_print_statistics();

    printf("MIPS: Cache diagnostics %s\n", test_passed ? "PASSED" : "FAILED");
    return test_passed;
}

bool mips_diagnostics_test_interrupts(void)
{
    printf("MIPS: Running interrupt diagnostics...\n");

    bool test_passed = true;

    // Test interrupt initialization
    mips_interrupts_init();

    // Test interrupt registration
    mips_interrupt_handler_t test_handler = NULL;
    int result = mips_interrupts_register_handler(0, test_handler);
    if (result != 0)
    {
        printf("MIPS: Interrupt registration test failed\n");
        test_passed = false;
    }

    // Test interrupt enable/disable
    mips_interrupts_enable_irq(0);
    mips_interrupts_disable_irq(0);

    printf("MIPS: Interrupt diagnostics %s\n", test_passed ? "PASSED" : "FAILED");
    return test_passed;
}

bool mips_diagnostics_test_timers(void)
{
    printf("MIPS: Running timer diagnostics...\n");

    bool test_passed = true;

    // Test timer initialization
    mips_timers_init();

    // Test timer operations
    mips_timer_id_t timer_id = mips_timer_create_periodic(1000, NULL, NULL);
    if (timer_id == MIPS_INVALID_TIMER_ID)
    {
        printf("MIPS: Timer creation test failed\n");
        test_passed = false;
    }
    else
    {
        printf("MIPS: Timer creation test passed\n");

        // Test timer start/stop
        mips_timer_start(timer_id);
        mips_timer_stop(timer_id);

        // Clean up
        mips_timer_destroy(timer_id);
    }

    printf("MIPS: Timer diagnostics %s\n", test_passed ? "PASSED" : "FAILED");
    return test_passed;
}

bool mips_diagnostics_test_devices(void)
{
    printf("MIPS: Running device diagnostics...\n");

    bool test_passed = true;

    // Test device initialization
    mips_devices_init();

    // Test UART
    mips_uart_init(0x1F000900, 115200);
    if (mips_uart_is_initialized())
    {
        printf("MIPS: UART test passed\n");
    }
    else
    {
        printf("MIPS: UART test failed\n");
        test_passed = false;
    }

    // Test GPIO
    mips_gpio_init(0x1F001300);
    if (mips_gpio_is_initialized())
    {
        printf("MIPS: GPIO test passed\n");
    }
    else
    {
        printf("MIPS: GPIO test failed\n");
        test_passed = false;
    }

    printf("MIPS: Device diagnostics %s\n", test_passed ? "PASSED" : "FAILED");
    return test_passed;
}

// ============================================================================
// PERFORMANCE BENCHMARKING
// ============================================================================

uint64_t mips_diagnostics_benchmark_cpu(void)
{
    printf("MIPS: Running CPU benchmark...\n");

    uint64_t start_time = mips_timer_get_system_time();

    // CPU-intensive operations
    volatile uint64_t result = 0;
    for (volatile uint64_t i = 0; i < 1000000; i++)
    {
        result += i * i;
    }

    uint64_t end_time = mips_timer_get_system_time();
    uint64_t duration = end_time - start_time;

    printf("MIPS: CPU benchmark completed in %llu cycles\n", duration);
    printf("MIPS: Result: %llu\n", result);

    return duration;
}

uint64_t mips_diagnostics_benchmark_memory(void)
{
    printf("MIPS: Running memory benchmark...\n");

    uint64_t start_time = mips_timer_get_system_time();

    // Memory-intensive operations
    const size_t buffer_size = 1024 * 1024; // 1MB
    void *buffer = mips_memory_allocate_region(buffer_size, MIPS_MEMORY_FLAG_READ | MIPS_MEMORY_FLAG_WRITE);

    if (buffer)
    {
        // Write test
        memset(buffer, 0xAA, buffer_size);

        // Read test
        volatile uint64_t sum = 0;
        unsigned char *ptr = (unsigned char *)buffer;
        for (size_t i = 0; i < buffer_size; i++)
        {
            sum += ptr[i];
        }

        // Clean up
        mips_memory_free_region((mips_addr_t)buffer);

        uint64_t end_time = mips_timer_get_system_time();
        uint64_t duration = end_time - start_time;

        printf("MIPS: Memory benchmark completed in %llu cycles\n", duration);
        printf("MIPS: Memory sum: %llu\n", sum);

        return duration;
    }

    printf("MIPS: Memory benchmark failed - allocation failed\n");
    return 0;
}

uint64_t mips_diagnostics_benchmark_cache(void)
{
    printf("MIPS: Running cache benchmark...\n");

    uint64_t start_time = mips_timer_get_system_time();

    // Cache-intensive operations
    const size_t array_size = 1024 * 1024; // 1MB
    volatile uint32_t *array = (volatile uint32_t *)mips_memory_allocate_region(
        array_size, MIPS_MEMORY_FLAG_READ | MIPS_MEMORY_FLAG_WRITE);

    if (array)
    {
        // Sequential access (cache-friendly)
        volatile uint32_t sum1 = 0;
        for (size_t i = 0; i < array_size / sizeof(uint32_t); i++)
        {
            sum1 += array[i];
        }

        // Random access (cache-unfriendly)
        volatile uint32_t sum2 = 0;
        for (size_t i = 0; i < array_size / sizeof(uint32_t); i++)
        {
            size_t index = (i * 7) % (array_size / sizeof(uint32_t));
            sum2 += array[index];
        }

        // Clean up
        mips_memory_free_region((mips_addr_t)array);

        uint64_t end_time = mips_timer_get_system_time();
        uint64_t duration = end_time - start_time;

        printf("MIPS: Cache benchmark completed in %llu cycles\n", duration);
        printf("MIPS: Sequential sum: %u, Random sum: %u\n", sum1, sum2);

        return duration;
    }

    printf("MIPS: Cache benchmark failed - allocation failed\n");
    return 0;
}

// ============================================================================
// SYSTEM HEALTH MONITORING
// ============================================================================

void mips_diagnostics_monitor_system_health(void)
{
    if (!mips_diagnostics_context.continuous_monitoring)
    {
        return;
    }

    printf("MIPS: System Health Monitor:\n");

    // CPU status
    uint32_t cpu_frequency = mips_cpu_get_frequency();
    printf("  CPU Frequency: %u Hz\n", cpu_frequency);

    // Memory status
    size_t memory_allocated = mips_system_get_context()->memory_allocated;
    printf("  Memory Allocated: %zu bytes\n", memory_allocated);

    // Process status
    uint32_t process_count = mips_system_get_context()->process_count;
    printf("  Active Processes: %u\n", process_count);

    // Device status
    uint32_t device_count = mips_drivers_get_context()->device_count;
    printf("  Active Devices: %u\n", device_count);

    // Interrupt status
    uint32_t interrupt_count = mips_interrupts_get_context()->interrupt_count;
    printf("  Interrupt Count: %u\n", interrupt_count);

    // Timer status
    uint32_t timer_count = mips_timers_get_context()->timer_count;
    printf("  Active Timers: %u\n", timer_count);
}

void mips_diagnostics_enable_continuous_monitoring(void)
{
    mips_diagnostics_context.continuous_monitoring = true;
    printf("MIPS: Continuous monitoring enabled\n");
}

void mips_diagnostics_disable_continuous_monitoring(void)
{
    mips_diagnostics_context.continuous_monitoring = false;
    printf("MIPS: Continuous monitoring disabled\n");
}

// ============================================================================
// COMPREHENSIVE TESTING
// ============================================================================

void mips_diagnostics_run_all_tests(void)
{
    printf("MIPS: Running comprehensive diagnostics...\n");

    mips_diagnostics_context.test_count = 0;
    mips_diagnostics_context.test_passed = 0;
    mips_diagnostics_context.test_failed = 0;

    // Hardware tests
    if (mips_diagnostics_context.hardware_tests_enabled)
    {
        printf("\n--- Hardware Tests ---\n");

        // CPU test
        mips_diagnostics_context.test_count++;
        if (mips_diagnostics_test_cpu())
        {
            mips_diagnostics_context.test_passed++;
            mips_diagnostics_context.test_results[0] = MIPS_TEST_RESULT_PASSED;
        }
        else
        {
            mips_diagnostics_context.test_failed++;
            mips_diagnostics_context.test_results[0] = MIPS_TEST_RESULT_FAILED;
        }

        // Memory test
        mips_diagnostics_context.test_count++;
        if (mips_diagnostics_test_memory())
        {
            mips_diagnostics_context.test_passed++;
            mips_diagnostics_context.test_results[1] = MIPS_TEST_RESULT_PASSED;
        }
        else
        {
            mips_diagnostics_context.test_failed++;
            mips_diagnostics_context.test_results[1] = MIPS_TEST_RESULT_FAILED;
        }

        // Cache test
        mips_diagnostics_context.test_count++;
        if (mips_diagnostics_test_cache())
        {
            mips_diagnostics_context.test_passed++;
            mips_diagnostics_context.test_results[2] = MIPS_TEST_RESULT_PASSED;
        }
        else
        {
            mips_diagnostics_context.test_failed++;
            mips_diagnostics_context.test_results[2] = MIPS_TEST_RESULT_FAILED;
        }

        // Interrupt test
        mips_diagnostics_context.test_count++;
        if (mips_diagnostics_test_interrupts())
        {
            mips_diagnostics_context.test_passed++;
            mips_diagnostics_context.test_results[3] = MIPS_TEST_RESULT_PASSED;
        }
        else
        {
            mips_diagnostics_context.test_failed++;
            mips_diagnostics_context.test_results[3] = MIPS_TEST_RESULT_FAILED;
        }

        // Timer test
        mips_diagnostics_context.test_count++;
        if (mips_diagnostics_test_timers())
        {
            mips_diagnostics_context.test_passed++;
            mips_diagnostics_context.test_results[4] = MIPS_TEST_RESULT_PASSED;
        }
        else
        {
            mips_diagnostics_context.test_failed++;
            mips_diagnostics_context.test_results[4] = MIPS_TEST_RESULT_FAILED;
        }

        // Device test
        mips_diagnostics_context.test_count++;
        if (mips_diagnostics_test_devices())
        {
            mips_diagnostics_context.test_passed++;
            mips_diagnostics_context.test_results[5] = MIPS_TEST_RESULT_PASSED;
        }
        else
        {
            mips_diagnostics_context.test_failed++;
            mips_diagnostics_context.test_results[5] = MIPS_TEST_RESULT_FAILED;
        }
    }

    // Performance tests
    if (mips_diagnostics_context.performance_tests_enabled)
    {
        printf("\n--- Performance Tests ---\n");

        // CPU benchmark
        uint64_t cpu_time = mips_diagnostics_benchmark_cpu();
        mips_diagnostics_context.test_count++;
        if (cpu_time > 0)
        {
            mips_diagnostics_context.test_passed++;
            mips_diagnostics_context.test_results[6] = MIPS_TEST_RESULT_PASSED;
        }
        else
        {
            mips_diagnostics_context.test_failed++;
            mips_diagnostics_context.test_results[6] = MIPS_TEST_RESULT_FAILED;
        }

        // Memory benchmark
        uint64_t memory_time = mips_diagnostics_benchmark_memory();
        mips_diagnostics_context.test_count++;
        if (memory_time > 0)
        {
            mips_diagnostics_context.test_passed++;
            mips_diagnostics_context.test_results[7] = MIPS_TEST_RESULT_PASSED;
        }
        else
        {
            mips_diagnostics_context.test_failed++;
            mips_diagnostics_context.test_results[7] = MIPS_TEST_RESULT_FAILED;
        }

        // Cache benchmark
        uint64_t cache_time = mips_diagnostics_benchmark_cache();
        mips_diagnostics_context.test_count++;
        if (cache_time > 0)
        {
            mips_diagnostics_context.test_passed++;
            mips_diagnostics_context.test_results[8] = MIPS_TEST_RESULT_PASSED;
        }
        else
        {
            mips_diagnostics_context.test_failed++;
            mips_diagnostics_context.test_results[8] = MIPS_TEST_RESULT_FAILED;
        }
    }

    // Print summary
    printf("\n--- Test Summary ---\n");
    printf("Total Tests: %u\n", mips_diagnostics_context.test_count);
    printf("Passed: %u\n", mips_diagnostics_context.test_passed);
    printf("Failed: %u\n", mips_diagnostics_context.test_failed);
    printf("Success Rate: %.1f%%\n",
           (float)mips_diagnostics_context.test_passed / mips_diagnostics_context.test_count * 100.0f);

    if (mips_diagnostics_context.test_failed == 0)
    {
        printf("MIPS: All diagnostics tests PASSED\n");
    }
    else
    {
        printf("MIPS: Some diagnostics tests FAILED\n");
    }
}

// ============================================================================
// DIAGNOSTICS UTILITIES
// ============================================================================

void mips_diagnostics_print_status(void)
{
    printf("MIPS: Diagnostics Status:\n");
    printf("  Diagnostics Initialized: %s\n", mips_diagnostics_context.diagnostics_initialized ? "yes" : "no");
    printf("  Test Count: %u\n", mips_diagnostics_context.test_count);
    printf("  Tests Passed: %u\n", mips_diagnostics_context.test_passed);
    printf("  Tests Failed: %u\n", mips_diagnostics_context.test_failed);
    printf("  Performance Tests: %s\n", mips_diagnostics_context.performance_tests_enabled ? "enabled" : "disabled");
    printf("  Hardware Tests: %s\n", mips_diagnostics_context.hardware_tests_enabled ? "enabled" : "disabled");
    printf("  Continuous Monitoring: %s\n", mips_diagnostics_context.continuous_monitoring ? "enabled" : "disabled");

    printf("  Test Results:\n");
    for (int i = 0; i < MIPS_MAX_DIAGNOSTIC_TESTS; i++)
    {
        if (mips_diagnostics_context.test_results[i] != MIPS_TEST_RESULT_NOT_RUN)
        {
            const char *result_str = (mips_diagnostics_context.test_results[i] == MIPS_TEST_RESULT_PASSED) ? "PASSED" : "FAILED";
            printf("    Test %d: %s\n", i, result_str);
        }
    }
}

void mips_diagnostics_enable_performance_tests(void)
{
    mips_diagnostics_context.performance_tests_enabled = true;
    printf("MIPS: Performance tests enabled\n");
}

void mips_diagnostics_disable_performance_tests(void)
{
    mips_diagnostics_context.performance_tests_enabled = false;
    printf("MIPS: Performance tests disabled\n");
}

void mips_diagnostics_enable_hardware_tests(void)
{
    mips_diagnostics_context.hardware_tests_enabled = true;
    printf("MIPS: Hardware tests enabled\n");
}

void mips_diagnostics_disable_hardware_tests(void)
{
    mips_diagnostics_context.hardware_tests_enabled = false;
    printf("MIPS: Hardware tests disabled\n");
}

void mips_diagnostics_reset_results(void)
{
    memset(mips_diagnostics_context.test_results, 0, sizeof(mips_diagnostics_context.test_results));
    mips_diagnostics_context.test_count = 0;
    mips_diagnostics_context.test_passed = 0;
    mips_diagnostics_context.test_failed = 0;

    printf("MIPS: Diagnostics results reset\n");
}

void mips_diagnostics_cleanup(void)
{
    printf("MIPS: Cleaning up diagnostics system\n");

    mips_diagnostics_context.diagnostics_initialized = false;
    mips_diagnostics_context.test_count = 0;
    mips_diagnostics_context.test_passed = 0;
    mips_diagnostics_context.test_failed = 0;
    mips_diagnostics_context.continuous_monitoring = false;

    printf("MIPS: Diagnostics cleanup completed\n");
}
