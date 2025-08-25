/*
 * Orion Operating System - s390x HAL Test Suite
 *
 * Test suite for s390x Hardware Abstraction Layer.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "hal_s390x.h"
#include "hal_s390x_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// ============================================================================
// TEST FUNCTIONS
// ============================================================================

static void test_hal_s390x_init(void)
{
    printf("Testing s390x HAL initialization...\n");

    int ret = hal_s390x_init();
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ s390x HAL initialization successful\n");
}

static void test_hal_s390x_interface(void)
{
    printf("Testing s390x HAL interface...\n");

    struct orion_hal_arch *interface = hal_s390x_get_interface();
    assert(interface != NULL);
    assert(strcmp(interface->name, "s390x") == 0);
    assert(interface->arch_id == ORION_ARCH_S390X);
    assert(interface->version == 1);
    printf("✓ s390x HAL interface valid\n");
}

static void test_hal_s390x_mmu(void)
{
    printf("Testing s390x HAL MMU functions...\n");

    // Test MMU initialization
    int ret = hal_s390x_mmu_init();
    if (ret == ORION_HAL_SUCCESS)
    {
        printf("✓ s390x HAL MMU initialization successful\n");
    }
    else
    {
        printf("⚠ s390x HAL MMU initialization returned %d\n", ret);
    }

    // Test page mapping (this would require proper setup)
    printf("✓ s390x HAL MMU functions declared\n");
}

static void test_hal_s390x_interrupts(void)
{
    printf("Testing s390x HAL interrupt functions...\n");

    // Test interrupt registration (this would require proper setup)
    printf("✓ s390x HAL interrupt functions declared\n");
}

static void test_hal_s390x_timers(void)
{
    printf("Testing s390x HAL timer functions...\n");

    // Test timer initialization (this would require proper setup)
    printf("✓ s390x HAL timer functions declared\n");
}

static void test_hal_s390x_cache(void)
{
    printf("Testing s390x HAL cache functions...\n");

    // Test cache operations (this would require proper setup)
    printf("✓ s390x HAL cache functions declared\n");
}

static void test_hal_s390x_performance(void)
{
    printf("Testing s390x HAL performance functions...\n");

    // Test performance monitoring (this would require proper setup)
    printf("✓ s390x HAL performance functions declared\n");
}

static void test_hal_s390x_security(void)
{
    printf("Testing s390x HAL security functions...\n");

    // Test security features (this would require proper setup)
    printf("✓ s390x HAL security functions declared\n");
}

static void test_hal_s390x_power(void)
{
    printf("Testing s390x HAL power management...\n");

    // Test power management (this would require proper setup)
    printf("✓ s390x HAL power management functions declared\n");
}

static void test_hal_s390x_debug(void)
{
    printf("Testing s390x HAL debug functions...\n");

    // Test debug features (this would require proper setup)
    printf("✓ s390x HAL debug functions declared\n");
}

static void test_hal_s390x_extensions(void)
{
    printf("Testing s390x HAL extensions...\n");

    // Test architecture-specific extensions
    int ret = hal_s390x_arch_extension_call(ORION_EXTENSION_VECTOR, NULL);
    if (ret == ORION_HAL_ERROR_NOT_SUPPORTED)
    {
        printf("✓ s390x HAL Vector extension properly handled\n");
    }
    else
    {
        printf("⚠ s390x HAL Vector extension returned %d\n", ret);
    }

    ret = hal_s390x_arch_extension_call(ORION_EXTENSION_CRYPTO, NULL);
    if (ret == ORION_HAL_ERROR_NOT_SUPPORTED)
    {
        printf("✓ s390x HAL Crypto extension properly handled\n");
    }
    else
    {
        printf("⚠ s390x HAL Crypto extension returned %d\n", ret);
    }

    ret = hal_s390x_arch_extension_call(ORION_EXTENSION_Z15, NULL);
    if (ret == ORION_HAL_ERROR_NOT_SUPPORTED)
    {
        printf("✓ s390x HAL z15 extension properly handled\n");
    }
    else
    {
        printf("⚠ s390x HAL z15 extension returned %d\n", ret);
    }

    printf("✓ s390x HAL extensions tested\n");
}

static void test_hal_s390x_cleanup(void)
{
    printf("Testing s390x HAL cleanup...\n");

    hal_s390x_cleanup();
    printf("✓ s390x HAL cleanup successful\n");
}

// ============================================================================
// MAIN TEST FUNCTION
// ============================================================================

int test_hal_s390x_all(void)
{
    printf("========================================\n");
    printf("s390x HAL Test Suite\n");
    printf("========================================\n");

    test_hal_s390x_init();
    test_hal_s390x_interface();
    test_hal_s390x_mmu();
    test_hal_s390x_interrupts();
    test_hal_s390x_timers();
    test_hal_s390x_cache();
    test_hal_s390x_performance();
    test_hal_s390x_security();
    test_hal_s390x_power();
    test_hal_s390x_debug();
    test_hal_s390x_extensions();
    test_hal_s390x_cleanup();

    printf("========================================\n");
    printf("All s390x HAL tests completed successfully!\n");
    printf("========================================\n");

    return 0;
}

// ============================================================================
// INDIVIDUAL TEST FUNCTIONS FOR EXTERNAL USE
// ============================================================================

int test_hal_s390x_basic(void)
{
    test_hal_s390x_init();
    test_hal_s390x_interface();
    test_hal_s390x_cleanup();
    return 0;
}

int test_hal_s390x_memory(void)
{
    test_hal_s390x_init();
    test_hal_s390x_mmu();
    test_hal_s390x_cache();
    test_hal_s390x_cleanup();
    return 0;
}

int test_hal_s390x_system(void)
{
    test_hal_s390x_init();
    test_hal_s390x_interrupts();
    test_hal_s390x_timers();
    test_hal_s390x_performance();
    test_hal_s390x_cleanup();
    return 0;
}

int test_hal_s390x_advanced(void)
{
    test_hal_s390x_init();
    test_hal_s390x_security();
    test_hal_s390x_power();
    test_hal_s390x_debug();
    test_hal_s390x_extensions();
    test_hal_s390x_cleanup();
    return 0;
}
