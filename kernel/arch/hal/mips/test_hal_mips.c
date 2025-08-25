/*
 * Orion Operating System - MIPS HAL Test Suite
 *
 * Test suite for MIPS Hardware Abstraction Layer.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "hal_mips.h"
#include "hal_mips_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// ============================================================================
// TEST FUNCTIONS
// ============================================================================

static void test_hal_mips_init(void)
{
    printf("Testing MIPS HAL initialization...\n");

    int ret = hal_mips_init();
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS HAL initialization successful\n");
}

static void test_hal_mips_mmu(void)
{
    printf("Testing MIPS HAL MMU functions...\n");

    // Test MMU initialization
    int ret = hal_mips_mmu_init();
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS MMU initialization successful\n");

    // Test memory mapping
    vaddr_t va = 0x1000000;
    paddr_t pa = 0x2000000;
    uint64_t flags = ORION_PAGE_PRESENT | ORION_PAGE_WRITABLE | ORION_PAGE_USER;

    ret = hal_mips_mmu_map(va, pa, flags);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS MMU mapping successful\n");

    // Test getting mapping
    paddr_t retrieved_pa;
    uint64_t retrieved_flags;
    ret = hal_mips_mmu_get_mapping(va, &retrieved_pa, &retrieved_flags);
    assert(ret == ORION_HAL_SUCCESS);
    assert(retrieved_pa == pa);
    assert(retrieved_flags == flags);
    printf("✓ MIPS MMU get mapping successful\n");

    // Test memory protection
    uint64_t new_flags = ORION_PAGE_PRESENT | ORION_PAGE_USER;
    ret = hal_mips_mmu_protect(va, 4096, new_flags);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS MMU protection successful\n");

    // Test TLB flush
    ret = hal_mips_mmu_flush_tlb();
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS MMU TLB flush successful\n");

    // Test memory unmapping
    ret = hal_mips_mmu_unmap(va, 4096);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS MMU unmapping successful\n");
}

static void test_hal_mips_interrupts(void)
{
    printf("Testing MIPS HAL interrupt functions...\n");

    // Test interrupt initialization
    int ret = hal_mips_interrupt_init();
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS interrupt initialization successful\n");

    // Test interrupt enable/disable
    ret = hal_mips_interrupt_enable(ORION_MIPS_IRQ_TIMER);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS interrupt enable successful\n");

    ret = hal_mips_interrupt_disable(ORION_MIPS_IRQ_TIMER);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS interrupt disable successful\n");

    // Test interrupt handler setting
    void timer_handler(void) { printf("Timer interrupt!\n"); }
    ret = hal_mips_interrupt_set_handler(ORION_MIPS_IRQ_TIMER, timer_handler);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS interrupt handler setting successful\n");

    // Test interrupt priority
    ret = hal_mips_interrupt_set_priority(ORION_MIPS_IRQ_TIMER, 1);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS interrupt priority setting successful\n");

    // Test interrupt pending operations
    ret = hal_mips_interrupt_get_pending(ORION_MIPS_IRQ_TIMER);
    assert(ret >= 0);
    printf("✓ MIPS interrupt pending check successful\n");

    ret = hal_mips_interrupt_clear_pending(ORION_MIPS_IRQ_TIMER);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS interrupt pending clear successful\n");
}

static void test_hal_mips_timers(void)
{
    printf("Testing MIPS HAL timer functions...\n");

    // Test timer initialization
    int ret = hal_mips_timer_init();
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS timer initialization successful\n");

    // Test timer interval setting
    ret = hal_mips_timer_set_interval(1000000); // 1ms
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS timer interval setting successful\n");

    // Test timer callback setting
    void timer_callback(void) { printf("Timer callback!\n"); }
    ret = hal_mips_timer_set_callback(timer_callback);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS timer callback setting successful\n");

    // Test timer enable/disable
    ret = hal_mips_timer_enable();
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS timer enable successful\n");

    ret = hal_mips_timer_disable();
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS timer disable successful\n");

    // Test timer frequency
    int freq = hal_mips_timer_get_frequency();
    assert(freq > 0);
    printf("✓ MIPS timer frequency: %d Hz\n", freq);
}

static void test_hal_mips_cache(void)
{
    printf("Testing MIPS HAL cache functions...\n");

    // Test cache initialization
    int ret = hal_mips_cache_init();
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS cache initialization successful\n");

    // Test cache operations
    vaddr_t va = 0x1000000;
    size_t len = 4096;

    ret = hal_mips_cache_invalidate_data(va, len);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS cache data invalidation successful\n");

    ret = hal_mips_cache_clean_data(va, len);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS cache data cleaning successful\n");

    ret = hal_mips_cache_flush_data(va, len);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS cache data flushing successful\n");

    ret = hal_mips_cache_invalidate_instruction(va, len);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS cache instruction invalidation successful\n");

    ret = hal_mips_cache_sync();
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS cache synchronization successful\n");
}

static void test_hal_mips_cpu(void)
{
    printf("Testing MIPS HAL CPU functions...\n");

    // Test CPU initialization
    int ret = hal_mips_cpu_init();
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS CPU initialization successful\n");

    // Test CPU information
    int cpu_id = hal_mips_cpu_get_id();
    assert(cpu_id >= 0);
    printf("✓ MIPS CPU ID: %d\n", cpu_id);

    int cpu_count = hal_mips_cpu_get_count();
    assert(cpu_count > 0);
    printf("✓ MIPS CPU count: %d\n", cpu_count);

    // Test CPU features
    uint64_t features;
    ret = hal_mips_cpu_get_features(&features);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS CPU features: 0x%016llx\n", (unsigned long long)features);
}

static void test_hal_mips_performance(void)
{
    printf("Testing MIPS HAL performance functions...\n");

    // Test performance initialization
    int ret = hal_mips_performance_init();
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS performance initialization successful\n");

    // Test performance counters
    for (uint32_t i = 0; i < ORION_MIPS_PMU_COUNTERS; i++)
    {
        ret = hal_mips_performance_start_counter(i);
        assert(ret == ORION_HAL_SUCCESS);
        printf("✓ MIPS performance counter %d start successful\n", i);

        ret = hal_mips_performance_stop_counter(i);
        assert(ret == ORION_HAL_SUCCESS);
        printf("✓ MIPS performance counter %d stop successful\n", i);

        uint64_t value;
        ret = hal_mips_performance_read_counter(i, &value);
        assert(ret == ORION_HAL_SUCCESS);
        printf("✓ MIPS performance counter %d read: %llu\n", i, (unsigned long long)value);
    }

    // Test performance events
    for (uint32_t i = 0; i < ORION_MIPS_PMU_COUNTERS; i++)
    {
        ret = hal_mips_performance_set_event(i, ORION_MIPS_PMU_EVENT_CYCLES);
        assert(ret == ORION_HAL_SUCCESS);
        printf("✓ MIPS performance event setting for counter %d successful\n", i);
    }

    // Test cycle and instruction counts
    uint64_t cycles, instructions;
    ret = hal_mips_performance_get_cycle_count(&cycles);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS cycle count: %llu\n", (unsigned long long)cycles);

    ret = hal_mips_performance_get_instruction_count(&instructions);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS instruction count: %llu\n", (unsigned long long)instructions);
}

static void test_hal_mips_power(void)
{
    printf("Testing MIPS HAL power functions...\n");

    // Test power initialization
    int ret = hal_mips_power_init();
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS power initialization successful\n");

    // Test power state operations
    uint32_t state;
    ret = hal_mips_power_get_state(&state);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS current power state: %d\n", state);

    ret = hal_mips_power_set_state(ORION_MIPS_POWER_STATE_IDLE);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS power state setting successful\n");

    ret = hal_mips_power_get_state(&state);
    assert(ret == ORION_HAL_SUCCESS);
    assert(state == ORION_MIPS_POWER_STATE_IDLE);
    printf("✓ MIPS power state verification successful\n");

    // Test frequency setting
    ret = hal_mips_power_set_frequency(0, 1000); // 1GHz
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS CPU frequency setting successful\n");
}

static void test_hal_mips_security(void)
{
    printf("Testing MIPS HAL security functions...\n");

    // Test security initialization
    int ret = hal_mips_security_init();
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS security initialization successful\n");

    // Test TrustZone operations
    ret = hal_mips_security_set_trustzone(1);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS TrustZone enable successful\n");

    uint32_t trustzone_enabled;
    ret = hal_mips_security_get_trustzone(&trustzone_enabled);
    assert(ret == ORION_HAL_SUCCESS);
    assert(trustzone_enabled == 1);
    printf("✓ MIPS TrustZone verification successful\n");

    // Test secure world operations
    ret = hal_mips_security_set_secure_world(1);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS secure world enable successful\n");

    uint32_t secure_world_enabled;
    ret = hal_mips_security_get_secure_world(&secure_world_enabled);
    assert(ret == ORION_HAL_SUCCESS);
    assert(secure_world_enabled == 1);
    printf("✓ MIPS secure world verification successful\n");
}

static void test_hal_mips_debug(void)
{
    printf("Testing MIPS HAL debug functions...\n");

    // Test debug initialization
    int ret = hal_mips_debug_init();
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS debug initialization successful\n");

    // Test breakpoint operations
    vaddr_t va = 0x1000000;
    ret = hal_mips_debug_set_breakpoint(va, ORION_MIPS_DEBUG_BREAKPOINT_SOFTWARE);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS software breakpoint setting successful\n");

    ret = hal_mips_debug_clear_breakpoint(va);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS breakpoint clearing successful\n");

    // Test watchpoint operations
    ret = hal_mips_debug_set_watchpoint(va, 4, ORION_MIPS_DEBUG_WATCHPOINT_READ);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS read watchpoint setting successful\n");

    ret = hal_mips_debug_clear_watchpoint(va);
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS watchpoint clearing successful\n");
}

static void test_hal_mips_arch_info(void)
{
    printf("Testing MIPS HAL architecture info...\n");

    struct orion_arch_info info;
    int ret = hal_mips_get_arch_info(&info);
    assert(ret == ORION_HAL_SUCCESS);

    assert(info.arch_id == ORION_ARCH_MIPS);
    assert(strcmp(info.arch_name, "MIPS") == 0);
    assert(strcmp(info.arch_version, "MIPS64") == 0);
    assert(info.page_size == ORION_MIPS_PAGE_SIZE_4K);
    assert(info.max_irqs == ORION_MIPS_IRQ_MAX);
    assert(info.max_exceptions == ORION_MIPS_EXC_MAX);
    assert(info.cache_line_size == ORION_MIPS_CACHE_LINE_SIZE);
    assert(info.tlb_entries == ORION_MIPS_TLB_ENTRIES);
    assert(info.pmu_counters == ORION_MIPS_PMU_COUNTERS);

    printf("✓ MIPS architecture info verification successful\n");
    printf("  - Architecture: %s %s\n", info.arch_name, info.arch_version);
    printf("  - Page size: %zu bytes\n", info.page_size);
    printf("  - Max IRQs: %zu\n", info.max_irqs);
    printf("  - Max exceptions: %zu\n", info.max_exceptions);
    printf("  - Cache line size: %zu bytes\n", info.cache_line_size);
    printf("  - TLB entries: %zu\n", info.tlb_entries);
    printf("  - PMU counters: %zu\n", info.pmu_counters);
}

static void test_hal_mips_cleanup(void)
{
    printf("Testing MIPS HAL cleanup...\n");

    int ret = hal_mips_cleanup();
    assert(ret == ORION_HAL_SUCCESS);
    printf("✓ MIPS HAL cleanup successful\n");
}

// ============================================================================
// MAIN TEST FUNCTION
// ============================================================================

int main(void)
{
    printf("========================================\n");
    printf("MIPS HAL Test Suite\n");
    printf("========================================\n\n");

    // Run all tests
    test_hal_mips_init();
    test_hal_mips_mmu();
    test_hal_mips_interrupts();
    test_hal_mips_timers();
    test_hal_mips_cache();
    test_hal_mips_cpu();
    test_hal_mips_performance();
    test_hal_mips_power();
    test_hal_mips_security();
    test_hal_mips_debug();
    test_hal_mips_arch_info();
    test_hal_mips_cleanup();

    printf("\n========================================\n");
    printf("All MIPS HAL tests passed successfully!\n");
    printf("========================================\n");

    return 0;
}
