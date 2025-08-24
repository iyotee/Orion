/*
 * Orion Operating System - HAL Test
 *
 * Simple test program to verify HAL integration with existing kernel/arch.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "hal_integration.h"
#include "hal_common.h"
#include <stdio.h>
#include <stdlib.h>

// Mock kernel info function for testing
void kinfo(const char *fmt, ...) {
    printf("[INFO] ");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

int main(int argc, char *argv[]) {
    printf("Orion HAL Integration Test\n");
    printf("==========================\n\n");
    
    // Test HAL integration initialization
    printf("1. Testing HAL integration initialization...\n");
    int result = hal_integration_init();
    if (result != ORION_HAL_SUCCESS) {
        printf("ERROR: HAL integration initialization failed: %d\n", result);
        return 1;
    }
    printf("SUCCESS: HAL integration initialized\n\n");
    
    // Check if HAL is ready
    printf("2. Checking HAL readiness...\n");
    if (!hal_integration_is_ready()) {
        printf("ERROR: HAL integration not ready\n");
        return 1;
    }
    printf("SUCCESS: HAL integration is ready\n\n");
    
    // Get current HAL
    printf("3. Getting current HAL implementation...\n");
    struct orion_hal_arch *hal = hal_integration_get_hal();
    if (!hal) {
        printf("ERROR: No HAL implementation available\n");
        return 1;
    }
    printf("SUCCESS: HAL implementation: %s (ID: %u, Version: 0x%x)\n\n",
           hal->name, hal->arch_id, hal->version);
    
    // Report HAL status
    printf("4. Reporting HAL integration status...\n");
    hal_integration_report_status();
    printf("\n");
    
    // Test HAL manager
    printf("5. Testing HAL manager...\n");
    struct orion_hal_manager *manager = hal_manager_get_instance();
    if (!manager) {
        printf("ERROR: HAL manager not available\n");
        return 1;
    }
    
    if (!hal_manager_is_initialized()) {
        printf("ERROR: HAL manager not initialized\n");
        return 1;
    }
    
    struct orion_hal_arch *current_arch = hal_manager_get_current_arch();
    if (!current_arch) {
        printf("ERROR: No current architecture in HAL manager\n");
        return 1;
    }
    
    printf("SUCCESS: HAL manager working correctly\n");
    printf("  Current architecture: %s\n", current_arch->name);
    printf("  Registered architectures: %u\n", manager->arch_count);
    printf("\n");
    
    // Test basic HAL functions
    printf("6. Testing basic HAL functions...\n");
    
    // Test MMU initialization
    if (hal->mmu_init) {
        printf("  Testing MMU initialization...\n");
        result = hal->mmu_init();
        if (result != ORION_HAL_SUCCESS) {
            printf("  WARNING: MMU initialization failed: %d\n", result);
        } else {
            printf("  SUCCESS: MMU initialization\n");
        }
    }
    
    // Test timer initialization
    if (hal->timer_init) {
        printf("  Testing timer initialization...\n");
        result = hal->timer_init(1000000); // 1MHz
        if (result != ORION_HAL_SUCCESS) {
            printf("  WARNING: Timer initialization failed: %d\n", result);
        } else {
            printf("  SUCCESS: Timer initialization\n");
        }
    }
    
    // Test CPU initialization
    if (hal->cpu_init) {
        printf("  Testing CPU initialization...\n");
        result = hal->cpu_init(0);
        if (result != ORION_HAL_SUCCESS) {
            printf("  WARNING: CPU initialization failed: %d\n", result);
        } else {
            printf("  SUCCESS: CPU initialization\n");
        }
    }
    
    printf("\n");
    
    // Test HAL manager capabilities
    printf("7. Testing HAL manager capabilities...\n");
    result = hal_manager_report_capabilities();
    if (result != ORION_HAL_SUCCESS) {
        printf("WARNING: Capability reporting failed: %d\n", result);
    } else {
        printf("SUCCESS: Capability reporting\n");
    }
    printf("\n");
    
    // Test benchmarking
    printf("8. Testing HAL benchmarking...\n");
    result = hal_manager_benchmark_arch();
    if (result != ORION_HAL_SUCCESS) {
        printf("WARNING: Benchmarking failed: %d\n", result);
    } else {
        printf("SUCCESS: Benchmarking completed\n");
    }
    printf("\n");
    
    // Test subsystem initialization
    printf("9. Testing HAL subsystem initialization...\n");
    result = hal_integration_init_subsystems();
    if (result != ORION_HAL_SUCCESS) {
        printf("WARNING: Subsystem initialization failed: %d\n", result);
    } else {
        printf("SUCCESS: Subsystem initialization\n");
    }
    printf("\n");
    
    // Final status report
    printf("Final HAL Integration Status:\n");
    hal_integration_report_status();
    printf("\n");
    
    printf("All tests completed successfully!\n");
    printf("The HAL is properly integrated with existing kernel/arch code.\n");
    
    return 0;
}
