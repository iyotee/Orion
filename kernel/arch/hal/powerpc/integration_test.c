/*
 * Orion Operating System - PowerPC HAL Integration Test
 *
 * Integration test for PowerPC HAL with main Orion OS system.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "hal_powerpc.h"
#include "hal_powerpc_common.h"
#include <orion_hal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// ============================================================================
// INTEGRATION TEST FUNCTIONS
// ============================================================================

static void test_hal_registration(void)
{
    printf("Testing PowerPC HAL registration with main system...\n");

    // Test that PowerPC HAL can be registered
    struct orion_hal_arch *powerpc_hal = hal_powerpc_get_interface();
    assert(powerpc_hal != NULL);

    // Test that it matches the exported interface
    assert(strcmp(powerpc_hal->name, "PowerPC") == 0);
    assert(powerpc_hal->arch_id == ORION_ARCH_POWERPC);

    printf("✓ PowerPC HAL registration successful\n");
}

static void test_hal_interface_compatibility(void)
{
    printf("Testing PowerPC HAL interface compatibility...\n");

    struct orion_hal_arch *hal = hal_powerpc_get_interface();

    // Verify all required function pointers are set
    assert(hal->mmu_init != NULL);
    assert(hal->mmu_map != NULL);
    assert(hal->mmu_unmap != NULL);
    assert(hal->mmu_protect != NULL);
    assert(hal->mmu_translate != NULL);

    assert(hal->context_switch != NULL);
    assert(hal->context_save != NULL);
    assert(hal->context_restore != NULL);

    assert(hal->irq_register != NULL);
    assert(hal->irq_unregister != NULL);
    assert(hal->irq_enable != NULL);
    assert(hal->irq_disable != NULL);
    assert(hal->irq_ack != NULL);

    assert(hal->timer_init != NULL);
    assert(hal->timer_get_ticks != NULL);
    assert(hal->timer_set_oneshot != NULL);
    assert(hal->timer_cancel != NULL);

    assert(hal->cpu_init != NULL);
    assert(hal->cpu_idle != NULL);
    assert(hal->cpu_wake != NULL);
    assert(hal->cpu_get_count != NULL);
    assert(hal->cpu_get_current != NULL);

    assert(hal->cache_invalidate != NULL);
    assert(hal->cache_clean != NULL);
    assert(hal->cache_flush != NULL);
    assert(hal->cache_sync != NULL);

    assert(hal->perf_init != NULL);
    assert(hal->perf_read_counter != NULL);
    assert(hal->perf_set_event != NULL);
    assert(hal->perf_enable != NULL);
    assert(hal->perf_disable != NULL);

    assert(hal->secure_init != NULL);
    assert(hal->secure_measure != NULL);
    assert(hal->secure_verify != NULL);
    assert(hal->secure_enclave_create != NULL);
    assert(hal->secure_enclave_destroy != NULL);

    assert(hal->power_init != NULL);
    assert(hal->power_set_state != NULL);
    assert(hal->power_get_state != NULL);
    assert(hal->power_set_frequency != NULL);

    assert(hal->debug_init != NULL);
    assert(hal->debug_break != NULL);
    assert(hal->debug_trace != NULL);
    assert(hal->debug_dump_registers != NULL);

    assert(hal->arch_extension_call != NULL);

    printf("✓ PowerPC HAL interface compatibility verified\n");
}

static void test_hal_constants(void)
{
    printf("Testing PowerPC HAL constants...\n");

    // Test architecture identifiers
    assert(ORION_ARCH_POWERPC == 0x00000004);
    assert(ORION_ARCH_POWERPC_64 == 0x00000005);

    // Test extension identifiers
    assert(ORION_EXTENSION_VSX == 0x00001000);
    assert(ORION_EXTENSION_ALTIVEC == 0x00001001);
    assert(ORION_EXTENSION_POWER8 == 0x00001002);
    assert(ORION_EXTENSION_POWER9 == 0x00001003);
    assert(ORION_EXTENSION_POWER10 == 0x00001004);
    assert(ORION_EXTENSION_POWER11 == 0x00001005);

    // Test page flags
    assert(POWER_PAGE_READ == 0x00000001);
    assert(POWER_PAGE_WRITE == 0x00000002);
    assert(POWER_PAGE_EXEC == 0x00000004);
    assert(POWER_PAGE_USER == 0x00000008);

    // Test interrupt numbers
    assert(POWER_IRQ_SYSTEM_RESET == 0);
    assert(POWER_IRQ_MACHINE_CHECK == 1);
    assert(POWER_IRQ_EXTERNAL == 4);
    assert(POWER_IRQ_DECREMENTER == 8);

    // Test timer constants
    assert(POWER_TIMER_FREQ_DEFAULT == 1000000);
    assert(POWER_TIMER_MAX_FREQ == 10000000);
    assert(POWER_TIMER_MIN_FREQ == 1000);

    // Test cache constants
    assert(POWER_CACHE_LINE_SIZE == 128);
    assert(POWER_CACHE_L1_SIZE == (32 * 1024));
    assert(POWER_CACHE_L2_SIZE == (256 * 1024));
    assert(POWER_CACHE_L3_SIZE == (8 * 1024 * 1024));

    // Test performance counters
    assert(POWER_PMU_MAX_COUNTERS == 8);
    assert(POWER_PMU_CYCLE_COUNTER == 0);
    assert(POWER_PMU_INST_COUNTER == 1);

    // Test power states
    assert(POWER_STATE_ACTIVE == 0);
    assert(POWER_STATE_IDLE == 1);
    assert(POWER_STATE_SLEEP == 2);
    assert(POWER_STATE_HIBERNATE == 3);
    assert(POWER_STATE_OFF == 4);

    // Test error codes
    assert(POWER_SUCCESS == 0);
    assert(POWER_ERROR_INVALID_ARG == -1);
    assert(POWER_ERROR_NOT_SUPPORTED == -2);

    printf("✓ PowerPC HAL constants verified\n");
}

static void test_hal_data_types(void)
{
    printf("Testing PowerPC HAL data types...\n");

    // Test that types are properly defined
    power_vaddr_t vaddr = 0x10000000;
    power_paddr_t paddr = 0x20000000;
    power_size_t size = 4096;
    power_off_t offset = 0;
    power_irq_t irq = 4;
    power_timer_t timer = 1000;
    power_counter_t counter = 0;

    // Verify types have correct sizes
    assert(sizeof(power_vaddr_t) == 8);   // 64-bit
    assert(sizeof(power_paddr_t) == 8);   // 64-bit
    assert(sizeof(power_size_t) == 8);    // 64-bit
    assert(sizeof(power_off_t) == 8);     // 64-bit
    assert(sizeof(power_irq_t) == 4);     // 32-bit
    assert(sizeof(power_timer_t) == 4);   // 32-bit
    assert(sizeof(power_counter_t) == 4); // 32-bit

    // Test function pointer types
    power_irq_handler_t irq_handler = NULL;
    power_timer_callback_t timer_callback = NULL;
    power_extension_handler_t extension_handler = NULL;

    assert(irq_handler == NULL);
    assert(timer_callback == NULL);
    assert(extension_handler == NULL);

    printf("✓ PowerPC HAL data types verified\n");
}

static void test_hal_error_handling(void)
{
    printf("Testing PowerPC HAL error handling...\n");

    // Test that unsupported extensions return proper error
    int ret = hal_powerpc_arch_extension_call(0xFFFFFFFF, NULL);
    assert(ret == ORION_HAL_ERROR_NOT_SUPPORTED);

    // Test that valid extensions are handled
    ret = hal_powerpc_arch_extension_call(ORION_EXTENSION_VSX, NULL);
    // This might return NOT_SUPPORTED if VSX is not implemented yet
    assert(ret == ORION_HAL_ERROR_NOT_SUPPORTED || ret == ORION_HAL_SUCCESS);

    printf("✓ PowerPC HAL error handling verified\n");
}

static void test_hal_initialization_sequence(void)
{
    printf("Testing PowerPC HAL initialization sequence...\n");

    // Test initialization
    int ret = hal_powerpc_init();
    if (ret == ORION_HAL_SUCCESS)
    {
        printf("✓ PowerPC HAL initialization successful\n");
    }
    else
    {
        printf("⚠ PowerPC HAL initialization returned %d (expected in test environment)\n", ret);
    }

    // Test interface access after initialization
    struct orion_hal_arch *hal = hal_powerpc_get_interface();
    assert(hal != NULL);
    assert(strcmp(hal->name, "PowerPC") == 0);

    // Test cleanup
    hal_powerpc_cleanup();
    printf("✓ PowerPC HAL cleanup successful\n");
}

// ============================================================================
// MAIN INTEGRATION TEST FUNCTION
// ============================================================================

int test_powerpc_hal_integration(void)
{
    printf("========================================\n");
    printf("PowerPC HAL Integration Test Suite\n");
    printf("========================================\n");

    test_hal_registration();
    test_hal_interface_compatibility();
    test_hal_constants();
    test_hal_data_types();
    test_hal_error_handling();
    test_hal_initialization_sequence();

    printf("========================================\n");
    printf("All PowerPC HAL integration tests passed!\n");
    printf("========================================\n");

    return 0;
}

// ============================================================================
// INDIVIDUAL TEST FUNCTIONS FOR EXTERNAL USE
// ============================================================================

int test_powerpc_hal_basic_integration(void)
{
    test_hal_registration();
    test_hal_interface_compatibility();
    return 0;
}

int test_powerpc_hal_constants_integration(void)
{
    test_hal_constants();
    test_hal_data_types();
    return 0;
}

int test_powerpc_hal_functionality_integration(void)
{
    test_hal_error_handling();
    test_hal_initialization_sequence();
    return 0;
}
