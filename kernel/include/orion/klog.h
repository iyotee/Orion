/*
 * Orion Operating System - Kernel Logging Header
 *
 * Advanced kernel logging system declarations.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_KLOG_H
#define ORION_KLOG_H

#include <orion/types.h>
#include <orion/spinlock.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Log levels
#define KLOG_EMERG 0
#define KLOG_ALERT 1
#define KLOG_CRIT 2
#define KLOG_ERR 3
#define KLOG_WARNING 4
#define KLOG_NOTICE 5
#define KLOG_INFO 6
#define KLOG_DEBUG 7

// Log categories
#define KLOG_CAT_KERNEL (1 << 0)
#define KLOG_CAT_DRIVER (1 << 1)
#define KLOG_CAT_SECURITY (1 << 2)
#define KLOG_CAT_NETWORK (1 << 3)
#define KLOG_CAT_FS (1 << 4)

    // Log entry structure
    typedef struct
    {
        uint64_t timestamp;
        uint8_t level;
        uint16_t category;
        uint64_t source_pid;
        uint64_t source_tid;
        char message[256];
        size_t message_len;
    } klog_entry_t;

    // Log buffer structure
    typedef struct
    {
        klog_entry_t entries[64];
        size_t head;
        size_t tail;
        size_t count;
        uint16_t category;
        bool active;
        spinlock_t lock;
    } klog_buffer_t;

    // Log context structure
    typedef struct
    {
        klog_buffer_t buffers[8];
        size_t buffer_count;
        uint8_t current_level;
        bool initialized;
        spinlock_t global_lock;
    } klog_context_t;

    // Logging functions
    int klog_init(void);
    int klog_write(uint8_t level, uint16_t category, const char *format, ...);
    int klog_write_va(uint8_t level, uint16_t category, const char *format, va_list args);
    int klog_flush(void);
    int klog_set_level(uint8_t level);
    uint8_t klog_get_level(void);
    int klog_add_buffer(uint16_t category);
    int klog_remove_buffer(size_t buffer_index);
    ssize_t klog_read_buffer(size_t buffer_index, klog_entry_t *entries, size_t max_entries);
    void klog_emergency(const char *format, ...);

// Buffer size constant
#define KLOG_BUFFER_SIZE (64 * sizeof(klog_entry_t))
#define KLOG_MAX_BUFFERS 8

#ifdef __cplusplus
}
#endif

#endif // ORION_KLOG_H
