/*
 * Orion Operating System - Kernel Logging System Implementation
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

#include <orion/klog.h>
#include <orion/string.h>
#include <orion/console.h>
#include <orion/time.h>
#include <orion/process.h>
#include <orion/mm.h>
#include <stdarg.h>

// Global kernel logger context
static klog_context_t g_klog_ctx = {0};

// Default log level (can be changed at runtime)
static uint8_t g_default_level = KLOG_INFO;

// Emergency logging buffer (works even when system is unstable)
static char g_emergency_buffer[1024];
static size_t g_emergency_head = 0;
static spinlock_t g_emergency_lock = SPINLOCK_INIT;

// Forward declarations
static int klog_write_to_buffer(klog_buffer_t *buffer, const klog_entry_t *entry);
static void klog_console_output(const klog_entry_t *entry);
static uint64_t klog_get_timestamp(void);
static void klog_format_timestamp(uint64_t timestamp, char *buffer, size_t size);

/**
 * Initialize the kernel logging system
 */
int klog_init(void)
{
    if (g_klog_ctx.initialized)
    {
        return 0; // Already initialized
    }

    // Initialize global lock
    spinlock_init(&g_klog_ctx.global_lock);

    // Initialize emergency logging lock
    spinlock_init(&g_emergency_lock);

    // Create default buffers
    int result = klog_add_buffer(KLOG_CAT_KERNEL);
    if (result < 0)
    {
        return result;
    }

    result = klog_add_buffer(KLOG_CAT_DRIVER);
    if (result < 0)
    {
        return result;
    }

    result = klog_add_buffer(KLOG_CAT_SECURITY);
    if (result < 0)
    {
        return result;
    }

    g_klog_ctx.current_level = g_default_level;
    g_klog_ctx.initialized = true;

    // Log initialization
    klog_info(KLOG_CAT_KERNEL, "Kernel logging system initialized");

    return 0;
}

/**
 * Write a log message
 */
int klog_write(uint8_t level, uint16_t category, const char *format, ...)
{
    if (!g_klog_ctx.initialized)
    {
        return -1; // System not initialized
    }

    if (level > g_klog_ctx.current_level)
    {
        return 0; // Log level too high
    }

    va_list args;
    va_start(args, format);
    int result = klog_write_va(level, category, format, args);
    va_end(args);

    return result;
}

/**
 * Write a log message with va_list
 */
int klog_write_va(uint8_t level, uint16_t category, const char *format, va_list args)
{
    if (!g_klog_ctx.initialized)
    {
        return -1;
    }

    if (level > g_klog_ctx.current_level)
    {
        return 0;
    }

    // Create log entry
    klog_entry_t entry = {0};
    entry.timestamp = klog_get_timestamp();
    entry.level = level;
    entry.category = category;

    // Get current process/thread info
    // TODO: Implement process_get_current_pid() and thread_get_current_tid()
    entry.source_pid = 0; // Placeholder
    entry.source_tid = 0; // Placeholder

    // Format message
    int msg_len = vsnprintf(entry.message, sizeof(entry.message), format, args);
    if (msg_len < 0 || msg_len >= sizeof(entry.message))
    {
        return -1;
    }
    entry.message_len = msg_len;

    // Write to appropriate buffers
    spinlock_acquire(&g_klog_ctx.global_lock);

    for (size_t i = 0; i < g_klog_ctx.buffer_count; i++)
    {
        if (g_klog_ctx.buffers[i].active &&
            (g_klog_ctx.buffers[i].category & category))
        {
            klog_write_to_buffer(&g_klog_ctx.buffers[i], &entry);
        }
    }

    spinlock_release(&g_klog_ctx.global_lock);

    // Output to console for important messages
    if (level <= KLOG_ERR)
    {
        klog_console_output(&entry);
    }

    return 0;
}

/**
 * Flush all log buffers
 */
int klog_flush(void)
{
    if (!g_klog_ctx.initialized)
    {
        return -1;
    }

    spinlock_acquire(&g_klog_ctx.global_lock);

    for (size_t i = 0; i < g_klog_ctx.buffer_count; i++)
    {
        if (g_klog_ctx.buffers[i].active)
        {
            spinlock_acquire(&g_klog_ctx.buffers[i].lock);
            g_klog_ctx.buffers[i].head = g_klog_ctx.buffers[i].tail;
            g_klog_ctx.buffers[i].count = 0;
            spinlock_release(&g_klog_ctx.buffers[i].lock);
        }
    }

    spinlock_release(&g_klog_ctx.global_lock);

    return 0;
}

/**
 * Set the current log level
 */
int klog_set_level(uint8_t level)
{
    if (level > KLOG_DEBUG)
    {
        return -1; // Invalid level
    }

    g_klog_ctx.current_level = level;
    return 0;
}

/**
 * Get the current log level
 */
uint8_t klog_get_level(void)
{
    return g_klog_ctx.current_level;
}

/**
 * Add a new log buffer
 */
int klog_add_buffer(uint16_t category)
{
    if (g_klog_ctx.buffer_count >= KLOG_MAX_BUFFERS)
    {
        return -1; // No more buffers available
    }

    size_t index = g_klog_ctx.buffer_count;
    klog_buffer_t *buffer = &g_klog_ctx.buffers[index];

    // Initialize buffer
    buffer->head = 0;
    buffer->tail = 0;
    buffer->count = 0;
    buffer->category = category;
    buffer->active = true;
    spinlock_init(&buffer->lock);

    g_klog_ctx.buffer_count++;

    return index;
}

/**
 * Remove a log buffer
 */
int klog_remove_buffer(size_t buffer_index)
{
    if (buffer_index >= g_klog_ctx.buffer_count)
    {
        return -1; // Invalid buffer index
    }

    klog_buffer_t *buffer = &g_klog_ctx.buffers[buffer_index];
    buffer->active = false;

    return 0;
}

/**
 * Read log entries from a buffer
 */
ssize_t klog_read_buffer(size_t buffer_index, klog_entry_t *entries, size_t max_entries)
{
    if (buffer_index >= g_klog_ctx.buffer_count || !entries || max_entries == 0)
    {
        return -1;
    }

    klog_buffer_t *buffer = &g_klog_ctx.buffers[buffer_index];
    if (!buffer->active)
    {
        return -1;
    }

    spinlock_acquire(&buffer->lock);

    size_t available = buffer->count;
    size_t to_read = (available < max_entries) ? available : max_entries;

    for (size_t i = 0; i < to_read; i++)
    {
        size_t index = (buffer->tail + i) % (KLOG_BUFFER_SIZE / sizeof(klog_entry_t));
        entries[i] = buffer->entries[index];
    }

    buffer->tail = (buffer->tail + to_read) % (KLOG_BUFFER_SIZE / sizeof(klog_entry_t));
    buffer->count -= to_read;

    spinlock_release(&buffer->lock);

    return to_read;
}

/**
 * Emergency logging (works even when system is unstable)
 */
void klog_emergency(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    spinlock_acquire(&g_emergency_lock);

    // Format message
    int msg_len = vsnprintf(g_emergency_buffer + g_emergency_head,
                            sizeof(g_emergency_buffer) - g_emergency_head,
                            format, args);

    if (msg_len > 0)
    {
        g_emergency_head += msg_len;
        if (g_emergency_head >= sizeof(g_emergency_buffer))
        {
            g_emergency_head = 0; // Wrap around
        }
    }

    spinlock_release(&g_emergency_lock);

    va_end(args);
}

// Internal helper functions

/**
 * Write entry to a specific buffer
 */
static int klog_write_to_buffer(klog_buffer_t *buffer, const klog_entry_t *entry)
{
    if (!buffer->active)
    {
        return -1;
    }

    spinlock_acquire(&buffer->lock);

    // Check if buffer is full
    if (buffer->count >= (KLOG_BUFFER_SIZE / sizeof(klog_entry_t)))
    {
        // Buffer full, advance tail to make room
        buffer->tail = (buffer->tail + 1) % (KLOG_BUFFER_SIZE / sizeof(klog_entry_t));
        buffer->count--;
    }

    // Write entry
    buffer->entries[buffer->head] = *entry;
    buffer->head = (buffer->head + 1) % (KLOG_BUFFER_SIZE / sizeof(klog_entry_t));
    buffer->count++;

    spinlock_release(&buffer->lock);

    return 0;
}

/**
 * Output log entry to console
 */
static void klog_console_output(const klog_entry_t *entry)
{
    char timestamp_str[32];
    klog_format_timestamp(entry->timestamp, timestamp_str, sizeof(timestamp_str));

    // Format level string
    const char *level_str = "UNKNOWN";
    switch (entry->level)
    {
    case KLOG_EMERG:
        level_str = "EMERG";
        break;
    case KLOG_ALERT:
        level_str = "ALERT";
        break;
    case KLOG_CRIT:
        level_str = "CRIT";
        break;
    case KLOG_ERR:
        level_str = "ERROR";
        break;
    case KLOG_WARNING:
        level_str = "WARN";
        break;
    case KLOG_NOTICE:
        level_str = "NOTICE";
        break;
    case KLOG_INFO:
        level_str = "INFO";
        break;
    case KLOG_DEBUG:
        level_str = "DEBUG";
        break;
    }

    // Output to console
    console_printf("[%s] %s: %s\n", timestamp_str, level_str, entry->message);
}

/**
 * Get current timestamp in nanoseconds
 */
static uint64_t klog_get_timestamp(void)
{
    // TODO: Implement proper high-resolution timer
    // For now, use a simple counter
    static uint64_t counter = 0;
    return ++counter;
}

/**
 * Format timestamp for display
 */
static void klog_format_timestamp(uint64_t timestamp, char *buffer, size_t size)
{
    // TODO: Implement proper timestamp formatting
    // For now, just show the raw value
    snprintf(buffer, size, "%llu", (unsigned long long)timestamp);
}
