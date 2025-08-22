/*
 * Orion Boot Protocol Specification v1.0
 * =======================================
 *
 * SHARED HEADER - Used by both bootloader and kernel
 *
 * A lightweight boot protocol for the Orion operating system kernel.
 * Designed for academic research and educational environments.
 *
 * Features:
 * - UEFI native support
 * - Minimal memory footprint
 * - Extensible information structures
 * - Built-in integrity validation
 *
 * Copyright (c) 2024 Orion OS Project
 * License: MIT
 */

#ifndef ORION_BOOT_PROTOCOL_H
#define ORION_BOOT_PROTOCOL_H

#ifdef __cplusplus
extern "C"
{
#endif

// Use kernel types if in kernel context, otherwise use standard types
#ifdef ORION_KERNEL_BUILD
#include <orion/types.h>
#else
#include <stdint.h>
#include <stddef.h>
#endif

    // ====================================
    // ORION BOOT PROTOCOL - CONSTANTS
    // ====================================

#define ORION_BOOT_MAGIC 0x4F52494F   // "ORIO" in ASCII
#define ORION_BOOT_VERSION 0x00010000 // Version 1.0.0
#define ORION_BOOT_SIGNATURE "ORION_KERNEL_v1.0"

// Information types passed to kernel
#define ORION_INFO_END 0x0000
#define ORION_INFO_MEMORY 0x0001
#define ORION_INFO_MODULES 0x0002
#define ORION_INFO_BOOTLOADER 0x0003
#define ORION_INFO_FRAMEBUFFER 0x0004
#define ORION_INFO_ACPI 0x0005
#define ORION_INFO_EFI 0x0006

    // ====================================
    // DATA STRUCTURES
    // ====================================

    /**
     * Generic information tag header
     */
    struct orion_info_tag
    {
        uint32_t type; // Information type (ORION_INFO_*)
        uint32_t size; // Size of this structure
    } __attribute__((packed));

    /**
     * Orion Boot Protocol kernel header
     *
     * This structure must be present in the kernel image at a known offset
     * for the bootloader to validate and load the kernel correctly.
     */
    struct orion_boot_header
    {
        uint32_t magic;       // ORION_BOOT_MAGIC
        uint32_t version;     // ORION_BOOT_VERSION
        char signature[20];   // "ORION_KERNEL_v1.0"
        uint64_t entry_point; // Kernel entry point address
        uint64_t stack_size;  // Required stack size in bytes
        uint32_t flags;       // Configuration flags
        uint32_t checksum;    // Header validation checksum
    } __attribute__((packed));

    /**
     * Main boot information structure passed to kernel
     */
    struct orion_boot_info
    {
        uint32_t magic;      // ORION_BOOT_MAGIC
        uint32_t version;    // ORION_BOOT_VERSION
        uint32_t total_size; // Total size of all information
        uint32_t info_count; // Number of info structures included

        // Security checksums
        uint32_t header_checksum; // Header checksum
        uint32_t data_checksum;   // Data checksum

        // Timestamps
        uint64_t boot_timestamp;   // Boot timestamp (UTC)
        uint32_t kernel_load_time; // Kernel load time in ms
        uint32_t reserved;         // Padding for alignment

        // Followed by information structures
    } __attribute__((packed));

    /**
     * Memory information structure
     */
    struct orion_memory_info
    {
        struct orion_info_tag header; // type = ORION_INFO_MEMORY
        uint64_t total_memory;        // Total physical memory in bytes
        uint64_t available_memory;    // Available memory for allocation
        uint32_t memory_map_entries;  // Number of memory map entries
        uint32_t reserved;            // Padding for alignment
        // Followed by memory map entries
    } __attribute__((packed));

    /**
     * Memory map entry
     */
    struct orion_memory_entry
    {
        uint64_t base_addr;  // Base address
        uint64_t length;     // Size in bytes
        uint32_t type;       // Memory type (RAM, RESERVED, etc.)
        uint32_t attributes; // EFI attributes
    } __attribute__((packed));

    /**
     * Bootloader information
     */
    struct orion_bootloader_info
    {
        struct orion_info_tag header; // type = ORION_INFO_BOOTLOADER
        char name[32];                // Bootloader name
        char version[16];             // Bootloader version
        uint32_t load_time;           // Boot time in ms
        uint32_t features;            // Supported features
    } __attribute__((packed));

    /**
     * EFI system information
     */
    struct orion_efi_info
    {
        struct orion_info_tag header; // type = ORION_INFO_EFI
        uint64_t system_table;        // EFI_SYSTEM_TABLE pointer
        uint64_t boot_services;       // EFI_BOOT_SERVICES pointer
        uint64_t runtime_services;    // EFI_RUNTIME_SERVICES pointer
        uint32_t firmware_revision;   // Firmware version
        uint32_t reserved;            // Padding
        char firmware_vendor[64];     // Firmware vendor string
    } __attribute__((packed));

    // ====================================
    // UTILITY FUNCTIONS
    // ====================================

    /**
     * Calculate Orion checksum
     */
    static inline uint32_t orion_checksum(const void *data, size_t size)
    {
        const uint8_t *bytes = (const uint8_t *)data;
        uint32_t checksum = 0;

        for (size_t i = 0; i < size; i++)
        {
            checksum += bytes[i];
            checksum = (checksum << 1) | (checksum >> 31); // Rotation
        }

        return ~checksum; // One's complement
    }

    /**
     * Validate Orion boot header
     */
    static inline int orion_validate_header(const struct orion_boot_header *header)
    {
        if (!header)
            return 0;

        // Check magic number
        if (header->magic != ORION_BOOT_MAGIC)
            return 0;

        // Check version compatibility
        uint32_t major = (header->version >> 16) & 0xFFFF;
        if (major != 1)
            return 0;

        return 1;
    }

#ifdef __cplusplus
}
#endif

#endif // ORION_BOOT_PROTOCOL_H
