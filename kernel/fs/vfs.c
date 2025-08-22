/*
 * Orion Operating System - Virtual File System (VFS)
 *
 * Complete VFS implementation with device drivers, file operations,
 * and directory management. Supports multiple file system types.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/kernel.h>
#include <orion/types.h>
#include <orion/mm.h>
#include <orion/security.h>
#include <orion/syscalls.h>

// ========================================
// VFS CONSTANTS AND CONFIGURATION
// ========================================

#define MAX_FILESYSTEMS     32          // Maximum registered file systems
#define MAX_MOUNTS          256         // Maximum mount points
#define MAX_OPEN_FILES      4096        // Maximum open files globally
#define MAX_FILENAME_LEN    255         // Maximum filename length
#define MAX_PATH_LEN        4096        // Maximum path length
#define VFS_CACHE_SIZE      1024        // VFS cache entries

// File types
#define S_IFMT      0170000             // File type mask
#define S_IFREG     0100000             // Regular file
#define S_IFDIR     0040000             // Directory
#define S_IFCHR     0020000             // Character device
#define S_IFBLK     0060000             // Block device
#define S_IFIFO     0010000             // FIFO
#define S_IFLNK     0120000             // Symbolic link
#define S_IFSOCK    0140000             // Socket

// File permissions
#define S_ISUID     0004000             // Set user ID on execution
#define S_ISGID     0002000             // Set group ID on execution
#define S_ISVTX     0001000             // Sticky bit
#define S_IRWXU     0000700             // User permissions
#define S_IRUSR     0000400             // User read
#define S_IWUSR     0000200             // User write
#define S_IXUSR     0000100             // User execute
#define S_IRWXG     0000070             // Group permissions
#define S_IRGRP     0000040             // Group read
#define S_IWGRP     0000020             // Group write
#define S_IXGRP     0000010             // Group execute
#define S_IRWXO     0000007             // Other permissions
#define S_IROTH     0000004             // Other read
#define S_IWOTH     0000002             // Other write
#define S_IXOTH     0000001             // Other execute

// File flags
#define O_RDONLY    0000000             // Read only
#define O_WRONLY    0000001             // Write only
#define O_RDWR      0000002             // Read and write
#define O_CREAT     0000100             // Create if not exists
#define O_EXCL      0000200             // Exclusive creation
#define O_TRUNC     0001000             // Truncate to zero length
#define O_APPEND    0002000             // Append mode
#define O_NONBLOCK  0004000             // Non-blocking I/O
#define O_SYNC      0010000             // Synchronous I/O
#define O_DIRECTORY 0200000             // Must be directory
#define O_NOFOLLOW  0400000             // Don't follow symlinks

// ========================================
// VFS DATA STRUCTURES
// ========================================

// File system statistics
typedef struct vfs_statfs {
    uint64_t f_type;                    // File system type
    uint64_t f_bsize;                   // Block size
    uint64_t f_blocks;                  // Total blocks
    uint64_t f_bfree;                   // Free blocks
    uint64_t f_bavail;                  // Available blocks
    uint64_t f_files;                   // Total inodes
    uint64_t f_ffree;                   // Free inodes
    uint64_t f_fsid;                    // File system ID
    uint64_t f_namelen;                 // Maximum filename length
    uint64_t f_frsize;                  // Fragment size
    uint64_t f_flags;                   // Mount flags
} vfs_statfs_t;

// File attributes
typedef struct vfs_stat {
    uint64_t st_dev;                    // Device ID
    uint64_t st_ino;                    // Inode number
    uint32_t st_mode;                   // File mode
    uint32_t st_nlink;                  // Number of hard links
    uint32_t st_uid;                    // User ID
    uint32_t st_gid;                    // Group ID
    uint64_t st_rdev;                   // Device ID (if special file)
    uint64_t st_size;                   // File size in bytes
    uint64_t st_blksize;                // Block size for I/O
    uint64_t st_blocks;                 // Number of 512B blocks
    uint64_t st_atime;                  // Last access time
    uint64_t st_mtime;                  // Last modification time
    uint64_t st_ctime;                  // Last status change time
} vfs_stat_t;

// Directory entry
typedef struct vfs_dirent {
    uint64_t d_ino;                     // Inode number
    uint64_t d_off;                     // Offset to next record
    uint16_t d_reclen;                  // Length of this record
    uint8_t  d_type;                    // File type
    char d_name[MAX_FILENAME_LEN + 1];  // Null-terminated filename
} vfs_dirent_t;

// Forward declarations
typedef struct vfs_inode vfs_inode_t;
typedef struct vfs_file vfs_file_t;
typedef struct vfs_dentry vfs_dentry_t;
typedef struct vfs_superblock vfs_superblock_t;
typedef struct vfs_filesystem vfs_filesystem_t;

// File operations structure
typedef struct vfs_file_ops {
    int (*open)(vfs_inode_t* inode, vfs_file_t* file);
    int (*close)(vfs_inode_t* inode, vfs_file_t* file);
    ssize_t (*read)(vfs_file_t* file, char* buffer, size_t count, uint64_t* offset);
    ssize_t (*write)(vfs_file_t* file, const char* buffer, size_t count, uint64_t* offset);
    int (*seek)(vfs_file_t* file, uint64_t offset, int whence);
    int (*ioctl)(vfs_file_t* file, uint32_t cmd, void* arg);
    int (*mmap)(vfs_file_t* file, uint64_t addr, size_t len, int prot, int flags);
    int (*flush)(vfs_file_t* file);
    int (*fsync)(vfs_file_t* file);
} vfs_file_ops_t;

// Inode operations structure
typedef struct vfs_inode_ops {
    int (*create)(vfs_inode_t* dir, vfs_dentry_t* dentry, uint32_t mode);
    int (*lookup)(vfs_inode_t* dir, vfs_dentry_t* dentry);
    int (*link)(vfs_dentry_t* old_dentry, vfs_inode_t* dir, vfs_dentry_t* dentry);
    int (*unlink)(vfs_inode_t* dir, vfs_dentry_t* dentry);
    int (*symlink)(vfs_inode_t* dir, vfs_dentry_t* dentry, const char* symname);
    int (*mkdir)(vfs_inode_t* dir, vfs_dentry_t* dentry, uint32_t mode);
    int (*rmdir)(vfs_inode_t* dir, vfs_dentry_t* dentry);
    int (*rename)(vfs_inode_t* old_dir, vfs_dentry_t* old_dentry,
                  vfs_inode_t* new_dir, vfs_dentry_t* new_dentry);
    int (*getattr)(vfs_dentry_t* dentry, vfs_stat_t* stat);
    int (*setattr)(vfs_dentry_t* dentry, vfs_stat_t* stat);
} vfs_inode_ops_t;

// Superblock operations
typedef struct vfs_super_ops {
    int (*read_inode)(vfs_superblock_t* sb, vfs_inode_t* inode);
    int (*write_inode)(vfs_superblock_t* sb, vfs_inode_t* inode);
    int (*delete_inode)(vfs_superblock_t* sb, vfs_inode_t* inode);
    int (*statfs)(vfs_superblock_t* sb, vfs_statfs_t* statfs);
    int (*sync_fs)(vfs_superblock_t* sb);
    int (*freeze_fs)(vfs_superblock_t* sb);
    int (*unfreeze_fs)(vfs_superblock_t* sb);
} vfs_super_ops_t;

// VFS inode structure
struct vfs_inode {
    uint64_t i_ino;                     // Inode number
    uint32_t i_mode;                    // File mode and permissions
    uint32_t i_uid;                     // Owner user ID
    uint32_t i_gid;                     // Owner group ID
    uint32_t i_nlink;                   // Number of hard links
    uint64_t i_size;                    // File size in bytes
    uint64_t i_blocks;                  // Number of blocks
    uint64_t i_atime;                   // Last access time
    uint64_t i_mtime;                   // Last modification time
    uint64_t i_ctime;                   // Last status change time
    uint64_t i_version;                 // Version number
    atomic64_t i_count;                 // Reference count
    spinlock_t i_lock;                  // Inode lock
    
    // File system specific data
    void* i_private;                    // File system private data
    vfs_superblock_t* i_sb;             // Superblock
    
    // Operations
    const vfs_file_ops_t* i_fop;        // File operations
    const vfs_inode_ops_t* i_op;        // Inode operations
    
    // Hash chain for inode cache
    struct vfs_inode* i_hash_next;
    struct vfs_inode* i_hash_prev;
    
    // LRU chain
    struct vfs_inode* i_lru_next;
    struct vfs_inode* i_lru_prev;
};

// VFS file structure
struct vfs_file {
    vfs_dentry_t* f_dentry;             // Associated directory entry
    vfs_inode_t* f_inode;               // Associated inode
    const vfs_file_ops_t* f_op;         // File operations
    uint64_t f_pos;                     // Current file position
    uint32_t f_flags;                   // File flags (O_RDONLY, etc.)
    uint32_t f_mode;                    // File mode
    atomic64_t f_count;                 // Reference count
    spinlock_t f_lock;                  // File lock
    void* f_private;                    // File system private data
    
    // Security context
    or_cap_t f_capability;              // Required capability
    uint64_t f_owner_pid;               // Process that opened the file
};

// VFS directory entry
struct vfs_dentry {
    char d_name[MAX_FILENAME_LEN + 1];  // Name
    vfs_inode_t* d_inode;               // Associated inode
    vfs_dentry_t* d_parent;             // Parent directory
    vfs_dentry_t* d_child;              // First child
    vfs_dentry_t* d_sibling;            // Next sibling
    atomic32_t d_count;                 // Reference count
    spinlock_t d_lock;                  // Dentry lock
    uint32_t d_flags;                   // Flags
    
    // Hash chain for dentry cache
    struct vfs_dentry* d_hash_next;
    struct vfs_dentry* d_hash_prev;
    
    // LRU chain
    struct vfs_dentry* d_lru_next;
    struct vfs_dentry* d_lru_prev;
};

// VFS superblock
struct vfs_superblock {
    uint64_t s_blocksize;               // Block size
    uint64_t s_blocksize_bits;          // Block size in bits
    uint64_t s_maxbytes;                // Maximum file size
    vfs_filesystem_t* s_type;           // File system type
    const vfs_super_ops_t* s_op;        // Superblock operations
    uint32_t s_flags;                   // Mount flags
    uint32_t s_magic;                   // File system magic number
    vfs_dentry_t* s_root;               // Root directory
    spinlock_t s_lock;                  // Superblock lock
    atomic32_t s_count;                 // Reference count
    void* s_private;                    // File system private data
    
    // Statistics
    uint64_t s_inodes_count;            // Total inodes
    uint64_t s_free_inodes_count;       // Free inodes
    uint64_t s_blocks_count;            // Total blocks
    uint64_t s_free_blocks_count;       // Free blocks
};

// File system type
struct vfs_filesystem {
    char name[32];                      // File system name
    uint32_t fs_flags;                  // File system flags
    
    // Mount function
    vfs_superblock_t* (*mount)(vfs_filesystem_t* fs, uint32_t flags, 
                               const char* dev_name, void* data);
    
    // Unmount function
    void (*unmount)(vfs_superblock_t* sb);
    
    // Next in list
    struct vfs_filesystem* next;
};

// Mount point
typedef struct vfs_mount {
    vfs_dentry_t* mnt_mountpoint;       // Directory where mounted
    vfs_dentry_t* mnt_root;             // Root of mounted file system
    vfs_superblock_t* mnt_sb;           // Superblock
    uint32_t mnt_flags;                 // Mount flags
    atomic32_t mnt_count;               // Reference count
    char mnt_devname[64];               // Device name
    struct vfs_mount* mnt_parent;       // Parent mount
    struct vfs_mount* mnt_next;         // Next mount
} vfs_mount_t;

// ========================================
// GLOBAL VFS STATE
// ========================================

static vfs_filesystem_t* filesystems = NULL;       // Registered file systems
static vfs_mount_t* mounts = NULL;                  // Mount points
static vfs_file_t* open_files[MAX_OPEN_FILES];     // Open file table
static vfs_dentry_t* vfs_root = NULL;               // Root directory
static spinlock_t vfs_lock = SPINLOCK_INIT;        // Global VFS lock
static bool vfs_initialized = false;

// Inode cache
static vfs_inode_t* inode_hash[256];                // Inode hash table
static vfs_inode_t* inode_lru_head = NULL;          // LRU list head
static spinlock_t inode_cache_lock = SPINLOCK_INIT;

// Dentry cache
static vfs_dentry_t* dentry_hash[256];              // Dentry hash table
static vfs_dentry_t* dentry_lru_head = NULL;        // LRU list head
static spinlock_t dentry_cache_lock = SPINLOCK_INIT;

// Statistics
static atomic64_t vfs_stats_open_files = ATOMIC_VAR_INIT(0);
static atomic64_t vfs_stats_cache_hits = ATOMIC_VAR_INIT(0);
static atomic64_t vfs_stats_cache_misses = ATOMIC_VAR_INIT(0);

// ========================================
// VFS INITIALIZATION
// ========================================

void vfs_init(void) {
    kinfo("Initializing Virtual File System");
    
    // Initialize open file table
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        open_files[i] = NULL;
    }
    
    // Initialize hash tables
    for (int i = 0; i < 256; i++) {
        inode_hash[i] = NULL;
        dentry_hash[i] = NULL;
    }
    
    vfs_initialized = true;
    
    kinfo("VFS initialized successfully");
    kinfo("  Maximum open files: %d", MAX_OPEN_FILES);
    kinfo("  Maximum filename length: %d", MAX_FILENAME_LEN);
    kinfo("  Cache size: %d entries", VFS_CACHE_SIZE);
}

// ========================================
// HASH AND CACHE MANAGEMENT
// ========================================

static uint32_t hash_string(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

static uint32_t hash_inode(uint64_t ino, vfs_superblock_t* sb) {
    return (uint32_t)((ino ^ (uint64_t)sb) % 256);
}

// ========================================
// FILE SYSTEM REGISTRATION
// ========================================

int vfs_register_filesystem(vfs_filesystem_t* fs) {
    if (!fs || !fs->name[0] || !fs->mount) {
        return -OR_EINVAL;
    }
    
    spinlock_lock(&vfs_lock);
    
    // Check if already registered
    vfs_filesystem_t* current = filesystems;
    while (current) {
        if (strcmp(current->name, fs->name) == 0) {
            spinlock_unlock(&vfs_lock);
            return -OR_EEXIST;
        }
        current = current->next;
    }
    
    // Add to list
    fs->next = filesystems;
    filesystems = fs;
    
    spinlock_unlock(&vfs_lock);
    
    kinfo("Registered file system: %s", fs->name);
    return OR_OK;
}

int vfs_unregister_filesystem(vfs_filesystem_t* fs) {
    if (!fs) {
        return -OR_EINVAL;
    }
    
    spinlock_lock(&vfs_lock);
    
    // Find and remove from list
    if (filesystems == fs) {
        filesystems = fs->next;
    } else {
        vfs_filesystem_t* current = filesystems;
        while (current && current->next != fs) {
            current = current->next;
        }
        if (current) {
            current->next = fs->next;
        } else {
            spinlock_unlock(&vfs_lock);
            return -OR_ENOENT;
        }
    }
    
    spinlock_unlock(&vfs_lock);
    
    kinfo("Unregistered file system: %s", fs->name);
    return OR_OK;
}

// ========================================
// INODE CACHE MANAGEMENT
// ========================================

static vfs_inode_t* inode_alloc(vfs_superblock_t* sb) {
    vfs_inode_t* inode = (vfs_inode_t*)kmalloc(sizeof(vfs_inode_t));
    if (!inode) {
        return NULL;
    }
    
    memset(inode, 0, sizeof(vfs_inode_t));
    atomic_store(&inode->i_count, 1);
    spinlock_init(&inode->i_lock);
    inode->i_sb = sb;
    
    return inode;
}

static void inode_free(vfs_inode_t* inode) {
    if (inode) {
        kfree(inode);
    }
}

vfs_inode_t* inode_get(vfs_superblock_t* sb, uint64_t ino) {
    uint32_t hash = hash_inode(ino, sb);
    
    spinlock_lock(&inode_cache_lock);
    
    // Search in cache
    vfs_inode_t* inode = inode_hash[hash];
    while (inode) {
        if (inode->i_ino == ino && inode->i_sb == sb) {
            atomic_fetch_add(&inode->i_count, 1);
            atomic_fetch_add(&vfs_stats_cache_hits, 1);
            spinlock_unlock(&inode_cache_lock);
            return inode;
        }
        inode = inode->i_hash_next;
    }
    
    atomic_fetch_add(&vfs_stats_cache_misses, 1);
    spinlock_unlock(&inode_cache_lock);
    
    // Not in cache, allocate new
    inode = inode_alloc(sb);
    if (!inode) {
        return NULL;
    }
    
    inode->i_ino = ino;
    
    // Read from storage
    if (sb->s_op && sb->s_op->read_inode) {
        int result = sb->s_op->read_inode(sb, inode);
        if (result != OR_OK) {
            inode_free(inode);
            return NULL;
        }
    }
    
    // Add to cache
    spinlock_lock(&inode_cache_lock);
    inode->i_hash_next = inode_hash[hash];
    if (inode_hash[hash]) {
        inode_hash[hash]->i_hash_prev = inode;
    }
    inode_hash[hash] = inode;
    spinlock_unlock(&inode_cache_lock);
    
    return inode;
}

void inode_put(vfs_inode_t* inode) {
    if (!inode) {
        return;
    }
    
    if (atomic_fetch_sub(&inode->i_count, 1) == 1) {
        // Last reference, remove from cache and free
        spinlock_lock(&inode_cache_lock);
        
        uint32_t hash = hash_inode(inode->i_ino, inode->i_sb);
        
        if (inode->i_hash_prev) {
            inode->i_hash_prev->i_hash_next = inode->i_hash_next;
        } else {
            inode_hash[hash] = inode->i_hash_next;
        }
        
        if (inode->i_hash_next) {
            inode->i_hash_next->i_hash_prev = inode->i_hash_prev;
        }
        
        spinlock_unlock(&inode_cache_lock);
        
        // Write back if dirty
        if (inode->i_sb->s_op && inode->i_sb->s_op->write_inode) {
            inode->i_sb->s_op->write_inode(inode->i_sb, inode);
        }
        
        inode_free(inode);
    }
}

// ========================================
// FILE OPERATIONS
// ========================================

int vfs_open(const char* path, uint32_t flags, uint32_t mode, vfs_file_t** file_out) {
    if (!path || !file_out) {
        return -OR_EINVAL;
    }
    
    // Security check
    process_t* current = scheduler_get_current_process();
    if (current && !security_check_syscall_allowed(SYS_OPEN, current->pid)) {
        return -OR_EPERM;
    }
    
    // Find free file descriptor
    int fd = -1;
    spinlock_lock(&vfs_lock);
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (open_files[i] == NULL) {
            fd = i;
            break;
        }
    }
    spinlock_unlock(&vfs_lock);
    
    if (fd == -1) {
        return -OR_EMFILE; // Too many open files
    }
    
    // Path resolution and dentry lookup
    vfs_inode_t* inode = NULL;
    vfs_dentry_t* dentry = NULL;
    
    // Resolve path to inode
    int resolve_result = vfs_path_resolve(path, &inode, &dentry);
    if (resolve_result != OR_OK) {
        kerror("Failed to resolve path '%s': %d", path, resolve_result);
        return resolve_result;
    }
    
    // Check if file exists and permissions
    if (!inode) {
        if (!(flags & O_CREAT)) {
            return -OR_ENOENT; // File doesn't exist and O_CREAT not specified
        }
        
        // Create new file
        inode = vfs_create_inode(path, mode, current ? current->pid : 0);
        if (!inode) {
            return -OR_ENOMEM;
        }
        
        // Create dentry for new file
        dentry = vfs_create_dentry(path, inode);
        if (!dentry) {
            inode_put(inode);
            return -OR_ENOMEM;
        }
    } else {
        // File exists, check permissions
        if (!vfs_check_permissions(inode, flags, current)) {
            inode_put(inode);
            return -OR_EACCES;
        }
        
        // Check if trying to open directory for writing
        if (S_ISDIR(inode->i_mode) && (flags & (O_WRONLY | O_RDWR))) {
            inode_put(inode);
            return -OR_EISDIR;
        }
    }
    
    // Create file structure with resolved inode
    
    vfs_file_t* file = (vfs_file_t*)kmalloc(sizeof(vfs_file_t));
    if (!file) {
        inode_put(inode);
        return -OR_ENOMEM;
    }
    
    memset(file, 0, sizeof(vfs_file_t));
    atomic_store(&file->f_count, 1);
    spinlock_init(&file->f_lock);
    file->f_flags = flags;
    file->f_mode = mode;
    file->f_pos = 0;
    file->f_inode = inode;
    file->f_dentry = dentry;
    
    // Set file operations based on inode type
    if (S_ISREG(inode->i_mode)) {
        file->f_op = &vfs_file_ops;
    } else if (S_ISDIR(inode->i_mode)) {
        file->f_op = &vfs_dir_ops;
    } else if (S_ISCHR(inode->i_mode) || S_ISBLK(inode->i_mode)) {
        file->f_op = &vfs_device_ops;
    }
    
    if (current) {
        file->f_owner_pid = current->pid;
        // Create capability for file access
        uint64_t cap_rights = CAP_READ;
        if (flags & (O_WRONLY | O_RDWR)) cap_rights |= CAP_WRITE;
        file->f_capability = cap_create(CAP_TYPE_FILE, (uint64_t)file, cap_rights, current->pid);
    }
    
    // Register in open files table
    spinlock_lock(&vfs_lock);
    open_files[fd] = file;
    spinlock_unlock(&vfs_lock);
    
    atomic_fetch_add(&vfs_stats_open_files, 1);
    
    *file_out = file;
    kdebug("Opened file: %s (fd=%d, flags=0x%x)", path, fd, flags);
    
    return fd;
}

int vfs_close(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILES) {
        return -OR_EBADF;
    }
    
    spinlock_lock(&vfs_lock);
    vfs_file_t* file = open_files[fd];
    if (!file) {
        spinlock_unlock(&vfs_lock);
        return -OR_EBADF;
    }
    
    open_files[fd] = NULL;
    spinlock_unlock(&vfs_lock);
    
    // Security check
    process_t* current = scheduler_get_current_process();
    if (current && file->f_owner_pid != current->pid) {
        if (!cap_check_rights(file->f_capability, CAP_READ, current->pid)) {
            return -OR_EPERM;
        }
    }
    
    // Decrease reference count
    if (atomic_fetch_sub(&file->f_count, 1) == 1) {
        // Last reference, cleanup
        if (file->f_op && file->f_op->close && file->f_inode) {
            file->f_op->close(file->f_inode, file);
        }
        
        if (file->f_inode) {
            inode_put(file->f_inode);
        }
        
        if (file->f_capability) {
            cap_destroy(file->f_capability);
        }
        
        kfree(file);
        atomic_fetch_sub(&vfs_stats_open_files, 1);
    }
    
    kdebug("Closed file descriptor %d", fd);
    return OR_OK;
}

ssize_t vfs_read(int fd, void* buffer, size_t count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !buffer) {
        return -OR_EINVAL;
    }
    
    spinlock_lock(&vfs_lock);
    vfs_file_t* file = open_files[fd];
    spinlock_unlock(&vfs_lock);
    
    if (!file) {
        return -OR_EBADF;
    }
    
    // Security check
    process_t* current = scheduler_get_current_process();
    if (current && file->f_owner_pid != current->pid) {
        if (!cap_check_rights(file->f_capability, CAP_READ, current->pid)) {
            return -OR_EPERM;
        }
    }
    
    // Check file flags
    if ((file->f_flags & O_WRONLY)) {
        return -OR_EBADF; // File not open for reading
    }
    
    // Call file system specific read
    if (file->f_op && file->f_op->read) {
        return file->f_op->read(file, (char*)buffer, count, &file->f_pos);
    }
    
    // Default: not implemented
    return -OR_ENOSYS;
}

ssize_t vfs_write(int fd, const void* buffer, size_t count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !buffer) {
        return -OR_EINVAL;
    }
    
    spinlock_lock(&vfs_lock);
    vfs_file_t* file = open_files[fd];
    spinlock_unlock(&vfs_lock);
    
    if (!file) {
        return -OR_EBADF;
    }
    
    // Security check
    process_t* current = scheduler_get_current_process();
    if (current && file->f_owner_pid != current->pid) {
        if (!cap_check_rights(file->f_capability, CAP_WRITE, current->pid)) {
            return -OR_EPERM;
        }
    }
    
    // Check file flags
    if ((file->f_flags & O_RDONLY)) {
        return -OR_EBADF; // File not open for writing
    }
    
    // Call file system specific write
    if (file->f_op && file->f_op->write) {
        return file->f_op->write(file, (const char*)buffer, count, &file->f_pos);
    }
    
    // Default: not implemented
    return -OR_ENOSYS;
}

uint64_t vfs_seek(int fd, uint64_t offset, int whence) {
    if (fd < 0 || fd >= MAX_OPEN_FILES) {
        return -OR_EBADF;
    }
    
    spinlock_lock(&vfs_lock);
    vfs_file_t* file = open_files[fd];
    spinlock_unlock(&vfs_lock);
    
    if (!file) {
        return -OR_EBADF;
    }
    
    // Security check
    process_t* current = scheduler_get_current_process();
    if (current && file->f_owner_pid != current->pid) {
        if (!cap_check_rights(file->f_capability, CAP_READ, current->pid)) {
            return -OR_EPERM;
        }
    }
    
    // Call file system specific seek
    if (file->f_op && file->f_op->seek) {
        int result = file->f_op->seek(file, offset, whence);
        if (result == OR_OK) {
            return file->f_pos;
        }
        return result;
    }
    
    // Default implementation
    switch (whence) {
        case 0: // SEEK_SET
            file->f_pos = offset;
            break;
        case 1: // SEEK_CUR
            file->f_pos += offset;
            break;
        case 2: // SEEK_END
            if (file->f_inode) {
                file->f_pos = file->f_inode->i_size + offset;
            } else {
                return -OR_EINVAL;
            }
            break;
        default:
            return -OR_EINVAL;
    }
    
    return file->f_pos;
}

// ========================================
// VFS STATISTICS AND DEBUG
// ========================================

void vfs_get_stats(uint64_t* open_files_count, uint64_t* cache_hits, 
                   uint64_t* cache_misses, uint64_t* mounted_fs) {
    if (open_files_count) {
        *open_files_count = atomic_load(&vfs_stats_open_files);
    }
    
    if (cache_hits) {
        *cache_hits = atomic_load(&vfs_stats_cache_hits);
    }
    
    if (cache_misses) {
        *cache_misses = atomic_load(&vfs_stats_cache_misses);
    }
    
    if (mounted_fs) {
        *mounted_fs = 0;
        spinlock_lock(&vfs_lock);
        vfs_mount_t* mount = mounts;
        while (mount) {
            (*mounted_fs)++;
            mount = mount->mnt_next;
        }
        spinlock_unlock(&vfs_lock);
    }
}

void vfs_debug_info(void) {
    uint64_t open_files, cache_hits, cache_misses, mounted_fs;
    
    vfs_get_stats(&open_files, &cache_hits, &cache_misses, &mounted_fs);
    
    kinfo("VFS Debug Information:");
    kinfo("  Open files: %llu/%d", (unsigned long long)open_files, MAX_OPEN_FILES);
    kinfo("  Cache hits: %llu", (unsigned long long)cache_hits);
    kinfo("  Cache misses: %llu", (unsigned long long)cache_misses);
    kinfo("  Cache hit ratio: %llu%%", 
          cache_hits + cache_misses > 0 ? 
          (cache_hits * 100) / (cache_hits + cache_misses) : 0);
    kinfo("  Mounted file systems: %llu", (unsigned long long)mounted_fs);
    
    // List registered file systems
    kinfo("  Registered file systems:");
    spinlock_lock(&vfs_lock);
    vfs_filesystem_t* fs = filesystems;
    while (fs) {
        kinfo("    - %s", fs->name);
        fs = fs->next;
    }
    spinlock_unlock(&vfs_lock);
}
