/*
 * ORION OS - MIPS Performance Management
 *
 * Performance management for MIPS architecture
 * Handles performance monitoring, counters, and optimization
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
// PERFORMANCE CONTEXT MANAGEMENT
// ============================================================================

static mips_performance_context_t mips_performance_context = {0};
static mips_debug_context_t mips_debug_context = {0};

void mips_performance_init(void)
{
    memset(&mips_performance_context, 0, sizeof(mips_performance_context));
    memset(&mips_debug_context, 0, sizeof(mips_debug_context));

    // Set default performance settings
    mips_performance_context.performance_monitoring_enabled = false;
    mips_performance_context.performance_counters = 0;
    mips_performance_context.cycle_count = 0;
    mips_performance_context.instruction_count = 0;

    // Set default debug settings
    mips_debug_context.debug_enabled = false;
    mips_debug_context.breakpoints = 0;
    mips_debug_context.watchpoints = 0;
    mips_debug_context.single_step = false;

    printf("MIPS: Performance contexts initialized\n");
}

mips_performance_context_t *mips_performance_get_context(void)
{
    return &mips_performance_context;
}

mips_debug_context_t *mips_debug_get_context(void)
{
    return &mips_debug_context;
}

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

void mips_performance_enable_monitoring(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_PERFORMANCE))
    {
        printf("MIPS: Performance monitoring not supported\n");
        return;
    }

    // Enable performance monitoring in CP0
    uint64_t perfctrl = mips_cpu_read_cp0_register(MIPS_CP0_PERFCONTROL);
    perfctrl |= MIPS_PERFCONTROL_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_PERFCONTROL, perfctrl);

    mips_performance_context.performance_monitoring_enabled = true;

    printf("MIPS: Performance monitoring enabled\n");
}

void mips_performance_disable_monitoring(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_PERFORMANCE))
    {
        return;
    }

    uint64_t perfctrl = mips_cpu_read_cp0_register(MIPS_CP0_PERFCONTROL);
    perfctrl &= ~MIPS_PERFCONTROL_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_PERFCONTROL, perfctrl);

    mips_performance_context.performance_monitoring_enabled = false;

    printf("MIPS: Performance monitoring disabled\n");
}

bool mips_performance_is_monitoring_enabled(void)
{
    return mips_performance_context.performance_monitoring_enabled;
}

// ============================================================================
// PERFORMANCE COUNTERS
// ============================================================================

uint64_t mips_performance_read_counter(uint32_t counter)
{
    if (!mips_performance_context.performance_monitoring_enabled)
    {
        return 0;
    }

    if (counter >= MIPS_MAX_PERFORMANCE_COUNTERS)
    {
        printf("MIPS: Invalid performance counter: %d\n", counter);
        return 0;
    }

    return mips_cpu_read_cp0_register(MIPS_CP0_PERFCOUNTER + counter);
}

void mips_performance_write_counter(uint32_t counter, uint64_t value)
{
    if (!mips_performance_context.performance_monitoring_enabled)
    {
        return;
    }

    if (counter >= MIPS_MAX_PERFORMANCE_COUNTERS)
    {
        printf("MIPS: Invalid performance counter: %d\n", counter);
        return;
    }

    mips_cpu_write_cp0_register(MIPS_CP0_PERFCOUNTER + counter, value);
}

void mips_performance_reset_counter(uint32_t counter)
{
    mips_performance_write_counter(counter, 0);
}

void mips_performance_reset_all_counters(void)
{
    for (uint32_t i = 0; i < MIPS_MAX_PERFORMANCE_COUNTERS; i++)
    {
        mips_performance_reset_counter(i);
    }

    printf("MIPS: All performance counters reset\n");
}

// ============================================================================
// PERFORMANCE EVENTS
// ============================================================================

void mips_performance_set_event(uint32_t counter, uint32_t event)
{
    if (!mips_performance_context.performance_monitoring_enabled)
    {
        return;
    }

    if (counter >= MIPS_MAX_PERFORMANCE_COUNTERS)
    {
        printf("MIPS: Invalid performance counter: %d\n", counter);
        return;
    }

    // Set event in performance control register
    uint64_t perfctrl = mips_cpu_read_cp0_register(MIPS_CP0_PERFCONTROL);
    uint64_t event_mask = MIPS_PERFCONTROL_EVENT_MASK << (counter * MIPS_PERFCONTROL_EVENT_BITS);
    uint64_t event_value = (event & MIPS_PERFCONTROL_EVENT_MASK) << (counter * MIPS_PERFCONTROL_EVENT_BITS);

    perfctrl = (perfctrl & ~event_mask) | event_value;
    mips_cpu_write_cp0_register(MIPS_CP0_PERFCONTROL, perfctrl);

    printf("MIPS: Performance counter %d set to event %d\n", counter, event);
}

uint32_t mips_performance_get_event(uint32_t counter)
{
    if (!mips_performance_context.performance_monitoring_enabled)
    {
        return 0;
    }

    if (counter >= MIPS_MAX_PERFORMANCE_COUNTERS)
    {
        printf("MIPS: Invalid performance counter: %d\n", counter);
        return 0;
    }

    uint64_t perfctrl = mips_cpu_read_cp0_register(MIPS_CP0_PERFCONTROL);
    uint64_t event_mask = MIPS_PERFCONTROL_EVENT_MASK << (counter * MIPS_PERFCONTROL_EVENT_BITS);
    return (uint32_t)((perfctrl & event_mask) >> (counter * MIPS_PERFCONTROL_EVENT_BITS));
}

// ============================================================================
// CYCLE AND INSTRUCTION COUNTING
// ============================================================================

uint64_t mips_performance_get_cycle_count(void)
{
    if (mips_performance_context.performance_monitoring_enabled)
    {
        mips_performance_context.cycle_count = mips_cpu_read_cp0_register(MIPS_CP0_COUNT);
    }

    return mips_performance_context.cycle_count;
}

uint64_t mips_performance_get_instruction_count(void)
{
    if (mips_performance_context.performance_monitoring_enabled)
    {
        mips_performance_context.instruction_count = mips_performance_read_counter(0);
    }

    return mips_performance_context.instruction_count;
}

void mips_performance_reset_counts(void)
{
    mips_performance_context.cycle_count = 0;
    mips_performance_context.instruction_count = 0;

    // Reset cycle counter
    mips_cpu_write_cp0_register(MIPS_CP0_COUNT, 0);

    // Reset instruction counter
    mips_performance_reset_counter(0);

    printf("MIPS: Performance counts reset\n");
}

// ============================================================================
// PERFORMANCE OPTIMIZATION
// ============================================================================

void mips_performance_optimize_cache(void)
{
    // Optimize cache performance
    mips_cache_sync_all();

    // Set optimal cache parameters
    mips_cache_context_t *icache = mips_cache_get_icache_context();
    mips_cache_context_t *dcache = mips_cache_get_dcache_context();

    if (icache && icache->enabled)
    {
        // Optimize instruction cache
        icache->write_back = false;
        icache->write_allocate = false;
    }

    if (dcache && dcache->enabled)
    {
        // Optimize data cache
        dcache->write_back = true;
        dcache->write_allocate = true;
    }

    printf("MIPS: Cache performance optimized\n");
}

void mips_performance_optimize_branch_prediction(void)
{
    // Optimize branch prediction
    uint64_t config = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG);

    // Enable branch prediction if supported
    if (config & MIPS_CONFIG_BRANCH_PREDICTION)
    {
        config |= MIPS_CONFIG_BRANCH_PREDICTION_ENABLE;
        mips_cpu_write_cp0_register(MIPS_CP0_CONFIG, config);
        printf("MIPS: Branch prediction optimized\n");
    }
}

void mips_performance_optimize_pipeline(void)
{
    // Optimize CPU pipeline
    uint64_t config = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG);

    // Set optimal pipeline parameters
    config |= MIPS_CONFIG_PIPELINE_OPTIMIZE;
    mips_cpu_write_cp0_register(MIPS_CP0_CONFIG, config);

    printf("MIPS: Pipeline performance optimized\n");
}

// ============================================================================
// DEBUG SUPPORT
// ============================================================================

void mips_debug_enable(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_DEBUG))
    {
        printf("MIPS: Debug features not supported\n");
        return;
    }

    // Enable debug mode in CP0
    uint64_t debug = mips_cpu_read_cp0_register(MIPS_CP0_DEBUG);
    debug |= MIPS_DEBUG_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_DEBUG, debug);

    mips_debug_context.debug_enabled = true;

    printf("MIPS: Debug mode enabled\n");
}

void mips_debug_disable(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_DEBUG))
    {
        return;
    }

    uint64_t debug = mips_cpu_read_cp0_register(MIPS_CP0_DEBUG);
    debug &= ~MIPS_DEBUG_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_DEBUG, debug);

    mips_debug_context.debug_enabled = false;

    printf("MIPS: Debug mode disabled\n");
}

bool mips_debug_is_enabled(void)
{
    return mips_debug_context.debug_enabled;
}

// ============================================================================
// BREAKPOINTS
// ============================================================================

void mips_debug_set_breakpoint(uint32_t breakpoint, mips_addr_t address)
{
    if (!mips_debug_context.debug_enabled)
    {
        printf("MIPS: Debug mode not enabled\n");
        return;
    }

    if (breakpoint >= MIPS_MAX_BREAKPOINTS)
    {
        printf("MIPS: Invalid breakpoint number: %d\n", breakpoint);
        return;
    }

    // Set breakpoint address
    mips_cpu_write_cp0_register(MIPS_CP0_DEBUG + breakpoint, address);

    mips_debug_context.breakpoints |= (1 << breakpoint);

    printf("MIPS: Breakpoint %d set at 0x%llx\n", breakpoint, address);
}

void mips_debug_clear_breakpoint(uint32_t breakpoint)
{
    if (!mips_debug_context.debug_enabled)
    {
        return;
    }

    if (breakpoint >= MIPS_MAX_BREAKPOINTS)
    {
        printf("MIPS: Invalid breakpoint number: %d\n", breakpoint);
        return;
    }

    // Clear breakpoint
    mips_cpu_write_cp0_register(MIPS_CP0_DEBUG + breakpoint, 0);

    mips_debug_context.breakpoints &= ~(1 << breakpoint);

    printf("MIPS: Breakpoint %d cleared\n", breakpoint);
}

void mips_debug_clear_all_breakpoints(void)
{
    if (!mips_debug_context.debug_enabled)
    {
        return;
    }

    for (uint32_t i = 0; i < MIPS_MAX_BREAKPOINTS; i++)
    {
        mips_debug_clear_breakpoint(i);
    }

    printf("MIPS: All breakpoints cleared\n");
}

// ============================================================================
// WATCHPOINTS
// ============================================================================

void mips_debug_set_watchpoint(uint32_t watchpoint, mips_addr_t address, uint32_t flags)
{
    if (!mips_debug_context.debug_enabled)
    {
        printf("MIPS: Debug mode not enabled\n");
        return;
    }

    if (watchpoint >= MIPS_MAX_WATCHPOINTS)
    {
        printf("MIPS: Invalid watchpoint number: %d\n", watchpoint);
        return;
    }

    // Set watchpoint address and flags
    uint64_t watchpoint_value = address | flags;
    mips_cpu_write_cp0_register(MIPS_CP0_WATCHPOINT + watchpoint, watchpoint_value);

    mips_debug_context.watchpoints |= (1 << watchpoint);

    printf("MIPS: Watchpoint %d set at 0x%llx with flags 0x%x\n", watchpoint, address, flags);
}

void mips_debug_clear_watchpoint(uint32_t watchpoint)
{
    if (!mips_debug_context.debug_enabled)
    {
        return;
    }

    if (watchpoint >= MIPS_MAX_WATCHPOINTS)
    {
        printf("MIPS: Invalid watchpoint number: %d\n", watchpoint);
        return;
    }

    // Clear watchpoint
    mips_cpu_write_cp0_register(MIPS_CP0_WATCHPOINT + watchpoint, 0);

    mips_debug_context.watchpoints &= ~(1 << watchpoint);

    printf("MIPS: Watchpoint %d cleared\n", watchpoint);
}

void mips_debug_clear_all_watchpoints(void)
{
    if (!mips_debug_context.debug_enabled)
    {
        return;
    }

    for (uint32_t i = 0; i < MIPS_MAX_WATCHPOINTS; i++)
    {
        mips_debug_clear_watchpoint(i);
    }

    printf("MIPS: All watchpoints cleared\n");
}

// ============================================================================
// SINGLE STEP
// ============================================================================

void mips_debug_enable_single_step(void)
{
    if (!mips_debug_context.debug_enabled)
    {
        printf("MIPS: Debug mode not enabled\n");
        return;
    }

    // Enable single step in CP0
    uint64_t debug = mips_cpu_read_cp0_register(MIPS_CP0_DEBUG);
    debug |= MIPS_DEBUG_SINGLE_STEP;
    mips_cpu_write_cp0_register(MIPS_CP0_DEBUG, debug);

    mips_debug_context.single_step = true;

    printf("MIPS: Single step enabled\n");
}

void mips_debug_disable_single_step(void)
{
    if (!mips_debug_context.debug_enabled)
    {
        return;
    }

    uint64_t debug = mips_cpu_read_cp0_register(MIPS_CP0_DEBUG);
    debug &= ~MIPS_DEBUG_SINGLE_STEP;
    mips_cpu_write_cp0_register(MIPS_CP0_DEBUG, debug);

    mips_debug_context.single_step = false;

    printf("MIPS: Single step disabled\n");
}

bool mips_debug_is_single_step_enabled(void)
{
    return mips_debug_context.single_step;
}

// ============================================================================
// PERFORMANCE STATISTICS
// ============================================================================

void mips_performance_print_statistics(void)
{
    printf("MIPS: Performance Statistics:\n");
    printf("  Performance Monitoring: %s\n",
           mips_performance_context.performance_monitoring_enabled ? "enabled" : "disabled");
    printf("  Performance Counters: %u\n", mips_performance_context.performance_counters);
    printf("  Cycle Count: %llu\n", mips_performance_get_cycle_count());
    printf("  Instruction Count: %llu\n", mips_performance_get_instruction_count());

    if (mips_performance_context.performance_monitoring_enabled)
    {
        for (uint32_t i = 0; i < MIPS_MAX_PERFORMANCE_COUNTERS; i++)
        {
            uint64_t value = mips_performance_read_counter(i);
            uint32_t event = mips_performance_get_event(i);
            if (value > 0 || event != 0)
            {
                printf("  Counter %d: %llu (Event: %u)\n", i, value, event);
            }
        }
    }

    printf("  Debug Mode: %s\n", mips_debug_context.debug_enabled ? "enabled" : "disabled");
    printf("  Breakpoints: %u\n", mips_debug_context.breakpoints);
    printf("  Watchpoints: %u\n", mips_debug_context.watchpoints);
    printf("  Single Step: %s\n", mips_debug_context.single_step ? "enabled" : "disabled");
}

// ============================================================================
// PERFORMANCE UTILITY FUNCTIONS
// ============================================================================

double mips_performance_calculate_cpi(void)
{
    uint64_t cycles = mips_performance_get_cycle_count();
    uint64_t instructions = mips_performance_get_instruction_count();

    if (instructions == 0)
    {
        return 0.0;
    }

    return (double)cycles / (double)instructions;
}

double mips_performance_calculate_mips(void)
{
    uint64_t instructions = mips_performance_get_instruction_count();
    uint64_t cycles = mips_performance_get_cycle_count();
    uint64_t frequency = mips_cpu_get_frequency();

    if (cycles == 0 || frequency == 0)
    {
        return 0.0;
    }

    double time_seconds = (double)cycles / (double)frequency;
    return (double)instructions / (time_seconds * 1000000.0);
}

void mips_performance_benchmark_start(void)
{
    mips_performance_reset_counts();
    printf("MIPS: Performance benchmark started\n");
}

void mips_performance_benchmark_end(void)
{
    uint64_t cycles = mips_performance_get_cycle_count();
    uint64_t instructions = mips_performance_get_instruction_count();
    double cpi = mips_performance_calculate_cpi();
    double mips = mips_performance_calculate_mips();

    printf("MIPS: Performance benchmark results:\n");
    printf("  Cycles: %llu\n", cycles);
    printf("  Instructions: %llu\n", instructions);
    printf("  CPI: %.2f\n", cpi);
    printf("  MIPS: %.2f\n", mips);
}
