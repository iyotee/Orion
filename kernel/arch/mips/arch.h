/*
 * ORION OS - MIPS Architecture Header
 *
 * Main header file for MIPS architecture (MIPS32, MIPS64)
 * Supports all MIPS processors: R3000, R4000, R5000, R10000, etc.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_MIPS_ARCH_H
#define ORION_MIPS_ARCH_H

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

typedef uint64_t mips_reg_t;
typedef uint64_t mips_addr_t;
typedef uint64_t mips_size_t;
typedef uint32_t mips_irq_t;
typedef uint32_t mips_exc_t;

// ============================================================================
// CPU REGISTERS
// ============================================================================

#define MIPS_REG_ZERO 0
#define MIPS_REG_AT 1
#define MIPS_REG_V0 2
#define MIPS_REG_V1 3
#define MIPS_REG_A0 4
#define MIPS_REG_A1 5
#define MIPS_REG_A2 6
#define MIPS_REG_A3 7
#define MIPS_REG_T0 8
#define MIPS_REG_T1 9
#define MIPS_REG_T2 10
#define MIPS_REG_T3 11
#define MIPS_REG_T4 12
#define MIPS_REG_T5 13
#define MIPS_REG_T6 14
#define MIPS_REG_T7 15
#define MIPS_REG_S0 16
#define MIPS_REG_S1 17
#define MIPS_REG_S2 18
#define MIPS_REG_S3 19
#define MIPS_REG_S4 20
#define MIPS_REG_S5 21
#define MIPS_REG_S6 22
#define MIPS_REG_S7 23
#define MIPS_REG_T8 24
#define MIPS_REG_T9 25
#define MIPS_REG_K0 26
#define MIPS_REG_K1 27
#define MIPS_REG_GP 28
#define MIPS_REG_SP 29
#define MIPS_REG_FP 30
#define MIPS_REG_RA 31

// ============================================================================
// CP0 REGISTERS
// ============================================================================

#define MIPS_CP0_INDEX 0
#define MIPS_CP0_RANDOM 1
#define MIPS_CP0_ENTRYLO0 2
#define MIPS_CP0_ENTRYLO1 3
#define MIPS_CP0_CONTEXT 4
#define MIPS_CP0_PAGEMASK 5
#define MIPS_CP0_WIRED 6
#define MIPS_CP0_HWRENA 7
#define MIPS_CP0_BADVADDR 8
#define MIPS_CP0_COUNT 9
#define MIPS_CP0_ENTRYHI 10
#define MIPS_CP0_COMPARE 11
#define MIPS_CP0_STATUS 12
#define MIPS_CP0_CAUSE 13
#define MIPS_CP0_EPC 14
#define MIPS_CP0_PRID 15
#define MIPS_CP0_CONFIG 16
#define MIPS_CP0_LLADDR 17
#define MIPS_CP0_WATCHLO 18
#define MIPS_CP0_WATCHHI 19
#define MIPS_CP0_XCONTEXT 20
#define MIPS_CP0_DEBUG 23
#define MIPS_CP0_DEPC 24
#define MIPS_CP0_PERFORMANCE 25
#define MIPS_CP0_ECC 26
#define MIPS_CP0_CACHEERR 27
#define MIPS_CP0_TAGLO 28
#define MIPS_CP0_TAGHI 29
#define MIPS_CP0_ERROREPC 30

// ============================================================================
// STATUS REGISTER BITS
// ============================================================================

#define MIPS_STATUS_IE 0x00000001
#define MIPS_STATUS_EXL 0x00000002
#define MIPS_STATUS_ERL 0x00000004
#define MIPS_STATUS_KSU 0x00000018
#define MIPS_STATUS_UX 0x00000020
#define MIPS_STATUS_SX 0x00000040
#define MIPS_STATUS_KX 0x00000080
#define MIPS_STATUS_IM 0x0000FF00
#define MIPS_STATUS_IP 0x0000FF00
#define MIPS_STATUS_IP0 0x00000100
#define MIPS_STATUS_IP1 0x00000200
#define MIPS_STATUS_IP2 0x00000400
#define MIPS_STATUS_IP3 0x00000800
#define MIPS_STATUS_IP4 0x00001000
#define MIPS_STATUS_IP5 0x00002000
#define MIPS_STATUS_IP6 0x00004000
#define MIPS_STATUS_IP7 0x00008000
#define MIPS_STATUS_CH 0x00010000
#define MIPS_STATUS_NMI 0x00020000
#define MIPS_STATUS_SR 0x00100000
#define MIPS_STATUS_TS 0x00200000
#define MIPS_STATUS_BEV 0x00400000
#define MIPS_STATUS_PX 0x00800000
#define MIPS_STATUS_MX 0x01000000
#define MIPS_STATUS_RE 0x02000000
#define MIPS_STATUS_FR 0x04000000
#define MIPS_STATUS_RP 0x08000000
#define MIPS_STATUS_CU 0xF0000000
#define MIPS_STATUS_CU0 0x10000000
#define MIPS_STATUS_CU1 0x20000000
#define MIPS_STATUS_CU2 0x40000000
#define MIPS_STATUS_CU3 0x80000000

// ============================================================================
// CAUSE REGISTER BITS
// ============================================================================

#define MIPS_CAUSE_EXCCODE 0x0000007C
#define MIPS_CAUSE_IP 0x0000FF00
#define MIPS_CAUSE_IP0 0x00000100
#define MIPS_CAUSE_IP1 0x00000200
#define MIPS_CAUSE_IP2 0x00000400
#define MIPS_CAUSE_IP3 0x00000800
#define MIPS_CAUSE_IP4 0x00001000
#define MIPS_CAUSE_IP5 0x00002000
#define MIPS_CAUSE_IP6 0x00004000
#define MIPS_CAUSE_IP7 0x00008000
#define MIPS_CAUSE_CE 0x30000000
#define MIPS_CAUSE_BD 0x80000000

// ============================================================================
// DATA STRUCTURES
// ============================================================================

typedef struct
{
    uint64_t at, v0, v1, a0, a1, a2, a3;
    uint64_t t0, t1, t2, t3, t4, t5, t6, t7;
    uint64_t s0, s1, s2, s3, s4, s5, s6, s7;
    uint64_t t8, t9, k0, k1, gp, sp, fp, ra;
    uint64_t lo, hi;
    uint64_t pc;
    uint64_t status, cause, epc;
} mips_context_t;

typedef struct
{
    uint64_t base_addr;
    uint64_t size;
    uint64_t flags;
    uint64_t domain;
} mips_memory_region_t;

typedef struct
{
    uint64_t virtual_addr;
    uint64_t physical_addr;
    uint64_t size;
    uint64_t flags;
    uint64_t domain;
} mips_page_table_entry_t;

typedef struct
{
    uint32_t irq_number;
    uint32_t priority;
    uint32_t trigger_mode;
    void (*handler)(void);
    void *data;
} mips_irq_info_t;

typedef struct
{
    uint32_t timer_id;
    uint64_t frequency;
    uint32_t mode;
    uint64_t value;
    uint64_t reload_value;
    void (*callback)(void);
} mips_timer_info_t;

typedef struct
{
    uint32_t cache_level;
    uint64_t size;
    uint32_t line_size;
    uint32_t associativity;
    uint32_t write_policy;
} mips_cache_info_t;

typedef struct
{
    uint64_t vector_base;
    uint32_t vector_count;
    uint32_t priority_bits;
    bool external_enabled;
    bool timer_enabled;
} mips_interrupt_controller_t;

typedef struct
{
    uint32_t tlb_enabled;
    uint32_t tlb_entries;
    uint32_t tlb_ways;
    uint32_t domain_count;
    mips_memory_region_t regions[16];
} mips_tlb_context_t;

typedef struct
{
    uint32_t timer_enabled;
    uint32_t timer_count;
    uint64_t system_tick;
    mips_timer_info_t timers[8];
} mips_timer_context_t;

typedef struct
{
    uint32_t cache_enabled;
    uint32_t cache_levels;
    mips_cache_info_t caches[3];
} mips_cache_context_t;

typedef struct
{
    uint32_t power_state;
    uint64_t cpu_frequency;
    uint32_t voltage_level;
    uint32_t temperature;
} mips_power_context_t;

typedef struct
{
    uint32_t security_enabled;
    uint32_t secure_boot_enabled;
    uint32_t memory_protection_enabled;
    uint32_t aslr_enabled;
} mips_security_context_t;

typedef struct
{
    uint32_t virtualization_enabled;
    uint32_t hypervisor_mode;
    uint32_t vm_count;
    uint32_t nested_virtualization;
} mips_virtualization_context_t;

typedef struct
{
    uint32_t performance_enabled;
    uint32_t pmu_enabled;
    uint64_t cycle_counter;
    uint64_t instruction_counter;
} mips_performance_context_t;

typedef struct
{
    uint32_t debug_enabled;
    uint32_t breakpoint_count;
    uint32_t watchpoint_count;
    uint32_t etm_enabled;
} mips_debug_context_t;

typedef struct
{
    uint32_t msa_enabled;
    uint32_t dsp_enabled;
    uint32_t fpu_enabled;
    uint32_t crc_enabled;
} mips_extension_context_t;

// ============================================================================
// FUNCTION DECLARATIONS - CPU MANAGEMENT
// ============================================================================

// CPU mode and privilege functions
bool mips_is_privileged(void);
uint32_t mips_get_current_mode(void);

// CPU Control
void mips_cpu_init(void);
void mips_cpu_reset(void);
void mips_cpu_halt(void);
void mips_cpu_wait_for_interrupt(void);
uint32_t mips_cpu_get_id(void);
uint32_t mips_cpu_get_revision(void);
uint64_t mips_cpu_get_frequency(void);
void mips_cpu_set_frequency(uint64_t frequency);

// CPU Control
void mips_cpu_init(void);
void mips_cpu_reset(void);
void mips_cpu_halt(void);
void mips_cpu_wait_for_interrupt(void);
uint32_t mips_cpu_get_id(void);
uint32_t mips_cpu_get_revision(void);
uint64_t mips_cpu_get_frequency(void);
void mips_cpu_set_frequency(uint64_t frequency);

// CPU Context
void mips_cpu_save_context(mips_context_t *context);
void mips_cpu_restore_context(const mips_context_t *context);
void mips_cpu_switch_context(mips_context_t *old_context, mips_context_t *new_context);

// Context functions
void mips_save_context(mips_context_t *context);
void mips_restore_context(const mips_context_t *context);

// CPU Features
bool mips_cpu_supports_msa(void);
bool mips_cpu_supports_dsp(void);
bool mips_cpu_supports_fpu(void);
bool mips_cpu_supports_vz(void);
uint32_t mips_cpu_get_feature_flags(void);

// ============================================================================
// FUNCTION DECLARATIONS - MEMORY MANAGEMENT
// ============================================================================

// Memory barrier and atomic operations
void mips_memory_barrier(void);
void mips_read_barrier(void);
void mips_write_barrier(void);
void mips_instruction_barrier(void);

uint64_t mips_atomic_swap(uint64_t *addr, uint64_t new_value);
uint64_t mips_atomic_add(uint64_t *addr, uint64_t value);
uint64_t mips_atomic_sub(uint64_t *addr, uint64_t value);
uint64_t mips_atomic_and(uint64_t *addr, uint64_t mask);
uint64_t mips_atomic_or(uint64_t *addr, uint64_t mask);
uint64_t mips_atomic_xor(uint64_t *addr, uint64_t mask);

// Memory management functions
void mips_setup_memory(void);
void mips_memory_init(void);
void mips_memory_map_region(uint64_t virtual_addr, uint64_t physical_addr, uint64_t size, uint64_t flags);
void mips_memory_unmap_region(uint64_t virtual_addr);
uint64_t mips_memory_get_physical_address(uint64_t virtual_addr);

// Memory management setup
void mips_setup_memory_regions(void);
void mips_setup_memory_protection(void);

// Memory management initialization
void mips_memory_setup_kernel_regions(void);
void mips_memory_setup_user_regions(void);

// Memory management utilities
void mips_memory_clear_region(uint64_t start_addr, uint64_t end_addr);
void mips_memory_copy_region(uint64_t dest_addr, uint64_t src_addr, uint64_t size);
void mips_memory_set_region(uint64_t addr, uint8_t value, uint64_t size);

// Memory management validation
bool mips_memory_is_valid_address(uint64_t addr);
bool mips_memory_is_kernel_address(uint64_t addr);
bool mips_memory_is_user_address(uint64_t addr);

// TLB Control
void mips_tlb_init(void);
void mips_tlb_enable(void);
void mips_tlb_disable(void);
bool mips_tlb_is_enabled(void);

// TLB Operations
void mips_tlb_setup_page_tables(void);
void mips_tlb_map_page(uint64_t virtual_addr, uint64_t physical_addr, uint64_t flags);
void mips_tlb_unmap_page(uint64_t virtual_addr);
uint64_t mips_tlb_get_physical_address(uint64_t virtual_addr);

// Memory Regions
void mips_tlb_add_memory_region(const mips_memory_region_t *region);
void mips_tlb_remove_memory_region(uint64_t base_addr);
mips_memory_region_t *mips_tlb_find_memory_region(uint64_t addr);

// TLB Management
void mips_tlb_invalidate_all(void);
void mips_tlb_invalidate_entry(uint64_t addr);
void mips_tlb_invalidate_range(uint64_t start_addr, uint64_t end_addr);

// ============================================================================
// FUNCTION DECLARATIONS - INTERRUPT MANAGEMENT
// ============================================================================

// Interrupt Controller
void mips_interrupt_init(void);
void mips_interrupt_enable_irq(uint32_t irq_number);
void mips_interrupt_disable_irq(uint32_t irq_number);
bool mips_interrupt_is_irq_enabled(uint32_t irq_number);

// Interrupt Handlers
void mips_interrupt_register_handler(uint32_t irq_number, void (*handler)(void), void *data);
void mips_interrupt_unregister_handler(uint32_t irq_number);
void mips_interrupt_set_priority(uint32_t irq_number, uint32_t priority);

// Interrupt Control
void mips_interrupt_enable_external(void);
void mips_interrupt_disable_external(void);
void mips_interrupt_enable_timer(void);
void mips_interrupt_disable_timer(void);

// Interrupt Status
bool mips_is_external_enabled(void);
bool mips_is_timer_enabled(void);

// Interrupt Handler Registration
int mips_register_exception_handler(uint32_t exception_type, void (*handler)(void), void *data);
void mips_unregister_exception_handler(uint32_t exception_type);
int mips_register_irq_handler(uint32_t irq_number, void (*handler)(void), void *data);
void mips_unregister_irq_handler(uint32_t irq_number);

// Exception Handling
void mips_exception_init(void);
void mips_exception_register_handler(uint32_t exception_type, void (*handler)(void));
void mips_exception_unregister_handler(uint32_t exception_type);

// ============================================================================
// FUNCTION DECLARATIONS - TIMER MANAGEMENT
// ============================================================================

// Timer Control
void mips_timer_init(void);
void mips_timer_start(uint32_t timer_id);
void mips_timer_stop(uint32_t timer_id);
void mips_timer_reset(uint32_t timer_id);

// Timer Configuration
void mips_timer_set_frequency(uint32_t timer_id, uint64_t frequency);
void mips_timer_set_mode(uint32_t timer_id, uint32_t mode);
void mips_timer_set_callback(uint32_t timer_id, void (*callback)(void));

// Timer Values
uint64_t mips_timer_get_value(uint32_t timer_id);
uint64_t mips_timer_get_frequency(uint32_t timer_id);
uint64_t mips_timer_get_system_tick(void);

// CP0 Timer Functions
uint64_t mips_cp0_count_get_value(void);
uint64_t mips_cp0_compare_get_value(void);
void mips_cp0_compare_set_value(uint64_t value);
uint64_t mips_cp0_count_get_frequency(void);

// System timer control functions
void mips_system_timer_start(void);
void mips_system_timer_stop(void);
void mips_system_timer_reset(void);

// Periodic timer control functions
void mips_periodic_timer_start(uint32_t timer_id);
void mips_periodic_timer_stop(uint32_t timer_id);
void mips_periodic_timer_reset(uint32_t timer_id);
void mips_periodic_timer_set_frequency(uint32_t timer_id, uint64_t frequency);
void mips_periodic_timer_set_callback(uint32_t timer_id, void (*callback)(void));

// High-resolution timer functions
uint64_t mips_high_res_timer_get_cycles(void);
uint64_t mips_high_res_timer_get_frequency(void);
uint64_t mips_high_res_timer_get_microseconds(void);
uint64_t mips_high_res_timer_get_milliseconds(void);
void mips_high_res_timer_start_measurement(void);
uint64_t mips_high_res_timer_end_measurement(void);
uint64_t mips_high_res_timer_measure_microseconds(void);
uint64_t mips_high_res_timer_measure_milliseconds(void);

// Timer utility functions
void mips_delay_cycles(uint64_t cycles);
void mips_delay_microseconds(uint64_t microseconds);
void mips_delay_milliseconds(uint64_t milliseconds);

// Timer tick processing
void mips_timer_tick(void);

// Timer status and control
bool mips_timer_is_enabled(uint32_t timer_id);
uint64_t mips_timer_get_value(uint32_t timer_id);
uint64_t mips_timer_get_frequency(uint32_t timer_id);
uint64_t mips_timer_get_system_tick(void);
uint32_t mips_timer_get_active_count(void);

// Timer calibration
void mips_timer_calibrate(void);

// Timer power management
void mips_timer_enter_low_power_mode(void);
void mips_timer_exit_low_power_mode(void);

// Timer debug and diagnostics
void mips_timer_print_status(void);
void mips_timer_reset_all(void);

// ============================================================================
// FUNCTION DECLARATIONS - CACHE MANAGEMENT
// ============================================================================

// Cache management functions
void mips_setup_cache(void);
void mips_cache_init(void);
void mips_cache_enable(void);
void mips_cache_disable(void);
bool mips_cache_is_enabled(void);

// Cache Operations
void mips_cache_flush_all(void);
void mips_cache_invalidate_all(void);
void mips_cache_flush_range(uint64_t start_addr, uint64_t end_addr);
void mips_cache_invalidate_range(uint64_t start_addr, uint64_t end_addr);

// Cache Information
mips_cache_info_t *mips_cache_get_info(uint32_t level);
uint32_t mips_cache_get_line_size(void);
uint64_t mips_cache_get_size(uint32_t level);

// ============================================================================
// FUNCTION DECLARATIONS - POWER MANAGEMENT
// ============================================================================

// Power Control
void mips_power_init(void);
void mips_power_set_state(uint32_t state);
uint32_t mips_power_get_state(void);
void mips_power_set_cpu_frequency(uint64_t frequency);

// Power Monitoring
uint64_t mips_power_get_cpu_frequency(void);
uint32_t mips_power_get_voltage_level(void);
uint32_t mips_power_get_temperature(void);
uint32_t mips_power_get_consumption(void);

// ============================================================================
// FUNCTION DECLARATIONS - SECURITY
// ============================================================================

// Security management functions
void mips_setup_security(void);
void mips_security_init(void);
void mips_security_enable_secure_boot(void);
void mips_security_disable_secure_boot(void);
bool mips_security_is_secure_boot_enabled(void);

// Memory Protection
void mips_security_enable_memory_protection(void);
void mips_security_disable_memory_protection(void);
bool mips_security_is_memory_protection_enabled(void);

// ASLR
void mips_security_enable_aslr(void);
void mips_security_disable_aslr(void);
bool mips_security_is_aslr_enabled(void);

// ============================================================================
// FUNCTION DECLARATIONS - VIRTUALIZATION
// ============================================================================

// Virtualization Control
void mips_virtualization_init(void);
void mips_virtualization_enable(void);
void mips_virtualization_disable(void);
bool mips_virtualization_is_enabled(void);

// Virtual Machine Management
uint32_t mips_virtualization_create_vm(void);
void mips_virtualization_destroy_vm(uint32_t vm_id);
void mips_virtualization_start_vm(uint32_t vm_id);
void mips_virtualization_stop_vm(uint32_t vm_id);

// ============================================================================
// FUNCTION DECLARATIONS - PERFORMANCE MONITORING
// ============================================================================

// Performance management functions
void mips_setup_performance(void);
void mips_performance_init(void);
void mips_performance_enable_pmu(void);
void mips_performance_disable_pmu(void);
bool mips_performance_is_pmu_enabled(void);

// Performance Control
void mips_performance_init(void);
void mips_performance_enable_pmu(void);
void mips_performance_disable_pmu(void);
bool mips_performance_is_pmu_enabled(void);

// Performance Counters
uint64_t mips_performance_get_cycle_counter(void);
uint64_t mips_performance_get_instruction_counter(void);
void mips_performance_reset_counters(void);

// Performance aliases
uint64_t mips_perf_get_cycle_counter(void);
uint64_t mips_perf_get_instruction_counter(void);

// ============================================================================
// FUNCTION DECLARATIONS - DEBUG SUPPORT
// ============================================================================

// Debug management functions
void mips_setup_debug(void);
void mips_debug_init(void);
void mips_debug_enable(void);
void mips_debug_disable(void);
bool mips_debug_is_enabled(void);

// Debug Control
void mips_debug_init(void);
void mips_debug_enable(void);
void mips_debug_disable(void);
bool mips_debug_is_enabled(void);

// Breakpoints and Watchpoints
void mips_debug_set_breakpoint(uint64_t addr);
void mips_debug_clear_breakpoint(uint64_t addr);
void mips_debug_set_watchpoint(uint64_t addr, uint64_t size, uint32_t type);
void mips_debug_clear_watchpoint(uint64_t addr);

// ============================================================================
// FUNCTION DECLARATIONS - EXTENSIONS
// ============================================================================

// Extension management functions
void mips_setup_extensions(void);
void mips_msa_init(void);
bool mips_msa_is_available(void);
void mips_msa_enable(void);
void mips_msa_disable(void);

// MSA Support
void mips_msa_init(void);
bool mips_msa_is_available(void);
void mips_msa_enable(void);
void mips_msa_disable(void);

// DSP Support
void mips_dsp_init(void);
bool mips_dsp_is_available(void);
void mips_dsp_enable(void);
void mips_dsp_disable(void);

// FPU Support
void mips_fpu_init(void);
bool mips_fpu_is_available(void);
void mips_fpu_enable(void);
void mips_fpu_disable(void);

// CRC Support
void mips_crc_init(void);
bool mips_crc_is_available(void);
void mips_crc_enable(void);
void mips_crc_disable(void);

// CRC support check
bool mips_cpu_supports_crc(void);

// ============================================================================
// FUNCTION DECLARATIONS - INTERRUPT MANAGEMENT
// ============================================================================

// Interrupt dispatching
void mips_dispatch_irq(uint32_t irq_number);
void mips_dispatch_exception(uint32_t exception_type);

// Interrupt acknowledgment
void mips_interrupt_acknowledge(uint32_t irq_number);
void mips_timer_interrupt_acknowledge(void);

// Interrupt statistics
uint64_t mips_get_irq_count(uint32_t irq_number);
uint64_t mips_get_exception_count(uint32_t exception_type);
uint64_t mips_get_total_irqs(void);
uint64_t mips_get_total_exceptions(void);
void mips_reset_interrupt_statistics(void);

// Interrupt priority management
uint32_t mips_get_highest_priority_irq(void);
void mips_set_global_priority_threshold(uint32_t threshold);

// Interrupt controller status
mips_interrupt_controller_t *mips_get_interrupt_controller(void);
bool mips_is_interrupt_pending(void);
uint32_t mips_get_pending_interrupts(void);



// ============================================================================
// FUNCTION DECLARATIONS - EXCEPTION HANDLING
// ============================================================================

// Exception handler functions
void mips_handle_cache_error(void);
void mips_handle_general_exception(void);
void mips_handle_load_store_error(void);
void mips_handle_instruction_bus_error(void);
void mips_handle_data_bus_error(void);
void mips_handle_breakpoint(void);
void mips_handle_reserved_instruction(void);
void mips_handle_coprocessor_unusable(void);
void mips_handle_overflow(void);
void mips_handle_trap(void);
void mips_handle_reserved(void);

// Exception handlers
void mips_handle_undefined_instruction(void);
void mips_handle_software_interrupt(void);
void mips_handle_prefetch_abort(void);
void mips_handle_data_abort(void);
void mips_handle_reserved_exception(void);
void mips_handle_irq(void);
void mips_handle_fiq(void);

// Exception handlers
void mips_handle_undefined_instruction(void);
void mips_handle_software_interrupt(void);
void mips_handle_prefetch_abort(void);
void mips_handle_data_abort(void);
void mips_handle_reserved_exception(void);
void mips_handle_irq(void);
void mips_handle_fiq(void);

// IRQ handlers
void mips_handle_irq_0(void);
void mips_handle_irq_1(void);
void mips_handle_irq_2(void);
void mips_handle_irq_3(void);
void mips_handle_irq_4(void);
void mips_handle_irq_5(void);
void mips_handle_irq_6(void);
void mips_handle_timer_interrupt(void);

// System call handlers
void mips_syscall_exit(void);
void mips_syscall_read(void);
void mips_syscall_write(void);
void mips_syscall_open(void);
void mips_syscall_close(void);
void mips_syscall_fork(void);
void mips_syscall_exec(void);
void mips_syscall_wait(void);
void mips_syscall_kill(void);
void mips_syscall_getpid(void);
void mips_syscall_sleep(void);
void mips_syscall_time(void);
void mips_syscall_brk(void);
void mips_syscall_mmap(void);
void mips_syscall_munmap(void);
void mips_syscall_socket(void);
void mips_syscall_bind(void);
void mips_syscall_listen(void);
void mips_syscall_accept(void);
void mips_syscall_connect(void);
void mips_syscall_send(void);
void mips_syscall_recv(void);

// Exception and interrupt statistics
uint64_t mips_get_exception_counter(uint32_t exception_type);
uint64_t mips_get_interrupt_counter(uint32_t irq_number);
void mips_reset_exception_counters(void);
void mips_reset_interrupt_counters(void);

// Error handling
void mips_handle_error(uint32_t error_code, const char *error_message);
void mips_handle_panic(const char *panic_message);

// ============================================================================
// FUNCTION DECLARATIONS - CONTEXT SWITCHING
// ============================================================================

// Context switching functions
void mips_context_switch(mips_context_t *old_context, mips_context_t *new_context);
void mips_interrupt_context_switch(mips_context_t *old_context, mips_context_t *new_context);
void mips_exception_context_switch(mips_context_t *old_context, mips_context_t *new_context);

// Mode switching functions
void mips_enter_user_mode(void);
void mips_enter_kernel_mode(void);

// Utility functions
uint64_t mips_get_stack_pointer(void);
void mips_set_stack_pointer(uint64_t sp);
uint64_t mips_get_frame_pointer(void);
void mips_set_frame_pointer(uint64_t fp);

// ============================================================================
// FUNCTION DECLARATIONS - ENTRY POINTS
// ============================================================================

// Entry point functions
void mips_entry_point(void);
void mips_exception_entry(void);
void mips_interrupt_entry(void);
void mips_syscall_entry(void);

// Setup functions
void mips_setup_stacks(void);
void mips_setup_exception_vectors(void);
void mips_setup_interrupts(void);
void mips_setup_memory(void);
void mips_setup_timers(void);
void mips_setup_cache(void);
void mips_setup_security(void);
void mips_setup_performance(void);
void mips_setup_debug(void);
void mips_setup_extensions(void);







// ============================================================================
// FUNCTION DECLARATIONS - ASSEMBLY FUNCTIONS
// ============================================================================

// Assembly CPU control functions
void mips_cpu_enable_interrupts_asm(void);
void mips_cpu_disable_interrupts_asm(void);
void mips_cpu_wait_for_interrupt_asm(void);
void mips_cpu_nop_asm(void);
void mips_cpu_sync_asm(void);
void mips_cpu_sync_p_asm(void);
void mips_cpu_sync_s_asm(void);

// Assembly CP0 register access functions
void mips_cpu_read_cp0_register_asm(void);
void mips_cpu_write_cp0_register_asm(void);
void mips_cpu_read_cp0_count_asm(void);
void mips_cpu_read_cp0_compare_asm(void);
void mips_cpu_write_cp0_compare_asm(void);
void mips_cpu_read_cp0_status_asm(void);
void mips_cpu_write_cp0_status_asm(void);
void mips_cpu_read_cp0_cause_asm(void);
void mips_cpu_read_cp0_epc_asm(void);
void mips_cpu_write_cp0_epc_asm(void);

// Assembly MMU control functions
void mips_tlb_invalidate_all_asm(void);
void mips_tlb_invalidate_entry_asm(void);
void mips_tlb_read_index_asm(void);
void mips_tlb_write_index_asm(void);
void mips_tlb_read_random_asm(void);
void mips_tlb_read_entrylo0_asm(void);
void mips_tlb_write_entrylo0_asm(void);
void mips_tlb_read_entrylo1_asm(void);
void mips_tlb_write_entrylo1_asm(void);
void mips_tlb_read_entryhi_asm(void);
void mips_tlb_write_entryhi_asm(void);

// Assembly cache control functions
void mips_cache_flush_all_asm(void);
void mips_cache_invalidate_all_asm(void);
void mips_cache_flush_range_asm(void);
void mips_cache_invalidate_range_asm(void);
void mips_cache_flush_line_asm(void);
void mips_cache_invalidate_line_asm(void);

// Assembly performance monitoring functions
void mips_perf_read_cycle_counter_asm(void);
void mips_perf_read_instruction_counter_asm(void);
void mips_perf_reset_counters_asm(void);

// Assembly NEON/VFP control functions
void mips_fpu_enable_asm(void);
void mips_fpu_disable_asm(void);
void mips_msa_enable_asm(void);
void mips_msa_disable_asm(void);

// Assembly timing and delay functions
void mips_delay_cycles_asm(void);
void mips_delay_microseconds_asm(void);
void mips_delay_milliseconds_asm(void);

// Assembly memory barrier functions
void mips_memory_barrier_asm(void);
void mips_read_barrier_asm(void);
void mips_write_barrier_asm(void);
void mips_instruction_barrier_asm(void);

// Assembly atomic operations
void mips_atomic_swap_asm(void);
void mips_atomic_add_asm(void);
void mips_atomic_sub_asm(void);
void mips_atomic_and_asm(void);
void mips_atomic_or_asm(void);
void mips_atomic_xor_asm(void);

// ============================================================================
// FUNCTION DECLARATIONS - SYSTEM CALL HANDLING
// ============================================================================

// System call handling
void mips_handle_system_call(void);
void mips_handle_fast_system_call(void);
void mips_execute_system_call(uint32_t syscall_number);

// System call handler registration
int mips_register_syscall_handler(uint32_t syscall_number, void (*handler)(void));
void mips_unregister_syscall_handler(uint32_t syscall_number);

// ============================================================================
// FUNCTION DECLARATIONS - SYSTEM INITIALIZATION
// ============================================================================

// System functions
uint64_t mips_get_system_ticks(void);
uint64_t mips_get_system_uptime_ms(void);

// Device driver initialization
void mips_device_drivers_init(void);
void mips_file_system_init(void);
void mips_network_stack_init(void);
void mips_user_space_init(void);

// Early Initialization
void mips_early_init(void);
void mips_system_init(void);
void mips_late_init(void);

// System Control
void mips_system_monitor(void);
void mips_system_test(void);
void mips_main_loop(void);
void mips_main(void);



// Bootstrap and Shutdown
void mips_bootstrap(void);
void mips_reset(void);
void mips_shutdown(void);



// ============================================================================
// MAIN ENTRY POINT
// ============================================================================

// Main entry point
int main(void);

// ============================================================================
// INLINE FUNCTIONS
// ============================================================================

static inline void mips_cpu_enable_interrupts(void)
{
    __asm__ volatile("ei");
}

static inline void mips_cpu_disable_interrupts(void)
{
    __asm__ volatile("di");
}

static inline void mips_cpu_nop(void)
{
    __asm__ volatile("nop");
}

static inline void mips_cpu_sync(void)
{
    __asm__ volatile("sync");
}

static inline void mips_cpu_sync_p(void)
{
    __asm__ volatile("sync.p");
}

static inline void mips_cpu_sync_s(void)
{
    __asm__ volatile("sync.s");
}

static inline uint64_t mips_cpu_read_cp0_register(uint32_t reg)
{
    uint64_t value;
    __asm__ volatile("dmfc0 %0, $%1" : "=r"(value) : "i"(reg));
    return value;
}

static inline void mips_cpu_write_cp0_register(uint32_t reg, uint64_t value)
{
    __asm__ volatile("dmtc0 %0, $%1" : : "r"(value), "i"(reg));
}

static inline uint64_t mips_cpu_read_cp0_count(void)
{
    return mips_cpu_read_cp0_register(MIPS_CP0_COUNT);
}

static inline uint64_t mips_cpu_read_cp0_compare(void)
{
    return mips_cpu_read_cp0_register(MIPS_CP0_COMPARE);
}

static inline void mips_cpu_write_cp0_compare(uint64_t value)
{
    mips_cpu_write_cp0_register(MIPS_CP0_COMPARE, value);
}

static inline uint64_t mips_cpu_read_cp0_status(void)
{
    return mips_cpu_read_cp0_register(MIPS_CP0_STATUS);
}

static inline void mips_cpu_write_cp0_status(uint64_t status)
{
    mips_cpu_write_cp0_register(MIPS_CP0_STATUS, status);
}

static inline uint64_t mips_cpu_read_cp0_cause(void)
{
    return mips_cpu_read_cp0_register(MIPS_CP0_CAUSE);
}

static inline uint64_t mips_cpu_read_cp0_epc(void)
{
    return mips_cpu_read_cp0_register(MIPS_CP0_EPC);
}

static inline void mips_cpu_write_cp0_epc(uint64_t epc)
{
    mips_cpu_write_cp0_register(MIPS_CP0_EPC, epc);
}

// ============================================================================
// MACROS
// ============================================================================

#define MIPS_CPU_ENTER_CRITICAL_SECTION() \
    do {                                  \
        uint64_t status = mips_cpu_read_cp0_status(); \
        mips_cpu_disable_interrupts();    \
    } while (0)

#define MIPS_CPU_EXIT_CRITICAL_SECTION() \
    do {                                 \
        mips_cpu_write_cp0_status(status); \
    } while (0)

#define MIPS_CPU_ENTER_EXCEPTION_SECTION() \
    do {                                   \
        uint64_t status = mips_cpu_read_cp0_status(); \
        status |= MIPS_STATUS_EXL;         \
        mips_cpu_write_cp0_status(status); \
    } while (0)

#define MIPS_CPU_EXIT_EXCEPTION_SECTION() \
    do {                                  \
        uint64_t status = mips_cpu_read_cp0_status(); \
        status &= ~MIPS_STATUS_EXL;       \
        mips_cpu_write_cp0_status(status); \
    } while (0)

#endif // ORION_MIPS_ARCH_H
