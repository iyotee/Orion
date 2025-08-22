/*
 * Orion Operating System - String and Memory Functions
 * 
 * Freestanding implementations of essential string and memory functions.
 * These functions don't depend on any external libraries.
 * 
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 * 
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/types.h>

// Simple kvprintf implementation (stub)
int kvprintf(const char *format, va_list args) {
    // Stub implementation - just return success
    // In a real implementation, this would format and output the string
    (void)format; // Suppress unused parameter warning
    (void)args;   // Suppress unused parameter warning
    return 0;
}

/**
 * Set memory to a specific value
 */
void *memset(void *s, int c, size_t n) {
    unsigned char *p = (unsigned char *)s;
    unsigned char val = (unsigned char)c;
    
    while (n--) {
        *p++ = val;
    }
    
    return s;
}

/**
 * Copy memory block
 */
void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    
    while (n--) {
        *d++ = *s++;
    }
    
    return dest;
}

/**
 * Compare memory blocks
 */
int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    
    return 0;
}

/**
 * Calculate string length
 */
size_t strlen(const char *s) {
    size_t len = 0;
    while (*s++) {
        len++;
    }
    return len;
}

/**
 * Copy string
 */
char *strcpy(char *dest, const char *src) {
    char *original_dest = dest;
    while ((*dest++ = *src++)) {
        // Loop body is empty
    }
    return original_dest;
}

/**
 * Compare strings
 */
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

/**
 * Compare strings with length limit
 */
int strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    
    if (n == 0) {
        return 0;
    }
    
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

/**
 * Complete snprintf implementation
 */
int snprintf(char *str, size_t size, const char *format, ...) {
    if (!str || !format || size == 0) {
        return 0;
    }
    
    va_list args;
    va_start(args, format);
    
    size_t written = 0;
    const char* p = format;
    
    while (*p && written < size - 1) {
        if (*p == '%' && *(p + 1)) {
            p++; // Skip %
            switch (*p) {
                case 's': {
                    const char* s = va_arg(args, const char*);
                    if (!s) s = "(null)";
                    while (*s && written < size - 1) {
                        str[written++] = *s++;
                    }
                    break;
                }
                case 'd': {
                    int num = va_arg(args, int);
                    char buffer[32];
                    int len = itoa(num, buffer, 10);
                    for (int i = 0; i < len && written < size - 1; i++) {
                        str[written++] = buffer[i];
                    }
                    break;
                }
                case 'u': {
                    unsigned int num = va_arg(args, unsigned int);
                    char buffer[32];
                    int len = utoa(num, buffer, 10);
                    for (int i = 0; i < len && written < size - 1; i++) {
                        str[written++] = buffer[i];
                    }
                    break;
                }
                case 'x': {
                    unsigned int num = va_arg(args, unsigned int);
                    char buffer[32];
                    int len = utoa(num, buffer, 16);
                    for (int i = 0; i < len && written < size - 1; i++) {
                        str[written++] = buffer[i];
                    }
                    break;
                }
                case 'X': {
                    unsigned int num = va_arg(args, unsigned int);
                    char buffer[32];
                    int len = utoa(num, buffer, 16);
                    for (int i = 0; i < len && written < size - 1; i++) {
                        char c = buffer[i];
                        if (c >= 'a' && c <= 'f') {
                            c = c - 'a' + 'A';
                        }
                        str[written++] = c;
                    }
                    break;
                }
                case 'p': {
                    void* ptr = va_arg(args, void*);
                    if (written < size - 3) {
                        str[written++] = '0';
                        str[written++] = 'x';
                    }
                    char buffer[32];
                    int len = utoa((uintptr_t)ptr, buffer, 16);
                    for (int i = 0; i < len && written < size - 1; i++) {
                        str[written++] = buffer[i];
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    str[written++] = c;
                    break;
                }
                case '%': {
                    str[written++] = '%';
                    break;
                }
                default: {
                    if (written < size - 2) {
                        str[written++] = '%';
                        str[written++] = *p;
                    }
                    break;
                }
            }
        } else {
            str[written++] = *p;
        }
        p++;
    }
    
    str[written] = '\0';
    va_end(args);
    
    return written;
}
