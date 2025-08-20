/*
 * Orion Operating System - RAM File System (RAMFS)
 *
 * Simple in-memory file system for demonstration and initial file operations.
 * Stores all files and directories in kernel memory.
 *
 * Developed by Jérémy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/kernel.h>
#include <orion/types.h>
#include <orion/mm.h>

// Import VFS structures (normally would be in vfs.h)
// This is a simplified version for demonstration

// ========================================
// RAMFS CONSTANTS
// ========================================

#define RAMFS_MAGIC         0x52414D46  // "RAMF"
#define RAMFS_MAX_NAME_LEN  255
#define RAMFS_MAX_FILES     1024
#define RAMFS_BLOCK_SIZE    4096

// ========================================
// RAMFS DATA STRUCTURES
// ========================================

// RAMFS-specific inode data
typedef struct ramfs_inode {
    uint64_t ino;                       // Inode number
    uint32_t mode;                      // File mode
    uint64_t size;                      // File size
    uint64_t blocks;                    // Number of blocks
    uint64_t atime, mtime, ctime;       // Timestamps
    
    // File data storage
    void** data_blocks;                 // Array of data block pointers
    uint32_t nr_blocks;                 // Number of allocated blocks
    uint32_t max_blocks;                // Maximum blocks allocated
    
    // Directory entries (if directory)
    struct ramfs_dirent* entries;       // Directory entries
    uint32_t nr_entries;                // Number of entries
    uint32_t max_entries;               // Maximum entries allocated
    
    spinlock_t lock;                    // Inode lock
} ramfs_inode_t;

// Directory entry
typedef struct ramfs_dirent {
    uint64_t ino;                       // Inode number
    char name[RAMFS_MAX_NAME_LEN + 1];  // File name
    uint8_t type;                       // File type
} ramfs_dirent_t;

// RAMFS superblock data
typedef struct ramfs_sb_info {
    uint64_t total_inodes;              // Total inodes
    uint64_t free_inodes;               // Free inodes
    uint64_t total_blocks;              // Total blocks (virtual)
    uint64_t free_blocks;               // Free blocks (virtual)
    uint64_t next_ino;                  // Next inode number
    ramfs_inode_t* inodes[RAMFS_MAX_FILES]; // Inode table
    spinlock_t lock;                    // Superblock lock
} ramfs_sb_info_t;

// ========================================
// RAMFS OPERATIONS
// ========================================

// Create a new RAMFS inode
static ramfs_inode_t* ramfs_create_inode(ramfs_sb_info_t* sbi, uint32_t mode) {
    ramfs_inode_t* inode = (ramfs_inode_t*)kmalloc(sizeof(ramfs_inode_t));
    if (!inode) {
        return NULL;
    }
    
    memset(inode, 0, sizeof(ramfs_inode_t));
    
    spinlock_lock(&sbi->lock);
    inode->ino = sbi->next_ino++;
    sbi->free_inodes--;
    spinlock_unlock(&sbi->lock);
    
    inode->mode = mode;
    inode->size = 0;
    inode->blocks = 0;
    
    uint64_t now = arch_get_timestamp();
    inode->atime = inode->mtime = inode->ctime = now;
    
    spinlock_init(&inode->lock);
    
    // Store in inode table
    if (inode->ino < RAMFS_MAX_FILES) {
        sbi->inodes[inode->ino] = inode;
    }
    
    kdebug("RAMFS: Created inode %llu (mode=0x%x)", 
           (unsigned long long)inode->ino, mode);
    
    return inode;
}

// Free a RAMFS inode
static void ramfs_free_inode(ramfs_sb_info_t* sbi, ramfs_inode_t* inode) {
    if (!inode) {
        return;
    }
    
    spinlock_lock(&inode->lock);
    
    // Free data blocks
    if (inode->data_blocks) {
        for (uint32_t i = 0; i < inode->nr_blocks; i++) {
            if (inode->data_blocks[i]) {
                kfree(inode->data_blocks[i]);
            }
        }
        kfree(inode->data_blocks);
    }
    
    // Free directory entries
    if (inode->entries) {
        kfree(inode->entries);
    }
    
    // Remove from inode table
    if (inode->ino < RAMFS_MAX_FILES) {
        sbi->inodes[inode->ino] = NULL;
    }
    
    spinlock_unlock(&inode->lock);
    
    spinlock_lock(&sbi->lock);
    sbi->free_inodes++;
    spinlock_unlock(&sbi->lock);
    
    kdebug("RAMFS: Freed inode %llu", (unsigned long long)inode->ino);
    kfree(inode);
}

// Read data from RAMFS file
static ssize_t ramfs_read(void* file, char* buffer, size_t count, uint64_t* offset) {
    // This would normally get the ramfs_inode from the VFS file structure
    // For simplicity, we'll create a placeholder implementation
    
    kdebug("RAMFS: Read request %llu bytes at offset %llu", 
           (unsigned long long)count, (unsigned long long)*offset);
    
    // Simulate reading "Hello, Orion!\n" from any file
    const char* data = "Hello, Orion!\n";
    size_t data_len = strlen(data);
    
    if (*offset >= data_len) {
        return 0; // EOF
    }
    
    size_t available = data_len - *offset;
    size_t to_read = (count < available) ? count : available;
    
    memcpy(buffer, data + *offset, to_read);
    *offset += to_read;
    
    return (ssize_t)to_read;
}

// Write data to RAMFS file
static ssize_t ramfs_write(void* file, const char* buffer, size_t count, uint64_t* offset) {
    kdebug("RAMFS: Write request %llu bytes at offset %llu", 
           (unsigned long long)count, (unsigned long long)*offset);
    
    // For now, just log what would be written and return success
    kdebug("RAMFS: Would write: %.*s", (int)count, buffer);
    
    *offset += count;
    return (ssize_t)count;
}

// RAMFS file operations
static struct {
    int (*open)(void* inode, void* file);
    int (*close)(void* inode, void* file);
    ssize_t (*read)(void* file, char* buffer, size_t count, uint64_t* offset);
    ssize_t (*write)(void* file, const char* buffer, size_t count, uint64_t* offset);
    int (*seek)(void* file, uint64_t offset, int whence);
} ramfs_file_operations = {
    .open = NULL,
    .close = NULL,
    .read = ramfs_read,
    .write = ramfs_write,
    .seek = NULL,
};

// Mount RAMFS
static void* ramfs_mount(void* fs, uint32_t flags, const char* dev_name, void* data) {
    (void)fs; (void)flags; (void)dev_name; (void)data;
    
    kinfo("RAMFS: Mounting RAMFS");
    
    // Allocate superblock info
    ramfs_sb_info_t* sbi = (ramfs_sb_info_t*)kmalloc(sizeof(ramfs_sb_info_t));
    if (!sbi) {
        return NULL;
    }
    
    memset(sbi, 0, sizeof(ramfs_sb_info_t));
    sbi->total_inodes = RAMFS_MAX_FILES;
    sbi->free_inodes = RAMFS_MAX_FILES;
    sbi->total_blocks = 0xFFFFFFFF; // Unlimited (virtual)
    sbi->free_blocks = 0xFFFFFFFF;
    sbi->next_ino = 1;
    spinlock_init(&sbi->lock);
    
    // Create root directory
    ramfs_inode_t* root_inode = ramfs_create_inode(sbi, 0755 | 0040000); // S_IFDIR
    if (!root_inode) {
        kfree(sbi);
        return NULL;
    }
    
    kinfo("RAMFS: Mounted successfully with %llu inodes", 
          (unsigned long long)sbi->total_inodes);
    
    return sbi;
}

// Unmount RAMFS
static void ramfs_unmount(void* sb) {
    ramfs_sb_info_t* sbi = (ramfs_sb_info_t*)sb;
    if (!sbi) {
        return;
    }
    
    kinfo("RAMFS: Unmounting RAMFS");
    
    // Free all inodes
    for (uint32_t i = 0; i < RAMFS_MAX_FILES; i++) {
        if (sbi->inodes[i]) {
            ramfs_free_inode(sbi, sbi->inodes[i]);
        }
    }
    
    kfree(sbi);
    kinfo("RAMFS: Unmounted successfully");
}

// RAMFS file system structure
static struct {
    char name[32];
    uint32_t fs_flags;
    void* (*mount)(void* fs, uint32_t flags, const char* dev_name, void* data);
    void (*unmount)(void* sb);
    void* next;
} ramfs_type = {
    .name = "ramfs",
    .fs_flags = 0,
    .mount = ramfs_mount,
    .unmount = ramfs_unmount,
    .next = NULL,
};

// ========================================
// RAMFS INITIALIZATION
// ========================================

void ramfs_init(void) {
    kinfo("Initializing RAMFS");
    
    // Register with VFS (simplified)
    // vfs_register_filesystem(&ramfs_type);
    
    kinfo("RAMFS initialized successfully");
    kinfo("  Maximum files: %d", RAMFS_MAX_FILES);
    kinfo("  Block size: %d bytes", RAMFS_BLOCK_SIZE);
    kinfo("  Magic number: 0x%08X", RAMFS_MAGIC);
}

// Get RAMFS statistics
void ramfs_get_stats(uint64_t* total_files, uint64_t* used_files, 
                     uint64_t* total_memory, uint64_t* used_memory) {
    // This would normally iterate through mounted RAMFS instances
    // For now, provide placeholder values
    
    if (total_files) {
        *total_files = RAMFS_MAX_FILES;
    }
    
    if (used_files) {
        *used_files = 10; // Placeholder
    }
    
    if (total_memory) {
        *total_memory = 64 * 1024 * 1024; // 64MB virtual
    }
    
    if (used_memory) {
        *used_memory = 1024 * 1024; // 1MB placeholder
    }
}

void ramfs_debug_info(void) {
    uint64_t total_files, used_files, total_memory, used_memory;
    
    ramfs_get_stats(&total_files, &used_files, &total_memory, &used_memory);
    
    kinfo("RAMFS Debug Information:");
    kinfo("  Files: %llu/%llu", (unsigned long long)used_files, 
          (unsigned long long)total_files);
    kinfo("  Memory usage: %llu KB / %llu KB", 
          (unsigned long long)(used_memory / 1024),
          (unsigned long long)(total_memory / 1024));
    kinfo("  Block size: %d bytes", RAMFS_BLOCK_SIZE);
    kinfo("  File system type: %s", ramfs_type.name);
}
