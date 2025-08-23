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
#include <orion/forward_decls.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // Thread creation and management
    thread_t *thread_create(process_t *process, void *entry_point, void *stack, size_t stack_size);
    void thread_destroy(thread_t *thread);
    int thread_start(thread_t *thread);
    int thread_stop(thread_t *thread);

    // Thread information
    thread_t *thread_get_by_tid(tid_t tid);
    thread_t *thread_get_current(void);
    tid_t thread_get_current_tid(void);

    // Thread scheduling
    void scheduler_add_thread_to_rq(thread_t *thread);
    void scheduler_remove_thread_from_rq(thread_t *thread);
    thread_t *scheduler_get_next_thread(void);

    // Thread synchronization
    typedef struct
    {
        uint32_t value;
        spinlock_t lock;
    } semaphore_t;

    int semaphore_init(semaphore_t *sem, uint32_t initial_value);
    int semaphore_wait(semaphore_t *sem);
    int semaphore_signal(semaphore_t *sem);
    void semaphore_destroy(semaphore_t *sem);

    // Thread-local storage
    void *thread_get_tls(void);
    int thread_set_tls(void *tls);

#ifdef __cplusplus
}
#endif

#endif // ORION_THREAD_H
