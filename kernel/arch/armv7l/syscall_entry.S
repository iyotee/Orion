/*
 * ORION OS - ARMv7l System Call Entry
 * 
 * This file implements the system call entry point and context switching
 * for ARMv7l architecture.
 */

#include "arch.h"

// ============================================================================
// EXTERNAL SYMBOLS
// ============================================================================

.extern arm_handle_system_call
.extern arm_syscall_table

// ============================================================================
// MACROS
// ============================================================================

.macro SAVE_CONTEXT
    // Save all general purpose registers
    push    {r0-r12, lr}
    
    // Save CPSR
    mrs     r0, cpsr
    push    {r0}
    
    // Save SPSR if available
    mrs     r0, spsr
    push    {r0}
    
    // Save stack pointer for current mode
    mov     r0, sp
    push    {r0}
    
    // Save link register for current mode
    mov     r0, lr
    push    {r0}
.endm

.macro RESTORE_CONTEXT
    // Restore link register for current mode
    pop     {r0}
    mov     lr, r0
    
    // Restore stack pointer for current mode
    pop     {r0}
    mov     sp, r0
    
    // Restore SPSR
    pop     {r0}
    msr     spsr, r0
    
    // Restore CPSR
    pop     {r0}
    msr     cpsr, r0
    
    // Restore all general purpose registers
    pop     {r0-r12, lr}
.endm

.macro SWITCH_TO_SVC_MODE
    // Switch to SVC mode
    mrs     r0, cpsr
    bic     r0, r0, #0x1F
    orr     r0, r0, #0x13
    msr     cpsr, r0
.endm

.macro SWITCH_TO_USER_MODE
    // Switch to user mode
    mrs     r0, cpsr
    bic     r0, r0, #0x1F
    orr     r0, r0, #0x10
    msr     cpsr, r0
.endm

// ============================================================================
// SYSTEM CALL ENTRY POINT
// ============================================================================

.global arm_syscall_entry
.type arm_syscall_entry, %function

arm_syscall_entry:
    // Save user context
    SAVE_CONTEXT
    
    // Switch to SVC mode
    SWITCH_TO_SVC_MODE
    
    // Set up kernel stack
    ldr     sp, =arm_kernel_stack_top
    
    // Call system call handler
    bl      arm_handle_system_call
    
    // Restore user context
    RESTORE_CONTEXT
    
    // Return to user mode
    SWITCH_TO_USER_MODE
    
    // Return from system call
    movs    pc, lr

// ============================================================================
// FAST SYSTEM CALL ENTRY
// ============================================================================

.global arm_fast_syscall_entry
.type arm_fast_syscall_entry, %function

arm_fast_syscall_entry:
    // Fast system call entry for frequently used calls
    // This bypasses some context saving for performance
    
    // Save minimal context
    push    {r0-r3, lr}
    
    // Switch to SVC mode
    SWITCH_TO_SVC_MODE
    
    // Set up kernel stack
    ldr     sp, =arm_kernel_stack_top
    
    // Call fast system call handler
    bl      arm_handle_fast_syscall
    
    // Restore minimal context
    pop     {r0-r3, lr}
    
    // Return to user mode
    SWITCH_TO_USER_MODE
    
    // Return from system call
    movs    pc, lr

// ============================================================================
// CONTEXT SWITCHING
// ============================================================================

.global arm_context_switch
.type arm_context_switch, %function

arm_context_switch:
    // Save current context
    SAVE_CONTEXT
    
    // Store current context pointer
    str     sp, [r0]
    
    // Load new context pointer
    ldr     sp, [r1]
    
    // Restore new context
    RESTORE_CONTEXT
    
    // Return to new context
    bx      lr

// ============================================================================
// INTERRUPT CONTEXT SWITCHING
// ============================================================================

.global arm_interrupt_context_switch
.type arm_interrupt_context_switch, %function

arm_interrupt_context_switch:
    // Save interrupt context
    push    {r0-r12, lr}
    
    // Save CPSR
    mrs     r0, cpsr
    push    {r0}
    
    // Switch to SVC mode
    SWITCH_TO_SVC_MODE
    
    // Set up kernel stack
    ldr     sp, =arm_kernel_stack_top
    
    // Call interrupt context switch handler
    bl      arm_handle_interrupt_context_switch
    
    // Restore interrupt context
    pop     {r0}
    msr     cpsr, r0
    
    pop     {r0-r12, lr}
    
    // Return from interrupt
    subs    pc, lr, #4

// ============================================================================
// EXCEPTION CONTEXT SWITCHING
// ============================================================================

.global arm_exception_context_switch
.type arm_exception_context_switch, %function

arm_exception_context_switch:
    // Save exception context
    push    {r0-r12, lr}
    
    // Save CPSR
    mrs     r0, cpsr
    push    {r0}
    
    // Save SPSR
    mrs     r0, spsr
    push    {r0}
    
    // Switch to SVC mode
    SWITCH_TO_SVC_MODE
    
    // Set up kernel stack
    ldr     sp, =arm_kernel_stack_top
    
    // Call exception context switch handler
    bl      arm_handle_exception_context_switch
    
    // Restore exception context
    pop     {r0}
    msr     spsr, r0
    
    pop     {r0}
    msr     cpsr, r0
    
    pop     {r0-r12, lr}
    
    // Return from exception
    subs    pc, lr, #4

// ============================================================================
// USER MODE ENTRY
// ============================================================================

.global arm_enter_user_mode
.type arm_enter_user_mode, %function

arm_enter_user_mode:
    // Set up user mode stack
    mov     sp, r0
    
    // Set up user mode link register
    mov     lr, r1
    
    // Switch to user mode
    SWITCH_TO_USER_MODE
    
    // Jump to user code
    bx      lr

// ============================================================================
// KERNEL MODE ENTRY
// ============================================================================

.global arm_enter_kernel_mode
.type arm_enter_kernel_mode, %function

arm_enter_kernel_mode:
    // Switch to SVC mode
    SWITCH_TO_SVC_MODE
    
    // Set up kernel stack
    ldr     sp, =arm_kernel_stack_top
    
    // Jump to kernel code
    bx      lr

// ============================================================================
// PRIVILEGE LEVEL SWITCHING
// ============================================================================

.global arm_switch_to_privileged
.type arm_switch_to_privileged, %function

arm_switch_to_privileged:
    // Switch to privileged mode (SVC)
    SWITCH_TO_SVC_MODE
    
    // Set up privileged stack
    ldr     sp, =arm_privileged_stack_top
    
    // Return
    bx      lr

.global arm_switch_to_unprivileged
.type arm_switch_to_unprivileged, %function

arm_switch_to_unprivileged:
    // Switch to unprivileged mode (USR)
    SWITCH_TO_USER_MODE
    
    // Set up unprivileged stack
    ldr     sp, =arm_user_stack_top
    
    // Return
    bx      lr

// ============================================================================
// STACK MANAGEMENT
// ============================================================================

.global arm_get_current_stack_pointer
.type arm_get_current_stack_pointer, %function

arm_get_current_stack_pointer:
    // Return current stack pointer
    mov     r0, sp
    bx      lr

.global arm_set_stack_pointer
.type arm_set_stack_pointer, %function

arm_set_stack_pointer:
    // Set stack pointer to value in r0
    mov     sp, r0
    bx      lr

// ============================================================================
// MODE SWITCHING UTILITIES
// ============================================================================

.global arm_get_current_mode
.type arm_get_current_mode, %function

arm_get_current_mode:
    // Get current CPU mode
    mrs     r0, cpsr
    and     r0, r0, #0x1F
    bx      lr

.global arm_is_privileged
.type arm_is_privileged, %function

arm_is_privileged:
    // Check if current mode is privileged
    mrs     r0, cpsr
    and     r0, r0, #0x1F
    cmp     r0, #0x10
    movge   r0, #1
    movlt   r0, #0
    bx      lr

// ============================================================================
// SYSTEM CALL TABLE ACCESS
// ============================================================================

.global arm_get_syscall_handler
.type arm_get_syscall_handler, %function

arm_get_syscall_handler:
    // Get system call handler from table
    // r0 contains system call number
    ldr     r1, =arm_syscall_table
    ldr     r0, [r1, r0, lsl #2]
    bx      lr

// ============================================================================
// ERROR HANDLING
// ============================================================================

.global arm_syscall_error
.type arm_syscall_error, %function

arm_syscall_error:
    // Handle system call errors
    mov     r0, #-1  // Return error code
    
    // Restore context and return
    RESTORE_CONTEXT
    SWITCH_TO_USER_MODE
    movs    pc, lr

// ============================================================================
// DEBUGGING SUPPORT
// ============================================================================

.global arm_debug_syscall
.type arm_debug_syscall, %function

arm_debug_syscall:
    // Debug system call entry
    // This can be used for tracing and debugging
    
    // Save debug context
    push    {r0-r3, lr}
    
    // Call debug handler
    bl      arm_handle_debug_syscall
    
    // Restore debug context
    pop     {r0-r3, lr}
    
    // Continue with normal system call
    b       arm_syscall_entry

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

.global arm_syscall_perf_start
.type arm_syscall_perf_start, %function

arm_syscall_perf_start:
    // Start performance monitoring for system call
    // This can be used for profiling
    
    // Save performance context
    push    {r0-r3, lr}
    
    // Start performance counter
    bl      arm_start_syscall_perf_counter
    
    // Restore performance context
    pop     {r0-r3, lr}
    
    // Continue with normal system call
    b       arm_syscall_entry

.global arm_syscall_perf_stop
.type arm_syscall_perf_stop, %function

arm_syscall_perf_stop:
    // Stop performance monitoring for system call
    // This can be used for profiling
    
    // Save performance context
    push    {r0-r3, lr}
    
    // Stop performance counter
    bl      arm_stop_syscall_perf_counter
    
    // Restore performance context
    pop     {r0-r3, lr}
    
    // Continue with normal system call return
    b       arm_syscall_return

// ============================================================================
// SECURITY CHECKS
// ============================================================================

.global arm_syscall_security_check
.type arm_syscall_security_check, %function

arm_syscall_security_check:
    // Perform security checks for system call
    // This can be used for access control
    
    // Save security context
    push    {r0-r3, lr}
    
    // Perform security check
    bl      arm_check_syscall_security
    
    // Check result
    cmp     r0, #0
    bne     arm_syscall_security_violation
    
    // Restore security context
    pop     {r0-r3, lr}
    
    // Continue with normal system call
    b       arm_syscall_entry

arm_syscall_security_violation:
    // Handle security violation
    mov     r0, #-EPERM  // Permission denied
    
    // Restore security context
    pop     {r0-r3, lr}
    
    // Return error
    b       arm_syscall_error

// ============================================================================
// DATA SECTION
// ============================================================================

.section .data

// Kernel stack (aligned to 8 bytes)
.align 8
arm_kernel_stack:
    .space 4096  // 4KB kernel stack
arm_kernel_stack_top:

// Privileged stack (aligned to 8 bytes)
.align 8
arm_privileged_stack:
    .space 2048  // 2KB privileged stack
arm_privileged_stack_top:

// User stack (aligned to 8 bytes)
.align 8
arm_user_stack:
    .space 2048  // 2KB user stack
arm_user_stack_top:

// System call table
arm_syscall_table:
    .word   arm_syscall_read
    .word   arm_syscall_write
    .word   arm_syscall_open
    .word   arm_syscall_close
    .word   arm_syscall_exit
    .word   arm_syscall_fork
    .word   arm_syscall_execve
    .word   arm_syscall_waitpid
    .word   arm_syscall_kill
    .word   arm_syscall_getpid
    .word   arm_syscall_sleep
    .word   0  // End of table marker
