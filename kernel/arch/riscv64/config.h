/*
 * ORION OS - RISC-V 64-bit Configuration
 *
 * Configuration file for RISC-V 64-bit architecture support
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_RISCV64_CONFIG_H
#define ORION_RISCV64_CONFIG_H

// ============================================================================
// ARCHITECTURE IDENTIFICATION
// ============================================================================

#define RISCV64_ARCH_ID 0x8000
#define RISCV64_ARCH_NAME "RISC-V64"
#define RISCV64_ARCH_VERSION 0x0100

// ============================================================================
// CPU CONFIGURATION
// ============================================================================

/* CPU Features */
#define RISCV64_FEATURE_RV64I (1ULL << 0) /* RV64I base integer */
#define RISCV64_FEATURE_RV64M (1ULL << 1) /* RV64M multiply/divide */
#define RISCV64_FEATURE_RV64A (1ULL << 2) /* RV64A atomic instructions */
#define RISCV64_FEATURE_RV64F (1ULL << 3) /* RV64F single-precision FP */
#define RISCV64_FEATURE_RV64D (1ULL << 4) /* RV64D double-precision FP */
#define RISCV64_FEATURE_RV64C (1ULL << 5) /* RV64C compressed instructions */
#define RISCV64_FEATURE_RVV (1ULL << 6)   /* Vector extension */
#define RISCV64_FEATURE_RVH (1ULL << 7)   /* Hypervisor extension */
#define RISCV64_FEATURE_RVP (1ULL << 8)   /* Packed SIMD extension */
#define RISCV64_FEATURE_RV64B (1ULL << 9) /* RV64B bit manipulation */

/* CPU Configuration */
#define RISCV64_CPU_MAX_FREQ_MHZ 2500
#define RISCV64_CPU_CACHE_LINE_SIZE 64
#define RISCV64_CPU_MAX_CORES 64
#define RISCV64_CPU_MAX_THREADS 128

// ============================================================================
// MEMORY MANAGEMENT
// ============================================================================

/* Page Sizes */
#define RISCV64_PAGE_SHIFT 12
#define RISCV64_PAGE_SIZE (1UL << RISCV64_PAGE_SHIFT)
#define RISCV64_PAGE_MASK (~(RISCV64_PAGE_SIZE - 1))

#define RISCV64_HUGE_PAGE_SHIFT 21
#define RISCV64_HUGE_PAGE_SIZE (1UL << RISCV64_HUGE_PAGE_SHIFT)
#define RISCV64_HUGE_PAGE_MASK (~(RISCV64_HUGE_PAGE_SIZE - 1))

#define RISCV64_GIGA_PAGE_SHIFT 30
#define RISCV64_GIGA_PAGE_SIZE (1UL << RISCV64_GIGA_PAGE_SHIFT)
#define RISCV64_GIGA_PAGE_MASK (~(RISCV64_GIGA_PAGE_SIZE - 1))

/* Virtual Address Space */
#define RISCV64_KERNEL_BASE 0x8000000000000000UL
#define RISCV64_KERNEL_SIZE 0x400000000UL /* 16GB */
#define RISCV64_USER_BASE 0x0000000000000000UL
#define RISCV64_USER_SIZE 0x8000000000000000UL /* 8EB */

/* TLB Configuration */
#define RISCV64_TLB_ENTRIES 512
#define RISCV64_TLB_WAYS 4

// ============================================================================
// INTERRUPT SYSTEM
// ============================================================================

/* Interrupt Numbers */
#define RISCV64_INT_SOFTWARE 1
#define RISCV64_INT_TIMER 5
#define RISCV64_INT_EXTERNAL 9

/* Exception Numbers */
#define RISCV64_EXC_INSTRUCTION_MISALIGNED 0
#define RISCV64_EXC_INSTRUCTION_ACCESS_FAULT 1
#define RISCV64_EXC_ILLEGAL_INSTRUCTION 2
#define RISCV64_EXC_BREAKPOINT 3
#define RISCV64_EXC_LOAD_ADDRESS_MISALIGNED 4
#define RISCV64_EXC_LOAD_ACCESS_FAULT 5
#define RISCV64_EXC_STORE_ADDRESS_MISALIGNED 6
#define RISCV64_EXC_STORE_ACCESS_FAULT 7
#define RISCV64_EXC_ECALL_U_MODE 8
#define RISCV64_EXC_ECALL_S_MODE 9
#define RISCV64_EXC_ECALL_M_MODE 11
#define RISCV64_EXC_INSTRUCTION_PAGE_FAULT 12
#define RISCV64_EXC_LOAD_PAGE_FAULT 13
#define RISCV64_EXC_STORE_PAGE_FAULT 15

/* Interrupt Controller */
#define RISCV64_PLIC_MAX_IRQS 1024
#define RISCV64_PLIC_MAX_CONTEXTS 256

// ============================================================================
// CSR DEFINITIONS
// ============================================================================

/* Supervisor Mode CSRs */
#define RISCV64_CSR_SSTATUS 0x100
#define RISCV64_CSR_SIE 0x104
#define RISCV64_CSR_STVEC 0x105
#define RISCV64_CSR_SCOUNTEREN 0x106
#define RISCV64_CSR_SSCRATCH 0x140
#define RISCV64_CSR_SEPC 0x141
#define RISCV64_CSR_SCAUSE 0x142
#define RISCV64_CSR_STVAL 0x143
#define RISCV64_CSR_SIP 0x144
#define RISCV64_CSR_SATP 0x180

// ============================================================================
// TIMER SYSTEM
// ============================================================================

/* Timer Configuration */
#define RISCV64_TIMER_FREQ 1000000000 /* 1GHz default */
#define RISCV64_TIMER_RESOLUTION_NS 1
#define RISCV64_TIMER_MAX_PERIOD 0xFFFFFFFFFFFFFFFFULL

/* CLINT Configuration */
#define RISCV64_CLINT_BASE 0x2000000
#define RISCV64_CLINT_SIZE 0x10000

// ============================================================================
// CACHE CONFIGURATION
// ============================================================================

/* Cache Sizes */
#define RISCV64_L1I_CACHE_SIZE (32 * 1024)      /* 32KB */
#define RISCV64_L1D_CACHE_SIZE (32 * 1024)      /* 32KB */
#define RISCV64_L2_CACHE_SIZE (256 * 1024)      /* 256KB */
#define RISCV64_L3_CACHE_SIZE (8 * 1024 * 1024) /* 8MB */

/* Cache Line Sizes */
#define RISCV64_L1_CACHE_LINE_SIZE 64
#define RISCV64_L2_CACHE_LINE_SIZE 64
#define RISCV64_L3_CACHE_LINE_SIZE 64

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

/* PMU Configuration */
#define RISCV64_PMU_MAX_COUNTERS 32
#define RISCV64_PMU_CYCLE_COUNTER 0
#define RISCV64_PMU_INSTRUCTION_COUNTER 1

// ============================================================================
// VECTOR EXTENSIONS
// ============================================================================

/* Vector Configuration */
#define RISCV64_VECTOR_MAX_LEN 1024
#define RISCV64_VECTOR_MAX_ELEN 64
#define RISCV64_VECTOR_REGISTERS 32

// ============================================================================
// FLOATING POINT
// ============================================================================

/* FPU Configuration */
#define RISCV64_FPU_REGISTERS 32
#define RISCV64_FPU_SINGLE_PRECISION 1
#define RISCV64_FPU_DOUBLE_PRECISION 1
#define RISCV64_FPU_QUAD_PRECISION 0

// ============================================================================
// SECURITY
// ============================================================================

/* Security Features */
#define RISCV64_SECURE_BOOT 1
#define RISCV64_MEMORY_PROTECTION 1
#define RISCV64_ASLR 1

// ============================================================================
// DEBUGGING
// ============================================================================

/* Debug Configuration */
#define RISCV64_DEBUG_MAX_BREAKPOINTS 8
#define RISCV64_DEBUG_MAX_WATCHPOINTS 4

// ============================================================================
// COMPILER SUPPORT
// ============================================================================

/* Compiler Attributes */
#define RISCV64_ALIGNED(x) __attribute__((aligned(x)))
#define RISCV64_PACKED __attribute__((packed))
#define RISCV64_NORETURN __attribute__((noreturn))
#define RISCV64_UNUSED __attribute__((unused))

/* MSVC Support */
#ifdef _MSC_VER
#define RISCV64_ALIGNED(x) __declspec(align(x))
#define RISCV64_PACKED
#define RISCV64_NORETURN __declspec(noreturn)
#define RISCV64_UNUSED
#endif

// ============================================================================
// SYSTEM LIMITS
// ============================================================================

#define RISCV64_MAX_IRQS 1024
#define RISCV64_MAX_CPUS 256
#define RISCV64_MAX_NODES 16
#define RISCV64_MAX_VMS 64
#define RISCV64_MAX_PROCESSES 32768
#define RISCV64_MAX_THREADS 65536
#define RISCV64_MAX_FILES 65536
#define RISCV64_MAX_SOCKETS 65536

// ============================================================================
// BUILD CONFIGURATION
// ============================================================================

/* Build Options */
#define RISCV64_DEBUG_BUILD 1
#define RISCV64_OPTIMIZATION_LEVEL 2
#define RISCV64_ASSERTIONS_ENABLED 1

/* Feature Selection */
#define RISCV64_ENABLE_VECTOR 1
#define RISCV64_ENABLE_FPU 1
#define RISCV64_ENABLE_HYPERVISOR 1
#define RISCV64_ENABLE_SECURITY 1
#define RISCV64_ENABLE_DEBUG 1

#endif /* ORION_RISCV64_CONFIG_H */
