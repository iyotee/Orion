/*
 * Orion Operating System - System Servers Header
 *
 * System server initialization and management declarations.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_SERVERS_H
#define ORION_SERVERS_H

#include <orion/types.h>

// ========================================
// SERVER MANAGEMENT FUNCTIONS
// ========================================

/**
 * Start file system server
 *
 * @return 0 on success, negative error code on failure
 */
int start_fs_server(void);

/**
 * Start network server
 *
 * @return 0 on success, negative error code on failure
 */
int start_network_server(void);

/**
 * Start device manager
 *
 * @return 0 on success, negative error code on failure
 */
int start_device_manager(void);

/**
 * Start IPC server
 *
 * @return 0 on success, negative error code on failure
 */
int start_ipc_server(void);

// ========================================
// SERVER MAIN FUNCTIONS
// ========================================

/**
 * File system server main function
 *
 * @param arg Server argument (unused)
 */
void fs_server_main(void *arg);

/**
 * Network server main function
 *
 * @param arg Server argument (unused)
 */
void net_server_main(void *arg);

/**
 * Device manager main function
 *
 * @param arg Server argument (unused)
 */
void device_manager_main(void *arg);

/**
 * IPC server main function
 *
 * @param arg Server argument (unused)
 */
void ipc_server_main(void *arg);

// ========================================
// SERVER UTILITY FUNCTIONS
// ========================================

/**
 * Get server count
 *
 * @return Number of active servers
 */
uint32_t server_get_count(void);

/**
 * Get server by type
 *
 * @param type Server type to find
 * @return Pointer to server or NULL if not found
 */
void *server_get_by_type(uint32_t type);

/**
 * Stop a server
 *
 * @param type Server type to stop
 * @return 0 on success, negative error code on failure
 */
int server_stop(uint32_t type);

/**
 * Initialize server management system
 */
void server_init(void);

#endif // ORION_SERVERS_H
