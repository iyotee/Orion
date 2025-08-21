/*
 * Orion Operating System - x86_64 Architecture Assembly Functions
 *
 * Assembly implementations for low-level architecture operations
 * that cannot be done in C.
 *
 * Developed by Jérémy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

.section .text
.code64

// ========================================
// INTERRUPT MANAGEMENT
// ========================================

// Load IDT
.global load_idt
.type load_idt, @function
load_idt:
    lidt (%rdi)
    ret

// Enable interrupts
.global enable_interrupts
.type enable_interrupts, @function
enable_interrupts:
    sti
    ret

// Disable interrupts
.global disable_interrupts
.type disable_interrupts, @function
disable_interrupts:
    cli
    ret

// Get RFLAGS
.global get_rflags
.type get_rflags, @function
get_rflags:
    pushfq
    popq %rax
    ret

// ========================================
// PIC OPERATIONS
// ========================================

// Send command to PIC
.global pic_send_command
.type pic_send_command, @function
pic_send_command:
    mov %rsi, %rdx      // port
    mov %rdi, %rax      // command
    outb %al, %dx
    ret

// Send data to PIC
.global pic_send_data
.type pic_send_data, @function
pic_send_data:
    mov %rsi, %rdx      // port
    mov %rdi, %rax      // data
    outb %al, %dx
    ret

// ========================================
// CPU OPERATIONS
// ========================================

// Get CPUID feature information
.global cpuid_get_feature_info
.type cpuid_get_feature_info, @function
cpuid_get_feature_info:
    mov $1, %eax
    cpuid
    mov %edx, %eax      // Return features in EDX
    ret

// Read MSR
.global msr_read
.type msr_read, @function
msr_read:
    mov %rdi, %rcx      // MSR number
    rdmsr
    shl $32, %rdx       // Shift high 32 bits
    or %rdx, %rax       // Combine with low 32 bits
    ret

// Write MSR
.global msr_write
.type msr_write, @function
msr_write:
    mov %rdi, %rcx      // MSR number
    mov %rsi, %rax      // Low 32 bits
    mov %rdx, %rdx      // High 32 bits
    wrmsr
    ret

// ========================================
// MEMORY OPERATIONS
// ========================================

// Read CR0
.global read_cr0
.type read_cr0, @function
read_cr0:
    mov %cr0, %rax
    ret

// Write CR0
.global write_cr0
.type write_cr0, @function
write_cr0:
    mov %rdi, %cr0
    ret

// Read CR3
.global read_cr3
.type read_cr3, @function
read_cr3:
    mov %cr3, %rax
    ret

// Write CR3
.global write_cr3
.type write_cr3, @function
write_cr3:
    mov %rdi, %cr3
    ret

// Read CR4
.global read_cr4
.type read_cr4, @function
read_cr4:
    mov %cr4, %rax
    ret

// Write CR4
.global write_cr4
.type write_cr4, @function
write_cr4:
    mov %rdi, %cr4
    ret

// ========================================
// TIMING OPERATIONS
// ========================================

// Read TSC
.global read_tsc
.type read_tsc, @function
read_tsc:
    rdtsc
    shl $32, %rdx       // Shift high 32 bits
    or %rdx, %rax       // Combine with low 32 bits
    ret

// CPU pause
.global cpu_pause
.type cpu_pause, @function
cpu_pause:
    pause
    ret

// CPU halt
.global cpu_halt
.type cpu_halt, @function
cpu_halt:
    hlt
    ret

// ========================================
// CACHE OPERATIONS
// ========================================

// Invalidate TLB
.global invlpg
.type invlpg, @function
invlpg:
    invlpg (%rdi)
    ret

// Flush cache line
.global clflush
.type clflush, @function
clflush:
    clflush (%rdi)
    ret

// Memory barrier
.global memory_barrier
.type memory_barrier, @function
memory_barrier:
    mfence
    ret

// Read barrier
.global read_barrier
.type read_barrier, @function
read_barrier:
    lfence
    ret

// Write barrier
.global write_barrier
.type write_barrier, @function
write_barrier:
    sfence
    ret
