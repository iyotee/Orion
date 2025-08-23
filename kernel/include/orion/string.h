/*
 * Orion Operating System - String Functions Header
 *
 * String manipulation functions for the Orion kernel.
 * Provides basic string operations without libc dependency.
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_STRING_H
#define ORION_STRING_H

#include <orion/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// String length
size_t strlen(const char *str);

// String comparison
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

// String copy
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);

// String concatenation
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);

// String search
char *strchr(const char *str, int c);
char *strrchr(const char *str, int c);
char *strstr(const char *haystack, const char *needle);

// Memory operations
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

// Number conversion
int itoa(int value, char *str, int base);
int utoa(unsigned int value, char *str, int base);

// Formatted output
int snprintf(char *str, size_t size, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif // ORION_STRING_H
