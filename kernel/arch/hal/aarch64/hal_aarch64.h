/*
 * Orion Operating System - AArch64 HAL Implementation
 *
 * Hardware Abstraction Layer for AArch64 architecture.
 * This implementation uses the existing kernel/arch/aarch64 code
 * and provides a unified interface for the rest of the kernel.
 *
 * Features:
 * - ARM64 optimizations (Cortex-A, Neoverse, Apple Silicon)
 * - ARM Pointer Authentication and Memory Tagging Extension
 * - ARM TrustZone integration
 * - ARM big.LITTLE scheduling optimizations
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_HAL_AARCH64_H
#define ORION_HAL_AARCH64_H

#include "hal_common.h"
#include <orion/types.h>
#include <orion/error.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* ============================================================================
     * AArch64 HAL Implementation Functions
     * ============================================================================ */

    /**
     * @brief Initialize AArch64 MMU using existing kernel/arch code
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_mmu_init(void);

    /**
     * @brief Map virtual address to physical address
     * @param va Virtual address
     * @param pa Physical address
     * @param flags Memory protection flags
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_mmu_map(vaddr_t va, paddr_t pa, uint64_t flags);

    /**
     * @brief Unmap virtual address range
     * @param va Virtual address
     * @param len Length to unmap
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_mmu_unmap(vaddr_t va, size_t len);

    /**
     * @brief Change memory protection
     * @param va Virtual address
     * @param len Length to change
     * @param flags New protection flags
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_mmu_protect(vaddr_t va, size_t len, uint64_t flags);

    /**
     * @brief Translate virtual address to physical address
     * @param va Virtual address
     * @return Physical address or 0 if not mapped
     */
    paddr_t hal_aarch64_mmu_translate(vaddr_t va);

    /**
     * @brief Initialize AArch64 timer using existing kernel/arch code
     * @param frequency_hz Timer frequency in Hz
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_timer_init(uint64_t frequency_hz);

    /**
     * @brief Get current timer ticks
     * @return Current timer value
     */
    uint64_t hal_aarch64_timer_get_ticks(void);

    /**
     * @brief Set one-shot timer
     * @param delay_ticks Delay in timer ticks
     * @param callback Callback function to execute
     */
    void hal_aarch64_timer_set_oneshot(uint64_t delay_ticks, timer_callback_t callback);

    /**
     * @brief Cancel one-shot timer
     */
    void hal_aarch64_timer_cancel(void);

    /**
     * @brief Initialize AArch64 interrupts using existing kernel/arch code
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_interrupt_init(void);

    /**
     * @brief Register interrupt handler
     * @param irq Interrupt number
     * @param handler Handler function
     * @param data Handler data
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_irq_register(uint32_t irq, irq_handler_t handler, void *data);

    /**
     * @brief Unregister interrupt handler
     * @param irq Interrupt number
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_irq_unregister(uint32_t irq);

    /**
     * @brief Enable interrupt
     * @param irq Interrupt number
     */
    void hal_aarch64_irq_enable(uint32_t irq);

    /**
     * @brief Disable interrupt
     * @param irq Interrupt number
     */
    void hal_aarch64_irq_disable(uint32_t irq);

    /**
     * @brief Acknowledge interrupt
     * @param irq Interrupt number
     */
    void hal_aarch64_irq_ack(uint32_t irq);

    /**
     * @brief Initialize AArch64 CPU using existing kernel/arch code
     * @param cpu_id CPU ID
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_cpu_init(uint32_t cpu_id);

    /**
     * @brief Put CPU in idle state
     */
    void hal_aarch64_cpu_idle(void);

    /**
     * @brief Wake up CPU
     * @param cpu_id CPU ID to wake
     */
    void hal_aarch64_cpu_wake(uint32_t cpu_id);

    /**
     * @brief Get CPU count
     * @return Number of CPUs
     */
    uint32_t hal_aarch64_cpu_get_count(void);

    /**
     * @brief Get current CPU ID
     * @return Current CPU ID
     */
    uint32_t hal_aarch64_cpu_get_current(void);

    /**
     * @brief Initialize AArch64 performance monitoring
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_perf_init(void);

    /**
     * @brief Read performance counter
     * @param counter_id Counter ID
     * @return Counter value
     */
    uint64_t hal_aarch64_perf_read_counter(uint32_t counter_id);

    /**
     * @brief Set performance event
     * @param counter_id Counter ID
     * @param event_id Event ID
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_perf_set_event(uint32_t counter_id, uint32_t event_id);

    /**
     * @brief Enable performance monitoring
     * @param counter_id Counter ID
     */
    void hal_aarch64_perf_enable(uint32_t counter_id);

    /**
     * @brief Disable performance monitoring
     * @param counter_id Counter ID
     */
    void hal_aarch64_perf_disable(uint32_t counter_id);

    /**
     * @brief Initialize AArch64 security features
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_secure_init(void);

    /**
     * @brief Measure secure data
     * @param data Data to measure
     * @param len Data length
     * @param measurement Output measurement
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_secure_measure(const void *data, size_t len, uint8_t *measurement);

    /**
     * @brief Verify secure data
     * @param data Data to verify
     * @param len Data length
     * @param measurement Expected measurement
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_secure_verify(const void *data, size_t len, const uint8_t *measurement);

    /**
     * @brief Create secure enclave
     * @param enclave_id Enclave ID
     * @param size Enclave size
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_secure_enclave_create(uint32_t enclave_id, size_t size);

    /**
     * @brief Destroy secure enclave
     * @param enclave_id Enclave ID
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_secure_enclave_destroy(uint32_t enclave_id);

    /**
     * @brief Initialize AArch64 power management
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_power_init(void);

    /**
     * @brief Set power state
     * @param cpu_id CPU ID
     * @param state Power state
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_power_set_state(uint32_t cpu_id, uint32_t state);

    /**
     * @brief Get power state
     * @param cpu_id CPU ID
     * @return Current power state
     */
    uint32_t hal_aarch64_power_get_state(uint32_t cpu_id);

    /**
     * @brief Set CPU frequency
     * @param cpu_id CPU ID
     * @param frequency_hz Frequency in Hz
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_power_set_frequency(uint32_t cpu_id, uint64_t frequency_hz);

    /**
     * @brief Initialize AArch64 debugging
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_debug_init(void);

    /**
     * @brief Set debug breakpoint
     * @param addr Address for breakpoint
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_debug_break(vaddr_t addr);

    /**
     * @brief Remove debug breakpoint
     * @param addr Address of breakpoint to remove
     * @return ORION_HAL_SUCCESS on success, error code on failure
     */
    int hal_aarch64_debug_remove_break(vaddr_t addr);

    /**
     * @brief Get AArch64 HAL implementation
     * @return Pointer to AArch64 HAL implementation
     */
    const struct orion_hal_arch *hal_aarch64_get_impl(void);

#ifdef __cplusplus
}
#endif

#endif /* ORION_HAL_AARCH64_H */
