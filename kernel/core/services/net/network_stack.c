/*
 * Orion Operating System - Network Stack Implementation
 *
 * Core network stack implementation with high-performance networking
 * supporting IPv4/IPv6, TCP/UDP, and advanced protocols
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "network_architecture.h"
#include "tcp_ip_stack.h"
#include "advanced_protocols.h"
#include <orion/klog.h>
#include <orion/mm.h>
#include <orion/string.h>
#include <orion/spinlock.h>
#include <orion/time.h>
#include <string.h>

// Global network stack state
static struct {
    bool initialized;
    orion_net_stack_config_t config;
    orion_net_driver_t *drivers[32];
    int driver_count;
    orion_net_iface_config_t *interfaces[64];
    int interface_count;
    spinlock_t lock;
} network_stack = {0};

// Network stack statistics
static struct {
    uint64_t packets_processed;
    uint64_t bytes_processed;
    uint64_t errors;
    uint64_t dropped_packets;
    uint64_t last_update;
} network_stats = {0};

/* ============================================================================
 * Network Stack Core Functions
 * ============================================================================ */

int orion_net_stack_init(const orion_net_stack_config_t *config)
{
    if (network_stack.initialized) {
        klog_error(KLOG_CAT_KERNEL, "Network stack already initialized");
        return -1;
    }

    if (!config) {
        klog_error(KLOG_CAT_KERNEL, "Invalid network stack configuration");
        return -1;
    }

    spinlock_acquire(&network_stack.lock);

    // Initialize network stack
    memcpy(&network_stack.config, config, sizeof(orion_net_stack_config_t));
    network_stack.driver_count = 0;
    network_stack.interface_count = 0;
    network_stack.initialized = true;

    // Initialize subsystems
    int result = 0;
    result |= orion_ip_init();
    result |= orion_tcp_init(NULL);
    result |= orion_udp_init();
    result |= orion_icmp_init();
    result |= orion_http_server_init(NULL);
    result |= orion_websocket_init();
    result |= orion_grpc_init();
    result |= orion_quic_init();
    result |= orion_dns_init();
    result |= orion_dhcp_init();

    if (result != 0) {
        klog_error(KLOG_CAT_KERNEL, "Failed to initialize network subsystems");
        network_stack.initialized = false;
        spinlock_release(&network_stack.lock);
        return -1;
    }

    klog_info(KLOG_CAT_KERNEL, "Network stack initialized successfully");
    klog_info(KLOG_CAT_KERNEL, "Max interfaces: %d, Max connections: %d",
              config->max_interfaces, config->max_connections);

    spinlock_release(&network_stack.lock);
    return 0;
}

int orion_net_stack_shutdown(void)
{
    if (!network_stack.initialized) {
        return 0;
    }

    spinlock_acquire(&network_stack.lock);

    klog_info(KLOG_CAT_KERNEL, "Shutting down network stack");

    // Shutdown subsystems
    orion_ip_shutdown();
    orion_tcp_shutdown();
    orion_udp_shutdown();
    orion_icmp_shutdown();

    // Unregister all drivers
    for (int i = 0; i < network_stack.driver_count; i++) {
        if (network_stack.drivers[i]) {
            orion_net_unregister_driver(network_stack.drivers[i]);
        }
    }

    network_stack.initialized = false;
    network_stack.driver_count = 0;
    network_stack.interface_count = 0;

    spinlock_release(&network_stack.lock);
    klog_info(KLOG_CAT_KERNEL, "Network stack shutdown complete");

    return 0;
}

int orion_net_stack_get_config(orion_net_stack_config_t *config)
{
    if (!network_stack.initialized || !config) {
        return -1;
    }

    spinlock_acquire(&network_stack.lock);
    memcpy(config, &network_stack.config, sizeof(orion_net_stack_config_t));
    spinlock_release(&network_stack.lock);

    return 0;
}

int orion_net_stack_set_config(const orion_net_stack_config_t *config)
{
    if (!network_stack.initialized || !config) {
        return -1;
    }

    spinlock_acquire(&network_stack.lock);
    memcpy(&network_stack.config, config, sizeof(orion_net_stack_config_t));
    spinlock_release(&network_stack.lock);

    klog_info(KLOG_CAT_KERNEL, "Network stack configuration updated");
    return 0;
}

int orion_net_stack_get_status(void)
{
    if (!network_stack.initialized) {
        return -1;
    }

    return 0; // 0 = running, negative = error
}

int orion_net_stack_get_stats(void *stats)
{
    if (!network_stack.initialized || !stats) {
        return -1;
    }

    spinlock_acquire(&network_stack.lock);
    memcpy(stats, &network_stats, sizeof(network_stats));
    spinlock_release(&network_stack.lock);

    return 0;
}

/* ============================================================================
 * Network Driver Management
 * ============================================================================ */

int orion_net_register_driver(orion_net_driver_t *driver)
{
    if (!network_stack.initialized || !driver) {
        return -1;
    }

    spinlock_acquire(&network_stack.lock);

    if (network_stack.driver_count >= 32) {
        klog_error(KLOG_CAT_KERNEL, "Maximum number of network drivers reached");
        spinlock_release(&network_stack.lock);
        return -1;
    }

    // Check if driver already registered
    for (int i = 0; i < network_stack.driver_count; i++) {
        if (network_stack.drivers[i] == driver) {
            klog_warning(KLOG_CAT_KERNEL, "Driver %s already registered", driver->name);
            spinlock_release(&network_stack.lock);
            return 0;
        }
    }

    network_stack.drivers[network_stack.driver_count++] = driver;
    klog_info(KLOG_CAT_KERNEL, "Network driver %s registered", driver->name);

    spinlock_release(&network_stack.lock);
    return 0;
}

int orion_net_unregister_driver(orion_net_driver_t *driver)
{
    if (!network_stack.initialized || !driver) {
        return -1;
    }

    spinlock_acquire(&network_stack.lock);

    for (int i = 0; i < network_stack.driver_count; i++) {
        if (network_stack.drivers[i] == driver) {
            // Remove driver from array
            for (int j = i; j < network_stack.driver_count - 1; j++) {
                network_stack.drivers[j] = network_stack.drivers[j + 1];
            }
            network_stack.driver_count--;
            klog_info(KLOG_CAT_KERNEL, "Network driver %s unregistered", driver->name);
            spinlock_release(&network_stack.lock);
            return 0;
        }
    }

    spinlock_release(&network_stack.lock);
    klog_warning(KLOG_CAT_KERNEL, "Driver %s not found for unregistration", driver->name);
    return -1;
}

/* ============================================================================
 * Network Interface Management
 * ============================================================================ */

orion_net_iface_config_t *orion_net_get_interface(const char *name)
{
    if (!network_stack.initialized || !name) {
        return NULL;
    }

    spinlock_acquire(&network_stack.lock);

    for (int i = 0; i < network_stack.interface_count; i++) {
        if (network_stack.interfaces[i] && 
            strcmp(network_stack.interfaces[i]->name, name) == 0) {
            spinlock_release(&network_stack.lock);
            return network_stack.interfaces[i];
        }
    }

    spinlock_release(&network_stack.lock);
    return NULL;
}

int orion_net_get_interfaces(orion_net_iface_config_t *interfaces, int max_count)
{
    if (!network_stack.initialized || !interfaces || max_count <= 0) {
        return -1;
    }

    spinlock_acquire(&network_stack.lock);

    int count = 0;
    for (int i = 0; i < network_stack.interface_count && count < max_count; i++) {
        if (network_stack.interfaces[i]) {
            memcpy(&interfaces[count], network_stack.interfaces[i], 
                   sizeof(orion_net_iface_config_t));
            count++;
        }
    }

    spinlock_release(&network_stack.lock);
    return count;
}

int orion_net_configure_interface(const char *name, const orion_net_iface_config_t *config)
{
    if (!network_stack.initialized || !name || !config) {
        return -1;
    }

    spinlock_acquire(&network_stack.lock);

    // Find existing interface
    for (int i = 0; i < network_stack.interface_count; i++) {
        if (network_stack.interfaces[i] && 
            strcmp(network_stack.interfaces[i]->name, name) == 0) {
            // Update existing interface
            memcpy(network_stack.interfaces[i], config, sizeof(orion_net_iface_config_t));
            klog_info(KLOG_CAT_KERNEL, "Interface %s configuration updated", name);
            spinlock_release(&network_stack.lock);
            return 0;
        }
    }

    // Create new interface
    if (network_stack.interface_count >= 64) {
        klog_error(KLOG_CAT_KERNEL, "Maximum number of interfaces reached");
        spinlock_release(&network_stack.lock);
        return -1;
    }

    network_stack.interfaces[network_stack.interface_count] = 
        kmalloc(sizeof(orion_net_iface_config_t));
    if (!network_stack.interfaces[network_stack.interface_count]) {
        klog_error(KLOG_CAT_KERNEL, "Failed to allocate memory for interface %s", name);
        spinlock_release(&network_stack.lock);
        return -1;
    }

    memcpy(network_stack.interfaces[network_stack.interface_count], config, 
           sizeof(orion_net_iface_config_t));
    network_stack.interface_count++;

    klog_info(KLOG_CAT_KERNEL, "Interface %s created", name);
    spinlock_release(&network_stack.lock);
    return 0;
}

/* ============================================================================
 * Network Feature Management
 * ============================================================================ */

int orion_net_enable_feature(uint64_t feature)
{
    if (!network_stack.initialized) {
        return -1;
    }

    spinlock_acquire(&network_stack.lock);
    network_stack.config.features |= feature;
    spinlock_release(&network_stack.lock);

    klog_info(KLOG_CAT_KERNEL, "Network feature 0x%llx enabled", feature);
    return 0;
}

int orion_net_disable_feature(uint64_t feature)
{
    if (!network_stack.initialized) {
        return -1;
    }

    spinlock_acquire(&network_stack.lock);
    network_stack.config.features &= ~feature;
    spinlock_release(&network_stack.lock);

    klog_info(KLOG_CAT_KERNEL, "Network feature 0x%llx disabled", feature);
    return 0;
}

bool orion_net_is_feature_enabled(uint64_t feature)
{
    if (!network_stack.initialized) {
        return false;
    }

    spinlock_acquire(&network_stack.lock);
    bool enabled = (network_stack.config.features & feature) != 0;
    spinlock_release(&network_stack.lock);

    return enabled;
}

int orion_net_get_performance_metrics(void *metrics)
{
    if (!network_stack.initialized || !metrics) {
        return -1;
    }

    // Update statistics
    spinlock_acquire(&network_stack.lock);
    network_stats.last_update = orion_get_timestamp();
    memcpy(metrics, &network_stats, sizeof(network_stats));
    spinlock_release(&network_stack.lock);

    return 0;
}

/* ============================================================================
 * Network Security Functions
 * ============================================================================ */

int orion_net_enable_security(uint64_t security)
{
    if (!network_stack.initialized) {
        return -1;
    }

    spinlock_acquire(&network_stack.lock);
    network_stack.config.security_level |= security;
    spinlock_release(&network_stack.lock);

    klog_info(KLOG_CAT_KERNEL, "Network security feature 0x%llx enabled", security);
    return 0;
}

int orion_net_disable_security(uint64_t security)
{
    if (!network_stack.initialized) {
        return -1;
    }

    spinlock_acquire(&network_stack.lock);
    network_stack.config.security_level &= ~security;
    spinlock_release(&network_stack.lock);

    klog_info(KLOG_CAT_KERNEL, "Network security feature 0x%llx disabled", security);
    return 0;
}

bool orion_net_is_security_enabled(uint64_t security)
{
    if (!network_stack.initialized) {
        return false;
    }

    spinlock_acquire(&network_stack.lock);
    bool enabled = (network_stack.config.security_level & security) != 0;
    spinlock_release(&network_stack.lock);

    return enabled;
}

int orion_net_get_security_status(void *status)
{
    if (!network_stack.initialized || !status) {
        return -1;
    }

    // Return security configuration
    spinlock_acquire(&network_stack.lock);
    memcpy(status, &network_stack.config.security_level, sizeof(uint32_t));
    spinlock_release(&network_stack.lock);

    return 0;
}

/* ============================================================================
 * Network Packet Processing
 * ============================================================================ */

int orion_net_process_packet(void *packet, size_t len)
{
    if (!network_stack.initialized || !packet || len == 0) {
        return -1;
    }

    spinlock_acquire(&network_stack.lock);
    network_stats.packets_processed++;
    network_stats.bytes_processed += len;
    spinlock_release(&network_stack.lock);

    // Process packet based on protocol
    // This is a simplified implementation
    // In a real system, this would parse headers and route to appropriate handlers

    klog_debug(KLOG_CAT_KERNEL, "Processed network packet, length: %zu", len);
    return 0;
}

/* ============================================================================
 * Network Stack Health Monitoring
 * ============================================================================ */

int orion_net_check_health(void)
{
    if (!network_stack.initialized) {
        return -1;
    }

    spinlock_acquire(&network_stack.lock);
    
    bool healthy = true;
    int issues = 0;

    // Check driver health
    for (int i = 0; i < network_stack.driver_count; i++) {
        if (!network_stack.drivers[i]) {
            issues++;
            healthy = false;
        }
    }

    // Check interface health
    for (int i = 0; i < network_stack.interface_count; i++) {
        if (!network_stack.interfaces[i]) {
            issues++;
            healthy = false;
        }
    }

    spinlock_release(&network_stack.lock);

    if (!healthy) {
        klog_warning(KLOG_CAT_KERNEL, "Network stack health check failed, %d issues found", issues);
        return -1;
    }

    klog_debug(KLOG_CAT_KERNEL, "Network stack health check passed");
    return 0;
}

int orion_net_get_driver_count(void)
{
    if (!network_stack.initialized) {
        return -1;
    }

    spinlock_acquire(&network_stack.lock);
    int count = network_stack.driver_count;
    spinlock_release(&network_stack.lock);

    return count;
}

int orion_net_get_interface_count(void)
{
    if (!network_stack.initialized) {
        return -1;
    }

    spinlock_acquire(&network_stack.lock);
    int count = network_stack.interface_count;
    spinlock_release(&network_stack.lock);

    return count;
}
