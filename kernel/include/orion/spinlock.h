/*
 * Orion Operating System - Spinlock Implementation
 *
 * Linux/GCC-compatible spinlock implementation for the Orion kernel.
 * Provides atomic locking primitives for kernel synchronization.
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

#ifdef __cplusplus
extern "C"
{
#endif

    // Spinlock structure
    typedef struct spinlock
    {
        volatile uint32_t locked;
        uint32_t cpu_id;    // CPU that holds the lock
        uint32_t irq_count; // IRQ disable count
    } spinlock_t;

    // Initialize a spinlock
    static inline void spinlock_init(spinlock_t *lock)
    {
        lock->locked = 0;
        lock->cpu_id = 0;
        lock->irq_count = 0;
    }

    // Acquire a spinlock
    static inline void spinlock_acquire(spinlock_t *lock)
    {
        while (__sync_lock_test_and_set(&lock->locked, 1))
        {
            // Spin until we get the lock
            __asm__ volatile("pause");
        }
        lock->cpu_id = 1; // TODO: Get actual CPU ID
    }

    // Release a spinlock
    static inline void spinlock_release(spinlock_t *lock)
    {
        lock->cpu_id = 0;
        __sync_lock_release(&lock->locked);
    }

    // Try to acquire a spinlock (non-blocking)
    static inline bool spinlock_try_acquire(spinlock_t *lock)
    {
        if (__sync_lock_test_and_set(&lock->locked, 1) == 0)
        {
            lock->cpu_id = 1; // TODO: Get actual CPU ID
            return true;
        }
        return false;
    }

    // Check if spinlock is held
    static inline bool spinlock_is_held(spinlock_t *lock)
    {
        return lock->locked != 0;
    }

    // IRQ-safe spinlock operations
    static inline void spinlock_acquire_irq(spinlock_t *lock)
    {
        // TODO: Disable interrupts
        spinlock_acquire(lock);
        lock->irq_count++;
    }

    static inline void spinlock_release_irq(spinlock_t *lock)
    {
        lock->irq_count--;
        spinlock_release(lock);
        // TODO: Restore interrupts if count reaches 0
    }

#ifdef __cplusplus
}
#endif

#endif // ORION_SPINLOCK_H
