#ifndef ORION_MM_H
#define ORION_MM_H

#include <orion/types.h>

// Types pour la gestion mémoire

// Structure pour espace mémoire virtuel
typedef struct vm_space
{
    uint64_t pml4_phys;  // Adresse physique de la PML4
    uint64_t start_addr; // Début de l'espace virtuel
    uint64_t end_addr;   // Fin de l'espace virtuel
    bool is_kernel;      // Espace noyau ou userland
} vm_space_t;

// Structure pour les blocs du heap
typedef struct heap_block
{
    uint32_t magic;          // Magic number pour validation
    uint64_t size;           // Taille du bloc
    bool is_free;            // Le bloc est-il libre?
    struct heap_block *next; // Bloc suivant
    struct heap_block *prev; // Bloc précédent
} heap_block_t;

// Structure pour cache slab
typedef struct slab
{
    uint64_t obj_size;      // Taille des objets
    uint64_t total_objects; // Nombre total d'objets
    uint64_t free_objects;  // Nombre d'objets libres
    void *free_list;        // Liste des blocs libres
    void *memory;           // Mémoire du slab
    struct slab *next;      // Slab suivant
} slab_t;

// Fonctions du Physical Memory Manager (PMM)
void pmm_init(void);
uint64_t pmm_alloc_page(void);
void pmm_free_page(uint64_t phys_addr);
uint64_t pmm_alloc_pages(uint64_t count);
void pmm_free_pages(uint64_t phys_addr, uint64_t count);
void pmm_get_stats(uint64_t *total, uint64_t *free, uint64_t *used);

// Fonctions du Virtual Memory Manager (VMM)
void vmm_init(void);
vm_space_t *vmm_create_space(bool is_kernel);
vm_space_t *vmm_get_kernel_space(void);
int vmm_map_page(vm_space_t *space, uint64_t vaddr, uint64_t paddr, uint64_t flags);
int vmm_unmap_page(vm_space_t *space, uint64_t vaddr);
int vmm_protect_page(vm_space_t *space, uint64_t vaddr, uint64_t new_flags);
uint64_t vmm_alloc_pages(vm_space_t *space, uint64_t count, uint64_t flags);
void vmm_free_pages(vm_space_t *space, uint64_t vaddr, uint64_t count);
void vmm_destroy_space(vm_space_t *space);
uint64_t mmu_virt_to_phys(uint64_t vaddr);
bool mmu_is_valid_addr(uint64_t vaddr);
void mmu_invalidate_page(uint64_t vaddr);
void mmu_flush_tlb(void);

// Fonctions architecture-specific
uint64_t read_cr3(void);

// Fonctions de l'allocateur Slab
void slab_init(void);
void *slab_alloc(uint64_t size);
void slab_free(void *ptr, uint64_t size);
void slab_get_stats(void);

// Fonctions du Heap noyau
void heap_init(void);
void heap_get_stats(void);

// Fonctions principales publiques du MM
void mm_init(void);

// Fonctions publiques d'allocation
void *kmalloc(uint64_t size);
void kfree(void *ptr);
void *krealloc(void *ptr, uint64_t new_size);

// Macros utilitaires mémoire
#define KMALLOC(type) ((type *)kmalloc(sizeof(type)))
#define KFREE(ptr)    \
    do                \
    {                 \
        kfree(ptr);   \
        (ptr) = NULL; \
    } while (0)

// Flags pour VMM
#define VM_FLAG_READ (1 << 0)
#define VM_FLAG_WRITE (1 << 1)
#define VM_FLAG_EXEC (1 << 2)
#define VM_FLAG_USER (1 << 3)

// Page flags (aliases for page table management)
#define PAGE_FLAG_PRESENT (1 << 0)
#define PAGE_FLAG_WRITE (1 << 1)
#define PAGE_FLAG_USER (1 << 2)
#define PAGE_FLAG_EXEC (1 << 3)
#define PAGE_FLAG_ACCESSED (1 << 5)
#define PAGE_FLAG_DIRTY (1 << 6)
#define PAGE_FLAG_GLOBAL (1 << 8)
#define PAGE_FLAG_NO_CACHE (1 << 4)

#endif // ORION_MM_H
