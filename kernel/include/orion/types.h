/*
 * Orion Operating System - Complete Type Definitions
 *
 * Core type definitions, macros, and utility functions for the Orion kernel.
 * Provides all fundamental types and macros needed throughout the system.
 *
 * Developed by Jérémy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_TYPES_H
#define ORION_TYPES_H

// Types fondamentaux pour Orion
// C11 freestanding - pas de dépendance à la libc

// Types entiers standards
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

// Types tailles
typedef uint64_t size_t;
typedef int64_t ssize_t;
typedef uint64_t uintptr_t;
typedef int64_t ptrdiff_t;

// Types bool
typedef uint8_t bool;
#define true 1
#define false 0

// NULL
#define NULL ((void *)0)

// Capabilities - identifiants 64-bit non-forgeables
typedef uint64_t or_cap_t;

// Codes d'erreur
typedef enum
{
    OR_OK = 0,
    OR_EINVAL,    // Argument invalide
    OR_ENOMEM,    // Mémoire insuffisante
    OR_ENOSYS,    // Appel système non implémenté
    OR_EPERM,     // Permission refusée
    OR_ENOENT,    // Objet non trouvé
    OR_EAGAIN,    // Ressource temporairement indisponible
    OR_ETIMEDOUT, // Timeout
    OR_EINTR,     // Interrompu
    OR_EBUSY,     // Ressource occupée
    OR_EFAULT,    // Adresse invalide
} or_error_t;

// Attributs compilateur - compatible MSVC/GCC/Clang
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

// Macros utilitaires
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ROUND_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))
#define ROUND_DOWN(x, align) ((x) & ~((align) - 1))
#define IS_ALIGNED(x, align) (((x) & ((align) - 1)) == 0)

// Pages mémoire
#define PAGE_SIZE 4096ULL
#define PAGE_MASK (PAGE_SIZE - 1)
#define PAGE_SHIFT 12

// Assertions - actives même en release pour le noyau
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
    void (*sigev_notify_function)(union sigval);
    void *sigev_notify_attributes;
};

union sigval
{
    int sival_int;   // Integer value
    void *sival_ptr; // Pointer value
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

#define IS_ALIGNED(x, align) (((x) & ((align) - 1)) == 0)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

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

// Spinlock functions
void spinlock_init(spinlock_t *lock);
void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);
bool spin_trylock(spinlock_t *lock);

// ====================================
// ATOMIC TYPES AND OPERATIONS
// ====================================

// Atomic types for lock-free programming
typedef struct {
    volatile uint64_t value;
} atomic64_t;

typedef struct {
    volatile uint32_t value;
} atomic32_t;

// Atomic initialization macros
#define ATOMIC_VAR_INIT(val) { .value = (val) }

// Atomic memory ordering constants
#define __ATOMIC_RELAXED 0
#define __ATOMIC_CONSUME 1
#define __ATOMIC_ACQUIRE 2
#define __ATOMIC_RELEASE 3
#define __ATOMIC_ACQ_REL 4
#define __ATOMIC_SEQ_CST 5

// Atomic operations (simplified for kernel use)
static inline uint64_t atomic_load_64(const atomic64_t *ptr) {
    return ptr->value;
}

static inline void atomic_store_64(atomic64_t *ptr, uint64_t val) {
    ptr->value = val;
}

static inline uint64_t atomic_fetch_add_64(atomic64_t *ptr, uint64_t val) {
    return __atomic_fetch_add(&ptr->value, val, __ATOMIC_SEQ_CST);
}

static inline uint32_t atomic_load_32(const atomic32_t *ptr) {
    return ptr->value;
}

static inline void atomic_store_32(atomic32_t *ptr, uint32_t val) {
    ptr->value = val;
}

static inline uint32_t atomic_fetch_add_32(atomic32_t *ptr, uint32_t val) {
    return __atomic_fetch_add(&ptr->value, val, __ATOMIC_SEQ_CST);
}

static inline bool atomic_compare_exchange_64(atomic64_t *ptr, uint64_t *expected, uint64_t desired) {
    return __atomic_compare_exchange_n(&ptr->value, expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

static inline bool atomic_compare_exchange_32(atomic32_t *ptr, uint32_t *expected, uint32_t desired) {
    return __atomic_compare_exchange_n(&ptr->value, expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

// ====================================
// ARCH FUNCTIONS
// ====================================

// Timing
uint64_t arch_get_timestamp(void);
void arch_halt(void);
void arch_pause(void);

// Memory
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
int snprintf(char *str, size_t size, const char *format, ...);

#endif // ORION_TYPES_H
