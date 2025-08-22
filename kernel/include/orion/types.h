/*
 * Orion Operating System - Complete Type Definitions
 *
 * Core type definitions, macros, and utility functions for the Orion kernel.
 * Provides all fundamental types and macros needed throughout the system.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_TYPES_H
#define ORION_TYPES_H

// Fundamental types for Orion
// C11 freestanding - no dependency on libc

// Standard integer types
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

// Size types
typedef uint64_t size_t;
typedef int64_t ssize_t;
typedef uint64_t uintptr_t;
typedef int64_t ptrdiff_t;

// Boolean types
typedef uint8_t bool;
#define true 1
#define false 0

// NULL
#define NULL ((void *)0)

// Capabilities - 64-bit non-forgeable identifiers
typedef uint64_t or_cap_t;

// Error codes
typedef enum
{
    OR_OK = 0,
    OR_EINVAL,    // Invalid argument
    OR_ENOMEM,    // Insufficient memory
    OR_ENOSYS,    // System call not implemented
    OR_EPERM,     // Permission denied
    OR_ENOENT,    // Object not found
    OR_EAGAIN,    // Resource temporarily unavailable
    OR_ETIMEDOUT, // Timeout
    OR_EINTR,     // Interrupted
    OR_EBUSY,     // Resource busy
    OR_EFAULT,    // Invalid address
    OR_ESRCH,     // Process not found
} or_error_t;

// Compiler attributes - MSVC/GCC/Clang compatible
#ifdef _MSC_VER
#define PACKED
#define ALIGNED(x) __declspec(align(x))
#define NORETURN __declspec(noreturn)
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#define PURE
#define CONST
#else
#define PACKED __attribute__((packed))
#define ALIGNED(x) __attribute__((aligned(x)))
#define NORETURN __attribute__((noreturn))
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define PURE __attribute__((pure))
#define CONST __attribute__((const))
#endif

// Utility macros
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ROUND_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))
#define ROUND_DOWN(x, align) ((x) & ~((align) - 1))
#define IS_ALIGNED(x, align) (((x) & ((align) - 1)) == 0)

// Memory pages
#define PAGE_SIZE 4096ULL
#define PAGE_MASK (PAGE_SIZE - 1)
#define PAGE_SHIFT 12

// Assertions - active even in release for kernel
#define VERIFY(cond)                                                        \
    do                                                                      \
    {                                                                       \
        if (UNLIKELY(!(cond)))                                              \
        {                                                                   \
            panic("VERIFY failed: %s at %s:%d", #cond, __FILE__, __LINE__); \
        }                                                                   \
    } while (0)

#ifdef DEBUG
#define DEBUG_ASSERT(cond) VERIFY(cond)
#else
#define DEBUG_ASSERT(cond) ((void)0)
#endif

// Forward declarations
NORETURN void panic(const char *fmt, ...);

// ====================================
// BOOT PROTOCOL STRUCTURES
// ====================================

// Boot information structure (forward declaration)
struct orion_boot_info;

// System information structure
typedef struct
{
    char kernel_version[32];
    uint64_t boot_time;
    uint64_t current_time;
    uint64_t total_memory;
    uint64_t free_memory;
    uint32_t cpu_count;
    uint32_t process_count;
    uint32_t thread_count;
} or_system_info_t;

// Virtual memory mapping structure
typedef struct
{
    uint64_t addr;    // Virtual address (0 = auto)
    size_t length;    // Size
    uint32_t prot;    // Protection (READ|WRITE|EXEC)
    uint32_t flags;   // Flags (PRIVATE|SHARED|FIXED)
    or_cap_t backing; // Backing object capability (0 = anonymous)
    uint64_t offset;  // Offset in backing object
} or_vm_map_t;

// IPC message structures
typedef struct
{
    or_cap_t target_port; // Destination port
    const void *data;     // Data to send
    size_t data_size;     // Size of data
    or_cap_t *caps;       // Capabilities to transfer
    size_t caps_count;    // Number of capabilities
    uint64_t timeout_ns;  // Timeout in nanoseconds
} or_msg_send_t;

typedef struct
{
    or_cap_t source_port; // Source port (output)
    void *buffer;         // Buffer for receiving
    size_t buffer_size;   // Size of buffer
    or_cap_t *caps;       // Buffer for received capabilities
    size_t caps_max;      // Maximum number of capabilities
    size_t caps_received; // Number of capabilities received (output)
    uint64_t timeout_ns;  // Timeout
} or_msg_recv_t;

// ====================================
// MISSING STANDARD TYPES
// ====================================

// Time structures
struct timespec
{
    uint64_t tv_sec;  // Seconds
    uint64_t tv_nsec; // Nanoseconds
};

struct itimerspec
{
    struct timespec it_interval; // Timer interval
    struct timespec it_value;    // Timer value
};

// Signal event structure
struct sigevent
{
    int sigev_notify; // Notification method
    int sigev_signo;  // Signal number
    union
    {
        int sival_int;   // Integer value
        void *sival_ptr; // Pointer value
    } sigev_value;

    void (*sigev_notify_function)(void *); // Simplified for now
    void *sigev_notify_attributes;
};

// ====================================
// CONTAINER_OF MACRO
// ====================================

#define container_of(ptr, type, member) ({             \
    const typeof(((type *)0)->member) *__mptr = (ptr); \
    (type *)((char *)__mptr - offsetof(type, member)); \
})

#define offsetof(type, member) ((size_t)&((type *)0)->member)

// ====================================
// UTILITY MACROS
// ====================================

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define ALIGN_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(x, align) ((x) & ~((align) - 1))

// IS_ALIGNED already defined line 90
// ARRAY_SIZE already defined line 85

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

// ====================================
// SPINLOCK DEFINITION
// ====================================

typedef struct
{
    volatile uint64_t locked; // Lock state
    uint32_t cpu;             // Owning CPU
    uint32_t padding;         // Padding for alignment
} spinlock_t;

#define SPINLOCK_INIT {.locked = 0, .cpu = 0, .padding = 0}

// Forward declaration
void arch_pause(void);

// Spinlock functions (inline implementations)
static inline void spinlock_init(spinlock_t *lock)
{
    if (lock)
    {
        lock->locked = 0;
        lock->cpu = 0;
    }
}

static inline void spin_lock(spinlock_t *lock)
{
    if (!lock)
        return;

    while (__atomic_test_and_set(&lock->locked, __ATOMIC_ACQUIRE))
    {
        arch_pause(); // CPU pause instruction for better performance
    }
}

static inline void spin_unlock(spinlock_t *lock)
{
    if (!lock)
        return;
    __atomic_clear(&lock->locked, __ATOMIC_RELEASE);
}

static inline bool spin_trylock(spinlock_t *lock)
{
    if (!lock)
        return false;

    return !__atomic_test_and_set(&lock->locked, __ATOMIC_ACQUIRE);
}

// Aliases for compatibility
#define spinlock_lock spin_lock
#define spinlock_unlock spin_unlock

// ====================================
// CODES D'ERREUR ORION
// ====================================
#define OR_OK 0
#define OR_EINVAL -1
#define OR_ENOMEM -2
#define OR_EPERM -3
#define OR_ENOENT -4
#define OR_EEXIST -5
#define OR_EBUSY -6
#define OR_ETIMEDOUT -7
#define OR_EAGAIN -8
#define OR_EFAULT -9
#define OR_ENOSYS -10
#define OR_EMFILE -11
#define OR_EBADF -12

// ====================================
// SYSTEM CONSTANTS
// ====================================

#include <orion/limits.h>

// ====================================
// ATOMIC TYPES AND OPERATIONS
// ====================================

// Atomic types for lock-free programming
typedef struct
{
    volatile uint64_t value;
} atomic64_t;

typedef struct
{
    volatile uint32_t value;
} atomic32_t;

// Atomic initialization macros
#define ATOMIC_VAR_INIT(val) {.value = (val)}

// Atomic memory ordering constants
#define __ATOMIC_RELAXED 0
#define __ATOMIC_CONSUME 1
#define __ATOMIC_ACQUIRE 2
#define __ATOMIC_RELEASE 3
#define __ATOMIC_ACQ_REL 4
#define __ATOMIC_SEQ_CST 5

// Atomic operations (simplified for kernel use)
static inline uint64_t atomic_load_64(const atomic64_t *ptr)
{
    return ptr->value;
}

static inline void atomic_store_64(atomic64_t *ptr, uint64_t val)
{
    ptr->value = val;
}

static inline uint64_t atomic_fetch_add_64(atomic64_t *ptr, uint64_t val)
{
    return __atomic_fetch_add(&ptr->value, val, __ATOMIC_SEQ_CST);
}

static inline uint32_t atomic_load_32(const atomic32_t *ptr)
{
    return ptr->value;
}

static inline void atomic_store_32(atomic32_t *ptr, uint32_t val)
{
    ptr->value = val;
}

static inline uint32_t atomic_fetch_add_32(atomic32_t *ptr, uint32_t val)
{
    return __atomic_fetch_add(&ptr->value, val, __ATOMIC_SEQ_CST);
}

static inline bool atomic_compare_exchange_64(atomic64_t *ptr, uint64_t *expected, uint64_t desired)
{
    return __atomic_compare_exchange_n(&ptr->value, expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

static inline bool atomic_compare_exchange_32(atomic32_t *ptr, uint32_t *expected, uint32_t desired)
{
    return __atomic_compare_exchange_n(&ptr->value, expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

// Additional atomic operations
static inline uint64_t atomic_load(const atomic64_t *ptr)
{
    return atomic_load_64(ptr);
}

static inline void atomic_store(atomic64_t *ptr, uint64_t val)
{
    atomic_store_64(ptr, val);
}

static inline uint64_t atomic_fetch_add(atomic64_t *ptr, uint64_t val)
{
    return atomic_fetch_add_64(ptr, val);
}

static inline uint64_t atomic_fetch_sub(atomic64_t *ptr, uint64_t val)
{
    return __atomic_fetch_sub(&ptr->value, val, __ATOMIC_SEQ_CST);
}

static inline uint64_t atomic_fetch_or(atomic64_t *ptr, uint64_t val)
{
    return __atomic_fetch_or(&ptr->value, val, __ATOMIC_SEQ_CST);
}

static inline uint64_t atomic_fetch_and(atomic64_t *ptr, uint64_t val)
{
    return __atomic_fetch_and(&ptr->value, val, __ATOMIC_SEQ_CST);
}

static inline bool atomic_compare_exchange_strong(atomic64_t *ptr, uint64_t *expected, uint64_t desired)
{
    return atomic_compare_exchange_64(ptr, expected, desired);
}

// ====================================
// ARCH FUNCTIONS
// ====================================

// Timing
uint64_t arch_get_timestamp(void);
void arch_halt(void);
void arch_pause(void);

// CPU Management
uint32_t arch_get_current_cpu(void);
// arch_interrupt_init() declared in kernel.h
void arch_timer_init(void);
void arch_disable_interrupts(void);

// Syscall interface
void syscall_entry(void);

// Memory translation (if not already defined in arch.h)
#ifndef PHYS_TO_VIRT
#define PHYS_TO_VIRT(addr) ((addr) + 0xFFFF800000000000ULL)
#endif

// Memory
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
int snprintf(char *str, size_t size, const char *format, ...);
// va_* macros for freestanding environment
typedef char *va_list;
#define va_start(v, l) ((v) = (char *)&(l) + sizeof(l))
#define va_end(v) ((v) = (char *)0)
#define va_arg(v, l) (*(l *)(((v) += sizeof(l)) - sizeof(l)))

int kvprintf(const char *format, va_list args);

#endif // ORION_TYPES_H
