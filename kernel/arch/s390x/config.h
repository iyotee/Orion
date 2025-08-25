/*
 * ORION OS - s390x Architecture Configuration
 *
 * Configuration file for IBM s390x architecture support (z13, z14, z15, z16)
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_S390X_CONFIG_H
#define ORION_S390X_CONFIG_H

// ============================================================================
// ARCHITECTURE IDENTIFICATION
// ============================================================================

#define S390X_ARCH_ID 0x9000
#define S390X_ARCH_NAME "s390x"
#define S390X_ARCH_VERSION 0x0100

// ============================================================================
// CPU CONFIGURATION
// ============================================================================

/* CPU Features */
#define S390X_FEATURE_Z13 1        /* z13 support */
#define S390X_FEATURE_Z14 1        /* z14 support */
#define S390X_FEATURE_Z15 1        /* z15 support */
#define S390X_FEATURE_Z16 1        /* z16 support */
#define S390X_FEATURE_VECTOR 1     /* Vector facility */
#define S390X_FEATURE_VECTOR_ENH 1 /* Vector enhancement facility */
#define S390X_FEATURE_CRYPTO 1     /* Cryptographic acceleration */
#define S390X_FEATURE_DFP 1        /* Decimal Floating Point */
#define S390X_FEATURE_HTM 1        /* Hardware Transactional Memory */
#define S390X_FEATURE_NNPA 1       /* Neural Network Processing Assist */

/* CPU Configuration */
#define S390X_CPU_MAX_FREQ_MHZ 5500
#define S390X_CPU_CACHE_LINE_SIZE 256
#define S390X_CPU_MAX_CORES 128
#define S390X_CPU_MAX_THREADS 256

// ============================================================================
// MEMORY MANAGEMENT
// ============================================================================

/* Page Sizes */
#define S390X_PAGE_SHIFT 12
#define S390X_PAGE_SIZE (1UL << S390X_PAGE_SHIFT)
#define S390X_PAGE_MASK (~(S390X_PAGE_SIZE - 1))

#define S390X_HUGE_PAGE_SHIFT 20
#define S390X_HUGE_PAGE_SIZE (1UL << S390X_HUGE_PAGE_SHIFT)
#define S390X_HUGE_PAGE_MASK (~(S390X_HUGE_PAGE_SIZE - 1))

#define S390X_GIGA_PAGE_SHIFT 30
#define S390X_GIGA_PAGE_SIZE (1UL << S390X_GIGA_PAGE_SHIFT)
#define S390X_GIGA_PAGE_MASK (~(S390X_GIGA_PAGE_SIZE - 1))

/* Virtual Address Space */
#define S390X_KERNEL_BASE 0x8000000000000000UL
#define S390X_KERNEL_SIZE 0x400000000UL /* 16GB */
#define S390X_USER_BASE 0x0000000000000000UL
#define S390X_USER_SIZE 0x8000000000000000UL /* 8EB */

/* TLB Configuration */
#define S390X_TLB_ENTRIES 2048
#define S390X_TLB_WAYS 16

// ============================================================================
// INTERRUPT SYSTEM
// ============================================================================

/* Interrupt Numbers */
#define S390X_INT_EXTERNAL 0x1004
#define S390X_INT_IO 0x2004
#define S390X_INT_CLOCK 0x1005
#define S390X_INT_CPU_TIMER 0x1006
#define S390X_INT_RESTART 0x1007
#define S390X_INT_EMERGENCY 0x1201
#define S390X_INT_SERVICE 0x2401

/* Exception Numbers */
#define S390X_EXC_OPERATION 0x0001
#define S390X_EXC_PRIVILEGED 0x0002
#define S390X_EXC_EXECUTE 0x0003
#define S390X_EXC_PROTECTION 0x0004
#define S390X_EXC_ADDRESSING 0x0005
#define S390X_EXC_SPECIFICATION 0x0006
#define S390X_EXC_DATA 0x0007
#define S390X_EXC_FIXED_POINT_OVERFLOW 0x0008
#define S390X_EXC_FIXED_POINT_DIVIDE 0x0009
#define S390X_EXC_DECIMAL_OVERFLOW 0x000A
#define S390X_EXC_DECIMAL_DIVIDE 0x000B
#define S390X_EXC_HFP_EXPONENT_OVERFLOW 0x000C
#define S390X_EXC_HFP_EXPONENT_UNDERFLOW 0x000D
#define S390X_EXC_HFP_SIGNIFICANCE 0x000E
#define S390X_EXC_HFP_DIVIDE 0x000F
#define S390X_EXC_SEGMENT_TRANSLATION 0x0010
#define S390X_EXC_PAGE_TRANSLATION 0x0011
#define S390X_EXC_TRANSLATION_SPECIFICATION 0x0012
#define S390X_EXC_SPECIAL_OPERATION 0x0013
#define S390X_EXC_OPERAND 0x0014
#define S390X_EXC_TRACE 0x0015
#define S390X_EXC_MONITOR 0x0016
#define S390X_EXC_AFX_TRANSLATION 0x0017
#define S390X_EXC_ASX_TRANSLATION 0x0018
#define S390X_EXC_PRIMARY_AFX 0x0019
#define S390X_EXC_SECONDARY_AFX 0x001A
#define S390X_EXC_PRIMARY_ASX 0x001B
#define S390X_EXC_SECONDARY_ASX 0x001C
#define S390X_EXC_LFX_TRANSLATION 0x001D
#define S390X_EXC_LSX_TRANSLATION 0x001E
#define S390X_EXC_PRIMARY_LFX 0x001F
#define S390X_EXC_SECONDARY_LFX 0x0020
#define S390X_EXC_PRIMARY_LSX 0x0021
#define S390X_EXC_SECONDARY_LSX 0x0022
#define S390X_EXC_VECTOR 0x0023
#define S390X_EXC_DATA_SPACE 0x0024
#define S390X_EXC_VECTOR_SPACE 0x0025
#define S390X_EXC_SPACE_SWITCH 0x0026
#define S390X_EXC_HFP_SQUARE_ROOT 0x0027
#define S390X_EXC_PC_TRANSLATION 0x0028
#define S390X_EXC_AFX_REGISTRATION 0x0029
#define S390X_EXC_ASX_REGISTRATION 0x002A
#define S390X_EXC_LFX_REGISTRATION 0x002B
#define S390X_EXC_LSX_REGISTRATION 0x002C
#define S390X_EXC_CRYPTO_OPERATION 0x002D

/* I/O Interrupt Configuration */
#define S390X_IO_MAX_IRQS 8192
#define S390X_IO_MAX_CPUS 256

// ============================================================================
// TIMER SYSTEM
// ============================================================================

/* Timer Configuration */
#define S390X_TIMER_FREQ 16777216 /* 16.777216MHz default */
#define S390X_TIMER_RESOLUTION_NS 60
#define S390X_TIMER_MAX_PERIOD 0xFFFFFFFFFFFFFFFFULL

/* TOD Configuration */
#define S390X_TOD_FREQ 4000000000 /* 4GHz */
#define S390X_TOD_RESOLUTION_NS 0.25

// ============================================================================
// CACHE CONFIGURATION
// ============================================================================

/* Cache Sizes */
#define S390X_L1I_CACHE_SIZE (128 * 1024)      /* 128KB */
#define S390X_L1D_CACHE_SIZE (128 * 1024)      /* 128KB */
#define S390X_L2_CACHE_SIZE (4 * 1024 * 1024)  /* 4MB */
#define S390X_L3_CACHE_SIZE (64 * 1024 * 1024) /* 64MB */

/* Cache Line Sizes */
#define S390X_L1_CACHE_LINE_SIZE 256
#define S390X_L2_CACHE_LINE_SIZE 256
#define S390X_L3_CACHE_LINE_SIZE 256

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

/* PMU Configuration */
#define S390X_PMU_MAX_COUNTERS 16
#define S390X_PMU_CYCLE_COUNTER 0
#define S390X_PMU_INSTRUCTION_COUNTER 1

// ============================================================================
// VECTOR EXTENSIONS
// ============================================================================

/* Vector Configuration */
#define S390X_VECTOR_MAX_LEN 256
#define S390X_VECTOR_MAX_ELEN 128
#define S390X_VECTOR_REGISTERS 32

// ============================================================================
// FLOATING POINT
// ============================================================================

/* FPU Configuration */
#define S390X_FPU_REGISTERS 16
#define S390X_FPU_SINGLE_PRECISION 1
#define S390X_FPU_DOUBLE_PRECISION 1
#define S390X_FPU_QUAD_PRECISION 1
#define S390X_FPU_DECIMAL 1

// ============================================================================
// SECURITY
// ============================================================================

/* Security Features */
#define S390X_SECURE_BOOT 1
#define S390X_MEMORY_PROTECTION 1
#define S390X_ASLR 1
#define S390X_CRYPTO_ACCELERATION 1

// ============================================================================
// DEBUGGING
// ============================================================================

/* Debug Configuration */
#define S390X_DEBUG_MAX_BREAKPOINTS 32
#define S390X_DEBUG_MAX_WATCHPOINTS 16

// ============================================================================
// COMPILER SUPPORT
// ============================================================================

/* Compiler Attributes */
#define S390X_ALIGNED(x) __attribute__((aligned(x)))
#define S390X_PACKED __attribute__((packed))
#define S390X_NORETURN __attribute__((noreturn))
#define S390X_UNUSED __attribute__((unused))

/* MSVC Support */
#ifdef _MSC_VER
#define S390X_ALIGNED(x) __declspec(align(x))
#define S390X_PACKED
#define S390X_NORETURN __declspec(noreturn)
#define S390X_UNUSED
#endif

// ============================================================================
// SYSTEM LIMITS
// ============================================================================

#define S390X_MAX_IRQS 8192
#define S390X_MAX_CPUS 256
#define S390X_MAX_NODES 64
#define S390X_MAX_VMS 256
#define S390X_MAX_PROCESSES 65536
#define S390X_MAX_THREADS 131072
#define S390X_MAX_FILES 131072
#define S390X_MAX_SOCKETS 131072

// ============================================================================
// BUILD CONFIGURATION
// ============================================================================

/* Build Options */
#define S390X_DEBUG_BUILD 1
#define S390X_OPTIMIZATION_LEVEL 2
#define S390X_ASSERTIONS_ENABLED 1

/* Feature Selection */
#define S390X_ENABLE_VECTOR 1
#define S390X_ENABLE_FPU 1
#define S390X_ENABLE_CRYPTO 1
#define S390X_ENABLE_HTM 1
#define S390X_ENABLE_SECURITY 1
#define S390X_ENABLE_DEBUG 1

#endif /* ORION_S390X_CONFIG_H */
