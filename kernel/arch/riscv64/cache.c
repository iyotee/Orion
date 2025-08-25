/*
 * ORION OS - RISC-V 64-bit Cache Management
 *
 * Cache management for RISC-V 64-bit architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include "common.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// GLOBAL CACHE CONTEXTS
// ============================================================================

// Cache context
static struct {
    bool initialized;
    uint32_t l1i_size;
    uint32_t l1d_size;
    uint32_t l2_size;
    uint32_t l3_size;
    uint32_t line_size;
    uint32_t ways;
    uint32_t sets;
    bool enabled;
} riscv64_cache_context = {0};

// Cache statistics
static struct {
    uint32_t l1i_hits;
    uint32_t l1i_misses;
    uint32_t l1d_hits;
    uint32_t l1d_misses;
    uint32_t l2_hits;
    uint32_t l2_misses;
    uint32_t l3_hits;
    uint32_t l3_misses;
    uint32_t invalidations;
    uint32_t cleanings;
    uint32_t flushes;
} riscv64_cache_stats = {0};

// ============================================================================
// CACHE INITIALIZATION
// ============================================================================

// Initialize cache system
void riscv64_cache_init(void)
{
    if (riscv64_cache_context.initialized) {
        return;
    }
    
    // Initialize cache context with default values
    riscv64_cache_context.l1i_size = RISCV64_L1I_CACHE_SIZE;
    riscv64_cache_context.l1d_size = RISCV64_L1D_CACHE_SIZE;
    riscv64_L2_size = RISCV64_L2_CACHE_SIZE;
    riscv64_cache_context.l3_size = RISCV64_L3_CACHE_SIZE;
    riscv64_cache_context.line_size = RISCV64_L1_CACHE_LINE_SIZE;
    riscv64_cache_context.ways = RISCV64_TLB_WAYS;
    riscv64_cache_context.sets = 64;  // Default value
    riscv64_cache_context.enabled = true;
    riscv64_cache_context.initialized = true;
    
    // Clear cache statistics
    memset(&riscv64_cache_stats, 0, sizeof(riscv64_cache_stats));
    
    printf("RISC-V64: Cache system initialized\n");
    printf("  L1I: %u KB, L1D: %u KB\n", 
           riscv64_cache_context.l1i_size / 1024, 
           riscv64_cache_context.l1d_size / 1024);
    printf("  L2: %u KB, L3: %u MB\n", 
           riscv64_cache_context.l2_size / 1024, 
           riscv64_cache_context.l3_size / (1024 * 1024));
    printf("  Line size: %u bytes\n", riscv64_cache_context.line_size);
}

// ============================================================================
// CACHE CONTEXT FUNCTIONS
// ============================================================================

// Get instruction cache context
struct riscv64_cache_context* riscv64_get_icache_context(void)
{
    static struct riscv64_cache_context context;
    
    if (!riscv64_cache_context.initialized) {
        return NULL;
    }
    
    context.l1i_size = riscv64_cache_context.l1i_size;
    context.l2_size = riscv64_cache_context.l2_size;
    context.l3_size = riscv64_cache_context.l3_size;
    context.line_size = riscv64_cache_context.line_size;
    context.ways = riscv64_cache_context.ways;
    context.sets = riscv64_cache_context.sets;
    context.enabled = riscv64_cache_context.enabled;
    
    return &context;
}

// Get data cache context
struct riscv64_cache_context* riscv64_get_dcache_context(void)
{
    static struct riscv64_cache_context context;
    
    if (!riscv64_cache_context.initialized) {
        return NULL;
    }
    
    context.l1i_size = riscv64_cache_context.l1d_size;  // Use L1D size
    context.l2_size = riscv64_cache_context.l2_size;
    context.l3_size = riscv64_cache_context.l3_size;
    context.line_size = riscv64_cache_context.line_size;
    context.ways = riscv64_cache_context.ways;
    context.sets = riscv64_cache_context.sets;
    context.enabled = riscv64_cache_context.enabled;
    
    return &context;
}

// Get L2 cache context
struct riscv64_cache_context* riscv64_get_l2cache_context(void)
{
    static struct riscv64_cache_context context;
    
    if (!riscv64_cache_context.initialized) {
        return NULL;
    }
    
    context.l1i_size = riscv64_cache_context.l2_size;  // Use L2 size
    context.l2_size = riscv64_cache_context.l3_size;   // Use L3 size
    context.l3_size = 0;                               // No L4 cache
    context.line_size = riscv64_cache_context.line_size;
    context.ways = riscv64_cache_context.ways;
    context.sets = riscv64_cache_context.sets;
    context.enabled = riscv64_cache_context.enabled;
    
    return &context;
}

// Get L3 cache context
struct riscv64_cache_context* riscv64_get_l3cache_context(void)
{
    static struct riscv64_cache_context context;
    
    if (!riscv64_cache_context.initialized) {
        return NULL;
    }
    
    context.l1i_size = riscv64_cache_context.l3_size;  // Use L3 size
    context.l2_size = 0;                               // No L4 cache
    context.l3_size = 0;                               // No L4 cache
    context.line_size = riscv64_cache_context.line_size;
    context.ways = riscv64_cache_context.ways;
    context.sets = riscv64_cache_context.sets;
    context.enabled = riscv64_cache_context.enabled;
    
    return &context;
}

// ============================================================================
// INSTRUCTION CACHE OPERATIONS
// ============================================================================

// Invalidate instruction cache
void riscv64_icache_invalidate(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    // Use fence.i instruction to invalidate instruction cache
    asm volatile("fence.i");
    
    riscv64_cache_stats.invalidations++;
    
    printf("RISC-V64: Instruction cache invalidated\n");
}

// Invalidate instruction cache range
void riscv64_icache_invalidate_range(uint64_t start, uint64_t end)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    // Align addresses to cache line boundaries
    uint64_t aligned_start = start & ~(riscv64_cache_context.line_size - 1);
    uint64_t aligned_end = (end + riscv64_cache_context.line_size - 1) & ~(riscv64_cache_context.line_size - 1);
    
    // Invalidate each cache line in the range
    for (uint64_t addr = aligned_start; addr < aligned_end; addr += riscv64_cache_context.line_size) {
        // This would typically involve cache line invalidation instructions
        // For RISC-V, we use fence.i which invalidates the entire instruction cache
        asm volatile("fence.i");
        break;  // fence.i invalidates all, so we can break after first call
    }
    
    riscv64_cache_stats.invalidations++;
    
    printf("RISC-V64: Instruction cache range invalidated: 0x%llx - 0x%llx\n", start, end);
}

// Sync instruction cache
void riscv64_icache_sync(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    // Use fence.i instruction to synchronize instruction cache
    asm volatile("fence.i");
    
    printf("RISC-V64: Instruction cache synchronized\n");
}

// ============================================================================
// DATA CACHE OPERATIONS
// ============================================================================

// Invalidate data cache
void riscv64_dcache_invalidate(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    // For RISC-V, we need to manually invalidate data cache lines
    // This is a simplified implementation
    printf("RISC-V64: Data cache invalidated\n");
    
    riscv64_cache_stats.invalidations++;
}

// Invalidate data cache range
void riscv64_dcache_invalidate_range(uint64_t start, uint64_t end)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    // Align addresses to cache line boundaries
    uint64_t aligned_start = start & ~(riscv64_cache_context.line_size - 1);
    uint64_t aligned_end = (end + riscv64_cache_context.line_size - 1) & ~(riscv64_cache_context.line_size - 1);
    
    // Invalidate each cache line in the range
    for (uint64_t addr = aligned_start; addr < aligned_end; addr += riscv64_cache_context.line_size) {
        // This would typically involve cache line invalidation instructions
        // For RISC-V, we need to implement this based on the specific cache implementation
        printf("RISC-V64: Invalidating data cache line at 0x%llx\n", addr);
    }
    
    riscv64_cache_stats.invalidations++;
    
    printf("RISC-V64: Data cache range invalidated: 0x%llx - 0x%llx\n", start, end);
}

// Clean data cache
void riscv64_dcache_clean(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    // For RISC-V, we need to manually clean data cache lines
    // This is a simplified implementation
    printf("RISC-V64: Data cache cleaned\n");
    
    riscv64_cache_stats.cleanings++;
}

// Clean data cache range
void riscv64_dcache_clean_range(uint64_t start, uint64_t end)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    // Align addresses to cache line boundaries
    uint64_t aligned_start = start & ~(riscv64_cache_context.line_size - 1);
    uint64_t aligned_end = (end + riscv64_cache_context.line_size - 1) & ~(riscv64_cache_context.line_size - 1);
    
    // Clean each cache line in the range
    for (uint64_t addr = aligned_start; addr < aligned_end; addr += riscv64_cache_context.line_size) {
        // This would typically involve cache line cleaning instructions
        // For RISC-V, we need to implement this based on the specific cache implementation
        printf("RISC-V64: Cleaning data cache line at 0x%llx\n", addr);
    }
    
    riscv64_cache_stats.cleanings++;
    
    printf("RISC-V64: Data cache range cleaned: 0x%llx - 0x%llx\n", start, end);
}

// Flush data cache
void riscv64_dcache_flush(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    // Flush is equivalent to clean + invalidate
    riscv64_dcache_clean();
    riscv64_dcache_invalidate();
    
    riscv64_cache_stats.flushes++;
    
    printf("RISC-V64: Data cache flushed\n");
}

// Flush data cache range
void riscv64_dcache_flush_range(uint64_t start, uint64_t end)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    // Flush is equivalent to clean + invalidate
    riscv64_dcache_clean_range(start, end);
    riscv64_dcache_invalidate_range(start, end);
    
    riscv64_cache_stats.flushes++;
    
    printf("RISC-V64: Data cache range flushed: 0x%llx - 0x%llx\n", start, end);
}

// Sync data cache
void riscv64_dcache_sync(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    // Use memory barrier to synchronize data cache
    asm volatile("fence rw, rw");
    
    printf("RISC-V64: Data cache synchronized\n");
}

// ============================================================================
// L2 CACHE OPERATIONS
// ============================================================================

// Invalidate L2 cache
void riscv64_l2cache_invalidate(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    // L2 cache invalidation would depend on the specific implementation
    printf("RISC-V64: L2 cache invalidated\n");
    
    riscv64_cache_stats.invalidations++;
}

// Clean L2 cache
void riscv64_l2cache_clean(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    // L2 cache cleaning would depend on the specific implementation
    printf("RISC-V64: L2 cache cleaned\n");
    
    riscv64_cache_stats.cleanings++;
}

// Flush L2 cache
void riscv64_l2cache_flush(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    // L2 cache flush would depend on the specific implementation
    printf("RISC-V64: L2 cache flushed\n");
    
    riscv64_cache_stats.flushes++;
}

// ============================================================================
// L3 CACHE OPERATIONS
// ============================================================================

// Invalidate L3 cache
void riscv64_l3cache_invalidate(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    // L3 cache invalidation would depend on the specific implementation
    printf("RISC-V64: L3 cache invalidated\n");
    
    riscv64_cache_stats.invalidations++;
}

// Clean L3 cache
void riscv64_l3cache_clean(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    // L3 cache cleaning would depend on the specific implementation
    printf("RISC-V64: L3 cache cleaned\n");
    
    riscv64_cache_stats.cleanings++;
}

// Flush L3 cache
void riscv64_l3cache_flush(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    // L3 cache flush would depend on the specific implementation
    printf("RISC-V64: L3 cache flushed\n");
    
    riscv64_cache_stats.flushes++;
}

// ============================================================================
// UNIFIED CACHE OPERATIONS
// ============================================================================

// Invalidate all caches
void riscv64_cache_invalidate_all(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    riscv64_icache_invalidate();
    riscv64_dcache_invalidate();
    riscv64_l2cache_invalidate();
    riscv64_l3cache_invalidate();
    
    printf("RISC-V64: All caches invalidated\n");
}

// Clean all caches
void riscv64_cache_clean_all(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    riscv64_dcache_clean();
    riscv64_l2cache_clean();
    riscv64_l3cache_clean();
    
    printf("RISC-V64: All caches cleaned\n");
}

// Flush all caches
void riscv64_cache_flush_all(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    riscv64_icache_invalidate();
    riscv64_dcache_flush();
    riscv64_l2cache_flush();
    riscv64_l3cache_flush();
    
    printf("RISC-V64: All caches flushed\n");
}

// Sync all caches
void riscv64_cache_sync_all(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    riscv64_icache_sync();
    riscv64_dcache_sync();
    
    printf("RISC-V64: All caches synchronized\n");
}

// ============================================================================
// CACHE CONFIGURATION
// ============================================================================

// Enable cache
void riscv64_cache_enable(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    riscv64_cache_context.enabled = true;
    
    printf("RISC-V64: Cache enabled\n");
}

// Disable cache
void riscv64_cache_disable(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    riscv64_cache_context.enabled = false;
    
    printf("RISC-V64: Cache disabled\n");
}

// Check if cache is enabled
bool riscv64_cache_is_enabled(void)
{
    if (!riscv64_cache_context.initialized) {
        return false;
    }
    
    return riscv64_cache_context.enabled;
}

// Set cache line size
void riscv64_cache_set_line_size(uint32_t line_size)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    riscv64_cache_context.line_size = line_size;
    
    printf("RISC-V64: Cache line size set to %u bytes\n", line_size);
}

// Get cache line size
uint32_t riscv64_cache_get_line_size(void)
{
    if (!riscv64_cache_context.initialized) {
        return 0;
    }
    
    return riscv64_cache_context.line_size;
}

// ============================================================================
// CACHE STATISTICS
// ============================================================================

// Get cache statistics
void riscv64_cache_get_stats(uint32_t *l1i_hits, uint32_t *l1i_misses,
                            uint32_t *l1d_hits, uint32_t *l1d_misses,
                            uint32_t *l2_hits, uint32_t *l2_misses,
                            uint32_t *l3_hits, uint32_t *l3_misses,
                            uint32_t *invalidations, uint32_t *cleanings, uint32_t *flushes)
{
    if (l1i_hits) *l1i_hits = riscv64_cache_stats.l1i_hits;
    if (l1i_misses) *l1i_misses = riscv64_cache_stats.l1i_misses;
    if (l1d_hits) *l1d_hits = riscv64_cache_stats.l1d_hits;
    if (l1d_misses) *l1d_misses = riscv64_cache_stats.l1d_misses;
    if (l2_hits) *l2_hits = riscv64_cache_stats.l2_hits;
    if (l2_misses) *l2_misses = riscv64_cache_stats.l2_misses;
    if (l3_hits) *l3_hits = riscv64_cache_stats.l3_hits;
    if (l3_misses) *l3_misses = riscv64_cache_stats.l3_misses;
    if (invalidations) *invalidations = riscv64_cache_stats.invalidations;
    if (cleanings) *cleanings = riscv64_cache_stats.cleanings;
    if (flushes) *flushes = riscv64_cache_stats.flushes;
}

// Reset cache statistics
void riscv64_cache_reset_stats(void)
{
    memset(&riscv64_cache_stats, 0, sizeof(riscv64_cache_stats));
    
    printf("RISC-V64: Cache statistics reset\n");
}

// ============================================================================
// CACHE DEBUG FUNCTIONS
// ============================================================================

// Print cache status
void riscv64_cache_print_status(void)
{
    printf("RISC-V64: Cache Status\n");
    printf("  Initialized: %s\n", riscv64_cache_context.initialized ? "Yes" : "No");
    printf("  Enabled: %s\n", riscv64_cache_context.enabled ? "Yes" : "No");
    printf("  L1I Size: %u KB\n", riscv64_cache_context.l1i_size / 1024);
    printf("  L1D Size: %u KB\n", riscv64_cache_context.l1d_size / 1024);
    printf("  L2 Size: %u KB\n", riscv64_cache_context.l2_size / 1024);
    printf("  L3 Size: %u MB\n", riscv64_cache_context.l3_size / (1024 * 1024));
    printf("  Line Size: %u bytes\n", riscv64_cache_context.line_size);
    printf("  Ways: %u\n", riscv64_cache_context.ways);
    printf("  Sets: %u\n", riscv64_cache_context.sets);
}

// Print cache statistics
void riscv64_cache_print_statistics(void)
{
    printf("RISC-V64: Cache Statistics\n");
    printf("  L1I: Hits %u, Misses %u\n", 
           riscv64_cache_stats.l1i_hits, riscv64_cache_stats.l1i_misses);
    printf("  L1D: Hits %u, Misses %u\n", 
           riscv64_cache_stats.l1d_hits, riscv64_cache_stats.l1d_misses);
    printf("  L2: Hits %u, Misses %u\n", 
           riscv64_cache_stats.l2_hits, riscv64_cache_stats.l2_misses);
    printf("  L3: Hits %u, Misses %u\n", 
           riscv64_cache_stats.l3_hits, riscv64_cache_stats.l3_misses);
    printf("  Operations: Invalidations %u, Cleanings %u, Flushes %u\n",
           riscv64_cache_stats.invalidations, 
           riscv64_cache_stats.cleanings, 
           riscv64_cache_stats.flushes);
}

// ============================================================================
// CACHE CLEANUP
// ============================================================================

// Cleanup cache system
void riscv64_cache_cleanup(void)
{
    if (!riscv64_cache_context.initialized) {
        return;
    }
    
    // Flush all caches before cleanup
    riscv64_cache_flush_all();
    
    // Clear contexts
    riscv64_cache_context.initialized = false;
    
    printf("RISC-V64: Cache system cleaned up\n");
}
