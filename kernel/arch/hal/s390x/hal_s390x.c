/*
 * Orion Operating System - s390x HAL Implementation
 *
 * Hardware Abstraction Layer implementation for s390x architecture.
 * This implementation bridges the HAL interface with the existing
 * kernel/arch/s390x code.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "hal_s390x.h"
#include "hal_s390x_common.h"
#include "../../s390x/arch.h"
#include "../../s390x/mmu.h"
#include "../../s390x/interrupts.h"
#include "../../s390x/timer.h"
#include "../../s390x/cache.h"
#include "../../s390x/performance.h"
#include "../../s390x/security.h"
#include "../../s390x/power.h"
#include "../../s390x/debug.h"
#include <orion/error.h>
#include <orion/thread.h>
#include <orion/ipc.h>

// s390x HAL context
static struct orion_hal_arch s390x_hal_arch = {
    .name = "s390x",
    .arch_id = ORION_ARCH_S390X,
    .version = 1,

    // Memory management
    .mmu_init = hal_s390x_mmu_init,
    .mmu_map = hal_s390x_mmu_map,
    .mmu_unmap = hal_s390x_mmu_unmap,
    .mmu_protect = hal_s390x_mmu_protect,
    .mmu_translate = hal_s390x_mmu_translate,

    // Context switching
    .context_switch = hal_s390x_context_switch,
    .context_save = hal_s390x_context_save,
    .context_restore = hal_s390x_context_restore,

    // IPC
    .ipc_fast_send = hal_s390x_ipc_fast_send,
    .ipc_fast_recv = hal_s390x_ipc_fast_recv,
    .ipc_setup_fast_path = hal_s390x_ipc_setup_fast_path,

    // Interrupt handling
    .irq_register = hal_s390x_irq_register,
    .irq_unregister = hal_s390x_irq_unregister,
    .irq_enable = hal_s390x_irq_enable,
    .irq_disable = hal_s390x_irq_disable,
    .irq_ack = hal_s390x_irq_ack,

    // Timer management
    .timer_init = hal_s390x_timer_init,
    .timer_get_ticks = hal_s390x_timer_get_ticks,
    .timer_set_oneshot = hal_s390x_timer_set_oneshot,
    .timer_cancel = hal_s390x_timer_cancel,

    // CPU management
    .cpu_init = hal_s390x_cpu_init,
    .cpu_idle = hal_s390x_cpu_idle,
    .cpu_wake = hal_s390x_cpu_wake,
    .cpu_get_count = hal_s390x_cpu_get_count,
    .cpu_get_current = hal_s390x_cpu_get_current,

    // Cache management
    .cache_invalidate = hal_s390x_cache_invalidate,
    .cache_clean = hal_s390x_cache_clean,
    .cache_flush = hal_s390x_cache_flush,
    .cache_sync = hal_s390x_cache_sync,

    // Performance monitoring
    .perf_init = hal_s390x_perf_init,
    .perf_read_counter = hal_s390x_perf_read_counter,
    .perf_set_event = hal_s390x_perf_set_event,
    .perf_enable = hal_s390x_perf_enable,
    .perf_disable = hal_s390x_perf_disable,

    // Security features
    .secure_init = hal_s390x_secure_init,
    .secure_measure = hal_s390x_secure_measure,
    .secure_verify = hal_s390x_secure_verify,
    .secure_enclave_create = hal_s390x_secure_enclave_create,
    .secure_enclave_destroy = hal_s390x_secure_enclave_destroy,

    // Power management
    .power_init = hal_s390x_power_init,
    .power_set_state = hal_s390x_power_set_state,
    .power_get_state = hal_s390x_power_get_state,
    .power_set_frequency = hal_s390x_power_set_frequency,

    // Debug and tracing
    .debug_init = hal_s390x_debug_init,
    .debug_break = hal_s390x_debug_break,
    .debug_trace = hal_s390x_debug_trace,
    .debug_dump_registers = hal_s390x_debug_dump_registers,

    // Architecture-specific extensions
    .arch_extension_call = hal_s390x_arch_extension_call,
    .arch_private = NULL};

// ============================================================================
// MEMORY MANAGEMENT
// ============================================================================

int hal_s390x_mmu_init(void)
{
    // Use existing s390x MMU implementation
    return s390x_mmu_init();
}

int hal_s390x_mmu_map(vaddr_t va, paddr_t pa, uint64_t flags)
{
    // Convert HAL flags to s390x-specific flags
    uint64_t s390x_flags = 0;
    if (flags & ORION_PAGE_READ)
        s390x_flags |= S390X_PAGE_READ;
    if (flags & ORION_PAGE_WRITE)
        s390x_flags |= S390X_PAGE_WRITE;
    if (flags & ORION_PAGE_EXEC)
        s390x_flags |= S390X_PAGE_EXEC;
    if (flags & ORION_PAGE_USER)
        s390x_flags |= S390X_PAGE_USER;

    return s390x_mmu_map_page(va, pa, s390x_flags);
}

int hal_s390x_mmu_unmap(vaddr_t va, size_t len)
{
    return s390x_mmu_unmap_pages(va, len);
}

int hal_s390x_mmu_protect(vaddr_t va, size_t len, uint64_t flags)
{
    uint64_t s390x_flags = 0;
    if (flags & ORION_PAGE_READ)
        s390x_flags |= S390X_PAGE_READ;
    if (flags & ORION_PAGE_WRITE)
        s390x_flags |= S390X_PAGE_WRITE;
    if (flags & ORION_PAGE_EXEC)
        s390x_flags |= S390X_PAGE_EXEC;
    if (flags & ORION_PAGE_USER)
        s390x_flags |= S390X_PAGE_USER;

    return s390x_mmu_protect_pages(va, len, s390x_flags);
}

paddr_t hal_s390x_mmu_translate(vaddr_t va)
{
    return s390x_mmu_translate_address(va);
}

// ============================================================================
// CONTEXT SWITCHING
// ============================================================================

void hal_s390x_context_switch(struct thread *prev, struct thread *next)
{
    s390x_context_switch(prev, next);
}

void hal_s390x_context_save(struct thread *thread)
{
    s390x_context_save(thread);
}

void hal_s390x_context_restore(struct thread *thread)
{
    s390x_context_restore(thread);
}

// ============================================================================
// IPC
// ============================================================================

ssize_t hal_s390x_ipc_fast_send(struct ipc_msg *msg, struct thread *target)
{
    // Use s390x-specific fast IPC if available
    return s390x_ipc_fast_send(msg, target);
}

ssize_t hal_s390x_ipc_fast_recv(struct ipc_msg *msg, struct thread *source)
{
    return s390x_ipc_fast_recv(msg, source);
}

int hal_s390x_ipc_setup_fast_path(struct thread *thread)
{
    return s390x_ipc_setup_fast_path(thread);
}

// ============================================================================
// INTERRUPT HANDLING
// ============================================================================

int hal_s390x_irq_register(uint32_t irq, irq_handler_t handler, void *data)
{
    return s390x_interrupt_register(irq, handler, data);
}

int hal_s390x_irq_unregister(uint32_t irq)
{
    return s390x_interrupt_unregister(irq);
}

void hal_s390x_irq_enable(uint32_t irq)
{
    s390x_interrupt_enable(irq);
}

void hal_s390x_irq_disable(uint32_t irq)
{
    s390x_interrupt_disable(irq);
}

void hal_s390x_irq_ack(uint32_t irq)
{
    s390x_interrupt_ack(irq);
}

// ============================================================================
// TIMER MANAGEMENT
// ============================================================================

int hal_s390x_timer_init(uint64_t frequency_hz)
{
    return s390x_timer_init(frequency_hz);
}

uint64_t hal_s390x_timer_get_ticks(void)
{
    return s390x_timer_get_ticks();
}

void hal_s390x_timer_set_oneshot(uint64_t delay_ticks, timer_callback_t callback)
{
    s390x_timer_set_oneshot(delay_ticks, callback);
}

void hal_s390x_timer_cancel(void)
{
    s390x_timer_cancel();
}

// ============================================================================
// CPU MANAGEMENT
// ============================================================================

int hal_s390x_cpu_init(uint32_t cpu_id)
{
    return s390x_cpu_init(cpu_id);
}

void hal_s390x_cpu_idle(void)
{
    s390x_cpu_idle();
}

void hal_s390x_cpu_wake(uint32_t cpu_id)
{
    s390x_cpu_wake(cpu_id);
}

uint32_t hal_s390x_cpu_get_count(void)
{
    return s390x_cpu_get_count();
}

uint32_t hal_s390x_cpu_get_current(void)
{
    return s390x_cpu_get_current();
}

// ============================================================================
// CACHE MANAGEMENT
// ============================================================================

void hal_s390x_cache_invalidate(void *addr, size_t size)
{
    s390x_cache_invalidate(addr, size);
}

void hal_s390x_cache_clean(void *addr, size_t size)
{
    s390x_cache_clean(addr, size);
}

void hal_s390x_cache_flush(void *addr, size_t size)
{
    s390x_cache_flush(addr, size);
}

void hal_s390x_cache_sync(void)
{
    s390x_cache_sync();
}

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

int hal_s390x_perf_init(void)
{
    return s390x_performance_init();
}

uint64_t hal_s390x_perf_read_counter(uint32_t counter_id)
{
    return s390x_performance_read_counter(counter_id);
}

int hal_s390x_perf_set_event(uint32_t counter_id, uint32_t event_id)
{
    return s390x_performance_set_event(counter_id, event_id);
}

void hal_s390x_perf_enable(uint32_t counter_id)
{
    s390x_performance_enable_counter(counter_id);
}

void hal_s390x_perf_disable(uint32_t counter_id)
{
    s390x_performance_disable_counter(counter_id);
}

// ============================================================================
// SECURITY FEATURES
// ============================================================================

int hal_s390x_secure_init(void)
{
    return s390x_security_init();
}

int hal_s390x_secure_measure(const void *data, size_t size, uint8_t *hash)
{
    return s390x_security_measure(data, size, hash);
}

int hal_s390x_secure_verify(const void *data, size_t size, const uint8_t *hash)
{
    return s390x_security_verify(data, size, hash);
}

int hal_s390x_secure_enclave_create(size_t size, void **enclave)
{
    return s390x_security_enclave_create(size, enclave);
}

int hal_s390x_secure_enclave_destroy(void *enclave)
{
    return s390x_security_enclave_destroy(enclave);
}

// ============================================================================
// POWER MANAGEMENT
// ============================================================================

int hal_s390x_power_init(void)
{
    return s390x_power_init();
}

int hal_s390x_power_set_state(power_state_t state)
{
    return s390x_power_set_state(state);
}

int hal_s390x_power_get_state(void)
{
    return s390x_power_get_state();
}

int hal_s390x_power_set_frequency(uint32_t cpu_id, uint32_t frequency_mhz)
{
    return s390x_power_set_frequency(cpu_id, frequency_mhz);
}

// ============================================================================
// DEBUG AND TRACING
// ============================================================================

int hal_s390x_debug_init(void)
{
    return s390x_debug_init();
}

void hal_s390x_debug_break(void)
{
    s390x_debug_break();
}

void hal_s390x_debug_trace(const char *message)
{
    s390x_debug_trace(message);
}

int hal_s390x_debug_dump_registers(void)
{
    return s390x_debug_dump_registers();
}

// ============================================================================
// ARCHITECTURE-SPECIFIC EXTENSIONS
// ============================================================================

int hal_s390x_arch_extension_call(uint32_t extension_id, void *args)
{
    // Handle s390x-specific extensions like z/Architecture, Vector, Crypto, etc.
    switch (extension_id)
    {
    case ORION_EXTENSION_Z196:
        return s390x_z196_extension_call(args);
    case ORION_EXTENSION_ZEC12:
        return s390x_zec12_extension_call(args);
    case ORION_EXTENSION_Z13:
        return s390x_z13_extension_call(args);
    case ORION_EXTENSION_Z14:
        return s390x_z14_extension_call(args);
    case ORION_EXTENSION_Z15:
        return s390x_z15_extension_call(args);
    case ORION_EXTENSION_Z16:
        return s390x_z16_extension_call(args);
    case ORION_EXTENSION_VECTOR:
        return s390x_vector_extension_call(args);
    case ORION_EXTENSION_CRYPTO:
        return s390x_crypto_extension_call(args);
    case ORION_EXTENSION_DECIMAL:
        return s390x_decimal_extension_call(args);
    case ORION_EXTENSION_TX:
        return s390x_tx_extension_call(args);
    default:
        return ORION_HAL_ERROR_NOT_SUPPORTED;
    }
}

// ============================================================================
// HAL INITIALIZATION AND ACCESS
// ============================================================================

int hal_s390x_init(void)
{
    // Initialize s390x architecture
    int ret = s390x_arch_init();
    if (ret != 0)
    {
        return ret;
    }

    // Initialize s390x HAL context
    s390x_hal_arch.arch_private = NULL;

    return ORION_HAL_SUCCESS;
}

void hal_s390x_cleanup(void)
{
    // Cleanup s390x architecture
    s390x_arch_cleanup();
}

struct orion_hal_arch *hal_s390x_get_interface(void)
{
    return &s390x_hal_arch;
}

// Export the s390x HAL interface
struct orion_hal_arch orion_hal_s390x = s390x_hal_arch;
