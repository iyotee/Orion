/*
 * ORION OS - MIPS Architecture Boot
 *
 * Assembly file for MIPS architecture (MIPS32, MIPS64)
 * Implements early boot sequence and exception vectors
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "config.h"

// ============================================================================
// ASSEMBLY MACROS
// ============================================================================

.macro SET_STACK stack_addr
    move $sp, \stack_addr
.endm

.macro CLEAR_BSS
    // Clear BSS section
    la $t0, __bss_start
    la $t1, __bss_end
    subu $t2, $t1, $t0
    beqz $t2, 1f
    move $t3, $zero
0:  sb $t3, 0($t0)
    addiu $t0, $t0, 1
    addiu $t2, $t2, -1
    bnez $t2, 0b
1:
.endm

.macro SETUP_EXCEPTION_VECTORS
    // Set up exception vector base
    la $t0, mips_exception_vector_table
    mtc0 $t0, $4  // Set Context register to vector base
.endm

.macro DISABLE_CACHES
    // Disable I-cache and D-cache
    mfc0 $t0, $16  // Read Config register
    andi $t0, $t0, ~((1 << 12) | (1 << 2))  // Clear I-cache and D-cache enable bits
    mtc0 $t0, $16  // Write Config register
    
    // Invalidate caches
    cache 0, 0($0)  // Index Writeback Invalidate Data
    cache 1, 0($0)  // Index Writeback Invalidate Instruction
.endm

.macro ENABLE_CACHES
    // Enable I-cache and D-cache
    mfc0 $t0, $16  // Read Config register
    ori $t0, $t0, (1 << 12) | (1 << 2)  // Set I-cache and D-cache enable bits
    mtc0 $t0, $16  // Write Config register
.endm

.macro SETUP_MMU
    // Set up basic MMU configuration
    // This is a simplified setup for early boot
    mfc0 $t0, $16  // Read Config register
    ori $t0, $t0, (1 << 3)  // Enable MMU
    mtc0 $t0, $16  // Write Config register
.endm

// ============================================================================
// ENTRY POINT
// ============================================================================

.global _start
.type _start, @function
_start:
    // Disable interrupts
    di
    
    // Set up initial stack
    la $sp, mips_kernel_stack + 16384
    
    // Clear BSS section
    CLEAR_BSS
    
    // Set up exception vectors
    SETUP_EXCEPTION_VECTORS
    
    // Disable caches initially
    DISABLE_CACHES
    
    // Call early C initialization
    jal mips_early_init
    
    // Enable caches
    ENABLE_CACHES
    
    // Set up MMU
    SETUP_MMU
    
    // Jump to bootstrap
    j mips_bootstrap

// ============================================================================
// EXCEPTION VECTOR TABLE
// ============================================================================

.section .text.vectors
.align 4

.global mips_exception_vector_table
mips_exception_vector_table:

// Vector 0: Reset
    j mips_reset_vector
    nop

// Vector 1: Cache Error
    j mips_cache_error_vector
    nop

// Vector 2: General Exception
    j mips_general_exception_vector
    nop

// Vector 3: Interrupt
    j mips_interrupt_vector
    nop

// Vector 4: Load/Store Error
    j mips_load_store_error_vector
    nop

// Vector 5: Instruction Bus Error
    j mips_instruction_bus_error_vector
    nop

// Vector 6: Data Bus Error
    j mips_data_bus_error_vector
    nop

// Vector 7: System Call
    j mips_syscall_vector
    nop

// Vector 8: Breakpoint
    j mips_breakpoint_vector
    nop

// Vector 9: Reserved Instruction
    j mips_reserved_instruction_vector
    nop

// Vector 10: Coprocessor Unusable
    j mips_coprocessor_unusable_vector
    nop

// Vector 11: Overflow
    j mips_overflow_vector
    nop

// Vector 12: Trap
    j mips_trap_vector
    nop

// Vector 13: Reserved
    j mips_reserved_vector
    nop

// Vector 14: Reserved
    j mips_reserved_vector
    nop

// Vector 15: Reserved
    j mips_reserved_vector
    nop

// ============================================================================
// EXCEPTION HANDLERS
// ============================================================================

mips_reset_vector:
    // Reset handler
    j _start
    nop

mips_cache_error_vector:
    // Cache error handler
    j mips_handle_cache_error
    nop

mips_general_exception_vector:
    // General exception handler
    j mips_handle_general_exception
    nop

mips_interrupt_vector:
    // Interrupt handler
    j mips_handle_interrupt
    nop

mips_load_store_error_vector:
    // Load/store error handler
    j mips_handle_load_store_error
    nop

mips_instruction_bus_error_vector:
    // Instruction bus error handler
    j mips_handle_instruction_bus_error
    nop

mips_data_bus_error_vector:
    // Data bus error handler
    j mips_handle_data_bus_error
    nop

mips_syscall_vector:
    // System call handler
    j mips_syscall_entry
    nop

mips_breakpoint_vector:
    // Breakpoint handler
    j mips_handle_breakpoint
    nop

mips_reserved_instruction_vector:
    // Reserved instruction handler
    j mips_handle_reserved_instruction
    nop

mips_coprocessor_unusable_vector:
    // Coprocessor unusable handler
    j mips_handle_coprocessor_unusable
    nop

mips_overflow_vector:
    // Overflow handler
    j mips_handle_overflow
    nop

mips_trap_vector:
    // Trap handler
    j mips_handle_trap
    nop

mips_reserved_vector:
    // Reserved vector handler
    j mips_handle_reserved
    nop

// ============================================================================
// DATA SECTIONS
// ============================================================================

.section .data
.align 16

// Kernel stack
.global mips_kernel_stack
mips_kernel_stack:
    .space 16384  // 16KB kernel stack

// Exception stack
.global mips_exception_stack
mips_exception_stack:
    .space 4096   // 4KB exception stack

// Interrupt stack
.global mips_interrupt_stack
mips_interrupt_stack:
    .space 4096   // 4KB interrupt stack

// ============================================================================
// SYMBOLS FOR LINKER
// ============================================================================

.section .text
.global mips_bootstrap
.global mips_handle_cache_error
.global mips_handle_general_exception
.global mips_handle_interrupt
.global mips_handle_load_store_error
.global mips_handle_instruction_bus_error
.global mips_handle_data_bus_error
.global mips_handle_breakpoint
.global mips_handle_reserved_instruction
.global mips_handle_coprocessor_unusable
.global mips_handle_overflow
.global mips_handle_trap
.global mips_handle_reserved
