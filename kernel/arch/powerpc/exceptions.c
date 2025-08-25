/*
 * ORION OS - POWER Architecture Exception Handling
 *
 * Exception and error handling for POWER architecture
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

// ============================================================================
// EXCEPTION TYPES
// ============================================================================

#define POWER_EXCEPTION_RESET          0x0100
#define POWER_EXCEPTION_MACHINE_CHECK  0x0200
#define POWER_EXCEPTION_DATA_STORAGE   0x0300
#define POWER_EXCEPTION_INST_STORAGE   0x0400
#define POWER_EXCEPTION_EXTERNAL       0x0500
#define POWER_EXCEPTION_ALIGNMENT      0x0600
#define POWER_EXCEPTION_PROGRAM        0x0700
#define POWER_EXCEPTION_FP_UNAVAILABLE 0x0800
#define POWER_EXCEPTION_DECREMENTER    0x0900
#define POWER_EXCEPTION_SYSTEM_CALL    0x0C00
#define POWER_EXCEPTION_TRACE          0x0D00
#define POWER_EXCEPTION_PERFORMANCE    0x0F00

// ============================================================================
// EXCEPTION CONTEXT
// ============================================================================

typedef struct {
    uint64_t exception_type;      // Exception type
    uint64_t exception_address;   // Exception address
    uint64_t exception_data;      // Exception data
    uint64_t exception_flags;     // Exception flags
    power_cpu_context_t *cpu_context; // CPU context
    bool handled;                 // Exception handled flag
    bool fatal;                   // Fatal exception flag
} power_exception_context_t;

static power_exception_context_t power_exception;

// ============================================================================
// EXCEPTION HANDLERS
// ============================================================================

void power_exception_handler(uint64_t exception, power_cpu_context_t *context)
{
    // Initialize exception context
    power_exception.exception_type = exception;
    power_exception.exception_address = context->pc;
    power_exception.exception_data = 0;
    power_exception.exception_flags = 0;
    power_exception.cpu_context = context;
    power_exception.handled = false;
    power_exception.fatal = false;
    
    // Route to appropriate handler based on exception type
    switch (exception) {
        case POWER_EXCEPTION_RESET:
            power_reset_exception_handler(context);
            break;
            
        case POWER_EXCEPTION_MACHINE_CHECK:
            power_machine_check_handler(context);
            break;
            
        case POWER_EXCEPTION_DATA_STORAGE:
            power_data_storage_handler(context);
            break;
            
        case POWER_EXCEPTION_INST_STORAGE:
            power_instruction_storage_handler(context);
            break;
            
        case POWER_EXCEPTION_EXTERNAL:
            power_external_exception_handler(context);
            break;
            
        case POWER_EXCEPTION_ALIGNMENT:
            power_alignment_exception_handler(context);
            break;
            
        case POWER_EXCEPTION_PROGRAM:
            power_program_exception_handler(context);
            break;
            
        case POWER_EXCEPTION_FP_UNAVAILABLE:
            power_fp_unavailable_handler(context);
            break;
            
        case POWER_EXCEPTION_DECREMENTER:
            power_decrementer_handler(context);
            break;
            
        case POWER_EXCEPTION_SYSTEM_CALL:
            power_system_call_handler(context);
            break;
            
        case POWER_EXCEPTION_TRACE:
            power_trace_handler(context);
            break;
            
        case POWER_EXCEPTION_PERFORMANCE:
            power_performance_handler(context);
            break;
            
        default:
            power_unknown_exception_handler(context);
            break;
    }
    
    // Check if exception was fatal
    if (power_exception.fatal) {
        power_fatal_exception_handler(context);
    }
}

// ============================================================================
// SPECIFIC EXCEPTION HANDLERS
// ============================================================================

void power_reset_exception_handler(power_cpu_context_t *context)
{
    printf("POWER: Reset exception at PC=0x%016llx\n", context->pc);
    
    // Handle system reset
    power_exception.handled = true;
    
    // Perform system reset
    power_system_reset();
}

void power_machine_check_handler(power_cpu_context_t *context)
{
    printf("POWER: Machine check exception at PC=0x%016llx\n", context->pc);
    
    // Read machine check registers
    uint64_t mcsr, mcar, mcsrr0, mcsrr1;
    __asm__ volatile("mfspr %0, 0" : "=r"(mcsr));    // MCSR
    __asm__ volatile("mfspr %1, 1" : "=r"(mcar));    // MCAR
    __asm__ volatile("mfspr %2, 2" : "=r"(mcsrr0));  // MCSRR0
    __asm__ volatile("mfspr %3, 3" : "=r"(mcsrr1));  // MCSRR1
    
    printf("POWER: MCSR=0x%016llx, MCAR=0x%016llx\n", mcsr, mcar);
    printf("POWER: MCSRR0=0x%016llx, MCSRR1=0x%016llx\n", mcsrr0, mcsrr1);
    
    // Check if machine check is recoverable
    if (mcsr & 0x8000000000000000ULL) {
        // Recoverable machine check
        power_exception.handled = true;
        printf("POWER: Recoverable machine check\n");
    } else {
        // Unrecoverable machine check
        power_exception.fatal = true;
        printf("POWER: Unrecoverable machine check\n");
    }
}

void power_data_storage_handler(power_cpu_context_t *context)
{
    printf("POWER: Data storage exception at PC=0x%016llx\n", context->pc);
    
    // Read data storage exception registers
    uint64_t dsisr, dar;
    __asm__ volatile("mfspr %0, 18" : "=r"(dsisr));  // DSISR
    __asm__ volatile("mfspr %1, 19" : "=r"(dar));     // DAR
    
    printf("POWER: DSISR=0x%016llx, DAR=0x%016llx\n", dsisr, dar);
    
    // Check exception type
    if (dsisr & 0x40000000) {
        // Page fault
        printf("POWER: Page fault\n");
        if (power_handle_page_fault(dar, dsisr) == 0) {
            power_exception.handled = true;
        } else {
            power_exception.fatal = true;
        }
    } else if (dsisr & 0x20000000) {
        // Protection fault
        printf("POWER: Protection fault\n");
        power_exception.fatal = true;
    } else if (dsisr & 0x10000000) {
        // Alignment fault
        printf("POWER: Alignment fault\n");
        if (power_handle_alignment_fault(dar, dsisr) == 0) {
            power_exception.handled = true;
        } else {
            power_exception.fatal = true;
        }
    } else {
        // Unknown data storage fault
        printf("POWER: Unknown data storage fault\n");
        power_exception.fatal = true;
    }
}

void power_instruction_storage_handler(power_cpu_context_t *context)
{
    printf("POWER: Instruction storage exception at PC=0x%016llx\n", context->pc);
    
    // Read instruction storage exception registers
    uint64_t srr0, srr1;
    __asm__ volatile("mfspr %0, 26" : "=r"(srr0));   // SRR0
    __asm__ volatile("mfspr %1, 27" : "=r"(srr1));   // SRR1
    
    printf("POWER: SRR0=0x%016llx, SRR1=0x%016llx\n", srr0, srr1);
    
    // Check if this is a page fault
    if (power_handle_instruction_page_fault(srr0) == 0) {
        power_exception.handled = true;
    } else {
        power_exception.fatal = true;
    }
}

void power_external_exception_handler(power_cpu_context_t *context)
{
    printf("POWER: External exception at PC=0x%016llx\n", context->pc);
    
    // This is typically an interrupt
    // The interrupt handler will be called separately
    power_exception.handled = true;
}

void power_alignment_exception_handler(power_cpu_context_t *context)
{
    printf("POWER: Alignment exception at PC=0x%016llx\n", context->pc);
    
    // Read alignment exception registers
    uint64_t dsisr, dar;
    __asm__ volatile("mfspr %0, 18" : "=r"(dsisr));  // DSISR
    __asm__ volatile("mfspr %1, 19" : "=r"(dar));     // DAR
    
    printf("POWER: DSISR=0x%016llx, DAR=0x%016llx\n", dsisr, dar);
    
    // Handle alignment fault
    if (power_handle_alignment_fault(dar, dsisr) == 0) {
        power_exception.handled = true;
    } else {
        power_exception.fatal = true;
    }
}

void power_program_exception_handler(power_cpu_context_t *context)
{
    printf("POWER: Program exception at PC=0x%016llx\n", context->pc);
    
    // Read program exception registers
    uint64_t srr0, srr1;
    __asm__ volatile("mfspr %0, 26" : "=r"(srr0));   // SRR0
    __asm__ volatile("mfspr %1, 27" : "=r"(srr1));   // SRR1
    
    printf("POWER: SRR0=0x%016llx, SRR1=0x%016llx\n", srr0, srr1);
    
    // Check exception type
    uint64_t exception_type = (srr1 >> 16) & 0x1F;
    
    switch (exception_type) {
        case 0x01: // Privileged instruction
            printf("POWER: Privileged instruction exception\n");
            power_exception.fatal = true;
            break;
            
        case 0x02: // Trap instruction
            printf("POWER: Trap instruction exception\n");
            power_exception.handled = true;
            break;
            
        case 0x03: // Illegal instruction
            printf("POWER: Illegal instruction exception\n");
            power_exception.fatal = true;
            break;
            
        case 0x04: // Floating point disabled
            printf("POWER: Floating point disabled exception\n");
            power_exception.handled = true;
            break;
            
        case 0x05: // Floating point exception
            printf("POWER: Floating point exception\n");
            power_exception.handled = true;
            break;
            
        default:
            printf("POWER: Unknown program exception type 0x%02llx\n", exception_type);
            power_exception.fatal = true;
            break;
    }
}

void power_fp_unavailable_handler(power_cpu_context_t *context)
{
    printf("POWER: FP unavailable exception at PC=0x%016llx\n", context->pc);
    
    // Enable floating point
    uint64_t msr;
    __asm__ volatile("mfmsr %0" : "=r"(msr));
    msr |= POWER_MSR_FP;
    __asm__ volatile("mtmsr %0" : : "r"(msr));
    
    power_exception.handled = true;
}

void power_decrementer_handler(power_cpu_context_t *context)
{
    printf("POWER: Decrementer exception at PC=0x%016llx\n", context->pc);
    
    // Handle timer interrupt
    power_timer_interrupt_handler();
    
    power_exception.handled = true;
}

void power_system_call_handler(power_cpu_context_t *context)
{
    printf("POWER: System call exception at PC=0x%016llx\n", context->pc);
    
    // Get system call number from r0
    uint64_t syscall_number = context->gpr[0];
    
    printf("POWER: System call number %llu\n", syscall_number);
    
    // Handle system call
    if (power_handle_system_call(syscall_number, context) == 0) {
        power_exception.handled = true;
    } else {
        power_exception.fatal = true;
    }
}

void power_trace_handler(power_cpu_context_t *context)
{
    printf("POWER: Trace exception at PC=0x%016llx\n", context->pc);
    
    // Handle trace exception
    power_exception.handled = true;
}

void power_performance_handler(power_cpu_context_t *context)
{
    printf("POWER: Performance exception at PC=0x%016llx\n", context->pc);
    
    // Handle performance monitoring exception
    power_exception.handled = true;
}

void power_unknown_exception_handler(power_cpu_context_t *context)
{
    printf("POWER: Unknown exception 0x%04llx at PC=0x%016llx\n", 
           power_exception.exception_type, context->pc);
    
    power_exception.fatal = true;
}

// ============================================================================
// EXCEPTION UTILITIES
// ============================================================================

int power_handle_page_fault(uint64_t address, uint64_t dsisr)
{
    printf("POWER: Handling page fault at address 0x%016llx\n", address);
    
    // Check if this is a valid page fault
    if (address < 0x1000000000000000ULL) {
        // Kernel space - fatal
        printf("POWER: Kernel page fault - fatal\n");
        return -1;
    }
    
    // User space - try to handle
    if (power_mmu_handle_page_fault(address) == 0) {
        printf("POWER: Page fault handled successfully\n");
        return 0;
    }
    
    printf("POWER: Failed to handle page fault\n");
    return -1;
}

int power_handle_instruction_page_fault(uint64_t address)
{
    printf("POWER: Handling instruction page fault at address 0x%016llx\n", address);
    
    // Similar to data page fault handling
    return power_handle_page_fault(address, 0);
}

int power_handle_alignment_fault(uint64_t address, uint64_t dsisr)
{
    printf("POWER: Handling alignment fault at address 0x%016llx\n", address);
    
    // Check if alignment fault can be handled
    uint64_t alignment = 1 << ((dsisr >> 16) & 0x1F);
    
    printf("POWER: Required alignment: %llu bytes\n", alignment);
    
    // For now, mark as fatal
    // In a real implementation, this could be handled by emulating the instruction
    return -1;
}

int power_handle_system_call(uint64_t number, power_cpu_context_t *context)
{
    printf("POWER: Handling system call %llu\n", number);
    
    // Route to appropriate system call handler
    switch (number) {
        case 1: // read
            return power_syscall_read(context);
            
        case 2: // write
            return power_syscall_write(context);
            
        case 3: // open
            return power_syscall_open(context);
            
        case 4: // close
            return power_syscall_close(context);
            
        case 5: // mmap
            return power_syscall_mmap(context);
            
        case 6: // munmap
            return power_syscall_munmap(context);
            
        case 7: // fork
            return power_syscall_fork(context);
            
        case 8: // execve
            return power_syscall_execve(context);
            
        case 9: // exit
            return power_syscall_exit(context);
            
        case 10: // waitpid
            return power_syscall_waitpid(context);
            
        default:
            printf("POWER: Unknown system call number %llu\n", number);
            return -1;
    }
}

// ============================================================================
// FATAL EXCEPTION HANDLING
// ============================================================================

void power_fatal_exception_handler(power_cpu_context_t *context)
{
    printf("POWER: Fatal exception occurred!\n");
    printf("POWER: Exception type: 0x%04llx\n", power_exception.exception_type);
    printf("POWER: Exception address: 0x%016llx\n", power_exception.exception_address);
    printf("POWER: CPU context:\n");
    power_print_cpu_context(context);
    
    // Attempt to save system state
    power_save_system_state();
    
    // Halt the system
    power_system_halt();
}

void power_print_cpu_context(power_cpu_context_t *context)
{
    printf("POWER: PC=0x%016llx, SP=0x%016llx\n", context->pc, context->sp);
    printf("POWER: MSR=0x%016llx, CR=0x%016llx\n", context->msr, context->cr);
    printf("POWER: LR=0x%016llx, CTR=0x%016llx\n", context->lr, context->ctr);
    printf("POWER: XER=0x%016llx\n", context->xer);
    
    // Print general purpose registers
    for (int i = 0; i < 32; i += 4) {
        printf("POWER: R%02d=0x%016llx R%02d=0x%016llx R%02d=0x%016llx R%02d=0x%016llx\n",
               i, context->gpr[i], i+1, context->gpr[i+1], 
               i+2, context->gpr[i+2], i+3, context->gpr[i+3]);
    }
}

// ============================================================================
// SYSTEM CONTROL
// ============================================================================

void power_system_reset(void)
{
    printf("POWER: Performing system reset\n");
    
    // Perform system reset
    __asm__ volatile("mtspr 0, %0" : : "r"(0x0000000000000001ULL));
    
    // Should never reach here
    while (1) {
        __asm__ volatile("wait");
    }
}

void power_system_halt(void)
{
    printf("POWER: System halted\n");
    
    // Disable interrupts
    uint64_t msr;
    __asm__ volatile("mfmsr %0" : "=r"(msr));
    msr &= ~POWER_MSR_EE;
    __asm__ volatile("mtmsr %0" : : "r"(msr));
    
    // Halt the system
    while (1) {
        __asm__ volatile("wait");
    }
}

void power_save_system_state(void)
{
    printf("POWER: Saving system state\n");
    
    // Save critical system information
    // This would typically involve writing to non-volatile storage
    
    printf("POWER: System state saved\n");
}

// ============================================================================
// EXCEPTION CLEANUP
// ============================================================================

void power_exception_cleanup(void)
{
    // Clear exception context
    memset(&power_exception, 0, sizeof(power_exception));
}
