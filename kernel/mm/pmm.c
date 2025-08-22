// Physical Memory Manager (PMM) for Orion
#include <orion/types.h>
#include <orion/mm.h>
#include <orion/kernel.h>

// Bitmap for physical pages (for 4GB = 1M pages = 128KB bitmap)
#define MAX_PAGES       (1024 * 1024)  // 4GB / 4KB
#define BITMAP_SIZE     (MAX_PAGES / 8)

static uint8_t page_bitmap[BITMAP_SIZE];
static uint64_t total_pages = 0;
static uint64_t free_pages = 0;
static uint64_t first_free_page = 0;
static bool pmm_initialized = false;

// Mark a page as used
static void set_page_used(uint64_t page_num) {
    if (page_num >= MAX_PAGES) return;
    
    uint64_t byte_index = page_num / 8;
    uint8_t bit_index = page_num % 8;
    
    if (!(page_bitmap[byte_index] & (1 << bit_index))) {
        page_bitmap[byte_index] |= (1 << bit_index);
        free_pages--;
    }
}

// Mark a page as free
static void set_page_free(uint64_t page_num) {
    if (page_num >= MAX_PAGES) return;
    
    uint64_t byte_index = page_num / 8;
    uint8_t bit_index = page_num % 8;
    
    if (page_bitmap[byte_index] & (1 << bit_index)) {
        page_bitmap[byte_index] &= ~(1 << bit_index);
        free_pages++;
        
        if (page_num < first_free_page) {
            first_free_page = page_num;
        }
    }
}

// Check if a page is free
static bool is_page_free(uint64_t page_num) {
    if (page_num >= MAX_PAGES) return false;
    
    uint64_t byte_index = page_num / 8;
    uint8_t bit_index = page_num % 8;
    
    return !(page_bitmap[byte_index] & (1 << bit_index));
}

    // Initialize PMM with basic configuration
void pmm_init(void) {
    kinfo("Initializing Physical Memory Manager");
    
    // Mark all pages as free initially
    for (size_t i = 0; i < BITMAP_SIZE; i++) {
        page_bitmap[i] = 0;
    }
    
    // Simplified configuration: assume 512MB usable RAM
    // Pages 0-255 (1MB): reserved for boot/kernel
    // Pages 256-131071 (512MB-1MB): free
    total_pages = 131072; // 512MB / 4KB
    free_pages = total_pages - 256;
    first_free_page = 256;
    
    // Mark first pages as used (boot + kernel)
    for (uint64_t i = 0; i < 256; i++) {
        set_page_used(i);
    }
    
    pmm_initialized = true;
    
    kinfo("PMM initialized:");
    kinfo("  Total pages: %llu (%llu MB)",
          (unsigned long long)total_pages,
          (unsigned long long)(total_pages * PAGE_SIZE / 1024 / 1024));
    kinfo("  Free pages: %llu (%llu MB)",
          (unsigned long long)free_pages,
          (unsigned long long)(free_pages * PAGE_SIZE / 1024 / 1024));
}

// Allocate a physical page
uint64_t pmm_alloc_page(void) {
    if (!pmm_initialized || free_pages == 0) {
        return 0; // No free pages
    }
    
    // Find first free page
    for (uint64_t page = first_free_page; page < total_pages; page++) {
        if (is_page_free(page)) {
            set_page_used(page);
            
            // Update first_free_page
            if (page == first_free_page) {
                while (first_free_page < total_pages && !is_page_free(first_free_page)) {
                    first_free_page++;
                }
            }
            
            kdebug("Allocated physical page %llu (addr 0x%p)",
                   (unsigned long long)page,
                   (void*)(page * PAGE_SIZE));
            
            return page * PAGE_SIZE; // Return physical address
        }
    }
    
    kerror("No free pages available");
    return 0;
}

// Free a physical page
void pmm_free_page(uint64_t phys_addr) {
    if (!pmm_initialized) {
        return;
    }
    
    if (!IS_ALIGNED(phys_addr, PAGE_SIZE)) {
        kerror("Attempt to free unaligned address: 0x%p", (void*)phys_addr);
        return;
    }
    
    uint64_t page_num = phys_addr / PAGE_SIZE;
    if (page_num >= total_pages) {
        kerror("Attempt to free invalid page: %llu", (unsigned long long)page_num);
        return;
    }
    
    if (is_page_free(page_num)) {
        kwarning("Attempt to free already free page: %llu", (unsigned long long)page_num);
        return;
    }
    
    set_page_free(page_num);
    
    kdebug("Freed physical page %llu (addr 0x%p)",
           (unsigned long long)page_num, (void*)phys_addr);
}

// Allocate multiple contiguous pages
uint64_t pmm_alloc_pages(size_t count) {
    if (!pmm_initialized || count == 0) {
        return 0;
    }
    
    if (count == 1) {
        return pmm_alloc_page();
    }
    
    // Find 'count' contiguous free pages
    for (uint64_t start_page = first_free_page; start_page + count <= total_pages; start_page++) {
        bool all_free = true;
        
        // Check if all pages are free
        for (size_t i = 0; i < count; i++) {
            if (!is_page_free(start_page + i)) {
                all_free = false;
                break;
            }
        }
        
        if (all_free) {
            // Allocate all pages
            for (size_t i = 0; i < count; i++) {
                set_page_used(start_page + i);
            }
            
            kdebug("Allocated %llu contiguous pages starting at %llu",
                   (unsigned long long)count, (unsigned long long)start_page);
            
            return start_page * PAGE_SIZE;
        }
    }
    
    kerror("Could not allocate %llu contiguous pages", (unsigned long long)count);
    return 0;
}

// Free multiple contiguous pages
void pmm_free_pages(uint64_t phys_addr, size_t count) {
    if (!pmm_initialized || count == 0) {
        return;
    }
    
    for (size_t i = 0; i < count; i++) {
        pmm_free_page(phys_addr + i * PAGE_SIZE);
    }
}

// Get memory statistics
void pmm_get_stats(uint64_t* total, uint64_t* free, uint64_t* used) {
    if (total) *total = total_pages;
    if (free) *free = free_pages;
    if (used) *used = total_pages - free_pages;
}
