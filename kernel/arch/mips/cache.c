/*
 * ORION OS - MIPS Cache Management
 *
 * Cache management for MIPS architecture
 * Handles cache operations, invalidation, and maintenance
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
#include <stdlib.h>

// ============================================================================
// CACHE CONTEXT MANAGEMENT
// ============================================================================

static mips_cache_context_t mips_icache_context = {0};
static mips_cache_context_t mips_dcache_context = {0};
static mips_cache_context_t mips_l2_cache_context = {0};
static mips_cache_context_t mips_l3_cache_context = {0};

void mips_cache_init(void)
{
    memset(&mips_icache_context, 0, sizeof(mips_icache_context));
    memset(&mips_dcache_context, 0, sizeof(mips_dcache_context));
    memset(&mips_l2_cache_context, 0, sizeof(mips_l2_cache_context));
    memset(&mips_l3_cache_context, 0, sizeof(mips_l3_cache_context));

    // Initialize cache contexts with default values
    mips_icache_context.level = 1;
    mips_icache_context.way = 4;
    mips_icache_context.line_size = 32;
    mips_icache_context.total_size = 32768; // 32KB default
    mips_icache_context.enabled = true;
    mips_icache_context.write_back = false;
    mips_icache_context.write_allocate = false;

    mips_dcache_context.level = 1;
    mips_dcache_context.way = 4;
    mips_dcache_context.line_size = 32;
    mips_dcache_context.total_size = 32768; // 32KB default
    mips_dcache_context.enabled = true;
    mips_dcache_context.write_back = true;
    mips_dcache_context.write_allocate = true;

    mips_l2_cache_context.level = 2;
    mips_l2_cache_context.way = 8;
    mips_l2_cache_context.line_size = 64;
    mips_l2_cache_context.total_size = 262144; // 256KB default
    mips_l2_cache_context.enabled = true;
    mips_l2_cache_context.write_back = true;
    mips_l2_cache_context.write_allocate = true;

    mips_l3_cache_context.level = 3;
    mips_l3_cache_context.way = 16;
    mips_l3_cache_context.line_size = 128;
    mips_l3_cache_context.total_size = 2097152; // 2MB default
    mips_l3_cache_context.enabled = true;
    mips_l3_cache_context.write_back = true;
    mips_l3_cache_context.write_allocate = true;

    printf("MIPS: Cache contexts initialized\n");
}

mips_cache_context_t *mips_cache_get_icache_context(void)
{
    return &mips_icache_context;
}

mips_cache_context_t *mips_cache_get_dcache_context(void)
{
    return &mips_dcache_context;
}

mips_cache_context_t *mips_cache_get_l2_cache_context(void)
{
    return &mips_l2_cache_context;
}

mips_cache_context_t *mips_cache_get_l3_cache_context(void)
{
    return &mips_l3_cache_context;
}

// ============================================================================
// INSTRUCTION CACHE OPERATIONS
// ============================================================================

void mips_icache_invalidate_all(void)
{
    if (!mips_icache_context.enabled)
    {
        return;
    }

    // Invalidate all instruction cache entries
    __asm__ volatile("cache 0, 0($0)");

    printf("MIPS: Instruction cache invalidated\n");
}

void mips_icache_invalidate_line(mips_addr_t addr)
{
    if (!mips_icache_context.enabled)
    {
        return;
    }

    // Invalidate specific instruction cache line
    __asm__ volatile("cache 0, 0(%0)" : : "r"(addr));
}

void mips_icache_invalidate_range(mips_addr_t start, mips_addr_t end)
{
    if (!mips_icache_context.enabled)
    {
        return;
    }

    mips_addr_t addr = start;
    while (addr < end)
    {
        mips_icache_invalidate_line(addr);
        addr += mips_icache_context.line_size;
    }

    printf("MIPS: Instruction cache range invalidated: 0x%llx - 0x%llx\n", start, end);
}

void mips_icache_sync(void)
{
    if (!mips_icache_context.enabled)
    {
        return;
    }

    // Synchronize instruction cache
    __asm__ volatile("cache 0, 0($0)");
}

// ============================================================================
// DATA CACHE OPERATIONS
// ============================================================================

void mips_dcache_invalidate_all(void)
{
    if (!mips_dcache_context.enabled)
    {
        return;
    }

    // Invalidate all data cache entries
    __asm__ volatile("cache 1, 0($0)");

    printf("MIPS: Data cache invalidated\n");
}

void mips_dcache_invalidate_line(mips_addr_t addr)
{
    if (!mips_dcache_context.enabled)
    {
        return;
    }

    // Invalidate specific data cache line
    __asm__ volatile("cache 1, 0(%0)" : : "r"(addr));
}

void mips_dcache_invalidate_range(mips_addr_t start, mips_addr_t end)
{
    if (!mips_dcache_context.enabled)
    {
        return;
    }

    mips_addr_t addr = start;
    while (addr < end)
    {
        mips_dcache_invalidate_line(addr);
        addr += mips_dcache_context.line_size;
    }

    printf("MIPS: Data cache range invalidated: 0x%llx - 0x%llx\n", start, end);
}

void mips_dcache_clean_all(void)
{
    if (!mips_dcache_context.enabled)
    {
        return;
    }

    // Clean all data cache entries
    __asm__ volatile("cache 1, 1($0)");

    printf("MIPS: Data cache cleaned\n");
}

void mips_dcache_clean_line(mips_addr_t addr)
{
    if (!mips_dcache_context.enabled)
    {
        return;
    }

    // Clean specific data cache line
    __asm__ volatile("cache 1, 1(%0)" : : "r"(addr));
}

void mips_dcache_clean_range(mips_addr_t start, mips_addr_t end)
{
    if (!mips_dcache_context.enabled)
    {
        return;
    }

    mips_addr_t addr = start;
    while (addr < end)
    {
        mips_dcache_clean_line(addr);
        addr += mips_dcache_context.line_size;
    }

    printf("MIPS: Data cache range cleaned: 0x%llx - 0x%llx\n", start, end);
}

void mips_dcache_flush_all(void)
{
    if (!mips_dcache_context.enabled)
    {
        return;
    }

    // Flush all data cache entries
    __asm__ volatile("cache 1, 2($0)");

    printf("MIPS: Data cache flushed\n");
}

void mips_dcache_flush_line(mips_addr_t addr)
{
    if (!mips_dcache_context.enabled)
    {
        return;
    }

    // Flush specific data cache line
    __asm__ volatile("cache 1, 2(%0)" : : "r"(addr));
}

void mips_dcache_flush_range(mips_addr_t start, mips_addr_t end)
{
    if (!mips_dcache_context.enabled)
    {
        return;
    }

    mips_addr_t addr = start;
    while (addr < end)
    {
        mips_dcache_flush_line(addr);
        addr += mips_dcache_context.line_size;
    }

    printf("MIPS: Data cache range flushed: 0x%llx - 0x%llx\n", start, end);
}

void mips_dcache_sync(void)
{
    if (!mips_dcache_context.enabled)
    {
        return;
    }

    // Synchronize data cache
    __asm__ volatile("cache 1, 0($0)");
}

// ============================================================================
// L2 CACHE OPERATIONS
// ============================================================================

void mips_l2_cache_invalidate_all(void)
{
    if (!mips_l2_cache_context.enabled)
    {
        return;
    }

    // Invalidate all L2 cache entries
    __asm__ volatile("cache 2, 0($0)");

    printf("MIPS: L2 cache invalidated\n");
}

void mips_l2_cache_invalidate_line(mips_addr_t addr)
{
    if (!mips_l2_cache_context.enabled)
    {
        return;
    }

    // Invalidate specific L2 cache line
    __asm__ volatile("cache 2, 0(%0)" : : "r"(addr));
}

void mips_l2_cache_invalidate_range(mips_addr_t start, mips_addr_t end)
{
    if (!mips_l2_cache_context.enabled)
    {
        return;
    }

    mips_addr_t addr = start;
    while (addr < end)
    {
        mips_l2_cache_invalidate_line(addr);
        addr += mips_l2_cache_context.line_size;
    }

    printf("MIPS: L2 cache range invalidated: 0x%llx - 0x%llx\n", start, end);
}

void mips_l2_cache_clean_all(void)
{
    if (!mips_l2_cache_context.enabled)
    {
        return;
    }

    // Clean all L2 cache entries
    __asm__ volatile("cache 2, 1($0)");

    printf("MIPS: L2 cache cleaned\n");
}

void mips_l2_cache_clean_line(mips_addr_t addr)
{
    if (!mips_l2_cache_context.enabled)
    {
        return;
    }

    // Clean specific L2 cache line
    __asm__ volatile("cache 2, 1(%0)" : : "r"(addr));
}

void mips_l2_cache_clean_range(mips_addr_t start, mips_addr_t end)
{
    if (!mips_l2_cache_context.enabled)
    {
        return;
    }

    mips_addr_t addr = start;
    while (addr < end)
    {
        mips_l2_cache_clean_line(addr);
        addr += mips_l2_cache_context.line_size;
    }

    printf("MIPS: L2 cache range cleaned: 0x%llx - 0x%llx\n", start, end);
}

void mips_l2_cache_flush_all(void)
{
    if (!mips_l2_cache_context.enabled)
    {
        return;
    }

    // Flush all L2 cache entries
    __asm__ volatile("cache 2, 2($0)");

    printf("MIPS: L2 cache flushed\n");
}

void mips_l2_cache_flush_line(mips_addr_t addr)
{
    if (!mips_l2_cache_context.enabled)
    {
        return;
    }

    // Flush specific L2 cache line
    __asm__ volatile("cache 2, 2(%0)" : : "r"(addr));
}

void mips_l2_cache_flush_range(mips_addr_t start, mips_addr_t end)
{
    if (!mips_l2_cache_context.enabled)
    {
        return;
    }

    mips_addr_t addr = start;
    while (addr < end)
    {
        mips_l2_cache_flush_line(addr);
        addr += mips_l2_cache_context.line_size;
    }

    printf("MIPS: L2 cache range flushed: 0x%llx - 0x%llx\n", start, end);
}

// ============================================================================
// L3 CACHE OPERATIONS
// ============================================================================

void mips_l3_cache_invalidate_all(void)
{
    if (!mips_l3_cache_context.enabled)
    {
        return;
    }

    // Invalidate all L3 cache entries
    __asm__ volatile("cache 3, 0($0)");

    printf("MIPS: L3 cache invalidated\n");
}

void mips_l3_cache_invalidate_line(mips_addr_t addr)
{
    if (!mips_l3_cache_context.enabled)
    {
        return;
    }

    // Invalidate specific L3 cache line
    __asm__ volatile("cache 3, 0(%0)" : : "r"(addr));
}

void mips_l3_cache_invalidate_range(mips_addr_t start, mips_addr_t end)
{
    if (!mips_l3_cache_context.enabled)
    {
        return;
    }

    mips_addr_t addr = start;
    while (addr < end)
    {
        mips_l3_cache_invalidate_line(addr);
        addr += mips_l3_cache_context.line_size;
    }

    printf("MIPS: L3 cache range invalidated: 0x%llx - 0x%llx\n", start, end);
}

void mips_l3_cache_clean_all(void)
{
    if (!mips_l3_cache_context.enabled)
    {
        return;
    }

    // Clean all L3 cache entries
    __asm__ volatile("cache 3, 1($0)");

    printf("MIPS: L3 cache cleaned\n");
}

void mips_l3_cache_clean_line(mips_addr_t addr)
{
    if (!mips_l3_cache_context.enabled)
    {
        return;
    }

    // Clean specific L3 cache line
    __asm__ volatile("cache 3, 1(%0)" : : "r"(addr));
}

void mips_l3_cache_clean_range(mips_addr_t start, mips_addr_t end)
{
    if (!mips_l3_cache_context.enabled)
    {
        return;
    }

    mips_addr_t addr = start;
    while (addr < end)
    {
        mips_l3_cache_clean_line(addr);
        addr += mips_l3_cache_context.line_size;
    }

    printf("MIPS: L3 cache range cleaned: 0x%llx - 0x%llx\n", start, end);
}

void mips_l3_cache_flush_all(void)
{
    if (!mips_l3_cache_context.enabled)
    {
        return;
    }

    // Flush all L3 cache entries
    __asm__ volatile("cache 3, 2($0)");

    printf("MIPS: L3 cache flushed\n");
}

void mips_l3_cache_flush_line(mips_addr_t addr)
{
    if (!mips_l3_cache_context.enabled)
    {
        return;
    }

    // Flush specific L3 cache line
    __asm__ volatile("cache 3, 2(%0)" : : "r"(addr));
}

void mips_l3_cache_flush_range(mips_addr_t start, mips_addr_t end)
{
    if (!mips_l3_cache_context.enabled)
    {
        return;
    }

    mips_addr_t addr = start;
    while (addr < end)
    {
        mips_l3_cache_flush_line(addr);
        addr += mips_l3_cache_context.line_size;
    }

    printf("MIPS: L3 cache range flushed: 0x%llx - 0x%llx\n", start, end);
}

// ============================================================================
// UNIFIED CACHE OPERATIONS
// ============================================================================

void mips_cache_invalidate_all(void)
{
    mips_icache_invalidate_all();
    mips_dcache_invalidate_all();
    mips_l2_cache_invalidate_all();
    mips_l3_cache_invalidate_all();

    printf("MIPS: All caches invalidated\n");
}

void mips_cache_clean_all(void)
{
    mips_dcache_clean_all();
    mips_l2_cache_clean_all();
    mips_l3_cache_clean_all();

    printf("MIPS: All caches cleaned\n");
}

void mips_cache_flush_all(void)
{
    mips_dcache_flush_all();
    mips_l2_cache_flush_all();
    mips_l3_cache_flush_all();

    printf("MIPS: All caches flushed\n");
}

void mips_cache_sync_all(void)
{
    mips_icache_sync();
    mips_dcache_sync();

    printf("MIPS: All caches synchronized\n");
}

// ============================================================================
// CACHE CONFIGURATION
// ============================================================================

void mips_cache_enable_icache(void)
{
    mips_icache_context.enabled = true;
    printf("MIPS: Instruction cache enabled\n");
}

void mips_cache_disable_icache(void)
{
    mips_icache_context.enabled = false;
    printf("MIPS: Instruction cache disabled\n");
}

void mips_cache_enable_dcache(void)
{
    mips_dcache_context.enabled = true;
    printf("MIPS: Data cache enabled\n");
}

void mips_cache_disable_dcache(void)
{
    mips_dcache_context.enabled = false;
    printf("MIPS: Data cache disabled\n");
}

void mips_cache_enable_l2_cache(void)
{
    mips_l2_cache_context.enabled = true;
    printf("MIPS: L2 cache enabled\n");
}

void mips_cache_disable_l2_cache(void)
{
    mips_l2_cache_context.enabled = false;
    printf("MIPS: L2 cache disabled\n");
}

void mips_cache_enable_l3_cache(void)
{
    mips_l3_cache_context.enabled = true;
    printf("MIPS: L3 cache enabled\n");
}

void mips_cache_disable_l3_cache(void)
{
    mips_l3_cache_context.enabled = false;
    printf("MIPS: L3 cache disabled\n");
}

// ============================================================================
// CACHE STATISTICS
// ============================================================================

void mips_cache_print_statistics(void)
{
    printf("MIPS: Cache Statistics:\n");
    printf("  Instruction Cache: %s, %uKB, %u-way, %u-byte lines\n",
           mips_icache_context.enabled ? "enabled" : "disabled",
           mips_icache_context.total_size / 1024,
           mips_icache_context.way,
           mips_icache_context.line_size);

    printf("  Data Cache: %s, %uKB, %u-way, %u-byte lines\n",
           mips_dcache_context.enabled ? "enabled" : "disabled",
           mips_dcache_context.total_size / 1024,
           mips_dcache_context.way,
           mips_dcache_context.line_size);

    printf("  L2 Cache: %s, %uKB, %u-way, %u-byte lines\n",
           mips_l2_cache_context.enabled ? "enabled" : "disabled",
           mips_l2_cache_context.total_size / 1024,
           mips_l2_cache_context.way,
           mips_l2_cache_context.line_size);

    printf("  L3 Cache: %s, %uKB, %u-way, %u-byte lines\n",
           mips_l3_cache_context.enabled ? "enabled" : "disabled",
           mips_l3_cache_context.total_size / 1024,
           mips_l3_cache_context.way,
           mips_l3_cache_context.line_size);
}

// ============================================================================
// CACHE UTILITY FUNCTIONS
// ============================================================================

mips_addr_t mips_cache_get_line_address(mips_addr_t addr, uint32_t line_size)
{
    return addr & ~(line_size - 1);
}

mips_addr_t mips_cache_get_next_line_address(mips_addr_t addr, uint32_t line_size)
{
    return mips_cache_get_line_address(addr, line_size) + line_size;
}

bool mips_cache_is_line_aligned(mips_addr_t addr, uint32_t line_size)
{
    return (addr & (line_size - 1)) == 0;
}

uint32_t mips_cache_get_line_offset(mips_addr_t addr, uint32_t line_size)
{
    return addr & (line_size - 1);
}

uint32_t mips_cache_get_set_index(mips_addr_t addr, uint32_t line_size, uint32_t way)
{
    return (addr / line_size) % way;
}

uint32_t mips_cache_get_tag(mips_addr_t addr, uint32_t line_size, uint32_t way)
{
    return addr / (line_size * way);
}
