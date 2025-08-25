/*
 * ORION OS - RISC-V 64-bit Hardware Abstraction Layer Header
 *
 * This file defines the hardware abstraction layer interface for RISC-V 64-bit
 * architecture, providing a unified interface for hardware operations.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_HAL_RISCV64_H
#define ORION_HAL_RISCV64_H

#include <stdint.h>
#include <stdbool.h>
#include "orion_hal.h"

// ============================================================================
// RISC-V SPECIFIC CONSTANTS
// ============================================================================

/* Page flags */
#define RISCV64_PAGE_READ (1ULL << 0)
#define RISCV64_PAGE_WRITE (1ULL << 1)
#define RISCV64_PAGE_EXECUTE (1ULL << 2)
#define RISCV64_PAGE_USER (1ULL << 3)
#define RISCV64_PAGE_GLOBAL (1ULL << 4)
#define RISCV64_PAGE_ACCESSED (1ULL << 5)
#define RISCV64_PAGE_DIRTY (1ULL << 6)

/* Page sizes */
#define RISCV64_PAGE_SIZE 4096ULL
#define RISCV64_HUGE_PAGE_SIZE (2ULL * 1024 * 1024)
#define RISCV64_GIGA_PAGE_SIZE (1ULL * 1024 * 1024 * 1024)

/* CPU features */
#define RISCV64_FEATURE_RV64I (1ULL << 0)
#define RISCV64_FEATURE_RV64M (1ULL << 1)
#define RISCV64_FEATURE_RV64A (1ULL << 2)
#define RISCV64_FEATURE_RV64F (1ULL << 3)
#define RISCV64_FEATURE_RV64D (1ULL << 4)
#define RISCV64_FEATURE_RV64C (1ULL << 5)
#define RISCV64_FEATURE_RVV (1ULL << 6)
#define RISCV64_FEATURE_RV64B (1ULL << 7)

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/* CPU features structure */
typedef struct
{
    uint64_t base_isa;
    uint64_t extensions;
    uint64_t vendor_id;
    uint64_t architecture_id;
    uint64_t implementation_id;
    uint64_t hart_id;
    uint64_t features;
} riscv64_cpu_features_t;

/* MMU context structure */
typedef struct
{
    uint64_t *root_page_table;
    uint64_t asid;
    uint64_t satp;
} riscv64_mmu_context_t;

/* Interrupt context structure */
typedef struct
{
    bool initialized;
    uint32_t irq_count;
    uint32_t enabled_irqs;
} riscv64_interrupt_context_t;

/* Timer context structure */
typedef struct
{
    bool initialized;
    uint64_t frequency;
    uint64_t current_value;
    uint64_t compare_value;
} riscv64_timer_context_t;

/* Cache context structure */
typedef struct
{
    bool initialized;
    uint32_t l1i_size;
    uint32_t l1d_size;
    uint32_t l2_size;
    uint32_t l3_size;
} riscv64_cache_context_t;

/* HAL context structure */
typedef struct
{
    riscv64_cpu_features_t cpu_features;
    riscv64_mmu_context_t mmu;
    riscv64_interrupt_context_t interrupts;
    riscv64_timer_context_t timer;
    riscv64_cache_context_t cache;
} riscv64_hal_context_t;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

/* HAL initialization and cleanup */
int riscv64_hal_init(void);
void riscv64_hal_cleanup(void);

/* CPU feature detection */
void riscv64_hal_detect_cpu_features(void);
bool riscv64_hal_has_feature(uint64_t feature);

/* Memory management */
int riscv64_hal_mmu_init(void);
void riscv64_hal_mmu_cleanup(void);
int riscv64_hal_mmu_map_page(uint64_t va, uint64_t pa, uint64_t flags);
int riscv64_hal_mmu_unmap_page(uint64_t va);
void riscv64_hal_mmu_invalidate_tlb(void);

/* Interrupt management */
int riscv64_hal_interrupt_init(void);
void riscv64_hal_interrupt_cleanup(void);
void riscv64_hal_interrupt_enable(uint32_t irq);
void riscv64_hal_interrupt_disable(uint32_t irq);

/* Timer management */
int riscv64_hal_timer_init(void);
void riscv64_hal_timer_cleanup(void);
uint64_t riscv64_hal_timer_read_ns(void);
int riscv64_hal_timer_set_oneshot(uint64_t deadline_ns);

/* Cache management */
int riscv64_hal_cache_init(void);
void riscv64_hal_cache_cleanup(void);
void riscv64_hal_cache_invalidate_all(void);
void riscv64_hal_cache_clean_all(void);

/* Utility functions */
void riscv64_hal_print_info(void);
const riscv64_hal_context_t *riscv64_hal_get_context(void);
const hal_ops_t *riscv64_hal_get_ops(void);

#endif // ORION_HAL_RISCV64_H
