/*
 * ORION OS - MIPS Device Management
 *
 * Device management for MIPS architecture
 * Handles UART, SPI, I2C, GPIO, and other peripherals
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
// DEVICE CONTEXT MANAGEMENT
// ============================================================================

static mips_device_context_t mips_device_context = {0};

void mips_devices_init(void)
{
    memset(&mips_device_context, 0, sizeof(mips_device_context));

    // Set default device settings
    mips_device_context.devices_initialized = false;
    mips_device_context.active_devices = 0;
    mips_device_context.device_count = 0;

    printf("MIPS: Device management initialized\n");
}

mips_device_context_t *mips_devices_get_context(void)
{
    return &mips_device_context;
}

// ============================================================================
// UART DEVICE
// ============================================================================

void mips_uart_init(mips_addr_t base_address, uint32_t baud_rate)
{
    if (mips_device_context.devices_initialized)
    {
        printf("MIPS: UART already initialized\n");
        return;
    }

    // Initialize UART device
    mips_device_context.uart.base_address = base_address;
    mips_device_context.uart.baud_rate = baud_rate;
    mips_device_context.uart.enabled = true;
    mips_device_context.uart.interrupt_enabled = false;

    // Set baud rate divisor
    uint32_t divisor = MIPS_UART_CLOCK_FREQUENCY / (16 * baud_rate);
    mips_uart_write_register(MIPS_UART_DL, divisor & 0xFF);
    mips_uart_write_register(MIPS_UART_DH, (divisor >> 8) & 0xFF);

    // Enable UART with 8N1 configuration
    uint8_t lcr = MIPS_UART_LCR_8BIT | MIPS_UART_LCR_1STOP | MIPS_UART_LCR_NOPARITY;
    mips_uart_write_register(MIPS_UART_LCR, lcr);

    // Enable FIFO
    mips_uart_write_register(MIPS_UART_FCR, MIPS_UART_FCR_ENABLE | MIPS_UART_FCR_CLEAR);

    // Enable UART
    mips_uart_write_register(MIPS_UART_MCR, MIPS_UART_MCR_DTR | MIPS_UART_MCR_RTS);

    mips_device_context.active_devices |= MIPS_DEVICE_UART;
    mips_device_context.device_count++;

    printf("MIPS: UART initialized at 0x%llx, baud rate %u\n", base_address, baud_rate);
}

void mips_uart_deinit(void)
{
    if (!(mips_device_context.active_devices & MIPS_DEVICE_UART))
    {
        return;
    }

    // Disable UART
    mips_uart_write_register(MIPS_UART_MCR, 0);

    mips_device_context.uart.enabled = false;
    mips_device_context.active_devices &= ~MIPS_DEVICE_UART;
    mips_device_context.device_count--;

    printf("MIPS: UART deinitialized\n");
}

bool mips_uart_is_initialized(void)
{
    return (mips_device_context.active_devices & MIPS_DEVICE_UART) != 0;
}

uint8_t mips_uart_read_register(uint8_t reg)
{
    if (!mips_uart_is_initialized())
    {
        return 0;
    }

    mips_addr_t addr = mips_device_context.uart.base_address + reg;
    return *(volatile uint8_t *)addr;
}

void mips_uart_write_register(uint8_t reg, uint8_t value)
{
    if (!mips_uart_is_initialized())
    {
        return;
    }

    mips_addr_t addr = mips_device_context.uart.base_address + reg;
    *(volatile uint8_t *)addr = value;
}

bool mips_uart_is_data_ready(void)
{
    if (!mips_uart_is_initialized())
    {
        return false;
    }

    uint8_t lsr = mips_uart_read_register(MIPS_UART_LSR);
    return (lsr & MIPS_UART_LSR_DR) != 0;
}

bool mips_uart_is_transmit_empty(void)
{
    if (!mips_uart_is_initialized())
    {
        return false;
    }

    uint8_t lsr = mips_uart_read_register(MIPS_UART_LSR);
    return (lsr & MIPS_UART_LSR_THRE) != 0;
}

uint8_t mips_uart_read_byte(void)
{
    if (!mips_uart_is_initialized())
    {
        return 0;
    }

    // Wait for data to be ready
    while (!mips_uart_is_data_ready())
    {
        // Wait
    }

    return mips_uart_read_register(MIPS_UART_RBR);
}

void mips_uart_write_byte(uint8_t byte)
{
    if (!mips_uart_is_initialized())
    {
        return;
    }

    // Wait for transmit buffer to be empty
    while (!mips_uart_is_transmit_empty())
    {
        // Wait
    }

    mips_uart_write_register(MIPS_UART_THR, byte);
}

void mips_uart_write_string(const char *string)
{
    if (!mips_uart_is_initialized())
    {
        return;
    }

    while (*string)
    {
        mips_uart_write_byte(*string++);
    }
}

void mips_uart_enable_interrupts(void)
{
    if (!mips_uart_is_initialized())
    {
        return;
    }

    // Enable UART interrupts
    mips_uart_write_register(MIPS_UART_IER, MIPS_UART_IER_RDA | MIPS_UART_IER_THRE);

    mips_device_context.uart.interrupt_enabled = true;

    printf("MIPS: UART interrupts enabled\n");
}

void mips_uart_disable_interrupts(void)
{
    if (!mips_uart_is_initialized())
    {
        return;
    }

    // Disable UART interrupts
    mips_uart_write_register(MIPS_UART_IER, 0);

    mips_device_context.uart.interrupt_enabled = false;

    printf("MIPS: UART interrupts disabled\n");
}

// ============================================================================
// SPI DEVICE
// ============================================================================

void mips_spi_init(mips_addr_t base_address, uint32_t clock_frequency)
{
    if (mips_device_context.devices_initialized)
    {
        printf("MIPS: SPI already initialized\n");
        return;
    }

    // Initialize SPI device
    mips_device_context.spi.base_address = base_address;
    mips_device_context.spi.clock_frequency = clock_frequency;
    mips_device_context.spi.enabled = true;
    mips_device_context.spi.interrupt_enabled = false;

    // Set clock frequency
    uint32_t divisor = MIPS_SPI_CLOCK_FREQUENCY / clock_frequency;
    mips_spi_write_register(MIPS_SPI_CLKDIV, divisor);

    // Configure SPI mode (0,0)
    uint32_t ctrl = MIPS_SPI_CTRL_ENABLE | MIPS_SPI_CTRL_MSB_FIRST;
    mips_spi_write_register(MIPS_SPI_CTRL, ctrl);

    mips_device_context.active_devices |= MIPS_DEVICE_SPI;
    mips_device_context.device_count++;

    printf("MIPS: SPI initialized at 0x%llx, clock frequency %u Hz\n", base_address, clock_frequency);
}

void mips_spi_deinit(void)
{
    if (!(mips_device_context.active_devices & MIPS_DEVICE_SPI))
    {
        return;
    }

    // Disable SPI
    mips_spi_write_register(MIPS_SPI_CTRL, 0);

    mips_device_context.spi.enabled = false;
    mips_device_context.active_devices &= ~MIPS_DEVICE_SPI;
    mips_device_context.device_count--;

    printf("MIPS: SPI deinitialized\n");
}

bool mips_spi_is_initialized(void)
{
    return (mips_device_context.active_devices & MIPS_DEVICE_SPI) != 0;
}

uint32_t mips_spi_read_register(uint8_t reg)
{
    if (!mips_spi_is_initialized())
    {
        return 0;
    }

    mips_addr_t addr = mips_device_context.spi.base_address + reg;
    return *(volatile uint32_t *)addr;
}

void mips_spi_write_register(uint8_t reg, uint32_t value)
{
    if (!mips_spi_is_initialized())
    {
        return;
    }

    mips_addr_t addr = mips_device_context.spi.base_address + reg;
    *(volatile uint32_t *)addr = value;
}

bool mips_spi_is_busy(void)
{
    if (!mips_spi_is_initialized())
    {
        return false;
    }

    uint32_t status = mips_spi_read_register(MIPS_SPI_STATUS);
    return (status & MIPS_SPI_STATUS_BUSY) != 0;
}

uint8_t mips_spi_transfer_byte(uint8_t byte)
{
    if (!mips_spi_is_initialized())
    {
        return 0;
    }

    // Wait for SPI to be ready
    while (mips_spi_is_busy())
    {
        // Wait
    }

    // Write data
    mips_spi_write_register(MIPS_SPI_DATA, byte);

    // Wait for transfer to complete
    while (mips_spi_is_busy())
    {
        // Wait
    }

    // Read received data
    return (uint8_t)mips_spi_read_register(MIPS_SPI_DATA);
}

void mips_spi_transfer_buffer(const uint8_t *tx_buffer, uint8_t *rx_buffer, uint32_t length)
{
    if (!mips_spi_is_initialized())
    {
        return;
    }

    for (uint32_t i = 0; i < length; i++)
    {
        uint8_t tx_byte = tx_buffer ? tx_buffer[i] : 0;
        uint8_t rx_byte = mips_spi_transfer_byte(tx_byte);

        if (rx_buffer)
        {
            rx_buffer[i] = rx_byte;
        }
    }
}

void mips_spi_enable_interrupts(void)
{
    if (!mips_spi_is_initialized())
    {
        return;
    }

    // Enable SPI interrupts
    mips_spi_write_register(MIPS_SPI_INTEN, MIPS_SPI_INTEN_TX | MIPS_SPI_INTEN_RX);

    mips_device_context.spi.interrupt_enabled = true;

    printf("MIPS: SPI interrupts enabled\n");
}

void mips_spi_disable_interrupts(void)
{
    if (!mips_spi_is_initialized())
    {
        return;
    }

    // Disable SPI interrupts
    mips_spi_write_register(MIPS_SPI_INTEN, 0);

    mips_device_context.spi.interrupt_enabled = false;

    printf("MIPS: SPI interrupts disabled\n");
}

// ============================================================================
// I2C DEVICE
// ============================================================================

void mips_i2c_init(mips_addr_t base_address, uint32_t clock_frequency)
{
    if (mips_device_context.devices_initialized)
    {
        printf("MIPS: I2C already initialized\n");
        return;
    }

    // Initialize I2C device
    mips_device_context.i2c.base_address = base_address;
    mips_device_context.i2c.clock_frequency = clock_frequency;
    mips_device_context.i2c.enabled = true;
    mips_device_context.i2c.interrupt_enabled = false;

    // Set clock frequency
    uint32_t divisor = MIPS_I2C_CLOCK_FREQUENCY / clock_frequency;
    mips_i2c_write_register(MIPS_I2C_CLKDIV, divisor);

    // Enable I2C
    mips_i2c_write_register(MIPS_I2C_CTRL, MIPS_I2C_CTRL_ENABLE);

    mips_device_context.active_devices |= MIPS_DEVICE_I2C;
    mips_device_context.device_count++;

    printf("MIPS: I2C initialized at 0x%llx, clock frequency %u Hz\n", base_address, clock_frequency);
}

void mips_i2c_deinit(void)
{
    if (!(mips_device_context.active_devices & MIPS_DEVICE_I2C))
    {
        return;
    }

    // Disable I2C
    mips_i2c_write_register(MIPS_I2C_CTRL, 0);

    mips_device_context.i2c.enabled = false;
    mips_device_context.active_devices &= ~MIPS_DEVICE_I2C;
    mips_device_context.device_count--;

    printf("MIPS: I2C deinitialized\n");
}

bool mips_i2c_is_initialized(void)
{
    return (mips_device_context.active_devices & MIPS_DEVICE_I2C) != 0;
}

uint32_t mips_i2c_read_register(uint8_t reg)
{
    if (!mips_i2c_is_initialized())
    {
        return 0;
    }

    mips_addr_t addr = mips_device_context.i2c.base_address + reg;
    return *(volatile uint32_t *)addr;
}

void mips_i2c_write_register(uint8_t reg, uint32_t value)
{
    if (!mips_i2c_is_initialized())
    {
        return;
    }

    mips_addr_t addr = mips_device_context.i2c.base_address + reg;
    *(volatile uint32_t *)addr = value;
}

bool mips_i2c_is_busy(void)
{
    if (!mips_i2c_is_initialized())
    {
        return false;
    }

    uint32_t status = mips_i2c_read_register(MIPS_I2C_STATUS);
    return (status & MIPS_I2C_STATUS_BUSY) != 0;
}

int mips_i2c_write(uint8_t device_address, const uint8_t *data, uint32_t length)
{
    if (!mips_i2c_is_initialized())
    {
        return -1;
    }

    // Wait for I2C to be ready
    while (mips_i2c_is_busy())
    {
        // Wait
    }

    // Set device address
    mips_i2c_write_register(MIPS_I2C_ADDR, device_address);

    // Set data length
    mips_i2c_write_register(MIPS_I2C_LEN, length);

    // Write data
    for (uint32_t i = 0; i < length; i++)
    {
        mips_i2c_write_register(MIPS_I2C_DATA, data[i]);
    }

    // Start write transaction
    mips_i2c_write_register(MIPS_I2C_CTRL, MIPS_I2C_CTRL_ENABLE | MIPS_I2C_CTRL_START | MIPS_I2C_CTRL_WRITE);

    // Wait for completion
    while (mips_i2c_is_busy())
    {
        // Wait
    }

    // Check for errors
    uint32_t status = mips_i2c_read_register(MIPS_I2C_STATUS);
    if (status & MIPS_I2C_STATUS_ERROR)
    {
        return -1;
    }

    return 0;
}

int mips_i2c_read(uint8_t device_address, uint8_t *data, uint32_t length)
{
    if (!mips_i2c_is_initialized())
    {
        return -1;
    }

    // Wait for I2C to be ready
    while (mips_i2c_is_busy())
    {
        // Wait
    }

    // Set device address
    mips_i2c_write_register(MIPS_I2C_ADDR, device_address);

    // Set data length
    mips_i2c_write_register(MIPS_I2C_LEN, length);

    // Start read transaction
    mips_i2c_write_register(MIPS_I2C_CTRL, MIPS_I2C_CTRL_ENABLE | MIPS_I2C_CTRL_START | MIPS_I2C_CTRL_READ);

    // Wait for completion
    while (mips_i2c_is_busy())
    {
        // Wait
    }

    // Read data
    for (uint32_t i = 0; i < length; i++)
    {
        data[i] = (uint8_t)mips_i2c_read_register(MIPS_I2C_DATA);
    }

    // Check for errors
    uint32_t status = mips_i2c_read_register(MIPS_I2C_STATUS);
    if (status & MIPS_I2C_STATUS_ERROR)
    {
        return -1;
    }

    return 0;
}

void mips_i2c_enable_interrupts(void)
{
    if (!mips_i2c_is_initialized())
    {
        return;
    }

    // Enable I2C interrupts
    mips_i2c_write_register(MIPS_I2C_INTEN, MIPS_I2C_INTEN_COMPLETE | MIPS_I2C_INTEN_ERROR);

    mips_device_context.i2c.interrupt_enabled = true;

    printf("MIPS: I2C interrupts enabled\n");
}

void mips_i2c_disable_interrupts(void)
{
    if (!mips_i2c_is_initialized())
    {
        return;
    }

    // Disable I2C interrupts
    mips_i2c_write_register(MIPS_I2C_INTEN, 0);

    mips_device_context.i2c.interrupt_enabled = false;

    printf("MIPS: I2C interrupts disabled\n");
}

// ============================================================================
// GPIO DEVICE
// ============================================================================

void mips_gpio_init(mips_addr_t base_address)
{
    if (mips_device_context.devices_initialized)
    {
        printf("MIPS: GPIO already initialized\n");
        return;
    }

    // Initialize GPIO device
    mips_device_context.gpio.base_address = base_address;
    mips_device_context.gpio.enabled = true;
    mips_device_context.gpio.interrupt_enabled = false;

    // Clear all GPIO pins
    mips_gpio_write_register(MIPS_GPIO_OUTPUT, 0);
    mips_gpio_write_register(MIPS_GPIO_DIRECTION, 0);
    mips_gpio_write_register(MIPS_GPIO_INTERRUPT_ENABLE, 0);

    mips_device_context.active_devices |= MIPS_DEVICE_GPIO;
    mips_device_context.device_count++;

    printf("MIPS: GPIO initialized at 0x%llx\n", base_address);
}

void mips_gpio_deinit(void)
{
    if (!(mips_device_context.active_devices & MIPS_DEVICE_GPIO))
    {
        return;
    }

    // Disable all GPIO interrupts
    mips_gpio_write_register(MIPS_GPIO_INTERRUPT_ENABLE, 0);

    mips_device_context.gpio.enabled = false;
    mips_device_context.active_devices &= ~MIPS_DEVICE_GPIO;
    mips_device_context.device_count--;

    printf("MIPS: GPIO deinitialized\n");
}

bool mips_gpio_is_initialized(void)
{
    return (mips_device_context.active_devices & MIPS_DEVICE_GPIO) != 0;
}

uint32_t mips_gpio_read_register(uint8_t reg)
{
    if (!mips_gpio_is_initialized())
    {
        return 0;
    }

    mips_addr_t addr = mips_device_context.gpio.base_address + reg;
    return *(volatile uint32_t *)addr;
}

void mips_gpio_write_register(uint8_t reg, uint32_t value)
{
    if (!mips_gpio_is_initialized())
    {
        return;
    }

    mips_addr_t addr = mips_device_context.gpio.base_address + reg;
    *(volatile uint32_t *)addr = value;
}

void mips_gpio_set_pin_direction(uint32_t pin, bool is_output)
{
    if (!mips_gpio_is_initialized())
    {
        return;
    }

    uint32_t direction = mips_gpio_read_register(MIPS_GPIO_DIRECTION);

    if (is_output)
    {
        direction |= (1 << pin);
    }
    else
    {
        direction &= ~(1 << pin);
    }

    mips_gpio_write_register(MIPS_GPIO_DIRECTION, direction);
}

bool mips_gpio_get_pin_direction(uint32_t pin)
{
    if (!mips_gpio_is_initialized())
    {
        return false;
    }

    uint32_t direction = mips_gpio_read_register(MIPS_GPIO_DIRECTION);
    return (direction & (1 << pin)) != 0;
}

void mips_gpio_set_pin_value(uint32_t pin, bool value)
{
    if (!mips_gpio_is_initialized())
    {
        return;
    }

    uint32_t output = mips_gpio_read_register(MIPS_GPIO_OUTPUT);

    if (value)
    {
        output |= (1 << pin);
    }
    else
    {
        output &= ~(1 << pin);
    }

    mips_gpio_write_register(MIPS_GPIO_OUTPUT, output);
}

bool mips_gpio_get_pin_value(uint32_t pin)
{
    if (!mips_gpio_is_initialized())
    {
        return false;
    }

    uint32_t input = mips_gpio_read_register(MIPS_GPIO_INPUT);
    return (input & (1 << pin)) != 0;
}

void mips_gpio_enable_interrupt(uint32_t pin, bool rising_edge)
{
    if (!mips_gpio_is_initialized())
    {
        return;
    }

    uint32_t interrupt_enable = mips_gpio_read_register(MIPS_GPIO_INTERRUPT_ENABLE);
    interrupt_enable |= (1 << pin);
    mips_gpio_write_register(MIPS_GPIO_INTERRUPT_ENABLE, interrupt_enable);

    // Set edge sensitivity
    uint32_t edge_sense = mips_gpio_read_register(MIPS_GPIO_EDGE_SENSE);
    if (rising_edge)
    {
        edge_sense |= (1 << pin);
    }
    else
    {
        edge_sense &= ~(1 << pin);
    }
    mips_gpio_write_register(MIPS_GPIO_EDGE_SENSE, edge_sense);
}

void mips_gpio_disable_interrupt(uint32_t pin)
{
    if (!mips_gpio_is_initialized())
    {
        return;
    }

    uint32_t interrupt_enable = mips_gpio_read_register(MIPS_GPIO_INTERRUPT_ENABLE);
    interrupt_enable &= ~(1 << pin);
    mips_gpio_write_register(MIPS_GPIO_INTERRUPT_ENABLE, interrupt_enable);
}

uint32_t mips_gpio_get_interrupt_status(void)
{
    if (!mips_gpio_is_initialized())
    {
        return 0;
    }

    return mips_gpio_read_register(MIPS_GPIO_INTERRUPT_STATUS);
}

void mips_gpio_clear_interrupt(uint32_t pin)
{
    if (!mips_gpio_is_initialized())
    {
        return;
    }

    uint32_t interrupt_status = mips_gpio_read_register(MIPS_GPIO_INTERRUPT_STATUS);
    interrupt_status |= (1 << pin);
    mips_gpio_write_register(MIPS_GPIO_INTERRUPT_STATUS, interrupt_status);
}

// ============================================================================
// DEVICE UTILITIES
// ============================================================================

void mips_devices_print_status(void)
{
    printf("MIPS: Device Status:\n");
    printf("  Devices Initialized: %s\n", mips_device_context.devices_initialized ? "yes" : "no");
    printf("  Active Devices: 0x%x\n", mips_device_context.active_devices);
    printf("  Device Count: %u\n", mips_device_context.device_count);

    if (mips_device_context.active_devices & MIPS_DEVICE_UART)
    {
        printf("  UART: enabled, interrupts: %s\n",
               mips_device_context.uart.interrupt_enabled ? "enabled" : "disabled");
    }

    if (mips_device_context.active_devices & MIPS_DEVICE_SPI)
    {
        printf("  SPI: enabled, interrupts: %s\n",
               mips_device_context.spi.interrupt_enabled ? "enabled" : "disabled");
    }

    if (mips_device_context.active_devices & MIPS_DEVICE_I2C)
    {
        printf("  I2C: enabled, interrupts: %s\n",
               mips_device_context.i2c.interrupt_enabled ? "enabled" : "disabled");
    }

    if (mips_device_context.active_devices & MIPS_DEVICE_GPIO)
    {
        printf("  GPIO: enabled, interrupts: %s\n",
               mips_device_context.gpio.interrupt_enabled ? "enabled" : "disabled");
    }
}

void mips_devices_enable_all_interrupts(void)
{
    if (mips_device_context.active_devices & MIPS_DEVICE_UART)
    {
        mips_uart_enable_interrupts();
    }

    if (mips_device_context.active_devices & MIPS_DEVICE_SPI)
    {
        mips_spi_enable_interrupts();
    }

    if (mips_device_context.active_devices & MIPS_DEVICE_I2C)
    {
        mips_i2c_enable_interrupts();
    }

    printf("MIPS: All device interrupts enabled\n");
}

void mips_devices_disable_all_interrupts(void)
{
    if (mips_device_context.active_devices & MIPS_DEVICE_UART)
    {
        mips_uart_disable_interrupts();
    }

    if (mips_device_context.active_devices & MIPS_DEVICE_SPI)
    {
        mips_spi_disable_interrupts();
    }

    if (mips_device_context.active_devices & MIPS_DEVICE_I2C)
    {
        mips_i2c_disable_interrupts();
    }

    printf("MIPS: All device interrupts disabled\n");
}

void mips_devices_deinit_all(void)
{
    mips_uart_deinit();
    mips_spi_deinit();
    mips_i2c_deinit();
    mips_gpio_deinit();

    mips_device_context.devices_initialized = false;
    mips_device_context.active_devices = 0;
    mips_device_context.device_count = 0;

    printf("MIPS: All devices deinitialized\n");
}
