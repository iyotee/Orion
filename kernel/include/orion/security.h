/*
 * Orion Operating System - Security System Header
 *
 * Advanced security framework with capability-based access control,
 * hardware security features, and real-time threat detection.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_SECURITY_H
#define ORION_SECURITY_H

#include <orion/types.h>

// ========================================
// CAPABILITY TYPES AND CONSTANTS
// ========================================

// Capability rights flags
#define CAP_READ (1ULL << 0)         // Read access
#define CAP_WRITE (1ULL << 1)        // Write access
#define CAP_EXEC (1ULL << 2)         // Execute access
#define CAP_GRANT (1ULL << 3)        // Grant rights to others
#define CAP_REVOKE (1ULL << 4)       // Revoke rights from others
#define CAP_DELETE (1ULL << 5)       // Delete object
#define CAP_CREATE (1ULL << 6)       // Create new objects
#define CAP_MODIFY (1ULL << 7)       // Modify metadata
#define CAP_TRAVERSE (1ULL << 8)     // Traverse directories
#define CAP_BIND (1ULL << 9)         // Bind network sockets
#define CAP_LISTEN (1ULL << 10)      // Listen on network
#define CAP_CONNECT (1ULL << 11)     // Connect to network
#define CAP_DEBUG (1ULL << 12)       // Debug processes
#define CAP_ADMIN (1ULL << 13)       // Administrative access
#define CAP_IMMORTAL (1ULL << 14)    // Cannot be revoked
#define CAP_DELEGATABLE (1ULL << 15) // Can be delegated

// Capability object types
typedef enum
{
    CAP_TYPE_NONE = 0,
    CAP_TYPE_MEMORY,
    CAP_TYPE_IPC_PORT,
    CAP_TYPE_PROCESS,
    CAP_TYPE_THREAD,
    CAP_TYPE_FILE,
    CAP_TYPE_DIRECTORY,
    CAP_TYPE_DEVICE,
    CAP_TYPE_NETWORK_SOCKET,
    CAP_TYPE_TIMER,
    CAP_TYPE_SECURITY_CONTEXT,
    CAP_TYPE_CRYPTO_KEY,
    CAP_TYPE_HARDWARE_RESOURCE,
} cap_type_t;

// Security levels
typedef enum
{
    SECURITY_LEVEL_PUBLIC = 0,   // Public access
    SECURITY_LEVEL_RESTRICTED,   // Restricted access
    SECURITY_LEVEL_CONFIDENTIAL, // Confidential
    SECURITY_LEVEL_SECRET,       // Secret
    SECURITY_LEVEL_TOP_SECRET,   // Top secret
} security_level_t;

// ========================================
// FUNCTION DECLARATIONS
// ========================================

// Core capability management
void capabilities_init(void);
or_cap_t cap_create(cap_type_t type, uint64_t object_id, uint64_t rights, uint64_t owner_pid);
bool cap_check_rights(or_cap_t cap_id, uint64_t required_rights, uint64_t caller_pid);
int cap_grant(or_cap_t cap_id, uint64_t target_pid, uint64_t rights, uint64_t caller_pid);
int cap_revoke(or_cap_t cap_id, uint64_t target_pid, uint64_t rights, uint64_t caller_pid);
void cap_destroy(or_cap_t cap_id);

// Hardware security features
void security_init(void);
void security_init_kaslr(void);
uint64_t security_get_kaslr_offset(void);
void security_init_stack_canary(void);
uint64_t security_get_stack_canary(void);
bool security_check_stack_canary(uint64_t canary);
void security_init_cfi(void);
bool security_register_cfi_target(uint64_t source, uint64_t target);
bool security_validate_cfi_jump(uint64_t source, uint64_t target);

// Memory protection
bool security_check_wx_violation(uint64_t vaddr, uint64_t flags);
bool security_is_address_valid(uint64_t vaddr, uint64_t size, bool write);
bool security_check_memory_limit(uint64_t pid, uint64_t requested_bytes);

// Policy enforcement
bool security_check_syscall_allowed(uint64_t syscall_num, uint64_t pid);

// Random number generation and entropy
uint64_t security_get_random(void);
void security_add_entropy(uint64_t data);

// Violation reporting and monitoring
void security_report_violation(const char *type, uint64_t severity,
                               uint64_t pid, const char *details);
void security_get_stats(uint64_t *capabilities_active, uint64_t *violations_total,
                        uint64_t *audit_entries, bool *alert_mode);

// ========================================
// ARCHITECTURE-SPECIFIC FUNCTIONS
// ========================================

// These functions must be implemented by each architecture

// Random number generation
uint64_t arch_get_rdtsc(void); // Read timestamp counter
// arch_get_timestamp declared in scheduler.h

// Hardware security features
void arch_enable_smep(void); // Enable SMEP if available
void arch_enable_smap(void); // Enable SMAP if available
void arch_enable_umip(void); // Enable UMIP if available

// Address validation
bool arch_validate_user_address(uint64_t vaddr, size_t size, bool write);

#endif // ORION_SECURITY_H
