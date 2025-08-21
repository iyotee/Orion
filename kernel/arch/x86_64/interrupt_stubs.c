/*
 * Orion Operating System - x86_64 Interrupt Stubs
 *
 * Temporary stub implementations for interrupt handlers
 * until the assembly file is properly compiled.
 *
 * Developed by Jérémy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/kernel.h>
#include <orion/types.h>

// ========================================
// TEMPORARY INTERRUPT STUBS
// ========================================

// Exception handlers (ISRs 0-31)
void isr0(void) { kerror("ISR0: Divide by zero - stub"); }
void isr1(void) { kerror("ISR1: Debug - stub"); }
void isr2(void) { kerror("ISR2: NMI - stub"); }
void isr3(void) { kerror("ISR3: Breakpoint - stub"); }
void isr4(void) { kerror("ISR4: Overflow - stub"); }
void isr5(void) { kerror("ISR5: Bound range exceeded - stub"); }
void isr6(void) { kerror("ISR6: Invalid opcode - stub"); }
void isr7(void) { kerror("ISR7: Device not available - stub"); }
void isr8(void) { kerror("ISR8: Double fault - stub"); }
void isr9(void) { kerror("ISR9: Coprocessor segment overrun - stub"); }
void isr10(void) { kerror("ISR10: Invalid TSS - stub"); }
void isr11(void) { kerror("ISR11: Segment not present - stub"); }
void isr12(void) { kerror("ISR12: Stack-segment fault - stub"); }
void isr13(void) { kerror("ISR13: General protection fault - stub"); }
void isr14(void) { kerror("ISR14: Page fault - stub"); }
void isr15(void) { kerror("ISR15: Reserved - stub"); }
void isr16(void) { kerror("ISR16: x87 FPU error - stub"); }
void isr17(void) { kerror("ISR17: Alignment check - stub"); }
void isr18(void) { kerror("ISR18: Machine check - stub"); }
void isr19(void) { kerror("ISR19: SIMD FPU exception - stub"); }
void isr20(void) { kerror("ISR20: Virtualization exception - stub"); }
void isr21(void) { kerror("ISR21: Reserved - stub"); }
void isr22(void) { kerror("ISR22: Reserved - stub"); }
void isr23(void) { kerror("ISR23: Reserved - stub"); }
void isr24(void) { kerror("ISR24: Reserved - stub"); }
void isr25(void) { kerror("ISR25: Reserved - stub"); }
void isr26(void) { kerror("ISR26: Reserved - stub"); }
void isr27(void) { kerror("ISR27: Reserved - stub"); }
void isr28(void) { kerror("ISR28: Reserved - stub"); }
void isr29(void) { kerror("ISR29: Reserved - stub"); }
void isr30(void) { kerror("ISR30: Reserved - stub"); }
void isr31(void) { kerror("ISR31: Reserved - stub"); }

// IRQ handlers (ISRs 32-47)
void irq0(void) { kerror("IRQ0: PIT timer - stub"); }
void irq1(void) { kerror("IRQ1: Keyboard - stub"); }
void irq2(void) { kerror("IRQ2: Cascade - stub"); }
void irq3(void) { kerror("IRQ3: COM2 - stub"); }
void irq4(void) { kerror("IRQ4: COM1 - stub"); }
void irq5(void) { kerror("IRQ5: LPT2 - stub"); }
void irq6(void) { kerror("IRQ6: Floppy - stub"); }
void irq7(void) { kerror("IRQ7: LPT1 - stub"); }
void irq8(void) { kerror("IRQ8: CMOS RTC - stub"); }
void irq9(void) { kerror("IRQ9: Free - stub"); }
void irq10(void) { kerror("IRQ10: Free - stub"); }
void irq11(void) { kerror("IRQ11: Free - stub"); }
void irq12(void) { kerror("IRQ12: PS/2 mouse - stub"); }
void irq13(void) { kerror("IRQ13: FPU - stub"); }
void irq14(void) { kerror("IRQ14: Primary ATA - stub"); }
void irq15(void) { kerror("IRQ15: Secondary ATA - stub"); }

// Common stubs
void isr_common_stub(void) { kerror("ISR common stub called - not implemented"); }
void irq_common_stub(void) { kerror("IRQ common stub called - not implemented"); }
