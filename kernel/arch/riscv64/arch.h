/*
 * ORION OS - RISC-V 64-bit Architecture Support
 *
 * This file defines RISC-V 64-bit specific constants, structures, and function
 * declarations for the ORION kernel.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_ARCH_RISCV64_H
#define ORION_ARCH_RISCV64_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

// ============================================================================
// ARCHITECTURE IDENTIFICATION
// ============================================================================

#define RISCV64_ARCH_ID 0x8000
#define RISCV64_ARCH_NAME "RISC-V64"
#define RISCV64_ARCH_VERSION 0x0100

// ============================================================================
// RISC-V SPECIFIC CONSTANTS
// ============================================================================

/* Privilege Levels */
#define RISCV64_MODE_U 0x00 // User mode
#define RISCV64_MODE_S 0x01 // Supervisor mode
#define RISCV64_MODE_M 0x03 // Machine mode

/* CSR Addresses */
#define RISCV64_CSR_MSTATUS 0x300
#define RISCV64_CSR_MISA 0x301
#define RISCV64_CSR_MIE 0x304
#define RISCV64_CSR_MTVEC 0x305
#define RISCV64_CSR_MEPC 0x341
#define RISCV64_CSR_MCAUSE 0x342
#define RISCV64_CSR_MTVAL 0x343
#define RISCV64_CSR_MIP 0x344
#define RISCV64_CSR_MHARTID 0xF14
#define RISCV64_CSR_MHPMCOUNTER3 0xB03
#define RISCV64_CSR_MHPMCOUNTER4 0xB04
#define RISCV64_CSR_MHPMCOUNTER5 0xB05
#define RISCV64_CSR_MHPMCOUNTER6 0xB06
#define RISCV64_CSR_MHPMCOUNTER7 0xB07
#define RISCV64_CSR_MHPMCOUNTER8 0xB08
#define RISCV64_CSR_MHPMCOUNTER9 0xB09
#define RISCV64_CSR_MHPMCOUNTER10 0xB0A
#define RISCV64_CSR_MHPMCOUNTER11 0xB0B
#define RISCV64_CSR_MHPMCOUNTER12 0xB0C
#define RISCV64_CSR_MHPMCOUNTER13 0xB0D
#define RISCV64_CSR_MHPMCOUNTER14 0xB0E
#define RISCV64_CSR_MHPMCOUNTER15 0xB0F
#define RISCV64_CSR_MHPMCOUNTER16 0xB10
#define RISCV64_CSR_MHPMCOUNTER17 0xB11
#define RISCV64_CSR_MHPMCOUNTER18 0xB12
#define RISCV64_CSR_MHPMCOUNTER19 0xB13
#define RISCV64_CSR_MHPMCOUNTER20 0xB14
#define RISCV64_CSR_MHPMCOUNTER21 0xB15
#define RISCV64_CSR_MHPMCOUNTER22 0xB16
#define RISCV64_CSR_MHPMCOUNTER23 0xB17
#define RISCV64_CSR_MHPMCOUNTER24 0xB18
#define RISCV64_CSR_MHPMCOUNTER25 0xB19
#define RISCV64_CSR_MHPMCOUNTER26 0xB1A
#define RISCV64_CSR_MHPMCOUNTER27 0xB1B
#define RISCV64_CSR_MHPMCOUNTER28 0xB1C
#define RISCV64_CSR_MHPMCOUNTER29 0xB1D
#define RISCV64_CSR_MHPMCOUNTER30 0xB1E
#define RISCV64_CSR_MHPMCOUNTER31 0xB1F

/* Memory Management - Defined in config.h */

/* Interrupt Numbers */
#define RISCV64_IRQ_SOFTWARE 3
#define RISCV64_IRQ_TIMER 7
#define RISCV64_IRQ_EXTERNAL 11

/* Exception Numbers */
#define RISCV64_EXC_INSTRUCTION_MISALIGNED 0
#define RISCV64_EXC_INSTRUCTION_ACCESS_FAULT 1
#define RISCV64_EXC_ILLEGAL_INSTRUCTION 2
#define RISCV64_EXC_BREAKPOINT 3
#define RISCV64_EXC_LOAD_ADDRESS_MISALIGNED 4
#define RISCV64_EXC_LOAD_ACCESS_FAULT 5
#define RISCV64_EXC_STORE_ADDRESS_MISALIGNED 6
#define RISCV64_EXC_STORE_ACCESS_FAULT 7
#define RISCV64_EXC_ECALL_U 8
#define RISCV64_EXC_ECALL_S 9
#define RISCV64_EXC_ECALL_M 11
#define RISCV64_EXC_INSTRUCTION_PAGE_FAULT 12
#define RISCV64_EXC_LOAD_PAGE_FAULT 13
#define RISCV64_EXC_STORE_PAGE_FAULT 15

// ============================================================================
// DATA TYPES
// ============================================================================

typedef uint64_t riscv64_vaddr_t;
typedef uint64_t riscv64_paddr_t;
typedef uint64_t riscv64_size_t;
typedef uint64_t riscv64_register_t;

// ============================================================================
// STRUCTURES
// ============================================================================

/* CPU Features */
struct riscv64_cpu_features
{
    uint64_t base_isa;
    uint64_t extensions;
    uint64_t vendor_id;
    uint64_t architecture_id;
    uint64_t implementation_id;
    uint64_t hart_id;
    uint64_t mxl;
    uint64_t sxl;
    uint64_t uxl;
};

/* MMU Context */
struct riscv64_mmu_context
{
    uint64_t satp; // Supervisor Address Translation and Protection
    uint64_t asid; // Address Space ID
    uint64_t *root_page_table;
    uint64_t page_table_count;
};

/* Interrupt Context */
struct riscv64_interrupt_context
{
    uint64_t ra, sp, gp, tp;
    uint64_t t0, t1, t2;
    uint64_t s0, s1;
    uint64_t a0, a1, a2, a3, a4, a5, a6, a7;
    uint64_t s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
    uint64_t t3, t4, t5, t6;
    uint64_t pc, status, cause, tval;
};

/* Timer Configuration */
struct riscv64_timer_config
{
    uint64_t frequency;
    uint64_t compare_value;
    uint64_t current_value;
    bool enabled;
    bool interrupt_enabled;
};

/* Power Management */
struct riscv64_power_state
{
    uint32_t current_state;
    uint32_t supported_states;
    uint64_t wakeup_time;
    bool deep_sleep_enabled;
};

/* NUMA Topology */
struct riscv64_numa_topology
{
    uint32_t node_count;
    uint32_t current_node;
    uint32_t cluster_count;
    uint32_t cores_per_cluster;
    uint32_t distance_matrix[16][16];
};

/* Performance Monitoring */
struct riscv64_pmu_config
{
    uint32_t counter_count;
    uint32_t event_count;
    uint64_t counters[32];
    uint64_t events[32];
    bool enabled;
};

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

// CPU management
void riscv64_detect_cpu_features(void);
bool riscv64_has_feature(uint64_t feature);
void riscv64_print_cpu_info(void);

// MMU management
void riscv64_mmu_init(void);
int riscv64_mmu_map_page(uint64_t va, uint64_t pa, uint64_t flags);
int riscv64_mmu_unmap_page(uint64_t va);
void riscv64_mmu_invalidate_tlb(void);

// Interrupt management
void riscv64_interrupts_init(void);
void riscv64_interrupt_enable(uint32_t irq);
void riscv64_interrupt_disable(uint32_t irq);
void riscv64_interrupt_set_handler(uint32_t irq, void (*handler)(void));

// Timer management
void riscv64_timer_init(void);
uint64_t riscv64_timer_read_ns(void);
int riscv64_timer_set_oneshot(uint64_t deadline_ns);

// Cache management
void riscv64_cache_init(void);
void riscv64_cache_invalidate_all(void);
void riscv64_cache_clean_all(void);

// Vector extensions
void riscv64_vector_init(void);
void riscv64_vector_enable(void);

// Security features
void riscv64_security_init(void);
void riscv64_pmp_init(void);

// Performance monitoring
void riscv64_pmu_init(void);
uint64_t riscv64_pmu_read_counter(uint32_t counter);
void riscv64_pmu_set_event(uint32_t counter, uint32_t event);

// Power management
void riscv64_power_init(void);
int riscv64_power_set_state(uint32_t state);
uint32_t riscv64_power_get_state(void);

// NUMA management
void riscv64_numa_init(void);
uint32_t riscv64_numa_get_node_count(void);
uint32_t riscv64_numa_get_current_node(void);

// Main initialization
void riscv64_arch_init(void);

// Exception handlers (C functions called from assembly)
void riscv64_sync_exception_handler(uint64_t cause, uint64_t epc, uint64_t tval);
void riscv64_interrupt_handler(void);
void riscv64_syscall_dispatcher(void);

// External variables
extern uint64_t riscv64_cpu_features;
extern uint32_t riscv64_cpu_family;
extern uint32_t riscv64_cpu_model;

// ============================================================================
// ADVANCED FEATURES DECLARATIONS
// ============================================================================

/* Vector Extensions */
int riscv64_vector_rvv_init(void);
uint32_t riscv64_vector_get_vlen(void);
uint32_t riscv64_vector_get_vlenb(void);

/* Cryptographic Acceleration */
int riscv64_crypto_init(void);
bool riscv64_crypto_aes_supported(void);
bool riscv64_crypto_sha_supported(void);

/* Performance Monitoring Advanced */
int riscv64_pmu_advanced_init(void);
uint64_t riscv64_pmu_read_cycle_counter(void);

/* Power Management Advanced */
int riscv64_power_advanced_init(void);
uint32_t riscv64_power_get_supported_states(void);

/* Virtualization Support */
int riscv64_virtualization_init(void);
bool riscv64_virtualization_is_supported(void);

/* MSVC Support */
#ifdef _MSC_VER
int riscv64_msvc_optimize(void);
void riscv64_msvc_intrinsics_init(void);
#endif

/* Advanced Features Integration */
int riscv64_advanced_features_init(void);
int riscv64_advanced_features_test(void);
void riscv64_advanced_features_cleanup(void);

#endif // ORION_ARCH_RISCV64_H
