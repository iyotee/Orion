/*
 * ORION OS - s390x Architecture Header
 *
 * Main header for IBM s390x architecture support
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_S390X_ARCH_H
#define ORION_S390X_ARCH_H

#include "config.h"
#include <orion/types.h>
#include <stdint.h>

// Forward declarations
struct thread;
struct process;

// ============================================================================
// ARCHITECTURE INITIALIZATION
// ============================================================================

int s390x_arch_init(void);
void s390x_arch_cleanup(void);
int s390x_detect_cpu_features(void);
void s390x_print_cpu_info(void);

// ============================================================================
// CPU MANAGEMENT
// ============================================================================

int s390x_cpu_init(uint32_t cpu_id);
void s390x_cpu_idle(void);
void s390x_cpu_wake(uint32_t cpu_id);
uint32_t s390x_cpu_get_count(void);
uint32_t s390x_cpu_get_current(void);
void s390x_cpu_set_frequency(uint32_t cpu_id, uint32_t freq_mhz);

// ============================================================================
// MEMORY MANAGEMENT
// ============================================================================

int s390x_mmu_init(void);
int s390x_mmu_map_page(vaddr_t va, paddr_t pa, uint64_t flags);
int s390x_mmu_unmap_pages(vaddr_t va, size_t len);
int s390x_mmu_protect_pages(vaddr_t va, size_t len, uint64_t flags);
paddr_t s390x_mmu_translate_address(vaddr_t va);
void s390x_mmu_invalidate_tlb(vaddr_t va, size_t len);

// ============================================================================
// INTERRUPT SYSTEM
// ============================================================================

int s390x_interrupt_register(uint32_t irq, irq_handler_t handler, void *data);
int s390x_interrupt_unregister(uint32_t irq);
void s390x_interrupt_enable(uint32_t irq);
void s390x_interrupt_disable(uint32_t irq);
void s390x_interrupt_ack(uint32_t irq);
void s390x_interrupt_init(void);

// ============================================================================
// TIMER SYSTEM
// ============================================================================

int s390x_timer_init(uint64_t frequency_hz);
uint64_t s390x_timer_get_ticks(void);
void s390x_timer_set_oneshot(uint64_t delay_ticks, timer_callback_t callback);
void s390x_timer_cancel(void);
void s390x_timer_set_frequency(uint64_t freq_hz);

// ============================================================================
// CACHE MANAGEMENT
// ============================================================================

void s390x_cache_invalidate(void *addr, size_t size);
void s390x_cache_clean(void *addr, size_t size);
void s390x_cache_flush(void *addr, size_t size);
void s390x_cache_sync(void);
void s390x_cache_init(void);

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

int s390x_performance_init(void);
uint64_t s390x_performance_read_counter(uint32_t counter_id);
int s390x_performance_set_event(uint32_t counter_id, uint32_t event_id);
void s390x_performance_enable_counter(uint32_t counter_id);
void s390x_performance_disable_counter(uint32_t counter_id);

// ============================================================================
// SECURITY FEATURES
// ============================================================================

int s390x_security_init(void);
int s390x_security_measure(const void *data, size_t size, uint8_t *hash);
int s390x_security_verify(const void *data, size_t size, const uint8_t *hash);
int s390x_security_enclave_create(size_t size, void **enclave);
int s390x_security_enclave_destroy(void *enclave);

// ============================================================================
// POWER MANAGEMENT
// ============================================================================

int s390x_power_init(void);
int s390x_power_set_state(power_state_t state);
int s390x_power_get_state(void);
int s390x_power_set_frequency(uint32_t cpu_id, uint32_t freq_mhz);

// ============================================================================
// DEBUG AND TRACING
// ============================================================================

int s390x_debug_init(void);
void s390x_debug_break(void);
void s390x_debug_trace(const char *message);
int s390x_debug_dump_registers(void);

// ============================================================================
// CONTEXT SWITCHING
// ============================================================================

void s390x_context_switch(struct thread *prev, struct thread *next);
void s390x_context_save(struct thread *thread);
void s390x_context_restore(struct thread *thread);

// ============================================================================
// IPC FAST PATH
// ============================================================================

ssize_t s390x_ipc_fast_send(struct ipc_msg *msg, struct thread *target);
ssize_t s390x_ipc_fast_recv(struct ipc_msg *msg, struct thread *source);
int s390x_ipc_setup_fast_path(struct thread *thread);

// ============================================================================
// ARCHITECTURE EXTENSIONS
// ============================================================================

int s390x_z196_extension_call(void *args);
int s390x_zec12_extension_call(void *args);
int s390x_z13_extension_call(void *args);
int s390x_z14_extension_call(void *args);
int s390x_z15_extension_call(void *args);
int s390x_z16_extension_call(void *args);
int s390x_vector_extension_call(void *args);
int s390x_crypto_extension_call(void *args);
int s390x_decimal_extension_call(void *args);
int s390x_tx_extension_call(void *args);

#endif /* ORION_S390X_ARCH_H */
