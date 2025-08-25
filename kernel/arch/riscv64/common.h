/*
 * ORION OS - RISC-V 64-bit Common Definitions
 *
 * Common definitions and types for RISC-V 64-bit architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_RISCV64_COMMON_H
#define ORION_RISCV64_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// COMMON TYPES
// ============================================================================

// Basic types
typedef uint64_t riscv64_reg_t;
typedef uint64_t riscv64_addr_t;
typedef uint64_t riscv64_size_t;
typedef uint32_t riscv64_irq_t;
typedef uint32_t riscv64_exception_t;
typedef uint32_t riscv64_syscall_t;
typedef uint32_t riscv64_process_id_t;
typedef uint32_t riscv64_thread_id_t;

// Invalid values
#define RISCV64_INVALID_ADDRESS 0xFFFFFFFFFFFFFFFFULL
#define RISCV64_INVALID_PROCESS_ID 0xFFFFFFFF
#define RISCV64_INVALID_THREAD_ID 0xFFFFFFFF
#define RISCV64_INVALID_IRQ 0xFFFFFFFF
#define RISCV64_INVALID_EXCEPTION 0xFFFFFFFF

// ============================================================================
// MEMORY REGION STRUCTURES
// ============================================================================

// Memory region descriptor
struct riscv64_memory_region {
    riscv64_addr_t base_address;
    riscv64_size_t size;
    uint64_t flags;
    uint32_t type;
    uint32_t reserved;
};

// TLB entry
struct riscv64_tlb_entry {
    riscv64_addr_t virtual_address;
    riscv64_addr_t physical_address;
    uint64_t flags;
    uint32_t asid;
    bool valid;
};

// Page table entry
struct riscv64_page_table {
    uint64_t entries[512];
    uint32_t level;
    uint32_t reserved;
};

// Vector table entry
struct riscv64_vector_table {
    void (*handlers[64])(void);
    uint32_t count;
    uint32_t reserved;
};

// ============================================================================
// CPU STATE STRUCTURES
// ============================================================================

// CPU state
struct riscv64_cpu_state {
    riscv64_reg_t ra, sp, gp, tp;
    riscv64_reg_t t0, t1, t2;
    riscv64_reg_t s0, s1;
    riscv64_reg_t a0, a1, a2, a3, a4, a5, a6, a7;
    riscv64_reg_t s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
    riscv64_reg_t t3, t4, t5, t6;
    riscv64_reg_t pc, status, cause, tval;
};

// CPU information
struct riscv64_cpu_info {
    uint32_t hart_id;
    uint32_t cpu_id;
    uint64_t frequency;
    uint64_t revision;
    uint64_t features;
    uint32_t vendor_id;
    uint32_t architecture_id;
    uint32_t implementation_id;
};

// Cache context
struct riscv64_cache_context {
    uint32_t l1i_size;
    uint32_t l1d_size;
    uint32_t l2_size;
    uint32_t l3_size;
    uint32_t line_size;
    uint32_t ways;
    uint32_t sets;
    bool enabled;
};

// Error context
struct riscv64_error_context {
    uint32_t error_type;
    uint32_t error_code;
    riscv64_addr_t error_address;
    riscv64_reg_t error_register;
    uint64_t timestamp;
    uint32_t severity;
};

// ============================================================================
// ENUMERATIONS
// ============================================================================

// CPU modes
enum riscv64_cpu_mode {
    RISCV64_MODE_USER = 0,
    RISCV64_MODE_SUPERVISOR = 1,
    RISCV64_MODE_MACHINE = 3
};

// Power states
enum riscv64_power_state {
    RISCV64_POWER_ACTIVE = 0,
    RISCV64_POWER_IDLE = 1,
    RISCV64_POWER_SLEEP = 2,
    RISCV64_POWER_DEEP_SLEEP = 3,
    RISCV64_POWER_OFF = 4
};

// CPU features
enum riscv64_cpu_feature {
    RISCV64_FEATURE_RV64I = 0,
    RISCV64_FEATURE_RV64M = 1,
    RISCV64_FEATURE_RV64A = 2,
    RISCV64_FEATURE_RV64F = 3,
    RISCV64_FEATURE_RV64D = 4,
    RISCV64_FEATURE_RV64C = 5,
    RISCV64_FEATURE_RVV = 6,
    RISCV64_FEATURE_RVH = 7,
    RISCV64_FEATURE_RVP = 8,
    RISCV64_FEATURE_RV64B = 9
};

// ============================================================================
// FUNCTION TYPES
// ============================================================================

// Interrupt handler type
typedef void (*riscv64_interrupt_handler_t)(void);

// Exception handler type
typedef void (*riscv64_exception_handler_t)(uint64_t cause, uint64_t epc, uint64_t tval);

// System call handler type
typedef void (*riscv64_syscall_handler_t)(void);

// Timer callback type
typedef void (*riscv64_timer_callback_t)(void);

// Error handler type
typedef void (*riscv64_error_handler_t)(const struct riscv64_error_context *ctx);

// ============================================================================
// UTILITY MACROS
// ============================================================================

// Alignment macros
#define RISCV64_ALIGN_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))
#define RISCV64_ALIGN_DOWN(x, align) ((x) & ~((align) - 1))
#define RISCV64_IS_ALIGNED(x, align) (((x) & ((align) - 1)) == 0)

// Bit manipulation macros
#define RISCV64_BIT(n) (1ULL << (n))
#define RISCV64_BIT_MASK(n) (RISCV64_BIT(n) - 1)
#define RISCV64_SET_BIT(x, n) ((x) |= RISCV64_BIT(n))
#define RISCV64_CLEAR_BIT(x, n) ((x) &= ~RISCV64_BIT(n))
#define RISCV64_TEST_BIT(x, n) (((x) & RISCV64_BIT(n)) != 0)

// Page manipulation macros
#define RISCV64_PAGE_ALIGN_UP(x) RISCV64_ALIGN_UP(x, RISCV64_PAGE_SIZE)
#define RISCV64_PAGE_ALIGN_DOWN(x) RISCV64_ALIGN_DOWN(x, RISCV64_PAGE_SIZE)
#define RISCV64_IS_PAGE_ALIGNED(x) RISCV64_IS_ALIGNED(x, RISCV64_PAGE_SIZE)

// Address manipulation macros
#define RISCV64_PAGE_NUMBER(addr) ((addr) >> RISCV64_PAGE_SHIFT)
#define RISCV64_PAGE_OFFSET(addr) ((addr) & RISCV64_PAGE_MASK)
#define RISCV64_PAGE_ADDRESS(addr) ((addr) & ~RISCV64_PAGE_MASK)

// ============================================================================
// INLINE FUNCTIONS
// ============================================================================

// Check if address is valid
static inline bool riscv64_is_valid_address(riscv64_addr_t addr)
{
    return addr != RISCV64_INVALID_ADDRESS;
}

// Check if process ID is valid
static inline bool riscv64_is_valid_process_id(riscv64_process_id_t pid)
{
    return pid != RISCV64_INVALID_PROCESS_ID;
}

// Check if thread ID is valid
static inline bool riscv64_is_valid_thread_id(riscv64_thread_id_t tid)
{
    return tid != RISCV64_INVALID_THREAD_ID;
}

// Check if IRQ is valid
static inline bool riscv64_is_valid_irq(riscv64_irq_t irq)
{
    return irq < RISCV64_MAX_IRQS;
}

// Check if exception is valid
static inline bool riscv64_is_valid_exception(riscv64_exception_t exc)
{
    return exc < 16;  // RISC-V has 16 standard exceptions
}

// Check if syscall is valid
static inline bool riscv64_is_valid_syscall(riscv64_syscall_t syscall)
{
    return syscall < 64;  // Support up to 64 system calls
}

// ============================================================================
// CONSTANTS
// ============================================================================

// Page flags
#define RISCV64_PAGE_VALID     0x001
#define RISCV64_PAGE_READ      0x002
#define RISCV64_PAGE_WRITE     0x004
#define RISCV64_PAGE_EXECUTE   0x008
#define RISCV64_PAGE_USER      0x010
#define RISCV64_PAGE_GLOBAL    0x020
#define RISCV64_PAGE_ACCESSED  0x040
#define RISCV64_PAGE_DIRTY     0x080
#define RISCV64_PAGE_HUGE      0x100
#define RISCV64_PAGE_NOCACHE   0x200
#define RISCV64_PAGE_WRITETHROUGH 0x400
#define RISCV64_PAGE_GUARD     0x800

// Memory region types
#define RISCV64_MEMORY_RAM     0x01
#define RISCV64_MEMORY_ROM     0x02
#define RISCV64_MEMORY_DEVICE  0x03
#define RISCV64_MEMORY_RESERVED 0x04
#define RISCV64_MEMORY_ACPI    0x05
#define RISCV64_MEMORY_NVS     0x06

// Memory region flags
#define RISCV64_MEMORY_READABLE    0x001
#define RISCV64_MEMORY_WRITABLE    0x002
#define RISCV64_MEMORY_EXECUTABLE  0x004
#define RISCV64_MEMORY_CACHABLE    0x008
#define RISCV64_MEMORY_BUFFERABLE  0x010
#define RISCV64_MEMORY_SHARED     0x020
#define RISCV64_MEMORY_PERSISTENT 0x040

// Error types
#define RISCV64_ERROR_NONE         0x00
#define RISCV64_ERROR_MEMORY      0x01
#define RISCV64_ERROR_INSTRUCTION 0x02
#define RISCV64_ERROR_DATA        0x03
#define RISCV64_ERROR_TIMEOUT     0x04
#define RISCV64_ERROR_HARDWARE    0x05
#define RISCV64_ERROR_SOFTWARE    0x06

// Error severity levels
#define RISCV64_ERROR_SEVERITY_LOW     0x01
#define RISCV64_ERROR_SEVERITY_MEDIUM 0x02
#define RISCV64_ERROR_SEVERITY_HIGH   0x03
#define RISCV64_ERROR_SEVERITY_CRITICAL 0x04

// ============================================================================
// COMPILER ATTRIBUTES
// ============================================================================

// Alignment attribute
#define RISCV64_ALIGNED(x) __attribute__((aligned(x)))

// Packed attribute
#define RISCV64_PACKED __attribute__((packed))

// No return attribute
#define RISCV64_NORETURN __attribute__((noreturn))

// Unused attribute
#define RISCV64_UNUSED __attribute__((unused))

// Section attribute
#define RISCV64_SECTION(x) __attribute__((section(x)))

// Weak attribute
#define RISCV64_WEAK __attribute__((weak))

// ============================================================================
// MSVC SUPPORT
// ============================================================================

#ifdef _MSC_VER
#undef RISCV64_ALIGNED
#undef RISCV64_PACKED
#undef RISCV64_NORETURN
#undef RISCV64_UNUSED
#undef RISCV64_SECTION
#undef RISCV64_WEAK

#define RISCV64_ALIGNED(x) __declspec(align(x))
#define RISCV64_PACKED
#define RISCV64_NORETURN __declspec(noreturn)
#define RISCV64_UNUSED
#define RISCV64_SECTION(x)
#define RISCV64_WEAK
#endif

#endif /* ORION_RISCV64_COMMON_H */
