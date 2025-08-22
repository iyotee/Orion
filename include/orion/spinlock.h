/*
 * Orion Operating System - Spinlock Implementation
 *
 * Lightweight spinlock implementation for kernel synchronization.
 * Provides efficient locking for short critical sections.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_SPINLOCK_H
#define ORION_SPINLOCK_H

#include <orion/types.h>

// Spinlock structure
typedef struct
{
    volatile uint32_t locked;
} spinlock_t;

// Spinlock constants
#define SPINLOCK_INIT {0}

// Spinlock operations
static inline void spinlock_init(spinlock_t *lock)
{
    lock->locked = 0;
}

static inline void spinlock_acquire(spinlock_t *lock)
{
    while (__sync_lock_test_and_set(&lock->locked, 1))
    {
        // Spin until lock is acquired
        __asm__ volatile("pause");
    }
}

static inline void spinlock_release(spinlock_t *lock)
{
    __sync_lock_release(&lock->locked);
}

static inline bool spinlock_try_acquire(spinlock_t *lock)
{
    return __sync_bool_compare_and_swap(&lock->locked, 0, 1);
}

// Convenience macros
#define SPINLOCK_INITIALIZER {0}

#endif // ORION_SPINLOCK_H
