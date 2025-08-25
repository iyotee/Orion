/*
 * ORION OS - RISC-V 64-bit CPU Management
 *
 * CPU-specific management for RISC-V 64-bit architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// GLOBAL CPU CONTEXTS
// ============================================================================

// CPU features context
static struct riscv64_cpu_features riscv64_cpu_features = {0};

// CPU state context
static struct {
    bool initialized;
    uint32_t current_mode;
    uint32_t supported_modes;
    uint64_t hart_id;
    uint64_t cpu_frequency;
    uint64_t cpu_revision;
    uint64_t cpu_vendor_id;
    uint64_t cpu_architecture_id;
    uint64_t cpu_implementation_id;
} riscv64_cpu_context = {0};

// Performance monitoring context
static struct {
    bool initialized;
    uint64_t cycle_counter;
    uint64_t instruction_counter;
    uint64_t time_counter;
    uint64_t performance_counters[32];
    bool counters_enabled;
} riscv64_performance_context = {0};

// ============================================================================
// CPU INITIALIZATION
// ============================================================================

// Initialize CPU system
void riscv64_cpu_init(void)
{
    if (riscv64_cpu_context.initialized) {
        return;
    }
    
    // Detect CPU features
    riscv64_detect_cpu_features();
    
    // Initialize CPU context
    riscv64_cpu_context.current_mode = RISCV64_MODE_M;  // Start in machine mode
    riscv64_cpu_context.supported_modes = RISCV64_MODE_M | RISCV64_MODE_S | RISCV64_MODE_U;
    riscv64_cpu_context.hart_id = riscv64_get_hart_id();
    riscv64_cpu_context.cpu_frequency = riscv64_get_cpu_frequency();
    riscv64_cpu_context.cpu_revision = riscv64_get_cpu_revision();
    riscv64_cpu_context.cpu_vendor_id = riscv64_cpu_features.vendor_id;
    riscv64_cpu_context.cpu_architecture_id = riscv64_cpu_features.architecture_id;
    riscv64_cpu_context.cpu_implementation_id = riscv64_cpu_features.implementation_id;
    riscv64_cpu_context.initialized = true;
    
    // Initialize performance monitoring
    riscv64_performance_context.initialized = true;
    riscv64_performance_context.counters_enabled = false;
    
    printf("RISC-V64: CPU initialized - Hart ID: %llu, Frequency: %llu MHz\n", 
           riscv64_cpu_context.hart_id, riscv64_cpu_context.cpu_frequency / 1000000);
}

// ============================================================================
// CPU FEATURE DETECTION
// ============================================================================

// Detect CPU features
void riscv64_detect_cpu_features(void)
{
    uint64_t misa;
    asm volatile("csrr %0, misa" : "=r"(misa));
    
    // Extract base ISA
    riscv64_cpu_features.base_isa = misa & 0x3F;
    
    // Extract extensions
    riscv64_cpu_features.extensions = (misa >> 26) & 0x3F;
    
    // Extract MXL (machine XLEN)
    riscv64_cpu_features.mxl = (misa >> 62) & 0x3;
    
    // Extract SXL (supervisor XLEN)
    riscv64_cpu_features.sxl = (misa >> 34) & 0x3;
    
    // Extract UXL (user XLEN)
    riscv64_cpu_features.uxl = (misa >> 32) & 0x3;
    
    // Get hart ID
    asm volatile("csrr %0, mhartid" : "=r"(riscv64_cpu_features.hart_id));
    
    // Get vendor ID
    asm volatile("csrr %0, mvendorid" : "=r"(riscv64_cpu_features.vendor_id));
    
    // Get architecture ID
    asm volatile("csrr %0, marchid" : "=r"(riscv64_cpu_features.architecture_id));
    
    // Get implementation ID
    asm volatile("csrr %0, mimpid" : "=r"(riscv64_cpu_features.implementation_id));
    
    printf("RISC-V64: CPU features detected\n");
    printf("  Base ISA: 0x%llx\n", riscv64_cpu_features.base_isa);
    printf("  Extensions: 0x%llx\n", riscv64_cpu_features.extensions);
    printf("  MXL: %llu\n", riscv64_cpu_features.mxl);
    printf("  SXL: %llu\n", riscv64_cpu_features.sxl);
    printf("  UXL: %llu\n", riscv64_cpu_features.uxl);
    printf("  Hart ID: %llu\n", riscv64_cpu_features.hart_id);
    printf("  Vendor ID: 0x%llx\n", riscv64_cpu_features.vendor_id);
    printf("  Architecture ID: 0x%llx\n", riscv64_cpu_features.architecture_id);
    printf("  Implementation ID: 0x%llx\n", riscv64_cpu_features.implementation_id);
}

// Check if CPU has specific feature
bool riscv64_has_feature(uint64_t feature)
{
    if (feature & RISCV64_FEATURE_RV64I) {
        return (riscv64_cpu_features.base_isa & 0x1) != 0;
    }
    
    if (feature & RISCV64_FEATURE_RV64M) {
        return (riscv64_cpu_features.extensions & 0x1) != 0;
    }
    
    if (feature & RISCV64_FEATURE_RV64A) {
        return (riscv64_cpu_features.extensions & 0x2) != 0;
    }
    
    if (feature & RISCV64_FEATURE_RV64F) {
        return (riscv64_cpu_features.extensions & 0x4) != 0;
    }
    
    if (feature & RISCV64_FEATURE_RV64D) {
        return (riscv64_cpu_features.extensions & 0x8) != 0;
    }
    
    if (feature & RISCV64_FEATURE_RV64C) {
        return (riscv64_cpu_features.extensions & 0x10) != 0;
    }
    
    if (feature & RISCV64_FEATURE_RVV) {
        return (riscv64_cpu_features.extensions & 0x20) != 0;
    }
    
    return false;
}

// ============================================================================
// CPU MODE MANAGEMENT
// ============================================================================

// Set CPU mode
int riscv64_cpu_set_mode(uint32_t mode)
{
    if (!riscv64_cpu_context.initialized) {
        return -1;
    }
    
    if (!(riscv64_cpu_context.supported_modes & mode)) {
        printf("RISC-V64: CPU mode 0x%x not supported\n", mode);
        return -1;
    }
    
    uint64_t mstatus;
    asm volatile("csrr %0, mstatus" : "=r"(mstatus));
    
    // Clear current MPP bits
    mstatus &= ~(0x3ULL << 11);
    
    // Set new MPP bits
    mstatus |= ((uint64_t)mode << 11);
    
    // Write back to mstatus
    asm volatile("csrw mstatus, %0" : : "r"(mstatus));
    
    riscv64_cpu_context.current_mode = mode;
    
    printf("RISC-V64: CPU mode set to 0x%x\n", mode);
    return 0;
}

// Get current CPU mode
uint32_t riscv64_cpu_get_mode(void)
{
    if (!riscv64_cpu_context.initialized) {
        return RISCV64_MODE_M;
    }
    
    return riscv64_cpu_context.current_mode;
}

// Check if CPU is in privileged mode
bool riscv64_cpu_is_privileged(void)
{
    uint32_t mode = riscv64_cpu_get_mode();
    return mode != RISCV64_MODE_U;
}

// ============================================================================
// CPU INFORMATION FUNCTIONS
// ============================================================================

// Get CPU information
struct riscv64_cpu_features* riscv64_get_cpu_info(void)
{
    if (!riscv64_cpu_context.initialized) {
        return NULL;
    }
    
    return &riscv64_cpu_features;
}

// Get CPU frequency
uint64_t riscv64_get_cpu_frequency(void)
{
    // This is typically a fixed value for RISC-V
    // In a real system, this would be read from device tree or configuration
    return RISCV64_CPU_MAX_FREQ_MHZ * 1000000ULL;
}

// Get CPU revision
uint64_t riscv64_get_cpu_revision(void)
{
    // Read from mimpid CSR
    uint64_t mimpid;
    asm volatile("csrr %0, mimpid" : "=r"(mimpid));
    return mimpid;
}

// Get hart ID
uint64_t riscv64_get_hart_id(void)
{
    uint64_t hartid;
    asm volatile("csrr %0, mhartid" : "=r"(hartid));
    return hartid;
}

// Get CPU vendor ID
uint64_t riscv64_get_cpu_vendor_id(void)
{
    uint64_t vendorid;
    asm volatile("csrr %0, mvendorid" : "=r"(vendorid));
    return vendorid;
}

// Get CPU architecture ID
uint64_t riscv64_get_cpu_architecture_id(void)
{
    uint64_t archid;
    asm volatile("csrr %0, marchid" : "=r"(archid));
    return archid;
}

// Get CPU implementation ID
uint64_t riscv64_get_cpu_implementation_id(void)
{
    uint64_t impid;
    asm volatile("csrr %0, mimpid" : "=r"(impid));
    return impid;
}

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

// Enable performance counters
void riscv64_performance_enable_counters(void)
{
    if (!riscv64_performance_context.initialized) {
        return;
    }
    
    riscv64_performance_context.counters_enabled = true;
    
    // Enable cycle counter
    uint64_t mcounteren = 1;
    asm volatile("csrw mcounteren, %0" : : "r"(mcounteren));
    
    // Enable instruction counter
    uint64_t mcounteren_inst = 2;
    asm volatile("csrw mcounteren, %0" : : "r"(mcounteren_inst));
    
    printf("RISC-V64: Performance counters enabled\n");
}

// Disable performance counters
void riscv64_performance_disable_counters(void)
{
    if (!riscv64_performance_context.initialized) {
        return;
    }
    
    riscv64_performance_context.counters_enabled = false;
    
    // Disable all counters
    asm volatile("csrw mcounteren, zero");
    
    printf("RISC-V64: Performance counters disabled\n");
}

// Read performance counter
uint64_t riscv64_performance_read_counter(uint32_t counter)
{
    if (!riscv64_performance_context.initialized || counter >= 32) {
        return 0;
    }
    
    uint64_t value = 0;
    
    switch (counter) {
        case 0:  // Cycle counter
            asm volatile("csrr %0, cycle" : "=r"(value));
            break;
        case 1:  // Instruction counter
            asm volatile("csrr %0, instret" : "=r"(value));
            break;
        case 2:  // Time counter
            asm volatile("csrr %0, time" : "=r"(value));
            break;
        default:  // Hardware performance counters
            if (counter >= 3 && counter <= 31) {
                asm volatile("csrr %0, mhpmcounter%1" : "=r"(value) : "i"(counter));
            }
            break;
    }
    
    riscv64_performance_context.performance_counters[counter] = value;
    return value;
}

// Write performance counter
void riscv64_performance_write_counter(uint32_t counter, uint64_t value)
{
    if (!riscv64_performance_context.initialized || counter >= 32) {
        return;
    }
    
    switch (counter) {
        case 0:  // Cycle counter
            asm volatile("csrw cycle, %0" : : "r"(value));
            break;
        case 1:  // Instruction counter
            asm volatile("csrw instret, %0" : : "r"(value));
            break;
        case 2:  // Time counter
            asm volatile("csrw time, %0" : : "r"(value));
            break;
        default:  // Hardware performance counters
            if (counter >= 3 && counter <= 31) {
                asm volatile("csrw mhpmcounter%0, %1" : : "i"(counter), "r"(value));
            }
            break;
    }
    
    riscv64_performance_context.performance_counters[counter] = value;
}

// Reset performance counter
void riscv64_performance_reset_counter(uint32_t counter)
{
    riscv64_performance_write_counter(counter, 0);
}

// Get performance counter value
uint64_t riscv64_performance_get_counter_value(uint32_t counter)
{
    if (!riscv64_performance_context.initialized || counter >= 32) {
        return 0;
    }
    
    return riscv64_performance_context.performance_counters[counter];
}

// ============================================================================
// CPU CONTROL FUNCTIONS
// ============================================================================

// Reset CPU
void riscv64_cpu_reset(void)
{
    printf("RISC-V64: CPU reset requested\n");
    
    // This would typically trigger a system reset
    // For now, just print a message
    printf("RISC-V64: CPU reset not implemented\n");
}

// Halt CPU
void riscv64_cpu_halt(void)
{
    printf("RISC-V64: CPU halt requested\n");
    
    // Enter WFI (Wait For Interrupt) mode
    asm volatile("wfi");
}

// Wait for interrupt
void riscv64_cpu_wait_for_interrupt(void)
{
    // Enter WFI mode
    asm volatile("wfi");
}

// Get CPU ID
uint32_t riscv64_cpu_get_id(void)
{
    return (uint32_t)riscv64_get_hart_id();
}

// Get CPU revision
uint32_t riscv64_cpu_get_revision(void)
{
    return (uint32_t)riscv64_get_cpu_revision();
}

// Get CPU frequency
uint32_t riscv64_cpu_get_frequency(void)
{
    return (uint32_t)(riscv64_get_cpu_frequency() / 1000000);  // Return in MHz
}

// Set CPU frequency
int riscv64_cpu_set_frequency(uint32_t frequency_mhz)
{
    printf("RISC-V64: CPU frequency set to %u MHz\n", frequency_mhz);
    
    // In a real system, this would configure the clock
    // For now, just update the context
    riscv64_cpu_context.cpu_frequency = (uint64_t)frequency_mhz * 1000000ULL;
    
    return 0;
}

// ============================================================================
// CPU DEBUG FUNCTIONS
// ============================================================================

// Print CPU information
void riscv64_cpu_print_info(void)
{
    if (!riscv64_cpu_context.initialized) {
        printf("RISC-V64: CPU not initialized\n");
        return;
    }
    
    printf("RISC-V64: CPU Information\n");
    printf("  Hart ID: %llu\n", riscv64_cpu_context.hart_id);
    printf("  Current Mode: 0x%x\n", riscv64_cpu_context.current_mode);
    printf("  Supported Modes: 0x%x\n", riscv64_cpu_context.supported_modes);
    printf("  Frequency: %llu Hz (%llu MHz)\n", 
           riscv64_cpu_context.cpu_frequency, 
           riscv64_cpu_context.cpu_frequency / 1000000);
    printf("  Revision: 0x%llx\n", riscv64_cpu_context.cpu_revision);
    printf("  Vendor ID: 0x%llx\n", riscv64_cpu_context.cpu_vendor_id);
    printf("  Architecture ID: 0x%llx\n", riscv64_cpu_context.cpu_architecture_id);
    printf("  Implementation ID: 0x%llx\n", riscv64_cpu_context.cpu_implementation_id);
    
    printf("  Features:\n");
    printf("    RV64I: %s\n", riscv64_has_feature(RISCV64_FEATURE_RV64I) ? "Yes" : "No");
    printf("    RV64M: %s\n", riscv64_has_feature(RISCV64_FEATURE_RV64M) ? "Yes" : "No");
    printf("    RV64A: %s\n", riscv64_has_feature(RISCV64_FEATURE_RV64A) ? "Yes" : "No");
    printf("    RV64F: %s\n", riscv64_has_feature(RISCV64_FEATURE_RV64F) ? "Yes" : "No");
    printf("    RV64D: %s\n", riscv64_has_feature(RISCV64_FEATURE_RV64D) ? "Yes" : "No");
    printf("    RV64C: %s\n", riscv64_has_feature(RISCV64_FEATURE_RV64C) ? "Yes" : "No");
    printf("    RVV: %s\n", riscv64_has_feature(RISCV64_FEATURE_RVV) ? "Yes" : "No");
}

// Print performance counters
void riscv64_cpu_print_performance_counters(void)
{
    if (!riscv64_performance_context.initialized) {
        printf("RISC-V64: Performance monitoring not initialized\n");
        return;
    }
    
    printf("RISC-V64: Performance Counters\n");
    printf("  Counters Enabled: %s\n", riscv64_performance_context.counters_enabled ? "Yes" : "No");
    printf("  Cycle Counter: %llu\n", riscv64_performance_read_counter(0));
    printf("  Instruction Counter: %llu\n", riscv64_performance_read_counter(1));
    printf("  Time Counter: %llu\n", riscv64_performance_read_counter(2));
    
    // Print hardware performance counters
    for (int i = 3; i < 32; i++) {
        uint64_t value = riscv64_performance_read_counter(i);
        if (value > 0) {
            printf("  HPM Counter %d: %llu\n", i, value);
        }
    }
}

// ============================================================================
// CPU CLEANUP
// ============================================================================

// Cleanup CPU system
void riscv64_cpu_cleanup(void)
{
    if (!riscv64_cpu_context.initialized) {
        return;
    }
    
    // Disable performance counters
    riscv64_performance_disable_counters();
    
    // Clear contexts
    riscv64_cpu_context.initialized = false;
    riscv64_performance_context.initialized = false;
    
    printf("RISC-V64: CPU system cleaned up\n");
}
