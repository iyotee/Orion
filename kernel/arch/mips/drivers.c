/*
 * ORION OS - MIPS Device Drivers
 *
 * Device driver management for MIPS architecture
 * Handles driver registration, device discovery, and hardware abstraction
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
// DRIVER CONTEXT MANAGEMENT
// ============================================================================

static mips_driver_context_t mips_driver_context = {0};

void mips_drivers_init(void)
{
    memset(&mips_driver_context, 0, sizeof(mips_driver_context));

    // Initialize driver context
    mips_driver_context.drivers_initialized = false;
    mips_driver_context.driver_count = 0;
    mips_driver_context.device_count = 0;
    mips_driver_context.auto_discovery_enabled = true;
    mips_driver_context.plug_and_play_enabled = true;

    // Initialize driver table
    for (int i = 0; i < MIPS_MAX_DRIVERS; i++)
    {
        mips_driver_context.driver_table[i] = NULL;
    }

    // Initialize device table
    for (int i = 0; i < MIPS_MAX_DEVICES; i++)
    {
        mips_driver_context.device_table[i] = NULL;
    }

    printf("MIPS: Device driver management initialized\n");
}

mips_driver_context_t *mips_drivers_get_context(void)
{
    return &mips_driver_context;
}

// ============================================================================
// DRIVER REGISTRATION
// ============================================================================

int mips_drivers_register_driver(const mips_driver_t *driver)
{
    if (!driver)
    {
        printf("MIPS: Invalid driver pointer\n");
        return -1;
    }

    if (mips_driver_context.driver_count >= MIPS_MAX_DRIVERS)
    {
        printf("MIPS: Maximum driver count reached\n");
        return -1;
    }

    // Find free driver slot
    int driver_id = -1;
    for (int i = 0; i < MIPS_MAX_DRIVERS; i++)
    {
        if (!mips_driver_context.driver_table[i])
        {
            driver_id = i;
            break;
        }
    }

    if (driver_id == -1)
    {
        printf("MIPS: No free driver slots\n");
        return -1;
    }

    // Allocate and copy driver
    mips_driver_t *new_driver = malloc(sizeof(mips_driver_t));
    if (!new_driver)
    {
        printf("MIPS: Failed to allocate driver memory\n");
        return -1;
    }

    memcpy(new_driver, driver, sizeof(mips_driver_t));
    mips_driver_context.driver_table[driver_id] = new_driver;
    mips_driver_context.driver_count++;

    printf("MIPS: Driver '%s' registered with ID %d\n", driver->name, driver_id);
    return driver_id;
}

void mips_drivers_unregister_driver(int driver_id)
{
    if (driver_id < 0 || driver_id >= MIPS_MAX_DRIVERS)
    {
        printf("MIPS: Invalid driver ID %d\n", driver_id);
        return;
    }

    mips_driver_t *driver = mips_driver_context.driver_table[driver_id];
    if (!driver)
    {
        printf("MIPS: Driver %d not registered\n", driver_id);
        return;
    }

    // Unload driver if loaded
    if (driver->loaded)
    {
        mips_drivers_unload_driver(driver_id);
    }

    // Free driver memory
    free(driver);
    mips_driver_context.driver_table[driver_id] = NULL;
    mips_driver_context.driver_count--;

    printf("MIPS: Driver %d unregistered\n", driver_id);
}

mips_driver_t *mips_drivers_get_driver(int driver_id)
{
    if (driver_id < 0 || driver_id >= MIPS_MAX_DRIVERS)
    {
        return NULL;
    }

    return mips_driver_context.driver_table[driver_id];
}

// ============================================================================
// DRIVER LOADING AND UNLOADING
// ============================================================================

int mips_drivers_load_driver(int driver_id)
{
    mips_driver_t *driver = mips_drivers_get_driver(driver_id);
    if (!driver)
    {
        printf("MIPS: Driver %d not found\n", driver_id);
        return -1;
    }

    if (driver->loaded)
    {
        printf("MIPS: Driver %d already loaded\n", driver_id);
        return 0;
    }

    // Call driver init function
    if (driver->init)
    {
        int result = driver->init();
        if (result != 0)
        {
            printf("MIPS: Driver %d initialization failed: %d\n", driver_id, result);
            return result;
        }
    }

    driver->loaded = true;
    printf("MIPS: Driver %d loaded successfully\n", driver_id);
    return 0;
}

void mips_drivers_unload_driver(int driver_id)
{
    mips_driver_t *driver = mips_drivers_get_driver(driver_id);
    if (!driver)
    {
        printf("MIPS: Driver %d not found\n", driver_id);
        return;
    }

    if (!driver->loaded)
    {
        printf("MIPS: Driver %d not loaded\n", driver_id);
        return;
    }

    // Call driver cleanup function
    if (driver->cleanup)
    {
        driver->cleanup();
    }

    driver->loaded = false;
    printf("MIPS: Driver %d unloaded\n", driver_id);
}

// ============================================================================
// DEVICE MANAGEMENT
// ============================================================================

int mips_drivers_register_device(const mips_device_t *device)
{
    if (!device)
    {
        printf("MIPS: Invalid device pointer\n");
        return -1;
    }

    if (mips_driver_context.device_count >= MIPS_MAX_DEVICES)
    {
        printf("MIPS: Maximum device count reached\n");
        return -1;
    }

    // Find free device slot
    int device_id = -1;
    for (int i = 0; i < MIPS_MAX_DEVICES; i++)
    {
        if (!mips_driver_context.device_table[i])
        {
            device_id = i;
            break;
        }
    }

    if (device_id == -1)
    {
        printf("MIPS: No free device slots\n");
        return -1;
    }

    // Allocate and copy device
    mips_device_t *new_device = malloc(sizeof(mips_device_t));
    if (!new_device)
    {
        printf("MIPS: Failed to allocate device memory\n");
        return -1;
    }

    memcpy(new_device, device, sizeof(mips_device_t));
    mips_driver_context.device_table[device_id] = new_device;
    mips_driver_context.device_count++;

    printf("MIPS: Device '%s' registered with ID %d\n", device->name, device_id);
    return device_id;
}

void mips_drivers_unregister_device(int device_id)
{
    if (device_id < 0 || device_id >= MIPS_MAX_DEVICES)
    {
        printf("MIPS: Invalid device ID %d\n", device_id);
        return;
    }

    mips_device_t *device = mips_driver_context.device_table[device_id];
    if (!device)
    {
        printf("MIPS: Device %d not registered\n", device_id);
        return;
    }

    // Free device memory
    free(device);
    mips_driver_context.device_table[device_id] = NULL;
    mips_driver_context.device_count--;

    printf("MIPS: Device %d unregistered\n", device_id);
}

mips_device_t *mips_drivers_get_device(int device_id)
{
    if (device_id < 0 || device_id >= MIPS_MAX_DEVICES)
    {
        return NULL;
    }

    return mips_driver_context.device_table[device_id];
}

// ============================================================================
// DEVICE DISCOVERY
// ============================================================================

void mips_drivers_discover_devices(void)
{
    if (!mips_driver_context.auto_discovery_enabled)
    {
        printf("MIPS: Auto-discovery disabled\n");
        return;
    }

    printf("MIPS: Starting device discovery\n");

    // Discover UART devices
    mips_drivers_discover_uart_devices();

    // Discover SPI devices
    mips_drivers_discover_spi_devices();

    // Discover I2C devices
    mips_drivers_discover_i2c_devices();

    // Discover GPIO devices
    mips_drivers_discover_gpio_devices();

    // Discover other devices
    mips_drivers_discover_other_devices();

    printf("MIPS: Device discovery completed\n");
}

void mips_drivers_discover_uart_devices(void)
{
    printf("MIPS: Discovering UART devices\n");

    // Common UART base addresses for MIPS systems
    mips_addr_t uart_addresses[] = {
        0x1F000900, // Standard UART0
        0x1F000A00, // Standard UART1
        0x1F000B00, // Standard UART2
        0x1F000C00  // Standard UART3
    };

    for (int i = 0; i < sizeof(uart_addresses) / sizeof(uart_addresses[0]); i++)
    {
        if (mips_drivers_probe_uart_device(uart_addresses[i]))
        {
            mips_device_t uart_device = {
                .name = "UART",
                .type = MIPS_DEVICE_TYPE_UART,
                .base_address = uart_addresses[i],
                .driver_id = -1,
                .enabled = true};

            int device_id = mips_drivers_register_device(&uart_device);
            if (device_id >= 0)
            {
                printf("MIPS: UART device discovered at 0x%llx (ID: %d)\n", uart_addresses[i], device_id);
            }
        }
    }
}

void mips_drivers_discover_spi_devices(void)
{
    printf("MIPS: Discovering SPI devices\n");

    // Common SPI base addresses for MIPS systems
    mips_addr_t spi_addresses[] = {
        0x1F000D00, // Standard SPI0
        0x1F000E00, // Standard SPI1
        0x1F000F00  // Standard SPI2
    };

    for (int i = 0; i < sizeof(spi_addresses) / sizeof(spi_addresses[0]); i++)
    {
        if (mips_drivers_probe_spi_device(spi_addresses[i]))
        {
            mips_device_t spi_device = {
                .name = "SPI",
                .type = MIPS_DEVICE_TYPE_SPI,
                .base_address = spi_addresses[i],
                .driver_id = -1,
                .enabled = true};

            int device_id = mips_drivers_register_device(&spi_device);
            if (device_id >= 0)
            {
                printf("MIPS: SPI device discovered at 0x%llx (ID: %d)\n", spi_addresses[i], device_id);
            }
        }
    }
}

void mips_drivers_discover_i2c_devices(void)
{
    printf("MIPS: Discovering I2C devices\n");

    // Common I2C base addresses for MIPS systems
    mips_addr_t i2c_addresses[] = {
        0x1F001000, // Standard I2C0
        0x1F001100, // Standard I2C1
        0x1F001200  // Standard I2C2
    };

    for (int i = 0; i < sizeof(i2c_addresses) / sizeof(i2c_addresses[0]); i++)
    {
        if (mips_drivers_probe_i2c_device(i2c_addresses[i]))
        {
            mips_device_t i2c_device = {
                .name = "I2C",
                .type = MIPS_DEVICE_TYPE_I2C,
                .base_address = i2c_addresses[i],
                .driver_id = -1,
                .enabled = true};

            int device_id = mips_drivers_register_device(&i2c_device);
            if (device_id >= 0)
            {
                printf("MIPS: I2C device discovered at 0x%llx (ID: %d)\n", i2c_addresses[i], device_id);
            }
        }
    }
}

void mips_drivers_discover_gpio_devices(void)
{
    printf("MIPS: Discovering GPIO devices\n");

    // Common GPIO base addresses for MIPS systems
    mips_addr_t gpio_addresses[] = {
        0x1F001300, // Standard GPIO0
        0x1F001400, // Standard GPIO1
        0x1F001500  // Standard GPIO2
    };

    for (int i = 0; i < sizeof(gpio_addresses) / sizeof(gpio_addresses[0]); i++)
    {
        if (mips_drivers_probe_gpio_device(gpio_addresses[i]))
        {
            mips_device_t gpio_device = {
                .name = "GPIO",
                .type = MIPS_DEVICE_TYPE_GPIO,
                .base_address = gpio_addresses[i],
                .driver_id = -1,
                .enabled = true};

            int device_id = mips_drivers_register_device(&gpio_device);
            if (device_id >= 0)
            {
                printf("MIPS: GPIO device discovered at 0x%llx (ID: %d)\n", gpio_addresses[i], device_id);
            }
        }
    }
}

void mips_drivers_discover_other_devices(void)
{
    printf("MIPS: Discovering other devices\n");

    // Discover additional device types as needed
    // This can include Ethernet, USB, PCI, etc.
}

// ============================================================================
// DEVICE PROBING
// ============================================================================

bool mips_drivers_probe_uart_device(mips_addr_t base_address)
{
    // Simple UART probing - check if registers are accessible
    volatile uint8_t *uart_base = (volatile uint8_t *)base_address;

    // Try to read UART status register
    uint8_t status = uart_base[MIPS_UART_LSR];

    // Check if the value is reasonable (not all 0xFF or 0x00)
    if (status != 0xFF && status != 0x00)
    {
        return true;
    }

    return false;
}

bool mips_drivers_probe_spi_device(mips_addr_t base_address)
{
    // Simple SPI probing - check if registers are accessible
    volatile uint32_t *spi_base = (volatile uint32_t *)base_address;

    // Try to read SPI status register
    uint32_t status = spi_base[MIPS_SPI_STATUS];

    // Check if the value is reasonable
    if (status != 0xFFFFFFFF && status != 0x00000000)
    {
        return true;
    }

    return false;
}

bool mips_drivers_probe_i2c_device(mips_addr_t base_address)
{
    // Simple I2C probing - check if registers are accessible
    volatile uint32_t *i2c_base = (volatile uint32_t *)base_address;

    // Try to read I2C status register
    uint32_t status = i2c_base[MIPS_I2C_STATUS];

    // Check if the value is reasonable
    if (status != 0xFFFFFFFF && status != 0x00000000)
    {
        return true;
    }

    return false;
}

bool mips_drivers_probe_gpio_device(mips_addr_t base_address)
{
    // Simple GPIO probing - check if registers are accessible
    volatile uint32_t *gpio_base = (volatile uint32_t *)base_address;

    // Try to read GPIO input register
    uint32_t input = gpio_base[MIPS_GPIO_INPUT];

    // Check if the value is reasonable
    if (input != 0xFFFFFFFF && input != 0x00000000)
    {
        return true;
    }

    return false;
}

// ============================================================================
// DRIVER UTILITIES
// ============================================================================

void mips_drivers_print_status(void)
{
    printf("MIPS: Driver Status:\n");
    printf("  Drivers Initialized: %s\n", mips_driver_context.drivers_initialized ? "yes" : "no");
    printf("  Driver Count: %u\n", mips_driver_context.driver_count);
    printf("  Device Count: %u\n", mips_driver_context.device_count);
    printf("  Auto-Discovery: %s\n", mips_driver_context.auto_discovery_enabled ? "enabled" : "disabled");
    printf("  Plug-and-Play: %s\n", mips_driver_context.plug_and_play_enabled ? "enabled" : "disabled");

    printf("  Registered Drivers:\n");
    for (int i = 0; i < MIPS_MAX_DRIVERS; i++)
    {
        mips_driver_t *driver = mips_driver_context.driver_table[i];
        if (driver)
        {
            printf("    ID %d: %s (loaded: %s)\n", i, driver->name, driver->loaded ? "yes" : "no");
        }
    }

    printf("  Registered Devices:\n");
    for (int i = 0; i < MIPS_MAX_DEVICES; i++)
    {
        mips_device_t *device = mips_driver_context.device_table[i];
        if (device)
        {
            printf("    ID %d: %s at 0x%llx (type: %u, enabled: %s)\n",
                   i, device->name, device->base_address, device->type, device->enabled ? "yes" : "no");
        }
    }
}

void mips_drivers_enable_auto_discovery(void)
{
    mips_driver_context.auto_discovery_enabled = true;
    printf("MIPS: Auto-discovery enabled\n");
}

void mips_drivers_disable_auto_discovery(void)
{
    mips_driver_context.auto_discovery_enabled = false;
    printf("MIPS: Auto-discovery disabled\n");
}

void mips_drivers_enable_plug_and_play(void)
{
    mips_driver_context.plug_and_play_enabled = true;
    printf("MIPS: Plug-and-play enabled\n");
}

void mips_drivers_disable_plug_and_play(void)
{
    mips_driver_context.plug_and_play_enabled = false;
    printf("MIPS: Plug-and-play disabled\n");
}

void mips_drivers_cleanup(void)
{
    printf("MIPS: Cleaning up drivers\n");

    // Unload all drivers
    for (int i = 0; i < MIPS_MAX_DRIVERS; i++)
    {
        if (mips_driver_context.driver_table[i])
        {
            mips_drivers_unload_driver(i);
        }
    }

    // Unregister all devices
    for (int i = 0; i < MIPS_MAX_DEVICES; i++)
    {
        if (mips_driver_context.device_table[i])
        {
            mips_drivers_unregister_device(i);
        }
    }

    mips_driver_context.drivers_initialized = false;
    mips_driver_context.driver_count = 0;
    mips_driver_context.device_count = 0;

    printf("MIPS: Driver cleanup completed\n");
}
