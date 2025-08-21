/*
 * Orion Operating System - x86_64 Interrupt Handlers
 *
 * C-language interrupt and exception handlers for x86_64 architecture.
 *
 * Developed by Jérémy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/kernel.h>
#include <orion/types.h>
#include <orion/scheduler.h>

// ========================================
// EXCEPTION HANDLERS
// ========================================

// Common exception handler
void isr_handler(uint64_t isr_num, uint64_t error_code, uint64_t rip, 
                uint64_t cs, uint64_t rflags, uint64_t rsp, uint64_t ss) {
    kerror("Exception %llu at RIP=0x%llx, CS=0x%llx, RFLAGS=0x%llx", 
           (unsigned long long)isr_num, (unsigned long long)rip, 
           (unsigned long long)cs, (unsigned long long)rflags);
    
    if (error_code != 0xFFFFFFFFFFFFFFFF) {
        kerror("Error code: 0x%llx", (unsigned long long)error_code);
    }
    
    // Handle specific exceptions
    switch (isr_num) {
        case 0:  // Divide by zero
            kerror("Divide by zero exception");
            break;
        case 6:  // Invalid opcode
            kerror("Invalid opcode exception");
            break;
        case 8:  // Double fault
            kerror("Double fault - system halted");
            arch_halt();
            break;
        case 13: // General protection fault
            kerror("General protection fault");
            break;
        case 14: // Page fault
            kerror("Page fault at 0x%llx", (unsigned long long)rip);
            // TODO: Implement page fault handler
            break;
        default:
            kerror("Unhandled exception %llu", (unsigned long long)isr_num);
            break;
    }
    
    // For now, halt the system on critical exceptions
    if (isr_num >= 8 && isr_num <= 14) {
        kerror("Critical exception - system halted");
        arch_halt();
    }
}

// Common IRQ handler
void irq_handler(uint64_t irq_num) {
    // Handle specific IRQs
    switch (irq_num) {
        case 0:  // PIT timer
            // TODO: Handle timer tick
            break;
        case 1:  // Keyboard
            // TODO: Handle keyboard input
            break;
        default:
            kdebug("Unhandled IRQ %llu", (unsigned long long)irq_num);
            break;
    }
    
    // Send EOI
    // TODO: Implement proper EOI handling
}
