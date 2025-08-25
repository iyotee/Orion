/*
 * ORION OS - ARMv7l Architecture Header
 *
 * Main header file for ARMv7l architecture (32-bit ARM)
 * Supports all Raspberry Pi models: Pi 1, Pi 2, Pi Zero, Pi Zero W
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_ARMV7L_ARCH_H
#define ORION_ARMV7L_ARCH_H

#include "config.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// COMPILER ATTRIBUTES
// ============================================================================

#define ALIGNED(x) __attribute__((aligned(x)))
#define NORETURN __attribute__((noreturn))
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define PURE __attribute__((pure))
#define ORION_CONST __attribute__((const))

// ============================================================================
// BASIC TYPES
// ============================================================================

typedef uint32_t arm_reg_t;
typedef uint32_t arm_addr_t;
typedef uint32_t arm_size_t;
typedef uint32_t arm_irq_t;
typedef uint32_t arm_exc_t;

// ============================================================================
// PRIVILEGE LEVELS
// ============================================================================

#define ARM_PRIVILEGE_USER 0
#define ARM_PRIVILEGE_SYSTEM 1
#define ARM_PRIVILEGE_SUPERVISOR 2
#define ARM_PRIVILEGE_ABORT 3
#define ARM_PRIVILEGE_UNDEFINED 4
#define ARM_PRIVILEGE_IRQ 5
#define ARM_PRIVILEGE_FIQ 6

// ============================================================================
// CPU MODES
// ============================================================================

#define ARM_MODE_USER 0x10
#define ARM_MODE_FIQ 0x11
#define ARM_MODE_IRQ 0x12
#define ARM_MODE_SUPERVISOR 0x13
#define ARM_MODE_ABORT 0x17
#define ARM_MODE_UNDEFINED 0x1B
#define ARM_MODE_SYSTEM 0x1F
#define ARM_MODE_MONITOR 0x16

// ============================================================================
// CPSR FLAGS
// ============================================================================

#define ARM_CPSR_N 0x80000000   // Negative flag
#define ARM_CPSR_Z 0x40000000   // Zero flag
#define ARM_CPSR_C 0x20000000   // Carry flag
#define ARM_CPSR_V 0x10000000   // Overflow flag
#define ARM_CPSR_Q 0x08000000   // Saturation flag
#define ARM_CPSR_IT 0x06000000  // IT state bits
#define ARM_CPSR_J 0x01000000   // Jazelle flag
#define ARM_CPSR_GE 0x000F0000  // Greater than or equal flags
#define ARM_CPSR_IT2 0x0000FC00 // IT state bits 2
#define ARM_CPSR_E 0x00000200   // Endianness flag
#define ARM_CPSR_A 0x00000100   // Abort disable flag
#define ARM_CPSR_I 0x00000080   // IRQ disable flag
#define ARM_CPSR_F 0x00000040   // FIQ disable flag
#define ARM_CPSR_T 0x00000020   // Thumb state flag
#define ARM_CPSR_M 0x0000001F   // Mode bits

// ============================================================================
// EXCEPTION VECTORS
// ============================================================================

#define ARM_VECTOR_RESET 0x00
#define ARM_VECTOR_UNDEFINED 0x04
#define ARM_VECTOR_SWI 0x08
#define ARM_VECTOR_PREFETCH_ABORT 0x0C
#define ARM_VECTOR_DATA_ABORT 0x10
#define ARM_VECTOR_RESERVED 0x14
#define ARM_VECTOR_IRQ 0x18
#define ARM_VECTOR_FIQ 0x1C

// Exception types for handler registration
#define ARM_EXCEPTION_UNDEFINED 0
#define ARM_EXCEPTION_SWI 1
#define ARM_EXCEPTION_PREFETCH_ABORT 2
#define ARM_EXCEPTION_DATA_ABORT 3
#define ARM_EXCEPTION_IRQ 4
#define ARM_EXCEPTION_FIQ 5

// ============================================================================
// MMU CONFIGURATION
// ============================================================================

#define ARM_MMU_DOMAIN_NO_ACCESS 0x00
#define ARM_MMU_DOMAIN_CLIENT 0x01
#define ARM_MMU_DOMAIN_MANAGER 0x03

#define ARM_MMU_AP_NO_ACCESS 0x00
#define ARM_MMU_AP_PRIVILEGED_ONLY 0x01
#define ARM_MMU_AP_READ_WRITE 0x02
#define ARM_MMU_AP_READ_ONLY 0x03

#define ARM_MMU_TEX_CACHEABLE 0x01
#define ARM_MMU_TEX_BUFFERABLE 0x02
#define ARM_MMU_TEX_SHAREABLE 0x04

// ============================================================================
// CACHE CONFIGURATION
// ============================================================================

#define ARM_CACHE_WRITE_BACK 0x00
#define ARM_CACHE_WRITE_THROUGH 0x01
#define ARM_CACHE_WRITE_ALLOCATE 0x02
#define ARM_CACHE_NO_WRITE_ALLOCATE 0x00

#define ARM_CACHE_READ_ALLOCATE 0x04
#define ARM_CACHE_NO_READ_ALLOCATE 0x00

// ============================================================================
// INTERRUPT CONFIGURATION
// ============================================================================

#define ARM_IRQ_PRIORITY_HIGH 0
#define ARM_IRQ_PRIORITY_NORMAL 1
#define ARM_IRQ_PRIORITY_LOW 2

#define ARM_IRQ_TRIGGER_LEVEL 0
#define ARM_IRQ_TRIGGER_EDGE 1

// ============================================================================
// TIMER CONFIGURATION
// ============================================================================

#define ARM_TIMER_MODE_PERIODIC 0
#define ARM_TIMER_MODE_ONE_SHOT 1
#define ARM_TIMER_MODE_FREE_RUNNING 2

// ============================================================================
// POWER MANAGEMENT
// ============================================================================

// Power states are defined in config.h

// ============================================================================
// DATA STRUCTURES
// ============================================================================

typedef struct
{
    uint32_t r0, r1, r2, r3, r4, r5, r6, r7;
    uint32_t r8, r9, r10, r11, r12, sp, lr, pc;
    uint32_t cpsr;
} arm_context_t;

// Alias for compatibility
typedef arm_context_t arm_cpu_context_t;

typedef struct
{
    uint32_t base_addr;
    uint32_t size;
    uint32_t flags;
    uint32_t domain;
} arm_memory_region_t;

typedef struct
{
    uint32_t virtual_addr;
    uint32_t physical_addr;
    uint32_t size;
    uint32_t flags;
    uint32_t domain;
} arm_page_table_entry_t;

typedef struct
{
    uint32_t irq_number;
    uint32_t priority;
    uint32_t trigger_mode;
    void (*handler)(void);
    void *data;
} arm_irq_info_t;

typedef struct
{
    uint32_t timer_id;
    uint32_t frequency;
    uint32_t mode;
    uint32_t value;
    uint32_t reload_value;
    void (*callback)(void);
} arm_timer_info_t;

typedef struct
{
    uint32_t cache_level;
    uint32_t size;
    uint32_t line_size;
    uint32_t associativity;
    uint32_t write_policy;
} arm_cache_info_t;

typedef struct
{
    uint32_t vector_base;
    uint32_t vector_count;
    uint32_t priority_bits;
    bool fiq_enabled;
    bool irq_enabled;
} arm_interrupt_controller_t;

typedef struct
{
    uint32_t mmu_enabled;
    uint32_t cache_enabled;
    uint32_t tlb_enabled;
    uint32_t domain_count;
    arm_memory_region_t regions[16];
} arm_mmu_context_t;

// arm_cpu_context_t is now an alias for arm_context_t above

typedef struct
{
    uint32_t timer_enabled;
    uint32_t timer_count;
    uint32_t system_tick;
    arm_timer_info_t timers[8];
} arm_timer_context_t;

typedef struct
{
    uint32_t cache_enabled;
    uint32_t cache_levels;
    arm_cache_info_t caches[3];
} arm_cache_context_t;

typedef struct
{
    uint32_t power_state;
    uint32_t cpu_frequency;
    uint32_t voltage_level;
    uint32_t temperature;
} arm_power_context_t;

typedef struct
{
    uint32_t security_enabled;
    uint32_t trustzone_enabled;
    uint32_t secure_monitor_enabled;
    uint32_t crypto_enabled;
} arm_security_context_t;

typedef struct
{
    uint32_t virtualization_enabled;
    uint32_t hypervisor_mode;
    uint32_t vm_count;
    uint32_t nested_virtualization;
} arm_virtualization_context_t;

typedef struct
{
    uint32_t performance_enabled;
    uint32_t pmu_enabled;
    uint32_t cycle_counter;
    uint32_t instruction_counter;
} arm_performance_context_t;

typedef struct
{
    uint32_t debug_enabled;
    uint32_t breakpoint_count;
    uint32_t watchpoint_count;
    uint32_t etm_enabled;
} arm_debug_context_t;

typedef struct
{
    uint32_t neon_enabled;
    uint32_t vfp_enabled;
    uint32_t crypto_enabled;
    uint32_t crc_enabled;
} arm_extension_context_t;

// ============================================================================
// FUNCTION DECLARATIONS - CPU MANAGEMENT
// ============================================================================

// CPU Control
void arm_cpu_init(void);
void arm_cpu_reset(void);
void arm_cpu_halt(void);
void arm_cpu_wait_for_interrupt(void);
uint32_t arm_cpu_get_id(void);
uint32_t arm_cpu_get_revision(void);
uint32_t arm_cpu_get_frequency(void);
void arm_cpu_set_frequency(uint32_t frequency);

// CPU Context
void arm_cpu_save_context(arm_cpu_context_t *context);
void arm_cpu_restore_context(const arm_cpu_context_t *context);
void arm_cpu_switch_context(arm_cpu_context_t *old_context, arm_cpu_context_t *new_context);

// Context Management (aliases for compatibility)
void arm_save_context(arm_cpu_context_t *context);
void arm_restore_context(const arm_cpu_context_t *context);

// CPU Modes
void arm_cpu_switch_mode(uint32_t mode);
uint32_t arm_cpu_get_current_mode(void);
bool arm_cpu_is_in_privileged_mode(void);
bool arm_is_privileged(void);
uint32_t arm_get_current_mode(void);
void arm_cpu_enter_user_mode(void);

// CPU Features
bool arm_cpu_supports_neon(void);
bool arm_cpu_supports_vfp(void);
bool arm_cpu_supports_crypto(void);
bool arm_cpu_supports_trustzone(void);
uint32_t arm_cpu_get_feature_flags(void);

// ============================================================================
// FUNCTION DECLARATIONS - MEMORY MANAGEMENT
// ============================================================================

// MMU Control
void arm_mmu_init(void);
void arm_mmu_enable(void);
void arm_mmu_disable(void);
bool arm_mmu_is_enabled(void);

// Page Tables
void arm_mmu_setup_page_tables(void);
void arm_mmu_map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);
void arm_mmu_unmap_page(uint32_t virtual_addr);
uint32_t arm_mmu_get_physical_address(uint32_t virtual_addr);

// Memory Regions
void arm_mmu_add_memory_region(const arm_memory_region_t *region);
void arm_mmu_remove_memory_region(uint32_t base_addr);
arm_memory_region_t *arm_mmu_find_memory_region(uint32_t addr);

// TLB Management
void arm_mmu_invalidate_tlb(void);
void arm_mmu_invalidate_tlb_entry(uint32_t addr);
void arm_mmu_invalidate_tlb_range(uint32_t start_addr, uint32_t end_addr);

// ============================================================================
// FUNCTION DECLARATIONS - INTERRUPT MANAGEMENT
// ============================================================================

// Interrupt Controller
void arm_interrupt_init(void);
void arm_interrupt_enable_irq(uint32_t irq_number);
void arm_interrupt_disable_irq(uint32_t irq_number);
bool arm_interrupt_is_irq_enabled(uint32_t irq_number);

// Interrupt Handlers
void arm_interrupt_register_handler(uint32_t irq_number, void (*handler)(void), void *data);
void arm_interrupt_unregister_handler(uint32_t irq_number);
void arm_interrupt_set_priority(uint32_t irq_number, uint32_t priority);

// Interrupt Control
void arm_interrupt_enable_fiq(void);
void arm_interrupt_disable_fiq(void);
void arm_interrupt_enable_irq(void);
void arm_interrupt_disable_irq(void);

// Interrupt Status
bool arm_is_irq_enabled(void);
bool arm_is_fiq_enabled(void);

// Interrupt Handler Registration
int arm_register_exception_handler(uint32_t exception_type, void (*handler)(void), void *data);
void arm_unregister_exception_handler(uint32_t exception_type);
int arm_register_irq_handler(uint32_t irq_number, void (*handler)(void), void *data);
void arm_unregister_irq_handler(uint32_t irq_number);
int arm_register_fiq_handler(void (*handler)(void), void *data);
void arm_unregister_fiq_handler(void);

// Exception Handling
void arm_exception_init(void);
void arm_exception_register_handler(uint32_t exception_type, void (*handler)(void));
void arm_exception_unregister_handler(uint32_t exception_type);

// ============================================================================
// FUNCTION DECLARATIONS - TIMER MANAGEMENT
// ============================================================================

// Timer Control
void arm_timer_init(void);
void arm_timer_start(uint32_t timer_id);
void arm_timer_stop(uint32_t timer_id);
void arm_timer_reset(uint32_t timer_id);

// Timer Configuration
void arm_timer_set_frequency(uint32_t timer_id, uint32_t frequency);
void arm_timer_set_mode(uint32_t timer_id, uint32_t mode);
void arm_timer_set_callback(uint32_t timer_id, void (*callback)(void));

// Timer Values
uint32_t arm_timer_get_value(uint32_t timer_id);
uint32_t arm_timer_get_frequency(uint32_t timer_id);
uint32_t arm_timer_get_system_tick(void);

// System Timer (Raspberry Pi specific)
uint32_t arm_system_timer_get_value(void);
uint32_t arm_system_timer_get_frequency(void);
void arm_system_timer_set_callback(void (*callback)(void));

// System Timer Control
void arm_system_timer_start(void);
void arm_system_timer_stop(void);
void arm_system_timer_reset(void);

// Periodic Timer Control
void arm_periodic_timer_start(void);
void arm_periodic_timer_stop(void);
void arm_periodic_timer_reset(void);

// ============================================================================
// FUNCTION DECLARATIONS - CACHE MANAGEMENT
// ============================================================================

// Cache Control
void arm_cache_init(void);
void arm_cache_enable(void);
void arm_cache_disable(void);
bool arm_cache_is_enabled(void);

// Cache Operations
void arm_cache_flush_all(void);
void arm_cache_invalidate_all(void);
void arm_cache_flush_range(uint32_t start_addr, uint32_t end_addr);
void arm_cache_invalidate_range(uint32_t start_addr, uint32_t end_addr);

// Cache Information
arm_cache_info_t *arm_cache_get_info(uint32_t level);
uint32_t arm_cache_get_line_size(void);
uint32_t arm_cache_get_size(uint32_t level);

// ============================================================================
// FUNCTION DECLARATIONS - POWER MANAGEMENT
// ============================================================================

// Power Control
void arm_power_init(void);
void arm_power_set_state(uint32_t state);
uint32_t arm_power_get_state(void);
void arm_power_set_cpu_frequency(uint32_t frequency);

// Power Monitoring
uint32_t arm_power_get_cpu_frequency(void);
uint32_t arm_power_get_voltage_level(void);
uint32_t arm_power_get_temperature(void);
uint32_t arm_power_get_consumption(void);

// ============================================================================
// FUNCTION DECLARATIONS - SECURITY
// ============================================================================

// Security Control
void arm_security_init(void);
void arm_security_enable_trustzone(void);
void arm_security_disable_trustzone(void);
bool arm_security_is_trustzone_enabled(void);

// Secure Monitor
void arm_security_enter_secure_monitor(void);
void arm_security_exit_secure_monitor(void);
bool arm_security_is_in_secure_mode(void);

// ============================================================================
// FUNCTION DECLARATIONS - VIRTUALIZATION
// ============================================================================

// Virtualization Control
void arm_virtualization_init(void);
void arm_virtualization_enable(void);
void arm_virtualization_disable(void);
bool arm_virtualization_is_enabled(void);

// Virtual Machine Management
uint32_t arm_virtualization_create_vm(void);
void arm_virtualization_destroy_vm(uint32_t vm_id);
void arm_virtualization_start_vm(uint32_t vm_id);
void arm_virtualization_stop_vm(uint32_t vm_id);

// ============================================================================
// FUNCTION DECLARATIONS - PERFORMANCE MONITORING
// ============================================================================

// Performance Control
void arm_performance_init(void);
void arm_performance_enable_pmu(void);
void arm_performance_disable_pmu(void);
bool arm_performance_is_pmu_enabled(void);

// Performance Counters
uint32_t arm_performance_get_cycle_counter(void);
uint32_t arm_performance_get_instruction_counter(void);
void arm_performance_reset_counters(void);

// Performance Aliases
uint64_t arm_perf_get_cycle_counter(void);
uint64_t arm_perf_get_instruction_counter(void);

// System Functions
uint64_t arm_get_system_ticks(void);
uint64_t arm_get_system_uptime_ms(void);

// ============================================================================
// FUNCTION DECLARATIONS - DEBUG SUPPORT
// ============================================================================

// Debug Control
void arm_debug_init(void);
void arm_debug_enable(void);
void arm_debug_disable(void);
bool arm_debug_is_enabled(void);

// Breakpoints and Watchpoints
void arm_debug_set_breakpoint(uint32_t addr);
void arm_debug_clear_breakpoint(uint32_t addr);
void arm_debug_set_watchpoint(uint32_t addr, uint32_t size, uint32_t type);
void arm_debug_clear_watchpoint(uint32_t addr);

// ============================================================================
// FUNCTION DECLARATIONS - MEMORY BARRIERS AND ATOMIC OPERATIONS
// ============================================================================

// Memory Barriers
void arm_memory_barrier(void);
void arm_read_barrier(void);
void arm_write_barrier(void);
void arm_instruction_barrier(void);

// Atomic Operations
uint32_t arm_atomic_swap(volatile uint32_t *ptr, uint32_t new_value);
uint32_t arm_atomic_add(volatile uint32_t *ptr, uint32_t value);
uint32_t arm_atomic_sub(volatile uint32_t *ptr, uint32_t value);
uint32_t arm_atomic_and(volatile uint32_t *ptr, uint32_t value);
uint32_t arm_atomic_or(volatile uint32_t *ptr, uint32_t value);
uint32_t arm_atomic_xor(volatile uint32_t *ptr, uint32_t value);

// ============================================================================
// FUNCTION DECLARATIONS - EXTENSIONS
// ============================================================================

// NEON Support
void arm_neon_init(void);
bool arm_neon_is_available(void);
void arm_neon_enable(void);
void arm_neon_disable(void);

// VFP Support
void arm_vfp_init(void);
bool arm_vfp_is_available(void);
void arm_vfp_enable(void);
void arm_vfp_disable(void);

// Crypto Support
void arm_crypto_init(void);
bool arm_crypto_is_available(void);
void arm_crypto_enable(void);
void arm_crypto_disable(void);

// ============================================================================
// FUNCTION DECLARATIONS - RASPBERRY PI SPECIFIC
// ============================================================================

// GPIO Management
void arm_gpio_init(void);
void arm_gpio_set_direction(uint32_t pin, uint32_t direction);
void arm_gpio_set_value(uint32_t pin, uint32_t value);
uint32_t arm_gpio_get_value(uint32_t pin);

// UART Management
void arm_uart_init(void);
void arm_uart_send_byte(uint8_t byte);
uint8_t arm_uart_receive_byte(void);
bool arm_uart_is_data_available(void);

// SPI Management
void arm_spi_init(void);
void arm_spi_transfer(uint8_t *data, uint32_t length);
void arm_spi_set_clock_frequency(uint32_t frequency);

// I2C Management
void arm_i2c_init(void);
void arm_i2c_write(uint8_t device_addr, uint8_t *data, uint32_t length);
void arm_i2c_read(uint8_t device_addr, uint8_t *data, uint32_t length);

// PWM Management
void arm_pwm_init(void);
void arm_pwm_set_frequency(uint32_t channel, uint32_t frequency);
void arm_pwm_set_duty_cycle(uint32_t channel, uint32_t duty_cycle);

// ============================================================================
// FUNCTION DECLARATIONS - SYSTEM INITIALIZATION
// ============================================================================

// Early Initialization
void arm_early_init(void);
void arm_system_init(void);
void arm_late_init(void);

// Device Driver Initialization
void arm_device_drivers_init(void);
void arm_file_system_init(void);
void arm_network_stack_init(void);
void arm_user_space_init(void);

// System Control
void arm_system_monitor(void);
void arm_system_test(void);
void arm_main_loop(void);
void arm_main(void);

// Bootstrap and Shutdown
void arm_bootstrap(void);
void arm_reset(void);
void arm_shutdown(void);

// ============================================================================
// INLINE FUNCTIONS
// ============================================================================

static inline void arm_cpu_enable_interrupts(void)
{
    __asm__ volatile("cpsie i");
}

static inline void arm_cpu_disable_interrupts(void)
{
    __asm__ volatile("cpsid i");
}

static inline void arm_cpu_enable_fiq(void)
{
    __asm__ volatile("cpsie f");
}

static inline void arm_cpu_disable_fiq(void)
{
    __asm__ volatile("cpsid f");
}

static inline void arm_cpu_nop(void)
{
    __asm__ volatile("nop");
}

static inline void arm_cpu_wfi(void)
{
    __asm__ volatile("wfi");
}

static inline void arm_cpu_wfe(void)
{
    __asm__ volatile("wfe");
}

static inline void arm_cpu_sev(void)
{
    __asm__ volatile("sev");
}

static inline uint32_t arm_cpu_read_cpsr(void)
{
    uint32_t cpsr;
    __asm__ volatile("mrs %0, cpsr" : "=r"(cpsr));
    return cpsr;
}

static inline void arm_cpu_write_cpsr(uint32_t cpsr)
{
    __asm__ volatile("msr cpsr, %0" : : "r"(cpsr));
}

static inline uint32_t arm_cpu_read_spsr(void)
{
    uint32_t spsr;
    __asm__ volatile("mrs %0, spsr" : "=r"(spsr));
    return spsr;
}

static inline void arm_cpu_write_spsr(uint32_t spsr)
{
    __asm__ volatile("msr spsr, %0" : : "r"(spsr));
}

// ============================================================================
// MACROS
// ============================================================================

#define ARM_CPU_ENTER_CRITICAL_SECTION()     \
    do {                                     \
        uint32_t cpsr = arm_cpu_read_cpsr(); \
        arm_cpu_disable_interrupts();        \
        arm_cpu_disable_fiq();               \
    } while (0)

#define ARM_CPU_EXIT_CRITICAL_SECTION() \
    do {                                \
        arm_cpu_write_cpsr(cpsr);       \
    } while (0)

#define ARM_CPU_ENTER_FIQ_CRITICAL_SECTION() \
    do {                                     \
        uint32_t cpsr = arm_cpu_read_cpsr(); \
        arm_cpu_disable_fiq();               \
    } while (0)

#define ARM_CPU_EXIT_FIQ_CRITICAL_SECTION() \
    do {                                     \
        arm_cpu_write_cpsr(cpsr);            \
    } while (0)

#define ARM_CPU_ENTER_IRQ_CRITICAL_SECTION() \
    do {                                      \
        uint32_t cpsr = arm_cpu_read_cpsr();  \
        arm_cpu_disable_irq();                \
    } while (0)

#define ARM_CPU_EXIT_IRQ_CRITICAL_SECTION() \
    do {                                     \
        arm_cpu_write_cpsr(cpsr);            \
    } while (0)

#endif // ORION_ARMV7L_ARCH_H
