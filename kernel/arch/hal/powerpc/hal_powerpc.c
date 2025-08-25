/*
 * Orion Operating System - PowerPC HAL Implementation
 *
 * Hardware Abstraction Layer implementation for PowerPC architecture.
 * This implementation bridges the HAL interface with the existing
 * kernel/arch/powerpc code.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "hal_powerpc.h"
#include "../../powerpc/arch.h"
#include "../../powerpc/mmu.h"
#include "../../powerpc/interrupts.h"
#include "../../powerpc/timer.h"
#include "../../powerpc/cache.h"
#include "../../powerpc/performance.h"
#include "../../powerpc/security.h"
#include "../../powerpc/power.h"
#include "../../powerpc/debug.h"
#include <orion/error.h>
#include <orion/thread.h>
#include <orion/ipc.h>

// PowerPC HAL context
static struct orion_hal_arch powerpc_hal_arch = {
    .name = "PowerPC",
    .arch_id = ORION_ARCH_POWERPC,
    .version = 1,

    // Memory management
    .mmu_init = hal_powerpc_mmu_init,
    .mmu_map = hal_powerpc_mmu_map,
    .mmu_unmap = hal_powerpc_mmu_unmap,
    .mmu_protect = hal_powerpc_mmu_protect,
    .mmu_translate = hal_powerpc_mmu_translate,

    // Context switching
    .context_switch = hal_powerpc_context_switch,
    .context_save = hal_powerpc_context_save,
    .context_restore = hal_powerpc_context_restore,

    // IPC
    .ipc_fast_send = hal_powerpc_ipc_fast_send,
    .ipc_fast_recv = hal_powerpc_ipc_fast_recv,
    .ipc_setup_fast_path = hal_powerpc_ipc_setup_fast_path,

    // Interrupt handling
    .irq_register = hal_powerpc_irq_register,
    .irq_unregister = hal_powerpc_irq_unregister,
    .irq_enable = hal_powerpc_irq_enable,
    .irq_disable = hal_powerpc_irq_disable,
    .irq_ack = hal_powerpc_irq_ack,

    // Timer management
    .timer_init = hal_powerpc_timer_init,
    .timer_get_ticks = hal_powerpc_timer_get_ticks,
    .timer_set_oneshot = hal_powerpc_timer_set_oneshot,
    .timer_cancel = hal_powerpc_timer_cancel,

    // CPU management
    .cpu_init = hal_powerpc_cpu_init,
    .cpu_idle = hal_powerpc_cpu_idle,
    .cpu_wake = hal_powerpc_cpu_wake,
    .cpu_get_count = hal_powerpc_cpu_get_count,
    .cpu_get_current = hal_powerpc_cpu_get_current,

    // Cache management
    .cache_invalidate = hal_powerpc_cache_invalidate,
    .cache_clean = hal_powerpc_cache_clean,
    .cache_flush = hal_powerpc_cache_flush,
    .cache_sync = hal_powerpc_cache_sync,

    // Performance monitoring
    .perf_init = hal_powerpc_perf_init,
    .perf_read_counter = hal_powerpc_perf_read_counter,
    .perf_set_event = hal_powerpc_perf_set_event,
    .perf_enable = hal_powerpc_perf_enable,
    .perf_disable = hal_powerpc_perf_disable,

    // Security features
    .secure_init = hal_powerpc_secure_init,
    .secure_measure = hal_powerpc_secure_measure,
    .secure_verify = hal_powerpc_secure_verify,
    .secure_enclave_create = hal_powerpc_secure_enclave_create,
    .secure_enclave_destroy = hal_powerpc_secure_enclave_destroy,

    // Power management
    .power_init = hal_powerpc_power_init,
    .power_set_state = hal_powerpc_power_set_state,
    .power_get_state = hal_powerpc_power_get_state,
    .power_set_frequency = hal_powerpc_power_set_frequency,

    // Debug and tracing
    .debug_init = hal_powerpc_debug_init,
    .debug_break = hal_powerpc_debug_break,
    .debug_trace = hal_powerpc_debug_trace,
    .debug_dump_registers = hal_powerpc_debug_dump_registers,

    // Architecture-specific extensions
    .arch_extension_call = hal_powerpc_arch_extension_call,
    .arch_private = NULL};

// ============================================================================
// MEMORY MANAGEMENT
// ============================================================================

int hal_powerpc_mmu_init(void)
{
    // Use existing PowerPC MMU implementation
    return power_mmu_init();
}

int hal_powerpc_mmu_map(vaddr_t va, paddr_t pa, uint64_t flags)
{
    // Convert HAL flags to PowerPC-specific flags
    uint64_t power_flags = 0;
    if (flags & ORION_PAGE_READ)
        power_flags |= POWER_PAGE_READ;
    if (flags & ORION_PAGE_WRITE)
        power_flags |= POWER_PAGE_WRITE;
    if (flags & ORION_PAGE_EXEC)
        power_flags |= POWER_PAGE_EXEC;
    if (flags & ORION_PAGE_USER)
        power_flags |= POWER_PAGE_USER;

    return power_mmu_map_page(va, pa, power_flags);
}

int hal_powerpc_mmu_unmap(vaddr_t va, size_t len)
{
    return power_mmu_unmap_pages(va, len);
}

int hal_powerpc_mmu_protect(vaddr_t va, size_t len, uint64_t flags)
{
    uint64_t power_flags = 0;
    if (flags & ORION_PAGE_READ)
        power_flags |= POWER_PAGE_READ;
    if (flags & ORION_PAGE_WRITE)
        power_flags |= POWER_PAGE_WRITE;
    if (flags & ORION_PAGE_EXEC)
        power_flags |= POWER_PAGE_EXEC;
    if (flags & ORION_PAGE_USER)
        power_flags |= POWER_PAGE_USER;

    return power_mmu_protect_pages(va, len, power_flags);
}

paddr_t hal_powerpc_mmu_translate(vaddr_t va)
{
    return power_mmu_translate_address(va);
}

// ============================================================================
// CONTEXT SWITCHING
// ============================================================================

void hal_powerpc_context_switch(struct thread *prev, struct thread *next)
{
    power_context_switch(prev, next);
}

void hal_powerpc_context_save(struct thread *thread)
{
    power_context_save(thread);
}

void hal_powerpc_context_restore(struct thread *thread)
{
    power_context_restore(thread);
}

// ============================================================================
// IPC
// ============================================================================

ssize_t hal_powerpc_ipc_fast_send(struct ipc_msg *msg, struct thread *target)
{
    // Use PowerPC-specific fast IPC if available
    return power_ipc_fast_send(msg, target);
}

ssize_t hal_powerpc_ipc_fast_recv(struct ipc_msg *msg, struct thread *source)
{
    return power_ipc_fast_recv(msg, source);
}

int hal_powerpc_ipc_setup_fast_path(struct thread *thread)
{
    return power_ipc_setup_fast_path(thread);
}

// ============================================================================
// INTERRUPT HANDLING
// ============================================================================

int hal_powerpc_irq_register(uint32_t irq, irq_handler_t handler, void *data)
{
    return power_interrupt_register(irq, handler, data);
}

int hal_powerpc_irq_unregister(uint32_t irq)
{
    return power_interrupt_unregister(irq);
}

void hal_powerpc_irq_enable(uint32_t irq)
{
    power_interrupt_enable(irq);
}

void hal_powerpc_irq_disable(uint32_t irq)
{
    power_interrupt_disable(irq);
}

void hal_powerpc_irq_ack(uint32_t irq)
{
    power_interrupt_ack(irq);
}

// ============================================================================
// TIMER MANAGEMENT
// ============================================================================

int hal_powerpc_timer_init(uint64_t frequency_hz)
{
    return power_timer_init(frequency_hz);
}

uint64_t hal_powerpc_timer_get_ticks(void)
{
    return power_timer_get_ticks();
}

void hal_powerpc_timer_set_oneshot(uint64_t delay_ticks, timer_callback_t callback)
{
    power_timer_set_oneshot(delay_ticks, callback);
}

void hal_powerpc_timer_cancel(void)
{
    power_timer_cancel();
}

// ============================================================================
// CPU MANAGEMENT
// ============================================================================

int hal_powerpc_cpu_init(uint32_t cpu_id)
{
    return power_cpu_init(cpu_id);
}

void hal_powerpc_cpu_idle(void)
{
    power_cpu_idle();
}

void hal_powerpc_cpu_wake(uint32_t cpu_id)
{
    power_cpu_wake(cpu_id);
}

uint32_t hal_powerpc_cpu_get_count(void)
{
    return power_cpu_get_count();
}

uint32_t hal_powerpc_cpu_get_current(void)
{
    return power_cpu_get_current();
}

// ============================================================================
// CACHE MANAGEMENT
// ============================================================================

void hal_powerpc_cache_invalidate(void *addr, size_t size)
{
    power_cache_invalidate(addr, size);
}

void hal_powerpc_cache_clean(void *addr, size_t size)
{
    power_cache_clean(addr, size);
}

void hal_powerpc_cache_flush(void *addr, size_t size)
{
    power_cache_flush(addr, size);
}

void hal_powerpc_cache_sync(void)
{
    power_cache_sync();
}

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

int hal_powerpc_perf_init(void)
{
    return power_performance_init();
}

uint64_t hal_powerpc_perf_read_counter(uint32_t counter_id)
{
    return power_performance_read_counter(counter_id);
}

int hal_powerpc_perf_set_event(uint32_t counter_id, uint32_t event_id)
{
    return power_performance_set_event(counter_id, event_id);
}

void hal_powerpc_perf_enable(uint32_t counter_id)
{
    power_performance_enable_counter(counter_id);
}

void hal_powerpc_perf_disable(uint32_t counter_id)
{
    power_performance_disable_counter(counter_id);
}

// ============================================================================
// SECURITY FEATURES
// ============================================================================

int hal_powerpc_secure_init(void)
{
    return power_security_init();
}

int hal_powerpc_secure_measure(const void *data, size_t size, uint8_t *hash)
{
    return power_security_measure(data, size, hash);
}

int hal_powerpc_secure_verify(const void *data, size_t size, const uint8_t *hash)
{
    return power_security_verify(data, size, hash);
}

int hal_powerpc_secure_enclave_create(size_t size, void **enclave)
{
    return power_security_enclave_create(size, enclave);
}

int hal_powerpc_secure_enclave_destroy(void *enclave)
{
    return power_security_enclave_destroy(enclave);
}

// ============================================================================
// POWER MANAGEMENT
// ============================================================================

int hal_powerpc_power_init(void)
{
    return power_power_init();
}

int hal_powerpc_power_set_state(power_state_t state)
{
    return power_power_set_state(state);
}

int hal_powerpc_power_get_state(void)
{
    return power_power_get_state();
}

int hal_powerpc_power_set_frequency(uint32_t cpu_id, uint32_t frequency_mhz)
{
    return power_power_set_frequency(cpu_id, frequency_mhz);
}

// ============================================================================
// DEBUG AND TRACING
// ============================================================================

int hal_powerpc_debug_init(void)
{
    return power_debug_init();
}

void hal_powerpc_debug_break(void)
{
    power_debug_break();
}

void hal_powerpc_debug_trace(const char *message)
{
    power_debug_trace(message);
}

int hal_powerpc_debug_dump_registers(void)
{
    return power_debug_dump_registers();
}

// ============================================================================
// ARCHITECTURE-SPECIFIC EXTENSIONS
// ============================================================================

int hal_powerpc_arch_extension_call(uint32_t extension_id, void *args)
{
    // Handle PowerPC-specific extensions like VSX, AltiVec, etc.
    switch (extension_id)
    {
    case ORION_EXTENSION_VSX:
        return power_vsx_extension_call(args);
    case ORION_EXTENSION_ALTIVEC:
        return power_altivec_extension_call(args);
    case ORION_EXTENSION_POWER8:
        return power_power8_extension_call(args);
    case ORION_EXTENSION_POWER9:
        return power_power9_extension_call(args);
    case ORION_EXTENSION_POWER10:
        return power_power10_extension_call(args);
    default:
        return ORION_HAL_ERROR_NOT_SUPPORTED;
    }
}

// ============================================================================
// HAL INITIALIZATION AND ACCESS
// ============================================================================

int hal_powerpc_init(void)
{
    // Initialize PowerPC architecture
    int ret = power_arch_init();
    if (ret != 0)
    {
        return ret;
    }

    // Initialize PowerPC HAL context
    powerpc_hal_arch.arch_private = NULL;

    return ORION_HAL_SUCCESS;
}

void hal_powerpc_cleanup(void)
{
    // Cleanup PowerPC architecture
    power_arch_cleanup();
}

struct orion_hal_arch *hal_powerpc_get_interface(void)
{
    return &powerpc_hal_arch;
}

// Export the PowerPC HAL interface
struct orion_hal_arch orion_hal_powerpc = powerpc_hal_arch;
