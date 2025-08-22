/*
 * Orion Operating System - Thread Management Header
 *
 * Thread creation, management, and lifecycle declarations.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_THREAD_H
#define ORION_THREAD_H

#include <orion/types.h>

// Forward declarations
struct process;
struct thread;

// Thread structure (defined in scheduler.h)
typedef struct thread thread_t;

// ========================================
// THREAD CREATION AND MANAGEMENT
// ========================================

/**
 * Create a new thread
 *
 * @param process Parent process
 * @param name Thread name
 * @param entry_point Entry point function
 * @param arg Argument passed to entry point
 * @return Pointer to created thread or NULL on failure
 */
thread_t *thread_create(process_t *process, const char *name, void (*entry_point)(void *), void *arg);

/**
 * Start a thread
 *
 * @param thread Thread to start
 * @return 0 on success, negative error code on failure
 */
int thread_start(thread_t *thread);

/**
 * Destroy a thread
 *
 * @param thread Thread to destroy
 */
void thread_destroy(thread_t *thread);

/**
 * Find thread by TID
 *
 * @param tid Thread ID to find
 * @return Pointer to thread or NULL if not found
 */
thread_t *thread_find(uint64_t tid);

/**
 * Get current thread
 *
 * @return Pointer to current thread or NULL
 */
thread_t *thread_get_current(void);

/**
 * Set thread priority
 *
 * @param thread Thread to set priority for
 * @param priority New priority (-20 to +19)
 * @return 0 on success, negative error code on failure
 */
int thread_set_priority(thread_t *thread, int priority);

/**
 * Put thread to sleep
 *
 * @param thread Thread to sleep
 * @param nanoseconds Sleep duration in nanoseconds
 * @return 0 on success, negative error code on failure
 */
int thread_sleep(thread_t *thread, uint64_t nanoseconds);

/**
 * Wake up a sleeping thread
 *
 * @param thread Thread to wake up
 * @return 0 on success, negative error code on failure
 */
int thread_wakeup(thread_t *thread);

/**
 * Get thread count
 *
 * @return Number of active threads
 */
uint32_t thread_get_count(void);

/**
 * Initialize thread management system
 */
void thread_init(void);

#endif // ORION_THREAD_H
