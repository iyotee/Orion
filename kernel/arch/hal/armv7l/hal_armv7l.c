/*
 * Orion Operating System - ARMv7l HAL Implementation
 *
 * Hardware Abstraction Layer implementation for ARMv7l architecture.
 * This implementation bridges the HAL interface with the existing
 * kernel/arch/armv7l code.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "hal_armv7l.h"
#include "hal_armv7l_common.h"
#include "../../armv7l/arch.h"
#include "../../armv7l/mmu.h"
#include "../../armv7l/interrupts.h"
#include "../../armv7l/timers.h"
#include "../../armv7l/cache.h"
#include "../../armv7l/performance.h"
#include "../../armv7l/security.h"
#include "../../armv7l/power.h"
#include "../../armv7l/debug.h"
#include <orion/error.h>
#include <orion/thread.h>
#include <orion/ipc.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

static bool armv7l_hal_initialized = false;
static struct orion_arch_info armv7l_arch_info;

// ============================================================================
// MEMORY MANAGEMENT FUNCTIONS
// ============================================================================

int hal_armv7l_mmu_init(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    // Use existing ARMv7l MMU implementation
    return armv7l_mmu_init();
}

int hal_armv7l_mmu_map(vaddr_t va, paddr_t pa, uint64_t flags)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    // Convert HAL flags to ARMv7l-specific flags
    uint64_t armv7l_flags = 0;
    if (flags & ORION_PAGE_PRESENT)
        armv7l_flags |= ORION_ARMV7L_PAGE_PRESENT;
    if (flags & ORION_PAGE_WRITABLE)
        armv7l_flags |= ORION_ARMV7L_PAGE_WRITABLE;
    if (flags & ORION_PAGE_USER)
        armv7l_flags |= ORION_ARMV7L_PAGE_USER;
    if (flags & ORION_PAGE_EXECUTABLE)
        armv7l_flags |= ORION_ARMV7L_PAGE_EXECUTABLE;
    if (flags & ORION_PAGE_KERNEL)
        armv7l_flags |= ORION_ARMV7L_PAGE_KERNEL;
    if (flags & ORION_PAGE_DEVICE)
        armv7l_flags |= ORION_ARMV7L_PAGE_DEVICE;
    if (flags & ORION_PAGE_NOCACHE)
        armv7l_flags |= ORION_ARMV7L_PAGE_NOCACHE;

    return armv7l_mmu_map(va, pa, armv7l_flags);
}

int hal_armv7l_mmu_unmap(vaddr_t va, size_t len)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_mmu_unmap(va, len);
}

int hal_armv7l_mmu_protect(vaddr_t va, size_t len, uint64_t flags)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    // Convert HAL flags to ARMv7l-specific flags
    uint64_t armv7l_flags = 0;
    if (flags & ORION_PAGE_PRESENT)
        armv7l_flags |= ORION_ARMV7L_PAGE_PRESENT;
    if (flags & ORION_PAGE_WRITABLE)
        armv7l_flags |= ORION_ARMV7L_PAGE_WRITABLE;
    if (flags & ORION_PAGE_USER)
        armv7l_flags |= ORION_ARMV7L_PAGE_USER;
    if (flags & ORION_PAGE_EXECUTABLE)
        armv7l_flags |= ORION_ARMV7L_PAGE_EXECUTABLE;

    return armv7l_mmu_protect(va, len, armv7l_flags);
}

int hal_armv7l_mmu_get_mapping(vaddr_t va, paddr_t *pa, uint64_t *flags)
{
    if (!armv7l_hal_initialized || !pa || !flags)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    uint64_t armv7l_flags;
    int ret = armv7l_mmu_get_mapping(va, pa, &armv7l_flags);
    if (ret != ORION_ARMV7L_SUCCESS)
    {
        return ret;
    }

    // Convert ARMv7l flags to HAL flags
    *flags = 0;
    if (armv7l_flags & ORION_ARMV7L_PAGE_PRESENT)
        *flags |= ORION_PAGE_PRESENT;
    if (armv7l_flags & ORION_ARMV7L_PAGE_WRITABLE)
        *flags |= ORION_PAGE_WRITABLE;
    if (armv7l_flags & ORION_ARMV7L_PAGE_USER)
        *flags |= ORION_PAGE_USER;
    if (armv7l_flags & ORION_ARMV7L_PAGE_EXECUTABLE)
        *flags |= ORION_PAGE_EXECUTABLE;
    if (armv7l_flags & ORION_ARMV7L_PAGE_KERNEL)
        *flags |= ORION_PAGE_KERNEL;
    if (armv7l_flags & ORION_ARMV7L_PAGE_DEVICE)
        *flags |= ORION_PAGE_DEVICE;
    if (armv7l_flags & ORION_ARMV7L_PAGE_NOCACHE)
        *flags |= ORION_PAGE_NOCACHE;

    return ORION_HAL_SUCCESS;
}

int hal_armv7l_mmu_flush_tlb(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_mmu_flush_tlb();
}

int hal_armv7l_mmu_flush_tlb_range(vaddr_t va, size_t len)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_mmu_flush_tlb_range(va, len);
}

int hal_armv7l_mmu_set_kernel_mapping(vaddr_t va, paddr_t pa, size_t len, uint64_t flags)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    // Convert HAL flags to ARMv7l-specific flags
    uint64_t armv7l_flags = ORION_ARMV7L_PAGE_KERNEL;
    if (flags & ORION_PAGE_PRESENT)
        armv7l_flags |= ORION_ARMV7L_PAGE_PRESENT;
    if (flags & ORION_PAGE_WRITABLE)
        armv7l_flags |= ORION_ARMV7L_PAGE_WRITABLE;
    if (flags & ORION_PAGE_EXECUTABLE)
        armv7l_flags |= ORION_ARMV7L_PAGE_EXECUTABLE;
    if (flags & ORION_PAGE_DEVICE)
        armv7l_flags |= ORION_ARMV7L_PAGE_DEVICE;
    if (flags & ORION_PAGE_NOCACHE)
        armv7l_flags |= ORION_ARMV7L_PAGE_NOCACHE;

    return armv7l_mmu_set_kernel_mapping(va, pa, len, armv7l_flags);
}

// ============================================================================
// INTERRUPT HANDLING FUNCTIONS
// ============================================================================

int hal_armv7l_interrupt_init(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_interrupts_init();
}

int hal_armv7l_interrupt_enable(uint32_t irq)
{
    if (!armv7l_hal_initialized || irq >= ORION_ARMV7L_IRQ_MAX)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_interrupt_enable(irq);
}

int hal_armv7l_interrupt_disable(uint32_t irq)
{
    if (!armv7l_hal_initialized || irq >= ORION_ARMV7L_IRQ_MAX)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_interrupt_disable(irq);
}

int hal_armv7l_interrupt_ack(uint32_t irq)
{
    if (!armv7l_hal_initialized || irq >= ORION_ARMV7L_IRQ_MAX)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_interrupt_ack(irq);
}

int hal_armv7l_interrupt_set_handler(uint32_t irq, void (*handler)(void))
{
    if (!armv7l_hal_initialized || irq >= ORION_ARMV7L_IRQ_MAX || !handler)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_interrupt_set_handler(irq, handler);
}

int hal_armv7l_interrupt_set_priority(uint32_t irq, uint32_t priority)
{
    if (!armv7l_hal_initialized || irq >= ORION_ARMV7L_IRQ_MAX)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_interrupt_set_priority(irq, priority);
}

int hal_armv7l_interrupt_get_pending(uint32_t irq)
{
    if (!armv7l_hal_initialized || irq >= ORION_ARMV7L_IRQ_MAX)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_interrupt_get_pending(irq);
}

int hal_armv7l_interrupt_clear_pending(uint32_t irq)
{
    if (!armv7l_hal_initialized || irq >= ORION_ARMV7L_IRQ_MAX)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_interrupt_clear_pending(irq);
}

// ============================================================================
// TIMER MANAGEMENT FUNCTIONS
// ============================================================================

int hal_armv7l_timer_init(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_timers_init();
}

int hal_armv7l_timer_set_interval(uint64_t interval_ns)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    if (interval_ns < ORION_ARMV7L_TIMER_MIN_INTERVAL ||
        interval_ns > ORION_ARMV7L_TIMER_MAX_INTERVAL)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_timer_set_interval(interval_ns);
}

int hal_armv7l_timer_get_current(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_timer_get_current();
}

int hal_armv7l_timer_enable(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_timer_enable();
}

int hal_armv7l_timer_disable(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_timer_disable();
}

int hal_armv7l_timer_set_callback(void (*callback)(void))
{
    if (!armv7l_hal_initialized || !callback)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_timer_set_callback(callback);
}

int hal_armv7l_timer_get_frequency(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_timer_get_frequency();
}

// ============================================================================
// CACHE MANAGEMENT FUNCTIONS
// ============================================================================

int hal_armv7l_cache_init(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_cache_init();
}

int hal_armv7l_cache_invalidate_data(vaddr_t va, size_t len)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_cache_invalidate_data(va, len);
}

int hal_armv7l_cache_clean_data(vaddr_t va, size_t len)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_cache_clean_data(va, len);
}

int hal_armv7l_cache_flush_data(vaddr_t va, size_t len)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_cache_flush_data(va, len);
}

int hal_armv7l_cache_invalidate_instruction(vaddr_t va, size_t len)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_cache_invalidate_instruction(va, len);
}

int hal_armv7l_cache_sync(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_cache_sync();
}

// ============================================================================
// CPU MANAGEMENT FUNCTIONS
// ============================================================================

int hal_armv7l_cpu_init(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_cpu_init();
}

int hal_armv7l_cpu_get_id(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_cpu_get_id();
}

int hal_armv7l_cpu_get_count(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_cpu_get_count();
}

int hal_armv7l_cpu_start(uint32_t cpu_id, vaddr_t entry_point)
{
    if (!armv7l_hal_initialized || !entry_point)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_cpu_start(cpu_id, entry_point);
}

int hal_armv7l_cpu_stop(uint32_t cpu_id)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_cpu_stop(cpu_id);
}

int hal_armv7l_cpu_wait(uint32_t cpu_id)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_cpu_wait(cpu_id);
}

int hal_armv7l_cpu_get_features(uint64_t *features)
{
    if (!armv7l_hal_initialized || !features)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_cpu_get_features(features);
}

// ============================================================================
// PERFORMANCE MONITORING FUNCTIONS
// ============================================================================

int hal_armv7l_performance_init(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_performance_init();
}

int hal_armv7l_performance_start_counter(uint32_t counter_id)
{
    if (!armv7l_hal_initialized || counter_id >= ORION_ARMV7L_PMU_COUNTERS)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_performance_start_counter(counter_id);
}

int hal_armv7l_performance_stop_counter(uint32_t counter_id)
{
    if (!armv7l_hal_initialized || counter_id >= ORION_ARMV7L_PMU_COUNTERS)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_performance_stop_counter(counter_id);
}

int hal_armv7l_performance_read_counter(uint32_t counter_id, uint64_t *value)
{
    if (!armv7l_hal_initialized || counter_id >= ORION_ARMV7L_PMU_COUNTERS || !value)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_performance_read_counter(counter_id, value);
}

int hal_armv7l_performance_set_event(uint32_t counter_id, uint32_t event_id)
{
    if (!armv7l_hal_initialized || counter_id >= ORION_ARMV7L_PMU_COUNTERS ||
        event_id >= ORION_ARMV7L_PMU_EVENTS)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_performance_set_event(counter_id, event_id);
}

int hal_armv7l_performance_get_cycle_count(uint64_t *cycles)
{
    if (!armv7l_hal_initialized || !cycles)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_performance_get_cycle_count(cycles);
}

int hal_armv7l_performance_get_instruction_count(uint64_t *instructions)
{
    if (!armv7l_hal_initialized || !instructions)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_performance_get_instruction_count(instructions);
}

// ============================================================================
// POWER MANAGEMENT FUNCTIONS
// ============================================================================

int hal_armv7l_power_init(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_power_init();
}

int hal_armv7l_power_set_state(uint32_t state)
{
    if (!armv7l_hal_initialized || state > ORION_ARMV7L_POWER_STATE_OFF)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_power_set_state(state);
}

int hal_armv7l_power_get_state(uint32_t *state)
{
    if (!armv7l_hal_initialized || !state)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_power_get_state(state);
}

int hal_armv7l_power_sleep(uint64_t duration_ns)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_power_sleep(duration_ns);
}

int hal_armv7l_power_wake(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_power_wake();
}

int hal_armv7l_power_set_frequency(uint32_t cpu_id, uint32_t frequency_mhz)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_power_set_frequency(cpu_id, frequency_mhz);
}

// ============================================================================
// SECURITY FEATURES FUNCTIONS
// ============================================================================

int hal_armv7l_security_init(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_security_init();
}

int hal_armv7l_security_set_trustzone(uint32_t enabled)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_security_set_trustzone(enabled);
}

int hal_armv7l_security_get_trustzone(uint32_t *enabled)
{
    if (!armv7l_hal_initialized || !enabled)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_security_get_trustzone(enabled);
}

int hal_armv7l_security_set_secure_world(uint32_t enabled)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_security_set_secure_world(enabled);
}

int hal_armv7l_security_get_secure_world(uint32_t *enabled)
{
    if (!armv7l_hal_initialized || !enabled)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_security_get_secure_world(enabled);
}

// ============================================================================
// DEBUG SUPPORT FUNCTIONS
// ============================================================================

int hal_armv7l_debug_init(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_debug_init();
}

int hal_armv7l_debug_set_breakpoint(vaddr_t va, uint32_t type)
{
    if (!armv7l_hal_initialized || type > ORION_ARMV7L_DEBUG_BREAKPOINT_DATA)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_debug_set_breakpoint(va, type);
}

int hal_armv7l_debug_clear_breakpoint(vaddr_t va)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_debug_clear_breakpoint(va);
}

int hal_armv7l_debug_set_watchpoint(vaddr_t va, size_t len, uint32_t type)
{
    if (!armv7l_hal_initialized || type > ORION_ARMV7L_DEBUG_WATCHPOINT_ACCESS)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_debug_set_watchpoint(va, len, type);
}

int hal_armv7l_debug_clear_watchpoint(vaddr_t va)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_debug_clear_watchpoint(va);
}

int hal_armv7l_debug_step(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_debug_step();
}

int hal_armv7l_debug_continue(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_debug_continue();
}

// ============================================================================
// CONTEXT SWITCHING FUNCTIONS
// ============================================================================

int hal_armv7l_context_switch(struct thread *from, struct thread *to)
{
    if (!armv7l_hal_initialized || !to)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    // Use existing ARMv7l context switching implementation
    return armv7l_context_switch(from, to);
}

int hal_armv7l_context_save(struct thread *thread)
{
    if (!armv7l_hal_initialized || !thread)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_context_save(thread);
}

int hal_armv7l_context_restore(struct thread *thread)
{
    if (!armv7l_hal_initialized || !thread)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_context_restore(thread);
}

int hal_armv7l_context_init(struct thread *thread, vaddr_t entry_point, vaddr_t stack_ptr)
{
    if (!armv7l_hal_initialized || !thread || !entry_point || !stack_ptr)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_context_init(thread, entry_point, stack_ptr);
}

// ============================================================================
// IPC FAST PATH FUNCTIONS
// ============================================================================

int hal_armv7l_ipc_fast_path(struct ipc_msg *msg)
{
    if (!armv7l_hal_initialized || !msg)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    // Use existing ARMv7l IPC implementation
    return armv7l_ipc_fast_path(msg);
}

int hal_armv7l_ipc_fast_path_setup(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_ipc_fast_path_setup();
}

int hal_armv7l_ipc_fast_path_cleanup(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_ipc_fast_path_cleanup();
}

// ============================================================================
// SYSTEM CALLS FUNCTIONS
// ============================================================================

int hal_armv7l_syscall_init(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return armv7l_syscall_init();
}

int hal_armv7l_syscall_set_handler(uint32_t syscall_num, void (*handler)(void))
{
    if (!armv7l_hal_initialized || !handler)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_syscall_set_handler(syscall_num, handler);
}

int hal_armv7l_syscall_get_handler(uint32_t syscall_num, void (**handler)(void))
{
    if (!armv7l_hal_initialized || !handler)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return armv7l_syscall_get_handler(syscall_num, handler);
}

// ============================================================================
// INITIALIZATION AND CLEANUP FUNCTIONS
// ============================================================================

int hal_armv7l_init(void)
{
    if (armv7l_hal_initialized)
    {
        return ORION_ERROR_ALREADY_EXISTS;
    }

    // Initialize ARMv7l architecture
    int ret = armv7l_arch_init();
    if (ret != ORION_ARMV7L_SUCCESS)
    {
        return ret;
    }

    // Initialize architecture info
    memset(&armv7l_arch_info, 0, sizeof(armv7l_arch_info));
    armv7l_arch_info.arch_id = ORION_ARCH_ARMV7L;
    armv7l_arch_info.arch_name = "ARMv7l";
    armv7l_arch_info.arch_version = "ARMv7-A";
    armv7l_arch_info.page_size = ORION_ARMV7L_PAGE_SIZE_4K;
    armv7l_arch_info.max_pages = 0x1000000; // 4GB virtual address space
    armv7l_arch_info.max_irqs = ORION_ARMV7L_IRQ_MAX;
    armv7l_arch_info.max_exceptions = ORION_ARMV7L_EXC_MAX;
    armv7l_arch_info.cache_line_size = ORION_ARMV7L_CACHE_LINE_SIZE;
    armv7l_arch_info.tlb_entries = ORION_ARMV7L_TLB_ENTRIES;
    armv7l_arch_info.pmu_counters = ORION_ARMV7L_PMU_COUNTERS;

    armv7l_hal_initialized = true;
    return ORION_HAL_SUCCESS;
}

int hal_armv7l_cleanup(void)
{
    if (!armv7l_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    // Cleanup ARMv7l architecture
    int ret = armv7l_arch_cleanup();
    if (ret != ORION_ARMV7L_SUCCESS)
    {
        return ret;
    }

    armv7l_hal_initialized = false;
    return ORION_HAL_SUCCESS;
}

int hal_armv7l_get_arch_info(struct orion_arch_info *info)
{
    if (!armv7l_hal_initialized || !info)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    memcpy(info, &armv7l_arch_info, sizeof(armv7l_arch_info));
    return ORION_HAL_SUCCESS;
}
