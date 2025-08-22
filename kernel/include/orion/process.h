/*
 * Orion Operating System - Process Management Header
 *
 * Process creation, management, and lifecycle declarations.
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
#include <orion/structures.h>

// ========================================
// PROCESS CREATION AND MANAGEMENT
// ========================================

/**
 * Create a new process
 *
 * @param name Process name
 * @param entry_point Entry point address
 * @param stack_pointer Initial stack pointer
 * @return Pointer to created process or NULL on failure
 */
process_t *process_create(const char *name, uint64_t entry_point, uint64_t stack_pointer);

/**
 * Start a process
 *
 * @param process Process to start
 * @return 0 on success, negative error code on failure
 */
int process_start(process_t *process);

/**
 * Destroy a process
 *
 * @param process Process to destroy
 */
void process_destroy(process_t *process);

/**
 * Find process by PID
 *
 * @param pid Process ID to find
 * @return Pointer to process or NULL if not found
 */
process_t *process_find(uint64_t pid);

/**
 * Get current process
 *
 * @return Pointer to current process or NULL
 */
process_t *process_get_current(void);

/**
 * Set process arguments
 *
 * @param process Process to set arguments for
 * @param argv Argument vector
 * @param envp Environment vector
 * @return 0 on success, negative error code on failure
 */
int process_set_args(process_t *process, char *const argv[], char *const envp[]);

/**
 * Get process count
 *
 * @return Number of active processes
 */
uint32_t process_get_count(void);

/**
 * Initialize process management system
 */
void process_init(void);

#endif // ORION_PROCESS_H
