/*
 * Orion Operating System - aarch64 Assembly Implementation
 *
 * Critical assembly functions for aarch64 architecture including:
 * - Context switching and thread management
 * - Interrupt and exception handling
 * - Cache and TLB management
 * - Memory barriers and atomic operations
 * - Boot and early initialization
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "config.h"

// ============================================================================
// MACROS AND CONSTANTS
// ============================================================================

// Exception levels
#define EL0    0
#define EL1    1
#define EL2    2
#define EL3    3

// System control register bits
#define SCTLR_EL1_M    0x00000001  // MMU enable
#define SCTLR_EL1_A    0x00000002  // Alignment check enable
#define SCTLR_EL1_C    0x00000004  // Data cache enable
#define SCTLR_EL1_I    0x00001000  // Instruction cache enable
#define SCTLR_EL1_W    0x00000008  // Write buffer enable
#define SCTLR_EL1_SA  0x00000040  // Stack alignment check enable
#define SCTLR_EL1_CP15B 0x00002000 // CP15 barrier enable
#define SCTLR_EL1_ITD  0x00000020  // IT disable
#define SCTLR_EL1_SED  0x00000010  // SETEND disable
#define SCTLR_EL1_UMA  0x00000080  // User mask access
#define SCTLR_EL1_SW   0x00000100  // SWP/SWPB enable
#define SCTLR_EL1_Z    0x00000800  // Branch prediction enable
#define SCTLR_EL1_DZ   0x00000400  // DC ZVA enable
#define SCTLR_EL1_UCT  0x00008000  // User cache maintenance
#define SCTLR_EL1_NTWI 0x00010000  // Non-Trap WFE
#define SCTLR_EL1_NTWE 0x00020000  // Non-Trap WFI
#define SCTLR_EL1_WXN  0x00080000  // Write permission implies XN
#define SCTLR_EL1_TSCXT 0x00100000 // Trap SMC, HVC, SVC, MCR, MRC
#define SCTLR_EL1_IESB 0x00200000  // Implicit Error Synchronization Barrier
#define SCTLR_EL1_EE   0x02000000  // Exception endianness
#define SCTLR_EL1_UCI  0x04000000  // User cache maintenance instructions
#define SCTLR_EL1_E0E  0x01000000  // Exception level 0 endianness
#define SCTLR_EL1_SPAN 0x00800000  // Set PAN bit
#define SCTLR_EL1_EIS  0x00400000  // Exception Inactive State
#define SCTLR_EL1_TIDCP 0x08000000 // Trap ID group 0
#define SCTLR_EL1_TID1  0x10000000 // Trap ID group 1
#define SCTLR_EL1_TID2  0x20000000 // Trap ID group 2
#define SCTLR_EL1_TID3  0x40000000 // Trap ID group 3
#define SCTLR_EL1_TSCIE 0x80000000 // Trap SMC instructions
#define SCTLR_EL1_RES1  0x00000000 // Reserved, must be 1

// HCR_EL2 bits
#define HCR_EL2_VM     0x0000000000000001
#define HCR_EL2_SWIO   0x0000000000000002
#define HCR_EL2_PTW    0x0000000000000004
#define HCR_EL2_FMO    0x0000000000000008
#define HCR_EL2_IMO    0x0000000000000010
#define HCR_EL2_AMO    0x0000000000000020
#define HCR_EL2_VF     0x0000000000000040
#define HCR_EL2_VI     0x0000000000000080
#define HCR_EL2_VSE    0x0000000000000100
#define HCR_EL2_FB     0x0000000000000200
#define HCR_EL2_BSU    0x0000000000000C00
#define HCR_EL2_DC     0x0000000000001000
#define HCR_EL2_TWI    0x0000000000002000
#define HCR_EL2_TWE    0x0000000000004000
#define HCR_EL2_TID0   0x0000000000008000
#define HCR_EL2_TID1   0x0000000000010000
#define HCR_EL2_TID2   0x0000000000020000
#define HCR_EL2_TID3   0x0000000000040000
#define HCR_EL2_TSC    0x0000000000080000
#define HCR_EL2_TIDCP  0x0000000000100000
#define HCR_EL2_TACR   0x0000000000200000
#define HCR_EL2_TSW    0x0000000000400000
#define HCR_EL2_TPC    0x0000000000800000
#define HCR_EL2_TPU    0x0000000001000000
#define HCR_EL2_TTLB   0x0000000002000000
#define HCR_EL2_TVM    0x0000000004000000
#define HCR_EL2_TDZ    0x0000000008000000
#define HCR_EL2_HCD    0x0000000010000000
#define HCR_EL2_TRVM   0x0000000020000000
#define HCR_EL2_RW     0x0000000040000000
#define HCR_EL2_TDCR   0x0000000080000000
#define HCR_EL2_TGE    0x0000000100000000
#define HCR_EL2_TVM    0x0000000004000000
#define HCR_EL2_TTLB   0x0000000002000000
#define HCR_EL2_TPU    0x0000000001000000
#define HCR_EL2_TPC    0x0000000000800000
#define HCR_EL2_TSW    0x0000000000400000
#define HCR_EL2_TACR   0x0000000000200000
#define HCR_EL2_TIDCP  0x0000000000100000
#define HCR_EL2_TSC    0x0000000000080000
#define HCR_EL2_TID3   0x0000000000040000
#define HCR_EL2_TID2   0x0000000000020000
#define HCR_EL2_TID1   0x0000000000010000
#define HCR_EL2_TID0   0x0000000000008000
#define HCR_EL2_TWE    0x0000000000004000
#define HCR_EL2_TWI    0x0000000000002000
#define HCR_EL2_DC     0x0000000000001000
#define HCR_EL2_BSU    0x0000000000000C00
#define HCR_EL2_FB     0x0000000000000200
#define HCR_EL2_VSE    0x0000000000000100
#define HCR_EL2_VI     0x0000000000000080
#define HCR_EL2_VF     0x0000000000000040
#define HCR_EL2_AMO    0x0000000000000020
#define HCR_EL2_IMO    0x0000000000000010
#define HCR_EL2_FMO    0x0000000000000008
#define HCR_EL2_PTW    0x0000000000000004
#define HCR_EL2_SWIO   0x0000000000000002
#define HCR_EL2_VM     0x0000000000000001

// ============================================================================
// EXCEPTION VECTORS
// ============================================================================

.section .text.vectors, "ax"
.align 11

.global aarch64_exception_vectors
aarch64_exception_vectors:
    // Current EL with SP0
    .align 7
    b aarch64_sync_handler_sp0
    .align 7
    b aarch64_irq_handler_sp0
    .align 7
    b aarch64_fiq_handler_sp0
    .align 7
    b aarch64_serror_handler_sp0
    
    // Current EL with SPx
    .align 7
    b aarch64_sync_handler_spx
    .align 7
    b aarch64_irq_handler_spx
    .align 7
    b aarch64_fiq_handler_spx
    .align 7
    b aarch64_serror_handler_spx
    
    // Lower EL using AArch64
    .align 7
    b aarch64_sync_handler_a64
    .align 7
    b aarch64_irq_handler_a64
    .align 7
    b aarch64_fiq_handler_a64
    .align 7
    b aarch64_serror_handler_a64
    
    // Lower EL using AArch32
    .align 7
    b aarch64_sync_handler_a32
    .align 7
    b aarch64_irq_handler_a32
    .align 7
    b aarch64_fiq_handler_a32
    .align 7
    b aarch64_serror_handler_a32

// ============================================================================
// EXCEPTION HANDLERS
// ============================================================================

aarch64_sync_handler_sp0:
    // Synchronous exception from current EL using SP0
    // This should not happen in normal operation
    b aarch64_panic_handler

aarch64_irq_handler_sp0:
    // IRQ from current EL using SP0
    // This should not happen in normal operation
    b aarch64_panic_handler

aarch64_fiq_handler_sp0:
    // FIQ from current EL using SP0
    // This should not happen in normal operation
    b aarch64_panic_handler

aarch64_serror_handler_sp0:
    // SError from current EL using SP0
    // This should not happen in normal operation
    b aarch64_panic_handler

aarch64_sync_handler_spx:
    // Synchronous exception from current EL using SPx
    // Save registers and handle
    sub sp, sp, #272
    stp x0, x1, [sp, #16 * 0]
    stp x2, x3, [sp, #16 * 1]
    stp x4, x5, [sp, #16 * 2]
    stp x6, x7, [sp, #16 * 3]
    stp x8, x9, [sp, #16 * 4]
    stp x10, x11, [sp, #16 * 5]
    stp x12, x13, [sp, #16 * 6]
    stp x14, x15, [sp, #16 * 7]
    stp x16, x17, [sp, #16 * 8]
    stp x18, x19, [sp, #16 * 9]
    stp x20, x21, [sp, #16 * 10]
    stp x22, x23, [sp, #16 * 11]
    stp x24, x25, [sp, #16 * 12]
    stp x26, x27, [sp, #16 * 13]
    stp x28, x29, [sp, #16 * 14]
    str x30, [sp, #16 * 15]
    
    // Get exception information
    mrs x0, esr_el1
    mrs x1, elr_el1
    mrs x2, far_el1
    
    // Call C handler
    bl aarch64_sync_exception_handler
    
    // Restore registers
    ldp x0, x1, [sp, #16 * 0]
    ldp x2, x3, [sp, #16 * 1]
    ldp x4, x5, [sp, #16 * 2]
    ldp x6, x7, [sp, #16 * 3]
    ldp x8, x9, [sp, #16 * 4]
    ldp x10, x11, [sp, #16 * 5]
    ldp x12, x13, [sp, #16 * 6]
    ldp x14, x15, [sp, #16 * 7]
    ldp x16, x17, [sp, #16 * 8]
    ldp x18, x19, [sp, #16 * 9]
    ldp x20, x21, [sp, #16 * 10]
    ldp x22, x23, [sp, #16 * 11]
    ldp x24, x25, [sp, #16 * 12]
    ldp x26, x27, [sp, #16 * 13]
    ldp x28, x29, [sp, #16 * 14]
    ldr x30, [sp, #16 * 15]
    add sp, sp, #272
    
    eret

aarch64_irq_handler_spx:
    // IRQ from current EL using SPx
    sub sp, sp, #272
    stp x0, x1, [sp, #16 * 0]
    stp x2, x3, [sp, #16 * 1]
    stp x4, x5, [sp, #16 * 2]
    stp x6, x7, [sp, #16 * 3]
    stp x8, x9, [sp, #16 * 4]
    stp x10, x11, [sp, #16 * 5]
    stp x12, x13, [sp, #16 * 6]
    stp x14, x15, [sp, #16 * 7]
    stp x16, x17, [sp, #16 * 8]
    stp x18, x19, [sp, #16 * 9]
    stp x20, x21, [sp, #16 * 10]
    stp x22, x23, [sp, #16 * 11]
    stp x24, x25, [sp, #16 * 12]
    stp x26, x27, [sp, #16 * 13]
    stp x28, x29, [sp, #16 * 14]
    str x30, [sp, #16 * 15]
    
    // Call C IRQ handler
    bl aarch64_irq_exception_handler
    
    // Restore registers
    ldp x0, x1, [sp, #16 * 0]
    ldp x2, x3, [sp, #16 * 1]
    ldp x4, x5, [sp, #16 * 2]
    ldp x6, x7, [sp, #16 * 3]
    ldp x8, x9, [sp, #16 * 4]
    ldp x10, x11, [sp, #16 * 5]
    ldp x12, x13, [sp, #16 * 6]
    ldp x14, x15, [sp, #16 * 7]
    ldp x16, x17, [sp, #16 * 8]
    ldp x18, x19, [sp, #16 * 9]
    ldp x20, x21, [sp, #16 * 10]
    ldp x22, x23, [sp, #16 * 11]
    ldp x24, x25, [sp, #16 * 12]
    ldp x26, x27, [sp, #16 * 13]
    ldp x28, x29, [sp, #16 * 14]
    ldr x30, [sp, #16 * 15]
    add sp, sp, #272
    
    eret

aarch64_fiq_handler_spx:
    // FIQ from current EL using SPx
    sub sp, sp, #272
    stp x0, x1, [sp, #16 * 0]
    stp x2, x3, [sp, #16 * 1]
    stp x4, x5, [sp, #16 * 2]
    stp x6, x7, [sp, #16 * 3]
    stp x8, x9, [sp, #16 * 4]
    stp x10, x11, [sp, #16 * 5]
    stp x12, x13, [sp, #16 * 6]
    stp x14, x15, [sp, #16 * 7]
    stp x16, x17, [sp, #16 * 8]
    stp x18, x19, [sp, #16 * 9]
    stp x20, x21, [sp, #16 * 10]
    stp x22, x23, [sp, #16 * 11]
    stp x24, x25, [sp, #16 * 12]
    stp x26, x27, [sp, #16 * 13]
    stp x28, x29, [sp, #16 * 14]
    str x30, [sp, #16 * 15]
    
    // Call C FIQ handler
    bl aarch64_fiq_exception_handler
    
    // Restore registers
    ldp x0, x1, [sp, #16 * 0]
    ldp x2, x3, [sp, #16 * 1]
    ldp x4, x5, [sp, #16 * 2]
    ldp x6, x7, [sp, #16 * 3]
    ldp x8, x9, [sp, #16 * 4]
    ldp x10, x11, [sp, #16 * 5]
    stp x12, x13, [sp, #16 * 6]
    stp x14, x15, [sp, #16 * 7]
    stp x16, x17, [sp, #16 * 8]
    stp x18, x19, [sp, #16 * 9]
    stp x20, x21, [sp, #16 * 10]
    stp x22, x23, [sp, #16 * 11]
    stp x24, x25, [sp, #16 * 12]
    stp x26, x27, [sp, #16 * 13]
    stp x28, x29, [sp, #16 * 14]
    ldr x30, [sp, #16 * 15]
    add sp, sp, #272
    
    eret

aarch64_serror_handler_spx:
    // SError from current EL using SPx
    sub sp, sp, #272
    stp x0, x1, [sp, #16 * 0]
    stp x2, x3, [sp, #16 * 1]
    stp x4, x5, [sp, #16 * 2]
    stp x6, x7, [sp, #16 * 3]
    stp x8, x9, [sp, #16 * 4]
    stp x10, x11, [sp, #16 * 5]
    stp x12, x13, [sp, #16 * 6]
    stp x14, x15, [sp, #16 * 7]
    stp x16, x17, [sp, #16 * 8]
    stp x18, x19, [sp, #16 * 9]
    stp x20, x21, [sp, #16 * 10]
    stp x22, x23, [sp, #16 * 11]
    stp x24, x25, [sp, #16 * 12]
    stp x26, x27, [sp, #16 * 13]
    stp x28, x29, [sp, #16 * 14]
    str x30, [sp, #16 * 15]
    
    // Call C SError handler
    bl aarch64_serror_exception_handler
    
    // Restore registers
    ldp x0, x1, [sp, #16 * 0]
    ldp x2, x3, [sp, #16 * 1]
    ldp x4, x5, [sp, #16 * 2]
    ldp x6, x7, [sp, #16 * 3]
    ldp x8, x9, [sp, #16 * 4]
    ldp x10, x11, [sp, #16 * 5]
    ldp x12, x13, [sp, #16 * 6]
    ldp x14, x15, [sp, #16 * 7]
    ldp x16, x17, [sp, #16 * 8]
    ldp x18, x19, [sp, #16 * 9]
    ldp x20, x21, [sp, #16 * 10]
    ldp x22, x23, [sp, #16 * 11]
    ldp x24, x25, [sp, #16 * 12]
    ldp x26, x27, [sp, #16 * 13]
    ldp x28, x29, [sp, #16 * 14]
    ldr x30, [sp, #16 * 15]
    add sp, sp, #272
    
    eret

aarch64_sync_handler_a64:
    // Synchronous exception from lower EL using AArch64
    // This is typically a system call
    sub sp, sp, #272
    stp x0, x1, [sp, #16 * 0]
    stp x2, x3, [sp, #16 * 1]
    stp x4, x5, [sp, #16 * 2]
    stp x6, x7, [sp, #16 * 3]
    stp x8, x9, [sp, #16 * 4]
    stp x10, x11, [sp, #16 * 5]
    stp x12, x13, [sp, #16 * 6]
    stp x14, x15, [sp, #16 * 7]
    stp x16, x17, [sp, #16 * 8]
    stp x18, x19, [sp, #16 * 9]
    stp x20, x21, [sp, #16 * 10]
    stp x22, x23, [sp, #16 * 11]
    stp x24, x25, [sp, #16 * 12]
    stp x26, x27, [sp, #16 * 13]
    stp x28, x29, [sp, #16 * 14]
    str x30, [sp, #16 * 15]
    
    // Check if this is a system call
    mrs x0, esr_el1
    lsr x1, x0, #26
    cmp x1, #0x15  // SVC instruction
    beq aarch64_syscall_handler
    
    // Not a system call, handle as exception
    bl aarch64_sync_exception_handler
    b aarch64_exception_return

aarch64_irq_handler_a64:
    // IRQ from lower EL using AArch64
    sub sp, sp, #272
    stp x0, x1, [sp, #16 * 0]
    stp x2, x3, [sp, #16 * 1]
    stp x4, x5, [sp, #16 * 2]
    stp x6, x7, [sp, #16 * 3]
    stp x8, x9, [sp, #16 * 4]
    stp x10, x11, [sp, #16 * 5]
    stp x12, x13, [sp, #16 * 6]
    stp x14, x15, [sp, #16 * 7]
    stp x16, x17, [sp, #16 * 8]
    stp x18, x19, [sp, #16 * 9]
    stp x20, x21, [sp, #16 * 10]
    stp x22, x23, [sp, #16 * 11]
    stp x24, x25, [sp, #16 * 12]
    stp x26, x27, [sp, #16 * 13]
    stp x28, x29, [sp, #16 * 14]
    str x30, [sp, #16 * 15]
    
    bl aarch64_irq_exception_handler
    b aarch64_exception_return

aarch64_fiq_handler_a64:
    // FIQ from lower EL using AArch64
    sub sp, sp, #272
    stp x0, x1, [sp, #16 * 0]
    stp x2, x3, [sp, #16 * 1]
    stp x4, x5, [sp, #16 * 2]
    stp x6, x7, [sp, #16 * 3]
    stp x8, x9, [sp, #16 * 4]
    stp x10, x11, [sp, #16 * 5]
    stp x12, x13, [sp, #16 * 6]
    stp x14, x15, [sp, #16 * 7]
    stp x16, x17, [sp, #16 * 8]
    stp x18, x19, [sp, #16 * 9]
    stp x20, x21, [sp, #16 * 10]
    stp x22, x23, [sp, #16 * 11]
    stp x24, x25, [sp, #16 * 12]
    stp x26, x27, [sp, #16 * 13]
    stp x28, x29, [sp, #16 * 14]
    str x30, [sp, #16 * 15]
    
    bl aarch64_fiq_exception_handler
    b aarch64_exception_return

aarch64_serror_handler_a64:
    // SError from lower EL using AArch64
    sub sp, sp, #272
    stp x0, x1, [sp, #16 * 0]
    stp x2, x3, [sp, #16 * 1]
    stp x4, x5, [sp, #16 * 2]
    stp x6, x7, [sp, #16 * 3]
    stp x8, x9, [sp, #16 * 4]
    stp x10, x11, [sp, #16 * 5]
    stp x12, x13, [sp, #16 * 6]
    stp x14, x15, [sp, #16 * 7]
    stp x16, x17, [sp, #16 * 8]
    stp x18, x19, [sp, #16 * 9]
    stp x20, x21, [sp, #16 * 10]
    stp x22, x23, [sp, #16 * 11]
    stp x24, x25, [sp, #16 * 12]
    stp x26, x27, [sp, #16 * 13]
    stp x28, x29, [sp, #16 * 14]
    str x30, [sp, #16 * 15]
    
    bl aarch64_serror_exception_handler
    b aarch64_exception_return

aarch64_sync_handler_a32:
    // Synchronous exception from lower EL using AArch32
    // This should not happen in 64-bit kernel
    b aarch64_panic_handler

aarch64_irq_handler_a32:
    // IRQ from lower EL using AArch32
    // This should not happen in 64-bit kernel
    b aarch64_panic_handler

aarch64_fiq_handler_a32:
    // FIQ from lower EL using AArch32
    // This should not happen in 64-bit kernel
    b aarch64_panic_handler

aarch64_serror_handler_a32:
    // SError from lower EL using AArch32
    // This should not happen in 64-bit kernel
    b aarch64_panic_handler

// ============================================================================
// SYSTEM CALL HANDLER
// ============================================================================

aarch64_syscall_handler:
    // System call from user space
    // x8 contains the system call number
    // x0-x7 contain the arguments
    
    // Call C system call handler
    bl aarch64_syscall_dispatcher
    
    // Return to user space
    b aarch64_exception_return

// ============================================================================
// EXCEPTION RETURN
// ============================================================================

aarch64_exception_return:
    // Restore registers
    ldp x0, x1, [sp, #16 * 0]
    ldp x2, x3, [sp, #16 * 1]
    ldp x4, x5, [sp, #16 * 2]
    ldp x6, x7, [sp, #16 * 3]
    ldp x8, x9, [sp, #16 * 4]
    ldp x10, x11, [sp, #16 * 5]
    ldp x12, x13, [sp, #16 * 6]
    ldp x14, x15, [sp, #16 * 7]
    ldp x16, x17, [sp, #16 * 8]
    ldp x18, x19, [sp, #16 * 9]
    ldp x20, x21, [sp, #16 * 10]
    ldp x22, x23, [sp, #16 * 11]
    ldp x24, x25, [sp, #16 * 12]
    ldp x26, x27, [sp, #16 * 13]
    ldp x28, x29, [sp, #16 * 14]
    ldr x30, [sp, #16 * 15]
    add sp, sp, #272
    
    eret

// ============================================================================
// PANIC HANDLER
// ============================================================================

aarch64_panic_handler:
    // Critical error - halt the system
    wfi
    b aarch64_panic_handler

// ============================================================================
// CONTEXT SWITCHING
// ============================================================================

.global aarch64_context_switch
aarch64_context_switch:
    // x0 = prev thread, x1 = next thread
    
    // Save current context
    stp x19, x20, [x0, #16 * 0]
    stp x21, x22, [x0, #16 * 1]
    stp x23, x24, [x0, #16 * 2]
    stp x25, x26, [x0, #16 * 3]
    stp x27, x28, [x0, #16 * 4]
    stp x29, x30, [x0, #16 * 5]
    mov x2, sp
    str x2, [x0, #16 * 6]
    
    // Restore next context
    ldp x19, x20, [x1, #16 * 0]
    ldp x21, x22, [x1, #16 * 1]
    ldp x23, x24, [x1, #16 * 2]
    ldp x25, x26, [x1, #16 * 3]
    ldp x27, x28, [x1, #16 * 4]
    ldp x29, x30, [x1, #16 * 5]
    ldr x2, [x1, #16 * 6]
    mov sp, x2
    
    ret

.global aarch64_enter_user_mode
aarch64_enter_user_mode:
    // x0 = user entry point, x1 = user stack, x2 = user data
    
    // Set up user stack
    msr sp_el0, x1
    
    // Set up user data pointer
    mov x3, x2
    
    // Set up return address for user space
    mov lr, x0
    
    // Set up user space entry point
    msr elr_el1, x0
    
    // Set up user space stack pointer
    msr sp_el0, x1
    
    // Set up user space data pointer
    mov x0, x2
    
    // Return to user space
    eret

// ============================================================================
// CACHE AND TLB MANAGEMENT
// ============================================================================

.global aarch64_tlb_flush_all
aarch64_tlb_flush_all:
    // Invalidate all TLB entries
    tlbi vmalle1
    
    // Ensure completion
    dsb ish
    isb
    
    ret

.global aarch64_tlb_flush_page
aarch64_tlb_flush_page:
    // x0 = virtual address to flush
    
    // Invalidate TLB entry for specific page
    tlbi vaae1, x0
    
    // Ensure completion
    dsb ish
    isb
    
    ret

.global aarch64_cache_flush_all
aarch64_cache_flush_all:
    // Flush all data caches
    dc civac, xzr
    
    // Ensure completion
    dsb ish
    isb
    
    ret

.global aarch64_cache_flush_page
aarch64_cache_flush_page:
    // x0 = address to flush
    
    // Flush data cache for specific page
    dc civac, x0
    
    // Ensure completion
    dsb ish
    isb
    
    ret

.global aarch64_cache_invalidate_page
aarch64_cache_invalidate_page:
    // x0 = address to invalidate
    
    // Invalidate data cache for specific page
    dc ivac, x0
    
    // Ensure completion
    dsb ish
    isb
    
    ret

.global aarch64_cache_clean_page
aarch64_cache_clean_page:
    // x0 = address to clean
    
    // Clean data cache for specific page
    dc cvac, x0
    
    // Ensure completion
    dsb ish
    isb
    
    ret

.global aarch64_icache_invalidate_all
aarch64_icache_invalidate_all:
    // Invalidate all instruction caches
    ic ialluis
    
    // Ensure completion
    dsb ish
    isb
    
    ret

.global aarch64_icache_invalidate_page
aarch64_icache_invalidate_page:
    // x0 = address to invalidate
    
    // Invalidate instruction cache for specific page
    ic ivau, x0
    
    // Ensure completion
    dsb ish
    isb
    
    ret

// ============================================================================
// MEMORY BARRIERS
// ============================================================================

.global aarch64_memory_barrier
aarch64_memory_barrier:
    // Full memory barrier
    dmb ish
    ret

.global aarch64_read_barrier
aarch64_read_barrier:
    // Read barrier
    dmb ishld
    ret

.global aarch64_write_barrier
aarch64_write_barrier:
    // Write barrier
    dmb ishst
    ret

.global aarch64_instruction_barrier
aarch64_instruction_barrier:
    // Instruction barrier
    isb
    ret

// ============================================================================
// ATOMIC OPERATIONS
// ============================================================================

.global aarch64_atomic_cas
aarch64_atomic_cas:
    // x0 = pointer, x1 = expected, x2 = desired
    // Returns old value in x0
    
    // Load-acquire, store-release compare-and-swap
    casal x1, x2, [x0]
    mov x0, x1
    ret

.global aarch64_atomic_add
aarch64_atomic_add:
    // x0 = pointer, x1 = value to add
    // Returns old value in x0
    
    // Atomic add
    ldaddal x1, x0, [x0]
    ret

.global aarch64_atomic_sub
aarch64_atomic_sub:
    // x0 = pointer, x1 = value to subtract
    // Returns old value in x0
    
    // Atomic subtract
    ldaddal x1, x0, [x0]
    neg x0, x0
    ret

.global aarch64_atomic_or
aarch64_atomic_or:
    // x0 = pointer, x1 = value to OR
    // Returns old value in x0
    
    // Atomic OR
    ldsetal x1, x0, [x0]
    ret

.global aarch64_atomic_and
aarch64_atomic_and:
    // x0 = pointer, x1 = value to AND
    // Returns old value in x0
    
    // Atomic AND
    ldclral x1, x0, [x0]
    ret

.global aarch64_atomic_xor
aarch64_atomic_xor:
    // x0 = pointer, x1 = value to XOR
    // Returns old value in x0
    
    // Atomic XOR
    ldeoral x1, x0, [x0]
    ret

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

.global aarch64_read_cpuid
aarch64_read_cpuid:
    // x0 = register to read
    mrs x0, x0
    ret

.global aarch64_write_cpuid
aarch64_write_cpuid:
    // x0 = register to write, x1 = value
    msr x0, x1
    ret

.global aarch64_read_msr
aarch64_read_msr:
    // x0 = register to read
    mrs x0, x0
    ret

.global aarch64_write_msr
aarch64_write_msr:
    // x0 = register to write, x1 = value
    msr x0, x1
    ret

.global aarch64_get_el
aarch64_get_el:
    // Get current exception level
    mrs x0, CurrentEL
    lsr x0, x0, #2
    ret

.global aarch64_get_cpu_id
aarch64_get_cpu_id:
    // Get current CPU ID
    mrs x0, MPIDR_EL1
    and x0, x0, #0xFF
    ret

.global aarch64_get_cluster_id
aarch64_get_cluster_id:
    // Get current cluster ID
    mrs x0, MPIDR_EL1
    lsr x0, x0, #8
    and x0, x0, #0xFF
    ret

// ============================================================================
// BOOT SUPPORT
// ============================================================================

.global aarch64_boot_init
aarch64_boot_init:
    // Early boot initialization
    
    // Set up exception vectors
    ldr x0, =aarch64_exception_vectors
    msr VBAR_EL1, x0
    
    // Set up stack
    ldr x0, =aarch64_boot_stack_top
    mov sp, x0
    
    // Clear BSS
    ldr x0, =__bss_start
    ldr x1, =__bss_end
    sub x1, x1, x0
    bl aarch64_memzero
    
    // Jump to C initialization
    bl aarch64_arch_init
    
    // Should not return
    b aarch64_panic_handler

// ============================================================================
// MEMORY OPERATIONS
// ============================================================================

aarch64_memzero:
    // x0 = start address, x1 = size
    cbz x1, 2f
1:  str xzr, [x0], #8
    subs x1, x1, #8
    b.hi 1b
2:  ret

.global aarch64_memcpy
aarch64_memcpy:
    // x0 = destination, x1 = source, x2 = size
    cbz x2, 2f
1:  ldrb w3, [x1], #1
    strb w3, [x0], #1
    subs x2, x2, #1
    b.ne 1b
2:  ret

.global aarch64_memset
aarch64_memset:
    // x0 = destination, x1 = value, x2 = size
    cbz x2, 2f
1:  strb w1, [x0], #1
    subs x2, x2, #1
    b.ne 1b
2:  ret

// ============================================================================
// STACK
// ============================================================================

.section .bss
.align 16

aarch64_boot_stack:
    .space 4096
aarch64_boot_stack_top:
