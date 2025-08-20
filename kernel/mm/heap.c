// Heap allocator pour Orion - malloc/free du noyau
#include <orion/types.h>
#include <orion/mm.h>
#include <orion/kernel.h>

// Taille minimum d'allocation
#define MIN_ALLOC_SIZE  16
#define HEAP_MAGIC      0xDEADBEEF

// Structure heap_block_t définie dans mm.h

// Zone de heap statique pour le noyau (1MB)
#define KERNEL_HEAP_SIZE (1024 * 1024)
#ifdef _MSC_VER
__declspec(align(16)) static char kernel_heap_memory[KERNEL_HEAP_SIZE];
#else
static char kernel_heap_memory[KERNEL_HEAP_SIZE] __attribute__((aligned(16)));
#endif
static heap_block_t* heap_start = NULL;
static bool heap_initialized = false;

// Statistiques du heap
static size_t total_allocated = 0;
static size_t peak_allocated = 0;
static size_t num_allocations = 0;
static size_t num_frees = 0;

// Initialiser le heap
void heap_init(void) {
    kinfo("Initializing kernel heap");
    
    // Initialiser le premier bloc (tout l'espace libre)
    heap_start = (heap_block_t*)kernel_heap_memory;
    heap_start->magic = HEAP_MAGIC;
    heap_start->size = KERNEL_HEAP_SIZE - sizeof(heap_block_t);
    heap_start->is_free = true;
    heap_start->next = NULL;
    heap_start->prev = NULL;
    
    heap_initialized = true;
    
    kinfo("Kernel heap initialized:");
    kinfo("  Size: %d KB", KERNEL_HEAP_SIZE / 1024);
    kinfo("  Available: %llu bytes", (unsigned long long)heap_start->size);
}

// Vérifier la validité d'un bloc
static bool validate_block(heap_block_t* block) {
    if (!block) return false;
    if (block->magic != HEAP_MAGIC) {
        kerror("Heap corruption: invalid magic 0x%x", block->magic);
        return false;
    }
    return true;
}

// Fusionner les blocs libres adjacents
static void coalesce_free_blocks(heap_block_t* block) {
    if (!validate_block(block) || !block->is_free) {
        return;
    }
    
    // Fusionner avec le bloc suivant s'il est libre
    if (block->next && block->next->is_free) {
        char* block_end = (char*)block + sizeof(heap_block_t) + block->size;
        if (block_end == (char*)block->next) {
            block->size += sizeof(heap_block_t) + block->next->size;
            if (block->next->next) {
                block->next->next->prev = block;
            }
            block->next = block->next->next;
        }
    }
    
    // Fusionner avec le bloc précédent s'il est libre
    if (block->prev && block->prev->is_free) {
        char* prev_end = (char*)block->prev + sizeof(heap_block_t) + block->prev->size;
        if (prev_end == (char*)block) {
            block->prev->size += sizeof(heap_block_t) + block->size;
            if (block->next) {
                block->next->prev = block->prev;
            }
            block->prev->next = block->next;
        }
    }
}

// Diviser un bloc libre si nécessaire
static heap_block_t* split_block(heap_block_t* block, size_t size) {
    if (!validate_block(block) || !block->is_free) {
        return NULL;
    }
    
    size_t remaining = block->size - size;
    
    // Si le reste est trop petit pour un bloc, ne pas diviser
    if (remaining < sizeof(heap_block_t) + MIN_ALLOC_SIZE) {
        return block;
    }
    
    // Créer un nouveau bloc avec le reste
    char* new_block_ptr = (char*)block + sizeof(heap_block_t) + size;
    heap_block_t* new_block = (heap_block_t*)new_block_ptr;
    
    new_block->magic = HEAP_MAGIC;
    new_block->size = remaining - sizeof(heap_block_t);
    new_block->is_free = true;
    new_block->next = block->next;
    new_block->prev = block;
    
    if (block->next) {
        block->next->prev = new_block;
    }
    
    block->next = new_block;
    block->size = size;
    
    return block;
}

// Allocation mémoire noyau
void* kmalloc(size_t size) {
    if (!heap_initialized || size == 0) {
        return NULL;
    }
    
    // Aligner sur 16 bytes minimum
    size = ROUND_UP(size, MIN_ALLOC_SIZE);
    
    // Pour les grosses allocations, utiliser directement le PMM
    if (size >= PAGE_SIZE) {
        size_t pages = ROUND_UP(size, PAGE_SIZE) / PAGE_SIZE;
        uint64_t phys_addr = pmm_alloc_pages(pages);
        if (phys_addr) {
            // Stocker la taille en pages dans les premiers 8 bytes pour kfree
            uint64_t* size_ptr = (uint64_t*)phys_addr;
            *size_ptr = pages;
            
            kdebug("Large kmalloc: %llu bytes (%llu pages) at 0x%p",
                   (unsigned long long)size,
                   (unsigned long long)pages,
                   (void*)phys_addr);
            return (void*)phys_addr; // Identity mapping pour l'instant
        }
        return NULL;
    }
    
    // Chercher un bloc libre assez grand
    heap_block_t* block = heap_start;
    while (block) {
        if (!validate_block(block)) {
            return NULL;
        }
        
        if (block->is_free && block->size >= size) {
            // Diviser le bloc si nécessaire
            block = split_block(block, size);
            if (!block) continue;
            
            // Marquer comme alloué
            block->is_free = false;
            
            // Mettre à jour les statistiques
            total_allocated += block->size;
            num_allocations++;
            if (total_allocated > peak_allocated) {
                peak_allocated = total_allocated;
            }
            
            kdebug("kmalloc: %llu bytes at 0x%p",
                   (unsigned long long)size,
                   (void*)((char*)block + sizeof(heap_block_t)));
            
            return (char*)block + sizeof(heap_block_t);
        }
        
        block = block->next;
    }
    
    kerror("kmalloc failed: no free block of size %llu", (unsigned long long)size);
    return NULL;
}

// Libération mémoire noyau
void kfree(void* ptr) {
    if (!heap_initialized || !ptr) {
        return;
    }
    
    // Vérifier si c'est une allocation de page directe
    if (IS_ALIGNED((uint64_t)ptr, PAGE_SIZE)) {
        // Essayer de déterminer la taille via les métadonnées
        // Pour les allocations page, on stocke la taille dans les premiers 8 bytes
        uint64_t* size_ptr = (uint64_t*)ptr;
        uint64_t size_pages = *size_ptr;
        
        // Validation basique de la taille
        if (size_pages > 0 && size_pages <= 1024) { // Max 4MB
            pmm_free_pages((uint64_t)ptr, size_pages);
            kdebug("Large kfree: 0x%p (%llu pages)", ptr, (unsigned long long)size_pages);
        } else {
            // Fallback: libérer une seule page
            pmm_free_page((uint64_t)ptr);
            kwarning("Large kfree: unknown size, freed 1 page at 0x%p", ptr);
        }
        return;
    }
    
    // Obtenir le header du bloc
    heap_block_t* block = (heap_block_t*)((char*)ptr - sizeof(heap_block_t));
    
    if (!validate_block(block)) {
        kerror("kfree: invalid block at 0x%p", ptr);
        return;
    }
    
    if (block->is_free) {
        kwarning("kfree: double free detected at 0x%p", ptr);
        return;
    }
    
    // Marquer comme libre
    block->is_free = true;
    
    // Mettre à jour les statistiques
    total_allocated -= block->size;
    num_frees++;
    
    kdebug("kfree: %llu bytes at 0x%p",
           (unsigned long long)block->size, ptr);
    
    // Fusionner avec les blocs adjacents libres
    coalesce_free_blocks(block);
}

// Réallocation
void* krealloc(void* ptr, size_t new_size) {
    if (!ptr) {
        return kmalloc(new_size);
    }
    
    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    // Optimiser en essayant d'étendre le bloc existant
    heap_block_t* old_block = (heap_block_t*)((char*)ptr - sizeof(heap_block_t));
    if (validate_block(old_block)) {
        size_t old_size = old_block->size;
        
        // Si la nouvelle taille est plus petite ou égale, réutiliser le bloc
        if (new_size <= old_size) {
            // Optionnellement diviser le bloc si la différence est significative
            if (old_size - new_size >= sizeof(heap_block_t) + MIN_ALLOC_SIZE) {
                split_block(old_block, new_size);
            }
            return ptr;
        }
        
        // Essayer d'étendre en fusionnant avec le bloc suivant libre
        if (old_block->next && old_block->next->is_free) {
            char* block_end = (char*)old_block + sizeof(heap_block_t) + old_block->size;
            if (block_end == (char*)old_block->next) {
                size_t combined_size = old_size + sizeof(heap_block_t) + old_block->next->size;
                
                if (combined_size >= new_size) {
                    // Fusionner les blocs
                    old_block->size = combined_size;
                    if (old_block->next->next) {
                        old_block->next->next->prev = old_block;
                    }
                    old_block->next = old_block->next->next;
                    
                    // Diviser si nécessaire
                    if (combined_size - new_size >= sizeof(heap_block_t) + MIN_ALLOC_SIZE) {
                        split_block(old_block, new_size);
                    }
                    
                    kdebug("krealloc: expanded in-place from %llu to %llu bytes",
                           (unsigned long long)old_size, (unsigned long long)new_size);
                    return ptr;
                }
            }
        }
    }
    
    // Allocation standard si l'optimisation a échoué
    void* new_ptr = kmalloc(new_size);
    if (!new_ptr) {
        return NULL;
    }
    
    // Déterminer la vraie taille de l'ancien bloc pour la copie
    size_t copy_size = new_size;
    if (validate_block(old_block)) {
        copy_size = MIN(old_block->size, new_size);
    }
    
    // Copie optimisée par chunks de 8 bytes
    char* src = (char*)ptr;
    char* dst = (char*)new_ptr;
    size_t remaining = copy_size;
    
    // Copie par uint64_t quand possible
    while (remaining >= 8 && IS_ALIGNED((uint64_t)src, 8) && IS_ALIGNED((uint64_t)dst, 8)) {
        *(uint64_t*)dst = *(uint64_t*)src;
        src += 8;
        dst += 8;
        remaining -= 8;
    }
    
    // Copie byte par byte pour le reste
    while (remaining > 0) {
        *dst++ = *src++;
        remaining--;
    }
    
    kfree(ptr);
    return new_ptr;
}

// Obtenir statistiques du heap
void heap_get_stats(void) {
    if (!heap_initialized) {
        return;
    }
    
    kinfo("Kernel heap statistics:");
    kinfo("  Total size: %d KB", KERNEL_HEAP_SIZE / 1024);
    kinfo("  Currently allocated: %llu bytes", (unsigned long long)total_allocated);
    kinfo("  Peak allocated: %llu bytes", (unsigned long long)peak_allocated);
    kinfo("  Allocations: %llu", (unsigned long long)num_allocations);
    kinfo("  Frees: %llu", (unsigned long long)num_frees);
    kinfo("  Active allocations: %llu", (unsigned long long)(num_allocations - num_frees));
}

// Initialisation complète du sous-système mémoire
void mm_init(void) {
    kinfo("Initializing memory management");
    
    pmm_init();
    // vmm_init(); // Déjà initialisé par mmu_init()
    slab_init();
    heap_init();
    
    kinfo("Memory management initialized");
}
