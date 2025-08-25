/*
 * ORION OS - POWER Architecture Cache Management
 *
 * Cache operations and management for POWER architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include <string.h>
#include <stdio.h>

// ============================================================================
// CACHE CONFIGURATION
// ============================================================================

#define POWER_CACHE_LINE_SIZE_32  32
#define POWER_CACHE_LINE_SIZE_64  64
#define POWER_CACHE_LINE_SIZE_128 128

typedef struct {
    uint64_t l1i_size;        // L1 instruction cache size
    uint64_t l1d_size;        // L1 data cache size
    uint64_t l2_size;         // L2 cache size
    uint64_t l3_size;         // L3 cache size
    uint64_t l1i_line;        // L1 instruction cache line size
    uint64_t l1d_line;        // L1 data cache line size
    uint64_t l2_line;         // L2 cache line size
    uint64_t l3_line;         // L3 cache line size
    uint64_t l1i_ways;        // L1 instruction cache associativity
    uint64_t l1d_ways;        // L1 data cache associativity
    uint64_t l2_ways;         // L2 cache associativity
    uint64_t l3_ways;         // L3 cache associativity
    bool l1i_enabled;         // L1 instruction cache enabled
    bool l1d_enabled;         // L1 data cache enabled
    bool l2_enabled;          // L2 cache enabled
    bool l3_enabled;          // L3 cache enabled
} power_cache_config_t;

static power_cache_config_t power_cache;

// ============================================================================
// CACHE INITIALIZATION
// ============================================================================

int power_cache_init(void)
{
    memset(&power_cache, 0, sizeof(power_cache));
    
    // Set default cache sizes and line sizes
    power_cache.l1i_size = POWER_L1I_CACHE_SIZE;
    power_cache.l1d_size = POWER_L1D_CACHE_SIZE;
    power_cache.l2_size = POWER_L2_CACHE_SIZE;
    power_cache.l3_size = POWER_L3_CACHE_SIZE;
    
    power_cache.l1i_line = POWER_L1_CACHE_LINE_SIZE;
    power_cache.l1d_line = POWER_L1_CACHE_LINE_SIZE;
    power_cache.l2_line = POWER_L2_CACHE_LINE_SIZE;
    power_cache.l3_line = POWER_L3_CACHE_LINE_SIZE;
    
    // Set default associativity
    power_cache.l1i_ways = 4;  // 4-way set associative
    power_cache.l1d_ways = 4;  // 4-way set associative
    power_cache.l2_ways = 8;   // 8-way set associative
    power_cache.l3_ways = 16;  // 16-way set associative
    
    // Enable all caches by default
    power_cache.l1i_enabled = true;
    power_cache.l1d_enabled = true;
    power_cache.l2_enabled = true;
    power_cache.l3_enabled = true;
    
    // Detect actual cache configuration from hardware
    return power_cache_detect_config();
}

int power_cache_detect_config(void)
{
    // Read cache configuration from hardware
    // This would typically involve reading from device tree or CPU registers
    
    // For now, use default values
    // In a real implementation, this would detect actual hardware configuration
    
    return 0;
}

// ============================================================================
// L1 INSTRUCTION CACHE OPERATIONS
// ============================================================================

int power_cache_flush_l1i(void)
{
    if (!power_cache.l1i_enabled) {
        return 0;
    }
    
    // Flush L1 instruction cache using icbi instruction
    // This invalidates all instruction cache lines
    
    uint64_t cache_size = power_cache.l1i_size;
    uint64_t line_size = power_cache.l1i_line;
    
    for (uint64_t addr = 0; addr < cache_size; addr += line_size) {
        __asm__ volatile("icbi 0, %0" : : "r"(addr));
    }
    
    // Synchronize to ensure all operations complete
    __asm__ volatile("sync");
    
    return 0;
}

int power_cache_invalidate_l1i(void)
{
    if (!power_cache.l1i_enabled) {
        return 0;
    }
    
    // Invalidate L1 instruction cache
    // This is similar to flush but doesn't write back dirty data
    
    uint64_t cache_size = power_cache.l1i_size;
    uint64_t line_size = power_cache.l1i_line;
    
    for (uint64_t addr = 0; addr < cache_size; addr += line_size) {
        __asm__ volatile("icbi 0, %0" : : "r"(addr));
    }
    
    // Synchronize to ensure all operations complete
    __asm__ volatile("sync");
    
    return 0;
}

int power_cache_flush_l1i_range(uint64_t start, uint64_t end)
{
    if (!power_cache.l1i_enabled) {
        return 0;
    }
    
    // Flush L1 instruction cache range
    uint64_t line_size = power_cache.l1i_line;
    uint64_t aligned_start = start & ~(line_size - 1);
    uint64_t aligned_end = (end + line_size - 1) & ~(line_size - 1);
    
    for (uint64_t addr = aligned_start; addr < aligned_end; addr += line_size) {
        __asm__ volatile("icbi 0, %0" : : "r"(addr));
    }
    
    // Synchronize to ensure all operations complete
    __asm__ volatile("sync");
    
    return 0;
}

// ============================================================================
// L1 DATA CACHE OPERATIONS
// ============================================================================

int power_cache_flush_l1d(void)
{
    if (!power_cache.l1d_enabled) {
        return 0;
    }
    
    // Flush L1 data cache using dcbf instruction
    // This writes back all dirty cache lines and invalidates them
    
    uint64_t cache_size = power_cache.l1d_size;
    uint64_t line_size = power_cache.l1d_line;
    
    for (uint64_t addr = 0; addr < cache_size; addr += line_size) {
        __asm__ volatile("dcbf 0, %0" : : "r"(addr));
    }
    
    // Synchronize to ensure all operations complete
    __asm__ volatile("sync");
    
    return 0;
}

int power_cache_invalidate_l1d(void)
{
    if (!power_cache.l1d_enabled) {
        return 0;
    }
    
    // Invalidate L1 data cache using dcbi instruction
    // This invalidates cache lines without writing back dirty data
    
    uint64_t cache_size = power_cache.l1d_size;
    uint64_t line_size = power_cache.l1d_line;
    
    for (uint64_t addr = 0; addr < cache_size; addr += line_size) {
        __asm__ volatile("dcbi 0, %0" : : "r"(addr));
    }
    
    // Synchronize to ensure all operations complete
    __asm__ volatile("sync");
    
    return 0;
}

int power_cache_flush_l1d_range(uint64_t start, uint64_t end)
{
    if (!power_cache.l1d_enabled) {
        return 0;
    }
    
    // Flush L1 data cache range
    uint64_t line_size = power_cache.l1d_line;
    uint64_t aligned_start = start & ~(line_size - 1);
    uint64_t aligned_end = (end + line_size - 1) & ~(line_size - 1);
    
    for (uint64_t addr = aligned_start; addr < aligned_end; addr += line_size) {
        __asm__ volatile("dcbf 0, %0" : : "r"(addr));
    }
    
    // Synchronize to ensure all operations complete
    __asm__ volatile("sync");
    
    return 0;
}

int power_cache_invalidate_l1d_range(uint64_t start, uint64_t end)
{
    if (!power_cache.l1d_enabled) {
        return 0;
    }
    
    // Invalidate L1 data cache range
    uint64_t line_size = power_cache.l1d_line;
    uint64_t aligned_start = start & ~(line_size - 1);
    uint64_t aligned_end = (end + line_size - 1) & ~(line_size - 1);
    
    for (uint64_t addr = aligned_start; addr < aligned_end; addr += line_size) {
        __asm__ volatile("dcbi 0, %0" : : "r"(addr));
    }
    
    // Synchronize to ensure all operations complete
    __asm__ volatile("sync");
    
    return 0;
}

// ============================================================================
// L2 CACHE OPERATIONS
// ============================================================================

int power_cache_flush_l2(void)
{
    if (!power_cache.l2_enabled) {
        return 0;
    }
    
    // Flush L2 cache
    // This would typically involve specific L2 cache operations
    // For now, use a generic approach
    
    uint64_t cache_size = power_cache.l2_size;
    uint64_t line_size = power_cache.l2_line;
    
    for (uint64_t addr = 0; addr < cache_size; addr += line_size) {
        // Use dcbf with L2 hint
        __asm__ volatile("dcbf 1, %0" : : "r"(addr));
    }
    
    // Synchronize to ensure all operations complete
    __asm__ volatile("sync");
    
    return 0;
}

int power_cache_invalidate_l2(void)
{
    if (!power_cache.l2_enabled) {
        return 0;
    }
    
    // Invalidate L2 cache
    // This would typically involve specific L2 cache operations
    
    uint64_t cache_size = power_cache.l2_size;
    uint64_t line_size = power_cache.l2_line;
    
    for (uint64_t addr = 0; addr < cache_size; addr += line_size) {
        // Use dcbi with L2 hint
        __asm__ volatile("dcbi 1, %0" : : "r"(addr));
    }
    
    // Synchronize to ensure all operations complete
    __asm__ volatile("sync");
    
    return 0;
}

// ============================================================================
// L3 CACHE OPERATIONS
// ============================================================================

int power_cache_flush_l3(void)
{
    if (!power_cache.l3_enabled) {
        return 0;
    }
    
    // Flush L3 cache
    // This would typically involve specific L3 cache operations
    
    uint64_t cache_size = power_cache.l3_size;
    uint64_t line_size = power_cache.l3_line;
    
    for (uint64_t addr = 0; addr < cache_size; addr += line_size) {
        // Use dcbf with L3 hint
        __asm__ volatile("dcbf 2, %0" : : "r"(addr));
    }
    
    // Synchronize to ensure all operations complete
    __asm__ volatile("sync");
    
    return 0;
}

int power_cache_invalidate_l3(void)
{
    if (!power_cache.l3_enabled) {
        return 0;
    }
    
    // Invalidate L3 cache
    // This would typically involve specific L3 cache operations
    
    uint64_t cache_size = power_cache.l3_size;
    uint64_t line_size = power_cache.l3_line;
    
    for (uint64_t addr = 0; addr < cache_size; addr += line_size) {
        // Use dcbi with L3 hint
        __asm__ volatile("dcbi 2, %0" : : "r"(addr));
    }
    
    // Synchronize to ensure all operations complete
    __asm__ volatile("sync");
    
    return 0;
}

// ============================================================================
// COMPREHENSIVE CACHE OPERATIONS
// ============================================================================

int power_cache_flush_all(void)
{
    // Flush all cache levels
    int ret;
    
    ret = power_cache_flush_l3();
    if (ret != 0) return ret;
    
    ret = power_cache_flush_l2();
    if (ret != 0) return ret;
    
    ret = power_cache_flush_l1d();
    if (ret != 0) return ret;
    
    ret = power_cache_flush_l1i();
    if (ret != 0) return ret;
    
    return 0;
}

int power_cache_invalidate_all(void)
{
    // Invalidate all cache levels
    int ret;
    
    ret = power_cache_invalidate_l3();
    if (ret != 0) return ret;
    
    ret = power_cache_invalidate_l2();
    if (ret != 0) return ret;
    
    ret = power_cache_invalidate_l1d();
    if (ret != 0) return ret;
    
    ret = power_cache_invalidate_l1i();
    if (ret != 0) return ret;
    
    return 0;
}

int power_cache_sync(void)
{
    // Synchronize all cache operations
    __asm__ volatile("sync");
    return 0;
}

// ============================================================================
// CACHE PREFETCHING
// ============================================================================

int power_cache_prefetch_l1i(uint64_t addr)
{
    if (!power_cache.l1i_enabled) {
        return 0;
    }
    
    // Prefetch instruction into L1 instruction cache
    // Use dcbt with instruction cache hint
    __asm__ volatile("dcbt 0, %0" : : "r"(addr));
    
    return 0;
}

int power_cache_prefetch_l1d(uint64_t addr)
{
    if (!power_cache.l1d_enabled) {
        return 0;
    }
    
    // Prefetch data into L1 data cache
    // Use dcbt with data cache hint
    __asm__ volatile("dcbt 0, %0" : : "r"(addr));
    
    return 0;
}

int power_cache_prefetch_l2(uint64_t addr)
{
    if (!power_cache.l2_enabled) {
        return 0;
    }
    
    // Prefetch data into L2 cache
    // Use dcbt with L2 cache hint
    __asm__ volatile("dcbt 1, %0" : : "r"(addr));
    
    return 0;
}

int power_cache_prefetch_l3(uint64_t addr)
{
    if (!power_cache.l3_enabled) {
        return 0;
    }
    
    // Prefetch data into L3 cache
    // Use dcbt with L3 cache hint
    __asm__ volatile("dcbt 2, %0" : : "r"(addr));
    
    return 0;
}

// ============================================================================
// CACHE CONTROL
// ============================================================================

int power_cache_enable_l1i(void)
{
    power_cache.l1i_enabled = true;
    return 0;
}

int power_cache_disable_l1i(void)
{
    power_cache.l1i_enabled = false;
    // Flush L1i before disabling
    return power_cache_flush_l1i();
}

int power_cache_enable_l1d(void)
{
    power_cache.l1d_enabled = true;
    return 0;
}

int power_cache_disable_l1d(void)
{
    power_cache.l1d_enabled = false;
    // Flush L1d before disabling
    return power_cache_flush_l1d();
}

int power_cache_enable_l2(void)
{
    power_cache.l2_enabled = true;
    return 0;
}

int power_cache_disable_l2(void)
{
    power_cache.l2_enabled = false;
    // Flush L2 before disabling
    return power_cache_flush_l2();
}

int power_cache_enable_l3(void)
{
    power_cache.l3_enabled = true;
    return 0;
}

int power_cache_disable_l3(void)
{
    power_cache.l3_enabled = false;
    // Flush L3 before disabling
    return power_cache_flush_l3();
}

// ============================================================================
// CACHE STATISTICS
// ============================================================================

int power_cache_get_stats(power_cache_stats_t *stats)
{
    if (!stats) {
        return -1;
    }
    
    stats->l1i_size = power_cache.l1i_size;
    stats->l1d_size = power_cache.l1d_size;
    stats->l2_size = power_cache.l2_size;
    stats->l3_size = power_cache.l3_size;
    
    stats->l1i_line = power_cache.l1i_line;
    stats->l1d_line = power_cache.l1d_line;
    stats->l2_line = power_cache.l2_line;
    stats->l3_line = power_cache.l3_line;
    
    stats->l1i_ways = power_cache.l1i_ways;
    stats->l1d_ways = power_cache.l1d_ways;
    stats->l2_ways = power_cache.l2_ways;
    stats->l3_ways = power_cache.l3_ways;
    
    stats->l1i_enabled = power_cache.l1i_enabled;
    stats->l1d_enabled = power_cache.l1d_enabled;
    stats->l2_enabled = power_cache.l2_enabled;
    stats->l3_enabled = power_cache.l3_enabled;
    
    return 0;
}

// ============================================================================
// CACHE CLEANUP
// ============================================================================

void power_cache_cleanup(void)
{
    // Flush all caches
    power_cache_flush_all();
    
    // Clear cache configuration
    memset(&power_cache, 0, sizeof(power_cache));
}
