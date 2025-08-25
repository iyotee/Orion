/*
 * ORION OS - ARMv7l Assembly Functions
 * 
 * This file implements assembly-level architecture-specific functions
 * for ARMv7l, including CPU control, memory management, and performance.
 */

#include "arch.h"

// ============================================================================
// CPU CONTROL FUNCTIONS
// ============================================================================

.global arm_cpu_enable_interrupts
.type arm_cpu_enable_interrupts, %function

arm_cpu_enable_interrupts:
    // Enable IRQ and FIQ
    cpsie   if
    bx      lr

.global arm_cpu_disable_interrupts
.type arm_cpu_disable_interrupts, %function

arm_cpu_disable_interrupts:
    // Disable IRQ and FIQ
    cpsid   if
    bx      lr

.global arm_cpu_enable_irq
.type arm_cpu_enable_irq, %function

arm_cpu_enable_irq:
    // Enable IRQ only
    cpsie   i
    bx      lr

.global arm_cpu_disable_irq
.type arm_cpu_disable_irq, %function

arm_cpu_disable_irq:
    // Disable IRQ only
    cpsid   i
    bx      lr

.global arm_cpu_enable_fiq
.type arm_cpu_enable_fiq, %function

arm_cpu_enable_fiq:
    // Enable FIQ only
    cpsie   f
    bx      lr

.global arm_cpu_disable_fiq
.type arm_cpu_disable_fiq, %function

arm_cpu_disable_fiq:
    // Disable FIQ only
    cpsid   f
    bx      lr

.global arm_cpu_nop
.type arm_cpu_nop, %function

arm_cpu_nop:
    // No operation
    nop
    bx      lr

.global arm_cpu_wfi
.type arm_cpu_wfi, %function

arm_cpu_wfi:
    // Wait for interrupt
    wfi
    bx      lr

.global arm_cpu_wfe
.type arm_cpu_wfe, %function

arm_cpu_wfe:
    // Wait for event
    wfe
    bx      lr

.global arm_cpu_sev
.type arm_cpu_sev, %function

arm_cpu_sev:
    // Send event
    sev
    bx      lr

// ============================================================================
// CPSR/SPSR ACCESS
// ============================================================================

.global arm_cpu_read_cpsr
.type arm_cpu_read_cpsr, %function

arm_cpu_read_cpsr:
    // Read CPSR
    mrs     r0, cpsr
    bx      lr

.global arm_cpu_write_cpsr
.type arm_cpu_write_cpsr, %function

arm_cpu_write_cpsr:
    // Write CPSR
    msr     cpsr, r0
    bx      lr

.global arm_cpu_read_spsr
.type arm_cpu_read_spsr, %function

arm_cpu_read_spsr:
    // Read SPSR
    mrs     r0, spsr
    bx      lr

.global arm_cpu_write_spsr
.type arm_cpu_write_spsr, %function

arm_cpu_write_spsr:
    // Write SPSR
    msr     spsr, r0
    bx      lr

// ============================================================================
// CPU ID AND FEATURES
// ============================================================================

.global arm_cpu_get_id
.type arm_cpu_get_id, %function

arm_cpu_get_id:
    // Get CPU ID
    mrc     p15, 0, r0, c0, c0, 0
    bx      lr

.global arm_cpu_get_revision
.type arm_cpu_get_revision, %function

arm_cpu_get_revision:
    // Get CPU revision
    mrc     p15, 0, r0, c0, c0, 1
    bx      lr

.global arm_cpu_get_features
.type arm_cpu_get_features, %function

arm_cpu_get_features:
    // Get CPU features
    mrc     p15, 0, r0, c0, c1, 0
    bx      lr

.global arm_cpu_get_cache_info
.type arm_cpu_get_cache_info, %function

arm_cpu_get_cache_info:
    // Get cache information
    mrc     p15, 0, r0, c0, c1, 1
    bx      lr

.global arm_cpu_get_tlb_info
.type arm_cpu_get_tlb_info, %function

arm_cpu_get_tlb_info:
    // Get TLB information
    mrc     p15, 0, r0, c0, c1, 2
    bx      lr

// ============================================================================
// MMU CONTROL
// ============================================================================

.global arm_mmu_enable
.type arm_mmu_enable, %function

arm_mmu_enable:
    // Enable MMU
    mrc     p15, 0, r0, c1, c0, 0
    orr     r0, r0, #0x00000001
    mcr     p15, 0, r0, c1, c0, 0
    
    // Flush pipeline
    mov     r0, r0
    sub     pc, pc, #4
    
    bx      lr

.global arm_mmu_disable
.type arm_mmu_disable, %function

arm_mmu_disable:
    // Disable MMU
    mrc     p15, 0, r0, c1, c0, 0
    bic     r0, r0, #0x00000001
    mcr     p15, 0, r0, c1, c0, 0
    
    // Flush pipeline
    mov     r0, r0
    sub     pc, pc, #4
    
    bx      lr

.global arm_mmu_is_enabled
.type arm_mmu_is_enabled, %function

arm_mmu_is_enabled:
    // Check if MMU is enabled
    mrc     p15, 0, r0, c1, c0, 0
    and     r0, r0, #0x00000001
    bx      lr

.global arm_mmu_invalidate_tlb
.type arm_mmu_invalidate_tlb, %function

arm_mmu_invalidate_tlb:
    // Invalidate entire TLB
    mov     r0, #0
    mcr     p15, 0, r0, c8, c7, 0
    bx      lr

.global arm_mmu_invalidate_tlb_entry
.type arm_mmu_invalidate_tlb_entry, %function

arm_mmu_invalidate_tlb_entry:
    // Invalidate TLB entry for address in r0
    mcr     p15, 0, r0, c8, c7, 1
    bx      lr

.global arm_mmu_invalidate_tlb_range
.type arm_mmu_invalidate_tlb_range, %function

arm_mmu_invalidate_tlb_range:
    // Invalidate TLB entries in range [r0, r1)
    // r0 = start address, r1 = end address
1:
    cmp     r0, r1
    mcrlt   p15, 0, r0, c8, c7, 1
    addlt   r0, r0, #0x1000  // 4KB pages
    blt     1b
    bx      lr

// ============================================================================
// CACHE CONTROL
// ============================================================================

.global arm_cache_enable
.type arm_cache_enable, %function

arm_cache_enable:
    // Enable caches
    mrc     p15, 0, r0, c1, c0, 0
    orr     r0, r0, #0x00000004  // Enable data cache
    orr     r0, r0, #0x00001000  // Enable instruction cache
    mcr     p15, 0, r0, c1, c0, 0
    bx      lr

.global arm_cache_disable
.type arm_cache_disable, %function

arm_cache_disable:
    // Disable caches
    mrc     p15, 0, r0, c1, c0, 0
    bic     r0, r0, #0x00000004  // Disable data cache
    bic     r0, r0, #0x00001000  // Disable instruction cache
    mcr     p15, 0, r0, c1, c0, 0
    bx      lr

.global arm_cache_is_enabled
.type arm_cache_is_enabled, %function

arm_cache_is_enabled:
    // Check if caches are enabled
    mrc     p15, 0, r0, c1, c0, 0
    and     r0, r0, #0x00001004
    cmp     r0, #0x00001004
    moveq   r0, #1
    movne   r0, #0
    bx      lr

.global arm_cache_flush_all
.type arm_cache_flush_all, %function

arm_cache_flush_all:
    // Flush all caches
    mov     r0, #0
    mcr     p15, 0, r0, c7, c7, 0
    bx      lr

.global arm_cache_invalidate_all
.type arm_cache_invalidate_all, %function

arm_cache_invalidate_all:
    // Invalidate all caches
    mov     r0, #0
    mcr     p15, 0, r0, c7, c7, 0
    bx      lr

.global arm_cache_flush_range
.type arm_cache_flush_range, %function

arm_cache_flush_range:
    // Flush cache range [r0, r1)
    // r0 = start address, r1 = end address
1:
    cmp     r0, r1
    mcrlt   p15, 0, r0, c7, c14, 1  // Clean and invalidate D entry
    addlt   r0, r0, #32  // Cache line size
    blt     1b
    bx      lr

.global arm_cache_invalidate_range
.type arm_cache_invalidate_range, %function

arm_cache_invalidate_range:
    // Invalidate cache range [r0, r1)
    // r0 = start address, r1 = end address
1:
    cmp     r0, r1
    mcrlt   p15, 0, r0, c7, c6, 1   // Invalidate D entry
    addlt   r0, r0, #32  // Cache line size
    blt     1b
    bx      lr

.global arm_cache_get_info
.type arm_cache_get_info, %function

arm_cache_get_info:
    // Get cache information
    // Returns cache info in r0
    mrc     p15, 0, r0, c0, c1, 1
    bx      lr

.global arm_cache_get_line_size
.type arm_cache_get_line_size, %function

arm_cache_get_line_size:
    // Get cache line size
    // Returns line size in bytes in r0
    mrc     p15, 0, r0, c0, c1, 1
    and     r0, r0, #0xFF
    mov     r0, r0, lsl #3  // Convert to bytes
    bx      lr

.global arm_cache_get_size
.type arm_cache_get_size, %function

arm_cache_get_size:
    // Get cache size
    // Returns cache size in bytes in r0
    mrc     p15, 0, r0, c0, c1, 1
    and     r0, r0, #0xFF00
    mov     r0, r0, lsr #8
    mov     r0, r0, lsl #10  // Convert to bytes
    bx      lr

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

.global arm_perf_get_cycle_counter
.type arm_perf_get_cycle_counter, %function

arm_perf_get_cycle_counter:
    // Get cycle counter
    mrc     p15, 0, r0, c9, c13, 0
    bx      lr

.global arm_perf_get_instruction_counter
.type arm_perf_get_instruction_counter, %function

arm_perf_get_instruction_counter:
    // Get instruction counter
    mrc     p15, 0, r0, c9, c13, 1
    bx      lr

.global arm_perf_reset_counters
.type arm_perf_reset_counters, %function

arm_perf_reset_counters:
    // Reset performance counters
    mov     r0, #0
    mcr     p15, 0, r0, c9, c12, 0
    bx      lr

.global arm_perf_enable_counters
.type arm_perf_enable_counters, %function

arm_perf_enable_counters:
    // Enable performance counters
    mov     r0, #1
    mcr     p15, 0, r0, c9, c12, 1
    bx      lr

.global arm_perf_disable_counters
.type arm_perf_disable_counters, %function

arm_perf_disable_counters:
    // Disable performance counters
    mov     r0, #0
    mcr     p15, 0, r0, c9, c12, 1
    bx      lr

// ============================================================================
// NEON/VFP CONTROL
// ============================================================================

.global arm_neon_enable
.type arm_neon_enable, %function

arm_neon_enable:
    // Enable NEON/VFP
    mrc     p15, 0, r0, c1, c0, 2
    orr     r0, r0, #0x0F00000
    mcr     p15, 0, r0, c1, c0, 2
    
    // Enable access to CP10 and CP11
    mov     r0, #0x00F00000
    mcr     p15, 0, r0, c1, c0, 2
    
    // Set up VFP
    fmrx    r0, FPEXC
    orr     r0, r0, #0x40000000
    fmxr    FPEXC, r0
    
    bx      lr

.global arm_neon_disable
.type arm_neon_disable, %function

arm_neon_disable:
    // Disable NEON/VFP
    fmrx    r0, FPEXC
    bic     r0, r0, #0x40000000
    fmxr    FPEXC, r0
    
    mrc     p15, 0, r0, c1, c0, 2
    bic     r0, r0, #0x0F00000
    mcr     p15, 0, r0, c1, c0, 2
    
    bx      lr

.global arm_neon_is_available
.type arm_neon_is_available, %function

arm_neon_is_available:
    // Check if NEON is available
    mrc     p15, 0, r0, c0, c1, 2
    and     r0, r0, #0x0F00000
    cmp     r0, #0x0F00000
    moveq   r0, #1
    movne   r0, #0
    bx      lr

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

.global arm_delay_cycles
.type arm_delay_cycles, %function

arm_delay_cycles:
    // Delay for r0 cycles
    // r0 = number of cycles to delay
1:
    subs    r0, r0, #1
    bne     1b
    bx      lr

.global arm_delay_microseconds
.type arm_delay_microseconds, %function

arm_delay_microseconds:
    // Delay for r0 microseconds
    // r0 = number of microseconds to delay
    // This is a rough approximation
    mov     r1, #1000  // Assume 1GHz CPU
    mul     r0, r0, r1
1:
    subs    r0, r0, #1
    bne     1b
    bx      lr

.global arm_delay_milliseconds
.type arm_delay_milliseconds, %function

arm_delay_milliseconds:
    // Delay for r0 milliseconds
    // r0 = number of milliseconds to delay
    mov     r1, #1000
    mul     r0, r0, r1
    bl      arm_delay_microseconds
    bx      lr

// ============================================================================
// MEMORY BARRIERS
// ============================================================================

.global arm_memory_barrier
.type arm_memory_barrier, %function

arm_memory_barrier:
    // Full memory barrier
    dmb
    bx      lr

.global arm_read_barrier
.type arm_read_barrier, %function

arm_read_barrier:
    // Read barrier
    dmb     ish
    bx      lr

.global arm_write_barrier
.type arm_write_barrier, %function

arm_write_barrier:
    // Write barrier
    dmb     ishst
    bx      lr

.global arm_instruction_barrier
.type arm_instruction_barrier, %function

arm_instruction_barrier:
    // Instruction barrier
    isb
    bx      lr

// ============================================================================
// ATOMIC OPERATIONS
// ============================================================================

.global arm_atomic_add
.type arm_atomic_add, %function

arm_atomic_add:
    // Atomic add: *r0 += r1
    // r0 = pointer to value, r1 = value to add
    // Returns old value in r0
1:
    ldrex   r2, [r0]
    add     r3, r2, r1
    strex   r1, r3, [r0]
    cmp     r1, #0
    bne     1b
    mov     r0, r2
    bx      lr

.global arm_atomic_sub
.type arm_atomic_sub, %function

arm_atomic_sub:
    // Atomic subtract: *r0 -= r1
    // r0 = pointer to value, r1 = value to subtract
    // Returns old value in r0
1:
    ldrex   r2, [r0]
    sub     r3, r2, r1
    strex   r1, r3, [r0]
    cmp     r1, #0
    bne     1b
    mov     r0, r2
    bx      lr

.global arm_atomic_swap
.type arm_atomic_swap, %function

arm_atomic_swap:
    // Atomic swap: *r0 = r1
    // r0 = pointer to value, r1 = new value
    // Returns old value in r0
1:
    ldrex   r2, [r0]
    strex   r3, r1, [r0]
    cmp     r3, #0
    bne     1b
    mov     r0, r2
    bx      lr

.global arm_atomic_compare_and_swap
.type arm_atomic_compare_and_swap, %function

arm_atomic_compare_and_swap:
    // Atomic compare and swap: if (*r0 == r1) then *r0 = r2
    // r0 = pointer to value, r1 = expected value, r2 = new value
    // Returns 1 if swap occurred, 0 otherwise
1:
    ldrex   r3, [r0]
    cmp     r3, r1
    bne     2f
    strex   r3, r2, [r0]
    cmp     r3, #0
    bne     1b
    mov     r0, #1
    bx      lr
2:
    clrex
    mov     r0, #0
    bx      lr

// ============================================================================
// CONTEXT SWITCHING
// ============================================================================

.global arm_save_context
.type arm_save_context, %function

arm_save_context:
    // Save CPU context to memory pointed by r0
    // r0 = pointer to context structure
    stmia   r0!, {r0-r12, sp, lr}
    mrs     r1, cpsr
    str     r1, [r0]
    bx      lr

.global arm_restore_context
.type arm_restore_context, %function

arm_restore_context:
    // Restore CPU context from memory pointed by r0
    // r0 = pointer to context structure
    ldmia   r0!, {r1-r12, sp, lr}
    ldr     r1, [r0]
    msr     cpsr, r1
    mov     r0, r1
    bx      lr

// ============================================================================
// MODE SWITCHING
// ============================================================================

.global arm_switch_mode
.type arm_switch_mode, %function

arm_switch_mode:
    // Switch to mode specified in r0
    // r0 = mode bits
    mrs     r1, cpsr
    bic     r1, r1, #0x1F
    orr     r1, r1, r0
    msr     cpsr, r1
    bx      lr

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
