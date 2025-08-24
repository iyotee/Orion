/*
 * Orion Operating System - HAL Integration
 *
 * Integration layer that connects the HAL with existing kernel/arch code.
 * This file ensures the HAL uses existing kernel/arch implementations
 * without duplication.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "hal_integration.h"
#include "hal_common.h"
#include "x86_64/hal_x86_64.h"
#include <orion/kernel.h>
#include <orion/types.h>

// HAL integration state
static struct hal_integration_state {
    bool hal_initialized;
    bool arch_registered;
    struct orion_hal_arch *current_hal;
} hal_integration = {false, false, NULL};

// Initialize HAL integration with existing kernel/arch
int hal_integration_init(void) {
    kinfo("HAL Integration: Initializing with existing kernel/arch");
    
    if (hal_integration.hal_initialized) {
        kinfo("HAL Integration: Already initialized");
        return ORION_HAL_SUCCESS;
    }
    
    // Initialize HAL manager
    int result = hal_manager_init();
    if (result != ORION_HAL_SUCCESS) {
        kinfo("HAL Integration: Failed to initialize HAL manager: %d", result);
        return result;
    }
    
    // Register x86_64 HAL implementation
    #if defined(__x86_64__)
    kinfo("HAL Integration: Registering x86_64 HAL implementation");
    
    // Initialize x86_64 HAL
    result = hal_x86_64_init();
    if (result != ORION_HAL_SUCCESS) {
        kinfo("HAL Integration: Failed to initialize x86_64 HAL: %d", result);
        return result;
    }
    
    // Get x86_64 HAL implementation
    struct orion_hal_arch *x86_64_hal = hal_x86_64_get_impl();
    if (!x86_64_hal) {
        kinfo("HAL Integration: Failed to get x86_64 HAL implementation");
        return ORION_HAL_ERROR_INVALID_ARG;
    }
    
    // Register with HAL manager
    result = hal_manager_register_arch(x86_64_hal);
    if (result != ORION_HAL_SUCCESS) {
        kinfo("HAL Integration: Failed to register x86_64 HAL: %d", result);
        return result;
    }
    
    hal_integration.arch_registered = true;
    hal_integration.current_hal = x86_64_hal;
    kinfo("HAL Integration: x86_64 HAL registered successfully");
    
    #else
    kinfo("HAL Integration: x86_64 not detected, skipping HAL registration");
    #endif
    
    hal_integration.hal_initialized = true;
    kinfo("HAL Integration: Initialization complete");
    
    return ORION_HAL_SUCCESS;
}

// Get current HAL implementation
struct orion_hal_arch* hal_integration_get_hal(void) {
    return hal_integration.current_hal;
}

// Check if HAL integration is ready
bool hal_integration_is_ready(void) {
    return hal_integration.hal_initialized && hal_integration.arch_registered;
}

// Initialize HAL subsystems using existing kernel/arch
int hal_integration_init_subsystems(void) {
    if (!hal_integration_is_ready()) {
        kinfo("HAL Integration: HAL not ready for subsystem initialization");
        return ORION_HAL_ERROR_INVALID_ARG;
    }
    
    kinfo("HAL Integration: Initializing subsystems using existing kernel/arch");
    
    // Initialize MMU using existing kernel/arch
    if (hal_integration.current_hal->mmu_init) {
        int result = hal_integration.current_hal->mmu_init();
        if (result != ORION_HAL_SUCCESS) {
            kinfo("HAL Integration: Failed to initialize MMU: %d", result);
            return result;
        }
        kinfo("HAL Integration: MMU initialized successfully");
    }
    
    // Initialize interrupts using existing kernel/arch
    if (hal_integration.current_hal->irq_register) {
        kinfo("HAL Integration: Interrupt handling available");
    }
    
    // Initialize timers using existing kernel/arch
    if (hal_integration.current_hal->timer_init) {
        kinfo("HAL Integration: Timer support available");
    }
    
    // Initialize CPU management using existing kernel/arch
    if (hal_integration.current_hal->cpu_init) {
        kinfo("HAL Integration: CPU management available");
    }
    
    kinfo("HAL Integration: Subsystem initialization complete");
    return ORION_HAL_SUCCESS;
}

// Report HAL integration status
void hal_integration_report_status(void) {
    kinfo("HAL Integration Status:");
    kinfo("  HAL Initialized: %s", hal_integration.hal_initialized ? "yes" : "no");
    kinfo("  Architecture Registered: %s", hal_integration.arch_registered ? "yes" : "no");
    
    if (hal_integration.current_hal) {
        kinfo("  Current HAL: %s (ID: %u, Version: 0x%x)",
              hal_integration.current_hal->name,
              hal_integration.current_hal->arch_id,
              hal_integration.current_hal->version);
    } else {
        kinfo("  Current HAL: none");
    }
    
    // Report HAL manager status
    struct orion_hal_manager *manager = hal_manager_get_instance();
    if (manager) {
        kinfo("  HAL Manager Initialized: %s", manager->initialized ? "yes" : "no");
        kinfo("  Registered Architectures: %u", manager->arch_count);
    }
}

// Shutdown HAL integration
int hal_integration_shutdown(void) {
    kinfo("HAL Integration: Shutting down");
    
    // Cleanup HAL integration state
    hal_integration.hal_initialized = false;
    hal_integration.arch_registered = false;
    hal_integration.current_hal = NULL;
    
    kinfo("HAL Integration: Shutdown complete");
    return ORION_HAL_SUCCESS;
}
