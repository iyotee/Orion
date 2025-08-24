/*
 * Orion Operating System - Advanced Network Protocols
 *
 * Implementation of modern network protocols including
 * HTTP/HTTPS, WebSocket, gRPC, QUIC, and more
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_ADVANCED_PROTOCOLS_H
#define ORION_ADVANCED_PROTOCOLS_H

#include <orion/types.h>
#include <orion/structures.h>
#include "network_architecture.h"
#include "tcp_ip_stack.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* ============================================================================
 * HTTP/HTTPS Protocol
 * ============================================================================ */

// HTTP methods
#define ORION_HTTP_METHOD_GET "GET"
#define ORION_HTTP_METHOD_POST "POST"
#define ORION_HTTP_METHOD_PUT "PUT"
#define ORION_HTTP_METHOD_DELETE "DELETE"
#define ORION_HTTP_METHOD_HEAD "HEAD"
#define ORION_HTTP_METHOD_OPTIONS "OPTIONS"
#define ORION_HTTP_METHOD_PATCH "PATCH"
#define ORION_HTTP_METHOD_TRACE "TRACE"

// HTTP status codes
#define ORION_HTTP_STATUS_OK 200
#define ORION_HTTP_STATUS_CREATED 201
#define ORION_HTTP_STATUS_ACCEPTED 202
#define ORION_HTTP_STATUS_NO_CONTENT 204
#define ORION_HTTP_STATUS_MOVED_PERMANENTLY 301
#define ORION_HTTP_STATUS_FOUND 302
#define ORION_HTTP_STATUS_NOT_MODIFIED 304
#define ORION_HTTP_STATUS_BAD_REQUEST 400
#define ORION_HTTP_STATUS_UNAUTHORIZED 401
#define ORION_HTTP_STATUS_FORBIDDEN 403
#define ORION_HTTP_STATUS_NOT_FOUND 404
#define ORION_HTTP_STATUS_METHOD_NOT_ALLOWED 405
#define ORION_HTTP_STATUS_INTERNAL_ERROR 500
#define ORION_HTTP_STATUS_NOT_IMPLEMENTED 501
#define ORION_HTTP_STATUS_BAD_GATEWAY 502
#define ORION_HTTP_STATUS_SERVICE_UNAVAILABLE 503

    // HTTP headers
    typedef struct
    {
        char *name;                     // Header name
        char *value;                    // Header value
        struct orion_http_header *next; // Next header
    } orion_http_header_t;

    // HTTP request structure
    typedef struct
    {
        char *method;                 // HTTP method
        char *uri;                    // Request URI
        char *version;                // HTTP version
        orion_http_header_t *headers; // Request headers
        void *body;                   // Request body
        size_t body_length;           // Body length
    } orion_http_request_t;

    // HTTP response structure
    typedef struct
    {
        char *version;                // HTTP version
        int status_code;              // Status code
        char *status_text;            // Status text
        orion_http_header_t *headers; // Response headers
        void *body;                   // Response body
        size_t body_length;           // Body length
    } orion_http_response_t;

    // HTTP server structure
    typedef struct orion_http_server
    {
        uint32_t bind_ip;    // Bind IP address
        uint16_t bind_port;  // Bind port
        int max_connections; // Maximum connections
        int (*request_handler)(struct orion_http_server *server,
                               const orion_http_request_t *request,
                               orion_http_response_t *response);
        void *private_data; // Private data
    } orion_http_server_t;

/* ============================================================================
 * WebSocket Protocol
 * ============================================================================ */

// WebSocket frame types
#define ORION_WS_FRAME_CONTINUATION 0x0
#define ORION_WS_FRAME_TEXT 0x1
#define ORION_WS_FRAME_BINARY 0x2
#define ORION_WS_FRAME_CLOSE 0x8
#define ORION_WS_FRAME_PING 0x9
#define ORION_WS_FRAME_PONG 0xA

    // WebSocket frame structure
    typedef struct __attribute__((packed))
    {
        uint8_t fin_rsv_opcode;        // FIN (1 bit) + RSV (3 bits) + Opcode (4 bits)
        uint8_t mask_payload_len;      // MASK (1 bit) + Payload Length (7 bits)
        uint16_t extended_payload_len; // Extended payload length (if needed)
        uint64_t masking_key;          // Masking key (if MASK is set)
        uint8_t payload_data[];        // Payload data
    } orion_websocket_frame_t;

    // WebSocket connection structure
    typedef struct orion_websocket_connection
    {
        orion_tcp_connection_t *tcp_conn; // Underlying TCP connection
        char *subprotocol;                // Subprotocol
        char *extensions;                 // Extensions
        uint64_t ping_interval;           // Ping interval
        uint64_t last_ping;               // Last ping time
        void *user_data;                  // User data
    } orion_websocket_connection_t;

/* ============================================================================
 * gRPC Protocol
 * ============================================================================ */

// gRPC frame types
#define ORION_GRPC_FRAME_DATA 0x0
#define ORION_GRPC_FRAME_HEADERS 0x1
#define ORION_GRPC_FRAME_RST_STREAM 0x3
#define ORION_GRPC_FRAME_SETTINGS 0x4
#define ORION_GRPC_FRAME_PING 0x6
#define ORION_GRPC_FRAME_GOAWAY 0x7
#define ORION_GRPC_FRAME_WINDOW_UPDATE 0x8

    // gRPC frame structure
    typedef struct __attribute__((packed))
    {
        uint8_t length[3];  // Length (24 bits)
        uint8_t type;       // Frame type
        uint8_t flags;      // Flags
        uint32_t stream_id; // Stream ID
        uint8_t payload[];  // Frame payload
    } orion_grpc_frame_t;

    // gRPC stream structure
    typedef struct orion_grpc_stream
    {
        uint32_t stream_id;   // Stream ID
        uint32_t window_size; // Window size
        uint32_t priority;    // Priority
        void *user_data;      // User data
    } orion_grpc_stream_t;

/* ============================================================================
 * QUIC Protocol
 * ============================================================================ */

// QUIC packet types
#define ORION_QUIC_PACKET_INITIAL 0x0
#define ORION_QUIC_PACKET_0RTT 0x1
#define ORION_QUIC_PACKET_HANDSHAKE 0x2
#define ORION_QUIC_PACKET_RETRY 0x3
#define ORION_QUIC_PACKET_1RTT 0x4

// QUIC frame types
#define ORION_QUIC_FRAME_PADDING 0x00
#define ORION_QUIC_FRAME_PING 0x01
#define ORION_QUIC_FRAME_ACK 0x02
#define ORION_QUIC_FRAME_RESET_STREAM 0x04
#define ORION_QUIC_FRAME_STOP_SENDING 0x05
#define ORION_QUIC_FRAME_CRYPTO 0x06
#define ORION_QUIC_FRAME_NEW_TOKEN 0x07
#define ORION_QUIC_FRAME_STREAM 0x08
#define ORION_QUIC_FRAME_MAX_DATA 0x10
#define ORION_QUIC_FRAME_MAX_STREAM_DATA 0x11
#define ORION_QUIC_FRAME_MAX_STREAMS 0x12
#define ORION_QUIC_FRAME_DATA_BLOCKED 0x14
#define ORION_QUIC_FRAME_STREAM_DATA_BLOCKED 0x15
#define ORION_QUIC_FRAME_STREAMS_BLOCKED 0x16
#define ORION_QUIC_FRAME_NEW_CONNECTION_ID 0x18
#define ORION_QUIC_FRAME_RETIRE_CONNECTION_ID 0x19
#define ORION_QUIC_FRAME_PATH_CHALLENGE 0x1A
#define ORION_QUIC_FRAME_PATH_RESPONSE 0x1B
#define ORION_QUIC_FRAME_CONNECTION_CLOSE 0x1C
#define ORION_QUIC_FRAME_APPLICATION_CLOSE 0x1D

    // QUIC connection structure
    typedef struct orion_quic_connection
    {
        uint64_t connection_id;    // Connection ID
        uint32_t version;          // QUIC version
        uint32_t state;            // Connection state
        uint64_t max_data;         // Maximum data
        uint64_t max_stream_data;  // Maximum stream data
        uint32_t max_streams_bidi; // Maximum bidirectional streams
        uint32_t max_streams_uni;  // Maximum unidirectional streams
        void *user_data;           // User data
    } orion_quic_connection_t;

/* ============================================================================
 * DNS Protocol
 * ============================================================================ */

// DNS record types
#define ORION_DNS_TYPE_A 1           // IPv4 address
#define ORION_DNS_TYPE_NS 2          // Name server
#define ORION_DNS_TYPE_CNAME 5       // Canonical name
#define ORION_DNS_TYPE_SOA 6         // Start of authority
#define ORION_DNS_TYPE_PTR 12        // Pointer
#define ORION_DNS_TYPE_MX 15         // Mail exchange
#define ORION_DNS_TYPE_TXT 16        // Text
#define ORION_DNS_TYPE_AAAA 28       // IPv6 address
#define ORION_DNS_TYPE_SRV 33        // Service record
#define ORION_DNS_TYPE_NAPTR 35      // Naming authority pointer
#define ORION_DNS_TYPE_OPT 41        // Option
#define ORION_DNS_TYPE_DS 43         // Delegation signer
#define ORION_DNS_TYPE_RRSIG 46      // Resource record signature
#define ORION_DNS_TYPE_NSEC 47       // Next secure
#define ORION_DNS_TYPE_DNSKEY 48     // DNS key
#define ORION_DNS_TYPE_NSEC3 50      // Next secure 3
#define ORION_DNS_TYPE_NSEC3PARAM 51 // NSEC3 parameters
#define ORION_DNS_TYPE_TLSA 52       // TLSA
#define ORION_DNS_TYPE_SMIMEA 53     // S/MIME certificate association
#define ORION_DNS_TYPE_HTTPS 65      // HTTPS binding
#define ORION_DNS_TYPE_SVCB 64       // Service binding
#define ORION_DNS_TYPE_OPENPGPKEY 61 // OpenPGP public key

    // DNS header structure
    typedef struct __attribute__((packed))
    {
        uint16_t id;      // Identification
        uint16_t flags;   // Flags
        uint16_t qdcount; // Question count
        uint16_t ancount; // Answer count
        uint16_t nscount; // Authority count
        uint16_t arcount; // Additional count
    } orion_dns_header_t;

    // DNS query structure
    typedef struct
    {
        char *name;      // Query name
        uint16_t qtype;  // Query type
        uint16_t qclass; // Query class
    } orion_dns_query_t;

    // DNS record structure
    typedef struct
    {
        char *name;        // Record name
        uint16_t type;     // Record type
        uint16_t class;    // Record class
        uint32_t ttl;      // Time to live
        uint16_t rdlength; // Record data length
        void *rdata;       // Record data
    } orion_dns_record_t;

    // DNS message structure
    typedef struct
    {
        orion_dns_header_t header;       // DNS header
        orion_dns_query_t *queries;      // Queries
        orion_dns_record_t *answers;     // Answers
        orion_dns_record_t *authorities; // Authorities
        orion_dns_record_t *additionals; // Additional records
    } orion_dns_message_t;

/* ============================================================================
 * DHCP Protocol
 * ============================================================================ */

// DHCP message types
#define ORION_DHCP_DISCOVER 1 // Client broadcast to locate available servers
#define ORION_DHCP_OFFER 2    // Server to client in response to DISCOVER
#define ORION_DHCP_REQUEST 3  // Client message to servers requesting offered address
#define ORION_DHCP_DECLINE 4  // Client to server indicating address is already in use
#define ORION_DHCP_ACK 5      // Server to client with configuration parameters
#define ORION_DHCP_NAK 6      // Server to client indicating client's notion of network address is incorrect
#define ORION_DHCP_RELEASE 7  // Client to server relinquishing network address
#define ORION_DHCP_INFORM 8   // Client to server asking only for local configuration parameters

// DHCP options
#define ORION_DHCP_OPT_SUBNET_MASK 1     // Subnet mask
#define ORION_DHCP_OPT_ROUTER 3          // Router
#define ORION_DHCP_OPT_DNS_SERVER 6      // DNS server
#define ORION_DHCP_OPT_DOMAIN_NAME 15    // Domain name
#define ORION_DHCP_OPT_BROADCAST 28      // Broadcast address
#define ORION_DHCP_OPT_REQUESTED_IP 50   // Requested IP address
#define ORION_DHCP_OPT_LEASE_TIME 51     // Lease time
#define ORION_DHCP_OPT_MESSAGE_TYPE 53   // Message type
#define ORION_DHCP_OPT_SERVER_ID 54      // Server identifier
#define ORION_DHCP_OPT_PARAM_REQUEST 55  // Parameter request list
#define ORION_DHCP_OPT_RENEWAL_TIME 58   // Renewal time
#define ORION_DHCP_OPT_REBINDING_TIME 59 // Rebinding time
#define ORION_DHCP_OPT_CLIENT_ID 61      // Client identifier
#define ORION_DHCP_OPT_END 255           // End

    // DHCP header structure
    typedef struct __attribute__((packed))
    {
        uint8_t op;         // Message op code
        uint8_t htype;      // Hardware address type
        uint8_t hlen;       // Hardware address length
        uint8_t hops;       // Hops
        uint32_t xid;       // Transaction ID
        uint16_t secs;      // Seconds elapsed
        uint16_t flags;     // Flags
        uint32_t ciaddr;    // Client IP address
        uint32_t yiaddr;    // Your IP address
        uint32_t siaddr;    // Server IP address
        uint32_t giaddr;    // Gateway IP address
        uint8_t chaddr[16]; // Client hardware address
        uint8_t sname[64];  // Server name
        uint8_t file[128];  // Boot file name
        uint8_t vend[64];   // Vendor specific information
    } orion_dhcp_header_t;

    /* ============================================================================
     * Protocol Functions
     * ============================================================================ */

    // HTTP functions
    int orion_http_server_init(orion_http_server_t *server);
    int orion_http_server_start(orion_http_server_t *server);
    int orion_http_server_stop(orion_http_server_t *server);
    int orion_http_parse_request(const char *data, size_t len, orion_http_request_t *request);
    int orion_http_create_response(int status_code, const char *body, size_t body_len, orion_http_response_t *response);
    int orion_http_send_response(orion_tcp_connection_t *conn, const orion_http_response_t *response);

    // WebSocket functions
    int orion_websocket_init(void);
    orion_websocket_connection_t *orion_websocket_accept(orion_tcp_connection_t *conn);
    int orion_websocket_send_frame(orion_websocket_connection_t *conn, uint8_t opcode, const void *data, size_t len);
    int orion_websocket_recv_frame(orion_websocket_connection_t *conn, void *data, size_t len);

    // gRPC functions
    int orion_grpc_init(void);
    orion_grpc_stream_t *orion_grpc_create_stream(uint32_t stream_id);
    int orion_grpc_send_frame(orion_grpc_stream_t *stream, uint8_t type, const void *data, size_t len);
    int orion_grpc_recv_frame(orion_grpc_stream_t *stream, void *data, size_t len);

    // QUIC functions
    int orion_quic_init(void);
    orion_quic_connection_t *orion_quic_connect(const char *host, uint16_t port);
    int orion_quic_send_packet(orion_quic_connection_t *conn, uint8_t type, const void *data, size_t len);
    int orion_quic_recv_packet(orion_quic_connection_t *conn, void *data, size_t len);

    // DNS functions
    int orion_dns_init(void);
    int orion_dns_resolve(const char *domain, uint16_t type, orion_dns_record_t **records);
    int orion_dns_send_query(const char *domain, uint16_t type, uint16_t class);
    int orion_dns_parse_response(const void *data, size_t len, orion_dns_message_t *message);

    // DHCP functions
    int orion_dhcp_init(void);
    int orion_dhcp_discover(void);
    int orion_dhcp_request(uint32_t requested_ip);
    int orion_dhcp_release(uint32_t ip);
    int orion_dhcp_parse_message(const void *data, size_t len, orion_dhcp_header_t *header);

    /* ============================================================================
     * Protocol Utilities
     * ============================================================================ */

    // URL parsing
    typedef struct
    {
        char *scheme;   // URL scheme (http, https, etc.)
        char *host;     // Host name or IP
        uint16_t port;  // Port number
        char *path;     // Path
        char *query;    // Query string
        char *fragment; // Fragment
    } orion_url_t;

    int orion_url_parse(const char *url, orion_url_t *parsed);
    void orion_url_free(orion_url_t *parsed);

    // MIME types
    const char *orion_mime_get_type(const char *filename);
    const char *orion_mime_get_extension(const char *mime_type);

    // Base64 encoding/decoding
    int orion_base64_encode(const void *data, size_t len, char *encoded, size_t encoded_len);
    int orion_base64_decode(const char *encoded, void *data, size_t data_len);

    // URL encoding/decoding
    int orion_url_encode(const char *str, char *encoded, size_t encoded_len);
    int orion_url_decode(const char *encoded, char *decoded, size_t decoded_len);

    // JSON parsing (basic)
    typedef struct orion_json_value
    {
        enum
        {
            ORION_JSON_NULL,
            ORION_JSON_BOOL,
            ORION_JSON_NUMBER,
            ORION_JSON_STRING,
            ORION_JSON_ARRAY,
            ORION_JSON_OBJECT
        } type;
        union
        {
            bool bool_value;
            double number_value;
            char *string_value;
            struct orion_json_value **array_value;
            struct orion_json_pair *object_value;
        } value;
    } orion_json_value_t;

    typedef struct orion_json_pair
    {
        char *key;
        orion_json_value_t *value;
        struct orion_json_pair *next;
    } orion_json_pair_t;

    orion_json_value_t *orion_json_parse(const char *json);
    void orion_json_free(orion_json_value_t *value);
    orion_json_value_t *orion_json_get_value(orion_json_value_t *obj, const char *key);

#ifdef __cplusplus
}
#endif

#endif // ORION_ADVANCED_PROTOCOLS_H
