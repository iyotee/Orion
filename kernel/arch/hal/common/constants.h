/*
 * Orion Operating System - System Constants Header
 *
 * System-wide constants and limits.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_CONSTANTS_H
#define ORION_CONSTANTS_H

#include <orion/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

// System limits
#define MAX_PROCESSES 1024
#define MAX_THREADS 4096
#define MAX_FILES 1024
#define MAX_MOUNTS 64
#define MAX_IPC_PORTS 256
#define MAX_CPUS 64

// Memory constants
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12
#define KERNEL_BASE 0xFFFFFFFF80000000
#define USER_BASE 0x0000000000000000

// Time constants
#define CLOCK_TICKS_PER_SECOND 1000
#define TIMER_RESOLUTION_NS 1000000

#ifdef __cplusplus
}
#endif

#endif // ORION_CONSTANTS_H
