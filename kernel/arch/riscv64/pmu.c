/*
 * ORION OS - RISC-V 64-bit PMU Management
 *
 * Performance Monitoring Unit management for RISC-V 64-bit architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include "common.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// GLOBAL PMU CONTEXTS
// ============================================================================

// PMU management context
static struct
{
    bool initialized;
    bool pmu_enabled;
    bool pmu_monitoring_enabled;
    bool pmu_optimization_enabled;
    uint32_t pmu_version;
    uint32_t pmu_counter_count;
    uint32_t current_pmu_mode;
    uint64_t pmu_capabilities;
} riscv64_pmu_context = {0};

// PMU counter information
static struct
{
    uint32_t counter_id;
    uint32_t counter_type;
    uint32_t counter_width;
    bool counter_enabled;
    bool counter_overflow;
    uint64_t counter_value;
    uint64_t counter_overflow_value;
    uint64_t counter_event_id;
} riscv64_pmu_counters[RISCV64_MAX_PMU_COUNTERS] = {0};

// PMU statistics
static struct
{
    uint32_t total_pmu_events;
    uint32_t pmu_counter_overflows;
    uint32_t pmu_mode_changes;
    uint32_t pmu_optimizations;
    uint64_t total_pmu_cycles;
    uint64_t total_pmu_instructions;
    uint64_t total_pmu_cache_misses;
    uint64_t total_pmu_branch_mispredicts;
} riscv64_pmu_stats = {0};

// PMU configuration
static struct
{
    uint32_t pmu_sampling_rate;
    uint32_t pmu_overflow_threshold;
    uint32_t pmu_optimization_threshold;
    bool pmu_auto_optimization;
    bool pmu_overflow_interrupts;
    bool pmu_sampling_enabled;
} riscv64_pmu_config = {0};

// ============================================================================
// PMU INITIALIZATION AND CONFIGURATION
// ============================================================================

void riscv64_pmu_init(void)
{
    printf("RISC-V64: Initializing Performance Monitoring Unit...\n");

    // Check if PMU is available
    if (!riscv64_pmu_check_availability())
    {
        printf("RISC-V64: PMU not available on this CPU\n");
        return;
    }

    // Initialize PMU context
    memset(&riscv64_pmu_context, 0, sizeof(riscv64_pmu_context));

    // Detect available counters
    riscv64_pmu_detect_counters();

    // Set up default event types
    riscv64_pmu_setup_default_events();

    // Enable PMU
    riscv64_pmu_enable();

    riscv64_pmu_context.initialized = true;

    printf("RISC-V64: PMU initialized successfully with %u counters\n",
           riscv64_pmu_context.counter_count);
}

bool riscv64_pmu_check_availability(void)
{
    // Check if HPM (Hardware Performance Monitor) is available
    uint64_t mcounteren;
    __asm__ volatile("csrr %0, mcounteren" : "=r"(mcounteren));

    // If any HPM counters are available, PMU is supported
    return (mcounteren & 0x1FFFFFFF) != 0;
}

void riscv64_pmu_detect_counters(void)
{
    printf("RISC-V64: Detecting PMU counters...\n");

    uint32_t counter_count = 0;

    // Check each HPM counter (3-31)
    for (int i = 3; i < 32; i++)
    {
        uint64_t counter_value;
        __asm__ volatile("csrr %0, mhpmcounter%d" : "=r"(counter_value) : "i"(i));

        // If counter is readable, it's available
        if (counter_value != 0xFFFFFFFFFFFFFFFFULL)
        {
            riscv64_pmu_context.counters[counter_count].id = i;
            riscv64_pmu_context.counters[counter_count].available = true;
            riscv64_pmu_context.counters[counter_count].enabled = false;
            riscv64_pmu_context.counters[counter_count].event_type = 0;
            riscv64_pmu_context.counters[counter_count].value = 0;

            counter_count++;
            printf("RISC-V64: HPM counter %d available\n", i);
        }
    }

    riscv64_pmu_context.counter_count = counter_count;
    printf("RISC-V64: Detected %u PMU counters\n", counter_count);
}

void riscv64_pmu_setup_default_events(void)
{
    printf("RISC-V64: Setting up default PMU events...\n");

    // Set up default event types for available counters
    for (uint32_t i = 0; i < riscv64_pmu_context.counter_count; i++)
    {
        uint32_t counter_id = riscv64_pmu_context.counters[i].id;

        // Set default event type based on counter ID
        uint64_t event_type = 0x01; // Default to cycle count

        switch (counter_id)
        {
        case 3:
            event_type = 0x01; // Cycle count
            break;
        case 4:
            event_type = 0x02; // Instruction count
            break;
        case 5:
            event_type = 0x03; // Cache misses
            break;
        case 6:
            event_type = 0x04; // Branch mispredictions
            break;
        default:
            event_type = 0x01; // Default to cycle count
            break;
        }

        riscv64_pmu_set_event(counter_id, event_type);
        printf("RISC-V64: Counter %u set to event type 0x%llx\n",
               counter_id, (unsigned long long)event_type);
    }
}

void riscv64_pmu_enable(void)
{
    printf("RISC-V64: Enabling PMU...\n");

    // Enable all available HPM counters
    uint64_t mcounteren = 0;

    for (uint32_t i = 0; i < riscv64_pmu_context.counter_count; i++)
    {
        uint32_t counter_id = riscv64_pmu_context.counters[i].id;
        mcounteren |= (1ULL << counter_id);
        riscv64_pmu_context.counters[i].enabled = true;
    }

    __asm__ volatile("csrw mcounteren, %0" : : "r"(mcounteren));

    printf("RISC-V64: PMU enabled with %u counters\n", riscv64_pmu_context.counter_count);
}

void riscv64_pmu_disable(void)
{
    printf("RISC-V64: Disabling PMU...\n");

    // Disable all HPM counters
    __asm__ volatile("csrw mcounteren, zero");

    // Mark all counters as disabled
    for (uint32_t i = 0; i < riscv64_pmu_context.counter_count; i++)
    {
        riscv64_pmu_context.counters[i].enabled = false;
    }

    printf("RISC-V64: PMU disabled\n");
}

// ============================================================================
// COUNTER OPERATIONS
// ============================================================================

uint64_t riscv64_pmu_read_counter(uint32_t counter)
{
    if (!riscv64_pmu_context.initialized)
    {
        return 0;
    }

    // Find counter in our context
    for (uint32_t i = 0; i < riscv64_pmu_context.counter_count; i++)
    {
        if (riscv64_pmu_context.counters[i].id == counter)
        {
            if (!riscv64_pmu_context.counters[i].enabled)
            {
                return 0;
            }

            // Read counter value from CSR
            uint64_t value;
            __asm__ volatile("csrr %0, mhpmcounter%d" : "=r"(value) : "i"(counter));

            // Update our context
            riscv64_pmu_context.counters[i].value = value;

            return value;
        }
    }

    return 0;
}

void riscv64_pmu_set_event(uint32_t counter, uint32_t event)
{
    if (!riscv64_pmu_context.initialized)
    {
        return;
    }

    // Find counter in our context
    for (uint32_t i = 0; i < riscv64_pmu_context.counter_count; i++)
    {
        if (riscv64_pmu_context.counters[i].id == counter)
        {
            // Set event type in HPM event CSR
            __asm__ volatile("csrw mhpmevent%d, %0" : : "i"(counter), "r"(event));

            // Update our context
            riscv64_pmu_context.counters[i].event_type = event;

            printf("RISC-V64: Counter %u event set to 0x%x\n", counter, event);
            return;
        }
    }

    printf("RISC-V64: Counter %u not found\n", counter);
}

void riscv64_pmu_reset_counter(uint32_t counter)
{
    if (!riscv64_pmu_context.initialized)
    {
        return;
    }

    // Find counter in our context
    for (uint32_t i = 0; i < riscv64_pmu_context.counter_count; i++)
    {
        if (riscv64_pmu_context.counters[i].id == counter)
        {
            // Reset counter value
            __asm__ volatile("csrw mhpmcounter%d, zero" : : "i"(counter));

            // Update our context
            riscv64_pmu_context.counters[i].value = 0;

            printf("RISC-V64: Counter %u reset\n", counter);
            return;
        }
    }
}

void riscv64_pmu_reset_all_counters(void)
{
    printf("RISC-V64: Resetting all PMU counters...\n");

    for (uint32_t i = 0; i < riscv64_pmu_context.counter_count; i++)
    {
        uint32_t counter_id = riscv64_pmu_context.counters[i].id;
        riscv64_pmu_reset_counter(counter_id);
    }
}

// ============================================================================
// ADVANCED PMU FEATURES
// ============================================================================

uint64_t riscv64_pmu_read_cycle_counter(void)
{
    uint64_t cycles;
    __asm__ volatile("csrr %0, cycle" : "=r"(cycles));
    return cycles;
}

uint64_t riscv64_pmu_read_instruction_counter(void)
{
    uint64_t instructions;
    __asm__ volatile("csrr %0, instret" : "=r"(instructions));
    return instructions;
}

uint64_t riscv64_pmu_read_time_counter(void)
{
    uint64_t time;
    __asm__ volatile("csrr %0, time" : "=r"(time));
    return time;
}

// ============================================================================
// PMU STATISTICS AND MONITORING
// ============================================================================

void riscv64_pmu_print_stats(void)
{
    if (!riscv64_pmu_context.initialized)
    {
        printf("RISC-V64: PMU not initialized\n");
        return;
    }

    printf("RISC-V64: PMU Statistics\n");
    printf("  Counters available: %u\n", riscv64_pmu_context.counter_count);
    printf("  Counters enabled: %u\n", riscv64_pmu_context.enabled_counters);

    for (uint32_t i = 0; i < riscv64_pmu_context.counter_count; i++)
    {
        const riscv64_pmu_counter_t *counter = &riscv64_pmu_context.counters[i];

        printf("  Counter %u: %s, Event: 0x%x, Value: %llu\n",
               counter->id,
               counter->enabled ? "Enabled" : "Disabled",
               counter->event_type,
               (unsigned long long)counter->value);
    }

    // Print basic counters
    printf("  Cycle counter: %llu\n", (unsigned long long)riscv64_pmu_read_cycle_counter());
    printf("  Instruction counter: %llu\n", (unsigned long long)riscv64_pmu_read_instruction_counter());
    printf("  Time counter: %llu\n", (unsigned long long)riscv64_pmu_read_time_counter());
}

uint32_t riscv64_pmu_get_counter_count(void)
{
    return riscv64_pmu_context.counter_count;
}

bool riscv64_pmu_is_counter_available(uint32_t counter)
{
    for (uint32_t i = 0; i < riscv64_pmu_context.counter_count; i++)
    {
        if (riscv64_pmu_context.counters[i].id == counter)
        {
            return riscv64_pmu_context.counters[i].available;
        }
    }
    return false;
}

bool riscv64_pmu_is_counter_enabled(uint32_t counter)
{
    for (uint32_t i = 0; i < riscv64_pmu_context.counter_count; i++)
    {
        if (riscv64_pmu_context.counters[i].id == counter)
        {
            return riscv64_pmu_context.counters[i].enabled;
        }
    }
    return false;
}

// ============================================================================
// PMU CLEANUP
// ============================================================================

void riscv64_pmu_cleanup(void)
{
    if (!riscv64_pmu_context.initialized)
    {
        return;
    }

    printf("RISC-V64: Cleaning up PMU...\n");

    // Disable all counters
    riscv64_pmu_disable();

    // Reset context
    memset(&riscv64_pmu_context, 0, sizeof(riscv64_pmu_context));

    printf("RISC-V64: PMU cleanup completed\n");
}
