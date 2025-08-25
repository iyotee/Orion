/*
 * ORION OS - MIPS CPU Management
 *
 * CPU-specific management for MIPS architecture
 * Handles CPU initialization, configuration, and control
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// CPU STATE MANAGEMENT
// ============================================================================

static mips_cpu_state_t mips_cpu_state = {0};
static mips_cpu_info_t mips_cpu_info = {0};

void mips_cpu_initialize_state(void)
{
    memset(&mips_cpu_state, 0, sizeof(mips_cpu_state));
    memset(&mips_cpu_info, 0, sizeof(mips_cpu_info));
    
    // Set default CPU state
    mips_cpu_state.current_mode = MIPS_MODE_KERNEL;
    mips_cpu_state.interrupts_enabled = false;
    mips_cpu_state.exception_level = 0;
    
    printf("MIPS: CPU state initialized\n");
}

mips_cpu_state_t* mips_cpu_get_state(void)
{
    return &mips_cpu_state;
}

mips_cpu_info_t* mips_cpu_get_info(void)
{
    return &mips_cpu_info;
}

// ============================================================================
// CPU MODE MANAGEMENT
// ============================================================================

void mips_cpu_set_mode(mips_cpu_mode_t mode)
{
    uint64_t status = mips_cpu_read_cp0_status();
    
    // Clear current mode bits
    status &= ~(MIPS_STATUS_KSU_MASK | MIPS_STATUS_EXL | MIPS_STATUS_ERL);
    
    switch (mode) {
        case MIPS_MODE_KERNEL:
            // Kernel mode - no additional bits needed
            break;
        case MIPS_MODE_SUPERVISOR:
            status |= MIPS_STATUS_KSU_SUPERVISOR;
            break;
        case MIPS_MODE_USER:
            status |= MIPS_STATUS_KSU_USER;
            break;
        default:
            printf("MIPS: Invalid CPU mode requested: %d\n", mode);
            return;
    }
    
    mips_cpu_write_cp0_status(status);
    mips_cpu_state.current_mode = mode;
    
    printf("MIPS: CPU mode set to %d\n", mode);
}

mips_cpu_mode_t mips_cpu_get_mode(void)
{
    return mips_cpu_state.current_mode;
}

bool mips_cpu_is_privileged(void)
{
    return mips_cpu_state.current_mode == MIPS_MODE_KERNEL;
}

// ============================================================================
// CPU FEATURE DETECTION
// ============================================================================

void mips_cpu_detect_features(void)
{
    uint64_t config0 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG);
    uint64_t config1 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 1);
    uint64_t prid = mips_cpu_read_cp0_register(MIPS_CP0_PRID);
    
    // CPU identification
    mips_cpu_info.vendor_id = (uint32_t)(prid >> 16);
    mips_cpu_info.cpu_id = (uint32_t)(prid & 0xFFFF);
    mips_cpu_info.revision = (uint32_t)((prid >> 8) & 0xFF);
    
    // Architecture features
    mips_cpu_info.is_64bit = (config0 & MIPS_CONFIG0_64BIT) != 0;
    mips_cpu_info.has_fpu = (config1 & MIPS_CONFIG1_FP) != 0;
    mips_cpu_info.has_dsp = (config1 & MIPS_CONFIG1_DSP) != 0;
    mips_cpu_info.has_msa = (config1 & MIPS_CONFIG1_MSA) != 0;
    mips_cpu_info.has_vz = (config1 & MIPS_CONFIG1_VZ) != 0;
    mips_cpu_info.has_crc = (config1 & MIPS_CONFIG1_CRC) != 0;
    mips_cpu_info.has_ginv = (config1 & MIPS_CONFIG1_GINV) != 0;
    
    // Cache information
    mips_cpu_info.icache_size = (config1 & MIPS_CONFIG1_IC) ? 
        (1 << ((config1 >> 22) & 0x7)) * 1024 : 0;
    mips_cpu_info.dcache_size = (config1 & MIPS_CONFIG1_DC) ? 
        (1 << ((config1 >> 12) & 0x7)) * 1024 : 0;
    
    printf("MIPS: CPU features detected - 64bit: %s, FPU: %s, DSP: %s\n",
           mips_cpu_info.is_64bit ? "yes" : "no",
           mips_cpu_info.has_fpu ? "yes" : "no",
           mips_cpu_info.has_dsp ? "yes" : "no");
}

bool mips_cpu_supports_feature(mips_cpu_feature_t feature)
{
    switch (feature) {
        case MIPS_FEATURE_64BIT:
            return mips_cpu_info.is_64bit;
        case MIPS_FEATURE_FPU:
            return mips_cpu_info.has_fpu;
        case MIPS_FEATURE_DSP:
            return mips_cpu_info.has_dsp;
        case MIPS_FEATURE_MSA:
            return mips_cpu_info.has_msa;
        case MIPS_FEATURE_VZ:
            return mips_cpu_info.has_vz;
        case MIPS_FEATURE_CRC:
            return mips_cpu_info.has_crc;
        case MIPS_FEATURE_GINV:
            return mips_cpu_info.has_ginv;
        default:
            return false;
    }
}

// ============================================================================
// CPU PERFORMANCE MONITORING
// ============================================================================

void mips_cpu_enable_performance_monitoring(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_PERFORMANCE)) {
        printf("MIPS: Performance monitoring not supported\n");
        return;
    }
    
    // Enable performance counters
    uint64_t perfctrl = mips_cpu_read_cp0_register(MIPS_CP0_PERFCONTROL);
    perfctrl |= MIPS_PERFCONTROL_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_PERFCONTROL, perfctrl);
    
    printf("MIPS: Performance monitoring enabled\n");
}

void mips_cpu_disable_performance_monitoring(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_PERFORMANCE)) {
        return;
    }
    
    uint64_t perfctrl = mips_cpu_read_cp0_register(MIPS_CP0_PERFCONTROL);
    perfctrl &= ~MIPS_PERFCONTROL_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_PERFCONTROL, perfctrl);
    
    printf("MIPS: Performance monitoring disabled\n");
}

uint64_t mips_cpu_read_performance_counter(uint32_t counter)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_PERFORMANCE)) {
        return 0;
    }
    
    if (counter >= MIPS_MAX_PERFORMANCE_COUNTERS) {
        printf("MIPS: Invalid performance counter: %d\n", counter);
        return 0;
    }
    
    return mips_cpu_read_cp0_register(MIPS_CP0_PERFCOUNTER + counter);
}

void mips_cpu_write_performance_counter(uint32_t counter, uint64_t value)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_PERFORMANCE)) {
        return;
    }
    
    if (counter >= MIPS_MAX_PERFORMANCE_COUNTERS) {
        printf("MIPS: Invalid performance counter: %d\n", counter);
        return;
    }
    
    mips_cpu_write_cp0_register(MIPS_CP0_PERFCOUNTER + counter, value);
}

// ============================================================================
// CPU POWER MANAGEMENT
// ============================================================================

void mips_cpu_set_power_state(mips_power_state_t state)
{
    switch (state) {
        case MIPS_POWER_STATE_ACTIVE:
            // Normal operation
            break;
        case MIPS_POWER_STATE_IDLE:
            // Enter idle mode
            mips_cpu_wait_for_interrupt();
            break;
        case MIPS_POWER_STATE_SLEEP:
            // Enter sleep mode
            mips_cpu_halt();
            break;
        default:
            printf("MIPS: Invalid power state: %d\n", state);
            return;
    }
    
    mips_power_context.power_state = state;
    printf("MIPS: Power state set to %d\n", state);
}

mips_power_state_t mips_cpu_get_power_state(void)
{
    return mips_power_context.power_state;
}

void mips_cpu_set_frequency(uint64_t frequency)
{
    if (frequency < MIPS_MIN_CPU_FREQUENCY || frequency > MIPS_MAX_CPU_FREQUENCY) {
        printf("MIPS: Invalid frequency: %llu Hz\n", frequency);
        return;
    }
    
    mips_power_context.cpu_frequency = frequency;
    printf("MIPS: CPU frequency set to %llu Hz\n", frequency);
}

uint64_t mips_cpu_get_frequency(void)
{
    return mips_power_context.cpu_frequency;
}

// ============================================================================
// CPU DEBUG SUPPORT
// ============================================================================

void mips_cpu_enable_debug(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_DEBUG)) {
        printf("MIPS: Debug support not available\n");
        return;
    }
    
    // Enable debug mode
    uint64_t debug = mips_cpu_read_cp0_register(MIPS_CP0_DEBUG);
    debug |= MIPS_DEBUG_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_DEBUG, debug);
    
    printf("MIPS: Debug mode enabled\n");
}

void mips_cpu_disable_debug(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_DEBUG)) {
        return;
    }
    
    uint64_t debug = mips_cpu_read_cp0_register(MIPS_CP0_DEBUG);
    debug &= ~MIPS_DEBUG_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_DEBUG, debug);
    
    printf("MIPS: Debug mode disabled\n");
}

void mips_cpu_set_breakpoint(uint32_t breakpoint, mips_addr_t address)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_DEBUG)) {
        return;
    }
    
    if (breakpoint >= MIPS_MAX_BREAKPOINTS) {
        printf("MIPS: Invalid breakpoint number: %d\n", breakpoint);
        return;
    }
    
    // Set breakpoint address
    mips_cpu_write_cp0_register(MIPS_CP0_DEBUG + breakpoint, address);
    printf("MIPS: Breakpoint %d set at 0x%llx\n", breakpoint, address);
}

void mips_cpu_clear_breakpoint(uint32_t breakpoint)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_DEBUG)) {
        return;
    }
    
    if (breakpoint >= MIPS_MAX_BREAKPOINTS) {
        printf("MIPS: Invalid breakpoint number: %d\n", breakpoint);
        return;
    }
    
    mips_cpu_write_cp0_register(MIPS_CP0_DEBUG + breakpoint, 0);
    printf("MIPS: Breakpoint %d cleared\n", breakpoint);
}

// ============================================================================
// CPU EXTENSION SUPPORT
// ============================================================================

bool mips_cpu_supports_extension(mips_extension_t extension)
{
    switch (extension) {
        case MIPS_EXTENSION_NEON:
            return mips_cpu_info.has_neon;
        case MIPS_EXTENSION_VFP:
            return mips_cpu_info.has_vfp;
        case MIPS_EXTENSION_CRYPTO:
            return mips_cpu_info.has_crypto;
        case MIPS_EXTENSION_SIMD:
            return mips_cpu_info.has_simd;
        default:
            return false;
    }
}

void mips_cpu_enable_extension(mips_extension_t extension)
{
    if (!mips_cpu_supports_extension(extension)) {
        printf("MIPS: Extension not supported: %d\n", extension);
        return;
    }
    
    // Enable extension in CP0
    uint64_t hwrena = mips_cpu_read_cp0_register(MIPS_CP0_HWRENA);
    hwrena |= (1 << extension);
    mips_cpu_write_cp0_register(MIPS_CP0_HWRENA, hwrena);
    
    printf("MIPS: Extension %d enabled\n", extension);
}

void mips_cpu_disable_extension(mips_extension_t extension)
{
    if (!mips_cpu_supports_extension(extension)) {
        return;
    }
    
    uint64_t hwrena = mips_cpu_read_cp0_register(MIPS_CP0_HWRENA);
    hwrena &= ~(1 << extension);
    mips_cpu_write_cp0_register(MIPS_CP0_HWRENA, hwrena);
    
    printf("MIPS: Extension %d disabled\n", extension);
}

// ============================================================================
// CPU UTILITY FUNCTIONS
// ============================================================================

void mips_cpu_nop(void)
{
    __asm__ volatile("nop");
}

void mips_cpu_yield(void)
{
    // Yield CPU to other threads
    __asm__ volatile("yield");
}

void mips_cpu_sync(void)
{
    // Synchronize memory operations
    __asm__ volatile("sync");
}

void mips_cpu_sync_p(void)
{
    // Synchronize prefetch operations
    __asm__ volatile("sync.p");
}

void mips_cpu_sync_s(void)
{
    // Synchronize store operations
    __asm__ volatile("sync.s");
}

uint64_t mips_cpu_get_cycle_count(void)
{
    return mips_cpu_read_cp0_register(MIPS_CP0_COUNT);
}

void mips_cpu_delay_cycles(uint64_t cycles)
{
    uint64_t start = mips_cpu_get_cycle_count();
    while ((mips_cpu_get_cycle_count() - start) < cycles) {
        mips_cpu_nop();
    }
}

void mips_cpu_delay_microseconds(uint64_t microseconds)
{
    uint64_t cycles = microseconds * (mips_cpu_get_frequency() / 1000000);
    mips_cpu_delay_cycles(cycles);
}

void mips_cpu_delay_milliseconds(uint64_t milliseconds)
{
    mips_cpu_delay_microseconds(milliseconds * 1000);
}
