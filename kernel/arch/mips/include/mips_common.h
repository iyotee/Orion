/*
 * ORION OS - MIPS Common Definitions
 *
 * Common definitions and types for MIPS architecture
 * Shared across all MIPS-related files
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_MIPS_COMMON_H
#define ORION_MIPS_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// COMMON TYPES
// ============================================================================

typedef uint64_t mips_reg_t;
typedef uint64_t mips_addr_t;
typedef uint64_t mips_size_t;
typedef uint32_t mips_irq_t;
typedef uint32_t mips_exc_t;
typedef uint32_t mips_asid_t;
typedef uint32_t mips_flags_t;

// ============================================================================
// COMMON CONSTANTS
// ============================================================================

#define MIPS_INVALID_ADDRESS ((mips_addr_t)0xFFFFFFFFFFFFFFFFULL)
#define MIPS_INVALID_IRQ ((mips_irq_t)0xFFFFFFFF)
#define MIPS_INVALID_EXC ((mips_exc_t)0xFFFFFFFF)

// ============================================================================
// COMMON STRUCTURES
// ============================================================================

typedef struct
{
    mips_addr_t vaddr;
    mips_addr_t paddr;
    mips_size_t size;
    mips_flags_t flags;
    mips_asid_t asid;
} mips_memory_region_t;

typedef struct
{
    mips_addr_t vaddr;
    mips_addr_t paddr;
    mips_flags_t flags;
    mips_asid_t asid;
} mips_tlb_entry_t;

typedef struct
{
    mips_memory_region_t regions[MIPS_MAX_MEMORY_REGIONS];
    mips_addr_t base_address;
    mips_size_t total_size;
} mips_page_table_t;

typedef struct
{
    mips_interrupt_handler_t interrupt_vectors[MIPS_MAX_IRQS];
    mips_exception_handler_t exception_vectors[MIPS_MAX_EXCEPTIONS];
    mips_syscall_handler_t syscall_vectors[MIPS_MAX_SYSCALLS];

    // Special vectors
    mips_interrupt_handler_t timer_interrupt;
    mips_interrupt_handler_t periodic_timer_interrupt;
    mips_interrupt_handler_t high_resolution_timer_interrupt;

    // Device vectors
    mips_interrupt_handler_t uart_interrupt;
    mips_interrupt_handler_t spi_interrupt;
    mips_interrupt_handler_t i2c_interrupt;
    mips_interrupt_handler_t gpio_interrupt;
    mips_interrupt_handler_t dma_interrupt;
    mips_interrupt_handler_t ethernet_interrupt;
    mips_interrupt_handler_t usb_interrupt;
    mips_interrupt_handler_t pci_interrupt;

    // Error vectors
    mips_interrupt_handler_t memory_error_interrupt;
    mips_interrupt_handler_t bus_error_interrupt;
    mips_interrupt_handler_t cache_error_interrupt;
    mips_interrupt_handler_t tlb_error_interrupt;
} mips_vector_table_t;

typedef struct
{
    mips_cpu_mode_t current_mode;
    bool interrupts_enabled;
    uint32_t exception_level;
    mips_addr_t stack_pointer;
    mips_addr_t frame_pointer;
} mips_cpu_state_t;

typedef struct
{
    uint32_t vendor_id;
    uint32_t cpu_id;
    uint32_t revision;
    bool is_64bit;
    bool has_fpu;
    bool has_dsp;
    bool has_msa;
    bool has_vz;
    bool has_crc;
    bool has_ginv;
    bool has_neon;
    bool has_vfp;
    bool has_crypto;
    bool has_simd;
    uint32_t icache_size;
    uint32_t dcache_size;
} mips_cpu_info_t;

typedef struct
{
    mips_tlb_entry_t entries[MIPS_TLB_ENTRIES];
    uint32_t current_index;
    uint32_t wired_count;
    bool initialized;
} mips_tlb_context_t;

typedef struct
{
    mips_irq_t irq;
    mips_interrupt_handler_t handler;
    mips_irq_priority_t priority;
    bool enabled;
    uint32_t count;
} mips_interrupt_handler_entry_t;

typedef struct
{
    mips_interrupt_handler_entry_t handlers[MIPS_MAX_IRQS];
    uint32_t active_irqs;
    bool initialized;
} mips_interrupt_controller_t;

typedef struct
{
    mips_timer_mode_t mode;
    mips_addr_t base_address;
    uint64_t frequency;
    uint64_t period;
    uint64_t current_value;
    bool enabled;
    bool periodic;
} mips_timer_context_t;

typedef struct
{
    uint32_t level;
    uint32_t way;
    uint32_t line_size;
    uint32_t total_size;
    bool enabled;
    bool write_back;
    bool write_allocate;
} mips_cache_context_t;

typedef struct
{
    mips_power_state_t power_state;
    uint64_t cpu_frequency;
    uint64_t voltage;
    uint64_t temperature;
    bool thermal_protection;
} mips_power_context_t;

typedef struct
{
    bool secure_mode;
    bool virtualization_enabled;
    bool debug_enabled;
    uint32_t security_level;
} mips_security_context_t;

typedef struct
{
    bool virtualization_enabled;
    uint32_t guest_id;
    uint32_t host_id;
    bool nested_virtualization;
} mips_virtualization_context_t;

typedef struct
{
    bool performance_monitoring_enabled;
    uint32_t performance_counters;
    uint64_t cycle_count;
    uint64_t instruction_count;
} mips_performance_context_t;

typedef struct
{
    bool debug_enabled;
    uint32_t breakpoints;
    uint32_t watchpoints;
    bool single_step;
} mips_debug_context_t;

typedef struct
{
    bool extensions_enabled;
    uint32_t supported_extensions;
    uint32_t active_extensions;
} mips_extension_context_t;

// ============================================================================
// COMMON ENUMS
// ============================================================================

typedef enum
{
    MIPS_MODE_KERNEL = 0,
    MIPS_MODE_SUPERVISOR = 1,
    MIPS_MODE_USER = 2
} mips_cpu_mode_t;

typedef enum
{
    MIPS_POWER_STATE_ACTIVE = 0,
    MIPS_POWER_STATE_IDLE = 1,
    MIPS_POWER_STATE_SLEEP = 2,
    MIPS_POWER_STATE_HIBERNATE = 3
} mips_power_state_t;

typedef enum
{
    MIPS_TIMER_MODE_ONE_SHOT = 0,
    MIPS_TIMER_MODE_PERIODIC = 1,
    MIPS_TIMER_MODE_CONTINUOUS = 2
} mips_timer_mode_t;

typedef enum
{
    MIPS_IRQ_PRIORITY_LOW = 0,
    MIPS_IRQ_PRIORITY_NORMAL = 1,
    MIPS_IRQ_PRIORITY_HIGH = 2,
    MIPS_IRQ_PRIORITY_CRITICAL = 3
} mips_irq_priority_t;

typedef enum
{
    MIPS_FEATURE_64BIT = 0,
    MIPS_FEATURE_FPU = 1,
    MIPS_FEATURE_DSP = 2,
    MIPS_FEATURE_MSA = 3,
    MIPS_FEATURE_VZ = 4,
    MIPS_FEATURE_CRC = 5,
    MIPS_FEATURE_GINV = 6,
    MIPS_FEATURE_PERFORMANCE = 7,
    MIPS_FEATURE_DEBUG = 8
} mips_cpu_feature_t;

typedef enum
{
    MIPS_EXTENSION_NEON = 0,
    MIPS_EXTENSION_VFP = 1,
    MIPS_EXTENSION_CRYPTO = 2,
    MIPS_EXTENSION_SIMD = 3
} mips_extension_t;

typedef enum
{
    MIPS_ACCESS_READ = 0x01,
    MIPS_ACCESS_WRITE = 0x02,
    MIPS_ACCESS_EXECUTE = 0x04
} mips_access_flags_t;

// ============================================================================
// COMMON FUNCTION TYPES
// ============================================================================

typedef void (*mips_interrupt_handler_t)(void);
typedef void (*mips_exception_handler_t)(void);
typedef void (*mips_syscall_handler_t)(void);
typedef void (*mips_timer_callback_t)(void *user_data);

// ============================================================================
// COMMON MACROS
// ============================================================================

#define MIPS_ALIGN_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))
#define MIPS_ALIGN_DOWN(x, align) ((x) & ~((align) - 1))
#define MIPS_IS_ALIGNED(x, align) (((x) & ((align) - 1)) == 0)

#define MIPS_BIT(n) (1ULL << (n))
#define MIPS_BIT_MASK(n) (MIPS_BIT(n) - 1)
#define MIPS_GET_BITS(x, start, end) (((x) >> (start)) & MIPS_BIT_MASK((end) - (start) + 1))
#define MIPS_SET_BITS(x, start, end, value)                           \
    ((x) = ((x) & ~(MIPS_BIT_MASK((end) - (start) + 1) << (start))) | \
           (((value) & MIPS_BIT_MASK((end) - (start) + 1)) << (start)))

#define MIPS_MIN(a, b) ((a) < (b) ? (a) : (b))
#define MIPS_MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIPS_CLAMP(x, min, max) (MIPS_MIN(MIPS_MAX((x), (min)), (max)))

// ============================================================================
// COMMON INLINE FUNCTIONS
// ============================================================================

static inline bool mips_is_power_of_2(uint64_t x)
{
    return x != 0 && (x & (x - 1)) == 0;
}

static inline uint64_t mips_next_power_of_2(uint64_t x)
{
    if (x == 0)
        return 1;
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return x + 1;
}

static inline uint64_t mips_log2(uint64_t x)
{
    if (x == 0)
        return 0;
    uint64_t result = 0;
    while (x > 1)
    {
        x >>= 1;
        result++;
    }
    return result;
}

static inline bool mips_is_aligned(mips_addr_t addr, mips_size_t alignment)
{
    return MIPS_IS_ALIGNED(addr, alignment);
}

static inline mips_addr_t mips_align_up(mips_addr_t addr, mips_size_t alignment)
{
    return MIPS_ALIGN_UP(addr, alignment);
}

static inline mips_addr_t mips_align_down(mips_addr_t addr, mips_size_t alignment)
{
    return MIPS_ALIGN_DOWN(addr, alignment);
}

#endif // ORION_MIPS_COMMON_H
