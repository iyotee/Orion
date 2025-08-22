/*
 * Orion Operating System - x86_64 Interrupt Management
 *
 * Complete interrupt descriptor table (IDT) setup, PIC/APIC configuration,
 * and exception/interrupt handlers for robust kernel operation.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/kernel.h>
#include <orion/types.h>
#include <orion/scheduler.h>

// ========================================
// CONSTANTS AND DEFINITIONS
// ========================================

#define IDT_ENTRIES 256
#define IDT_GATE_INTERRUPT 0x8E
#define IDT_GATE_TRAP 0x8F
#define IDT_GATE_TASK 0x85

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20

#define APIC_BASE 0xFEE00000
#define APIC_ID 0x20
#define APIC_VERSION 0x30
#define APIC_TPR 0x80
#define APIC_EOI 0xB0
#define APIC_SIVR 0xF0
#define APIC_LVT_TIMER 0x320
#define APIC_LVT_PERF 0x340
#define APIC_LVT_LINT0 0x350
#define APIC_LVT_LINT1 0x360
#define APIC_LVT_ERROR 0x370
#define APIC_TIMER_DIV 0x3E0
#define APIC_TIMER_INIT 0x380
#define APIC_TIMER_CURR 0x390

// ========================================
// DATA STRUCTURES
// ========================================

// Interrupt descriptor table entry
#pragma pack(push, 1)
typedef struct
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t flags;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} idt_entry_t;

// IDT pointer for lidt instruction
typedef struct
{
    uint16_t limit;
    uint64_t base;
} idt_ptr_t;
#pragma pack(pop)

// ========================================
// GLOBAL VARIABLES
// ========================================

static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t idt_ptr;
static bool apic_available = false;
static uint32_t apic_id = 0;

// ========================================
// ASSEMBLY INTERFACE
// ========================================

// Assembly interrupt handlers (defined in interrupts.S)
extern void isr_common_stub(void);
extern void irq_common_stub(void);

// C interrupt handlers are defined in interrupt_handlers.c

// Individual exception handlers
extern void isr0(void);  // Divide by zero
extern void isr1(void);  // Debug
extern void isr2(void);  // Non-maskable interrupt
extern void isr3(void);  // Breakpoint
extern void isr4(void);  // Overflow
extern void isr5(void);  // Bound range exceeded
extern void isr6(void);  // Invalid opcode
extern void isr7(void);  // Device not available
extern void isr8(void);  // Double fault
extern void isr9(void);  // Coprocessor segment overrun
extern void isr10(void); // Invalid TSS
extern void isr11(void); // Segment not present
extern void isr12(void); // Stack-segment fault
extern void isr13(void); // General protection fault
extern void isr14(void); // Page fault
extern void isr15(void); // Reserved
extern void isr16(void); // x87 FPU error
extern void isr17(void); // Alignment check
extern void isr18(void); // Machine check
extern void isr19(void); // SIMD FPU exception
extern void isr20(void); // Virtualization exception
extern void isr21(void); // Reserved
extern void isr22(void); // Reserved
extern void isr23(void); // Reserved
extern void isr24(void); // Reserved
extern void isr25(void); // Reserved
extern void isr26(void); // Reserved
extern void isr27(void); // Reserved
extern void isr28(void); // Reserved
extern void isr29(void); // Reserved
extern void isr30(void); // Reserved
extern void isr31(void); // Reserved

// IRQ handlers
extern void irq0(void);  // PIT timer
extern void irq1(void);  // Keyboard
extern void irq2(void);  // Cascade
extern void irq3(void);  // COM2
extern void irq4(void);  // COM1
extern void irq5(void);  // LPT2
extern void irq6(void);  // Floppy
extern void irq7(void);  // LPT1
extern void irq8(void);  // CMOS RTC
extern void irq9(void);  // Free
extern void irq10(void); // Free
extern void irq11(void); // Free
extern void irq12(void); // PS/2 mouse
extern void irq13(void); // FPU
extern void irq14(void); // Primary ATA
extern void irq15(void); // Secondary ATA

// ========================================
// INTERRUPT DESCRIPTOR TABLE SETUP
// ========================================

// Set up an IDT entry
static void idt_set_gate(uint8_t num, uint64_t handler, uint8_t flags)
{
    idt[num].offset_low = (uint16_t)(handler & 0xFFFF);
    idt[num].selector = 0x08; // Kernel code segment
    idt[num].ist = 0;
    idt[num].flags = flags;
    idt[num].offset_mid = (uint16_t)((handler >> 16) & 0xFFFF);
    idt[num].offset_high = (uint32_t)((handler >> 32) & 0xFFFFFFFF);
    idt[num].reserved = 0;
}

// Initialize the IDT
static void idt_init(void)
{
    // Set up exception handlers (ISRs 0-31)
    idt_set_gate(0, (uint64_t)isr0, IDT_GATE_TRAP);
    idt_set_gate(1, (uint64_t)isr1, IDT_GATE_TRAP);
    idt_set_gate(2, (uint64_t)isr2, IDT_GATE_INTERRUPT);
    idt_set_gate(3, (uint64_t)isr3, IDT_GATE_TRAP);
    idt_set_gate(4, (uint64_t)isr4, IDT_GATE_TRAP);
    idt_set_gate(5, (uint64_t)isr5, IDT_GATE_TRAP);
    idt_set_gate(6, (uint64_t)isr6, IDT_GATE_TRAP);
    idt_set_gate(7, (uint64_t)isr7, IDT_GATE_TRAP);
    idt_set_gate(8, (uint64_t)isr8, IDT_GATE_TRAP);
    idt_set_gate(9, (uint64_t)isr9, IDT_GATE_TRAP);
    idt_set_gate(10, (uint64_t)isr10, IDT_GATE_TRAP);
    idt_set_gate(11, (uint64_t)isr11, IDT_GATE_TRAP);
    idt_set_gate(12, (uint64_t)isr12, IDT_GATE_TRAP);
    idt_set_gate(13, (uint64_t)isr13, IDT_GATE_TRAP);
    idt_set_gate(14, (uint64_t)isr14, IDT_GATE_TRAP);
    idt_set_gate(15, (uint64_t)isr15, IDT_GATE_TRAP);
    idt_set_gate(16, (uint64_t)isr16, IDT_GATE_TRAP);
    idt_set_gate(17, (uint64_t)isr17, IDT_GATE_TRAP);
    idt_set_gate(18, (uint64_t)isr18, IDT_GATE_TRAP);
    idt_set_gate(19, (uint64_t)isr19, IDT_GATE_TRAP);
    idt_set_gate(20, (uint64_t)isr20, IDT_GATE_TRAP);
    idt_set_gate(21, (uint64_t)isr21, IDT_GATE_TRAP);
    idt_set_gate(22, (uint64_t)isr22, IDT_GATE_TRAP);
    idt_set_gate(23, (uint64_t)isr23, IDT_GATE_TRAP);
    idt_set_gate(24, (uint64_t)isr24, IDT_GATE_TRAP);
    idt_set_gate(25, (uint64_t)isr25, IDT_GATE_TRAP);
    idt_set_gate(26, (uint64_t)isr26, IDT_GATE_TRAP);
    idt_set_gate(27, (uint64_t)isr27, IDT_GATE_TRAP);
    idt_set_gate(28, (uint64_t)isr28, IDT_GATE_TRAP);
    idt_set_gate(29, (uint64_t)isr29, IDT_GATE_TRAP);
    idt_set_gate(30, (uint64_t)isr30, IDT_GATE_TRAP);
    idt_set_gate(31, (uint64_t)isr31, IDT_GATE_TRAP);

    // Set up IRQ handlers (ISRs 32-47)
    idt_set_gate(32, (uint64_t)irq0, IDT_GATE_INTERRUPT);
    idt_set_gate(33, (uint64_t)irq1, IDT_GATE_INTERRUPT);
    idt_set_gate(34, (uint64_t)irq2, IDT_GATE_INTERRUPT);
    idt_set_gate(35, (uint64_t)irq3, IDT_GATE_INTERRUPT);
    idt_set_gate(36, (uint64_t)irq4, IDT_GATE_INTERRUPT);
    idt_set_gate(37, (uint64_t)irq5, IDT_GATE_INTERRUPT);
    idt_set_gate(38, (uint64_t)irq6, IDT_GATE_INTERRUPT);
    idt_set_gate(39, (uint64_t)irq7, IDT_GATE_INTERRUPT);
    idt_set_gate(40, (uint64_t)irq8, IDT_GATE_INTERRUPT);
    idt_set_gate(41, (uint64_t)irq9, IDT_GATE_INTERRUPT);
    idt_set_gate(42, (uint64_t)irq10, IDT_GATE_INTERRUPT);
    idt_set_gate(43, (uint64_t)irq11, IDT_GATE_INTERRUPT);
    idt_set_gate(44, (uint64_t)irq12, IDT_GATE_INTERRUPT);
    idt_set_gate(45, (uint64_t)irq13, IDT_GATE_INTERRUPT);
    idt_set_gate(46, (uint64_t)irq14, IDT_GATE_INTERRUPT);
    idt_set_gate(47, (uint64_t)irq15, IDT_GATE_INTERRUPT);

    // Set up the IDT pointer
    idt_ptr.limit = (sizeof(idt_entry_t) * IDT_ENTRIES) - 1;
    idt_ptr.base = (uint64_t)&idt;

    // Load the IDT using assembly function
    extern void load_idt(uint64_t idt_ptr);
    load_idt((uint64_t)&idt_ptr);

    kinfo("IDT initialized with %d entries", IDT_ENTRIES);
}

// ========================================
// PIC CONFIGURATION
// ========================================

// Initialize the PIC using assembly functions
static void pic_init(void)
{
    extern void pic_send_command(uint8_t command, uint8_t port);
    extern void pic_send_data(uint8_t data, uint8_t port);

    // ICW1: start initialization sequence
    pic_send_command(0x11, PIC1_COMMAND);
    pic_send_command(0x11, PIC2_COMMAND);

    // ICW2: set vector offsets
    pic_send_data(0x20, PIC1_DATA);
    pic_send_data(0x28, PIC2_DATA);

    // ICW3: tell PICs how they're cascaded
    pic_send_data(0x04, PIC1_DATA);
    pic_send_data(0x02, PIC2_DATA);

    // ICW4: set 8086 mode
    pic_send_data(0x01, PIC1_DATA);
    pic_send_data(0x01, PIC2_DATA);

    // Mask all IRQs except timer and keyboard
    pic_send_data(0xFC, PIC1_DATA);
    pic_send_data(0xFF, PIC2_DATA);

    kinfo("PIC initialized and configured");
}

// Send EOI to PIC
void pic_send_eoi(uint8_t irq)
{
    extern void pic_send_data(uint8_t data, uint8_t port);

    if (irq >= 8)
    {
        pic_send_data(PIC_EOI, PIC2_COMMAND);
    }
    pic_send_data(PIC_EOI, PIC1_COMMAND);
}

// ========================================
// APIC DETECTION AND SETUP
// ========================================

// Check if APIC is available
static bool apic_check_availability(void)
{
    extern uint32_t cpuid_get_feature_info(void);
    extern uint64_t msr_read(uint32_t msr);

    // Check CPUID for APIC support
    uint32_t features = cpuid_get_feature_info();
    if (!(features & (1 << 9)))
    {
        return false; // No APIC
    }

    // Check if APIC is enabled
    uint64_t msr_val = msr_read(0x1B);
    if (!(msr_val & (1ULL << 11)))
    {
        return false; // APIC disabled
    }

    return true;
}

// Initialize APIC
static void apic_init(void)
{
    if (!apic_check_availability())
    {
        kinfo("APIC not available, using PIC only");
        return;
    }

    apic_available = true;

    // Get APIC ID
    uint32_t *apic_base = (uint32_t *)APIC_BASE;
    apic_id = (apic_base[APIC_ID / 4] >> 24) & 0xFF;

    // Enable APIC
    apic_base[APIC_SIVR / 4] |= 0x100;

    // Configure LVT entries
    apic_base[APIC_LVT_TIMER / 4] = 0x20000; // Timer disabled
    apic_base[APIC_LVT_LINT0 / 4] = 0x10000; // LINT0 disabled
    apic_base[APIC_LVT_LINT1 / 4] = 0x10000; // LINT1 disabled
    apic_base[APIC_LVT_ERROR / 4] = 0x10000; // Error disabled

    kinfo("APIC initialized (ID: %d)", apic_id);
}

// ========================================
// EXCEPTION HANDLERS
// ========================================

// These functions are now defined in interrupt_handlers.c
extern void isr_handler(uint64_t isr_num, uint64_t error_code, uint64_t rip, uint64_t cs, uint64_t rflags, uint64_t rsp, uint64_t ss);
extern void irq_handler(uint64_t irq_num);

// ========================================
// PUBLIC INTERFACE
// ========================================

// Initialize interrupt subsystem
void arch_interrupt_init_real(void)
{
    kinfo("x86_64 interrupt subsystem initialization started");

    // Disable interrupts during setup
    extern void disable_interrupts(void);
    disable_interrupts();

    // Initialize IDT
    idt_init();

    // Initialize PIC
    pic_init();

    // Initialize APIC if available
    apic_init();

    // Re-enable interrupts
    extern void enable_interrupts(void);
    enable_interrupts();

    kinfo("Interrupt subsystem fully initialized");
}

// Enable interrupts
void arch_enable_interrupts(void)
{
    extern void enable_interrupts(void);
    enable_interrupts();
}

// Disable interrupts
void arch_disable_interrupts(void)
{
    extern void disable_interrupts(void);
    disable_interrupts();
}

// Check if interrupts are enabled
bool arch_interrupts_enabled(void)
{
    extern uint64_t get_rflags(void);
    uint64_t rflags = get_rflags();
    return (rflags & (1ULL << 9)) != 0;
}

// Get APIC ID
uint32_t arch_get_apic_id(void)
{
    return apic_id;
}

// Check if APIC is available
bool arch_apic_available(void)
{
    return apic_available;
}
