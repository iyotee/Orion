/*
 * ORION OS - POWER Architecture Boot Configuration
 *
 * Boot parameters and device tree configuration for POWER architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_ARCH_POWER_BOOT_CONFIG_H
#define ORION_ARCH_POWER_BOOT_CONFIG_H

#include "config.h"

// ============================================================================
// BOOT PARAMETERS
// ============================================================================

#define POWER_BOOT_MAGIC 0x4F52494F4E000000ULL  // "ORION\0\0\0"
#define POWER_BOOT_VERSION 0x00010000             // Version 1.0

#define POWER_BOOT_FLAG_64BIT     0x00000001
#define POWER_BOOT_FLAG_BIG_ENDIAN 0x00000002
#define POWER_BOOT_FLAG_HAS_DTB   0x00000004
#define POWER_BOOT_FLAG_HAS_RAMDISK 0x00000008
#define POWER_BOOT_FLAG_HAS_CMDLINE 0x00000010
#define POWER_BOOT_FLAG_HAS_INITRD 0x00000020

// ============================================================================
// MEMORY LAYOUT
// ============================================================================

#define POWER_KERNEL_BASE     0x0000000001000000ULL
#define POWER_KERNEL_SIZE     0x01000000ULL        // 16MB
#define POWER_STACK_BASE      0x0000000002000000ULL
#define POWER_STACK_SIZE      0x00100000ULL        // 1MB
#define POWER_HEAP_BASE       0x0000000002100000ULL
#define POWER_HEAP_SIZE       0x00F00000ULL        // 15MB
#define POWER_DEVICE_BASE     0x4000000000000000ULL
#define POWER_DEVICE_SIZE     0x400000000ULL       // 1GB
#define POWER_HIGH_BASE       0x8000000000000000ULL
#define POWER_HIGH_SIZE       0x8000000000000000ULL // 1TB

// ============================================================================
// DEVICE TREE
// ============================================================================

#define POWER_DTB_MAGIC 0xD00DFEED
#define POWER_DTB_VERSION 17

#define POWER_DTB_COMPATIBLE_POWER8 "ibm,power8"
#define POWER_DTB_COMPATIBLE_POWER9 "ibm,power9"
#define POWER_DTB_COMPATIBLE_POWER10 "ibm,power10"

// ============================================================================
// INTERRUPT CONFIGURATION
// ============================================================================

#define POWER_IRQ_BASE        0x00000000
#define POWER_IRQ_COUNT       256
#define POWER_IRQ_MAX_PRIORITY 15

#define POWER_IRQ_TIMER       0
#define POWER_IRQ_UART        1
#define POWER_IRQ_GPIO        2
#define POWER_IRQ_I2C         3
#define POWER_IRQ_SPI         4
#define POWER_IRQ_USB         5
#define POWER_IRQ_ETHERNET    6
#define POWER_IRQ_SDMMC       7

// ============================================================================
// TIMER CONFIGURATION
// ============================================================================

#define POWER_TIMER_BASE_FREQ 1000000000ULL       // 1GHz
#define POWER_TIMER_MIN_FREQ  1000ULL             // 1kHz
#define POWER_TIMER_MAX_FREQ  1000000000ULL       // 1GHz

#define POWER_TIMER_DECREMENTER 22                // SPR 22
#define POWER_TIMER_TBU        268                // SPR 268
#define POWER_TIMER_TBL        269                // SPR 269

// ============================================================================
// CACHE CONFIGURATION
// ============================================================================

#define POWER_CACHE_L1I_SIZE  0x00020000          // 128KB
#define POWER_CACHE_L1D_SIZE  0x00020000          // 128KB
#define POWER_CACHE_L2_SIZE   0x00100000          // 1MB
#define POWER_CACHE_L3_SIZE   0x02000000          // 32MB

#define POWER_CACHE_L1_LINE   64                  // 64 bytes
#define POWER_CACHE_L2_LINE   128                 // 128 bytes
#define POWER_CACHE_L3_LINE   128                 // 128 bytes

// ============================================================================
// MMU CONFIGURATION
// ============================================================================

#define POWER_MMU_PAGE_SIZE_4K   0x1000
#define POWER_MMU_PAGE_SIZE_64K  0x10000
#define POWER_MMU_PAGE_SIZE_16M  0x1000000
#define POWER_MMU_PAGE_SIZE_1G   0x40000000

#define POWER_MMU_TLB_ENTRIES    1024
#define POWER_MMU_MAX_ASID       256

#define POWER_MMU_SDR1           25               // SPR 25
#define POWER_MMU_AMR            13               // SPR 13

// ============================================================================
// UART CONFIGURATION
// ============================================================================

#define POWER_UART_BASE       0x4000000000000000ULL
#define POWER_UART_SIZE       0x1000
#define POWER_UART_IRQ        POWER_IRQ_UART
#define POWER_UART_CLOCK      115200

// ============================================================================
// GPIO CONFIGURATION
// ============================================================================

#define POWER_GPIO_BASE       0x4000000000001000ULL
#define POWER_GPIO_SIZE       0x1000
#define POWER_GPIO_IRQ        POWER_IRQ_GPIO
#define POWER_GPIO_COUNT      64

// ============================================================================
// I2C CONFIGURATION
// ============================================================================

#define POWER_I2C_BASE        0x4000000000002000ULL
#define POWER_I2C_SIZE        0x1000
#define POWER_I2C_IRQ         POWER_IRQ_I2C
#define POWER_I2C_CLOCK       100000              // 100kHz

// ============================================================================
// SPI CONFIGURATION
// ============================================================================

#define POWER_SPI_BASE        0x4000000000003000ULL
#define POWER_SPI_SIZE        0x1000
#define POWER_SPI_IRQ         POWER_IRQ_SPI
#define POWER_SPI_CLOCK       1000000             // 1MHz

// ============================================================================
// USB CONFIGURATION
// ============================================================================

#define POWER_USB_BASE        0x4000000000004000ULL
#define POWER_USB_SIZE        0x10000
#define POWER_USB_IRQ         POWER_IRQ_USB
#define POWER_USB_VERSION     2                   // USB 2.0

// ============================================================================
// ETHERNET CONFIGURATION
// ============================================================================

#define POWER_ETHERNET_BASE   0x4000000000005000ULL
#define POWER_ETHERNET_SIZE   0x1000
#define POWER_ETHERNET_IRQ    POWER_IRQ_ETHERNET
#define POWER_ETHERNET_SPEED  1000000000          // 1Gbps

// ============================================================================
// SD/MMC CONFIGURATION
// ============================================================================

#define POWER_SDMMC_BASE      0x4000000000006000ULL
#define POWER_SDMMC_SIZE      0x1000
#define POWER_SDMMC_IRQ       POWER_IRQ_SDMMC
#define POWER_SDMMC_CLOCK     25000000            // 25MHz

// ============================================================================
// BOOT STRUCTURE
// ============================================================================

typedef struct {
    uint64_t magic;           // Boot magic number
    uint32_t version;         // Boot version
    uint32_t flags;           // Boot flags
    uint64_t kernel_base;     // Kernel base address
    uint64_t kernel_size;     // Kernel size
    uint64_t dtb_base;        // Device tree base address
    uint64_t dtb_size;        // Device tree size
    uint64_t ramdisk_base;    // Ramdisk base address
    uint64_t ramdisk_size;    // Ramdisk size
    uint64_t cmdline_base;    // Command line base address
    uint64_t cmdline_size;    // Command line size
    uint64_t initrd_base;     // Initrd base address
    uint64_t initrd_size;     // Initrd size
    uint64_t stack_base;      // Stack base address
    uint64_t stack_size;      // Stack size
    uint64_t heap_base;       // Heap base address
    uint64_t heap_size;       // Heap size
} power_boot_params_t;

// ============================================================================
// DEVICE TREE STRUCTURE
// ============================================================================

typedef struct {
    uint32_t magic;           // Device tree magic
    uint32_t totalsize;       // Total size
    uint32_t off_dt_struct;   // Offset to structure
    uint32_t off_dt_strings;  // Offset to strings
    uint32_t off_mem_rsvmap;  // Offset to memory reserve map
    uint32_t version;         // Version
    uint32_t last_comp_version; // Last compatible version
    uint32_t boot_cpuid_phys; // Boot CPU physical ID
    uint32_t size_dt_strings; // Size of strings section
    uint32_t size_dt_struct;  // Size of structure section
} power_dtb_header_t;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

int power_boot_init(power_boot_params_t *params);
int power_boot_parse_dtb(void *dtb_base);
int power_boot_setup_memory(void);
int power_boot_setup_devices(void);
int power_boot_setup_interrupts(void);
int power_boot_setup_mmu(void);
int power_boot_setup_timer(void);
int power_boot_setup_cache(void);
int power_boot_setup_uart(void);
int power_boot_setup_gpio(void);
int power_boot_setup_i2c(void);
int power_boot_setup_spi(void);
int power_boot_setup_usb(void);
int power_boot_setup_ethernet(void);
int power_boot_setup_sdmmc(void);

// ============================================================================
// BOOT UTILITIES
// ============================================================================

void power_boot_print(const char *str);
void power_boot_print_hex(uint64_t value);
void power_boot_print_dec(uint64_t value);
void power_boot_delay(uint64_t cycles);
uint64_t power_boot_get_time(void);

#endif // ORION_ARCH_POWER_BOOT_CONFIG_H
