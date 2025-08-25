/*
 * ORION OS - RISC-V 64-bit Error Management
 *
 * Error management for RISC-V 64-bit architecture
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
// GLOBAL ERROR CONTEXTS
// ============================================================================

// Error context
static struct {
    bool initialized;
    bool error_reporting_enabled;
    bool error_recovery_enabled;
    bool error_logging_enabled;
    uint32_t max_error_count;
    uint32_t error_severity_level;
    uint64_t error_handlers;
} riscv64_error_context = {0};

// Error statistics
static struct {
    uint32_t total_errors;
    uint32_t critical_errors;
    uint32_t recoverable_errors;
    uint32_t non_recoverable_errors;
    uint32_t error_recovery_attempts;
    uint32_t successful_recoveries;
    uint32_t failed_recoveries;
    uint32_t error_timeouts;
} riscv64_error_stats = {0};

// Error log
static struct {
    uint32_t log_size;
    uint32_t log_index;
    uint32_t max_log_entries;
    struct riscv64_error_entry *entries;
} riscv64_error_log = {0};

// ============================================================================
// ERROR INITIALIZATION
// ============================================================================

// Initialize error management system
void riscv64_error_init(void)
{
    if (riscv64_error_context.initialized) {
        return;
    }
    
    // Initialize error context with default values
    riscv64_error_context.error_reporting_enabled = true;
    riscv64_error_context.error_recovery_enabled = true;
    riscv64_error_context.error_logging_enabled = true;
    riscv64_error_context.max_error_count = RISCV64_MAX_ERROR_COUNT;
    riscv64_error_context.error_severity_level = RISCV64_ERROR_SEVERITY_MEDIUM;
    riscv64_error_context.error_handlers = 0;
    riscv64_error_context.initialized = true;
    
    // Clear error statistics
    memset(&riscv64_error_stats, 0, sizeof(riscv64_error_stats));
    
    // Initialize error log
    riscv64_error_log.max_log_entries = RISCV64_MAX_ERROR_LOG_ENTRIES;
    riscv64_error_log.log_size = 0;
    riscv64_error_log.log_index = 0;
    riscv64_error_log.entries = NULL;  // Will be allocated when needed
    
    printf("RISC-V64: Error management system initialized\n");
    printf("  Error Reporting: %s\n", riscv64_error_context.error_reporting_enabled ? "Enabled" : "Disabled");
    printf("  Error Recovery: %s\n", riscv64_error_context.error_recovery_enabled ? "Enabled" : "Disabled");
    printf("  Error Logging: %s\n", riscv64_error_context.error_logging_enabled ? "Enabled" : "Disabled");
    printf("  Max Error Count: %u\n", riscv64_error_context.max_error_count);
    printf("  Error Severity Level: %u\n", riscv64_error_context.error_severity_level);
}

// ============================================================================
// ERROR CONTEXT FUNCTIONS
// ============================================================================

// Get error context
struct riscv64_error_context* riscv64_get_error_context(void)
{
    if (!riscv64_error_context.initialized) {
        return NULL;
    }
    
    return &riscv64_error_context;
}

// Enable error reporting
void riscv64_error_reporting_enable(void)
{
    if (!riscv64_error_context.initialized) {
        return;
    }
    
    riscv64_error_context.error_reporting_enabled = true;
    
    printf("RISC-V64: Error reporting enabled\n");
}

// Disable error reporting
void riscv64_error_reporting_disable(void)
{
    if (!riscv64_error_context.initialized) {
        return;
    }
    
    riscv64_error_context.error_reporting_enabled = false;
    
    printf("RISC-V64: Error reporting disabled\n");
}

// Check if error reporting is enabled
bool riscv64_error_reporting_is_enabled(void)
{
    if (!riscv64_error_context.initialized) {
        return false;
    }
    
    return riscv64_error_context.error_reporting_enabled;
}

// Enable error recovery
void riscv64_error_recovery_enable(void)
{
    if (!riscv64_error_context.initialized) {
        return;
    }
    
    riscv64_error_context.error_recovery_enabled = true;
    
    printf("RISC-V64: Error recovery enabled\n");
}

// Disable error recovery
void riscv64_error_recovery_disable(void)
{
    if (!riscv64_error_context.initialized) {
        return;
    }
    
    riscv64_error_context.error_recovery_enabled = false;
    
    printf("RISC-V64: Error recovery disabled\n");
}

// Check if error recovery is enabled
bool riscv64_error_recovery_is_enabled(void)
{
    if (!riscv64_error_context.initialized) {
        return false;
    }
    
    return riscv64_error_context.error_recovery_enabled;
}

// Enable error logging
void riscv64_error_logging_enable(void)
{
    if (!riscv64_error_context.initialized) {
        return;
    }
    
    riscv64_error_context.error_logging_enabled = true;
    
    printf("RISC-V64: Error logging enabled\n");
}

// Disable error logging
void riscv64_error_logging_disable(void)
{
    if (!riscv64_error_context.initialized) {
        return;
    }
    
    riscv64_error_context.error_logging_enabled = false;
    
    printf("RISC-V64: Error logging disabled\n");
}

// Check if error logging is enabled
bool riscv64_error_logging_is_enabled(void)
{
    if (!riscv64_error_context.initialized) {
        return false;
    }
    
    return riscv64_error_context.error_logging_enabled;
}

// Set error severity level
void riscv64_error_set_severity_level(uint32_t level)
{
    if (!riscv64_error_context.initialized) {
        return;
    }
    
    if (level <= RISCV64_ERROR_SEVERITY_MAX) {
        riscv64_error_context.error_severity_level = level;
        printf("RISC-V64: Error severity level set to %u\n", level);
    }
}

// Get error severity level
uint32_t riscv64_error_get_severity_level(void)
{
    if (!riscv64_error_context.initialized) {
        return RISCV64_ERROR_SEVERITY_NONE;
    }
    
    return riscv64_error_context.error_severity_level;
}

// ============================================================================
// ERROR HANDLING
// ============================================================================

// Report error
void riscv64_error_report(uint32_t error_type, uint32_t error_code, uint64_t error_address, uint32_t severity)
{
    if (!riscv64_error_context.initialized) {
        return;
    }
    
    // Update error statistics
    riscv64_error_stats.total_errors++;
    
    switch (severity) {
        case RISCV64_ERROR_SEVERITY_LOW:
            // Log low severity error
            break;
        case RISCV64_ERROR_SEVERITY_MEDIUM:
            riscv64_error_stats.recoverable_errors++;
            break;
        case RISCV64_ERROR_SEVERITY_HIGH:
            riscv64_error_stats.critical_errors++;
            break;
        case RISCV64_ERROR_SEVERITY_CRITICAL:
            riscv64_error_stats.non_recoverable_errors++;
            break;
        default:
            break;
    }
    
    // Log error if logging is enabled
    if (riscv64_error_context.error_logging_enabled) {
        riscv64_error_log_entry(error_type, error_code, error_address, severity);
    }
    
    // Report error if reporting is enabled
    if (riscv64_error_context.error_reporting_enabled) {
        printf("RISC-V64: Error reported\n");
        printf("  Type: %u\n", error_type);
        printf("  Code: %u\n", error_code);
        printf("  Address: 0x%llx\n", error_address);
        printf("  Severity: %u\n", severity);
    }
    
    // Attempt error recovery if enabled and error is recoverable
    if (riscv64_error_context.error_recovery_enabled && 
        severity <= RISCV64_ERROR_SEVERITY_HIGH) {
        riscv64_error_attempt_recovery(error_type, error_code, error_address, severity);
    }
    
    // Handle critical errors
    if (severity >= RISCV64_ERROR_SEVERITY_CRITICAL) {
        riscv64_error_handle_critical(error_type, error_code, error_address, severity);
    }
}

// Handle critical error
void riscv64_error_handle_critical(uint32_t error_type, uint32_t error_code, uint64_t error_address, uint32_t severity)
{
    if (!riscv64_error_context.initialized) {
        return;
    }
    
    printf("RISC-V64: Critical error detected - halting system\n");
    printf("  Type: %u\n", error_type);
    printf("  Code: %u\n", error_code);
    printf("  Address: 0x%llx\n", error_address);
    printf("  Severity: %u\n", severity);
    
    // Halt the system for critical errors
    riscv64_cpu_halt();
}

// Attempt error recovery
void riscv64_error_attempt_recovery(uint32_t error_type, uint32_t error_code, uint64_t error_address, uint32_t severity)
{
    if (!riscv64_error_context.initialized) {
        return;
    }
    
    riscv64_error_stats.error_recovery_attempts++;
    
    printf("RISC-V64: Attempting error recovery\n");
    printf("  Type: %u\n", error_type);
    printf("  Code: %u\n", error_code);
    printf("  Address: 0x%llx\n", error_address);
    printf("  Severity: %u\n", severity);
    
    // Attempt recovery based on error type
    bool recovery_successful = false;
    
    switch (error_type) {
        case RISCV64_ERROR_TYPE_MEMORY:
            recovery_successful = riscv64_error_recover_memory(error_code, error_address);
            break;
        case RISCV64_ERROR_TYPE_CPU:
            recovery_successful = riscv64_error_recover_cpu(error_code, error_address);
            break;
        case RISCV64_ERROR_TYPE_CACHE:
            recovery_successful = riscv64_error_recover_cache(error_code, error_address);
            break;
        case RISCV64_ERROR_TYPE_INTERRUPT:
            recovery_successful = riscv64_error_recover_interrupt(error_code, error_address);
            break;
        case RISCV64_ERROR_TYPE_TIMER:
            recovery_successful = riscv64_error_recover_timer(error_code, error_address);
            break;
        default:
            recovery_successful = riscv64_error_recover_generic(error_code, error_address);
            break;
    }
    
    if (recovery_successful) {
        riscv64_error_stats.successful_recoveries++;
        printf("RISC-V64: Error recovery successful\n");
    } else {
        riscv64_error_stats.failed_recoveries++;
        printf("RISC-V64: Error recovery failed\n");
    }
}

// ============================================================================
// ERROR RECOVERY FUNCTIONS
// ============================================================================

// Recover from memory error
bool riscv64_error_recover_memory(uint32_t error_code, uint64_t error_address)
{
    // Attempt memory error recovery
    // This might involve invalidating cache lines, remapping memory, etc.
    
    printf("RISC-V64: Attempting memory error recovery\n");
    printf("  Error Code: %u\n", error_code);
    printf("  Error Address: 0x%llx\n", error_address);
    
    // For now, return success (placeholder implementation)
    return true;
}

// Recover from CPU error
bool riscv64_error_recover_cpu(uint32_t error_code, uint64_t error_address)
{
    // Attempt CPU error recovery
    // This might involve resetting CPU state, clearing registers, etc.
    
    printf("RISC-V64: Attempting CPU error recovery\n");
    printf("  Error Code: %u\n", error_code);
    printf("  Error Address: 0x%llx\n", error_address);
    
    // For now, return success (placeholder implementation)
    return true;
}

// Recover from cache error
bool riscv64_error_recover_cache(uint32_t error_code, uint64_t error_address)
{
    // Attempt cache error recovery
    // This might involve invalidating cache lines, clearing cache, etc.
    
    printf("RISC-V64: Attempting cache error recovery\n");
    printf("  Error Code: %u\n", error_code);
    printf("  Error Address: 0x%llx\n", error_address);
    
    // Invalidate cache lines around the error address
    if (riscv64_cache_context.initialized) {
        riscv64_dcache_invalidate_range(error_address - 64, error_address + 64);
    }
    
    return true;
}

// Recover from interrupt error
bool riscv64_error_recover_interrupt(uint32_t error_code, uint64_t error_address)
{
    // Attempt interrupt error recovery
    // This might involve reinitializing interrupt system, clearing interrupt flags, etc.
    
    printf("RISC-V64: Attempting interrupt error recovery\n");
    printf("  Error Code: %u\n", error_code);
    printf("  Error Address: 0x%llx\n", error_address);
    
    // For now, return success (placeholder implementation)
    return true;
}

// Recover from timer error
bool riscv64_error_recover_timer(uint32_t error_code, uint64_t error_address)
{
    // Attempt timer error recovery
    // This might involve reinitializing timer system, clearing timer flags, etc.
    
    printf("RISC-V64: Attempting timer error recovery\n");
    printf("  Error Code: %u\n", error_code);
    printf("  Error Address: 0x%llx\n", error_address);
    
    // For now, return success (placeholder implementation)
    return true;
}

// Recover from generic error
bool riscv64_error_recover_generic(uint32_t error_code, uint64_t error_address)
{
    // Attempt generic error recovery
    // This might involve system reset, clearing error flags, etc.
    
    printf("RISC-V64: Attempting generic error recovery\n");
    printf("  Error Code: %u\n", error_code);
    printf("  Error Address: 0x%llx\n", error_address);
    
    // For now, return success (placeholder implementation)
    return true;
}

// ============================================================================
// ERROR LOGGING
// ============================================================================

// Log error entry
void riscv64_error_log_entry(uint32_t error_type, uint32_t error_code, uint64_t error_address, uint32_t severity)
{
    if (!riscv64_error_context.initialized || !riscv64_error_context.error_logging_enabled) {
        return;
    }
    
    // Allocate error log if not already allocated
    if (riscv64_error_log.entries == NULL) {
        riscv64_error_log.entries = malloc(sizeof(struct riscv64_error_entry) * riscv64_error_log.max_log_entries);
        if (riscv64_error_log.entries == NULL) {
            return;  // Cannot log error if memory allocation fails
        }
        memset(riscv64_error_log.entries, 0, sizeof(struct riscv64_error_entry) * riscv64_error_log.max_log_entries);
    }
    
    // Add error entry to log
    struct riscv64_error_entry *entry = &riscv64_error_log.entries[riscv64_error_log.log_index];
    
    entry->timestamp = riscv64_timer_read_ns();
    entry->error_type = error_type;
    entry->error_code = error_code;
    entry->error_address = error_address;
    entry->severity = severity;
    entry->cpu_id = riscv64_cpu_get_hart_id();
    entry->privilege_level = riscv64_cpu_get_mode();
    
    // Update log index (circular buffer)
    riscv64_error_log.log_index = (riscv64_error_log.log_index + 1) % riscv64_error_log.max_log_entries;
    
    // Update log size
    if (riscv64_error_log.log_size < riscv64_error_log.max_log_entries) {
        riscv64_error_log.log_size++;
    }
}

// Get error log entry
struct riscv64_error_entry* riscv64_error_get_log_entry(uint32_t index)
{
    if (!riscv64_error_context.initialized || !riscv64_error_context.error_logging_enabled) {
        return NULL;
    }
    
    if (riscv64_error_log.entries == NULL || index >= riscv64_error_log.log_size) {
        return NULL;
    }
    
    return &riscv64_error_log.entries[index];
}

// Clear error log
void riscv64_error_clear_log(void)
{
    if (!riscv64_error_context.initialized) {
        return;
    }
    
    if (riscv64_error_log.entries != NULL) {
        memset(riscv64_error_log.entries, 0, sizeof(struct riscv64_error_entry) * riscv64_error_log.max_log_entries);
    }
    
    riscv64_error_log.log_size = 0;
    riscv64_error_log.log_index = 0;
    
    printf("RISC-V64: Error log cleared\n");
}

// ============================================================================
// ERROR STATISTICS
// ============================================================================

// Get error statistics
void riscv64_error_get_stats(uint32_t *total_errors, uint32_t *critical_errors,
                            uint32_t *recoverable_errors, uint32_t *non_recoverable_errors,
                            uint32_t *error_recovery_attempts, uint32_t *successful_recoveries,
                            uint32_t *failed_recoveries, uint32_t *error_timeouts)
{
    if (total_errors) *total_errors = riscv64_error_stats.total_errors;
    if (critical_errors) *critical_errors = riscv64_error_stats.critical_errors;
    if (recoverable_errors) *recoverable_errors = riscv64_error_stats.recoverable_errors;
    if (non_recoverable_errors) *non_recoverable_errors = riscv64_error_stats.non_recoverable_errors;
    if (error_recovery_attempts) *error_recovery_attempts = riscv64_error_stats.error_recovery_attempts;
    if (successful_recoveries) *successful_recoveries = riscv64_error_stats.successful_recoveries;
    if (failed_recoveries) *failed_recoveries = riscv64_error_stats.failed_recoveries;
    if (error_timeouts) *error_timeouts = riscv64_error_stats.error_timeouts;
}

// Reset error statistics
void riscv64_error_reset_stats(void)
{
    memset(&riscv64_error_stats, 0, sizeof(riscv64_error_stats));
    
    printf("RISC-V64: Error statistics reset\n");
}

// ============================================================================
// ERROR DEBUG FUNCTIONS
// ============================================================================

// Print error status
void riscv64_error_print_status(void)
{
    printf("RISC-V64: Error Status\n");
    printf("  Initialized: %s\n", riscv64_error_context.initialized ? "Yes" : "No");
    printf("  Error Reporting: %s\n", riscv64_error_context.error_reporting_enabled ? "Enabled" : "Disabled");
    printf("  Error Recovery: %s\n", riscv64_error_context.error_recovery_enabled ? "Enabled" : "Disabled");
    printf("  Error Logging: %s\n", riscv64_error_context.error_logging_enabled ? "Enabled" : "Disabled");
    printf("  Max Error Count: %u\n", riscv64_error_context.max_error_count);
    printf("  Error Severity Level: %u\n", riscv64_error_context.error_severity_level);
    printf("  Error Handlers: 0x%llx\n", riscv64_error_context.error_handlers);
}

// Print error statistics
void riscv64_error_print_statistics(void)
{
    printf("RISC-V64: Error Statistics\n");
    printf("  Total Errors: %u\n", riscv64_error_stats.total_errors);
    printf("  Critical Errors: %u\n", riscv64_error_stats.critical_errors);
    printf("  Recoverable Errors: %u\n", riscv64_error_stats.recoverable_errors);
    printf("  Non-Recoverable Errors: %u\n", riscv64_error_stats.non_recoverable_errors);
    printf("  Error Recovery Attempts: %u\n", riscv64_error_stats.error_recovery_attempts);
    printf("  Successful Recoveries: %u\n", riscv64_error_stats.successful_recoveries);
    printf("  Failed Recoveries: %u\n", riscv64_error_stats.failed_recoveries);
    printf("  Error Timeouts: %u\n", riscv64_error_stats.error_timeouts);
}

// Print error log
void riscv64_error_print_log(void)
{
    if (!riscv64_error_context.initialized || !riscv64_error_context.error_logging_enabled) {
        printf("RISC-V64: Error logging not available\n");
        return;
    }
    
    printf("RISC-V64: Error Log (%u entries)\n", riscv64_error_log.log_size);
    
    if (riscv64_error_log.entries == NULL || riscv64_error_log.log_size == 0) {
        printf("  No error entries\n");
        return;
    }
    
    for (uint32_t i = 0; i < riscv64_error_log.log_size; i++) {
        struct riscv64_error_entry *entry = &riscv64_error_log.entries[i];
        
        printf("  Entry %u:\n", i);
        printf("    Timestamp: %llu ns\n", entry->timestamp);
        printf("    Type: %u\n", entry->error_type);
        printf("    Code: %u\n", entry->error_code);
        printf("    Address: 0x%llx\n", entry->error_address);
        printf("    Severity: %u\n", entry->severity);
        printf("    CPU ID: %u\n", entry->cpu_id);
        printf("    Privilege Level: %u\n", entry->privilege_level);
    }
}

// ============================================================================
// ERROR CLEANUP
// ============================================================================

// Cleanup error management system
void riscv64_error_cleanup(void)
{
    if (!riscv64_error_context.initialized) {
        return;
    }
    
    // Free error log memory
    if (riscv64_error_log.entries != NULL) {
        free(riscv64_error_log.entries);
        riscv64_error_log.entries = NULL;
    }
    
    // Clear contexts
    riscv64_error_context.initialized = false;
    
    printf("RISC-V64: Error management system cleaned up\n");
}
