/*
 * Orion Operating System - MIPS HAL Common Definitions
 *
 * Common constants, types, and definitions for MIPS HAL.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_HAL_MIPS_COMMON_H
#define ORION_HAL_MIPS_COMMON_H

#include <stdint.h>

// ============================================================================
// MIPS ARCHITECTURE IDENTIFIERS
// ============================================================================

#define ORION_ARCH_MIPS 0x00000008
#define ORION_ARCH_MIPS32 0x00000009
#define ORION_ARCH_MIPS64 0x0000000A

// ============================================================================
// MIPS EXTENSION IDENTIFIERS
// ============================================================================

#define ORION_MIPS_EXT_MIPS16 0x00000001
#define ORION_MIPS_EXT_MIPS32 0x00000002
#define ORION_MIPS_EXT_MIPS64 0x00000004
#define ORION_MIPS_EXT_MIPS3D 0x00000008
#define ORION_MIPS_EXT_MDMX 0x00000010
#define ORION_MIPS_EXT_DSP 0x00000020
#define ORION_MIPS_EXT_DSP2 0x00000040
#define ORION_MIPS_EXT_DSP3 0x00000080
#define ORION_MIPS_EXT_EVA 0x00000100
#define ORION_MIPS_EXT_MSA 0x00000200
#define ORION_MIPS_EXT_VZ 0x00000400
#define ORION_MIPS_EXT_CRC 0x00000800
#define ORION_MIPS_EXT_GINV 0x00001000
#define ORION_MIPS_EXT_LOONGSON 0x00002000

// ============================================================================
// MIPS PROCESSOR FAMILIES
// ============================================================================

#define ORION_MIPS_FAMILY_R3000 0x00000001
#define ORION_MIPS_FAMILY_R4000 0x00000002
#define ORION_MIPS_FAMILY_R5000 0x00000004
#define ORION_MIPS_FAMILY_R6000 0x00000008
#define ORION_MIPS_FAMILY_R8000 0x00000010
#define ORION_MIPS_FAMILY_R10000 0x00000020
#define ORION_MIPS_FAMILY_R12000 0x00000040
#define ORION_MIPS_FAMILY_R14000 0x00000080
#define ORION_MIPS_FAMILY_R16000 0x00000100
#define ORION_MIPS_FAMILY_5K 0x00000200
#define ORION_MIPS_FAMILY_20K 0x00000400
#define ORION_MIPS_FAMILY_24K 0x00000800
#define ORION_MIPS_FAMILY_25K 0x00001000
#define ORION_MIPS_FAMILY_34K 0x00002000
#define ORION_MIPS_FAMILY_74K 0x00004000
#define ORION_MIPS_FAMILY_1004K 0x00008000
#define ORION_MIPS_FAMILY_1074K 0x00010000
#define ORION_MIPS_FAMILY_4K 0x00020000
#define ORION_MIPS_FAMILY_5K 0x00040000
#define ORION_MIPS_FAMILY_6K 0x00080000
#define ORION_MIPS_FAMILY_7K 0x00100000
#define ORION_MIPS_FAMILY_8K 0x00200000
#define ORION_MIPS_FAMILY_9K 0x00400000
#define ORION_MIPS_FAMILY_10K 0x00800000
#define ORION_MIPS_FAMILY_11K 0x01000000
#define ORION_MIPS_FAMILY_12K 0x02000000
#define ORION_MIPS_FAMILY_14K 0x04000000
#define ORION_MIPS_FAMILY_16K 0x08000000
#define ORION_MIPS_FAMILY_18K 0x10000000
#define ORION_MIPS_FAMILY_20K 0x20000000
#define ORION_MIPS_FAMILY_22K 0x40000000
#define ORION_MIPS_FAMILY_24K 0x80000000

// ============================================================================
// MIPS FEATURE FLAGS
// ============================================================================

#define ORION_MIPS_FEATURE_MMU 0x00000001
#define ORION_MIPS_FEATURE_FPU 0x00000002
#define ORION_MIPS_FEATURE_CP2 0x00000004
#define ORION_MIPS_FEATURE_WATCH 0x00000008
#define ORION_MIPS_FEATURE_VCED 0x00000010
#define ORION_MIPS_FEATURE_VCEI 0x00000020
#define ORION_MIPS_FEATURE_MSA 0x00000040
#define ORION_MIPS_FEATURE_CRC 0x00000080
#define ORION_MIPS_FEATURE_GINV 0x00000100
#define ORION_MIPS_FEATURE_LOONGSON 0x00000200
#define ORION_MIPS_FEATURE_ULRI 0x00000400
#define ORION_MIPS_FEATURE_ULRI 0x00000800
#define ORION_MIPS_FEATURE_MICROMIPS 0x00001000
#define ORION_MIPS_FEATURE_VZ 0x00002000
#define ORION_MIPS_FEATURE_MT 0x00004000
#define ORION_MIPS_FEATURE_SMARTMIPS 0x00008000

// ============================================================================
// MIPS PAGE FLAGS
// ============================================================================

#define ORION_MIPS_PAGE_PRESENT 0x00000001
#define ORION_MIPS_PAGE_WRITABLE 0x00000002
#define ORION_MIPS_PAGE_USER 0x00000004
#define ORION_MIPS_PAGE_WRITETHROUGH 0x00000008
#define ORION_MIPS_PAGE_CACHED 0x00000010
#define ORION_MIPS_PAGE_ACCESSED 0x00000020
#define ORION_MIPS_PAGE_DIRTY 0x00000040
#define ORION_MIPS_PAGE_GLOBAL 0x00000080
#define ORION_MIPS_PAGE_EXECUTABLE 0x00000100
#define ORION_MIPS_PAGE_KERNEL 0x00000200
#define ORION_MIPS_PAGE_DEVICE 0x00000400
#define ORION_MIPS_PAGE_NOCACHE 0x00000800
#define ORION_MIPS_PAGE_WRITEBACK 0x00001000
#define ORION_MIPS_PAGE_COHERENT 0x00002000

// ============================================================================
// MIPS INTERRUPT NUMBERS
// ============================================================================

#define ORION_MIPS_IRQ_TIMER 0
#define ORION_MIPS_IRQ_KEYBOARD 1
#define ORION_MIPS_IRQ_SERIAL 2
#define ORION_MIPS_IRQ_DISK 3
#define ORION_MIPS_IRQ_NETWORK 4
#define ORION_MIPS_IRQ_USB 5
#define ORION_MIPS_IRQ_GPIO 6
#define ORION_MIPS_IRQ_DMA 7
#define ORION_MIPS_IRQ_PCI 8
#define ORION_MIPS_IRQ_ACPI 9
#define ORION_MIPS_IRQ_MAX 10

// ============================================================================
// MIPS EXCEPTION NUMBERS
// ============================================================================

#define ORION_MIPS_EXC_INT 0
#define ORION_MIPS_EXC_MOD 1
#define ORION_MIPS_EXC_TLBL 2
#define ORION_MIPS_EXC_TLBS 3
#define ORION_MIPS_EXC_ADEL 4
#define ORION_MIPS_EXC_ADES 5
#define ORION_MIPS_EXC_IBE 6
#define ORION_MIPS_EXC_DBE 7
#define ORION_MIPS_EXC_SYS 8
#define ORION_MIPS_EXC_BP 9
#define ORION_MIPS_EXC_RI 10
#define ORION_MIPS_EXC_CPU 11
#define ORION_MIPS_EXC_OV 12
#define ORION_MIPS_EXC_TR 13
#define ORION_MIPS_EXC_FPE 15
#define ORION_MIPS_EXC_MAX 16

// ============================================================================
// MIPS TIMER CONFIGURATION
// ============================================================================

#define ORION_MIPS_TIMER_FREQ_DEFAULT 1000000 // 1MHz default
#define ORION_MIPS_TIMER_MAX_INTERVAL 0xFFFFFFFFFFFFFFFFULL
#define ORION_MIPS_TIMER_MIN_INTERVAL 1000 // 1 microsecond minimum

// ============================================================================
// MIPS CACHE CONFIGURATION
// ============================================================================

#define ORION_MIPS_CACHE_LINE_SIZE 32
#define ORION_MIPS_CACHE_L1_SIZE 32768   // 32KB
#define ORION_MIPS_CACHE_L2_SIZE 262144  // 256KB
#define ORION_MIPS_CACHE_L3_SIZE 8388608 // 8MB

// ============================================================================
// MIPS MMU CONFIGURATION
// ============================================================================

#define ORION_MIPS_PAGE_SIZE_4K 4096
#define ORION_MIPS_PAGE_SIZE_16K 16384
#define ORION_MIPS_PAGE_SIZE_64K 65536
#define ORION_MIPS_PAGE_SIZE_1M 1048576
#define ORION_MIPS_PAGE_SIZE_16M 16777216
#define ORION_MIPS_PAGE_SIZE_256M 268435456
#define ORION_MIPS_PAGE_SIZE_1G 1073741824

#define ORION_MIPS_TLB_ENTRIES 64
#define ORION_MIPS_TLB_WIRED 8

// ============================================================================
// MIPS PERFORMANCE COUNTERS
// ============================================================================

#define ORION_MIPS_PMU_COUNTERS 4
#define ORION_MIPS_PMU_EVENTS 16

#define ORION_MIPS_PMU_EVENT_CYCLES 0
#define ORION_MIPS_PMU_EVENT_INSTRUCTIONS 1
#define ORION_MIPS_PMU_EVENT_CACHE_MISSES 2
#define ORION_MIPS_PMU_EVENT_BRANCH_MISSES 3
#define ORION_MIPS_PMU_EVENT_TLB_MISSES 4

// ============================================================================
// MIPS POWER STATES
// ============================================================================

#define ORION_MIPS_POWER_STATE_RUNNING 0
#define ORION_MIPS_POWER_STATE_IDLE 1
#define ORION_MIPS_POWER_STATE_SLEEP 2
#define ORION_MIPS_POWER_STATE_HIBERNATE 3
#define ORION_MIPS_POWER_STATE_OFF 4

// ============================================================================
// MIPS DEBUG TYPES
// ============================================================================

#define ORION_MIPS_DEBUG_BREAKPOINT_SOFTWARE 0
#define ORION_MIPS_DEBUG_BREAKPOINT_HARDWARE 1
#define ORION_MIPS_DEBUG_BREAKPOINT_DATA 2

#define ORION_MIPS_DEBUG_WATCHPOINT_READ 0
#define ORION_MIPS_DEBUG_WATCHPOINT_WRITE 1
#define ORION_MIPS_DEBUG_WATCHPOINT_ACCESS 2

// ============================================================================
// MIPS ERROR CODES
// ============================================================================

#define ORION_MIPS_SUCCESS 0
#define ORION_MIPS_ERROR_INVALID_ARGUMENT -1
#define ORION_MIPS_ERROR_NOT_IMPLEMENTED -2
#define ORION_MIPS_ERROR_OUT_OF_MEMORY -3
#define ORION_MIPS_ERROR_DEVICE_ERROR -4
#define ORION_MIPS_ERROR_TIMEOUT -5
#define ORION_MIPS_ERROR_BUSY -6
#define ORION_MIPS_ERROR_NOT_FOUND -7
#define ORION_MIPS_ERROR_ALREADY_EXISTS -8
#define ORION_MIPS_ERROR_PERMISSION_DENIED -9
#define ORION_MIPS_ERROR_INVALID_STATE -10

#endif // ORION_HAL_MIPS_COMMON_H
