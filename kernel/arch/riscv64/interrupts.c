/*
 * ORION OS - RISC-V 64-bit Interrupt Management
 *
 * Core interrupt management system for RISC-V 64-bit architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// GLOBAL INTERRUPT CONTEXTS
// ============================================================================

// Interrupt controller context
static struct {
    bool initialized;
    uint32_t irq_count;
    uint32_t irq_enabled[32];  // 1024 IRQs / 32 = 32 words
    void (*irq_handlers[1024])(void);
    uint32_t irq_priorities[1024];
    uint32_t irq_stats[1024];
} riscv64_interrupt_context = {0};

// Exception handler context
static struct {
    bool initialized;
    void (*exception_handlers[16])(uint64_t cause, uint64_t epc, uint64_t tval);
    uint32_t exception_stats[16];
    uint32_t exception_count;
} riscv64_exception_context = {0};

// System call context
static struct {
    bool initialized;
    void (*syscall_handlers[64])(void);
    uint32_t syscall_stats[64];
    uint32_t syscall_count;
} riscv64_syscall_context = {0};

// Interrupt statistics
static struct {
    uint32_t total_interrupts;
    uint32_t total_exceptions;
    uint32_t total_syscalls;
    uint32_t spurious_interrupts;
    uint32_t interrupt_errors;
} riscv64_interrupt_stats = {0};

// ============================================================================
// INTERRUPT CONTROL FUNCTIONS
// ============================================================================

// Initialize interrupt system
void riscv64_interrupts_init(void)
{
    if (riscv64_interrupt_context.initialized) {
        return;
    }
    
    // Clear all interrupt contexts
    for (int i = 0; i < 32; i++) {
        riscv64_interrupt_context.irq_enabled[i] = 0;
    }
    
    for (int i = 0; i < 1024; i++) {
        riscv64_interrupt_context.irq_handlers[i] = NULL;
        riscv64_interrupt_context.irq_priorities[i] = 0;
        riscv64_interrupt_context.irq_stats[i] = 0;
    }
    
    // Initialize exception handlers
    for (int i = 0; i < 16; i++) {
        riscv64_exception_context.exception_handlers[i] = NULL;
        riscv64_exception_context.exception_stats[i] = 0;
    }
    
    // Initialize system call handlers
    for (int i = 0; i < 64; i++) {
        riscv64_syscall_context.syscall_handlers[i] = NULL;
        riscv64_syscall_context.syscall_stats[i] = 0;
    }
    
    // Set default exception handlers
    riscv64_exception_context.exception_handlers[RISCV64_EXC_INSTRUCTION_MISALIGNED] = riscv64_instruction_misaligned_handler;
    riscv64_exception_context.exception_handlers[RISCV64_EXC_INSTRUCTION_ACCESS_FAULT] = riscv64_instruction_access_fault_handler;
    riscv64_exception_context.exception_handlers[RISCV64_EXC_ILLEGAL_INSTRUCTION] = riscv64_illegal_instruction_handler;
    riscv64_exception_context.exception_handlers[RISCV64_EXC_BREAKPOINT] = riscv64_breakpoint_handler;
    riscv64_exception_context.exception_handlers[RISCV64_EXC_LOAD_ADDRESS_MISALIGNED] = riscv64_load_address_misaligned_handler;
    riscv64_exception_context.exception_handlers[RISCV64_EXC_LOAD_ACCESS_FAULT] = riscv64_load_access_fault_handler;
    riscv64_exception_context.exception_handlers[RISCV64_EXC_STORE_ADDRESS_MISALIGNED] = riscv64_store_address_misaligned_handler;
    riscv64_exception_context.exception_handlers[RISCV64_EXC_STORE_ACCESS_FAULT] = riscv64_store_access_fault_handler;
    riscv64_exception_context.exception_handlers[RISCV64_EXC_ECALL_U] = riscv64_ecall_u_handler;
    riscv64_exception_context.exception_handlers[RISCV64_EXC_ECALL_S] = riscv64_ecall_s_handler;
    riscv64_exception_context.exception_handlers[RISCV64_EXC_ECALL_M] = riscv64_ecall_m_handler;
    riscv64_exception_context.exception_handlers[RISCV64_EXC_INSTRUCTION_PAGE_FAULT] = riscv64_instruction_page_fault_handler;
    riscv64_exception_context.exception_handlers[RISCV64_EXC_LOAD_PAGE_FAULT] = riscv64_load_page_fault_handler;
    riscv64_exception_context.exception_handlers[RISCV64_EXC_STORE_PAGE_FAULT] = riscv64_store_page_fault_handler;
    
    // Set default interrupt handlers
    riscv64_interrupt_context.irq_handlers[RISCV64_IRQ_SOFTWARE] = riscv64_software_interrupt_handler;
    riscv64_interrupt_context.irq_handlers[RISCV64_IRQ_TIMER] = riscv64_timer_interrupt_handler;
    riscv64_interrupt_context.irq_handlers[RISCV64_IRQ_EXTERNAL] = riscv64_external_interrupt_handler;
    
    // Set default system call handlers
    riscv64_syscall_context.syscall_handlers[0] = riscv64_syscall_exit;
    riscv64_syscall_context.syscall_handlers[1] = riscv64_syscall_read;
    riscv64_syscall_context.syscall_handlers[2] = riscv64_syscall_write;
    riscv64_syscall_context.syscall_handlers[3] = riscv64_syscall_open;
    riscv64_syscall_context.syscall_handlers[4] = riscv64_syscall_close;
    riscv64_syscall_context.syscall_handlers[5] = riscv64_syscall_fork;
    riscv64_syscall_context.syscall_handlers[6] = riscv64_syscall_exec;
    riscv64_syscall_context.syscall_handlers[7] = riscv64_syscall_wait;
    riscv64_syscall_context.syscall_handlers[8] = riscv64_syscall_mmap;
    riscv64_syscall_context.syscall_handlers[9] = riscv64_syscall_munmap;
    riscv64_syscall_context.syscall_handlers[10] = riscv64_syscall_socket;
    riscv64_syscall_context.syscall_handlers[11] = riscv64_syscall_bind;
    riscv64_syscall_context.syscall_handlers[12] = riscv64_syscall_listen;
    riscv64_syscall_context.syscall_handlers[13] = riscv64_syscall_accept;
    riscv64_syscall_context.syscall_handlers[14] = riscv64_syscall_connect;
    riscv64_syscall_context.syscall_handlers[15] = riscv64_syscall_send;
    riscv64_syscall_context.syscall_handlers[16] = riscv64_syscall_recv;
    
    riscv64_interrupt_context.initialized = true;
    riscv64_exception_context.initialized = true;
    riscv64_syscall_context.initialized = true;
    
    riscv64_interrupt_context.irq_count = 1024;
    riscv64_exception_context.exception_count = 16;
    riscv64_syscall_context.syscall_count = 64;
    
    printf("RISC-V64: Interrupt system initialized\n");
}

// Enable/disable specific IRQ
void riscv64_interrupt_enable(uint32_t irq)
{
    if (irq >= 1024) {
        return;
    }
    
    uint32_t word_index = irq / 32;
    uint32_t bit_index = irq % 32;
    
    riscv64_interrupt_context.irq_enabled[word_index] |= (1U << bit_index);
    
    // Update hardware interrupt enable
    if (irq < 64) {
        uint64_t mie_val;
        asm volatile("csrr %0, mie" : "=r"(mie_val));
        mie_val |= (1ULL << irq);
        asm volatile("csrw mie, %0" : : "r"(mie_val));
    }
}

void riscv64_interrupt_disable(uint32_t irq)
{
    if (irq >= 1024) {
        return;
    }
    
    uint32_t word_index = irq / 32;
    uint32_t bit_index = irq % 32;
    
    riscv64_interrupt_context.irq_enabled[word_index] &= ~(1U << bit_index);
    
    // Update hardware interrupt enable
    if (irq < 64) {
        uint64_t mie_val;
        asm volatile("csrr %0, mie" : "=r"(mie_val));
        mie_val &= ~(1ULL << irq);
        asm volatile("csrw mie, %0" : : "r"(mie_val));
    }
}

// Check if IRQ is enabled
bool riscv64_interrupt_is_enabled(uint32_t irq)
{
    if (irq >= 1024) {
        return false;
    }
    
    uint32_t word_index = irq / 32;
    uint32_t bit_index = irq % 32;
    
    return (riscv64_interrupt_context.irq_enabled[word_index] & (1U << bit_index)) != 0;
}

// Set interrupt handler
void riscv64_interrupt_set_handler(uint32_t irq, void (*handler)(void))
{
    if (irq >= 1024 || !handler) {
        return;
    }
    
    riscv64_interrupt_context.irq_handlers[irq] = handler;
}

// Set interrupt priority
void riscv64_interrupt_set_priority(uint32_t irq, uint32_t priority)
{
    if (irq >= 1024) {
        return;
    }
    
    riscv64_interrupt_context.irq_priorities[irq] = priority;
}

// Get interrupt priority
uint32_t riscv64_interrupt_get_priority(uint32_t irq)
{
    if (irq >= 1024) {
        return 0;
    }
    
    return riscv64_interrupt_context.irq_priorities[irq];
}

// ============================================================================
// EXCEPTION HANDLER FUNCTIONS
// ============================================================================

// Set exception handler
void riscv64_exception_set_handler(uint32_t exception, void (*handler)(uint64_t cause, uint64_t epc, uint64_t tval))
{
    if (exception >= 16 || !handler) {
        return;
    }
    
    riscv64_exception_context.exception_handlers[exception] = handler;
}

// Get exception handler
void (*riscv64_exception_get_handler(uint32_t exception))(uint64_t cause, uint64_t epc, uint64_t tval)
{
    if (exception >= 16) {
        return NULL;
    }
    
    return riscv64_exception_context.exception_handlers[exception];
}

// ============================================================================
// SYSTEM CALL HANDLER FUNCTIONS
// ============================================================================

// Set system call handler
void riscv64_syscall_set_handler(uint32_t syscall, void (*handler)(void))
{
    if (syscall >= 64 || !handler) {
        return;
    }
    
    riscv64_syscall_context.syscall_handlers[syscall] = handler;
}

// Get system call handler
void (*riscv64_syscall_get_handler(uint32_t syscall))(void)
{
    if (syscall >= 64) {
        return NULL;
    }
    
    return riscv64_syscall_context.syscall_handlers[syscall];
}

// ============================================================================
// INTERRUPT DISPATCHING
// ============================================================================

// Dispatch interrupt
void riscv64_interrupt_dispatch(uint32_t irq)
{
    if (irq >= 1024) {
        riscv64_interrupt_stats.spurious_interrupts++;
        return;
    }
    
    if (!riscv64_interrupt_is_enabled(irq)) {
        riscv64_interrupt_stats.spurious_interrupts++;
        return;
    }
    
    void (*handler)(void) = riscv64_interrupt_context.irq_handlers[irq];
    if (!handler) {
        riscv64_interrupt_stats.spurious_interrupts++;
        return;
    }
    
    riscv64_interrupt_stats.total_interrupts++;
    riscv64_interrupt_context.irq_stats[irq]++;
    
    // Call interrupt handler
    handler();
}

// Dispatch exception
void riscv64_exception_dispatch(uint64_t cause, uint64_t epc, uint64_t tval)
{
    if (cause >= 16) {
        riscv64_interrupt_stats.interrupt_errors++;
        return;
    }
    
    void (*handler)(uint64_t cause, uint64_t epc, uint64_t tval) = riscv64_exception_context.exception_handlers[cause];
    if (!handler) {
        riscv64_interrupt_stats.interrupt_errors++;
        return;
    }
    
    riscv64_interrupt_stats.total_exceptions++;
    riscv64_exception_context.exception_stats[cause]++;
    
    // Call exception handler
    handler(cause, epc, tval);
}

// Dispatch system call
void riscv64_syscall_dispatch(uint32_t syscall)
{
    if (syscall >= 64) {
        riscv64_interrupt_stats.interrupt_errors++;
        return;
    }
    
    void (*handler)(void) = riscv64_syscall_context.syscall_handlers[syscall];
    if (!handler) {
        riscv64_interrupt_stats.interrupt_errors++;
        return;
    }
    
    riscv64_interrupt_stats.total_syscalls++;
    riscv64_syscall_context.syscall_stats[syscall]++;
    
    // Call system call handler
    handler();
}

// ============================================================================
// INTERRUPT STATISTICS
// ============================================================================

// Get interrupt statistics
uint32_t riscv64_interrupt_get_stats(uint32_t irq)
{
    if (irq >= 1024) {
        return 0;
    }
    
    return riscv64_interrupt_context.irq_stats[irq];
}

// Get exception statistics
uint32_t riscv64_exception_get_stats(uint32_t exception)
{
    if (exception >= 16) {
        return 0;
    }
    
    return riscv64_exception_context.exception_stats[exception];
}

// Get system call statistics
uint32_t riscv64_syscall_get_stats(uint32_t syscall)
{
    if (syscall >= 64) {
        return 0;
    }
    
    return riscv64_syscall_context.syscall_stats[syscall];
}

// Get total statistics
void riscv64_interrupt_get_total_stats(uint32_t *total_interrupts, uint32_t *total_exceptions, uint32_t *total_syscalls)
{
    if (total_interrupts) {
        *total_interrupts = riscv64_interrupt_stats.total_interrupts;
    }
    if (total_exceptions) {
        *total_exceptions = riscv64_interrupt_stats.total_exceptions;
    }
    if (total_syscalls) {
        *total_syscalls = riscv64_interrupt_stats.total_syscalls;
    }
}

// ============================================================================
// INTERRUPT STATUS FUNCTIONS
// ============================================================================

// Get pending interrupts
uint64_t riscv64_interrupt_get_pending(void)
{
    uint64_t mip_val;
    asm volatile("csrr %0, mip" : "=r"(mip_val));
    return mip_val;
}

// Get enabled interrupts
uint64_t riscv64_interrupt_get_enabled(void)
{
    uint64_t mie_val;
    asm volatile("csrr %0, mie" : "=r"(mie_val));
    return mie_val;
}

// Check if specific interrupt is pending
bool riscv64_interrupt_is_pending(uint32_t irq)
{
    if (irq >= 64) {
        return false;
    }
    
    uint64_t mip_val;
    asm volatile("csrr %0, mip" : "=r"(mip_val));
    return (mip_val & (1ULL << irq)) != 0;
}

// ============================================================================
// INTERRUPT PRIORITY MANAGEMENT
// ============================================================================

// Get highest priority pending interrupt
int riscv64_interrupt_get_highest_priority(void)
{
    uint64_t mip_val = riscv64_interrupt_get_pending();
    uint64_t mie_val = riscv64_interrupt_get_enabled();
    uint64_t active_interrupts = mip_val & mie_val;
    
    if (!active_interrupts) {
        return -1;  // No active interrupts
    }
    
    int highest_irq = -1;
    uint32_t highest_priority = 0;
    
    for (int i = 0; i < 64; i++) {
        if (active_interrupts & (1ULL << i)) {
            uint32_t priority = riscv64_interrupt_get_priority(i);
            if (priority > highest_priority) {
                highest_priority = priority;
                highest_irq = i;
            }
        }
    }
    
    return highest_irq;
}

// ============================================================================
// INTERRUPT CLEANUP
// ============================================================================

// Cleanup interrupt system
void riscv64_interrupts_cleanup(void)
{
    if (!riscv64_interrupt_context.initialized) {
        return;
    }
    
    // Disable all interrupts
    for (int i = 0; i < 1024; i++) {
        riscv64_interrupt_disable(i);
    }
    
    // Clear all handlers
    for (int i = 0; i < 1024; i++) {
        riscv64_interrupt_context.irq_handlers[i] = NULL;
    }
    
    // Clear all exception handlers
    for (int i = 0; i < 16; i++) {
        riscv64_exception_context.exception_handlers[i] = NULL;
    }
    
    // Clear all system call handlers
    for (int i = 0; i < 64; i++) {
        riscv64_syscall_context.syscall_handlers[i] = NULL;
    }
    
    riscv64_interrupt_context.initialized = false;
    riscv64_exception_context.initialized = false;
    riscv64_syscall_context.initialized = false;
    
    printf("RISC-V64: Interrupt system cleaned up\n");
}

// ============================================================================
// INTERRUPT DEBUG FUNCTIONS
// ============================================================================

// Print interrupt status
void riscv64_interrupt_print_status(void)
{
    printf("RISC-V64: Interrupt System Status\n");
    printf("  Initialized: %s\n", riscv64_interrupt_context.initialized ? "Yes" : "No");
    printf("  Total IRQs: %u\n", riscv64_interrupt_context.irq_count);
    printf("  Total Exceptions: %u\n", riscv64_exception_context.exception_count);
    printf("  Total System Calls: %u\n", riscv64_syscall_context.syscall_count);
    printf("  Total Interrupts: %u\n", riscv64_interrupt_stats.total_interrupts);
    printf("  Total Exceptions: %u\n", riscv64_interrupt_stats.total_exceptions);
    printf("  Total System Calls: %u\n", riscv64_interrupt_stats.total_syscalls);
    printf("  Spurious Interrupts: %u\n", riscv64_interrupt_stats.spurious_interrupts);
    printf("  Interrupt Errors: %u\n", riscv64_interrupt_stats.interrupt_errors);
}

// Print specific IRQ status
void riscv64_interrupt_print_irq_status(uint32_t irq)
{
    if (irq >= 1024) {
        printf("RISC-V64: Invalid IRQ number %u\n", irq);
        return;
    }
    
    printf("RISC-V64: IRQ %u Status\n", irq);
    printf("  Enabled: %s\n", riscv64_interrupt_is_enabled(irq) ? "Yes" : "No");
    printf("  Pending: %s\n", riscv64_interrupt_is_pending(irq) ? "Yes" : "No");
    printf("  Priority: %u\n", riscv64_interrupt_get_priority(irq));
    printf("  Handler: %p\n", riscv64_interrupt_context.irq_handlers[irq]);
    printf("  Statistics: %u\n", riscv64_interrupt_get_stats(irq));
}
