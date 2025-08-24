/*
 * Orion Operating System - HAL Common Header
 *
 * Common definitions and structures for the Hardware Abstraction Layer.
 * This header provides shared constants, types, and function declarations
 * that coordinate with existing kernel/arch implementations.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_HAL_COMMON_H
#define ORION_HAL_COMMON_H

#include "orion_hal.h"
#include <orion/types.h>
#include <stdbool.h>

// HAL limits and constants
#define ORION_HAL_MAX_ARCHITECTURES 16
#define ORION_HAL_MAX_EXTENSIONS 256
#define ORION_HAL_MAX_PERF_COUNTERS 32

// HAL error codes
#define ORION_HAL_SUCCESS 0
#define ORION_HAL_ERROR_INVALID_ARG -1
#define ORION_HAL_ERROR_OUT_OF_MEMORY -2
#define ORION_HAL_ERROR_UNSUPPORTED_ARCH -3
#define ORION_HAL_ERROR_NO_ARCH -4
#define ORION_HAL_ERROR_NOT_IMPLEMENTED -5

// HAL manager structure
struct orion_hal_manager {
    struct orion_hal_arch *current_arch;
    uint32_t arch_count;
    struct orion_hal_arch *arch_list[ORION_HAL_MAX_ARCHITECTURES];
    bool initialized;
};

// HAL manager function declarations
int hal_manager_register_arch(struct orion_hal_arch *arch);
int hal_manager_detect_arch(void);
int hal_manager_switch_arch(uint32_t arch_id);
struct orion_hal_arch* hal_manager_get_current_arch(void);
struct orion_hal_arch* hal_manager_get_arch(uint32_t arch_id);
int hal_manager_init(void);
int hal_manager_benchmark_arch(void);
int hal_manager_report_capabilities(void);
struct orion_hal_manager* hal_manager_get_instance(void);
bool hal_manager_is_initialized(void);

#endif // ORION_HAL_COMMON_H
