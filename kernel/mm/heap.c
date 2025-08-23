// Heap allocator for Orion - kernel malloc/free
#include <orion/types.h>
#include <orion/mm.h>
#include <orion/kernel.h>
#include <orion/logging.h>

// Minimum allocation size
#define MIN_ALLOC_SIZE 16
#define HEAP_MAGIC 0xDEADBEEF

// heap_block_t structure defined in mm.h

// Static heap zone for kernel (1MB)
#define KERNEL_HEAP_SIZE (1024 * 1024)
#ifdef _MSC_VER
__declspec(align(16)) static char kernel_heap_memory[KERNEL_HEAP_SIZE];
#else
static char kernel_heap_memory[KERNEL_HEAP_SIZE] __attribute__((aligned(16)));
#endif
static heap_block_t *heap_start = NULL;
static bool heap_initialized = false;

// Heap statistics
static size_t total_allocated = 0;
static size_t peak_allocated = 0;
static size_t num_allocations = 0;
static size_t num_frees = 0;

// Initialize the heap
void heap_init(void)
{
    kinfo("Initializing kernel heap");

    // Initialize the first block (all free space)
    heap_start = (heap_block_t *)kernel_heap_memory;
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

// Validate a block
static bool validate_block(heap_block_t *block)
{
    if (!block)
        return false;
    if (block->magic != HEAP_MAGIC)
    {
        kerror("Heap corruption: invalid magic 0x%x", block->magic);
        return false;
    }
    return true;
}

// Merge adjacent free blocks
static void coalesce_free_blocks(heap_block_t *block)
{
    if (!validate_block(block) || !block->is_free)
    {
        return;
    }

    // Merge with next block if it's free
    if (block->next && block->next->is_free)
    {
        char *block_end = (char *)block + sizeof(heap_block_t) + block->size;
        if (block_end == (char *)block->next)
        {
            block->size += sizeof(heap_block_t) + block->next->size;
            if (block->next->next)
            {
                block->next->next->prev = block;
            }
            block->next = block->next->next;
        }
    }

    // Merge with previous block if it's free
    if (block->prev && block->prev->is_free)
    {
        char *prev_end = (char *)block->prev + sizeof(heap_block_t) + block->prev->size;
        if (prev_end == (char *)block)
        {
            block->prev->size += sizeof(heap_block_t) + block->size;
            if (block->next)
            {
                block->next->prev = block->prev;
            }
            block->prev->next = block->next;
        }
    }
}

// Split a free block if necessary
static heap_block_t *split_block(heap_block_t *block, size_t size)
{
    if (!validate_block(block) || !block->is_free)
    {
        return NULL;
    }

    size_t remaining = block->size - size;

    // If the remainder is too small for a block, don't split
    if (remaining < sizeof(heap_block_t) + MIN_ALLOC_SIZE)
    {
        return block;
    }

    // Create a new block with the remainder
    char *new_block_ptr = (char *)block + sizeof(heap_block_t) + size;
    heap_block_t *new_block = (heap_block_t *)new_block_ptr;

    new_block->magic = HEAP_MAGIC;
    new_block->size = remaining - sizeof(heap_block_t);
    new_block->is_free = true;
    new_block->next = block->next;
    new_block->prev = block;

    if (block->next)
    {
        block->next->prev = new_block;
    }

    block->next = new_block;
    block->size = size;

    return block;
}

// Kernel memory allocation
void *kmalloc(size_t size)
{
    if (!heap_initialized || size == 0)
    {
        return NULL;
    }

    // Align on 16 bytes minimum
    size = ROUND_UP(size, MIN_ALLOC_SIZE);

    // For large allocations, use PMM directly
    if (size >= PAGE_SIZE)
    {
        size_t pages = ROUND_UP(size, PAGE_SIZE) / PAGE_SIZE;
        uint64_t phys_addr = pmm_alloc_pages(pages);
        if (phys_addr)
        {
            // Store page count in first 8 bytes for kfree
            uint64_t *size_ptr = (uint64_t *)phys_addr;
            *size_ptr = pages;

            kdebug("Large kmalloc: %llu bytes (%llu pages) at 0x%p",
                   (unsigned long long)size,
                   (unsigned long long)pages,
                   (void *)phys_addr);
            return (void *)phys_addr; // Identity mapping for now
        }
        return NULL;
    }

    // Find a free block large enough
    heap_block_t *block = heap_start;
    while (block)
    {
        if (!validate_block(block))
        {
            return NULL;
        }

        if (block->is_free && block->size >= size)
        {
            // Split block if necessary
            block = split_block(block, size);
            if (!block)
                continue;

            // Mark as allocated
            block->is_free = false;

            // Update statistics
            total_allocated += block->size;
            num_allocations++;
            if (total_allocated > peak_allocated)
            {
                peak_allocated = total_allocated;
            }

            kdebug("kmalloc: %llu bytes at 0x%p",
                   (unsigned long long)size,
                   (void *)((char *)block + sizeof(heap_block_t)));

            return (char *)block + sizeof(heap_block_t);
        }

        block = block->next;
    }

    kerror("kmalloc failed: no free block of size %llu", (unsigned long long)size);
    return NULL;
}

// Kernel memory deallocation
void kfree(void *ptr)
{
    if (!heap_initialized || !ptr)
    {
        return;
    }

    // Check if it's a direct page allocation
    if (IS_ALIGNED((uint64_t)ptr, PAGE_SIZE))
    {
        // Try to determine size via metadata
        // For page allocations, we store size in first 8 bytes
        uint64_t *size_ptr = (uint64_t *)ptr;
        uint64_t size_pages = *size_ptr;

        // Basic size validation
        if (size_pages > 0 && size_pages <= 1024)
        { // Max 4MB
            pmm_free_pages((uint64_t)ptr, size_pages);
            kdebug("Large kfree: 0x%p (%llu pages)", ptr, (unsigned long long)size_pages);
        }
        else
        {
            // Fallback: free a single page
            pmm_free_page((uint64_t)ptr);
            kwarning("Large kfree: unknown size, freed 1 page at 0x%p", ptr);
        }
        return;
    }

    // Get block header
    heap_block_t *block = (heap_block_t *)((char *)ptr - sizeof(heap_block_t));

    if (!validate_block(block))
    {
        kerror("kfree: invalid block at 0x%p", ptr);
        return;
    }

    if (block->is_free)
    {
        kwarning("kfree: double free detected at 0x%p", ptr);
        return;
    }

    // Mark as free
    block->is_free = true;

    // Update statistics
    total_allocated -= block->size;
    num_frees++;

    kdebug("kfree: %llu bytes at 0x%p",
           (unsigned long long)block->size, ptr);

    // Merge with adjacent free blocks
    coalesce_free_blocks(block);
}

// Reallocation
void *krealloc(void *ptr, size_t new_size)
{
    if (!ptr)
    {
        return kmalloc(new_size);
    }

    if (new_size == 0)
    {
        kfree(ptr);
        return NULL;
    }

    // Optimize by trying to extend existing block
    heap_block_t *old_block = (heap_block_t *)((char *)ptr - sizeof(heap_block_t));
    if (validate_block(old_block))
    {
        size_t old_size = old_block->size;

        // If new size is smaller or equal, reuse block
        if (new_size <= old_size)
        {
            // Optionally split block if difference is significant
            if (old_size - new_size >= sizeof(heap_block_t) + MIN_ALLOC_SIZE)
            {
                split_block(old_block, new_size);
            }
            return ptr;
        }

        // Try to extend by merging with next free block
        if (old_block->next && old_block->next->is_free)
        {
            char *block_end = (char *)old_block + sizeof(heap_block_t) + old_block->size;
            if (block_end == (char *)old_block->next)
            {
                size_t combined_size = old_size + sizeof(heap_block_t) + old_block->next->size;

                if (combined_size >= new_size)
                {
                    // Merge blocks
                    old_block->size = combined_size;
                    if (old_block->next->next)
                    {
                        old_block->next->next->prev = old_block;
                    }
                    old_block->next = old_block->next->next;

                    // Split if necessary
                    if (combined_size - new_size >= sizeof(heap_block_t) + MIN_ALLOC_SIZE)
                    {
                        split_block(old_block, new_size);
                    }

                    kdebug("krealloc: expanded in-place from %llu to %llu bytes",
                           (unsigned long long)old_size, (unsigned long long)new_size);
                    return ptr;
                }
            }
        }
    }

    // Standard allocation if optimization failed
    void *new_ptr = kmalloc(new_size);
    if (!new_ptr)
    {
        return NULL;
    }

    // Determine real size of old block for copy
    size_t copy_size = new_size;
    if (validate_block(old_block))
    {
        copy_size = MIN(old_block->size, new_size);
    }

    // Optimized copy by 8-byte chunks
    char *src = (char *)ptr;
    char *dst = (char *)new_ptr;
    size_t remaining = copy_size;

    // Copy by uint64_t when possible
    while (remaining >= 8 && IS_ALIGNED((uint64_t)src, 8) && IS_ALIGNED((uint64_t)dst, 8))
    {
        *(uint64_t *)dst = *(uint64_t *)src;
        src += 8;
        dst += 8;
        remaining -= 8;
    }

    // Copy byte by byte for the rest
    while (remaining > 0)
    {
        *dst++ = *src++;
        remaining--;
    }

    kfree(ptr);
    return new_ptr;
}

// Get heap statistics
void heap_get_stats(void)
{
    if (!heap_initialized)
    {
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

// Complete memory subsystem initialization
void mm_init(void)
{
    kinfo("Initializing memory management");

    pmm_init();
    // vmm_init(); // Already initialized by mmu_init()
    slab_init();
    heap_init();

    kinfo("Memory management initialized");
}
