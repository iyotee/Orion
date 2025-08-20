/*
 * Orion Boot Protocol - Kernel Interface
 *
 * This header defines the kernel-side structures and functions for handling
 * boot information passed by the Orion bootloader.
 *
 * Copyright (c) 2024 Orion OS Project
 * License: MIT
 */

#ifndef ORION_BOOT_H
#define ORION_BOOT_H

#include <orion/types.h>

// Include the shared boot protocol header
#define ORION_KERNEL_BUILD
#include "../../../include/orion-boot-protocol.h"

// Boot information management
/**
 * Initialize boot information processing
 * @param boot_info Pointer to boot information from bootloader
 * @return 0 on success, negative error code on failure
 */
int orion_boot_init(const struct orion_boot_info *boot_info);

/**
 * Get memory map information
 * @param memory_info Output pointer for memory information
 * @return 0 on success, negative error code on failure
 */
int orion_boot_get_memory_info(struct orion_memory_info **memory_info);

/**
 * Get bootloader information
 * @param bootloader_info Output pointer for bootloader information
 * @return 0 on success, negative error code on failure
 */
int orion_boot_get_bootloader_info(struct orion_bootloader_info **bootloader_info);

/**
 * Get EFI system information
 * @param efi_info Output pointer for EFI information
 * @return 0 on success, negative error code on failure
 */
int orion_boot_get_efi_info(struct orion_efi_info **efi_info);

/**
 * Validate boot information integrity
 * @param boot_info Pointer to boot information to validate
 * @return 1 if valid, 0 if invalid
 */
int orion_boot_validate(const struct orion_boot_info *boot_info);

/**
 * Print boot information for debugging
 * @param boot_info Pointer to boot information
 */
void orion_boot_debug_print(const struct orion_boot_info *boot_info);

// Boot flags for kernel configuration
#define ORION_BOOT_FLAG_DEBUG 0x00000001    // Enable debug output
#define ORION_BOOT_FLAG_SERIAL 0x00000002   // Enable serial console
#define ORION_BOOT_FLAG_GRAPHICS 0x00000004 // Enable graphics mode
#define ORION_BOOT_FLAG_SMP 0x00000008      // Enable SMP support
#define ORION_BOOT_FLAG_ACPI 0x00000010     // Enable ACPI support

// Global boot information (set during kernel initialization)
extern struct orion_boot_info *g_boot_info;
extern struct orion_memory_info *g_memory_info;
extern struct orion_efi_info *g_efi_info;

#endif // ORION_BOOT_H
