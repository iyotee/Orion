#ifndef ORION_CAPABILITIES_H
#define ORION_CAPABILITIES_H

#include <orion/types.h>

// Object types for capabilities
typedef enum
{
    CAP_TYPE_NONE = 0,
    CAP_TYPE_MEMORY,
    CAP_TYPE_IPC_PORT,
    CAP_TYPE_PROCESS,
    CAP_TYPE_THREAD,
    CAP_TYPE_FILE,
    CAP_TYPE_DEVICE,
    CAP_TYPE_TIMER,
} cap_type_t;

// Capability rights
#define CAP_RIGHT_READ (1ULL << 0)
#define CAP_RIGHT_WRITE (1ULL << 1)
#define CAP_RIGHT_EXEC (1ULL << 2)
#define CAP_RIGHT_GRANT (1ULL << 3)
#define CAP_RIGHT_REVOKE (1ULL << 4)
#define CAP_RIGHT_DELETE (1ULL << 5)
#define CAP_RIGHT_SEND (1ULL << 6)  // For IPC
#define CAP_RIGHT_RECV (1ULL << 7)  // For IPC
#define CAP_RIGHT_MAP (1ULL << 8)   // For memory
#define CAP_RIGHT_UNMAP (1ULL << 9) // For memory

// Capability system functions
void capabilities_init(void);

// Create a new capability
or_cap_t cap_create(cap_type_t type, uint64_t object_id,
                    uint64_t rights, uint64_t owner_pid);

// Check capability rights
bool cap_check_rights(or_cap_t cap_id, uint64_t required_rights,
                      uint64_t caller_pid);

// Grant/revoke rights
int cap_grant(or_cap_t cap_id, uint64_t target_pid,
              uint64_t rights, uint64_t caller_pid);
int cap_revoke(or_cap_t cap_id, uint64_t target_pid,
               uint64_t rights, uint64_t caller_pid);

// Destroy a capability
void cap_destroy(or_cap_t cap_id);

// Macros for common checks
#define CAP_CHECK_READ(cap, pid) \
    cap_check_rights(cap, CAP_RIGHT_READ, pid)

#define CAP_CHECK_WRITE(cap, pid) \
    cap_check_rights(cap, CAP_RIGHT_WRITE, pid)

#define CAP_CHECK_SEND(cap, pid) \
    cap_check_rights(cap, CAP_RIGHT_SEND, pid)

#define CAP_CHECK_RECV(cap, pid) \
    cap_check_rights(cap, CAP_RIGHT_RECV, pid)

#endif // ORION_CAPABILITIES_H
