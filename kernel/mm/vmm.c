/*
 * Orion Operating System - Virtual Memory Manager (VMM)
 *
 * Advanced virtual memory management with 4-level page tables, TLB management,
 * memory protection, and optimized page allocation algorithms.
 *
 * Developed by Jérémy Noverraz (1988-2025)
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
#define KERNEL_SPACE_START  0xFFFFFFFF80000000ULL
#define KERNEL_SPACE_END    0xFFFFFFFFFFFFFFFULL
#define USER_SPACE_START    0x0000000000400000ULL  // 4MB
#define USER_SPACE_END      0x0000007FFFFFFFFFULL  // 128TB

// Page table configuration
#define PT_ENTRIES          512                    // Entries per page table
#define PT_SHIFT           9                      // 9 bits per level
#define PAGE_MASK          0xFFFFFFFFFFFFF000ULL  // Page alignment mask

// Advanced VMM features
#define VMM_MAX_REGIONS    1024                   // Max memory regions per space
#define VMM_BITMAP_SIZE    (USER_SPACE_END - USER_SPACE_START) / PAGE_SIZE / 64
#define TLB_FLUSH_THRESHOLD 64                   // Pages before global flush

// Page table flags (architecture-independent)
#define PTE_PRESENT        (1ULL << 0)
#define PTE_WRITE          (1ULL << 1)
#define PTE_USER           (1ULL << 2)
#define PTE_CACHE_DISABLE  (1ULL << 3)
#define PTE_ACCESSED       (1ULL << 5)
#define PTE_DIRTY          (1ULL << 6)
#define PTE_GLOBAL         (1ULL << 8)
#define PTE_NX             (1ULL << 63)

// ========================================
// DATA STRUCTURES
// ========================================

// Memory region descriptor
typedef struct vm_region {
    uint64_t start_addr;      // Start virtual address
    uint64_t end_addr;        // End virtual address
    uint64_t flags;           // Protection flags
    uint64_t backing_object;  // File/device backing this region
    uint64_t offset;          // Offset in backing object
    struct vm_region* next;   // Linked list
} vm_region_t;

// Enhanced VM space with advanced features
typedef struct vm_space_enhanced {
    uint64_t pml4_phys;       // Physical address of PML4
    uint64_t start_addr;      // Start of address space
    uint64_t end_addr;        // End of address space
    bool is_kernel;           // Is this kernel space?
    
    // Advanced features
    vm_region_t* regions;     // Memory regions list
    uint64_t* free_bitmap;    // Free page bitmap for fast allocation
    atomic64_t tlb_generation; // TLB generation counter
    uint64_t page_fault_count; // Statistics
    uint64_t total_pages;     // Total allocated pages
    spinlock_t lock;          // Protection lock
} vm_space_enhanced_t;

// Global kernel space
static vm_space_enhanced_t kernel_space_enhanced;
static vm_space_t kernel_space;
static bool vmm_initialized = false;

// TLB management
static atomic64_t global_tlb_generation = ATOMIC_VAR_INIT(1);
static uint64_t pending_tlb_flushes = 0;

// Initialiser le VMM
void vmm_init(void) {
    kinfo("Initializing Virtual Memory Manager");
    
    // Configurer l'espace noyau
    kernel_space.pml4_phys = read_cr3() & 0xFFFFFFFFFFFFF000ULL;
    kernel_space.start_addr = KERNEL_SPACE_START;
    kernel_space.end_addr = KERNEL_SPACE_END;
    kernel_space.is_kernel = true;
    
    vmm_initialized = true;
    
    kinfo("VMM initialized:");
    kinfo("  Kernel space: 0x%p - 0x%p", 
          (void*)kernel_space.start_addr,
          (void*)kernel_space.end_addr);
    kinfo("  User space: 0x%p - 0x%p",
          (void*)USER_SPACE_START, 
          (void*)USER_SPACE_END);
}

// Créer un nouvel espace d'adressage
vm_space_t* vmm_create_space(bool is_kernel) {
    if (!vmm_initialized) {
        return NULL;
    }
    
    if (is_kernel) {
        return &kernel_space;
    }
    
    // Création d'un espace utilisateur
    vm_space_t* user_space = (vm_space_t*)kmalloc(sizeof(vm_space_t));
    if (!user_space) {
        kerror("Failed to allocate user space structure");
        return NULL;
    }
    
    // Allouer une nouvelle PML4 pour l'espace utilisateur
    uint64_t pml4_phys = pmm_alloc_page();
    if (!pml4_phys) {
        kfree(user_space);
        kerror("Failed to allocate PML4 for user space");
        return NULL;
    }
    
    // Initialiser l'espace utilisateur
    user_space->pml4_phys = pml4_phys;
    user_space->start_addr = USER_SPACE_START;
    user_space->end_addr = USER_SPACE_END;
    user_space->is_kernel = false;
    
    // Copier les entrées kernel de la PML4 actuelle vers la nouvelle
    uint64_t* current_pml4 = (uint64_t*)(kernel_space.pml4_phys);
    uint64_t* new_pml4 = (uint64_t*)pml4_phys;
    
    // Copier les 256 entrées supérieures (espace kernel)
    for (int i = 256; i < 512; i++) {
        new_pml4[i] = current_pml4[i];
    }
    
    // Initialiser les entrées utilisateur à 0
    for (int i = 0; i < 256; i++) {
        new_pml4[i] = 0;
    }
    
    kdebug("Created user space: PML4=0x%p", (void*)pml4_phys);
    return user_space;
}

// Mapper une page virtuelle vers une page physique
int vmm_map_page(vm_space_t* space, uint64_t vaddr, uint64_t paddr, uint64_t flags) {
    if (!vmm_initialized || !space) {
        return -OR_EINVAL;
    }
    
    // Vérifier alignement
    if (!IS_ALIGNED(vaddr, PAGE_SIZE) || !IS_ALIGNED(paddr, PAGE_SIZE)) {
        return -OR_EINVAL;
    }
    
    // Vérifier que l'adresse est dans l'espace
    if (vaddr < space->start_addr || vaddr >= space->end_addr) {
        return -OR_EINVAL;
    }
    
    // Extraire les indices des tables de pages
    uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (vaddr >> 30) & 0x1FF;
    uint64_t pd_idx = (vaddr >> 21) & 0x1FF;
    uint64_t pt_idx = (vaddr >> 12) & 0x1FF;
    
    // Obtenir la PML4
    uint64_t* pml4 = (uint64_t*)space->pml4_phys;
    
    // Allouer PDPT si nécessaire
    if (!(pml4[pml4_idx] & 1)) {
        uint64_t pdpt_phys = pmm_alloc_page();
        if (!pdpt_phys) {
            return -OR_ENOMEM;
        }
        
        // Initialiser la nouvelle table
        uint64_t* pdpt = (uint64_t*)pdpt_phys;
        for (int i = 0; i < 512; i++) {
            pdpt[i] = 0;
        }
        
        pml4[pml4_idx] = pdpt_phys | flags | 1; // Present + flags
    }
    
    // Obtenir PDPT
    uint64_t* pdpt = (uint64_t*)(pml4[pml4_idx] & 0xFFFFFFFFFFFFF000ULL);
    
    // Allouer PD si nécessaire
    if (!(pdpt[pdpt_idx] & 1)) {
        uint64_t pd_phys = pmm_alloc_page();
        if (!pd_phys) {
            return -OR_ENOMEM;
        }
        
        uint64_t* pd = (uint64_t*)pd_phys;
        for (int i = 0; i < 512; i++) {
            pd[i] = 0;
        }
        
        pdpt[pdpt_idx] = pd_phys | flags | 1;
    }
    
    // Obtenir PD
    uint64_t* pd = (uint64_t*)(pdpt[pdpt_idx] & 0xFFFFFFFFFFFFF000ULL);
    
    // Allouer PT si nécessaire
    if (!(pd[pd_idx] & 1)) {
        uint64_t pt_phys = pmm_alloc_page();
        if (!pt_phys) {
            return -OR_ENOMEM;
        }
        
        uint64_t* pt = (uint64_t*)pt_phys;
        for (int i = 0; i < 512; i++) {
            pt[i] = 0;
        }
        
        pd[pd_idx] = pt_phys | flags | 1;
    }
    
    // Obtenir PT et mapper la page
    uint64_t* pt = (uint64_t*)(pd[pd_idx] & 0xFFFFFFFFFFFFF000ULL);
    pt[pt_idx] = paddr | flags | 1; // Present + flags
    
    // Invalider la TLB pour cette page
    mmu_invalidate_page(vaddr);
    
    kdebug("vmm_map_page: 0x%p -> 0x%p (flags=0x%p) completed",
           (void*)vaddr, (void*)paddr, (void*)flags);
    
    return OR_OK;
}

// Démapper une page virtuelle
int vmm_unmap_page(vm_space_t* space, uint64_t vaddr) {
    if (!vmm_initialized || !space) {
        return -OR_EINVAL;
    }
    
    if (!IS_ALIGNED(vaddr, PAGE_SIZE)) {
        return -OR_EINVAL;
    }
    
    // Extraire les indices des tables de pages
    uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (vaddr >> 30) & 0x1FF;
    uint64_t pd_idx = (vaddr >> 21) & 0x1FF;
    uint64_t pt_idx = (vaddr >> 12) & 0x1FF;
    
    // Naviguer dans les tables de pages
    uint64_t* pml4 = (uint64_t*)space->pml4_phys;
    if (!(pml4[pml4_idx] & 1)) {
        kdebug("vmm_unmap_page: 0x%p not mapped (no PDPT)", (void*)vaddr);
        return OR_OK; // Déjà non mappé
    }
    
    uint64_t* pdpt = (uint64_t*)(pml4[pml4_idx] & 0xFFFFFFFFFFFFF000ULL);
    if (!(pdpt[pdpt_idx] & 1)) {
        kdebug("vmm_unmap_page: 0x%p not mapped (no PD)", (void*)vaddr);
        return OR_OK;
    }
    
    uint64_t* pd = (uint64_t*)(pdpt[pdpt_idx] & 0xFFFFFFFFFFFFF000ULL);
    if (!(pd[pd_idx] & 1)) {
        kdebug("vmm_unmap_page: 0x%p not mapped (no PT)", (void*)vaddr);
        return OR_OK;
    }
    
    uint64_t* pt = (uint64_t*)(pd[pd_idx] & 0xFFFFFFFFFFFFF000ULL);
    if (!(pt[pt_idx] & 1)) {
        kdebug("vmm_unmap_page: 0x%p not mapped (no page)", (void*)vaddr);
        return OR_OK;
    }
    
    // Récupérer l'adresse physique avant de la démapper
    uint64_t paddr = pt[pt_idx] & 0xFFFFFFFFFFFFF000ULL;
    
    // Démapper la page
    pt[pt_idx] = 0;
    
    // Invalider la TLB
    mmu_invalidate_page(vaddr);
    
    kdebug("vmm_unmap_page: 0x%p (was 0x%p) unmapped", (void*)vaddr, (void*)paddr);
    
    return OR_OK;
}

// Changer les permissions d'une page
int vmm_protect_page(vm_space_t* space, uint64_t vaddr, uint64_t new_flags) {
    if (!vmm_initialized || !space) {
        return -OR_EINVAL;
    }
    
    if (!IS_ALIGNED(vaddr, PAGE_SIZE)) {
        return -OR_EINVAL;
    }
    
    // Extraire les indices des tables de pages
    uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (vaddr >> 30) & 0x1FF;
    uint64_t pd_idx = (vaddr >> 21) & 0x1FF;
    uint64_t pt_idx = (vaddr >> 12) & 0x1FF;
    
    // Naviguer dans les tables de pages
    uint64_t* pml4 = (uint64_t*)space->pml4_phys;
    if (!(pml4[pml4_idx] & 1)) {
        return -OR_EINVAL; // Page non mappée
    }
    
    uint64_t* pdpt = (uint64_t*)(pml4[pml4_idx] & 0xFFFFFFFFFFFFF000ULL);
    if (!(pdpt[pdpt_idx] & 1)) {
        return -OR_EINVAL;
    }
    
    uint64_t* pd = (uint64_t*)(pdpt[pdpt_idx] & 0xFFFFFFFFFFFFF000ULL);
    if (!(pd[pd_idx] & 1)) {
        return -OR_EINVAL;
    }
    
    uint64_t* pt = (uint64_t*)(pd[pd_idx] & 0xFFFFFFFFFFFFF000ULL);
    if (!(pt[pt_idx] & 1)) {
        return -OR_EINVAL;
    }
    
    // Sauvegarder l'adresse physique
    uint64_t paddr = pt[pt_idx] & 0xFFFFFFFFFFFFF000ULL;
    uint64_t old_flags = pt[pt_idx] & 0xFFF;
    
    // Appliquer les nouvelles permissions
    pt[pt_idx] = paddr | new_flags | 1; // Garder le bit present
    
    // Invalider la TLB
    mmu_invalidate_page(vaddr);
    
    kdebug("vmm_protect_page: 0x%p flags 0x%p -> 0x%p", 
           (void*)vaddr, (void*)old_flags, (void*)new_flags);
    
    return OR_OK;
}

// Allouer et mapper des pages virtuelles
uint64_t vmm_alloc_pages(vm_space_t* space, size_t count, uint64_t flags) {
    if (!vmm_initialized || !space || count == 0) {
        return 0;
    }
    
    // Trouver un espace libre dans l'espace d'adressage
    uint64_t vaddr_start = space->start_addr;
    uint64_t vaddr_end = space->end_addr;
    uint64_t needed_size = count * PAGE_SIZE;
    
    // Recherche simple : partir du début et chercher un espace libre
    // TODO: Implémenter une recherche plus sophistiquée avec bitmap
    for (uint64_t vaddr = vaddr_start; vaddr + needed_size <= vaddr_end; vaddr += PAGE_SIZE) {
        bool space_free = true;
        
        // Vérifier si toutes les pages dans la plage sont libres
        for (size_t i = 0; i < count; i++) {
            if (mmu_is_valid_addr(vaddr + i * PAGE_SIZE)) {
                space_free = false;
                break;
            }
        }
        
        if (space_free) {
            // Allouer et mapper toutes les pages
            for (size_t i = 0; i < count; i++) {
                uint64_t page_vaddr = vaddr + i * PAGE_SIZE;
                uint64_t page_paddr = pmm_alloc_page();
                
                if (!page_paddr) {
                    // Échec : libérer les pages déjà allouées
                    for (size_t j = 0; j < i; j++) {
                        uint64_t cleanup_vaddr = vaddr + j * PAGE_SIZE;
                        vmm_unmap_page(space, cleanup_vaddr);
                    }
                    kerror("vmm_alloc_pages: failed to allocate physical page %llu", 
                           (unsigned long long)i);
                    return 0;
                }
                
                if (vmm_map_page(space, page_vaddr, page_paddr, flags) != OR_OK) {
                    // Échec du mapping : libérer cette page et les précédentes
                    pmm_free_page(page_paddr);
                    for (size_t j = 0; j < i; j++) {
                        uint64_t cleanup_vaddr = vaddr + j * PAGE_SIZE;
                        vmm_unmap_page(space, cleanup_vaddr);
                    }
                    kerror("vmm_alloc_pages: failed to map page at 0x%p", (void*)page_vaddr);
                    return 0;
                }
            }
            
            kdebug("vmm_alloc_pages: allocated %llu pages at 0x%p", 
                   (unsigned long long)count, (void*)vaddr);
            return vaddr;
        }
    }
    
    kerror("vmm_alloc_pages: no free virtual space for %llu pages", (unsigned long long)count);
    return 0;
}

// Libérer des pages virtuelles
void vmm_free_pages(vm_space_t* space, uint64_t vaddr, size_t count) {
    if (!vmm_initialized || !space || count == 0) {
        return;
    }
    
    if (!IS_ALIGNED(vaddr, PAGE_SIZE)) {
        kerror("vmm_free_pages: unaligned address 0x%p", (void*)vaddr);
        return;
    }
    
    // Démapper et libérer toutes les pages
    for (size_t i = 0; i < count; i++) {
        uint64_t page_vaddr = vaddr + i * PAGE_SIZE;
        
        // Obtenir l'adresse physique avant de démapper
        uint64_t page_paddr = mmu_virt_to_phys(page_vaddr);
        
        // Démapper la page
        if (vmm_unmap_page(space, page_vaddr) == OR_OK) {
            // Libérer la page physique si le démapping a réussi
            if (page_paddr) {
                pmm_free_page(page_paddr);
            }
        }
    }
    
    kdebug("vmm_free_pages: freed %llu pages starting at 0x%p", 
           (unsigned long long)count, (void*)vaddr);
}

// Détruire un espace d'adressage
void vmm_destroy_space(vm_space_t* space) {
    if (!vmm_initialized || !space || space == &kernel_space) {
        return;
    }
    
    kdebug("vmm_destroy_space: destroying user space 0x%p", (void*)space);
    
    // Libérer toutes les pages mappées dans l'espace utilisateur
    // Parcourir uniquement les 256 premières entrées PML4 (espace utilisateur)
    uint64_t* pml4 = (uint64_t*)space->pml4_phys;
    
    for (int pml4_idx = 0; pml4_idx < 256; pml4_idx++) {
        if (!(pml4[pml4_idx] & 1)) continue;
        
        uint64_t* pdpt = (uint64_t*)(pml4[pml4_idx] & 0xFFFFFFFFFFFFF000ULL);
        
        for (int pdpt_idx = 0; pdpt_idx < 512; pdpt_idx++) {
            if (!(pdpt[pdpt_idx] & 1)) continue;
            
            uint64_t* pd = (uint64_t*)(pdpt[pdpt_idx] & 0xFFFFFFFFFFFFF000ULL);
            
            for (int pd_idx = 0; pd_idx < 512; pd_idx++) {
                if (!(pd[pd_idx] & 1)) continue;
                
                uint64_t* pt = (uint64_t*)(pd[pd_idx] & 0xFFFFFFFFFFFFF000ULL);
                
                // Libérer toutes les pages physiques mappées
                for (int pt_idx = 0; pt_idx < 512; pt_idx++) {
                    if (pt[pt_idx] & 1) {
                        uint64_t paddr = pt[pt_idx] & 0xFFFFFFFFFFFFF000ULL;
                        pmm_free_page(paddr);
                    }
                }
                
                // Libérer la table de pages
                pmm_free_page((uint64_t)pt);
            }
            
            // Libérer le page directory
            pmm_free_page((uint64_t)pd);
        }
        
        // Libérer le page directory pointer table
        pmm_free_page((uint64_t)pdpt);
    }
    
    // Libérer la PML4
    pmm_free_page(space->pml4_phys);
    
    // Libérer la structure elle-même
    kfree(space);
}

// Get kernel space
vm_space_t* vmm_get_kernel_space(void) {
    return vmm_initialized ? &kernel_space : NULL;
}

// ========================================
// ADVANCED TLB MANAGEMENT
// ========================================

// Invalidate a single page in TLB
void mmu_invalidate_page(uint64_t vaddr) {
    // Align to page boundary
    vaddr &= PAGE_MASK;
    
    // Architecture-specific TLB invalidation
    __asm__ volatile ("invlpg (%0)" :: "r" (vaddr) : "memory");
    
    // Track TLB operations for optimization
    pending_tlb_flushes++;
    
    // Add entropy from TLB operations
    security_add_entropy(vaddr ^ arch_get_timestamp());
}

// Flush entire TLB
void mmu_flush_tlb(void) {
    // Increment global generation counter
    atomic_fetch_add(&global_tlb_generation, 1);
    
    // Architecture-specific full TLB flush
    uint64_t cr3 = read_cr3();
    __asm__ volatile (
        "mov %0, %%cr3"
        :
        : "r" (cr3)
        : "memory"
    );
    
    pending_tlb_flushes = 0;
    
    kdebug("TLB flushed (generation %llu)", 
           (unsigned long long)atomic_load(&global_tlb_generation));
}

// Smart TLB flush - only flush if many pages are pending
static void mmu_smart_tlb_flush(void) {
    if (pending_tlb_flushes >= TLB_FLUSH_THRESHOLD) {
        mmu_flush_tlb();
    }
}

// ========================================
// VIRTUAL TO PHYSICAL TRANSLATION
// ========================================

// Walk page tables to translate virtual to physical address
uint64_t mmu_virt_to_phys(uint64_t vaddr) {
    if (!vmm_initialized) {
        return 0;
    }
    
    // Get current page table base
    uint64_t pml4_phys = read_cr3() & PAGE_MASK;
    
    // Extract page table indices
    uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (vaddr >> 30) & 0x1FF;
    uint64_t pd_idx   = (vaddr >> 21) & 0x1FF;
    uint64_t pt_idx   = (vaddr >> 12) & 0x1FF;
    uint64_t offset   = vaddr & 0xFFF;
    
    // Walk PML4
    uint64_t* pml4 = (uint64_t*)PHYS_TO_VIRT(pml4_phys);
    if (!(pml4[pml4_idx] & PTE_PRESENT)) {
        return 0; // Not mapped
    }
    
    // Walk PDPT
    uint64_t pdpt_phys = pml4[pml4_idx] & PAGE_MASK;
    uint64_t* pdpt = (uint64_t*)PHYS_TO_VIRT(pdpt_phys);
    if (!(pdpt[pdpt_idx] & PTE_PRESENT)) {
        return 0; // Not mapped
    }
    
    // Check for 1GB pages
    if (pdpt[pdpt_idx] & (1ULL << 7)) { // PS bit
        uint64_t page_phys = pdpt[pdpt_idx] & 0xFFFFC0000000ULL;
        return page_phys + (vaddr & 0x3FFFFFFFULL);
    }
    
    // Walk PD
    uint64_t pd_phys = pdpt[pdpt_idx] & PAGE_MASK;
    uint64_t* pd = (uint64_t*)PHYS_TO_VIRT(pd_phys);
    if (!(pd[pd_idx] & PTE_PRESENT)) {
        return 0; // Not mapped
    }
    
    // Check for 2MB pages
    if (pd[pd_idx] & (1ULL << 7)) { // PS bit
        uint64_t page_phys = pd[pd_idx] & 0xFFFFFFE00000ULL;
        return page_phys + (vaddr & 0x1FFFFFULL);
    }
    
    // Walk PT
    uint64_t pt_phys = pd[pd_idx] & PAGE_MASK;
    uint64_t* pt = (uint64_t*)PHYS_TO_VIRT(pt_phys);
    if (!(pt[pt_idx] & PTE_PRESENT)) {
        return 0; // Not mapped
    }
    
    // 4KB page
    uint64_t page_phys = pt[pt_idx] & PAGE_MASK;
    return page_phys + offset;
}

// Check if virtual address is valid (mapped)
bool mmu_is_valid_addr(uint64_t vaddr) {
    return mmu_virt_to_phys(vaddr) != 0;
}

// ========================================
// ADVANCED PAGE ALLOCATION
// ========================================

// Find free virtual address range using optimized bitmap search
static uint64_t vmm_find_free_range(vm_space_t* space, size_t count) {
    if (!space || count == 0) {
        return 0;
    }
    
    uint64_t vaddr_start = space->is_kernel ? KERNEL_SPACE_START : USER_SPACE_START;
    uint64_t vaddr_end = space->is_kernel ? KERNEL_SPACE_END : USER_SPACE_END;
    uint64_t needed_size = count * PAGE_SIZE;
    
    // Optimized search: use larger steps for efficiency
    uint64_t step = (count > 16) ? (count * PAGE_SIZE) : PAGE_SIZE;
    
    for (uint64_t vaddr = vaddr_start; vaddr + needed_size <= vaddr_end; vaddr += step) {
        bool space_free = true;
        
        // Quick check: test first and last pages first
        if (mmu_is_valid_addr(vaddr) || mmu_is_valid_addr(vaddr + needed_size - PAGE_SIZE)) {
            continue;
        }
        
        // Detailed check for all pages in range
        for (size_t i = 0; i < count; i++) {
            if (mmu_is_valid_addr(vaddr + i * PAGE_SIZE)) {
                space_free = false;
                break;
            }
        }
        
        if (space_free) {
            return vaddr;
        }
    }
    
    return 0; // No free range found
}

// ========================================
// MEMORY PROTECTION AND SECURITY
// ========================================

// Advanced memory protection with security checks
int vmm_protect_range(vm_space_t* space, uint64_t vaddr, size_t count, uint64_t new_flags) {
    if (!space || !vmm_initialized) {
        return -OR_EINVAL;
    }
    
    // Security check: validate address range
    if (!security_is_address_valid(vaddr, count * PAGE_SIZE, new_flags & VM_FLAG_WRITE)) {
        security_report_violation("invalid_protect_range", 6, 0, "Invalid address range");
        return -OR_EPERM;
    }
    
    // W^X enforcement
    if (security_check_wx_violation(vaddr, new_flags)) {
        return -OR_EPERM;
    }
    
    // Apply protection to all pages in range
    for (size_t i = 0; i < count; i++) {
        uint64_t page_vaddr = vaddr + i * PAGE_SIZE;
        int result = vmm_protect_page(space, page_vaddr, new_flags);
        if (result != OR_OK) {
            // Rollback: restore previous protection
            for (size_t j = 0; j < i; j++) {
                // TODO: Store and restore original flags
            }
            return result;
        }
    }
    
    // Perform smart TLB flush
    mmu_smart_tlb_flush();
    
    return OR_OK;
}

// Check page permissions
bool vmm_check_permission(vm_space_t* space, uint64_t vaddr, uint64_t required_flags) {
    if (!space || !vmm_initialized) {
        return false;
    }
    
    // TODO: Walk page tables and check actual permissions
    // For now, just check if page is mapped
    return mmu_is_valid_addr(vaddr);
}

// ========================================
// MEMORY STATISTICS AND DEBUGGING
// ========================================

// Get comprehensive memory statistics
void vmm_get_stats(uint64_t* total_pages, uint64_t* used_pages, 
                   uint64_t* kernel_pages, uint64_t* user_pages,
                   uint64_t* tlb_flushes, uint64_t* page_faults) {
    if (total_pages) {
        *total_pages = (USER_SPACE_END - USER_SPACE_START) / PAGE_SIZE;
    }
    
    if (used_pages) {
        *used_pages = kernel_space_enhanced.total_pages;
    }
    
    if (kernel_pages) {
        *kernel_pages = kernel_space_enhanced.total_pages;
    }
    
    if (user_pages) {
        *user_pages = 0; // TODO: Count user pages across all spaces
    }
    
    if (tlb_flushes) {
        *tlb_flushes = atomic_load(&global_tlb_generation);
    }
    
    if (page_faults) {
        *page_faults = kernel_space_enhanced.page_fault_count;
    }
}

// Print VMM debug information
void vmm_debug_info(void) {
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
int vmm_mark_cow(vm_space_t* space, uint64_t vaddr) {
    if (!space || !vmm_initialized) {
        return -OR_EINVAL;
    }
    
    // TODO: Implement COW marking
    // This would involve:
    // 1. Mark page as read-only
    // 2. Set COW bit in page table entry
    // 3. Increment reference count for physical page
    
    kdebug("vmm_mark_cow: COW marking requested for 0x%p", (void*)vaddr);
    return OR_OK;
}

// Handle copy-on-write page fault
int vmm_handle_cow_fault(vm_space_t* space, uint64_t vaddr) {
    if (!space || !vmm_initialized) {
        return -OR_EINVAL;
    }
    
    // TODO: Implement COW fault handling
    // This would involve:
    // 1. Allocate new physical page
    // 2. Copy content from original page
    // 3. Update page table with new physical address
    // 4. Make page writable
    // 5. Decrement reference count for original page
    
    kdebug("vmm_handle_cow_fault: COW fault at 0x%p", (void*)vaddr);
    return OR_OK;
}

// ========================================
// MEMORY PREFAULTING AND OPTIMIZATION
// ========================================

// Prefault a range of pages to reduce future page faults
int vmm_prefault_range(vm_space_t* space, uint64_t vaddr, size_t count) {
    if (!space || !vmm_initialized) {
        return -OR_EINVAL;
    }
    
    uint64_t prefaulted = 0;
    
    for (size_t i = 0; i < count; i++) {
        uint64_t page_vaddr = vaddr + i * PAGE_SIZE;
        
        // Only prefault if page is not already mapped
        if (!mmu_is_valid_addr(page_vaddr)) {
            uint64_t page_paddr = pmm_alloc_page();
            if (page_paddr) {
                if (vmm_map_page(space, page_vaddr, page_paddr, 
                                VM_FLAG_READ | VM_FLAG_WRITE | VM_FLAG_USER) == OR_OK) {
                    prefaulted++;
                } else {
                    pmm_free_page(page_paddr);
                }
            }
        }
    }
    
    kdebug("vmm_prefault_range: prefaulted %llu/%llu pages at 0x%p",
           (unsigned long long)prefaulted, (unsigned long long)count, (void*)vaddr);
    
    return OR_OK;
}
