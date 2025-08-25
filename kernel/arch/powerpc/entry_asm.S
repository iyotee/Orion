/*
 * ORION OS - POWER Architecture Assembly Entry Point
 *
 * Assembly bootstrap and interrupt handlers for POWER architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "config.h"

.section .text.boot
.global _start
.global power_bootstrap_asm
.global power_interrupt_vector
.global power_exception_vector

// ============================================================================
// BOOTSTRAP CODE
// ============================================================================

_start:
    // Set up initial stack pointer
    lis     %r1, __stack_top@ha
    addi    %r1, %r1, __stack_top@l
    
    // Clear BSS section
    lis     %r3, __bss_start@ha
    addi    %r3, %r3, __bss_start@l
    lis     %r4, __bss_end@ha
    addi    %r4, %r4, __bss_end@l
    subf    %r5, %r3, %r4
    bl      memset
    
    // Set up initial MSR
    li      %r3, 0
    mtmsr   %r3
    
    // Set up initial SPRs
    li      %r3, 0
    mtspr   1, %r3      // XER
    mtspr   8, %r3      // LR
    mtspr   9, %r3      // CTR
    
    // Set up initial CR
    li      %r3, 0
    mtcr    %r3
    
    // Call C bootstrap function
    bl      power_bootstrap_asm
    
    // Should never return
1:  b       1b

// ============================================================================
// INTERRUPT VECTOR TABLE
// ============================================================================

power_interrupt_vector:
    // Save registers
    stdu    %r1, -256(%r1)
    std     %r0, 16(%r1)
    std     %r2, 24(%r1)
    std     %r3, 32(%r1)
    std     %r4, 40(%r1)
    std     %r5, 48(%r1)
    std     %r6, 56(%r1)
    std     %r7, 64(%r1)
    std     %r8, 72(%r1)
    std     %r9, 80(%r1)
    std     %r10, 88(%r1)
    std     %r11, 96(%r1)
    std     %r12, 104(%r1)
    std     %r13, 112(%r1)
    std     %r14, 120(%r1)
    std     %r15, 128(%r1)
    std     %r16, 136(%r1)
    std     %r17, 144(%r1)
    std     %r18, 152(%r1)
    std     %r19, 160(%r1)
    std     %r20, 168(%r1)
    std     %r21, 176(%r1)
    std     %r22, 184(%r1)
    std     %r23, 192(%r1)
    std     %r24, 200(%r1)
    std     %r25, 208(%r1)
    std     %r26, 216(%r1)
    std     %r27, 224(%r1)
    std     %r28, 232(%r1)
    std     %r29, 240(%r1)
    std     %r30, 248(%r1)
    std     %r31, 256(%r1)
    
    // Save MSR
    mfmsr   %r3
    std     %r3, 264(%r1)
    
    // Save CR
    mfcr    %r3
    std     %r3, 272(%r1)
    
    // Save LR
    mflr    %r3
    std     %r3, 280(%r1)
    
    // Save CTR
    mfctr   %r3
    std     %r3, 288(%r1)
    
    // Save XER
    mfxer   %r3
    std     %r3, 296(%r1)
    
    // Get interrupt number
    mfspr   %r3, 63      // IVPR
    rlwinm  %r3, %r3, 0, 24, 31
    
    // Call C interrupt handler
    mr      %r4, %r1
    bl      power_interrupt_handler
    
    // Restore registers
    ld      %r31, 256(%r1)
    ld      %r30, 248(%r1)
    ld      %r29, 240(%r1)
    ld      %r28, 232(%r1)
    ld      %r27, 224(%r1)
    ld      %r26, 216(%r1)
    ld      %r25, 208(%r1)
    ld      %r24, 200(%r1)
    ld      %r23, 192(%r1)
    ld      %r22, 184(%r1)
    ld      %r21, 176(%r1)
    ld      %r20, 168(%r1)
    ld      %r19, 160(%r1)
    ld      %r18, 152(%r1)
    ld      %r17, 144(%r1)
    ld      %r16, 136(%r1)
    ld      %r15, 128(%r1)
    ld      %r14, 120(%r1)
    ld      %r13, 112(%r1)
    ld      %r12, 104(%r1)
    ld      %r11, 96(%r1)
    ld      %r10, 88(%r1)
    ld      %r9, 80(%r1)
    ld      %r8, 72(%r1)
    ld      %r7, 64(%r1)
    ld      %r6, 56(%r1)
    ld      %r5, 48(%r1)
    ld      %r4, 40(%r1)
    ld      %r3, 32(%r1)
    ld      %r2, 24(%r1)
    ld      %r0, 16(%r1)
    
    // Restore MSR
    ld      %r3, 264(%r1)
    mtmsr   %r3
    
    // Restore CR
    ld      %r3, 272(%r1)
    mtcr    %r3
    
    // Restore LR
    ld      %r3, 280(%r1)
    mtlr    %r3
    
    // Restore CTR
    ld      %r3, 288(%r1)
    mtctr   %r3
    
    // Restore XER
    ld      %r3, 296(%r1)
    mtxer   %r3
    
    // Restore stack pointer
    addi    %r1, %r1, 256
    
    // Return from interrupt
    rfi

// ============================================================================
// EXCEPTION VECTOR
// ============================================================================

power_exception_vector:
    // Save registers (similar to interrupt)
    stdu    %r1, -256(%r1)
    std     %r0, 16(%r1)
    std     %r2, 24(%r1)
    std     %r3, 32(%r1)
    std     %r4, 40(%r1)
    std     %r5, 48(%r1)
    std     %r6, 56(%r1)
    std     %r7, 64(%r1)
    std     %r8, 72(%r1)
    std     %r9, 80(%r1)
    std     %r10, 88(%r1)
    std     %r11, 96(%r1)
    std     %r12, 104(%r1)
    std     %r13, 112(%r1)
    std     %r14, 120(%r1)
    std     %r15, 128(%r1)
    std     %r16, 136(%r1)
    std     %r17, 144(%r1)
    std     %r18, 152(%r1)
    std     %r19, 160(%r1)
    std     %r20, 168(%r1)
    std     %r21, 176(%r1)
    std     %r22, 184(%r1)
    std     %r23, 192(%r1)
    std     %r24, 200(%r1)
    std     %r25, 208(%r1)
    std     %r26, 216(%r1)
    std     %r27, 224(%r1)
    std     %r28, 232(%r1)
    std     %r29, 240(%r1)
    std     %r30, 248(%r1)
    std     %r31, 256(%r1)
    
    // Save MSR
    mfmsr   %r3
    std     %r3, 264(%r1)
    
    // Save CR
    mfcr    %r3
    std     %r3, 272(%r1)
    
    // Save LR
    mflr    %r3
    std     %r3, 280(%r1)
    
    // Save CTR
    mfctr   %r3
    std     %r3, 288(%r1)
    
    // Save XER
    mfxer   %r3
    std     %r3, 296(%r1)
    
    // Get exception number
    mfspr   %r3, 63      // IVPR
    rlwinm  %r3, %r3, 0, 24, 31
    
    // Call C exception handler
    mr      %r4, %r1
    bl      power_exception_handler
    
    // Restore registers (similar to interrupt)
    ld      %r31, 256(%r1)
    ld      %r30, 248(%r1)
    ld      %r29, 240(%r1)
    ld      %r28, 232(%r1)
    ld      %r27, 224(%r1)
    ld      %r26, 216(%r1)
    ld      %r25, 208(%r1)
    ld      %r24, 200(%r1)
    ld      %r23, 192(%r1)
    ld      %r22, 184(%r1)
    ld      %r21, 176(%r1)
    ld      %r20, 168(%r1)
    ld      %r19, 160(%r1)
    ld      %r18, 152(%r1)
    ld      %r17, 144(%r1)
    ld      %r16, 136(%r1)
    ld      %r15, 128(%r1)
    ld      %r14, 120(%r1)
    ld      %r13, 112(%r1)
    ld      %r12, 104(%r1)
    ld      %r11, 96(%r1)
    ld      %r10, 88(%r1)
    ld      %r9, 80(%r1)
    ld      %r8, 72(%r1)
    ld      %r7, 64(%r1)
    ld      %r6, 56(%r1)
    ld      %r5, 48(%r1)
    ld      %r4, 40(%r1)
    ld      %r3, 32(%r1)
    ld      %r2, 24(%r1)
    ld      %r0, 16(%r1)
    
    // Restore MSR
    ld      %r3, 264(%r1)
    mtmsr   %r3
    
    // Restore CR
    ld      %r3, 272(%r1)
    mtcr    %r3
    
    // Restore LR
    ld      %r3, 280(%r1)
    mtlr    %r3
    
    // Restore CTR
    ld      %r3, 288(%r1)
    mtctr   %r3
    
    // Restore XER
    ld      %r3, 296(%r1)
    mtxer   %r3
    
    // Restore stack pointer
    addi    %r1, %r1, 256
    
    // Return from exception
    rfi

// ============================================================================
// SYSTEM CALL ENTRY
// ============================================================================

power_syscall_entry:
    // Save registers
    stdu    %r1, -256(%r1)
    std     %r0, 16(%r1)
    std     %r2, 24(%r1)
    std     %r3, 32(%r1)
    std     %r4, 40(%r1)
    std     %r5, 48(%r1)
    std     %r6, 56(%r1)
    std     %r7, 64(%r1)
    std     %r8, 72(%r1)
    std     %r9, 80(%r1)
    std     %r10, 88(%r1)
    std     %r11, 96(%r1)
    std     %r12, 104(%r1)
    std     %r13, 112(%r1)
    std     %r14, 120(%r1)
    std     %r15, 128(%r1)
    std     %r16, 136(%r1)
    std     %r17, 144(%r1)
    std     %r18, 152(%r1)
    std     %r19, 160(%r1)
    std     %r20, 168(%r1)
    std     %r21, 176(%r1)
    std     %r22, 184(%r1)
    std     %r23, 192(%r1)
    std     %r24, 200(%r1)
    std     %r25, 208(%r1)
    std     %r26, 216(%r1)
    std     %r27, 224(%r1)
    std     %r28, 232(%r1)
    std     %r29, 240(%r1)
    std     %r30, 248(%r1)
    std     %r31, 256(%r1)
    
    // Save MSR
    mfmsr   %r3
    std     %r3, 264(%r1)
    
    // Save CR
    mfcr    %r3
    std     %r3, 272(%r1)
    
    // Save LR
    mflr    %r3
    std     %r3, 280(%r1)
    
    // Save CTR
    mfctr   %r3
    std     %r3, 288(%r1)
    
    // Save XER
    mfxer   %r3
    std     %r3, 296(%r1)
    
    // Get system call number from r0
    mr      %r3, %r0
    
    // Call C system call handler
    mr      %r4, %r1
    bl      power_system_call_handler
    
    // Restore registers
    ld      %r31, 256(%r1)
    ld      %r30, 248(%r1)
    ld      %r29, 240(%r1)
    ld      %r28, 232(%r1)
    ld      %r27, 224(%r1)
    ld      %r26, 216(%r1)
    ld      %r25, 208(%r1)
    ld      %r24, 200(%r1)
    ld      %r23, 192(%r1)
    ld      %r22, 184(%r1)
    ld      %r21, 176(%r1)
    ld      %r20, 168(%r1)
    ld      %r19, 160(%r1)
    ld      %r18, 152(%r1)
    ld      %r17, 144(%r1)
    ld      %r16, 136(%r1)
    ld      %r15, 128(%r1)
    ld      %r14, 120(%r1)
    ld      %r13, 112(%r1)
    ld      %r12, 104(%r1)
    ld      %r11, 96(%r1)
    ld      %r10, 88(%r1)
    ld      %r9, 80(%r1)
    ld      %r8, 72(%r1)
    ld      %r7, 64(%r1)
    ld      %r6, 56(%r1)
    ld      %r5, 48(%r1)
    ld      %r4, 40(%r1)
    ld      %r3, 32(%r1)
    ld      %r2, 24(%r1)
    ld      %r0, 16(%r1)
    
    // Restore MSR
    ld      %r3, 264(%r1)
    mtmsr   %r3
    
    // Restore CR
    ld      %r3, 272(%r1)
    mtcr    %r3
    
    // Restore LR
    ld      %r3, 280(%r1)
    mtlr    %r3
    
    // Restore CTR
    ld      %r3, 288(%r1)
    mtctr   %r3
    
    // Restore XER
    ld      %r3, 296(%r1)
    mtxer   %r3
    
    // Restore stack pointer
    addi    %r1, %r1, 256
    
    // Return from system call
    rfi

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

memset:
    // r3 = destination, r5 = count
    cmpdi   %r5, 0
    beqlr
    
    mtctr   %r5
    li      %r6, 0
    
1:  stb     %r6, 0(%r3)
    addi    %r3, %r3, 1
    bdnz    1b
    
    blr

// ============================================================================
// POWER-SPECIFIC FUNCTIONS
// ============================================================================

power_bootstrap_asm:
    // Set up initial TLB
    bl      power_tlb_init_asm
    
    // Set up initial interrupt vectors
    bl      power_interrupt_init_asm
    
    // Set up initial timer
    bl      power_timer_init_asm
    
    // Call C bootstrap function
    bl      power_bootstrap
    
    blr

power_tlb_init_asm:
    // Clear TLB entries
    li      %r3, 0
    mtctr   %r3
    
    // Set up initial page table
    lis     %r3, 0x1000  // Page table base
    mtspr   25, %r3      // SDR1
    
    blr

power_interrupt_init_asm:
    // Set up interrupt vector base
    lis     %r3, power_interrupt_vector@ha
    addi    %r3, %r3, power_interrupt_vector@l
    mtspr   63, %r3      // IVPR
    
    // Set up exception vector base
    lis     %r3, power_exception_vector@ha
    addi    %r3, %r3, power_exception_vector@l
    mtspr   62, %r3      // IVOR
    
    blr

power_timer_init_asm:
    // Set up decrementer
    li      %r3, 0xFFFFFFFF
    mtspr   22, %r3      // DEC
    
    // Set up time base
    li      %r3, 0
    mtspr   268, %r3     // TBU
    mtspr   269, %r3     // TBL
    
    blr
