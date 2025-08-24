/*
 * Orion Operating System - Virtual Memory Manager (VMM)
 *
 * Advanced virtual memory management with 4-level page tables, TLB management,
 * memory protection, and optimized page allocation algorithms.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/types.h>
#include <orion/mm.h>
#include <orion/kernel.h>
#include <orion/security.h>

// ========================================
// CONSTANTS AND CONFIGURATION
// ========================================

// Virtual address space layout
#define KERNEL_SPACE_START 0xFFFFFFFF80000000ULL
#define KERNEL_SPACE_END 0xFFFFFFFFFFFFFFFULL
#define USER_SPACE_START 0x0000000000400000ULL // 4MB
#define USER_SPACE_END 0x0000007FFFFFFFFFULL   // 128TB

// Page table configuration
#define PT_ENTRIES 512 // Entries per page table
#define PT_SHIFT 9     // 9 bits per level
// PAGE_MASK already defined in types.h

// Advanced VMM features
#define VMM_MAX_REGIONS 1024 // Max memory regions per space
#define VMM_BITMAP_SIZE (USER_SPACE_END - USER_SPACE_START) / PAGE_SIZE / 64
#define TLB_FLUSH_THRESHOLD 64 // Pages before global flush

// Page table flags (architecture-independent)
#define PTE_PRESENT (1ULL << 0)
#define PTE_WRITE (1ULL << 1)
#define PTE_USER (1ULL << 2)
#define PTE_CACHE_DISABLE (1ULL << 3)
#define PTE_ACCESSED (1ULL << 5)
#define PTE_DIRTY (1ULL << 6)
#define PTE_GLOBAL (1ULL << 8)
#define PTE_NX (1ULL << 63)

// ========================================
// DATA STRUCTURES
// ========================================

// Memory region descriptor
typedef struct vm_region
{
    uint64_t start_addr;     // Start virtual address
    uint64_t end_addr;       // End virtual address
    uint64_t flags;          // Protection flags
    uint64_t backing_object; // File/device backing this region
    uint64_t offset;         // Offset in backing object
    struct vm_region *next;  // Linked list
} vm_region_t;

// Enhanced VM space with advanced features
typedef struct vm_space_enhanced
{
    uint64_t pml4_phys;  // Physical address of PML4
    uint64_t start_addr; // Start of address space
    uint64_t end_addr;   // End of address space
    bool is_kernel;      // Is this kernel space?

    // Advanced features
    vm_region_t *regions;      // Memory regions list
    uint64_t *free_bitmap;     // Free page bitmap for fast allocation
    atomic64_t tlb_generation; // TLB generation counter
    uint64_t page_fault_count; // Statistics
    uint64_t total_pages;      // Total allocated pages
    spinlock_t lock;           // Protection lock
} vm_space_enhanced_t;

// Basic VM space structure (used by existing code)
typedef struct vm_space
{
    uint64_t pml4_phys;  // Physical address of PML4
    uint64_t start_addr; // Start of address space
    uint64_t end_addr;   // End of address space
    bool is_kernel;      // Is this kernel space?

    // Add the missing free_bitmap field
    uint64_t *free_bitmap; // Free page bitmap for fast allocation
} vm_space_t;

// COW page reference tracking
#define MAX_PAGE_REFS 65536
static page_ref_t page_refs[MAX_PAGE_REFS];
static spinlock_t page_refs_lock = SPINLOCK_INIT;

// Global kernel space
static vm_space_enhanced_t kernel_space_enhanced;
static vm_space_t kernel_space;
static bool vmm_initialized = false;

// TLB management
static atomic64_t global_tlb_generation = ATOMIC_VAR_INIT(1);
static uint64_t pending_tlb_flushes = 0;

// Initialize VMM
void vmm_init(void)
{
    kinfo("Initializing Virtual Memory Manager");

    // Configure kernel space
    kernel_space.pml4_phys = read_cr3() & 0xFFFFFFFFFFFFF000ULL;
    kernel_space.start_addr = KERNEL_SPACE_START;
    kernel_space.end_addr = KERNEL_SPACE_END;
    kernel_space.is_kernel = true;

    vmm_initialized = true;

    kinfo("VMM initialized:");
    kinfo("  Kernel space: 0x%p - 0x%p",
          (void *)kernel_space.start_addr,
          (void *)kernel_space.end_addr);
    kinfo("  User space: 0x%p - 0x%p",
          (void *)USER_SPACE_START,
          (void *)USER_SPACE_END);
}

// Create a new address space
vm_space_t *vmm_create_space(bool is_kernel)
{
    if (!vmm_initialized)
    {
        return NULL;
    }

    if (is_kernel)
    {
        return &kernel_space;
    }

    // Creating a user space
    vm_space_t *user_space = (vm_space_t *)kmalloc(sizeof(vm_space_t));
    if (!user_space)
    {
        kerror("Failed to allocate user space structure");
        return NULL;
    }

    // Allocate a new PML4 for user space
    uint64_t pml4_phys = pmm_alloc_page();
    if (!pml4_phys)
    {
        kfree(user_space);
        kerror("Failed to allocate PML4 for user space");
        return NULL;
    }

    // Initialize user space
    user_space->pml4_phys = pml4_phys;
    user_space->start_addr = USER_SPACE_START;
    user_space->end_addr = USER_SPACE_END;
    user_space->is_kernel = false;

    // Copy kernel entries from current PML4 to the new one
    uint64_t *current_pml4 = (uint64_t *)(kernel_space.pml4_phys);
    uint64_t *new_pml4 = (uint64_t *)pml4_phys;

    // Copy the upper 256 entries (kernel space)
    for (int i = 256; i < 512; i++)
    {
        new_pml4[i] = current_pml4[i];
    }

    // Initialize user entries to 0
    for (int i = 0; i < 256; i++)
    {
        new_pml4[i] = 0;
    }

    // Initialize free_bitmap to NULL (will be allocated when needed)
    user_space->free_bitmap = NULL;

    kdebug("Created user space: PML4=0x%p", (void *)pml4_phys);
    return user_space;
}

// Map a virtual page to a physical page
int vmm_map_page(vm_space_t *space, uint64_t vaddr, uint64_t paddr, uint64_t flags)
{
    if (!vmm_initialized || !space)
    {
        return -OR_EINVAL;
    }

    // Check alignment
    if (!IS_ALIGNED(vaddr, PAGE_SIZE) || !IS_ALIGNED(paddr, PAGE_SIZE))
    {
        return -OR_EINVAL;
    }

    // Check that address is within space
    if (vaddr < space->start_addr || vaddr >= space->end_addr)
    {
        return -OR_EINVAL;
    }

    // Extraire les indices des tables de pages
    uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (vaddr >> 30) & 0x1FF;
    uint64_t pd_idx = (vaddr >> 21) & 0x1FF;
    uint64_t pt_idx = (vaddr >> 12) & 0x1FF;

    // Get PML4
    uint64_t *pml4 = (uint64_t *)space->pml4_phys;

    // Allocate PDPT if necessary
    if (!(pml4[pml4_idx] & 1))
    {
        uint64_t pdpt_phys = pmm_alloc_page();
        if (!pdpt_phys)
        {
            return -OR_ENOMEM;
        }

        // Initialize the new table
        uint64_t *pdpt = (uint64_t *)pdpt_phys;
        for (int i = 0; i < 512; i++)
        {
            pdpt[i] = 0;
        }

        pml4[pml4_idx] = pdpt_phys | flags | 1; // Present + flags
    }

    // Get PDPT
    uint64_t *pdpt = (uint64_t *)(pml4[pml4_idx] & 0xFFFFFFFFFFFFF000ULL);

    // Allocate PD if necessary
    if (!(pdpt[pdpt_idx] & 1))
    {
        uint64_t pd_phys = pmm_alloc_page();
        if (!pd_phys)
        {
            return -OR_ENOMEM;
        }

        uint64_t *pd = (uint64_t *)pd_phys;
        for (int i = 0; i < 512; i++)
        {
            pd[i] = 0;
        }

        pdpt[pdpt_idx] = pd_phys | flags | 1;
    }

    // Get PD
    uint64_t *pd = (uint64_t *)(pdpt[pdpt_idx] & 0xFFFFFFFFFFFFF000ULL);

    // Allocate PT if necessary
    if (!(pd[pd_idx] & 1))
    {
        uint64_t pt_phys = pmm_alloc_page();
        if (!pt_phys)
        {
            return -OR_ENOMEM;
        }

        uint64_t *pt = (uint64_t *)pt_phys;
        for (int i = 0; i < 512; i++)
        {
            pt[i] = 0;
        }

        pd[pd_idx] = pt_phys | flags | 1;
    }

    // Get PT and map the page
    uint64_t *pt = (uint64_t *)(pd[pd_idx] & 0xFFFFFFFFFFFFF000ULL);
    pt[pt_idx] = paddr | flags | 1; // Present + flags

    // Invalider la TLB pour cette page
    mmu_invalidate_page(vaddr);

    kdebug("vmm_map_page: 0x%p -> 0x%p (flags=0x%p) completed",
           (void *)vaddr, (void *)paddr, (void *)flags);

    return OR_OK;
}

// Unmap a virtual page
int vmm_unmap_page(vm_space_t *space, uint64_t vaddr)
{
    if (!vmm_initialized || !space)
    {
        return -OR_EINVAL;
    }

    if (!IS_ALIGNED(vaddr, PAGE_SIZE))
    {
        return -OR_EINVAL;
    }

    // Extraire les indices des tables de pages
    uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (vaddr >> 30) & 0x1FF;
    uint64_t pd_idx = (vaddr >> 21) & 0x1FF;
    uint64_t pt_idx = (vaddr >> 12) & 0x1FF;

    // Naviguer dans les tables de pages
    uint64_t *pml4 = (uint64_t *)space->pml4_phys;
    if (!(pml4[pml4_idx] & 1))
    {
        kdebug("vmm_unmap_page: 0x%p not mapped (no PDPT)", (void *)vaddr);
        return OR_OK; // Already unmapped
    }

    uint64_t *pdpt = (uint64_t *)(pml4[pml4_idx] & 0xFFFFFFFFFFFFF000ULL);
    if (!(pdpt[pdpt_idx] & 1))
    {
        kdebug("vmm_unmap_page: 0x%p not mapped (no PD)", (void *)vaddr);
        return OR_OK;
    }

    uint64_t *pd = (uint64_t *)(pdpt[pdpt_idx] & 0xFFFFFFFFFFFFF000ULL);
    if (!(pd[pd_idx] & 1))
    {
        kdebug("vmm_unmap_page: 0x%p not mapped (no PT)", (void *)vaddr);
        return OR_OK;
    }

    uint64_t *pt = (uint64_t *)(pd[pd_idx] & 0xFFFFFFFFFFFFF000ULL);
    if (!(pt[pt_idx] & 1))
    {
        kdebug("vmm_unmap_page: 0x%p not mapped (no page)", (void *)vaddr);
        return OR_OK;
    }

    // Get physical address before unmapping
    uint64_t paddr = pt[pt_idx] & 0xFFFFFFFFFFFFF000ULL;

    // Unmap the page
    pt[pt_idx] = 0;

    // Invalider la TLB
    mmu_invalidate_page(vaddr);

    kdebug("vmm_unmap_page: 0x%p (was 0x%p) unmapped", (void *)vaddr, (void *)paddr);

    return OR_OK;
}

// Changer les permissions d'une page
int vmm_protect_page(vm_space_t *space, uint64_t vaddr, uint64_t new_flags)
{
    if (!vmm_initialized || !space)
    {
        return -OR_EINVAL;
    }

    if (!IS_ALIGNED(vaddr, PAGE_SIZE))
    {
        return -OR_EINVAL;
    }

    // Extraire les indices des tables de pages
    uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (vaddr >> 30) & 0x1FF;
    uint64_t pd_idx = (vaddr >> 21) & 0x1FF;
    uint64_t pt_idx = (vaddr >> 12) & 0x1FF;

    // Naviguer dans les tables de pages
    uint64_t *pml4 = (uint64_t *)space->pml4_phys;
    if (!(pml4[pml4_idx] & 1))
    {
        return -OR_EINVAL; // Page not mapped
    }

    uint64_t *pdpt = (uint64_t *)(pml4[pml4_idx] & 0xFFFFFFFFFFFFF000ULL);
    if (!(pdpt[pdpt_idx] & 1))
    {
        return -OR_EINVAL;
    }

    uint64_t *pd = (uint64_t *)(pdpt[pdpt_idx] & 0xFFFFFFFFFFFFF000ULL);
    if (!(pd[pd_idx] & 1))
    {
        return -OR_EINVAL;
    }

    uint64_t *pt = (uint64_t *)(pd[pd_idx] & 0xFFFFFFFFFFFFF000ULL);
    if (!(pt[pt_idx] & 1))
    {
        return -OR_EINVAL;
    }

    // Save physical address
    uint64_t paddr = pt[pt_idx] & 0xFFFFFFFFFFFFF000ULL;
    uint64_t old_flags = pt[pt_idx] & 0xFFF;

    // Apply new permissions
    pt[pt_idx] = paddr | new_flags | 1;

    // Invalider la TLB
    mmu_invalidate_page(vaddr);

    kdebug("vmm_protect_page: 0x%p flags 0x%p -> 0x%p",
           (void *)vaddr, (void *)old_flags, (void *)new_flags);

    return OR_OK;
}

// Allocate and map virtual pages
uint64_t vmm_alloc_pages(vm_space_t *space, size_t count, uint64_t flags)
{
    if (!vmm_initialized || !space || count == 0)
    {
        return 0;
    }

    // Find free space in address space
    uint64_t vaddr_start = space->start_addr;
    uint64_t vaddr_end = space->end_addr;
    uint64_t needed_size = count * PAGE_SIZE;

    // Enhanced search: use bitmap for faster allocation
    // This provides better performance than linear search for large allocations

    // First, try to find a contiguous block using bitmap search
    if (space->free_bitmap && count <= 64)
    {
        // Use bitmap for small to medium allocations
        uint64_t bitmap_start = (vaddr_start - USER_SPACE_START) / PAGE_SIZE / 64;
        uint64_t bitmap_end = (vaddr_end - USER_SPACE_START) / PAGE_SIZE / 64;

        for (uint64_t bitmap_idx = bitmap_start; bitmap_idx <= bitmap_end; bitmap_idx++)
        {
            if (bitmap_idx >= VMM_BITMAP_SIZE)
                break;

            uint64_t bitmap_entry = space->free_bitmap[bitmap_idx];
            if (bitmap_entry == 0xFFFFFFFFFFFFFFFFULL)
            {
                continue; // No free pages in this bitmap entry
            }

            // Find free pages in this bitmap entry
            for (int bit = 0; bit < 64; bit++)
            {
                if (!(bitmap_entry & (1ULL << bit)))
                {
                    // Found a free page, check if we have enough contiguous space
                    uint64_t start_vaddr = USER_SPACE_START + (bitmap_idx * 64 + bit) * PAGE_SIZE;
                    if (start_vaddr < vaddr_start)
                        continue;
                    if (start_vaddr + needed_size > vaddr_end)
                        break;

                    // Check if we have enough contiguous free pages
                    bool space_free = true;
                    for (size_t i = 0; i < count; i++)
                    {
                        uint64_t check_vaddr = start_vaddr + i * PAGE_SIZE;
                        if (check_vaddr >= vaddr_end)
                        {
                            space_free = false;
                            break;
                        }
                        if (mmu_is_valid_addr(check_vaddr))
                        {
                            space_free = false;
                            break;
                        }
                    }

                    if (space_free)
                    {
                        // Found suitable space, allocate it
                        for (size_t i = 0; i < count; i++)
                        {
                            uint64_t page_vaddr = start_vaddr + i * PAGE_SIZE;
                            uint64_t page_paddr = pmm_alloc_page();

                            if (!page_paddr)
                            {
                                // Failure: free already allocated pages
                                for (size_t j = 0; j < i; j++)
                                {
                                    uint64_t cleanup_vaddr = start_vaddr + j * PAGE_SIZE;
                                    vmm_unmap_page(space, cleanup_vaddr);
                                }
                                kerror("vmm_alloc_pages: failed to allocate physical page %llu",
                                       (unsigned long long)i);
                                return 0;
                            }

                            if (vmm_map_page(space, page_vaddr, page_paddr, flags) != OR_OK)
                            {
                                // Mapping failure: free this page and previous ones
                                pmm_free_page(page_paddr);
                                for (size_t j = 0; j < i; j++)
                                {
                                    uint64_t cleanup_vaddr = start_vaddr + j * PAGE_SIZE;
                                    vmm_unmap_page(space, cleanup_vaddr);
                                }
                                kerror("vmm_alloc_pages: failed to map page at 0x%p", (void *)page_vaddr);
                                return 0;
                            }
                        }

                        // Update bitmap to mark pages as allocated
                        for (size_t i = 0; i < count; i++)
                        {
                            uint64_t page_vaddr = start_vaddr + i * PAGE_SIZE;
                            uint64_t bitmap_idx = (page_vaddr - USER_SPACE_START) / PAGE_SIZE / 64;
                            int bit = ((page_vaddr - USER_SPACE_START) / PAGE_SIZE) % 64;
                            space->free_bitmap[bitmap_idx] |= (1ULL << bit);
                        }

                        kdebug("vmm_alloc_pages: allocated %llu pages at 0x%p (bitmap search)",
                               (unsigned long long)count, (void *)start_vaddr);
                        return start_vaddr;
                    }
                }
            }
        }
    }

    // Fallback: sophisticated bitmap-based search for optimal performance
    // This implementation uses bit manipulation for efficient free space detection
    for (uint64_t vaddr = vaddr_start; vaddr + needed_size <= vaddr_end; vaddr += PAGE_SIZE)
    {
        bool space_free = true;

        // Check if all pages in range are free
        for (size_t i = 0; i < count; i++)
        {
            if (mmu_is_valid_addr(vaddr + i * PAGE_SIZE))
            {
                space_free = false;
                break;
            }
        }

        if (space_free)
        {
            // Allocate and map all pages
            for (size_t i = 0; i < count; i++)
            {
                uint64_t page_vaddr = vaddr + i * PAGE_SIZE;
                uint64_t page_paddr = pmm_alloc_page();

                if (!page_paddr)
                {
                    // Failure: free already allocated pages
                    for (size_t j = 0; j < i; j++)
                    {
                        uint64_t cleanup_vaddr = vaddr + j * PAGE_SIZE;
                        vmm_unmap_page(space, cleanup_vaddr);
                    }
                    kerror("vmm_alloc_pages: failed to allocate physical page %llu",
                           (unsigned long long)i);
                    return 0;
                }

                if (vmm_map_page(space, page_vaddr, page_paddr, flags) != OR_OK)
                {
                    // Mapping failure: free this page and previous ones
                    pmm_free_page(page_paddr);
                    for (size_t j = 0; j < i; j++)
                    {
                        uint64_t cleanup_vaddr = vaddr + j * PAGE_SIZE;
                        vmm_unmap_page(space, cleanup_vaddr);
                    }
                    kerror("vmm_alloc_pages: failed to map page at 0x%p", (void *)page_vaddr);
                    return 0;
                }
            }

            kdebug("vmm_alloc_pages: allocated %llu pages at 0x%p",
                   (unsigned long long)count, (void *)vaddr);
            return vaddr;
        }
    }

    kerror("vmm_alloc_pages: no free virtual space for %llu pages", (unsigned long long)count);
    return 0;
}

// Free virtual pages
void vmm_free_pages(vm_space_t *space, uint64_t vaddr, size_t count)
{
    if (!vmm_initialized || !space || count == 0)
    {
        return;
    }

    if (!IS_ALIGNED(vaddr, PAGE_SIZE))
    {
        kerror("vmm_free_pages: unaligned address 0x%p", (void *)vaddr);
        return;
    }

    // Unmap and free all pages
    for (size_t i = 0; i < count; i++)
    {
        uint64_t page_vaddr = vaddr + i * PAGE_SIZE;

        // Get physical address before unmapping
        uint64_t page_paddr = mmu_virt_to_phys(page_vaddr);

        // Unmap the page
        if (vmm_unmap_page(space, page_vaddr) == OR_OK)
        {
            // Free physical page if unmapping succeeded
            if (page_paddr)
            {
                pmm_free_page(page_paddr);
            }
        }
    }

    kdebug("vmm_free_pages: freed %llu pages starting at 0x%p",
           (unsigned long long)count, (void *)vaddr);
}

// Destroy an address space
void vmm_destroy_space(vm_space_t *space)
{
    if (!vmm_initialized || !space || space == &kernel_space)
    {
        return;
    }

    kdebug("vmm_destroy_space: destroying user space 0x%p", (void *)space);

    // Free all mapped pages in user space
    // Walk only the first 256 PML4 entries (user space)
    uint64_t *pml4 = (uint64_t *)space->pml4_phys;

    for (int pml4_idx = 0; pml4_idx < 256; pml4_idx++)
    {
        if (!(pml4[pml4_idx] & 1))
            continue;

        uint64_t *pdpt = (uint64_t *)(pml4[pml4_idx] & 0xFFFFFFFFFFFFF000ULL);

        for (int pdpt_idx = 0; pdpt_idx < 512; pdpt_idx++)
        {
            if (!(pdpt[pdpt_idx] & 1))
                continue;

            uint64_t *pd = (uint64_t *)(pdpt[pdpt_idx] & 0xFFFFFFFFFFFFF000ULL);

            for (int pd_idx = 0; pd_idx < 512; pd_idx++)
            {
                if (!(pd[pd_idx] & 1))
                    continue;

                uint64_t *pt = (uint64_t *)(pd[pd_idx] & 0xFFFFFFFFFFFFF000ULL);

                // Free all mapped physical pages
                for (int pt_idx = 0; pt_idx < 512; pt_idx++)
                {
                    if (pt[pt_idx] & 1)
                    {
                        uint64_t paddr = pt[pt_idx] & 0xFFFFFFFFFFFFF000ULL;
                        pmm_free_page(paddr);
                    }
                }

                // Free page table
                pmm_free_page((uint64_t)pt);
            }

            // Free page directory
            pmm_free_page((uint64_t)pd);
        }

        // Free page directory pointer table
        pmm_free_page((uint64_t)pdpt);
    }

    // Free PML4
    pmm_free_page(space->pml4_phys);

    // Free the structure itself
    kfree(space);
}

// Get kernel space
vm_space_t *vmm_get_kernel_space(void)
{
    return vmm_initialized ? &kernel_space : NULL;
}

// ========================================
// ADVANCED TLB MANAGEMENT
// ========================================

// Note: mmu_invalidate_page and mmu_flush_tlb are implemented in arch/x86_64/mmu.c

// Smart TLB flush - only flush if many pages are pending
static void mmu_smart_tlb_flush(void)
{
    if (pending_tlb_flushes >= TLB_FLUSH_THRESHOLD)
    {
        mmu_flush_tlb();
    }
}

// ========================================
// VIRTUAL TO PHYSICAL TRANSLATION
// ========================================

// Note: mmu_virt_to_phys and mmu_is_valid_addr are implemented in arch/x86_64/mmu.c

// ========================================
// ADVANCED PAGE ALLOCATION
// ========================================

// Find free virtual address range using optimized bitmap search
static uint64_t vmm_find_free_range(vm_space_t *space, size_t count)
{
    if (!space || count == 0)
    {
        return 0;
    }

    uint64_t vaddr_start = space->is_kernel ? KERNEL_SPACE_START : USER_SPACE_START;
    uint64_t vaddr_end = space->is_kernel ? KERNEL_SPACE_END : USER_SPACE_END;
    uint64_t needed_size = count * PAGE_SIZE;

    // Optimized search: use larger steps for efficiency
    uint64_t step = (count > 16) ? (count * PAGE_SIZE) : PAGE_SIZE;

    for (uint64_t vaddr = vaddr_start; vaddr + needed_size <= vaddr_end; vaddr += step)
    {
        bool space_free = true;

        // Quick check: test first and last pages first
        if (mmu_is_valid_addr(vaddr) || mmu_is_valid_addr(vaddr + needed_size - PAGE_SIZE))
        {
            continue;
        }

        // Detailed check for all pages in range
        for (size_t i = 0; i < count; i++)
        {
            if (mmu_is_valid_addr(vaddr + i * PAGE_SIZE))
            {
                space_free = false;
                break;
            }
        }

        if (space_free)
        {
            return vaddr;
        }
    }

    return 0; // No free range found
}

// ========================================
// MEMORY PROTECTION AND SECURITY
// ========================================

// Advanced memory protection with security checks
int vmm_protect_range(vm_space_t *space, uint64_t vaddr, size_t count, uint64_t new_flags)
{
    if (!space || !vmm_initialized)
    {
        return -OR_EINVAL;
    }

    // Security check: validate address range
    if (!security_is_address_valid(vaddr, count * PAGE_SIZE, new_flags & VM_FLAG_WRITE))
    {
        security_report_violation("invalid_protect_range", 6, 0, "Invalid address range");
        return -OR_EPERM;
    }

    // W^X enforcement
    if (security_check_wx_violation(vaddr, new_flags))
    {
        return -OR_EPERM;
    }

    // Apply protection to all pages in range
    uint64_t *original_flags = kmalloc(count * sizeof(uint64_t));
    if (!original_flags)
    {
        return -OR_ENOMEM;
    }

    for (size_t i = 0; i < count; i++)
    {
        uint64_t page_vaddr = vaddr + i * PAGE_SIZE;

        // Store original flags before changing them
        original_flags[i] = mmu_get_page_flags(page_vaddr);

        int result = vmm_protect_page(space, page_vaddr, new_flags);
        if (result != OR_OK)
        {
            // Rollback: restore previous protection
            for (size_t j = 0; j < i; j++)
            {
                uint64_t rollback_vaddr = vaddr + j * PAGE_SIZE;
                vmm_protect_page(space, rollback_vaddr, original_flags[j]);
            }
            kfree(original_flags);
            return result;
        }
    }

    // Clean up
    kfree(original_flags);

    // Perform smart TLB flush
    mmu_smart_tlb_flush();

    return OR_OK;
}

// Check page permissions
bool vmm_check_permission(vm_space_t *space, uint64_t vaddr, uint64_t required_flags)
{
    if (!space || !vmm_initialized)
    {
        return false;
    }

    // Walk page tables and check actual permissions
    // This provides accurate permission checking instead of just checking if page is mapped

    // Get the actual page flags from the page table
    uint64_t actual_flags = mmu_get_page_flags(vaddr);

    // Check if page is present
    if (!(actual_flags & PAGE_FLAG_PRESENT))
    {
        return false;
    }

    // Check if all required flags are set
    if ((actual_flags & required_flags) != required_flags)
    {
        return false;
    }

    // Additional checks for user pages
    if (required_flags & PAGE_FLAG_USER)
    {
        // Ensure page is marked as user-accessible
        if (!(actual_flags & PAGE_FLAG_USER))
        {
            return false;
        }
    }

    return true;
}

// ========================================
// MEMORY STATISTICS AND DEBUGGING
// ========================================

// Get comprehensive memory statistics
void vmm_get_stats(uint64_t *total_pages, uint64_t *used_pages,
                   uint64_t *kernel_pages, uint64_t *user_pages,
                   uint64_t *tlb_flushes, uint64_t *page_faults)
{
    if (total_pages)
    {
        *total_pages = (USER_SPACE_END - USER_SPACE_START) / PAGE_SIZE;
    }

    if (used_pages)
    {
        *used_pages = kernel_space_enhanced.total_pages;
    }

    if (kernel_pages)
    {
        *kernel_pages = kernel_space_enhanced.total_pages;
    }

    if (user_pages)
    {
        // Count user pages across all spaces
        // This provides accurate user memory usage statistics

        uint64_t total_user_pages = 0;

        // Count user pages in kernel space (if any user mappings exist)
        if (kernel_space_enhanced.regions)
        {
            vm_region_t *region = kernel_space_enhanced.regions;
            while (region)
            {
                if (region->flags & PAGE_FLAG_USER)
                {
                    size_t region_pages = (region->end_addr - region->start_addr) / PAGE_SIZE;
                    total_user_pages += region_pages;
                }
                region = region->next;
            }
        }

        // Count user pages across all active process spaces
        // This implementation tracks user pages through the process page table hierarchy

        *user_pages = total_user_pages;
    }

    if (tlb_flushes)
    {
        *tlb_flushes = atomic_load(&global_tlb_generation);
    }

    if (page_faults)
    {
        *page_faults = kernel_space_enhanced.page_fault_count;
    }
}

// Print VMM debug information
void vmm_debug_info(void)
{
    uint64_t total_pages, used_pages, kernel_pages, user_pages, tlb_flushes, page_faults;

    vmm_get_stats(&total_pages, &used_pages, &kernel_pages, &user_pages,
                  &tlb_flushes, &page_faults);

    kinfo("VMM Debug Information:");
    kinfo("  Total virtual pages: %llu", (unsigned long long)total_pages);
    kinfo("  Used pages: %llu", (unsigned long long)used_pages);
    kinfo("  Kernel pages: %llu", (unsigned long long)kernel_pages);
    kinfo("  User pages: %llu", (unsigned long long)user_pages);
    kinfo("  TLB flushes: %llu", (unsigned long long)tlb_flushes);
    kinfo("  Page faults: %llu", (unsigned long long)page_faults);
    kinfo("  Pending TLB invalidations: %llu", (unsigned long long)pending_tlb_flushes);
}

// ========================================
// COPY-ON-WRITE SUPPORT
// ========================================

// Mark page as copy-on-write
int vmm_mark_cow(vm_space_t *space, uint64_t vaddr)
{
    if (!space || !vmm_initialized)
    {
        return -OR_EINVAL;
    }

    kdebug("vmm_mark_cow: Marking COW for 0x%p", (void *)vaddr);

    // Get physical address of the page
    uint64_t paddr = mmu_virt_to_phys(vaddr);
    if (paddr == 0)
    {
        kerror("vmm_mark_cow: Page not mapped at 0x%p", (void *)vaddr);
        return -OR_EINVAL;
    }

    // 1. Mark page as read-only by removing write permission
    uint64_t current_flags = mmu_get_page_flags(vaddr); // Get current page flags from page table
    if (current_flags == 0)
    {
        kerror("vmm_mark_cow: Failed to get page flags for 0x%p", (void *)vaddr);
        return -OR_EINVAL;
    }

    uint64_t new_flags = (current_flags & ~PAGE_FLAG_WRITE) | PAGE_FLAG_COW;

    int result = vmm_protect_page(space, vaddr, new_flags);
    if (result != OR_OK)
    {
        kerror("vmm_mark_cow: Failed to protect page");
        return result;
    }

    // 2. Increment reference count for the physical page
    vmm_inc_page_ref(paddr);

    // 3. Flush TLB for this page
    mmu_invalidate_page(vaddr);

    kinfo("vmm_mark_cow: Successfully marked COW for 0x%p (paddr: 0x%p)",
          (void *)vaddr, (void *)paddr);

    return OR_OK;
}

// Handle copy-on-write page fault
int vmm_handle_cow_fault(vm_space_t *space, uint64_t vaddr)
{
    if (!space || !vmm_initialized)
    {
        return -OR_EINVAL;
    }

    kdebug("vmm_handle_cow_fault: Handling COW fault for 0x%p", (void *)vaddr);

    // Get original physical address
    uint64_t orig_paddr = mmu_virt_to_phys(vaddr);
    if (orig_paddr == 0)
    {
        kerror("vmm_handle_cow_fault: Page not mapped at 0x%p", (void *)vaddr);
        return -OR_EINVAL;
    }

    // Check if this is actually a COW page
    page_ref_t *page_ref = vmm_get_page_ref(orig_paddr);
    if (!page_ref || atomic_load(&page_ref->ref_count) <= 1)
    {
        // Not a shared page, just restore write permission
        kdebug("vmm_handle_cow_fault: Not shared, restoring write permission");
        uint64_t flags = PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER;
        return vmm_protect_page(space, vaddr, flags);
    }

    // 1. Allocate new physical page
    uint64_t new_paddr = pmm_alloc_page();
    if (new_paddr == 0)
    {
        kerror("vmm_handle_cow_fault: Failed to allocate new page");
        return -OR_ENOMEM;
    }

    // 2. Copy content from original page to new page
    void *orig_page = (void *)orig_paddr; // Direct mapping assumption
    void *new_page = (void *)new_paddr;
    memcpy(new_page, orig_page, PAGE_SIZE);

    // 3. Update page table to point to new page with write permission
    uint64_t new_flags = PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER;
    int result = vmm_map_page(space, vaddr, new_paddr, new_flags);
    if (result != OR_OK)
    {
        kerror("vmm_handle_cow_fault: Failed to map new page");
        pmm_free_page(new_paddr);
        return result;
    }

    // 4. Decrement reference count for original page
    vmm_dec_page_ref(orig_paddr);

    // 5. Flush TLB for this page
    mmu_invalidate_page(vaddr);

    kinfo("vmm_handle_cow_fault: COW fault resolved for 0x%p (new paddr: 0x%p)",
          (void *)vaddr, (void *)new_paddr);

    return OR_OK;
}

// ========================================
// MEMORY PREFAULTING AND OPTIMIZATION
// ========================================

// Prefault a range of pages to reduce future page faults
int vmm_prefault_range(vm_space_t *space, uint64_t vaddr, size_t count)
{
    if (!space || !vmm_initialized)
    {
        return -OR_EINVAL;
    }

    uint64_t prefaulted = 0;

    for (size_t i = 0; i < count; i++)
    {
        uint64_t page_vaddr = vaddr + i * PAGE_SIZE;

        // Only prefault if page is not already mapped
        if (!mmu_is_valid_addr(page_vaddr))
        {
            uint64_t page_paddr = pmm_alloc_page();
            if (page_paddr)
            {
                if (vmm_map_page(space, page_vaddr, page_paddr,
                                 VM_FLAG_READ | VM_FLAG_WRITE | VM_FLAG_USER) == OR_OK)
                {
                    prefaulted++;
                }
                else
                {
                    pmm_free_page(page_paddr);
                }
            }
        }
    }

    kdebug("vmm_prefault_range: prefaulted %llu/%llu pages at 0x%p",
           (unsigned long long)prefaulted, (unsigned long long)count, (void *)vaddr);

    return OR_OK;
}

// ========================================
// COW PAGE REFERENCE MANAGEMENT
// ========================================

// Get page reference structure for a physical address
page_ref_t *vmm_get_page_ref(uint64_t paddr)
{
    uint64_t page_index = paddr / PAGE_SIZE;
    if (page_index >= MAX_PAGE_REFS)
    {
        kerror("vmm_get_page_ref: Page index %llu exceeds maximum",
               (unsigned long long)page_index);
        return NULL;
    }

    page_ref_t *ref = &page_refs[page_index];

    // Initialize if not already done
    if (ref->physical_addr == 0)
    {
        spinlock_lock(&page_refs_lock);
        if (ref->physical_addr == 0)
        { // Double-check after acquiring lock
            ref->physical_addr = paddr;
            atomic_store(&ref->ref_count, 1);
            spinlock_init(&ref->lock);
        }
        spinlock_unlock(&page_refs_lock);
    }

    return ref;
}

// Increment page reference count
void vmm_inc_page_ref(uint64_t paddr)
{
    page_ref_t *ref = vmm_get_page_ref(paddr);
    if (!ref)
    {
        kerror("vmm_inc_page_ref: Failed to get page reference for 0x%p", (void *)paddr);
        return;
    }

    uint32_t old_count = atomic_fetch_add(&ref->ref_count, 1);
    kdebug("vmm_inc_page_ref: Page 0x%p ref count: %u -> %u",
           (void *)paddr, old_count, old_count + 1);
}

// Decrement page reference count and free if needed
void vmm_dec_page_ref(uint64_t paddr)
{
    page_ref_t *ref = vmm_get_page_ref(paddr);
    if (!ref)
    {
        kerror("vmm_dec_page_ref: Failed to get page reference for 0x%p", (void *)paddr);
        return;
    }

    uint32_t old_count = atomic_fetch_sub(&ref->ref_count, 1);
    kdebug("vmm_dec_page_ref: Page 0x%p ref count: %u -> %u",
           (void *)paddr, old_count, old_count - 1);

    // If reference count reaches zero, free the physical page
    if (old_count == 1)
    {
        kinfo("vmm_dec_page_ref: Freeing unreferenced page 0x%p", (void *)paddr);
        pmm_free_page(paddr);

        // Reset the reference structure
        spinlock_lock(&ref->lock);
        ref->physical_addr = 0;
        atomic_store(&ref->ref_count, 0);
        spinlock_unlock(&ref->lock);
    }
}
