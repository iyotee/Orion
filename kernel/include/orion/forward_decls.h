/*
 * Orion Operating System - Forward Declarations
 * 
 * This header contains forward declarations for all major structures
 * to avoid circular dependencies between headers.
 * 
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_FORWARD_DECLS_H
#define ORION_FORWARD_DECLS_H

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations for all major structures
struct process;
struct thread;
struct vm_space;
struct cpu_runqueue;
struct orion_handle;
struct capability;
struct security_context;
struct ipc_port;
struct vfs_inode;
struct vfs_file;
struct vfs_mount;

// Typedefs for forward declarations
typedef struct process process_t;
typedef struct thread thread_t;
typedef struct vm_space vm_space_t;
typedef struct cpu_runqueue cpu_runqueue_t;
typedef struct orion_handle orion_handle_t;
typedef struct capability capability_t;
typedef struct security_context security_context_t;
typedef struct ipc_port ipc_port_t;
typedef struct vfs_inode vfs_inode_t;
typedef struct vfs_file vfs_file_t;
typedef struct vfs_mount vfs_mount_t;

#ifdef __cplusplus
}
#endif

#endif // ORION_FORWARD_DECLS_H
