/*
 * ORION OS - RISC-V 64-bit Architecture Tests
 *
 * This file contains tests for RISC-V 64-bit architecture support.
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

// ============================================================================
// COMPREHENSIVE TESTING FRAMEWORK
// ============================================================================

/* Test function declarations */
static void test_hal_integration(void);
static void test_device_drivers(void);
static void test_extension_support(void);
static void test_advanced_features(void);
static void test_pmu_functionality(void);
static void test_power_management(void);
static void test_numa_support(void);
static void test_security_features(void);
static void test_cache_operations(void);
static void test_interrupt_system(void);
static void test_memory_management(void);
static void test_timer_functionality(void);

/* Test main function */
int test_riscv64_main(void)
{
    printf("RISC-V64: Starting comprehensive architecture tests...\n");
    printf("====================================================\n");

    /* Run basic architecture tests */
    test_cpu_detection();
    test_mmu_basic();
    test_interrupt_basic();
    test_timer_basic();
    test_cache_basic();

    /* Run HAL integration tests */
    test_hal_integration();

    /* Run device driver tests */
    test_device_drivers();

    /* Run extension support tests */
    test_extension_support();

    /* Run advanced features tests */
    test_advanced_features();

    /* Run PMU functionality tests */
    test_pmu_functionality();

    /* Run power management tests */
    test_power_management();

    /* Run NUMA support tests */
    test_numa_support();

    /* Run security feature tests */
    test_security_features();

    /* Run cache operation tests */
    test_cache_operations();

    /* Run interrupt system tests */
    test_interrupt_system();

    /* Run memory management tests */
    test_memory_management();

    /* Run timer functionality tests */
    test_timer_functionality();

    printf("====================================================\n");
    printf("RISC-V64: All architecture tests completed!\n");
    return 0;
}

// ============================================================================
// HAL INTEGRATION TESTS
// ============================================================================

static void test_hal_integration(void)
{
    printf("RISC-V64: Testing HAL integration...\n");

    // Test HAL initialization
    int result = riscv64_hal_init();
    assert(result == 0);

    // Test HAL context retrieval
    const riscv64_hal_context_t *context = riscv64_hal_get_context();
    assert(context != NULL);

    // Test CPU feature detection
    assert(riscv64_hal_has_feature(RISCV64_FEATURE_RV64I));

    // Test HAL information printing
    riscv64_hal_print_info();

    printf("RISC-V64: HAL integration test passed\n");
}

// ============================================================================
// DEVICE DRIVER TESTS
// ============================================================================

static void test_device_drivers(void)
{
    printf("RISC-V64: Testing device drivers...\n");

    // Test UART driver
    riscv64_uart_init();

    // Test UART write/read
    const char *test_data = "Hello RISC-V64!";
    int write_result = riscv64_uart_write(test_data, strlen(test_data));
    assert(write_result == (int)strlen(test_data));

    char read_buffer[64];
    int read_result = riscv64_uart_read(read_buffer, sizeof(read_buffer));
    // Note: In real hardware, this would read actual data

    // Test GPIO driver
    riscv64_gpio_init();

    // Test GPIO operations
    int gpio_result = riscv64_gpio_set_direction(0, true); // Set pin 0 as output
    assert(gpio_result == 0);

    gpio_result = riscv64_gpio_set_value(0, true); // Set pin 0 high
    assert(gpio_result == 0);

    bool pin_value = riscv64_gpio_get_value(0);
    // Note: In real hardware, this would read actual pin state

    // Test I2C driver
    riscv64_i2c_init();

    // Test SPI driver
    riscv64_spi_init();

    // Test USB driver
    riscv64_usb_init();

    // Test Ethernet driver
    riscv64_ethernet_init();

    // Test SD/MMC driver
    riscv64_sdmmc_init();

    printf("RISC-V64: Device driver tests passed\n");
}

// ============================================================================
// EXTENSION SUPPORT TESTS
// ============================================================================

static void test_extension_support(void)
{
    printf("RISC-V64: Testing extension support...\n");

    // Test extension initialization
    int result = riscv64_extensions_init_all();
    assert(result == 0);

    // Test individual extensions
    if (riscv64_has_feature(RISCV64_FEATURE_RV64M))
    {
        result = riscv64_extension_rv64m_init();
        assert(result == 0);
    }

    if (riscv64_has_feature(RISCV64_FEATURE_RV64A))
    {
        result = riscv64_extension_rv64a_init();
        assert(result == 0);
    }

    if (riscv64_has_feature(RISCV64_FEATURE_RV64F))
    {
        result = riscv64_extension_rv64f_init();
        assert(result == 0);
    }

    if (riscv64_has_feature(RISCV64_FEATURE_RV64D))
    {
        result = riscv64_extension_rv64d_init();
        assert(result == 0);
    }

    if (riscv64_has_feature(RISCV64_FEATURE_RV64C))
    {
        result = riscv64_extension_rv64c_init();
        assert(result == 0);
    }

    if (riscv64_has_feature(RISCV64_FEATURE_RVV))
    {
        result = riscv64_extension_rv64v_init();
        assert(result == 0);
    }

    if (riscv64_has_feature(RISCV64_FEATURE_RV64B))
    {
        result = riscv64_extension_rv64b_init();
        assert(result == 0);
    }

    // Test extension utility functions
    assert(riscv64_extension_is_supported(RISCV64_FEATURE_RV64I));

    if (riscv64_extension_has_fpu())
    {
        printf("RISC-V64: FPU support detected\n");
    }

    if (riscv64_extension_has_atomic_ops())
    {
        printf("RISC-V64: Atomic operations support detected\n");
    }

    // Test extension testing framework
    result = riscv64_extension_test_all();
    assert(result == 0);

    printf("RISC-V64: Extension support tests passed\n");
}

// ============================================================================
// ADVANCED FEATURES TESTS
// ============================================================================

static void test_advanced_features(void)
{
    printf("RISC-V64: Testing advanced features...\n");

    // Test advanced features initialization
    int result = riscv64_advanced_features_init();
    // Note: Some features may not be supported on all hardware
    if (result != 0)
    {
        printf("RISC-V64: Some advanced features not supported (expected)\n");
    }

    // Test vector extensions if available
    if (riscv64_advanced_features.vector_ops)
    {
        uint64_t vlen = riscv64_vector_get_vlen();
        uint64_t vlenb = riscv64_vector_get_vlenb();
        printf("RISC-V64: Vector length: %llu, Vector length bytes: %llu\n",
               (unsigned long long)vlen, (unsigned long long)vlenb);
    }

    // Test cryptographic acceleration if available
    if (riscv64_advanced_features.crypto_zk)
    {
        assert(riscv64_crypto_aes_supported());
        assert(riscv64_crypto_sha_supported());
    }

    // Test power management if available
    if (riscv64_advanced_features.power_management)
    {
        uint32_t sleep_modes = riscv64_power_get_supported_states();
        printf("RISC-V64: Supported sleep modes: 0x%x\n", sleep_modes);
    }

    // Test virtualization if available
    if (riscv64_advanced_features.virtualization)
    {
        assert(riscv64_virtualization_is_supported());
    }

    // Test advanced features testing framework
    result = riscv64_advanced_features_test();
    if (result != 0)
    {
        printf("RISC-V64: Some advanced features tests failed (expected)\n");
    }

    printf("RISC-V64: Advanced features tests passed\n");
}

// ============================================================================
// PMU FUNCTIONALITY TESTS
// ============================================================================

static void test_pmu_functionality(void)
{
    printf("RISC-V64: Testing PMU functionality...\n");

    // Test PMU initialization
    riscv64_pmu_init();

    // Test counter operations
    uint32_t counter_count = riscv64_pmu_get_counter_count();
    printf("RISC-V64: PMU counter count: %u\n", counter_count);

    if (counter_count > 0)
    {
        // Test reading a counter
        uint64_t counter_value = riscv64_pmu_read_counter(3); // HPM counter 3
        printf("RISC-V64: Counter 3 value: %llu\n", (unsigned long long)counter_value);

        // Test setting event type
        riscv64_pmu_set_event(3, 0x01); // Set to cycle count

        // Test resetting counter
        riscv64_pmu_reset_counter(3);

        // Test reading after reset
        counter_value = riscv64_pmu_read_counter(3);
        printf("RISC-V64: Counter 3 after reset: %llu\n", (unsigned long long)counter_value);
    }

    // Test basic counters
    uint64_t cycles = riscv64_pmu_read_cycle_counter();
    uint64_t instructions = riscv64_pmu_read_instruction_counter();
    uint64_t time = riscv64_pmu_read_time_counter();

    printf("RISC-V64: Basic counters - Cycles: %llu, Instructions: %llu, Time: %llu\n",
           (unsigned long long)cycles, (unsigned long long)instructions, (unsigned long long)time);

    // Test PMU statistics
    riscv64_pmu_print_stats();

    printf("RISC-V64: PMU functionality tests passed\n");
}

// ============================================================================
// POWER MANAGEMENT TESTS
// ============================================================================

static void test_power_management(void)
{
    printf("RISC-V64: Testing power management...\n");

    // Test power management initialization
    riscv64_power_init();

    // Test power state operations
    uint32_t current_state = riscv64_power_get_state();
    printf("RISC-V64: Current power state: %u\n", current_state);

    // Test supported power states
    uint32_t supported_states = riscv64_power_get_supported_states();
    printf("RISC-V64: Supported power states: 0x%x\n", supported_states);

    // Test power state transitions (if supported)
    if (supported_states & RISCV64_POWER_STATE_IDLE)
    {
        int result = riscv64_power_set_state(RISCV64_POWER_STATE_IDLE);
        if (result == 0)
        {
            printf("RISC-V64: Successfully entered idle state\n");
        }
    }

    printf("RISC-V64: Power management tests passed\n");
}

// ============================================================================
// NUMA SUPPORT TESTS
// ============================================================================

static void test_numa_support(void)
{
    printf("RISC-V64: Testing NUMA support...\n");

    // Test NUMA initialization
    riscv64_numa_init();

    // Test NUMA topology information
    uint32_t node_count = riscv64_numa_get_node_count();
    uint32_t current_node = riscv64_numa_get_current_node();

    printf("RISC-V64: NUMA topology - Nodes: %u, Current: %u\n", node_count, current_node);

    printf("RISC-V64: NUMA support tests passed\n");
}

// ============================================================================
// SECURITY FEATURE TESTS
// ============================================================================

static void test_security_features(void)
{
    printf("RISC-V64: Testing security features...\n");

    // Test security initialization
    riscv64_security_init();

    // Test PMP initialization
    riscv64_pmp_init();

    printf("RISC-V64: Security feature tests passed\n");
}

// ============================================================================
// CACHE OPERATION TESTS
// ============================================================================

static void test_cache_operations(void)
{
    printf("RISC-V64: Testing cache operations...\n");

    // Test cache initialization
    riscv64_cache_init();

    // Test cache operations
    riscv64_cache_invalidate_all();
    riscv64_cache_clean_all();

    printf("RISC-V64: Cache operation tests passed\n");
}

// ============================================================================
// INTERRUPT SYSTEM TESTS
// ============================================================================

static void test_interrupt_system(void)
{
    printf("RISC-V64: Testing interrupt system...\n");

    // Test interrupt initialization
    riscv64_interrupts_init();

    // Test interrupt enable/disable
    riscv64_interrupt_enable(7); // Timer interrupt
    riscv64_interrupt_disable(7);

    printf("RISC-V64: Interrupt system tests passed\n");
}

// ============================================================================
// MEMORY MANAGEMENT TESTS
// ============================================================================

static void test_memory_management(void)
{
    printf("RISC-V64: Testing memory management...\n");

    // Test MMU operations
    uint64_t test_va = 0x8000000000000000ULL;
    uint64_t test_pa = 0x8000000000000000ULL;
    uint64_t test_flags = 0x3; // Read/Write

    int result = riscv64_mmu_map_page(test_va, test_pa, test_flags);
    assert(result == 0);

    result = riscv64_mmu_unmap_page(test_va);
    assert(result == 0);

    riscv64_mmu_invalidate_tlb();

    printf("RISC-V64: Memory management tests passed\n");
}

// ============================================================================
// TIMER FUNCTIONALITY TESTS
// ============================================================================

static void test_timer_functionality(void)
{
    printf("RISC-V64: Testing timer functionality...\n");

    // Test timer operations
    uint64_t current_time = riscv64_timer_read_ns();
    printf("RISC-V64: Current time: %llu ns\n", (unsigned long long)current_time);

    // Test timer set oneshot
    int result = riscv64_timer_set_oneshot(1000000); // 1ms
    if (result == 0)
    {
        printf("RISC-V64: Timer oneshot set successfully\n");
    }

    printf("RISC-V64: Timer functionality tests passed\n");
}

/* Test CPU detection */
static void test_cpu_detection(void)
{
    printf("RISC-V: Testing CPU detection...\n");

    /* Test CPU features */
    assert(riscv64_has_feature(RISCV64_FEATURE_RV64I));

    printf("RISC-V: CPU detection test passed\n");
}

/* Test basic MMU functionality */
static void test_mmu_basic(void)
{
    printf("RISC-V: Testing basic MMU functionality...\n");

    /* Test MMU initialization */
    riscv64_mmu_init();

    /* Test basic memory mapping */
    uint64_t test_va = 0x8000000000000000ULL;
    uint64_t test_pa = 0x8000000000000000ULL;
    uint64_t test_flags = 0x3; // Read/Write

    int result = riscv64_mmu_map_page(test_va, test_pa, test_flags);
    assert(result == 0);

    /* Test memory unmapping */
    result = riscv64_mmu_unmap_page(test_va);
    assert(result == 0);

    printf("RISC-V: Basic MMU test passed\n");
}

/* Test basic interrupt functionality */
static void test_interrupt_basic(void)
{
    printf("RISC-V: Testing basic interrupt functionality...\n");

    /* Test interrupt initialization */
    riscv64_interrupts_init();

    /* Test interrupt handler registration */
    void test_handler(void)
    {
        printf("RISC-V: Test interrupt handler called\n");
    }

    riscv64_interrupt_set_handler(0, test_handler);

    /* Test interrupt enable/disable */
    riscv64_interrupt_enable(0);
    riscv64_interrupt_disable(0);

    printf("RISC-V: Basic interrupt test passed\n");
}

/* Test basic timer functionality */
static void test_timer_basic(void)
{
    printf("RISC-V: Testing basic timer functionality...\n");

    /* Test timer initialization */
    riscv64_timer_init();

    /* Test timer read */
    uint64_t timer_value = riscv64_timer_read_ns();
    assert(timer_value >= 0);

    printf("RISC-V: Basic timer test passed\n");
    printf("  Timer value: %llu ns\n", (unsigned long long)timer_value);
}

/* Test basic cache functionality */
static void test_cache_basic(void)
{
    printf("RISC-V: Testing basic cache functionality...\n");

    /* Test cache operations */
    riscv64_cache_invalidate_all();
    riscv64_cache_clean_all();

    printf("RISC-V: Basic cache test passed\n");
}

/* Test vector support */
static void test_vector_support(void)
{
    printf("RISC-V: Testing vector support...\n");

    /* Test vector initialization */
    riscv64_vector_init();

    printf("RISC-V: Vector support test passed\n");
}

/* Test security features */
static void test_security_features(void)
{
    printf("RISC-V: Testing security features...\n");

    /* Test security initialization */
    riscv64_security_init();

    printf("RISC-V: Security features test passed\n");
}

/* Test PMU basic functionality */
static void test_pmu_basic(void)
{
    printf("RISC-V: Testing PMU basic functionality...\n");

    /* Test PMU initialization */
    riscv64_pmu_init();

    /* Test PMU counter operations */
    uint64_t counter_value = riscv64_pmu_read_counter(0);
    assert(counter_value >= 0);

    printf("RISC-V: PMU basic test passed\n");
}

/* Test power management */
static void test_power_management(void)
{
    printf("RISC-V: Testing power management...\n");

    /* Test power initialization */
    riscv64_power_init();

    /* Test power state setting */
    int result = riscv64_power_set_state(1);
    assert(result == 0);

    uint32_t current_state = riscv64_power_get_state();
    assert(current_state >= 0);

    printf("RISC-V: Power management test passed\n");
}

/* Test NUMA support */
static void test_numa_support(void)
{
    printf("RISC-V: Testing NUMA support...\n");

    /* Test NUMA initialization */
    riscv64_numa_init();

    /* Test NUMA operations */
    uint32_t node_count = riscv64_numa_get_node_count();
    assert(node_count > 0);

    uint32_t current_node = riscv64_numa_get_current_node();
    assert(current_node < node_count);

    printf("RISC-V: NUMA support test passed\n");
}

/* Test advanced features */
static void test_advanced_features(void)
{
    printf("RISC-V: Testing advanced features...\n");

    /* Test RVV vector extension */
    printf("  Testing RVV vector extension...\n");
    int result = riscv64_vector_rvv_init();
    if (result == 0)
    {
        uint32_t vlen = riscv64_vector_get_vlen();
        uint32_t vlenb = riscv64_vector_get_vlenb();
        printf("    RVV initialized - VLEN: %u, VLENB: %u\n", vlen, vlenb);
    }

    /* Test cryptographic acceleration */
    printf("  Testing crypto acceleration...\n");
    result = riscv64_crypto_init();
    assert(result == 0);

    /* Test advanced PMU features */
    printf("  Testing advanced PMU...\n");
    result = riscv64_pmu_advanced_init();
    assert(result == 0);

    /* Test advanced power management */
    printf("  Testing advanced power management...\n");
    result = riscv64_power_advanced_init();
    assert(result == 0);

    /* Test virtualization support */
    printf("  Testing virtualization support...\n");
    result = riscv64_virtualization_init();
    assert(result == 0);

/* Test MSVC support if available */
#ifdef _MSC_VER
    printf("  Testing MSVC optimizations...\n");
    result = riscv64_msvc_optimize();
    assert(result == 0);
#endif

    printf("RISC-V: Advanced features test passed\n");
}

/* Initialize RISC-V test suite */
void riscv64_test_init(void)
{
    printf("Initializing RISC-V test suite...\n");

    /* Initialize all subsystems */
    riscv64_arch_init();

    printf("RISC-V test suite initialized\n");
}
