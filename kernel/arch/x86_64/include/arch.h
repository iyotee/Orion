/*
 * Orion Operating System - x86_64 Architecture Header
 *
 * Architecture-specific definitions, constants, and inline functions
 * for x86_64 processors. Includes register structures, CPU features,
 * and low-level hardware access functions.
 *
 * Developed by Jérémy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_ARCH_X86_64_H
#define ORION_ARCH_X86_64_H

#include <orion/types.h>

// x86_64 architecture-specific definitions

// Registres CPU complets
typedef struct
{
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t rip, rflags;
} cpu_regs_t;

// Registres pour interruptions
typedef struct
{
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
} registers_t;

// Structure IDT entry
typedef struct
{
    uint16_t offset_low;  // Offset bits 0-15
    uint16_t selector;    // Code segment selector
    uint8_t ist;          // Interrupt stack table
    uint8_t type_attr;    // Type et attributs
    uint16_t offset_mid;  // Offset bits 16-31
    uint32_t offset_high; // Offset bits 32-63
    uint32_t zero;        // Réservé
} PACKED idt_entry_t;

// Structure IDTR
typedef struct
{
    uint16_t limit;
    uint64_t base;
} PACKED idtr_t;

// Constants CR4
#define CR4_SMEP (1ULL << 20) // Supervisor Mode Execution Prevention
#define CR4_SMAP (1ULL << 21) // Supervisor Mode Access Prevention
#define CR4_UMIP (1ULL << 11) // User Mode Instruction Prevention

// MSR Constants
#define MSR_EFER 0xC0000080   // Extended Feature Enable Register
#define MSR_STAR 0xC0000081   // SYSCALL target address
#define MSR_LSTAR 0xC0000082  // Long mode SYSCALL target
#define MSR_CSTAR 0xC0000083  // Compatibility mode SYSCALL target
#define MSR_SFMASK 0xC0000084 // SYSCALL flag mask

// EFER Constants
#define EFER_NXE (1ULL << 11) // No-Execute Enable

// GDT/IDT
#define GDT_ENTRIES 5
#define IDT_ENTRIES 256

// Core architecture functions
void arch_init(void);
void arch_early_init(void);
void arch_late_init(void);
void arch_cpu_init(void);

// Subsystem initialization
void mmu_init(void);
void interrupts_init(void);
void detect_cpu(cpu_info_t *info);

// Missing function declarations
void arch_halt(void);
void arch_pause(void);
void syscall_entry(void); // Assembly syscall entry point

// Control register access
uint64_t read_cr3(void);
void write_cr3(uint64_t cr3);
uint64_t read_cr0(void);
void write_cr0(uint64_t cr0);
uint64_t read_cr4(void);
void write_cr4(uint64_t cr4);

// Virtual address conversion (defined in mm.h but used here)
#define PHYS_TO_VIRT(paddr) ((void *)((paddr) + 0xFFFF800000000000ULL))
#define VIRT_TO_PHYS(vaddr) ((uint64_t)(vaddr) - 0xFFFF800000000000ULL)

// Fonctions assembleur avec stubs MSVC
#ifdef _MSC_VER
// Stubs MSVC - seront remplacés par assembleur
static inline void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{
    (void)leaf;
    *eax = *ebx = *ecx = *edx = 0;
}
static inline uint64_t read_cr4(void) { return 0; }
static inline void write_cr4(uint64_t val) { (void)val; }
static inline void write_cr3(uint64_t val) { (void)val; }
static inline uint64_t rdmsr(uint32_t msr)
{
    (void)msr;
    return 0;
}
static inline void wrmsr(uint32_t msr, uint64_t val)
{
    (void)msr;
    (void)val;
}
static inline uint8_t __builtin_inb(uint16_t port)
{
    (void)port;
    return 0;
}
static inline void __builtin_outb(uint16_t port, uint8_t data)
{
    (void)port;
    (void)data;
}
static inline void idt_load(idtr_t *idtr) { (void)idtr; }
static inline void sti(void) { /* MSVC stub */ }
#else
// Vraies fonctions assembleur GCC/Clang
static inline void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{
    __asm__ volatile("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(leaf));
}
static inline uint64_t read_cr4(void)
{
    uint64_t val;
    __asm__ volatile("mov %%cr4, %0" : "=r"(val));
    return val;
}
static inline void write_cr4(uint64_t val)
{
    __asm__ volatile("mov %0, %%cr4" ::"r"(val));
}
static inline void write_cr3(uint64_t val)
{
    __asm__ volatile("mov %0, %%cr3" ::"r"(val));
}
static inline uint64_t rdmsr(uint32_t msr)
{
    uint32_t low, high;
    __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}
static inline void wrmsr(uint32_t msr, uint64_t val)
{
    __asm__ volatile("wrmsr" ::"a"((uint32_t)val), "d"((uint32_t)(val >> 32)), "c"(msr));
}
static inline uint8_t __builtin_inb(uint16_t port)
{
    uint8_t data;
    __asm__ volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}
static inline void __builtin_outb(uint16_t port, uint8_t data)
{
    __asm__ volatile("outb %0, %1" ::"a"(data), "Nd"(port));
}
static inline void idt_load(idtr_t *idtr)
{
    __asm__ volatile("lidt %0" ::"m"(*idtr));
}
static inline void sti(void)
{
    __asm__ volatile("sti");
}
#endif

// I/O ports
static inline void outb(uint16_t port, uint8_t data)
{
    __builtin_outb(port, data);
}

static inline uint8_t inb(uint16_t port)
{
    return __builtin_inb(port);
}

// Console série
void serial_init(void);
void serial_putchar(char c);

#endif // ORION_ARCH_X86_64_H
