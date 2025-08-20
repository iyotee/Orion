/*
 * Orion Operating System - Virtual File System Header
 *
 * VFS interface definitions and function declarations for the complete
 * virtual file system with security integration and device support.
 *
 * Developed by Jérémy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_VFS_H
#define ORION_VFS_H

#include <orion/types.h>

// ========================================
// VFS CONSTANTS
// ========================================

// File types
#define S_IFMT 0170000   // File type mask
#define S_IFREG 0100000  // Regular file
#define S_IFDIR 0040000  // Directory
#define S_IFCHR 0020000  // Character device
#define S_IFBLK 0060000  // Block device
#define S_IFIFO 0010000  // FIFO
#define S_IFLNK 0120000  // Symbolic link
#define S_IFSOCK 0140000 // Socket

// File permissions
#define S_ISUID 0004000 // Set user ID on execution
#define S_ISGID 0002000 // Set group ID on execution
#define S_ISVTX 0001000 // Sticky bit
#define S_IRWXU 0000700 // User permissions
#define S_IRUSR 0000400 // User read
#define S_IWUSR 0000200 // User write
#define S_IXUSR 0000100 // User execute
#define S_IRWXG 0000070 // Group permissions
#define S_IRGRP 0000040 // Group read
#define S_IWGRP 0000020 // Group write
#define S_IXGRP 0000010 // Group execute
#define S_IRWXO 0000007 // Other permissions
#define S_IROTH 0000004 // Other read
#define S_IWOTH 0000002 // Other write
#define S_IXOTH 0000001 // Other execute

// File flags
#define O_RDONLY 0000000    // Read only
#define O_WRONLY 0000001    // Write only
#define O_RDWR 0000002      // Read and write
#define O_CREAT 0000100     // Create if not exists
#define O_EXCL 0000200      // Exclusive creation
#define O_TRUNC 0001000     // Truncate to zero length
#define O_APPEND 0002000    // Append mode
#define O_NONBLOCK 0004000  // Non-blocking I/O
#define O_SYNC 0010000      // Synchronous I/O
#define O_DIRECTORY 0200000 // Must be directory
#define O_NOFOLLOW 0400000  // Don't follow symlinks

// Seek constants
#define SEEK_SET 0 // Seek from beginning
#define SEEK_CUR 1 // Seek from current position
#define SEEK_END 2 // Seek from end

// ========================================
// VFS STRUCTURES
// ========================================

// File attributes
typedef struct vfs_stat
{
    uint64_t st_dev;     // Device ID
    uint64_t st_ino;     // Inode number
    uint32_t st_mode;    // File mode
    uint32_t st_nlink;   // Number of hard links
    uint32_t st_uid;     // User ID
    uint32_t st_gid;     // Group ID
    uint64_t st_rdev;    // Device ID (if special file)
    uint64_t st_size;    // File size in bytes
    uint64_t st_blksize; // Block size for I/O
    uint64_t st_blocks;  // Number of 512B blocks
    uint64_t st_atime;   // Last access time
    uint64_t st_mtime;   // Last modification time
    uint64_t st_ctime;   // Last status change time
} vfs_stat_t;

// Directory entry
typedef struct vfs_dirent
{
    uint64_t d_ino;    // Inode number
    uint64_t d_off;    // Offset to next record
    uint16_t d_reclen; // Length of this record
    uint8_t d_type;    // File type
    char d_name[256];  // Null-terminated filename
} vfs_dirent_t;

// File system statistics
typedef struct vfs_statfs
{
    uint64_t f_type;    // File system type
    uint64_t f_bsize;   // Block size
    uint64_t f_blocks;  // Total blocks
    uint64_t f_bfree;   // Free blocks
    uint64_t f_bavail;  // Available blocks
    uint64_t f_files;   // Total inodes
    uint64_t f_ffree;   // Free inodes
    uint64_t f_fsid;    // File system ID
    uint64_t f_namelen; // Maximum filename length
    uint64_t f_frsize;  // Fragment size
    uint64_t f_flags;   // Mount flags
} vfs_statfs_t;

// Forward declarations for opaque types
typedef struct vfs_file vfs_file_t;
typedef struct vfs_filesystem vfs_filesystem_t;

// ========================================
// VFS FUNCTION DECLARATIONS
// ========================================

// Core VFS functions
void vfs_init(void);

// File system registration
int vfs_register_filesystem(vfs_filesystem_t *fs);
int vfs_unregister_filesystem(vfs_filesystem_t *fs);

// File operations
int vfs_open(const char *path, uint32_t flags, uint32_t mode, vfs_file_t **file_out);
int vfs_close(int fd);
ssize_t vfs_read(int fd, void *buffer, size_t count);
ssize_t vfs_write(int fd, const void *buffer, size_t count);
uint64_t vfs_seek(int fd, uint64_t offset, int whence);

// Directory operations
int vfs_mkdir(const char *path, uint32_t mode);
int vfs_rmdir(const char *path);
int vfs_opendir(const char *path);
int vfs_readdir(int fd, vfs_dirent_t *dirent);
int vfs_closedir(int fd);

// File metadata operations
int vfs_stat(const char *path, vfs_stat_t *stat);
int vfs_fstat(int fd, vfs_stat_t *stat);
int vfs_chmod(const char *path, uint32_t mode);
int vfs_chown(const char *path, uint32_t uid, uint32_t gid);

// File system operations
int vfs_mount(const char *source, const char *target, const char *fstype, uint32_t flags, void *data);
int vfs_unmount(const char *target);
int vfs_statfs(const char *path, vfs_statfs_t *statfs);

// Path operations
int vfs_link(const char *oldpath, const char *newpath);
int vfs_unlink(const char *path);
int vfs_rename(const char *oldpath, const char *newpath);
int vfs_symlink(const char *target, const char *linkpath);
ssize_t vfs_readlink(const char *path, char *buffer, size_t bufsize);

// Advanced operations
int vfs_ioctl(int fd, uint32_t cmd, void *arg);
int vfs_sync(void);
int vfs_fsync(int fd);

// Statistics and debugging
void vfs_get_stats(uint64_t *open_files_count, uint64_t *cache_hits,
                   uint64_t *cache_misses, uint64_t *mounted_fs);
void vfs_debug_info(void);

// ========================================
// FILE SYSTEM SPECIFIC FUNCTIONS
// ========================================

// RAMFS functions
void ramfs_init(void);
void ramfs_get_stats(uint64_t *total_files, uint64_t *used_files,
                     uint64_t *total_memory, uint64_t *used_memory);
void ramfs_debug_info(void);

// Device file system functions
void devfs_init(void);
int devfs_register_device(const char *name, uint32_t major, uint32_t minor,
                          uint32_t type, void *ops);
int devfs_unregister_device(const char *name);
void devfs_debug_info(void);

// Proc file system functions
void procfs_init(void);
void procfs_debug_info(void);

#endif // ORION_VFS_H
