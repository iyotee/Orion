/*
 * Orion Operating System - aarch64 Architecture Header
 *
 * Architecture-specific definitions and declarations for aarch64
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_ARCH_AARCH64_H
#define ORION_ARCH_AARCH64_H

#include "../hal/common/types.h"
#include "config.h"

// ============================================================================
// ARCHITECTURE CONSTANTS
// ============================================================================

#define AARCH64_PAGE_SIZE 4096
#define AARCH64_PAGE_SHIFT 12
#define AARCH64_PAGE_MASK (AARCH64_PAGE_SIZE - 1)

#define AARCH64_LARGE_PAGE_SIZE 2097152
#define AARCH64_LARGE_PAGE_SHIFT 21
#define AARCH64_LARGE_PAGE_MASK (AARCH64_LARGE_PAGE_SIZE - 1)

#define AARCH64_HUGE_PAGE_SIZE 1073741824
#define AARCH64_HUGE_PAGE_SHIFT 30
#define AARCH64_HUGE_PAGE_MASK (AARCH64_HUGE_PAGE_SIZE - 1)

// Virtual address space layout
#define AARCH64_KERNEL_BASE 0x40000000
#define AARCH64_KERNEL_SIZE 0x40000000 // 1GB
#define AARCH64_USER_BASE 0x0000000000000000
#define AARCH64_USER_SIZE 0x0000004000000000 // 256GB

// Exception levels
#define AARCH64_EL0 0
#define AARCH64_EL1 1
#define AARCH64_EL2 2
#define AARCH64_EL3 3

// ============================================================================
// CPU FAMILY DEFINITIONS
// ============================================================================

// ARM Cortex-A series
#define AARCH64_CORTEX_A53 0x1001
#define AARCH64_CORTEX_A35 0x1002
#define AARCH64_CORTEX_A55 0x1003
#define AARCH64_CORTEX_A57 0x1004
#define AARCH64_CORTEX_A72 0x1005
#define AARCH64_CORTEX_A73 0x1006
#define AARCH64_CORTEX_A75 0x1007
#define AARCH64_CORTEX_A76 0x1008
#define AARCH64_CORTEX_A77 0x1009
#define AARCH64_CORTEX_A78 0x100A
#define AARCH64_CORTEX_A78C 0x100B
#define AARCH64_CORTEX_A510 0x100C
#define AARCH64_CORTEX_A710 0x100D
#define AARCH64_CORTEX_A715 0x100E
#define AARCH64_CORTEX_A720 0x100F
#define AARCH64_CORTEX_X1 0x1010
#define AARCH64_CORTEX_X2 0x1011
#define AARCH64_CORTEX_X3 0x1012

// Apple Silicon
#define AARCH64_APPLE_M1 0x2001
#define AARCH64_APPLE_M1_PRO 0x2002
#define AARCH64_APPLE_M1_MAX 0x2003
#define AARCH64_APPLE_M1_ULTRA 0x2004
#define AARCH64_APPLE_M2 0x2005
#define AARCH64_APPLE_M2_PRO 0x2006
#define AARCH64_APPLE_M2_MAX 0x2007
#define AARCH64_APPLE_M2_ULTRA 0x2008
#define AARCH64_APPLE_M3 0x2009
#define AARCH64_APPLE_M3_PRO 0x200A
#define AARCH64_APPLE_M3_MAX 0x200B
#define AARCH64_APPLE_M3_ULTRA 0x200C

// ============================================================================
// CPU FEATURE FLAGS
// ============================================================================

// ARMv8-A base features
#define AARCH64_FEATURE_FP (1ULL << 0)
#define AARCH64_FEATURE_ASIMD (1ULL << 1)
#define AARCH64_FEATURE_AES (1ULL << 2)
#define AARCH64_FEATURE_SHA1 (1ULL << 3)
#define AARCH64_FEATURE_SHA2 (1ULL << 4)
#define AARCH64_FEATURE_CRC32 (1ULL << 5)
#define AARCH64_FEATURE_ATOMICS (1ULL << 6)
#define AARCH64_FEATURE_FPHP (1ULL << 7)
#define AARCH64_FEATURE_ASIMDHP (1ULL << 8)
#define AARCH64_FEATURE_CPUID (1ULL << 9)
#define AARCH64_FEATURE_ASIMDRDM (1ULL << 10)
#define AARCH64_FEATURE_JSCVT (1ULL << 11)
#define AARCH64_FEATURE_FCMA (1ULL << 12)
#define AARCH64_FEATURE_LRCPC (1ULL << 13)
#define AARCH64_FEATURE_DCPOP (1ULL << 14)
#define AARCH64_FEATURE_SHA3 (1ULL << 15)
#define AARCH64_FEATURE_SM3 (1ULL << 16)
#define AARCH64_FEATURE_SM4 (1ULL << 17)
#define AARCH64_FEATURE_ASIMDDP (1ULL << 18)
#define AARCH64_FEATURE_SHA512 (1ULL << 19)

// ARMv8.1 features
#define AARCH64_FEATURE_PAN (1ULL << 20)
#define AARCH64_FEATURE_LOR (1ULL << 21)
#define AARCH64_FEATURE_VHE (1ULL << 22)
#define AARCH64_FEATURE_HPDS (1ULL << 23)
#define AARCH64_FEATURE_VMID16 (1ULL << 24)
#define AARCH64_FEATURE_PAN2 (1ULL << 25)
#define AARCH64_FEATURE_UAO (1ULL << 26)
#define AARCH64_FEATURE_DPB (1ULL << 27)
#define AARCH64_FEATURE_DPB2 (1ULL << 28)
#define AARCH64_FEATURE_DCPODP (1ULL << 29)
#define AARCH64_FEATURE_SB (1ULL << 30)
#define AARCH64_FEATURE_IBT (1ULL << 31)

// ARMv8.2 features
#define AARCH64_FEATURE_CSV2 (1ULL << 32)
#define AARCH64_FEATURE_CSV3 (1ULL << 33)

// ARMv8.3 features
#define AARCH64_FEATURE_PAUTH (1ULL << 34)
#define AARCH64_FEATURE_ECV (1ULL << 35)
#define AARCH64_FEATURE_AFP (1ULL << 36)
#define AARCH64_FEATURE_RPRES (1ULL << 37)
#define AARCH64_FEATURE_MTE (1ULL << 38)

// ARMv8.4 features
#define AARCH64_FEATURE_DIT (1ULL << 39)
#define AARCH64_FEATURE_TLBI (1ULL << 40)
#define AARCH64_FEATURE_FLAGM (1ULL << 41)
#define AARCH64_FEATURE_IDST (1ULL << 42)
#define AARCH64_FEATURE_RNG (1ULL << 43)
#define AARCH64_FEATURE_BTI (1ULL << 44)

// ARMv8.5 features
#define AARCH64_FEATURE_SSBS (1ULL << 45)

// ARMv9 features
#define AARCH64_FEATURE_SVE (1ULL << 46)
#define AARCH64_FEATURE_SVE2 (1ULL << 47)
#define AARCH64_FEATURE_SVE2_AES (1ULL << 48)
#define AARCH64_FEATURE_SVE2_SM4 (1ULL << 49)
#define AARCH64_FEATURE_SVE2_SHA3 (1ULL << 50)
#define AARCH64_FEATURE_SVE2_BITPERM (1ULL << 51)
#define AARCH64_FEATURE_SVE2_I8MM (1ULL << 52)
#define AARCH64_FEATURE_SVE2_F32MM (1ULL << 53)
#define AARCH64_FEATURE_SVE2_F64MM (1ULL << 54)

// Apple-specific features
#define AARCH64_FEATURE_APPLE_UNIFIED_MEMORY (1ULL << 55)
#define AARCH64_FEATURE_APPLE_NEURAL (1ULL << 56)
#define AARCH64_FEATURE_APPLE_SECURE_ENCLAVE (1ULL << 57)

// ============================================================================
// POWER MANAGEMENT
// ============================================================================

// Power states
#define AARCH64_POWER_STATE_C0 0 // Running
#define AARCH64_POWER_STATE_C1 1 // Halt
#define AARCH64_POWER_STATE_C2 2 // Stop Grant
#define AARCH64_POWER_STATE_C3 3 // Stop Grant with cache flush
#define AARCH64_POWER_STATE_C4 4 // Deep Stop Grant
#define AARCH64_POWER_STATE_C5 5 // Deep Stop Grant with cache flush
#define AARCH64_POWER_STATE_C6 6 // Deep Power Down

// ============================================================================
// STRUCTURES
// ============================================================================

// CPU registers for context switching
struct aarch64_cpu_context
{
    uint64_t x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30;
    uint64_t sp;
    uint64_t pc;
    uint64_t cpsr;
};

// Exception frame
struct aarch64_exception_frame
{
    uint64_t x0, x1, x2, x3, x4, x5, x6, x7;
    uint64_t x8, x9, x10, x11, x12, x13, x14, x15;
    uint64_t x16, x17, x18, x19, x20, x21, x22, x23;
    uint64_t x24, x25, x26, x27, x28, x29, x30;
    uint64_t sp, lr, pc, cpsr;
    uint64_t esr, far, elr;
};

// MMU page table entry
struct aarch64_pte
{
    uint64_t valid : 1;
    uint64_t table : 1;
    uint64_t attr_index : 3;
    uint64_t ns : 1;
    uint64_t ap : 2;
    uint64_t sh : 2;
    uint64_t af : 1;
    uint64_t ng : 1;
    uint64_t oa : 9;
    uint64_t reserved : 2;
    uint64_t output_addr : 36;
    uint64_t reserved2 : 3;
    uint64_t dbm : 1;
    uint64_t contiguous : 1;
    uint64_t pxn : 1;
    uint64_t uxn : 1;
};

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

// CPU management
void aarch64_detect_cpu_features(void);
bool aarch64_has_feature(uint64_t feature);
void aarch64_print_cpu_info(void);

// MMU management
void aarch64_mmu_init(void);
int aarch64_mmu_map_page(uint64_t va, uint64_t pa, uint64_t flags);
int aarch64_mmu_unmap_page(uint64_t va);
void aarch64_mmu_invalidate_tlb(void);

// Interrupt management
void aarch64_interrupts_init(void);
void aarch64_interrupt_enable(uint32_t irq);
void aarch64_interrupt_disable(uint32_t irq);
void aarch64_interrupt_set_handler(uint32_t irq, void (*handler)(void));

// Timer management
void aarch64_timer_init(void);
uint64_t aarch64_timer_read_ns(void);
int aarch64_timer_set_oneshot(uint64_t deadline_ns);

// Cache management
void aarch64_cache_init(void);
void aarch64_cache_invalidate_all(void);
void aarch64_cache_clean_all(void);

// Vector extensions
void aarch64_neon_init(void);
void aarch64_sve_init(void);

// Security features
void aarch64_security_init(void);
void aarch64_pauth_init(void);
void aarch64_mte_init(void);

// Performance monitoring
void aarch64_pmu_init(void);
uint64_t aarch64_pmu_read_counter(uint32_t counter);
void aarch64_pmu_set_event(uint32_t counter, uint32_t event);

// Power management
void aarch64_power_init(void);
int aarch64_power_set_state(uint32_t state);
uint32_t aarch64_power_get_state(void);

// NUMA management
void aarch64_numa_init(void);
uint32_t aarch64_numa_get_node_count(void);
uint32_t aarch64_numa_get_current_node(void);

// Main initialization
void aarch64_arch_init(void);

// Exception handlers (C functions called from assembly)
void aarch64_sync_exception_handler(uint64_t esr, uint64_t elr, uint64_t far);
void aarch64_irq_exception_handler(void);
void aarch64_fiq_exception_handler(void);
void aarch64_serror_exception_handler(void);
void aarch64_syscall_dispatcher(void);

// External variables
extern uint64_t aarch64_cpu_features;
extern uint32_t aarch64_cpu_family;
extern uint32_t aarch64_cpu_model;

// ============================================================================
// ADVANCED FEATURES DECLARATIONS
// ============================================================================

/* SVE2 Support */
int aarch64_sve2_init(void);
uint32_t aarch64_sve2_get_vector_length(void);
uint32_t aarch64_sve2_get_predicate_registers(void);
uint32_t aarch64_sve2_get_vector_registers(void);

/* NEON Optimizations */
int aarch64_neon_optimize(void);

/* Cryptographic Acceleration */
int aarch64_crypto_init(void);

/* Performance Monitoring Advanced */
int aarch64_pmu_advanced_init(void);

/* Power Management Advanced */
int aarch64_power_advanced_init(void);

/* Virtualization Support */
int aarch64_virtualization_init(void);

/* MSVC Support */
#ifdef _MSC_VER
int aarch64_msvc_optimize(void);
void aarch64_msvc_intrinsics_init(void);
#endif

/* Advanced Features Integration */
int aarch64_advanced_features_init(void);
int aarch64_advanced_features_test(void);
void aarch64_advanced_features_cleanup(void);

#endif // ORION_ARCH_AARCH64_H
