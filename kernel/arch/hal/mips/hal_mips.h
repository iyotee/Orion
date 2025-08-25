/*
 * Orion Operating System - MIPS HAL Header
 *
 * Hardware Abstraction Layer header for MIPS architecture.
 * This header declares the HAL interface functions that use
 * the existing kernel/arch/mips implementation.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_HAL_MIPS_H
#define ORION_HAL_MIPS_H

#include "orion_hal.h"
#include "hal_common.h"
#include <orion/types.h>

// Forward declarations
struct thread;
struct ipc_msg;

// HAL function declarations - these use existing kernel/arch code

// Memory management
int hal_mips_mmu_init(void);
int hal_mips_mmu_map(vaddr_t va, paddr_t pa, uint64_t flags);
int hal_mips_mmu_unmap(vaddr_t va, size_t len);
int hal_mips_mmu_protect(vaddr_t va, size_t len, uint64_t flags);
int hal_mips_mmu_get_mapping(vaddr_t va, paddr_t *pa, uint64_t *flags);
int hal_mips_mmu_flush_tlb(void);
int hal_mips_mmu_flush_tlb_range(vaddr_t va, size_t len);
int hal_mips_mmu_set_kernel_mapping(vaddr_t va, paddr_t pa, size_t len, uint64_t flags);

// Interrupt handling
int hal_mips_interrupt_init(void);
int hal_mips_interrupt_enable(uint32_t irq);
int hal_mips_interrupt_disable(uint32_t irq);
int hal_mips_interrupt_ack(uint32_t irq);
int hal_mips_interrupt_set_handler(uint32_t irq, void (*handler)(void));
int hal_mips_interrupt_set_priority(uint32_t irq, uint32_t priority);
int hal_mips_interrupt_get_pending(uint32_t irq);
int hal_mips_interrupt_clear_pending(uint32_t irq);

// Timer management
int hal_mips_timer_init(void);
int hal_mips_timer_set_interval(uint64_t interval_ns);
int hal_mips_timer_get_current(void);
int hal_mips_timer_enable(void);
int hal_mips_timer_disable(void);
int hal_mips_timer_set_callback(void (*callback)(void));
int hal_mips_timer_get_frequency(void);

// Cache management
int hal_mips_cache_init(void);
int hal_mips_cache_invalidate_data(vaddr_t va, size_t len);
int hal_mips_cache_clean_data(vaddr_t va, size_t len);
int hal_mips_cache_flush_data(vaddr_t va, size_t len);
int hal_mips_cache_invalidate_instruction(vaddr_t va, size_t len);
int hal_mips_cache_sync(void);

// CPU management
int hal_mips_cpu_init(void);
int hal_mips_cpu_get_id(void);
int hal_mips_cpu_get_count(void);
int hal_mips_cpu_start(uint32_t cpu_id, vaddr_t entry_point);
int hal_mips_cpu_stop(uint32_t cpu_id);
int hal_mips_cpu_wait(uint32_t cpu_id);
int hal_mips_cpu_get_features(uint64_t *features);

// Performance monitoring
int hal_mips_performance_init(void);
int hal_mips_performance_start_counter(uint32_t counter_id);
int hal_mips_performance_stop_counter(uint32_t counter_id);
int hal_mips_performance_read_counter(uint32_t counter_id, uint64_t *value);
int hal_mips_performance_set_event(uint32_t counter_id, uint32_t event_id);
int hal_mips_performance_get_cycle_count(uint64_t *cycles);
int hal_mips_performance_get_instruction_count(uint64_t *instructions);

// Power management
int hal_mips_power_init(void);
int hal_mips_power_set_state(uint32_t state);
int hal_mips_power_get_state(uint32_t *state);
int hal_mips_power_sleep(uint64_t duration_ns);
int hal_mips_power_wake(void);
int hal_mips_power_set_frequency(uint32_t cpu_id, uint32_t frequency_mhz);

// Security features
int hal_mips_security_init(void);
int hal_mips_security_set_trustzone(uint32_t enabled);
int hal_mips_security_get_trustzone(uint32_t *enabled);
int hal_mips_security_set_secure_world(uint32_t enabled);
int hal_mips_security_get_secure_world(uint32_t *enabled);

// Debug support
int hal_mips_debug_init(void);
int hal_mips_debug_set_breakpoint(vaddr_t va, uint32_t type);
int hal_mips_debug_clear_breakpoint(vaddr_t va);
int hal_mips_debug_set_watchpoint(vaddr_t va, size_t len, uint32_t type);
int hal_mips_debug_clear_watchpoint(vaddr_t va);
int hal_mips_debug_step(void);
int hal_mips_debug_continue(void);

// Context switching
int hal_mips_context_switch(struct thread *from, struct thread *to);
int hal_mips_context_save(struct thread *thread);
int hal_mips_context_restore(struct thread *thread);
int hal_mips_context_init(struct thread *thread, vaddr_t entry_point, vaddr_t stack_ptr);

// IPC fast path
int hal_mips_ipc_fast_path(struct ipc_msg *msg);
int hal_mips_ipc_fast_path_setup(void);
int hal_mips_ipc_fast_path_cleanup(void);

// System calls
int hal_mips_syscall_init(void);
int hal_mips_syscall_set_handler(uint32_t syscall_num, void (*handler)(void));
int hal_mips_syscall_get_handler(uint32_t syscall_num, void (**handler)(void));

// Initialization and cleanup
int hal_mips_init(void);
int hal_mips_cleanup(void);
int hal_mips_get_arch_info(struct orion_arch_info *info);

#endif // ORION_HAL_MIPS_H
