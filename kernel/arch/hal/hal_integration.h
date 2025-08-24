/*
 * Orion Operating System - HAL Integration Header
 *
 * Header for HAL integration layer that connects the HAL with
 * existing kernel/arch implementations without duplication.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_HAL_INTEGRATION_H
#define ORION_HAL_INTEGRATION_H

#include "orion_hal.h"
#include <stdbool.h>

// HAL integration function declarations
int hal_integration_init(void);
struct orion_hal_arch* hal_integration_get_hal(void);
bool hal_integration_is_ready(void);
int hal_integration_init_subsystems(void);
void hal_integration_report_status(void);
int hal_integration_shutdown(void);

#endif // ORION_HAL_INTEGRATION_H
