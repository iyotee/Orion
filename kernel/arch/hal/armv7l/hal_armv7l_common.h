/*
 * Orion Operating System - ARMv7l HAL Common Definitions
 *
 * Common constants, types, and definitions for ARMv7l HAL.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_HAL_ARMV7L_COMMON_H
#define ORION_HAL_ARMV7L_COMMON_H

#include <stdint.h>

// ============================================================================
// ARMV7L ARCHITECTURE IDENTIFIERS
// ============================================================================

#define ORION_ARCH_ARMV7L 0x0000000B
#define ORION_ARCH_ARM32 0x0000000C

// ============================================================================
// ARMV7L EXTENSION IDENTIFIERS
// ============================================================================

#define ORION_ARMV7L_EXT_NEON 0x00000001
#define ORION_ARMV7L_EXT_VFP 0x00000002
#define ORION_ARMV7L_EXT_VFPV3 0x00000004
#define ORION_ARMV7L_EXT_VFPV4 0x00000008
#define ORION_ARMV7L_EXT_ARM 0x00000010
#define ORION_ARMV7L_EXT_THUMB 0x00000020
#define ORION_ARMV7L_EXT_THUMB2 0x00000040
#define ORION_ARMV7L_EXT_JAZELLE 0x00000080
#define ORION_ARMV7L_EXT_DSP 0x00000100
#define ORION_ARMV7L_EXT_SIMD 0x00000200
#define ORION_ARMV7L_EXT_CRYPTO 0x00000400
#define ORION_ARMV7L_EXT_TRUSTZONE 0x00000800
#define ORION_ARMV7L_EXT_VIRTUALIZATION 0x00001000
#define ORION_ARMV7L_EXT_LPAE 0x00002000
#define ORION_ARMV7L_EXT_BIGEND 0x00004000
#define ORION_ARMV7L_EXT_SMP 0x00008000

// ============================================================================
// ARMV7L PROCESSOR FAMILIES
// ============================================================================

#define ORION_ARMV7L_FAMILY_CORTEX_A5 0x00000001
#define ORION_ARMV7L_FAMILY_CORTEX_A7 0x00000002
#define ORION_ARMV7L_FAMILY_CORTEX_A8 0x00000004
#define ORION_ARMV7L_FAMILY_CORTEX_A9 0x00000008
#define ORION_ARMV7L_FAMILY_CORTEX_A12 0x00000010
#define ORION_ARMV7L_FAMILY_CORTEX_A15 0x00000020
#define ORION_ARMV7L_FAMILY_CORTEX_A17 0x00000040
#define ORION_ARMV7L_FAMILY_CORTEX_R4 0x00000080
#define ORION_ARMV7L_FAMILY_CORTEX_R5 0x00000100
#define ORION_ARMV7L_FAMILY_CORTEX_R7 0x00000200
#define ORION_ARMV7L_FAMILY_CORTEX_M3 0x00000400
#define ORION_ARMV7L_FAMILY_CORTEX_M4 0x00000800
#define ORION_ARMV7L_FAMILY_CORTEX_M7 0x00001000

// ============================================================================
// ARMV7L FEATURE FLAGS
// ============================================================================

#define ORION_ARMV7L_FEATURE_MMU 0x00000001
#define ORION_ARMV7L_FEATURE_FPU 0x00000002
#define ORION_ARMV7L_FEATURE_NEON 0x00000004
#define ORION_ARMV7L_FEATURE_VFP 0x00000008
#define ORION_ARMV7L_FEATURE_TRUSTZONE 0x00000010
#define ORION_ARMV7L_FEATURE_VIRTUALIZATION 0x00000020
#define ORION_ARMV7L_FEATURE_LPAE 0x00000040
#define ORION_ARMV7L_FEATURE_SMP 0x00000080
#define ORION_ARMV7L_FEATURE_CRYPTO 0x00000100
#define ORION_ARMV7L_FEATURE_DSP 0x00000200
#define ORION_ARMV7L_FEATURE_SIMD 0x00000400
#define ORION_ARMV7L_FEATURE_JAZELLE 0x00000800
#define ORION_ARMV7L_FEATURE_THUMB 0x00001000
#define ORION_ARMV7L_FEATURE_THUMB2 0x00002000
#define ORION_ARMV7L_FEATURE_ARM 0x00004000
#define ORION_ARMV7L_FEATURE_BIGEND 0x00008000

// ============================================================================
// ARMV7L PAGE FLAGS
// ============================================================================

#define ORION_ARMV7L_PAGE_PRESENT 0x00000001
#define ORION_ARMV7L_PAGE_WRITABLE 0x00000002
#define ORION_ARMV7L_PAGE_USER 0x00000004
#define ORION_ARMV7L_PAGE_WRITETHROUGH 0x00000008
#define ORION_ARMV7L_PAGE_CACHED 0x00000010
#define ORION_ARMV7L_PAGE_ACCESSED 0x00000020
#define ORION_ARMV7L_PAGE_DIRTY 0x00000040
#define ORION_ARMV7L_PAGE_GLOBAL 0x00000080
#define ORION_ARMV7L_PAGE_EXECUTABLE 0x00000100
#define ORION_ARMV7L_PAGE_KERNEL 0x00000200
#define ORION_ARMV7L_PAGE_DEVICE 0x00000400
#define ORION_ARMV7L_PAGE_NOCACHE 0x00000800
#define ORION_ARMV7L_PAGE_WRITEBACK 0x00001000
#define ORION_ARMV7L_PAGE_COHERENT 0x00002000
#define ORION_ARMV7L_PAGE_SHARED 0x00004000
#define ORION_ARMV7L_PAGE_NONSHARED 0x00008000

// ============================================================================
// ARMV7L INTERRUPT NUMBERS
// ============================================================================

#define ORION_ARMV7L_IRQ_TIMER 0
#define ORION_ARMV7L_IRQ_KEYBOARD 1
#define ORION_ARMV7L_IRQ_SERIAL 2
#define ORION_ARMV7L_IRQ_DISK 3
#define ORION_ARMV7L_IRQ_NETWORK 4
#define ORION_ARMV7L_IRQ_USB 5
#define ORION_ARMV7L_IRQ_GPIO 6
#define ORION_ARMV7L_IRQ_DMA 7
#define ORION_ARMV7L_IRQ_PCI 8
#define ORION_ARMV7L_IRQ_ACPI 9
#define ORION_ARMV7L_IRQ_MAX 10

// ============================================================================
// ARMV7L EXCEPTION NUMBERS
// ============================================================================

#define ORION_ARMV7L_EXC_RESET 0
#define ORION_ARMV7L_EXC_UNDEFINED 1
#define ORION_ARMV7L_EXC_SWI 2
#define ORION_ARMV7L_EXC_PREFETCH 3
#define ORION_ARMV7L_EXC_DATA 4
#define ORION_ARMV7L_EXC_RESERVED 5
#define ORION_ARMV7L_EXC_IRQ 6
#define ORION_ARMV7L_EXC_FIQ 7
#define ORION_ARMV7L_EXC_MAX 8

// ============================================================================
// ARMV7L TIMER CONFIGURATION
// ============================================================================

#define ORION_ARMV7L_TIMER_FREQ_DEFAULT 1000000 // 1MHz default
#define ORION_ARMV7L_TIMER_MAX_INTERVAL 0xFFFFFFFFFFFFFFFFULL
#define ORION_ARMV7L_TIMER_MIN_INTERVAL 1000 // 1 microsecond minimum

// ============================================================================
// ARMV7L CACHE CONFIGURATION
// ============================================================================

#define ORION_ARMV7L_CACHE_LINE_SIZE 32
#define ORION_ARMV7L_CACHE_L1_SIZE 32768   // 32KB
#define ORION_ARMV7L_CACHE_L2_SIZE 262144  // 256KB
#define ORION_ARMV7L_CACHE_L3_SIZE 8388608 // 8MB

// ============================================================================
// ARMV7L MMU CONFIGURATION
// ============================================================================

#define ORION_ARMV7L_PAGE_SIZE_4K 4096
#define ORION_ARMV7L_PAGE_SIZE_64K 65536
#define ORION_ARMV7L_PAGE_SIZE_1M 1048576
#define ORION_ARMV7L_PAGE_SIZE_16M 16777216
#define ORION_ARMV7L_PAGE_SIZE_1G 1073741824

#define ORION_ARMV7L_TLB_ENTRIES 64
#define ORION_ARMV7L_TLB_WIRED 8

// ============================================================================
// ARMV7L PERFORMANCE COUNTERS
// ============================================================================

#define ORION_ARMV7L_PMU_COUNTERS 4
#define ORION_ARMV7L_PMU_EVENTS 16

#define ORION_ARMV7L_PMU_EVENT_CYCLES 0
#define ORION_ARMV7L_PMU_EVENT_INSTRUCTIONS 1
#define ORION_ARMV7L_PMU_EVENT_CACHE_MISSES 2
#define ORION_ARMV7L_PMU_EVENT_BRANCH_MISSES 3
#define ORION_ARMV7L_PMU_EVENT_TLB_MISSES 4

// ============================================================================
// ARMV7L POWER STATES
// ============================================================================

#define ORION_ARMV7L_POWER_STATE_RUNNING 0
#define ORION_ARMV7L_POWER_STATE_IDLE 1
#define ORION_ARMV7L_POWER_STATE_SLEEP 2
#define ORION_ARMV7L_POWER_STATE_HIBERNATE 3
#define ORION_ARMV7L_POWER_STATE_OFF 4

// ============================================================================
// ARMV7L DEBUG TYPES
// ============================================================================

#define ORION_ARMV7L_DEBUG_BREAKPOINT_SOFTWARE 0
#define ORION_ARMV7L_DEBUG_BREAKPOINT_HARDWARE 1
#define ORION_ARMV7L_DEBUG_BREAKPOINT_DATA 2

#define ORION_ARMV7L_DEBUG_WATCHPOINT_READ 0
#define ORION_ARMV7L_DEBUG_WATCHPOINT_WRITE 1
#define ORION_ARMV7L_DEBUG_WATCHPOINT_ACCESS 2

// ============================================================================
// ARMV7L ERROR CODES
// ============================================================================

#define ORION_ARMV7L_SUCCESS 0
#define ORION_ARMV7L_ERROR_INVALID_ARGUMENT -1
#define ORION_ARMV7L_ERROR_NOT_IMPLEMENTED -2
#define ORION_ARMV7L_ERROR_OUT_OF_MEMORY -3
#define ORION_ARMV7L_ERROR_DEVICE_ERROR -4
#define ORION_ARMV7L_ERROR_TIMEOUT -5
#define ORION_ARMV7L_ERROR_BUSY -6
#define ORION_ARMV7L_ERROR_NOT_FOUND -7
#define ORION_ARMV7L_ERROR_ALREADY_EXISTS -8
#define ORION_ARMV7L_ERROR_PERMISSION_DENIED -9
#define ORION_ARMV7L_ERROR_INVALID_STATE -10

#endif // ORION_HAL_ARMV7L_COMMON_H
