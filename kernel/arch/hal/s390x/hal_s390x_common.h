/*
 * Orion Operating System - s390x HAL Common Definitions
 *
 * Common constants, types, and definitions for s390x HAL.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_HAL_S390X_COMMON_H
#define ORION_HAL_S390X_COMMON_H

#include <stdint.h>

// ============================================================================
// S390X ARCHITECTURE IDENTIFIERS
// ============================================================================

#define ORION_ARCH_S390X 0x00000006
#define ORION_ARCH_S390X_64 0x00000007

// ============================================================================
// S390X EXTENSION IDENTIFIERS
// ============================================================================

#define ORION_EXTENSION_Z196 0x00002000
#define ORION_EXTENSION_ZEC12 0x00002001
#define ORION_EXTENSION_Z13 0x00002002
#define ORION_EXTENSION_Z14 0x00002003
#define ORION_EXTENSION_Z15 0x00002004
#define ORION_EXTENSION_Z16 0x00002005
#define ORION_EXTENSION_VECTOR 0x00002006
#define ORION_EXTENSION_CRYPTO 0x00002007
#define ORION_EXTENSION_DECIMAL 0x00002008
#define ORION_EXTENSION_TX 0x00002009

// ============================================================================
// S390X PAGE FLAGS
// ============================================================================

#define S390X_PAGE_READ 0x00000001
#define S390X_PAGE_WRITE 0x00000002
#define S390X_PAGE_EXEC 0x00000004
#define S390X_PAGE_USER 0x00000008
#define S390X_PAGE_GUARDED 0x00000010
#define S390X_PAGE_COHERENT 0x00000020
#define S390X_PAGE_WRITETHROUGH 0x00000040
#define S390X_PAGE_CACHEINHIBIT 0x00000080
#define S390X_PAGE_4K 0x00000100
#define S390X_PAGE_1M 0x00000200
#define S390X_PAGE_2G 0x00000400

// ============================================================================
// S390X INTERRUPT NUMBERS
// ============================================================================

#define S390X_IRQ_EXTERNAL 0
#define S390X_IRQ_IO 1
#define S390X_IRQ_SERVICE 2
#define S390X_IRQ_RESTART 3
#define S390X_IRQ_EMERGENCY 4
#define S390X_IRQ_MACHINE_CHECK 5
#define S390X_IRQ_TIMER 6
#define S390X_IRQ_CPU_TIMER 7
#define S390X_IRQ_CPU_MEASUREMENT 8
#define S390X_IRQ_CPU_DIAGNOSTIC 9
#define S390X_IRQ_VIRTUAL 10
#define S390X_IRQ_CRYPTO 11

// ============================================================================
// S390X TIMER CONSTANTS
// ============================================================================

#define S390X_TIMER_FREQ_DEFAULT 4194304 // 4.194304 MHz default
#define S390X_TIMER_MAX_FREQ 16777216    // 16.777216 MHz max
#define S390X_TIMER_MIN_FREQ 1048576     // 1.048576 MHz min

// ============================================================================
// S390X CACHE CONSTANTS
// ============================================================================

#define S390X_CACHE_LINE_SIZE 256               // 256 bytes typical
#define S390X_CACHE_L1_SIZE (128 * 1024)        // 128KB typical
#define S390X_CACHE_L2_SIZE (4 * 1024 * 1024)   // 4MB typical
#define S390X_CACHE_L3_SIZE (128 * 1024 * 1024) // 128MB typical

// ============================================================================
// S390X PERFORMANCE COUNTERS
// ============================================================================

#define S390X_PMU_MAX_COUNTERS 16
#define S390X_PMU_CYCLE_COUNTER 0
#define S390X_PMU_INST_COUNTER 1
#define S390X_PMU_L1_DCACHE_MISS 2
#define S390X_PMU_L1_ICACHE_MISS 3
#define S390X_PMU_L2_CACHE_MISS 4
#define S390X_PMU_L3_CACHE_MISS 5
#define S390X_PMU_BRANCH_MISS 6
#define S390X_PMU_FP_OPERATIONS 7
#define S390X_PMU_VECTOR_OPERATIONS 8
#define S390X_PMU_CRYPTO_OPERATIONS 9
#define S390X_PMU_DECIMAL_OPERATIONS 10

// ============================================================================
// S390X POWER STATES
// ============================================================================

#define S390X_STATE_ACTIVE 0
#define S390X_STATE_IDLE 1
#define S390X_STATE_SLEEP 2
#define S390X_STATE_HIBERNATE 3
#define S390X_STATE_OFF 4
#define S390X_STATE_ENTERPRISE 5

// ============================================================================
// S390X SECURITY FEATURES
// ============================================================================

#define S390X_SECURITY_CRYPTO 0x00000001
#define S390X_SECURITY_TPM 0x00000002
#define S390X_SECURITY_CCA 0x00000004
#define S390X_SECURITY_EP11 0x00000008
#define S390X_SECURITY_PAES 0x00000010
#define S390X_SECURITY_KMAC 0x00000020

// ============================================================================
// S390X DEBUG FEATURES
// ============================================================================

#define S390X_DEBUG_BREAKPOINT 0x00000001
#define S390X_DEBUG_WATCHPOINT 0x00000002
#define S390X_DEBUG_TRACE 0x00000004
#define S390X_DEBUG_PROFILING 0x00000008
#define S390X_DEBUG_MEASUREMENT 0x00000010

// ============================================================================
// S390X ERROR CODES
// ============================================================================

#define S390X_SUCCESS 0
#define S390X_ERROR_INVALID_ARG -1
#define S390X_ERROR_NOT_SUPPORTED -2
#define S390X_ERROR_OUT_OF_MEMORY -3
#define S390X_ERROR_TIMEOUT -4
#define S390X_ERROR_HARDWARE -5
#define S390X_ERROR_SECURITY -6
#define S390X_ERROR_MAINFRAME -7

// ============================================================================
// S390X DATA TYPES
// ============================================================================

typedef uint64_t s390x_vaddr_t;
typedef uint64_t s390x_paddr_t;
typedef uint64_t s390x_size_t;
typedef uint64_t s390x_off_t;
typedef uint32_t s390x_irq_t;
typedef uint32_t s390x_timer_t;
typedef uint32_t s390x_counter_t;

// ============================================================================
// S390X FUNCTION TYPES
// ============================================================================

typedef void (*s390x_irq_handler_t)(uint32_t irq, void *data);
typedef void (*s390x_timer_callback_t)(void *data);
typedef int (*s390x_extension_handler_t)(void *args);

// ============================================================================
// S390X VECTOR FACILITY
// ============================================================================

#define S390X_VECTOR_LENGTH 16    // 16 bytes per vector
#define S390X_VECTOR_REGISTERS 32 // 32 vector registers
#define S390X_VECTOR_CRYPTO 0x00000001
#define S390X_VECTOR_DECIMAL 0x00000002
#define S390X_VECTOR_STRING 0x00000004

// ============================================================================
// S390X CRYPTO FACILITY
// ============================================================================

#define S390X_CRYPTO_AES 0x00000001
#define S390X_CRYPTO_SHA 0x00000002
#define S390X_CRYPTO_RSA 0x00000004
#define S390X_CRYPTO_ECC 0x00000008
#define S390X_CRYPTO_QUANTUM 0x00000010

#endif /* ORION_HAL_S390X_COMMON_H */
