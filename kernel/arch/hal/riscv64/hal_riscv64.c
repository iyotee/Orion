/*
 * ORION OS - RISC-V 64-bit Hardware Abstraction Layer
 *
 * This file implements the hardware abstraction layer for RISC-V 64-bit
 * architecture, providing a unified interface for hardware operations.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "hal_riscv64.h"
#include "orion_hal.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

static bool hal_initialized = false;
static riscv64_hal_context_t hal_context;

// ============================================================================
// HAL INITIALIZATION
// ============================================================================

int riscv64_hal_init(void)
{
    if (hal_initialized)
    {
        return 0;
    }

    printf("RISC-V64 HAL: Initializing hardware abstraction layer...\n");

    // Initialize HAL context
    memset(&hal_context, 0, sizeof(hal_context));

    // Detect CPU features
    riscv64_hal_detect_cpu_features();

    // Initialize memory management
    if (riscv64_hal_mmu_init() != 0)
    {
        printf("RISC-V64 HAL: Failed to initialize MMU\n");
        return -1;
    }

    // Initialize interrupt controller
    if (riscv64_hal_interrupt_init() != 0)
    {
        printf("RISC-V64 HAL: Failed to initialize interrupts\n");
        return -1;
    }

    // Initialize timer subsystem
    if (riscv64_hal_timer_init() != 0)
    {
        printf("RISC-V64 HAL: Failed to initialize timers\n");
        return -1;
    }

    // Initialize cache subsystem
    if (riscv64_hal_cache_init() != 0)
    {
        printf("RISC-V64 HAL: Failed to initialize cache\n");
        return -1;
    }

    hal_initialized = true;
    printf("RISC-V64 HAL: Hardware abstraction layer initialized successfully\n");

    return 0;
}

void riscv64_hal_cleanup(void)
{
    if (!hal_initialized)
    {
        return;
    }

    printf("RISC-V64 HAL: Cleaning up hardware abstraction layer...\n");

    // Cleanup subsystems
    riscv64_hal_timer_cleanup();
    riscv64_hal_cache_cleanup();
    riscv64_hal_interrupt_cleanup();
    riscv64_hal_mmu_cleanup();

    hal_initialized = false;
    printf("RISC-V64 HAL: Hardware abstraction layer cleaned up\n");
}

// ============================================================================
// CPU FEATURE DETECTION
// ============================================================================

void riscv64_hal_detect_cpu_features(void)
{
    printf("RISC-V64 HAL: Detecting CPU features...\n");

    // Read MISA CSR to detect base ISA and extensions
    uint64_t misa;
    __asm__ volatile("csrr %0, %1" : "=r"(misa) : "i"(0x301));

    // Extract base ISA (bits 62-30)
    uint64_t base_isa = (misa >> 30) & 0x3F;
    hal_context.cpu_features.base_isa = base_isa;

    // Extract extensions (bits 25-0)
    uint64_t extensions = misa & 0x3FFFFFF;
    hal_context.cpu_features.extensions = extensions;

    // Read vendor and implementation information
    __asm__ volatile("csrr %0, %1" : "=r"(hal_context.cpu_features.vendor_id) : "i"(0xF11));
    __asm__ volatile("csrr %0, %1" : "=r"(hal_context.cpu_features.architecture_id) : "i"(0xF12));
    __asm__ volatile("csrr %0, %1" : "=r"(hal_context.cpu_features.implementation_id) : "i"(0xF13));
    __asm__ volatile("csrr %0, %1" : "=r"(hal_context.cpu_features.hart_id) : "i"(0xF14));

    // Set feature flags
    hal_context.cpu_features.features = 0;
    if (extensions & (1 << ('I' - 'A')))
        hal_context.cpu_features.features |= RISCV64_FEATURE_RV64I;
    if (extensions & (1 << ('M' - 'A')))
        hal_context.cpu_features.features |= RISCV64_FEATURE_RV64M;
    if (extensions & (1 << ('A' - 'A')))
        hal_context.cpu_features.features |= RISCV64_FEATURE_RV64A;
    if (extensions & (1 << ('F' - 'A')))
        hal_context.cpu_features.features |= RISCV64_FEATURE_RV64F;
    if (extensions & (1 << ('D' - 'A')))
        hal_context.cpu_features.features |= RISCV64_FEATURE_RV64D;
    if (extensions & (1 << ('C' - 'A')))
        hal_context.cpu_features.features |= RISCV64_FEATURE_RV64C;
    if (extensions & (1 << ('V' - 'A')))
        hal_context.cpu_features.features |= RISCV64_FEATURE_RVV;
    if (extensions & (1 << ('B' - 'A')))
        hal_context.cpu_features.features |= RISCV64_FEATURE_RV64B;

    printf("RISC-V64 HAL: CPU features detected - Base: RV64%c, Extensions: 0x%llx\n",
           (char)(base_isa + 'A'), (unsigned long long)extensions);
}

bool riscv64_hal_has_feature(uint64_t feature)
{
    return (hal_context.cpu_features.features & feature) != 0;
}

// ============================================================================
// MEMORY MANAGEMENT
// ============================================================================

int riscv64_hal_mmu_init(void)
{
    printf("RISC-V64 HAL: Initializing MMU...\n");

    // Initialize page table
    hal_context.mmu.root_page_table = (uint64_t *)0x8000000000000000ULL;
    hal_context.mmu.asid = 0;
    hal_context.mmu.satp = 0;

    // Set up initial page table structure
    memset(hal_context.mmu.root_page_table, 0, RISCV64_PAGE_SIZE);

    // Identity map kernel space
    uint64_t kernel_base = 0x8000000000000000ULL;
    uint64_t kernel_size = 0x400000000ULL; // 16GB

    for (uint64_t addr = kernel_base; addr < kernel_base + kernel_size; addr += RISCV64_PAGE_SIZE)
    {
        riscv64_hal_mmu_map_page(addr, addr, RISCV64_PAGE_READ | RISCV64_PAGE_WRITE | RISCV64_PAGE_EXECUTE);
    }

    // Enable MMU
    __asm__ volatile("csrw satp, %0" : : "r"(hal_context.mmu.satp));
    __asm__ volatile("sfence.vma");

    printf("RISC-V64 HAL: MMU initialized successfully\n");
    return 0;
}

void riscv64_hal_mmu_cleanup(void)
{
    printf("RISC-V64 HAL: Cleaning up MMU...\n");

    // Disable MMU
    __asm__ volatile("csrw satp, zero");
    __asm__ volatile("sfence.vma");

    hal_context.mmu.root_page_table = NULL;
    hal_context.mmu.satp = 0;
}

int riscv64_hal_mmu_map_page(uint64_t va, uint64_t pa, uint64_t flags)
{
    // Simple identity mapping for now
    // In a full implementation, this would walk the page table hierarchy

    printf("RISC-V64 HAL: Mapping page VA: 0x%llx -> PA: 0x%llx, flags: 0x%llx\n",
           (unsigned long long)va, (unsigned long long)pa, (unsigned long long)flags);

    return 0;
}

int riscv64_hal_mmu_unmap_page(uint64_t va)
{
    printf("RISC-V64 HAL: Unmapping page VA: 0x%llx\n", (unsigned long long)va);

    // Invalidate TLB entry
    __asm__ volatile("sfence.vma");

    return 0;
}

void riscv64_hal_mmu_invalidate_tlb(void)
{
    __asm__ volatile("sfence.vma");
}

// ============================================================================
// INTERRUPT MANAGEMENT
// ============================================================================

int riscv64_hal_interrupt_init(void)
{
    printf("RISC-V64 HAL: Initializing interrupt system...\n");

    // Set up interrupt vector table
    uint64_t mtvec;
    __asm__ volatile("csrr %0, mtvec" : "=r"(mtvec));

    // Set up interrupt enable register
    uint64_t mie = (1 << 7) | (1 << 3) | (1 << 1); // Timer, software, external
    __asm__ volatile("csrw mie, %0" : : "r"(mie));

    // Enable global interrupts
    uint64_t mstatus;
    __asm__ volatile("csrr %0, mstatus" : "=r"(mstatus));
    mstatus |= (1 << 3); // MIE bit
    __asm__ volatile("csrw mstatus, %0" : : "r"(mstatus));

    hal_context.interrupts.initialized = true;
    printf("RISC-V64 HAL: Interrupt system initialized successfully\n");

    return 0;
}

void riscv64_hal_interrupt_cleanup(void)
{
    printf("RISC-V64 HAL: Cleaning up interrupt system...\n");

    // Disable all interrupts
    __asm__ volatile("csrw mie, zero");

    // Disable global interrupts
    uint64_t mstatus;
    __asm__ volatile("csrr %0, mstatus" : "=r"(mstatus));
    mstatus &= ~(1 << 3); // Clear MIE bit
    __asm__ volatile("csrw mstatus, %0" : : "r"(mstatus));

    hal_context.interrupts.initialized = false;
}

void riscv64_hal_interrupt_enable(uint32_t irq)
{
    if (!hal_context.interrupts.initialized)
    {
        return;
    }

    uint64_t mie;
    __asm__ volatile("csrr %0, mie" : "=r"(mie));
    mie |= (1ULL << irq);
    __asm__ volatile("csrw mie, %0" : : "r"(mie));
}

void riscv64_hal_interrupt_disable(uint32_t irq)
{
    if (!hal_context.interrupts.initialized)
    {
        return;
    }

    uint64_t mie;
    __asm__ volatile("csrr %0, mie" : "=r"(mie));
    mie &= ~(1ULL << irq);
    __asm__ volatile("csrw mie, %0" : : "r"(mie));
}

// ============================================================================
// TIMER MANAGEMENT
// ============================================================================

int riscv64_hal_timer_init(void)
{
    printf("RISC-V64 HAL: Initializing timer system...\n");

    // Get timer frequency (assume 10MHz for now)
    hal_context.timer.frequency = 10000000;

    // Set up timer compare register
    uint64_t current_time = riscv64_hal_timer_read_ns();
    uint64_t compare_time = current_time + (1000000000ULL / 100); // 10ms

    __asm__ volatile("csrw mtimecmp, %0" : : "r"(compare_time));

    // Enable timer interrupt
    riscv64_hal_interrupt_enable(7); // Timer interrupt

    hal_context.timer.initialized = true;
    printf("RISC-V64 HAL: Timer system initialized successfully\n");

    return 0;
}

void riscv64_hal_timer_cleanup(void)
{
    printf("RISC-V64 HAL: Cleaning up timer system...\n");

    // Disable timer interrupt
    riscv64_hal_interrupt_disable(7);

    hal_context.timer.initialized = false;
}

uint64_t riscv64_hal_timer_read_ns(void)
{
    uint64_t time;
    __asm__ volatile("csrr %0, time" : "=r"(time));

    // Convert to nanoseconds (assuming 10MHz clock)
    return time * 100;
}

int riscv64_hal_timer_set_oneshot(uint64_t deadline_ns)
{
    if (!hal_context.timer.initialized)
    {
        return -1;
    }

    uint64_t current_time = riscv64_hal_timer_read_ns();
    uint64_t compare_time = current_time + deadline_ns;

    __asm__ volatile("csrw mtimecmp, %0" : : "r"(compare_time));

    return 0;
}

// ============================================================================
// CACHE MANAGEMENT
// ============================================================================

int riscv64_hal_cache_init(void)
{
    printf("RISC-V64 HAL: Initializing cache system...\n");

    // RISC-V doesn't have explicit cache control instructions in the base ISA
    // Cache management is typically handled by the platform or through custom instructions

    hal_context.cache.initialized = true;
    printf("RISC-V64 HAL: Cache system initialized successfully\n");

    return 0;
}

void riscv64_hal_cache_cleanup(void)
{
    printf("RISC-V64 HAL: Cleaning up cache system...\n");
    hal_context.cache.initialized = false;
}

void riscv64_hal_cache_invalidate_all(void)
{
    // Invalidate all caches
    // This is platform-specific and may require custom instructions
    printf("RISC-V64 HAL: Invalidating all caches\n");
}

void riscv64_hal_cache_clean_all(void)
{
    // Clean all caches
    // This is platform-specific and may require custom instructions
    printf("RISC-V64 HAL: Cleaning all caches\n");
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void riscv64_hal_print_info(void)
{
    printf("RISC-V64 HAL: Hardware Abstraction Layer Information\n");
    printf("  Initialized: %s\n", hal_initialized ? "Yes" : "No");
    printf("  CPU Features: 0x%llx\n", (unsigned long long)hal_context.cpu_features.features);
    printf("  MMU Active: %s\n", hal_context.mmu.root_page_table ? "Yes" : "No");
    printf("  Interrupts: %s\n", hal_context.interrupts.initialized ? "Yes" : "No");
    printf("  Timer: %s\n", hal_context.timer.initialized ? "Yes" : "No");
    printf("  Cache: %s\n", hal_context.cache.initialized ? "Yes" : "No");
}

const riscv64_hal_context_t *riscv64_hal_get_context(void)
{
    return &hal_context;
}

// ============================================================================
// HAL INTERFACE IMPLEMENTATION
// ============================================================================

// Implement the generic HAL interface
static const hal_ops_t riscv64_hal_ops = {
    .init = (hal_init_fn)riscv64_hal_init,
    .cleanup = (hal_cleanup_fn)riscv64_hal_cleanup,
    .mmu_init = (hal_mmu_init_fn)riscv64_hal_mmu_init,
    .mmu_cleanup = (hal_mmu_cleanup_fn)riscv64_hal_mmu_cleanup,
    .interrupt_init = (hal_interrupt_init_fn)riscv64_hal_interrupt_init,
    .interrupt_cleanup = (hal_interrupt_cleanup_fn)riscv64_hal_interrupt_cleanup,
    .timer_init = (hal_timer_init_fn)riscv64_hal_timer_init,
    .timer_cleanup = (hal_timer_cleanup_fn)riscv64_hal_timer_cleanup,
    .cache_init = (hal_cache_init_fn)riscv64_hal_cache_init,
    .cache_cleanup = (hal_cache_cleanup_fn)riscv64_hal_cache_cleanup,
};

const hal_ops_t *riscv64_hal_get_ops(void)
{
    return &riscv64_hal_ops;
}
