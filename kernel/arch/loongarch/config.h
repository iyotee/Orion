/*
 * ORION OS - LoongArch Configuration
 *
 * This file contains configuration options for LoongArch architecture support.
 */

#ifndef ORION_LOONGARCH_CONFIG_H
#define ORION_LOONGARCH_CONFIG_H

/* ============================================================================
 * LoongArch Architecture Configuration
 * ============================================================================ */

/* CPU Configuration */
#define LOONGARCH_CPU_MAX_CORES 256
#define LOONGARCH_CPU_MAX_THREADS 512
#define LOONGARCH_CPU_MAX_FREQ_MHZ 5000
#define LOONGARCH_CPU_MIN_FREQ_MHZ 100

/* Memory Configuration */
#define LOONGARCH_MEMORY_MAX_SIZE 0x1000000000000000UL /* 16EB */
#define LOONGARCH_MEMORY_PAGE_SIZE 0x1000              /* 4KB */
#define LOONGARCH_MEMORY_HUGE_PAGE_SIZE 0x200000       /* 2MB */
#define LOONGARCH_MEMORY_GIGA_PAGE_SIZE 0x40000000     /* 1GB */
#define LOONGARCH_MEMORY_TLB_ENTRIES 1024
#define LOONGARCH_MEMORY_STLB_ENTRIES 256

/* Cache Configuration */
#define LOONGARCH_CACHE_L1I_SIZE 64 * 1024       /* 64KB */
#define LOONGARCH_CACHE_L1D_SIZE 64 * 1024       /* 64KB */
#define LOONGARCH_CACHE_L2_SIZE 256 * 1024       /* 256KB */
#define LOONGARCH_CACHE_L3_SIZE 16 * 1024 * 1024 /* 16MB */
#define LOONGARCH_CACHE_LINE_SIZE 64
#define LOONGARCH_CACHE_LINE_MASK 63

/* Interrupt Configuration */
#define LOONGARCH_IRQ_MAX_COUNT 256
#define LOONGARCH_IRQ_TIMER 2
#define LOONGARCH_IRQ_UART 1
#define LOONGARCH_IRQ_GPIO 3
#define LOONGARCH_IRQ_PCI 4
#define LOONGARCH_IRQ_EXTENDED 0x80

/* Timer Configuration */
#define LOONGARCH_TIMER_FREQ 1000000000 /* 1GHz */
#define LOONGARCH_TIMER_DIVIDER 1000    /* 1MHz */
#define LOONGARCH_TIMER_MAX_PERIOD 0xFFFFFFFFFFFFFFFFUL
#define LOONGARCH_TIMER_MIN_PERIOD 1000 /* 1μs */

/* Vector Extensions Configuration */
#define LOONGARCH_LSX_REG_COUNT 32
#define LOONGARCH_LSX_REG_SIZE 128 /* 16 bytes */
#define LOONGARCH_LASX_REG_COUNT 32
#define LOONGARCH_LASX_REG_SIZE 256 /* 32 bytes */

/* Crypto Configuration */
#define LOONGARCH_CRYPTO_AES_BLOCK_SIZE 16
#define LOONGARCH_CRYPTO_SHA_BLOCK_SIZE 64
#define LOONGARCH_CRYPTO_MAX_KEY_SIZE 256

/* Virtualization Configuration */
#define LOONGARCH_VIRT_MAX_VMS 64
#define LOONGARCH_VIRT_MAX_VCPUS 256
#define LOONGARCH_VIRT_MAX_MEMORY 0x1000000000000000UL /* 16EB */

/* NUMA Configuration */
#define LOONGARCH_NUMA_MAX_NODES 16
#define LOONGARCH_NUMA_MAX_DISTANCE 255
#define LOONGARCH_NUMA_DEFAULT_DISTANCE 10

/* Performance Monitoring Configuration */
#define LOONGARCH_PMU_MAX_COUNTERS 8
#define LOONGARCH_PMU_COUNTER_WIDTH 64
#define LOONGARCH_PMU_MAX_EVENTS 256

/* Binary Translation Configuration */
#define LOONGARCH_LBT_MAX_CODE_SIZE 0x1000000 /* 16MB */
#define LOONGARCH_LBT_MAX_TRANSLATIONS 1000
#define LOONGARCH_LBT_CACHE_SIZE 0x100000 /* 1MB */

/* Debug Configuration */
#define LOONGARCH_DEBUG_MAX_BREAKPOINTS 8
#define LOONGARCH_DEBUG_MAX_WATCHPOINTS 4
#define LOONGARCH_DEBUG_TRACE_BUFFER_SIZE 0x100000 /* 1MB */

/* Power Management Configuration */
#define LOONGARCH_POWER_MAX_STATES 8
#define LOONGARCH_POWER_MIN_FREQ_MHZ 100
#define LOONGARCH_POWER_MAX_FREQ_MHZ 5000

/* Security Configuration */
#define LOONGARCH_SECURITY_MAX_DOMAINS 16
#define LOONGARCH_SECURITY_MAX_CAPS 256
#define LOONGARCH_SECURITY_KEY_SIZE 256

/* ============================================================================
 * Feature Enable/Disable Configuration
 * ============================================================================ */

/* Enable/disable specific features */
#define LOONGARCH_FEATURE_LSX_ENABLED 1
#define LOONGARCH_FEATURE_LASX_ENABLED 1
#define LOONGARCH_FEATURE_CRYPTO_ENABLED 1
#define LOONGARCH_FEATURE_VIRT_ENABLED 1
#define LOONGARCH_FEATURE_NUMA_ENABLED 1
#define LOONGARCH_FEATURE_LBT_ENABLED 1
#define LOONGARCH_FEATURE_PMU_ENABLED 1
#define LOONGARCH_FEATURE_DEBUG_ENABLED 1
#define LOONGARCH_FEATURE_POWER_ENABLED 1
#define LOONGARCH_FEATURE_SECURITY_ENABLED 1

/* ============================================================================
 * Performance Configuration
 * ============================================================================ */

/* Performance targets */
#define LOONGARCH_PERF_SYSCALL_LATENCY_NS 300
#define LOONGARCH_PERF_CONTEXT_SWITCH_NS 500
#define LOONGARCH_PERF_IPC_THROUGHPUT_MSG_S 4000000
#define LOONGARCH_PERF_MEMORY_BANDWIDTH_PCT 85

/* Optimization levels */
#define LOONGARCH_OPT_LEVEL_DEBUG 0
#define LOONGARCH_OPT_LEVEL_RELEASE 1
#define LOONGARCH_OPT_LEVEL_PERFORMANCE 2
#define LOONGARCH_OPT_LEVEL_SIZE 3

/* ============================================================================
 * Debug Configuration
 * ============================================================================ */

/* Debug levels */
#define LOONGARCH_DEBUG_LEVEL_NONE 0
#define LOONGARCH_DEBUG_LEVEL_ERROR 1
#define LOONGARCH_DEBUG_LEVEL_WARNING 2
#define LOONGARCH_DEBUG_LEVEL_INFO 3
#define LOONGARCH_DEBUG_LEVEL_DEBUG 4
#define LOONGARCH_DEBUG_LEVEL_TRACE 5

/* Debug features */
#define LOONGARCH_DEBUG_PRINT_ENABLED 1
#define LOONGARCH_DEBUG_ASSERT_ENABLED 1
#define LOONGARCH_DEBUG_TRACE_ENABLED 0
#define LOONGARCH_DEBUG_PROFILE_ENABLED 0

/* ============================================================================
 * Compiler Configuration
 * ============================================================================ */

/* Compiler-specific settings */
#ifdef __GNUC__
#define LOONGARCH_GNUC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#define LOONGARCH_GNUC_4_8_OR_LATER (LOONGARCH_GNUC_VERSION >= 40800)
#define LOONGARCH_GNUC_5_0_OR_LATER (LOONGARCH_GNUC_VERSION >= 50000)
#define LOONGARCH_GNUC_6_0_OR_LATER (LOONGARCH_GNUC_VERSION >= 60000)
#define LOONGARCH_GNUC_7_0_OR_LATER (LOONGARCH_GNUC_VERSION >= 70000)
#define LOONGARCH_GNUC_8_0_OR_LATER (LOONGARCH_GNUC_VERSION >= 80000)
#define LOONGARCH_GNUC_9_0_OR_LATER (LOONGARCH_GNUC_VERSION >= 90000)
#define LOONGARCH_GNUC_10_0_OR_LATER (LOONGARCH_GNUC_VERSION >= 100000)
#define LOONGARCH_GNUC_11_0_OR_LATER (LOONGARCH_GNUC_VERSION >= 110000)
#define LOONGARCH_GNUC_12_0_OR_LATER (LOONGARCH_GNUC_VERSION >= 120000)
#define LOONGARCH_GNUC_13_0_OR_LATER (LOONGARCH_GNUC_VERSION >= 130000)
#define LOONGARCH_GNUC_14_0_OR_LATER (LOONGARCH_GNUC_VERSION >= 140000)
#define LOONGARCH_GNUC_15_0_OR_LATER (LOONGARCH_GNUC_VERSION >= 150000)
#else
#define LOONGARCH_GNUC_4_8_OR_LATER 0
#define LOONGARCH_GNUC_5_0_OR_LATER 0
#define LOONGARCH_GNUC_6_0_OR_LATER 0
#define LOONGARCH_GNUC_7_0_OR_LATER 0
#define LOONGARCH_GNUC_8_0_OR_LATER 0
#define LOONGARCH_GNUC_9_0_OR_LATER 0
#define LOONGARCH_GNUC_10_0_OR_LATER 0
#define LOONGARCH_GNUC_11_0_OR_LATER 0
#define LOONGARCH_GNUC_12_0_OR_LATER 0
#define LOONGARCH_GNUC_13_0_OR_LATER 0
#define LOONGARCH_GNUC_14_0_OR_LATER 0
#define LOONGARCH_GNUC_15_0_OR_LATER 0
#endif

/* ============================================================================
 * Platform Configuration
 * ============================================================================ */

/* Platform-specific settings */
#ifdef LOONGARCH_PLATFORM_3A5000
#define LOONGARCH_CPU_MODEL_3A5000 1
#define LOONGARCH_CPU_CORES 4
#define LOONGARCH_CPU_THREADS 4
#define LOONGARCH_CPU_MAX_FREQ_MHZ 2500
#define LOONGARCH_CPU_L1I_SIZE 64 * 1024
#define LOONGARCH_CPU_L1D_SIZE 64 * 1024
#define LOONGARCH_CPU_L2_SIZE 256 * 1024
#define LOONGARCH_CPU_L3_SIZE 16 * 1024 * 1024
#elif defined(LOONGARCH_PLATFORM_3C5000)
#define LOONGARCH_CPU_MODEL_3C5000 1
#define LOONGARCH_CPU_CORES 16
#define LOONGARCH_CPU_THREADS 16
#define LOONGARCH_CPU_MAX_FREQ_MHZ 2500
#define LOONGARCH_CPU_L1I_SIZE 64 * 1024
#define LOONGARCH_CPU_L1D_SIZE 64 * 1024
#define LOONGARCH_CPU_L2_SIZE 256 * 1024
#define LOONGARCH_CPU_L3_SIZE 64 * 1024 * 1024
#elif defined(LOONGARCH_PLATFORM_3A6000)
#define LOONGARCH_CPU_MODEL_3A6000 1
#define LOONGARCH_CPU_CORES 4
#define LOONGARCH_CPU_THREADS 4
#define LOONGARCH_CPU_MAX_FREQ_MHZ 3000
#define LOONGARCH_CPU_L1I_SIZE 64 * 1024
#define LOONGARCH_CPU_L1D_SIZE 64 * 1024
#define LOONGARCH_CPU_L2_SIZE 256 * 1024
#define LOONGARCH_CPU_L3_SIZE 16 * 1024 * 1024
#elif defined(LOONGARCH_PLATFORM_2K1000)
#define LOONGARCH_CPU_MODEL_2K1000 1
#define LOONGARCH_CPU_CORES 2
#define LOONGARCH_CPU_THREADS 2
#define LOONGARCH_CPU_MAX_FREQ_MHZ 1000
#define LOONGARCH_CPU_L1I_SIZE 32 * 1024
#define LOONGARCH_CPU_L1D_SIZE 32 * 1024
#define LOONGARCH_CPU_L2_SIZE 128 * 1024
#define LOONGARCH_CPU_L3_SIZE 0
#else
/* Generic LoongArch configuration */
#define LOONGARCH_CPU_MODEL_GENERIC 1
#define LOONGARCH_CPU_CORES 1
#define LOONGARCH_CPU_THREADS 1
#define LOONGARCH_CPU_MAX_FREQ_MHZ 2000
#define LOONGARCH_CPU_L1I_SIZE 64 * 1024
#define LOONGARCH_CPU_L1D_SIZE 64 * 1024
#define LOONGARCH_CPU_L2_SIZE 256 * 1024
#define LOONGARCH_CPU_L3_SIZE 16 * 1024 * 1024
#endif

/* ============================================================================
 * Build Configuration
 * ============================================================================ */

/* Build type */
#ifdef NDEBUG
#define LOONGARCH_BUILD_TYPE_RELEASE 1
#define LOONGARCH_BUILD_TYPE_DEBUG 0
#else
#define LOONGARCH_BUILD_TYPE_RELEASE 0
#define LOONGARCH_BUILD_TYPE_DEBUG 1
#endif

/* Optimization level */
#ifdef LOONGARCH_OPT_LEVEL
#define LOONGARCH_OPT_LEVEL_VALUE LOONGARCH_OPT_LEVEL
#else
#define LOONGARCH_OPT_LEVEL_VALUE LOONGARCH_OPT_LEVEL_RELEASE
#endif

/* Debug level */
#ifdef LOONGARCH_DEBUG_LEVEL
#define LOONGARCH_DEBUG_LEVEL_VALUE LOONGARCH_DEBUG_LEVEL
#else
#ifdef LOONGARCH_BUILD_TYPE_DEBUG
#define LOONGARCH_DEBUG_LEVEL_VALUE LOONGARCH_DEBUG_LEVEL_DEBUG
#else
#define LOONGARCH_DEBUG_LEVEL_VALUE LOONGARCH_DEBUG_LEVEL_ERROR
#endif
#endif

/* ============================================================================
 * Validation Configuration
 * ============================================================================ */

/* Configuration validation */
#if LOONGARCH_CPU_MAX_CORES > 256
#error "LOONGARCH_CPU_MAX_CORES cannot exceed 256"
#endif

#if LOONGARCH_MEMORY_MAX_SIZE > 0x1000000000000000UL
#error "LOONGARCH_MEMORY_MAX_SIZE cannot exceed 16EB"
#endif

#if LOONGARCH_IRQ_MAX_COUNT > 256
#error "LOONGARCH_IRQ_MAX_COUNT cannot exceed 256"
#endif

#if LOONGARCH_NUMA_MAX_NODES > 16
#error "LOONGARCH_NUMA_MAX_NODES cannot exceed 16"
#endif

#if LOONGARCH_VIRT_MAX_VMS > 64
#error "LOONGARCH_VIRT_MAX_VMS cannot exceed 64"
#endif

/* ============================================================================
 * Default Configuration Values
 * ============================================================================ */

/* Set default values for undefined configurations */
#ifndef LOONGARCH_CPU_CORES
#define LOONGARCH_CPU_CORES LOONGARCH_CPU_CORES
#endif

#ifndef LOONGARCH_CPU_THREADS
#define LOONGARCH_CPU_THREADS LOONGARCH_CPU_THREADS
#endif

#ifndef LOONGARCH_CPU_MAX_FREQ_MHZ
#define LOONGARCH_CPU_MAX_FREQ_MHZ LOONGARCH_CPU_MAX_FREQ_MHZ
#endif

#ifndef LOONGARCH_CPU_L1I_SIZE
#define LOONGARCH_CPU_L1I_SIZE LOONGARCH_CPU_L1I_SIZE
#endif

#ifndef LOONGARCH_CPU_L1D_SIZE
#define LOONGARCH_CPU_L1D_SIZE LOONGARCH_CPU_L1D_SIZE
#endif

#ifndef LOONGARCH_CPU_L2_SIZE
#define LOONGARCH_CPU_L2_SIZE LOONGARCH_CPU_L2_SIZE
#endif

#ifndef LOONGARCH_CPU_L3_SIZE
#define LOONGARCH_CPU_L3_SIZE LOONGARCH_CPU_L3_SIZE
#endif

#endif /* ORION_LOONGARCH_CONFIG_H */
