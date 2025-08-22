// x86_64 Assembly functions for Orion OS
// Pure assembly implementation for critical low-level operations
#include <orion/types.h>

#ifdef _MSC_VER
// MSVC inline assembly implementation

// Disable interrupts
void cli(void) {
    __asm {
        cli
    }
}

// Halt CPU
void hlt(void) {
    __asm {
        hlt
    }
}

// Read CR3 register (page table base)
uint64_t read_cr3(void) {
    uint64_t cr3_val;
    __asm {
        mov rax, cr3
        mov cr3_val, rax
    }
    return cr3_val;
}

// Read CR2 register (page fault address)
uint64_t read_cr2(void) {
    uint64_t cr2_val;
    __asm {
        mov rax, cr2
        mov cr2_val, rax
    }
    return cr2_val;
}

// Write CR3 register
void write_cr3(uint64_t cr3) {
    uint64_t cr3_val = cr3;
    __asm {
        mov rax, cr3_val
        mov cr3, rax
    }
}

// Read CR4 register
uint64_t read_cr4(void) {
    uint64_t cr4_val;
    __asm {
        mov rax, cr4
        mov cr4_val, rax
    }
    return cr4_val;
}

// Write CR4 register  
void write_cr4(uint64_t cr4) {
    uint64_t cr4_val = cr4;
    __asm {
        mov rax, cr4_val
        mov cr4, rax
    }
}

// Read from I/O port (8-bit)
unsigned char inb(unsigned short port) {
    unsigned char result;
    unsigned short port_val = port;
    __asm {
        mov dx, port_val
        in al, dx
        mov result, al
    }
    return result;
}

// Write to I/O port (8-bit)
void outb(unsigned short port, unsigned char data) {
    unsigned short port_val = port;
    unsigned char data_val = data;
    __asm {
        mov dx, port_val
        mov al, data_val
        out dx, al
    }
}

// Read from I/O port (16-bit)
unsigned short inw(unsigned short port) {
    unsigned short result;
    unsigned short port_val = port;
    __asm {
        mov dx, port_val
        in ax, dx
        mov result, ax
    }
    return result;
}

// Write to I/O port (16-bit)
void outw(unsigned short port, unsigned short data) {
    unsigned short port_val = port;
    unsigned short data_val = data;
    __asm {
        mov dx, port_val
        mov ax, data_val
        out dx, ax
    }
}

// Read from I/O port (32-bit)
unsigned int inl(unsigned short port) {
    unsigned int result;
    unsigned short port_val = port;
    __asm {
        mov dx, port_val
        in eax, dx
        mov result, eax
    }
    return result;
}

// Write to I/O port (32-bit)
void outl(unsigned short port, unsigned int data) {
    unsigned short port_val = port;
    unsigned int data_val = data;
    __asm {
        mov dx, port_val
        mov eax, data_val
        out dx, eax
    }
}

// Memory barrier
void memory_barrier(void) {
    __asm {
        mfence
    }
}

// CPU pause (for spinlocks)
void cpu_pause(void) {
    __asm {
        pause
    }
}

// Invalidate TLB entry
void invlpg(uint64_t addr) {
    uint64_t addr_val = addr;
    __asm {
        mov rax, addr_val
        invlpg [rax]
    }
}

// Read MSR (Model Specific Register)
uint64_t msr_read(uint32_t msr) {
    uint32_t low, high;
    uint32_t msr_val = msr;
    __asm {
        mov ecx, msr_val
        rdmsr
        mov low, eax
        mov high, edx
    }
    return ((uint64_t)high << 32) | low;
}

// Write MSR (Model Specific Register)
void msr_write(uint32_t msr, uint64_t value) {
    uint32_t low = (uint32_t)value;
    uint32_t high = (uint32_t)(value >> 32);
    uint32_t msr_val = msr;
    __asm {
        mov ecx, msr_val
        mov eax, low
        mov edx, high
        wrmsr
    }
}

// CPUID instruction
void cpuid(uint32_t leaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    uint32_t a, b, c, d;
    uint32_t leaf_val = leaf;
    __asm {
        mov eax, leaf_val
        cpuid
        mov a, eax
        mov b, ebx
        mov c, ecx
        mov d, edx
    }
    if (eax) *eax = a;
    if (ebx) *ebx = b;
    if (ecx) *ecx = c;
    if (edx) *edx = d;
}

#else
// GCC/Clang inline assembly implementation

// Disable interrupts
void cli(void) {
    __asm__ volatile("cli" ::: "memory");
}

// Enable interrupts
void sti(void) {
    __asm__ volatile("sti" ::: "memory");
}

// Halt CPU
void hlt(void) {
    __asm__ volatile("hlt");
}

// Read CR3 register
uint64_t read_cr3(void) {
    uint64_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

// Write CR3 register
void write_cr3(uint64_t cr3) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(cr3) : "memory");
}

// Read CR4 register
uint64_t read_cr4(void) {
    uint64_t cr4;
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    return cr4;
}

// Write CR4 register
void write_cr4(uint64_t cr4) {
    __asm__ volatile("mov %0, %%cr4" : : "r"(cr4) : "memory");
}

// I/O port operations
unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

unsigned short inw(unsigned short port) {
    unsigned short result;
    __asm__ volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outw(unsigned short port, unsigned short data) {
    __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}

unsigned int inl(unsigned short port) {
    unsigned int result;
    __asm__ volatile("inl %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outl(unsigned short port, unsigned int data) {
    __asm__ volatile("outl %0, %1" : : "a"(data), "Nd"(port));
}

// Memory barrier
void memory_barrier(void) {
    __asm__ volatile("mfence" ::: "memory");
}

// CPU pause
void cpu_pause(void) {
    __asm__ volatile("pause");
}

// Invalidate TLB entry
void invlpg(uint64_t addr) {
    __asm__ volatile("invlpg (%0)" : : "r"(addr) : "memory");
}

// MSR operations
uint64_t msr_read(uint32_t msr) {
    uint32_t low, high;
    __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

void msr_write(uint32_t msr, uint64_t value) {
    uint32_t low = (uint32_t)value;
    uint32_t high = (uint32_t)(value >> 32);
    __asm__ volatile("wrmsr" : : "a"(low), "d"(high), "c"(msr));
}

// CPUID instruction
void cpuid(uint32_t leaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    uint32_t a, b, c, d;
    uint32_t leaf_val = leaf;
    __asm {
        mov eax, leaf_val
        cpuid
        mov a, eax
        mov b, ebx
        mov c, ecx
        mov d, edx
    }
    if (eax) *eax = a;
    if (ebx) *ebx = b;
    if (ecx) *ecx = c;
    if (edx) *edx = d;
}

// Read TSC (Time Stamp Counter)
uint64_t read_tsc(void) {
    uint32_t low, high;
    __asm {
        rdtsc
        mov low, eax
        mov high, edx
    }
    return ((uint64_t)high << 32) | low;
}

// Read performance monitoring counter
uint64_t read_pmc(uint32_t counter) {
    uint32_t low, high;
    uint32_t counter_val = counter;
    __asm {
        mov ecx, counter_val
        rdpmc
        mov low, eax
        mov high, edx
    }
    return ((uint64_t)high << 32) | low;
}

// Hardware random number generation
uint64_t rdrand(void) {
    uint64_t result;
    __asm {
        rdrand rax
        mov result, rax
    }
    return result;
}

// Hardware seed generation
uint64_t rdseed(void) {
    uint64_t result;
    __asm {
        rdseed rax
        mov result, rax
    }
    return result;
}

// Flush cache line
void clflush(void* addr) {
    uint64_t addr_val = (uint64_t)addr;
    __asm {
        clflush [addr_val]
    }
}

// Prefetch cache line
void prefetch(void* addr) {
    uint64_t addr_val = (uint64_t)addr;
    __asm {
        prefetcht0 [addr_val]
    }
}

// Read debug registers
uint64_t read_dr(uint32_t reg) {
    uint64_t value;
    uint64_t rax_val;
    switch (reg) {
        case 0: __asm { mov rax_val, dr0 } value = rax_val; break;
        case 1: __asm { mov rax_val, dr1 } value = rax_val; break;
        case 2: __asm { mov rax_val, dr2 } value = rax_val; break;
        case 3: __asm { mov rax_val, dr3 } value = rax_val; break;
        case 6: __asm { mov rax_val, dr6 } value = rax_val; break;
        case 7: __asm { mov rax_val, dr7 } value = rax_val; break;
        default: value = 0; break;
    }
    return value;
}

// Write debug registers
void write_dr(uint32_t reg, uint64_t value) {
    uint64_t temp = value;
    uint64_t rax_temp;
    switch (reg) {
        case 0: __asm { mov rax_temp, temp; mov dr0, rax_temp } break;
        case 1: __asm { mov rax_temp, temp; mov dr1, rax_temp } break;
        case 2: __asm { mov rax_temp, temp; mov dr2, rax_temp } break;
        case 3: __asm { mov rax_temp, temp; mov dr3, rax_temp } break;
        case 6: __asm { mov rax_temp, temp; mov dr6, rax_temp } break;
        case 7: __asm { mov rax_temp, temp; mov dr7, rax_temp } break;
    }
}

// Read RFLAGS register
uint64_t read_rflags(void) {
    uint64_t flags;
    __asm {
        pushfq
        pop rax
        mov flags, rax
    }
    return flags;
}

// Memory barrier operations
void memory_barrier(void) {
    __asm {
        mfence
    }
}

// Load fence (read barrier)
void lfence(void) {
    __asm {
        lfence
    }
}

// Store fence (write barrier)
void sfence(void) {
    __asm {
        sfence
    }
}

// CPU pause instruction
void cpu_pause(void) {
    __asm {
        pause
    }
}

#endif // _MSC_VER
