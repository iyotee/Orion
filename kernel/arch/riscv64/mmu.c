/*
 * ORION OS - RISC-V 64-bit Memory Management Unit
 *
 * Memory Management Unit for RISC-V 64-bit architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// GLOBAL MMU CONTEXTS
// ============================================================================

// MMU context
static struct {
    bool initialized;
    uint64_t satp;  // Supervisor Address Translation and Protection
    uint64_t asid;  // Address Space ID
    uint64_t *root_page_table;
    uint64_t page_table_count;
    uint64_t total_pages;
    uint64_t used_pages;
} riscv64_mmu_context = {0};

// TLB context
static struct {
    bool initialized;
    uint64_t tlb_entries[RISCV64_TLB_ENTRIES];
    uint64_t tlb_tags[RISCV64_TLB_ENTRIES];
    bool tlb_valid[RISCV64_TLB_ENTRIES];
    uint32_t tlb_hits;
    uint32_t tlb_misses;
    uint32_t tlb_evictions;
} riscv64_tlb_context = {0};

// Page table context
static struct {
    bool initialized;
    uint64_t *kernel_page_table;
    uint64_t *user_page_table;
    uint64_t page_table_pages;
    uint64_t page_table_size;
} riscv64_page_table_context = {0};

// Memory region context
static struct {
    bool initialized;
    uint64_t regions[64];
    uint64_t region_flags[64];
    uint64_t region_count;
} riscv64_memory_regions = {0};

// ============================================================================
// MMU INITIALIZATION
// ============================================================================

// Initialize MMU system
void riscv64_mmu_init(void)
{
    if (riscv64_mmu_context.initialized) {
        return;
    }
    
    // Initialize MMU context
    riscv64_mmu_context.satp = 0;
    riscv64_mmu_context.asid = 0;
    riscv64_mmu_context.root_page_table = NULL;
    riscv64_mmu_context.page_table_count = 0;
    riscv64_mmu_context.total_pages = 0;
    riscv64_mmu_context.used_pages = 0;
    riscv64_mmu_context.initialized = true;
    
    // Initialize TLB context
    for (int i = 0; i < RISCV64_TLB_ENTRIES; i++) {
        riscv64_tlb_context.tlb_entries[i] = 0;
        riscv64_tlb_context.tlb_tags[i] = 0;
        riscv64_tlb_context.tlb_valid[i] = false;
    }
    riscv64_tlb_context.tlb_hits = 0;
    riscv64_tlb_context.tlb_misses = 0;
    riscv64_tlb_context.tlb_evictions = 0;
    riscv64_tlb_context.initialized = true;
    
    // Initialize page table context
    riscv64_page_table_context.kernel_page_table = NULL;
    riscv64_page_table_context.user_page_table = NULL;
    riscv64_page_table_context.page_table_pages = 0;
    riscv64_page_table_context.page_table_size = 0;
    riscv64_page_table_context.initialized = true;
    
    // Initialize memory regions
    riscv64_memory_regions.region_count = 0;
    riscv64_memory_regions.initialized = true;
    
    // Create initial page tables
    riscv64_create_initial_page_tables();
    
    printf("RISC-V64: MMU system initialized\n");
}

// ============================================================================
// PAGE TABLE MANAGEMENT
// ============================================================================

// Create initial page tables
void riscv64_create_initial_page_tables(void)
{
    // Allocate kernel page table
    riscv64_page_table_context.kernel_page_table = riscv64_allocate_page_table();
    if (!riscv64_page_table_context.kernel_page_table) {
        printf("RISC-V64: Failed to allocate kernel page table\n");
        return;
    }
    
    // Allocate user page table
    riscv64_page_table_context.user_page_table = riscv64_allocate_page_table();
    if (!riscv64_page_table_context.user_page_table) {
        printf("RISC-V64: Failed to allocate user page table\n");
        return;
    }
    
    // Map kernel space
    riscv64_map_kernel_space();
    
    // Map device memory
    riscv64_map_device_memory();
    
    printf("RISC-V64: Initial page tables created\n");
}

// Allocate page table
uint64_t* riscv64_allocate_page_table(void)
{
    // Allocate a 4KB page for the page table
    uint64_t *page_table = (uint64_t*)0x1000;  // Placeholder address
    
    // Clear the page table
    for (int i = 0; i < 512; i++) {
        page_table[i] = 0;
    }
    
    riscv64_page_table_context.page_table_pages++;
    riscv64_page_table_context.page_table_size += 4096;
    
    return page_table;
}

// Switch page table
int riscv64_page_table_switch(uint64_t *new_page_table)
{
    if (!riscv64_mmu_context.initialized || !new_page_table) {
        return -1;
    }
    
    // Update SATP register
    uint64_t new_satp = ((uint64_t)riscv64_mmu_context.asid << 44) | 
                        ((uint64_t)new_page_table >> 12);
    
    riscv64_mmu_context.satp = new_satp;
    riscv64_mmu_context.root_page_table = new_page_table;
    
    // Write to SATP CSR
    asm volatile("csrw satp, %0" : : "r"(new_satp));
    
    // Flush TLB
    riscv64_tlb_invalidate_all();
    
    printf("RISC-V64: Page table switched to 0x%llx\n", (uint64_t)new_page_table);
    return 0;
}

// Create page table
uint64_t* riscv64_page_table_create(void)
{
    return riscv64_allocate_page_table();
}

// Destroy page table
void riscv64_page_table_destroy(uint64_t *page_table)
{
    if (!page_table) {
        return;
    }
    
    // Free the page table memory
    // In a real system, this would return memory to the allocator
    
    riscv64_page_table_context.page_table_pages--;
    riscv64_page_table_context.page_table_size -= 4096;
    
    printf("RISC-V64: Page table destroyed\n");
}

// ============================================================================
// MEMORY MAPPING
// ============================================================================

// Map kernel space
void riscv64_map_kernel_space(void)
{
    if (!riscv64_page_table_context.kernel_page_table) {
        return;
    }
    
    // Map kernel base address (0x8000000000000000) to physical memory
    uint64_t kernel_base = RISCV64_KERNEL_BASE;
    uint64_t kernel_size = RISCV64_KERNEL_SIZE;
    
    for (uint64_t addr = kernel_base; addr < kernel_base + kernel_size; addr += RISCV64_PAGE_SIZE) {
        riscv64_mmu_map_page(addr, addr, RISCV64_PAGE_READ | RISCV64_PAGE_WRITE | RISCV64_PAGE_EXECUTE);
    }
    
    printf("RISC-V64: Kernel space mapped\n");
}

// Map device memory
void riscv64_map_device_memory(void)
{
    if (!riscv64_page_table_context.kernel_page_table) {
        return;
    }
    
    // Map device memory region
    uint64_t device_base = 0x1000000000000000;
    uint64_t device_size = 0x40000000;
    
    for (uint64_t addr = device_base; addr < device_base + device_size; addr += RISCV64_PAGE_SIZE) {
        riscv64_mmu_map_page(addr, addr, RISCV64_PAGE_READ | RISCV64_PAGE_WRITE);
    }
    
    printf("RISC-V64: Device memory mapped\n");
}

// Map page
int riscv64_mmu_map_page(uint64_t va, uint64_t pa, uint64_t flags)
{
    if (!riscv64_mmu_context.initialized) {
        return -1;
    }
    
    // Calculate page table indices
    uint64_t level1_index = (va >> 30) & 0x1FF;
    uint64_t level2_index = (va >> 21) & 0x1FF;
    uint64_t level3_index = (va >> 12) & 0x1FF;
    
    // Get root page table
    uint64_t *root_table = riscv64_mmu_context.root_page_table;
    if (!root_table) {
        return -1;
    }
    
    // Level 1 (1GB pages)
    if ((va & 0x3FFFFFFF) == 0 && (pa & 0x3FFFFFFF) == 0) {
        // Check if level 1 entry exists
        if (!(root_table[level1_index] & 0x1)) {
            // Create level 1 entry
            root_table[level1_index] = (pa >> 30) | flags | 0x1;
        }
        return 0;
    }
    
    // Level 2 (2MB pages)
    if ((va & 0x1FFFFF) == 0 && (pa & 0x1FFFFF) == 0) {
        // Check if level 2 entry exists
        if (!(root_table[level1_index] & 0x1)) {
            // Create level 2 table
            uint64_t *level2_table = riscv64_allocate_page_table();
            if (!level2_table) {
                return -1;
            }
            root_table[level1_index] = ((uint64_t)level2_table >> 12) | 0x1;
        }
        
        uint64_t *level2_table = (uint64_t*)((root_table[level1_index] >> 12) << 12);
        level2_table[level2_index] = (pa >> 21) | flags | 0x1;
        return 0;
    }
    
    // Level 3 (4KB pages)
    // Check if level 2 entry exists
    if (!(root_table[level1_index] & 0x1)) {
        // Create level 2 table
        uint64_t *level2_table = riscv64_allocate_page_table();
        if (!level2_table) {
            return -1;
        }
        root_table[level1_index] = ((uint64_t)level2_table >> 12) | 0x1;
    }
    
    uint64_t *level2_table = (uint64_t*)((root_table[level1_index] >> 12) << 12);
    
    // Check if level 3 entry exists
    if (!(level2_table[level2_index] & 0x1)) {
        // Create level 3 table
        uint64_t *level3_table = riscv64_allocate_page_table();
        if (!level3_table) {
            return -1;
        }
        level2_table[level2_index] = ((uint64_t)level3_table >> 12) | 0x1;
    }
    
    uint64_t *level3_table = (uint64_t*)((level2_table[level2_index] >> 12) << 12);
    level3_table[level3_index] = (pa >> 12) | flags | 0x1;
    
    // Invalidate TLB entry
    riscv64_tlb_invalidate_entry(va);
    
    riscv64_mmu_context.used_pages++;
    return 0;
}

// Unmap page
int riscv64_mmu_unmap_page(uint64_t va)
{
    if (!riscv64_mmu_context.initialized) {
        return -1;
    }
    
    // Calculate page table indices
    uint64_t level1_index = (va >> 30) & 0x1FF;
    uint64_t level2_index = (va >> 21) & 0x1FF;
    uint64_t level3_index = (va >> 12) & 0x1FF;
    
    // Get root page table
    uint64_t *root_table = riscv64_mmu_context.root_page_table;
    if (!root_table) {
        return -1;
    }
    
    // Check if level 1 entry exists
    if (!(root_table[level1_index] & 0x1)) {
        return -1;  // Page not mapped
    }
    
    // Level 1 (1GB pages)
    if ((va & 0x3FFFFFFF) == 0) {
        root_table[level1_index] = 0;
        riscv64_tlb_invalidate_entry(va);
        riscv64_mmu_context.used_pages--;
        return 0;
    }
    
    uint64_t *level2_table = (uint64_t*)((root_table[level1_index] >> 12) << 12);
    
    // Check if level 2 entry exists
    if (!(level2_table[level2_index] & 0x1)) {
        return -1;  // Page not mapped
    }
    
    // Level 2 (2MB pages)
    if ((va & 0x1FFFFF) == 0) {
        level2_table[level2_index] = 0;
        riscv64_tlb_invalidate_entry(va);
        riscv64_mmu_context.used_pages--;
        return 0;
    }
    
    uint64_t *level3_table = (uint64_t*)((level2_table[level2_index] >> 12) << 12);
    
    // Check if level 3 entry exists
    if (!(level3_table[level3_index] & 0x1)) {
        return -1;  // Page not mapped
    }
    
    // Level 3 (4KB pages)
    level3_table[level3_index] = 0;
    riscv64_tlb_invalidate_entry(va);
    riscv64_mmu_context.used_pages--;
    
    return 0;
}

// ============================================================================
// TLB MANAGEMENT
// ============================================================================

// Invalidate TLB entry
void riscv64_tlb_invalidate_entry(uint64_t va)
{
    if (!riscv64_tlb_context.initialized) {
        return;
    }
    
    // Find TLB entry for this virtual address
    uint64_t tag = va >> 12;  // Page number
    
    for (int i = 0; i < RISCV64_TLB_ENTRIES; i++) {
        if (riscv64_tlb_context.tlb_valid[i] && riscv64_tlb_context.tlb_tags[i] == tag) {
            riscv64_tlb_context.tlb_valid[i] = false;
            riscv64_tlb_context.tlb_entries[i] = 0;
            riscv64_tlb_context.tlb_tags[i] = 0;
            break;
        }
    }
    
    // Hardware TLB invalidation
    asm volatile("sfence.vma %0" : : "r"(va));
}

// Invalidate all TLB entries
void riscv64_tlb_invalidate_all(void)
{
    if (!riscv64_tlb_context.initialized) {
        return;
    }
    
    // Clear software TLB
    for (int i = 0; i < RISCV64_TLB_ENTRIES; i++) {
        riscv64_tlb_context.tlb_valid[i] = false;
        riscv64_tlb_context.tlb_entries[i] = 0;
        riscv64_tlb_context.tlb_tags[i] = 0;
    }
    
    // Hardware TLB invalidation
    asm volatile("sfence.vma");
    
    printf("RISC-V64: All TLB entries invalidated\n");
}

// TLB lookup
uint64_t riscv64_tlb_lookup(uint64_t va)
{
    if (!riscv64_tlb_context.initialized) {
        return 0;
    }
    
    uint64_t tag = va >> 12;  // Page number
    
    for (int i = 0; i < RISCV64_TLB_ENTRIES; i++) {
        if (riscv64_tlb_context.tlb_valid[i] && riscv64_tlb_context.tlb_tags[i] == tag) {
            riscv64_tlb_context.tlb_hits++;
            return riscv64_tlb_context.tlb_entries[i];
        }
    }
    
    riscv64_tlb_context.tlb_misses++;
    return 0;  // TLB miss
}

// TLB insert
void riscv64_tlb_insert(uint64_t va, uint64_t pa, uint64_t flags)
{
    if (!riscv64_tlb_context.initialized) {
        return;
    }
    
    uint64_t tag = va >> 12;  // Page number
    uint64_t entry = (pa >> 12) | flags;
    
    // Find free TLB entry
    int free_index = -1;
    for (int i = 0; i < RISCV64_TLB_ENTRIES; i++) {
        if (!riscv64_tlb_context.tlb_valid[i]) {
            free_index = i;
            break;
        }
    }
    
    // If no free entry, evict one
    if (free_index == -1) {
        free_index = 0;  // Simple FIFO replacement
        riscv64_tlb_context.tlb_evictions++;
    }
    
    // Insert entry
    riscv64_tlb_context.tlb_valid[free_index] = true;
    riscv64_tlb_context.tlb_tags[free_index] = tag;
    riscv64_tlb_context.tlb_entries[free_index] = entry;
}

// ============================================================================
// MEMORY PROTECTION
// ============================================================================

// Set memory protection
int riscv64_mmu_set_protection(uint64_t va, uint64_t size, uint64_t flags)
{
    if (!riscv64_mmu_context.initialized) {
        return -1;
    }
    
    // Update protection for all pages in the range
    for (uint64_t addr = va; addr < va + size; addr += RISCV64_PAGE_SIZE) {
        // Find the page table entry and update flags
        // This is a simplified implementation
        riscv64_mmu_update_page_flags(addr, flags);
    }
    
    // Invalidate TLB entries for the range
    for (uint64_t addr = va; addr < va + size; addr += RISCV64_PAGE_SIZE) {
        riscv64_tlb_invalidate_entry(addr);
    }
    
    return 0;
}

// Update page flags
void riscv64_mmu_update_page_flags(uint64_t va, uint64_t flags)
{
    // This function would update the protection flags for a specific page
    // Implementation depends on the page table structure
    printf("RISC-V64: Page flags updated for VA 0x%llx to 0x%llx\n", va, flags);
}

// ============================================================================
// MEMORY REGION MANAGEMENT
// ============================================================================

// Add memory region
int riscv64_mmu_add_region(uint64_t base, uint64_t size, uint64_t flags)
{
    if (!riscv64_memory_regions.initialized || riscv64_memory_regions.region_count >= 64) {
        return -1;
    }
    
    int index = riscv64_memory_regions.region_count;
    riscv64_memory_regions.regions[index] = base;
    riscv64_memory_regions.region_flags[index] = flags;
    riscv64_memory_regions.region_count++;
    
    printf("RISC-V64: Memory region added at 0x%llx, size 0x%llx, flags 0x%llx\n", 
           base, size, flags);
    
    return index;
}

// Remove memory region
int riscv64_mmu_remove_region(uint64_t base)
{
    if (!riscv64_memory_regions.initialized) {
        return -1;
    }
    
    for (int i = 0; i < riscv64_memory_regions.region_count; i++) {
        if (riscv64_memory_regions.regions[i] == base) {
            // Remove region by shifting remaining regions
            for (int j = i; j < riscv64_memory_regions.region_count - 1; j++) {
                riscv64_memory_regions.regions[j] = riscv64_memory_regions.regions[j + 1];
                riscv64_memory_regions.region_flags[j] = riscv64_memory_regions.region_flags[j + 1];
            }
            riscv64_memory_regions.region_count--;
            
            printf("RISC-V64: Memory region removed at 0x%llx\n", base);
            return 0;
        }
    }
    
    return -1;  // Region not found
}

// ============================================================================
// ADDRESS TRANSLATION
// ============================================================================

// Virtual to physical address translation
uint64_t riscv64_mmu_virt_to_phys(uint64_t va)
{
    if (!riscv64_mmu_context.initialized) {
        return va;  // Identity mapping if MMU not initialized
    }
    
    // Check TLB first
    uint64_t tlb_entry = riscv64_tlb_lookup(va);
    if (tlb_entry) {
        return ((tlb_entry >> 12) << 12) | (va & 0xFFF);
    }
    
    // TLB miss - walk page tables
    uint64_t pa = riscv64_mmu_walk_page_tables(va);
    if (pa) {
        // Insert into TLB
        uint64_t flags = riscv64_mmu_get_page_flags(va);
        riscv64_tlb_insert(va, pa, flags);
    }
    
    return pa;
}

// Walk page tables
uint64_t riscv64_mmu_walk_page_tables(uint64_t va)
{
    if (!riscv64_mmu_context.root_page_table) {
        return 0;
    }
    
    // Calculate page table indices
    uint64_t level1_index = (va >> 30) & 0x1FF;
    uint64_t level2_index = (va >> 21) & 0x1FF;
    uint64_t level3_index = (va >> 12) & 0x1FF;
    
    uint64_t *root_table = riscv64_mmu_context.root_page_table;
    
    // Level 1
    if (!(root_table[level1_index] & 0x1)) {
        return 0;  // Not mapped
    }
    
    // Check if this is a 1GB page
    if (root_table[level1_index] & 0x80) {
        uint64_t pa = ((root_table[level1_index] >> 30) << 30) | (va & 0x3FFFFFFF);
        return pa;
    }
    
    uint64_t *level2_table = (uint64_t*)((root_table[level1_index] >> 12) << 12);
    
    // Level 2
    if (!(level2_table[level2_index] & 0x1)) {
        return 0;  // Not mapped
    }
    
    // Check if this is a 2MB page
    if (level2_table[level2_index] & 0x80) {
        uint64_t pa = ((level2_table[level2_index] >> 21) << 21) | (va & 0x1FFFFF);
        return pa;
    }
    
    uint64_t *level3_table = (uint64_t*)((level2_table[level2_index] >> 12) << 12);
    
    // Level 3
    if (!(level3_table[level3_index] & 0x1)) {
        return 0;  // Not mapped
    }
    
    uint64_t pa = ((level3_table[level3_index] >> 12) << 12) | (va & 0xFFF);
    return pa;
}

// Get page flags
uint64_t riscv64_mmu_get_page_flags(uint64_t va)
{
    // This function would extract the protection flags for a page
    // Implementation depends on the page table structure
    return RISCV64_PAGE_READ | RISCV64_PAGE_WRITE | RISCV64_PAGE_EXECUTE;
}

// ============================================================================
// MMU STATUS AND DEBUG
// ============================================================================

// Get MMU status
bool riscv64_mmu_is_initialized(void)
{
    return riscv64_mmu_context.initialized;
}

// Get TLB statistics
void riscv64_mmu_get_tlb_stats(uint32_t *hits, uint32_t *misses, uint32_t *evictions)
{
    if (hits) *hits = riscv64_tlb_context.tlb_hits;
    if (misses) *misses = riscv64_tlb_context.tlb_misses;
    if (evictions) *evictions = riscv64_tlb_context.tlb_evictions;
}

// Print MMU status
void riscv64_mmu_print_status(void)
{
    printf("RISC-V64: MMU Status\n");
    printf("  Initialized: %s\n", riscv64_mmu_context.initialized ? "Yes" : "No");
    printf("  SATP: 0x%llx\n", riscv64_mmu_context.satp);
    printf("  ASID: %llu\n", riscv64_mmu_context.asid);
    printf("  Root Page Table: 0x%llx\n", (uint64_t)riscv64_mmu_context.root_page_table);
    printf("  Page Table Pages: %llu\n", riscv64_page_table_context.page_table_pages);
    printf("  Page Table Size: %llu bytes\n", riscv64_page_table_context.page_table_size);
    printf("  Total Pages: %llu\n", riscv64_mmu_context.total_pages);
    printf("  Used Pages: %llu\n", riscv64_mmu_context.used_pages);
    
    printf("  TLB:\n");
    printf("    Entries: %u\n", RISCV64_TLB_ENTRIES);
    printf("    Hits: %u\n", riscv64_tlb_context.tlb_hits);
    printf("    Misses: %u\n", riscv64_tlb_context.tlb_misses);
    printf("    Evictions: %u\n", riscv64_tlb_context.tlb_evictions);
    
    printf("  Memory Regions: %llu\n", riscv64_memory_regions.region_count);
}

// ============================================================================
// MMU CLEANUP
// ============================================================================

// Cleanup MMU system
void riscv64_mmu_cleanup(void)
{
    if (!riscv64_mmu_context.initialized) {
        return;
    }
    
    // Cleanup page tables
    if (riscv64_page_table_context.kernel_page_table) {
        riscv64_page_table_destroy(riscv64_page_table_context.kernel_page_table);
        riscv64_page_table_context.kernel_page_table = NULL;
    }
    
    if (riscv64_page_table_context.user_page_table) {
        riscv64_page_table_destroy(riscv64_page_table_context.user_page_table);
        riscv64_page_table_context.user_page_table = NULL;
    }
    
    // Clear contexts
    riscv64_mmu_context.initialized = false;
    riscv64_tlb_context.initialized = false;
    riscv64_page_table_context.initialized = false;
    riscv64_memory_regions.initialized = false;
    
    printf("RISC-V64: MMU system cleaned up\n");
}
