/*
 * Orion Operating System - Console Interface
 *
 * Console output functions for kernel logging and user interaction.
 * Provides basic text output capabilities.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_CONSOLE_H
#define ORION_CONSOLE_H

#include <orion/types.h>
#include <stdarg.h>

// Console functions
int console_printf(const char *format, ...);
int console_vprintf(const char *format, va_list args);
void console_putchar(char c);
void console_clear(void);

// Legacy kprintf function (for compatibility)
#define kprintf console_printf

#endif // ORION_CONSOLE_H
