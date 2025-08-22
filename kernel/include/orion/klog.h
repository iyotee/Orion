/*
 * Orion Operating System - Kernel Logging System
 *
 * Comprehensive logging system for kernel debugging and monitoring
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_KLOG_H
#define ORION_KLOG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <orion/types.h>

    // ========================================
    // LOG CATEGORIES
    // ========================================

    typedef enum
    {
        KLOG_CAT_KERNEL = 0, // Kernel core operations
        KLOG_CAT_MEMORY,     // Memory management
        KLOG_CAT_PROCESS,    // Process/thread management
        KLOG_CAT_SCHEDULER,  // Scheduler operations
        KLOG_CAT_DRIVER,     // Device drivers
        KLOG_CAT_FILESYSTEM, // File system operations
        KLOG_CAT_NETWORK,    // Network operations
        KLOG_CAT_SECURITY,   // Security events
        KLOG_CAT_IPC,        // Inter-process communication
        KLOG_CAT_TIMER,      // Timer and scheduling
        KLOG_CAT_INTERRUPT,  // Interrupt handling
        KLOG_CAT_ARCH,       // Architecture-specific
        KLOG_CAT_MAX
    } klog_category_t;

    // ========================================
    // LOG LEVELS
    // ========================================

    typedef enum
    {
        KLOG_EMERGENCY = 0, // System unusable
        KLOG_ALERT,         // Action must be taken immediately
        KLOG_CRITICAL,      // Critical conditions
        KLOG_ERROR,         // Error conditions
        KLOG_WARNING,       // Warning conditions
        KLOG_NOTICE,        // Normal but significant condition
        KLOG_INFO,          // Informational messages
        KLOG_DEBUG          // Debug-level messages
    } klog_level_t;

    // ========================================
    // LOG BUFFER STRUCTURE
    // ========================================

    typedef struct
    {
        char *buffer;    // Log buffer
        size_t size;     // Buffer size
        size_t head;     // Write position
        size_t tail;     // Read position
        size_t count;    // Number of entries
        spinlock_t lock; // Protection lock
        bool overflow;   // Buffer overflow flag
    } klog_buffer_t;

    // ========================================
    // LOG ENTRY STRUCTURE
    // ========================================

    typedef struct
    {
        uint64_t timestamp;       // Timestamp in nanoseconds
        uint32_t cpu_id;          // CPU that generated the log
        uint32_t pid;             // Process ID (if applicable)
        uint32_t tid;             // Thread ID (if applicable)
        klog_category_t category; // Log category
        klog_level_t level;       // Log level
        char message[256];        // Log message
    } klog_entry_t;

    // ========================================
    // CORE LOGGING FUNCTIONS
    // ========================================

    /**
     * Initialize the kernel logging system
     * @return 0 on success, negative error code on failure
     */
    int klog_init(void);

    /**
     * Set global log level
     * @param level New log level
     */
    void klog_set_level(klog_level_t level);

    /**
     * Get current global log level
     * @return Current log level
     */
    klog_level_t klog_get_level(void);

    /**
     * Add a new log buffer for a specific category
     * @param category Category for the buffer
     * @param buffer_size Size of the buffer in bytes
     * @return Buffer ID on success, negative error code on failure
     */
    int klog_add_buffer(klog_category_t category, size_t buffer_size);

    /**
     * Remove a log buffer
     * @param buffer_id ID of the buffer to remove
     * @return 0 on success, negative error code on failure
     */
    void klog_remove_buffer(int buffer_id);

    /**
     * Flush all log buffers
     */
    void klog_flush(void);

    // ========================================
    // LOGGING MACROS
    // ========================================

#define KLOG_EMERG(cat, fmt, ...) \
    klog_write(cat, KLOG_EMERGENCY, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define KLOG_ALERT(cat, fmt, ...) \
    klog_write(cat, KLOG_ALERT, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define KLOG_CRITICAL(cat, fmt, ...) \
    klog_write(cat, KLOG_CRITICAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define KLOG_ERROR(cat, fmt, ...) \
    klog_write(cat, KLOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define KLOG_WARNING(cat, fmt, ...) \
    klog_write(cat, KLOG_WARNING, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define KLOG_NOTICE(cat, fmt, ...) \
    klog_write(cat, KLOG_NOTICE, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define KLOG_INFO(cat, fmt, ...) \
    klog_write(cat, KLOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define KLOG_DEBUG(cat, fmt, ...) \
    klog_write(cat, KLOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

    // ========================================
    // CONVENIENCE FUNCTIONS
    // ========================================

    void klog_emergency(const char *fmt, ...);
    void klog_alert(const char *fmt, ...);
    void klog_critical(const char *fmt, ...);
    void klog_err(const char *fmt, ...);
    void klog_warning(const char *fmt, ...);
    void klog_notice(const char *fmt, ...);
    void klog_info(const char *fmt, ...);
    void klog_debug(const char *fmt, ...);

    // Category-specific logging
    void klog_info(klog_category_t category, const char *fmt, ...);
    void klog_warning(klog_category_t category, const char *fmt, ...);
    void klog_err(klog_category_t category, const char *fmt, ...);
    void klog_debug(klog_category_t category, const char *fmt, ...);

    // Driver logging
    void driver_log(const char *fmt, ...);
    void driver_error(const char *fmt, ...);
    void driver_warning(const char *fmt, ...);
    void driver_info(const char *fmt, ...);

    // ========================================
    // INTERNAL FUNCTIONS
    // ========================================

    int klog_write(klog_category_t category, klog_level_t level,
                   const char *file, int line, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // ORION_KLOG_H
