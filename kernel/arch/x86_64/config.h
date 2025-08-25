/*
 * Orion Operating System - x86_64 Configuration
 *
 * Advanced configuration options for x86_64 architecture
 * Intel/AMD specific optimizations and performance targets
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_X86_64_CONFIG_H
#define ORION_X86_64_CONFIG_H

#include "../hal/common/types.h"

// ============================================================================
// CPU FAMILY CONFIGURATION
// ============================================================================

// Intel CPU families
#define X86_64_INTEL_CORE_12TH_GEN 1 // Alder Lake (P/E cores)
#define X86_64_INTEL_CORE_13TH_GEN 2 // Raptor Lake
#define X86_64_INTEL_CORE_14TH_GEN 3 // Raptor Lake Refresh
#define X86_64_INTEL_CORE_15TH_GEN 4 // Arrow Lake
#define X86_64_INTEL_XEON_4TH_GEN 5  // Sapphire Rapids
#define X86_64_INTEL_XEON_5TH_GEN 6  // Emerald Rapids

// AMD CPU families
#define X86_64_AMD_RYZEN_5000 10 // Zen 3
#define X86_64_AMD_RYZEN_6000 11 // Zen 3+ (mobile)
#define X86_64_AMD_RYZEN_7000 12 // Zen 4
#define X86_64_AMD_RYZEN_8000 13 // Zen 5
#define X86_64_AMD_EPYC_7003 20  // Milan (Zen 3)
#define X86_64_AMD_EPYC_7004 21  // Genoa (Zen 4)
#define X86_64_AMD_EPYC_8004 22  // Bergamo (Zen 4c)
#define X86_64_AMD_EPYC_9004 23  // Genoa-X (Zen 4)

// ============================================================================
// FEATURE FLAGS
// ============================================================================

// Vector Extensions
#define X86_64_FEATURE_SSE BIT(0)
#define X86_64_FEATURE_SSE2 BIT(1)
#define X86_64_FEATURE_SSE3 BIT(2)
#define X86_64_FEATURE_SSSE3 BIT(3)
#define X86_64_FEATURE_SSE41 BIT(4)
#define X86_64_FEATURE_SSE42 BIT(5)
#define X86_64_FEATURE_AVX BIT(6)
#define X86_64_FEATURE_AVX2 BIT(7)
#define X86_64_FEATURE_AVX512F BIT(8)
#define X86_64_FEATURE_AVX512DQ BIT(9)
#define X86_64_FEATURE_AVX512BW BIT(10)
#define X86_64_FEATURE_AVX512VL BIT(11)
#define X86_64_FEATURE_AVX512CD BIT(12)
#define X86_64_FEATURE_AVX512ER BIT(13)
#define X86_64_FEATURE_AVX512PF BIT(14)
#define X86_64_FEATURE_AVX512_4FMAPS BIT(15)
#define X86_64_FEATURE_AVX512_4VNNIW BIT(16)
#define X86_64_FEATURE_AVX512_BITALG BIT(17)
#define X86_64_FEATURE_AVX512_IFMA BIT(18)
#define X86_64_FEATURE_AVX512_VBMI BIT(19)
#define X86_64_FEATURE_AVX512_VBMI2 BIT(20)
#define X86_64_FEATURE_AVX512_VNNI BIT(21)
#define X86_64_FEATURE_AVX512_VPOPCNTDQ BIT(22)
#define X86_64_FEATURE_AVX512_BF16 BIT(23)
#define X86_64_FEATURE_AVX512_FP16 BIT(24)

// Security Features
#define X86_64_FEATURE_SMEP BIT(30)
#define X86_64_FEATURE_SMAP BIT(31)
#define X86_64_FEATURE_UMIP BIT(32)
#define X86_64_FEATURE_CET_SS BIT(33)  // Shadow Stack
#define X86_64_FEATURE_CET_IBT BIT(34) // Indirect Branch Tracking
#define X86_64_FEATURE_MPX BIT(35)     // Memory Protection Extensions
#define X86_64_FEATURE_TME BIT(36)     // Total Memory Encryption
#define X86_64_FEATURE_TDX BIT(37)     // Trust Domain Extensions
#define X86_64_FEATURE_SGX BIT(38)     // Software Guard Extensions
#define X86_64_FEATURE_SEV BIT(39)     // AMD Secure Encrypted Virtualization
#define X86_64_FEATURE_SEV_ES BIT(40)  // SEV Encrypted State
#define X86_64_FEATURE_SEV_SNP BIT(41) // SEV Secure Nested Paging

// Advanced Features
#define X86_64_FEATURE_TSX BIT(50)    // Transactional Synchronization
#define X86_64_FEATURE_TSX_NI BIT(51) // TSX with Intel TSX-NI
#define X86_64_FEATURE_HLE BIT(52)    // Hardware Lock Elision
#define X86_64_FEATURE_RTM BIT(53)    // Restricted Transactional Memory
#define X86_64_FEATURE_RDRAND BIT(54)
#define X86_64_FEATURE_RDSEED BIT(55)
#define X86_64_FEATURE_ADX BIT(56)  // Multi-Precision Add-Carry
#define X86_64_FEATURE_BMI1 BIT(57) // Bit Manipulation Instructions
#define X86_64_FEATURE_BMI2 BIT(58)
#define X86_64_FEATURE_CLFLUSHOPT BIT(59)
#define X86_64_FEATURE_CLWB BIT(60)
#define X86_64_FEATURE_CLDEMOTE BIT(61)
#define X86_64_FEATURE_MOVDIRI BIT(62)
#define X86_64_FEATURE_MOVDIR64B BIT(63)

// ============================================================================
// PERFORMANCE TARGETS
// ============================================================================

// High-performance targets (Intel Core i9, AMD Ryzen 9, EPYC)
#define X86_64_PERF_TARGET_SYSCALL_NS 120    // < 120ns syscall latency
#define X86_64_PERF_TARGET_CTX_SWITCH_NS 200 // < 200ns context switch
#define X86_64_PERF_TARGET_IPC_MSGS_S 12     // > 12M IPC messages/sec
#define X86_64_PERF_TARGET_MEM_BW_PCT 90     // > 90% STREAM bandwidth

// Mid-range targets (Intel Core i5, AMD Ryzen 5)
#define X86_64_PERF_TARGET_SYSCALL_NS_MID 200    // < 200ns syscall latency
#define X86_64_PERF_TARGET_CTX_SWITCH_NS_MID 400 // < 400ns context switch
#define X86_64_PERF_TARGET_IPC_MSGS_S_MID 8      // > 8M IPC messages/sec
#define X86_64_PERF_TARGET_MEM_BW_PCT_MID 85     // > 85% STREAM bandwidth

// ============================================================================
// MEMORY CONFIGURATION
// ============================================================================

// Page sizes supported
#define X86_64_PAGE_SIZE_4K (4ULL * 1024)
#define X86_64_PAGE_SIZE_2M (2ULL * 1024 * 1024)
#define X86_64_PAGE_SIZE_1G (1ULL * 1024 * 1024 * 1024)

// Virtual address space layout
#define X86_64_KERNEL_BASE 0xFFFFFFFF80000000ULL
#define X86_64_KERNEL_SIZE (512ULL * 1024 * 1024 * 1024) // 512GB
#define X86_64_USER_BASE 0x0000000000000000ULL
#define X86_64_USER_SIZE (128ULL * 1024 * 1024 * 1024) // 128GB

// NUMA configuration
#define X86_64_MAX_NUMA_NODES 8
#define X86_64_MAX_SOCKETS_PER_NODE 2
#define X86_64_MAX_CORES_PER_SOCKET 64

// ============================================================================
// CACHE CONFIGURATION
// ============================================================================

// Cache line sizes
#define X86_64_CACHE_LINE_SIZE 64
#define X86_64_CACHE_LINE_SIZE_L2 64
#define X86_64_CACHE_LINE_SIZE_L3 64

// Cache sizes (typical values, will be auto-detected)
#define X86_64_L1I_CACHE_SIZE 32  // KB
#define X86_64_L1D_CACHE_SIZE 32  // KB
#define X86_64_L2_CACHE_SIZE 256  // KB
#define X86_64_L3_CACHE_SIZE 8192 // KB

// ============================================================================
// INTERRUPT CONFIGURATION
// ============================================================================

// APIC configuration
#define X86_64_APIC_BASE 0xFEE00000
#define X86_64_APIC_MAX_VECTORS 256
#define X86_64_APIC_TIMER_VECTOR 32
#define X86_64_APIC_ERROR_VECTOR 19

// MSI-X support
#define X86_64_MSIX_MAX_VECTORS 2048
#define X86_64_MSIX_TABLE_SIZE 64

// ============================================================================
// TIMER CONFIGURATION
// ============================================================================

// TSC configuration
#define X86_64_TSC_FREQ_DEFAULT 2400000000ULL // 2.4 GHz default
#define X86_64_TSC_INVARIANT true

// HPET configuration
#define X86_64_HPET_BASE 0xFED00000
#define X86_64_HPET_TIMER_COUNT 8

// ============================================================================
// POWER MANAGEMENT
// ============================================================================

// CPU frequency scaling
#define X86_64_MIN_FREQ_MHZ 800
#define X86_64_MAX_FREQ_MHZ 5500
#define X86_64_TURBO_BOOST true

// Power states
#define X86_64_POWER_STATE_C0 0  // Running
#define X86_64_POWER_STATE_C1 1  // Halt
#define X86_64_POWER_STATE_C1E 2 // Enhanced Halt
#define X86_64_POWER_STATE_C2 3  // Stop Grant
#define X86_64_POWER_STATE_C3 4  // Deep Sleep
#define X86_64_POWER_STATE_C6 5  // Deep Power Down
#define X86_64_POWER_STATE_C7 6  // Deeper Power Down

// ============================================================================
// DEBUG AND TRACING
// ============================================================================

// Debug registers
#define X86_64_DEBUG_REG_COUNT 8
#define X86_64_DEBUG_REG_DR0 0
#define X86_64_DEBUG_REG_DR1 1
#define X86_64_DEBUG_REG_DR2 2
#define X86_64_DEBUG_REG_DR3 3
#define X86_64_DEBUG_REG_DR6 6
#define X86_64_DEBUG_REG_DR7 7

// Performance monitoring
#define X86_64_PMU_COUNTER_COUNT 8
#define X86_64_PMU_FIXED_COUNTER_COUNT 3

// ============================================================================
// COMPILER CONFIGURATION
// ============================================================================

#ifdef __GNUC__
#define X86_64_ALIGN_CACHE_LINE __attribute__((aligned(64)))
#define X86_64_NO_INLINE __attribute__((noinline))
#define X86_64_ALWAYS_INLINE __attribute__((always_inline))
#define X86_64_PACKED __attribute__((packed))
#elif defined(_MSC_VER)
#define X86_64_ALIGN_CACHE_LINE __declspec(align(64))
#define X86_64_NO_INLINE __declspec(noinline)
#define X86_64_ALWAYS_INLINE __forceinline
#define X86_64_PACKED
#else
#define X86_64_ALIGN_CACHE_LINE
#define X86_64_NO_INLINE
#define X86_64_ALWAYS_INLINE
#define X86_64_PACKED
#endif

// ============================================================================
// OPTIMIZATION LEVELS
// ============================================================================

// Compiler optimization levels
#define X86_64_OPT_LEVEL_SIZE 0       // Size optimization
#define X86_64_OPT_LEVEL_SPEED 1      // Speed optimization
#define X86_64_OPT_LEVEL_AGGRESSIVE 2 // Aggressive optimization
#define X86_64_OPT_LEVEL_SAFETY 3     // Safety-focused optimization

// Current optimization level
#define X86_64_CURRENT_OPT_LEVEL X86_64_OPT_LEVEL_SPEED

// ============================================================================
// DEBUG LEVELS
// ============================================================================

// Debug verbosity levels
#define X86_64_DEBUG_LEVEL_NONE 0
#define X86_64_DEBUG_LEVEL_ERROR 1
#define X86_64_DEBUG_LEVEL_WARN 2
#define X86_64_DEBUG_LEVEL_INFO 3
#define X86_64_DEBUG_LEVEL_DEBUG 4
#define X86_64_DEBUG_LEVEL_TRACE 5

// Current debug level
#define X86_64_CURRENT_DEBUG_LEVEL X86_64_DEBUG_LEVEL_INFO

// ============================================================================
// PLATFORM-SPECIFIC CONFIGURATIONS
// ============================================================================

// Intel specific configurations
#ifdef X86_64_INTEL_SPECIFIC
#define X86_64_INTEL_THREAD_DIRECTOR true
#define X86_64_INTEL_TURBO_BOOST_MAX true
#define X86_64_INTEL_THERMAL_MONITOR true
#define X86_64_INTEL_POWER_MANAGEMENT true
#endif

// AMD specific configurations
#ifdef X86_64_AMD_SPECIFIC
#define X86_64_AMD_PRECISION_BOOST true
#define X86_64_AMD_EXTENDED_FREQ_RANGE true
#define X86_64_AMD_THERMAL_MANAGEMENT true
#define X86_64_AMD_POWER_MANAGEMENT true
#endif

// ============================================================================
// FEATURE DETECTION MACROS
// ============================================================================

// Check if feature is available
#define X86_64_HAS_FEATURE(feature) ((x86_64_cpu_features & (feature)) != 0)

// Check CPU family
#define X86_64_IS_INTEL(family) (x86_64_cpu_family == (family))
#define X86_64_IS_AMD(family) (x86_64_cpu_family == (family))

// ============================================================================
// EXTERNAL VARIABLES
// ============================================================================

// CPU features bitmap (defined in cpu.c)
extern uint64_t x86_64_cpu_features;

// CPU family identifier (defined in cpu.c)
extern uint32_t x86_64_cpu_family;

// CPU model identifier (defined in cpu.c)
extern uint32_t x86_64_cpu_model;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

// CPU feature detection
void x86_64_detect_cpu_features(void);
bool x86_64_has_feature(uint64_t feature);
void x86_64_print_cpu_info(void);

// Performance monitoring
void x86_64_pmu_init(void);
uint64_t x86_64_pmu_read_counter(uint32_t counter);
void x86_64_pmu_set_event(uint32_t counter, uint32_t event);

// Power management
void x86_64_power_init(void);
int x86_64_power_set_state(uint32_t state);
uint32_t x86_64_power_get_state(void);

// Cache management
void x86_64_cache_init(void);
void x86_64_cache_invalidate_all(void);
void x86_64_cache_clean_all(void);

// NUMA management
void x86_64_numa_init(void);
uint32_t x86_64_numa_get_node_count(void);
uint32_t x86_64_numa_get_current_node(void);

#endif // ORION_X86_64_CONFIG_H
