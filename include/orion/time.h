/*
 * Orion Operating System - Time Management
 *
 * Time-related functions and structures for the kernel.
 * Provides system time, timestamps, and timing utilities.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_TIME_H
#define ORION_TIME_H

#include <orion/types.h>

// Time functions
uint64_t arch_get_timestamp(void);
void arch_delay_ms(uint32_t milliseconds);

#endif // ORION_TIME_H
