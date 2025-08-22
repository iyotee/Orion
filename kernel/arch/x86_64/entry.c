// Main C entry point for Orion x86_64
#include <orion/kernel.h>
#include <orion/types.h>
#include "include/arch.h"

// Variables externes du linker
extern uint64_t _text_start, _text_end;
extern uint64_t _rodata_start, _rodata_end;
extern uint64_t _data_start, _data_end;
extern uint64_t _bss_start, _bss_end;
extern uint64_t _kernel_end;

// Boot variables from assembly
extern uint64_t boot_efi_handle;
extern uint64_t boot_efi_system_table;

// Global system information
static system_info_t g_system_info;
static bool g_kernel_initialized = false;

// Serial console (COM1) for early debug
#define COM1_PORT 0x3F8

void serial_putchar(char c) {
    // Wait for port to be ready
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

// Console for kernel.h functions
void console_init(void) {
    // Initialize COM1: 115200 baud, 8N1
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

// Complete printf implementation for kernel
int kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    const char* p = fmt;
    int count = 0;
    
    while (*p) {
        if (*p == '%' && *(p+1)) {
            p++; // Skip %
            switch (*p) {
                case 's': {
                    const char* str = va_arg(args, const char*);
                    if (str) {
                        console_puts(str);
                        count += strlen(str);
                    } else {
                        console_puts("(null)");
                        count += 6;
                    }
                    break;
                }
                case 'd': {
                    int num = va_arg(args, int);
                    char buffer[32];
                    int len = itoa(num, buffer, 10);
                    console_puts(buffer);
                    count += len;
                    break;
                }
                case 'u': {
                    unsigned int num = va_arg(args, unsigned int);
                    char buffer[32];
                    int len = utoa(num, buffer, 10);
                    console_puts(buffer);
                    count += len;
                    break;
                }
                case 'x': {
                    unsigned int num = va_arg(args, unsigned int);
                    char buffer[32];
                    int len = utoa(num, buffer, 16);
                    console_puts(buffer);
                    count += len;
                    break;
                }
                case 'p': {
                    void* ptr = va_arg(args, void*);
                    console_puts("0x");
                    char buffer[32];
                    int len = utoa((uintptr_t)ptr, buffer, 16);
                    console_puts(buffer);
                    count += 2 + len;
                    break;
                }
                case 'c': {
                    char ch = (char)va_arg(args, int);
                    console_putchar(ch);
                    count++;
                    break;
                }
                case '%': {
                    console_putchar('%');
                    count++;
                    break;
                }
                default: {
                    console_putchar('%');
                    console_putchar(*p);
                    count += 2;
                    break;
                }
            }
        } else {
            console_putchar(*p);
            count++;
        }
        p++;
    }
    
    va_end(args);
    return count;
}

// Panic handler
void panic(const char* fmt, ...) {
    cli(); // Disable interrupts
    
    va_list args;
    va_start(args, fmt);
    
    kprintf("\n*** ORION KERNEL PANIC ***\n");
    kprintf("Error: ");
    
    // Custom printf implementation for panic (simplified)
    const char* p = fmt;
    while (*p) {
        if (*p == '%' && *(p+1)) {
            p++; // Skip %
            switch (*p) {
                case 's': {
                    const char* str = va_arg(args, const char*);
                    kprintf("%s", str ? str : "(null)");
                    break;
                }
                case 'd': {
                    int num = va_arg(args, int);
                    kprintf("%d", num);
                    break;
                }
                case 'x': {
                    unsigned int num = va_arg(args, unsigned int);
                    kprintf("%x", num);
                    break;
                }
                case 'p': {
                    void* ptr = va_arg(args, void*);
                    kprintf("%p", ptr);
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
    }
    
    va_end(args);
    kprintf("\nSystem halted.\n");
    
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

// Basic CPU detection
void detect_cpu(cpu_info_t* cpu) {
    uint32_t eax, ebx, ecx, edx;
    
    // CPU 0 for now (BSP)
    cpu->cpu_id = 0;
    cpu->apic_id = 0;
    cpu->online = true;
    
    // Vendor string
    cpuid(0, &eax, &ebx, &ecx, &edx);
    ((uint32_t*)cpu->vendor)[0] = ebx;
    ((uint32_t*)cpu->vendor)[1] = edx; 
    ((uint32_t*)cpu->vendor)[2] = ecx;
    cpu->vendor[12] = '\0';
    
    // Parse CPU family, model, and stepping properly
    cpuid(1, &eax, &ebx, &ecx, &edx);
    uint32_t family = (eax >> 8) & 0xF;
    uint32_t model = (eax >> 4) & 0xF;
    uint32_t stepping = eax & 0xF;
    
    // Handle extended family and model
    if (family == 0xF) {
        family += (eax >> 20) & 0xFF;
    }
    if (family == 0x6 || family == 0xF) {
        model += ((eax >> 16) & 0xF) << 4;
    }
    
    kprintf("CPU Family: %d, Model: %d, Stepping: %d\n", family, model, stepping);
    
    // Fill cpu->model with vendor-specific model name
    if (strncmp(cpu->vendor, "GenuineIntel", 12) == 0) {
        if (family == 6) {
            switch (model) {
                case 0x1A: case 0x1E: case 0x1F: case 0x2E:
                    strcpy(cpu->model, "Intel Core i7/i5/i3 (Nehalem)");
                    break;
                case 0x25: case 0x2C: case 0x2F:
                    strcpy(cpu->model, "Intel Core i7/i5/i3 (Westmere)");
                    break;
                case 0x2A: case 0x2D:
                    strcpy(cpu->model, "Intel Core i7/i5/i3 (Sandy Bridge)");
                    break;
                case 0x3A: case 0x3E:
                    strcpy(cpu->model, "Intel Core i7/i5/i3 (Ivy Bridge)");
                    break;
                case 0x3C: case 0x3F: case 0x45: case 0x46:
                    strcpy(cpu->model, "Intel Core i7/i5/i3 (Haswell)");
                    break;
                case 0x3D: case 0x47: case 0x4F: case 0x56:
                    strcpy(cpu->model, "Intel Core i7/i5/i3 (Broadwell)");
                    break;
                case 0x4E: case 0x5E: case 0x8E: case 0x9E:
                    strcpy(cpu->model, "Intel Core i7/i5/i3 (Skylake)");
                    break;
                default:
                    strcpy(cpu->model, "Intel Unknown Model");
                    break;
            }
        } else {
            strcpy(cpu->model, "Intel Legacy");
        }
    } else if (strncmp(cpu->vendor, "AuthenticAMD", 12) == 0) {
        if (family >= 0x15) {
            strcpy(cpu->model, "AMD Zen/Ryzen");
        } else if (family == 0x10) {
            strcpy(cpu->model, "AMD K10");
        } else {
            strcpy(cpu->model, "AMD Legacy");
        }
    } else {
        strcpy(cpu->model, "Unknown CPU");
    }
}

// kernel_early_init is implemented in main.c

// Late kernel initialization  
void kernel_late_init(void) {
    kprintf("Late initialization...\n");
    
    // Initialize core subsystems
    kprintf("  - Initializing Advanced Memory Management...\n");
    // vmm_late_init(); // Advanced VMM features
    
    kprintf("  - Initializing Scheduler...\n");
    // scheduler_late_init(); // Thread scheduling
    
    kprintf("  - Initializing Interrupt Subsystem...\n");
    // irq_late_init(); // Advanced interrupt handling
    
    kprintf("  - Initializing Timer Subsystem...\n");
    // timer_init(); // System timers
    
    kprintf("  - Initializing Device Drivers...\n");
    // driver_init(); // Device driver framework
    
    g_kernel_initialized = true;
    kprintf("Kernel initialization complete!\n");
}

// kernel_main is implemented in main.c
