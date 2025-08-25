/*
 * ORION OS - MIPS Architecture Assembly
 *
 * Assembly file for MIPS architecture (MIPS32, MIPS64)
 * Implements low-level CPU control and hardware access
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "config.h"

// ============================================================================
// CPU CONTROL FUNCTIONS
// ============================================================================

.global mips_cpu_enable_interrupts_asm
.type mips_cpu_enable_interrupts_asm, @function
mips_cpu_enable_interrupts_asm:
    ei
    jr $ra

.global mips_cpu_disable_interrupts_asm
.type mips_cpu_disable_interrupts_asm, @function
mips_cpu_disable_interrupts_asm:
    di
    jr $ra

.global mips_cpu_wait_for_interrupt_asm
.type mips_cpu_wait_for_interrupt_asm, @function
mips_cpu_wait_for_interrupt_asm:
    wait
    jr $ra

.global mips_cpu_nop_asm
.type mips_cpu_nop_asm, @function
mips_cpu_nop_asm:
    nop
    jr $ra

.global mips_cpu_sync_asm
.type mips_cpu_sync_asm, @function
mips_cpu_sync_asm:
    sync
    jr $ra

.global mips_cpu_sync_p_asm
.type mips_cpu_sync_p_asm, @function
mips_cpu_sync_p_asm:
    sync.p
    jr $ra

.global mips_cpu_sync_s_asm
.type mips_cpu_sync_s_asm, @function
mips_cpu_sync_s_asm:
    sync.s
    jr $ra

// ============================================================================
// CP0 REGISTER ACCESS FUNCTIONS
// ============================================================================

.global mips_cpu_read_cp0_register_asm
.type mips_cpu_read_cp0_register_asm, @function
mips_cpu_read_cp0_register_asm:
    dmfc0 $v0, $0  // Read CP0 register (register number in $a0)
    jr $ra

.global mips_cpu_write_cp0_register_asm
.type mips_cpu_write_cp0_register_asm, @function
mips_cpu_write_cp0_register_asm:
    dmtc0 $a1, $0  // Write CP0 register (register number in $a0, value in $a1)
    jr $ra

.global mips_cpu_read_cp0_count_asm
.type mips_cpu_read_cp0_count_asm, @function
mips_cpu_read_cp0_count_asm:
    dmfc0 $v0, $9  // Read CP0 Count register
    jr $ra

.global mips_cpu_read_cp0_compare_asm
.type mips_cpu_read_cp0_compare_asm, @function
mips_cpu_read_cp0_compare_asm:
    dmfc0 $v0, $11 // Read CP0 Compare register
    jr $ra

.global mips_cpu_write_cp0_compare_asm
.type mips_cpu_write_cp0_compare_asm, @function
mips_cpu_write_cp0_compare_asm:
    dmtc0 $a0, $11 // Write CP0 Compare register (value in $a0)
    jr $ra

.global mips_cpu_read_cp0_status_asm
.type mips_cpu_read_cp0_status_asm, @function
mips_cpu_read_cp0_status_asm:
    dmfc0 $v0, $12 // Read CP0 Status register
    jr $ra

.global mips_cpu_write_cp0_status_asm
.type mips_cpu_write_cp0_status_asm, @function
mips_cpu_write_cp0_status_asm:
    dmtc0 $a0, $12 // Write CP0 Status register (value in $a0)
    jr $ra

.global mips_cpu_read_cp0_cause_asm
.type mips_cpu_read_cp0_cause_asm, @function
mips_cpu_read_cp0_cause_asm:
    dmfc0 $v0, $13 // Read CP0 Cause register
    jr $ra

.global mips_cpu_read_cp0_epc_asm
.type mips_cpu_read_cp0_epc_asm, @function
mips_cpu_read_cp0_epc_asm:
    dmfc0 $v0, $14 // Read CP0 EPC register
    jr $ra

.global mips_cpu_write_cp0_epc_asm
.type mips_cpu_write_cp0_epc_asm, @function
mips_cpu_write_cp0_epc_asm:
    dmtc0 $a0, $14 // Write CP0 EPC register (value in $a0)
    jr $ra

// ============================================================================
// MMU CONTROL FUNCTIONS
// ============================================================================

.global mips_tlb_invalidate_all_asm
.type mips_tlb_invalidate_all_asm, @function
mips_tlb_invalidate_all_asm:
    tlbp
    tlbwi
    jr $ra

.global mips_tlb_invalidate_entry_asm
.type mips_tlb_invalidate_entry_asm, @function
mips_tlb_invalidate_entry_asm:
    // Address to invalidate is in $a0
    dmtc0 $a0, $10 // Write to EntryHi
    tlbp
    tlbwi
    jr $ra

.global mips_tlb_read_index_asm
.type mips_tlb_read_index_asm, @function
mips_tlb_read_index_asm:
    tlbp
    dmfc0 $v0, $0  // Read Index register
    jr $ra

.global mips_tlb_write_index_asm
.type mips_tlb_write_index_asm, @function
mips_tlb_write_index_asm:
    dmtc0 $a0, $0  // Write Index register (value in $a0)
    jr $ra

.global mips_tlb_read_random_asm
.type mips_tlb_read_random_asm, @function
mips_tlb_read_random_asm:
    dmfc0 $v0, $1  // Read Random register
    jr $ra

.global mips_tlb_read_entrylo0_asm
.type mips_tlb_read_entrylo0_asm, @function
mips_tlb_read_entrylo0_asm:
    dmfc0 $v0, $2  // Read EntryLo0 register
    jr $ra

.global mips_tlb_write_entrylo0_asm
.type mips_tlb_write_entrylo0_asm, @function
mips_tlb_write_entrylo0_asm:
    dmtc0 $a0, $2  // Write EntryLo0 register (value in $a0)
    jr $ra

.global mips_tlb_read_entrylo1_asm
.type mips_tlb_read_entrylo1_asm, @function
mips_tlb_read_entrylo1_asm:
    dmfc0 $v0, $3  // Read EntryLo1 register
    jr $ra

.global mips_tlb_write_entrylo1_asm
.type mips_tlb_write_entrylo1_asm, @function
mips_tlb_write_entrylo1_asm:
    dmtc0 $a0, $3  // Write EntryLo1 register (value in $a0)
    jr $ra

.global mips_tlb_read_entryhi_asm
.type mips_tlb_read_entryhi_asm, @function
mips_tlb_read_entryhi_asm:
    dmfc0 $v0, $10 // Read EntryHi register
    jr $ra

.global mips_tlb_write_entryhi_asm
.type mips_tlb_write_entryhi_asm, @function
mips_tlb_write_entryhi_asm:
    dmtc0 $a0, $10 // Write EntryHi register (value in $a0)
    jr $ra

// ============================================================================
// CACHE CONTROL FUNCTIONS
// ============================================================================

.global mips_cache_flush_all_asm
.type mips_cache_flush_all_asm, @function
mips_cache_flush_all_asm:
    // Flush I-cache
    cache 1, 0($0)  // Index Writeback Invalidate Instruction
    // Flush D-cache
    cache 0, 0($0)  // Index Writeback Invalidate Data
    jr $ra

.global mips_cache_invalidate_all_asm
.type mips_cache_invalidate_all_asm, @function
mips_cache_invalidate_all_asm:
    // Invalidate I-cache
    cache 1, 0($0)  // Index Writeback Invalidate Instruction
    // Invalidate D-cache
    cache 0, 0($0)  // Index Writeback Invalidate Data
    jr $ra

.global mips_cache_flush_range_asm
.type mips_cache_flush_range_asm, @function
mips_cache_flush_range_asm:
    // Start address in $a0, end address in $a1
    move $t0, $a0
1:  cache 0, 0($t0)  // Hit Writeback Invalidate Data
    addiu $t0, $t0, 64  // Cache line size
    bltu $t0, $a1, 1b
    jr $ra

.global mips_cache_invalidate_range_asm
.type mips_cache_invalidate_range_asm, @function
mips_cache_invalidate_range_asm:
    // Start address in $a0, end address in $a1
    move $t0, $a0
1:  cache 0, 0($t0)  // Hit Writeback Invalidate Data
    addiu $t0, $t0, 64  // Cache line size
    bltu $t0, $a1, 1b
    jr $ra

.global mips_cache_flush_line_asm
.type mips_cache_flush_line_asm, @function
mips_cache_flush_line_asm:
    // Address in $a0
    cache 0, 0($a0)  // Hit Writeback Invalidate Data
    jr $ra

.global mips_cache_invalidate_line_asm
.type mips_cache_invalidate_line_asm, @function
mips_cache_invalidate_line_asm:
    // Address in $a0
    cache 0, 0($a0)  // Hit Writeback Invalidate Data
    jr $ra

// ============================================================================
// PERFORMANCE MONITORING FUNCTIONS
// ============================================================================

.global mips_perf_read_cycle_counter_asm
.type mips_perf_read_cycle_counter_asm, @function
mips_perf_read_cycle_counter_asm:
    dmfc0 $v0, $25 // Read Performance register
    jr $ra

.global mips_perf_read_instruction_counter_asm
.type mips_perf_read_instruction_counter_asm, @function
mips_perf_read_instruction_counter_asm:
    dmfc0 $v0, $26 // Read Performance register + 1
    jr $ra

.global mips_perf_reset_counters_asm
.type mips_perf_reset_counters_asm, @function
mips_perf_reset_counters_asm:
    dmtc0 $0, $25  // Reset Performance register
    dmtc0 $0, $26  // Reset Performance register + 1
    jr $ra

// ============================================================================
// NEON/VFP CONTROL FUNCTIONS
// ============================================================================

.global mips_fpu_enable_asm
.type mips_fpu_enable_asm, @function
mips_fpu_enable_asm:
    // Enable FPU access
    mfc0 $t0, $12  // Read Status register
    ori $t0, $t0, 0x20000000  // Set CU1 bit
    mtc0 $t0, $12  // Write Status register
    jr $ra

.global mips_fpu_disable_asm
.type mips_fpu_disable_asm, @function
mips_fpu_disable_asm:
    // Disable FPU access
    mfc0 $t0, $12  // Read Status register
    andi $t0, $t0, ~0x20000000  // Clear CU1 bit
    mtc0 $t0, $12  // Write Status register
    jr $ra

.global mips_msa_enable_asm
.type mips_msa_enable_asm, @function
mips_msa_enable_asm:
    // Enable MSA access
    mfc0 $t0, $12  // Read Status register
    ori $t0, $t0, 0x40000000  // Set CU2 bit
    mtc0 $t0, $12  // Write Status register
    jr $ra

.global mips_msa_disable_asm
.type mips_msa_disable_asm, @function
mips_msa_disable_asm:
    // Disable MSA access
    mfc0 $t0, $12  // Read Status register
    andi $t0, $t0, ~0x40000000  // Clear CU2 bit
    mtc0 $t0, $12  // Write Status register
    jr $ra

// ============================================================================
// TIMING AND DELAY FUNCTIONS
// ============================================================================

.global mips_delay_cycles_asm
.type mips_delay_cycles_asm, @function
mips_delay_cycles_asm:
    // Number of cycles to delay in $a0
    beqz $a0, 2f
1:  addiu $a0, $a0, -1
    bnez $a0, 1b
2:  jr $ra

.global mips_delay_microseconds_asm
.type mips_delay_microseconds_asm, @function
mips_delay_microseconds_asm:
    // Microseconds to delay in $a0
    // Assuming 1GHz CPU, 1 microsecond = 1000 cycles
    sll $a0, $a0, 10  // Multiply by 1024 (close to 1000)
    beqz $a0, 2f
1:  addiu $a0, $a0, -1
    bnez $a0, 1b
2:  jr $ra

.global mips_delay_milliseconds_asm
.type mips_delay_milliseconds_asm, @function
mips_delay_milliseconds_asm:
    // Milliseconds to delay in $a0
    // Assuming 1GHz CPU, 1 millisecond = 1,000,000 cycles
    sll $a0, $a0, 20  // Multiply by 1,048,576 (close to 1,000,000)
    beqz $a0, 2f
1:  addiu $a0, $a0, -1
    bnez $a0, 1b
2:  jr $ra

// ============================================================================
// MEMORY BARRIER FUNCTIONS
// ============================================================================

.global mips_memory_barrier_asm
.type mips_memory_barrier_asm, @function
mips_memory_barrier_asm:
    sync
    jr $ra

.global mips_read_barrier_asm
.type mips_read_barrier_asm, @function
mips_read_barrier_asm:
    sync
    jr $ra

.global mips_write_barrier_asm
.type mips_write_barrier_asm, @function
mips_write_barrier_asm:
    sync
    jr $ra

.global mips_instruction_barrier_asm
.type mips_instruction_barrier_asm, @function
mips_instruction_barrier_asm:
    sync
    jr $ra

// ============================================================================
// ATOMIC OPERATIONS
// ============================================================================

.global mips_atomic_swap_asm
.type mips_atomic_swap_asm, @function
mips_atomic_swap_asm:
    // Address in $a0, new value in $a1
    // Returns old value in $v0
    ll $t0, 0($a0)     // Load linked
    move $v0, $t0       // Save old value
    sc $a1, 0($a0)     // Store conditional
    beqz $v1, mips_atomic_swap_asm  // Retry if failed
    jr $ra

.global mips_atomic_add_asm
.type mips_atomic_add_asm, @function
mips_atomic_add_asm:
    // Address in $a0, value to add in $a1
    // Returns new value in $v0
1:  ll $t0, 0($a0)     // Load linked
    add $t1, $t0, $a1  // Add value
    sc $t1, 0($a0)     // Store conditional
    beqz $t1, 1b        // Retry if failed
    move $v0, $t1       // Return new value
    jr $ra

.global mips_atomic_sub_asm
.type mips_atomic_sub_asm, @function
mips_atomic_sub_asm:
    // Address in $a0, value to subtract in $a1
    // Returns new value in $v0
1:  ll $t0, 0($a0)     // Load linked
    sub $t1, $t0, $a1  // Subtract value
    sc $t1, 0($a0)     // Store conditional
    beqz $t1, 1b        // Retry if failed
    move $v0, $t1       // Return new value
    jr $ra

.global mips_atomic_and_asm
.type mips_atomic_and_asm, @function
mips_atomic_and_asm:
    // Address in $a0, mask in $a1
    // Returns new value in $v0
1:  ll $t0, 0($a0)     // Load linked
    and $t1, $t0, $a1  // AND with mask
    sc $t1, 0($a0)     // Store conditional
    beqz $t1, 1b        // Retry if failed
    move $v0, $t1       // Return new value
    jr $ra

.global mips_atomic_or_asm
.type mips_atomic_or_asm, @function
mips_atomic_or_asm:
    // Address in $a0, mask in $a1
    // Returns new value in $v0
1:  ll $t0, 0($a0)     // Load linked
    or $t1, $t0, $a1   // OR with mask
    sc $t1, 0($a0)     // Store conditional
    beqz $t1, 1b        // Retry if failed
    move $v0, $t1       // Return new value
    jr $ra

.global mips_atomic_xor_asm
.type mips_atomic_xor_asm, @function
mips_atomic_xor_asm:
    // Address in $a0, mask in $a1
    // Returns new value in $v0
1:  ll $t0, 0($a0)     // Load linked
    xor $t1, $t0, $a1  // XOR with mask
    sc $t1, 0($a0)     // Store conditional
    beqz $t1, 1b        // Retry if failed
    move $v0, $t1       // Return new value
    jr $ra
