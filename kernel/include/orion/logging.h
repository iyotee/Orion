/*
 * Orion Operating System - Logging Functions Header
 *
 * Unified logging interface for the Orion kernel.
 * Provides consistent logging across all kernel components.
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_LOGGING_H
#define ORION_LOGGING_H

#include <orion/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // Log levels
    typedef enum
    {
        LOG_LEVEL_DEBUG = 0,
        LOG_LEVEL_INFO,
        LOG_LEVEL_WARNING,
        LOG_LEVEL_ERROR,
        LOG_LEVEL_PANIC
    } log_level_t;

    // Basic logging functions
    void kinfo(const char *fmt, ...);
    void kdebug(const char *fmt, ...);
    void kwarning(const char *fmt, ...);
    void kerror(const char *fmt, ...);
    void kpanic(const char *fmt, ...);

    // Formatted logging
    void kprintf(const char *fmt, ...);

    // Console output
    void console_putchar(char c);
    void console_puts(const char *str);

#ifdef __cplusplus
}
#endif

#endif // ORION_LOGGING_H
