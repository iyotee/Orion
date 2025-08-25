/*
 * ORION OS - MIPS Stubs
 *
 * Stub functions and linking support for MIPS architecture
 * Provides essential system call and library function stubs
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// SYSTEM CALL STUBS
// ============================================================================

int mips_syscall_exit(int status)
{
    printf("MIPS: System call exit(%d)\n", status);
    // In a real implementation, this would terminate the process
    return 0;
}

int mips_syscall_read(int fd, void* buf, size_t count)
{
    printf("MIPS: System call read(%d, %p, %zu)\n", fd, buf, count);
    // In a real implementation, this would read from file descriptor
    return -1; // Not implemented
}

int mips_syscall_write(int fd, const void* buf, size_t count)
{
    printf("MIPS: System call write(%d, %p, %zu)\n", fd, buf, count);
    // In a real implementation, this would write to file descriptor
    return -1; // Not implemented
}

int mips_syscall_open(const char* pathname, int flags, mode_t mode)
{
    printf("MIPS: System call open(%s, %d, %d)\n", pathname, flags, mode);
    // In a real implementation, this would open a file
    return -1; // Not implemented
}

int mips_syscall_close(int fd)
{
    printf("MIPS: System call close(%d)\n", fd);
    // In a real implementation, this would close a file descriptor
    return -1; // Not implemented
}

int mips_syscall_fork(void)
{
    printf("MIPS: System call fork()\n");
    // In a real implementation, this would create a new process
    return -1; // Not implemented
}

int mips_syscall_execve(const char* filename, char* const argv[], char* const envp[])
{
    printf("MIPS: System call execve(%s, %p, %p)\n", filename, argv, envp);
    // In a real implementation, this would execute a new program
    return -1; // Not implemented
}

int mips_syscall_wait(int* status)
{
    printf("MIPS: System call wait(%p)\n", status);
    // In a real implementation, this would wait for child process
    return -1; // Not implemented
}

int mips_syscall_kill(pid_t pid, int sig)
{
    printf("MIPS: System call kill(%d, %d)\n", pid, sig);
    // In a real implementation, this would send a signal
    return -1; // Not implemented
}

pid_t mips_syscall_getpid(void)
{
    printf("MIPS: System call getpid()\n");
    // In a real implementation, this would return process ID
    return 1; // Default process ID
}

unsigned int mips_syscall_sleep(unsigned int seconds)
{
    printf("MIPS: System call sleep(%u)\n", seconds);
    // In a real implementation, this would sleep for specified time
    return 0; // Sleep completed
}

time_t mips_syscall_time(time_t* tloc)
{
    printf("MIPS: System call time(%p)\n", tloc);
    // In a real implementation, this would return current time
    time_t current_time = 0; // Default time
    if (tloc) {
        *tloc = current_time;
    }
    return current_time;
}

void* mips_syscall_brk(void* addr)
{
    printf("MIPS: System call brk(%p)\n", addr);
    // In a real implementation, this would change program break
    return NULL; // Not implemented
}

void* mips_syscall_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    printf("MIPS: System call mmap(%p, %zu, %d, %d, %d, %ld)\n", addr, length, prot, flags, fd, offset);
    // In a real implementation, this would map memory
    return MAP_FAILED; // Not implemented
}

int mips_syscall_munmap(void* addr, size_t length)
{
    printf("MIPS: System call munmap(%p, %zu)\n", addr, length);
    // In a real implementation, this would unmap memory
    return -1; // Not implemented
}

int mips_syscall_socket(int domain, int type, int protocol)
{
    printf("MIPS: System call socket(%d, %d, %d)\n", domain, type, protocol);
    // In a real implementation, this would create a socket
    return -1; // Not implemented
}

int mips_syscall_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    printf("MIPS: System call bind(%d, %p, %u)\n", sockfd, addr, addrlen);
    // In a real implementation, this would bind a socket
    return -1; // Not implemented
}

int mips_syscall_listen(int sockfd, int backlog)
{
    printf("MIPS: System call listen(%d, %d)\n", sockfd, backlog);
    // In a real implementation, this would listen on a socket
    return -1; // Not implemented
}

int mips_syscall_accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
    printf("MIPS: System call accept(%d, %p, %p)\n", sockfd, addr, addrlen);
    // In a real implementation, this would accept a connection
    return -1; // Not implemented
}

int mips_syscall_connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    printf("MIPS: System call connect(%d, %p, %u)\n", sockfd, addr, addrlen);
    // In a real implementation, this would connect a socket
    return -1; // Not implemented
}

ssize_t mips_syscall_send(int sockfd, const void* buf, size_t len, int flags)
{
    printf("MIPS: System call send(%d, %p, %zu, %d)\n", sockfd, buf, len, flags);
    // In a real implementation, this would send data
    return -1; // Not implemented
}

ssize_t mips_syscall_recv(int sockfd, void* buf, size_t len, int flags)
{
    printf("MIPS: System call recv(%d, %p, %zu, %d)\n", sockfd, buf, len, flags);
    // In a real implementation, this would receive data
    return -1; // Not implemented
}

// ============================================================================
// LIBRARY FUNCTION STUBS
// ============================================================================

void* mips_malloc(size_t size)
{
    printf("MIPS: malloc(%zu)\n", size);
    // In a real implementation, this would allocate memory
    return NULL; // Not implemented
}

void mips_free(void* ptr)
{
    printf("MIPS: free(%p)\n", ptr);
    // In a real implementation, this would free memory
}

void* mips_calloc(size_t nmemb, size_t size)
{
    printf("MIPS: calloc(%zu, %zu)\n", nmemb, size);
    // In a real implementation, this would allocate zeroed memory
    return NULL; // Not implemented
}

void* mips_realloc(void* ptr, size_t size)
{
    printf("MIPS: realloc(%p, %zu)\n", ptr, size);
    // In a real implementation, this would reallocate memory
    return NULL; // Not implemented
}

int mips_memcmp(const void* s1, const void* s2, size_t n)
{
    printf("MIPS: memcmp(%p, %p, %zu)\n", s1, s2, n);
    // In a real implementation, this would compare memory
    return 0; // Equal
}

void* mips_memcpy(void* dest, const void* src, size_t n)
{
    printf("MIPS: memcpy(%p, %p, %zu)\n", dest, src, n);
    // In a real implementation, this would copy memory
    return dest;
}

void* mips_memmove(void* dest, const void* src, size_t n)
{
    printf("MIPS: memmove(%p, %p, %zu)\n", dest, src, n);
    // In a real implementation, this would move memory
    return dest;
}

void* mips_memset(void* s, int c, size_t n)
{
    printf("MIPS: memset(%p, %d, %zu)\n", s, c, n);
    // In a real implementation, this would set memory
    return s;
}

size_t mips_strlen(const char* s)
{
    printf("MIPS: strlen(%s)\n", s);
    // In a real implementation, this would return string length
    return 0; // Empty string
}

char* mips_strcpy(char* dest, const char* src)
{
    printf("MIPS: strcpy(%p, %s)\n", dest, src);
    // In a real implementation, this would copy string
    return dest;
}

char* mips_strncpy(char* dest, const char* src, size_t n)
{
    printf("MIPS: strncpy(%p, %s, %zu)\n", dest, src, n);
    // In a real implementation, this would copy string with limit
    return dest;
}

int mips_strcmp(const char* s1, const char* s2)
{
    printf("MIPS: strcmp(%s, %s)\n", s1, s2);
    // In a real implementation, this would compare strings
    return 0; // Equal
}

int mips_strncmp(const char* s1, const char* s2, size_t n)
{
    printf("MIPS: strncmp(%s, %s, %zu)\n", s1, s2, n);
    // In a real implementation, this would compare strings with limit
    return 0; // Equal
}

char* mips_strcat(char* dest, const char* src)
{
    printf("MIPS: strcat(%p, %s)\n", dest, src);
    // In a real implementation, this would concatenate strings
    return dest;
}

char* mips_strncat(char* dest, const char* src, size_t n)
{
    printf("MIPS: strncat(%p, %s, %zu)\n", dest, src, n);
    // In a real implementation, this would concatenate strings with limit
    return dest;
}

char* mips_strchr(const char* s, int c)
{
    printf("MIPS: strchr(%s, %d)\n", s, c);
    // In a real implementation, this would find character in string
    return NULL; // Not found
}

char* mips_strrchr(const char* s, int c)
{
    printf("MIPS: strrchr(%s, %d)\n", s, c);
    // In a real implementation, this would find last character in string
    return NULL; // Not found
}

char* mips_strstr(const char* haystack, const char* needle)
{
    printf("MIPS: strstr(%s, %s)\n", haystack, needle);
    // In a real implementation, this would find substring
    return NULL; // Not found
}

// ============================================================================
// MATH FUNCTION STUBS
// ============================================================================

double mips_sin(double x)
{
    printf("MIPS: sin(%f)\n", x);
    // In a real implementation, this would calculate sine
    return 0.0; // Not implemented
}

double mips_cos(double x)
{
    printf("MIPS: cos(%f)\n", x);
    // In a real implementation, this would calculate cosine
    return 0.0; // Not implemented
}

double mips_tan(double x)
{
    printf("MIPS: tan(%f)\n", x);
    // In a real implementation, this would calculate tangent
    return 0.0; // Not implemented
}

double mips_sqrt(double x)
{
    printf("MIPS: sqrt(%f)\n", x);
    // In a real implementation, this would calculate square root
    return 0.0; // Not implemented
}

double mips_pow(double x, double y)
{
    printf("MIPS: pow(%f, %f)\n", x, y);
    // In a real implementation, this would calculate power
    return 0.0; // Not implemented
}

double mips_log(double x)
{
    printf("MIPS: log(%f)\n", x);
    // In a real implementation, this would calculate natural logarithm
    return 0.0; // Not implemented
}

double mips_log10(double x)
{
    printf("MIPS: log10(%f)\n", x);
    // In a real implementation, this would calculate base-10 logarithm
    return 0.0; // Not implemented
}

double mips_exp(double x)
{
    printf("MIPS: exp(%f)\n", x);
    // In a real implementation, this would calculate exponential
    return 0.0; // Not implemented
}

double mips_floor(double x)
{
    printf("MIPS: floor(%f)\n", x);
    // In a real implementation, this would calculate floor
    return 0.0; // Not implemented
}

double mips_ceil(double x)
{
    printf("MIPS: ceil(%f)\n", x);
    // In a real implementation, this would calculate ceiling
    return 0.0; // Not implemented
}

// ============================================================================
// UTILITY FUNCTION STUBS
// ============================================================================

void mips_abort(void)
{
    printf("MIPS: abort()\n");
    // In a real implementation, this would abort the program
    while (1) {
        // Infinite loop
    }
}

void mips_exit(int status)
{
    printf("MIPS: exit(%d)\n", status);
    // In a real implementation, this would exit the program
    while (1) {
        // Infinite loop
    }
}

int mips_atexit(void (*func)(void))
{
    printf("MIPS: atexit(%p)\n", func);
    // In a real implementation, this would register exit function
    return 0; // Success
}

void mips_qsort(void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*))
{
    printf("MIPS: qsort(%p, %zu, %zu, %p)\n", base, nmemb, size, compar);
    // In a real implementation, this would sort an array
}

void* mips_bsearch(const void* key, const void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*))
{
    printf("MIPS: bsearch(%p, %p, %zu, %zu, %p)\n", key, base, nmemb, size, compar);
    // In a real implementation, this would search in sorted array
    return NULL; // Not found
}

int mips_abs(int j)
{
    printf("MIPS: abs(%d)\n", j);
    // In a real implementation, this would return absolute value
    return (j < 0) ? -j : j;
}

long mips_labs(long j)
{
    printf("MIPS: labs(%ld)\n", j);
    // In a real implementation, this would return absolute value
    return (j < 0) ? -j : j;
}

long long mips_llabs(long long j)
{
    printf("MIPS: llabs(%lld)\n", j);
    // In a real implementation, this would return absolute value
    return (j < 0) ? -j : j;
}

int mips_rand(void)
{
    printf("MIPS: rand()\n");
    // In a real implementation, this would return random number
    return 0; // Not implemented
}

void mips_srand(unsigned int seed)
{
    printf("MIPS: srand(%u)\n", seed);
    // In a real implementation, this would seed random number generator
}
