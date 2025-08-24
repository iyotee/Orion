/*
 * Orion Operating System - Ethernet Driver
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

#ifndef ORION_ETHERNET_DRIVER_H
#define ORION_ETHERNET_DRIVER_H

#include <orion/types.h>
#include <orion/structures.h>
#include "../network_architecture.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* ============================================================================
     * Ethernet Frame Structure
     * ============================================================================ */

    // Ethernet frame header
    typedef struct __attribute__((packed))
    {
        uint8_t dst_mac[6]; // Destination MAC address
        uint8_t src_mac[6]; // Source MAC address
        uint16_t ethertype; // EtherType
    } orion_ethernet_header_t;

// Common EtherTypes
#define ORION_ETHERTYPE_IPV4 0x0800  // IPv4
#define ORION_ETHERTYPE_IPV6 0x86DD  // IPv6
#define ORION_ETHERTYPE_ARP 0x0806   // ARP
#define ORION_ETHERTYPE_RARP 0x8035  // RARP
#define ORION_ETHERTYPE_VLAN 0x8100  // VLAN
#define ORION_ETHERTYPE_MPLS 0x8847  // MPLS
#define ORION_ETHERTYPE_PPPOE 0x8864 // PPPoE
#define ORION_ETHERTYPE_LLDP 0x88CC  // LLDP
#define ORION_ETHERTYPE_8021X 0x888E // 802.1X

/* ============================================================================
 * Ethernet Driver Features
 * ============================================================================ */

// Hardware features
#define ORION_ETH_FEATURE_10BASE_T (1ULL << 0)   // 10 Mbps
#define ORION_ETH_FEATURE_100BASE_T (1ULL << 1)  // 100 Mbps
#define ORION_ETH_FEATURE_1000BASE_T (1ULL << 2) // 1 Gbps
#define ORION_ETH_FEATURE_2_5GBASE_T (1ULL << 3) // 2.5 Gbps
#define ORION_ETH_FEATURE_5GBASE_T (1ULL << 4)   // 5 Gbps
#define ORION_ETH_FEATURE_10GBASE_T (1ULL << 5)  // 10 Gbps
#define ORION_ETH_FEATURE_25GBASE_T (1ULL << 6)  // 25 Gbps
#define ORION_ETH_FEATURE_40GBASE_T (1ULL << 7)  // 40 Gbps
#define ORION_ETH_FEATURE_100GBASE_T (1ULL << 8) // 100 Gbps

// Duplex modes
#define ORION_ETH_DUPLEX_HALF 0x01 // Half duplex
#define ORION_ETH_DUPLEX_FULL 0x02 // Full duplex
#define ORION_ETH_DUPLEX_AUTO 0x04 // Auto-negotiation

// Flow control
#define ORION_ETH_FLOW_CONTROL_NONE 0x00 // No flow control
#define ORION_ETH_FLOW_CONTROL_TX 0x01   // Transmit flow control
#define ORION_ETH_FLOW_CONTROL_RX 0x02   // Receive flow control
#define ORION_ETH_FLOW_CONTROL_BOTH 0x03 // Both directions

    /* ============================================================================
     * Ethernet Statistics
     * ============================================================================ */

    typedef struct
    {
        // Frame statistics
        uint64_t rx_frames; // Received frames
        uint64_t tx_frames; // Transmitted frames
        uint64_t rx_bytes;  // Received bytes
        uint64_t tx_bytes;  // Transmitted bytes

        // Error statistics
        uint64_t rx_errors;           // Receive errors
        uint64_t tx_errors;           // Transmit errors
        uint64_t rx_crc_errors;       // CRC errors
        uint64_t rx_frame_errors;     // Frame errors
        uint64_t rx_fifo_errors;      // FIFO errors
        uint64_t rx_missed_errors;    // Missed errors
        uint64_t tx_carrier_errors;   // Carrier errors
        uint64_t tx_fifo_errors;      // TX FIFO errors
        uint64_t tx_heartbeat_errors; // Heartbeat errors
        uint64_t tx_window_errors;    // Window errors

        // Collision statistics
        uint64_t collisions;           // Collisions
        uint64_t late_collisions;      // Late collisions
        uint64_t excessive_collisions; // Excessive collisions
        uint64_t multiple_collisions;  // Multiple collisions

        // Flow control statistics
        uint64_t rx_pause_frames; // Received pause frames
        uint64_t tx_pause_frames; // Transmitted pause frames

        // Hardware statistics
        uint64_t rx_length_errors; // Length errors
        uint64_t rx_over_errors;   // Overrun errors
        uint64_t rx_noskbuffers;   // No socket buffers
        uint64_t rx_dropped;       // Dropped packets
        uint64_t tx_dropped;       // Dropped TX packets

        // Timestamp statistics
        uint64_t last_rx_time;     // Last receive time
        uint64_t last_tx_time;     // Last transmit time
        uint64_t last_change_time; // Last state change time
    } orion_ethernet_stats_t;

    /* ============================================================================
     * Ethernet Configuration
     * ============================================================================ */

    typedef struct
    {
        char name[32];           // Interface name
        uint8_t mac_addr[6];     // MAC address
        uint32_t mtu;            // Maximum Transmission Unit
        uint32_t speed;          // Interface speed in Mbps
        uint32_t duplex;         // Duplex mode
        uint32_t flow_control;   // Flow control settings
        uint32_t features;       // Supported features
        uint32_t offload_caps;   // Offloading capabilities
        uint32_t max_frame_size; // Maximum frame size
        uint32_t min_frame_size; // Minimum frame size
        uint32_t rx_buffer_size; // Receive buffer size
        uint32_t tx_buffer_size; // Transmit buffer size
        uint32_t rx_queue_size;  // Receive queue size
        uint32_t tx_queue_size;  // Transmit queue size
    } orion_ethernet_config_t;

    /* ============================================================================
     * Ethernet Driver Operations
     * ============================================================================ */

    typedef struct orion_ethernet_driver
    {
        const char *name;      // Driver name
        const char *version;   // Driver version
        uint32_t features;     // Driver features
        uint32_t offload_caps; // Offloading capabilities

        // Driver operations
        int (*probe)(struct orion_ethernet_driver *driver, void *device);
        int (*remove)(struct orion_ethernet_driver *driver, void *device);
        int (*open)(struct orion_ethernet_driver *driver);
        int (*close)(struct orion_ethernet_driver *driver);
        int (*start)(struct orion_ethernet_driver *driver);
        int (*stop)(struct orion_ethernet_driver *driver);
        int (*reset)(struct orion_ethernet_driver *driver);

        // Configuration operations
        int (*get_config)(struct orion_ethernet_driver *driver, orion_ethernet_config_t *config);
        int (*set_config)(struct orion_ethernet_driver *driver, const orion_ethernet_config_t *config);

        // Statistics operations
        int (*get_stats)(struct orion_ethernet_driver *driver, orion_ethernet_stats_t *stats);
        int (*reset_stats)(struct orion_ethernet_driver *driver);

        // Packet operations
        int (*transmit)(struct orion_ethernet_driver *driver, const void *packet, size_t len);
        int (*receive)(struct orion_ethernet_driver *driver, void *packet, size_t len);

        // Hardware operations
        int (*set_mac_address)(struct orion_ethernet_driver *driver, const uint8_t *mac);
        int (*get_mac_address)(struct orion_ethernet_driver *driver, uint8_t *mac);
        int (*set_speed)(struct orion_ethernet_driver *driver, uint32_t speed);
        int (*get_speed)(struct orion_ethernet_driver *driver, uint32_t *speed);
        int (*set_duplex)(struct orion_ethernet_driver *driver, uint32_t duplex);
        int (*get_duplex)(struct orion_ethernet_driver *driver, uint32_t *duplex);
        int (*set_flow_control)(struct orion_ethernet_driver *driver, uint32_t flow_control);
        int (*get_flow_control)(struct orion_ethernet_driver *driver, uint32_t *flow_control);

        // Power management
        int (*suspend)(struct orion_ethernet_driver *driver);
        int (*resume)(struct orion_ethernet_driver *driver);
        int (*set_power_state)(struct orion_ethernet_driver *driver, uint32_t power_state);

        // Interrupt handling
        int (*enable_interrupts)(struct orion_ethernet_driver *driver);
        int (*disable_interrupts)(struct orion_ethernet_driver *driver);
        int (*set_interrupt_mode)(struct orion_ethernet_driver *driver, uint32_t mode);

        // DMA operations
        int (*setup_dma)(struct orion_ethernet_driver *driver);
        int (*cleanup_dma)(struct orion_ethernet_driver *driver);
        int (*alloc_dma_buffer)(struct orion_ethernet_driver *driver, size_t size, void **buffer, uint64_t *dma_addr);
        int (*free_dma_buffer)(struct orion_ethernet_driver *driver, void *buffer, uint64_t dma_addr);

        // Private data
        void *private_data;
    } orion_ethernet_driver_t;

    /* ============================================================================
     * Ethernet Driver Functions
     * ============================================================================ */

    /**
     * @brief Register Ethernet driver
     * @param driver Ethernet driver structure
     * @return 0 on success, negative value on error
     */
    int orion_ethernet_register_driver(orion_ethernet_driver_t *driver);

    /**
     * @brief Unregister Ethernet driver
     * @param driver Ethernet driver structure
     * @return 0 on success, negative value on error
     */
    int orion_ethernet_unregister_driver(orion_ethernet_driver_t *driver);

    /**
     * @brief Get Ethernet interface by name
     * @param name Interface name
     * @return Interface configuration or NULL if not found
     */
    orion_ethernet_config_t *orion_ethernet_get_interface(const char *name);

    /**
     * @brief Get all Ethernet interfaces
     * @param interfaces Output array of interfaces
     * @param max_count Maximum number of interfaces to return
     * @return Number of interfaces found
     */
    int orion_ethernet_get_interfaces(orion_ethernet_config_t *interfaces, int max_count);

    /**
     * @brief Configure Ethernet interface
     * @param name Interface name
     * @param config Interface configuration
     * @return 0 on success, negative value on error
     */
    int orion_ethernet_configure_interface(const char *name, const orion_ethernet_config_t *config);

    /**
     * @brief Get Ethernet interface statistics
     * @param name Interface name
     * @param stats Output statistics structure
     * @return 0 on success, negative value on error
     */
    int orion_ethernet_get_stats(const char *name, orion_ethernet_stats_t *stats);

    /**
     * @brief Reset Ethernet interface statistics
     * @param name Interface name
     * @return 0 on success, negative value on error
     */
    int orion_ethernet_reset_stats(const char *name);

    /* ============================================================================
     * Ethernet Packet Functions
     * ============================================================================ */

    /**
     * @brief Create Ethernet frame
     * @param dst_mac Destination MAC address
     * @param src_mac Source MAC address
     * @param ethertype EtherType
     * @param payload Payload data
     * @param payload_len Payload length
     * @param frame Output frame buffer
     * @param frame_len Frame buffer length
     * @return 0 on success, negative value on error
     */
    int orion_ethernet_create_frame(const uint8_t *dst_mac, const uint8_t *src_mac,
                                    uint16_t ethertype, const void *payload, size_t payload_len,
                                    void *frame, size_t frame_len);

    /**
     * @brief Parse Ethernet frame
     * @param frame Frame data
     * @param frame_len Frame length
     * @param header Output header structure
     * @param payload Output payload pointer
     * @param payload_len Output payload length
     * @return 0 on success, negative value on error
     */
    int orion_ethernet_parse_frame(const void *frame, size_t frame_len,
                                   orion_ethernet_header_t *header,
                                   const void **payload, size_t *payload_len);

    /**
     * @brief Validate Ethernet frame
     * @param frame Frame data
     * @param frame_len Frame length
     * @return 0 if valid, negative value if invalid
     */
    int orion_ethernet_validate_frame(const void *frame, size_t frame_len);

    /**
     * @brief Calculate Ethernet frame checksum
     * @param frame Frame data
     * @param frame_len Frame length
     * @return Checksum value
     */
    uint32_t orion_ethernet_calculate_checksum(const void *frame, size_t frame_len);

    /* ============================================================================
     * Ethernet Utility Functions
     * ============================================================================ */

    /**
     * @brief Convert MAC address to string
     * @param mac MAC address
     * @param str Output string buffer
     * @param str_len String buffer length
     * @return 0 on success, negative value on error
     */
    int orion_ethernet_mac_to_string(const uint8_t *mac, char *str, size_t str_len);

    /**
     * @brief Convert string to MAC address
     * @param str MAC address string
     * @param mac Output MAC address
     * @return 0 on success, negative value on error
     */
    int orion_ethernet_string_to_mac(const char *str, uint8_t *mac);

    /**
     * @brief Check if MAC address is valid
     * @param mac MAC address
     * @return true if valid, false otherwise
     */
    bool orion_ethernet_is_valid_mac(const uint8_t *mac);

    /**
     * @brief Check if MAC address is broadcast
     * @param mac MAC address
     * @return true if broadcast, false otherwise
     */
    bool orion_ethernet_is_broadcast_mac(const uint8_t *mac);

    /**
     * @brief Check if MAC address is multicast
     * @param mac MAC address
     * @return true if multicast, false otherwise
     */
    bool orion_ethernet_is_multicast_mac(const uint8_t *mac);

    /**
     * @brief Check if MAC address is locally administered
     * @param mac MAC address
     * @return true if locally administered, false otherwise
     */
    bool orion_ethernet_is_locally_administered_mac(const uint8_t *mac);

    /**
     * @brief Generate random MAC address
     * @param mac Output MAC address
     * @param locally_administered Generate locally administered address
     * @return 0 on success, negative value on error
     */
    int orion_ethernet_generate_random_mac(uint8_t *mac, bool locally_administered);

#ifdef __cplusplus
}
#endif

#endif // ORION_ETHERNET_DRIVER_H
