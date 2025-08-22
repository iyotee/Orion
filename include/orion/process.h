/*
 * Orion Operating System - Process Management
 *
 * Process-related functions and structures for the kernel.
 * Provides process creation, management, and lifecycle functions.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_PROCESS_H
#define ORION_PROCESS_H

#include <orion/types.h>

// Process functions
uint16_t process_get_current_pid(void);

#endif // ORION_PROCESS_H
