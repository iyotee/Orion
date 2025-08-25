/*
 * ORION OS - LoongArch Boot Assembly
 * 
 * This file handles the early boot process for LoongArch systems.
 */

#include "arch.h"

.section .text.boot
.align 4

.global _start
.type _start, @function
_start:
    /* Disable interrupts and exceptions */
    csrwr $zero, 0x6    /* ECFG - disable all interrupts */
    csrwr $zero, 0x7    /* ESTAT - clear exception status */
    
    /* Set up initial stack pointer */
    la $sp, _kernel_stack_end
    
    /* Clear BSS section */
    la $t0, _kernel_bss_start
    la $t1, _kernel_bss_end
1:
    st.d $zero, $t0, 0
    addi.d $t0, $t0, 8
    blt $t0, $t1, 1b
    
    /* Set up exception vectors */
    la $t0, loongarch_exception_vectors
    csrwr $t0, 0x8      /* EENTRY - exception entry point */
    
    /* Configure initial MMU */
    /* Set up direct mapping windows */
    la $t0, 0x9000000000000000  /* Kernel base address */
    csrwr $t0, 0x180    /* DMW0 - direct mapping window 0 */
    la $t0, 0x0000000000000000  /* Physical base address */
    csrwr $t0, 0x181    /* DMW1 - direct mapping window 1 */
    
    /* Enable MMU */
    csrrd $t0, 0x3      /* Read CRMD */
    ori $t0, $t0, 0x1   /* Set PG bit */
    csrwr $t0, 0x3      /* Write CRMD */
    
    /* Jump to C code */
    j loongarch_boot_init

.size _start, .-_start

/* Early exception handler */
.global loongarch_early_exception
.type loongarch_early_exception, @function
loongarch_early_exception:
    /* Early exception handling during boot */
    /* Just halt the system for now */
1:
    j 1b
.size loongarch_early_exception, .-loongarch_early_exception

/* Early interrupt handler */
.global loongarch_early_interrupt
.type loongarch_early_interrupt, @function
loongarch_early_interrupt:
    /* Early interrupt handling during boot */
    /* Just return for now */
    ertn
.size loongarch_early_interrupt, .-loongarch_early_interrupt

/* CPU initialization */
.global loongarch_cpu_early_init
.type loongarch_cpu_early_init, @function
loongarch_cpu_early_init:
    /* Early CPU initialization */
    
    /* Set up initial CPU state */
    csrrd $t0, 0x3      /* Read CRMD */
    andi $t0, $t0, ~0x7 /* Clear PLV bits */
    ori $t0, $t0, 0x0   /* Set PLV to 0 (kernel mode) */
    csrwr $t0, 0x3      /* Write CRMD */
    
    /* Enable basic CPU features */
    csrrd $t0, 0x5      /* Read EUEN */
    ori $t0, $t0, 0x1   /* Enable LSX if available */
    csrwr $t0, 0x5      /* Write EUEN */
    
    /* Set up initial exception handling */
    la $t0, loongarch_early_exception
    csrwr $t0, 0x8      /* EENTRY */
    
    jr $ra
.size loongarch_cpu_early_init, .-loongarch_cpu_early_init

/* Memory initialization */
.global loongarch_memory_early_init
.type loongarch_memory_early_init, @function
loongarch_memory_early_init:
    /* Early memory initialization */
    
    /* Set up initial page tables */
    /* TODO: Implement basic page table setup */
    
    /* Configure TLB */
    /* TODO: Implement TLB configuration */
    
    /* Set up memory windows */
    la $t0, 0x9000000000000000  /* Kernel virtual base */
    csrwr $t0, 0x180    /* DMW0 */
    la $t0, 0x0000000000000000  /* Physical base */
    csrwr $t0, 0x181    /* DMW1 */
    
    jr $ra
.size loongarch_memory_early_init, .-loongarch_memory_early_init

/* Interrupt initialization */
.global loongarch_interrupt_early_init
.type loongarch_interrupt_early_init, @function
loongarch_interrupt_early_init:
    /* Early interrupt initialization */
    
    /* Disable all interrupts initially */
    csrwr $zero, 0x6    /* ECFG */
    csrwr $zero, 0x7    /* ESTAT */
    
    /* Set up basic exception vectors */
    la $t0, loongarch_early_exception
    csrwr $t0, 0x8      /* EENTRY */
    
    /* Configure basic interrupt handling */
    /* TODO: Implement basic interrupt controller setup */
    
    jr $ra
.size loongarch_interrupt_early_init, .-loongarch_interrupt_early_init

/* Timer initialization */
.global loongarch_timer_early_init
.type loongarch_timer_early_init, @function
loongarch_timer_early_init:
    /* Early timer initialization */
    
    /* Configure basic timer */
    /* TODO: Implement basic timer setup */
    
    /* Set up timer interrupt */
    /* TODO: Configure timer interrupt */
    
    jr $ra
.size loongarch_timer_early_init, .-loongarch_timer_early_init

/* Cache initialization */
.global loongarch_cache_early_init
.type loongarch_cache_early_init, @function
loongarch_cache_early_init:
    /* Early cache initialization */
    
    /* Flush and invalidate caches */
    csrrd $t0, 0x21     /* Read ICACHE */
    andi $t0, $t0, 0x1  /* Check if I-cache is enabled */
    beqz $t0, 1f        /* Skip if I-cache disabled */
    
    /* Flush I-cache */
    csrwr $zero, 0x22   /* ICACHEOP */
    ori $t0, $zero, 0x1 /* Set flush bit */
    csrwr $t0, 0x22     /* ICACHEOP */
    
1:
    csrrd $t0, 0x23     /* Read DCACHE */
    andi $t0, $t0, 0x1  /* Check if D-cache is enabled */
    beqz $t0, 2f        /* Skip if D-cache disabled */
    
    /* Flush D-cache */
    csrwr $zero, 0x24   /* DCACHEOP */
    ori $t0, $zero, 0x1 /* Set flush bit */
    csrwr $t0, 0x24     /* DCACHEOP */
    
2:
    jr $ra
.size loongarch_cache_early_init, .-loongarch_cache_early_init

/* Secondary CPU boot */
.global loongarch_boot_secondary_cpu
.type loongarch_boot_secondary_cpu, @function
loongarch_boot_secondary_cpu:
    /* Boot secondary CPU */
    /* a0: CPU ID, a1: entry point */
    
    /* Set up secondary CPU stack */
    la $t0, _kernel_stack_end
    slli.d $t1, $a0, 16  /* 64KB per CPU stack */
    sub.d $sp, $t0, $t1
    
    /* Set up secondary CPU exception vectors */
    la $t0, loongarch_exception_vectors
    csrwr $t0, 0x8      /* EENTRY */
    
    /* Enable MMU */
    csrrd $t0, 0x3      /* Read CRMD */
    ori $t0, $t0, 0x1   /* Set PG bit */
    csrwr $t0, 0x3      /* Write CRMD */
    
    /* Jump to entry point */
    jr $a1
.size loongarch_boot_secondary_cpu, .-loongarch_boot_secondary_cpu

/* CPU halt */
.global loongarch_cpu_halt
.type loongarch_cpu_halt, @function
loongarch_cpu_halt:
    /* Halt the CPU */
    /* Disable interrupts */
    csrwr $zero, 0x6    /* ECFG */
    
    /* Enter low-power mode */
1:
    idle 0
    j 1b
.size loongarch_cpu_halt, .-loongarch_cpu_halt

/* CPU reset */
.global loongarch_cpu_reset
.type loongarch_cpu_reset, @function
loongarch_cpu_reset:
    /* Reset the CPU */
    break 0
.size loongarch_cpu_reset, .-loongarch_cpu_reset

/* Memory barriers */
.global loongarch_memory_barrier_early
.type loongarch_memory_barrier_early, @function
loongarch_memory_barrier_early:
    /* Early memory barrier */
    dbar 0
    jr $ra
.size loongarch_memory_barrier_early, .-loongarch_memory_barrier_early

.global loongarch_read_barrier_early
.type loongarch_read_barrier_early, @function
loongarch_read_barrier_early:
    /* Early read barrier */
    dbar 0
    jr $ra
.size loongarch_read_barrier_early, .-loongarch_read_barrier_early

.global loongarch_write_barrier_early
.type loongarch_write_barrier_early, @function
loongarch_write_barrier_early:
    /* Early write barrier */
    dbar 0
    jr $ra
.size loongarch_write_barrier_early, .-loongarch_write_barrier_early

/* CPU feature detection */
.global loongarch_detect_features_early
.type loongarch_detect_features_early, @function
loongarch_detect_features_early:
    /* Early CPU feature detection */
    
    /* Read CPU ID */
    csrrd $t0, 0x1A     /* CPUCFG.CPUID */
    csrrd $t1, 0x1B     /* CPUCFG.PRID */
    
    /* Read CPU configuration */
    csrrd $t2, 0x1C     /* CPUCFG.CFG */
    
    /* Store features in global variables */
    /* TODO: Store detected features */
    
    jr $ra
.size loongarch_detect_features_early, .-loongarch_detect_features_early

/* System call entry */
.global loongarch_syscall_entry
.type loongarch_syscall_entry, @function
loongarch_syscall_entry:
    /* System call entry point */
    /* Save registers */
    addi.d $sp, $sp, -280
    st.d $ra, $sp, 0
    st.d $tp, $sp, 8
    st.d $a0, $sp, 24
    st.d $a1, $sp, 32
    st.d $a2, $sp, 40
    st.d $a3, $sp, 48
    st.d $a4, $sp, 56
    st.d $a5, $sp, 64
    st.d $a6, $sp, 72
    st.d $a7, $sp, 80
    st.d $t0, $sp, 88
    st.d $t1, $sp, 96
    st.d $t2, $sp, 104
    st.d $t3, $sp, 112
    st.d $t4, $sp, 120
    st.d $t5, $sp, 128
    st.d $t6, $sp, 136
    st.d $t7, $sp, 144
    st.d $t8, $sp, 152
    st.d $s0, $sp, 160
    st.d $s1, $sp, 168
    st.d $s2, $sp, 176
    st.d $s3, $sp, 184
    st.d $s4, $sp, 192
    st.d $s5, $sp, 200
    st.d $s6, $sp, 208
    st.d $s7, $sp, 216
    st.d $s8, $sp, 224
    
    /* Save CSR registers */
    csrrd $t0, 0x1      /* ERA */
    st.d $t0, $sp, 232
    csrrd $t0, 0x2      /* BADVADDR */
    st.d $t0, $sp, 240
    csrrd $t0, 0x3      /* CRMD */
    st.d $t0, $sp, 248
    csrrd $t0, 0x4      /* PRMD */
    st.d $t0, $sp, 256
    csrrd $t0, 0x5      /* EUEN */
    st.d $t0, $sp, 264
    csrrd $t0, 0x6      /* ECFG */
    st.d $t0, $sp, 272
    csrrd $t0, 0x7      /* ESTAT */
    st.d $t0, $sp, 280
    
    /* Call system call handler */
    move $a0, $sp        /* Pass context as first argument */
    call loongarch_syscall_handler_c
    
    /* Restore registers */
    ld.d $ra, $sp, 0
    ld.d $tp, $sp, 8
    ld.d $a0, $sp, 24
    ld.d $a1, $sp, 32
    ld.d $a2, $sp, 40
    ld.d $a3, $sp, 48
    ld.d $a4, $sp, 56
    ld.d $a5, $sp, 64
    ld.d $a6, $sp, 72
    ld.d $a7, $sp, 80
    ld.d $t0, $sp, 88
    ld.d $t1, $sp, 96
    ld.d $t2, $sp, 104
    ld.d $t3, $sp, 112
    ld.d $t4, $sp, 120
    ld.d $t5, $sp, 128
    ld.d $t6, $sp, 136
    ld.d $t7, $sp, 144
    ld.d $t8, $sp, 152
    ld.d $s0, $sp, 160
    ld.d $s1, $sp, 168
    ld.d $s2, $sp, 176
    ld.d $s3, $sp, 184
    ld.d $s4, $sp, 192
    ld.d $s5, $sp, 200
    ld.d $s6, $sp, 208
    ld.d $s7, $sp, 216
    ld.d $s8, $sp, 224
    
    /* Restore CSR registers */
    ld.d $t0, $sp, 232
    csrwr $t0, 0x1      /* ERA */
    ld.d $t0, $sp, 240
    csrwr $t0, 0x2      /* BADVADDR */
    ld.d $t0, $sp, 248
    csrwr $t0, 0x3      /* CRMD */
    ld.d $t0, $sp, 256
    csrwr $t0, 0x4      /* PRMD */
    ld.d $t0, $sp, 264
    csrwr $t0, 0x5      /* EUEN */
    ld.d $t0, $sp, 272
    csrwr $t0, 0x6      /* ECFG */
    ld.d $t0, $sp, 280
    csrwr $t0, 0x7      /* ESTAT */
    
    /* Restore stack */
    addi.d $sp, $sp, 280
    
    /* Return from system call */
    ertn
.size loongarch_syscall_entry, .-loongarch_syscall_entry

/* Interrupt entry */
.global loongarch_interrupt_entry
.type loongarch_interrupt_entry, @function
loongarch_interrupt_entry:
    /* Interrupt entry point */
    /* Save registers */
    addi.d $sp, $sp, -280
    st.d $ra, $sp, 0
    st.d $tp, $sp, 8
    st.d $a0, $sp, 24
    st.d $a1, $sp, 32
    st.d $a2, $sp, 40
    st.d $a3, $sp, 48
    st.d $a4, $sp, 56
    st.d $a5, $sp, 64
    st.d $a6, $sp, 72
    st.d $a7, $sp, 80
    st.d $t0, $sp, 88
    st.d $t1, $sp, 96
    st.d $t2, $sp, 104
    st.d $t3, $sp, 112
    st.d $t4, $sp, 120
    st.d $t5, $sp, 128
    st.d $t6, $sp, 136
    st.d $t7, $sp, 144
    st.d $t8, $sp, 152
    st.d $s0, $sp, 160
    st.d $s1, $sp, 168
    st.d $s2, $sp, 176
    st.d $s3, $sp, 184
    st.d $s4, $sp, 192
    st.d $s5, $sp, 200
    st.d $s6, $sp, 208
    st.d $s7, $sp, 216
    st.d $s8, $sp, 224
    
    /* Save CSR registers */
    csrrd $t0, 0x1      /* ERA */
    st.d $t0, $sp, 232
    csrrd $t0, 0x2      /* BADVADDR */
    st.d $t0, $sp, 240
    csrrd $t0, 0x3      /* CRMD */
    st.d $t0, $sp, 248
    csrrd $t0, 0x4      /* PRMD */
    st.d $t0, $sp, 256
    csrrd $t0, 0x5      /* EUEN */
    st.d $t0, $sp, 264
    csrrd $t0, 0x6      /* ECFG */
    st.d $t0, $sp, 272
    csrrd $t0, 0x7      /* ESTAT */
    st.d $t0, $sp, 280
    
    /* Call interrupt handler */
    move $a0, $sp        /* Pass context as first argument */
    call loongarch_interrupt_handler
    
    /* Restore registers */
    ld.d $ra, $sp, 0
    ld.d $tp, $sp, 8
    ld.d $a0, $sp, 24
    ld.d $a1, $sp, 32
    ld.d $a2, $sp, 40
    ld.d $a3, $sp, 48
    ld.d $a4, $sp, 56
    ld.d $a5, $sp, 64
    ld.d $a6, $sp, 72
    ld.d $a7, $sp, 80
    ld.d $t0, $sp, 88
    ld.d $t1, $sp, 96
    ld.d $t2, $sp, 104
    ld.d $t3, $sp, 112
    ld.d $t4, $sp, 120
    ld.d $t5, $sp, 128
    ld.d $t6, $sp, 136
    ld.d $t7, $sp, 144
    ld.d $t8, $sp, 152
    ld.d $s0, $sp, 160
    ld.d $s1, $sp, 168
    ld.d $s2, $sp, 176
    ld.d $s3, $sp, 184
    ld.d $s4, $sp, 192
    ld.d $s5, $sp, 200
    ld.d $s6, $sp, 208
    ld.d $s7, $sp, 216
    ld.d $s8, $sp, 224
    
    /* Restore CSR registers */
    ld.d $t0, $sp, 232
    csrwr $t0, 0x1      /* ERA */
    ld.d $t0, $sp, 240
    csrwr $t0, 0x2      /* BADVADDR */
    ld.d $t0, $sp, 248
    csrwr $t0, 0x3      /* CRMD */
    ld.d $t0, $sp, 256
    csrwr $t0, 0x4      /* PRMD */
    ld.d $t0, $sp, 264
    csrwr $t0, 0x5      /* EUEN */
    ld.d $t0, $sp, 272
    csrwr $t0, 0x6      /* ECFG */
    ld.d $t0, $sp, 280
    csrwr $t0, 0x7      /* ESTAT */
    
    /* Restore stack */
    addi.d $sp, $sp, 280
    
    /* Return from interrupt */
    ertn
.size loongarch_interrupt_entry, .-loongarch_interrupt_entry
