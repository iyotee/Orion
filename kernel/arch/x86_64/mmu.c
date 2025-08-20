// Gestion MMU x86_64 pour Orion
#include <orion/kernel.h>
#include <orion/types.h>
#include <arch.h>

// Flags de pages
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

// Masques d'adresses
#define ADDR_MASK       0x000FFFFFFFFFF000ULL
#define PAGE_OFFSET_MASK 0x0FFFULL

// Table de pages de boot (identity mapping bas + high mapping)
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

// Initialiser les tables de pages de boot
static void setup_boot_page_tables(void) {
    // Clear toutes les tables
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
    
    // PD entries - utiliser pages 2MB pour simplicité
    // Identity mapping: 0x0 - 0x40000000 (1GB)
    for (int i = 0; i < 512; i++) {
        uint64_t addr = i * 0x200000; // 2MB pages
        boot_pd_low[i] = addr | PAGE_PRESENT | PAGE_WRITE | PAGE_PS;
    }
    
    // High mapping: 0xFFFFFFFF80000000+ vers 0x0+
    for (int i = 0; i < 512; i++) {
        uint64_t addr = i * 0x200000; // 2MB pages
        boot_pd_high[i] = addr | PAGE_PRESENT | PAGE_WRITE | PAGE_PS;
    }
    
    kinfo("Boot page tables configured");
    kinfo("  Identity map: 0x0 - 0x40000000");
    kinfo("  High map: 0xFFFFFFFF80000000 -> 0x0");
}

// Initialiser la MMU
void mmu_init(void) {
    kinfo("Initializing x86_64 MMU");
    
    // Configurer les tables de pages de boot
    setup_boot_page_tables();
    
    // Charger CR3 avec notre PML4
    uint64_t pml4_phys = (uint64_t)boot_pml4;
    write_cr3(pml4_phys);
    
    kinfo("MMU initialized with boot page tables");
    kinfo("  PML4 at: 0x%p", (void*)pml4_phys);
}

// Mapper une page (version basique)
void mmu_map_page(uint64_t vaddr, uint64_t paddr, uint64_t flags) {
    // TODO: Implémenter mapping dynamique de pages
    // Pour l'instant, juste un stub
    (void)vaddr;
    (void)paddr; 
    (void)flags;
    
    kdebug("mmu_map_page: 0x%p -> 0x%p (flags=0x%p) - NOT IMPLEMENTED",
           (void*)vaddr, (void*)paddr, (void*)flags);
}

// Obtenir l'adresse physique d'une adresse virtuelle
uint64_t mmu_virt_to_phys(uint64_t vaddr) {
    // Pour les adresses du noyau en high memory, simple soustraction
    if (vaddr >= 0xFFFFFFFF80000000ULL) {
        return vaddr - 0xFFFFFFFF80000000ULL;
    }
    
    // Pour les autres adresses, identity mapping pour l'instant
    if (vaddr < 0x40000000ULL) {
        return vaddr;
    }
    
    // Adresse invalide
    return 0;
}

// Vérifier si une adresse virtuelle est valide
bool mmu_is_valid_addr(uint64_t vaddr) {
    // Vérifier si c'est dans nos mappings connus
    if (vaddr < 0x40000000ULL) {
        return true; // Identity mapping
    }
    
    if (vaddr >= 0xFFFFFFFF80000000ULL && vaddr < 0xFFFFFFFFC0000000ULL) {
        return true; // Kernel high mapping
    }
    
    return false;
}

// Invalidate TLB pour une adresse
void mmu_invalidate_page(uint64_t vaddr) {
#ifdef _MSC_VER
    // MSVC stub - sera remplacé par assembleur
    (void)vaddr;
#else
    __asm__ volatile ("invlpg (%0)" : : "r"(vaddr) : "memory");
#endif
}

// Flush complet du TLB
void mmu_flush_tlb(void) {
    uint64_t cr3 = read_cr3();
    write_cr3(cr3); // Recharger CR3 flush le TLB
}
