/*
 * Orion Operating System - MIPS HAL Implementation
 *
 * Hardware Abstraction Layer implementation for MIPS architecture.
 * This implementation bridges the HAL interface with the existing
 * kernel/arch/mips code.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "hal_mips.h"
#include "hal_mips_common.h"
#include "../../mips/arch.h"
#include "../../mips/mmu.h"
#include "../../mips/interrupts.h"
#include "../../mips/timers.h"
#include "../../mips/cache.h"
#include "../../mips/performance.h"
#include "../../mips/security.h"
#include "../../mips/power.h"
#include "../../mips/debug.h"
#include <orion/error.h>
#include <orion/thread.h>
#include <orion/ipc.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

static bool mips_hal_initialized = false;
static struct orion_arch_info mips_arch_info;

// ============================================================================
// MEMORY MANAGEMENT FUNCTIONS
// ============================================================================

int hal_mips_mmu_init(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    // Use existing MIPS MMU implementation
    return mips_mmu_init();
}

int hal_mips_mmu_map(vaddr_t va, paddr_t pa, uint64_t flags)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    // Convert HAL flags to MIPS-specific flags
    uint64_t mips_flags = 0;
    if (flags & ORION_PAGE_PRESENT)
        mips_flags |= ORION_MIPS_PAGE_PRESENT;
    if (flags & ORION_PAGE_WRITABLE)
        mips_flags |= ORION_MIPS_PAGE_WRITABLE;
    if (flags & ORION_PAGE_USER)
        mips_flags |= ORION_MIPS_PAGE_USER;
    if (flags & ORION_PAGE_EXECUTABLE)
        mips_flags |= ORION_MIPS_PAGE_EXECUTABLE;
    if (flags & ORION_PAGE_KERNEL)
        mips_flags |= ORION_MIPS_PAGE_KERNEL;
    if (flags & ORION_PAGE_DEVICE)
        mips_flags |= ORION_MIPS_PAGE_DEVICE;
    if (flags & ORION_PAGE_NOCACHE)
        mips_flags |= ORION_MIPS_PAGE_NOCACHE;

    return mips_mmu_map(va, pa, mips_flags);
}

int hal_mips_mmu_unmap(vaddr_t va, size_t len)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_mmu_unmap(va, len);
}

int hal_mips_mmu_protect(vaddr_t va, size_t len, uint64_t flags)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    // Convert HAL flags to MIPS-specific flags
    uint64_t mips_flags = 0;
    if (flags & ORION_PAGE_PRESENT)
        mips_flags |= ORION_MIPS_PAGE_PRESENT;
    if (flags & ORION_PAGE_WRITABLE)
        mips_flags |= ORION_MIPS_PAGE_WRITABLE;
    if (flags & ORION_PAGE_USER)
        mips_flags |= ORION_MIPS_PAGE_USER;
    if (flags & ORION_PAGE_EXECUTABLE)
        mips_flags |= ORION_MIPS_PAGE_EXECUTABLE;

    return mips_mmu_protect(va, len, mips_flags);
}

int hal_mips_mmu_get_mapping(vaddr_t va, paddr_t *pa, uint64_t *flags)
{
    if (!mips_hal_initialized || !pa || !flags)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    uint64_t mips_flags;
    int ret = mips_mmu_get_mapping(va, pa, &mips_flags);
    if (ret != ORION_MIPS_SUCCESS)
    {
        return ret;
    }

    // Convert MIPS flags to HAL flags
    *flags = 0;
    if (mips_flags & ORION_MIPS_PAGE_PRESENT)
        *flags |= ORION_PAGE_PRESENT;
    if (mips_flags & ORION_MIPS_PAGE_WRITABLE)
        *flags |= ORION_PAGE_WRITABLE;
    if (mips_flags & ORION_MIPS_PAGE_USER)
        *flags |= ORION_PAGE_USER;
    if (mips_flags & ORION_MIPS_PAGE_EXECUTABLE)
        *flags |= ORION_PAGE_EXECUTABLE;
    if (mips_flags & ORION_MIPS_PAGE_KERNEL)
        *flags |= ORION_PAGE_KERNEL;
    if (mips_flags & ORION_MIPS_PAGE_DEVICE)
        *flags |= ORION_PAGE_DEVICE;
    if (mips_flags & ORION_MIPS_PAGE_NOCACHE)
        *flags |= ORION_PAGE_NOCACHE;

    return ORION_HAL_SUCCESS;
}

int hal_mips_mmu_flush_tlb(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_mmu_flush_tlb();
}

int hal_mips_mmu_flush_tlb_range(vaddr_t va, size_t len)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_mmu_flush_tlb_range(va, len);
}

int hal_mips_mmu_set_kernel_mapping(vaddr_t va, paddr_t pa, size_t len, uint64_t flags)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    // Convert HAL flags to MIPS-specific flags
    uint64_t mips_flags = ORION_MIPS_PAGE_KERNEL;
    if (flags & ORION_PAGE_PRESENT)
        mips_flags |= ORION_MIPS_PAGE_PRESENT;
    if (flags & ORION_PAGE_WRITABLE)
        mips_flags |= ORION_MIPS_PAGE_WRITABLE;
    if (flags & ORION_PAGE_EXECUTABLE)
        mips_flags |= ORION_MIPS_PAGE_EXECUTABLE;
    if (flags & ORION_PAGE_DEVICE)
        mips_flags |= ORION_MIPS_PAGE_DEVICE;
    if (flags & ORION_PAGE_NOCACHE)
        mips_flags |= ORION_MIPS_PAGE_NOCACHE;

    return mips_mmu_set_kernel_mapping(va, pa, len, mips_flags);
}

// ============================================================================
// INTERRUPT HANDLING FUNCTIONS
// ============================================================================

int hal_mips_interrupt_init(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_interrupts_init();
}

int hal_mips_interrupt_enable(uint32_t irq)
{
    if (!mips_hal_initialized || irq >= ORION_MIPS_IRQ_MAX)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_interrupt_enable(irq);
}

int hal_mips_interrupt_disable(uint32_t irq)
{
    if (!mips_hal_initialized || irq >= ORION_MIPS_IRQ_MAX)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_interrupt_disable(irq);
}

int hal_mips_interrupt_ack(uint32_t irq)
{
    if (!mips_hal_initialized || irq >= ORION_MIPS_IRQ_MAX)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_interrupt_ack(irq);
}

int hal_mips_interrupt_set_handler(uint32_t irq, void (*handler)(void))
{
    if (!mips_hal_initialized || irq >= ORION_MIPS_IRQ_MAX || !handler)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_interrupt_set_handler(irq, handler);
}

int hal_mips_interrupt_set_priority(uint32_t irq, uint32_t priority)
{
    if (!mips_hal_initialized || irq >= ORION_MIPS_IRQ_MAX)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_interrupt_set_priority(irq, priority);
}

int hal_mips_interrupt_get_pending(uint32_t irq)
{
    if (!mips_hal_initialized || irq >= ORION_MIPS_IRQ_MAX)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_interrupt_get_pending(irq);
}

int hal_mips_interrupt_clear_pending(uint32_t irq)
{
    if (!mips_hal_initialized || irq >= ORION_MIPS_IRQ_MAX)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_interrupt_clear_pending(irq);
}

// ============================================================================
// TIMER MANAGEMENT FUNCTIONS
// ============================================================================

int hal_mips_timer_init(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_timers_init();
}

int hal_mips_timer_set_interval(uint64_t interval_ns)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    if (interval_ns < ORION_MIPS_TIMER_MIN_INTERVAL ||
        interval_ns > ORION_MIPS_TIMER_MAX_INTERVAL)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_timer_set_interval(interval_ns);
}

int hal_mips_timer_get_current(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_timer_get_current();
}

int hal_mips_timer_enable(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_timer_enable();
}

int hal_mips_timer_disable(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_timer_disable();
}

int hal_mips_timer_set_callback(void (*callback)(void))
{
    if (!mips_hal_initialized || !callback)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_timer_set_callback(callback);
}

int hal_mips_timer_get_frequency(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_timer_get_frequency();
}

// ============================================================================
// CACHE MANAGEMENT FUNCTIONS
// ============================================================================

int hal_mips_cache_init(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_cache_init();
}

int hal_mips_cache_invalidate_data(vaddr_t va, size_t len)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_cache_invalidate_data(va, len);
}

int hal_mips_cache_clean_data(vaddr_t va, size_t len)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_cache_clean_data(va, len);
}

int hal_mips_cache_flush_data(vaddr_t va, size_t len)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_cache_flush_data(va, len);
}

int hal_mips_cache_invalidate_instruction(vaddr_t va, size_t len)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_cache_invalidate_instruction(va, len);
}

int hal_mips_cache_sync(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_cache_sync();
}

// ============================================================================
// CPU MANAGEMENT FUNCTIONS
// ============================================================================

int hal_mips_cpu_init(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_cpu_init();
}

int hal_mips_cpu_get_id(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_cpu_get_id();
}

int hal_mips_cpu_get_count(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_cpu_get_count();
}

int hal_mips_cpu_start(uint32_t cpu_id, vaddr_t entry_point)
{
    if (!mips_hal_initialized || !entry_point)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_cpu_start(cpu_id, entry_point);
}

int hal_mips_cpu_stop(uint32_t cpu_id)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_cpu_stop(cpu_id);
}

int hal_mips_cpu_wait(uint32_t cpu_id)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_cpu_wait(cpu_id);
}

int hal_mips_cpu_get_features(uint64_t *features)
{
    if (!mips_hal_initialized || !features)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_cpu_get_features(features);
}

// ============================================================================
// PERFORMANCE MONITORING FUNCTIONS
// ============================================================================

int hal_mips_performance_init(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_performance_init();
}

int hal_mips_performance_start_counter(uint32_t counter_id)
{
    if (!mips_hal_initialized || counter_id >= ORION_MIPS_PMU_COUNTERS)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_performance_start_counter(counter_id);
}

int hal_mips_performance_stop_counter(uint32_t counter_id)
{
    if (!mips_hal_initialized || counter_id >= ORION_MIPS_PMU_COUNTERS)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_performance_stop_counter(counter_id);
}

int hal_mips_performance_read_counter(uint32_t counter_id, uint64_t *value)
{
    if (!mips_hal_initialized || counter_id >= ORION_MIPS_PMU_COUNTERS || !value)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_performance_read_counter(counter_id, value);
}

int hal_mips_performance_set_event(uint32_t counter_id, uint32_t event_id)
{
    if (!mips_hal_initialized || counter_id >= ORION_MIPS_PMU_COUNTERS ||
        event_id >= ORION_MIPS_PMU_EVENTS)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_performance_set_event(counter_id, event_id);
}

int hal_mips_performance_get_cycle_count(uint64_t *cycles)
{
    if (!mips_hal_initialized || !cycles)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_performance_get_cycle_count(cycles);
}

int hal_mips_performance_get_instruction_count(uint64_t *instructions)
{
    if (!mips_hal_initialized || !instructions)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_performance_get_instruction_count(instructions);
}

// ============================================================================
// POWER MANAGEMENT FUNCTIONS
// ============================================================================

int hal_mips_power_init(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_power_init();
}

int hal_mips_power_set_state(uint32_t state)
{
    if (!mips_hal_initialized || state > ORION_MIPS_POWER_STATE_OFF)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_power_set_state(state);
}

int hal_mips_power_get_state(uint32_t *state)
{
    if (!mips_hal_initialized || !state)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_power_get_state(state);
}

int hal_mips_power_sleep(uint64_t duration_ns)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_power_sleep(duration_ns);
}

int hal_mips_power_wake(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_power_wake();
}

int hal_mips_power_set_frequency(uint32_t cpu_id, uint32_t frequency_mhz)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_power_set_frequency(cpu_id, frequency_mhz);
}

// ============================================================================
// SECURITY FEATURES FUNCTIONS
// ============================================================================

int hal_mips_security_init(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_security_init();
}

int hal_mips_security_set_trustzone(uint32_t enabled)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_security_set_trustzone(enabled);
}

int hal_mips_security_get_trustzone(uint32_t *enabled)
{
    if (!mips_hal_initialized || !enabled)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_security_get_trustzone(enabled);
}

int hal_mips_security_set_secure_world(uint32_t enabled)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_security_set_secure_world(enabled);
}

int hal_mips_security_get_secure_world(uint32_t *enabled)
{
    if (!mips_hal_initialized || !enabled)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_security_get_secure_world(enabled);
}

// ============================================================================
// DEBUG SUPPORT FUNCTIONS
// ============================================================================

int hal_mips_debug_init(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_debug_init();
}

int hal_mips_debug_set_breakpoint(vaddr_t va, uint32_t type)
{
    if (!mips_hal_initialized || type > ORION_MIPS_DEBUG_BREAKPOINT_DATA)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_debug_set_breakpoint(va, type);
}

int hal_mips_debug_clear_breakpoint(vaddr_t va)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_debug_clear_breakpoint(va);
}

int hal_mips_debug_set_watchpoint(vaddr_t va, size_t len, uint32_t type)
{
    if (!mips_hal_initialized || type > ORION_MIPS_DEBUG_WATCHPOINT_ACCESS)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_debug_set_watchpoint(va, len, type);
}

int hal_mips_debug_clear_watchpoint(vaddr_t va)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_debug_clear_watchpoint(va);
}

int hal_mips_debug_step(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_debug_step();
}

int hal_mips_debug_continue(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_debug_continue();
}

// ============================================================================
// CONTEXT SWITCHING FUNCTIONS
// ============================================================================

int hal_mips_context_switch(struct thread *from, struct thread *to)
{
    if (!mips_hal_initialized || !to)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    // Use existing MIPS context switching implementation
    return mips_context_switch(from, to);
}

int hal_mips_context_save(struct thread *thread)
{
    if (!mips_hal_initialized || !thread)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_context_save(thread);
}

int hal_mips_context_restore(struct thread *thread)
{
    if (!mips_hal_initialized || !thread)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_context_restore(thread);
}

int hal_mips_context_init(struct thread *thread, vaddr_t entry_point, vaddr_t stack_ptr)
{
    if (!mips_hal_initialized || !thread || !entry_point || !stack_ptr)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_context_init(thread, entry_point, stack_ptr);
}

// ============================================================================
// IPC FAST PATH FUNCTIONS
// ============================================================================

int hal_mips_ipc_fast_path(struct ipc_msg *msg)
{
    if (!mips_hal_initialized || !msg)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    // Use existing MIPS IPC implementation
    return mips_ipc_fast_path(msg);
}

int hal_mips_ipc_fast_path_setup(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_ipc_fast_path_setup();
}

int hal_mips_ipc_fast_path_cleanup(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_ipc_fast_path_cleanup();
}

// ============================================================================
// SYSTEM CALLS FUNCTIONS
// ============================================================================

int hal_mips_syscall_init(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    return mips_syscall_init();
}

int hal_mips_syscall_set_handler(uint32_t syscall_num, void (*handler)(void))
{
    if (!mips_hal_initialized || !handler)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_syscall_set_handler(syscall_num, handler);
}

int hal_mips_syscall_get_handler(uint32_t syscall_num, void (**handler)(void))
{
    if (!mips_hal_initialized || !handler)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    return mips_syscall_get_handler(syscall_num, handler);
}

// ============================================================================
// INITIALIZATION AND CLEANUP FUNCTIONS
// ============================================================================

int hal_mips_init(void)
{
    if (mips_hal_initialized)
    {
        return ORION_ERROR_ALREADY_EXISTS;
    }

    // Initialize MIPS architecture
    int ret = mips_arch_init();
    if (ret != ORION_MIPS_SUCCESS)
    {
        return ret;
    }

    // Initialize architecture info
    memset(&mips_arch_info, 0, sizeof(mips_arch_info));
    mips_arch_info.arch_id = ORION_ARCH_MIPS;
    mips_arch_info.arch_name = "MIPS";
    mips_arch_info.arch_version = "MIPS64";
    mips_arch_info.page_size = ORION_MIPS_PAGE_SIZE_4K;
    mips_arch_info.max_pages = 0x1000000; // 64GB virtual address space
    mips_arch_info.max_irqs = ORION_MIPS_IRQ_MAX;
    mips_arch_info.max_exceptions = ORION_MIPS_EXC_MAX;
    mips_arch_info.cache_line_size = ORION_MIPS_CACHE_LINE_SIZE;
    mips_arch_info.tlb_entries = ORION_MIPS_TLB_ENTRIES;
    mips_arch_info.pmu_counters = ORION_MIPS_PMU_COUNTERS;

    mips_hal_initialized = true;
    return ORION_HAL_SUCCESS;
}

int hal_mips_cleanup(void)
{
    if (!mips_hal_initialized)
    {
        return ORION_ERROR_INVALID_STATE;
    }

    // Cleanup MIPS architecture
    int ret = mips_arch_cleanup();
    if (ret != ORION_MIPS_SUCCESS)
    {
        return ret;
    }

    mips_hal_initialized = false;
    return ORION_HAL_SUCCESS;
}

int hal_mips_get_arch_info(struct orion_arch_info *info)
{
    if (!mips_hal_initialized || !info)
    {
        return ORION_ERROR_INVALID_ARGUMENT;
    }

    memcpy(info, &mips_arch_info, sizeof(mips_arch_info));
    return ORION_HAL_SUCCESS;
}
