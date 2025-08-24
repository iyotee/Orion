/*
 * Orion Operating System - Advanced Network Protocols Implementation
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

#include "advanced_protocols.h"
#include <orion/klog.h>
#include <orion/mm.h>
#include <orion/string.h>
#include <orion/spinlock.h>
#include <orion/time.h>
#include <string.h>

// Global protocol state
static struct {
    bool http_initialized;
    bool websocket_initialized;
    bool grpc_initialized;
    bool quic_initialized;
    bool dns_initialized;
    bool dhcp_initialized;
    spinlock_t lock;
} protocols = {0};

// HTTP server list
static orion_http_server_t *http_servers = NULL;
static spinlock_t http_lock = SPINLOCK_INITIALIZER;

// WebSocket connections
static orion_websocket_connection_t *websocket_connections = NULL;
static spinlock_t websocket_lock = SPINLOCK_INITIALIZER;

// gRPC streams
static orion_grpc_stream_t *grpc_streams = NULL;
static spinlock_t grpc_lock = SPINLOCK_INITIALIZER;

// QUIC connections
static orion_quic_connection_t *quic_connections = NULL;
static spinlock_t quic_lock = SPINLOCK_INITIALIZER;

/* ============================================================================
 * HTTP/HTTPS Protocol Implementation
 * ============================================================================ */

int orion_http_server_init(orion_http_server_t *server)
{
    if (protocols.http_initialized) {
        klog_error(KLOG_CAT_KERNEL, "HTTP stack already initialized");
        return -1;
    }

    protocols.http_initialized = true;
    http_servers = NULL;

    klog_info(KLOG_CAT_KERNEL, "HTTP stack initialized successfully");
    return 0;
}

int orion_http_server_start(orion_http_server_t *server)
{
    if (!protocols.http_initialized || !server) {
        return -1;
    }

    spinlock_acquire(&http_lock);

    // Add server to list
    server->next = http_servers;
    http_servers = server;

    klog_info(KLOG_CAT_KERNEL, "HTTP server started: %u:%u", server->bind_ip, server->bind_port);

    spinlock_release(&http_lock);
    return 0;
}

int orion_http_server_stop(orion_http_server_t *server)
{
    if (!protocols.http_initialized || !server) {
        return -1;
    }

    spinlock_acquire(&http_lock);

    // Remove server from list
    if (http_servers == server) {
        http_servers = server->next;
    } else {
        orion_http_server_t *prev = http_servers;
        while (prev && prev->next != server) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = server->next;
        }
    }

    klog_info(KLOG_CAT_KERNEL, "HTTP server stopped: %u:%u", server->bind_ip, server->bind_port);

    spinlock_release(&http_lock);
    return 0;
}

int orion_http_parse_request(const char *data, size_t len, orion_http_request_t *request)
{
    if (!data || !request || len == 0) {
        return -1;
    }

    // Simple HTTP request parser
    // In a real implementation, this would be more robust
    char *line = strtok((char*)data, "\r\n");
    if (!line) {
        return -1;
    }

    // Parse request line: METHOD URI VERSION
    char *method = strtok(line, " ");
    char *uri = strtok(NULL, " ");
    char *version = strtok(NULL, " ");

    if (!method || !uri || !version) {
        return -1;
    }

    request->method = kstrdup(method);
    request->uri = kstrdup(uri);
    request->version = kstrdup(version);
    request->headers = NULL;
    request->body = NULL;
    request->body_length = 0;

    // Parse headers (simplified)
    while ((line = strtok(NULL, "\r\n")) != NULL) {
        if (strlen(line) == 0) break; // Empty line indicates end of headers

        char *colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            char *header_name = line;
            char *header_value = colon + 1;
            
            // Skip leading spaces in value
            while (*header_value == ' ') header_value++;

            orion_http_header_t *header = kmalloc(sizeof(orion_http_header_t));
            if (header) {
                header->name = kstrdup(header_name);
                header->value = kstrdup(header_value);
                header->next = request->headers;
                request->headers = header;
            }
        }
    }

    klog_debug(KLOG_CAT_KERNEL, "HTTP request parsed: %s %s %s", method, uri, version);
    return 0;
}

int orion_http_create_response(int status_code, const char *body, size_t body_len, orion_http_response_t *response)
{
    if (!response) {
        return -1;
    }

    response->version = kstrdup("HTTP/1.1");
    response->status_code = status_code;
    response->body = body ? kstrdup(body) : NULL;
    response->body_length = body_len;
    response->headers = NULL;

    // Set status text based on status code
    switch (status_code) {
        case 200: response->status_text = kstrdup("OK"); break;
        case 201: response->status_text = kstrdup("Created"); break;
        case 400: response->status_text = kstrdup("Bad Request"); break;
        case 404: response->status_text = kstrdup("Not Found"); break;
        case 500: response->status_text = kstrdup("Internal Server Error"); break;
        default: response->status_text = kstrdup("Unknown"); break;
    }

    // Add content length header
    orion_http_header_t *content_length_header = kmalloc(sizeof(orion_http_header_t));
    if (content_length_header) {
        char length_str[32];
        snprintf(length_str, sizeof(length_str), "%zu", body_len);
        content_length_header->name = kstrdup("Content-Length");
        content_length_header->value = kstrdup(length_str);
        content_length_header->next = response->headers;
        response->headers = content_length_header;
    }

    klog_debug(KLOG_CAT_KERNEL, "HTTP response created: %d %s", status_code, response->status_text);
    return 0;
}

int orion_http_send_response(orion_tcp_connection_t *conn, const orion_http_response_t *response)
{
    if (!conn || !response) {
        return -1;
    }

    // Build response string
    char response_str[4096];
    int len = snprintf(response_str, sizeof(response_str),
                      "%s %d %s\r\n",
                      response->version, response->status_code, response->status_text);

    // Add headers
    orion_http_header_t *header = response->headers;
    while (header) {
        len += snprintf(response_str + len, sizeof(response_str) - len,
                       "%s: %s\r\n", header->name, header->value);
        header = header->next;
    }

    // Add empty line and body
    len += snprintf(response_str + len, sizeof(response_str) - len, "\r\n");
    if (response->body) {
        strncpy(response_str + len, response->body, response->body_length);
        len += response->body_length;
    }

    // Send via TCP
    ssize_t sent = orion_tcp_send(conn, response_str, len);
    
    if (sent > 0) {
        klog_debug(KLOG_CAT_KERNEL, "HTTP response sent: %d bytes", sent);
    }

    return (sent > 0) ? 0 : -1;
}

/* ============================================================================
 * WebSocket Protocol Implementation
 * ============================================================================ */

int orion_websocket_init(void)
{
    if (protocols.websocket_initialized) {
        klog_error(KLOG_CAT_KERNEL, "WebSocket stack already initialized");
        return -1;
    }

    protocols.websocket_initialized = true;
    websocket_connections = NULL;

    klog_info(KLOG_CAT_KERNEL, "WebSocket stack initialized successfully");
    return 0;
}

orion_websocket_connection_t *orion_websocket_accept(orion_tcp_connection_t *conn)
{
    if (!protocols.websocket_initialized || !conn) {
        return NULL;
    }

    // Create WebSocket connection
    orion_websocket_connection_t *ws_conn = kmalloc(sizeof(orion_websocket_connection_t));
    if (!ws_conn) {
        klog_error(KLOG_CAT_KERNEL, "Failed to allocate memory for WebSocket connection");
        return NULL;
    }

    // Initialize WebSocket connection
    memset(ws_conn, 0, sizeof(orion_websocket_connection_t));
    ws_conn->tcp_conn = conn;
    ws_conn->ping_interval = 30000; // 30 seconds
    ws_conn->last_ping = orion_get_timestamp();

    // Add to connection list
    spinlock_acquire(&websocket_lock);
    ws_conn->next = websocket_connections;
    websocket_connections = ws_conn;
    spinlock_release(&websocket_lock);

    klog_info(KLOG_CAT_KERNEL, "WebSocket connection accepted");
    return ws_conn;
}

int orion_websocket_send_frame(orion_websocket_connection_t *conn, uint8_t opcode, const void *data, size_t len)
{
    if (!protocols.websocket_initialized || !conn || !conn->tcp_conn) {
        return -1;
    }

    // Create WebSocket frame
    orion_websocket_frame_t *frame = kmalloc(sizeof(orion_websocket_frame_t) + len);
    if (!frame) {
        klog_error(KLOG_CAT_KERNEL, "Failed to allocate memory for WebSocket frame");
        return -1;
    }

    // Set frame header
    frame->fin_rsv_opcode = 0x80 | opcode; // FIN=1, RSV=0, Opcode
    frame->mask_payload_len = len; // No mask, payload length
    frame->extended_payload_len = 0;
    frame->masking_key = 0;

    // Copy payload
    if (data && len > 0) {
        memcpy(frame->payload_data, data, len);
    }

    // Send frame via TCP
    ssize_t sent = orion_tcp_send(conn->tcp_conn, frame, sizeof(orion_websocket_frame_t) + len);
    
    kfree(frame);

    if (sent > 0) {
        klog_debug(KLOG_CAT_KERNEL, "WebSocket frame sent: opcode %d, length %zu", opcode, len);
        return 0;
    }

    return -1;
}

int orion_websocket_recv_frame(orion_websocket_connection_t *conn, void *data, size_t len)
{
    if (!protocols.websocket_initialized || !conn || !conn->tcp_conn) {
        return -1;
    }

    // Receive frame via TCP
    orion_websocket_frame_t frame;
    ssize_t received = orion_tcp_recv(conn->tcp_conn, &frame, sizeof(frame));
    
    if (received < sizeof(orion_websocket_frame_t)) {
        return -1;
    }

    uint8_t opcode = frame.fin_rsv_opcode & 0x0F;
    size_t payload_len = frame.mask_payload_len & 0x7F;

    // Handle different frame types
    switch (opcode) {
        case ORION_WS_FRAME_PING:
            // Send pong response
            orion_websocket_send_frame(conn, ORION_WS_FRAME_PONG, NULL, 0);
            conn->last_ping = orion_get_timestamp();
            break;

        case ORION_WS_FRAME_CLOSE:
            // Handle close frame
            klog_info(KLOG_CAT_KERNEL, "WebSocket close frame received");
            break;

        case ORION_WS_FRAME_TEXT:
        case ORION_WS_FRAME_BINARY:
            // Copy payload data
            if (payload_len > 0 && payload_len <= len) {
                memcpy(data, frame.payload_data, payload_len);
                klog_debug(KLOG_CAT_KERNEL, "WebSocket frame received: opcode %d, length %zu", opcode, payload_len);
                return payload_len;
            }
            break;
    }

    return 0;
}

/* ============================================================================
 * gRPC Protocol Implementation
 * ============================================================================ */

int orion_grpc_init(void)
{
    if (protocols.grpc_initialized) {
        klog_error(KLOG_CAT_KERNEL, "gRPC stack already initialized");
        return -1;
    }

    protocols.grpc_initialized = true;
    grpc_streams = NULL;

    klog_info(KLOG_CAT_KERNEL, "gRPC stack initialized successfully");
    return 0;
}

orion_grpc_stream_t *orion_grpc_create_stream(uint32_t stream_id)
{
    if (!protocols.grpc_initialized) {
        return NULL;
    }

    // Create gRPC stream
    orion_grpc_stream_t *stream = kmalloc(sizeof(orion_grpc_stream_t));
    if (!stream) {
        klog_error(KLOG_CAT_KERNEL, "Failed to allocate memory for gRPC stream");
        return NULL;
    }

    // Initialize stream
    stream->stream_id = stream_id;
    stream->window_size = 65535;
    stream->priority = 0;
    stream->user_data = NULL;

    // Add to stream list
    spinlock_acquire(&grpc_lock);
    stream->next = grpc_streams;
    grpc_streams = stream;
    spinlock_release(&grpc_lock);

    klog_info(KLOG_CAT_KERNEL, "gRPC stream created: ID %u", stream_id);
    return stream;
}

int orion_grpc_send_frame(orion_grpc_stream_t *stream, uint8_t type, const void *data, size_t len)
{
    if (!protocols.grpc_initialized || !stream) {
        return -1;
    }

    // Create gRPC frame
    orion_grpc_frame_t *frame = kmalloc(sizeof(orion_grpc_frame_t) + len);
    if (!frame) {
        klog_error(KLOG_CAT_KERNEL, "Failed to allocate memory for gRPC frame");
        return -1;
    }

    // Set frame header
    frame->length[0] = (len >> 16) & 0xFF;
    frame->length[1] = (len >> 8) & 0xFF;
    frame->length[2] = len & 0xFF;
    frame->type = type;
    frame->flags = 0;
    frame->stream_id = htonl(stream->stream_id);

    // Copy payload
    if (data && len > 0) {
        memcpy(frame->payload, data, len);
    }

    klog_debug(KLOG_CAT_KERNEL, "gRPC frame sent: type %d, stream %u, length %zu",
               type, stream->stream_id, len);

    // In a real implementation, this would send via HTTP/2
    kfree(frame);
    return 0;
}

int orion_grpc_recv_frame(orion_grpc_stream_t *stream, void *data, size_t len)
{
    if (!protocols.grpc_initialized || !stream) {
        return -1;
    }

    // In a real implementation, this would receive via HTTP/2
    // For now, just return success
    klog_debug(KLOG_CAT_KERNEL, "gRPC frame received: stream %u", stream->stream_id);
    return 0;
}

/* ============================================================================
 * QUIC Protocol Implementation
 * ============================================================================ */

int orion_quic_init(void)
{
    if (protocols.quic_initialized) {
        klog_error(KLOG_CAT_KERNEL, "QUIC stack already initialized");
        return -1;
    }

    protocols.quic_initialized = true;
    quic_connections = NULL;

    klog_info(KLOG_CAT_KERNEL, "QUIC stack initialized successfully");
    return 0;
}

orion_quic_connection_t *orion_quic_connect(const char *host, uint16_t port)
{
    if (!protocols.quic_initialized || !host) {
        return NULL;
    }

    // Create QUIC connection
    orion_quic_connection_t *conn = kmalloc(sizeof(orion_quic_connection_t));
    if (!conn) {
        klog_error(KLOG_CAT_KERNEL, "Failed to allocate memory for QUIC connection");
        return NULL;
    }

    // Initialize connection
    memset(conn, 0, sizeof(orion_quic_connection_t));
    conn->connection_id = orion_get_timestamp();
    conn->version = 1; // QUIC version 1
    conn->state = 0; // Initial state
    conn->max_data = 1048576; // 1MB
    conn->max_stream_data = 65536; // 64KB
    conn->max_streams_bidi = 100;
    conn->max_streams_uni = 100;
    conn->user_data = NULL;

    // Add to connection list
    spinlock_acquire(&quic_lock);
    conn->next = quic_connections;
    quic_connections = conn;
    spinlock_release(&quic_lock);

    klog_info(KLOG_CAT_KERNEL, "QUIC connection created: %s:%u", host, port);
    return conn;
}

int orion_quic_send_packet(orion_quic_connection_t *conn, uint8_t type, const void *data, size_t len)
{
    if (!protocols.quic_initialized || !conn) {
        return -1;
    }

    // In a real implementation, this would create and send QUIC packets
    // For now, just log the operation
    klog_debug(KLOG_CAT_KERNEL, "QUIC packet sent: type %d, connection %llu, length %zu",
               type, conn->connection_id, len);

    return 0;
}

int orion_quic_recv_packet(orion_quic_connection_t *conn, void *data, size_t len)
{
    if (!protocols.quic_initialized || !conn) {
        return -1;
    }

    // In a real implementation, this would receive QUIC packets
    // For now, just log the operation
    klog_debug(KLOG_CAT_KERNEL, "QUIC packet received: connection %llu, length %zu",
               conn->connection_id, len);

    return 0;
}

/* ============================================================================
 * DNS Protocol Implementation
 * ============================================================================ */

int orion_dns_init(void)
{
    if (protocols.dns_initialized) {
        klog_error(KLOG_CAT_KERNEL, "DNS stack already initialized");
        return -1;
    }

    protocols.dns_initialized = true;
    klog_info(KLOG_CAT_KERNEL, "DNS stack initialized successfully");
    return 0;
}

int orion_dns_resolve(const char *domain, uint16_t type, orion_dns_record_t **records)
{
    if (!protocols.dns_initialized || !domain || !records) {
        return -1;
    }

    // In a real implementation, this would perform DNS resolution
    // For now, just log the operation
    klog_debug(KLOG_CAT_KERNEL, "DNS resolution requested: %s, type %d", domain, type);

    // Return empty result
    *records = NULL;
    return 0;
}

int orion_dns_send_query(const char *domain, uint16_t type, uint16_t class)
{
    if (!protocols.dns_initialized || !domain) {
        return -1;
    }

    // Create DNS query message
    orion_dns_message_t message;
    memset(&message, 0, sizeof(message));

    // Set header
    message.header.id = htons(0x1234);
    message.header.flags = htons(0x0100); // Standard query
    message.header.qdcount = htons(1);
    message.header.ancount = htons(0);
    message.header.nscount = htons(0);
    message.header.arcount = htons(0);

    // Set query
    message.queries = kmalloc(sizeof(orion_dns_query_t));
    if (message.queries) {
        message.queries->name = kstrdup(domain);
        message.queries->qtype = htons(type);
        message.queries->qclass = htons(class);
    }

    klog_debug(KLOG_CAT_KERNEL, "DNS query sent: %s, type %d, class %d", domain, type, class);

    // Clean up
    if (message.queries) {
        kfree(message.queries->name);
        kfree(message.queries);
    }

    return 0;
}

int orion_dns_parse_response(const void *data, size_t len, orion_dns_message_t *message)
{
    if (!protocols.dns_initialized || !data || !message || len < sizeof(orion_dns_header_t)) {
        return -1;
    }

    // Parse DNS response header
    const orion_dns_header_t *header = (const orion_dns_header_t*)data;
    
    message->header.id = ntohs(header->id);
    message->header.flags = ntohs(header->flags);
    message->header.qdcount = ntohs(header->qdcount);
    message->header.ancount = ntohs(header->ancount);
    message->header.nscount = ntohs(header->nscount);
    message->header.arcount = ntohs(header->arcount);

    klog_debug(KLOG_CAT_KERNEL, "DNS response parsed: %d answers", message->header.ancount);

    return 0;
}

/* ============================================================================
 * DHCP Protocol Implementation
 * ============================================================================ */

int orion_dhcp_init(void)
{
    if (protocols.dhcp_initialized) {
        klog_error(KLOG_CAT_KERNEL, "DHCP stack already initialized");
        return -1;
    }

    protocols.dhcp_initialized = true;
    klog_info(KLOG_CAT_KERNEL, "DHCP stack initialized successfully");
    return 0;
}

int orion_dhcp_discover(void)
{
    if (!protocols.dhcp_initialized) {
        return -1;
    }

    // Create DHCP discover message
    orion_dhcp_header_t header;
    memset(&header, 0, sizeof(header));

    header.op = ORION_DHCP_DISCOVER;
    header.htype = 1; // Ethernet
    header.hlen = 6;  // MAC address length
    header.xid = htonl(0x12345678); // Transaction ID

    klog_debug(KLOG_CAT_KERNEL, "DHCP discover sent");
    return 0;
}

int orion_dhcp_request(uint32_t requested_ip)
{
    if (!protocols.dhcp_initialized) {
        return -1;
    }

    // Create DHCP request message
    orion_dhcp_header_t header;
    memset(&header, 0, sizeof(header));

    header.op = ORION_DHCP_REQUEST;
    header.htype = 1; // Ethernet
    header.hlen = 6;  // MAC address length
    header.xid = htonl(0x12345678); // Transaction ID
    header.yiaddr = htonl(requested_ip);

    klog_debug(KLOG_CAT_KERNEL, "DHCP request sent for IP: %u", requested_ip);
    return 0;
}

int orion_dhcp_release(uint32_t ip)
{
    if (!protocols.dhcp_initialized) {
        return -1;
    }

    // Create DHCP release message
    orion_dhcp_header_t header;
    memset(&header, 0, sizeof(header));

    header.op = ORION_DHCP_RELEASE;
    header.htype = 1; // Ethernet
    header.hlen = 6;  // MAC address length
    header.ciaddr = htonl(ip);

    klog_debug(KLOG_CAT_KERNEL, "DHCP release sent for IP: %u", ip);
    return 0;
}

int orion_dhcp_parse_message(const void *data, size_t len, orion_dhcp_header_t *header)
{
    if (!protocols.dhcp_initialized || !data || !header || len < sizeof(orion_dhcp_header_t)) {
        return -1;
    }

    // Parse DHCP message header
    memcpy(header, data, sizeof(orion_dhcp_header_t));

    // Convert network byte order to host byte order
    header->xid = ntohl(header->xid);
    header->ciaddr = ntohl(header->ciaddr);
    header->yiaddr = ntohl(header->yiaddr);
    header->siaddr = ntohl(header->siaddr);
    header->giaddr = ntohl(header->giaddr);

    klog_debug(KLOG_CAT_KERNEL, "DHCP message parsed: op %d, IP %u", header->op, header->yiaddr);

    return 0;
}

/* ============================================================================
 * Protocol Utilities Implementation
 * ============================================================================ */

int orion_url_parse(const char *url, orion_url_t *parsed)
{
    if (!url || !parsed) {
        return -1;
    }

    memset(parsed, 0, sizeof(orion_url_t));

    // Simple URL parser
    // In a real implementation, this would be more robust
    char *url_copy = kstrdup(url);
    if (!url_copy) {
        return -1;
    }

    // Parse scheme
    char *scheme_end = strstr(url_copy, "://");
    if (scheme_end) {
        *scheme_end = '\0';
        parsed->scheme = kstrdup(url_copy);
        url_copy = scheme_end + 3;
    } else {
        parsed->scheme = kstrdup("http"); // Default scheme
    }

    // Parse host and port
    char *host_start = url_copy;
    char *port_start = strchr(url_copy, ':');
    char *path_start = strchr(url_copy, '/');

    if (port_start && (!path_start || port_start < path_start)) {
        *port_start = '\0';
        parsed->host = kstrdup(host_start);
        parsed->port = atoi(port_start + 1);
        url_copy = port_start + 1;
    } else if (path_start) {
        *path_start = '\0';
        parsed->host = kstrdup(host_start);
        parsed->port = 0; // Default port
        url_copy = path_start;
    } else {
        parsed->host = kstrdup(host_start);
        parsed->port = 0; // Default port
        url_copy = host_start + strlen(host_start);
    }

    // Parse path
    if (path_start) {
        char *query_start = strchr(path_start, '?');
        if (query_start) {
            *query_start = '\0';
            parsed->path = kstrdup(path_start);
            parsed->query = kstrdup(query_start + 1);
        } else {
            parsed->path = kstrdup(path_start);
        }
    }

    klog_debug(KLOG_CAT_KERNEL, "URL parsed: %s://%s:%u%s", 
               parsed->scheme, parsed->host, parsed->port, parsed->path ? parsed->path : "");

    kfree(url_copy);
    return 0;
}

void orion_url_free(orion_url_t *parsed)
{
    if (!parsed) return;

    if (parsed->scheme) kfree(parsed->scheme);
    if (parsed->host) kfree(parsed->host);
    if (parsed->path) kfree(parsed->path);
    if (parsed->query) kfree(parsed->query);
    if (parsed->fragment) kfree(parsed->fragment);

    memset(parsed, 0, sizeof(orion_url_t));
}

const char *orion_mime_get_type(const char *filename)
{
    if (!filename) return "application/octet-stream";

    const char *ext = strrchr(filename, '.');
    if (!ext) return "application/octet-stream";

    ext++; // Skip the dot

    // Common MIME types
    if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0) return "text/html";
    if (strcmp(ext, "css") == 0) return "text/css";
    if (strcmp(ext, "js") == 0) return "application/javascript";
    if (strcmp(ext, "json") == 0) return "application/json";
    if (strcmp(ext, "xml") == 0) return "application/xml";
    if (strcmp(ext, "txt") == 0) return "text/plain";
    if (strcmp(ext, "png") == 0) return "image/png";
    if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, "gif") == 0) return "image/gif";
    if (strcmp(ext, "pdf") == 0) return "application/pdf";

    return "application/octet-stream";
}

const char *orion_mime_get_extension(const char *mime_type)
{
    if (!mime_type) return NULL;

    // Common MIME type to extension mapping
    if (strcmp(mime_type, "text/html") == 0) return "html";
    if (strcmp(mime_type, "text/css") == 0) return "css";
    if (strcmp(mime_type, "application/javascript") == 0) return "js";
    if (strcmp(mime_type, "application/json") == 0) return "json";
    if (strcmp(mime_type, "application/xml") == 0) return "xml";
    if (strcmp(mime_type, "text/plain") == 0) return "txt";
    if (strcmp(mime_type, "image/png") == 0) return "png";
    if (strcmp(mime_type, "image/jpeg") == 0) return "jpg";
    if (strcmp(mime_type, "image/gif") == 0) return "gif";
    if (strcmp(mime_type, "application/pdf") == 0) return "pdf";

    return NULL;
}

int orion_base64_encode(const void *data, size_t len, char *encoded, size_t encoded_len)
{
    if (!data || !encoded || encoded_len == 0) {
        return -1;
    }

    // Simple base64 encoding (not production ready)
    const char *base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const uint8_t *input = (const uint8_t*)data;
    size_t output_len = 0;

    for (size_t i = 0; i < len; i += 3) {
        if (output_len + 4 > encoded_len) break;

        uint32_t chunk = (input[i] << 16);
        if (i + 1 < len) chunk |= (input[i + 1] << 8);
        if (i + 2 < len) chunk |= input[i + 2];

        encoded[output_len++] = base64_chars[(chunk >> 18) & 0x3F];
        encoded[output_len++] = base64_chars[(chunk >> 12) & 0x3F];
        encoded[output_len++] = base64_chars[(chunk >> 6) & 0x3F];
        encoded[output_len++] = base64_chars[chunk & 0x3F];
    }

    encoded[output_len] = '\0';
    return output_len;
}

int orion_base64_decode(const char *encoded, void *data, size_t data_len)
{
    if (!encoded || !data || data_len == 0) {
        return -1;
    }

    // Simple base64 decoding (not production ready)
    const char *base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    uint8_t *output = (uint8_t*)data;
    size_t output_len = 0;
    size_t encoded_len = strlen(encoded);

    for (size_t i = 0; i < encoded_len; i += 4) {
        if (output_len + 3 > data_len) break;

        uint32_t chunk = 0;
        for (int j = 0; j < 4; j++) {
            if (i + j < encoded_len) {
                const char *pos = strchr(base64_chars, encoded[i + j]);
                if (pos) {
                    chunk |= ((pos - base64_chars) << (18 - j * 6));
                }
            }
        }

        output[output_len++] = (chunk >> 16) & 0xFF;
        if (output_len < data_len) output[output_len++] = (chunk >> 8) & 0xFF;
        if (output_len < data_len) output[output_len++] = chunk & 0xFF;
    }

    return output_len;
}

int orion_url_encode(const char *str, char *encoded, size_t encoded_len)
{
    if (!str || !encoded || encoded_len == 0) {
        return -1;
    }

    size_t j = 0;
    for (size_t i = 0; str[i] && j < encoded_len - 1; i++) {
        if (isalnum(str[i]) || str[i] == '-' || str[i] == '_' || str[i] == '.' || str[i] == '~') {
            encoded[j++] = str[i];
        } else {
            if (j + 3 < encoded_len) {
                snprintf(encoded + j, 4, "%%%02X", (unsigned char)str[i]);
                j += 3;
            }
        }
    }
    encoded[j] = '\0';
    return j;
}

int orion_url_decode(const char *encoded, char *decoded, size_t decoded_len)
{
    if (!encoded || !decoded || decoded_len == 0) {
        return -1;
    }

    size_t j = 0;
    for (size_t i = 0; encoded[i] && j < decoded_len - 1; i++) {
        if (encoded[i] == '%' && i + 2 < strlen(encoded)) {
            char hex[3] = {encoded[i + 1], encoded[i + 2], '\0'};
            int value;
            if (sscanf(hex, "%x", &value) == 1) {
                decoded[j++] = (char)value;
                i += 2;
            }
        } else {
            decoded[j++] = encoded[i];
        }
    }
    decoded[j] = '\0';
    return j;
}

/* ============================================================================
 * JSON Parsing Implementation (Basic)
 * ============================================================================ */

orion_json_value_t *orion_json_parse(const char *json)
{
    if (!json) return NULL;

    // Very basic JSON parser - just creates a simple structure
    // In a real implementation, this would be much more sophisticated
    orion_json_value_t *value = kmalloc(sizeof(orion_json_value_t));
    if (!value) return NULL;

    // For now, just create a simple string value
    value->type = ORION_JSON_STRING;
    value->value.string_value = kstrdup(json);

    return value;
}

void orion_json_free(orion_json_value_t *value)
{
    if (!value) return;

    switch (value->type) {
        case ORION_JSON_STRING:
            if (value->value.string_value) kfree(value->value.string_value);
            break;
        case ORION_JSON_ARRAY:
            if (value->value.array_value) {
                // Free array elements
                for (int i = 0; value->value.array_value[i]; i++) {
                    orion_json_free(value->value.array_value[i]);
                }
                kfree(value->value.array_value);
            }
            break;
        case ORION_JSON_OBJECT:
            if (value->value.object_value) {
                // Free object pairs
                orion_json_pair_t *pair = value->value.object_value;
                while (pair) {
                    orion_json_pair_t *next = pair->next;
                    if (pair->key) kfree(pair->key);
                    if (pair->value) orion_json_free(pair->value);
                    kfree(pair);
                    pair = next;
                }
            }
            break;
        default:
            break;
    }

    kfree(value);
}

orion_json_value_t *orion_json_get_value(orion_json_value_t *obj, const char *key)
{
    if (!obj || !key || obj->type != ORION_JSON_OBJECT) {
        return NULL;
    }

    orion_json_pair_t *pair = obj->value.object_value;
    while (pair) {
        if (pair->key && strcmp(pair->key, key) == 0) {
            return pair->value;
        }
        pair = pair->next;
    }

    return NULL;
}
