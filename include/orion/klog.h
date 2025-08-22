/*
 * Orion Operating System - Kernel Logging System
 *
 * Centralized logging infrastructure for the kernel and drivers.
 * Provides efficient, reliable logging with multiple backends.
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

// Log levels (in order of severity)
#define KLOG_EMERG 0   // System is unusable
#define KLOG_ALERT 1   // Action must be taken immediately
#define KLOG_CRIT 2    // Critical conditions
#define KLOG_ERR 3     // Error conditions
#define KLOG_WARNING 4 // Warning conditions
#define KLOG_NOTICE 5  // Normal but significant condition
#define KLOG_INFO 6    // Informational
#define KLOG_DEBUG 7   // Debug-level messages

// Log categories
#define KLOG_CAT_KERNEL 0x0001   // Kernel core
#define KLOG_CAT_DRIVER 0x0002   // Device drivers
#define KLOG_CAT_MEMORY 0x0004   // Memory management
#define KLOG_CAT_PROCESS 0x0008  // Process management
#define KLOG_CAT_FS 0x0010       // File system
#define KLOG_CAT_NETWORK 0x0020  // Networking
#define KLOG_CAT_SECURITY 0x0040 // Security events
#define KLOG_CAT_HARDWARE 0x0080 // Hardware events

// Log buffer configuration
#define KLOG_BUFFER_SIZE (64 * 1024) // 64KB per buffer
#define KLOG_MAX_BUFFERS 8           // Maximum number of buffers
#define KLOG_MAX_MESSAGE_SIZE 512    // Maximum message size

    // Log entry structure
    typedef struct
    {
        uint64_t timestamp;                  // Timestamp in nanoseconds
        uint8_t level;                       // Log level
        uint16_t category;                   // Log category
        uint16_t source_pid;                 // Source process ID
        uint16_t source_tid;                 // Source thread ID
        uint16_t message_len;                // Message length
        char message[KLOG_MAX_MESSAGE_SIZE]; // Message content
    } klog_entry_t;

    // Log buffer structure
    typedef struct
    {
        klog_entry_t entries[KLOG_BUFFER_SIZE / sizeof(klog_entry_t)];
        size_t head;       // Write position
        size_t tail;       // Read position
        size_t count;      // Number of entries
        spinlock_t lock;   // Buffer lock
        uint16_t category; // Buffer category
        bool active;       // Buffer active flag
    } klog_buffer_t;

    // Kernel logger context
    typedef struct
    {
        klog_buffer_t buffers[KLOG_MAX_BUFFERS];
        size_t buffer_count;
        uint8_t current_level;
        bool initialized;
        spinlock_t global_lock;
    } klog_context_t;

    // Function prototypes

    /**
     * Initialize the kernel logging system
     * @return 0 on success, negative error code on failure
     */
    int klog_init(void);

    /**
     * Write a log message
     * @param level Log level
     * @param category Log category
     * @param format Message format string
     * @param ... Variable arguments
     * @return 0 on success, negative error code on failure
     */
    int klog_write(uint8_t level, uint16_t category, const char *format, ...);

    /**
     * Write a log message with va_list
     * @param level Log level
     * @param category Log category
     * @param format Message format string
     * @param args Variable arguments
     * @return 0 on success, negative error code on failure
     */
    int klog_write_va(uint8_t level, uint16_t category, const char *format, va_list args);

    /**
     * Flush all log buffers
     * @return 0 on success, negative error code on failure
     */
    int klog_flush(void);

    /**
     * Set the current log level
     * @param level New log level
     * @return 0 on success, negative error code on failure
     */
    int klog_set_level(uint8_t level);

    /**
     * Get the current log level
     * @return Current log level
     */
    uint8_t klog_get_level(void);

    /**
     * Add a new log buffer
     * @param category Buffer category
     * @return Buffer index on success, negative error code on failure
     */
    int klog_add_buffer(uint16_t category);

    /**
     * Remove a log buffer
     * @param buffer_index Index of buffer to remove
     * @return 0 on success, negative error code on failure
     */
    int klog_remove_buffer(size_t buffer_index);

    /**
     * Read log entries from a buffer
     * @param buffer_index Buffer index
     * @param entries Output array for entries
     * @param max_entries Maximum number of entries to read
     * @return Number of entries read, negative error code on failure
     */
    ssize_t klog_read_buffer(size_t buffer_index, klog_entry_t *entries, size_t max_entries);

    /**
     * Emergency logging (works even when system is unstable)
     * @param format Message format string
     * @param ... Variable arguments
     */
    void klog_emergency(const char *format, ...);

// Convenience macros for different log levels
#define klog_emerg(cat, fmt, ...) klog_write(KLOG_EMERG, cat, fmt, ##__VA_ARGS__)
#define klog_alert(cat, fmt, ...) klog_write(KLOG_ALERT, cat, fmt, ##__VA_ARGS__)
#define klog_crit(cat, fmt, ...) klog_write(KLOG_CRIT, cat, fmt, ##__VA_ARGS__)
#define klog_err(cat, fmt, ...) klog_write(KLOG_ERR, cat, fmt, ##__VA_ARGS__)
#define klog_warning(cat, fmt, ...) klog_write(KLOG_WARNING, cat, fmt, ##__VA_ARGS__)
#define klog_notice(cat, fmt, ...) klog_write(KLOG_NOTICE, cat, fmt, ##__VA_ARGS__)
#define klog_info(cat, fmt, ...) klog_write(KLOG_INFO, cat, fmt, ##__VA_ARGS__)
#define klog_debug(cat, fmt, ...) klog_write(KLOG_DEBUG, cat, fmt, ##__VA_ARGS__)

// Driver logging convenience macros
#define driver_log(fmt, ...) klog_info(KLOG_CAT_DRIVER, fmt, ##__VA_ARGS__)
#define driver_error(fmt, ...) klog_err(KLOG_CAT_DRIVER, fmt, ##__VA_ARGS__)
#define driver_warning(fmt, ...) klog_warning(KLOG_CAT_DRIVER, fmt, ##__VA_ARGS__)
#define driver_debug(fmt, ...) klog_debug(KLOG_CAT_DRIVER, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // ORION_KLOG_H
