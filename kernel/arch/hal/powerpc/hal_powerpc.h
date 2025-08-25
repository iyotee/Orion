/*
 * Orion Operating System - PowerPC HAL Header
 *
 * Hardware Abstraction Layer header for PowerPC architecture.
 * This header declares the HAL interface functions that use
 * the existing kernel/arch/powerpc implementation.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_HAL_POWERPC_H
#define ORION_HAL_POWERPC_H

#include "orion_hal.h"
#include "hal_common.h"
#include <orion/types.h>

// Forward declarations
struct thread;
struct ipc_msg;

// HAL function declarations - these use existing kernel/arch code

// Memory management
int hal_powerpc_mmu_init(void);
int hal_powerpc_mmu_map(vaddr_t va, paddr_t pa, uint64_t flags);
int hal_powerpc_mmu_unmap(vaddr_t va, size_t len);
int hal_powerpc_mmu_protect(vaddr_t va, size_t len, uint64_t flags);
paddr_t hal_powerpc_mmu_translate(vaddr_t va);

// Context switching
void hal_powerpc_context_switch(struct thread *prev, struct thread *next);
void hal_powerpc_context_save(struct thread *thread);
void hal_powerpc_context_restore(struct thread *thread);

// IPC
ssize_t hal_powerpc_ipc_fast_send(struct ipc_msg *msg, struct thread *target);
ssize_t hal_powerpc_ipc_fast_recv(struct ipc_msg *msg, struct thread *source);
int hal_powerpc_ipc_setup_fast_path(struct thread *thread);

// Interrupt handling
int hal_powerpc_irq_register(uint32_t irq, irq_handler_t handler, void *data);
int hal_powerpc_irq_unregister(uint32_t irq);
void hal_powerpc_irq_enable(uint32_t irq);
void hal_powerpc_irq_disable(uint32_t irq);
void hal_powerpc_irq_ack(uint32_t irq);

// Timer management
int hal_powerpc_timer_init(uint64_t frequency_hz);
uint64_t hal_powerpc_timer_get_ticks(void);
void hal_powerpc_timer_set_oneshot(uint64_t delay_ticks, timer_callback_t callback);
void hal_powerpc_timer_cancel(void);

// CPU management
int hal_powerpc_cpu_init(uint32_t cpu_id);
void hal_powerpc_cpu_idle(void);
void hal_powerpc_cpu_wake(uint32_t cpu_id);
uint32_t hal_powerpc_cpu_get_count(void);
uint32_t hal_powerpc_cpu_get_current(void);

// Cache management
void hal_powerpc_cache_invalidate(void *addr, size_t size);
void hal_powerpc_cache_clean(void *addr, size_t size);
void hal_powerpc_cache_flush(void *addr, size_t size);
void hal_powerpc_cache_sync(void);

// Performance monitoring
int hal_powerpc_perf_init(void);
uint64_t hal_powerpc_perf_read_counter(uint32_t counter_id);
int hal_powerpc_perf_set_event(uint32_t counter_id, uint32_t event_id);
void hal_powerpc_perf_enable(uint32_t counter_id);
void hal_powerpc_perf_disable(uint32_t counter_id);

// Security features
int hal_powerpc_secure_init(void);
int hal_powerpc_secure_measure(const void *data, size_t size, uint8_t *hash);
int hal_powerpc_secure_verify(const void *data, size_t size, const uint8_t *hash);
int hal_powerpc_secure_enclave_create(size_t size, void **enclave);
int hal_powerpc_secure_enclave_destroy(void *enclave);

// Power management
int hal_powerpc_power_init(void);
int hal_powerpc_power_set_state(power_state_t state);
int hal_powerpc_power_get_state(void);
int hal_powerpc_power_set_frequency(uint32_t cpu_id, uint32_t frequency_mhz);

// Debug and tracing
int hal_powerpc_debug_init(void);
void hal_powerpc_debug_break(void);
void hal_powerpc_debug_trace(const char *message);
int hal_powerpc_debug_dump_registers(void);

// Architecture-specific extensions
int hal_powerpc_arch_extension_call(uint32_t extension_id, void *args);

// HAL initialization and access
int hal_powerpc_init(void);
void hal_powerpc_cleanup(void);
struct orion_hal_arch *hal_powerpc_get_interface(void);

#endif /* ORION_HAL_POWERPC_H */
