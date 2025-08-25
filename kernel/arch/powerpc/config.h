/*
 * ORION OS - POWER Architecture Configuration
 *
 * Configuration file for POWER architecture support (POWER8, POWER9, POWER10)
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_POWER_CONFIG_H
#define ORION_POWER_CONFIG_H

// ============================================================================
// ARCHITECTURE IDENTIFICATION
// ============================================================================

#define POWER_ARCH_ID 0x5000
#define POWER_ARCH_NAME "POWER"
#define POWER_ARCH_VERSION 0x0100

// ============================================================================
// CPU CONFIGURATION
// ============================================================================

/* CPU Features */
#define POWER_FEATURE_POWER8 1  /* POWER8 support */
#define POWER_FEATURE_POWER9 1  /* POWER9 support */
#define POWER_FEATURE_POWER10 1 /* POWER10 support */
#define POWER_FEATURE_VSX 1     /* Vector Scalar eXtension */
#define POWER_FEATURE_ALTIVEC 1 /* AltiVec/VMX support */
#define POWER_FEATURE_CRYPTO 1  /* Cryptographic acceleration */
#define POWER_FEATURE_HTM 1     /* Hardware Transactional Memory */
#define POWER_FEATURE_DFP 1     /* Decimal Floating Point */
#define POWER_FEATURE_ISA3 1    /* Power ISA v3.0 */

/* CPU Configuration */
#define POWER_CPU_MAX_FREQ_MHZ 4000
#define POWER_CPU_CACHE_LINE_SIZE 128
#define POWER_CPU_MAX_CORES 256
#define POWER_CPU_MAX_THREADS 1024

// ============================================================================
// MEMORY MANAGEMENT
// ============================================================================

/* Page Sizes */
#define POWER_PAGE_SHIFT 12
#define POWER_PAGE_SIZE (1UL << POWER_PAGE_SHIFT)
#define POWER_PAGE_MASK (~(POWER_PAGE_SIZE - 1))

#define POWER_HUGE_PAGE_SHIFT 24
#define POWER_HUGE_PAGE_SIZE (1UL << POWER_HUGE_PAGE_SHIFT)
#define POWER_HUGE_PAGE_MASK (~(POWER_HUGE_PAGE_SIZE - 1))

#define POWER_GIGA_PAGE_SHIFT 30
#define POWER_GIGA_PAGE_SIZE (1UL << POWER_GIGA_PAGE_SHIFT)
#define POWER_GIGA_PAGE_MASK (~(POWER_GIGA_PAGE_SIZE - 1))

/* Virtual Address Space */
#define POWER_KERNEL_BASE 0xC000000000000000UL
#define POWER_KERNEL_SIZE 0x400000000UL /* 16GB */
#define POWER_USER_BASE 0x0000000000000000UL
#define POWER_USER_SIZE 0x8000000000000000UL /* 8EB */

/* TLB Configuration */
#define POWER_TLB_ENTRIES 1024
#define POWER_TLB_WAYS 8

// ============================================================================
// INTERRUPT SYSTEM
// ============================================================================

/* Interrupt Numbers */
#define POWER_INT_DECREMENTER 0x900
#define POWER_INT_EXTERNAL 0x500
#define POWER_INT_PERFORMANCE 0xF00
#define POWER_INT_DECREMENTER_HV 0x980

/* Exception Numbers */
#define POWER_EXC_SYSTEM_RESET 0x100
#define POWER_EXC_MACHINE_CHECK 0x200
#define POWER_EXC_DATA_STORAGE 0x300
#define POWER_EXC_DATA_SEGMENT 0x380
#define POWER_EXC_INST_STORAGE 0x400
#define POWER_EXC_INST_SEGMENT 0x480
#define POWER_EXC_EXTERNAL 0x500
#define POWER_EXC_ALIGNMENT 0x600
#define POWER_EXC_PROGRAM 0x700
#define POWER_EXC_FP_UNAVAILABLE 0x800
#define POWER_EXC_DECREMENTER 0x900
#define POWER_EXC_SYSTEM_CALL 0xC00
#define POWER_EXC_TRACE 0xD00
#define POWER_EXC_PERFORMANCE 0xF00

/* XIVE Configuration */
#define POWER_XIVE_MAX_IRQS 4096
#define POWER_XIVE_MAX_CPUS 256

// ============================================================================
// TIMER SYSTEM
// ============================================================================

/* Timer Configuration */
#define POWER_TIMER_FREQ 512000000 /* 512MHz default */
#define POWER_TIMER_RESOLUTION_NS 2
#define POWER_TIMER_MAX_PERIOD 0xFFFFFFFFFFFFFFFFULL

/* TB Configuration */
#define POWER_TB_FREQ 512000000
#define POWER_TB_RESOLUTION_NS 2

// ============================================================================
// CACHE CONFIGURATION
// ============================================================================

/* Cache Sizes */
#define POWER_L1I_CACHE_SIZE (32 * 1024)       /* 32KB */
#define POWER_L1D_CACHE_SIZE (64 * 1024)       /* 64KB */
#define POWER_L2_CACHE_SIZE (512 * 1024)       /* 512KB */
#define POWER_L3_CACHE_SIZE (32 * 1024 * 1024) /* 32MB */

/* Cache Line Sizes */
#define POWER_L1_CACHE_LINE_SIZE 128
#define POWER_L2_CACHE_LINE_SIZE 128
#define POWER_L3_CACHE_LINE_SIZE 128

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

/* PMU Configuration */
#define POWER_PMU_MAX_COUNTERS 8
#define POWER_PMU_CYCLE_COUNTER 0
#define POWER_PMU_INSTRUCTION_COUNTER 1

// ============================================================================
// VECTOR EXTENSIONS
// ============================================================================

/* VSX Configuration */
#define POWER_VSX_MAX_LEN 128
#define POWER_VSX_MAX_ELEN 128
#define POWER_VSX_REGISTERS 64

/* AltiVec Configuration */
#define POWER_ALTIVEC_MAX_LEN 128
#define POWER_ALTIVEC_MAX_ELEN 128
#define POWER_ALTIVEC_REGISTERS 32

// ============================================================================
// FLOATING POINT
// ============================================================================

/* FPU Configuration */
#define POWER_FPU_REGISTERS 32
#define POWER_FPU_SINGLE_PRECISION 1
#define POWER_FPU_DOUBLE_PRECISION 1
#define POWER_FPU_QUAD_PRECISION 1

// ============================================================================
// SECURITY
// ============================================================================

/* Security Features */
#define POWER_SECURE_BOOT 1
#define POWER_MEMORY_PROTECTION 1
#define POWER_ASLR 1
#define POWER_HTM 1

// ============================================================================
// DEBUGGING
// ============================================================================

/* Debug Configuration */
#define POWER_DEBUG_MAX_BREAKPOINTS 16
#define POWER_DEBUG_MAX_WATCHPOINTS 8

// ============================================================================
// COMPILER SUPPORT
// ============================================================================

/* Compiler Attributes */
#define POWER_ALIGNED(x) __attribute__((aligned(x)))
#define POWER_PACKED __attribute__((packed))
#define POWER_NORETURN __attribute__((noreturn))
#define POWER_UNUSED __attribute__((unused))

/* MSVC Support */
#ifdef _MSC_VER
#define POWER_ALIGNED(x) __declspec(align(x))
#define POWER_PACKED
#define POWER_NORETURN __declspec(noreturn)
#define POWER_UNUSED
#endif

// ============================================================================
// SYSTEM LIMITS
// ============================================================================

#define POWER_MAX_IRQS 4096
#define POWER_MAX_CPUS 256
#define POWER_MAX_NODES 32
#define POWER_MAX_VMS 128
#define POWER_MAX_PROCESSES 65536
#define POWER_MAX_THREADS 131072
#define POWER_MAX_FILES 131072
#define POWER_MAX_SOCKETS 131072

// ============================================================================
// BUILD CONFIGURATION
// ============================================================================

/* Build Options */
#define POWER_DEBUG_BUILD 1
#define POWER_OPTIMIZATION_LEVEL 2
#define POWER_ASSERTIONS_ENABLED 1

/* Feature Selection */
#define POWER_ENABLE_VSX 1
#define POWER_ENABLE_ALTIVEC 1
#define POWER_ENABLE_FPU 1
#define POWER_ENABLE_HTM 1
#define POWER_ENABLE_SECURITY 1
#define POWER_ENABLE_DEBUG 1

#endif /* ORION_POWER_CONFIG_H */
