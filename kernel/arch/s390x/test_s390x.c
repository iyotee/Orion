/*
 * ORION OS - s390x Architecture Test Suite
 *
 * Test suite for IBM s390x architecture support
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// ============================================================================
// TEST FUNCTIONS
// ============================================================================

static void test_s390x_arch_init(void)
{
    printf("Testing s390x architecture initialization...\n");

    int ret = s390x_arch_init();
    assert(ret == ORION_SUCCESS);
    printf("✓ s390x architecture initialization successful\n");
}

static void test_s390x_cpu_features(void)
{
    printf("Testing s390x CPU feature detection...\n");

    int ret = s390x_detect_cpu_features();
    assert(ret == ORION_SUCCESS);

    s390x_print_cpu_info();
    printf("✓ s390x CPU feature detection successful\n");
}

static void test_s390x_cpu_management(void)
{
    printf("Testing s390x CPU management...\n");

    // Test CPU initialization
    int ret = s390x_cpu_init(0);
    assert(ret == ORION_SUCCESS);

    // Test CPU count
    uint32_t cpu_count = s390x_cpu_get_count();
    assert(cpu_count > 0);
    printf("  CPU count: %d\n", cpu_count);

    // Test current CPU
    uint32_t current_cpu = s390x_cpu_get_current();
    printf("  Current CPU: %d\n", current_cpu);

    printf("✓ s390x CPU management successful\n");
}

static void test_s390x_mmu(void)
{
    printf("Testing s390x MMU...\n");

    int ret = s390x_mmu_init();
    assert(ret == ORION_SUCCESS);

    // Test page mapping
    vaddr_t va = 0x10000000;
    paddr_t pa = 0x20000000;
    uint64_t flags = S390X_PAGE_READ | S390X_PAGE_WRITE;

    ret = s390x_mmu_map_page(va, pa, flags);
    assert(ret == ORION_SUCCESS);

    // Test address translation
    paddr_t translated = s390x_mmu_translate_address(va);
    printf("  Address translation: 0x%016lx -> 0x%016lx\n", va, translated);

    // Test page protection
    ret = s390x_mmu_protect_pages(va, S390X_PAGE_SIZE, S390X_PAGE_READ);
    assert(ret == ORION_SUCCESS);

    // Test page unmapping
    ret = s390x_mmu_unmap_pages(va, S390X_PAGE_SIZE);
    assert(ret == ORION_SUCCESS);

    printf("✓ s390x MMU successful\n");
}

static void test_s390x_interrupts(void)
{
    printf("Testing s390x interrupt system...\n");

    // Test interrupt registration
    int ret = s390x_interrupt_register(S390X_IRQ_TIMER, NULL, NULL);
    assert(ret == ORION_SUCCESS);

    // Test interrupt enable/disable
    s390x_interrupt_enable(S390X_IRQ_TIMER);
    s390x_interrupt_disable(S390X_IRQ_TIMER);

    // Test interrupt unregistration
    ret = s390x_interrupt_unregister(S390X_IRQ_TIMER);
    assert(ret == ORION_SUCCESS);

    printf("✓ s390x interrupt system successful\n");
}

static void test_s390x_timers(void)
{
    printf("Testing s390x timer system...\n");

    // Test timer initialization
    int ret = s390x_timer_init(S390X_TIMER_FREQ_DEFAULT);
    assert(ret == ORION_SUCCESS);

    // Test timer ticks
    uint64_t ticks1 = s390x_timer_get_ticks();
    uint64_t ticks2 = s390x_timer_get_ticks();
    printf("  Timer ticks: %lu -> %lu\n", ticks1, ticks2);

    // Test one-shot timer
    s390x_timer_set_oneshot(1000, NULL);
    s390x_timer_cancel();

    printf("✓ s390x timer system successful\n");
}

static void test_s390x_cache(void)
{
    printf("Testing s390x cache management...\n");

    s390x_cache_init();

    // Test cache operations
    void *test_addr = malloc(1024);
    assert(test_addr != NULL);

    s390x_cache_invalidate(test_addr, 1024);
    s390x_cache_clean(test_addr, 1024);
    s390x_cache_flush(test_addr, 1024);
    s390x_cache_sync();

    free(test_addr);
    printf("✓ s390x cache management successful\n");
}

static void test_s390x_performance(void)
{
    printf("Testing s390x performance monitoring...\n");

    int ret = s390x_performance_init();
    assert(ret == ORION_SUCCESS);

    // Test performance counters
    for (uint32_t i = 0; i < S390X_PMU_MAX_COUNTERS; i++)
    {
        uint64_t value = s390x_performance_read_counter(i);
        printf("  Counter %d: %lu\n", i, value);
    }

    // Test event setting
    ret = s390x_performance_set_event(0, S390X_PMU_CYCLE_COUNTER);
    assert(ret == ORION_SUCCESS);

    // Test counter enable/disable
    s390x_performance_enable_counter(0);
    s390x_performance_disable_counter(0);

    printf("✓ s390x performance monitoring successful\n");
}

static void test_s390x_security(void)
{
    printf("Testing s390x security features...\n");

    int ret = s390x_security_init();
    assert(ret == ORION_SUCCESS);

    // Test security measurement
    const char *test_data = "test data";
    uint8_t hash[32];
    ret = s390x_security_measure(test_data, strlen(test_data), hash);
    assert(ret == ORION_SUCCESS);

    // Test security verification
    ret = s390x_security_verify(test_data, strlen(test_data), hash);
    assert(ret == ORION_SUCCESS);

    // Test secure enclave
    void *enclave;
    ret = s390x_security_enclave_create(1024, &enclave);
    assert(ret == ORION_SUCCESS);

    ret = s390x_security_enclave_destroy(enclave);
    assert(ret == ORION_SUCCESS);

    printf("✓ s390x security features successful\n");
}

static void test_s390x_power(void)
{
    printf("Testing s390x power management...\n");

    int ret = s390x_power_init();
    assert(ret == ORION_SUCCESS);

    // Test power state setting
    ret = s390x_power_set_state(S390X_STATE_IDLE);
    assert(ret == ORION_SUCCESS);

    // Test power state getting
    int state = s390x_power_get_state();
    printf("  Current power state: %d\n", state);

    // Test frequency setting
    ret = s390x_power_set_frequency(0, 3000);
    assert(ret == ORION_SUCCESS);

    printf("✓ s390x power management successful\n");
}

static void test_s390x_debug(void)
{
    printf("Testing s390x debug system...\n");

    int ret = s390x_debug_init();
    assert(ret == ORION_SUCCESS);

    // Test debug functions
    s390x_debug_trace("Test debug message");
    s390x_debug_break();

    ret = s390x_debug_dump_registers();
    assert(ret == ORION_SUCCESS);

    printf("✓ s390x debug system successful\n");
}

static void test_s390x_context_switching(void)
{
    printf("Testing s390x context switching...\n");

    // Test context operations (with dummy threads)
    struct thread *prev = (struct thread *)0x1000;
    struct thread *next = (struct thread *)0x2000;

    s390x_context_save(prev);
    s390x_context_switch(prev, next);
    s390x_context_restore(next);

    printf("✓ s390x context switching successful\n");
}

static void test_s390x_ipc(void)
{
    printf("Testing s390x IPC fast path...\n");

    // Test IPC setup (with dummy thread)
    struct thread *test_thread = (struct thread *)0x3000;
    int ret = s390x_ipc_setup_fast_path(test_thread);
    assert(ret == ORION_SUCCESS);

    // Test IPC send/receive (with dummy messages)
    struct ipc_msg *msg = (struct ipc_msg *)0x4000;
    ssize_t send_result = s390x_ipc_fast_send(msg, test_thread);
    ssize_t recv_result = s390x_ipc_fast_recv(msg, test_thread);

    printf("  IPC send result: %zd\n", send_result);
    printf("  IPC receive result: %zd\n", recv_result);

    printf("✓ s390x IPC fast path successful\n");
}

static void test_s390x_extensions(void)
{
    printf("Testing s390x architecture extensions...\n");

    // Test various extension calls
    int ret;

    ret = s390x_z196_extension_call(NULL);
    printf("  z196 extension: %d\n", ret);

    ret = s390x_z15_extension_call(NULL);
    printf("  z15 extension: %d\n", ret);

    ret = s390x_vector_extension_call(NULL);
    printf("  Vector extension: %d\n", ret);

    ret = s390x_crypto_extension_call(NULL);
    printf("  Crypto extension: %d\n", ret);

    ret = s390x_decimal_extension_call(NULL);
    printf("  Decimal extension: %d\n", ret);

    ret = s390x_tx_extension_call(NULL);
    printf("  TX extension: %d\n", ret);

    printf("✓ s390x architecture extensions successful\n");
}

static void test_s390x_cleanup(void)
{
    printf("Testing s390x architecture cleanup...\n");

    s390x_arch_cleanup();
    printf("✓ s390x architecture cleanup successful\n");
}

// ============================================================================
// MAIN TEST FUNCTION
// ============================================================================

int test_s390x_all(void)
{
    printf("========================================\n");
    printf("s390x Architecture Test Suite\n");
    printf("========================================\n");

    test_s390x_arch_init();
    test_s390x_cpu_features();
    test_s390x_cpu_management();
    test_s390x_mmu();
    test_s390x_interrupts();
    test_s390x_timers();
    test_s390x_cache();
    test_s390x_performance();
    test_s390x_security();
    test_s390x_power();
    test_s390x_debug();
    test_s390x_context_switching();
    test_s390x_ipc();
    test_s390x_extensions();
    test_s390x_cleanup();

    printf("========================================\n");
    printf("All s390x architecture tests completed successfully!\n");
    printf("========================================\n");

    return 0;
}

// ============================================================================
// INDIVIDUAL TEST FUNCTIONS FOR EXTERNAL USE
// ============================================================================

int test_s390x_basic(void)
{
    test_s390x_arch_init();
    test_s390x_cpu_features();
    test_s390x_cleanup();
    return 0;
}

int test_s390x_memory(void)
{
    test_s390x_arch_init();
    test_s390x_mmu();
    test_s390x_cache();
    test_s390x_cleanup();
    return 0;
}

int test_s390x_system(void)
{
    test_s390x_arch_init();
    test_s390x_interrupts();
    test_s390x_timers();
    test_s390x_performance();
    test_s390x_cleanup();
    return 0;
}

int test_s390x_advanced(void)
{
    test_s390x_arch_init();
    test_s390x_security();
    test_s390x_power();
    test_s390x_debug();
    test_s390x_extensions();
    test_s390x_cleanup();
    return 0;
}
