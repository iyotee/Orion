/*
 * ORION OS - RISC-V 64-bit Power Management
 *
 * Power management for RISC-V 64-bit architecture
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
// GLOBAL POWER CONTEXTS
// ============================================================================

// Power management context
static struct
{
    bool initialized;
    bool power_management_enabled;
    bool power_saving_enabled;
    bool power_monitoring_enabled;
    uint32_t current_power_state;
    uint32_t target_power_state;
    uint32_t power_transition_timeout;
    uint64_t power_consumption;
    uint64_t power_efficiency;
} riscv64_power_context = {0};

// Power statistics
static struct
{
    uint32_t total_power_transitions;
    uint32_t power_state_changes;
    uint32_t power_saving_cycles;
    uint32_t power_management_events;
    uint64_t total_power_consumption;
    uint64_t average_power_consumption;
    uint64_t peak_power_consumption;
    uint64_t low_power_duration;
} riscv64_power_stats = {0};

// Power management configuration
static struct
{
    uint32_t power_threshold_low;
    uint32_t power_threshold_high;
    uint32_t power_saving_delay;
    uint32_t power_transition_delay;
    bool auto_power_saving;
    bool power_threshold_alerts;
} riscv64_power_config = {0};

// ============================================================================
// POWER INITIALIZATION
// ============================================================================

// Initialize power management system
void riscv64_power_init(void)
{
    if (riscv64_power_context.initialized)
    {
        return;
    }

    // Initialize power context with default values
    riscv64_power_context.power_management_enabled = true;
    riscv64_power_context.power_saving_enabled = true;
    riscv64_power_context.power_monitoring_enabled = true;
    riscv64_power_context.current_power_state = RISCV64_POWER_STATE_ACTIVE;
    riscv64_power_context.target_power_state = RISCV64_POWER_STATE_ACTIVE;
    riscv64_power_context.power_transition_timeout = RISCV64_POWER_TRANSITION_TIMEOUT;
    riscv64_power_context.power_consumption = 0;
    riscv64_power_context.power_efficiency = 100;
    riscv64_power_context.initialized = true;

    // Initialize power configuration
    riscv64_power_config.power_threshold_low = RISCV64_POWER_THRESHOLD_LOW;
    riscv64_power_config.power_threshold_high = RISCV64_POWER_THRESHOLD_HIGH;
    riscv64_power_config.power_saving_delay = RISCV64_POWER_SAVING_DELAY;
    riscv64_power_config.power_transition_delay = RISCV64_POWER_TRANSITION_DELAY;
    riscv64_power_config.auto_power_saving = true;
    riscv64_power_config.power_threshold_alerts = true;

    // Clear power statistics
    memset(&riscv64_power_stats, 0, sizeof(riscv64_power_stats));

    printf("RISC-V64: Power management system initialized\n");
    printf("  Power Management: %s\n", riscv64_power_context.power_management_enabled ? "Enabled" : "Disabled");
    printf("  Power Saving: %s\n", riscv64_power_context.power_saving_enabled ? "Enabled" : "Disabled");
    printf("  Power Monitoring: %s\n", riscv64_power_context.power_monitoring_enabled ? "Enabled" : "Disabled");
    printf("  Current Power State: %u\n", riscv64_power_context.current_power_state);
    printf("  Power Threshold Low: %u\n", riscv64_power_config.power_threshold_low);
    printf("  Power Threshold High: %u\n", riscv64_power_config.power_threshold_high);
}

// ============================================================================
// POWER CONTEXT FUNCTIONS
// ============================================================================

// Get power management context
struct riscv64_power_context *riscv64_get_power_context(void)
{
    if (!riscv64_power_context.initialized)
    {
        return NULL;
    }

    return &riscv64_power_context;
}

// Enable power management
void riscv64_power_management_enable(void)
{
    if (!riscv64_power_context.initialized)
    {
        return;
    }

    riscv64_power_context.power_management_enabled = true;

    printf("RISC-V64: Power management enabled\n");
}

// Disable power management
void riscv64_power_management_disable(void)
{
    if (!riscv64_power_context.initialized)
    {
        return;
    }

    riscv64_power_context.power_management_enabled = false;

    printf("RISC-V64: Power management disabled\n");
}

// Check if power management is enabled
bool riscv64_power_management_is_enabled(void)
{
    if (!riscv64_power_context.initialized)
    {
        return false;
    }

    return riscv64_power_context.power_management_enabled;
}

// Enable power saving
void riscv64_power_saving_enable(void)
{
    if (!riscv64_power_context.initialized)
    {
        return;
    }

    riscv64_power_context.power_saving_enabled = true;

    printf("RISC-V64: Power saving enabled\n");
}

// Disable power saving
void riscv64_power_saving_disable(void)
{
    if (!riscv64_power_context.initialized)
    {
        return;
    }

    riscv64_power_context.power_saving_enabled = false;

    printf("RISC-V64: Power saving disabled\n");
}

// Check if power saving is enabled
bool riscv64_power_saving_is_enabled(void)
{
    if (!riscv64_power_context.initialized)
    {
        return false;
    }

    return riscv64_power_context.power_saving_enabled;
}

// Enable power monitoring
void riscv64_power_monitoring_enable(void)
{
    if (!riscv64_power_context.initialized)
    {
        return;
    }

    riscv64_power_context.power_monitoring_enabled = true;

    printf("RISC-V64: Power monitoring enabled\n");
}

// Disable power monitoring
void riscv64_power_monitoring_disable(void)
{
    if (!riscv64_power_context.initialized)
    {
        return;
    }

    riscv64_power_context.power_monitoring_enabled = false;

    printf("RISC-V64: Power monitoring disabled\n");
}

// Check if power monitoring is enabled
bool riscv64_power_monitoring_is_enabled(void)
{
    if (!riscv64_power_context.initialized)
    {
        return false;
    }

    return riscv64_power_context.power_monitoring_enabled;
}

// ============================================================================
// POWER STATE MANAGEMENT
// ============================================================================

// Set power state
int riscv64_power_set_state(uint32_t power_state)
{
    if (!riscv64_power_context.initialized || !riscv64_power_context.power_management_enabled)
    {
        return RISCV64_ERROR_NOT_INITIALIZED;
    }

    if (power_state >= RISCV64_POWER_STATE_MAX)
    {
        return RISCV64_ERROR_INVALID_PARAMETER;
    }

    // Check if power state transition is allowed
    if (!riscv64_power_state_transition_allowed(riscv64_power_context.current_power_state, power_state))
    {
        return RISCV64_ERROR_INVALID_TRANSITION;
    }

    // Set target power state
    riscv64_power_context.target_power_state = power_state;

    // Perform power state transition
    int result = riscv64_power_transition_to_state(power_state);
    if (result == RISCV64_SUCCESS)
    {
        riscv64_power_context.current_power_state = power_state;
        riscv64_power_stats.power_state_changes++;
        riscv64_power_stats.total_power_transitions++;

        printf("RISC-V64: Power state changed to %u\n", power_state);
    }

    return result;
}

// Get current power state
uint32_t riscv64_power_get_state(void)
{
    if (!riscv64_power_context.initialized)
    {
        return RISCV64_POWER_STATE_UNKNOWN;
    }

    return riscv64_power_context.current_power_state;
}

// Get target power state
uint32_t riscv64_power_get_target_state(void)
{
    if (!riscv64_power_context.initialized)
    {
        return RISCV64_POWER_STATE_UNKNOWN;
    }

    return riscv64_power_context.target_power_state;
}

// Check if power state transition is allowed
bool riscv64_power_state_transition_allowed(uint32_t from_state, uint32_t to_state)
{
    // Basic transition rules
    switch (from_state)
    {
    case RISCV64_POWER_STATE_ACTIVE:
        return (to_state == RISCV64_POWER_STATE_IDLE ||
                to_state == RISCV64_POWER_STATE_SLEEP ||
                to_state == RISCV64_POWER_STATE_DEEP_SLEEP);
    case RISCV64_POWER_STATE_IDLE:
        return (to_state == RISCV64_POWER_STATE_ACTIVE ||
                to_state == RISCV64_POWER_STATE_SLEEP);
    case RISCV64_POWER_STATE_SLEEP:
        return (to_state == RISCV64_POWER_STATE_ACTIVE ||
                to_state == RISCV64_POWER_STATE_DEEP_SLEEP);
    case RISCV64_POWER_STATE_DEEP_SLEEP:
        return (to_state == RISCV64_POWER_STATE_ACTIVE);
    default:
        return false;
    }
}

// ============================================================================
// POWER TRANSITIONS
// ============================================================================

// Transition to specific power state
int riscv64_power_transition_to_state(uint32_t power_state)
{
    if (!riscv64_power_context.initialized)
    {
        return RISCV64_ERROR_NOT_INITIALIZED;
    }

    printf("RISC-V64: Transitioning to power state %u\n", power_state);

    switch (power_state)
    {
    case RISCV64_POWER_STATE_ACTIVE:
        return riscv64_power_transition_to_active();
    case RISCV64_POWER_STATE_IDLE:
        return riscv64_power_transition_to_idle();
    case RISCV64_POWER_STATE_SLEEP:
        return riscv64_power_transition_to_sleep();
    case RISCV64_POWER_STATE_DEEP_SLEEP:
        return riscv64_power_transition_to_deep_sleep();
    default:
        return RISCV64_ERROR_INVALID_PARAMETER;
    }
}

// Transition to active state
int riscv64_power_transition_to_active(void)
{
    // Enable all CPU features and peripherals
    riscv64_cpu_set_mode(RISCV64_CPU_MODE_SUPERVISOR);

    // Enable interrupts
    riscv64_interrupts_enable();

    // Enable timers
    riscv64_timer_enable();

    // Enable cache
    riscv64_cache_enable();

    // Update power consumption
    riscv64_power_context.power_consumption = RISCV64_POWER_CONSUMPTION_ACTIVE;

    printf("RISC-V64: Transitioned to active power state\n");
    return RISCV64_SUCCESS;
}

// Transition to idle state
int riscv64_power_transition_to_idle(void)
{
    // Disable some CPU features to save power
    riscv64_cpu_set_mode(RISCV64_CPU_MODE_SUPERVISOR);

    // Keep interrupts enabled for wake-up
    riscv64_interrupts_enable();

    // Keep timers enabled
    riscv64_timer_enable();

    // Disable cache to save power
    riscv64_cache_disable();

    // Update power consumption
    riscv64_power_context.power_consumption = RISCV64_POWER_CONSUMPTION_IDLE;

    printf("RISC-V64: Transitioned to idle power state\n");
    return RISCV64_SUCCESS;
}

// Transition to sleep state
int riscv64_power_transition_to_sleep(void)
{
    // Disable most CPU features
    riscv64_cpu_set_mode(RISCV64_CPU_MODE_MACHINE);

    // Disable interrupts except for critical ones
    riscv64_interrupts_disable();

    // Disable timers
    riscv64_timer_disable();

    // Disable cache
    riscv64_cache_disable();

    // Update power consumption
    riscv64_power_context.power_consumption = RISCV64_POWER_CONSUMPTION_SLEEP;

    printf("RISC-V64: Transitioned to sleep power state\n");
    return RISCV64_SUCCESS;
}

// Transition to deep sleep state
int riscv64_power_transition_to_deep_sleep(void)
{
    // Disable all CPU features
    riscv64_cpu_set_mode(RISCV64_CPU_MODE_MACHINE);

    // Disable all interrupts
    riscv64_interrupts_disable();

    // Disable all timers
    riscv64_timer_disable();

    // Disable cache
    riscv64_cache_disable();

    // Disable MMU
    riscv64_mmu_disable();

    // Update power consumption
    riscv64_power_context.power_consumption = RISCV64_POWER_CONSUMPTION_DEEP_SLEEP;

    printf("RISC-V64: Transitioned to deep sleep power state\n");
    return RISCV64_SUCCESS;
}

// ============================================================================
// POWER SAVING FUNCTIONS
// ============================================================================

// Enter power saving mode
int riscv64_power_enter_saving_mode(void)
{
    if (!riscv64_power_context.initialized || !riscv64_power_context.power_saving_enabled)
    {
        return RISCV64_ERROR_NOT_ENABLED;
    }

    printf("RISC-V64: Entering power saving mode\n");

    // Check current power consumption
    uint64_t current_consumption = riscv64_power_get_consumption();

    if (current_consumption > riscv64_power_config.power_threshold_high)
    {
        // High power consumption, enter deeper sleep
        return riscv64_power_transition_to_state(RISCV64_POWER_STATE_SLEEP);
    }
    else if (current_consumption > riscv64_power_config.power_threshold_low)
    {
        // Medium power consumption, enter idle
        return riscv64_power_transition_to_state(RISCV64_POWER_STATE_IDLE);
    }
    else
    {
        // Low power consumption, stay active
        return RISCV64_SUCCESS;
    }
}

// Exit power saving mode
int riscv64_power_exit_saving_mode(void)
{
    if (!riscv64_power_context.initialized)
    {
        return RISCV64_ERROR_NOT_INITIALIZED;
    }

    printf("RISC-V64: Exiting power saving mode\n");

    // Always return to active state
    return riscv64_power_transition_to_state(RISCV64_POWER_STATE_ACTIVE);
}

// Check if power saving mode is active
bool riscv64_power_saving_mode_is_active(void)
{
    if (!riscv64_power_context.initialized)
    {
        return false;
    }

    return (riscv64_power_context.current_power_state != RISCV64_POWER_STATE_ACTIVE);
}

// ============================================================================
// POWER MONITORING
// ============================================================================

// Get current power consumption
uint64_t riscv64_power_get_consumption(void)
{
    if (!riscv64_power_context.initialized || !riscv64_power_context.power_monitoring_enabled)
    {
        return 0;
    }

    // For now, return the stored power consumption
    // In a real system, this would read from power monitoring hardware
    return riscv64_power_context.power_consumption;
}

// Get power efficiency
uint64_t riscv64_power_get_efficiency(void)
{
    if (!riscv64_power_context.initialized)
    {
        return 0;
    }

    return riscv64_power_context.power_efficiency;
}

// Set power consumption threshold
void riscv64_power_set_threshold(uint32_t threshold_low, uint32_t threshold_high)
{
    if (!riscv64_power_context.initialized)
    {
        return;
    }

    if (threshold_low < threshold_high)
    {
        riscv64_power_config.power_threshold_low = threshold_low;
        riscv64_power_config.power_threshold_high = threshold_high;

        printf("RISC-V64: Power thresholds set to low=%u, high=%u\n", threshold_low, threshold_high);
    }
}

// Get power consumption thresholds
void riscv64_power_get_threshold(uint32_t *threshold_low, uint32_t *threshold_high)
{
    if (!riscv64_power_context.initialized)
    {
        return;
    }

    if (threshold_low)
        *threshold_low = riscv64_power_config.power_threshold_low;
    if (threshold_high)
        *threshold_high = riscv64_power_config.power_threshold_high;
}

// Check power consumption alerts
void riscv64_power_check_alerts(void)
{
    if (!riscv64_power_context.initialized || !riscv64_power_config.power_threshold_alerts)
    {
        return;
    }

    uint64_t current_consumption = riscv64_power_get_consumption();

    if (current_consumption > riscv64_power_config.power_threshold_high)
    {
        printf("RISC-V64: Power consumption alert: %llu (above high threshold)\n", current_consumption);
        riscv64_power_stats.power_management_events++;
    }
    else if (current_consumption < riscv64_power_config.power_threshold_low)
    {
        printf("RISC-V64: Power consumption alert: %llu (below low threshold)\n", current_consumption);
        riscv64_power_stats.power_management_events++;
    }
}

// ============================================================================
// POWER CONFIGURATION
// ============================================================================

// Set power saving delay
void riscv64_power_set_saving_delay(uint32_t delay)
{
    if (!riscv64_power_context.initialized)
    {
        return;
    }

    riscv64_power_config.power_saving_delay = delay;
    printf("RISC-V64: Power saving delay set to %u\n", delay);
}

// Get power saving delay
uint32_t riscv64_power_get_saving_delay(void)
{
    if (!riscv64_power_context.initialized)
    {
        return 0;
    }

    return riscv64_power_config.power_saving_delay;
}

// Set power transition delay
void riscv64_power_set_transition_delay(uint32_t delay)
{
    if (!riscv64_power_context.initialized)
    {
        return;
    }

    riscv64_power_config.power_transition_delay = delay;
    printf("RISC-V64: Power transition delay set to %u\n", delay);
}

// Get power transition delay
uint32_t riscv64_power_get_transition_delay(void)
{
    if (!riscv64_power_context.initialized)
    {
        return 0;
    }

    return riscv64_power_config.power_transition_delay;
}

// Enable auto power saving
void riscv64_power_auto_saving_enable(void)
{
    if (!riscv64_power_context.initialized)
    {
        return;
    }

    riscv64_power_config.auto_power_saving = true;
    printf("RISC-V64: Auto power saving enabled\n");
}

// Disable auto power saving
void riscv64_power_auto_saving_disable(void)
{
    if (!riscv64_power_context.initialized)
    {
        return;
    }

    riscv64_power_config.auto_power_saving = false;
    printf("RISC-V64: Auto power saving disabled\n");
}

// Check if auto power saving is enabled
bool riscv64_power_auto_saving_is_enabled(void)
{
    if (!riscv64_power_context.initialized)
    {
        return false;
    }

    return riscv64_power_config.auto_power_saving;
}

// ============================================================================
// POWER STATISTICS
// ============================================================================

// Get power statistics
void riscv64_power_get_stats(uint32_t *total_transitions, uint32_t *state_changes,
                             uint32_t *saving_cycles, uint32_t *management_events,
                             uint64_t *total_consumption, uint64_t *average_consumption,
                             uint64_t *peak_consumption, uint64_t *low_power_duration)
{
    if (total_transitions)
        *total_transitions = riscv64_power_stats.total_power_transitions;
    if (state_changes)
        *state_changes = riscv64_power_stats.power_state_changes;
    if (saving_cycles)
        *saving_cycles = riscv64_power_stats.power_saving_cycles;
    if (management_events)
        *management_events = riscv64_power_stats.power_management_events;
    if (total_consumption)
        *total_consumption = riscv64_power_stats.total_power_consumption;
    if (average_consumption)
        *average_consumption = riscv64_power_stats.average_power_consumption;
    if (peak_consumption)
        *peak_consumption = riscv64_power_stats.peak_power_consumption;
    if (low_power_duration)
        *low_power_duration = riscv64_power_stats.low_power_duration;
}

// Reset power statistics
void riscv64_power_reset_stats(void)
{
    memset(&riscv64_power_stats, 0, sizeof(riscv64_power_stats));

    printf("RISC-V64: Power statistics reset\n");
}

// ============================================================================
// POWER DEBUG FUNCTIONS
// ============================================================================

// Print power status
void riscv64_power_print_status(void)
{
    printf("RISC-V64: Power Status\n");
    printf("  Initialized: %s\n", riscv64_power_context.initialized ? "Yes" : "No");
    printf("  Power Management: %s\n", riscv64_power_context.power_management_enabled ? "Enabled" : "Disabled");
    printf("  Power Saving: %s\n", riscv64_power_context.power_saving_enabled ? "Enabled" : "Disabled");
    printf("  Power Monitoring: %s\n", riscv64_power_context.power_monitoring_enabled ? "Enabled" : "Disabled");
    printf("  Current Power State: %u\n", riscv64_power_context.current_power_state);
    printf("  Target Power State: %u\n", riscv64_power_context.target_power_state);
    printf("  Power Consumption: %llu\n", riscv64_power_context.power_consumption);
    printf("  Power Efficiency: %llu%%\n", riscv64_power_context.power_efficiency);
}

// Print power configuration
void riscv64_power_print_configuration(void)
{
    printf("RISC-V64: Power Configuration\n");
    printf("  Power Threshold Low: %u\n", riscv64_power_config.power_threshold_low);
    printf("  Power Threshold High: %u\n", riscv64_power_config.power_threshold_high);
    printf("  Power Saving Delay: %u\n", riscv64_power_config.power_saving_delay);
    printf("  Power Transition Delay: %u\n", riscv64_power_config.power_transition_delay);
    printf("  Auto Power Saving: %s\n", riscv64_power_config.auto_power_saving ? "Enabled" : "Disabled");
    printf("  Power Threshold Alerts: %s\n", riscv64_power_config.power_threshold_alerts ? "Enabled" : "Disabled");
}

// Print power statistics
void riscv64_power_print_statistics(void)
{
    printf("RISC-V64: Power Statistics\n");
    printf("  Total Power Transitions: %u\n", riscv64_power_stats.total_power_transitions);
    printf("  Power State Changes: %u\n", riscv64_power_stats.power_state_changes);
    printf("  Power Saving Cycles: %u\n", riscv64_power_stats.power_saving_cycles);
    printf("  Power Management Events: %u\n", riscv64_power_stats.power_management_events);
    printf("  Total Power Consumption: %llu\n", riscv64_power_stats.total_power_consumption);
    printf("  Average Power Consumption: %llu\n", riscv64_power_stats.average_power_consumption);
    printf("  Peak Power Consumption: %llu\n", riscv64_power_stats.peak_power_consumption);
    printf("  Low Power Duration: %llu\n", riscv64_power_stats.low_power_duration);
}

// ============================================================================
// POWER CLEANUP
// ============================================================================

// Cleanup power management system
void riscv64_power_cleanup(void)
{
    if (!riscv64_power_context.initialized)
    {
        return;
    }

    // Return to active state before cleanup
    riscv64_power_transition_to_state(RISCV64_POWER_STATE_ACTIVE);

    // Clear contexts
    riscv64_power_context.initialized = false;

    printf("RISC-V64: Power management system cleaned up\n");
}
