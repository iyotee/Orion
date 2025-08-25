/*
 * ORION OS - LoongArch Assembly Functions
 * 
 * This file contains critical LoongArch assembly functions for the ORION kernel.
 */

#include "arch.h"

.section .text
.align 4

/* ============================================================================
 * Context Switching Functions
 * ============================================================================ */

.global loongarch_context_switch
.type loongarch_context_switch, @function
loongarch_context_switch:
    /* Save current context */
    /* Save general registers */
    st.d $ra, $a0, 0
    st.d $tp, $a0, 8
    st.d $sp, $a0, 16
    st.d $a0, $a0, 24
    st.d $a1, $a0, 32
    st.d $a2, $a0, 40
    st.d $a3, $a0, 48
    st.d $a4, $a0, 56
    st.d $a5, $a0, 64
    st.d $a6, $a0, 72
    st.d $a7, $a0, 80
    st.d $t0, $a0, 88
    st.d $t1, $a0, 96
    st.d $t2, $a0, 104
    st.d $t3, $a0, 112
    st.d $t4, $a0, 120
    st.d $t5, $a0, 128
    st.d $t6, $a0, 136
    st.d $t7, $a0, 144
    st.d $t8, $a0, 152
    st.d $s0, $a0, 160
    st.d $s1, $a0, 168
    st.d $s2, $a0, 176
    st.d $s3, $a0, 184
    st.d $s4, $a0, 192
    st.d $s5, $a0, 200
    st.d $s6, $a0, 208
    st.d $s7, $a0, 216
    st.d $s8, $a0, 224
    
    /* Save CSR registers */
    csrrd $t0, 0x1    /* ERA */
    st.d $t0, $a0, 232
    csrrd $t0, 0x2    /* BADVADDR */
    st.d $t0, $a0, 240
    csrrd $t0, 0x3    /* CRMD */
    st.d $t0, $a0, 248
    csrrd $t0, 0x4    /* PRMD */
    st.d $t0, $a0, 256
    csrrd $t0, 0x5    /* EUEN */
    st.d $t0, $a0, 264
    csrrd $t0, 0x6    /* ECFG */
    st.d $t0, $a0, 272
    csrrd $t0, 0x7    /* ESTAT */
    st.d $t0, $a0, 280
    
    /* Switch to new context */
    move $a0, $a1
    
    /* Restore general registers */
    ld.d $ra, $a0, 0
    ld.d $tp, $a0, 8
    ld.d $sp, $a0, 16
    ld.d $a0, $a0, 24
    ld.d $a1, $a0, 32
    ld.d $a2, $a0, 40
    ld.d $a3, $a0, 48
    ld.d $a4, $a0, 56
    ld.d $a5, $a0, 64
    ld.d $a6, $a0, 72
    ld.d $a7, $a0, 80
    ld.d $t0, $a0, 88
    ld.d $t1, $a0, 96
    ld.d $t2, $a0, 104
    ld.d $t3, $a0, 112
    ld.d $t4, $a0, 120
    ld.d $t5, $a0, 128
    ld.d $t6, $a0, 136
    ld.d $t7, $a0, 144
    ld.d $t8, $a0, 152
    ld.d $s0, $a0, 160
    ld.d $s1, $a0, 168
    ld.d $s2, $a0, 176
    ld.d $s3, $a0, 184
    ld.d $s4, $a0, 192
    ld.d $s5, $a0, 200
    ld.d $s6, $a0, 208
    ld.d $s7, $a0, 216
    ld.d $s8, $a0, 224
    
    /* Restore CSR registers */
    ld.d $t0, $a0, 232
    csrwr $t0, 0x1    /* ERA */
    ld.d $t0, $a0, 240
    csrwr $t0, 0x2    /* BADVADDR */
    ld.d $t0, $a0, 248
    csrwr $t0, 0x3    /* CRMD */
    ld.d $t0, $a0, 256
    csrwr $t0, 0x4    /* PRMD */
    ld.d $t0, $a0, 264
    csrwr $t0, 0x5    /* EUEN */
    ld.d $t0, $a0, 272
    csrwr $t0, 0x6    /* ECFG */
    ld.d $t0, $a0, 280
    csrwr $t0, 0x7    /* ESTAT */
    
    jr $ra
.size loongarch_context_switch, .-loongarch_context_switch

.global loongarch_enter_user_mode
.type loongarch_enter_user_mode, @function
loongarch_enter_user_mode:
    /* a0: entry point, a1: stack, a2: argument */
    move $t0, $a0    /* Save entry point */
    move $sp, $a1    /* Set user stack */
    move $a0, $a2    /* Set first argument */
    
    /* Switch to user mode */
    csrrd $t1, 0x3   /* Read CRMD */
    andi $t1, $t1, ~0x7  /* Clear PLV bits */
    ori $t1, $t1, 0x3    /* Set PLV to 3 (user mode) */
    csrwr $t1, 0x3   /* Write CRMD */
    
    /* Jump to user code */
    jr $t0
.size loongarch_enter_user_mode, .-loongarch_enter_user_mode

.global loongarch_return_from_interrupt
.type loongarch_return_from_interrupt, @function
loongarch_return_from_interrupt:
    /* a0: interrupt context */
    /* Restore all registers from context */
    ld.d $ra, $a0, 0
    ld.d $tp, $a0, 8
    ld.d $sp, $a0, 16
    ld.d $a0, $a0, 24
    ld.d $a1, $a0, 32
    ld.d $a2, $a0, 40
    ld.d $a3, $a0, 48
    ld.d $a4, $a0, 56
    ld.d $a5, $a0, 64
    ld.d $a6, $a0, 72
    ld.d $a7, $a0, 80
    ld.d $t0, $a0, 88
    ld.d $t1, $a0, 96
    ld.d $t2, $a0, 104
    ld.d $t3, $a0, 112
    ld.d $t4, $a0, 120
    ld.d $t5, $a0, 128
    ld.d $t6, $a0, 136
    ld.d $t7, $a0, 144
    ld.d $t8, $a0, 152
    ld.d $s0, $a0, 160
    ld.d $s1, $a0, 168
    ld.d $s2, $a0, 176
    ld.d $s3, $a0, 184
    ld.d $s4, $a0, 192
    ld.d $s5, $a0, 200
    ld.d $s6, $a0, 208
    ld.d $s7, $a0, 216
    ld.d $s8, $a0, 224
    
    /* Restore CSR registers */
    ld.d $t0, $a0, 232
    csrwr $t0, 0x1    /* ERA */
    ld.d $t0, $a0, 240
    csrwr $t0, 0x2    /* BADVADDR */
    ld.d $t0, $a0, 248
    csrwr $t0, 0x3    /* CRMD */
    ld.d $t0, $a0, 256
    csrwr $t0, 0x4    /* PRMD */
    ld.d $t0, $a0, 264
    csrwr $t0, 0x5    /* EUEN */
    ld.d $t0, $a0, 272
    csrwr $t0, 0x6    /* ECFG */
    ld.d $t0, $a0, 280
    csrwr $t0, 0x7    /* ESTAT */
    
    /* Return from interrupt */
    ertn
.size loongarch_return_from_interrupt, .-loongarch_return_from_interrupt

/* ============================================================================
 * TLB Management Functions
 * ============================================================================ */

.global loongarch_flush_tlb_all
.type loongarch_flush_tlb_all, @function
loongarch_flush_tlb_all:
    /* Invalidate entire TLB */
    csrwr $zero, 0x18   /* TLBIDX */
    csrwr $zero, 0x19   /* TLBEHI */
    csrwr $zero, 0x1A   /* TLBELO0 */
    csrwr $zero, 0x1B   /* TLBELO1 */
    tlbwr                 /* Write TLB entry */
    
    /* Invalidate STLB if present */
    csrrd $t0, 0x1C     /* Read STLBPS */
    beqz $t0, 1f         /* Skip if no STLB */
    
    /* Invalidate STLB */
    csrwr $zero, 0x1D   /* STLBIDX */
    csrwr $zero, 0x1E   /* STLBEHI */
    csrwr $zero, 0x1F   /* STLBELO0 */
    csrwr $zero, 0x20   /* STLBELO1 */
    stlbwr                /* Write STLB entry */
    
1:
    jr $ra
.size loongarch_flush_tlb_all, .-loongarch_flush_tlb_all

.global loongarch_flush_tlb_page
.type loongarch_flush_tlb_page, @function
loongarch_flush_tlb_page:
    /* a0: virtual address to flush */
    /* Extract page number */
    srli.d $t0, $a0, 12  /* Shift right by 12 bits */
    slli.d $t0, $t0, 12  /* Shift left by 12 bits */
    
    /* Set TLB entry to invalidate */
    csrwr $t0, 0x19      /* TLBEHI */
    csrwr $zero, 0x1A    /* TLBELO0 */
    csrwr $zero, 0x1B    /* TLBELO1 */
    
    /* Invalidate TLB entry */
    tlbwr
    
    /* Also invalidate STLB if present */
    csrrd $t1, 0x1C      /* Read STLBPS */
    beqz $t1, 1f         /* Skip if no STLB */
    
    csrwr $t0, 0x1E      /* STLBEHI */
    csrwr $zero, 0x1F    /* STLBELO0 */
    csrwr $zero, 0x20    /* STLBELO1 */
    stlbwr
    
1:
    jr $ra
.size loongarch_flush_tlb_page, .-loongarch_flush_tlb_page

/* ============================================================================
 * Cache Management Functions
 * ============================================================================ */

.global loongarch_flush_icache_all
.type loongarch_flush_icache_all, @function
loongarch_flush_icache_all:
    /* Flush entire instruction cache */
    csrrd $t0, 0x21     /* Read ICACHE */
    andi $t0, $t0, 0x1  /* Check if I-cache is enabled */
    beqz $t0, 1f        /* Skip if I-cache disabled */
    
    /* Flush I-cache */
    csrwr $zero, 0x22   /* ICACHEOP */
    ori $t0, $zero, 0x1 /* Set flush bit */
    csrwr $t0, 0x22     /* ICACHEOP */
    
1:
    jr $ra
.size loongarch_flush_icache_all, .-loongarch_flush_icache_all

.global loongarch_flush_dcache_all
.type loongarch_flush_dcache_all, @function
loongarch_flush_dcache_all:
    /* Flush entire data cache */
    csrrd $t0, 0x23     /* Read DCACHE */
    andi $t0, $t0, 0x1  /* Check if D-cache is enabled */
    beqz $t0, 1f        /* Skip if D-cache disabled */
    
    /* Flush D-cache */
    csrwr $zero, 0x24   /* DCACHEOP */
    ori $t0, $zero, 0x1 /* Set flush bit */
    csrwr $t0, 0x24     /* DCACHEOP */
    
1:
    jr $ra
.size loongarch_flush_dcache_all, .-loongarch_flush_dcache_all

.global loongarch_sync_icache
.type loongarch_sync_icache, @function
loongarch_sync_icache:
    /* a0: virtual address, a1: length */
    /* Sync instruction cache for specific range */
    csrrd $t0, 0x21     /* Read ICACHE */
    andi $t0, $t0, 0x1  /* Check if I-cache is enabled */
    beqz $t0, 1f        /* Skip if I-cache disabled */
    
    /* Calculate end address */
    add.d $t1, $a0, $a1
    
    /* Align to cache line */
    andi $t0, $a0, ~(LOONGARCH_CACHE_LINE_SIZE - 1)
    
    /* Sync each cache line */
2:
    csrwr $t0, 0x25     /* ICACHEOP_RANGE */
    ori $t2, $zero, 0x2 /* Set sync bit */
    csrwr $t2, 0x25     /* ICACHEOP_RANGE */
    
    addi.d $t0, $t0, LOONGARCH_CACHE_LINE_SIZE
    blt $t0, $t1, 2b
    
1:
    jr $ra
.size loongarch_sync_icache, .-loongarch_sync_icache

.global loongarch_sync_dcache
.type loongarch_sync_dcache, @function
loongarch_sync_dcache:
    /* a0: virtual address, a1: length */
    /* Sync data cache for specific range */
    csrrd $t0, 0x23     /* Read DCACHE */
    andi $t0, $t0, 0x1  /* Check if D-cache is enabled */
    beqz $t0, 1f        /* Skip if D-cache disabled */
    
    /* Calculate end address */
    add.d $t1, $a0, $a1
    
    /* Align to cache line */
    andi $t0, $a0, ~(LOONGARCH_CACHE_LINE_SIZE - 1)
    
    /* Sync each cache line */
2:
    csrwr $t0, 0x26     /* DCACHEOP_RANGE */
    ori $t2, $zero, 0x2 /* Set sync bit */
    csrwr $t2, 0x26     /* DCACHEOP_RANGE */
    
    addi.d $t0, $t0, LOONGARCH_CACHE_LINE_SIZE
    blt $t0, $t1, 2b
    
1:
    jr $ra
.size loongarch_sync_dcache, .-loongarch_sync_dcache

/* ============================================================================
 * Memory Barrier Functions
 * ============================================================================ */

.global loongarch_memory_barrier
.type loongarch_memory_barrier, @function
loongarch_memory_barrier:
    /* Full memory barrier */
    dbar 0
    jr $ra
.size loongarch_memory_barrier, .-loongarch_memory_barrier

.global loongarch_read_barrier
.type loongarch_read_barrier, @function
loongarch_read_barrier:
    /* Read memory barrier */
    dbar 0
    jr $ra
.size loongarch_read_barrier, .-loongarch_read_barrier

.global loongarch_write_barrier
.type loongarch_write_barrier, @function
loongarch_write_barrier:
    /* Write memory barrier */
    dbar 0
    jr $ra
.size loongarch_write_barrier, .-loongarch_write_barrier

/* ============================================================================
 * Atomic Operations
 * ============================================================================ */

.global loongarch_atomic_cas
.type loongarch_atomic_cas, @function
loongarch_atomic_cas:
    /* a0: pointer, a1: old value, a2: new value */
    /* Return value in a0: 1 if successful, 0 if failed */
    
    /* Load linked */
    ld.d $t0, $a0, 0
    
    /* Compare with old value */
    bne $t0, $a1, 1f
    
    /* Store conditional */
    sc.d $t1, $a2, $a0, 0
    
    /* Check if store was successful */
    beqz $t1, 2f
    
    /* Success */
    li $a0, 1
    jr $ra
    
1:
    /* Failure - values don't match */
    li $a0, 0
    jr $ra
    
2:
    /* Failure - store failed */
    li $a0, 0
    jr $ra
.size loongarch_atomic_cas, .-loongarch_atomic_cas

.global loongarch_atomic_add
.type loongarch_atomic_add, @function
loongarch_atomic_add:
    /* a0: pointer, a1: value to add */
    /* Return value in a0: old value */
    
    /* Load linked */
    ld.d $t0, $a0, 0
    
    /* Add value */
    add.d $t1, $t0, $a1
    
    /* Store conditional */
    sc.d $t2, $t1, $a0, 0
    
    /* Check if store was successful */
    beqz $t2, loongarch_atomic_add
    
    /* Return old value */
    move $a0, $t0
    jr $ra
.size loongarch_atomic_add, .-loongarch_atomic_add

.global loongarch_atomic_sub
.type loongarch_atomic_sub, @function
loongarch_atomic_sub:
    /* a0: pointer, a1: value to subtract */
    /* Return value in a0: old value */
    
    /* Load linked */
    ld.d $t0, $a0, 0
    
    /* Subtract value */
    sub.d $t1, $t0, $a1
    
    /* Store conditional */
    sc.d $t2, $t1, $a0, 0
    
    /* Check if store was successful */
    beqz $t2, loongarch_atomic_sub
    
    /* Return old value */
    move $a0, $t0
    jr $ra
.size loongarch_atomic_sub, .-loongarch_atomic_sub

/* ============================================================================
 * Vector Extension Support
 * ============================================================================ */

.global loongarch_lsx_save_context
.type loongarch_lsx_save_context, @function
loongarch_lsx_save_context:
    /* a0: pointer to save area */
    /* Save LSX registers if supported */
    csrrd $t0, 0x5      /* Read EUEN */
    andi $t0, $t0, 0x1  /* Check LSX bit */
    beqz $t0, 1f        /* Skip if LSX not supported */
    
    /* Save LSX registers */
    /* TODO: Implement LSX register saving */
    
1:
    jr $ra
.size loongarch_lsx_save_context, .-loongarch_lsx_save_context

.global loongarch_lsx_restore_context
.type loongarch_lsx_restore_context, @function
loongarch_lsx_restore_context:
    /* a0: pointer to restore area */
    /* Restore LSX registers if supported */
    csrrd $t0, 0x5      /* Read EUEN */
    andi $t0, $t0, 0x1  /* Check LSX bit */
    beqz $t0, 1f        /* Skip if LSX not supported */
    
    /* Restore LSX registers */
    /* TODO: Implement LSX register restoration */
    
1:
    jr $ra
.size loongarch_lsx_restore_context, .-loongarch_lsx_restore_context

/* ============================================================================
 * Exception Vector Stubs
 * ============================================================================ */

.section .text.vectors
.align 4

.global loongarch_exception_vectors
loongarch_exception_vectors:
    /* Reset vector */
    j loongarch_exception_reset
    
    /* TLB refill vector */
    j loongarch_exception_tlb_refill
    
    /* TLB invalid vector */
    j loongarch_exception_tlb_invalid
    
    /* TLB modified vector */
    j loongarch_exception_tlb_modified
    
    /* TLB load vector */
    j loongarch_exception_tlb_load
    
    /* TLB store vector */
    j loongarch_exception_tlb_store
    
    /* Address error vector */
    j loongarch_exception_addr_error
    
    /* Address error store vector */
    j loongarch_exception_addr_error_store
    
    /* System call vector */
    j loongarch_exception_syscall
    
    /* Break vector */
    j loongarch_exception_break
    
    /* Reserved vector */
    j loongarch_exception_reserved
    
    /* Floating point vector */
    j loongarch_exception_floating
    
    /* Load vector */
    j loongarch_exception_load
    
    /* Store vector */
    j loongarch_exception_store
    
    /* Load guest vector */
    j loongarch_exception_load_guest
    
    /* Store guest vector */
    j loongarch_exception_store_guest

/* Exception handlers */
loongarch_exception_reset:
    j loongarch_panic_handler

loongarch_exception_tlb_refill:
    j loongarch_tlb_refill_handler

loongarch_exception_tlb_invalid:
    j loongarch_tlb_invalid_handler

loongarch_exception_tlb_modified:
    j loongarch_tlb_modified_handler

loongarch_exception_tlb_load:
    j loongarch_tlb_load_handler

loongarch_exception_tlb_store:
    j loongarch_tlb_store_handler

loongarch_exception_addr_error:
    j loongarch_addr_error_handler

loongarch_exception_addr_error_store:
    j loongarch_addr_error_store_handler

loongarch_exception_syscall:
    j loongarch_syscall_handler

loongarch_exception_break:
    j loongarch_break_handler

loongarch_exception_reserved:
    j loongarch_panic_handler

loongarch_exception_floating:
    j loongarch_floating_handler

loongarch_exception_load:
    j loongarch_load_handler

loongarch_exception_store:
    j loongarch_store_handler

loongarch_exception_load_guest:
    j loongarch_load_guest_handler

loongarch_exception_store_guest:
    j loongarch_store_guest_handler

/* Default handlers (implement in C) */
loongarch_panic_handler:
    j loongarch_panic

loongarch_tlb_refill_handler:
    j loongarch_tlb_refill_handler_c

loongarch_tlb_invalid_handler:
    j loongarch_tlb_invalid_handler_c

loongarch_tlb_modified_handler:
    j loongarch_tlb_modified_handler_c

loongarch_tlb_load_handler:
    j loongarch_tlb_load_handler_c

loongarch_tlb_store_handler:
    j loongarch_tlb_store_handler_c

loongarch_addr_error_handler:
    j loongarch_addr_error_handler_c

loongarch_addr_error_store_handler:
    j loongarch_addr_error_store_handler_c

loongarch_syscall_handler:
    j loongarch_syscall_handler_c

loongarch_break_handler:
    j loongarch_break_handler_c

loongarch_floating_handler:
    j loongarch_floating_handler_c

loongarch_load_handler:
    j loongarch_load_handler_c

loongarch_store_handler:
    j loongarch_store_handler_c

loongarch_load_guest_handler:
    j loongarch_load_guest_handler_c

loongarch_store_guest_handler:
    j loongarch_store_guest_handler_c
