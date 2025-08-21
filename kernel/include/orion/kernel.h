/*
 * Orion Operating System - Main Kernel Header
 *
 * Core kernel definitions, constants, and function declarations.
 * This header provides the main interface for kernel components.
 *
 * Developed by Jérémy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_KERNEL_H
#define ORION_KERNEL_H

#include <orion/types.h>
#include <orion/scheduler.h>
// orion_boot_info structure forward declaration
struct orion_boot_info;

// Orion kernel version information
#define ORION_VERSION_MAJOR 1
#define ORION_VERSION_MINOR 0
#define ORION_VERSION_PATCH 0
#define ORION_VERSION_STR "1.0.0-alpha"

// System limits and constraints
#define MAX_PROCESSES 65536
#define MAX_THREADS 1048576
#define MAX_CAPABILITIES 4294967296ULL

// Note: MAX_CPUS and thread_state_t are defined in scheduler.h

// Structure CPU info
typedef struct cpu_info
{
    uint32_t cpu_id;
    uint32_t apic_id;
    bool online;
    char vendor[16];
    char model[64];
} cpu_info_t;

// Structure système info
typedef struct system_info
{
    char kernel_version[32];
    uint64_t boot_time;
    uint64_t total_memory;
    uint64_t free_memory;
    uint32_t cpu_count;
    cpu_info_t cpus[MAX_CPUS];
} system_info_t;

// Fonctions principales du noyau
void kernel_main(struct orion_boot_info *boot_info);
void kernel_late_init(void);

// Gestion de la mémoire
void mm_init(void);
void *kmalloc(uint64_t size);
void kfree(void *ptr);

// Planificateur
void sched_yield(void);
void scheduler_init(void);

// Initialisation des sous-systèmes
void arch_interrupt_init(void);
void arch_timer_init(void);
void ipc_init(void);
void syscalls_init(void);
void capabilities_init(void);
void security_init(void);
void arch_disable_interrupts(void);

// Gestion des interruptions
void irq_init(void);
void irq_enable(void);
void irq_disable(void);

// Console pour debugging
void console_init(void);
void console_putchar(char c);
void console_puts(const char *str);
int kprintf(const char *fmt, ...);

// Fonctions assembleur pour MSVC
#ifdef _MSC_VER
void cli(void);
void hlt(void);
#else
static inline void cli(void) { __asm__ volatile("cli"); }
static inline void hlt(void) { __asm__ volatile("hlt"); }
#endif

// Note: read_cr3 is declared in mm.h and implemented in arch/x86_64/cpu.c

// Panic et gestion d'erreurs
// panic() déclaré dans types.h
NORETURN void kernel_halt(void);
void kernel_panic(const char *message);

// Boot functions
struct orion_boot_info;
int orion_boot_init(const struct orion_boot_info *boot_info);
int orion_boot_validate(const struct orion_boot_info *boot_info);
void orion_boot_debug_print(const struct orion_boot_info *boot_info);

// Macros de debugging
#define KLOG_EMERGENCY 0
#define KLOG_ALERT 1
#define KLOG_CRITICAL 2
#define KLOG_ERROR 3
#define KLOG_WARNING 4
#define KLOG_NOTICE 5
#define KLOG_INFO 6
#define KLOG_DEBUG 7

#ifdef DEBUG
#define klog(level, fmt, ...) kprintf("[%d] " fmt "\n", level, ##__VA_ARGS__)
#define kdebug(fmt, ...) klog(KLOG_DEBUG, fmt, ##__VA_ARGS__)
#else
#define klog(level, fmt, ...)                                \
    do                                                       \
    {                                                        \
        if (level <= KLOG_WARNING)                           \
            kprintf("[%d] " fmt "\n", level, ##__VA_ARGS__); \
    } while (0)
#define kdebug(fmt, ...) ((void)0)
#endif

#define kerror(fmt, ...) klog(KLOG_ERROR, fmt, ##__VA_ARGS__)
#define kwarning(fmt, ...) klog(KLOG_WARNING, fmt, ##__VA_ARGS__)
#define kinfo(fmt, ...) klog(KLOG_INFO, fmt, ##__VA_ARGS__)

#endif // ORION_KERNEL_H
