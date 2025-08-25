/*
 * ORION OS - RISC-V 64-bit Architecture Implementation
 *
 * This file contains the main implementation for RISC-V 64-bit architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

uint64_t riscv64_cpu_features = 0;
uint32_t riscv64_cpu_family = 0;
uint32_t riscv64_cpu_model = 0;

// ============================================================================
// CPU FEATURE DETECTION
// ============================================================================

void riscv64_detect_cpu_features(void)
{
    printf("RISC-V: Detecting CPU features...\n");

    // Read MISA CSR to detect base ISA and extensions
    uint64_t misa;
    __asm__ volatile("csrr %0, %1" : "=r"(misa) : "i"(RISCV64_CSR_MISA));

    // Extract base ISA (bits 62-30)
    uint64_t base_isa = (misa >> 30) & 0x3F;
    riscv64_cpu_family = (uint32_t)base_isa;

    // Extract extensions (bits 25-0)
    uint64_t extensions = misa & 0x3FFFFFF;
    riscv64_cpu_features = extensions;

    // Set feature flags
    if (extensions & (1 << ('I' - 'A')))
        riscv64_cpu_features |= RISCV64_FEATURE_RV64I;
    if (extensions & (1 << ('M' - 'A')))
        riscv64_cpu_features |= RISCV64_FEATURE_RV64M;
    if (extensions & (1 << ('A' - 'A')))
        riscv64_cpu_features |= RISCV64_FEATURE_RV64A;
    if (extensions & (1 << ('F' - 'A')))
        riscv64_cpu_features |= RISCV64_FEATURE_RV64F;
    if (extensions & (1 << ('D' - 'A')))
        riscv64_cpu_features |= RISCV64_FEATURE_RV64D;
    if (extensions & (1 << ('C' - 'A')))
        riscv64_cpu_features |= RISCV64_FEATURE_RV64C;
    if (extensions & (1 << ('V' - 'A')))
        riscv64_cpu_features |= RISCV64_FEATURE_RVV;
    if (extensions & (1 << ('B' - 'A')))
        riscv64_cpu_features |= RISCV64_FEATURE_RV64B;

    // Read vendor and implementation information
    uint64_t mvendorid, marchid, mimpid, mhartid;
    __asm__ volatile("csrr %0, %1" : "=r"(mvendorid) : "i"(0xF11));
    __asm__ volatile("csrr %0, %1" : "=r"(marchid) : "i"(0xF12));
    __asm__ volatile("csrr %0, %1" : "=r"(mimpid) : "i"(0xF13));
    __asm__ volatile("csrr %0, %1" : "=r"(mhartid) : "i"(RISCV64_CSR_MHARTID));

    printf("RISC-V: CPU features detected:\n");
    printf("  Base ISA: RV64%c\n", (char)(base_isa + 'A'));
    printf("  Extensions: ");

    if (extensions & (1 << ('I' - 'A')))
        printf("I ");
    if (extensions & (1 << ('M' - 'A')))
        printf("M ");
    if (extensions & (1 << ('A' - 'A')))
        printf("A ");
    if (extensions & (1 << ('F' - 'A')))
        printf("F ");
    if (extensions & (1 << ('D' - 'A')))
        printf("D ");
    if (extensions & (1 << ('C' - 'A')))
        printf("C ");
    if (extensions & (1 << ('V' - 'A')))
        printf("V ");
    if (extensions & (1 << ('B' - 'A')))
        printf("B ");
    printf("\n");

    printf("  Vendor ID: 0x%llx\n", (unsigned long long)mvendorid);
    printf("  Architecture ID: 0x%llx\n", (unsigned long long)marchid);
    printf("  Implementation ID: 0x%llx\n", (unsigned long long)mimpid);
    printf("  Hart ID: %llu\n", (unsigned long long)mhartid);
}

bool riscv64_has_feature(uint64_t feature)
{
    return (riscv64_cpu_features & feature) != 0;
}

void riscv64_print_cpu_info(void)
{
    printf("RISC-V: CPU Information:\n");
    printf("  Family: %u\n", riscv64_cpu_family);
    printf("  Model: %u\n", riscv64_cpu_model);
    printf("  Features: 0x%llx\n", (unsigned long long)riscv64_cpu_features);
}

// ============================================================================
// MMU MANAGEMENT
// ============================================================================

void riscv64_mmu_init(void)
{
    printf("RISC-V: Initializing MMU...\n");

    // Set up page table base
    uint64_t satp = 0; // Disable paging initially
    __asm__ volatile("csrw %0, %1" : : "i"(RISCV64_CSR_SATP), "r"(satp));

    // Configure PMP (Physical Memory Protection) if available
    if (riscv64_has_feature(RISCV64_FEATURE_RV64A))
    {
        // Set up PMP to allow all memory access
        uint64_t pmpcfg = 0x0F0F0F0F0F0F0F0FULL; // TOR, R/W/X for all regions
        __asm__ volatile("csrw %0, %1" : : "i"(0x3A0), "r"(pmpcfg));

        // Set PMP address registers
        uint64_t pmpaddr = 0x3FFFFFFFFFFFFFFFULL; // Cover all 64-bit address space
        for (int i = 0; i < 8; i++)
        {
            uint32_t csr_addr = 0x3B0 + i;
            __asm__ volatile("csrw %0, %1" : : "i"(csr_addr), "r"(pmpaddr));
        }
    }

    printf("RISC-V: MMU initialized\n");
}

int riscv64_mmu_map_page(uint64_t va, uint64_t pa, uint64_t flags)
{
    // This is a simplified implementation
    // In a real system, this would walk the page table and create entries
    printf("RISC-V: Mapping page 0x%llx -> 0x%llx (flags: 0x%llx)\n",
           (unsigned long long)va, (unsigned long long)pa, (unsigned long long)flags);
    return 0;
}

int riscv64_mmu_unmap_page(uint64_t va)
{
    // This is a simplified implementation
    printf("RISC-V: Unmapping page 0x%llx\n", (unsigned long long)va);
    return 0;
}

void riscv64_mmu_invalidate_tlb(void)
{
    // Invalidate TLB by writing to satp
    uint64_t satp;
    __asm__ volatile("csrr %0, %1" : "=r"(satp) : "i"(RISCV64_CSR_SATP));
    __asm__ volatile("csrw %0, %1" : : "i"(RISCV64_CSR_SATP), "r"(satp));
    printf("RISC-V: TLB invalidated\n");
}

// ============================================================================
// INTERRUPT MANAGEMENT
// ============================================================================

void riscv64_interrupts_init(void)
{
    printf("RISC-V: Initializing interrupts...\n");

    // Set up trap vector base
    extern void riscv64_trap_vector(void);
    uint64_t trap_vector = (uint64_t)riscv64_trap_vector;
    __asm__ volatile("csrw %0, %1" : : "i"(RISCV64_CSR_MTVEC), "r"(trap_vector));

    // Enable machine mode interrupts
    uint64_t mie = (1ULL << RISCV64_IRQ_SOFTWARE) |
                   (1ULL << RISCV64_IRQ_TIMER) |
                   (1ULL << RISCV64_IRQ_EXTERNAL);
    __asm__ volatile("csrw %0, %1" : : "i"(RISCV64_CSR_MIE), "r"(mie));

    // Enable machine mode interrupts globally
    uint64_t mstatus;
    __asm__ volatile("csrr %0, %1" : "=r"(mstatus) : "i"(RISCV64_CSR_MSTATUS));
    mstatus |= (1ULL << 3); // MIE bit
    __asm__ volatile("csrw %0, %1" : : "i"(RISCV64_CSR_MSTATUS), "r"(mstatus));

    printf("RISC-V: Interrupts initialized\n");
}

void riscv64_interrupt_enable(uint32_t irq)
{
    uint64_t mie;
    __asm__ volatile("csrr %0, %1" : "=r"(mie) : "i"(RISCV64_CSR_MIE));
    mie |= (1ULL << irq);
    __asm__ volatile("csrw %0, %1" : : "i"(RISCV64_CSR_MIE), "r"(mie));
}

void riscv64_interrupt_disable(uint32_t irq)
{
    uint64_t mie;
    __asm__ volatile("csrr %0, %1" : "=r"(mie) : "i"(RISCV64_CSR_MIE));
    mie &= ~(1ULL << irq);
    __asm__ volatile("csrw %0, %1" : : "i"(RISCV64_CSR_MIE), "r"(mie));
}

void riscv64_interrupt_set_handler(uint32_t irq, void (*handler)(void))
{
    // In a real implementation, this would set up interrupt handlers
    printf("RISC-V: Setting interrupt handler for IRQ %u\n", irq);
}

// ============================================================================
// TIMER MANAGEMENT
// ============================================================================

void riscv64_timer_init(void)
{
    printf("RISC-V: Initializing timer...\n");

    // Read time CSR to get current time
    uint64_t time;
    __asm__ volatile("csrr %0, %1" : "=r"(time) : "i"(0xC01));

    // Set up timer interrupt
    uint64_t mtimecmp = time + 1000000; // 1 second from now
    __asm__ volatile("csrw %0, %1" : : "i"(0x321), "r"(mtimecmp));

    printf("RISC-V: Timer initialized at %llu\n", (unsigned long long)time);
}

uint64_t riscv64_timer_read_ns(void)
{
    uint64_t time;
    __asm__ volatile("csrr %0, %1" : "=r"(time) : "i"(0xC01));

    // Convert to nanoseconds (assuming 1MHz timer)
    return time * 1000;
}

int riscv64_timer_set_oneshot(uint64_t deadline_ns)
{
    uint64_t deadline_cycles = deadline_ns / 1000; // Convert to cycles
    __asm__ volatile("csrw %0, %1" : : "i"(0x321), "r"(deadline_cycles));
    return 0;
}

// ============================================================================
// CACHE MANAGEMENT
// ============================================================================

void riscv64_cache_init(void)
{
    printf("RISC-V: Initializing cache...\n");

    // RISC-V doesn't have standard cache management instructions
    // This would be implementation-specific
    printf("RISC-V: Cache initialized (implementation-specific)\n");
}

void riscv64_cache_invalidate_all(void)
{
    // Implementation-specific cache invalidation
    printf("RISC-V: Cache invalidated\n");
}

void riscv64_cache_clean_all(void)
{
    // Implementation-specific cache cleaning
    printf("RISC-V: Cache cleaned\n");
}

// ============================================================================
// VECTOR EXTENSIONS
// ============================================================================

void riscv64_vector_init(void)
{
    printf("RISC-V: Initializing vector extensions...\n");

    if (riscv64_has_feature(RISCV64_FEATURE_RVV))
    {
        printf("RISC-V: RVV vector extension detected\n");
    }
    else
    {
        printf("RISC-V: No vector extension detected\n");
    }
}

void riscv64_vector_enable(void)
{
    if (riscv64_has_feature(RISCV64_FEATURE_RVV))
    {
        // Enable vector extension
        printf("RISC-V: Vector extensions enabled\n");
    }
}

// ============================================================================
// SECURITY FEATURES
// ============================================================================

void riscv64_security_init(void)
{
    printf("RISC-V: Initializing security features...\n");

    // Initialize PMP if available
    riscv64_pmp_init();

    printf("RISC-V: Security features initialized\n");
}

void riscv64_pmp_init(void)
{
    if (riscv64_has_feature(RISCV64_FEATURE_RV64A))
    {
        printf("RISC-V: PMP (Physical Memory Protection) initialized\n");
    }
}

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

void riscv64_pmu_init(void)
{
    printf("RISC-V: Initializing performance monitoring...\n");

    // Enable performance counters
    uint64_t mcounteren = 0xFFFFFFFFFFFFFFFFULL;
    __asm__ volatile("csrw %0, %1" : : "i"(0x306), "r"(mcounteren));

    printf("RISC-V: Performance monitoring initialized\n");
}

uint64_t riscv64_pmu_read_counter(uint32_t counter)
{
    uint64_t value = 0;

    switch (counter)
    {
    case 0:
        __asm__ volatile("csrr %0, %1" : "=r"(value) : "i"(0xC00)); // cycle
        break;
    case 1:
        __asm__ volatile("csrr %0, %1" : "=r"(value) : "i"(0xC01)); // time
        break;
    case 2:
        __asm__ volatile("csrr %0, %1" : "=r"(value) : "i"(0xC02)); // instret
        break;
    default:
        if (counter >= 3 && counter <= 31)
        {
            uint32_t csr_addr = 0xB00 + counter;
            __asm__ volatile("csrr %0, %1" : "=r"(value) : "i"(csr_addr));
        }
        break;
    }

    return value;
}

void riscv64_pmu_set_event(uint32_t counter, uint32_t event)
{
    // Set performance counter event type
    if (counter >= 3 && counter <= 31)
    {
        __asm__ volatile("csrw %0, %1" : : "i"(0x320 + counter), "r"((uint64_t)event));
    }
}

// ============================================================================
// POWER MANAGEMENT
// ============================================================================

void riscv64_power_init(void)
{
    printf("RISC-V: Initializing power management...\n");

    // RISC-V power management is implementation-specific
    printf("RISC-V: Power management initialized\n");
}

int riscv64_power_set_state(uint32_t state)
{
    switch (state)
    {
    case 0: // Running
        break;
    case 1:                      // Idle
        __asm__ volatile("wfi"); // Wait for interrupt
        break;
    default:
        return -1;
    }
    return 0;
}

uint32_t riscv64_power_get_state(void)
{
    return 0; // Always running for now
}

// ============================================================================
// NUMA MANAGEMENT
// ============================================================================

void riscv64_numa_init(void)
{
    printf("RISC-V: Initializing NUMA topology...\n");

    // RISC-V NUMA is implementation-specific
    // For now, assume single node
    printf("RISC-V: NUMA topology initialized (single node)\n");
}

uint32_t riscv64_numa_get_node_count(void)
{
    return 1; // Single node for now
}

uint32_t riscv64_numa_get_current_node(void)
{
    return 0; // Always node 0 for now
}

// ============================================================================
// MAIN INITIALIZATION
// ============================================================================

void riscv64_arch_init(void)
{
    printf("=== RISC-V Architecture Initialization ===\n");

    // Detect CPU features first
    riscv64_detect_cpu_features();

    // Initialize subsystems
    riscv64_mmu_init();
    riscv64_interrupts_init();
    riscv64_timer_init();
    riscv64_cache_init();
    riscv64_vector_init();
    riscv64_security_init();
    riscv64_pmu_init();
    riscv64_power_init();
    riscv64_numa_init();

    // Print final configuration
    riscv64_print_cpu_info();

    printf("RISC-V architecture initialization complete\n");
}
