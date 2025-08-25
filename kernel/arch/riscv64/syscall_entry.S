/*
 * ORION OS - RISC-V 64-bit System Call Entry Points
 *
 * Assembly system call entry points and context switching for RISC-V 64-bit
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

.section .text
.global riscv64_syscall_entry
.global riscv64_fast_syscall_entry
.global riscv64_context_switch
.global riscv64_switch_to_kernel_mode
.global riscv64_switch_to_user_mode

// ============================================================================
// SYSTEM CALL ENTRY POINTS
// ============================================================================

// Standard system call entry point
riscv64_syscall_entry:
    // Save user context
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
    
    // Save user stack pointer
    csrr t0, sscratch
    sd t0, 248(sp)
    
    // Set kernel stack
    la t0, _kernel_stack_top
    csrw sscratch, t0
    mv sp, t0
    
    // Call system call dispatcher
    call riscv64_syscall_dispatcher
    
    // Restore user context
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
    
    // Restore user stack pointer
    ld t0, 248(sp)
    csrw sscratch, t0
    mv sp, t0
    addi sp, sp, 256
    
    // Return to user mode
    sret

// Fast system call entry point (for frequently used calls)
riscv64_fast_syscall_entry:
    // Save minimal context for fast path
    addi sp, sp, -64
    sd ra, 0(sp)
    sd a0, 8(sp)
    sd a1, 16(sp)
    sd a2, 24(sp)
    sd a3, 32(sp)
    sd a4, 40(sp)
    sd a5, 48(sp)
    sd a6, 56(sp)
    
    // Call fast system call handler
    call riscv64_fast_syscall_handler
    
    // Restore minimal context
    ld ra, 0(sp)
    ld a0, 8(sp)
    ld a1, 16(sp)
    ld a2, 24(sp)
    ld a3, 32(sp)
    ld a4, 40(sp)
    ld a5, 48(sp)
    ld a6, 56(sp)
    addi sp, sp, 64
    
    // Return to user mode
    sret

// ============================================================================
// CONTEXT SWITCHING
// ============================================================================

// Context switch between processes
// a0: pointer to current context
// a1: pointer to new context
riscv64_context_switch:
    // Save current context
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
    
    // Store current context
    mv t0, a0
    sd sp, 0(t0)  // Save stack pointer
    
    // Load new context
    mv sp, a1
    ld sp, 0(sp)  // Load stack pointer
    
    // Restore new context
    ld ra, 0(sp)
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
    
    // Return to new context
    ret

// ============================================================================
// MODE SWITCHING
// ============================================================================

// Switch to kernel mode
riscv64_switch_to_kernel_mode:
    // Save user registers
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
    
    // Switch to kernel stack
    la t0, _kernel_stack_top
    mv sp, t0
    
    // Set kernel mode flags
    li t0, 0x1800  // MPP = 01 (Supervisor mode)
    csrw mstatus, t0
    
    ret

// Switch to user mode
riscv64_switch_to_user_mode:
    // Restore user registers
    ld ra, 0(sp)
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
    
    // Restore user stack pointer
    ld sp, 8(sp)
    addi sp, sp, 256
    
    // Set user mode flags
    li t0, 0x2000  // SPIE = 1
    csrw sstatus, t0
    
    // Return to user mode
    sret

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Get current privilege level
.global riscv64_get_privilege_level
riscv64_get_privilege_level:
    csrr a0, mstatus
    srli a0, a0, 11  // Extract MPP bits
    andi a0, a0, 0x3
    ret

// Set interrupt enable/disable
.global riscv64_set_interrupt_enable
riscv64_set_interrupt_enable:
    // a0: interrupt number, a1: enable (1) or disable (0)
    beqz a1, 1f
    
    // Enable interrupt
    li t0, 1
    sll t0, t0, a0
    csrrs zero, mie, t0
    j 2f
    
1:  // Disable interrupt
    li t0, 1
    sll t0, t0, a0
    csrrc zero, mie, t0
    
2:  ret

// Memory barrier
.global riscv64_memory_barrier
riscv64_memory_barrier:
    fence rw, rw
    ret

// Atomic operations
.global riscv64_atomic_add
riscv64_atomic_add:
    // a0: address, a1: value to add
    amoadd.d a0, a1, (a0)
    ret

.global riscv64_atomic_swap
riscv64_atomic_swap:
    // a0: address, a1: new value
    amoswap.d a0, a1, (a0)
    ret

.global riscv64_atomic_compare_swap
riscv64_atomic_compare_swap:
    // a0: address, a1: expected value, a2: new value
    mv t0, a0
    mv t1, a1
    mv t2, a2
    lr.d a0, (t0)
    bne a0, t1, 1f
    sc.d t3, t2, (t0)
    bnez t3, riscv64_atomic_compare_swap
1:  ret

// ============================================================================
// INTERRUPT STATUS
// ============================================================================

// Get interrupt status
.global riscv64_get_interrupt_status
riscv64_get_interrupt_status:
    csrr a0, mip
    ret

// Get interrupt enable status
.global riscv64_get_interrupt_enable
riscv64_get_interrupt_enable:
    csrr a0, mie
    ret

// ============================================================================
// INTERRUPT HANDLER REGISTRATION
// ============================================================================

// Set interrupt handler
.global riscv64_set_interrupt_handler
riscv64_set_interrupt_handler:
    // a0: interrupt number, a1: handler address
    slli t0, a0, 3  // Multiply by 8 (pointer size)
    la t1, riscv64_interrupt_handler_table
    add t1, t1, t0
    sd a1, (t1)
    ret

// ============================================================================
// PERFORMANCE ALIASES
// ============================================================================

// Performance monitoring aliases
.global riscv64_read_cycle_counter
riscv64_read_cycle_counter:
    csrr a0, cycle
    ret

.global riscv64_read_instruction_counter
riscv64_read_instruction_counter:
    csrr a0, instret
    ret

.global riscv64_read_time_counter
riscv64_read_time_counter:
    csrr a0, time
    ret

// ============================================================================
// SYSTEM FUNCTIONS
// ============================================================================

// System reset
.global riscv64_system_reset
riscv64_system_reset:
    // Trigger system reset
    li t0, 0x100000
    sw zero, (t0)
    j .

// System halt
.global riscv64_system_halt
riscv64_system_halt:
    // Halt the system
    wfi
    j .

// ============================================================================
// CONTEXT FUNCTIONS
// ============================================================================

// Get context size
.global riscv64_get_context_size
riscv64_get_context_size:
    li a0, 256  // Context size in bytes
    ret

// Initialize context
.global riscv64_init_context
riscv64_init_context:
    // a0: context pointer
    mv t0, a0
    li t1, 0
    li t2, 256
1:  sb t1, (t0)
    addi t0, t0, 1
    addi t2, t2, -1
    bnez t2, 1b
    ret

// ============================================================================
// CPU MODE AND PRIVILEGE
// ============================================================================

// Get CPU mode
.global riscv64_get_cpu_mode
riscv64_get_cpu_mode:
    csrr a0, mstatus
    srli a0, a0, 11  // Extract MPP bits
    andi a0, a0, 0x3
    ret

// Check if privileged
.global riscv64_is_privileged
riscv64_is_privileged:
    csrr a0, mstatus
    srli a0, a0, 11  // Extract MPP bits
    andi a0, a0, 0x3
    snez a0, a0  // Return 1 if privileged, 0 if not
    ret

// ============================================================================
// MEMORY MANAGEMENT
// ============================================================================

// Flush TLB
.global riscv64_flush_tlb
riscv64_flush_tlb:
    sfence.vma
    ret

// Flush instruction cache
.global riscv64_flush_icache
riscv64_flush_icache:
    fence.i
    ret

// ============================================================================
// STACK SPACE
// ============================================================================

.section .bss
.align 16
_kernel_stack:
    .space 16384  // 16KB kernel stack
_kernel_stack_top:

// Interrupt handler table
.section .data
.align 8
riscv64_interrupt_handler_table:
    .quad 0  // interrupt_0
    .quad 0  // interrupt_1
    .quad 0  // interrupt_2
    .quad 0  // interrupt_3
    .quad 0  // interrupt_4
    .quad 0  // interrupt_5
    .quad 0  // interrupt_6
    .quad 0  // interrupt_7
    .quad 0  // interrupt_8
    .quad 0  // interrupt_9
    .quad 0  // interrupt_10
    .quad 0  // interrupt_11
    .quad 0  // interrupt_12
    .quad 0  // interrupt_13
    .quad 0  // interrupt_14
    .quad 0  // interrupt_15
