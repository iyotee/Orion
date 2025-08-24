/*
 * Orion Operating System - Scheduler Header
 *
 * Process and thread scheduling system declarations.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_SCHEDULER_H
#define ORION_SCHEDULER_H

#include <orion/types.h>
#include <orion/forward_decls.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // Scheduler functions
    void scheduler_init(void);
    void scheduler_add_thread_to_rq(thread_t *thread);
    void scheduler_remove_thread_from_rq(thread_t *thread);
    thread_t *scheduler_get_next_thread(void);
    process_t *scheduler_get_current_process(void);

    // Handle structure
    typedef struct
    {
        uint32_t type;
        uint64_t object_id;
        uint32_t permissions;
        uint32_t ref_count;
    } orion_handle_t;

    // Utilities
    void handle_cleanup(orion_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif // ORION_SCHEDULER_H
