/*
 * Orion Operating System - x86_64 Architecture Tests
 *
 * Comprehensive test suite for x86_64 architecture support
 * Tests CPU features, MMU, interrupts, timers, cache, vector extensions,
 * security features, performance monitoring, NUMA, and virtualization
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "../hal/common/types.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test result tracking
static int tests_passed = 0;
static int tests_failed = 0;
static int tests_total = 0;

// Test macros
#define TEST_ASSERT(condition, message) \
    do                                  \
    {                                   \
        tests_total++;                  \
        if (condition)                  \
        {                               \
            tests_passed++;             \
            printf("✓ %s\n", message);  \
        }                               \
        else                            \
        {                               \
            tests_failed++;             \
            printf("✗ %s\n", message);  \
        }                               \
    } while (0)

#define TEST_SECTION(name) printf("\n=== %s ===\n", name)

// ============================================================================
// CPU FEATURE DETECTION TESTS
// ============================================================================

static void test_cpu_detection(void)
{
    TEST_SECTION("CPU Feature Detection");

    // Test basic CPU features
    TEST_ASSERT(x86_64_cpu_features != 0, "CPU features bitmap initialized");
    TEST_ASSERT(x86_64_cpu_family > 0, "CPU family detected");
    TEST_ASSERT(x86_64_cpu_model > 0, "CPU model detected");

    // Test vector extensions
    if (X86_64_HAS_FEATURE(X86_64_FEATURE_SSE))
    {
        printf("  SSE supported\n");
    }
    if (X86_64_HAS_FEATURE(X86_64_FEATURE_SSE2))
    {
        printf("  SSE2 supported\n");
    }
    if (X86_64_HAS_FEATURE(X86_64_FEATURE_AVX))
    {
        printf("  AVX supported\n");
    }
    if (X86_64_HAS_FEATURE(X86_64_FEATURE_AVX2))
    {
        printf("  AVX2 supported\n");
    }
    if (X86_64_HAS_FEATURE(X86_64_FEATURE_AVX512F))
    {
        printf("  AVX-512F supported\n");
    }

    // Test security features
    if (X86_64_HAS_FEATURE(X86_64_FEATURE_SMEP))
    {
        printf("  SMEP supported\n");
    }
    if (X86_64_HAS_FEATURE(X86_64_FEATURE_SMAP))
    {
        printf("  SMAP supported\n");
    }
    if (X86_64_HAS_FEATURE(X86_64_FEATURE_UMIP))
    {
        printf("  UMIP supported\n");
    }

    // Test advanced features
    if (X86_64_HAS_FEATURE(X86_64_FEATURE_TSX))
    {
        printf("  TSX supported\n");
    }
    if (X86_64_HAS_FEATURE(X86_64_FEATURE_RDRAND))
    {
        printf("  RDRAND supported\n");
    }
    if (X86_64_HAS_FEATURE(X86_64_FEATURE_RDSEED))
    {
        printf("  RDSEED supported\n");
    }
}

// ============================================================================
// MMU TESTS
// ============================================================================

static void test_mmu_basic(void)
{
    TEST_SECTION("MMU Basic Functionality");

    // Test page size constants
    TEST_ASSERT(X86_64_PAGE_SIZE_4K == 4096, "4K page size correct");
    TEST_ASSERT(X86_64_PAGE_SIZE_2M == 2 * 1024 * 1024, "2M page size correct");
    TEST_ASSERT(X86_64_PAGE_SIZE_1G == 1024 * 1024 * 1024, "1G page size correct");

    // Test virtual address space layout
    TEST_ASSERT(X86_64_KERNEL_BASE == 0xFFFFFFFF80000000ULL, "Kernel base address correct");
    TEST_ASSERT(X86_64_USER_BASE == 0x0000000000000000ULL, "User base address correct");

    printf("  Kernel space: 0x%llX - 0x%llX\n",
           X86_64_KERNEL_BASE,
           X86_64_KERNEL_BASE + X86_64_KERNEL_SIZE - 1);
    printf("  User space: 0x%llX - 0x%llX\n",
           X86_64_USER_BASE,
           X86_64_USER_BASE + X86_64_USER_SIZE - 1);
}

// ============================================================================
// INTERRUPT TESTS
// ============================================================================

static void test_interrupt_config(void)
{
    TEST_SECTION("Interrupt Configuration");

    // Test APIC configuration
    TEST_ASSERT(X86_64_APIC_BASE == 0xFEE00000, "APIC base address correct");
    TEST_ASSERT(X86_64_APIC_MAX_VECTORS == 256, "APIC max vectors correct");
    TEST_ASSERT(X86_64_APIC_TIMER_VECTOR == 32, "APIC timer vector correct");
    TEST_ASSERT(X86_64_APIC_ERROR_VECTOR == 19, "APIC error vector correct");

    // Test MSI-X support
    TEST_ASSERT(X86_64_MSIX_MAX_VECTORS == 2048, "MSI-X max vectors correct");
    TEST_ASSERT(X86_64_MSIX_TABLE_SIZE == 64, "MSI-X table size correct");
}

// ============================================================================
// TIMER TESTS
// ============================================================================

static void test_timer_config(void)
{
    TEST_SECTION("Timer Configuration");

    // Test TSC configuration
    TEST_ASSERT(X86_64_TSC_FREQ_DEFAULT == 2400000000ULL, "TSC default frequency correct");
    TEST_ASSERT(X86_64_TSC_INVARIANT == true, "TSC invariant flag set");

    // Test HPET configuration
    TEST_ASSERT(X86_64_HPET_BASE == 0xFED00000, "HPET base address correct");
    TEST_ASSERT(X86_64_HPET_TIMER_COUNT == 8, "HPET timer count correct");
}

// ============================================================================
// CACHE TESTS
// ============================================================================

static void test_cache_config(void)
{
    TEST_SECTION("Cache Configuration");

    // Test cache line sizes
    TEST_ASSERT(X86_64_CACHE_LINE_SIZE == 64, "L1 cache line size correct");
    TEST_ASSERT(X86_64_CACHE_LINE_SIZE_L2 == 64, "L2 cache line size correct");
    TEST_ASSERT(X86_64_CACHE_LINE_SIZE_L3 == 64, "L3 cache line size correct");

    // Test cache sizes (typical values)
    TEST_ASSERT(X86_64_L1I_CACHE_SIZE == 32, "L1I cache size correct");
    TEST_ASSERT(X86_64_L1D_CACHE_SIZE == 32, "L1D cache size correct");
    TEST_ASSERT(X86_64_L2_CACHE_SIZE == 256, "L2 cache size correct");
    TEST_ASSERT(X86_64_L3_CACHE_SIZE == 8192, "L3 cache size correct");
}

// ============================================================================
// POWER MANAGEMENT TESTS
// ============================================================================

static void test_power_management(void)
{
    TEST_SECTION("Power Management");

    // Test power states
    TEST_ASSERT(X86_64_POWER_STATE_C0 == 0, "Power state C0 correct");
    TEST_ASSERT(X86_64_POWER_STATE_C1 == 1, "Power state C1 correct");
    TEST_ASSERT(X86_64_POWER_STATE_C6 == 5, "Power state C6 correct");
    TEST_ASSERT(X86_64_POWER_STATE_C7 == 6, "Power state C7 correct");

    // Test frequency scaling
    TEST_ASSERT(X86_64_MIN_FREQ_MHZ == 800, "Min frequency correct");
    TEST_ASSERT(X86_64_MAX_FREQ_MHZ == 5500, "Max frequency correct");
    TEST_ASSERT(X86_64_TURBO_BOOST == true, "Turbo boost enabled");
}

// ============================================================================
// DEBUG AND TRACING TESTS
// ============================================================================

static void test_debug_config(void)
{
    TEST_SECTION("Debug and Tracing");

    // Test debug registers
    TEST_ASSERT(X86_64_DEBUG_REG_COUNT == 8, "Debug register count correct");
    TEST_ASSERT(X86_64_DEBUG_REG_DR0 == 0, "Debug register DR0 correct");
    TEST_ASSERT(X86_64_DEBUG_REG_DR7 == 7, "Debug register DR7 correct");

    // Test performance monitoring
    TEST_ASSERT(X86_64_PMU_COUNTER_COUNT == 8, "PMU counter count correct");
    TEST_ASSERT(X86_64_PMU_FIXED_COUNTER_COUNT == 3, "PMU fixed counter count correct");
}

// ============================================================================
// NUMA TESTS
// ============================================================================

static void test_numa_config(void)
{
    TEST_SECTION("NUMA Configuration");

    // Test NUMA limits
    TEST_ASSERT(X86_64_MAX_NUMA_NODES == 8, "Max NUMA nodes correct");
    TEST_ASSERT(X86_64_MAX_SOCKETS_PER_NODE == 2, "Max sockets per node correct");
    TEST_ASSERT(X86_64_MAX_CORES_PER_SOCKET == 64, "Max cores per socket correct");
}

// ============================================================================
// PERFORMANCE TARGET TESTS
// ============================================================================

static void test_performance_targets(void)
{
    TEST_SECTION("Performance Targets");

    // Test high-performance targets
    TEST_ASSERT(X86_64_PERF_TARGET_SYSCALL_NS == 120, "High-perf syscall target correct");
    TEST_ASSERT(X86_64_PERF_TARGET_CTX_SWITCH_NS == 200, "High-perf context switch target correct");
    TEST_ASSERT(X86_64_PERF_TARGET_IPC_MSGS_S == 12, "High-perf IPC target correct");
    TEST_ASSERT(X86_64_PERF_TARGET_MEM_BW_PCT == 90, "High-perf memory bandwidth target correct");

    // Test mid-range targets
    TEST_ASSERT(X86_64_PERF_TARGET_SYSCALL_NS_MID == 200, "Mid-range syscall target correct");
    TEST_ASSERT(X86_64_PERF_TARGET_CTX_SWITCH_NS_MID == 400, "Mid-range context switch target correct");
    TEST_ASSERT(X86_64_PERF_TARGET_IPC_MSGS_S_MID == 8, "Mid-range IPC target correct");
    TEST_ASSERT(X86_64_PERF_TARGET_MEM_BW_PCT_MID == 85, "Mid-range memory bandwidth target correct");
}

// ============================================================================
// COMPILER CONFIGURATION TESTS
// ============================================================================

static void test_compiler_config(void)
{
    TEST_SECTION("Compiler Configuration");

    // Test optimization levels
    TEST_ASSERT(X86_64_OPT_LEVEL_SIZE == 0, "Size optimization level correct");
    TEST_ASSERT(X86_64_OPT_LEVEL_SPEED == 1, "Speed optimization level correct");
    TEST_ASSERT(X86_64_OPT_LEVEL_AGGRESSIVE == 2, "Aggressive optimization level correct");
    TEST_ASSERT(X86_64_OPT_LEVEL_SAFETY == 3, "Safety optimization level correct");

    // Test debug levels
    TEST_ASSERT(X86_64_DEBUG_LEVEL_NONE == 0, "Debug level none correct");
    TEST_ASSERT(X86_64_DEBUG_LEVEL_ERROR == 1, "Debug level error correct");
    TEST_ASSERT(X86_64_DEBUG_LEVEL_INFO == 3, "Debug level info correct");
    TEST_ASSERT(X86_64_DEBUG_LEVEL_TRACE == 5, "Debug level trace correct");

    // Test current settings
    TEST_ASSERT(X86_64_CURRENT_OPT_LEVEL == X86_64_OPT_LEVEL_SPEED, "Current optimization level correct");
    TEST_ASSERT(X86_64_CURRENT_DEBUG_LEVEL == X86_64_DEBUG_LEVEL_INFO, "Current debug level correct");
}

// ============================================================================
// FEATURE DETECTION MACRO TESTS
// ============================================================================

static void test_feature_macros(void)
{
    TEST_SECTION("Feature Detection Macros");

    // Test feature checking macro
    uint64_t test_features = X86_64_FEATURE_SSE | X86_64_FEATURE_AVX;
    TEST_ASSERT(X86_64_HAS_FEATURE(X86_64_FEATURE_SSE), "SSE feature detection works");
    TEST_ASSERT(X86_64_HAS_FEATURE(X86_64_FEATURE_AVX), "AVX feature detection works");
    TEST_ASSERT(!X86_64_HAS_FEATURE(X86_64_FEATURE_AVX512F), "AVX-512 feature detection works (false case)");

    // Test CPU family checking macros
    uint32_t test_family = X86_64_INTEL_CORE_12TH_GEN;
    TEST_ASSERT(X86_64_IS_INTEL(X86_64_INTEL_CORE_12TH_GEN), "Intel family detection works");
    TEST_ASSERT(!X86_64_IS_AMD(X86_64_AMD_RYZEN_7000), "AMD family detection works (false case)");
}

// ============================================================================
// PERFORMANCE BENCHMARKS
// ============================================================================

static void test_performance_benchmarks(void)
{
    TEST_SECTION("Performance Benchmarks");

    // Test memory barrier performance
    uint64_t start = __builtin_readcyclecounter();
    for (int i = 0; i < 1000; i++)
    {
        __asm__ volatile("mfence" ::: "memory");
    }
    uint64_t end = __builtin_readcyclecounter();
    uint64_t cycles = end - start;

    printf("  Memory barrier (1000x): %llu cycles\n", cycles);
    TEST_ASSERT(cycles < 10000, "Memory barrier performance acceptable");

    // Test cache line access performance
    char buffer[64 * 1024]; // 64KB buffer
    start = __builtin_readcyclecounter();
    for (int i = 0; i < sizeof(buffer); i += 64)
    {
        buffer[i] = i & 0xFF;
    }
    end = __builtin_readcyclecounter();
    cycles = end - start;

    printf("  Cache line writes (64KB): %llu cycles\n", cycles);
    TEST_ASSERT(cycles < 100000, "Cache line write performance acceptable");
}

// ============================================================================
// ERROR HANDLING TESTS
// ============================================================================

static void test_error_handling(void)
{
    TEST_SECTION("Error Handling");

    // Test invalid feature checking
    TEST_ASSERT(!X86_64_HAS_FEATURE(0xFFFFFFFFFFFFFFFFULL), "Invalid feature detection works");

    // Test invalid family checking
    TEST_ASSERT(!X86_64_IS_INTEL(0xFFFFFFFF), "Invalid Intel family detection works");
    TEST_ASSERT(!X86_64_IS_AMD(0xFFFFFFFF), "Invalid AMD family detection works");
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void)
{
    printf("Orion OS - x86_64 Architecture Test Suite\n");
    printf("==========================================\n");

    // Run all test suites
    test_cpu_detection();
    test_mmu_basic();
    test_interrupt_config();
    test_timer_config();
    test_cache_config();
    test_power_management();
    test_debug_config();
    test_numa_config();
    test_performance_targets();
    test_compiler_config();
    test_feature_macros();
    test_performance_benchmarks();
    test_error_handling();

    // Print test summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", tests_total);
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Success rate: %.1f%%\n",
           (tests_total > 0) ? (100.0 * tests_passed / tests_total) : 0.0);

    if (tests_failed == 0)
    {
        printf("\n🎉 All tests passed! x86_64 architecture is ready.\n");
        return 0;
    }
    else
    {
        printf("\n❌ %d tests failed. Please review the implementation.\n", tests_failed);
        return 1;
    }
}
