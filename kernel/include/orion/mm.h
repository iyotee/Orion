#ifndef ORION_MM_H
#define ORION_MM_H

#include <orion/types.h>

// Types for memory management

// Structure for virtual memory space
typedef struct vm_space
{
    uint64_t pml4_phys;  // Physical address of PML4
    uint64_t start_addr; // Start of virtual space
    uint64_t end_addr;   // End of virtual space
    bool is_kernel;      // Kernel or userland space
} vm_space_t;

// Structure for heap blocks
typedef struct heap_block
{
    uint32_t magic;          // Magic number for validation
    uint64_t size;           // Block size
    bool is_free;            // Is the block free?
    struct heap_block *next; // Next block
    struct heap_block *prev; // Previous block
} heap_block_t;

// Structure for slab cache
typedef struct slab
{
    uint64_t obj_size;      // Object size
    uint64_t total_objects; // Total number of objects
    uint64_t free_objects;  // Number of free objects
    void *free_list;        // Free block list
    void *memory;           // Slab memory
    struct slab *next;      // Next slab
} slab_t;

// Physical Memory Manager (PMM) functions
void pmm_init(void);
uint64_t pmm_alloc_page(void);
void pmm_free_page(uint64_t phys_addr);
uint64_t pmm_alloc_pages(uint64_t count);
void pmm_free_pages(uint64_t phys_addr, uint64_t count);
void pmm_get_stats(uint64_t *total, uint64_t *free, uint64_t *used);

// Virtual Memory Manager (VMM) functions
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

// Copy-on-Write (COW) functions
int vmm_mark_cow(vm_space_t *space, uint64_t vaddr);
int vmm_handle_cow_fault(vm_space_t *space, uint64_t vaddr);
page_ref_t *vmm_get_page_ref(uint64_t paddr);
void vmm_inc_page_ref(uint64_t paddr);
void vmm_dec_page_ref(uint64_t paddr);

// Architecture-specific functions
uint64_t read_cr3(void);

// Slab allocator functions
void slab_init(void);
void *slab_alloc(uint64_t size);
void slab_free(void *ptr, uint64_t size);
void slab_get_stats(void);

// Kernel heap functions
void heap_init(void);
void heap_get_stats(void);

// Main public MM functions
void mm_init(void);

// Public allocation functions
void *kmalloc(uint64_t size);
void kfree(void *ptr);
void *krealloc(void *ptr, uint64_t new_size);

// Memory utility macros
#define KMALLOC(type) ((type *)kmalloc(sizeof(type)))
// Use kfree() directly instead of KFREE macro for consistency

// VMM flags
#define VM_FLAG_READ (1 << 0)
#define VM_FLAG_WRITE (1 << 1)
#define VM_FLAG_EXEC (1 << 2)
#define VM_FLAG_USER (1 << 3)

// Page flags (aliases for page table management)
#define PAGE_FLAG_PRESENT (1 << 0)
#define PAGE_FLAG_WRITE (1 << 1)
#define PAGE_FLAG_USER (1 << 2)
#define PAGE_FLAG_EXEC (1 << 3)
#define PAGE_FLAG_NO_CACHE (1 << 4)
#define PAGE_FLAG_ACCESSED (1 << 5)
#define PAGE_FLAG_DIRTY (1 << 6)
#define PAGE_FLAG_GLOBAL (1 << 8)
#define PAGE_FLAG_COW (1 << 9)    // Copy-on-Write flag
#define PAGE_FLAG_SHARED (1 << 10) // Shared page flag

// COW-specific page management
typedef struct page_ref {
    atomic32_t ref_count;     // Reference counter for shared pages
    uint64_t physical_addr;   // Physical address
    spinlock_t lock;          // Lock for atomic operations
} page_ref_t;

#endif // ORION_MM_H
