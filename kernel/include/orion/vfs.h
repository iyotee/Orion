/*
 * Orion Operating System - Virtual File System Header
 *
 * VFS interface declarations for file system operations.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_VFS_H
#define ORION_VFS_H

#include <orion/types.h>
#include <orion/forward_decls.h>

// Define mode_t if not already defined
#ifndef mode_t
typedef uint32_t mode_t;
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    // VFS operations
    int vfs_mount(const char *source, const char *target, const char *fstype);
    int vfs_umount(const char *target);
    int vfs_path_resolve(const char *path, vfs_inode_t **inode);
    int vfs_create_inode(const char *path, mode_t mode);
    int vfs_create_dentry(const char *path, vfs_inode_t *inode);
    int vfs_check_permissions(const char *path, mode_t mode);

    // VFS file operations
    const vfs_file_ops_t *vfs_file_ops;
    const vfs_file_ops_t *vfs_dir_ops;
    const vfs_file_ops_t *vfs_device_ops;

// VFS constants
#define OR_EACCES 13
#define OR_EISDIR 21

// File type checking macros
#define S_ISREG(m) (((m) & 0170000) == 0100000)
#define S_ISDIR(m) (((m) & 0170000) == 0040000)
#define S_ISCHR(m) (((m) & 0170000) == 0020000)
#define S_ISBLK(m) (((m) & 0170000) == 0060000)

    // File operations structures (forward declarations)
    typedef struct vfs_file_ops vfs_file_ops_t;

    // External file operations
    extern const vfs_file_ops_t vfs_file_ops;
    extern const vfs_file_ops_t vfs_dir_ops;
    extern const vfs_file_ops_t vfs_device_ops;

#ifdef __cplusplus
}
#endif

#endif // ORION_VFS_H
