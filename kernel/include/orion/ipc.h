/*
 * Orion Operating System - Inter-Process Communication Header
 *
 * IPC system declarations for process communication.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_IPC_H
#define ORION_IPC_H

#include <orion/types.h>
#include <orion/forward_decls.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // IPC functions
    ipc_port_t *ipc_create_port(const char *name);
    void ipc_destroy_port(ipc_port_t *port);
    int ipc_send_message(ipc_port_t *port, const void *data, size_t size);
    int ipc_receive_message(ipc_port_t *port, void *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif // ORION_IPC_H
