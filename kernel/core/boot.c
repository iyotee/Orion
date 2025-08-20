/*
 * Orion Boot Protocol - Kernel Implementation
 * 
 * This module handles boot information processing and validation
 * for the Orion operating system kernel.
 * 
 * Copyright (c) 2024 Orion OS Project
 * License: MIT
 */

#include <orion/boot.h>
#include <orion/kernel.h>
#include <orion/mm.h>

// Global boot information
struct orion_boot_info *g_boot_info = NULL;
struct orion_memory_info *g_memory_info = NULL;
struct orion_efi_info *g_efi_info = NULL;

/**
 * Initialize boot information processing
 */
int orion_boot_init(const struct orion_boot_info *boot_info)
{
    if (!boot_info) {
        return -EINVAL;
    }

    // Validate boot information
    if (!orion_boot_validate(boot_info)) {
        kprintf("ERROR: Invalid boot information\n");
        return -EINVAL;
    }

    // Store global reference
    g_boot_info = (struct orion_boot_info *)boot_info;

    // Parse boot information structures
    const uint8_t *ptr = (const uint8_t *)boot_info + sizeof(struct orion_boot_info);
    
    for (uint32_t i = 0; i < boot_info->info_count; i++) {
        const struct orion_info_tag *tag = (const struct orion_info_tag *)ptr;
        
        switch (tag->type) {
            case ORION_INFO_MEMORY:
                g_memory_info = (struct orion_memory_info *)tag;
                kprintf("Boot: Memory information found\n");
                break;
                
            case ORION_INFO_EFI:
                g_efi_info = (struct orion_efi_info *)tag;
                kprintf("Boot: EFI information found\n");
                break;
                
            case ORION_INFO_BOOTLOADER:
                kprintf("Boot: Bootloader information found\n");
                break;
                
            default:
                kprintf("Boot: Unknown info type: 0x%x\n", tag->type);
                break;
        }
        
        ptr += tag->size;
    }

    kprintf("Boot: Orion Boot Protocol v%d.%d initialized\n",
            (boot_info->version >> 16) & 0xFFFF,
            boot_info->version & 0xFFFF);

    return 0;
}

/**
 * Get memory map information
 */
int orion_boot_get_memory_info(struct orion_memory_info **memory_info)
{
    if (!memory_info) {
        return -EINVAL;
    }

    *memory_info = g_memory_info;
    return g_memory_info ? 0 : -ENOENT;
}

/**
 * Get bootloader information
 */
int orion_boot_get_bootloader_info(struct orion_bootloader_info **bootloader_info)
{
    if (!bootloader_info) {
        return -EINVAL;
    }

    // Search for bootloader info in boot information
    if (!g_boot_info) {
        return -ENOENT;
    }

    const uint8_t *ptr = (const uint8_t *)g_boot_info + sizeof(struct orion_boot_info);
    
    for (uint32_t i = 0; i < g_boot_info->info_count; i++) {
        const struct orion_info_tag *tag = (const struct orion_info_tag *)ptr;
        
        if (tag->type == ORION_INFO_BOOTLOADER) {
            *bootloader_info = (struct orion_bootloader_info *)tag;
            return 0;
        }
        
        ptr += tag->size;
    }

    return -ENOENT;
}

/**
 * Get EFI system information
 */
int orion_boot_get_efi_info(struct orion_efi_info **efi_info)
{
    if (!efi_info) {
        return -EINVAL;
    }

    *efi_info = g_efi_info;
    return g_efi_info ? 0 : -ENOENT;
}

/**
 * Validate boot information integrity
 */
int orion_boot_validate(const struct orion_boot_info *boot_info)
{
    if (!boot_info) {
        return 0;
    }

    // Check magic number
    if (boot_info->magic != ORION_BOOT_MAGIC) {
        kprintf("Boot: Invalid magic number: 0x%x (expected 0x%x)\n",
                boot_info->magic, ORION_BOOT_MAGIC);
        return 0;
    }

    // Check version compatibility
    uint32_t major = (boot_info->version >> 16) & 0xFFFF;
    uint32_t minor = boot_info->version & 0xFFFF;
    
    if (major != 1 || minor > 0) {
        kprintf("Boot: Unsupported protocol version: %d.%d\n", major, minor);
        return 0;
    }

    // Validate checksums
    uint32_t header_sum = orion_checksum(boot_info, sizeof(struct orion_boot_info));
    if (header_sum != boot_info->header_checksum) {
        kprintf("Boot: Header checksum mismatch\n");
        return 0;
    }

    // Validate total size
    if (boot_info->total_size < sizeof(struct orion_boot_info)) {
        kprintf("Boot: Invalid total size: %d\n", boot_info->total_size);
        return 0;
    }

    return 1;
}

/**
 * Print boot information for debugging
 */
void orion_boot_debug_print(const struct orion_boot_info *boot_info)
{
    if (!boot_info) {
        kprintf("Boot: No boot information available\n");
        return;
    }

    kprintf("=== Orion Boot Information ===\n");
    kprintf("Magic: 0x%08x\n", boot_info->magic);
    kprintf("Version: %d.%d\n", 
            (boot_info->version >> 16) & 0xFFFF,
            boot_info->version & 0xFFFF);
    kprintf("Total size: %d bytes\n", boot_info->total_size);
    kprintf("Info count: %d\n", boot_info->info_count);
    kprintf("Boot timestamp: %lu\n", boot_info->boot_timestamp);
    kprintf("Kernel load time: %d ms\n", boot_info->kernel_load_time);

    // Print memory information if available
    if (g_memory_info) {
        kprintf("\n=== Memory Information ===\n");
        kprintf("Total memory: %lu bytes (%.2f MB)\n",
                g_memory_info->total_memory,
                g_memory_info->total_memory / (1024.0 * 1024.0));
        kprintf("Available memory: %lu bytes (%.2f MB)\n",
                g_memory_info->available_memory,
                g_memory_info->available_memory / (1024.0 * 1024.0));
        kprintf("Memory map entries: %d\n", g_memory_info->memory_map_entries);
    }

    // Print EFI information if available
    if (g_efi_info) {
        kprintf("\n=== EFI Information ===\n");
        kprintf("System table: 0x%016lx\n", g_efi_info->system_table);
        kprintf("Boot services: 0x%016lx\n", g_efi_info->boot_services);
        kprintf("Runtime services: 0x%016lx\n", g_efi_info->runtime_services);
        kprintf("Firmware revision: %d\n", g_efi_info->firmware_revision);
    }

    kprintf("===============================\n");
}
