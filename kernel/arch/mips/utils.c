/*
 * ORION OS - MIPS Utilities
 *
 * System utilities for MIPS architecture
 * Handles string operations, memory utilities, mathematical functions, and system utilities
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// ============================================================================
// STRING OPERATIONS
// ============================================================================

size_t mips_strlen(const char *str)
{
    if (!str)
    {
        return 0;
    }

    size_t len = 0;
    while (str[len] != '\0')
    {
        len++;
    }

    return len;
}

char *mips_strcpy(char *dest, const char *src)
{
    if (!dest || !src)
    {
        return dest;
    }

    char *d = dest;
    while (*src)
    {
        *d++ = *src++;
    }
    *d = '\0';

    return dest;
}

char *mips_strncpy(char *dest, const char *src, size_t n)
{
    if (!dest || !src)
    {
        return dest;
    }

    char *d = dest;
    size_t i;

    for (i = 0; i < n && src[i] != '\0'; i++)
    {
        d[i] = src[i];
    }

    for (; i < n; i++)
    {
        d[i] = '\0';
    }

    return dest;
}

int mips_strcmp(const char *str1, const char *str2)
{
    if (!str1 || !str2)
    {
        return str1 ? 1 : (str2 ? -1 : 0);
    }

    while (*str1 && *str2 && *str1 == *str2)
    {
        str1++;
        str2++;
    }

    return (int)(*str1 - *str2);
}

int mips_strncmp(const char *str1, const char *str2, size_t n)
{
    if (!str1 || !str2)
    {
        return str1 ? 1 : (str2 ? -1 : 0);
    }

    size_t i = 0;
    while (i < n && str1[i] && str2[i] && str1[i] == str2[i])
    {
        i++;
    }

    if (i == n)
    {
        return 0;
    }

    return (int)(str1[i] - str2[i]);
}

char *mips_strcat(char *dest, const char *src)
{
    if (!dest || !src)
    {
        return dest;
    }

    char *d = dest;
    while (*d)
    {
        d++;
    }

    while (*src)
    {
        *d++ = *src++;
    }
    *d = '\0';

    return dest;
}

char *mips_strncat(char *dest, const char *src, size_t n)
{
    if (!dest || !src)
    {
        return dest;
    }

    char *d = dest;
    while (*d)
    {
        d++;
    }

    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
    {
        d[i] = src[i];
    }
    d[i] = '\0';

    return dest;
}

char *mips_strchr(const char *str, int c)
{
    if (!str)
    {
        return NULL;
    }

    while (*str)
    {
        if (*str == (char)c)
        {
            return (char *)str;
        }
        str++;
    }

    if (c == '\0')
    {
        return (char *)str;
    }

    return NULL;
}

char *mips_strrchr(const char *str, int c)
{
    if (!str)
    {
        return NULL;
    }

    const char *last = NULL;

    while (*str)
    {
        if (*str == (char)c)
        {
            last = str;
        }
        str++;
    }

    if (c == '\0')
    {
        return (char *)str;
    }

    return (char *)last;
}

// ============================================================================
// MEMORY UTILITIES
// ============================================================================

void *mips_memset(void *ptr, int value, size_t num)
{
    if (!ptr)
    {
        return NULL;
    }

    unsigned char *p = (unsigned char *)ptr;
    unsigned char v = (unsigned char)value;

    for (size_t i = 0; i < num; i++)
    {
        p[i] = v;
    }

    return ptr;
}

void *mips_memcpy(void *dest, const void *src, size_t num)
{
    if (!dest || !src)
    {
        return dest;
    }

    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    for (size_t i = 0; i < num; i++)
    {
        d[i] = s[i];
    }

    return dest;
}

void *mips_memmove(void *dest, const void *src, size_t num)
{
    if (!dest || !src)
    {
        return dest;
    }

    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    if (d < s)
    {
        // Copy forward
        for (size_t i = 0; i < num; i++)
        {
            d[i] = s[i];
        }
    }
    else if (d > s)
    {
        // Copy backward to avoid overlap
        for (size_t i = num; i > 0; i--)
        {
            d[i - 1] = s[i - 1];
        }
    }

    return dest;
}

int mips_memcmp(const void *ptr1, const void *ptr2, size_t num)
{
    if (!ptr1 || !ptr2)
    {
        return ptr1 ? 1 : (ptr2 ? -1 : 0);
    }

    const unsigned char *p1 = (const unsigned char *)ptr1;
    const unsigned char *p2 = (const unsigned char *)ptr2;

    for (size_t i = 0; i < num; i++)
    {
        if (p1[i] != p2[i])
        {
            return (int)(p1[i] - p2[i]);
        }
    }

    return 0;
}

void *mips_memchr(const void *ptr, int value, size_t num)
{
    if (!ptr)
    {
        return NULL;
    }

    const unsigned char *p = (const unsigned char *)ptr;
    unsigned char v = (unsigned char)value;

    for (size_t i = 0; i < num; i++)
    {
        if (p[i] == v)
        {
            return (void *)(p + i);
        }
    }

    return NULL;
}

// ============================================================================
// MATHEMATICAL FUNCTIONS
// ============================================================================

int mips_abs(int x)
{
    return (x < 0) ? -x : x;
}

long mips_labs(long x)
{
    return (x < 0) ? -x : x;
}

long long mips_llabs(long long x)
{
    return (x < 0) ? -x : x;
}

int mips_min(int a, int b)
{
    return (a < b) ? a : b;
}

int mips_max(int a, int b)
{
    return (a > b) ? a : b;
}

unsigned int mips_minu(unsigned int a, unsigned int b)
{
    return (a < b) ? a : b;
}

unsigned int mips_maxu(unsigned int a, unsigned int b)
{
    return (a > b) ? a : b;
}

int mips_clamp(int value, int min, int max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

unsigned int mips_clampu(unsigned int value, unsigned int min, unsigned int max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

int mips_sign(int x)
{
    if (x > 0)
        return 1;
    if (x < 0)
        return -1;
    return 0;
}

bool mips_is_power_of_two(unsigned int x)
{
    return x != 0 && (x & (x - 1)) == 0;
}

unsigned int mips_next_power_of_two(unsigned int x)
{
    if (x == 0)
        return 1;

    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;

    return x;
}

unsigned int mips_log2(unsigned int x)
{
    if (x == 0)
        return 0;

    unsigned int log = 0;
    while (x >>= 1)
    {
        log++;
    }

    return log;
}

// ============================================================================
// BIT MANIPULATION
// ============================================================================

unsigned int mips_popcount(unsigned int x)
{
    unsigned int count = 0;
    while (x)
    {
        count += x & 1;
        x >>= 1;
    }
    return count;
}

unsigned int mips_leading_zeros(unsigned int x)
{
    if (x == 0)
        return 32;

    unsigned int count = 0;
    while ((x & 0x80000000) == 0)
    {
        count++;
        x <<= 1;
    }

    return count;
}

unsigned int mips_trailing_zeros(unsigned int x)
{
    if (x == 0)
        return 32;

    unsigned int count = 0;
    while ((x & 1) == 0)
    {
        count++;
        x >>= 1;
    }

    return count;
}

unsigned int mips_reverse_bits(unsigned int x)
{
    x = ((x & 0x55555555) << 1) | ((x & 0xAAAAAAAA) >> 1);
    x = ((x & 0x33333333) << 2) | ((x & 0xCCCCCCCC) >> 2);
    x = ((x & 0x0F0F0F0F) << 4) | ((x & 0xF0F0F0F0) >> 4);
    x = ((x & 0x00FF00FF) << 8) | ((x & 0xFF00FF00) >> 8);
    x = ((x & 0x0000FFFF) << 16) | ((x & 0xFFFF0000) >> 16);

    return x;
}

unsigned int mips_rotate_left(unsigned int x, unsigned int shift)
{
    shift &= 31;
    return (x << shift) | (x >> (32 - shift));
}

unsigned int mips_rotate_right(unsigned int x, unsigned int shift)
{
    shift &= 31;
    return (x >> shift) | (x << (32 - shift));
}

// ============================================================================
// FORMATTING AND PRINTING
// ============================================================================

int mips_sprintf(char *str, const char *format, ...)
{
    if (!str || !format)
    {
        return -1;
    }

    va_list args;
    va_start(args, format);

    int result = mips_vsprintf(str, format, args);

    va_end(args);
    return result;
}

int mips_vsprintf(char *str, const char *format, va_list args)
{
    if (!str || !format)
    {
        return -1;
    }

    char *ptr = str;
    const char *fmt = format;

    while (*fmt)
    {
        if (*fmt == '%')
        {
            fmt++;
            switch (*fmt)
            {
            case 'd':
            case 'i':
            {
                int value = va_arg(args, int);
                ptr += mips_int_to_string(ptr, value, 10);
                break;
            }
            case 'u':
            {
                unsigned int value = va_arg(args, unsigned int);
                ptr += mips_uint_to_string(ptr, value, 10);
                break;
            }
            case 'x':
            case 'X':
            {
                unsigned int value = va_arg(args, unsigned int);
                ptr += mips_uint_to_string(ptr, value, 16);
                break;
            }
            case 'o':
            {
                unsigned int value = va_arg(args, unsigned int);
                ptr += mips_uint_to_string(ptr, value, 8);
                break;
            }
            case 's':
            {
                const char *value = va_arg(args, const char *);
                if (value)
                {
                    ptr += mips_strcpy(ptr, value) - ptr;
                }
                break;
            }
            case 'c':
            {
                int value = va_arg(args, int);
                *ptr++ = (char)value;
                break;
            }
            case '%':
            {
                *ptr++ = '%';
                break;
            }
            default:
                *ptr++ = '%';
                *ptr++ = *fmt;
                break;
            }
        }
        else
        {
            *ptr++ = *fmt;
        }
        fmt++;
    }

    *ptr = '\0';
    return (int)(ptr - str);
}

int mips_int_to_string(char *str, int value, int base)
{
    if (!str)
    {
        return 0;
    }

    if (value == 0)
    {
        str[0] = '0';
        str[1] = '\0';
        return 1;
    }

    char buffer[32];
    int index = 0;
    bool negative = false;

    if (value < 0 && base == 10)
    {
        negative = true;
        value = -value;
    }

    while (value > 0)
    {
        int digit = value % base;
        buffer[index++] = (digit < 10) ? '0' + digit : 'a' + digit - 10;
        value /= base;
    }

    int str_index = 0;

    if (negative)
    {
        str[str_index++] = '-';
    }

    while (index > 0)
    {
        str[str_index++] = buffer[--index];
    }

    str[str_index] = '\0';
    return str_index;
}

int mips_uint_to_string(char *str, unsigned int value, int base)
{
    if (!str)
    {
        return 0;
    }

    if (value == 0)
    {
        str[0] = '0';
        str[1] = '\0';
        return 1;
    }

    char buffer[32];
    int index = 0;

    while (value > 0)
    {
        int digit = value % base;
        buffer[index++] = (digit < 10) ? '0' + digit : 'a' + digit - 10;
        value /= base;
    }

    int str_index = 0;

    while (index > 0)
    {
        str[str_index++] = buffer[--index];
    }

    str[str_index] = '\0';
    return str_index;
}

// ============================================================================
// SYSTEM UTILITIES
// ============================================================================

void mips_delay_cycles(unsigned int cycles)
{
    // Simple delay loop
    for (unsigned int i = 0; i < cycles; i++)
    {
        __asm__ volatile("nop");
    }
}

void mips_delay_microseconds(unsigned int microseconds)
{
    // Approximate delay based on CPU frequency
    unsigned int cycles = microseconds * (mips_cpu_get_frequency() / 1000000);
    mips_delay_cycles(cycles);
}

void mips_delay_milliseconds(unsigned int milliseconds)
{
    mips_delay_microseconds(milliseconds * 1000);
}

void mips_spin_lock(volatile int *lock)
{
    while (__sync_lock_test_and_set(lock, 1))
    {
        // Wait for lock to be released
        __asm__ volatile("pause");
    }
}

void mips_spin_unlock(volatile int *lock)
{
    __sync_lock_release(lock);
}

bool mips_try_lock(volatile int *lock)
{
    return __sync_lock_test_and_set(lock, 1) == 0;
}

void mips_memory_barrier(void)
{
    __asm__ volatile("sync" : : : "memory");
}

void mips_read_barrier(void)
{
    __asm__ volatile("sync" : : : "memory");
}

void mips_write_barrier(void)
{
    __asm__ volatile("sync" : : : "memory");
}

// ============================================================================
// DEBUGGING UTILITIES
// ============================================================================

void mips_debug_break(void)
{
    __asm__ volatile("break");
}

void mips_debug_print_register(const char *name, uint64_t value)
{
    printf("MIPS: %s = 0x%016llx (%lld)\n", name, value, (long long)value);
}

void mips_debug_print_memory(const void *ptr, size_t size)
{
    if (!ptr)
    {
        printf("MIPS: Invalid memory pointer\n");
        return;
    }

    const unsigned char *data = (const unsigned char *)ptr;

    printf("MIPS: Memory dump at 0x%p (%zu bytes):\n", ptr, size);

    for (size_t i = 0; i < size; i += 16)
    {
        printf("  %04zx: ", i);

        // Print hex values
        for (size_t j = 0; j < 16 && i + j < size; j++)
        {
            printf("%02x ", data[i + j]);
        }

        // Print ASCII representation
        printf("  ");
        for (size_t j = 0; j < 16 && i + j < size; j++)
        {
            unsigned char c = data[i + j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }

        printf("\n");
    }
}

void mips_debug_print_stack_trace(void)
{
    printf("MIPS: Stack trace not implemented\n");
}

void mips_debug_assert(bool condition, const char *message)
{
    if (!condition)
    {
        printf("MIPS: ASSERTION FAILED: %s\n", message);
        mips_debug_break();
    }
}
