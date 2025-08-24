/*
 * Orion Operating System - HAL Rust Bridge
 *
 * Interface C-Rust pour permettre au runtime Rust d'utiliser
 * les informations HAL du kernel C sans duplication.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_HAL_RUST_BRIDGE_H
#define ORION_HAL_RUST_BRIDGE_H

#include "orion_hal.h"
#include <orion/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* ============================================================================
     * Architecture Information Bridge
     * ============================================================================ */

    /**
     * @brief Get current architecture information for Rust runtime
     * @param arch_id Output architecture ID
     * @param arch_name Output architecture name
     * @param features Output architecture features
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_rust_get_architecture_info(uint32_t *arch_id,
                                       char *arch_name,
                                       size_t name_len,
                                       uint64_t *features);

    /**
     * @brief Get CPU information for Rust runtime
     * @param cpu_count Output CPU count
     * @param cpu_info Output CPU information array
     * @param max_cpus Maximum CPUs to return
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_rust_get_cpu_info(uint32_t *cpu_count,
                              struct orion_cpu_info *cpu_info,
                              uint32_t max_cpus);

    /**
     * @brief Get memory information for Rust runtime
     * @param total_memory Output total memory in bytes
     * @param available_memory Output available memory in bytes
     * @param numa_nodes Output NUMA node count
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_rust_get_memory_info(uint64_t *total_memory,
                                 uint64_t *available_memory,
                                 uint32_t *numa_nodes);

    /* ============================================================================
     * Performance Monitoring Bridge
     * ============================================================================ */

    /**
     * @brief Get performance counters for Rust runtime
     * @param counter_id Counter ID
     * @param value Output counter value
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_rust_get_performance_counter(uint32_t counter_id, uint64_t *value);

    /**
     * @brief Set performance event for Rust runtime
     * @param counter_id Counter ID
     * @param event_id Event ID
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_rust_set_performance_event(uint32_t counter_id, uint32_t event_id);

    /* ============================================================================
     * Security Features Bridge
     * ============================================================================ */

    /**
     * @brief Get security capabilities for Rust runtime
     * @param capabilities Output security capabilities
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_rust_get_security_capabilities(uint64_t *capabilities);

    /**
     * @brief Measure secure data for Rust runtime
     * @param data Data to measure
     * @param len Data length
     * @param measurement Output measurement
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_rust_secure_measure(const void *data,
                                size_t len,
                                uint8_t *measurement);

    /* ============================================================================
     * Power Management Bridge
     * ============================================================================ */

    /**
     * @brief Get power state for Rust runtime
     * @param cpu_id CPU ID
     * @param state Output power state
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_rust_get_power_state(uint32_t cpu_id, uint32_t *state);

    /**
     * @brief Set power state for Rust runtime
     * @param cpu_id CPU ID
     * @param state Power state to set
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_rust_set_power_state(uint32_t cpu_id, uint32_t state);

    /* ============================================================================
     * Debug and Tracing Bridge
     * ============================================================================ */

    /**
     * @brief Log message from Rust runtime to kernel
     * @param level Log level
     * @param message Message to log
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_rust_log_message(uint32_t level, const char *message);

    /**
     * @brief Get debug information for Rust runtime
     * @param info Output debug information
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_rust_get_debug_info(struct orion_debug_info *info);

#ifdef __cplusplus
}
#endif

#endif /* ORION_HAL_RUST_BRIDGE_H */
