/*
 * ORION OS - ARMv7l Architecture Assembly Entry
 *
 * Assembly-level bootstrap code, interrupt vectors, and exception handling
 * Supports all Raspberry Pi models: Pi 1, Pi 2, Pi Zero, Pi Zero W
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
    stmfd sp!, {r0-r12, lr}
    mrs r0, cpsr
    stmfd sp!, {r0}
    mrs r0, spsr
    stmfd sp!, {r0}
.endm

.macro RESTORE_CONTEXT
    ldmfd sp!, {r0}
    msr spsr, r0
    ldmfd sp!, {r0}
    msr cpsr, r0
    ldmfd sp!, {r0-r12, lr}
.endm

.macro SWITCH_MODE mode
    mrs r0, cpsr
    bic r0, r0, #0x1F
    orr r0, r0, #\mode
    msr cpsr, r0
.endm

.macro SET_STACK mode, stack_addr
    SWITCH_MODE \mode
    ldr sp, =\stack_addr
.endm

// ============================================================================
// SECTION DEFINITIONS
// ============================================================================

.section .text.boot, "ax"
.global _start
.global arm_reset_vector

// ============================================================================
// BOOTSTRAP CODE
// ============================================================================

_start:
    // Disable all interrupts
    cpsid if
    
    // Set up initial stack for each mode
    SET_STACK #0x11, #0x8000  // FIQ mode stack
    SET_STACK #0x12, #0x7000  // IRQ mode stack
    SET_STACK #0x13, #0x6000  // Supervisor mode stack
    SET_STACK #0x17, #0x5000  // Abort mode stack
    SET_STACK #0x1B, #0x4000  // Undefined mode stack
    SET_STACK #0x1F, #0x3000  // System mode stack
    
    // Switch to supervisor mode
    SWITCH_MODE #0x13
    
    // Set up supervisor mode stack
    ldr sp, =#0x6000
    
    // Clear BSS section
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
1:
    cmp r0, r1
    strlt r2, [r0], #4
    blt 1b
    
    // Set up exception vectors
    ldr r0, =exception_vector_table
    mcr p15, 0, r0, c12, c0, 0  // Set VBAR (Vector Base Address Register)
    
    // Initialize basic hardware
    bl arm_early_init
    
    // Jump to C bootstrap
    bl arm_bootstrap
    
    // Should never reach here
1:  b 1b

// ============================================================================
// EXCEPTION VECTOR TABLE
// ============================================================================

.align 5
exception_vector_table:
    // Reset vector
    ldr pc, reset_handler_addr
    
    // Undefined instruction
    ldr pc, undefined_handler_addr
    
    // Software interrupt (SWI)
    ldr pc, swi_handler_addr
    
    // Prefetch abort
    ldr pc, prefetch_abort_handler_addr
    
    // Data abort
    ldr pc, data_abort_handler_addr
    
    // Reserved
    ldr pc, reserved_handler_addr
    
    // IRQ
    ldr pc, irq_handler_addr
    
    // FIQ
    ldr pc, fiq_handler_addr

// ============================================================================
// EXCEPTION HANDLER ADDRESSES
// ============================================================================

reset_handler_addr:
    .word arm_reset_vector

undefined_handler_addr:
    .word arm_undefined_handler

swi_handler_addr:
    .word arm_swi_handler

prefetch_abort_handler_addr:
    .word arm_prefetch_abort_handler

data_abort_handler_addr:
    .word arm_data_abort_handler

reserved_handler_addr:
    .word arm_reserved_handler

irq_handler_addr:
    .word arm_irq_handler

fiq_handler_addr:
    .word arm_fiq_handler

// ============================================================================
// EXCEPTION HANDLERS
// ============================================================================

arm_reset_vector:
    // Reset handler - jump to bootstrap
    ldr pc, =_start

arm_undefined_handler:
    // Save context
    SAVE_CONTEXT
    
    // Handle undefined instruction
    bl arm_undefined_exception_handler
    
    // Restore context
    RESTORE_CONTEXT
    subs pc, lr, #4

arm_swi_handler:
    // Save context
    SAVE_CONTEXT
    
    // Handle software interrupt
    bl arm_swi_exception_handler
    
    // Restore context
    RESTORE_CONTEXT
    subs pc, lr, #4

arm_prefetch_abort_handler:
    // Save context
    SAVE_CONTEXT
    
    // Handle prefetch abort
    bl arm_prefetch_abort_exception_handler
    
    // Restore context
    RESTORE_CONTEXT
    subs pc, lr, #4

arm_data_abort_handler:
    // Save context
    SAVE_CONTEXT
    
    // Handle data abort
    bl arm_data_abort_exception_handler
    
    // Restore context
    RESTORE_CONTEXT
    subs pc, lr, #4

arm_reserved_handler:
    // Save context
    SAVE_CONTEXT
    
    // Handle reserved exception
    bl arm_reserved_exception_handler
    
    // Restore context
    RESTORE_CONTEXT
    subs pc, lr, #4

arm_irq_handler:
    // Save context
    SAVE_CONTEXT
    
    // Handle IRQ
    bl arm_irq_exception_handler
    
    // Restore context
    RESTORE_CONTEXT
    subs pc, lr, #4

arm_fiq_handler:
    // Save context
    SAVE_CONTEXT
    
    // Handle FIQ
    bl arm_fiq_exception_handler
    
    // Restore context
    RESTORE_CONTEXT
    subs pc, lr, #4

// ============================================================================
// EXTERNAL EXCEPTION HANDLERS
// ============================================================================

.extern arm_undefined_exception_handler
.extern arm_swi_exception_handler
.extern arm_prefetch_abort_exception_handler
.extern arm_data_abort_exception_handler
.extern arm_reserved_exception_handler
.extern arm_irq_exception_handler
.extern arm_fiq_exception_handler

// ============================================================================
// SYSTEM CALL ENTRY POINT
// ============================================================================

.global arm_syscall_entry
arm_syscall_entry:
    // Save context
    SAVE_CONTEXT
    
    // Call system call handler
    bl arm_syscall_handler
    
    // Restore context
    RESTORE_CONTEXT
    movs pc, lr

.extern arm_syscall_handler

// ============================================================================
// CONTEXT SWITCHING
// ============================================================================

.global arm_context_switch
arm_context_switch:
    // r0 = old context, r1 = new context
    
    // Save old context
    cmp r0, #0
    beq 1f
    
    // Save all registers
    stmfd r0!, {r0-r12, sp, lr}
    mrs r2, cpsr
    stmfd r0!, {r2}
    
1:
    // Restore new context
    cmp r1, #0
    beq 2f
    
    // Restore CPSR
    ldmfd r1!, {r2}
    msr cpsr, r2
    
    // Restore all registers
    ldmfd r1!, {r0-r12, sp, lr}
    
2:
    // Return to new context
    bx lr

// ============================================================================
// CPU CONTROL FUNCTIONS
// ============================================================================

.global arm_cpu_enable_interrupts
arm_cpu_enable_interrupts:
    cpsie i
    bx lr

.global arm_cpu_disable_interrupts
arm_cpu_disable_interrupts:
    cpsid i
    bx lr

.global arm_cpu_enable_fiq
arm_cpu_enable_fiq:
    cpsie f
    bx lr

.global arm_cpu_disable_fiq
arm_cpu_disable_fiq:
    cpsid f
    bx lr

.global arm_cpu_nop
arm_cpu_nop:
    nop
    bx lr

.global arm_cpu_wfi
arm_cpu_wfi:
    wfi
    bx lr

.global arm_cpu_wfe
arm_cpu_wfe:
    wfe
    bx lr

.global arm_cpu_sev
arm_cpu_sev:
    sev
    bx lr

// ============================================================================
// CPSR/SPSR ACCESS
// ============================================================================

.global arm_cpu_read_cpsr
arm_cpu_read_cpsr:
    mrs r0, cpsr
    bx lr

.global arm_cpu_write_cpsr
arm_cpu_write_cpsr:
    msr cpsr, r0
    bx lr

.global arm_cpu_read_spsr
arm_cpu_read_spsr:
    mrs r0, spsr
    bx lr

.global arm_cpu_write_spsr
arm_cpu_write_spsr:
    msr spsr, r0
    bx lr

// ============================================================================
// CPU ID AND FEATURES
// ============================================================================

.global arm_cpu_get_id
arm_cpu_get_id:
    mrc p15, 0, r0, c0, c0, 0
    bx lr

.global arm_cpu_get_revision
arm_cpu_get_revision:
    mrc p15, 0, r0, c0, c1, 0
    bx lr

.global arm_cpu_get_feature_flags
arm_cpu_get_feature_flags:
    mrc p15, 0, r0, c0, c1, 0
    bx lr

// ============================================================================
// MMU CONTROL
// ============================================================================

.global arm_mmu_enable
arm_mmu_enable:
    mrc p15, 0, r0, c1, c0, 0
    orr r0, r0, #0x1
    mcr p15, 0, r0, c1, c0, 0
    isb
    bx lr

.global arm_mmu_disable
arm_mmu_disable:
    mrc p15, 0, r0, c1, c0, 0
    bic r0, r0, #0x1
    mcr p15, 0, r0, c1, c0, 0
    isb
    bx lr

.global arm_mmu_invalidate_tlb
arm_mmu_invalidate_tlb:
    mcr p15, 0, r0, c8, c7, 0
    isb
    bx lr

.global arm_mmu_invalidate_tlb_entry
arm_mmu_invalidate_tlb_entry:
    mcr p15, 0, r0, c8, c7, 1
    isb
    bx lr

// ============================================================================
// CACHE CONTROL
// ============================================================================

.global arm_cache_enable
arm_cache_enable:
    // Enable I-cache
    mrc p15, 0, r0, c1, c0, 0
    orr r0, r0, #0x1000
    mcr p15, 0, r0, c1, c0, 0
    isb
    
    // Enable D-cache
    mrc p15, 0, r0, c1, c0, 0
    orr r0, r0, #0x4
    mcr p15, 0, r0, c1, c0, 0
    isb
    
    bx lr

.global arm_cache_disable
arm_cache_disable:
    // Disable I-cache
    mrc p15, 0, r0, c1, c0, 0
    bic r0, r0, #0x1000
    mcr p15, 0, r0, c1, c0, 0
    isb
    
    // Disable D-cache
    mrc p15, 0, r0, c1, c0, 0
    bic r0, r0, #0x4
    mcr p15, 0, r0, c1, c0, 0
    isb
    
    bx lr

.global arm_cache_flush_all
arm_cache_flush_all:
    mcr p15, 0, r0, c7, c14, 0
    isb
    bx lr

.global arm_cache_invalidate_all
arm_cache_invalidate_all:
    mcr p15, 0, r0, c7, c7, 0
    isb
    bx lr

.global arm_cache_flush_range
arm_cache_flush_range:
    // r0 = start address, r1 = end address
1:
    cmp r0, r1
    bge 2f
    mcr p15, 0, r0, c7, c14, 1
    add r0, r0, #32
    b 1b
2:
    isb
    bx lr

.global arm_cache_invalidate_range
arm_cache_invalidate_range:
    // r0 = start address, r1 = end address
1:
    cmp r0, r1
    bge 2f
    mcr p15, 0, r0, c7, c7, 1
    add r0, r0, #32
    b 1b
2:
    isb
    bx lr

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

.global arm_performance_get_cycle_counter
arm_performance_get_cycle_counter:
    mrc p15, 0, r0, c9, c13, 0
    bx lr

.global arm_performance_get_instruction_counter
arm_performance_get_instruction_counter:
    mrc p15, 0, r0, c9, c13, 1
    bx lr

.global arm_performance_reset_counters
arm_performance_reset_counters:
    mcr p15, 0, r0, c9, c12, 0
    isb
    bx lr

// ============================================================================
// NEON AND VFP CONTROL
// ============================================================================

.global arm_neon_enable
arm_neon_enable:
    // Enable NEON access
    mrc p15, 0, r0, c1, c0, 2
    orr r0, r0, #0x00F00000
    mcr p15, 0, r0, c1, c0, 2
    isb
    
    // Enable NEON
    mov r0, #0x40000000
    vmsr fpexc, r0
    
    bx lr

.global arm_vfp_enable
arm_vfp_enable:
    // Enable VFP access
    mrc p15, 0, r0, c1, c0, 2
    orr r0, r0, #0x00F00000
    mcr p15, 0, r0, c1, c0, 2
    isb
    
    // Enable VFP
    mov r0, #0x40000000
    vmsr fpexc, r0
    
    bx lr

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

.global arm_delay_cycles
arm_delay_cycles:
    // r0 = number of cycles to delay
1:
    subs r0, r0, #1
    bne 1b
    bx lr

.global arm_delay_microseconds
arm_delay_microseconds:
    // r0 = microseconds to delay
    // Assuming 1MHz timer frequency
    mov r1, #1000
    mul r0, r0, r1
    b arm_delay_cycles

.global arm_delay_milliseconds
arm_delay_milliseconds:
    // r0 = milliseconds to delay
    mov r1, #1000000
    mul r0, r0, r1
    b arm_delay_cycles

// ============================================================================
// MEMORY BARRIERS
// ============================================================================

.global arm_memory_barrier
arm_memory_barrier:
    dmb
    bx lr

.global arm_memory_barrier_read
arm_memory_barrier_read:
    dmb ish
    bx lr

.global arm_memory_barrier_write
arm_memory_barrier_write:
    dmb ishst
    bx lr

.global arm_instruction_barrier
arm_instruction_barrier:
    isb
    bx lr

// ============================================================================
// ATOMIC OPERATIONS
// ============================================================================

.global arm_atomic_add
arm_atomic_add:
    // r0 = address, r1 = value to add
    dmb ish
1:
    ldrex r2, [r0]
    add r2, r2, r1
    strex r3, r2, [r0]
    cmp r3, #0
    bne 1b
    dmb ish
    mov r0, r2
    bx lr

.global arm_atomic_sub
arm_atomic_sub:
    // r0 = address, r1 = value to subtract
    dmb ish
1:
    ldrex r2, [r0]
    sub r2, r2, r1
    strex r3, r2, [r0]
    cmp r3, #0
    bne 1b
    dmb ish
    mov r0, r2
    bx lr

.global arm_atomic_swap
arm_atomic_swap:
    // r0 = address, r1 = new value
    dmb ish
1:
    ldrex r2, [r0]
    strex r3, r1, [r0]
    cmp r3, #0
    bne 1b
    dmb ish
    mov r0, r2
    bx lr

.global arm_atomic_compare_and_swap
arm_atomic_compare_and_swap:
    // r0 = address, r1 = expected value, r2 = new value
    dmb ish
1:
    ldrex r3, [r0]
    cmp r3, r1
    bne 2f
    strex r3, r2, [r0]
    cmp r3, #0
    bne 1b
    mov r0, #1
    b 3f
2:
    clrex
    mov r0, #0
3:
    dmb ish
    bx lr

// ============================================================================
// END OF FILE
// ============================================================================
