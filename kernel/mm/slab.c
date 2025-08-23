// Slab allocator for Orion - Fixed-size objects
#include <orion/types.h>
#include <orion/mm.h>
#include <orion/logging.h>
#include <orion/kernel.h>

// Common slab sizes
static const size_t slab_sizes[] = {
    16, 32, 64, 128, 256, 512, 1024, 2048, 4096};

#define NUM_SLAB_SIZES (sizeof(slab_sizes) / sizeof(slab_sizes[0]))

// Structure of a free block in a slab
typedef struct free_block
{
    struct free_block *next;
} free_block_t;

// slab_t structure defined in mm.h

// Slab cache for each size
static slab_t *slab_caches[NUM_SLAB_SIZES];
static bool slab_initialized = false;

// Create a new slab for a given size
static slab_t *create_slab(size_t obj_size)
{
    // Allocate a page for the slab
    uint64_t page_phys = pmm_alloc_page();
    if (!page_phys)
    {
        kerror("Failed to allocate page for slab");
        return NULL;
    }

    // Map the page (for now, simple identity mapping)
    void *memory = (void *)page_phys;

    // Allocate slab structure from the same page
    slab_t *slab = (slab_t *)memory;
    memory = (char *)memory + sizeof(slab_t);

    // Calculate how many objects we can store
    size_t available_space = PAGE_SIZE - sizeof(slab_t);
    size_t num_objects = available_space / obj_size;

    slab->obj_size = obj_size;
    slab->total_objects = num_objects;
    slab->free_objects = num_objects;
    slab->memory = memory;
    slab->next = NULL;

    // Initialize the free blocks list
    slab->free_list = NULL;
    char *obj_ptr = (char *)memory;

    for (size_t i = 0; i < num_objects; i++)
    {
        free_block_t *block = (free_block_t *)obj_ptr;
        block->next = slab->free_list;
        slab->free_list = block;
        obj_ptr += obj_size;
    }

    kdebug("Created slab: obj_size=%llu, objects=%llu",
           (unsigned long long)obj_size,
           (unsigned long long)num_objects);

    return slab;
}

// Initialize slab allocator
void slab_init(void)
{
    kinfo("Initializing slab allocator");

    // Initialize the caches
    for (size_t i = 0; i < NUM_SLAB_SIZES; i++)
    {
        slab_caches[i] = NULL;
    }

    slab_initialized = true;
    kinfo("Slab allocator initialized");
}

// Find cache index for a size
static int find_cache_index(size_t size)
{
    for (size_t i = 0; i < NUM_SLAB_SIZES; i++)
    {
        if (size <= slab_sizes[i])
        {
            return i;
        }
    }
    return -1; // Too large for slabs
}

// Allocate an object from slabs
void *slab_alloc(size_t size)
{
    if (!slab_initialized || size == 0)
    {
        return NULL;
    }

    // Find appropriate cache
    int cache_index = find_cache_index(size);
    if (cache_index < 0)
    {
        kdebug("Size %llu too large for slab allocation", (unsigned long long)size);
        return NULL;
    }

    // Look for a slab with free objects
    slab_t *slab = slab_caches[cache_index];
    while (slab && slab->free_objects == 0)
    {
        slab = slab->next;
    }

    // Create new slab if necessary
    if (!slab)
    {
        slab = create_slab(slab_sizes[cache_index]);
        if (!slab)
        {
            return NULL;
        }

        // Add to cache
        slab->next = slab_caches[cache_index];
        slab_caches[cache_index] = slab;
    }

    // Allocate from slab
    if (slab->free_list)
    {
        void *obj = slab->free_list;
        slab->free_list = ((free_block_t *)slab->free_list)->next;
        slab->free_objects--;

        kdebug("Allocated object of size %llu from slab (cache %d)",
               (unsigned long long)size, cache_index);

        return obj;
    }

    kerror("Slab corruption: free_objects > 0 but no free_list");
    return NULL;
}

// Free an object to slabs
void slab_free(void *ptr, size_t size)
{
    if (!slab_initialized || !ptr || size == 0)
    {
        return;
    }

    int cache_index = find_cache_index(size);
    if (cache_index < 0)
    {
        kdebug("Cannot free: size %llu not handled by slabs", (unsigned long long)size);
        return;
    }

    // Find the slab that contains this object
    slab_t *slab = slab_caches[cache_index];
    while (slab)
    {
        // Check if object belongs to this slab
        char *slab_start = (char *)slab->memory;
        char *slab_end = slab_start + (slab->total_objects * slab->obj_size);

        if (ptr >= (void *)slab_start && ptr < (void *)slab_end)
        {
            // Check alignment
            size_t offset = (char *)ptr - slab_start;
            if (offset % slab->obj_size != 0)
            {
                kerror("Invalid pointer alignment in slab_free");
                return;
            }

            // Add to free list
            free_block_t *block = (free_block_t *)ptr;
            block->next = slab->free_list;
            slab->free_list = block;
            slab->free_objects++;

            kdebug("Freed object to slab (cache %d)", cache_index);
            return;
        }

        slab = slab->next;
    }

    kerror("Pointer not found in any slab for size %llu", (unsigned long long)size);
}

// Get slab statistics
void slab_get_stats(void)
{
    if (!slab_initialized)
    {
        return;
    }

    kinfo("Slab allocator statistics:");

    for (size_t i = 0; i < NUM_SLAB_SIZES; i++)
    {
        size_t total_slabs = 0;
        size_t total_objects = 0;
        size_t free_objects = 0;

        slab_t *slab = slab_caches[i];
        while (slab)
        {
            total_slabs++;
            total_objects += slab->total_objects;
            free_objects += slab->free_objects;
            slab = slab->next;
        }

        if (total_slabs > 0)
        {
            kinfo("  Size %llu: %llu slabs, %llu/%llu objects used",
                  (unsigned long long)slab_sizes[i],
                  (unsigned long long)total_slabs,
                  (unsigned long long)(total_objects - free_objects),
                  (unsigned long long)total_objects);
        }
    }
}
