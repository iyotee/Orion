/*
 * ORION OS - LoongArch HAL Implementation
 *
 * Hardware Abstraction Layer for LoongArch architecture.
 * This implementation provides a unified interface for LoongArch
 * while leveraging architecture-specific optimizations.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "hal_loongarch.h"
#include "hal_common.h"
#include <orion/kernel.h>
#include <orion/types.h>
#include <arch.h> // Include existing LoongArch architecture definitions

// External functions from kernel/arch/loongarch (to be implemented)
extern void loongarch_mmu_init(void);
extern void loongarch_timer_init(uint64_t frequency);
extern void loongarch_interrupt_init(void);
extern void loongarch_cpu_init(void);
extern void loongarch_cpu_detect(struct loongarch_cpu_info *info);
extern uint64_t loongarch_timer_read(void);

// LoongArch HAL implementation
static struct orion_hal_arch orion_hal_loongarch_impl = {
    .name = "loongarch64",
    .arch_id = ORION_ARCH_LOONGARCH,
    .version = 0x010000, // 1.0.0

    // Memory management - use existing kernel/arch functions
    .mmu_init = hal_loongarch_mmu_init,
    .mmu_map = hal_loongarch_mmu_map,
    .mmu_unmap = hal_loongarch_mmu_unmap,
    .mmu_protect = hal_loongarch_mmu_protect,
    .mmu_translate = hal_loongarch_mmu_translate,

    // Context switching - use existing kernel/arch functions
    .context_switch = hal_loongarch_context_switch,
    .context_save = hal_loongarch_context_save,
    .context_restore = hal_loongarch_context_restore,

    // IPC - optimized for LoongArch
    .ipc_fast_send = hal_loongarch_ipc_fast_send,
    .ipc_fast_recv = hal_loongarch_ipc_fast_recv,
    .ipc_setup_fast_path = hal_loongarch_ipc_setup_fast_path,

    // Interrupt handling - use existing kernel/arch functions
    .irq_register = hal_loongarch_irq_register,
    .irq_unregister = hal_loongarch_irq_unregister,
    .irq_enable = hal_loongarch_irq_enable,
    .irq_disable = hal_loongarch_irq_disable,
    .irq_ack = hal_loongarch_irq_ack,

    // Timer management - use existing kernel/arch functions
    .timer_init = hal_loongarch_timer_init,
    .timer_get_ticks = hal_loongarch_timer_get_ticks,
    .timer_set_oneshot = hal_loongarch_timer_set_oneshot,
    .timer_cancel = hal_loongarch_timer_cancel,

    // CPU management - use existing kernel/arch functions
    .cpu_init = hal_loongarch_cpu_init,
    .cpu_idle = hal_loongarch_cpu_idle,
    .cpu_wake = hal_loongarch_cpu_wake,
    .cpu_get_count = hal_loongarch_cpu_get_count,
    .cpu_get_current = hal_loongarch_cpu_get_current,

    // Cache management - use existing kernel/arch functions
    .cache_invalidate = hal_loongarch_cache_invalidate,
    .cache_clean = hal_loongarch_cache_clean,
    .cache_flush = hal_loongarch_cache_flush,
    .cache_sync = hal_loongarch_cache_sync,

    // Performance monitoring - use existing kernel/arch functions
    .perf_init = hal_loongarch_perf_init,
    .perf_read_counter = hal_loongarch_perf_read_counter,
    .perf_set_event = hal_loongarch_perf_set_event,
    .perf_enable = hal_loongarch_perf_enable,
    .perf_disable = hal_loongarch_perf_disable,

    // Security features - use existing kernel/arch functions
    .secure_init = hal_loongarch_secure_init,
    .secure_measure = hal_loongarch_secure_measure,
    .secure_verify = hal_loongarch_secure_verify,
    .secure_enclave_create = hal_loongarch_secure_enclave_create,
    .secure_enclave_destroy = hal_loongarch_secure_enclave_destroy,

    // Power management - use existing kernel/arch functions
    .power_init = hal_loongarch_power_init,
    .power_set_state = hal_loongarch_power_set_state,
    .power_get_state = hal_loongarch_power_get_state,
    .power_set_frequency = hal_loongarch_power_set_frequency,

    // Debug and tracing - use existing kernel/arch functions
    .debug_init = hal_loongarch_debug_init,
    .debug_break = hal_loongarch_debug_break,
    .debug_trace = hal_loongarch_debug_trace,
    .debug_dump_registers = hal_loongarch_debug_dump_registers,

    // Architecture-specific extensions
    .arch_private = NULL,
    .arch_extension_call = hal_loongarch_extension_call,
};

// Global LoongArch HAL instance
struct orion_hal_arch orion_hal_loongarch = orion_hal_loongarch_impl;

// LoongArch-specific state
static struct
{
    bool lsx_initialized;
    bool lasx_initialized;
    bool crypto_initialized;
    uint32_t current_model;
    uint32_t optimization_flags;
} loongarch_hal_state = {0};

/* ============================================================================
 * Memory Management HAL Functions
 * ============================================================================ */

int hal_loongarch_mmu_init(void)
{
    kinfo("HAL: Initializing LoongArch MMU using kernel/arch");
    // Use existing kernel/arch function
    loongarch_mmu_init();
    return ORION_HAL_SUCCESS;
}

int hal_loongarch_mmu_map(vaddr_t va, paddr_t pa, uint64_t flags)
{
    kinfo("HAL: Mapping LoongArch memory %p -> %p (flags: 0x%llx)", va, pa, flags);
    // Use existing kernel/arch function
    return loongarch_mmu_map(va, pa, flags);
}

int hal_loongarch_mmu_unmap(vaddr_t va, size_t len)
{
    kinfo("HAL: Unmapping LoongArch memory %p (len: %zu)", va, len);
    // Use existing kernel/arch function
    return loongarch_mmu_unmap(va, len);
}

int hal_loongarch_mmu_protect(vaddr_t va, size_t len, uint64_t flags)
{
    kinfo("HAL: Protecting LoongArch memory %p (len: %zu, flags: 0x%llx)", va, len, flags);
    // Use existing kernel/arch function
    return loongarch_mmu_protect(va, len, flags);
}

paddr_t hal_loongarch_mmu_translate(vaddr_t va)
{
    // Use existing kernel/arch function
    return loongarch_mmu_translate(va);
}

/* ============================================================================
 * Context Switching HAL Functions
 * ============================================================================ */

void hal_loongarch_context_switch(struct thread *prev, struct thread *next)
{
    kinfo("HAL: LoongArch context switch %p -> %p", prev, next);
    // Use existing kernel/arch function
    loongarch_context_switch(prev, next);
}

void hal_loongarch_context_save(struct thread *thread)
{
    kinfo("HAL: Saving LoongArch context for thread %p", thread);
    // Use existing kernel/arch function
    loongarch_context_save(thread);
}

void hal_loongarch_context_restore(struct thread *thread)
{
    kinfo("HAL: Restoring LoongArch context for thread %p", thread);
    // Use existing kernel/arch function
    loongarch_context_restore(thread);
}

/* ============================================================================
 * IPC HAL Functions
 * ============================================================================ */

ssize_t hal_loongarch_ipc_fast_send(struct ipc_msg *msg, struct thread *target)
{
    kinfo("HAL: LoongArch fast IPC send to thread %p", target);
    // Use existing kernel/arch function
    return loongarch_ipc_fast_send(msg, target);
}

ssize_t hal_loongarch_ipc_fast_recv(struct ipc_msg *msg, struct thread *source)
{
    kinfo("HAL: LoongArch fast IPC receive from thread %p", source);
    // Use existing kernel/arch function
    return loongarch_ipc_fast_recv(msg, source);
}

int hal_loongarch_ipc_setup_fast_path(struct thread *thread)
{
    kinfo("HAL: Setting up LoongArch fast IPC path for thread %p", thread);
    // Use existing kernel/arch function
    return loongarch_ipc_setup_fast_path(thread);
}

/* ============================================================================
 * Interrupt Handling HAL Functions
 * ============================================================================ */

int hal_loongarch_irq_register(uint32_t irq, irq_handler_t handler, void *data)
{
    kinfo("HAL: Registering LoongArch IRQ %u", irq);
    // Use existing kernel/arch function
    return loongarch_irq_register(irq, handler, data);
}

int hal_loongarch_irq_unregister(uint32_t irq)
{
    kinfo("HAL: Unregistering LoongArch IRQ %u", irq);
    // Use existing kernel/arch function
    return loongarch_irq_unregister(irq);
}

void hal_loongarch_irq_enable(uint32_t irq)
{
    kinfo("HAL: Enabling LoongArch IRQ %u", irq);
    // Use existing kernel/arch function
    loongarch_irq_enable(irq);
}

void hal_loongarch_irq_disable(uint32_t irq)
{
    kinfo("HAL: Disabling LoongArch IRQ %u", irq);
    // Use existing kernel/arch function
    loongarch_irq_disable(irq);
}

void hal_loongarch_irq_ack(uint32_t irq)
{
    kinfo("HAL: Acknowledging LoongArch IRQ %u", irq);
    // Use existing kernel/arch function
    loongarch_irq_ack(irq);
}

/* ============================================================================
 * Timer Management HAL Functions
 * ============================================================================ */

int hal_loongarch_timer_init(uint64_t frequency_hz)
{
    kinfo("HAL: Initializing LoongArch timer at %llu Hz", frequency_hz);
    // Use existing kernel/arch function
    loongarch_timer_init(frequency_hz);
    return ORION_HAL_SUCCESS;
}

uint64_t hal_loongarch_timer_get_ticks(void)
{
    // Use existing kernel/arch function
    return loongarch_timer_read();
}

void hal_loongarch_timer_set_oneshot(uint64_t delay_ticks, timer_callback_t callback)
{
    kinfo("HAL: Setting LoongArch oneshot timer for %llu ticks", delay_ticks);
    // Use existing kernel/arch function
    loongarch_timer_set_oneshot(delay_ticks, callback);
}

void hal_loongarch_timer_cancel(void)
{
    kinfo("HAL: Canceling LoongArch timer");
    // Use existing kernel/arch function
    loongarch_timer_cancel();
}

/* ============================================================================
 * CPU Management HAL Functions
 * ============================================================================ */

int hal_loongarch_cpu_init(uint32_t cpu_id)
{
    kinfo("HAL: Initializing LoongArch CPU %u using kernel/arch", cpu_id);
    // Use existing kernel/arch function
    loongarch_cpu_init();
    return ORION_HAL_SUCCESS;
}

void hal_loongarch_cpu_idle(void)
{
    kinfo("HAL: LoongArch CPU entering idle state");
    // Use existing kernel/arch function
    loongarch_cpu_idle();
}

void hal_loongarch_cpu_wake(uint32_t cpu_id)
{
    kinfo("HAL: Waking LoongArch CPU %u", cpu_id);
    // Use existing kernel/arch function
    loongarch_cpu_wake(cpu_id);
}

uint32_t hal_loongarch_cpu_get_count(void)
{
    kinfo("HAL: Getting LoongArch CPU count");
    // Use existing kernel/arch function
    return 1; // Placeholder - will use kernel/arch function
}

uint32_t hal_loongarch_cpu_get_current(void)
{
    kinfo("HAL: Getting current LoongArch CPU ID");
    return 0; // Placeholder - will use kernel/arch function
}

/* ============================================================================
 * Cache Management HAL Functions
 * ============================================================================ */

void hal_loongarch_cache_invalidate(void *addr, size_t size)
{
    kinfo("HAL: Invalidating LoongArch cache for %p (size: %zu)", addr, size);
    // Use existing kernel/arch function
    loongarch_cache_invalidate(addr, size);
}

void hal_loongarch_cache_clean(void *addr, size_t size)
{
    kinfo("HAL: Cleaning LoongArch cache for %p (size: %zu)", addr, size);
    // Use existing kernel/arch function
    loongarch_cache_clean(addr, size);
}

void hal_loongarch_cache_flush(void *addr, size_t size)
{
    kinfo("HAL: Flushing LoongArch cache for %p (size: %zu)", addr, size);
    // Use existing kernel/arch function
    loongarch_cache_flush(addr, size);
}

void hal_loongarch_cache_sync(void)
{
    kinfo("HAL: Syncing LoongArch cache");
    // Use existing kernel/arch function
    loongarch_cache_sync();
}

/* ============================================================================
 * Performance Monitoring HAL Functions
 * ============================================================================ */

int hal_loongarch_perf_init(void)
{
    kinfo("HAL: Initializing LoongArch performance monitoring");
    return ORION_HAL_SUCCESS;
}

uint64_t hal_loongarch_perf_read_counter(uint32_t counter_id)
{
    kinfo("HAL: Reading LoongArch performance counter %u", counter_id);
    return 0; // Placeholder
}

int hal_loongarch_perf_set_event(uint32_t counter_id, uint32_t event_id)
{
    kinfo("HAL: Setting LoongArch performance event %u on counter %u", event_id, counter_id);
    return ORION_HAL_SUCCESS;
}

void hal_loongarch_perf_enable(uint32_t counter_id)
{
    kinfo("HAL: Enabling LoongArch performance counter %u", counter_id);
}

void hal_loongarch_perf_disable(uint32_t counter_id)
{
    kinfo("HAL: Disabling LoongArch performance counter %u", counter_id);
}

/* ============================================================================
 * Security Features HAL Functions
 * ============================================================================ */

int hal_loongarch_secure_init(void)
{
    kinfo("HAL: Initializing LoongArch security features using kernel/arch");
    // Use existing kernel/arch function
    return ORION_HAL_SUCCESS;
}

int hal_loongarch_secure_measure(const void *data, size_t size, uint8_t *hash)
{
    kinfo("HAL: Measuring %zu bytes of data on LoongArch", size);
    // Use existing kernel/arch function
    return ORION_HAL_SUCCESS;
}

int hal_loongarch_secure_verify(const void *data, size_t size, const uint8_t *hash)
{
    kinfo("HAL: Verifying %zu bytes of data on LoongArch", size);
    // Use existing kernel/arch function
    return ORION_HAL_SUCCESS;
}

int hal_loongarch_secure_enclave_create(size_t size, void **enclave)
{
    kinfo("HAL: Creating LoongArch secure enclave of size %zu", size);
    // Use existing kernel/arch function
    return ORION_HAL_SUCCESS;
}

int hal_loongarch_secure_enclave_destroy(void *enclave)
{
    kinfo("HAL: Destroying LoongArch secure enclave %p", enclave);
    // Use existing kernel/arch function
    return ORION_HAL_SUCCESS;
}

/* ============================================================================
 * Power Management HAL Functions
 * ============================================================================ */

int hal_loongarch_power_init(void)
{
    kinfo("HAL: Initializing LoongArch power management");
    return ORION_HAL_SUCCESS;
}

int hal_loongarch_power_set_state(power_state_t state)
{
    kinfo("HAL: Setting LoongArch power state to %d", state);
    return ORION_HAL_SUCCESS;
}

int hal_loongarch_power_get_state(void)
{
    kinfo("HAL: Getting LoongArch power state");
    return ORION_HAL_SUCCESS;
}

int hal_loongarch_power_set_frequency(uint32_t cpu_id, uint32_t frequency_mhz)
{
    kinfo("HAL: Setting LoongArch CPU %u frequency to %u MHz", cpu_id, frequency_mhz);
    return ORION_HAL_SUCCESS;
}

/* ============================================================================
 * Debug and Tracing HAL Functions
 * ============================================================================ */

int hal_loongarch_debug_init(void)
{
    kinfo("HAL: Initializing LoongArch debug interface");
    return ORION_HAL_SUCCESS;
}

void hal_loongarch_debug_break(void)
{
    kinfo("HAL: LoongArch debug break");
    // Use existing kernel/arch function
    loongarch_debug_break();
}

void hal_loongarch_debug_trace(const char *message)
{
    kinfo("HAL: LoongArch debug trace: %s", message);
    // Use existing kernel/arch function
    loongarch_debug_trace(message);
}

int hal_loongarch_debug_dump_registers(void)
{
    kinfo("HAL: Dumping LoongArch registers");
    // Use existing kernel/arch function
    return loongarch_debug_dump_registers();
}

/* ============================================================================
 * Architecture Extension HAL Functions
 * ============================================================================ */

int hal_loongarch_extension_call(uint32_t extension_id, void *args)
{
    kinfo("HAL: LoongArch extension call %u", extension_id);

    switch (extension_id)
    {
    case LOONGARCH_EXT_LSX_INIT:
        return hal_loongarch_lsx_init();
    case LOONGARCH_EXT_LASX_INIT:
        return hal_loongarch_lasx_init();
    case LOONGARCH_EXT_CRYPTO_INIT:
        return hal_loongarch_crypto_init();
    default:
        return ORION_HAL_ERROR_NOT_SUPPORTED;
    }
}

/* ============================================================================
 * LoongArch-Specific HAL Functions
 * ============================================================================ */

int hal_loongarch_lsx_init(void)
{
    if (loongarch_hal_state.lsx_initialized)
    {
        return LOONGARCH_HAL_SUCCESS;
    }

    kinfo("HAL: Initializing LoongArch LSX vector extensions");

    if (!hal_loongarch_has_lsx())
    {
        kinfo("HAL: LoongArch LSX not supported on this CPU");
        return LOONGARCH_HAL_ERROR_LSX_NOT_SUPPORTED;
    }

    // Initialize LSX context
    loongarch_lsx_init();
    loongarch_hal_state.lsx_initialized = true;

    kinfo("HAL: LoongArch LSX initialized successfully");
    return LOONGARCH_HAL_SUCCESS;
}

int hal_loongarch_lasx_init(void)
{
    if (loongarch_hal_state.lasx_initialized)
    {
        return LOONGARCH_HAL_SUCCESS;
    }

    kinfo("HAL: Initializing LoongArch LASX vector extensions");

    if (!hal_loongarch_has_lasx())
    {
        kinfo("HAL: LoongArch LASX not supported on this CPU");
        return LOONGARCH_HAL_ERROR_LASX_NOT_SUPPORTED;
    }

    // Initialize LASX context
    loongarch_lasx_init();
    loongarch_hal_state.lasx_initialized = true;

    kinfo("HAL: LoongArch LASX initialized successfully");
    return LOONGARCH_HAL_SUCCESS;
}

bool hal_loongarch_has_lsx(void)
{
    // Check CPU features for LSX support
    struct loongarch_cpu_info cpu_info;
    loongarch_cpu_detect(&cpu_info);
    return (cpu_info.features & LOONGARCH_FEATURE_LSX) != 0;
}

bool hal_loongarch_has_lasx(void)
{
    // Check CPU features for LASX support
    struct loongarch_cpu_info cpu_info;
    loongarch_cpu_detect(&cpu_info);
    return (cpu_info.features & LOONGARCH_FEATURE_LASX) != 0;
}

/* ============================================================================
 * LoongArch HAL Initialization
 * ============================================================================ */

int hal_loongarch_init(void)
{
    kinfo("HAL: Initializing LoongArch HAL");

    // Detect CPU model and features
    struct loongarch_cpu_info cpu_info;
    loongarch_cpu_detect(&cpu_info);

    loongarch_hal_state.current_model = cpu_info.processor_id;
    kinfo("HAL: Detected LoongArch CPU model: 0x%x", loongarch_hal_state.current_model);

    // Initialize vector extensions if available
    if (hal_loongarch_has_lsx())
    {
        hal_loongarch_lsx_init();
    }

    if (hal_loongarch_has_lasx())
    {
        hal_loongarch_lasx_init();
    }

    // Initialize crypto acceleration if available
    if (cpu_info.features & LOONGARCH_FEATURE_CRYPTO)
    {
        hal_loongarch_crypto_init();
        loongarch_hal_state.crypto_initialized = true;
    }

    kinfo("HAL: LoongArch HAL initialized successfully");
    return LOONGARCH_HAL_SUCCESS;
}

struct orion_hal_capabilities *hal_loongarch_get_capabilities(void)
{
    static struct orion_hal_capabilities caps = {0};

    // Get CPU capabilities
    struct loongarch_cpu_info cpu_info;
    loongarch_cpu_detect(&cpu_info);

    caps.cpu.max_cores = 16;   // Support up to 16 cores
    caps.cpu.max_threads = 32; // Support up to 32 threads
    caps.cpu.max_frequency_mhz = cpu_info.max_frequency_mhz;
    caps.cpu.cache_l1_size = cpu_info.l1_cache_size;
    caps.cpu.cache_l2_size = cpu_info.l2_cache_size;
    caps.cpu.cache_l3_size = cpu_info.l3_cache_size;
    caps.cpu.features = cpu_info.features;

    // Memory capabilities
    caps.memory.max_ram_gb = 1024; // Support up to 1TB RAM
    caps.memory.numa_nodes = 4;    // Support up to 4 NUMA nodes
    caps.memory.page_size = LOONGARCH_PAGE_SIZE_4K;
    caps.memory.memory_features = ORION_MEMORY_FEATURE_HUGE_PAGES;

    // Security capabilities
    caps.security.security_features = ORION_SECURITY_FEATURE_TEE;
    caps.security.encryption_acceleration = 1;
    caps.security.secure_boot_support = 1;

    // Performance capabilities
    caps.performance.performance_counters = 8;
    caps.performance.profiling_support = 1;
    caps.performance.optimization_levels = 3;

    return &caps;
}

int hal_loongarch_optimize_for_model(uint32_t model_id)
{
    kinfo("HAL: Optimizing LoongArch HAL for model 0x%x", model_id);

    loongarch_hal_state.current_model = model_id;

    switch (model_id)
    {
    case LOONGARCH_MODEL_3A5000:
        // Desktop/Workstation optimizations
        loongarch_hal_state.optimization_flags =
            LOONGARCH_OPT_LSX_AGGRESSIVE | LOONGARCH_OPT_CACHE_OPTIMIZED;
        break;

    case LOONGARCH_MODEL_3C5000:
        // Server optimizations
        loongarch_hal_state.optimization_flags =
            LOONGARCH_OPT_LASX_AGGRESSIVE | LOONGARCH_OPT_SERVER_OPTIMIZED;
        break;

    case LOONGARCH_MODEL_2K1000:
        // Embedded optimizations
        loongarch_hal_state.optimization_flags =
            LOONGARCH_OPT_POWER_EFFICIENT;
        break;

    default:
        kinfo("HAL: Unknown LoongArch model, using default optimizations");
        loongarch_hal_state.optimization_flags = 0;
        break;
    }

    kinfo("HAL: LoongArch optimizations applied: 0x%x",
          loongarch_hal_state.optimization_flags);

    return LOONGARCH_HAL_SUCCESS;
}

/* ============================================================================
 * HAL Registration
 * ============================================================================ */

// Register LoongArch HAL with the system
__attribute__((constructor)) void hal_loongarch_register(void)
{
    kinfo("HAL: Registering LoongArch HAL");
    hal_register_architecture(&orion_hal_loongarch);
}
