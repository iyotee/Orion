/*
 * ORION OS - LoongArch System Call Entry Point
 *
 * This file provides the low-level entry point and dispatcher for system calls
 * on LoongArch architecture.
 */

#include "arch.h"

/* ============================================================================
 * System Call Numbers
 * ============================================================================ */

/* Standard system calls */
#define LOONGARCH_SYS_READ        0
#define LOONGARCH_SYS_WRITE       1
#define LOONGARCH_SYS_OPEN        2
#define LOONGARCH_SYS_CLOSE       3
#define LOONGARCH_SYS_EXECVE      59
#define LOONGARCH_SYS_EXIT        60
#define LOONGARCH_SYS_FORK        57
#define LOONGARCH_SYS_GETPID      39
#define LOONGARCH_SYS_SLEEP       35

/* ORION-specific system calls */
#define LOONGARCH_SYS_ORION_GET_INFO    1000
#define LOONGARCH_SYS_ORION_SET_CONFIG  1001
#define LOONGARCH_SYS_ORION_GET_STATUS  1002
#define LOONGARCH_SYS_ORION_PERFORMANCE 1003

/* ============================================================================
 * System Call Entry Point
 * ============================================================================ */

.section .text
.align 4
.global loongarch_syscall_entry
.type loongarch_syscall_entry, @function

loongarch_syscall_entry:
    /* Save all general-purpose registers */
    addi.d $sp, $sp, -256
    st.d $ra, $sp, 0
    st.d $tp, $sp, 8
    st.d $sp, $sp, 16
    st.d $gp, $sp, 24
    st.d $t0, $sp, 32
    st.d $t1, $sp, 40
    st.d $t2, $sp, 48
    st.d $t3, $sp, 56
    st.d $t4, $sp, 64
    st.d $t5, $sp, 72
    st.d $t6, $sp, 80
    st.d $t7, $sp, 88
    st.d $t8, $sp, 96
    st.d $a0, $sp, 104
    st.d $a1, $sp, 112
    st.d $a2, $sp, 120
    st.d $a3, $sp, 128
    st.d $a4, $sp, 136
    st.d $a5, $sp, 144
    st.d $a6, $sp, 152
    st.d $a7, $sp, 160
    st.d $s0, $sp, 168
    st.d $s1, $sp, 176
    st.d $s2, $sp, 184
    st.d $s3, $sp, 192
    st.d $s4, $sp, 200
    st.d $s5, $sp, 208
    st.d $s6, $sp, 216
    st.d $s7, $sp, 224
    st.d $s8, $sp, 232
    st.d $fp, $sp, 240

    /* Extract system call number from $a7 */
    move $t0, $a7

    /* Extract arguments from $a0-$a6 */
    move $a0, $a0  /* First argument */
    move $a1, $a1  /* Second argument */
    move $a2, $a2  /* Third argument */
    move $a3, $a3  /* Fourth argument */
    move $a4, $a4  /* Fifth argument */
    move $a5, $a5  /* Sixth argument */
    move $a6, $a6  /* Seventh argument */

    /* Dispatch to appropriate system call handler */
    la.global $t1, loongarch_syscall_table
    slli.d $t2, $t0, 3  /* Multiply by 8 (pointer size) */
    add.d $t1, $t1, $t2
    ld.d $t1, $t1, 0

    /* Call the system call handler */
    jalr $t1

    /* Restore all registers */
    ld.d $ra, $sp, 0
    ld.d $tp, $sp, 8
    ld.d $sp, $sp, 16
    ld.d $gp, $sp, 24
    ld.d $t0, $sp, 32
    ld.d $t1, $sp, 40
    ld.d $t2, $sp, 48
    ld.d $t3, $sp, 56
    ld.d $t4, $sp, 64
    ld.d $t5, $sp, 72
    ld.d $t6, $sp, 80
    ld.d $t7, $sp, 88
    ld.d $t8, $sp, 96
    ld.d $a0, $sp, 104
    ld.d $a1, $sp, 112
    ld.d $a2, $sp, 120
    ld.d $a3, $sp, 128
    ld.d $a4, $sp, 136
    ld.d $a5, $sp, 144
    ld.d $a6, $sp, 152
    ld.d $a7, $sp, 160
    ld.d $s0, $sp, 168
    ld.d $s1, $sp, 176
    ld.d $s2, $sp, 184
    ld.d $s3, $sp, 192
    ld.d $s4, $sp, 200
    ld.d $s5, $sp, 208
    ld.d $s6, $sp, 216
    ld.d $s7, $sp, 224
    ld.d $s8, $sp, 232
    ld.d $fp, $sp, 240
    addi.d $sp, $sp, 256

    /* Return from system call */
    ertn

/* ============================================================================
 * System Call Table
 * ============================================================================ */

.section .data
.align 8

loongarch_syscall_table:
    .quad loongarch_syscall_read      /* 0 */
    .quad loongarch_syscall_write     /* 1 */
    .quad loongarch_syscall_open      /* 2 */
    .quad loongarch_syscall_close     /* 3 */
    .quad loongarch_syscall_invalid  /* 4 */
    .quad loongarch_syscall_invalid  /* 5 */
    .quad loongarch_syscall_invalid  /* 6 */
    .quad loongarch_syscall_invalid  /* 7 */
    .quad loongarch_syscall_invalid  /* 8 */
    .quad loongarch_syscall_invalid  /* 9 */
    .quad loongarch_syscall_invalid  /* 10 */
    .quad loongarch_syscall_invalid  /* 11 */
    .quad loongarch_syscall_invalid  /* 12 */
    .quad loongarch_syscall_invalid  /* 13 */
    .quad loongarch_syscall_invalid  /* 14 */
    .quad loongarch_syscall_invalid  /* 15 */
    .quad loongarch_syscall_invalid  /* 16 */
    .quad loongarch_syscall_invalid  /* 17 */
    .quad loongarch_syscall_invalid  /* 18 */
    .quad loongarch_syscall_invalid  /* 19 */
    .quad loongarch_syscall_invalid  /* 20 */
    .quad loongarch_syscall_invalid  /* 21 */
    .quad loongarch_syscall_invalid  /* 22 */
    .quad loongarch_syscall_invalid  /* 23 */
    .quad loongarch_syscall_invalid  /* 24 */
    .quad loongarch_syscall_invalid  /* 25 */
    .quad loongarch_syscall_invalid  /* 26 */
    .quad loongarch_syscall_invalid  /* 27 */
    .quad loongarch_syscall_invalid  /* 28 */
    .quad loongarch_syscall_invalid  /* 29 */
    .quad loongarch_syscall_invalid  /* 30 */
    .quad loongarch_syscall_invalid  /* 31 */
    .quad loongarch_syscall_invalid  /* 32 */
    .quad loongarch_syscall_invalid  /* 33 */
    .quad loongarch_syscall_invalid  /* 34 */
    .quad loongarch_syscall_sleep    /* 35 */
    .quad loongarch_syscall_invalid  /* 36 */
    .quad loongarch_syscall_invalid  /* 37 */
    .quad loongarch_syscall_invalid  /* 38 */
    .quad loongarch_syscall_getpid   /* 39 */
    .quad loongarch_syscall_invalid  /* 40 */
    .quad loongarch_syscall_invalid  /* 41 */
    .quad loongarch_syscall_invalid  /* 42 */
    .quad loongarch_syscall_invalid  /* 43 */
    .quad loongarch_syscall_invalid  /* 44 */
    .quad loongarch_syscall_invalid  /* 45 */
    .quad loongarch_syscall_invalid  /* 46 */
    .quad loongarch_syscall_invalid  /* 47 */
    .quad loongarch_syscall_invalid  /* 48 */
    .quad loongarch_syscall_invalid  /* 49 */
    .quad loongarch_syscall_invalid  /* 50 */
    .quad loongarch_syscall_invalid  /* 51 */
    .quad loongarch_syscall_invalid  /* 52 */
    .quad loongarch_syscall_invalid  /* 53 */
    .quad loongarch_syscall_invalid  /* 54 */
    .quad loongarch_syscall_invalid  /* 55 */
    .quad loongarch_syscall_invalid  /* 56 */
    .quad loongarch_syscall_fork     /* 57 */
    .quad loongarch_syscall_invalid  /* 58 */
    .quad loongarch_syscall_execve   /* 59 */
    .quad loongarch_syscall_exit     /* 60 */

/* ============================================================================
 * System Call Handlers
 * ============================================================================ */

.section .text
.align 4

/* Invalid system call handler */
loongarch_syscall_invalid:
    li.d $a0, -1  /* Return error */
    j loongarch_syscall_return

/* Read system call */
loongarch_syscall_read:
    /* Placeholder for read implementation */
    li.d $a0, 0   /* Return success */
    j loongarch_syscall_return

/* Write system call */
loongarch_syscall_write:
    /* Placeholder for write implementation */
    li.d $a0, 0   /* Return success */
    j loongarch_syscall_return

/* Open system call */
loongarch_syscall_open:
    /* Placeholder for open implementation */
    li.d $a0, 0   /* Return success */
    j loongarch_syscall_return

/* Close system call */
loongarch_syscall_close:
    /* Placeholder for close implementation */
    li.d $a0, 0   /* Return success */
    j loongarch_syscall_return

/* Sleep system call */
loongarch_syscall_sleep:
    /* Placeholder for sleep implementation */
    li.d $a0, 0   /* Return success */
    j loongarch_syscall_return

/* Get PID system call */
loongarch_syscall_getpid:
    /* Placeholder for getpid implementation */
    li.d $a0, 1   /* Return PID 1 */
    j loongarch_syscall_return

/* Fork system call */
loongarch_syscall_fork:
    /* Placeholder for fork implementation */
    li.d $a0, 0   /* Return success */
    j loongarch_syscall_return

/* Execve system call */
loongarch_syscall_execve:
    /* Placeholder for execve implementation */
    li.d $a0, 0   /* Return success */
    j loongarch_syscall_return

/* Exit system call */
loongarch_syscall_exit:
    /* Placeholder for exit implementation */
    li.d $a0, 0   /* Return success */
    j loongarch_syscall_return

/* Common return path */
loongarch_syscall_return:
    ret

/* ============================================================================
 * System Call Statistics
 * ============================================================================ */

.section .data
.align 8

loongarch_syscall_stats:
    .quad 0  /* Total system calls */
    .quad 0  /* Invalid system calls */
    .quad 0  /* Successful system calls */

.size loongarch_syscall_entry, .-loongarch_syscall_entry
