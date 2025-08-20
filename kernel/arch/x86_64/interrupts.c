// Gestion des interruptions x86_64 pour Orion
#include <orion/kernel.h>
#include <orion/types.h>
#include <arch.h>

// IDT avec 256 entrées
#ifdef _MSC_VER
__declspec(align(16)) static idt_entry_t idt[256];
#else
static idt_entry_t idt[256] __attribute__((aligned(16)));
#endif
static idtr_t idtr;

// Handlers d'interruption par défaut
extern void isr_stub_0(void);
extern void isr_stub_1(void);
extern void isr_stub_2(void);
extern void isr_stub_3(void);
extern void isr_stub_4(void);
extern void isr_stub_5(void);
extern void isr_stub_6(void);
extern void isr_stub_7(void);
extern void isr_stub_8(void);
extern void isr_stub_9(void);
extern void isr_stub_10(void);
extern void isr_stub_11(void);
extern void isr_stub_12(void);
extern void isr_stub_13(void);
extern void isr_stub_14(void);
extern void isr_stub_15(void);
extern void isr_stub_16(void);
extern void isr_stub_17(void);
extern void isr_stub_18(void);
extern void isr_stub_19(void);
extern void isr_stub_20(void);

// Noms des exceptions
static const char* exception_names[] = {
    "Division by Zero",
    "Debug",
    "Non-Maskable Interrupt", 
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating Point Exception",
    "Virtualization Exception"
};

// Configurer une entrée IDT
static void set_idt_entry(uint8_t vector, uint64_t handler, uint8_t flags) {
    idt[vector].offset_low = handler & 0xFFFF;
    idt[vector].selector = 0x08; // Segment code noyau
    idt[vector].ist = 0;         // Pas d'IST pour l'instant
    idt[vector].type_attr = flags;
    idt[vector].offset_mid = (handler >> 16) & 0xFFFF;
    idt[vector].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[vector].zero = 0;
}

// Handler C pour les exceptions
void exception_handler(uint64_t vector, uint64_t error_code, registers_t* regs) {
    (void)error_code; // Éviter warning pour l'instant
    (void)regs;
    
    kprintf("\n*** EXCEPTION ***\n");
    
    if (vector < ARRAY_SIZE(exception_names)) {
        kprintf("Exception: %s (vector %llu)\n", 
                exception_names[vector], (unsigned long long)vector);
    } else {
        kprintf("Unknown exception (vector %llu)\n", 
                (unsigned long long)vector);
    }
    
    if (vector == 14) { // Page fault
        uint64_t cr2;
#ifdef _MSC_VER
        cr2 = 0; // MSVC stub
#else
        __asm__ volatile ("mov %%cr2, %0" : "=r"(cr2));
#endif
        kprintf("Page fault address: 0x%p\n", (void*)cr2);
        kprintf("Error code: 0x%p\n", (void*)error_code);
    }
    
    kprintf("System will halt.\n");
    
    // Halt le système
    cli();
    while (1) {
        hlt();
    }
}

// Stub assembleur pour les ISR (à définir dans un fichier .S séparé)
// Pour l'instant, version C inline simple
#ifndef _MSC_VER
__asm__(
    ".section .text\n"
    
    // Macro pour ISR sans code d'erreur
    ".macro ISR_NOERRCODE num\n"
    "isr_stub_\\num:\n"
    "    cli\n"
    "    pushq $0\n"          // Code d'erreur dummy
    "    pushq $\\num\n"      // Numéro vecteur
    "    jmp isr_common\n"
    ".endm\n"
    
    // Macro pour ISR avec code d'erreur  
    ".macro ISR_ERRCODE num\n"
    "isr_stub_\\num:\n"
    "    cli\n"
    "    pushq $\\num\n"      // Numéro vecteur
    "    jmp isr_common\n"
    ".endm\n"
    
    // Générer les stubs
    "ISR_NOERRCODE 0\n"   // Division by zero
    "ISR_NOERRCODE 1\n"   // Debug
    "ISR_NOERRCODE 2\n"   // NMI
    "ISR_NOERRCODE 3\n"   // Breakpoint
    "ISR_NOERRCODE 4\n"   // Overflow
    "ISR_NOERRCODE 5\n"   // Bound range
    "ISR_NOERRCODE 6\n"   // Invalid opcode
    "ISR_NOERRCODE 7\n"   // Device not available
    "ISR_ERRCODE   8\n"   // Double fault
    "ISR_NOERRCODE 9\n"   // Coprocessor overrun
    "ISR_ERRCODE   10\n"  // Invalid TSS
    "ISR_ERRCODE   11\n"  // Segment not present
    "ISR_ERRCODE   12\n"  // Stack fault
    "ISR_ERRCODE   13\n"  // GPF
    "ISR_ERRCODE   14\n"  // Page fault
    "ISR_NOERRCODE 15\n"  // Reserved
    "ISR_NOERRCODE 16\n"  // x87 FPU
    "ISR_ERRCODE   17\n"  // Alignment check
    "ISR_NOERRCODE 18\n"  // Machine check
    "ISR_NOERRCODE 19\n"  // SIMD FP
    "ISR_NOERRCODE 20\n"  // Virtualization
    
    // Handler commun
    "isr_common:\n"
    "    pushq %rax\n"
    "    pushq %rbx\n"
    "    pushq %rcx\n"
    "    pushq %rdx\n"
    "    pushq %rsi\n"
    "    pushq %rdi\n"
    "    pushq %rbp\n"
    "    pushq %r8\n"
    "    pushq %r9\n"
    "    pushq %r10\n"
    "    pushq %r11\n"
    "    pushq %r12\n"
    "    pushq %r13\n"
    "    pushq %r14\n"
    "    pushq %r15\n"
    
    // Appeler le handler C
    "    mov %rsp, %rdx\n"    // 3e param: registres
    "    mov 120(%rsp), %rsi\n" // 2e param: code erreur
    "    mov 128(%rsp), %rdi\n" // 1er param: vecteur
    "    call exception_handler\n"
    
    // Restaurer registres (ne devrait jamais arriver)
    "    popq %r15\n"
    "    popq %r14\n"
    "    popq %r13\n"
    "    popq %r12\n"
    "    popq %r11\n"
    "    popq %r10\n"
    "    popq %r9\n"
    "    popq %r8\n"
    "    popq %rbp\n"
    "    popq %rdi\n"
    "    popq %rsi\n"
    "    popq %rdx\n"
    "    popq %rcx\n"
    "    popq %rbx\n"
    "    popq %rax\n"
    "    add $16, %rsp\n"     // Nettoyer vecteur + code erreur
    "    iretq\n"
);
#endif // _MSC_VER

// Initialiser les interruptions
void interrupts_init(void) {
    kinfo("Initializing x86_64 interrupt handling");
    
    // Clear IDT
    for (int i = 0; i < 256; i++) {
        idt[i] = (idt_entry_t){0};
    }
    
    // Configurer les handlers d'exception
    set_idt_entry(0,  (uint64_t)isr_stub_0,  0x8E);  // Trap gate
    set_idt_entry(1,  (uint64_t)isr_stub_1,  0x8E);
    set_idt_entry(2,  (uint64_t)isr_stub_2,  0x8E);
    set_idt_entry(3,  (uint64_t)isr_stub_3,  0x8E);
    set_idt_entry(4,  (uint64_t)isr_stub_4,  0x8E);
    set_idt_entry(5,  (uint64_t)isr_stub_5,  0x8E);
    set_idt_entry(6,  (uint64_t)isr_stub_6,  0x8E);
    set_idt_entry(7,  (uint64_t)isr_stub_7,  0x8E);
    set_idt_entry(8,  (uint64_t)isr_stub_8,  0x8E);
    set_idt_entry(9,  (uint64_t)isr_stub_9,  0x8E);
    set_idt_entry(10, (uint64_t)isr_stub_10, 0x8E);
    set_idt_entry(11, (uint64_t)isr_stub_11, 0x8E);
    set_idt_entry(12, (uint64_t)isr_stub_12, 0x8E);
    set_idt_entry(13, (uint64_t)isr_stub_13, 0x8E);
    set_idt_entry(14, (uint64_t)isr_stub_14, 0x8E);
    set_idt_entry(15, (uint64_t)isr_stub_15, 0x8E);
    set_idt_entry(16, (uint64_t)isr_stub_16, 0x8E);
    set_idt_entry(17, (uint64_t)isr_stub_17, 0x8E);
    set_idt_entry(18, (uint64_t)isr_stub_18, 0x8E);
    set_idt_entry(19, (uint64_t)isr_stub_19, 0x8E);
    set_idt_entry(20, (uint64_t)isr_stub_20, 0x8E);
    
    // Configurer IDTR
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)idt;
    
    // Charger IDT
    idt_load(&idtr);
    
    kinfo("IDT loaded with %d entries", 256);
    kinfo("Interrupt handling initialized");
}

// Enregistrer un handler personnalisé
void register_interrupt_handler(uint8_t vector, void* handler) {
    if (vector >= 32) { // Seuls les IRQ (32+) peuvent être redéfinis
        set_idt_entry(vector, (uint64_t)handler, 0x8E);
        kdebug("Registered handler for IRQ %d", vector - 32);
    }
}

// Enable/disable interrupts (wrappers pour les fonctions inline d'arch.h)
void irq_enable(void) {
    sti();
}

void irq_disable(void) {
    cli();
}

void irq_init(void) {
    interrupts_init();
}
