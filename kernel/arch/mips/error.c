/*
 * ORION OS - MIPS Error Handling
 *
 * Error handling for MIPS architecture
 * Handles error detection, reporting, and recovery
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// ERROR CONTEXT MANAGEMENT
// ============================================================================

static mips_error_context_t mips_error_context = {0};

void mips_error_init(void)
{
    memset(&mips_error_context, 0, sizeof(mips_error_context));

    // Set default error handling settings
    mips_error_context.error_count = 0;
    mips_error_context.last_error = MIPS_ERROR_NONE;
    mips_error_context.error_handler = NULL;
    mips_error_context.recovery_enabled = true;

    printf("MIPS: Error handling initialized\n");
}

mips_error_context_t *mips_error_get_context(void)
{
    return &mips_error_context;
}

// ============================================================================
// ERROR DETECTION
// ============================================================================

mips_error_t mips_error_detect(void)
{
    uint64_t status = mips_cpu_read_cp0_status();
    uint64_t cause = mips_cpu_read_cp0_cause();

    // Check for various error conditions
    if (status & MIPS_STATUS_ERL)
    {
        return MIPS_ERROR_ERL_SET;
    }

    if (status & MIPS_STATUS_EXL)
    {
        return MIPS_ERROR_EXL_SET;
    }

    if (cause & MIPS_CAUSE_BD)
    {
        return MIPS_ERROR_BRANCH_DELAY;
    }

    if (cause & MIPS_CAUSE_CE)
    {
        return MIPS_ERROR_COPROCESSOR;
    }

    if (cause & MIPS_CAUSE_DC)
    {
        return MIPS_ERROR_DEBUG;
    }

    if (cause & MIPS_CAUSE_RI)
    {
        return MIPS_ERROR_RESERVED_INSTRUCTION;
    }

    if (cause & MIPS_CAUSE_OV)
    {
        return MIPS_ERROR_OVERFLOW;
    }

    if (cause & MIPS_CAUSE_TR)
    {
        return MIPS_ERROR_TRAP;
    }

    if (cause & MIPS_CAUSE_FPE)
    {
        return MIPS_ERROR_FLOATING_POINT;
    }

    if (cause & MIPS_CAUSE_ADE)
    {
        return MIPS_ERROR_ADDRESS_ERROR;
    }

    if (cause & MIPS_CAUSE_IBE)
    {
        return MIPS_ERROR_INSTRUCTION_BUS_ERROR;
    }

    if (cause & MIPS_CAUSE_DBE)
    {
        return MIPS_ERROR_DATA_BUS_ERROR;
    }

    if (cause & MIPS_CAUSE_SYS)
    {
        return MIPS_ERROR_SYSTEM_CALL;
    }

    if (cause & MIPS_CAUSE_BP)
    {
        return MIPS_ERROR_BREAKPOINT;
    }

    if (cause & MIPS_CAUSE_II)
    {
        return MIPS_ERROR_INTERRUPT;
    }

    return MIPS_ERROR_NONE;
}

bool mips_error_is_present(void)
{
    return mips_error_detect() != MIPS_ERROR_NONE;
}

mips_error_t mips_error_get_last(void)
{
    return mips_error_context.last_error;
}

uint32_t mips_error_get_count(void)
{
    return mips_error_context.error_count;
}

// ============================================================================
// ERROR HANDLING
// ============================================================================

void mips_error_set_handler(mips_error_handler_t handler)
{
    mips_error_context.error_handler = handler;
    printf("MIPS: Error handler set to %p\n", handler);
}

mips_error_handler_t mips_error_get_handler(void)
{
    return mips_error_context.error_handler;
}

void mips_error_handle(mips_error_t error, void *context)
{
    // Update error context
    mips_error_context.last_error = error;
    mips_error_context.error_count++;

    // Log error
    printf("MIPS: Error detected: %s (0x%x)\n", mips_error_get_string(error), error);

    // Call error handler if set
    if (mips_error_context.error_handler)
    {
        mips_error_context.error_handler(error, context);
    }

    // Attempt recovery if enabled
    if (mips_error_context.recovery_enabled)
    {
        mips_error_recover(error);
    }
}

void mips_error_handle_with_context(mips_error_t error, mips_error_context_info_t *context_info)
{
    // Handle error with detailed context
    mips_error_handle(error, context_info);

    // Log additional context information
    if (context_info)
    {
        printf("MIPS: Error context - PC: 0x%llx, EPC: 0x%llx, BadVAddr: 0x%llx\n",
               context_info->program_counter,
               context_info->exception_program_counter,
               context_info->bad_virtual_address);
    }
}

// ============================================================================
// ERROR RECOVERY
// ============================================================================

void mips_error_enable_recovery(void)
{
    mips_error_context.recovery_enabled = true;
    printf("MIPS: Error recovery enabled\n");
}

void mips_error_disable_recovery(void)
{
    mips_error_context.recovery_enabled = false;
    printf("MIPS: Error recovery disabled\n");
}

bool mips_error_is_recovery_enabled(void)
{
    return mips_error_context.recovery_enabled;
}

void mips_error_recover(mips_error_t error)
{
    if (!mips_error_context.recovery_enabled)
    {
        return;
    }

    printf("MIPS: Attempting error recovery for: %s\n", mips_error_get_string(error));

    switch (error)
    {
    case MIPS_ERROR_OVERFLOW:
        mips_error_recover_overflow();
        break;
    case MIPS_ERROR_FLOATING_POINT:
        mips_error_recover_fpu();
        break;
    case MIPS_ERROR_ADDRESS_ERROR:
        mips_error_recover_address();
        break;
    case MIPS_ERROR_BUS_ERROR:
        mips_error_recover_bus();
        break;
    case MIPS_ERROR_CACHE_ERROR:
        mips_error_recover_cache();
        break;
    case MIPS_ERROR_TLB_ERROR:
        mips_error_recover_tlb();
        break;
    default:
        printf("MIPS: No recovery method for error: %s\n", mips_error_get_string(error));
        break;
    }
}

void mips_error_recover_overflow(void)
{
    printf("MIPS: Recovering from overflow error\n");

    // Clear overflow flag
    uint64_t cause = mips_cpu_read_cp0_cause();
    cause &= ~MIPS_CAUSE_OV;
    mips_cpu_write_cp0_cause(cause);

    // Continue execution
    printf("MIPS: Overflow recovery complete\n");
}

void mips_error_recover_fpu(void)
{
    printf("MIPS: Recovering from FPU error\n");

    // Clear FPU error flag
    uint64_t cause = mips_cpu_read_cp0_cause();
    cause &= ~MIPS_CAUSE_FPE;
    mips_cpu_write_cp0_cause(cause);

    // Reset FPU state if needed
    if (mips_extensions_has_vfp())
    {
        // Reset VFP registers
        __asm__ volatile("cfc1 $t0, $31");
        __asm__ volatile("ctc1 $zero, $31");
        printf("MIPS: VFP reset complete\n");
    }

    printf("MIPS: FPU recovery complete\n");
}

void mips_error_recover_address(void)
{
    printf("MIPS: Recovering from address error\n");

    // Clear address error flag
    uint64_t cause = mips_cpu_read_cp0_cause();
    cause &= ~(MIPS_CAUSE_ADE | MIPS_CAUSE_IBE | MIPS_CAUSE_DBE);
    mips_cpu_write_cp0_cause(cause);

    // Invalidate TLB entries that might be causing issues
    mips_tlb_invalidate_all();

    printf("MIPS: Address error recovery complete\n");
}

void mips_error_recover_bus(void)
{
    printf("MIPS: Recovering from bus error\n");

    // Clear bus error flags
    uint64_t cause = mips_cpu_read_cp0_cause();
    cause &= ~(MIPS_CAUSE_IBE | MIPS_CAUSE_DBE);
    mips_cpu_write_cp0_cause(cause);

    // Reset bus interface if possible
    // This would be hardware-specific

    printf("MIPS: Bus error recovery complete\n");
}

void mips_error_recover_cache(void)
{
    printf("MIPS: Recovering from cache error\n");

    // Clear cache error flags
    uint64_t cause = mips_cpu_read_cp0_cause();
    cause &= ~MIPS_CAUSE_CACHE_ERROR;
    mips_cpu_write_cp0_cause(cause);

    // Invalidate and reinitialize caches
    mips_cache_invalidate_all();
    mips_cache_sync_all();

    printf("MIPS: Cache error recovery complete\n");
}

void mips_error_recover_tlb(void)
{
    printf("MIPS: Recovering from TLB error\n");

    // Clear TLB error flags
    uint64_t cause = mips_cpu_read_cp0_cause();
    cause &= ~MIPS_CAUSE_TLB_ERROR;
    mips_cpu_write_cp0_cause(cause);

    // Invalidate and reinitialize TLB
    mips_tlb_invalidate_all();

    printf("MIPS: TLB error recovery complete\n");
}

// ============================================================================
// ERROR REPORTING
// ============================================================================

const char *mips_error_get_string(mips_error_t error)
{
    switch (error)
    {
    case MIPS_ERROR_NONE:
        return "No Error";
    case MIPS_ERROR_ERL_SET:
        return "Error Level Set";
    case MIPS_ERROR_EXL_SET:
        return "Exception Level Set";
    case MIPS_ERROR_BRANCH_DELAY:
        return "Branch Delay";
    case MIPS_ERROR_COPROCESSOR:
        return "Coprocessor Error";
    case MIPS_ERROR_DEBUG:
        return "Debug Error";
    case MIPS_ERROR_RESERVED_INSTRUCTION:
        return "Reserved Instruction";
    case MIPS_ERROR_OVERFLOW:
        return "Overflow";
    case MIPS_ERROR_TRAP:
        return "Trap";
    case MIPS_ERROR_FLOATING_POINT:
        return "Floating Point Error";
    case MIPS_ERROR_ADDRESS_ERROR:
        return "Address Error";
    case MIPS_ERROR_INSTRUCTION_BUS_ERROR:
        return "Instruction Bus Error";
    case MIPS_ERROR_DATA_BUS_ERROR:
        return "Data Bus Error";
    case MIPS_ERROR_SYSTEM_CALL:
        return "System Call";
    case MIPS_ERROR_BREAKPOINT:
        return "Breakpoint";
    case MIPS_ERROR_INTERRUPT:
        return "Interrupt";
    case MIPS_ERROR_CACHE_ERROR:
        return "Cache Error";
    case MIPS_ERROR_TLB_ERROR:
        return "TLB Error";
    case MIPS_ERROR_MEMORY_ERROR:
        return "Memory Error";
    case MIPS_ERROR_TIMEOUT:
        return "Timeout";
    case MIPS_ERROR_INVALID_OPERATION:
        return "Invalid Operation";
    case MIPS_ERROR_HARDWARE_FAULT:
        return "Hardware Fault";
    case MIPS_ERROR_SOFTWARE_FAULT:
        return "Software Fault";
    default:
        return "Unknown Error";
    }
}

void mips_error_print_context(void)
{
    printf("MIPS: Error Context:\n");
    printf("  Error Count: %u\n", mips_error_context.error_count);
    printf("  Last Error: %s (0x%x)\n",
           mips_error_get_string(mips_error_context.last_error),
           mips_error_context.last_error);
    printf("  Error Handler: %p\n", mips_error_context.error_handler);
    printf("  Recovery Enabled: %s\n",
           mips_error_context.recovery_enabled ? "yes" : "no");
}

void mips_error_print_detailed_context(void)
{
    mips_error_print_context();

    // Print current CPU state
    uint64_t status = mips_cpu_read_cp0_status();
    uint64_t cause = mips_cpu_read_cp0_cause();
    uint64_t epc = mips_cpu_read_cp0_epc();
    uint64_t badvaddr = mips_cpu_read_cp0_register(MIPS_CP0_BADVADDR);

    printf("MIPS: CPU State:\n");
    printf("  Status: 0x%llx\n", status);
    printf("  Cause: 0x%llx\n", cause);
    printf("  EPC: 0x%llx\n", epc);
    printf("  BadVAddr: 0x%llx\n", badvaddr);

    // Print current mode
    mips_cpu_mode_t mode = mips_cpu_get_mode();
    printf("  Current Mode: %d\n", mode);

    // Print interrupt status
    bool interrupts_enabled = mips_interrupts_are_enabled();
    printf("  Interrupts Enabled: %s\n", interrupts_enabled ? "yes" : "no");
}

// ============================================================================
// ERROR STATISTICS
// ============================================================================

void mips_error_reset_statistics(void)
{
    mips_error_context.error_count = 0;
    mips_error_context.last_error = MIPS_ERROR_NONE;

    printf("MIPS: Error statistics reset\n");
}

void mips_error_print_statistics(void)
{
    printf("MIPS: Error Statistics:\n");
    printf("  Total Errors: %u\n", mips_error_context.error_count);
    printf("  Last Error: %s\n", mips_error_get_string(mips_error_context.last_error));
    printf("  Recovery Enabled: %s\n",
           mips_error_context.recovery_enabled ? "yes" : "no");
    printf("  Error Handler: %s\n",
           mips_error_context.error_handler ? "set" : "not set");
}

// ============================================================================
// ERROR UTILITIES
// ============================================================================

bool mips_error_is_recoverable(mips_error_t error)
{
    switch (error)
    {
    case MIPS_ERROR_OVERFLOW:
    case MIPS_ERROR_FLOATING_POINT:
    case MIPS_ERROR_ADDRESS_ERROR:
    case MIPS_ERROR_BUS_ERROR:
    case MIPS_ERROR_CACHE_ERROR:
    case MIPS_ERROR_TLB_ERROR:
    case MIPS_ERROR_TIMEOUT:
        return true;
    case MIPS_ERROR_HARDWARE_FAULT:
    case MIPS_ERROR_INVALID_OPERATION:
    case MIPS_ERROR_ERL_SET:
    case MIPS_ERROR_EXL_SET:
        return false;
    default:
        return false;
    }
}

bool mips_error_is_critical(mips_error_t error)
{
    switch (error)
    {
    case MIPS_ERROR_HARDWARE_FAULT:
    case MIPS_ERROR_MEMORY_ERROR:
    case MIPS_ERROR_ERL_SET:
    case MIPS_ERROR_EXL_SET:
        return true;
    default:
        return false;
    }
}

void mips_error_panic(mips_error_t error, const char *message)
{
    printf("MIPS: PANIC - Critical error: %s\n", mips_error_get_string(error));
    if (message)
    {
        printf("MIPS: PANIC - Message: %s\n", message);
    }

    // Print detailed context
    mips_error_print_detailed_context();

    // Halt system
    printf("MIPS: System halted due to critical error\n");
    mips_cpu_halt();
}

void mips_error_assert(bool condition, const char *message)
{
    if (!condition)
    {
        printf("MIPS: ASSERTION FAILED: %s\n", message);
        mips_error_panic(MIPS_ERROR_SOFTWARE_FAULT, message);
    }
}
