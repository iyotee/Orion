/*
 * Orion Operating System - System Limits Header
 *
 * System limits and boundaries.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_LIMITS_H
#define ORION_LIMITS_H

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

// Memory limits
#define MAX_MEMORY_SIZE (1ULL << 48)   // 256TB
#define MAX_USER_MEMORY (1ULL << 47)   // 128TB
#define MAX_KERNEL_MEMORY (1ULL << 47) // 128TB

// File system limits
#define MAX_PATH_LENGTH 4096
#define MAX_FILENAME_LENGTH 255
#define MAX_SYMLINK_DEPTH 40

// Process limits
#define MAX_PROCESS_ARGS 64
#define MAX_PROCESS_ENV 64
#define MAX_PROCESS_FILES 1024

#ifdef __cplusplus
}
#endif

#endif // ORION_LIMITS_H
