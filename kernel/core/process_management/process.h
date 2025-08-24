/*
 * Orion Operating System - Process Management Header
 *
 * Process and thread management structures and functions.
 * Provides the core process management interface.
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_PROCESS_H
#define ORION_PROCESS_H

#include <orion/types.h>
#include <orion/forward_decls.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // Process states
    typedef enum
    {
        PROCESS_STATE_NEW = 0,
        PROCESS_STATE_READY,
        PROCESS_STATE_RUNNING,
        PROCESS_STATE_WAITING,
        PROCESS_STATE_TERMINATED,
        PROCESS_STATE_ZOMBIE
    } process_state_t;

    // Process structure is defined in structures.h
    // This file only contains forward declarations and function prototypes

    // Thread states
    typedef enum
    {
        THREAD_STATE_NEW = 0,
        THREAD_STATE_READY,
        THREAD_STATE_RUNNING,
        THREAD_STATE_WAITING,
        THREAD_STATE_TERMINATED
    } thread_state_t;

    // Thread structure is defined in structures.h
    // This file only contains forward declarations and function prototypes

    // Process management functions
    process_t *process_create(const char *name, uint64_t priority);
    void process_destroy(process_t *process);
    int process_start(process_t *process);
    int process_stop(process_t *process);
    process_t *process_get_by_pid(pid_t pid);
    process_t *process_get_current(void);
    pid_t process_get_current_pid(void);

    // Thread management functions
    thread_t *thread_create(process_t *process, void *entry_point, void *stack, size_t stack_size);
    void thread_destroy(thread_t *thread);
    int thread_start(thread_t *thread);
    int thread_stop(thread_t *thread);
    thread_t *thread_get_by_tid(tid_t tid);
    thread_t *thread_get_current(void);
    tid_t thread_get_current_tid(void);

    // Process/Thread scheduling
    void scheduler_add_thread_to_rq(thread_t *thread);
    void scheduler_remove_thread_from_rq(thread_t *thread);
    thread_t *scheduler_get_next_thread(void);

#ifdef __cplusplus
}
#endif

#endif // ORION_PROCESS_H
