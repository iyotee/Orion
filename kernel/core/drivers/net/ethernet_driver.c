/*
 * Orion Operating System - Ethernet Driver Implementation
 *
 * High-performance Ethernet driver with hardware offloading support
 * for various network interface cards
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "ethernet_driver.h"
#include <orion/klog.h>
#include <orion/mm.h>
#include <orion/string.h>
#include <orion/spinlock.h>
#include <orion/time.h>
#include <string.h>

// Global Ethernet driver state
static struct {
    bool initialized;
    orion_ethernet_driver_t *drivers[32];
    int driver_count;
    spinlock_t lock;
} ethernet_drivers = {0};

// Ethernet interface list
static orion_ethernet_config_t *ethernet_interfaces[64];
static int interface_count = 0;
static spinlock_t interface_lock = SPINLOCK_INITIALIZER;

// Ethernet statistics
static orion_ethernet_stats_t global_stats = {0};
static spinlock_t stats_lock = SPINLOCK_INITIALIZER;

/* ============================================================================
 * Ethernet Driver Management
 * ============================================================================ */

int orion_ethernet_register_driver(orion_ethernet_driver_t *driver)
{
    if (!driver || !driver->name) {
        klog_error(KLOG_CAT_KERNEL, "Invalid Ethernet driver");
        return -1;
    }

    spinlock_acquire(&ethernet_drivers.lock);

    if (ethernet_drivers.driver_count >= 32) {
        klog_error(KLOG_CAT_KERNEL, "Maximum number of Ethernet drivers reached");
        spinlock_release(&ethernet_drivers.lock);
        return -1;
    }

    // Check if driver already registered
    for (int i = 0; i < ethernet_drivers.driver_count; i++) {
        if (ethernet_drivers.drivers[i] == driver) {
            klog_warning(KLOG_CAT_KERNEL, "Ethernet driver %s already registered", driver->name);
            spinlock_release(&ethernet_drivers.lock);
            return 0;
        }
    }

    ethernet_drivers.drivers[ethernet_drivers.driver_count++] = driver;
    klog_info(KLOG_CAT_KERNEL, "Ethernet driver %s registered", driver->name);

    spinlock_release(&ethernet_drivers.lock);
    return 0;
}

int orion_ethernet_unregister_driver(orion_ethernet_driver_t *driver)
{
    if (!driver) {
        return -1;
    }

    spinlock_acquire(&ethernet_drivers.lock);

    for (int i = 0; i < ethernet_drivers.driver_count; i++) {
        if (ethernet_drivers.drivers[i] == driver) {
            // Remove driver from array
            for (int j = i; j < ethernet_drivers.driver_count - 1; j++) {
                ethernet_drivers.drivers[j] = ethernet_drivers.drivers[j + 1];
            }
            ethernet_drivers.driver_count--;
            klog_info(KLOG_CAT_KERNEL, "Ethernet driver %s unregistered", driver->name);
            spinlock_release(&ethernet_drivers.lock);
            return 0;
        }
    }

    spinlock_release(&ethernet_drivers.lock);
    klog_warning(KLOG_CAT_KERNEL, "Ethernet driver %s not found for unregistration", driver->name);
    return -1;
}

/* ============================================================================
 * Ethernet Interface Management
 * ============================================================================ */

orion_ethernet_config_t *orion_ethernet_get_interface(const char *name)
{
    if (!name) {
        return NULL;
    }

    spinlock_acquire(&interface_lock);

    for (int i = 0; i < interface_count; i++) {
        if (ethernet_interfaces[i] && 
            strcmp(ethernet_interfaces[i]->name, name) == 0) {
            spinlock_release(&interface_lock);
            return ethernet_interfaces[i];
        }
    }

    spinlock_release(&interface_lock);
    return NULL;
}

int orion_ethernet_get_interfaces(orion_ethernet_config_t *interfaces, int max_count)
{
    if (!interfaces || max_count <= 0) {
        return -1;
    }

    spinlock_acquire(&interface_lock);

    int count = 0;
    for (int i = 0; i < interface_count && count < max_count; i++) {
        if (ethernet_interfaces[i]) {
            memcpy(&interfaces[count], ethernet_interfaces[i], 
                   sizeof(orion_ethernet_config_t));
            count++;
        }
    }

    spinlock_release(&interface_lock);
    return count;
}

int orion_ethernet_configure_interface(const char *name, const orion_ethernet_config_t *config)
{
    if (!name || !config) {
        return -1;
    }

    spinlock_acquire(&interface_lock);

    // Find existing interface
    for (int i = 0; i < interface_count; i++) {
        if (ethernet_interfaces[i] && 
            strcmp(ethernet_interfaces[i]->name, name) == 0) {
            // Update existing interface
            memcpy(ethernet_interfaces[i], config, sizeof(orion_ethernet_config_t));
            klog_info(KLOG_CAT_KERNEL, "Ethernet interface %s configuration updated", name);
            spinlock_release(&interface_lock);
            return 0;
        }
    }

    // Create new interface
    if (interface_count >= 64) {
        klog_error(KLOG_CAT_KERNEL, "Maximum number of Ethernet interfaces reached");
        spinlock_release(&interface_lock);
        return -1;
    }

    ethernet_interfaces[interface_count] = 
        kmalloc(sizeof(orion_ethernet_config_t));
    if (!ethernet_interfaces[interface_count]) {
        klog_error(KLOG_CAT_KERNEL, "Failed to allocate memory for Ethernet interface %s", name);
        spinlock_release(&interface_lock);
        return -1;
    }

    memcpy(ethernet_interfaces[interface_count], config, 
           sizeof(orion_ethernet_config_t));
    interface_count++;

    klog_info(KLOG_CAT_KERNEL, "Ethernet interface %s created", name);
    spinlock_release(&interface_lock);
    return 0;
}

/* ============================================================================
 * Ethernet Statistics Management
 * ============================================================================ */

int orion_ethernet_get_stats(const char *name, orion_ethernet_stats_t *stats)
{
    if (!name || !stats) {
        return -1;
    }

    // Get interface-specific statistics
    orion_ethernet_config_t *iface = orion_ethernet_get_interface(name);
    if (iface) {
        memcpy(stats, &iface->stats, sizeof(orion_ethernet_stats_t));
        return 0;
    }

    // Return global statistics if interface not found
    spinlock_acquire(&stats_lock);
    memcpy(stats, &global_stats, sizeof(orion_ethernet_stats_t));
    spinlock_release(&stats_lock);

    return 0;
}

int orion_ethernet_reset_stats(const char *name)
{
    if (!name) {
        return -1;
    }

    // Reset interface-specific statistics
    orion_ethernet_config_t *iface = orion_ethernet_get_interface(name);
    if (iface) {
        memset(&iface->stats, 0, sizeof(orion_ethernet_stats_t));
        klog_info(KLOG_CAT_KERNEL, "Ethernet interface %s statistics reset", name);
        return 0;
    }

    // Reset global statistics
    spinlock_acquire(&stats_lock);
    memset(&global_stats, 0, sizeof(orion_ethernet_stats_t));
    spinlock_release(&stats_lock);

    klog_info(KLOG_CAT_KERNEL, "Global Ethernet statistics reset");
    return 0;
}

/* ============================================================================
 * Ethernet Packet Functions
 * ============================================================================ */

int orion_ethernet_create_frame(const uint8_t *dst_mac, const uint8_t *src_mac,
                               uint16_t ethertype, const void *payload, size_t payload_len,
                               void **frame, size_t *frame_len)
{
    if (!dst_mac || !src_mac || !frame || !frame_len) {
        return -1;
    }

    size_t total_len = sizeof(orion_ethernet_header_t) + payload_len;
    void *frame_data = kmalloc(total_len);
    if (!frame_data) {
        klog_error(KLOG_CAT_KERNEL, "Failed to allocate memory for Ethernet frame");
        return -1;
    }

    // Create Ethernet header
    orion_ethernet_header_t *header = (orion_ethernet_header_t*)frame_data;
    memcpy(header->dst_mac, dst_mac, 6);
    memcpy(header->src_mac, src_mac, 6);
    header->ethertype = htons(ethertype);

    // Copy payload
    if (payload && payload_len > 0) {
        memcpy((char*)frame_data + sizeof(orion_ethernet_header_t), payload, payload_len);
    }

    *frame = frame_data;
    *frame_len = total_len;

    klog_debug(KLOG_CAT_KERNEL, "Ethernet frame created: %zu bytes, type 0x%04x", 
               total_len, ethertype);

    return 0;
}

int orion_ethernet_parse_frame(const void *frame, size_t frame_len,
                              orion_ethernet_header_t **header, void **payload, size_t *payload_len)
{
    if (!frame || !header || !payload || !payload_len || 
        frame_len < sizeof(orion_ethernet_header_t)) {
        return -1;
    }

    *header = (orion_ethernet_header_t*)frame;
    *payload = (char*)frame + sizeof(orion_ethernet_header_t);
    *payload_len = frame_len - sizeof(orion_ethernet_header_t);

    uint16_t ethertype = ntohs((*header)->ethertype);
    klog_debug(KLOG_CAT_KERNEL, "Ethernet frame parsed: %zu bytes, type 0x%04x", 
               frame_len, ethertype);

    return 0;
}

int orion_ethernet_validate_frame(const void *frame, size_t frame_len)
{
    if (!frame || frame_len < sizeof(orion_ethernet_header_t)) {
        return -1;
    }

    const orion_ethernet_header_t *header = (const orion_ethernet_header_t*)frame;

    // Check minimum frame size (64 bytes including header)
    if (frame_len < 64) {
        klog_warning(KLOG_CAT_KERNEL, "Ethernet frame too short: %zu bytes", frame_len);
        return -1;
    }

    // Check maximum frame size (1518 bytes including header)
    if (frame_len > 1518) {
        klog_warning(KLOG_CAT_KERNEL, "Ethernet frame too long: %zu bytes", frame_len);
        return -1;
    }

    // Validate MAC addresses (check for broadcast/multicast)
    bool is_broadcast = true;
    bool is_multicast = false;

    for (int i = 0; i < 6; i++) {
        if (header->dst_mac[i] != 0xFF) {
            is_broadcast = false;
        }
        if (i == 0 && (header->dst_mac[0] & 0x01)) {
            is_multicast = true;
        }
    }

    if (is_broadcast) {
        klog_debug(KLOG_CAT_KERNEL, "Ethernet broadcast frame received");
    } else if (is_multicast) {
        klog_debug(KLOG_CAT_KERNEL, "Ethernet multicast frame received");
    }

    return 0;
}

/* ============================================================================
 * Ethernet MAC Address Functions
 * ============================================================================ */

int orion_ethernet_mac_to_string(const uint8_t *mac, char *str, size_t str_len)
{
    if (!mac || !str || str_len < 18) {
        return -1;
    }

    snprintf(str, str_len, "%02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return 0;
}

int orion_ethernet_string_to_mac(const char *str, uint8_t *mac)
{
    if (!str || !mac) {
        return -1;
    }

    // Parse MAC address format: xx:xx:xx:xx:xx:xx
    int result = sscanf(str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                        &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

    if (result != 6) {
        klog_error(KLOG_CAT_KERNEL, "Invalid MAC address format: %s", str);
        return -1;
    }

    return 0;
}

bool orion_ethernet_is_broadcast_mac(const uint8_t *mac)
{
    if (!mac) return false;

    for (int i = 0; i < 6; i++) {
        if (mac[i] != 0xFF) {
            return false;
        }
    }

    return true;
}

bool orion_ethernet_is_multicast_mac(const uint8_t *mac)
{
    if (!mac) return false;

    // Check if the least significant bit of the first byte is set
    return (mac[0] & 0x01) != 0;
}

bool orion_ethernet_is_unicast_mac(const uint8_t *mac)
{
    if (!mac) return false;

    // Check if the least significant bit of the first byte is clear
    return (mac[0] & 0x01) == 0;
}

/* ============================================================================
 * Ethernet Feature Functions
 * ============================================================================ */

bool orion_ethernet_supports_speed(const orion_ethernet_config_t *config, uint32_t speed)
{
    if (!config) return false;

    switch (speed) {
        case 10:
            return (config->features & ORION_ETH_FEATURE_10BASE_T) != 0;
        case 100:
            return (config->features & ORION_ETH_FEATURE_100BASE_T) != 0;
        case 1000:
            return (config->features & ORION_ETH_FEATURE_1000BASE_T) != 0;
        case 2500:
            return (config->features & ORION_ETH_FEATURE_2_5GBASE_T) != 0;
        case 5000:
            return (config->features & ORION_ETH_FEATURE_5GBASE_T) != 0;
        case 10000:
            return (config->features & ORION_ETH_FEATURE_10GBASE_T) != 0;
        case 25000:
            return (config->features & ORION_ETH_FEATURE_25GBASE_T) != 0;
        case 40000:
            return (config->features & ORION_ETH_FEATURE_40GBASE_T) != 0;
        case 100000:
            return (config->features & ORION_ETH_FEATURE_100GBASE_T) != 0;
        default:
            return false;
    }
}

bool orion_ethernet_supports_duplex(const orion_ethernet_config_t *config, uint32_t duplex)
{
    if (!config) return false;

    switch (duplex) {
        case ORION_ETH_DUPLEX_HALF:
        case ORION_ETH_DUPLEX_FULL:
        case ORION_ETH_DUPLEX_AUTO:
            return (config->features & duplex) != 0;
        default:
            return false;
    }
}

bool orion_ethernet_supports_flow_control(const orion_ethernet_config_t *config, uint32_t flow_control)
{
    if (!config) return false;

    switch (flow_control) {
        case ORION_ETH_FLOW_CONTROL_NONE:
        case ORION_ETH_FLOW_CONTROL_TX:
        case ORION_ETH_FLOW_CONTROL_RX:
        case ORION_ETH_FLOW_CONTROL_BOTH:
            return true; // Most modern Ethernet interfaces support flow control
        default:
            return false;
    }
}

/* ============================================================================
 * Ethernet Statistics Functions
 * ============================================================================ */

void orion_ethernet_update_rx_stats(orion_ethernet_stats_t *stats, size_t frame_len, bool has_errors)
{
    if (!stats) return;

    spinlock_acquire(&stats_lock);

    stats->rx_frames++;
    stats->rx_bytes += frame_len;
    stats->last_rx_time = orion_get_timestamp();

    if (has_errors) {
        stats->rx_errors++;
    }

    // Update global statistics
    global_stats.rx_frames++;
    global_stats.rx_bytes += frame_len;
    global_stats.last_rx_time = orion_get_timestamp();

    if (has_errors) {
        global_stats.rx_errors++;
    }

    spinlock_release(&stats_lock);
}

void orion_ethernet_update_tx_stats(orion_ethernet_stats_t *stats, size_t frame_len, bool has_errors)
{
    if (!stats) return;

    spinlock_acquire(&stats_lock);

    stats->tx_frames++;
    stats->tx_bytes += frame_len;
    stats->last_tx_time = orion_get_timestamp();

    if (has_errors) {
        stats->tx_errors++;
    }

    // Update global statistics
    global_stats.tx_frames++;
    global_stats.tx_bytes += frame_len;
    global_stats.last_tx_time = orion_get_timestamp();

    if (has_errors) {
        global_stats.tx_errors++;
    }

    spinlock_release(&stats_lock);
}

void orion_ethernet_update_error_stats(orion_ethernet_stats_t *stats, uint32_t error_type)
{
    if (!stats) return;

    spinlock_acquire(&stats_lock);

    switch (error_type) {
        case ORION_ETH_ERROR_CRC:
            stats->rx_crc_errors++;
            global_stats.rx_crc_errors++;
            break;
        case ORION_ETH_ERROR_FRAME:
            stats->rx_frame_errors++;
            global_stats.rx_frame_errors++;
            break;
        case ORION_ETH_ERROR_FIFO:
            stats->rx_fifo_errors++;
            global_stats.rx_fifo_errors++;
            break;
        case ORION_ETH_ERROR_MISSED:
            stats->rx_missed_errors++;
            global_stats.rx_missed_errors++;
            break;
        case ORION_ETH_ERROR_CARRIER:
            stats->tx_carrier_errors++;
            global_stats.tx_carrier_errors++;
            break;
        case ORION_ETH_ERROR_HEARTBEAT:
            stats->tx_heartbeat_errors++;
            global_stats.tx_heartbeat_errors++;
            break;
        case ORION_ETH_ERROR_WINDOW:
            stats->tx_window_errors++;
            global_stats.tx_window_errors++;
            break;
        default:
            break;
    }

    spinlock_release(&stats_lock);
}

/* ============================================================================
 * Ethernet Utility Functions
 * ============================================================================ */

uint32_t orion_ethernet_calculate_checksum(const void *data, size_t len)
{
    if (!data || len == 0) return 0;

    const uint8_t *bytes = (const uint8_t*)data;
    uint32_t checksum = 0;

    // Simple checksum calculation
    for (size_t i = 0; i < len; i++) {
        checksum += bytes[i];
    }

    return checksum;
}

bool orion_ethernet_is_valid_mtu(uint32_t mtu)
{
    // Ethernet MTU must be between 68 and 9000 bytes
    // 68 = minimum frame size (64) - header (14) + FCS (4) + minimum payload (46)
    // 9000 = typical jumbo frame limit
    return (mtu >= 68 && mtu <= 9000);
}

uint32_t orion_ethernet_get_max_payload_size(uint32_t mtu)
{
    if (!orion_ethernet_is_valid_mtu(mtu)) {
        return 0;
    }

    // Maximum payload = MTU - header (14) - FCS (4)
    return mtu - 18;
}

uint32_t orion_ethernet_get_min_payload_size(void)
{
    // Minimum payload size to ensure minimum frame size of 64 bytes
    // 64 - header (14) - FCS (4) = 46 bytes
    return 46;
}

/* ============================================================================
 * Ethernet Driver Helper Functions
 * ============================================================================ */

int orion_ethernet_driver_probe(orion_ethernet_driver_t *driver, void *device)
{
    if (!driver || !driver->probe) {
        return -1;
    }

    int result = driver->probe(driver, device);
    if (result == 0) {
        klog_info(KLOG_CAT_KERNEL, "Ethernet driver %s probed successfully", driver->name);
    } else {
        klog_error(KLOG_CAT_KERNEL, "Ethernet driver %s probe failed", driver->name);
    }

    return result;
}

int orion_ethernet_driver_remove(orion_ethernet_driver_t *driver, void *device)
{
    if (!driver || !driver->remove) {
        return -1;
    }

    int result = driver->remove(driver, device);
    if (result == 0) {
        klog_info(KLOG_CAT_KERNEL, "Ethernet driver %s removed successfully", driver->name);
    } else {
        klog_error(KLOG_CAT_KERNEL, "Ethernet driver %s removal failed", driver->name);
    }

    return result;
}

int orion_ethernet_driver_open(orion_ethernet_driver_t *driver)
{
    if (!driver || !driver->open) {
        return -1;
    }

    int result = driver->open(driver);
    if (result == 0) {
        klog_info(KLOG_CAT_KERNEL, "Ethernet driver %s opened successfully", driver->name);
    } else {
        klog_error(KLOG_CAT_KERNEL, "Ethernet driver %s open failed", driver->name);
    }

    return result;
}

int orion_ethernet_driver_close(orion_ethernet_driver_t *driver)
{
    if (!driver || !driver->close) {
        return -1;
    }

    int result = driver->close(driver);
    if (result == 0) {
        klog_info(KLOG_CAT_KERNEL, "Ethernet driver %s closed successfully", driver->name);
    } else {
        klog_error(KLOG_CAT_KERNEL, "Ethernet driver %s close failed", driver->name);
    }

    return result;
}

/* ============================================================================
 * Ethernet Initialization and Cleanup
 * ============================================================================ */

int orion_ethernet_init(void)
{
    if (ethernet_drivers.initialized) {
        klog_error(KLOG_CAT_KERNEL, "Ethernet subsystem already initialized");
        return -1;
    }

    ethernet_drivers.initialized = true;
    ethernet_drivers.driver_count = 0;
    interface_count = 0;

    // Initialize global statistics
    memset(&global_stats, 0, sizeof(global_stats));

    klog_info(KLOG_CAT_KERNEL, "Ethernet subsystem initialized successfully");
    return 0;
}

void orion_ethernet_cleanup(void)
{
    if (!ethernet_drivers.initialized) {
        return;
    }

    klog_info(KLOG_CAT_KERNEL, "Cleaning up Ethernet subsystem");

    // Clean up interfaces
    spinlock_acquire(&interface_lock);
    for (int i = 0; i < interface_count; i++) {
        if (ethernet_interfaces[i]) {
            kfree(ethernet_interfaces[i]);
            ethernet_interfaces[i] = NULL;
        }
    }
    interface_count = 0;
    spinlock_release(&interface_lock);

    // Clean up drivers
    spinlock_acquire(&ethernet_drivers.lock);
    ethernet_drivers.driver_count = 0;
    spinlock_release(&ethernet_drivers.lock);

    ethernet_drivers.initialized = false;
    klog_info(KLOG_CAT_KERNEL, "Ethernet subsystem cleanup complete");
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

uint64_t orion_get_timestamp(void)
{
    // Simple timestamp implementation
    // In a real system, this would use proper time functions
    static uint64_t timestamp = 0;
    return ++timestamp;
}

// Error type definitions for statistics
#define ORION_ETH_ERROR_CRC        1
#define ORION_ETH_ERROR_FRAME      2
#define ORION_ETH_ERROR_FIFO       3
#define ORION_ETH_ERROR_MISSED     4
#define ORION_ETH_ERROR_CARRIER    5
#define ORION_ETH_ERROR_HEARTBEAT  6
#define ORION_ETH_ERROR_WINDOW     7
