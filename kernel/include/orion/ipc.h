#ifndef ORION_IPC_H
#define ORION_IPC_H

#include <orion/types.h>

// Constantes IPC
#define IPC_MAX_MESSAGE_SIZE (64 * 1024) // 64KB max par message
#define IPC_MAX_CAPABILITIES 16          // Max capabilities par message
#define IPC_TIMEOUT_INFINITE 0xFFFFFFFFFFFFFFFFULL

// Flags pour messages IPC
#define IPC_FLAG_NONBLOCK (1 << 0)
#define IPC_FLAG_SYNC (1 << 1)
#define IPC_FLAG_ZEROCOPY (1 << 2)

// Types de messages IPC
typedef enum
{
    IPC_MSG_DATA = 0,   // Message avec données
    IPC_MSG_CAPABILITY, // Transfert de capability
    IPC_MSG_PAGE,       // Transfert de page (zero-copy)
    IPC_MSG_INTERRUPT,  // Notification d'interruption
} ipc_msg_type_t;

// Fonctions du système IPC
void ipc_init(void);

// Gestion des ports IPC
or_cap_t ipc_port_create(uint64_t owner_pid);
void ipc_port_destroy(or_cap_t port_cap);

// Envoi/réception de messages
int ipc_send_message(or_cap_t port, const void *data,
                     size_t size, uint64_t timeout_ns);
int ipc_recv_message(or_cap_t port, void *buffer,
                     size_t buffer_size, uint64_t timeout_ns);

// IPC avancé
int ipc_send_capabilities(or_cap_t port, or_cap_t *caps,
                          size_t caps_count, uint64_t timeout_ns);
int ipc_send_page(or_cap_t port, uint64_t page_addr,
                  uint64_t flags, uint64_t timeout_ns);

// Notifications asynchrones
int ipc_notify(or_cap_t port, uint64_t event_mask);
int ipc_wait_notification(or_cap_t port, uint64_t *event_mask,
                          uint64_t timeout_ns);

#endif // ORION_IPC_H
