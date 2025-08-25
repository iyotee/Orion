/*
 * Orion Operating System - aarch64 Boot Code
 *
 * Early boot process for aarch64 architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "config.h"

.section .text.boot, "ax"
.global _start

_start:
    // Disable all interrupts
    msr daifset, #0xF
    
    // Set up initial stack
    ldr x0, =aarch64_boot_stack_top
    mov sp, x0
    
    // Clear BSS section
    ldr x0, =__bss_start
    ldr x1, =__bss_end
    sub x1, x1, x0
    bl aarch64_memzero
    
    // Set up exception vectors
    ldr x0, =aarch64_exception_vectors
    msr VBAR_EL1, x0
    
    // Configure initial MMU (identity mapping)
    bl aarch64_mmu_early_init
    
    // Enable MMU
    bl aarch64_mmu_enable
    
    // Jump to C initialization
    bl aarch64_arch_init
    
    // Should not return
    b aarch64_panic_handler

aarch64_mmu_early_init:
    // Set up initial page tables for identity mapping
    ret

aarch64_mmu_enable:
    // Enable MMU
    mrs x0, SCTLR_EL1
    orr x0, x0, #SCTLR_EL1_M
    msr SCTLR_EL1, x0
    
    // Ensure completion
    isb
    ret

aarch64_memzero:
    // x0 = start address, x1 = size
    cbz x1, 2f
1:  str xzr, [x0], #8
    subs x1, x1, #8
    b.hi 1b
2:  ret

aarch64_panic_handler:
    // Critical error - halt the system
    wfi
    b aarch64_panic_handler
