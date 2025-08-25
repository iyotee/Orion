/*
 * ORION OS - MIPS MMU Management
 *
 * Memory Management Unit for MIPS architecture
 * Handles TLB, page tables, and memory protection
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
#include <stdlib.h>

// ============================================================================
// TLB MANAGEMENT
// ============================================================================

static mips_tlb_entry_t mips_tlb_table[MIPS_TLB_ENTRIES] = {0};
static uint32_t mips_tlb_index = 0;

void mips_tlb_init(void)
{
    memset(mips_tlb_table, 0, sizeof(mips_tlb_table));
    mips_tlb_index = 0;
    
    // Clear all TLB entries
    for (int i = 0; i < MIPS_TLB_ENTRIES; i++) {
        mips_tlb_write_index(i, 0, 0, 0, 0);
    }
    
    printf("MIPS: TLB initialized with %d entries\n", MIPS_TLB_ENTRIES);
}

void mips_tlb_write_index(uint32_t index, mips_addr_t vaddr, mips_addr_t paddr, 
                         uint32_t flags, uint32_t asid)
{
    if (index >= MIPS_TLB_ENTRIES) {
        printf("MIPS: Invalid TLB index: %d\n", index);
        return;
    }
    
    // Write EntryHi
    mips_cpu_write_cp0_register(MIPS_CP0_ENTRYHI, 
        (vaddr & MIPS_ENTRYHI_VPN_MASK) | (asid & MIPS_ENTRYHI_ASID_MASK));
    
    // Write EntryLo0
    mips_cpu_write_cp0_register(MIPS_CP0_ENTRYLO0, 
        (paddr & MIPS_ENTRYLO_PFN_MASK) | (flags & MIPS_ENTRYLO_FLAGS_MASK));
    
    // Write EntryLo1 (for 2MB pages)
    mips_cpu_write_cp0_register(MIPS_CP0_ENTRYLO1, 
        ((paddr + 0x1000) & MIPS_ENTRYLO_PFN_MASK) | (flags & MIPS_ENTRYLO_FLAGS_MASK));
    
    // Set index and write TLB entry
    mips_cpu_write_cp0_register(MIPS_CP0_INDEX, index);
    __asm__ volatile("tlbwi");
    
    // Update local table
    mips_tlb_table[index].vaddr = vaddr;
    mips_tlb_table[index].paddr = paddr;
    mips_tlb_table[index].flags = flags;
    mips_tlb_table[index].asid = asid;
}

void mips_tlb_read_index(uint32_t index, mips_addr_t* vaddr, mips_addr_t* paddr, 
                        uint32_t* flags, uint32_t* asid)
{
    if (index >= MIPS_TLB_ENTRIES) {
        printf("MIPS: Invalid TLB index: %d\n", index);
        return;
    }
    
    // Set index and read TLB entry
    mips_cpu_write_cp0_register(MIPS_CP0_INDEX, index);
    __asm__ volatile("tlbr");
    
    // Read EntryHi
    uint64_t entryhi = mips_cpu_read_cp0_register(MIPS_CP0_ENTRYHI);
    *vaddr = entryhi & MIPS_ENTRYHI_VPN_MASK;
    *asid = entryhi & MIPS_ENTRYHI_ASID_MASK;
    
    // Read EntryLo0
    uint64_t entrylo0 = mips_cpu_read_cp0_register(MIPS_CP0_ENTRYLO0);
    *paddr = entrylo0 & MIPS_ENTRYLO_PFN_MASK;
    *flags = entrylo0 & MIPS_ENTRYLO_FLAGS_MASK;
}

void mips_tlb_write_random(mips_addr_t vaddr, mips_addr_t paddr, 
                          uint32_t flags, uint32_t asid)
{
    // Write EntryHi
    mips_cpu_write_cp0_register(MIPS_CP0_ENTRYHI, 
        (vaddr & MIPS_ENTRYHI_VPN_MASK) | (asid & MIPS_ENTRYHI_ASID_MASK));
    
    // Write EntryLo0
    mips_cpu_write_cp0_register(MIPS_CP0_ENTRYLO0, 
        (paddr & MIPS_ENTRYLO_PFN_MASK) | (flags & MIPS_ENTRYLO_FLAGS_MASK));
    
    // Write EntryLo1
    mips_cpu_write_cp0_register(MIPS_CP0_ENTRYLO1, 
        ((paddr + 0x1000) & MIPS_ENTRYLO_PFN_MASK) | (flags & MIPS_ENTRYLO_FLAGS_MASK));
    
    // Write random TLB entry
    __asm__ volatile("tlbwr");
}

void mips_tlb_invalidate_all(void)
{
    // Invalidate all TLB entries
    __asm__ volatile("tlbp");
    
    printf("MIPS: All TLB entries invalidated\n");
}

void mips_tlb_invalidate_asid(uint32_t asid)
{
    for (int i = 0; i < MIPS_TLB_ENTRIES; i++) {
        mips_addr_t vaddr, paddr;
        uint32_t flags, entry_asid;
        
        mips_tlb_read_index(i, &vaddr, &paddr, &flags, &entry_asid);
        
        if (entry_asid == asid) {
            mips_tlb_write_index(i, 0, 0, 0, 0);
        }
    }
    
    printf("MIPS: TLB entries for ASID %d invalidated\n", asid);
}

void mips_tlb_invalidate_vaddr(mips_addr_t vaddr, uint32_t asid)
{
    for (int i = 0; i < MIPS_TLB_ENTRIES; i++) {
        mips_addr_t entry_vaddr, paddr;
        uint32_t flags, entry_asid;
        
        mips_tlb_read_index(i, &entry_vaddr, &paddr, &flags, &entry_asid);
        
        if (entry_vaddr == vaddr && entry_asid == asid) {
            mips_tlb_write_index(i, 0, 0, 0, 0);
            break;
        }
    }
}

// ============================================================================
// PAGE TABLE MANAGEMENT
// ============================================================================

static mips_page_table_t* mips_kernel_page_table = NULL;
static mips_page_table_t* mips_current_page_table = NULL;

void mips_page_table_init(void)
{
    // Allocate kernel page table
    mips_kernel_page_table = (mips_page_table_t*)malloc(sizeof(mips_page_table_t));
    if (!mips_kernel_page_table) {
        printf("MIPS: Failed to allocate kernel page table\n");
        return;
    }
    
    memset(mips_kernel_page_table, 0, sizeof(mips_page_table_t));
    mips_current_page_table = mips_kernel_page_table;
    
    printf("MIPS: Page table system initialized\n");
}

void mips_page_table_switch(mips_page_table_t* page_table)
{
    if (!page_table) {
        printf("MIPS: Invalid page table\n");
        return;
    }
    
    mips_current_page_table = page_table;
    
    // Update page table base register
    mips_cpu_write_cp0_register(MIPS_CP0_CONTEXT, 
        (uint64_t)page_table & MIPS_CONTEXT_PTEBASE_MASK);
    
    // Invalidate TLB
    mips_tlb_invalidate_all();
    
    printf("MIPS: Page table switched to 0x%llx\n", (uint64_t)page_table);
}

mips_page_table_t* mips_page_table_get_current(void)
{
    return mips_current_page_table;
}

mips_page_table_t* mips_page_table_create(void)
{
    mips_page_table_t* page_table = (mips_page_table_t*)malloc(sizeof(mips_page_table_t));
    if (!page_table) {
        printf("MIPS: Failed to allocate page table\n");
        return NULL;
    }
    
    memset(page_table, 0, sizeof(mips_page_table_t));
    
    printf("MIPS: New page table created\n");
    return page_table;
}

void mips_page_table_destroy(mips_page_table_t* page_table)
{
    if (!page_table) {
        return;
    }
    
    // Free page table memory
    free(page_table);
    
    printf("MIPS: Page table destroyed\n");
}

// ============================================================================
// MEMORY MAPPING
// ============================================================================

int mips_memory_map(mips_addr_t vaddr, mips_addr_t paddr, mips_size_t size, 
                   uint32_t flags, uint32_t asid)
{
    if (size == 0 || (size % MIPS_PAGE_SIZE) != 0) {
        printf("MIPS: Invalid mapping size: %llu\n", size);
        return -1;
    }
    
    uint32_t pages = size / MIPS_PAGE_SIZE;
    
    for (uint32_t i = 0; i < pages; i++) {
        mips_addr_t page_vaddr = vaddr + (i * MIPS_PAGE_SIZE);
        mips_addr_t page_paddr = paddr + (i * MIPS_PAGE_SIZE);
        
        // Find free TLB entry
        uint32_t tlb_index = mips_tlb_find_free();
        if (tlb_index == MIPS_TLB_ENTRIES) {
            printf("MIPS: No free TLB entries\n");
            return -1;
        }
        
        // Map page
        mips_tlb_write_index(tlb_index, page_vaddr, page_paddr, flags, asid);
    }
    
    printf("MIPS: Mapped %u pages from 0x%llx to 0x%llx\n", pages, vaddr, vaddr + size);
    return 0;
}

int mips_memory_unmap(mips_addr_t vaddr, mips_size_t size, uint32_t asid)
{
    if (size == 0 || (size % MIPS_PAGE_SIZE) != 0) {
        printf("MIPS: Invalid unmapping size: %llu\n", size);
        return -1;
    }
    
    uint32_t pages = size / MIPS_PAGE_SIZE;
    
    for (uint32_t i = 0; i < pages; i++) {
        mips_addr_t page_vaddr = vaddr + (i * MIPS_PAGE_SIZE);
        mips_tlb_invalidate_vaddr(page_vaddr, asid);
    }
    
    printf("MIPS: Unmapped %u pages starting at 0x%llx\n", pages, vaddr);
    return 0;
}

mips_addr_t mips_memory_virt_to_phys(mips_addr_t vaddr)
{
    // Search TLB for virtual address
    for (int i = 0; i < MIPS_TLB_ENTRIES; i++) {
        mips_addr_t entry_vaddr, paddr;
        uint32_t flags, asid;
        
        mips_tlb_read_index(i, &entry_vaddr, &paddr, &flags, &asid);
        
        if (entry_vaddr == (vaddr & MIPS_PAGE_MASK)) {
            return paddr | (vaddr & ~MIPS_PAGE_MASK);
        }
    }
    
    // Page not found in TLB
    return MIPS_INVALID_ADDRESS;
}

// ============================================================================
// MEMORY PROTECTION
// ============================================================================

void mips_memory_protect(mips_addr_t vaddr, mips_size_t size, uint32_t flags)
{
    if (size == 0 || (size % MIPS_PAGE_SIZE) != 0) {
        printf("MIPS: Invalid protection size: %llu\n", size);
        return;
    }
    
    uint32_t pages = size / MIPS_PAGE_SIZE;
    
    for (uint32_t i = 0; i < pages; i++) {
        mips_addr_t page_vaddr = vaddr + (i * MIPS_PAGE_SIZE);
        
        // Find TLB entry and update protection
        for (int j = 0; j < MIPS_TLB_ENTRIES; j++) {
            mips_addr_t entry_vaddr, paddr;
            uint32_t entry_flags, asid;
            
            mips_tlb_read_index(j, &entry_vaddr, &paddr, &entry_flags, &asid);
            
            if (entry_vaddr == page_vaddr) {
                // Update protection flags
                uint32_t new_flags = (entry_flags & ~MIPS_ENTRYLO_FLAGS_MASK) | 
                                   (flags & MIPS_ENTRYLO_FLAGS_MASK);
                mips_tlb_write_index(j, entry_vaddr, paddr, new_flags, asid);
                break;
            }
        }
    }
    
    printf("MIPS: Protection updated for %u pages starting at 0x%llx\n", pages, vaddr);
}

bool mips_memory_is_accessible(mips_addr_t vaddr, uint32_t access_flags)
{
    // Search TLB for virtual address
    for (int i = 0; i < MIPS_TLB_ENTRIES; i++) {
        mips_addr_t entry_vaddr, paddr;
        uint32_t flags, asid;
        
        mips_tlb_read_index(i, &entry_vaddr, &paddr, &flags, &asid);
        
        if (entry_vaddr == (vaddr & MIPS_PAGE_MASK)) {
            // Check if access is allowed
            if (access_flags & MIPS_ACCESS_READ && !(flags & MIPS_ENTRYLO_READ)) {
                return false;
            }
            if (access_flags & MIPS_ACCESS_WRITE && !(flags & MIPS_ENTRYLO_WRITE)) {
                return false;
            }
            if (access_flags & MIPS_ACCESS_EXECUTE && !(flags & MIPS_ENTRYLO_EXECUTE)) {
                return false;
            }
            return true;
        }
    }
    
    // Page not found in TLB
    return false;
}

// ============================================================================
// MEMORY REGION MANAGEMENT
// ============================================================================

int mips_memory_region_create(mips_addr_t vaddr, mips_size_t size, uint32_t flags)
{
    if (!mips_current_page_table) {
        printf("MIPS: No current page table\n");
        return -1;
    }
    
    // Find free region in page table
    mips_memory_region_t* region = mips_memory_region_find_free(size);
    if (!region) {
        printf("MIPS: No free memory region for size %llu\n", size);
        return -1;
    }
    
    // Allocate physical memory
    mips_addr_t paddr = mips_memory_alloc_physical(size);
    if (paddr == MIPS_INVALID_ADDRESS) {
        printf("MIPS: Failed to allocate physical memory\n");
        return -1;
    }
    
    // Map memory
    if (mips_memory_map(vaddr, paddr, size, flags, 0) != 0) {
        mips_memory_free_physical(paddr, size);
        return -1;
    }
    
    // Update region
    region->vaddr = vaddr;
    region->paddr = paddr;
    region->size = size;
    region->flags = flags;
    region->used = true;
    
    printf("MIPS: Memory region created at 0x%llx, size %llu\n", vaddr, size);
    return 0;
}

void mips_memory_region_destroy(mips_addr_t vaddr)
{
    if (!mips_current_page_table) {
        return;
    }
    
    // Find region
    mips_memory_region_t* region = mips_memory_region_find(vaddr);
    if (!region) {
        printf("MIPS: Memory region not found at 0x%llx\n", vaddr);
        return;
    }
    
    // Unmap memory
    mips_memory_unmap(region->vaddr, region->size, 0);
    
    // Free physical memory
    mips_memory_free_physical(region->paddr, region->size);
    
    // Clear region
    memset(region, 0, sizeof(mips_memory_region_t));
    
    printf("MIPS: Memory region destroyed at 0x%llx\n", vaddr);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

uint32_t mips_tlb_find_free(void)
{
    for (uint32_t i = 0; i < MIPS_TLB_ENTRIES; i++) {
        mips_addr_t vaddr, paddr;
        uint32_t flags, asid;
        
        mips_tlb_read_index(i, &vaddr, &paddr, &flags, &asid);
        
        if (vaddr == 0 && paddr == 0) {
            return i;
        }
    }
    
    return MIPS_TLB_ENTRIES; // No free entries
}

mips_memory_region_t* mips_memory_region_find(mips_addr_t vaddr)
{
    if (!mips_current_page_table) {
        return NULL;
    }
    
    for (int i = 0; i < MIPS_MAX_MEMORY_REGIONS; i++) {
        mips_memory_region_t* region = &mips_current_page_table->regions[i];
        
        if (region->used && 
            vaddr >= region->vaddr && 
            vaddr < region->vaddr + region->size) {
            return region;
        }
    }
    
    return NULL;
}

mips_memory_region_t* mips_memory_region_find_free(mips_size_t size)
{
    if (!mips_current_page_table) {
        return NULL;
    }
    
    for (int i = 0; i < MIPS_MAX_MEMORY_REGIONS; i++) {
        mips_memory_region_t* region = &mips_current_page_table->regions[i];
        
        if (!region->used && region->size >= size) {
            return region;
        }
    }
    
    return NULL;
}

mips_addr_t mips_memory_alloc_physical(mips_size_t size)
{
    // Simple physical memory allocator
    // In a real implementation, this would use a proper memory manager
    static mips_addr_t next_physical_addr = MIPS_PHYSICAL_MEMORY_BASE;
    
    mips_addr_t addr = next_physical_addr;
    next_physical_addr += size;
    
    return addr;
}

void mips_memory_free_physical(mips_addr_t paddr, mips_size_t size)
{
    // Simple physical memory deallocator
    // In a real implementation, this would use a proper memory manager
    (void)paddr;
    (void)size;
}
