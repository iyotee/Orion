/*
 * ORION OS - RISC-V 64-bit Boot Sequence
 *
 * Assembly boot code for RISC-V 64-bit architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

.section .text.boot
.global _start
.type _start, @function

_start:
    // Set up stack pointer
    la sp, _stack_top
    
    // Clear BSS section
    la t0, _bss_start
    la t1, _bss_end
    bgeu t0, t1, 2f
1:
    sd zero, (t0)
    addi t0, t0, 8
    bltu t0, t1, 1b
2:
    
    // Set up exception vectors
    la t0, riscv64_exception_vector_table
    csrw mtvec, t0
    
    // Disable all interrupts initially
    csrw mie, zero
    csrw mip, zero
    
    // Set up supervisor mode
    li t0, 0x1800  // MPP = 01 (Supervisor mode)
    csrw mstatus, t0
    
    // Set up supervisor trap vector
    la t0, riscv64_supervisor_trap_vector
    csrw stvec, t0
    
    // Set up supervisor interrupt enable
    li t0, 0x222   // SSIE, STIE, SEIE
    csrw sie, t0
    
    // Set up supervisor status
    li t0, 0x2000  // SPIE = 1
    csrw sstatus, t0
    
    // Jump to C entry point
    call riscv64_arch_init
    
    // Should never return
    j .

// Exception vector table
.align 4
riscv64_exception_vector_table:
    j riscv64_exception_handler
    j riscv64_exception_handler
    j riscv64_exception_handler
    j riscv64_exception_handler
    j riscv64_exception_handler
    j riscv64_exception_handler
    j riscv64_exception_handler
    j riscv64_exception_handler
    j riscv64_exception_handler
    j riscv64_exception_handler
    j riscv64_exception_handler
    j riscv64_exception_handler
    j riscv64_exception_handler
    j riscv64_exception_handler
    j riscv64_exception_handler
    j riscv64_exception_handler

// Supervisor trap vector
.align 4
riscv64_supervisor_trap_vector:
    j riscv64_supervisor_trap_handler

// Exception handler stub
riscv64_exception_handler:
    // Save registers
    addi sp, sp, -256
    sd ra, 0(sp)
    sd sp, 8(sp)
    sd gp, 16(sp)
    sd tp, 24(sp)
    sd t0, 32(sp)
    sd t1, 40(sp)
    sd t2, 48(sp)
    sd s0, 56(sp)
    sd s1, 64(sp)
    sd a0, 72(sp)
    sd a1, 80(sp)
    sd a2, 88(sp)
    sd a3, 96(sp)
    sd a4, 104(sp)
    sd a5, 112(sp)
    sd a6, 120(sp)
    sd a7, 128(sp)
    sd s2, 136(sp)
    sd s3, 144(sp)
    sd s4, 152(sp)
    sd s5, 160(sp)
    sd s6, 168(sp)
    sd s7, 176(sp)
    sd s8, 184(sp)
    sd s9, 192(sp)
    sd s10, 200(sp)
    sd s11, 208(sp)
    sd t3, 216(sp)
    sd t4, 224(sp)
    sd t5, 232(sp)
    sd t6, 240(sp)
    
    // Get exception cause and PC
    csrr a0, mcause
    csrr a1, mepc
    csrr a2, mtval
    
    // Call C exception handler
    call riscv64_sync_exception_handler
    
    // Restore registers
    ld ra, 0(sp)
    ld sp, 8(sp)
    ld gp, 16(sp)
    ld tp, 24(sp)
    ld t0, 32(sp)
    ld t1, 40(sp)
    ld t2, 48(sp)
    ld s0, 56(sp)
    ld s1, 64(sp)
    ld a0, 72(sp)
    ld a1, 80(sp)
    ld a2, 88(sp)
    ld a3, 96(sp)
    ld a4, 104(sp)
    ld a5, 112(sp)
    ld a6, 120(sp)
    ld a7, 128(sp)
    ld s2, 136(sp)
    ld s3, 144(sp)
    ld s4, 152(sp)
    ld s5, 160(sp)
    ld s6, 168(sp)
    ld s7, 176(sp)
    ld s8, 184(sp)
    ld s9, 192(sp)
    ld s10, 200(sp)
    ld s11, 208(sp)
    ld t3, 216(sp)
    ld t4, 224(sp)
    ld t5, 232(sp)
    ld t6, 240(sp)
    addi sp, sp, 256
    
    // Return from exception
    mret

// Supervisor trap handler stub
riscv64_supervisor_trap_handler:
    // Save registers
    addi sp, sp, -256
    sd ra, 0(sp)
    sd sp, 8(sp)
    sd gp, 16(sp)
    sd tp, 24(sp)
    sd t0, 32(sp)
    sd t1, 40(sp)
    sd t2, 48(sp)
    sd s0, 56(sp)
    sd s1, 64(sp)
    sd a0, 72(sp)
    sd a1, 80(sp)
    sd a2, 88(sp)
    sd a3, 96(sp)
    sd a4, 104(sp)
    sd a5, 112(sp)
    sd a6, 120(sp)
    sd a7, 128(sp)
    sd s2, 136(sp)
    sd s3, 144(sp)
    sd s4, 152(sp)
    sd s5, 160(sp)
    sd s6, 168(sp)
    sd s7, 176(sp)
    sd s8, 184(sp)
    sd s9, 192(sp)
    sd s10, 200(sp)
    sd s11, 208(sp)
    sd t3, 216(sp)
    sd t4, 224(sp)
    sd t5, 232(sp)
    sd t6, 240(sp)
    
    // Get trap cause and PC
    csrr a0, scause
    csrr a1, sepc
    csrr a2, stval
    
    // Check if it's a system call
    li t0, 8
    beq a0, t0, riscv64_syscall_dispatcher
    
    // Call C trap handler
    call riscv64_supervisor_trap_handler_c
    
    // Restore registers
    ld ra, 0(sp)
    ld sp, 8(sp)
    ld gp, 16(sp)
    ld tp, 24(sp)
    ld t0, 32(sp)
    ld t1, 40(sp)
    ld t2, 48(sp)
    ld s0, 56(sp)
    ld s1, 64(sp)
    ld a0, 72(sp)
    ld a1, 80(sp)
    ld a2, 88(sp)
    ld a3, 96(sp)
    ld a4, 104(sp)
    ld a5, 112(sp)
    ld a6, 120(sp)
    ld a7, 128(sp)
    ld s2, 136(sp)
    ld s3, 144(sp)
    ld s4, 152(sp)
    ld s5, 160(sp)
    ld s6, 168(sp)
    ld s7, 176(sp)
    ld s8, 184(sp)
    ld s9, 192(sp)
    ld s10, 200(sp)
    ld s11, 208(sp)
    ld t3, 216(sp)
    ld t4, 224(sp)
    ld t5, 232(sp)
    ld t6, 240(sp)
    addi sp, sp, 256
    
    // Return from trap
    sret

// System call dispatcher
riscv64_syscall_dispatcher:
    // System call number is in a7
    mv t0, a7
    
    // Check system call number range
    li t1, 64  // Maximum system call number
    bgeu t0, t1, riscv64_invalid_syscall
    
    // Load system call handler address
    la t1, riscv64_syscall_table
    slli t2, t0, 3  // Multiply by 8 (pointer size)
    add t1, t1, t2
    ld t1, (t1)
    
    // Call system call handler
    jalr ra, t1, 0
    
    // Return to user mode
    j riscv64_syscall_return

riscv64_invalid_syscall:
    li a0, -1  // Invalid system call
    li a1, 0

riscv64_syscall_return:
    // Restore registers and return
    ld ra, 0(sp)
    ld sp, 8(sp)
    ld gp, 16(sp)
    ld tp, 24(sp)
    ld t0, 32(sp)
    ld t1, 40(sp)
    ld t2, 48(sp)
    ld s0, 56(sp)
    ld s1, 64(sp)
    ld a0, 72(sp)
    ld a1, 80(sp)
    ld a2, 88(sp)
    ld a3, 96(sp)
    ld a4, 104(sp)
    ld a5, 112(sp)
    ld a6, 120(sp)
    ld a7, 128(sp)
    ld s2, 136(sp)
    ld s3, 144(sp)
    ld s4, 152(sp)
    ld s5, 160(sp)
    ld s6, 168(sp)
    ld s7, 176(sp)
    ld s8, 184(sp)
    ld s9, 192(sp)
    ld s10, 200(sp)
    ld s11, 208(sp)
    ld t3, 216(sp)
    ld t4, 224(sp)
    ld t5, 232(sp)
    ld t6, 240(sp)
    addi sp, sp, 256
    
    sret

// System call table (filled by C code)
.section .data
.align 8
riscv64_syscall_table:
    .quad 0  // syscall_0
    .quad 0  // syscall_1
    .quad 0  // syscall_2
    .quad 0  // syscall_3
    .quad 0  // syscall_4
    .quad 0  // syscall_5
    .quad 0  // syscall_6
    .quad 0  // syscall_7
    .quad 0  // syscall_8
    .quad 0  // syscall_9
    .quad 0  // syscall_10
    .quad 0  // syscall_11
    .quad 0  // syscall_12
    .quad 0  // syscall_13
    .quad 0  // syscall_14
    .quad 0  // syscall_15
    .quad 0  // syscall_16
    .quad 0  // syscall_17
    .quad 0  // syscall_18
    .quad 0  // syscall_19
    .quad 0  // syscall_20
    .quad 0  // syscall_21
    .quad 0  // syscall_22
    .quad 0  // syscall_23
    .quad 0  // syscall_24
    .quad 0  // syscall_25
    .quad 0  // syscall_26
    .quad 0  // syscall_27
    .quad 0  // syscall_28
    .quad 0  // syscall_29
    .quad 0  // syscall_30
    .quad 0  // syscall_31
    .quad 0  // syscall_32
    .quad 0  // syscall_33
    .quad 0  // syscall_34
    .quad 0  // syscall_35
    .quad 0  // syscall_36
    .quad 0  // syscall_37
    .quad 0  // syscall_38
    .quad 0  // syscall_39
    .quad 0  // syscall_40
    .quad 0  // syscall_41
    .quad 0  // syscall_42
    .quad 0  // syscall_43
    .quad 0  // syscall_44
    .quad 0  // syscall_45
    .quad 0  // syscall_46
    .quad 0  // syscall_47
    .quad 0  // syscall_48
    .quad 0  // syscall_49
    .quad 0  // syscall_50
    .quad 0  // syscall_51
    .quad 0  // syscall_52
    .quad 0  // syscall_53
    .quad 0  // syscall_54
    .quad 0  // syscall_55
    .quad 0  // syscall_56
    .quad 0  // syscall_57
    .quad 0  // syscall_58
    .quad 0  // syscall_59
    .quad 0  // syscall_60
    .quad 0  // syscall_61
    .quad 0  // syscall_62
    .quad 0  // syscall_63

// Stack space
.section .bss
.align 16
_stack:
    .space 16384  // 16KB stack
_stack_top:

// BSS section markers
.section .bss
.align 8
_bss_start:
_bss_end:
