/*
 * Orion Operating System - String Functions
 *
 * String manipulation and memory functions for the kernel.
 * Provides basic string operations and memory utilities.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_STRING_H
#define ORION_STRING_H

#include <orion/types.h>

// String functions
size_t strlen(const char *str);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
char *strchr(const char *str, int c);
char *strrchr(const char *str, int c);

// Memory functions
void *memset(void *ptr, int value, size_t num);
void *memcpy(void *dest, const void *src, size_t num);
void *memmove(void *dest, const void *src, size_t num);
int memcmp(const void *ptr1, const void *ptr2, size_t num);

// Formatting functions
int snprintf(char *str, size_t size, const char *format, ...);
int vsnprintf(char *str, size_t size, const char *format, va_list args);

#endif // ORION_STRING_H

