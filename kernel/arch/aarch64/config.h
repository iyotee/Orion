/*
 * Orion Operating System - aarch64 Configuration
 *
 * Advanced configuration options for aarch64 architecture
 * ARM Cortex-A, Apple Silicon, and other ARM64 specific optimizations
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_AARCH64_CONFIG_H
#define ORION_AARCH64_CONFIG_H

#include "../hal/common/types.h"

// ============================================================================
// CPU FAMILY CONFIGURATION
// ============================================================================

// ARM Cortex-A families
#define AARCH64_CORTEX_A53 1   // ARMv8-A, 64-bit
#define AARCH64_CORTEX_A55 2   // ARMv8.2-A, DynamIQ
#define AARCH64_CORTEX_A57 3   // ARMv8-A, big.LITTLE
#define AARCH64_CORTEX_A72 4   // ARMv8-A, big.LITTLE
#define AARCH64_CORTEX_A73 5   // ARMv8-A, big.LITTLE
#define AARCH64_CORTEX_A75 6   // ARMv8.2-A, DynamIQ
#define AARCH64_CORTEX_A76 7   // ARMv8.2-A, DynamIQ
#define AARCH64_CORTEX_A77 8   // ARMv8.2-A, DynamIQ
#define AARCH64_CORTEX_A78 9   // ARMv8.2-A, DynamIQ
#define AARCH64_CORTEX_A78C 10 // ARMv8.2-A, DynamIQ (custom)
#define AARCH64_CORTEX_A510 11 // ARMv9-A, little core
#define AARCH64_CORTEX_A710 12 // ARMv9-A, big core
#define AARCH64_CORTEX_A715 13 // ARMv9-A, big core
#define AARCH64_CORTEX_A720 14 // ARMv9-A, big core
#define AARCH64_CORTEX_X1 15   // ARMv8.2-A, performance
#define AARCH64_CORTEX_X2 16   // ARMv9-A, performance
#define AARCH64_CORTEX_X3 17   // ARMv9-A, performance

// Apple Silicon families
#define AARCH64_APPLE_M1 20       // Apple M1 (Firestorm/Icestorm)
#define AARCH64_APPLE_M1_PRO 21   // Apple M1 Pro
#define AARCH64_APPLE_M1_MAX 22   // Apple M1 Max
#define AARCH64_APPLE_M1_ULTRA 23 // Apple M1 Ultra
#define AARCH64_APPLE_M2 24       // Apple M2 (Avalanche/Blizzard)
#define AARCH64_APPLE_M2_PRO 25   // Apple M2 Pro
#define AARCH64_APPLE_M2_MAX 26   // Apple M2 Max
#define AARCH64_APPLE_M2_ULTRA 27 // Apple M2 Ultra
#define AARCH64_APPLE_M3 28       // Apple M3 (Efficiency/Performance)
#define AARCH64_APPLE_M3_PRO 29   // Apple M3 Pro
#define AARCH64_APPLE_M3_MAX 30   // Apple M3 Max
#define AARCH64_APPLE_M3_ULTRA 31 // Apple M3 Ultra

// Qualcomm Snapdragon families
#define AARCH64_QUALCOMM_8CX 40 // Snapdragon 8cx Gen 1/2/3
#define AARCH64_QUALCOMM_7C 41  // Snapdragon 7c Gen 1/2/3
#define AARCH64_QUALCOMM_8C 42  // Snapdragon 8c

// NVIDIA Tegra families
#define AARCH64_NVIDIA_TEGRA_X1 50 // Tegra X1
#define AARCH64_NVIDIA_TEGRA_X2 51 // Tegra X2
#define AARCH64_NVIDIA_ORIN 52     // Tegra Orin

// ============================================================================
// FEATURE FLAGS
// ============================================================================

// ARMv8-A features
#define AARCH64_FEATURE_FP BIT(0)            // Floating point
#define AARCH64_FEATURE_ASIMD BIT(1)         // Advanced SIMD (NEON)
#define AARCH64_FEATURE_EVTSTRM BIT(2)       // Event stream
#define AARCH64_FEATURE_AES BIT(3)           // AES instructions
#define AARCH64_FEATURE_PMULL BIT(4)         // Polynomial multiply
#define AARCH64_FEATURE_SHA1 BIT(5)          // SHA1 instructions
#define AARCH64_FEATURE_SHA2 BIT(6)          // SHA2 instructions
#define AARCH64_FEATURE_CRC32 BIT(7)         // CRC32 instructions
#define AARCH64_FEATURE_ATOMICS BIT(8)       // Large System Extensions (LSE)
#define AARCH64_FEATURE_FPHP BIT(9)          // Half-precision FP
#define AARCH64_FEATURE_ASIMDHP BIT(10)      // Half-precision SIMD
#define AARCH64_FEATURE_CPUID BIT(11)        // CPU identification
#define AARCH64_FEATURE_ASIMDRDM BIT(12)     // Rounding double multiply
#define AARCH64_FEATURE_JSCVT BIT(13)        // JavaScript conversion
#define AARCH64_FEATURE_FCMA BIT(14)         // Floating point complex
#define AARCH64_FEATURE_LRCPC BIT(15)        // Load-acquire RCpc
#define AARCH64_FEATURE_DCPOP BIT(16)        // Data cache clean
#define AARCH64_FEATURE_SHA3 BIT(17)         // SHA3 instructions
#define AARCH64_FEATURE_SM3 BIT(18)          // SM3 instructions
#define AARCH64_FEATURE_SM4 BIT(19)          // SM4 instructions
#define AARCH64_FEATURE_ASIMDDP BIT(20)      // Dot product
#define AARCH64_FEATURE_SHA512 BIT(21)       // SHA512 instructions
#define AARCH64_FEATURE_SVE BIT(22)          // Scalable Vector Extension
#define AARCH64_FEATURE_SVE2 BIT(23)         // SVE2
#define AARCH64_FEATURE_SVE2_AES BIT(24)     // SVE2 AES
#define AARCH64_FEATURE_SVE2_SM4 BIT(25)     // SVE2 SM4
#define AARCH64_FEATURE_SVE2_SHA3 BIT(26)    // SVE2 SHA3
#define AARCH64_FEATURE_SVE2_BITPERM BIT(27) // SVE2 bit permute

// ARMv8.1 features
#define AARCH64_FEATURE_PAN BIT(30)    // Privileged Access Never
#define AARCH64_FEATURE_LOR BIT(31)    // Limited Ordering Regions
#define AARCH64_FEATURE_VHE BIT(32)    // Virtualization Host Extensions
#define AARCH64_FEATURE_HPDS BIT(33)   // Hierarchical Permission Disable
#define AARCH64_FEATURE_VMID16 BIT(34) // 16-bit VMID
#define AARCH64_FEATURE_LSE BIT(35)    // Large System Extensions
#define AARCH64_FEATURE_PAN2 BIT(36)   // PAN2
#define AARCH64_FEATURE_UAO BIT(37)    // User Access Override
#define AARCH64_FEATURE_DPB BIT(38)    // DC CVAP
#define AARCH64_FEATURE_DPB2 BIT(39)   // DC CVADP
#define AARCH64_FEATURE_DCPODP BIT(40) // DC CVADP
#define AARCH64_FEATURE_SB BIT(41)     // Speculation Barrier
#define AARCH64_FEATURE_IBT BIT(42)    // Indirect Branch Tracking
#define AARCH64_FEATURE_SSBS BIT(43)   // Speculative Store Bypass Safe

// ARMv8.2 features
#define AARCH64_FEATURE_CSV2 BIT(50)         // Cache Speculation Variant 2
#define AARCH64_FEATURE_CSV3 BIT(51)         // Cache Speculation Variant 3
#define AARCH64_FEATURE_DCPODP BIT(52)       // DC CVADP
#define AARCH64_FEATURE_UAO BIT(53)          // User Access Override
#define AARCH64_FEATURE_ALG BIT(54)          // Alternative Generator
#define AARCH64_FEATURE_RNDR BIT(55)         // Random number
#define AARCH64_FEATURE_RNDRRS BIT(56)       // Random number with seed
#define AARCH64_FEATURE_SVE BIT(57)          // Scalable Vector Extension
#define AARCH64_FEATURE_SVE2 BIT(58)         // SVE2
#define AARCH64_FEATURE_SVE2_AES BIT(59)     // SVE2 AES
#define AARCH64_FEATURE_SVE2_SM4 BIT(60)     // SVE2 SM4
#define AARCH64_FEATURE_SVE2_SHA3 BIT(61)    // SVE2 SHA3
#define AARCH64_FEATURE_SVE2_BITPERM BIT(62) // SVE2 bit permute

// ARMv8.3 features
#define AARCH64_FEATURE_PAUTH BIT(70) // Pointer Authentication
#define AARCH64_FEATURE_DPB2 BIT(71)  // DC CVADP
#define AARCH64_FEATURE_ECV BIT(72)   // Enhanced Counter Virtualization
#define AARCH64_FEATURE_AFP BIT(73)   // Alternate Floating Point
#define AARCH64_FEATURE_RPRES BIT(74) // Reduced Precision Reciprocal
#define AARCH64_FEATURE_MTE BIT(75)   // Memory Tagging Extension
#define AARCH64_FEATURE_MTE2 BIT(76)  // MTE2
#define AARCH64_FEATURE_MTE3 BIT(77)  // MTE3
#define AARCH64_FEATURE_MTE4 BIT(78)  // MTE4

// ARMv8.4 features
#define AARCH64_FEATURE_DIT BIT(80)   // Data Independent Timing
#define AARCH64_FEATURE_TLBI BIT(81)  // TLB Invalidate
#define AARCH64_FEATURE_FLAGM BIT(82) // Flag Manipulation
#define AARCH64_FEATURE_IDST BIT(83)  // ID space trapping
#define AARCH64_FEATURE_RNG BIT(84)   // Random number
#define AARCH64_FEATURE_BTI BIT(85)   // Branch Target Identification
#define AARCH64_FEATURE_SSBS BIT(86)  // Speculative Store Bypass Safe

// ARMv8.5 features
#define AARCH64_FEATURE_MTE BIT(90)  // Memory Tagging Extension
#define AARCH64_FEATURE_BTI BIT(91)  // Branch Target Identification
#define AARCH64_FEATURE_SSBS BIT(92) // Speculative Store Bypass Safe
#define AARCH64_FEATURE_CSV2 BIT(93) // Cache Speculation Variant 2
#define AARCH64_FEATURE_CSV3 BIT(94) // Cache Speculation Variant 3

// ARMv9 features
#define AARCH64_FEATURE_SVE2 BIT(100)         // SVE2
#define AARCH64_FEATURE_SVE2_AES BIT(101)     // SVE2 AES
#define AARCH64_FEATURE_SVE2_SM4 BIT(102)     // SVE2 SM4
#define AARCH64_FEATURE_SVE2_SHA3 BIT(103)    // SVE2 SHA3
#define AARCH64_FEATURE_SVE2_BITPERM BIT(104) // SVE2 bit permute
#define AARCH64_FEATURE_SVE2_I8MM BIT(105)    // SVE2 Int8 matrix multiply
#define AARCH64_FEATURE_SVE2_F32MM BIT(106)   // SVE2 FP32 matrix multiply
#define AARCH64_FEATURE_SVE2_F64MM BIT(107)   // SVE2 FP64 matrix multiply

// Apple Silicon specific features
#define AARCH64_FEATURE_APPLE_AMX BIT(110)            // Apple Matrix extensions
#define AARCH64_FEATURE_APPLE_NEURAL BIT(111)         // Neural Engine
#define AARCH64_FEATURE_APPLE_UNIFIED_MEMORY BIT(112) // Unified memory
#define AARCH64_FEATURE_APPLE_SECURE_ENCLAVE BIT(113) // Secure Enclave

// ============================================================================
// PERFORMANCE TARGETS
// ============================================================================

// High-performance targets (Apple M1-M3, ARM Cortex-X, high-end Cortex-A)
#define AARCH64_PERF_TARGET_SYSCALL_NS 100    // < 100ns syscall latency
#define AARCH64_PERF_TARGET_CTX_SWITCH_NS 150 // < 150ns context switch
#define AARCH64_PERF_TARGET_IPC_MSGS_S 15     // > 15M IPC messages/sec
#define AARCH64_PERF_TARGET_MEM_BW_PCT 95     // > 95% STREAM bandwidth

// Mid-range targets (ARM Cortex-A76+, mid-range ARM)
#define AARCH64_PERF_TARGET_SYSCALL_NS_MID 200    // < 200ns syscall latency
#define AARCH64_PERF_TARGET_CTX_SWITCH_NS_MID 300 // < 300ns context switch
#define AARCH64_PERF_TARGET_IPC_MSGS_S_MID 8      // > 8M IPC messages/sec
#define AARCH64_PERF_TARGET_MEM_BW_PCT_MID 85     // > 85% STREAM bandwidth

// Embedded targets (ARM Cortex-A53, Raspberry Pi)
#define AARCH64_PERF_TARGET_SYSCALL_NS_EMB 400    // < 400ns syscall latency
#define AARCH64_PERF_TARGET_CTX_SWITCH_NS_EMB 600 // < 600ns context switch
#define AARCH64_PERF_TARGET_IPC_MSGS_S_EMB 4      // > 4M IPC messages/sec
#define AARCH64_PERF_TARGET_MEM_BW_PCT_EMB 75     // > 75% STREAM bandwidth

// ============================================================================
// MEMORY CONFIGURATION
// ============================================================================

// Page sizes supported
#define AARCH64_PAGE_SIZE_4K (4ULL * 1024)
#define AARCH64_PAGE_SIZE_16K (16ULL * 1024)
#define AARCH64_PAGE_SIZE_64K (64ULL * 1024)
#define AARCH64_PAGE_SIZE_2M (2ULL * 1024 * 1024)
#define AARCH64_PAGE_SIZE_32M (32ULL * 1024 * 1024)
#define AARCH64_PAGE_SIZE_512M (512ULL * 1024 * 1024)
#define AARCH64_PAGE_SIZE_1G (1ULL * 1024 * 1024 * 1024)

// Virtual address space layout
#define AARCH64_KERNEL_BASE 0xFFFFFF8000000000ULL
#define AARCH64_KERNEL_SIZE (256ULL * 1024 * 1024 * 1024) // 256GB
#define AARCH64_USER_BASE 0x0000000000000000ULL
#define AARCH64_USER_SIZE (128ULL * 1024 * 1024 * 1024) // 128GB

// NUMA configuration
#define AARCH64_MAX_NUMA_NODES 8
#define AARCH64_MAX_CLUSTERS 4
#define AARCH64_MAX_CORES_PER_CLUSTER 8

// ============================================================================
// CACHE CONFIGURATION
// ============================================================================

// Cache line sizes
#define AARCH64_CACHE_LINE_SIZE 64
#define AARCH64_CACHE_LINE_SIZE_L2 64
#define AARCH64_CACHE_LINE_SIZE_L3 64

// Cache sizes (typical values, will be auto-detected)
#define AARCH64_L1I_CACHE_SIZE 64  // KB
#define AARCH64_L1D_CACHE_SIZE 64  // KB
#define AARCH64_L2_CACHE_SIZE 512  // KB
#define AARCH64_L3_CACHE_SIZE 8192 // KB

// ============================================================================
// INTERRUPT CONFIGURATION
// ============================================================================

// GIC configuration
#define AARCH64_GIC_BASE 0x08000000
#define AARCH64_GIC_MAX_IRQS 1024
#define AARCH64_GIC_MAX_CPUS 256

// GIC version support
#define AARCH64_GIC_VERSION_2 2
#define AARCH64_GIC_VERSION_3 3
#define AARCH64_GIC_VERSION_4 4

// ============================================================================
// TIMER CONFIGURATION
// ============================================================================

// Generic timer configuration
#define AARCH64_TIMER_FREQ_DEFAULT 24000000ULL // 24 MHz default
#define AARCH64_TIMER_CNTFRQ_EL0 true

// ============================================================================
// POWER MANAGEMENT
// ============================================================================

// CPU frequency scaling
#define AARCH64_MIN_FREQ_MHZ 400
#define AARCH64_MAX_FREQ_MHZ 3200
#define AARCH64_TURBO_BOOST true

// Power states
#define AARCH64_POWER_STATE_C0 0 // Running
#define AARCH64_POWER_STATE_C1 1 // Halt
#define AARCH64_POWER_STATE_C2 2 // Stop Grant
#define AARCH64_POWER_STATE_C3 3 // Deep Sleep
#define AARCH64_POWER_STATE_C4 4 // Deeper Sleep
#define AARCH64_POWER_STATE_C5 5 // Deepest Sleep
#define AARCH64_POWER_STATE_C6 6 // Power Down

// ============================================================================
// DEBUG AND TRACING
// ============================================================================

// Debug registers
#define AARCH64_DEBUG_REG_COUNT 16
#define AARCH64_DEBUG_REG_DBGDR0 0
#define AARCH64_DEBUG_REG_DBGDR1 1
#define AARCH64_DEBUG_REG_DBGDR2 2
#define AARCH64_DEBUG_REG_DBGDR3 3

// Performance monitoring
#define AARCH64_PMU_COUNTER_COUNT 32
#define AARCH64_PMU_CYCLE_COUNTER 31

// ============================================================================
// COMPILER CONFIGURATION
// ============================================================================

#ifdef __GNUC__
#define AARCH64_ALIGN_CACHE_LINE __attribute__((aligned(64)))
#define AARCH64_NO_INLINE __attribute__((noinline))
#define AARCH64_ALWAYS_INLINE __attribute__((always_inline))
#define AARCH64_PACKED __attribute__((packed))
#elif defined(_MSC_VER)
#define AARCH64_ALIGN_CACHE_LINE __declspec(align(64))
#define AARCH64_NO_INLINE __declspec(noinline)
#define AARCH64_ALWAYS_INLINE __forceinline
#define AARCH64_PACKED
#else
#define AARCH64_ALIGN_CACHE_LINE
#define AARCH64_NO_INLINE
#define AARCH64_ALWAYS_INLINE
#define AARCH64_PACKED
#endif

// ============================================================================
// OPTIMIZATION LEVELS
// ============================================================================

// Compiler optimization levels
#define AARCH64_OPT_LEVEL_SIZE 0       // Size optimization
#define AARCH64_OPT_LEVEL_SPEED 1      // Speed optimization
#define AARCH64_OPT_LEVEL_AGGRESSIVE 2 // Aggressive optimization
#define AARCH64_OPT_LEVEL_SAFETY 3     // Safety-focused optimization

// Current optimization level
#define AARCH64_CURRENT_OPT_LEVEL AARCH64_OPT_LEVEL_SPEED

// ============================================================================
// DEBUG LEVELS
// ============================================================================

// Debug verbosity levels
#define AARCH64_DEBUG_LEVEL_NONE 0
#define AARCH64_DEBUG_LEVEL_ERROR 1
#define AARCH64_DEBUG_LEVEL_WARN 2
#define AARCH64_DEBUG_LEVEL_INFO 3
#define AARCH64_DEBUG_LEVEL_DEBUG 4
#define AARCH64_DEBUG_LEVEL_TRACE 5

// Current debug level
#define AARCH64_CURRENT_DEBUG_LEVEL AARCH64_DEBUG_LEVEL_INFO

// ============================================================================
// PLATFORM-SPECIFIC CONFIGURATIONS
// ============================================================================

// Apple Silicon specific configurations
#ifdef AARCH64_APPLE_SPECIFIC
#define AARCH64_APPLE_UNIFIED_MEMORY true
#define AARCH64_APPLE_NEURAL_ENGINE true
#define AARCH64_APPLE_SECURE_ENCLAVE true
#define AARCH64_APPLE_METAL_API true
#endif

// ARM Cortex specific configurations
#ifdef AARCH64_CORTEX_SPECIFIC
#define AARCH64_CORTEX_BIG_LITTLE true
#define AARCH64_CORTEX_DYNAMIQ true
#define AARCH64_CORTEX_ARM_TRUSTZONE true
#endif

// ============================================================================
// FEATURE DETECTION MACROS
// ============================================================================

// Check if feature is available
#define AARCH64_HAS_FEATURE(feature) ((aarch64_cpu_features & (feature)) != 0)

// Check CPU family
#define AARCH64_IS_CORTEX(family) (aarch64_cpu_family >= 1 && aarch64_cpu_family <= 17)
#define AARCH64_IS_APPLE(family) (aarch64_cpu_family >= 20 && aarch64_cpu_family <= 31)
#define AARCH64_IS_QUALCOMM(family) (aarch64_cpu_family >= 40 && aarch64_cpu_family <= 42)
#define AARCH64_IS_NVIDIA(family) (aarch64_cpu_family >= 50 && aarch64_cpu_family <= 52)

// ============================================================================
// EXTERNAL VARIABLES
// ============================================================================

// CPU features bitmap (defined in arch.c)
extern uint64_t aarch64_cpu_features;

// CPU family identifier (defined in arch.c)
extern uint32_t aarch64_cpu_family;

// CPU model identifier (defined in arch.c)
extern uint32_t aarch64_cpu_model;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

// CPU feature detection
void aarch64_detect_cpu_features(void);
bool aarch64_has_feature(uint64_t feature);
void aarch64_print_cpu_info(void);

// Performance monitoring
void aarch64_pmu_init(void);
uint64_t aarch64_pmu_read_counter(uint32_t counter);
void aarch64_pmu_set_event(uint32_t counter, uint32_t event);

// Power management
void aarch64_power_init(void);
int aarch64_power_set_state(uint32_t state);
uint32_t aarch64_power_get_state(void);

// Cache management
void aarch64_cache_init(void);
void aarch64_cache_invalidate_all(void);
void aarch64_cache_clean_all(void);

// NUMA management
void aarch64_numa_init(void);
uint32_t aarch64_numa_get_node_count(void);
uint32_t aarch64_numa_get_current_node(void);

// Vector extensions
void aarch64_sve_init(void);
void aarch64_neon_init(void);

// Security features
void aarch64_security_init(void);
void aarch64_pauth_init(void);
void aarch64_mte_init(void);

#endif // ORION_AARCH64_CONFIG_H
