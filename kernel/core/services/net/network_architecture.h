/*
 * Orion Operating System - Network Architecture
 *
 * Ultra-modern network stack with high-performance networking
 * supporting IPv4/IPv6, TCP/UDP, advanced protocols, and hardware offloading
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_NETWORK_ARCHITECTURE_H
#define ORION_NETWORK_ARCHITECTURE_H

#include <orion/types.h>
#include <orion/structures.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* ============================================================================
 * Network Architecture Overview
 * ============================================================================
 *
 * ORION Network Stack Architecture:
 * ┌─────────────────────────────────────────────────────────────┐
 * │                    Application Layer                        │
 * │  HTTP/HTTPS, FTP, SMTP, DNS, WebSocket, gRPC, etc.        │
 * └─────────────────────────────────────────────────────────────┘
 * ┌─────────────────────────────────────────────────────────────┐
 * │                    Transport Layer                          │
 * │  TCP (with congestion control), UDP, SCTP, QUIC           │
 * └─────────────────────────────────────────────────────────────┘
 * ┌─────────────────────────────────────────────────────────────┐
 * │                    Network Layer                           │
 * │  IPv4/IPv6, ICMP, routing, NAT, firewall, QoS             │
 * └─────────────────────────────────────────────────────────────┘
 * ┌─────────────────────────────────────────────────────────────┐
 * │                    Data Link Layer                         │
 * │  Ethernet, WiFi, cellular, driver abstraction             │
 * └─────────────────────────────────────────────────────────────┘
 * ┌─────────────────────────────────────────────────────────────┘
 * │                    Physical Layer                           │
 * │  Hardware drivers, DMA, interrupts, offloading            │
 * └─────────────────────────────────────────────────────────────┘
 */

/* ============================================================================
 * Network Protocol Support
 * ============================================================================ */

// Transport protocols
#define ORION_NET_PROTO_TCP 1
#define ORION_NET_PROTO_UDP 2
#define ORION_NET_PROTO_SCTP 3
#define ORION_NET_PROTO_QUIC 4
#define ORION_NET_PROTO_ICMP 5
#define ORION_NET_PROTO_IGMP 6

// Network protocols
#define ORION_NET_PROTO_IPV4 1
#define ORION_NET_PROTO_IPV6 2
#define ORION_NET_PROTO_ARP 3
#define ORION_NET_PROTO_RARP 4

// Application protocols
#define ORION_NET_PROTO_HTTP 80
#define ORION_NET_PROTO_HTTPS 443
#define ORION_NET_PROTO_FTP 21
#define ORION_NET_PROTO_SMTP 25
#define ORION_NET_PROTO_DNS 53
#define ORION_NET_PROTO_DHCP 67
#define ORION_NET_PROTO_NTP 123

/* ============================================================================
 * Network Features
 * ============================================================================ */

// Advanced networking features
#define ORION_NET_FEATURE_IPSEC (1ULL << 0)         // IP Security
#define ORION_NET_FEATURE_VPN (1ULL << 1)           // Virtual Private Network
#define ORION_NET_FEATURE_LOAD_BALANCER (1ULL << 2) // Load Balancing
#define ORION_NET_FEATURE_PROXY (1ULL << 3)         // Proxy/Reverse Proxy
#define ORION_NET_FEATURE_CACHE (1ULL << 4)         // Network Caching
#define ORION_NET_FEATURE_CDN (1ULL << 5)           // Content Delivery Network
#define ORION_NET_FEATURE_DDOS_PROTECT (1ULL << 6)  // DDoS Protection
#define ORION_NET_FEATURE_QOS (1ULL << 7)           // Quality of Service
#define ORION_NET_FEATURE_VLAN (1ULL << 8)          // Virtual LAN
#define ORION_NET_FEATURE_BONDING (1ULL << 9)       // Network Bonding
#define ORION_NET_FEATURE_BRIDGING (1ULL << 10)     // Network Bridging
#define ORION_NET_FEATURE_ROUTING (1ULL << 11)      // Advanced Routing
#define ORION_NET_FEATURE_NAT (1ULL << 12)          // Network Address Translation
#define ORION_NET_FEATURE_FIREWALL (1ULL << 13)     // Stateful Firewall
#define ORION_NET_FEATURE_IDS (1ULL << 14)          // Intrusion Detection
#define ORION_NET_FEATURE_IPS (1ULL << 15)          // Intrusion Prevention

/* ============================================================================
 * Network Performance Features
 * ============================================================================ */

// Hardware offloading capabilities
#define ORION_NET_OFFLOAD_TCP_CSUM (1ULL << 0)    // TCP Checksum
#define ORION_NET_OFFLOAD_UDP_CSUM (1ULL << 1)    // UDP Checksum
#define ORION_NET_OFFLOAD_IP_CSUM (1ULL << 2)     // IP Checksum
#define ORION_NET_OFFLOAD_TCP_SEGMENT (1ULL << 3) // TCP Segmentation
#define ORION_NET_OFFLOAD_TCP_LRO (1ULL << 4)     // Large Receive Offload
#define ORION_NET_OFFLOAD_TCP_GRO (1ULL << 5)     // Generic Receive Offload
#define ORION_NET_OFFLOAD_TCP_TSO (1ULL << 6)     // TCP Segmentation Offload
#define ORION_NET_OFFLOAD_RSS (1ULL << 7)         // Receive Side Scaling
#define ORION_NET_OFFLOAD_RPS (1ULL << 8)         // Receive Packet Steering
#define ORION_NET_OFFLOAD_XPS (1ULL << 9)         // Transmit Packet Steering
#define ORION_NET_OFFLOAD_DMA (1ULL << 10)        // Direct Memory Access
#define ORION_NET_OFFLOAD_IRQ_COAL (1ULL << 11)   // Interrupt Coalescing

/* ============================================================================
 * Network Security Features
 * ============================================================================ */

// Security protocols and features
#define ORION_NET_SECURITY_TLS_1_3 (1ULL << 0)   // TLS 1.3
#define ORION_NET_SECURITY_TLS_1_2 (1ULL << 1)   // TLS 1.2
#define ORION_NET_SECURITY_DTLS (1ULL << 2)      // Datagram TLS
#define ORION_NET_SECURITY_QUIC (1ULL << 3)      // QUIC Protocol
#define ORION_NET_SECURITY_IPSEC (1ULL << 4)     // IP Security
#define ORION_NET_SECURITY_WIREGUARD (1ULL << 5) // WireGuard VPN
#define ORION_NET_SECURITY_OPENVPN (1ULL << 6)   // OpenVPN
#define ORION_NET_SECURITY_L2TP (1ULL << 7)      // L2TP
#define ORION_NET_SECURITY_PPTP (1ULL << 8)      // PPTP
#define ORION_NET_SECURITY_SSTP (1ULL << 9)      // SSTP

    /* ============================================================================
     * Network Interface Types
     * ============================================================================ */

    typedef enum
    {
        ORION_NET_IFACE_ETHERNET = 1, // Wired Ethernet
        ORION_NET_IFACE_WIFI,         // Wireless WiFi
        ORION_NET_IFACE_CELLULAR,     // Cellular (4G/5G)
        ORION_NET_IFACE_BLUETOOTH,    // Bluetooth
        ORION_NET_IFACE_VIRTUAL,      // Virtual interface
        ORION_NET_IFACE_TUNNEL,       // Tunnel interface
        ORION_NET_IFACE_BRIDGE,       // Bridge interface
        ORION_NET_IFACE_BOND,         // Bonded interface
        ORION_NET_IFACE_VLAN,         // VLAN interface
        ORION_NET_IFACE_LOOPBACK      // Loopback interface
    } orion_net_iface_type_t;

    /* ============================================================================
     * Network Interface States
     * ============================================================================ */

    typedef enum
    {
        ORION_NET_IFACE_DOWN = 0, // Interface down
        ORION_NET_IFACE_UP,       // Interface up
        ORION_NET_IFACE_RUNNING,  // Interface running
        ORION_NET_IFACE_ERROR,    // Interface error
        ORION_NET_IFACE_DISABLED, // Interface disabled
        ORION_NET_IFACE_SLEEPING, // Interface sleeping (power save)
        ORION_NET_IFACE_SUSPENDED // Interface suspended
    } orion_net_iface_state_t;

    /* ============================================================================
     * Network Interface Features
     * ============================================================================ */

    typedef struct
    {
        uint64_t speed;   // Interface speed in Mbps
        uint64_t duplex;  // Duplex mode (half/full)
        uint64_t autoneg; // Auto-negotiation support
        uint64_t pause;   // Flow control support
        uint64_t wol;     // Wake-on-LAN support
        uint64_t eee;     // Energy Efficient Ethernet
        uint64_t fcoe;    // Fibre Channel over Ethernet
        uint64_t iscsi;   // iSCSI support
        uint64_t rdma;    // RDMA support
        uint64_t sr_iov;  // Single Root I/O Virtualization
        uint64_t macsec;  // MAC Security
        uint64_t ptp;     // Precision Time Protocol
    } orion_net_iface_features_t;

    /* ============================================================================
     * Network Statistics
     * ============================================================================ */

    typedef struct
    {
        // Packet statistics
        uint64_t rx_packets; // Received packets
        uint64_t tx_packets; // Transmitted packets
        uint64_t rx_bytes;   // Received bytes
        uint64_t tx_bytes;   // Transmitted bytes
        uint64_t rx_errors;  // Receive errors
        uint64_t tx_errors;  // Transmit errors
        uint64_t rx_dropped; // Dropped receive packets
        uint64_t tx_dropped; // Dropped transmit packets

        // Protocol statistics
        uint64_t tcp_segments;  // TCP segments
        uint64_t udp_datagrams; // UDP datagrams
        uint64_t icmp_messages; // ICMP messages

        // Error statistics
        uint64_t rx_fifo_errors;    // Receive FIFO errors
        uint64_t tx_fifo_errors;    // Transmit FIFO errors
        uint64_t rx_frame_errors;   // Receive frame errors
        uint64_t tx_carrier_errors; // Transmit carrier errors
        uint64_t rx_crc_errors;     // Receive CRC errors
        uint64_t rx_length_errors;  // Receive length errors
        uint64_t rx_missed_errors;  // Receive missed errors

        // Performance statistics
        uint64_t rx_compressed; // Compressed receive packets
        uint64_t tx_compressed; // Compressed transmit packets
        uint64_t collisions;    // Collisions
        uint64_t rx_multicast;  // Multicast receive packets
        uint64_t tx_multicast;  // Multicast transmit packets

        // Timestamp statistics
        uint64_t last_rx_time;     // Last receive time
        uint64_t last_tx_time;     // Last transmit time
        uint64_t last_change_time; // Last state change time
    } orion_net_iface_stats_t;

    /* ============================================================================
     * Network Configuration
     * ============================================================================ */

    typedef struct
    {
        char name[32];                          // Interface name
        orion_net_iface_type_t type;            // Interface type
        orion_net_iface_state_t state;          // Interface state
        uint8_t mac_addr[6];                    // MAC address
        uint32_t mtu;                           // Maximum Transmission Unit
        uint32_t flags;                         // Interface flags
        uint32_t features;                      // Interface features
        uint32_t offload_caps;                  // Offloading capabilities
        uint32_t security_caps;                 // Security capabilities
        orion_net_iface_features_t hw_features; // Hardware features
        orion_net_iface_stats_t stats;          // Interface statistics
    } orion_net_iface_config_t;

    /* ============================================================================
     * Network Driver Interface
     * ============================================================================ */

    typedef struct orion_net_driver
    {
        const char *name;      // Driver name
        const char *version;   // Driver version
        uint32_t features;     // Driver features
        uint32_t offload_caps; // Offloading capabilities

        // Driver operations
        int (*probe)(struct orion_net_driver *driver, void *device);
        int (*remove)(struct orion_net_driver *driver, void *device);
        int (*open)(struct orion_net_driver *driver);
        int (*close)(struct orion_net_driver *driver);
        int (*start)(struct orion_net_driver *driver);
        int (*stop)(struct orion_net_driver *driver);
        int (*reset)(struct orion_net_driver *driver);

        // Packet operations
        int (*transmit)(struct orion_net_driver *driver, void *packet, size_t len);
        int (*receive)(struct orion_net_driver *driver, void *packet, size_t len);

        // Configuration operations
        int (*get_config)(struct orion_net_driver *driver, orion_net_iface_config_t *config);
        int (*set_config)(struct orion_net_driver *driver, const orion_net_iface_config_t *config);

        // Statistics operations
        int (*get_stats)(struct orion_net_driver *driver, orion_net_iface_stats_t *stats);
        int (*reset_stats)(struct orion_net_driver *driver);

        // Power management
        int (*suspend)(struct orion_net_driver *driver);
        int (*resume)(struct orion_net_driver *driver);

        // Private data
        void *private_data;
    } orion_net_driver_t;

    /* ============================================================================
     * Network Stack Configuration
     * ============================================================================ */

    typedef struct
    {
        uint32_t max_interfaces;    // Maximum network interfaces
        uint32_t max_connections;   // Maximum network connections
        uint32_t max_sockets;       // Maximum network sockets
        uint32_t buffer_size;       // Network buffer size
        uint32_t queue_size;        // Network queue size
        uint32_t timeout_ms;        // Network timeout in milliseconds
        uint32_t retry_count;       // Network retry count
        uint32_t features;          // Enabled network features
        uint32_t security_level;    // Network security level
        uint32_t performance_level; // Network performance level
    } orion_net_stack_config_t;

    /* ============================================================================
     * Network Stack Functions
     * ============================================================================ */

    /**
     * @brief Initialize the network stack
     * @param config Network stack configuration
     * @return 0 on success, negative value on error
     */
    int orion_net_stack_init(const orion_net_stack_config_t *config);

    /**
     * @brief Shutdown the network stack
     * @return 0 on success, negative value on error
     */
    int orion_net_stack_shutdown(void);

    /**
     * @brief Get network stack configuration
     * @param config Output configuration structure
     * @return 0 on success, negative value on error
     */
    int orion_net_stack_get_config(orion_net_stack_config_t *config);

    /**
     * @brief Set network stack configuration
     * @param config Input configuration structure
     * @return 0 on success, negative value on error
     */
    int orion_net_stack_set_config(const orion_net_stack_config_t *config);

    /**
     * @brief Get network stack status
     * @return Network stack status
     */
    int orion_net_stack_get_status(void);

    /**
     * @brief Get network stack statistics
     * @param stats Output statistics structure
     * @return 0 on success, negative value on error
     */
    int orion_net_stack_get_stats(void *stats);

    /* ============================================================================
     * Network Interface Management
     * ============================================================================ */

    /**
     * @brief Register a network driver
     * @param driver Network driver structure
     * @return 0 on success, negative value on error
     */
    int orion_net_register_driver(orion_net_driver_t *driver);

    /**
     * @brief Unregister a network driver
     * @param driver Network driver structure
     * @return 0 on success, negative value on error
     */
    int orion_net_unregister_driver(orion_net_driver_t *driver);

    /**
     * @brief Get network interface by name
     * @param name Interface name
     * @return Interface configuration or NULL if not found
     */
    orion_net_iface_config_t *orion_net_get_interface(const char *name);

    /**
     * @brief Get all network interfaces
     * @param interfaces Output array of interfaces
     * @param max_count Maximum number of interfaces to return
     * @return Number of interfaces found
     */
    int orion_net_get_interfaces(orion_net_iface_config_t *interfaces, int max_count);

    /**
     * @brief Configure network interface
     * @param name Interface name
     * @param config Interface configuration
     * @return 0 on success, negative value on error
     */
    int orion_net_configure_interface(const char *name, const orion_net_iface_config_t *config);

    /* ============================================================================
     * Network Performance Functions
     * ============================================================================ */

    /**
     * @brief Enable network feature
     * @param feature Feature to enable
     * @return 0 on success, negative value on error
     */
    int orion_net_enable_feature(uint64_t feature);

    /**
     * @brief Disable network feature
     * @param feature Feature to disable
     * @return 0 on success, negative value on error
     */
    int orion_net_disable_feature(uint64_t feature);

    /**
     * @brief Check if network feature is enabled
     * @param feature Feature to check
     * @return true if enabled, false otherwise
     */
    bool orion_net_is_feature_enabled(uint64_t feature);

    /**
     * @brief Get network performance metrics
     * @param metrics Output metrics structure
     * @return 0 on success, negative value on error
     */
    int orion_net_get_performance_metrics(void *metrics);

    /* ============================================================================
     * Network Security Functions
     * ============================================================================ */

    /**
     * @brief Enable network security feature
     * @param security Security feature to enable
     * @return 0 on success, negative value on error
     */
    int orion_net_enable_security(uint64_t security);

    /**
     * @brief Disable network security feature
     * @param security Security feature to disable
     * @return 0 on success, negative value on error
     */
    int orion_net_disable_security(uint64_t security);

    /**
     * @brief Check if network security feature is enabled
     * @param security Security feature to check
     * @return true if enabled, false otherwise
     */
    bool orion_net_is_security_enabled(uint64_t security);

    /**
     * @brief Get network security status
     * @param status Output security status
     * @return 0 on success, negative value on error
     */
    int orion_net_get_security_status(void *status);

#ifdef __cplusplus
}
#endif

#endif // ORION_NETWORK_ARCHITECTURE_H
