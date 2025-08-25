/*
 * Orion Operating System - x86_64 Advanced Features
 *
 * Implementation of advanced x86_64 features including:
 * - Performance Monitoring Unit (PMU)
 * - Power management and frequency scaling
 * - Cache management and optimization
 * - NUMA topology detection and management
 * - Advanced security features (CET, SGX, SEV)
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

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// CPU features bitmap (populated by detect_cpu_features)
uint64_t x86_64_cpu_features = 0;

// CPU family and model identifiers
uint32_t x86_64_cpu_family = 0;
uint32_t x86_64_cpu_model = 0;

// NUMA topology information
static struct
{
    uint32_t node_count;
    uint32_t current_node;
    uint32_t socket_count;
    uint32_t cores_per_socket;
} numa_topology = {0};

// Power management state
static struct
{
    uint32_t current_state;
    uint32_t current_frequency_mhz;
    bool turbo_boost_enabled;
} power_state = {0};

// Cache information (auto-detected)
static struct
{
    uint32_t l1i_size_kb;
    uint32_t l1d_size_kb;
    uint32_t l2_size_kb;
    uint32_t l3_size_kb;
    uint32_t line_size;
} cache_info = {0};

// ============================================================================
// CPU FEATURE DETECTION
// ============================================================================

void x86_64_detect_cpu_features(void)
{
    uint32_t eax, ebx, ecx, edx;

    // Get CPU vendor and basic info
    cpuid(0, &eax, &ebx, &ecx, &edx);

    // Check if Intel or AMD
    bool is_intel = (ebx == 0x756E6547) && (edx == 0x49656E69) && (ecx == 0x6C65746E);
    bool is_amd = (ebx == 0x68747541) && (edx == 0x69746E65) && (ecx == 0x444D4163);

    if (is_intel)
    {
        printf("CPU Vendor: Intel\n");
    }
    else if (is_amd)
    {
        printf("CPU Vendor: AMD\n");
    }
    else
    {
        printf("CPU Vendor: Unknown\n");
    }

    // Get CPU family and model
    cpuid(1, &eax, &ebx, &ecx, &edx);
    x86_64_cpu_family = (eax >> 8) & 0xF;
    x86_64_cpu_model = (eax >> 4) & 0xF;

    if ((eax >> 20) & 0xFF)
    {
        x86_64_cpu_model |= ((eax >> 16) & 0xF) << 4;
    }

    printf("CPU Family: %u, Model: %u\n", x86_64_cpu_family, x86_64_cpu_model);

    // Detect basic features (CPUID.1)
    if (edx & (1 << 25))
        x86_64_cpu_features |= X86_64_FEATURE_SSE;
    if (edx & (1 << 26))
        x86_64_cpu_features |= X86_64_FEATURE_SSE2;
    if (ecx & (1 << 0))
        x86_64_cpu_features |= X86_64_FEATURE_SSE3;
    if (ecx & (1 << 9))
        x86_64_cpu_features |= X86_64_FEATURE_SSSE3;
    if (ecx & (1 << 19))
        x86_64_cpu_features |= X86_64_FEATURE_SSE41;
    if (ecx & (1 << 20))
        x86_64_cpu_features |= X86_64_FEATURE_SSE42;
    if (ecx & (1 << 28))
        x86_64_cpu_features |= X86_64_FEATURE_AVX;
    if (ecx & (1 << 30))
        x86_64_cpu_features |= X86_64_FEATURE_RDRAND;

    // Detect extended features (CPUID.7)
    cpuid(7, &eax, &ebx, &ecx, &edx);
    if (ebx & (1 << 5))
        x86_64_cpu_features |= X86_64_FEATURE_AVX2;
    if (ebx & (1 << 7))
        x86_64_cpu_features |= X86_64_FEATURE_SMEP;
    if (ebx & (1 << 20))
        x86_64_cpu_features |= X86_64_FEATURE_SMAP;
    if (ecx & (1 << 2))
        x86_64_cpu_features |= X86_64_FEATURE_UMIP;
    if (ebx & (1 << 18))
        x86_64_cpu_features |= X86_64_FEATURE_RDSEED;

    // Detect AVX-512 features
    if (ebx & (1 << 16))
        x86_64_cpu_features |= X86_64_FEATURE_AVX512F;
    if (ebx & (1 << 17))
        x86_64_cpu_features |= X86_64_FEATURE_AVX512DQ;
    if (ebx & (1 << 18))
        x86_64_cpu_features |= X86_64_FEATURE_AVX512BW;
    if (ebx & (1 << 19))
        x86_64_cpu_features |= X86_64_FEATURE_AVX512VL;
    if (ebx & (1 << 20))
        x86_64_cpu_features |= X86_64_FEATURE_AVX512CD;
    if (ebx & (1 << 21))
        x86_64_cpu_features |= X86_64_FEATURE_AVX512ER;
    if (ebx & (1 << 22))
        x86_64_cpu_features |= X86_64_FEATURE_AVX512PF;

    // Detect security features
    if (ecx & (1 << 7))
        x86_64_cpu_features |= X86_64_FEATURE_CET_SS;
    if (ecx & (1 << 20))
        x86_64_cpu_features |= X86_64_FEATURE_CET_IBT;

    // Detect TSX features
    if (ebx & (1 << 4))
        x86_64_cpu_features |= X86_64_FEATURE_TSX;
    if (ebx & (1 << 11))
        x86_64_cpu_features |= X86_64_FEATURE_TSX_NI;

    printf("CPU Features detected: 0x%llX\n", x86_64_cpu_features);
}

bool x86_64_has_feature(uint64_t feature)
{
    return (x86_64_cpu_features & feature) != 0;
}

void x86_64_print_cpu_info(void)
{
    printf("=== x86_64 CPU Information ===\n");
    printf("Family: %u, Model: %u\n", x86_64_cpu_family, x86_64_cpu_model);
    printf("Features: 0x%llX\n", x86_64_cpu_features);

    // Print supported features
    if (x86_64_has_feature(X86_64_FEATURE_SSE))
        printf("  SSE\n");
    if (x86_64_has_feature(X86_64_FEATURE_AVX))
        printf("  AVX\n");
    if (x86_64_has_feature(X86_64_FEATURE_AVX2))
        printf("  AVX2\n");
    if (x86_64_has_feature(X86_64_FEATURE_AVX512F))
        printf("  AVX-512F\n");
    if (x86_64_has_feature(X86_64_FEATURE_SMEP))
        printf("  SMEP\n");
    if (x86_64_has_feature(X86_64_FEATURE_SMAP))
        printf("  SMAP\n");
    if (x86_64_has_feature(X86_64_FEATURE_TSX))
        printf("  TSX\n");
}

// ============================================================================
// PERFORMANCE MONITORING UNIT (PMU)
// ============================================================================

void x86_64_pmu_init(void)
{
    printf("Initializing x86_64 PMU...\n");

    // Check if PMU is supported
    uint32_t eax, ebx, ecx, edx;
    cpuid(0xA, &eax, &ebx, &ecx, &edx);

    if (eax == 0)
    {
        printf("PMU not supported\n");
        return;
    }

    uint32_t pmu_version = eax & 0xFF;
    uint32_t num_counters = (eax >> 8) & 0xFF;
    uint32_t counter_width = (eax >> 16) & 0xFF;

    printf("PMU Version: %u, Counters: %u, Width: %u bits\n",
           pmu_version, num_counters, counter_width);

    // Enable PMU for current CPU
    uint64_t cr4 = read_cr4();
    write_cr4(cr4 | (1ULL << 13)); // Set PCE bit

    // Initialize fixed counters
    uint64_t fixed_ctr_ctrl = 0;
    fixed_ctr_ctrl |= (1ULL << 0); // Enable fixed counter 0 (instructions)
    fixed_ctr_ctrl |= (1ULL << 4); // Enable fixed counter 1 (cycles)
    fixed_ctr_ctrl |= (1ULL << 8); // Enable fixed counter 2 (reference cycles)

    wrmsr(0x38D, fixed_ctr_ctrl); // IA32_FIXED_CTR_CTRL

    printf("PMU initialized successfully\n");
}

uint64_t x86_64_pmu_read_counter(uint32_t counter)
{
    if (counter < 8)
    {
        // General purpose counters
        uint64_t value;
        __asm__ volatile("rdpmc" : "=A"(value) : "c"(counter));
        return value;
    }
    else if (counter < 11)
    {
        // Fixed counters
        uint64_t value;
        __asm__ volatile("rdpmc" : "=A"(value) : "c"(0x40000000 | (counter - 8)));
        return value;
    }
    return 0;
}

void x86_64_pmu_set_event(uint32_t counter, uint32_t event)
{
    if (counter >= 8)
        return;

    // Set event select register
    uint64_t event_select = event & 0xFF;
    wrmsr(0x186 + counter, event_select);

    // Enable counter
    uint64_t global_ctrl = rdmsr(0x38F); // IA32_PERF_GLOBAL_CTRL
    global_ctrl |= (1ULL << counter);
    wrmsr(0x38F, global_ctrl);
}

// ============================================================================
// POWER MANAGEMENT
// ============================================================================

void x86_64_power_init(void)
{
    printf("Initializing x86_64 power management...\n");

    // Initialize power state
    power_state.current_state = X86_64_POWER_STATE_C0;
    power_state.current_frequency_mhz = X86_64_TSC_FREQ_DEFAULT / 1000000;
    power_state.turbo_boost_enabled = true;

    // Check for Intel SpeedStep or AMD PowerNow
    uint32_t eax, ebx, ecx, edx;
    cpuid(6, &eax, &ebx, &ecx, &edx);

    if (eax & (1 << 1))
    {
        printf("Intel SpeedStep supported\n");
    }

    printf("Power management initialized\n");
}

int x86_64_power_set_state(uint32_t state)
{
    if (state >= 7)
        return -1;

    switch (state)
    {
    case X86_64_POWER_STATE_C0:
        // Running state - no special action needed
        break;
    case X86_64_POWER_STATE_C1:
        // Halt - use HLT instruction
        __asm__ volatile("hlt");
        break;
    case X86_64_POWER_STATE_C1E:
        // Enhanced halt - similar to C1
        __asm__ volatile("hlt");
        break;
    case X86_64_POWER_STATE_C2:
        // Stop grant - use MWAIT
        __asm__ volatile("mwait");
        break;
    case X86_64_POWER_STATE_C3:
        // Deep sleep - use MWAIT with deeper sleep
        __asm__ volatile("mwait");
        break;
    default:
        return -1;
    }

    power_state.current_state = state;
    return 0;
}

uint32_t x86_64_power_get_state(void)
{
    return power_state.current_state;
}

// ============================================================================
// CACHE MANAGEMENT
// ============================================================================

void x86_64_cache_init(void)
{
    printf("Initializing x86_64 cache management...\n");

    // Detect cache sizes using CPUID
    uint32_t eax, ebx, ecx, edx;

    // L1 cache info
    cpuid(0x80000005, &eax, &ebx, &ecx, &edx);
    cache_info.l1d_size_kb = (ecx >> 24) & 0xFF;
    cache_info.l1i_size_kb = (edx >> 24) & 0xFF;

    // L2 and L3 cache info
    cpuid(0x80000006, &eax, &ebx, &ecx, &edx);
    cache_info.l2_size_kb = (ecx >> 16) & 0xFFFF;
    cache_info.l3_size_kb = (edx >> 18) & 0x3FFF;

    // Cache line size
    cache_info.line_size = (ecx >> 16) & 0xFF;

    printf("Cache sizes detected:\n");
    printf("  L1I: %u KB\n", cache_info.l1i_size_kb);
    printf("  L1D: %u KB\n", cache_info.l1d_size_kb);
    printf("  L2:  %u KB\n", cache_info.l2_size_kb);
    printf("  L3:  %u KB\n", cache_info.l3_size_kb);
    printf("  Line size: %u bytes\n", cache_info.line_size);
}

void x86_64_cache_invalidate_all(void)
{
    // Invalidate all caches using WBINVD
    __asm__ volatile("wbinvd" ::: "memory");
}

void x86_64_cache_clean_all(void)
{
    // Clean all caches using CLFLUSH for each cache line
    // This is a simplified implementation
    for (uint64_t addr = 0; addr < 64 * 1024; addr += 64)
    {
        __asm__ volatile("clflush (%0)" : : "r"(addr) : "memory");
    }
}

// ============================================================================
// NUMA MANAGEMENT
// ============================================================================

void x86_64_numa_init(void)
{
    printf("Initializing x86_64 NUMA topology...\n");

    // Detect NUMA topology using ACPI or CPUID
    // For now, use simplified detection
    numa_topology.node_count = 1;
    numa_topology.current_node = 0;
    numa_topology.socket_count = 1;
    numa_topology.cores_per_socket = 1;

    // Try to detect using CPUID
    uint32_t eax, ebx, ecx, edx;
    cpuid(0x80000008, &eax, &ebx, &ecx, &edx);

    uint32_t cores_per_socket = (ecx >> 12) & 0xF;
    if (cores_per_socket > 0)
    {
        numa_topology.cores_per_socket = cores_per_socket;
    }

    printf("NUMA topology: %u nodes, %u sockets, %u cores/socket\n",
           numa_topology.node_count,
           numa_topology.socket_count,
           numa_topology.cores_per_socket);
}

uint32_t x86_64_numa_get_node_count(void)
{
    return numa_topology.node_count;
}

uint32_t x86_64_numa_get_current_node(void)
{
    return numa_topology.current_node;
}

// ============================================================================
// ADVANCED SECURITY FEATURES
// ============================================================================

void x86_64_security_init(void)
{
    printf("Initializing x86_64 security features...\n");

    // Enable SMEP if supported
    if (x86_64_has_feature(X86_64_FEATURE_SMEP))
    {
        uint64_t cr4 = read_cr4();
        write_cr4(cr4 | CR4_SMEP);
        printf("SMEP enabled\n");
    }

    // Enable SMAP if supported
    if (x86_64_has_feature(X86_64_FEATURE_SMAP))
    {
        uint64_t cr4 = read_cr4();
        write_cr4(cr4 | CR4_SMAP);
        printf("SMAP enabled\n");
    }

    // Enable UMIP if supported
    if (x86_64_has_feature(X86_64_FEATURE_UMIP))
    {
        uint64_t cr4 = read_cr4();
        write_cr4(cr4 | CR4_UMIP);
        printf("UMIP enabled\n");
    }

    // Enable CET if supported
    if (x86_64_has_feature(X86_64_FEATURE_CET_SS))
    {
        uint64_t cr4 = read_cr4();
        write_cr4(cr4 | (1ULL << 23)); // Set CET bit
        printf("CET Shadow Stack enabled\n");
    }

    if (x86_64_has_feature(X86_64_FEATURE_CET_IBT))
    {
        uint64_t cr4 = read_cr4();
        write_cr4(cr4 | (1ULL << 20)); // Set IBT bit
        printf("CET Indirect Branch Tracking enabled\n");
    }

    printf("Security features initialized\n");
}

// ============================================================================
// VECTOR EXTENSIONS INITIALIZATION
// ============================================================================

void x86_64_vector_init(void)
{
    printf("Initializing x86_64 vector extensions...\n");

    // Enable SSE/SSE2
    if (x86_64_has_feature(X86_64_FEATURE_SSE))
    {
        uint64_t cr4 = read_cr4();
        write_cr4(cr4 | (1ULL << 9)); // Set OSFXSR bit
        printf("SSE enabled\n");
    }

    if (x86_64_has_feature(X86_64_FEATURE_SSE2))
    {
        printf("SSE2 enabled\n");
    }

    // Enable AVX
    if (x86_64_has_feature(X86_64_FEATURE_AVX))
    {
        uint64_t cr4 = read_cr4();
        write_cr4(cr4 | (1ULL << 18)); // Set OSXSAVE bit

        uint64_t xcr0 = rdmsr(0xC0000080); // XCR0
        xcr0 |= (1ULL << 1) | (1ULL << 2); // Enable XMM and YMM
        wrmsr(0xC0000080, xcr0);

        printf("AVX enabled\n");
    }

    // Enable AVX-512
    if (x86_64_has_feature(X86_64_FEATURE_AVX512F))
    {
        uint64_t xcr0 = rdmsr(0xC0000080);               // XCR0
        xcr0 |= (1ULL << 5) | (1ULL << 6) | (1ULL << 7); // Enable ZMM, Hi16_ZMM, Opmask
        wrmsr(0xC0000080, xcr0);

        printf("AVX-512 enabled\n");
    }

    printf("Vector extensions initialized\n");
}

// ============================================================================
// INITIALIZATION FUNCTION
// ============================================================================

void x86_64_advanced_init(void)
{
    printf("=== x86_64 Advanced Features Initialization ===\n");

    // Detect CPU features first
    x86_64_detect_cpu_features();

    // Initialize subsystems
    x86_64_pmu_init();
    x86_64_power_init();
    x86_64_cache_init();
    x86_64_numa_init();
    x86_64_security_init();
    x86_64_vector_init();

    // Print final configuration
    x86_64_print_cpu_info();

    printf("x86_64 advanced features initialization complete\n");
}
