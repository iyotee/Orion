/*
 * ORION OS - MIPS Validation System
 *
 * Validation and testing system for MIPS architecture
 * Handles unit tests, integration tests, and system validation
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
// VALIDATION CONTEXT MANAGEMENT
// ============================================================================

static mips_validation_context_t mips_validation_context = {0};

void mips_validation_init(void)
{
    memset(&mips_validation_context, 0, sizeof(mips_validation_context));

    // Initialize validation context
    mips_validation_context.validation_initialized = false;
    mips_validation_context.test_suite_count = 0;
    mips_validation_context.test_case_count = 0;
    mips_validation_context.test_passed = 0;
    mips_validation_context.test_failed = 0;
    mips_validation_context.test_skipped = 0;
    mips_validation_context.validation_mode = MIPS_VALIDATION_MODE_NORMAL;
    mips_validation_context.auto_run_tests = false;

    // Initialize test suites
    for (int i = 0; i < MIPS_MAX_TEST_SUITES; i++)
    {
        mips_validation_context.test_suites[i] = NULL;
    }

    printf("MIPS: Validation system initialized\n");
}

mips_validation_context_t *mips_validation_get_context(void)
{
    return &mips_validation_context;
}

// ============================================================================
// TEST SUITE MANAGEMENT
// ============================================================================

int mips_validation_register_test_suite(const mips_test_suite_t *suite)
{
    if (!suite)
    {
        printf("MIPS: Invalid test suite pointer\n");
        return -1;
    }

    if (mips_validation_context.test_suite_count >= MIPS_MAX_TEST_SUITES)
    {
        printf("MIPS: Maximum test suite count reached\n");
        return -1;
    }

    // Find free test suite slot
    int suite_id = -1;
    for (int i = 0; i < MIPS_MAX_TEST_SUITES; i++)
    {
        if (!mips_validation_context.test_suites[i])
        {
            suite_id = i;
            break;
        }
    }

    if (suite_id == -1)
    {
        printf("MIPS: No free test suite slots\n");
        return -1;
    }

    // Allocate and copy test suite
    mips_test_suite_t *new_suite = malloc(sizeof(mips_test_suite_t));
    if (!new_suite)
    {
        printf("MIPS: Failed to allocate test suite memory\n");
        return -1;
    }

    memcpy(new_suite, suite, sizeof(mips_test_suite_t));
    mips_validation_context.test_suites[suite_id] = new_suite;
    mips_validation_context.test_suite_count++;

    printf("MIPS: Test suite '%s' registered with ID %d\n", suite->name, suite_id);
    return suite_id;
}

void mips_validation_unregister_test_suite(int suite_id)
{
    if (suite_id < 0 || suite_id >= MIPS_MAX_TEST_SUITES)
    {
        printf("MIPS: Invalid test suite ID %d\n", suite_id);
        return;
    }

    mips_test_suite_t *suite = mips_validation_context.test_suites[suite_id];
    if (!suite)
    {
        printf("MIPS: Test suite %d not registered\n", suite_id);
        return;
    }

    // Free test suite memory
    free(suite);
    mips_validation_context.test_suites[suite_id] = NULL;
    mips_validation_context.test_suite_count--;

    printf("MIPS: Test suite %d unregistered\n", suite_id);
}

mips_test_suite_t *mips_validation_get_test_suite(int suite_id)
{
    if (suite_id < 0 || suite_id >= MIPS_MAX_TEST_SUITES)
    {
        return NULL;
    }

    return mips_validation_context.test_suites[suite_id];
}

// ============================================================================
// TEST EXECUTION
// ============================================================================

void mips_validation_run_test_suite(int suite_id)
{
    mips_test_suite_t *suite = mips_validation_get_test_suite(suite_id);
    if (!suite)
    {
        printf("MIPS: Test suite %d not found\n", suite_id);
        return;
    }

    printf("MIPS: Running test suite '%s'...\n", suite->name);

    // Initialize test suite
    if (suite->init)
    {
        suite->init();
    }

    // Run test cases
    for (int i = 0; i < suite->test_case_count; i++)
    {
        mips_test_case_t *test_case = &suite->test_cases[i];

        printf("MIPS: Running test case '%s'...\n", test_case->name);

        // Run test case
        bool result = test_case->test_function();

        if (result)
        {
            printf("MIPS: Test case '%s' PASSED\n", test_case->name);
            mips_validation_context.test_passed++;
        }
        else
        {
            printf("MIPS: Test case '%s' FAILED\n", test_case->name);
            mips_validation_context.test_failed++;
        }

        mips_validation_context.test_case_count++;
    }

    // Cleanup test suite
    if (suite->cleanup)
    {
        suite->cleanup();
    }

    printf("MIPS: Test suite '%s' completed\n", suite->name);
}

void mips_validation_run_all_test_suites(void)
{
    printf("MIPS: Running all test suites...\n");

    mips_validation_context.test_case_count = 0;
    mips_validation_context.test_passed = 0;
    mips_validation_context.test_failed = 0;
    mips_validation_context.test_skipped = 0;

    for (int i = 0; i < MIPS_MAX_TEST_SUITES; i++)
    {
        if (mips_validation_context.test_suites[i])
        {
            mips_validation_run_test_suite(i);
        }
    }

    // Print summary
    printf("\nMIPS: Test execution summary:\n");
    printf("  Test Suites: %u\n", mips_validation_context.test_suite_count);
    printf("  Test Cases: %u\n", mips_validation_context.test_case_count);
    printf("  Passed: %u\n", mips_validation_context.test_passed);
    printf("  Failed: %u\n", mips_validation_context.test_failed);
    printf("  Skipped: %u\n", mips_validation_context.test_skipped);

    if (mips_validation_context.test_failed == 0)
    {
        printf("MIPS: All tests PASSED\n");
    }
    else
    {
        printf("MIPS: Some tests FAILED\n");
    }
}

// ============================================================================
// UNIT TESTS
// ============================================================================

bool mips_validation_test_cpu_basic(void)
{
    printf("MIPS: Testing CPU basic functionality...\n");

    bool test_passed = true;

    // Test CPU initialization
    mips_cpu_init();

    // Test CPU identification
    uint32_t cpu_id = mips_cpu_get_id();
    if (cpu_id == 0 || cpu_id == 0xFFFFFFFF)
    {
        printf("MIPS: CPU ID test failed: 0x%x\n", cpu_id);
        test_passed = false;
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

    return test_passed;
}

bool mips_validation_test_memory_basic(void)
{
    printf("MIPS: Testing memory basic functionality...\n");

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

    return test_passed;
}

bool mips_validation_test_interrupts_basic(void)
{
    printf("MIPS: Testing interrupts basic functionality...\n");

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

    return test_passed;
}

bool mips_validation_test_timers_basic(void)
{
    printf("MIPS: Testing timers basic functionality...\n");

    bool test_passed = true;

    // Test timer initialization
    mips_timers_init();

    // Test timer creation
    mips_timer_id_t timer_id = mips_timer_create_periodic(1000, NULL, NULL);
    if (timer_id == MIPS_INVALID_TIMER_ID)
    {
        printf("MIPS: Timer creation test failed\n");
        test_passed = false;
    }
    else
    {
        // Test timer start/stop
        mips_timer_start(timer_id);
        mips_timer_stop(timer_id);

        // Clean up
        mips_timer_destroy(timer_id);
    }

    return test_passed;
}

bool mips_validation_test_devices_basic(void)
{
    printf("MIPS: Testing devices basic functionality...\n");

    bool test_passed = true;

    // Test device initialization
    mips_devices_init();

    // Test UART
    mips_uart_init(0x1F000900, 115200);
    if (!mips_uart_is_initialized())
    {
        printf("MIPS: UART test failed\n");
        test_passed = false;
    }

    // Test GPIO
    mips_gpio_init(0x1F001300);
    if (!mips_gpio_is_initialized())
    {
        printf("MIPS: GPIO test failed\n");
        test_passed = false;
    }

    return test_passed;
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

bool mips_validation_test_system_integration(void)
{
    printf("MIPS: Testing system integration...\n");

    bool test_passed = true;

    // Initialize all subsystems
    mips_cpu_init();
    mips_memory_init();
    mips_interrupts_init();
    mips_timers_init();
    mips_devices_init();
    mips_system_init();

    // Test system functionality
    mips_process_id_t pid = mips_system_create_process("test", 0x1000);
    if (pid == MIPS_INVALID_PROCESS_ID)
    {
        printf("MIPS: Process creation test failed\n");
        test_passed = false;
    }
    else
    {
        printf("MIPS: Process creation test passed\n");
        mips_system_destroy_process(pid);
    }

    return test_passed;
}

bool mips_validation_test_performance_integration(void)
{
    printf("MIPS: Testing performance integration...\n");

    bool test_passed = true;

    // Initialize performance monitoring
    mips_performance_init();

    // Test performance counters
    mips_performance_enable_counters();

    // Run performance test
    uint64_t start_time = mips_timer_get_system_time();

    // CPU-intensive operation
    volatile uint64_t result = 0;
    for (volatile uint64_t i = 0; i < 100000; i++)
    {
        result += i * i;
    }

    uint64_t end_time = mips_timer_get_system_time();
    uint64_t duration = end_time - start_time;

    printf("MIPS: Performance test completed in %llu cycles\n", duration);
    printf("MIPS: Result: %llu\n", result);

    if (duration == 0)
    {
        printf("MIPS: Performance test failed - zero duration\n");
        test_passed = false;
    }

    return test_passed;
}

bool mips_validation_test_security_integration(void)
{
    printf("MIPS: Testing security integration...\n");

    bool test_passed = true;

    // Initialize security system
    mips_security_init();

    // Test secure mode
    mips_security_enable_secure_mode();
    if (!mips_security_is_secure_mode())
    {
        printf("MIPS: Secure mode test failed\n");
        test_passed = false;
    }

    // Test memory protection
    mips_memory_enable_protection();

    // Test access control
    bool access_allowed = mips_security_check_access(0x1000, MIPS_ACCESS_READ);
    if (!access_allowed)
    {
        printf("MIPS: Access control test failed\n");
        test_passed = false;
    }

    return test_passed;
}

// ============================================================================
// SYSTEM VALIDATION
// ============================================================================

void mips_validation_validate_system_architecture(void)
{
    printf("MIPS: Validating system architecture...\n");

    // Validate CPU architecture
    mips_validation_validate_cpu_architecture();

    // Validate memory architecture
    mips_validation_validate_memory_architecture();

    // Validate interrupt architecture
    mips_validation_validate_interrupt_architecture();

    // Validate device architecture
    mips_validation_validate_device_architecture();

    printf("MIPS: System architecture validation completed\n");
}

void mips_validation_validate_cpu_architecture(void)
{
    printf("MIPS: Validating CPU architecture...\n");

    // Check CPU features
    mips_cpu_detect_features();

    // Validate register set
    mips_validation_validate_cpu_registers();

    // Validate instruction set
    mips_validation_validate_instruction_set();

    // Validate coprocessor support
    mips_validation_validate_coprocessor_support();
}

void mips_validation_validate_memory_architecture(void)
{
    printf("MIPS: Validating memory architecture...\n");

    // Validate memory layout
    mips_validation_validate_memory_layout();

    // Validate TLB configuration
    mips_validation_validate_tlb_configuration();

    // Validate cache configuration
    mips_validation_validate_cache_configuration();
}

void mips_validation_validate_interrupt_architecture(void)
{
    printf("MIPS: Validating interrupt architecture...\n");

    // Validate interrupt vectors
    mips_validation_validate_interrupt_vectors();

    // Validate interrupt priorities
    mips_validation_validate_interrupt_priorities();

    // Validate exception handling
    mips_validation_validate_exception_handling();
}

void mips_validation_validate_device_architecture(void)
{
    printf("MIPS: Validating device architecture...\n");

    // Validate device discovery
    mips_validation_validate_device_discovery();

    // Validate device drivers
    mips_validation_validate_device_drivers();

    // Validate device communication
    mips_validation_validate_device_communication();
}

// ============================================================================
// VALIDATION UTILITIES
// ============================================================================

void mips_validation_print_status(void)
{
    printf("MIPS: Validation Status:\n");
    printf("  Validation Initialized: %s\n", mips_validation_context.validation_initialized ? "yes" : "no");
    printf("  Test Suite Count: %u\n", mips_validation_context.test_suite_count);
    printf("  Test Case Count: %u\n", mips_validation_context.test_case_count);
    printf("  Tests Passed: %u\n", mips_validation_context.test_passed);
    printf("  Tests Failed: %u\n", mips_validation_context.test_failed);
    printf("  Tests Skipped: %u\n", mips_validation_context.test_skipped);
    printf("  Validation Mode: %u\n", mips_validation_context.validation_mode);
    printf("  Auto Run Tests: %s\n", mips_validation_context.auto_run_tests ? "enabled" : "disabled");

    printf("  Test Suites:\n");
    for (int i = 0; i < MIPS_MAX_TEST_SUITES; i++)
    {
        mips_test_suite_t *suite = mips_validation_context.test_suites[i];
        if (suite)
        {
            printf("    ID %d: %s (%u test cases)\n", i, suite->name, suite->test_case_count);
        }
    }
}

void mips_validation_set_validation_mode(mips_validation_mode_t mode)
{
    mips_validation_context.validation_mode = mode;
    printf("MIPS: Validation mode set to %u\n", mode);
}

void mips_validation_enable_auto_run_tests(void)
{
    mips_validation_context.auto_run_tests = true;
    printf("MIPS: Auto-run tests enabled\n");
}

void mips_validation_disable_auto_run_tests(void)
{
    mips_validation_context.auto_run_tests = false;
    printf("MIPS: Auto-run tests disabled\n");
}

void mips_validation_reset_results(void)
{
    mips_validation_context.test_case_count = 0;
    mips_validation_context.test_passed = 0;
    mips_validation_context.test_failed = 0;
    mips_validation_context.test_skipped = 0;

    printf("MIPS: Validation results reset\n");
}

void mips_validation_cleanup(void)
{
    printf("MIPS: Cleaning up validation system\n");

    // Unregister all test suites
    for (int i = 0; i < MIPS_MAX_TEST_SUITES; i++)
    {
        if (mips_validation_context.test_suites[i])
        {
            mips_validation_unregister_test_suite(i);
        }
    }

    mips_validation_context.validation_initialized = false;
    mips_validation_context.test_suite_count = 0;
    mips_validation_context.test_case_count = 0;
    mips_validation_context.test_passed = 0;
    mips_validation_context.test_failed = 0;
    mips_validation_context.test_skipped = 0;

    printf("MIPS: Validation cleanup completed\n");
}

// ============================================================================
// PLACEHOLDER VALIDATION FUNCTIONS
// ============================================================================

void mips_validation_validate_cpu_registers(void)
{
    printf("MIPS: CPU register validation not implemented\n");
}

void mips_validation_validate_instruction_set(void)
{
    printf("MIPS: Instruction set validation not implemented\n");
}

void mips_validation_validate_coprocessor_support(void)
{
    printf("MIPS: Coprocessor support validation not implemented\n");
}

void mips_validation_validate_memory_layout(void)
{
    printf("MIPS: Memory layout validation not implemented\n");
}

void mips_validation_validate_tlb_configuration(void)
{
    printf("MIPS: TLB configuration validation not implemented\n");
}

void mips_validation_validate_cache_configuration(void)
{
    printf("MIPS: Cache configuration validation not implemented\n");
}

void mips_validation_validate_interrupt_vectors(void)
{
    printf("MIPS: Interrupt vector validation not implemented\n");
}

void mips_validation_validate_interrupt_priorities(void)
{
    printf("MIPS: Interrupt priority validation not implemented\n");
}

void mips_validation_validate_exception_handling(void)
{
    printf("MIPS: Exception handling validation not implemented\n");
}

void mips_validation_validate_device_discovery(void)
{
    printf("MIPS: Device discovery validation not implemented\n");
}

void mips_validation_validate_device_drivers(void)
{
    printf("MIPS: Device driver validation not implemented\n");
}

void mips_validation_validate_device_communication(void)
{
    printf("MIPS: Device communication validation not implemented\n");
}
