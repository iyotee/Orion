/*
 * Orion Operating System - x86_64 Architecture Stubs
 *
 * Temporary stub implementations for architecture-specific functions.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/kernel.h>
#include <orion/types.h>

// ========================================
// CPU MANAGEMENT STUBS
// ========================================

// Get current CPU ID (single CPU for now)
uint32_t arch_get_current_cpu(void) {
    return 0; // Always CPU 0 for now
}

// Initialize interrupt subsystem
void arch_interrupt_init(void) {
    // This is now implemented in interrupts.c
    // Call the real implementation
    extern void arch_interrupt_init_real(void);
    arch_interrupt_init_real();
}





// ========================================
// SYSCALL ENTRY STUB
// ========================================

// Syscall entry point (assembly would be better, but stub for now)
void syscall_entry(void) {
    // This should be implemented in assembly
    // For now, just a placeholder
    kerror("syscall_entry called - not implemented!");
    arch_halt();
}
