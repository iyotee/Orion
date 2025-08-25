/*
 * ORION OS - LoongArch HAL Implementation Header
 *
 * Hardware Abstraction Layer for LoongArch architecture.
 * This implementation provides a unified interface for LoongArch
 * while leveraging architecture-specific optimizations.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_HAL_LOONGARCH_H
#define ORION_HAL_LOONGARCH_H

#include "orion_hal.h"
#include <orion/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* ============================================================================
     * LoongArch HAL Implementation
     * ============================================================================ */

    /**
     * @brief LoongArch HAL architecture interface
     *
     * This structure provides the LoongArch-specific implementation
     * of the universal HAL interface, optimized for LoongArch features
     * like LSX/LASX vector extensions and custom MMU operations.
     */
    extern struct orion_hal_arch orion_hal_loongarch;

    /* ============================================================================
     * LoongArch-Specific HAL Functions
     * ============================================================================ */

    /* Vector Extensions Support */
    int hal_loongarch_lsx_init(void);
    int hal_loongarch_lasx_init(void);
    bool hal_loongarch_has_lsx(void);
    bool hal_loongarch_has_lasx(void);

    /* Custom MMU Operations */
    int hal_loongarch_mmu_set_custom_flags(vaddr_t va, uint64_t custom_flags);
    int hal_loongarch_mmu_enable_huge_pages(size_t page_size);
    int hal_loongarch_mmu_set_asid(uint64_t asid);

    /* LoongArch Interrupt Controller */
    int hal_loongarch_irq_set_priority(uint32_t irq, uint8_t priority);
    int hal_loongarch_irq_set_custom_flags(uint32_t irq, uint64_t flags);
    int hal_loongarch_irq_get_context(uint32_t irq, struct loongarch_irq_context *ctx);

    /* Performance Monitoring */
    int hal_loongarch_perf_init_custom(void);
    uint64_t hal_loongarch_perf_read_vector_counter(uint32_t counter_id);
    int hal_loongarch_perf_set_vector_event(uint32_t counter_id, uint32_t event_id);

    /* Crypto Acceleration */
    int hal_loongarch_crypto_init(void);
    int hal_loongarch_crypto_encrypt(const void *input, void *output, size_t len, uint32_t key_id);
    int hal_loongarch_crypto_decrypt(const void *input, void *output, size_t len, uint32_t key_id);

    /* ============================================================================
     * LoongArch HAL Initialization
     * ============================================================================ */

    /**
     * @brief Initialize LoongArch HAL
     *
     * @return int 0 on success, negative error code on failure
     */
    int hal_loongarch_init(void);

    /**
     * @brief Get LoongArch HAL capabilities
     *
     * @return struct orion_hal_capabilities* Pointer to LoongArch capabilities
     */
    struct orion_hal_capabilities *hal_loongarch_get_capabilities(void);

    /**
     * @brief Optimize HAL for specific LoongArch model
     *
     * @param model_id LoongArch model identifier
     * @return int 0 on success, negative error code on failure
     */
    int hal_loongarch_optimize_for_model(uint32_t model_id);

/* ============================================================================
 * LoongArch Model-Specific Optimizations
 * ============================================================================ */

/* LoongArch model identifiers */
#define LOONGARCH_MODEL_3A5000 0x3A5000
#define LOONGARCH_MODEL_3C5000 0x3C5000
#define LOONGARCH_MODEL_3A6000 0x3A6000
#define LOONGARCH_MODEL_2K1000 0x2K1000

/* Model-specific optimization flags */
#define LOONGARCH_OPT_LSX_AGGRESSIVE 0x00000001
#define LOONGARCH_OPT_LASX_AGGRESSIVE 0x00000002
#define LOONGARCH_OPT_CACHE_OPTIMIZED 0x00000004
#define LOONGARCH_OPT_POWER_EFFICIENT 0x00000008
#define LOONGARCH_OPT_SERVER_OPTIMIZED 0x00000010

    /* ============================================================================
     * LoongArch HAL Error Codes
     * ============================================================================ */

#define LOONGARCH_HAL_SUCCESS 0
#define LOONGARCH_HAL_ERROR_LSX_NOT_SUPPORTED -1001
#define LOONGARCH_HAL_ERROR_LASX_NOT_SUPPORTED -1002
#define LOONGARCH_HAL_ERROR_CRYPTO_NOT_SUPPORTED -1003
#define LOONGARCH_HAL_ERROR_CUSTOM_MMU_FAILED -1004
#define LOONGARCH_HAL_ERROR_VECTOR_INIT_FAILED -1005

#ifdef __cplusplus
}
#endif

#endif /* ORION_HAL_LOONGARCH_H */
