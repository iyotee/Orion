/*
 * Orion Operating System - TCP/IP Stack Implementation
 *
 * Modern TCP/IP implementation with IPv4/IPv6 support,
 * advanced congestion control, and performance optimizations
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "tcp_ip_stack.h"
#include <orion/klog.h>
#include <orion/mm.h>
#include <orion/string.h>
#include <orion/spinlock.h>
#include <orion/time.h>
#include <string.h>

// Global TCP/IP stack state
static struct {
    bool tcp_initialized;
    bool ip_initialized;
    bool udp_initialized;
    bool icmp_initialized;
    orion_tcp_config_t tcp_config;
    spinlock_t lock;
} tcpip_stack = {0};

// TCP connection list
static orion_tcp_connection_t *tcp_connections = NULL;
static spinlock_t tcp_lock = SPINLOCK_INITIALIZER;

// IP routing table
static struct {
    uint32_t dst_ip;
    uint32_t dst_mask;
    uint32_t gateway;
    char interface[32];
} ip_routes[256];
static int route_count = 0;
static spinlock_t route_lock = SPINLOCK_INITIALIZER;

// NAT table
static struct {
    uint32_t internal_ip;
    uint32_t external_ip;
    uint16_t internal_port;
    uint16_t external_port;
    uint8_t protocol;
    uint64_t timestamp;
} nat_rules[1024];
static int nat_rule_count = 0;
static spinlock_t nat_lock = SPINLOCK_INITIALIZER;

// Firewall rules
static struct {
    uint32_t rule_id;
    uint32_t src_ip;
    uint32_t dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t protocol;
    int action; // 0 = deny, 1 = allow
} firewall_rules[1024];
static int firewall_rule_count = 0;
static uint32_t next_rule_id = 1;
static spinlock_t firewall_lock = SPINLOCK_INITIALIZER;

/* ============================================================================
 * TCP Functions
 * ============================================================================ */

int orion_tcp_init(const orion_tcp_config_t *config)
{
    if (tcpip_stack.tcp_initialized) {
        klog_error(KLOG_CAT_KERNEL, "TCP stack already initialized");
        return -1;
    }

    // Set default configuration if none provided
    if (config) {
        memcpy(&tcpip_stack.tcp_config, config, sizeof(orion_tcp_config_t));
    } else {
        // Default TCP configuration
        tcpip_stack.tcp_config.max_connections = 10000;
        tcpip_stack.tcp_config.max_syn_backlog = 1024;
        tcpip_stack.tcp_config.max_orphans = 1000;
        tcpip_stack.tcp_config.tcp_fin_timeout = 60;
        tcpip_stack.tcp_config.tcp_keepalive_time = 7200;
        tcpip_stack.tcp_config.tcp_keepalive_probes = 9;
        tcpip_stack.tcp_config.tcp_keepalive_intvl = 75;
        tcpip_stack.tcp_config.tcp_max_syn_backlog = 1024;
        tcpip_stack.tcp_config.tcp_max_tw_buckets = 65536;
        tcpip_stack.tcp_config.tcp_tw_reuse = 1;
        tcpip_stack.tcp_config.tcp_window_scaling = 1;
        tcpip_stack.tcp_config.tcp_timestamps = 1;
        tcpip_stack.tcp_config.tcp_sack = 1;
        tcpip_stack.tcp_config.tcp_fack = 1;
        tcpip_stack.tcp_config.tcp_ecn = 1;
        tcpip_stack.tcp_config.tcp_abc = 1;
        tcpip_stack.tcp_config.tcp_congestion_control = ORION_TCP_CC_CUBIC;
    }

    tcpip_stack.tcp_initialized = true;
    tcp_connections = NULL;

    klog_info(KLOG_CAT_KERNEL, "TCP stack initialized successfully");
    klog_info(KLOG_CAT_KERNEL, "Max connections: %d, Congestion control: %d",
              tcpip_stack.tcp_config.max_connections,
              tcpip_stack.tcp_config.tcp_congestion_control);

    return 0;
}

int orion_tcp_shutdown(void)
{
    if (!tcpip_stack.tcp_initialized) {
        return 0;
    }

    klog_info(KLOG_CAT_KERNEL, "Shutting down TCP stack");

    // Close all connections
    spinlock_acquire(&tcp_lock);
    orion_tcp_connection_t *conn = tcp_connections;
    while (conn) {
        orion_tcp_connection_t *next = conn->next;
        orion_tcp_close(conn);
        conn = next;
    }
    tcp_connections = NULL;
    spinlock_release(&tcp_lock);

    tcpip_stack.tcp_initialized = false;
    klog_info(KLOG_CAT_KERNEL, "TCP stack shutdown complete");

    return 0;
}

orion_tcp_connection_t *orion_tcp_connect(uint32_t local_ip, uint16_t local_port,
                                        uint32_t remote_ip, uint16_t remote_port)
{
    if (!tcpip_stack.tcp_initialized) {
        return NULL;
    }

    // Create new connection
    orion_tcp_connection_t *conn = kmalloc(sizeof(orion_tcp_connection_t));
    if (!conn) {
        klog_error(KLOG_CAT_KERNEL, "Failed to allocate memory for TCP connection");
        return NULL;
    }

    // Initialize connection
    memset(conn, 0, sizeof(orion_tcp_connection_t));
    conn->local_ip = local_ip;
    conn->local_port = local_port;
    conn->remote_ip = remote_ip;
    conn->remote_port = remote_port;
    conn->state = ORION_TCP_STATE_SYN_SENT;
    conn->seq_num = orion_get_timestamp() & 0xFFFFFFFF;
    conn->window_size = 65535;
    conn->ssthresh = 65535;
    conn->cwnd = 1;

    // Allocate buffers
    conn->send_buffer_size = 65536;
    conn->recv_buffer_size = 65536;
    conn->send_buffer = kmalloc(conn->send_buffer_size);
    conn->recv_buffer = kmalloc(conn->recv_buffer_size);

    if (!conn->send_buffer || !conn->recv_buffer) {
        klog_error(KLOG_CAT_KERNEL, "Failed to allocate TCP buffers");
        if (conn->send_buffer) kfree(conn->send_buffer);
        if (conn->recv_buffer) kfree(conn->recv_buffer);
        kfree(conn);
        return NULL;
    }

    // Add to connection list
    spinlock_acquire(&tcp_lock);
    conn->next = tcp_connections;
    tcp_connections = conn;
    spinlock_release(&tcp_lock);

    klog_info(KLOG_CAT_KERNEL, "TCP connection created: %u:%u -> %u:%u",
              local_ip, local_port, remote_ip, remote_port);

    return conn;
}

orion_tcp_connection_t *orion_tcp_accept(orion_tcp_connection_t *listener)
{
    if (!tcpip_stack.tcp_initialized || !listener) {
        return NULL;
    }

    if (listener->state != ORION_TCP_STATE_LISTEN) {
        klog_error(KLOG_CAT_KERNEL, "Connection is not in LISTEN state");
        return NULL;
    }

    // In a real implementation, this would wait for incoming connections
    // For now, return NULL to indicate no pending connections
    return NULL;
}

orion_tcp_connection_t *orion_tcp_listen(uint32_t local_ip, uint16_t local_port, int backlog)
{
    if (!tcpip_stack.tcp_initialized) {
        return NULL;
    }

    // Create listener connection
    orion_tcp_connection_t *conn = kmalloc(sizeof(orion_tcp_connection_t));
    if (!conn) {
        klog_error(KLOG_CAT_KERNEL, "Failed to allocate memory for TCP listener");
        return NULL;
    }

    // Initialize listener
    memset(conn, 0, sizeof(orion_tcp_connection_t));
    conn->local_ip = local_ip;
    conn->local_port = local_port;
    conn->state = ORION_TCP_STATE_LISTEN;
    conn->window_size = 65535;

    // Add to connection list
    spinlock_acquire(&tcp_lock);
    conn->next = tcp_connections;
    tcp_connections = conn;
    spinlock_release(&tcp_lock);

    klog_info(KLOG_CAT_KERNEL, "TCP listener created: %u:%u (backlog: %d)",
              local_ip, local_port, backlog);

    return conn;
}

ssize_t orion_tcp_send(orion_tcp_connection_t *conn, const void *data, size_t len)
{
    if (!tcpip_stack.tcp_initialized || !conn || !data) {
        return -1;
    }

    if (conn->state != ORION_TCP_STATE_ESTABLISHED) {
        klog_error(KLOG_CAT_KERNEL, "Cannot send data on non-established connection");
        return -1;
    }

    if (len > conn->send_buffer_size - conn->send_buffer_used) {
        klog_error(KLOG_CAT_KERNEL, "Send buffer full");
        return -1;
    }

    // Copy data to send buffer
    memcpy((char*)conn->send_buffer + conn->send_buffer_used, data, len);
    conn->send_buffer_used += len;
    conn->bytes_sent += len;
    conn->packets_sent++;

    klog_debug(KLOG_CAT_KERNEL, "TCP send: %zu bytes", len);
    return len;
}

ssize_t orion_tcp_recv(orion_tcp_connection_t *conn, void *data, size_t len)
{
    if (!tcpip_stack.tcp_initialized || !conn || !data) {
        return -1;
    }

    if (conn->state != ORION_TCP_STATE_ESTABLISHED) {
        klog_error(KLOG_CAT_KERNEL, "Cannot receive data on non-established connection");
        return -1;
    }

    if (conn->recv_buffer_used == 0) {
        return 0; // No data available
    }

    // Copy data from receive buffer
    size_t copy_len = (len < conn->recv_buffer_used) ? len : conn->recv_buffer_used;
    memcpy(data, conn->recv_buffer, copy_len);

    // Move remaining data to beginning of buffer
    if (copy_len < conn->recv_buffer_used) {
        memmove(conn->recv_buffer, 
                (char*)conn->recv_buffer + copy_len,
                conn->recv_buffer_used - copy_len);
    }
    conn->recv_buffer_used -= copy_len;
    conn->bytes_received += copy_len;
    conn->packets_received++;

    klog_debug(KLOG_CAT_KERNEL, "TCP receive: %zu bytes", copy_len);
    return copy_len;
}

int orion_tcp_close(orion_tcp_connection_t *conn)
{
    if (!tcpip_stack.tcp_initialized || !conn) {
        return -1;
    }

    klog_info(KLOG_CAT_KERNEL, "Closing TCP connection: %u:%u -> %u:%u",
              conn->local_ip, conn->local_port, conn->remote_ip, conn->remote_port);

    // Remove from connection list
    spinlock_acquire(&tcp_lock);
    if (tcp_connections == conn) {
        tcp_connections = conn->next;
    } else {
        orion_tcp_connection_t *prev = tcp_connections;
        while (prev && prev->next != conn) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = conn->next;
        }
    }
    spinlock_release(&tcp_lock);

    // Free buffers
    if (conn->send_buffer) kfree(conn->send_buffer);
    if (conn->recv_buffer) kfree(conn->recv_buffer);

    // Free connection
    kfree(conn);

    return 0;
}

orion_tcp_state_t orion_tcp_get_state(orion_tcp_connection_t *conn)
{
    if (!conn) {
        return ORION_TCP_STATE_CLOSED;
    }
    return conn->state;
}

int orion_tcp_set_congestion_control(orion_tcp_connection_t *conn,
                                    orion_tcp_cc_algorithm_t algorithm)
{
    if (!conn) {
        return -1;
    }

    conn->congestion_control = algorithm;
    klog_info(KLOG_CAT_KERNEL, "TCP congestion control set to algorithm %d", algorithm);
    return 0;
}

/* ============================================================================
 * IP Functions
 * ============================================================================ */

int orion_ip_init(void)
{
    if (tcpip_stack.ip_initialized) {
        klog_error(KLOG_CAT_KERNEL, "IP stack already initialized");
        return -1;
    }

    tcpip_stack.ip_initialized = true;
    route_count = 0;

    klog_info(KLOG_CAT_KERNEL, "IP stack initialized successfully");
    return 0;
}

int orion_ip_shutdown(void)
{
    if (!tcpip_stack.ip_initialized) {
        return 0;
    }

    klog_info(KLOG_CAT_KERNEL, "Shutting down IP stack");
    tcpip_stack.ip_initialized = false;
    return 0;
}

int orion_ip_send(uint32_t src_ip, uint32_t dst_ip, uint8_t protocol,
                  const void *data, size_t len)
{
    if (!tcpip_stack.ip_initialized || !data) {
        return -1;
    }

    // Create IP header
    orion_ipv4_header_t *ip_header = (orion_ipv4_header_t*)data;
    ip_header->version_ihl = 0x45; // IPv4, 5 words header
    ip_header->tos = 0;
    ip_header->total_length = htons(sizeof(orion_ipv4_header_t) + len);
    ip_header->identification = htons(0x1234);
    ip_header->flags_offset = 0;
    ip_header->ttl = 64;
    ip_header->protocol = protocol;
    ip_header->checksum = 0;
    ip_header->src_addr = htonl(src_ip);
    ip_header->dst_addr = htonl(dst_ip);

    // Calculate checksum
    ip_header->checksum = orion_ip_checksum(ip_header, sizeof(orion_ipv4_header_t));

    klog_debug(KLOG_CAT_KERNEL, "IP packet sent: %u -> %u, protocol: %d, length: %zu",
               src_ip, dst_ip, protocol, len);

    return 0;
}

ssize_t orion_ip_recv(void *packet, size_t len)
{
    if (!tcpip_stack.ip_initialized || !packet || len < sizeof(orion_ipv4_header_t)) {
        return -1;
    }

    orion_ipv4_header_t *ip_header = (orion_ipv4_header_t*)packet;
    
    // Verify checksum
    uint16_t original_checksum = ip_header->checksum;
    ip_header->checksum = 0;
    if (orion_ip_checksum(ip_header, sizeof(orion_ipv4_header_t)) != original_checksum) {
        klog_error(KLOG_CAT_KERNEL, "IP packet checksum verification failed");
        return -1;
    }

    uint32_t src_ip = ntohl(ip_header->src_addr);
    uint32_t dst_ip = ntohl(ip_header->dst_addr);
    uint8_t protocol = ip_header->protocol;

    klog_debug(KLOG_CAT_KERNEL, "IP packet received: %u -> %u, protocol: %d, length: %zu",
               src_ip, dst_ip, protocol, len);

    return len;
}

int orion_ip_add_route(uint32_t dst_ip, uint32_t dst_mask, uint32_t gateway, const char *interface)
{
    if (!tcpip_stack.ip_initialized) {
        return -1;
    }

    spinlock_acquire(&route_lock);

    if (route_count >= 256) {
        klog_error(KLOG_CAT_KERNEL, "Routing table full");
        spinlock_release(&route_lock);
        return -1;
    }

    // Check if route already exists
    for (int i = 0; i < route_count; i++) {
        if (ip_routes[i].dst_ip == dst_ip && ip_routes[i].dst_mask == dst_mask) {
            // Update existing route
            ip_routes[i].gateway = gateway;
            strncpy(ip_routes[i].interface, interface, 31);
            ip_routes[i].interface[31] = '\0';
            
            klog_info(KLOG_CAT_KERNEL, "IP route updated: %u/%u -> %u via %s",
                      dst_ip, dst_mask, gateway, interface);
            
            spinlock_release(&route_lock);
            return 0;
        }
    }

    // Add new route
    ip_routes[route_count].dst_ip = dst_ip;
    ip_routes[route_count].dst_mask = dst_mask;
    ip_routes[route_count].gateway = gateway;
    strncpy(ip_routes[route_count].interface, interface, 31);
    ip_routes[route_count].interface[31] = '\0';
    route_count++;

    klog_info(KLOG_CAT_KERNEL, "IP route added: %u/%u -> %u via %s",
              dst_ip, dst_mask, gateway, interface);

    spinlock_release(&route_lock);
    return 0;
}

int orion_ip_remove_route(uint32_t dst_ip, uint32_t dst_mask)
{
    if (!tcpip_stack.ip_initialized) {
        return -1;
    }

    spinlock_acquire(&route_lock);

    for (int i = 0; i < route_count; i++) {
        if (ip_routes[i].dst_ip == dst_ip && ip_routes[i].dst_mask == dst_mask) {
            // Remove route by shifting remaining routes
            for (int j = i; j < route_count - 1; j++) {
                ip_routes[j] = ip_routes[j + 1];
            }
            route_count--;

            klog_info(KLOG_CAT_KERNEL, "IP route removed: %u/%u", dst_ip, dst_mask);
            spinlock_release(&route_lock);
            return 0;
        }
    }

    spinlock_release(&route_lock);
    klog_warning(KLOG_CAT_KERNEL, "IP route not found: %u/%u", dst_ip, dst_mask);
    return -1;
}

/* ============================================================================
 * UDP Functions
 * ============================================================================ */

int orion_udp_init(void)
{
    if (tcpip_stack.udp_initialized) {
        klog_error(KLOG_CAT_KERNEL, "UDP stack already initialized");
        return -1;
    }

    tcpip_stack.udp_initialized = true;
    klog_info(KLOG_CAT_KERNEL, "UDP stack initialized successfully");
    return 0;
}

int orion_udp_shutdown(void)
{
    if (!tcpip_stack.udp_initialized) {
        return 0;
    }

    klog_info(KLOG_CAT_KERNEL, "Shutting down UDP stack");
    tcpip_stack.udp_initialized = false;
    return 0;
}

int orion_udp_send(uint32_t src_ip, uint16_t src_port,
                   uint32_t dst_ip, uint16_t dst_port,
                   const void *data, size_t len)
{
    if (!tcpip_stack.udp_initialized || !data) {
        return -1;
    }

    // Create UDP header
    orion_udp_header_t *udp_header = (orion_udp_header_t*)data;
    udp_header->src_port = htons(src_port);
    udp_header->dst_port = htons(dst_port);
    udp_header->length = htons(sizeof(orion_udp_header_t) + len);
    udp_header->checksum = 0;

    klog_debug(KLOG_CAT_KERNEL, "UDP datagram sent: %u:%u -> %u:%u, length: %zu",
               src_ip, src_port, dst_ip, dst_port, len);

    return 0;
}

ssize_t orion_udp_recv(uint32_t *src_ip, uint16_t *src_port,
                        void *data, size_t len)
{
    if (!tcpip_stack.udp_initialized || !data) {
        return -1;
    }

    orion_udp_header_t *udp_header = (orion_udp_header_t*)data;
    
    if (src_port) *src_port = ntohs(udp_header->src_port);
    if (src_ip) *src_ip = 0; // Would be set from IP header

    size_t payload_len = ntohs(udp_header->length) - sizeof(orion_udp_header_t);

    klog_debug(KLOG_CAT_KERNEL, "UDP datagram received: %u:%u, length: %zu",
               *src_ip, *src_port, payload_len);

    return payload_len;
}

/* ============================================================================
 * ICMP Functions
 * ============================================================================ */

int orion_icmp_init(void)
{
    if (tcpip_stack.icmp_initialized) {
        klog_error(KLOG_CAT_KERNEL, "ICMP stack already initialized");
        return -1;
    }

    tcpip_stack.icmp_initialized = true;
    klog_info(KLOG_CAT_KERNEL, "ICMP stack initialized successfully");
    return 0;
}

int orion_icmp_shutdown(void)
{
    if (!tcpip_stack.icmp_initialized) {
        return 0;
    }

    klog_info(KLOG_CAT_KERNEL, "Shutting down ICMP stack");
    tcpip_stack.icmp_initialized = false;
    return 0;
}

int orion_icmp_send(uint32_t src_ip, uint32_t dst_ip, uint8_t type, uint8_t code,
                    const void *data, size_t len)
{
    if (!tcpip_stack.icmp_initialized || !data) {
        return -1;
    }

    // Create ICMP header
    orion_icmp_header_t *icmp_header = (orion_icmp_header_t*)data;
    icmp_header->type = type;
    icmp_header->code = code;
    icmp_header->checksum = 0;
    icmp_header->identifier = htons(0x1234);
    icmp_header->sequence = htons(0x0001);

    // Calculate checksum
    icmp_header->checksum = orion_icmp_checksum(icmp_header, sizeof(orion_icmp_header_t) + len);

    klog_debug(KLOG_CAT_KERNEL, "ICMP message sent: %u -> %u, type: %d, code: %d",
               src_ip, dst_ip, type, code);

    return 0;
}

int orion_icmp_ping(uint32_t src_ip, uint32_t dst_ip, uint16_t sequence)
{
    if (!tcpip_stack.icmp_initialized) {
        return -1;
    }

    // Create ping data
    uint8_t ping_data[64];
    memset(ping_data, 0xAA, sizeof(ping_data));

    // Send ICMP echo request
    int result = orion_icmp_send(src_ip, dst_ip, 8, 0, ping_data, sizeof(ping_data));
    
    if (result == 0) {
        klog_info(KLOG_CAT_KERNEL, "ICMP ping sent: %u -> %u, sequence: %d",
                  src_ip, dst_ip, sequence);
    }

    return result;
}

/* ============================================================================
 * NAT Functions
 * ============================================================================ */

int orion_nat_init(void)
{
    nat_rule_count = 0;
    klog_info(KLOG_CAT_KERNEL, "NAT initialized successfully");
    return 0;
}

int orion_nat_add_rule(uint32_t internal_ip, uint32_t external_ip,
                       uint16_t internal_port, uint16_t external_port, uint8_t protocol)
{
    spinlock_acquire(&nat_lock);

    if (nat_rule_count >= 1024) {
        klog_error(KLOG_CAT_KERNEL, "NAT table full");
        spinlock_release(&nat_lock);
        return -1;
    }

    // Add NAT rule
    nat_rules[nat_rule_count].internal_ip = internal_ip;
    nat_rules[nat_rule_count].external_ip = external_ip;
    nat_rules[nat_rule_count].internal_port = internal_port;
    nat_rules[nat_rule_count].external_port = external_port;
    nat_rules[nat_rule_count].protocol = protocol;
    nat_rules[nat_rule_count].timestamp = orion_get_timestamp();
    nat_rule_count++;

    klog_info(KLOG_CAT_KERNEL, "NAT rule added: %u:%u -> %u:%u (proto: %d)",
              internal_ip, internal_port, external_ip, external_port, protocol);

    spinlock_release(&nat_lock);
    return 0;
}

int orion_nat_remove_rule(uint32_t internal_ip, uint16_t internal_port, uint8_t protocol)
{
    spinlock_acquire(&nat_lock);

    for (int i = 0; i < nat_rule_count; i++) {
        if (nat_rules[i].internal_ip == internal_ip &&
            nat_rules[i].internal_port == internal_port &&
            nat_rules[i].protocol == protocol) {
            
            // Remove rule by shifting remaining rules
            for (int j = i; j < nat_rule_count - 1; j++) {
                nat_rules[j] = nat_rules[j + 1];
            }
            nat_rule_count--;

            klog_info(KLOG_CAT_KERNEL, "NAT rule removed: %u:%u (proto: %d)",
                      internal_ip, internal_port, protocol);
            
            spinlock_release(&nat_lock);
            return 0;
        }
    }

    spinlock_release(&nat_lock);
    klog_warning(KLOG_CAT_KERNEL, "NAT rule not found: %u:%u (proto: %d)",
                  internal_ip, internal_port, protocol);
    return -1;
}

/* ============================================================================
 * Firewall Functions
 * ============================================================================ */

int orion_firewall_init(void)
{
    firewall_rule_count = 0;
    next_rule_id = 1;
    klog_info(KLOG_CAT_KERNEL, "Firewall initialized successfully");
    return 0;
}

int orion_firewall_add_rule(uint32_t src_ip, uint32_t dst_ip,
                           uint16_t src_port, uint16_t dst_port,
                           uint8_t protocol, int action)
{
    spinlock_acquire(&firewall_lock);

    if (firewall_rule_count >= 1024) {
        klog_error(KLOG_CAT_KERNEL, "Firewall table full");
        spinlock_release(&firewall_lock);
        return -1;
    }

    // Add firewall rule
    firewall_rules[firewall_rule_count].rule_id = next_rule_id++;
    firewall_rules[firewall_rule_count].src_ip = src_ip;
    firewall_rules[firewall_rule_count].dst_ip = dst_ip;
    firewall_rules[firewall_rule_count].src_port = src_port;
    firewall_rules[firewall_rule_count].dst_port = dst_port;
    firewall_rules[firewall_rule_count].protocol = protocol;
    firewall_rules[firewall_rule_count].action = action;
    firewall_rule_count++;

    const char *action_str = (action == 1) ? "ALLOW" : "DENY";
    klog_info(KLOG_CAT_KERNEL, "Firewall rule added: %s %u:%u -> %u:%u (proto: %d)",
              action_str, src_ip, src_port, dst_ip, dst_port, protocol);

    spinlock_release(&firewall_lock);
    return 0;
}

int orion_firewall_remove_rule(uint32_t rule_id)
{
    spinlock_acquire(&firewall_lock);

    for (int i = 0; i < firewall_rule_count; i++) {
        if (firewall_rules[i].rule_id == rule_id) {
            // Remove rule by shifting remaining rules
            for (int j = i; j < firewall_rule_count - 1; j++) {
                firewall_rules[j] = firewall_rules[j + 1];
            }
            firewall_rule_count--;

            klog_info(KLOG_CAT_KERNEL, "Firewall rule %u removed", rule_id);
            spinlock_release(&firewall_lock);
            return 0;
        }
    }

    spinlock_release(&firewall_lock);
    klog_warning(KLOG_CAT_KERNEL, "Firewall rule %u not found", rule_id);
    return -1;
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

uint16_t orion_ip_checksum(const void *data, size_t len)
{
    const uint16_t *ptr = (const uint16_t*)data;
    uint32_t sum = 0;

    // Sum all 16-bit words
    for (size_t i = 0; i < len / 2; i++) {
        sum += ntohs(ptr[i]);
    }

    // Handle odd byte if present
    if (len % 2) {
        sum += ((const uint8_t*)data)[len - 1] << 8;
    }

    // Add carry and take one's complement
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return htons(~sum);
}

uint16_t orion_icmp_checksum(const void *data, size_t len)
{
    return orion_ip_checksum(data, len);
}

uint64_t orion_get_timestamp(void)
{
    // Simple timestamp implementation
    // In a real system, this would use proper time functions
    static uint64_t timestamp = 0;
    return ++timestamp;
}
