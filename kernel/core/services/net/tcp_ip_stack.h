/*
 * Orion Operating System - TCP/IP Stack
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

#ifndef ORION_TCP_IP_STACK_H
#define ORION_TCP_IP_STACK_H

#include <orion/types.h>
#include <orion/structures.h>
#include "network_architecture.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* ============================================================================
     * TCP/IP Protocol Headers
     * ============================================================================ */

    // IPv4 header structure
    typedef struct __attribute__((packed))
    {
        uint8_t version_ihl;     // Version (4 bits) + IHL (4 bits)
        uint8_t tos;             // Type of Service
        uint16_t total_length;   // Total length
        uint16_t identification; // Identification
        uint16_t flags_offset;   // Flags (3 bits) + Fragment Offset (13 bits)
        uint8_t ttl;             // Time to Live
        uint8_t protocol;        // Protocol
        uint16_t checksum;       // Header checksum
        uint32_t src_addr;       // Source address
        uint32_t dst_addr;       // Destination address
    } orion_ipv4_header_t;

    // IPv6 header structure
    typedef struct __attribute__((packed))
    {
        uint32_t version_class_flow; // Version (4 bits) + Class (8 bits) + Flow Label (20 bits)
        uint16_t payload_length;     // Payload length
        uint8_t next_header;         // Next header
        uint8_t hop_limit;           // Hop limit
        uint8_t src_addr[16];        // Source address
        uint8_t dst_addr[16];        // Destination address
    } orion_ipv6_header_t;

    // TCP header structure
    typedef struct __attribute__((packed))
    {
        uint16_t src_port;            // Source port
        uint16_t dst_port;            // Destination port
        uint32_t seq_num;             // Sequence number
        uint32_t ack_num;             // Acknowledgment number
        uint8_t data_offset_reserved; // Data offset (4 bits) + Reserved (4 bits)
        uint8_t flags;                // Flags (6 bits) + Reserved (2 bits)
        uint16_t window_size;         // Window size
        uint16_t checksum;            // Checksum
        uint16_t urgent_ptr;          // Urgent pointer
    } orion_tcp_header_t;

    // UDP header structure
    typedef struct __attribute__((packed))
    {
        uint16_t src_port; // Source port
        uint16_t dst_port; // Destination port
        uint16_t length;   // Length
        uint16_t checksum; // Checksum
    } orion_udp_header_t;

    // ICMP header structure
    typedef struct __attribute__((packed))
    {
        uint8_t type;        // Type
        uint8_t code;        // Code
        uint16_t checksum;   // Checksum
        uint16_t identifier; // Identifier
        uint16_t sequence;   // Sequence number
    } orion_icmp_header_t;

    /* ============================================================================
     * TCP Flags
     * ============================================================================ */

#define ORION_TCP_FLAG_FIN 0x01 // Finish
#define ORION_TCP_FLAG_SYN 0x02 // Synchronize
#define ORION_TCP_FLAG_RST 0x04 // Reset
#define ORION_TCP_FLAG_PSH 0x08 // Push
#define ORION_TCP_FLAG_ACK 0x10 // Acknowledgment
#define ORION_TCP_FLAG_URG 0x20 // Urgent
#define ORION_TCP_FLAG_ECE 0x40 // Explicit Congestion Notification Echo
#define ORION_TCP_FLAG_CWR 0x80 // Congestion Window Reduced

    /* ============================================================================
     * TCP States
     * ============================================================================ */

    typedef enum
    {
        ORION_TCP_STATE_CLOSED = 0,   // Connection closed
        ORION_TCP_STATE_LISTEN,       // Listening for connections
        ORION_TCP_STATE_SYN_SENT,     // SYN sent, waiting for SYN+ACK
        ORION_TCP_STATE_SYN_RECEIVED, // SYN received, waiting for ACK
        ORION_TCP_STATE_ESTABLISHED,  // Connection established
        ORION_TCP_STATE_FIN_WAIT_1,   // FIN sent, waiting for ACK
        ORION_TCP_STATE_FIN_WAIT_2,   // FIN sent, ACK received, waiting for FIN
        ORION_TCP_STATE_CLOSE_WAIT,   // FIN received, waiting for close
        ORION_TCP_STATE_CLOSING,      // FIN sent, FIN received, waiting for ACK
        ORION_TCP_STATE_LAST_ACK,     // FIN sent, waiting for ACK
        ORION_TCP_STATE_TIME_WAIT,    // Time wait
        ORION_TCP_STATE_CLOSE         // Connection closing
    } orion_tcp_state_t;

    /* ============================================================================
     * TCP Connection Structure
     * ============================================================================ */

    typedef struct orion_tcp_connection
    {
        // Connection identifiers
        uint32_t local_ip;    // Local IP address
        uint32_t remote_ip;   // Remote IP address
        uint16_t local_port;  // Local port
        uint16_t remote_port; // Remote port

        // TCP state
        orion_tcp_state_t state; // Current TCP state
        uint32_t seq_num;        // Sequence number
        uint32_t ack_num;        // Acknowledgment number

        // Flow control
        uint16_t window_size;   // Receive window size
        uint16_t remote_window; // Remote window size
        uint32_t ssthresh;      // Slow start threshold
        uint32_t cwnd;          // Congestion window

        // Timers
        uint64_t rtt;            // Round trip time
        uint64_t rto;            // Retransmission timeout
        uint64_t last_ack_time;  // Last acknowledgment time
        uint64_t last_data_time; // Last data time

        // Buffers
        void *send_buffer;       // Send buffer
        void *recv_buffer;       // Receive buffer
        size_t send_buffer_size; // Send buffer size
        size_t recv_buffer_size; // Receive buffer size
        size_t send_buffer_used; // Send buffer used
        size_t recv_buffer_used; // Receive buffer used

        // Congestion control
        uint32_t snd_una; // Send unacknowledged
        uint32_t snd_nxt; // Send next
        uint32_t snd_wnd; // Send window
        uint32_t rcv_nxt; // Receive next
        uint32_t rcv_wnd; // Receive window

        // Statistics
        uint64_t bytes_sent;       // Bytes sent
        uint64_t bytes_received;   // Bytes received
        uint64_t packets_sent;     // Packets sent
        uint64_t packets_received; // Packets received
        uint64_t retransmissions;  // Retransmissions
        uint64_t timeouts;         // Timeouts

        // Next connection in list
        struct orion_tcp_connection *next;
    } orion_tcp_connection_t;

    /* ============================================================================
     * TCP Options
     * ============================================================================ */

    typedef struct
    {
        uint8_t mss;            // Maximum Segment Size
        uint8_t window_scale;   // Window Scale
        uint8_t sack_permitted; // SACK Permitted
        uint8_t timestamp;      // Timestamp
        uint8_t nop;            // No Operation
        uint8_t end;            // End of Options
    } orion_tcp_options_t;

    /* ============================================================================
     * TCP Congestion Control Algorithms
     * ============================================================================ */

    typedef enum
    {
        ORION_TCP_CC_RENO = 0, // Reno algorithm
        ORION_TCP_CC_NEW_RENO, // New Reno algorithm
        ORION_TCP_CC_CUBIC,    // CUBIC algorithm
        ORION_TCP_CC_BBR,      // BBR algorithm
        ORION_TCP_CC_VEGAS,    // Vegas algorithm
        ORION_TCP_CC_WESTWOOD, // Westwood algorithm
        ORION_TCP_CC_HTCP,     // HTCP algorithm
        ORION_TCP_CC_SCALABLE, // Scalable algorithm
        ORION_TCP_CC_HSTCP,    // High Speed TCP
        ORION_TCP_CC_BIC,      // BIC algorithm
        ORION_TCP_CC_YEAH      // YeAH algorithm
    } orion_tcp_cc_algorithm_t;

    /* ============================================================================
     * TCP Configuration
     * ============================================================================ */

    typedef struct
    {
        uint32_t max_connections;        // Maximum TCP connections
        uint32_t max_syn_backlog;        // Maximum SYN backlog
        uint32_t max_orphans;            // Maximum orphaned connections
        uint32_t tcp_fin_timeout;        // TCP FIN timeout
        uint32_t tcp_keepalive_time;     // TCP keepalive time
        uint32_t tcp_keepalive_probes;   // TCP keepalive probes
        uint32_t tcp_keepalive_intvl;    // TCP keepalive interval
        uint32_t tcp_max_syn_backlog;    // TCP max SYN backlog
        uint32_t tcp_max_tw_buckets;     // TCP max TIME_WAIT buckets
        uint32_t tcp_tw_reuse;           // TCP TIME_WAIT reuse
        uint32_t tcp_window_scaling;     // TCP window scaling
        uint32_t tcp_timestamps;         // TCP timestamps
        uint32_t tcp_sack;               // TCP SACK
        uint32_t tcp_fack;               // TCP FACK
        uint32_t tcp_ecn;                // TCP ECN
        uint32_t tcp_abc;                // TCP ABC
        uint32_t tcp_congestion_control; // TCP congestion control algorithm
    } orion_tcp_config_t;

    /* ============================================================================
     * TCP Functions
     * ============================================================================ */

    /**
     * @brief Initialize TCP stack
     * @param config TCP configuration
     * @return 0 on success, negative value on error
     */
    int orion_tcp_init(const orion_tcp_config_t *config);

    /**
     * @brief Shutdown TCP stack
     * @return 0 on success, negative value on error
     */
    int orion_tcp_shutdown(void);

    /**
     * @brief Create TCP connection
     * @param local_ip Local IP address
     * @param local_port Local port
     * @param remote_ip Remote IP address
     * @param remote_port Remote port
     * @return TCP connection or NULL on error
     */
    orion_tcp_connection_t *orion_tcp_connect(uint32_t local_ip, uint16_t local_port,
                                              uint32_t remote_ip, uint16_t remote_port);

    /**
     * @brief Accept TCP connection
     * @param listener Listener connection
     * @return Accepted connection or NULL on error
     */
    orion_tcp_connection_t *orion_tcp_accept(orion_tcp_connection_t *listener);

    /**
     * @brief Listen for TCP connections
     * @param local_ip Local IP address
     * @param local_port Local port
     * @param backlog Connection backlog
     * @return Listener connection or NULL on error
     */
    orion_tcp_connection_t *orion_tcp_listen(uint32_t local_ip, uint16_t local_port, int backlog);

    /**
     * @brief Send data over TCP connection
     * @param conn TCP connection
     * @param data Data to send
     * @param len Data length
     * @return Number of bytes sent or negative value on error
     */
    ssize_t orion_tcp_send(orion_tcp_connection_t *conn, const void *data, size_t len);

    /**
     * @brief Receive data from TCP connection
     * @param conn TCP connection
     * @param data Data buffer
     * @param len Buffer length
     * @return Number of bytes received or negative value on error
     */
    ssize_t orion_tcp_recv(orion_tcp_connection_t *conn, void *data, size_t len);

    /**
     * @brief Close TCP connection
     * @param conn TCP connection
     * @return 0 on success, negative value on error
     */
    int orion_tcp_close(orion_tcp_connection_t *conn);

    /**
     * @brief Get TCP connection state
     * @param conn TCP connection
     * @return TCP state
     */
    orion_tcp_state_t orion_tcp_get_state(orion_tcp_connection_t *conn);

    /**
     * @brief Set TCP congestion control algorithm
     * @param conn TCP connection
     * @param algorithm Congestion control algorithm
     * @return 0 on success, negative value on error
     */
    int orion_tcp_set_congestion_control(orion_tcp_connection_t *conn,
                                         orion_tcp_cc_algorithm_t algorithm);

    /* ============================================================================
     * IP Functions
     * ============================================================================ */

    /**
     * @brief Initialize IP stack
     * @return 0 on success, negative value on error
     */
    int orion_ip_init(void);

    /**
     * @brief Shutdown IP stack
     * @return 0 on success, negative value on error
     */
    int orion_ip_shutdown(void);

    /**
     * @brief Send IP packet
     * @param src_ip Source IP address
     * @param dst_ip Destination IP address
     * @param protocol Protocol
     * @param data Packet data
     * @param len Data length
     * @return 0 on success, negative value on error
     */
    int orion_ip_send(uint32_t src_ip, uint32_t dst_ip, uint8_t protocol,
                      const void *data, size_t len);

    /**
     * @brief Receive IP packet
     * @param packet Packet buffer
     * @param len Buffer length
     * @return Number of bytes received or negative value on error
     */
    ssize_t orion_ip_recv(void *packet, size_t len);

    /**
     * @brief Add IP route
     * @param dst_ip Destination IP address
     * @param dst_mask Destination mask
     * @param gateway Gateway IP address
     * @param interface Interface name
     * @return 0 on success, negative value on error
     */
    int orion_ip_add_route(uint32_t dst_ip, uint32_t dst_mask, uint32_t gateway, const char *interface);

    /**
     * @brief Remove IP route
     * @param dst_ip Destination IP address
     * @param dst_mask Destination mask
     * @return 0 on success, negative value on error
     */
    int orion_ip_remove_route(uint32_t dst_ip, uint32_t dst_mask);

    /* ============================================================================
     * UDP Functions
     * ============================================================================ */

    /**
     * @brief Initialize UDP stack
     * @return 0 on success, negative value on error
     */
    int orion_udp_init(void);

    /**
     * @brief Shutdown UDP stack
     * @return 0 on success, negative value on error
     */
    int orion_udp_shutdown(void);

    /**
     * @brief Send UDP datagram
     * @param src_ip Source IP address
     * @param src_port Source port
     * @param dst_ip Destination IP address
     * @param dst_port Destination port
     * @param data Datagram data
     * @param len Data length
     * @return 0 on success, negative value on error
     */
    int orion_udp_send(uint32_t src_ip, uint16_t src_port,
                       uint32_t dst_ip, uint16_t dst_port,
                       const void *data, size_t len);

    /**
     * @brief Receive UDP datagram
     * @param src_ip Source IP address (output)
     * @param src_port Source port (output)
     * @param data Datagram buffer
     * @param len Buffer length
     * @return Number of bytes received or negative value on error
     */
    ssize_t orion_udp_recv(uint32_t *src_ip, uint16_t *src_port,
                           void *data, size_t len);

    /* ============================================================================
     * ICMP Functions
     * ============================================================================ */

    /**
     * @brief Initialize ICMP stack
     * @return 0 on success, negative value on error
     */
    int orion_icmp_init(void);

    /**
     * @brief Shutdown ICMP stack
     * @return 0 on success, negative value on error
     */
    int orion_icmp_shutdown(void);

    /**
     * @brief Send ICMP message
     * @param src_ip Source IP address
     * @param dst_ip Destination IP address
     * @param type ICMP type
     * @param code ICMP code
     * @param data Message data
     * @param len Data length
     * @return 0 on success, negative value on error
     */
    int orion_icmp_send(uint32_t src_ip, uint32_t dst_ip, uint8_t type, uint8_t code,
                        const void *data, size_t len);

    /**
     * @brief Send ICMP echo request (ping)
     * @param src_ip Source IP address
     * @param dst_ip Destination IP address
     * @param sequence Sequence number
     * @return 0 on success, negative value on error
     */
    int orion_icmp_ping(uint32_t src_ip, uint32_t dst_ip, uint16_t sequence);

    /* ============================================================================
     * Network Address Translation (NAT)
     * ============================================================================ */

    /**
     * @brief Initialize NAT
     * @return 0 on success, negative value on error
     */
    int orion_nat_init(void);

    /**
     * @brief Add NAT rule
     * @param internal_ip Internal IP address
     * @param external_ip External IP address
     * @param internal_port Internal port
     * @param external_port External port
     * @param protocol Protocol
     * @return 0 on success, negative value on error
     */
    int orion_nat_add_rule(uint32_t internal_ip, uint32_t external_ip,
                           uint16_t internal_port, uint16_t external_port, uint8_t protocol);

    /**
     * @brief Remove NAT rule
     * @param internal_ip Internal IP address
     * @param internal_port Internal port
     * @param protocol Protocol
     * @return 0 on success, negative value on error
     */
    int orion_nat_remove_rule(uint32_t internal_ip, uint16_t internal_port, uint8_t protocol);

    /* ============================================================================
     * Firewall Functions
     * ============================================================================ */

    /**
     * @brief Initialize firewall
     * @return 0 on success, negative value on error
     */
    int orion_firewall_init(void);

    /**
     * @brief Add firewall rule
     * @param src_ip Source IP address
     * @param dst_ip Destination IP address
     * @param src_port Source port
     * @param dst_port Destination port
     * @param protocol Protocol
     * @param action Action (allow/deny)
     * @return 0 on success, negative value on error
     */
    int orion_firewall_add_rule(uint32_t src_ip, uint32_t dst_ip,
                                uint16_t src_port, uint16_t dst_port,
                                uint8_t protocol, int action);

    /**
     * @brief Remove firewall rule
     * @param rule_id Rule ID
     * @return 0 on success, negative value on error
     */
    int orion_firewall_remove_rule(uint32_t rule_id);

#ifdef __cplusplus
}
#endif

#endif // ORION_TCP_IP_STACK_H
