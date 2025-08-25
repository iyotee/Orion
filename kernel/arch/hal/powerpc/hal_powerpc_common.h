/*
 * Orion Operating System - PowerPC HAL Common Definitions
 *
 * Common constants, types, and definitions for PowerPC HAL.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_HAL_POWERPC_COMMON_H
#define ORION_HAL_POWERPC_COMMON_H

#include <stdint.h>

// ============================================================================
// POWERPC ARCHITECTURE IDENTIFIERS
// ============================================================================

#define ORION_ARCH_POWERPC 0x00000004
#define ORION_ARCH_POWERPC_64 0x00000005

// ============================================================================
// POWERPC EXTENSION IDENTIFIERS
// ============================================================================

#define ORION_EXTENSION_VSX 0x00001000
#define ORION_EXTENSION_ALTIVEC 0x00001001
#define ORION_EXTENSION_POWER8 0x00001002
#define ORION_EXTENSION_POWER9 0x00001003
#define ORION_EXTENSION_POWER10 0x00001004
#define ORION_EXTENSION_POWER11 0x00001005

// ============================================================================
// POWERPC PAGE FLAGS
// ============================================================================

#define POWER_PAGE_READ 0x00000001
#define POWER_PAGE_WRITE 0x00000002
#define POWER_PAGE_EXEC 0x00000004
#define POWER_PAGE_USER 0x00000008
#define POWER_PAGE_GUARDED 0x00000010
#define POWER_PAGE_COHERENT 0x00000020
#define POWER_PAGE_WRITETHROUGH 0x00000040
#define POWER_PAGE_CACHEINHIBIT 0x00000080

// ============================================================================
// POWERPC INTERRUPT NUMBERS
// ============================================================================

#define POWER_IRQ_SYSTEM_RESET 0
#define POWER_IRQ_MACHINE_CHECK 1
#define POWER_IRQ_DATA_STORAGE 2
#define POWER_IRQ_INSTRUCTION_STORAGE 3
#define POWER_IRQ_EXTERNAL 4
#define POWER_IRQ_ALIGNMENT 5
#define POWER_IRQ_PROGRAM 6
#define POWER_IRQ_FP_UNAVAILABLE 7
#define POWER_IRQ_DECREMENTER 8
#define POWER_IRQ_SYSTEM_CALL 9
#define POWER_IRQ_TRACE 10
#define POWER_IRQ_PERFORMANCE 11
#define POWER_IRQ_VIRTUALIZATION 12
#define POWER_IRQ_DATA_SEGMENT 13
#define POWER_IRQ_INST_SEGMENT 14
#define POWER_IRQ_FP_ASSIST 15

// ============================================================================
// POWERPC TIMER CONSTANTS
// ============================================================================

#define POWER_TIMER_FREQ_DEFAULT 1000000 // 1MHz default
#define POWER_TIMER_MAX_FREQ 10000000    // 10MHz max
#define POWER_TIMER_MIN_FREQ 1000        // 1kHz min

// ============================================================================
// POWERPC CACHE CONSTANTS
// ============================================================================

#define POWER_CACHE_LINE_SIZE 128             // 128 bytes typical
#define POWER_CACHE_L1_SIZE (32 * 1024)       // 32KB typical
#define POWER_CACHE_L2_SIZE (256 * 1024)      // 256KB typical
#define POWER_CACHE_L3_SIZE (8 * 1024 * 1024) // 8MB typical

// ============================================================================
// POWERPC PERFORMANCE COUNTERS
// ============================================================================

#define POWER_PMU_MAX_COUNTERS 8
#define POWER_PMU_CYCLE_COUNTER 0
#define POWER_PMU_INST_COUNTER 1
#define POWER_PMU_L1_DCACHE_MISS 2
#define POWER_PMU_L1_ICACHE_MISS 3
#define POWER_PMU_L2_CACHE_MISS 4
#define POWER_PMU_BRANCH_MISS 5
#define POWER_PMU_FP_OPERATIONS 6
#define POWER_PMU_VECTOR_OPERATIONS 7

// ============================================================================
// POWERPC POWER STATES
// ============================================================================

#define POWER_STATE_ACTIVE 0
#define POWER_STATE_IDLE 1
#define POWER_STATE_SLEEP 2
#define POWER_STATE_HIBERNATE 3
#define POWER_STATE_OFF 4

// ============================================================================
// POWERPC SECURITY FEATURES
// ============================================================================

#define POWER_SECURITY_AMR 0x00000001
#define POWER_SECURITY_UAMOR 0x00000002
#define POWER_SECURITY_DAWR 0x00000004
#define POWER_SECURITY_CIABR 0x00000008
#define POWER_SECURITY_PPR 0x00000010

// ============================================================================
// POWERPC DEBUG FEATURES
// ============================================================================

#define POWER_DEBUG_BREAKPOINT 0x00000001
#define POWER_DEBUG_WATCHPOINT 0x00000002
#define POWER_DEBUG_TRACE 0x00000004
#define POWER_DEBUG_PROFILING 0x00000008

// ============================================================================
// POWERPC ERROR CODES
// ============================================================================

#define POWER_SUCCESS 0
#define POWER_ERROR_INVALID_ARG -1
#define POWER_ERROR_NOT_SUPPORTED -2
#define POWER_ERROR_OUT_OF_MEMORY -3
#define POWER_ERROR_TIMEOUT -4
#define POWER_ERROR_HARDWARE -5
#define POWER_ERROR_SECURITY -6

// ============================================================================
// POWERPC DATA TYPES
// ============================================================================

typedef uint64_t power_vaddr_t;
typedef uint64_t power_paddr_t;
typedef uint64_t power_size_t;
typedef uint64_t power_off_t;
typedef uint32_t power_irq_t;
typedef uint32_t power_timer_t;
typedef uint32_t power_counter_t;

// ============================================================================
// POWERPC FUNCTION TYPES
// ============================================================================

typedef void (*power_irq_handler_t)(uint32_t irq, void *data);
typedef void (*power_timer_callback_t)(void *data);
typedef int (*power_extension_handler_t)(void *args);

#endif /* ORION_HAL_POWERPC_COMMON_H */
