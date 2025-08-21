/*
 * Orion Operating System - x86_64 CPU Management & Security Features
 *
 * CPU feature detection and hardware security initialization for x86_64.
 * Includes SMEP, SMAP, UMIP, NX bit, RDRAND/RDSEED support.
 *
 * Developed by Jérémy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/kernel.h>
#include <orion/types.h>
#include <orion/security.h>
#include <arch.h>

// CPU feature flags
static struct {
    bool sse;
    bool sse2;
    bool sse3;
    bool ssse3;
    bool sse41;
    bool sse42;
    bool avx;
    bool avx2;
    bool smep;
    bool smap;
    bool umip;
    bool rdrand;
    bool rdseed;
} cpu_features;

// Détecter les features CPU
static void detect_cpu_features(void) {
    uint32_t eax, ebx, ecx, edx;
    
    // CPUID.1 - Features de base
    cpuid(1, &eax, &ebx, &ecx, &edx);
    
    cpu_features.sse = (edx & (1 << 25)) != 0;
    cpu_features.sse2 = (edx & (1 << 26)) != 0;
    cpu_features.sse3 = (ecx & (1 << 0)) != 0;
    cpu_features.ssse3 = (ecx & (1 << 9)) != 0;
    cpu_features.sse41 = (ecx & (1 << 19)) != 0;
    cpu_features.sse42 = (ecx & (1 << 20)) != 0;
    cpu_features.avx = (ecx & (1 << 28)) != 0;
    cpu_features.rdrand = (ecx & (1 << 30)) != 0;
    
    // CPUID.7 - Features étendues
    cpuid(7, &eax, &ebx, &ecx, &edx);
    
    cpu_features.avx2 = (ebx & (1 << 5)) != 0;
    cpu_features.smep = (ebx & (1 << 7)) != 0;
    cpu_features.smap = (ebx & (1 << 20)) != 0;
    cpu_features.umip = (ecx & (1 << 2)) != 0;
    cpu_features.rdseed = (ebx & (1 << 18)) != 0;
    
    kinfo("CPU Features detected:");
    kinfo("  SSE: %s, SSE2: %s, AVX: %s, AVX2: %s", 
          cpu_features.sse ? "yes" : "no",
          cpu_features.sse2 ? "yes" : "no", 
          cpu_features.avx ? "yes" : "no",
          cpu_features.avx2 ? "yes" : "no");
    kinfo("  Security: SMEP=%s, SMAP=%s, UMIP=%s",
          cpu_features.smep ? "yes" : "no",
          cpu_features.smap ? "yes" : "no",
          cpu_features.umip ? "yes" : "no");
}

// Configurer les features de sécurité dans CR4
static void enable_security_features(void) {
    uint64_t cr4 = read_cr4();
    bool changed = false;
    
    // Activer SMEP si disponible
    if (cpu_features.smep) {
        cr4 |= CR4_SMEP;
        changed = true;
        kinfo("SMEP enabled");
    }
    
    // Activer SMAP si disponible
    if (cpu_features.smap) {
        cr4 |= CR4_SMAP;
        changed = true;
        kinfo("SMAP enabled");
    }
    
    // Activer UMIP si disponible
    if (cpu_features.umip) {
        cr4 |= CR4_UMIP;
        changed = true;
        kinfo("UMIP enabled");
    }
    
    if (changed) {
        write_cr4(cr4);
    }
}

// Configurer EFER pour les features de sécurité
static void enable_efer_features(void) {
    uint64_t efer = msr_read(MSR_EFER);
    bool changed = false;
    
    // S'assurer que NX est activé
    if (!(efer & EFER_NXE)) {
        efer |= EFER_NXE;
        changed = true;
        kinfo("NX bit enabled");
    }
    
    if (changed) {
        msr_write(MSR_EFER, efer);
    }
}

// Initialisation précoce de l'architecture
void arch_early_init(void) {
    kinfo("x86_64 architecture early initialization");
    
    // Détecter les features CPU
    detect_cpu_features();
    
    // Activer les features de sécurité
    enable_security_features();
    enable_efer_features();
    
    kinfo("x86_64 early init complete");
}

// Initialisation tardive de l'architecture
void arch_late_init(void) {
    kinfo("x86_64 architecture late initialization");
    
    // TODO: Initialiser APIC, SMP, etc.
    
    kinfo("x86_64 late init complete");
}

// Initialisation du CPU actuel
void arch_cpu_init(void) {
    // Pour l'instant, juste le BSP
    kinfo("CPU 0 initialized");
}

// CPU feature getters
bool cpu_has_sse(void) { return cpu_features.sse; }
bool cpu_has_sse2(void) { return cpu_features.sse2; }
bool cpu_has_avx(void) { return cpu_features.avx; }
bool cpu_has_avx2(void) { return cpu_features.avx2; }
bool cpu_has_smep(void) { return cpu_features.smep; }
bool cpu_has_smap(void) { return cpu_features.smap; }
bool cpu_has_rdrand(void) { return cpu_features.rdrand; }
bool cpu_has_rdseed(void) { return cpu_features.rdseed; }

// ========================================
// SECURITY-SPECIFIC FUNCTIONS
// ========================================

// Get high-precision timestamp counter for entropy
uint64_t arch_get_rdtsc(void) {
    uint32_t low, high;
    __asm__ volatile ("rdtsc" : "=a" (low), "=d" (high));
    return ((uint64_t)high << 32) | low;
}



// Enable SMEP (Supervisor Mode Execution Prevention)
void arch_enable_smep(void) {
    if (!cpu_features.smep) {
        kdebug("SMEP not supported by CPU");
        return;
    }
    
    uint64_t cr4 = read_cr4();
    if (!(cr4 & CR4_SMEP)) {
        cr4 |= CR4_SMEP;
        write_cr4(cr4);
        kinfo("SMEP enabled");
    }
}

// Enable SMAP (Supervisor Mode Access Prevention)
void arch_enable_smap(void) {
    if (!cpu_features.smap) {
        kdebug("SMAP not supported by CPU");
        return;
    }
    
    uint64_t cr4 = read_cr4();
    if (!(cr4 & CR4_SMAP)) {
        cr4 |= CR4_SMAP;
        write_cr4(cr4);
        kinfo("SMAP enabled");
    }
}

// Enable UMIP (User Mode Instruction Prevention)
void arch_enable_umip(void) {
    if (!cpu_features.umip) {
        kdebug("UMIP not supported by CPU");
        return;
    }
    
    uint64_t cr4 = read_cr4();
    if (!(cr4 & CR4_UMIP)) {
        cr4 |= CR4_UMIP;
        write_cr4(cr4);
        kinfo("UMIP enabled");
    }
}

// Validate user address ranges
bool arch_validate_user_address(uint64_t vaddr, uint64_t size, bool write) {
    // x86_64 canonical address check
    if (vaddr >= 0x0000800000000000ULL && vaddr < 0xFFFF800000000000ULL) {
        return false; // Non-canonical address
    }
    
    // Check for user space (below 0x00007FFFFFFFFFFFULL)
    if (vaddr >= 0x0000800000000000ULL) {
        return false; // Kernel space
    }
    
    // Check for overflow
    if (vaddr + size < vaddr) {
        return false;
    }
    
    // Check against known dangerous areas
    if (vaddr < 0x1000) {
        return false; // NULL pointer dereference protection
    }
    
    // Additional checks for specific operations
    if (write) {
        // Check if memory is marked as read-only in page tables
        // TODO: Implement page table walk for write permission check
    }
    
    return true;
}

// Get hardware random number using RDRAND
uint64_t arch_get_hardware_random(void) {
    if (!cpu_features.rdrand) {
        return 0; // Not supported
    }
    
    uint64_t result;
    int retries = 10;
    
    while (retries-- > 0) {
        __asm__ volatile (
            "rdrand %0"
            : "=r" (result)
            :
            : "cc"
        );
        
        // Check if RDRAND succeeded (CF flag set)
        uint64_t flags;
        __asm__ volatile ("pushfq; popq %0" : "=r" (flags));
        
        if (flags & 0x1) { // CF flag
            return result;
        }
    }
    
    return 0; // Failed to get random number
}

// Get hardware seed using RDSEED
uint64_t arch_get_hardware_seed(void) {
    if (!cpu_features.rdseed) {
        return 0; // Not supported
    }
    
    uint64_t result;
    int retries = 10;
    
    while (retries-- > 0) {
        __asm__ volatile (
            "rdseed %0"
            : "=r" (result)
            :
            : "cc"
        );
        
        // Check if RDSEED succeeded (CF flag set)
        uint64_t flags;
        __asm__ volatile ("pushfq; popq %0" : "=r" (flags));
        
        if (flags & 0x1) { // CF flag
            return result;
        }
    }
    
    return 0; // Failed to get seed
}

// Setup syscall interface (MSR configuration)
void arch_setup_syscall_interface(void) {
    // Configure SYSCALL/SYSRET MSRs
    // STAR: CS selectors for kernel/user
    uint64_t star = ((uint64_t)0x08 << 32) | ((uint64_t)0x18 << 48);
    msr_write(MSR_STAR, star);
    
    // LSTAR: Syscall entry point
    msr_write(MSR_LSTAR, (uint64_t)syscall_entry);
    
    // SFMASK: RFLAGS mask
    msr_write(MSR_SFMASK, 0x200); // Clear IF (interrupts)
    
    kinfo("x86_64 syscall interface configured");
}

// Context switch (save/restore processor state)
void arch_context_switch(thread_t *prev, thread_t *next) {
    // Save current context
    __asm__ volatile (
        "pushfq\n\t"
        "pushq %%rax\n\t"
        "pushq %%rbx\n\t"
        "pushq %%rcx\n\t"
        "pushq %%rdx\n\t"
        "pushq %%rsi\n\t"
        "pushq %%rdi\n\t"
        "pushq %%rbp\n\t"
        "pushq %%r8\n\t"
        "pushq %%r9\n\t"
        "pushq %%r10\n\t"
        "pushq %%r11\n\t"
        "pushq %%r12\n\t"
        "pushq %%r13\n\t"
        "pushq %%r14\n\t"
        "pushq %%r15\n\t"
        "movq %%rsp, %0\n\t"
        "movq %1, %%rsp\n\t"
        "popq %%r15\n\t"
        "popq %%r14\n\t"
        "popq %%r13\n\t"
        "popq %%r12\n\t"
        "popq %%r11\n\t"
        "popq %%r10\n\t"
        "popq %%r9\n\t"
        "popq %%r8\n\t"
        "popq %%rbp\n\t"
        "popq %%rdi\n\t"
        "popq %%rsi\n\t"
        "popq %%rdx\n\t"
        "popq %%rcx\n\t"
        "popq %%rbx\n\t"
        "popq %%rax\n\t"
        "popfq"
        : "=m" (prev->rsp)
        : "m" (next->rsp)
        : "memory"
    );
}

// CPU idle (halt until interrupt)
void arch_cpu_idle(void) {
    __asm__ volatile ("hlt");
}

// Get boot time from CMOS/RTC
uint64_t arch_get_boot_time(void) {
    // TODO: Read from RTC
    return 0; // Placeholder
}

// Get CPU count from ACPI/MP tables
uint32_t arch_get_cpu_count(void) {
    // TODO: Parse ACPI MADT or MP tables
    return 1; // Single CPU for now
}

// ========================================
// MISSING ARCHITECTURE FUNCTIONS
// ========================================

// Halt the CPU indefinitely
void arch_halt(void) {
    // Disable interrupts and halt
    __asm__ volatile (
        "cli\n\t"
        "hlt"
        :
        :
        : "memory"
    );
    
    // If we somehow continue, loop forever
    while (1) {
        arch_cpu_idle();
    }
}

// Pause instruction for spinlock optimization
void arch_pause(void) {
    __asm__ volatile ("pause" ::: "memory");
}

// Check if CPU is halted
bool arch_is_halted(void) {
    // Check RFLAGS.IF (interrupt flag)
    uint64_t flags;
    __asm__ volatile ("pushfq; popq %0" : "=r" (flags));
    return !(flags & 0x200); // IF flag is bit 9
}

// Get CPU feature flags
uint32_t arch_get_cpu_features(void) {
    uint32_t features = 0;
    
    if (cpu_has_sse()) features |= (1 << 0);
    if (cpu_has_sse2()) features |= (1 << 1);
    if (cpu_has_avx()) features |= (1 << 2);
    if (cpu_has_avx2()) features |= (1 << 3);
    if (cpu_has_smep()) features |= (1 << 4);
    if (cpu_has_smap()) features |= (1 << 5);
    if (cpu_has_rdrand()) features |= (1 << 6);
    if (cpu_has_rdseed()) features |= (1 << 7);
    
    return features;
}

// Invalidate instruction cache
void arch_flush_icache(void) {
    // x86_64 has coherent instruction cache
    // Just ensure memory ordering
    __asm__ volatile ("mfence" ::: "memory");
}

// Memory barrier operations
void arch_memory_barrier(void) {
    __asm__ volatile ("mfence" ::: "memory");
}

void arch_read_barrier(void) {
    __asm__ volatile ("lfence" ::: "memory");
}

void arch_write_barrier(void) {
    __asm__ volatile ("sfence" ::: "memory");
}

// CPU model detection
void arch_get_cpu_info(char* vendor, char* model, uint32_t* family, uint32_t* stepping) {
    uint32_t eax, ebx, ecx, edx;
    
    // Get vendor string
    cpuid(0, &eax, &ebx, &ecx, &edx);
    if (vendor) {
        *(uint32_t*)(vendor + 0) = ebx;
        *(uint32_t*)(vendor + 4) = edx;
        *(uint32_t*)(vendor + 8) = ecx;
        vendor[12] = '\0';
    }
    
    // Get family and stepping
    cpuid(1, &eax, &ebx, &ecx, &edx);
    if (family) {
        *family = ((eax >> 8) & 0xF) + ((eax >> 20) & 0xFF);
    }
    if (stepping) {
        *stepping = eax & 0xF;
    }
    
    // Model name would require CPUID 0x80000002-0x80000004
    if (model) {
        strcpy(model, "Unknown x86_64 CPU");
    }
}

// Performance counter access
uint64_t arch_read_performance_counter(uint32_t counter) {
    // Read performance monitoring counter
    uint32_t low, high;
    __asm__ volatile (
        "rdpmc"
        : "=a" (low), "=d" (high)
        : "c" (counter)
    );
    return ((uint64_t)high << 32) | low;
}

// CPU cache operations
void arch_flush_cache_line(void* addr) {
    __asm__ volatile ("clflush (%0)" :: "r" (addr) : "memory");
}

void arch_prefetch_cache_line(void* addr) {
    __asm__ volatile ("prefetcht0 (%0)" :: "r" (addr) : "memory");
}

// I/O port access (extended)
uint16_t arch_inw(uint16_t port) {
    uint16_t data;
    __asm__ volatile ("inw %1, %0" : "=a" (data) : "Nd" (port));
    return data;
}

uint32_t arch_inl(uint16_t port) {
    uint32_t data;
    __asm__ volatile ("inl %1, %0" : "=a" (data) : "Nd" (port));
    return data;
}

void arch_outw(uint16_t port, uint16_t data) {
    __asm__ volatile ("outw %0, %1" :: "a" (data), "Nd" (port));
}

void arch_outl(uint16_t port, uint32_t data) {
    __asm__ volatile ("outl %0, %1" :: "a" (data), "Nd" (port));
}

// Debug register access
uint64_t arch_read_debug_register(uint32_t reg) {
    uint64_t value = 0;
    switch (reg) {
        case 0: __asm__ volatile ("mov %%dr0, %0" : "=r" (value)); break;
        case 1: __asm__ volatile ("mov %%dr1, %0" : "=r" (value)); break;
        case 2: __asm__ volatile ("mov %%dr2, %0" : "=r" (value)); break;
        case 3: __asm__ volatile ("mov %%dr3, %0" : "=r" (value)); break;
        case 6: __asm__ volatile ("mov %%dr6, %0" : "=r" (value)); break;
        case 7: __asm__ volatile ("mov %%dr7, %0" : "=r" (value)); break;
    }
    return value;
}

void arch_write_debug_register(uint32_t reg, uint64_t value) {
    switch (reg) {
        case 0: __asm__ volatile ("mov %0, %%dr0" :: "r" (value)); break;
        case 1: __asm__ volatile ("mov %0, %%dr1" :: "r" (value)); break;
        case 2: __asm__ volatile ("mov %0, %%dr2" :: "r" (value)); break;
        case 3: __asm__ volatile ("mov %0, %%dr3" :: "r" (value)); break;
        case 6: __asm__ volatile ("mov %0, %%dr6" :: "r" (value)); break;
        case 7: __asm__ volatile ("mov %0, %%dr7" :: "r" (value)); break;
    }
}


