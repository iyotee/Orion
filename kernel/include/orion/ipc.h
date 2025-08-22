#ifndef ORION_IPC_H
#define ORION_IPC_H

#include <orion/types.h>

// IPC Constants
#define IPC_MAX_MESSAGE_SIZE (64 * 1024) // 64KB max per message
#define IPC_MAX_CAPABILITIES 16          // Max capabilities per message
#define IPC_TIMEOUT_INFINITE 0xFFFFFFFFFFFFFFFFULL

// IPC message flags
#define IPC_FLAG_NONBLOCK (1 << 0)
#define IPC_FLAG_SYNC (1 << 1)
#define IPC_FLAG_ZEROCOPY (1 << 2)

// IPC message types
typedef enum
{
    IPC_MSG_DATA = 0,   // Message with data
    IPC_MSG_CAPABILITY, // Capability transfer
    IPC_MSG_PAGE,       // Page transfer (zero-copy)
    IPC_MSG_INTERRUPT,  // Interrupt notification
} ipc_msg_type_t;

// IPC system functions
void ipc_init(void);

// IPC port management
or_cap_t ipc_port_create(uint64_t owner_pid);
void ipc_port_destroy(or_cap_t port_cap);

// Message sending/receiving
int ipc_send_message(or_cap_t port, const void *data,
                     size_t size, uint64_t timeout_ns);
int ipc_recv_message(or_cap_t port, void *buffer,
                     size_t buffer_size, uint64_t timeout_ns);

// Advanced IPC
int ipc_send_capabilities(or_cap_t port, or_cap_t *caps,
                          size_t caps_count, uint64_t timeout_ns);
int ipc_send_page(or_cap_t port, uint64_t page_addr,
                  uint64_t flags, uint64_t timeout_ns);

// Notifications asynchrones
int ipc_notify(or_cap_t port, uint64_t event_mask);
int ipc_wait_notification(or_cap_t port, uint64_t *event_mask,
                          uint64_t timeout_ns);

#endif // ORION_IPC_H
