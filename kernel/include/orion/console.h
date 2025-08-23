/*
 * Orion Operating System - Console Functions Header
 *
 * Console output functions for the Orion kernel.
 * Provides basic console operations without libc dependency.
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_CONSOLE_H
#define ORION_CONSOLE_H

#include <orion/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Console output functions
void console_putchar(char c);
void console_puts(const char *str);
void console_printf(const char *fmt, ...);

// Console control
void console_clear(void);
void console_set_color(uint8_t foreground, uint8_t background);
void console_get_cursor_position(uint16_t *row, uint16_t *col);
void console_set_cursor_position(uint16_t row, uint16_t *col);

// Console colors
#define CONSOLE_COLOR_BLACK        0
#define CONSOLE_COLOR_BLUE         1
#define CONSOLE_COLOR_GREEN        2
#define CONSOLE_COLOR_CYAN         3
#define CONSOLE_COLOR_RED          4
#define CONSOLE_COLOR_MAGENTA      5
#define CONSOLE_COLOR_BROWN        6
#define CONSOLE_COLOR_LIGHT_GRAY   7
#define CONSOLE_COLOR_DARK_GRAY    8
#define CONSOLE_COLOR_LIGHT_BLUE   9
#define CONSOLE_COLOR_LIGHT_GREEN  10
#define CONSOLE_COLOR_LIGHT_CYAN   11
#define CONSOLE_COLOR_LIGHT_RED    12
#define CONSOLE_COLOR_LIGHT_MAGENTA 13
#define CONSOLE_COLOR_YELLOW       14
#define CONSOLE_COLOR_WHITE        15

#ifdef __cplusplus
}
#endif

#endif // ORION_CONSOLE_H
