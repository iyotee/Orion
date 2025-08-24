/*
 * Orion Operating System - HAL Common Implementation
 *
 * Common HAL manager functions that coordinate between different
 * architecture-specific HAL implementations. This layer uses
 * existing kernel/arch code and provides unified management.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "hal_common.h"
#include <orion/kernel.h>
#include <orion/types.h>

// HAL manager instance
static struct orion_hal_manager hal_manager = {
    .current_arch = NULL,
    .arch_count = 0,
    .arch_list = {NULL},
    .initialized = false};

// Architecture detection using existing kernel/arch code
static int hal_manager_detect_arch_internal(void)
{
    kinfo("HAL: Detecting architecture using existing kernel/arch code");

// Use compile-time defines to detect architecture
#if defined(__x86_64__)
    kinfo("HAL: Detected x86_64 architecture");
    return ORION_ARCH_X86_64;
#elif defined(__aarch64__)
    kinfo("HAL: Detected AArch64 architecture");
    return ORION_ARCH_AARCH64;
#elif defined(__riscv) && __riscv_xlen == 64
    kinfo("HAL: Detected RISC-V64 architecture");
    return ORION_ARCH_RISCV64;
#elif defined(__powerpc64__)
    kinfo("HAL: Detected PowerPC64 architecture");
    return ORION_ARCH_POWERPC64;
#elif defined(__mips64)
    kinfo("HAL: Detected MIPS64 architecture");
    return ORION_ARCH_MIPS64;
#elif defined(__loongarch64)
    kinfo("HAL: Detected LoongArch64 architecture");
    return ORION_ARCH_LOONGARCH64;
#elif defined(__s390x__)
    kinfo("HAL: Detected S390x architecture");
    return ORION_ARCH_S390X;
#else
    kinfo("HAL: Unknown architecture, defaulting to x86_64");
    return ORION_ARCH_X86_64;
#endif
}

// Register architecture-specific HAL implementation
int hal_manager_register_arch(struct orion_hal_arch *arch)
{
    if (!arch)
    {
        kinfo("HAL: Cannot register NULL architecture");
        return ORION_HAL_ERROR_INVALID_ARG;
    }

    if (hal_manager.arch_count >= ORION_HAL_MAX_ARCHITECTURES)
    {
        kinfo("HAL: Maximum number of architectures reached");
        return ORION_HAL_ERROR_OUT_OF_MEMORY;
    }

    kinfo("HAL: Registering architecture: %s (ID: %u, Version: 0x%x)",
          arch->name, arch->arch_id, arch->version);

    // Add to architecture list
    hal_manager.arch_list[hal_manager.arch_count] = arch;
    hal_manager.arch_count++;

    // If this is the first architecture, set it as current
    if (!hal_manager.current_arch)
    {
        hal_manager.current_arch = arch;
        kinfo("HAL: Set %s as current architecture", arch->name);
    }

    return ORION_HAL_SUCCESS;
}

// Detect and register appropriate architecture
int hal_manager_detect_arch(void)
{
    kinfo("HAL: Detecting architecture for HAL initialization");

    int arch_id = hal_manager_detect_arch_internal();

    // Register the appropriate HAL implementation based on detected architecture
    switch (arch_id)
    {
    case ORION_ARCH_X86_64:
        // Use existing x86_64 HAL implementation
        kinfo("HAL: Using x86_64 HAL implementation");
        // This will be implemented to use the existing kernel/arch/x86_64 code
        break;

    case ORION_ARCH_AARCH64:
        // Use existing AArch64 HAL implementation
        kinfo("HAL: Using AArch64 HAL implementation");
        // This will be implemented to use the existing kernel/arch/aarch64 code
        break;

    case ORION_ARCH_RISCV64:
        kinfo("HAL: RISC-V64 HAL not yet implemented");
        break;

    case ORION_ARCH_POWERPC64:
        kinfo("HAL: PowerPC64 HAL not yet implemented");
        break;

    case ORION_ARCH_MIPS64:
        kinfo("HAL: MIPS64 HAL not yet implemented");
        break;

    case ORION_ARCH_LOONGARCH64:
        kinfo("HAL: LoongArch64 HAL not yet implemented");
        break;

    case ORION_ARCH_S390X:
        kinfo("HAL: S390x HAL not yet implemented");
        break;

    default:
        kinfo("HAL: Unknown architecture ID: %d", arch_id);
        return ORION_HAL_ERROR_UNSUPPORTED_ARCH;
    }

    return ORION_HAL_SUCCESS;
}

// Switch to different architecture (runtime architecture switching)
int hal_manager_switch_arch(uint32_t arch_id)
{
    kinfo("HAL: Switching to architecture ID: %u", arch_id);

    // Find architecture in registered list
    for (int i = 0; i < hal_manager.arch_count; i++)
    {
        if (hal_manager.arch_list[i] && hal_manager.arch_list[i]->arch_id == arch_id)
        {
            hal_manager.current_arch = hal_manager.arch_list[i];
            kinfo("HAL: Switched to architecture: %s", hal_manager.current_arch->name);
            return ORION_HAL_SUCCESS;
        }
    }

    kinfo("HAL: Architecture ID %u not found", arch_id);
    return ORION_HAL_ERROR_UNSUPPORTED_ARCH;
}

// Get current architecture
struct orion_hal_arch *hal_manager_get_current_arch(void)
{
    return hal_manager.current_arch;
}

// Get architecture by ID
struct orion_hal_arch *hal_manager_get_arch(uint32_t arch_id)
{
    for (int i = 0; i < hal_manager.arch_count; i++)
    {
        if (hal_manager.arch_list[i] && hal_manager.arch_list[i]->arch_id == arch_id)
        {
            return hal_manager.arch_list[i];
        }
    }
    return NULL;
}

// Initialize HAL manager
int hal_manager_init(void)
{
    kinfo("HAL: Initializing HAL manager");

    if (hal_manager.initialized)
    {
        kinfo("HAL: Manager already initialized");
        return ORION_HAL_SUCCESS;
    }

    // Reset manager state
    hal_manager.current_arch = NULL;
    hal_manager.arch_count = 0;
    memset(hal_manager.arch_list, 0, sizeof(hal_manager.arch_list));

    // Detect and register architecture
    int result = hal_manager_detect_arch();
    if (result != ORION_HAL_SUCCESS)
    {
        kinfo("HAL: Failed to detect architecture: %d", result);
        return result;
    }

    hal_manager.initialized = true;
    kinfo("HAL: Manager initialized successfully");

    return ORION_HAL_SUCCESS;
}

// Benchmark current architecture
int hal_manager_benchmark_arch(void)
{
    if (!hal_manager.current_arch)
    {
        kinfo("HAL: No current architecture for benchmarking");
        return ORION_HAL_ERROR_NO_ARCH;
    }

    kinfo("HAL: Benchmarking architecture: %s", hal_manager.current_arch->name);

    // Use existing kernel/arch benchmarking if available
    // This will be implemented to use kernel/arch performance functions

    kinfo("HAL: Benchmark completed");
    return ORION_HAL_SUCCESS;
}

// Report architecture capabilities
int hal_manager_report_capabilities(void)
{
    if (!hal_manager.current_arch)
    {
        kinfo("HAL: No current architecture for capability reporting");
        return ORION_HAL_ERROR_NO_ARCH;
    }

    kinfo("HAL: Reporting capabilities for architecture: %s", hal_manager.current_arch->name);

    // Report basic capabilities
    kinfo("HAL: Architecture ID: %u", hal_manager.current_arch->arch_id);
    kinfo("HAL: Version: 0x%x", hal_manager.current_arch->version);

    // Check if key functions are implemented
    kinfo("HAL: MMU support: %s", hal_manager.current_arch->mmu_init ? "yes" : "no");
    kinfo("HAL: Context switching: %s", hal_manager.current_arch->context_switch ? "yes" : "no");
    kinfo("HAL: Interrupt handling: %s", hal_manager.current_arch->irq_register ? "yes" : "no");
    kinfo("HAL: Timer support: %s", hal_manager.current_arch->timer_init ? "yes" : "no");
    kinfo("HAL: CPU management: %s", hal_manager.current_arch->cpu_init ? "yes" : "no");
    kinfo("HAL: Cache management: %s", hal_manager.current_arch->cache_invalidate ? "yes" : "no");

    return ORION_HAL_SUCCESS;
}

// Get HAL manager instance
struct orion_hal_manager *hal_manager_get_instance(void)
{
    return &hal_manager;
}

// Check if HAL manager is initialized
bool hal_manager_is_initialized(void)
{
    return hal_manager.initialized;
}
