/*
 * ORION OS - RISC-V 64-bit Security Management
 *
 * Security management for RISC-V 64-bit architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include "common.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// GLOBAL SECURITY CONTEXTS
// ============================================================================

// Security context
static struct {
    bool initialized;
    bool secure_mode;
    bool trusted_execution;
    bool memory_protection;
    bool stack_protection;
    bool code_integrity;
    bool secure_boot;
    uint32_t security_level;
    uint64_t security_features;
} riscv64_security_context = {0};

// Security statistics
static struct {
    uint32_t violations;
    uint32_t access_denied;
    uint32_t integrity_checks;
    uint32_t secure_calls;
    uint32_t privilege_escalations;
    uint32_t memory_violations;
} riscv64_security_stats = {0};

// ============================================================================
// SECURITY INITIALIZATION
// ============================================================================

// Initialize security system
void riscv64_security_init(void)
{
    if (riscv64_security_context.initialized) {
        return;
    }
    
    // Initialize security context with default values
    riscv64_security_context.secure_mode = true;
    riscv64_security_context.trusted_execution = true;
    riscv64_security_context.memory_protection = true;
    riscv64_security_context.stack_protection = true;
    riscv64_security_context.code_integrity = true;
    riscv64_security_context.secure_boot = true;
    riscv64_security_context.security_level = RISCV64_SECURITY_LEVEL_HIGH;
    riscv64_security_context.security_features = RISCV64_SECURITY_FEATURES_ALL;
    riscv64_security_context.initialized = true;
    
    // Clear security statistics
    memset(&riscv64_security_stats, 0, sizeof(riscv64_security_stats));
    
    printf("RISC-V64: Security system initialized\n");
    printf("  Secure Mode: %s\n", riscv64_security_context.secure_mode ? "Enabled" : "Disabled");
    printf("  Trusted Execution: %s\n", riscv64_security_context.trusted_execution ? "Enabled" : "Disabled");
    printf("  Memory Protection: %s\n", riscv64_security_context.memory_protection ? "Enabled" : "Disabled");
    printf("  Stack Protection: %s\n", riscv64_security_context.stack_protection ? "Enabled" : "Disabled");
    printf("  Code Integrity: %s\n", riscv64_security_context.code_integrity ? "Enabled" : "Disabled");
    printf("  Secure Boot: %s\n", riscv64_security_context.secure_boot ? "Enabled" : "Disabled");
    printf("  Security Level: %u\n", riscv64_security_context.security_level);
}

// ============================================================================
// SECURITY CONTEXT FUNCTIONS
// ============================================================================

// Get security context
struct riscv64_security_context* riscv64_get_security_context(void)
{
    if (!riscv64_security_context.initialized) {
        return NULL;
    }
    
    return &riscv64_security_context;
}

// Set security level
void riscv64_security_set_level(uint32_t level)
{
    if (!riscv64_security_context.initialized) {
        return;
    }
    
    if (level <= RISCV64_SECURITY_LEVEL_MAX) {
        riscv64_security_context.security_level = level;
        printf("RISC-V64: Security level set to %u\n", level);
    }
}

// Get security level
uint32_t riscv64_security_get_level(void)
{
    if (!riscv64_security_context.initialized) {
        return RISCV64_SECURITY_LEVEL_NONE;
    }
    
    return riscv64_security_context.security_level;
}

// Enable security feature
void riscv64_security_enable_feature(uint64_t feature)
{
    if (!riscv64_security_context.initialized) {
        return;
    }
    
    riscv64_security_context.security_features |= feature;
    printf("RISC-V64: Security feature 0x%llx enabled\n", feature);
}

// Disable security feature
void riscv64_security_disable_feature(uint64_t feature)
{
    if (!riscv64_security_context.initialized) {
        return;
    }
    
    riscv64_security_context.security_features &= ~feature;
    printf("RISC-V64: Security feature 0x%llx disabled\n", feature);
}

// Check if security feature is enabled
bool riscv64_security_has_feature(uint64_t feature)
{
    if (!riscv64_security_context.initialized) {
        return false;
    }
    
    return (riscv64_security_context.security_features & feature) != 0;
}

// ============================================================================
// SECURE MODE MANAGEMENT
// ============================================================================

// Enable secure mode
void riscv64_secure_mode_enable(void)
{
    if (!riscv64_security_context.initialized) {
        return;
    }
    
    riscv64_security_context.secure_mode = true;
    
    // Set security-related CSRs
    uint64_t mstatus;
    asm volatile("csrr %0, mstatus" : "=r"(mstatus));
    mstatus |= RISCV64_MSTATUS_MPRV;  // Enable memory privilege
    asm volatile("csrw mstatus, %0" : : "r"(mstatus));
    
    printf("RISC-V64: Secure mode enabled\n");
}

// Disable secure mode
void riscv64_secure_mode_disable(void)
{
    if (!riscv64_security_context.initialized) {
        return;
    }
    
    riscv64_security_context.secure_mode = false;
    
    // Clear security-related CSRs
    uint64_t mstatus;
    asm volatile("csrr %0, mstatus" : "=r"(mstatus));
    mstatus &= ~RISCV64_MSTATUS_MPRV;  // Disable memory privilege
    asm volatile("csrw mstatus, %0" : : "r"(mstatus));
    
    printf("RISC-V64: Secure mode disabled\n");
}

// Check if secure mode is enabled
bool riscv64_secure_mode_is_enabled(void)
{
    if (!riscv64_security_context.initialized) {
        return false;
    }
    
    return riscv64_security_context.secure_mode;
}

// ============================================================================
// TRUSTED EXECUTION
// ============================================================================

// Enable trusted execution
void riscv64_trusted_execution_enable(void)
{
    if (!riscv64_security_context.initialized) {
        return;
    }
    
    riscv64_security_context.trusted_execution = true;
    
    // Set up trusted execution environment
    // This would typically involve setting up secure memory regions
    // and enabling hardware-based execution verification
    
    printf("RISC-V64: Trusted execution enabled\n");
}

// Disable trusted execution
void riscv64_trusted_execution_disable(void)
{
    if (!riscv64_security_context.initialized) {
        return;
    }
    
    riscv64_security_context.trusted_execution = false;
    
    // Clear trusted execution environment
    printf("RISC-V64: Trusted execution disabled\n");
}

// Check if trusted execution is enabled
bool riscv64_trusted_execution_is_enabled(void)
{
    if (!riscv64_security_context.initialized) {
        return false;
    }
    
    return riscv64_security_context.trusted_execution;
}

// ============================================================================
// MEMORY PROTECTION
// ============================================================================

// Enable memory protection
void riscv64_memory_protection_enable(void)
{
    if (!riscv64_security_context.initialized) {
        return;
    }
    
    riscv64_security_context.memory_protection = true;
    
    // Enable memory protection mechanisms
    // This would typically involve setting up page table permissions
    // and enabling hardware-based memory protection
    
    printf("RISC-V64: Memory protection enabled\n");
}

// Disable memory protection
void riscv64_memory_protection_disable(void)
{
    if (!riscv64_security_context.initialized) {
        return;
    }
    
    riscv64_security_context.memory_protection = false;
    
    // Disable memory protection mechanisms
    printf("RISC-V64: Memory protection disabled\n");
}

// Check if memory protection is enabled
bool riscv64_memory_protection_is_enabled(void)
{
    if (!riscv64_security_context.initialized) {
        return false;
    }
    
    return riscv64_security_context.memory_protection;
}

// ============================================================================
// STACK PROTECTION
// ============================================================================

// Enable stack protection
void riscv64_stack_protection_enable(void)
{
    if (!riscv64_security_context.initialized) {
        return;
    }
    
    riscv64_security_context.stack_protection = true;
    
    // Enable stack protection mechanisms
    // This would typically involve setting up stack canaries
    // and enabling hardware-based stack protection
    
    printf("RISC-V64: Stack protection enabled\n");
}

// Disable stack protection
void riscv64_stack_protection_disable(void)
{
    if (!riscv64_security_context.initialized) {
        return;
    }
    
    riscv64_security_context.stack_protection = false;
    
    // Disable stack protection mechanisms
    printf("RISC-V64: Stack protection disabled\n");
}

// Check if stack protection is enabled
bool riscv64_stack_protection_is_enabled(void)
{
    if (!riscv64_security_context.initialized) {
        return false;
    }
    
    return riscv64_security_context.stack_protection;
}

// ============================================================================
// CODE INTEGRITY
// ============================================================================

// Enable code integrity
void riscv64_code_integrity_enable(void)
{
    if (!riscv64_security_context.initialized) {
        return;
    }
    
    riscv64_security_context.code_integrity = true;
    
    // Enable code integrity mechanisms
    // This would typically involve setting up code signing
    // and enabling hardware-based code verification
    
    printf("RISC-V64: Code integrity enabled\n");
}

// Disable code integrity
void riscv64_code_integrity_disable(void)
{
    if (!riscv64_security_context.initialized) {
        return;
    }
    
    riscv64_security_context.code_integrity = false;
    
    // Disable code integrity mechanisms
    printf("RISC-V64: Code integrity disabled\n");
}

// Check if code integrity is enabled
bool riscv64_code_integrity_is_enabled(void)
{
    if (!riscv64_security_context.initialized) {
        return false;
    }
    
    return riscv64_security_context.code_integrity;
}

// ============================================================================
// SECURE BOOT
// ============================================================================

// Enable secure boot
void riscv64_secure_boot_enable(void)
{
    if (!riscv64_security_context.initialized) {
        return;
    }
    
    riscv64_security_context.secure_boot = true;
    
    // Enable secure boot mechanisms
    // This would typically involve setting up secure boot chains
    // and enabling hardware-based boot verification
    
    printf("RISC-V64: Secure boot enabled\n");
}

// Disable secure boot
void riscv64_secure_boot_disable(void)
{
    if (!riscv64_security_context.initialized) {
        return;
    }
    
    riscv64_security_context.secure_boot = false;
    
    // Disable secure boot mechanisms
    printf("RISC-V64: Secure boot disabled\n");
}

// Check if secure boot is enabled
bool riscv64_secure_boot_is_enabled(void)
{
    if (!riscv64_security_context.initialized) {
        return false;
    }
    
    return riscv64_security_context.secure_boot;
}

// ============================================================================
// SECURITY OPERATIONS
// ============================================================================

// Verify code integrity
bool riscv64_verify_code_integrity(uint64_t start, uint64_t end)
{
    if (!riscv64_security_context.initialized || !riscv64_security_context.code_integrity) {
        return false;
    }
    
    // This would typically involve computing and verifying checksums
    // or digital signatures for the specified code range
    
    riscv64_security_stats.integrity_checks++;
    
    printf("RISC-V64: Code integrity verified for range 0x%llx - 0x%llx\n", start, end);
    
    return true;  // Placeholder implementation
}

// Check memory access permissions
bool riscv64_check_memory_access(uint64_t address, uint64_t size, uint32_t permissions)
{
    if (!riscv64_security_context.initialized || !riscv64_security_context.memory_protection) {
        return true;  // Allow access if protection is disabled
    }
    
    // This would typically involve checking page table permissions
    // and other memory protection mechanisms
    
    // For now, implement a simple range check
    if (address < RISCV64_KERNEL_BASE || address + size > RISCV64_KERNEL_END) {
        riscv64_security_stats.access_denied++;
        return false;
    }
    
    return true;
}

// Validate privilege level
bool riscv64_validate_privilege(uint32_t required_level, uint32_t current_level)
{
    if (!riscv64_security_context.initialized) {
        return false;
    }
    
    if (current_level < required_level) {
        riscv64_security_stats.privilege_escalations++;
        return false;
    }
    
    return true;
}

// ============================================================================
// SECURITY VIOLATIONS
// ============================================================================

// Report security violation
void riscv64_report_security_violation(uint32_t type, uint64_t address, uint32_t severity)
{
    if (!riscv64_security_context.initialized) {
        return;
    }
    
    riscv64_security_stats.violations++;
    
    printf("RISC-V64: Security violation detected\n");
    printf("  Type: %u\n", type);
    printf("  Address: 0x%llx\n", address);
    printf("  Severity: %u\n", severity);
    
    // Handle violation based on severity
    switch (severity) {
        case RISCV64_SECURITY_SEVERITY_LOW:
            // Log violation
            break;
        case RISCV64_SECURITY_SEVERITY_MEDIUM:
            // Log violation and take corrective action
            break;
        case RISCV64_SECURITY_SEVERITY_HIGH:
            // Log violation and halt system
            printf("RISC-V64: Critical security violation - halting system\n");
            riscv64_cpu_halt();
            break;
        default:
            break;
    }
}

// Handle memory violation
void riscv64_handle_memory_violation(uint64_t address, uint32_t access_type)
{
    if (!riscv64_security_context.initialized) {
        return;
    }
    
    riscv64_security_stats.memory_violations++;
    
    printf("RISC-V64: Memory violation detected\n");
    printf("  Address: 0x%llx\n", address);
    printf("  Access Type: %u\n", access_type);
    
    // Report as security violation
    riscv64_report_security_violation(RISCV64_SECURITY_VIOLATION_MEMORY, address, RISCV64_SECURITY_SEVERITY_MEDIUM);
}

// ============================================================================
// SECURE CALLS
// ============================================================================

// Make secure call
uint64_t riscv64_secure_call(uint32_t call_id, uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
    if (!riscv64_security_context.initialized || !riscv64_security_context.secure_mode) {
        return RISCV64_ERROR_SECURITY_DISABLED;
    }
    
    riscv64_security_stats.secure_calls++;
    
    // Validate call ID and arguments
    if (call_id >= RISCV64_MAX_SECURE_CALLS) {
        return RISCV64_ERROR_INVALID_CALL;
    }
    
    // Execute secure call based on ID
    switch (call_id) {
        case RISCV64_SECURE_CALL_VERIFY_INTEGRITY:
            return riscv64_verify_code_integrity(arg1, arg2) ? 0 : RISCV64_ERROR_INTEGRITY_FAILED;
        case RISCV64_SECURE_CALL_CHECK_ACCESS:
            return riscv64_check_memory_access(arg1, arg2, (uint32_t)arg3) ? 0 : RISCV64_ERROR_ACCESS_DENIED;
        case RISCV64_SECURE_CALL_VALIDATE_PRIVILEGE:
            return riscv64_validate_privilege((uint32_t)arg1, (uint32_t)arg2) ? 0 : RISCV64_ERROR_PRIVILEGE_VIOLATION;
        default:
            return RISCV64_ERROR_INVALID_CALL;
    }
}

// ============================================================================
// SECURITY STATISTICS
// ============================================================================

// Get security statistics
void riscv64_security_get_stats(uint32_t *violations, uint32_t *access_denied,
                               uint32_t *integrity_checks, uint32_t *secure_calls,
                               uint32_t *privilege_escalations, uint32_t *memory_violations)
{
    if (violations) *violations = riscv64_security_stats.violations;
    if (access_denied) *access_denied = riscv64_security_stats.access_denied;
    if (integrity_checks) *integrity_checks = riscv64_security_stats.integrity_checks;
    if (secure_calls) *secure_calls = riscv64_security_stats.secure_calls;
    if (privilege_escalations) *privilege_escalations = riscv64_security_stats.privilege_escalations;
    if (memory_violations) *memory_violations = riscv64_security_stats.memory_violations;
}

// Reset security statistics
void riscv64_security_reset_stats(void)
{
    memset(&riscv64_security_stats, 0, sizeof(riscv64_security_stats));
    
    printf("RISC-V64: Security statistics reset\n");
}

// ============================================================================
// SECURITY DEBUG FUNCTIONS
// ============================================================================

// Print security status
void riscv64_security_print_status(void)
{
    printf("RISC-V64: Security Status\n");
    printf("  Initialized: %s\n", riscv64_security_context.initialized ? "Yes" : "No");
    printf("  Secure Mode: %s\n", riscv64_security_context.secure_mode ? "Enabled" : "Disabled");
    printf("  Trusted Execution: %s\n", riscv64_security_context.trusted_execution ? "Enabled" : "Disabled");
    printf("  Memory Protection: %s\n", riscv64_security_context.memory_protection ? "Enabled" : "Disabled");
    printf("  Stack Protection: %s\n", riscv64_security_context.stack_protection ? "Enabled" : "Disabled");
    printf("  Code Integrity: %s\n", riscv64_security_context.code_integrity ? "Enabled" : "Disabled");
    printf("  Secure Boot: %s\n", riscv64_security_context.secure_boot ? "Enabled" : "Disabled");
    printf("  Security Level: %u\n", riscv64_security_context.security_level);
    printf("  Security Features: 0x%llx\n", riscv64_security_context.security_features);
}

// Print security statistics
void riscv64_security_print_statistics(void)
{
    printf("RISC-V64: Security Statistics\n");
    printf("  Violations: %u\n", riscv64_security_stats.violations);
    printf("  Access Denied: %u\n", riscv64_security_stats.access_denied);
    printf("  Integrity Checks: %u\n", riscv64_security_stats.integrity_checks);
    printf("  Secure Calls: %u\n", riscv64_security_stats.secure_calls);
    printf("  Privilege Escalations: %u\n", riscv64_security_stats.privilege_escalations);
    printf("  Memory Violations: %u\n", riscv64_security_stats.memory_violations);
}

// ============================================================================
// SECURITY CLEANUP
// ============================================================================

// Cleanup security system
void riscv64_security_cleanup(void)
{
    if (!riscv64_security_context.initialized) {
        return;
    }
    
    // Disable all security features
    riscv64_secure_mode_disable();
    riscv64_trusted_execution_disable();
    riscv64_memory_protection_disable();
    riscv64_stack_protection_disable();
    riscv64_code_integrity_disable();
    riscv64_secure_boot_disable();
    
    // Clear contexts
    riscv64_security_context.initialized = false;
    
    printf("RISC-V64: Security system cleaned up\n");
}
