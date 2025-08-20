/*
 * Orion Operating System - x86_64 System Call Entry Point
 *
 * Fast system call entry using SYSCALL/SYSRET instructions.
 * Implements the x86_64 System V ABI calling convention.
 *
 * Developed by Jérémy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

.section .text
.global syscall_entry

/*
 * System call entry point
 * 
 * Register layout on entry (System V ABI):
 * - RAX: syscall number
 * - RDI: arg0
 * - RSI: arg1  
 * - RDX: arg2
 * - R10: arg3 (RCX is clobbered by SYSCALL)
 * - R8:  arg4
 * - R9:  arg5
 *
 * We need to save/restore registers and call the C syscall handler.
 */
syscall_entry:
    /* Save user registers */
    push %rcx        /* RIP to return to */
    push %r11        /* RFLAGS to restore */
    push %rbp
    push %rbx
    push %r12
    push %r13
    push %r14
    push %r15
    
    /* Save user stack pointer */
    mov %rsp, %rbp
    
    /* Switch to kernel stack if needed */
    /* TODO: Implement per-CPU kernel stack */
    
    /* Prepare arguments for C function call */
    /* syscall_handler(uint64_t syscall_num, uint64_t arg0, uint64_t arg1, 
                       uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) */
    mov %rax, %rdi   /* syscall_num -> arg0 */
    mov %rdi, %rsi   /* arg0 -> arg1 */
    mov %rsi, %rdx   /* arg1 -> arg2 */
    mov %rdx, %rcx   /* arg2 -> arg3 */
    mov %r10, %r8    /* arg3 -> arg4 (r10 because rcx was clobbered) */
    mov %r8, %r9     /* arg4 -> arg5 */
    push %r9         /* arg5 -> stack (7th argument) */
    
    /* Call the C syscall handler */
    call syscall_handler
    
    /* Clean up stack argument */
    add $8, %rsp
    
    /* Result is in RAX, keep it there for return */
    
    /* Restore user registers */
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %rbx
    pop %rbp
    pop %r11         /* RFLAGS */
    pop %rcx         /* RIP */
    
    /* Return to user space */
    sysretq

/*
 * System call handler C function prototype:
 * extern uint64_t syscall_handler(uint64_t syscall_num, uint64_t arg0, uint64_t arg1,
 *                                 uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);
 */
