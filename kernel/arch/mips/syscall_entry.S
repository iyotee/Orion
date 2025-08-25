/*
 * ORION OS - MIPS Architecture System Call Entry
 *
 * Assembly file for MIPS architecture (MIPS32, MIPS64)
 * Implements system call entry points and context switching
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

.macro SAVE_CONTEXT
    // Save all general purpose registers
    sd $at, 0($sp)
    sd $v0, 8($sp)
    sd $v1, 16($sp)
    sd $a0, 24($sp)
    sd $a1, 32($sp)
    sd $a2, 40($sp)
    sd $a3, 48($sp)
    sd $t0, 56($sp)
    sd $t1, 64($sp)
    sd $t2, 72($sp)
    sd $t3, 80($sp)
    sd $t4, 88($sp)
    sd $t5, 96($sp)
    sd $t6, 104($sp)
    sd $t7, 112($sp)
    sd $s0, 120($sp)
    sd $s1, 128($sp)
    sd $s2, 136($sp)
    sd $s3, 144($sp)
    sd $s4, 152($sp)
    sd $s5, 160($sp)
    sd $s6, 168($sp)
    sd $s7, 176($sp)
    sd $t8, 184($sp)
    sd $t9, 192($sp)
    sd $k0, 200($sp)
    sd $k1, 208($sp)
    sd $gp, 216($sp)
    sd $fp, 232($sp)
    sd $ra, 240($sp)
    
    // Save HI/LO registers
    mflo $t0
    sd $t0, 248($sp)
    mfhi $t0
    sd $t0, 256($sp)
    
    // Save PC
    sd $ra, 264($sp)
.endm

.macro RESTORE_CONTEXT
    // Restore HI/LO registers first
    ld $t0, 248($sp)
    mtlo $t0
    ld $t0, 256($sp)
    mthi $t0
    
    // Restore all general purpose registers
    ld $at, 0($sp)
    ld $v0, 8($sp)
    ld $v1, 16($sp)
    ld $a0, 24($sp)
    ld $a1, 32($sp)
    ld $a2, 40($sp)
    ld $a3, 48($sp)
    ld $t0, 56($sp)
    ld $t1, 64($sp)
    ld $t2, 72($sp)
    ld $t3, 80($sp)
    ld $t4, 88($sp)
    ld $t5, 96($sp)
    ld $t6, 104($sp)
    ld $t7, 112($sp)
    ld $s0, 120($sp)
    ld $s1, 128($sp)
    ld $s2, 136($sp)
    ld $s3, 144($sp)
    ld $s4, 152($sp)
    ld $s5, 160($sp)
    ld $s6, 168($sp)
    ld $s7, 176($sp)
    ld $t8, 184($sp)
    ld $t9, 192($sp)
    ld $k0, 200($sp)
    ld $k1, 208($sp)
    ld $gp, 216($sp)
    ld $fp, 232($sp)
    ld $ra, 240($sp)
.endm

.macro SWITCH_TO_KERNEL_MODE
    // Switch to kernel mode (disable interrupts)
    di
    // Set kernel stack
    move $k0, $sp
.endm

.macro SWITCH_TO_USER_MODE
    // Switch to user mode (enable interrupts)
    ei
    // Restore user stack
    move $sp, $k0
.endm

// ============================================================================
// SYSTEM CALL ENTRY POINTS
// ============================================================================

.global mips_syscall_entry
.type mips_syscall_entry, @function
mips_syscall_entry:
    // Save context
    SAVE_CONTEXT
    
    // Switch to kernel mode
    SWITCH_TO_KERNEL_MODE
    
    // Call C handler
    jal mips_handle_system_call
    
    // Switch back to user mode
    SWITCH_TO_USER_MODE
    
    // Restore context
    RESTORE_CONTEXT
    
    // Return from system call
    eret

.global mips_fast_syscall_entry
.type mips_fast_syscall_entry, @function
mips_fast_syscall_entry:
    // Fast system call entry (minimal context save)
    // Save only essential registers
    sd $v0, 8($sp)
    sd $a0, 24($sp)
    sd $a1, 32($sp)
    sd $a2, 40($sp)
    sd $a3, 48($sp)
    sd $ra, 240($sp)
    
    // Call fast handler
    jal mips_handle_fast_system_call
    
    // Restore essential registers
    ld $v0, 8($sp)
    ld $a0, 24($sp)
    ld $a1, 32($sp)
    ld $a2, 40($sp)
    ld $a3, 48($sp)
    ld $ra, 240($sp)
    
    // Return
    jr $ra

// ============================================================================
// CONTEXT SWITCHING FUNCTIONS
// ============================================================================

.global mips_context_switch
.type mips_context_switch, @function
mips_context_switch:
    // Save current context
    SAVE_CONTEXT
    
    // Save current stack pointer
    sd $sp, 224($sp)
    
    // Switch to new context
    move $sp, $a0
    
    // Restore new context
    RESTORE_CONTEXT
    
    // Return to new context
    jr $ra

.global mips_interrupt_context_switch
.type mips_interrupt_context_switch, @function
mips_interrupt_context_switch:
    // Save current context
    SAVE_CONTEXT
    
    // Save current stack pointer
    sd $sp, 224($sp)
    
    // Switch to new context
    move $sp, $a0
    
    // Restore new context
    RESTORE_CONTEXT
    
    // Return from interrupt
    eret

.global mips_exception_context_switch
.type mips_exception_context_switch, @function
mips_exception_context_switch:
    // Save current context
    SAVE_CONTEXT
    
    // Save current stack pointer
    sd $sp, 224($sp)
    
    // Switch to new context
    move $sp, $a0
    
    // Restore new context
    RESTORE_CONTEXT
    
    // Return from exception
    eret

// ============================================================================
// MODE SWITCHING FUNCTIONS
// ============================================================================

.global mips_enter_user_mode
.type mips_enter_user_mode, @function
mips_enter_user_mode:
    // Set user mode flags
    mfc0 $t0, $12  // Read Status register
    ori $t0, $t0, 0x10  // Set User mode bit
    mtc0 $t0, $12  // Write Status register
    
    // Enable interrupts
    ei
    
    // Return to user mode
    eret

.global mips_enter_kernel_mode
.type mips_enter_kernel_mode, @function
mips_enter_kernel_mode:
    // Set kernel mode flags
    mfc0 $t0, $12  // Read Status register
    andi $t0, $t0, ~0x10  // Clear User mode bit
    mtc0 $t0, $12  // Write Status register
    
    // Disable interrupts
    di
    
    // Return
    jr $ra

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

.global mips_get_stack_pointer
.type mips_get_stack_pointer, @function
mips_get_stack_pointer:
    move $v0, $sp
    jr $ra

.global mips_set_stack_pointer
.type mips_set_stack_pointer, @function
mips_set_stack_pointer:
    move $sp, $a0
    jr $ra

.global mips_get_frame_pointer
.type mips_get_frame_pointer, @function
mips_get_frame_pointer:
    move $v0, $fp
    jr $ra

.global mips_set_frame_pointer
.type mips_set_frame_pointer, @function
mips_set_frame_pointer:
    move $fp, $a0
    jr $ra

// ============================================================================
// DATA SECTIONS
// ============================================================================

.section .data
.align 8

// System call table
.global mips_syscall_table
mips_syscall_table:
    .quad 0  // syscall 0
    .quad 0  // syscall 1
    .quad 0  // syscall 2
    .quad 0  // syscall 3
    .quad 0  // syscall 4
    .quad 0  // syscall 5
    .quad 0  // syscall 6
    .quad 0  // syscall 7
    .quad 0  // syscall 8
    .quad 0  // syscall 9
    .quad 0  // syscall 10
    .quad 0  // syscall 11
    .quad 0  // syscall 12
    .quad 0  // syscall 13
    .quad 0  // syscall 14
    .quad 0  // syscall 15

// Stack alignment
.align 16
.global mips_syscall_stack
mips_syscall_stack:
    .space 4096  // 4KB stack for system calls
