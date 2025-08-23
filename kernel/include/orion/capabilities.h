/*
 * Orion Operating System - Capability System Header
 *
 * Capability-based security system declarations.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_CAPABILITIES_H
#define ORION_CAPABILITIES_H

#include <orion/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // Capability types
    typedef enum
    {
        CAP_TYPE_NONE = 0,
        CAP_TYPE_READ,
        CAP_TYPE_WRITE,
        CAP_TYPE_EXECUTE,
        CAP_TYPE_DELETE,
        CAP_TYPE_MOUNT,
        CAP_TYPE_IPC,
        CAP_TYPE_NETWORK,
        CAP_TYPE_DEVICE,
        CAP_TYPE_SYSTEM,
        CAP_TYPE_FILE
    } cap_type_t;

// Capability rights
#define CAP_READ (1 << 0)
#define CAP_WRITE (1 << 1)
#define CAP_EXECUTE (1 << 2)
#define CAP_DELETE (1 << 3)

    // Capability structure
    typedef struct
    {
        cap_type_t type;
        uint64_t rights;
        uint64_t target;
        uint64_t owner_pid;
    } or_cap_t;

    // Capability functions
    or_cap_t cap_create(cap_type_t type, uint64_t rights, uint64_t target, uint64_t owner_pid);
    bool cap_check_rights(or_cap_t cap, uint64_t rights, uint64_t target);
    void cap_destroy(or_cap_t cap);

#ifdef __cplusplus
}
#endif

#endif // ORION_CAPABILITIES_H
