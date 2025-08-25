/*
 * ORION OS - POWER Architecture MMU Management
 *
 * Memory Management Unit implementation for POWER architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include <string.h>
#include <stdio.h>

// ============================================================================
// PAGE TABLE STRUCTURES
// ============================================================================

#define POWER_PAGE_SIZE_4K  0x1000
#define POWER_PAGE_SIZE_64K 0x10000
#define POWER_PAGE_SIZE_16M 0x1000000
#define POWER_PAGE_SIZE_1G  0x40000000

#define POWER_PAGE_PRESENT    0x001
#define POWER_PAGE_WRITE      0x002
#define POWER_PAGE_USER       0x004
#define POWER_PAGE_WRITETHROUGH 0x008
#define POWER_PAGE_CACHE      0x010
#define POWER_PAGE_ACCESSED   0x020
#define POWER_PAGE_DIRTY      0x040
#define POWER_PAGE_GUARD      0x080
#define POWER_PAGE_SPECIAL    0x100

typedef struct {
    uint64_t pte;      // Page table entry
    uint64_t flags;    // Additional flags
    uint64_t asid;     // Address space ID
} power_tlb_entry_t;

typedef struct {
    uint64_t base_addr;        // Base address of page table
    uint64_t size;             // Size of page table
    uint64_t levels;           // Number of page table levels
    uint64_t asid;             // Address space ID
    power_tlb_entry_t tlb[POWER_TLB_ENTRIES]; // TLB entries
    uint64_t tlb_used;         // Number of used TLB entries
} power_mmu_context_t;

static power_mmu_context_t power_mmu;

// ============================================================================
// MMU INITIALIZATION
// ============================================================================

int power_mmu_init(void)
{
    memset(&power_mmu, 0, sizeof(power_mmu));
    
    // Set up page table base
    power_mmu.base_addr = 0x1000;  // Example base address
    power_mmu.size = 0x1000000;    // 16MB page table
    power_mmu.levels = 3;          // 3-level page table
    power_mmu.asid = 0;
    power_mmu.tlb_used = 0;
    
    // Clear TLB entries
    for (int i = 0; i < POWER_TLB_ENTRIES; i++) {
        power_mmu.tlb[i].pte = 0;
        power_mmu.tlb[i].flags = 0;
        power_mmu.tlb[i].asid = 0;
    }
    
    // Set up SDR1 (Segment Descriptor Register 1)
    uint64_t sdr1 = (power_mmu.base_addr & 0xFFFFFFFFFFF00000ULL) | 
                    ((power_mmu.size - 1) & 0x00000000000FFFFFULL);
    __asm__ volatile("mtspr 25, %0" : : "r"(sdr1));
    
    // Set up initial page table
    return power_mmu_setup_page_table();
}

int power_mmu_setup_page_table(void)
{
    // Allocate and initialize page table
    // This would typically involve allocating memory and setting up page table entries
    
    // Set up identity mapping for low memory
    uint64_t low_mem_start = 0x0000000000000000ULL;
    uint64_t low_mem_end = 0x0000000000100000ULL;  // 1MB
    
    for (uint64_t addr = low_mem_start; addr < low_mem_end; addr += POWER_PAGE_SIZE_4K) {
        int ret = power_mmu_map_page(addr, addr, 
                                   POWER_PAGE_PRESENT | POWER_PAGE_WRITE | POWER_PAGE_USER);
        if (ret != 0) {
            return ret;
        }
    }
    
    // Set up kernel mapping
    uint64_t kernel_start = 0x0000000001000000ULL;
    uint64_t kernel_end = 0x0000000002000000ULL;   // 16MB
    
    for (uint64_t addr = kernel_start; addr < kernel_end; addr += POWER_PAGE_SIZE_4K) {
        int ret = power_mmu_map_page(addr, addr, 
                                   POWER_PAGE_PRESENT | POWER_PAGE_WRITE);
        if (ret != 0) {
            return ret;
        }
    }
    
    return 0;
}

// ============================================================================
// PAGE MAPPING
// ============================================================================

int power_mmu_map_page(power_vaddr_t vaddr, power_paddr_t paddr, uint64_t flags)
{
    // Check if page is already mapped
    power_paddr_t existing_paddr;
    uint64_t existing_flags;
    if (power_mmu_get_page(vaddr, &existing_paddr, &existing_flags) == 0) {
        // Page already mapped, unmap it first
        power_mmu_unmap_page(vaddr);
    }
    
    // Add to TLB if there's space
    if (power_mmu.tlb_used < POWER_TLB_ENTRIES) {
        int index = power_mmu.tlb_used++;
        power_mmu.tlb[index].pte = paddr | flags;
        power_mmu.tlb[index].flags = flags;
        power_mmu.tlb[index].asid = power_mmu.asid;
        
        // Invalidate TLB entry
        __asm__ volatile("tlbie %0" : : "r"(vaddr));
        
        return 0;
    }
    
    // TLB full, need to evict an entry
    // Simple LRU: evict the first entry
    power_mmu.tlb[0].pte = paddr | flags;
    power_mmu.tlb[0].flags = flags;
    power_mmu.tlb[0].asid = power_mmu.asid;
    
    // Invalidate TLB entry
    __asm__ volatile("tlbie %0" : : "r"(vaddr));
    
    return 0;
}

int power_mmu_unmap_page(power_vaddr_t vaddr)
{
    // Find and remove TLB entry
    for (int i = 0; i < power_mmu.tlb_used; i++) {
        if (power_mmu.tlb[i].pte != 0) {
            // Check if this entry matches the virtual address
            uint64_t entry_vaddr = power_mmu.tlb[i].pte & ~0xFFF;
            if (entry_vaddr == (vaddr & ~0xFFF)) {
                // Clear the entry
                power_mmu.tlb[i].pte = 0;
                power_mmu.tlb[i].flags = 0;
                power_mmu.tlb[i].asid = 0;
                
                // Invalidate TLB entry
                __asm__ volatile("tlbie %0" : : "r"(vaddr));
                
                // Compact TLB entries
                for (int j = i; j < power_mmu.tlb_used - 1; j++) {
                    power_mmu.tlb[j] = power_mmu.tlb[j + 1];
                }
                power_mmu.tlb_used--;
                
                return 0;
            }
        }
    }
    
    return -1; // Page not found
}

int power_mmu_get_page(power_vaddr_t vaddr, power_paddr_t *paddr, uint64_t *flags)
{
    if (!paddr || !flags) {
        return -1;
    }
    
    // Search TLB for the page
    for (int i = 0; i < power_mmu.tlb_used; i++) {
        if (power_mmu.tlb[i].pte != 0) {
            uint64_t entry_vaddr = power_mmu.tlb[i].pte & ~0xFFF;
            if (entry_vaddr == (vaddr & ~0xFFF)) {
                *paddr = power_mmu.tlb[i].pte & ~0xFFF;
                *flags = power_mmu.tlb[i].flags;
                return 0;
            }
        }
    }
    
    return -1; // Page not found
}

// ============================================================================
// TLB MANAGEMENT
// ============================================================================

int power_mmu_flush_tlb(void)
{
    // Flush all TLB entries
    for (int i = 0; i < power_mmu.tlb_used; i++) {
        power_mmu.tlb[i].pte = 0;
        power_mmu.tlb[i].flags = 0;
        power_mmu.tlb[i].asid = 0;
    }
    power_mmu.tlb_used = 0;
    
    // Invalidate all TLB entries using tlbie instruction
    __asm__ volatile("tlbie");
    
    return 0;
}

int power_mmu_flush_tlb_range(power_vaddr_t start, power_vaddr_t end)
{
    // Flush TLB entries in range
    for (int i = 0; i < power_mmu.tlb_used; i++) {
        if (power_mmu.tlb[i].pte != 0) {
            uint64_t entry_vaddr = power_mmu.tlb[i].pte & ~0xFFF;
            if (entry_vaddr >= start && entry_vaddr < end) {
                // Invalidate this TLB entry
                __asm__ volatile("tlbie %0" : : "r"(entry_vaddr));
                
                // Clear the entry
                power_mmu.tlb[i].pte = 0;
                power_mmu.tlb[i].flags = 0;
                power_mmu.tlb[i].asid = 0;
            }
        }
    }
    
    // Compact TLB entries
    int j = 0;
    for (int i = 0; i < power_mmu.tlb_used; i++) {
        if (power_mmu.tlb[i].pte != 0) {
            if (i != j) {
                power_mmu.tlb[j] = power_mmu.tlb[i];
            }
            j++;
        }
    }
    power_mmu.tlb_used = j;
    
    return 0;
}

int power_mmu_flush_tlb_asid(uint64_t asid)
{
    // Flush TLB entries for specific ASID
    for (int i = 0; i < power_mmu.tlb_used; i++) {
        if (power_mmu.tlb[i].asid == asid) {
            uint64_t entry_vaddr = power_mmu.tlb[i].pte & ~0xFFF;
            __asm__ volatile("tlbie %0" : : "r"(entry_vaddr));
            
            power_mmu.tlb[i].pte = 0;
            power_mmu.tlb[i].flags = 0;
            power_mmu.tlb[i].asid = 0;
        }
    }
    
    // Compact TLB entries
    int j = 0;
    for (int i = 0; i < power_mmu.tlb_used; i++) {
        if (power_mmu.tlb[i].pte != 0) {
            if (i != j) {
                power_mmu.tlb[j] = power_mmu.tlb[i];
            }
            j++;
        }
    }
    power_mmu.tlb_used = j;
    
    return 0;
}

// ============================================================================
// ASID MANAGEMENT
// ============================================================================

int power_mmu_set_asid(uint64_t asid)
{
    power_mmu.asid = asid;
    
    // Set ASID in SPR
    __asm__ volatile("mtspr 13, %0" : : "r"(asid));
    
    return 0;
}

int power_mmu_get_asid(uint64_t *asid)
{
    if (!asid) {
        return -1;
    }
    
    *asid = power_mmu.asid;
    return 0;
}

// ============================================================================
// PAGE PROTECTION
// ============================================================================

int power_mmu_protect_page(power_vaddr_t vaddr, uint64_t flags)
{
    // Find TLB entry and update flags
    for (int i = 0; i < power_mmu.tlb_used; i++) {
        if (power_mmu.tlb[i].pte != 0) {
            uint64_t entry_vaddr = power_mmu.tlb[i].pte & ~0xFFF;
            if (entry_vaddr == (vaddr & ~0xFFF)) {
                // Update flags
                power_mmu.tlb[i].flags = flags;
                power_mmu.tlb[i].pte = (power_mmu.tlb[i].pte & ~0xFFF) | flags;
                
                // Invalidate TLB entry
                __asm__ volatile("tlbie %0" : : "r"(vaddr));
                
                return 0;
            }
        }
    }
    
    return -1; // Page not found
}

int power_mmu_get_page_protection(power_vaddr_t vaddr, uint64_t *flags)
{
    if (!flags) {
        return -1;
    }
    
    // Search TLB for the page
    for (int i = 0; i < power_mmu.tlb_used; i++) {
        if (power_mmu.tlb[i].pte != 0) {
            uint64_t entry_vaddr = power_mmu.tlb[i].pte & ~0xFFF;
            if (entry_vaddr == (vaddr & ~0xFFF)) {
                *flags = power_mmu.tlb[i].flags;
                return 0;
            }
        }
    }
    
    return -1; // Page not found
}

// ============================================================================
// MEMORY ALLOCATION
// ============================================================================

int power_mmu_alloc_pages(uint64_t count, power_paddr_t *paddr)
{
    if (!paddr || count == 0) {
        return -1;
    }
    
    // Simple page allocator
    // In a real implementation, this would use a proper memory allocator
    
    static uint64_t next_page = 0x2000000; // Start after kernel
    
    *paddr = next_page;
    next_page += count * POWER_PAGE_SIZE_4K;
    
    return 0;
}

int power_mmu_free_pages(power_paddr_t paddr, uint64_t count)
{
    // Simple page deallocator
    // In a real implementation, this would return pages to the allocator
    
    (void)paddr;
    (void)count;
    
    return 0;
}

// ============================================================================
// MMU STATISTICS
// ============================================================================

int power_mmu_get_stats(power_mmu_stats_t *stats)
{
    if (!stats) {
        return -1;
    }
    
    stats->tlb_entries = POWER_TLB_ENTRIES;
    stats->tlb_used = power_mmu.tlb_used;
    stats->tlb_free = POWER_TLB_ENTRIES - power_mmu.tlb_used;
    stats->asid = power_mmu.asid;
    stats->base_addr = power_mmu.base_addr;
    stats->size = power_mmu.size;
    stats->levels = power_mmu.levels;
    
    return 0;
}

// ============================================================================
// MMU CLEANUP
// ============================================================================

void power_mmu_cleanup(void)
{
    // Flush all TLB entries
    power_mmu_flush_tlb();
    
    // Clear MMU context
    memset(&power_mmu, 0, sizeof(power_mmu));
}
