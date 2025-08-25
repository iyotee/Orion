/*
 * ORION OS - MIPS Architecture Configuration
 *
 * Configuration file for MIPS architecture support (MIPS32, MIPS64)
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_MIPS_CONFIG_H
#define ORION_MIPS_CONFIG_H

// ============================================================================
// ARCHITECTURE IDENTIFICATION
// ============================================================================

#define MIPS_ARCH_ID 0x6000
#define MIPS_ARCH_NAME "MIPS"
#define MIPS_ARCH_VERSION 0x0100

// ============================================================================
// CPU CONFIGURATION
// ============================================================================

/* CPU Features */
#define MIPS_FEATURE_MIPS32 1    /* MIPS32 support */
#define MIPS_FEATURE_MIPS64 1    /* MIPS64 support */
#define MIPS_FEATURE_MIPS16 1    /* MIPS16 support */
#define MIPS_FEATURE_MICROMIPS 1 /* microMIPS support */
#define MIPS_FEATURE_DSP 1       /* DSP ASE support */
#define MIPS_FEATURE_MSA 1       /* MIPS SIMD Architecture */
#define MIPS_FEATURE_VZ 1        /* Virtualization support */
#define MIPS_FEATURE_CRC 1       /* CRC ASE support */
#define MIPS_FEATURE_GINV 1      /* Global Invalidate support */

/* CPU Configuration */
#define MIPS_CPU_MAX_FREQ_MHZ 2000
#define MIPS_CPU_CACHE_LINE_SIZE 32
#define MIPS_CPU_MAX_CORES 64
#define MIPS_CPU_MAX_THREADS 128

// ============================================================================
// MEMORY MANAGEMENT
// ============================================================================

/* Page Sizes */
#define MIPS_PAGE_SHIFT 12
#define MIPS_PAGE_SIZE (1UL << MIPS_PAGE_SHIFT)
#define MIPS_PAGE_MASK (~(MIPS_PAGE_SIZE - 1))

#define MIPS_HUGE_PAGE_SHIFT 21
#define MIPS_HUGE_PAGE_SIZE (1UL << MIPS_HUGE_PAGE_SHIFT)
#define MIPS_HUGE_PAGE_MASK (~(MIPS_HUGE_PAGE_SIZE - 1))

#define MIPS_GIGA_PAGE_SHIFT 30
#define MIPS_GIGA_PAGE_SIZE (1UL << MIPS_GIGA_PAGE_SHIFT)
#define MIPS_GIGA_PAGE_MASK (~(MIPS_GIGA_PAGE_SIZE - 1))

/* Virtual Address Space */
#define MIPS_KERNEL_BASE 0x8000000000000000UL
#define MIPS_KERNEL_SIZE 0x400000000UL /* 16GB */
#define MIPS_USER_BASE 0x0000000000000000UL
#define MIPS_USER_SIZE 0x8000000000000000UL /* 8EB */

/* TLB Configuration */
#define MIPS_TLB_ENTRIES 64
#define MIPS_TLB_WAYS 4

// ============================================================================
// INTERRUPT SYSTEM
// ============================================================================

/* Interrupt Numbers */
#define MIPS_INT_SOFTWARE0 0
#define MIPS_INT_SOFTWARE1 1
#define MIPS_INT_TIMER 7
#define MIPS_INT_EXTERNAL 2
#define MIPS_INT_PERFORMANCE 5

/* Exception Numbers */
#define MIPS_EXC_INTERRUPT 0
#define MIPS_EXC_TLB_MODIFICATION 1
#define MIPS_EXC_TLB_MISS_LOAD 2
#define MIPS_EXC_TLB_MISS_STORE 3
#define MIPS_EXC_ADDRESS_ERROR_LOAD 4
#define MIPS_EXC_ADDRESS_ERROR_STORE 5
#define MIPS_EXC_INSTRUCTION_BUS_ERROR 6
#define MIPS_EXC_DATA_BUS_ERROR 7
#define MIPS_EXC_SYSCALL 8
#define MIPS_EXC_BREAKPOINT 9
#define MIPS_EXC_RESERVED_INSTRUCTION 10
#define MIPS_EXC_COPROCESSOR_UNUSABLE 11
#define MIPS_EXC_OVERFLOW 12
#define MIPS_EXC_TRAP 13
#define MIPS_EXC_FLOATING_POINT 15
#define MIPS_EXC_COPROCESSOR2 18
#define MIPS_EXC_MDMX 22
#define MIPS_EXC_WATCH 23
#define MIPS_EXC_MCHECK 24
#define MIPS_EXC_THREAD 25
#define MIPS_EXC_DSP 26
#define MIPS_EXC_VIRTUALIZATION 27
#define MIPS_EXC_MSA_FP 28

/* Interrupt Controller */
#define MIPS_GIC_MAX_IRQS 256
#define MIPS_GIC_MAX_CPUS 64

// ============================================================================
// TIMER SYSTEM
// ============================================================================

/* Timer Configuration */
#define MIPS_TIMER_FREQ 100000000 /* 100MHz default */
#define MIPS_TIMER_RESOLUTION_NS 10
#define MIPS_TIMER_MAX_PERIOD 0xFFFFFFFFFFFFFFFFULL

/* CP0 Configuration */
#define MIPS_CP0_COUNT_FREQ 100000000
#define MIPS_CP0_COMPARE_FREQ 100000000

// ============================================================================
// CACHE CONFIGURATION
// ============================================================================

/* Cache Sizes */
#define MIPS_L1I_CACHE_SIZE (32 * 1024)      /* 32KB */
#define MIPS_L1D_CACHE_SIZE (32 * 1024)      /* 32KB */
#define MIPS_L2_CACHE_SIZE (256 * 1024)      /* 256KB */
#define MIPS_L3_CACHE_SIZE (4 * 1024 * 1024) /* 4MB */

/* Cache Line Sizes */
#define MIPS_L1_CACHE_LINE_SIZE 32
#define MIPS_L2_CACHE_LINE_SIZE 32
#define MIPS_L3_CACHE_LINE_SIZE 64

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

/* PMU Configuration */
#define MIPS_PMU_MAX_COUNTERS 4
#define MIPS_PMU_CYCLE_COUNTER 0
#define MIPS_PMU_INSTRUCTION_COUNTER 1

// ============================================================================
// VECTOR EXTENSIONS
// ============================================================================

/* MSA Configuration */
#define MIPS_MSA_MAX_LEN 128
#define MIPS_MSA_MAX_ELEN 128
#define MIPS_MSA_REGISTERS 32

/* DSP Configuration */
#define MIPS_DSP_MAX_LEN 64
#define MIPS_DSP_MAX_ELEN 64
#define MIPS_DSP_REGISTERS 8

// ============================================================================
// FLOATING POINT
// ============================================================================

/* FPU Configuration */
#define MIPS_FPU_REGISTERS 32
#define MIPS_FPU_SINGLE_PRECISION 1
#define MIPS_FPU_DOUBLE_PRECISION 1
#define MIPS_FPU_QUAD_PRECISION 0

// ============================================================================
// SECURITY
// ============================================================================

/* Security Features */
#define MIPS_SECURE_BOOT 1
#define MIPS_MEMORY_PROTECTION 1
#define MIPS_ASLR 1

// ============================================================================
// DEBUGGING
// ============================================================================

/* Debug Configuration */
#define MIPS_DEBUG_MAX_BREAKPOINTS 8
#define MIPS_DEBUG_MAX_WATCHPOINTS 4

// ============================================================================
// COMPILER SUPPORT
// ============================================================================

/* Compiler Attributes */
#define MIPS_ALIGNED(x) __attribute__((aligned(x)))
#define MIPS_PACKED __attribute__((packed))
#define MIPS_NORETURN __attribute__((noreturn))
#define MIPS_UNUSED __attribute__((unused))

/* MSVC Support */
#ifdef _MSC_VER
#define MIPS_ALIGNED(x) __declspec(align(x))
#define MIPS_PACKED
#define MIPS_NORETURN __declspec(noreturn)
#define MIPS_UNUSED
#endif

// ============================================================================
// SYSTEM LIMITS
// ============================================================================

#define MIPS_MAX_IRQS 256
#define MIPS_MAX_CPUS 64
#define MIPS_MAX_NODES 16
#define MIPS_MAX_VMS 32
#define MIPS_MAX_PROCESSES 32768
#define MIPS_MAX_THREADS 65536
#define MIPS_MAX_FILES 65536
#define MIPS_MAX_SOCKETS 65536

// ============================================================================
// SYSTEM LIMITS
// ============================================================================

#define MIPS_MAX_IRQS 8
#define MIPS_MAX_EXCEPTIONS 16
#define MIPS_MAX_SYSCALLS 32
#define MIPS_MAX_TIMERS 8

// ============================================================================
// TIMER CONFIGURATION
// ============================================================================

#define MIPS_TIMER_MODE_ONE_SHOT 0
#define MIPS_TIMER_MODE_PERIODIC 1
#define MIPS_TIMER_MODE_CONTINUOUS 2

// ============================================================================
// VECTOR AND INTERRUPT CONFIGURATION
// ============================================================================

#define MIPS_VECTOR_BASE 0x80000000
#define MIPS_PRIORITY_BITS 3

// ============================================================================
// EXCEPTION TYPES
// ============================================================================

#define MIPS_EXCEPTION_UNDEFINED 0
#define MIPS_EXCEPTION_SOFTWARE 1
#define MIPS_EXCEPTION_PREFETCH 2
#define MIPS_EXCEPTION_DATA 3
#define MIPS_EXCEPTION_RESERVED 4
#define MIPS_EXCEPTION_IRQ 5
#define MIPS_EXCEPTION_FIQ 6

// ============================================================================
// BUILD CONFIGURATION
// ============================================================================

/* Build Options */
#define MIPS_DEBUG_BUILD 1
#define MIPS_OPTIMIZATION_LEVEL 2
#define MIPS_ASSERTIONS_ENABLED 1

/* Feature Selection */
#define MIPS_ENABLE_MSA 1
#define MIPS_ENABLE_DSP 1
#define MIPS_ENABLE_FPU 1
#define MIPS_ENABLE_VZ 1
#define MIPS_ENABLE_SECURITY 1
#define MIPS_ENABLE_DEBUG 1

#endif /* ORION_MIPS_CONFIG_H */
