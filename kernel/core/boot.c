/*
 * Orion Operating System - Boot Information Processing
 *
 * This module handles boot information passed from the Orion bootloader
 * via the Orion Boot Protocol.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/kernel.h>
#include <orion/types.h>
#include "orion-boot-protocol.h"
#include <orion/mm.h>

// Missing constants
#define ORION_BOOT_VERSION_MIN 0x00010000
#define OR_ENODATA -2

// ========================================
// BOOT INFORMATION PROCESSING
// ========================================

// Global boot information
static struct orion_boot_info *g_boot_info = NULL;
static bool boot_info_initialized = false;

/**
 * Initialize boot information processing
 * @param boot_info Pointer to boot information from bootloader
 * @return 0 on success, negative error code on failure
 */
int orion_boot_init(const struct orion_boot_info *boot_info)
{
    if (!boot_info)
    {
        kerror("Boot info is NULL");
        return -OR_EINVAL;
    }

    // Validate boot information
    if (!orion_boot_validate(boot_info))
    {
        kerror("Boot information validation failed");
        return -OR_EINVAL;
    }

    // Store boot information
    g_boot_info = (struct orion_boot_info *)boot_info;
    boot_info_initialized = true;

    kinfo("Boot information initialized successfully");
    return OR_OK;
}

/**
 * Get memory map information
 * @param memory_info Output pointer for memory information
 * @return 0 on success, negative error code on failure
 */
int orion_boot_get_memory_info(struct orion_memory_info **memory_info)
{
    if (!boot_info_initialized || !g_boot_info)
    {
        return -OR_EINVAL;
    }

    // Extract memory map from boot info
    if (!memory_info)
    {
        return -OR_EINVAL;
    }

    // Walk through boot info structures to find memory info
    uint8_t *data = (uint8_t *)g_boot_info + sizeof(struct orion_boot_info);
    uint32_t offset = 0;

    while (offset < g_boot_info->total_size - sizeof(struct orion_boot_info))
    {
        struct orion_info_tag *header = (struct orion_info_tag *)(data + offset);

        if (header->type == ORION_INFO_MEMORY && header->size >= sizeof(struct orion_memory_info))
        {
            *memory_info = (struct orion_memory_info *)header;
            kinfo("Found memory info: %llu KB total, %llu KB available",
                  (unsigned long long)((*memory_info)->total_memory / 1024),
                  (unsigned long long)((*memory_info)->available_memory / 1024));
            return OR_OK;
        }

        offset += header->size;
    }

    // No memory info found
    *memory_info = NULL;
    kwarn("No memory information found in boot data");
    return -OR_ENODATA;
}

/**
 * Get bootloader information
 * @param bootloader_info Output pointer for bootloader information
 * @return 0 on success, negative error code on failure
 */
int orion_boot_get_bootloader_info(struct orion_bootloader_info **bootloader_info)
{
    if (!boot_info_initialized || !g_boot_info)
    {
        return -OR_EINVAL;
    }

    // Extract bootloader info from boot data
    if (!bootloader_info)
    {
        return -OR_EINVAL;
    }

    // Walk through boot info structures to find bootloader info
    uint8_t *data = (uint8_t *)g_boot_info + sizeof(struct orion_boot_info);
    uint32_t offset = 0;

    while (offset < g_boot_info->total_size - sizeof(struct orion_boot_info))
    {
        struct orion_info_tag *header = (struct orion_info_tag *)(data + offset);

        if (header->type == ORION_INFO_BOOTLOADER && header->size >= sizeof(struct orion_bootloader_info))
        {
            *bootloader_info = (struct orion_bootloader_info *)header;
            kinfo("Found bootloader info: version %s, features 0x%x",
                  (*bootloader_info)->version, (*bootloader_info)->features);
            return OR_OK;
        }

        offset += header->size;
    }

    // No bootloader info found
    *bootloader_info = NULL;
    kwarn("No bootloader information found in boot data");
    return -1;
}

/**
 * Get EFI system information
 * @param efi_info Output pointer for EFI information
 * @return 0 on success, negative error code on failure
 */
int orion_boot_get_efi_info(struct orion_efi_info **efi_info)
{
    if (!boot_info_initialized || !g_boot_info)
    {
        return -OR_EINVAL;
    }

    // Extract EFI info from boot data
    if (!efi_info)
    {
        return -OR_EINVAL;
    }

    // Walk through boot info structures to find EFI info
    uint8_t *data = (uint8_t *)g_boot_info + sizeof(struct orion_boot_info);
    uint32_t offset = 0;

    while (offset < g_boot_info->total_size - sizeof(struct orion_boot_info))
    {
        struct orion_info_tag *header = (struct orion_info_tag *)(data + offset);

        if (header->type == ORION_INFO_EFI && header->size >= sizeof(struct orion_efi_info))
        {
            *efi_info = (struct orion_efi_info *)header;
            kinfo("Found EFI info: system table at 0x%llx, revision 0x%x",
                  (unsigned long long)(*efi_info)->system_table,
                  (*efi_info)->firmware_revision);
            return OR_OK;
        }

        offset += header->size;
    }

    // No EFI info found
    *efi_info = NULL;
    kwarn("No EFI information found in boot data");
    return -1;
}

/**
 * Validate boot information integrity
 * @param boot_info Pointer to boot information to validate
 * @return 1 if valid, 0 if invalid
 */
int orion_boot_validate(const struct orion_boot_info *boot_info)
{
    if (!boot_info)
    {
        return 0;
    }

    // Validate magic number
    if (boot_info->magic != ORION_BOOT_MAGIC)
    {
        kerror("Invalid boot info magic: 0x%x (expected 0x%x)",
               boot_info->magic, ORION_BOOT_MAGIC);
        return 0;
    }

    // Validate version
    if (boot_info->version < ORION_BOOT_VERSION_MIN ||
        boot_info->version > ORION_BOOT_VERSION)
    {
        kerror("Unsupported boot info version: 0x%x", boot_info->version);
        return 0;
    }

    // Validate checksums
    uint32_t calculated_header_checksum = orion_checksum(boot_info, sizeof(struct orion_boot_info) - sizeof(uint32_t));
    if (calculated_header_checksum != boot_info->header_checksum)
    {
        kerror("Boot info header checksum mismatch: 0x%x vs 0x%x",
               calculated_header_checksum, boot_info->header_checksum);
        return 0;
    }

    if (boot_info->total_size > sizeof(struct orion_boot_info))
    {
        uint8_t *data = (uint8_t *)boot_info + sizeof(struct orion_boot_info);
        uint32_t data_size = boot_info->total_size - sizeof(struct orion_boot_info);
        uint32_t calculated_data_checksum = orion_checksum(data, data_size);
        if (calculated_data_checksum != boot_info->data_checksum)
        {
            kerror("Boot info data checksum mismatch: 0x%x vs 0x%x",
                   calculated_data_checksum, boot_info->data_checksum);
            return 0;
        }
    }

    // Validate structure count and data size consistency
    uint8_t *data = (uint8_t *)boot_info + sizeof(struct orion_boot_info);
    uint32_t offset = 0;
    uint32_t structure_count = 0;
    uint32_t data_size = boot_info->total_size - sizeof(struct orion_boot_info);

    while (offset < data_size)
    {
        struct orion_info_tag *header = (struct orion_info_tag *)(data + offset);

        // Validate structure header
        if (header->size < sizeof(struct orion_info_tag))
        {
            kerror("Invalid structure size: %u", header->size);
            return 0;
        }

        if (offset + header->size > data_size)
        {
            kerror("Structure extends beyond data size");
            return 0;
        }

        structure_count++;
        offset += header->size;
    }

    if (structure_count != boot_info->info_count)
    {
        kerror("Structure count mismatch: %u vs %u",
               structure_count, boot_info->info_count);
        return 0;
    }

    kinfo("Boot info validation passed");
    return 1;
}

/**
 * Print boot information for debugging
 * @param boot_info Pointer to boot information
 */
void orion_boot_debug_print(const struct orion_boot_info *boot_info)
{
    if (!boot_info)
    {
        kerror("Cannot print NULL boot info");
        return;
    }

    kinfo("=== ORION BOOT INFORMATION ===");
    kinfo("Boot info address: 0x%llx", (unsigned long long)boot_info);
    kinfo("Magic: 0x%x", boot_info->magic);
    kinfo("Version: 0x%x", boot_info->version);
    kinfo("Total size: %u bytes", boot_info->total_size);
    kinfo("Data size: %u bytes", boot_info->total_size - sizeof(struct orion_boot_info));
    kinfo("Structure count: %u", boot_info->info_count);
    kinfo("Kernel load time: %u ms", boot_info->kernel_load_time);
    kinfo("Header checksum: 0x%x", boot_info->header_checksum);
    kinfo("Data checksum: 0x%x", boot_info->data_checksum);

    // Print information about contained structures
    if (boot_info->total_size > sizeof(struct orion_boot_info) && boot_info->info_count > 0)
    {
        kinfo("--- Boot Info Structures ---");
        uint8_t *data = (uint8_t *)boot_info + sizeof(struct orion_boot_info);
        uint32_t offset = 0;
        uint32_t struct_num = 1;
        uint32_t data_size = boot_info->total_size - sizeof(struct orion_boot_info);

        while (offset < data_size)
        {
            struct orion_info_tag *header = (struct orion_info_tag *)(data + offset);

            const char *type_name;
            switch (header->type)
            {
            case ORION_INFO_MEMORY:
                type_name = "Memory Map";
                break;
            case ORION_INFO_BOOTLOADER:
                type_name = "Bootloader Info";
                break;
            case ORION_INFO_EFI:
                type_name = "EFI System Info";
                break;
            default:
                type_name = "Unknown";
                break;
            }

            kinfo("  %u. %s (type=%u, size=%u bytes)",
                  struct_num++, type_name, header->type, header->size);

            offset += header->size;
        }
    }

    kinfo("=====================================");
}
