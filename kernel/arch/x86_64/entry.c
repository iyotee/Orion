// Point d'entrée C principal pour Orion x86_64
#include <orion/kernel.h>
#include <orion/types.h>
#include "include/arch.h"

// Variables externes du linker
extern uint64_t _text_start, _text_end;
extern uint64_t _rodata_start, _rodata_end;
extern uint64_t _data_start, _data_end;
extern uint64_t _bss_start, _bss_end;
extern uint64_t _kernel_end;

// Variables de boot depuis l'assembleur
extern uint64_t boot_efi_handle;
extern uint64_t boot_efi_system_table;

// Informations système globales
static system_info_t g_system_info;
static bool g_kernel_initialized = false;

// Console série (COM1) pour debug précoce
#define COM1_PORT 0x3F8

void serial_putchar(char c) {
    // Attendre que le port soit prêt
    while ((__builtin_inb(COM1_PORT + 5) & 0x20) == 0);
    __builtin_outb(COM1_PORT, (uint8_t)c);
}

static void serial_puts(const char* str) {
    while (*str) {
        if (*str == '\n') {
            serial_putchar('\r');
        }
        serial_putchar(*str++);
    }
}

// Console pour les fonctions kernel.h
void console_init(void) {
    // Initialiser COM1 : 115200 baud, 8N1
    __builtin_outb((uint16_t)(COM1_PORT + 1), 0x00);  // Disable interrupts
    __builtin_outb((uint16_t)(COM1_PORT + 3), 0x80);  // Enable DLAB
    __builtin_outb((uint16_t)(COM1_PORT + 0), 0x01);  // Divisor low (115200 baud)
    __builtin_outb((uint16_t)(COM1_PORT + 1), 0x00);  // Divisor high
    __builtin_outb((uint16_t)(COM1_PORT + 3), 0x03);  // 8N1, disable DLAB
    __builtin_outb((uint16_t)(COM1_PORT + 2), 0xC7);  // Enable FIFO, clear buffers
    __builtin_outb((uint16_t)(COM1_PORT + 4), 0x0B);  // IRQs enabled, RTS/DSR set
}

void console_putchar(char c) {
    serial_putchar(c);
}

void console_puts(const char* str) {
    serial_puts(str);
}

// Printf simple pour le noyau (version basique)
int kprintf(const char* fmt, ...) {
    // Version très basique - TODO: implémenter printf complet
    const char* p = fmt;
    int count = 0;
    
    while (*p) {
        if (*p == '%' && *(p+1)) {
            p++; // Skip %
            switch (*p) {
                case 's': {
                    // String - TODO: récupérer depuis va_list
                    console_puts("(string)");
                    break;
                }
                case 'd': 
                case 'x':
                case 'p': {
                    // Nombres - TODO: implémenter
                    console_puts("(number)");
                    break;
                }
                case '%': {
                    console_putchar('%');
                    break;
                }
                default: {
                    console_putchar('%');
                    console_putchar(*p);
                    break;
                }
            }
        } else {
            console_putchar(*p);
        }
        p++;
        count++;
    }
    
    return count;
}

// Panic handler
void panic(const char* fmt, ...) {
    cli(); // Disable interrupts
    
    kprintf("\n*** ORION KERNEL PANIC ***\n");
    kprintf("Error: %s\n", fmt); // Simplified - TODO: va_args
    kprintf("System halted.\n");
    
    // Halt forever
    while (1) {
        hlt();
    }
}

void kernel_halt(void) {
    kprintf("Kernel halting gracefully...\n");
    while (1) {
        hlt();
    }
}

// Détection CPU basique
void detect_cpu(cpu_info_t* cpu) {
    uint32_t eax, ebx, ecx, edx;
    
    // CPU 0 pour l'instant (BSP)
    cpu->cpu_id = 0;
    cpu->apic_id = 0;
    cpu->online = true;
    
    // Vendor string
    cpuid(0, &eax, &ebx, &ecx, &edx);
    ((uint32_t*)cpu->vendor)[0] = ebx;
    ((uint32_t*)cpu->vendor)[1] = edx; 
    ((uint32_t*)cpu->vendor)[2] = ecx;
    cpu->vendor[12] = '\0';
    
    // Model basique (TODO: parser proprement)
    cpuid(1, &eax, &ebx, &ecx, &edx);
    kprintf("CPU Family: %d, Model: %d, Stepping: %d\n",
            (eax >> 8) & 0xF, (eax >> 4) & 0xF, eax & 0xF);
    
    // Pour l'instant, modèle générique
    (void)kprintf; // Éviter warning unused
    // TODO: remplir cpu->model proprement
    cpu->model[0] = '\0';
}

// kernel_early_init is implemented in main.c

// Initialisation tardive du noyau  
void kernel_late_init(void) {
    kprintf("Late initialization...\n");
    
    // TODO: initialiser les sous-systèmes
    // mm_init();
    // sched_init(); 
    // irq_init();
    
    g_kernel_initialized = true;
    kprintf("Kernel initialization complete!\n");
}

// kernel_main is implemented in main.c
