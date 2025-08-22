// x86_64 MMU management for Orion
#include <orion/kernel.h>
#include <orion/types.h>
#include <arch.h>

// Page flags
#define PAGE_PRESENT    (1ULL << 0)
#define PAGE_WRITE      (1ULL << 1)
#define PAGE_USER       (1ULL << 2)
#define PAGE_PWT        (1ULL << 3)
#define PAGE_PCD        (1ULL << 4)
#define PAGE_ACCESSED   (1ULL << 5)
#define PAGE_DIRTY      (1ULL << 6)
#define PAGE_PS         (1ULL << 7)   // Page Size (2MB/1GB)
#define PAGE_GLOBAL     (1ULL << 8)
#define PAGE_NX         (1ULL << 63)  // No Execute

// Address masks
#define ADDR_MASK       0x000FFFFFFFFFF000ULL
#define PAGE_OFFSET_MASK 0x0FFFULL

// Boot page table (identity mapping low + high mapping)
#ifdef _MSC_VER
__declspec(align(4096)) static uint64_t boot_pml4[512];
__declspec(align(4096)) static uint64_t boot_pdpt_low[512];
__declspec(align(4096)) static uint64_t boot_pdpt_high[512];
__declspec(align(4096)) static uint64_t boot_pd_low[512];
__declspec(align(4096)) static uint64_t boot_pd_high[512];
#else
static uint64_t boot_pml4[512] __attribute__((aligned(4096)));
static uint64_t boot_pdpt_low[512] __attribute__((aligned(4096)));
static uint64_t boot_pdpt_high[512] __attribute__((aligned(4096)));
static uint64_t boot_pd_low[512] __attribute__((aligned(4096)));
static uint64_t boot_pd_high[512] __attribute__((aligned(4096)));
#endif

// Initialize boot page tables
static void setup_boot_page_tables(void) {
    // Clear all tables
    for (int i = 0; i < 512; i++) {
        boot_pml4[i] = 0;
        boot_pdpt_low[i] = 0;
        boot_pdpt_high[i] = 0;
        boot_pd_low[i] = 0;
        boot_pd_high[i] = 0;
    }
    
    // PML4 entries
    // Entry 0: Identity mapping (0x0 - 0x40000000 = 1GB)
    boot_pml4[0] = ((uint64_t)boot_pdpt_low) | PAGE_PRESENT | PAGE_WRITE;
    
    // Entry 511: High mapping (0xFFFFFFFF80000000+)  
    boot_pml4[511] = ((uint64_t)boot_pdpt_high) | PAGE_PRESENT | PAGE_WRITE;
    
    // PDPT low - identity mapping
    boot_pdpt_low[0] = ((uint64_t)boot_pd_low) | PAGE_PRESENT | PAGE_WRITE;
    
    // PDPT high - kernel mapping
    boot_pdpt_high[510] = ((uint64_t)boot_pd_high) | PAGE_PRESENT | PAGE_WRITE;
    
    // PD entries - use 2MB pages for simplicity
    // Identity mapping: 0x0 - 0x40000000 (1GB)
    for (int i = 0; i < 512; i++) {
        uint64_t addr = i * 0x200000; // 2MB pages
        boot_pd_low[i] = addr | PAGE_PRESENT | PAGE_WRITE | PAGE_PS;
    }
    
    // High mapping: 0xFFFFFFFF80000000+ to 0x0+
    for (int i = 0; i < 512; i++) {
        uint64_t addr = i * 0x200000; // 2MB pages
        boot_pd_high[i] = addr | PAGE_PRESENT | PAGE_WRITE | PAGE_PS;
    }
    
    kinfo("Boot page tables configured");
    kinfo("  Identity map: 0x0 - 0x40000000");
    kinfo("  High map: 0xFFFFFFFF80000000 -> 0x0");
}

// Initialize MMU
void mmu_init(void) {
    kinfo("Initializing x86_64 MMU");
    
    // Configure boot page tables
    setup_boot_page_tables();
    
    // Load CR3 with our PML4
    uint64_t pml4_phys = (uint64_t)boot_pml4;
    write_cr3(pml4_phys);
    
    kinfo("MMU initialized with boot page tables");
    kinfo("  PML4 at: 0x%p", (void*)pml4_phys);
}

// Map a page dynamically
void mmu_map_page(uint64_t vaddr, uint64_t paddr, uint64_t flags) {
    // Get current PML4 from CR3
    uint64_t cr3 = read_cr3();
    uint64_t* pml4 = (uint64_t*)(cr3 & ADDR_MASK);
    
    // Extract page table indices from virtual address
    uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (vaddr >> 30) & 0x1FF;
    uint64_t pd_idx = (vaddr >> 21) & 0x1FF;
    uint64_t pt_idx = (vaddr >> 12) & 0x1FF;
    
    // Ensure valid indices
    if (pml4_idx >= 512 || pdpt_idx >= 512 || pd_idx >= 512 || pt_idx >= 512) {
        kerror("mmu_map_page: Invalid virtual address 0x%p", (void*)vaddr);
        return;
    }
    
    // Navigate/create page table hierarchy
    uint64_t* pdpt;
    if (!(pml4[pml4_idx] & PAGE_PRESENT)) {
        // Allocate new PDPT
        uint64_t pdpt_phys = pmm_alloc_page();
        if (!pdpt_phys) {
            kerror("mmu_map_page: Failed to allocate PDPT");
            return;
        }
        pdpt = (uint64_t*)pdpt_phys;
        // Clear the new table
        for (int i = 0; i < 512; i++) {
            pdpt[i] = 0;
        }
        pml4[pml4_idx] = pdpt_phys | PAGE_PRESENT | PAGE_WRITE | (flags & PAGE_USER);
    } else {
        pdpt = (uint64_t*)(pml4[pml4_idx] & ADDR_MASK);
    }
    
    uint64_t* pd;
    if (!(pdpt[pdpt_idx] & PAGE_PRESENT)) {
        // Allocate new PD
        uint64_t pd_phys = pmm_alloc_page();
        if (!pd_phys) {
            kerror("mmu_map_page: Failed to allocate PD");
            return;
        }
        pd = (uint64_t*)pd_phys;
        // Clear the new table
        for (int i = 0; i < 512; i++) {
            pd[i] = 0;
        }
        pdpt[pdpt_idx] = pd_phys | PAGE_PRESENT | PAGE_WRITE | (flags & PAGE_USER);
    } else {
        pd = (uint64_t*)(pdpt[pdpt_idx] & ADDR_MASK);
    }
    
    uint64_t* pt;
    if (!(pd[pd_idx] & PAGE_PRESENT)) {
        // Allocate new PT
        uint64_t pt_phys = pmm_alloc_page();
        if (!pt_phys) {
            kerror("mmu_map_page: Failed to allocate PT");
            return;
        }
        pt = (uint64_t*)pt_phys;
        // Clear the new table
        for (int i = 0; i < 512; i++) {
            pt[i] = 0;
        }
        pd[pd_idx] = pt_phys | PAGE_PRESENT | PAGE_WRITE | (flags & PAGE_USER);
    } else {
        pt = (uint64_t*)(pd[pd_idx] & ADDR_MASK);
    }
    
    // Map the actual page
    pt[pt_idx] = (paddr & ADDR_MASK) | flags | PAGE_PRESENT;
    
    // Invalidate TLB for this page
    mmu_invalidate_page(vaddr);
    
    kdebug("mmu_map_page: Mapped 0x%p -> 0x%p (flags=0x%p)",
           (void*)vaddr, (void*)paddr, (void*)flags);
}

// Unmap a page dynamically
void mmu_unmap_page(uint64_t vaddr) {
    // Get current PML4 from CR3
    uint64_t cr3 = read_cr3();
    uint64_t* pml4 = (uint64_t*)(cr3 & ADDR_MASK);
    
    // Extract page table indices from virtual address
    uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (vaddr >> 30) & 0x1FF;
    uint64_t pd_idx = (vaddr >> 21) & 0x1FF;
    uint64_t pt_idx = (vaddr >> 12) & 0x1FF;
    
    // Navigate page table hierarchy
    if (!(pml4[pml4_idx] & PAGE_PRESENT)) {
        kdebug("mmu_unmap_page: PML4 entry not present for 0x%p", (void*)vaddr);
        return;
    }
    uint64_t* pdpt = (uint64_t*)(pml4[pml4_idx] & ADDR_MASK);
    
    if (!(pdpt[pdpt_idx] & PAGE_PRESENT)) {
        kdebug("mmu_unmap_page: PDPT entry not present for 0x%p", (void*)vaddr);
        return;
    }
    uint64_t* pd = (uint64_t*)(pdpt[pdpt_idx] & ADDR_MASK);
    
    if (!(pd[pd_idx] & PAGE_PRESENT)) {
        kdebug("mmu_unmap_page: PD entry not present for 0x%p", (void*)vaddr);
        return;
    }
    uint64_t* pt = (uint64_t*)(pd[pd_idx] & ADDR_MASK);
    
    if (!(pt[pt_idx] & PAGE_PRESENT)) {
        kdebug("mmu_unmap_page: Page not mapped at 0x%p", (void*)vaddr);
        return;
    }
    
    // Unmap the page
    pt[pt_idx] = 0;
    
    // Invalidate TLB for this page
    mmu_invalidate_page(vaddr);
    
    kdebug("mmu_unmap_page: Unmapped 0x%p", (void*)vaddr);
}

    // Get physical address from virtual address
uint64_t mmu_virt_to_phys(uint64_t vaddr) {
    // For kernel addresses in high memory, simple subtraction
    if (vaddr >= 0xFFFFFFFF80000000ULL) {
        return vaddr - 0xFFFFFFFF80000000ULL;
    }
    
    // For other addresses, identity mapping for now
    if (vaddr < 0x40000000ULL) {
        return vaddr;
    }
    
    // Invalid address
    return 0;
}

// Check if a virtual address is valid
bool mmu_is_valid_addr(uint64_t vaddr) {
    // Check if it's in our known mappings
    if (vaddr < 0x40000000ULL) {
        return true; // Identity mapping
    }
    
    if (vaddr >= 0xFFFFFFFF80000000ULL && vaddr < 0xFFFFFFFFC0000000ULL) {
        return true; // Kernel high mapping
    }
    
    return false;
}

// Invalidate TLB for an address
void mmu_invalidate_page(uint64_t vaddr) {
#ifdef _MSC_VER
    // MSVC stub - will be replaced by assembly
    (void)vaddr;
#else
    __asm__ volatile ("invlpg (%0)" : : "r"(vaddr) : "memory");
#endif
}

// Complete TLB flush
void mmu_flush_tlb(void) {
    uint64_t cr3 = read_cr3();
    write_cr3(cr3); // Reloading CR3 flushes the TLB
}
