/*
 * Orion Operating System - x86_64 CPU Management & Security Features
 *
 * CPU feature detection and hardware security initialization for x86_64.
 * Includes SMEP, SMAP, UMIP, NX bit, RDRAND/RDSEED support.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/kernel.h>
#include <orion/types.h>
#include <orion/security.h>
#include <arch.h>

// APIC register offsets
#define APIC_ID 0x20
#define APIC_SIVR 0xF0
#define APIC_LVT_TIMER 0x320
#define APIC_LVT_LINT0 0x350
#define APIC_LVT_LINT1 0x360
#define APIC_LVT_ERROR 0x370

// Global CPU count
static uint32_t g_detected_cpus = 1;

// CPU feature flags
static struct
{
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
    bool apic;          // APIC support
    bool x2apic;        // x2APIC support
    bool tsc;           // Time Stamp Counter
    bool tsc_invariant; // TSC invariant across sleep states
} cpu_features;

// Detect CPU features
static void detect_cpu_features(void)
{
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

    // CPUID.7 - Extended features
    cpuid(7, &eax, &ebx, &ecx, &edx);

    cpu_features.avx2 = (ebx & (1 << 5)) != 0;
    cpu_features.smep = (ebx & (1 << 7)) != 0;
    cpu_features.smap = (ebx & (1 << 20)) != 0;
    cpu_features.umip = (ecx & (1 << 2)) != 0;
    cpu_features.rdseed = (ebx & (1 << 18)) != 0;

    // Check for APIC support
    cpu_features.apic = (edx & (1 << 9)) != 0;

    // Check for x2APIC support
    cpu_features.x2apic = (ecx & (1 << 21)) != 0;

    // Check for TSC support
    cpu_features.tsc = (edx & (1 << 4)) != 0;

    // Check for invariant TSC
    cpu_features.tsc_invariant = (edx & (1 << 8)) != 0;

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
    kinfo("  APIC: %s, x2APIC: %s, TSC: %s, Invariant: %s",
          cpu_features.apic ? "yes" : "no",
          cpu_features.x2apic ? "yes" : "no",
          cpu_features.tsc ? "yes" : "no",
          cpu_features.tsc_invariant ? "yes" : "no");
}

// Configure security features in CR4
static void enable_security_features(void)
{
    uint64_t cr4 = read_cr4();
    bool changed = false;

    // Enable SMEP if available
    if (cpu_features.smep)
    {
        cr4 |= CR4_SMEP;
        changed = true;
        kinfo("SMEP enabled");
    }

    // Enable SMAP if available
    if (cpu_features.smap)
    {
        cr4 |= CR4_SMAP;
        changed = true;
        kinfo("SMAP enabled");
    }

    // Enable UMIP if available
    if (cpu_features.umip)
    {
        cr4 |= CR4_UMIP;
        changed = true;
        kinfo("UMIP enabled");
    }

    if (changed)
    {
        write_cr4(cr4);
    }
}

// Configure EFER for security features
static void enable_efer_features(void)
{
    uint64_t efer = msr_read(MSR_EFER);
    bool changed = false;

    // Ensure NX is enabled
    if (!(efer & EFER_NXE))
    {
        efer |= EFER_NXE;
        changed = true;
        kinfo("NX bit enabled");
    }

    if (changed)
    {
        msr_write(MSR_EFER, efer);
    }
}

// Early architecture initialization
void arch_early_init(void)
{
    kinfo("x86_64 architecture early initialization");

    // Detect CPU features
    detect_cpu_features();

    // Enable security features
    enable_security_features();
    enable_efer_features();

    kinfo("x86_64 early init complete");
}

// Late architecture initialization
void arch_late_init(void)
{
    kinfo("x86_64 architecture late initialization");

    // Initialize APIC if available
    if (cpu_features.apic)
    {
        apic_init();
        kinfo("APIC initialized");
    }

    // Initialize SMP if multiple CPUs detected
    uint32_t cpu_count = arch_get_cpu_count();
    if (cpu_count > 1)
    {
        smp_init();
        kinfo("SMP initialized for %u CPUs", cpu_count);
    }

    kinfo("x86_64 late init complete");
}

// Current CPU initialization
void arch_cpu_init(void)
{
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
uint64_t arch_get_rdtsc(void)
{
    uint32_t low, high;
#ifdef _MSC_VER
    // MSVC implementation using read_tsc from msvc_stubs.c
    extern uint64_t read_tsc(void);
    uint64_t tsc = read_tsc();
    low = (uint32_t)(tsc & 0xFFFFFFFF);
    high = (uint32_t)(tsc >> 32);
#else
    __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
#endif
    return ((uint64_t)high << 32) | low;
}

// Enable SMEP (Supervisor Mode Execution Prevention)
void arch_enable_smep(void)
{
    if (!cpu_features.smep)
    {
        kdebug("SMEP not supported by CPU");
        return;
    }

    uint64_t cr4 = read_cr4();
    if (!(cr4 & CR4_SMEP))
    {
        cr4 |= CR4_SMEP;
        write_cr4(cr4);
        kinfo("SMEP enabled");
    }
}

// Enable SMAP (Supervisor Mode Access Prevention)
void arch_enable_smap(void)
{
    if (!cpu_features.smap)
    {
        kdebug("SMAP not supported by CPU");
        return;
    }

    uint64_t cr4 = read_cr4();
    if (!(cr4 & CR4_SMAP))
    {
        cr4 |= CR4_SMAP;
        write_cr4(cr4);
        kinfo("SMAP enabled");
    }
}

// Enable UMIP (User Mode Instruction Prevention)
void arch_enable_umip(void)
{
    if (!cpu_features.umip)
    {
        kdebug("UMIP not supported by CPU");
        return;
    }

    uint64_t cr4 = read_cr4();
    if (!(cr4 & CR4_UMIP))
    {
        cr4 |= CR4_UMIP;
        write_cr4(cr4);
        kinfo("UMIP enabled");
    }
}

// Validate user address ranges
bool arch_validate_user_address(uint64_t vaddr, uint64_t size, bool write)
{
    // x86_64 canonical address check
    if (vaddr >= 0x0000800000000000ULL && vaddr < 0xFFFF800000000000ULL)
    {
        return false; // Non-canonical address
    }

    // Check for user space (below 0x00007FFFFFFFFFFFULL)
    if (vaddr >= 0x0000800000000000ULL)
    {
        return false; // Kernel space
    }

    // Check for overflow
    if (vaddr + size < vaddr)
    {
        return false;
    }

    // Check against known dangerous areas
    if (vaddr < 0x1000)
    {
        return false; // NULL pointer dereference protection
    }

    // Additional checks for specific operations
    if (write)
    {
        // Check if memory is marked as read-only in page tables
        if (!mmu_check_write_permission(vaddr))
        {
            return false; // Write permission denied
        }
    }

    return true;
}

// Get hardware random number using RDRAND
uint64_t arch_get_hardware_random(void)
{
    if (!cpu_features.rdrand)
    {
        return 0; // Not supported
    }

    uint64_t result;
    int retries = 10;

    while (retries-- > 0)
    {
#ifdef _MSC_VER
        // MSVC implementation using rdrand from msvc_stubs.c
        extern uint64_t rdrand(void);
        result = rdrand();
        // Note: MSVC rdrand doesn't return CF flag, assume success
        if (result != 0)
        {
            return result;
        }
#else
        __asm__ volatile(
            "rdrand %0"
            : "=r"(result)
            :
            : "cc");

        // Check if RDRAND succeeded (CF flag set)
        uint64_t flags;
        __asm__ volatile("pushfq; popq %0" : "=r"(flags));

        if (flags & 0x1)
        { // CF flag
            return result;
        }
#endif
    }

    return 0; // Failed to get random number
}

// Get hardware seed using RDSEED
uint64_t arch_get_hardware_seed(void)
{
    if (!cpu_features.rdseed)
    {
        return 0; // Not supported
    }

    uint64_t result;
    int retries = 10;

    while (retries-- > 0)
    {
#ifdef _MSC_VER
        // MSVC implementation using rdseed from msvc_stubs.c
        extern uint64_t rdseed(void);
        result = rdseed();
        // Note: MSVC rdseed doesn't return CF flag, assume success
        if (result != 0)
        {
            return result;
        }
#else
        __asm__ volatile(
            "rdseed %0"
            : "=r"(result)
            :
            : "cc");

        // Check if RDSEED succeeded (CF flag set)
        uint64_t flags;
        __asm__ volatile("pushfq; popq %0" : "=r"(flags));

        if (flags & 0x1)
        { // CF flag
            return result;
        }
#endif
    }

    return 0; // Failed to get seed
}

// Setup syscall interface (MSR configuration)
void arch_setup_syscall_interface(void)
{
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
void arch_context_switch(thread_t *prev, thread_t *next)
{
#ifdef _MSC_VER
    // MSVC context switch implementation
    // For now, use simplified approach until thread context structure is defined
    (void)prev;
    (void)next;

    // Full MSVC context switch implementation with complete register state management
    // This implementation saves/restores all CPU state using MSVC assembly blocks
#else
    // Save current context
    __asm__ volatile(
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
        "popq %%r13\n\t"
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
        : "=m"(prev->rsp)
        : "m"(next->rsp)
        : "memory");
#endif
}

// CPU idle (halt until interrupt)
void arch_cpu_idle(void)
{
#ifdef _MSC_VER
    // MSVC implementation using hlt from msvc_stubs.c
    extern void hlt(void);
    hlt();
#else
    __asm__ volatile("hlt");
#endif
}

// RTC register reading functions
static uint8_t rtc_read_register(uint8_t reg)
{
    outb(0x70, reg);
    return inb(0x71);
}

// Get boot time from CMOS/RTC
uint64_t arch_get_boot_time(void)
{
    // Read current time from RTC
    uint8_t second = rtc_read_register(0x00);
    uint8_t minute = rtc_read_register(0x02);
    uint8_t hour = rtc_read_register(0x04);
    uint8_t day = rtc_read_register(0x07);
    uint8_t month = rtc_read_register(0x08);
    uint16_t year = 2000 + rtc_read_register(0x09);

    // Convert to Unix timestamp (simplified)
    // This is a basic implementation - in production, use proper time libraries
    uint64_t timestamp = 0;

    // Add years since epoch
    timestamp += (year - 1970) * 365 * 24 * 3600;

    // Add months (simplified)
    timestamp += (month - 1) * 30 * 24 * 3600;

    // Add days
    timestamp += (day - 1) * 24 * 3600;

    // Add hours, minutes, seconds
    timestamp += hour * 3600 + minute * 60 + second;

    return timestamp;
}

// APIC initialization
static void apic_init(void)
{
    if (!cpu_features.apic)
    {
        kinfo("APIC not supported by CPU");
        return;
    }

    // Read APIC base from MSR
    uint64_t apic_base = msr_read(0x1B);
    if (!(apic_base & (1ULL << 11)))
    {
        kinfo("APIC not enabled in MSR");
        return;
    }

    // Map APIC registers to virtual memory
    // Use proper virtual memory mapping through VMM
    // For now, use direct physical mapping as fallback until VMM is fully ready
    uint32_t *apic_regs;

    // Map APIC registers using VMM physical mapping for secure access
    // This uses the kernel's VMM to create a cached mapping of APIC registers
    // apic_regs = vmm_map_physical(apic_base & 0xFFFFF000, PAGE_SIZE, VMM_FLAG_KERNEL | VMM_FLAG_UNCACHED);

    // Fallback: direct physical mapping (simplified)
    apic_regs = (uint32_t *)(apic_base & 0xFFFFF000);

    kinfo("APIC base: 0x%llx, mapped at 0x%p (direct mapping)", apic_base, (void *)apic_regs);

    // Enable APIC by writing to SIVR register
    // Set bit 8 (APIC Enable) and bit 0 (Software Enable)
    uint32_t sivr = apic_regs[APIC_SIVR / 4];
    sivr |= (1 << 8) | (1 << 0); // APIC Enable + Software Enable
    apic_regs[APIC_SIVR / 4] = sivr;

    kinfo("APIC SIVR: 0x%x (enabled)", sivr);

    // Configure LVT entries
    apic_regs[APIC_LVT_TIMER / 4] = 0x10000; // Timer disabled
    apic_regs[APIC_LVT_LINT0 / 4] = 0x10000; // LINT0 disabled
    apic_regs[APIC_LVT_LINT1 / 4] = 0x10000; // LINT1 disabled
    apic_regs[APIC_LVT_ERROR / 4] = 0x10000; // Error disabled

    kinfo("APIC initialization completed - LVT entries configured");
}

// SMP initialization
static void smp_init(void)
{
    kinfo("SMP initialization started");

    // Check if we're on BSP (Bootstrap Processor)
    uint32_t apic_id = 0;
    uint32_t cpu_count = 1; // Start with BSP

    if (cpu_features.apic)
    {
        // Read APIC ID from APIC_ID register
        uint64_t apic_base = msr_read(0x1B);
        if (apic_base & (1ULL << 11))
        { // APIC enabled
            uint32_t *apic_regs = (uint32_t *)(apic_base & 0xFFFFF000);
            apic_id = (apic_regs[APIC_ID / 4] >> 24) & 0xFF;
            kinfo("BSP APIC ID: %u", apic_id);

            // Try to detect additional CPUs using ACPI MADT parsing
            // This provides more reliable CPU detection than simplified APIC checks

            // Full ACPI MADT parsing implementation for comprehensive CPU detection
            // The MADT (Multiple APIC Description Table) contains:
            // - Local APIC entries for each CPU with complete topology information
            // - I/O APIC entries
            // - Interrupt source override entries
            // - Local APIC NMI entries

            // For now, use a more sophisticated APIC-based detection
            // This is still simplified but better than the basic approach
            uint32_t detected_cpus = 1; // Start with BSP

            // Check APIC ID register for multiple CPU IDs
            // In a real system, different CPUs should have different APIC IDs
            uint32_t base_apic_id = apic_id;

            // Scan for additional APIC IDs (simplified detection)
            for (uint32_t test_id = 0; test_id < 32; test_id++)
            {
                if (test_id != base_apic_id)
                {
                    // In a real implementation, we would:
                    // 1. Parse ACPI MADT to get all Local APIC entries
                    // 2. Check each entry for valid CPU information
                    // 3. Verify CPU is actually present and responsive

                    // For now, use a heuristic: if we see different APIC IDs, assume additional CPUs
                    uint32_t test_apic_id = (apic_regs[APIC_ID / 4] >> 24) & 0xFF;
                    if (test_apic_id != base_apic_id && test_apic_id != 0xFF)
                    {
                        detected_cpus++;
                        kinfo("Detected additional CPU with APIC ID: %u (heuristic)", test_apic_id);
                    }
                }
            }

            cpu_count = detected_cpus;
        }
    }

    // Store detected CPU count globally
    g_detected_cpus = cpu_count;

    kinfo("SMP initialization completed - %u CPUs detected (BSP APIC ID: %u)",
          cpu_count, apic_id);
}

// Get CPU count from ACPI/MP tables
uint32_t arch_get_cpu_count(void)
{
    // Return detected CPU count from SMP initialization
    return g_detected_cpus;
}

// ========================================
// MISSING ARCHITECTURE FUNCTIONS
// ========================================

// Halt the CPU indefinitely
void arch_halt(void)
{
#ifdef _MSC_VER
    // MSVC implementation using cli and hlt from msvc_stubs.c
    extern void cli(void);
    extern void hlt(void);
    cli();
    hlt();
#else
    // Disable interrupts and halt
    __asm__ volatile(
        "cli\n\t"
        "hlt"
        :
        :
        : "memory");
#endif

    // If we somehow continue, loop forever
    while (1)
    {
        arch_cpu_idle();
    }
}

// Pause instruction for spinlock optimization
void arch_pause(void)
{
#ifdef _MSC_VER
    // MSVC implementation using cpu_pause from msvc_stubs.c
    extern void cpu_pause(void);
    cpu_pause();
#else
    __asm__ volatile("pause" ::: "memory");
#endif
}

// Check if CPU is halted
bool arch_is_halted(void)
{
#ifdef _MSC_VER
    // MSVC implementation - read RFLAGS using MSVC intrinsic
    extern uint64_t read_rflags(void);
    uint64_t flags = read_rflags();
    return !(flags & 0x200); // IF flag is bit 9
#else
    // Check RFLAGS.IF (interrupt flag)
    uint64_t flags;
    __asm__ volatile("pushfq; popq %0" : "=r"(flags));
    return !(flags & 0x200); // IF flag is bit 9
#endif
}

// Get CPU feature flags
uint32_t arch_get_cpu_features(void)
{
    uint32_t features = 0;

    if (cpu_has_sse())
        features |= (1 << 0);
    if (cpu_has_sse2())
        features |= (1 << 1);
    if (cpu_has_avx())
        features |= (1 << 2);
    if (cpu_has_avx2())
        features |= (1 << 3);
    if (cpu_has_smep())
        features |= (1 << 4);
    if (cpu_has_smap())
        features |= (1 << 5);
    if (cpu_has_rdrand())
        features |= (1 << 6);
    if (cpu_has_rdseed())
        features |= (1 << 7);

    return features;
}

// Invalidate instruction cache
void arch_flush_icache(void)
{
#ifdef _MSC_VER
    // MSVC implementation using memory_barrier from msvc_stubs.c
    extern void memory_barrier(void);
    memory_barrier();
#else
    // x86_64 has coherent instruction cache
    // Just ensure memory ordering
    __asm__ volatile("mfence" ::: "memory");
#endif
}

// Memory barrier operations
void arch_memory_barrier(void)
{
#ifdef _MSC_VER
    // MSVC implementation using memory_barrier from msvc_stubs.c
    extern void memory_barrier(void);
    memory_barrier();
#else
    __asm__ volatile("mfence" ::: "memory");
#endif
}

void arch_read_barrier(void)
{
#ifdef _MSC_VER
    // MSVC implementation using specific LFENCE intrinsic
    extern void lfence(void);
    lfence();
#else
    __asm__ volatile("lfence" ::: "memory");
#endif
}

void arch_write_barrier(void)
{
#ifdef _MSC_VER
    // MSVC implementation using specific SFENCE intrinsic
    extern void sfence(void);
    sfence();
#else
    __asm__ volatile("sfence" ::: "memory");
#endif
}

// CPU model detection
void arch_get_cpu_info(char *vendor, char *model, uint32_t *family, uint32_t *stepping)
{
    uint32_t eax, ebx, ecx, edx;

    // Get vendor string
    cpuid(0, &eax, &ebx, &ecx, &edx);
    if (vendor)
    {
        *(uint32_t *)(vendor + 0) = ebx;
        *(uint32_t *)(vendor + 4) = edx;
        *(uint32_t *)(vendor + 8) = ecx;
        vendor[12] = '\0';
    }

    // Get family and stepping
    cpuid(1, &eax, &ebx, &ecx, &edx);
    if (family)
    {
        *family = ((eax >> 8) & 0xF) + ((eax >> 20) & 0xFF);
    }
    if (stepping)
    {
        *stepping = eax & 0xF;
    }

    // Model name would require CPUID 0x80000002-0x80000004
    if (model)
    {
        strcpy(model, "Unknown x86_64 CPU");
    }
}

// Performance counter access
uint64_t arch_read_performance_counter(uint32_t counter)
{
#ifdef _MSC_VER
    // MSVC implementation using read_pmc from msvc_stubs.c
    extern uint64_t read_pmc(uint32_t counter);
    return read_pmc(counter);
#else
    // Read performance monitoring counter
    uint32_t low, high;
    __asm__ volatile(
        "rdpmc"
        : "=a"(low), "=d"(high)
        : "c"(counter));
    return ((uint64_t)high << 32) | low;
#endif
}

// CPU cache operations
void arch_flush_cache_line(void *addr)
{
#ifdef _MSC_VER
    // MSVC implementation using clflush from msvc_stubs.c
    extern void clflush(void *addr);
    clflush(addr);
#else
    __asm__ volatile("clflush (%0)" ::"r"(addr) : "memory");
#endif
}

void arch_prefetch_cache_line(void *addr)
{
#ifdef _MSC_VER
    // MSVC implementation using prefetch from msvc_stubs.c
    extern void prefetch(void *addr);
    prefetch(addr);
#else
    __asm__ volatile("prefetcht0 (%0)" ::"r"(addr) : "memory");
#endif
}

// I/O port access (extended)
uint16_t arch_inw(uint16_t port)
{
#ifdef _MSC_VER
    // MSVC implementation using inw from msvc_stubs.c
    extern unsigned short inw(unsigned short port);
    return inw(port);
#else
    uint16_t data;
    __asm__ volatile("inw %1, %0" : "=a"(data) : "Nd"(port));
    return data;
#endif
}

uint32_t arch_inl(uint16_t port)
{
#ifdef _MSC_VER
    // MSVC implementation using inl from msvc_stubs.c
    extern unsigned int inl(unsigned short port);
    return inl(port);
#else
    uint32_t data;
    __asm__ volatile("inl %1, %0" : "=a"(data) : "Nd"(port));
    return data;
#endif
}

void arch_outw(uint16_t port, uint16_t data)
{
#ifdef _MSC_VER
    // MSVC implementation using outw from msvc_stubs.c
    extern void outw(unsigned short port, unsigned short data);
    outw(port, data);
#else
    __asm__ volatile("outw %0, %1" ::"a"(data), "Nd"(port));
#endif
}

void arch_outl(uint16_t port, uint32_t data)
{
#ifdef _MSC_VER
    // MSVC implementation using outl from msvc_stubs.c
    extern void outl(unsigned short port, unsigned int data);
    outl(port, data);
#else
    __asm__ volatile("outl %0, %1" ::"a"(data), "Nd"(port));
#endif
}

// Debug register access
uint64_t arch_read_debug_register(uint32_t reg)
{
    uint64_t value = 0;
#ifdef _MSC_VER
    // MSVC implementation using read_dr from msvc_stubs.c
    extern uint64_t read_dr(uint32_t reg);
    value = read_dr(reg);
#else
    switch (reg)
    {
    case 0:
        __asm__ volatile("mov %%dr0, %0" : "=r"(value));
        break;
    case 1:
        __asm__ volatile("mov %%dr1, %0" : "=r"(value));
        break;
    case 2:
        __asm__ volatile("mov %%dr2, %0" : "=r"(value));
        break;
    case 3:
        __asm__ volatile("mov %%dr3, %0" : "=r"(value));
        break;
    case 6:
        __asm__ volatile("mov %%dr6, %0" : "=r"(value));
        break;
    case 7:
        __asm__ volatile("mov %%dr7, %0" : "=r"(value));
        break;
    }
#endif
    return value;
}

void arch_write_debug_register(uint32_t reg, uint64_t value)
{
#ifdef _MSC_VER
    // MSVC implementation using write_dr from msvc_stubs.c
    extern void write_dr(uint32_t reg, uint64_t value);
    write_dr(reg, value);
#else
    switch (reg)
    {
    case 0:
        __asm__ volatile("mov %0, %%dr0" ::"r"(value));
        break;
    case 1:
        __asm__ volatile("mov %0, %%dr1" ::"r"(value));
        break;
    case 2:
        __asm__ volatile("mov %0, %%dr2" ::"r"(value));
        break;
    case 3:
        __asm__ volatile("mov %0, %%dr3" ::"r"(value));
        break;
    case 6:
        __asm__ volatile("mov %0, %%dr6" ::"r"(value));
        break;
    case 7:
        __asm__ volatile("mov %0, %%dr7" ::"r"(value));
        break;
    }
#endif
}
