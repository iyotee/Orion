/*
 * ORION OS - RISC-V 64-bit Exception Management
 *
 * Exception management for RISC-V 64-bit architecture
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
// GLOBAL EXCEPTION CONTEXTS
// ============================================================================

// Exception context
static struct
{
    bool initialized;
    bool exception_handling_enabled;
    bool exception_logging_enabled;
    bool exception_recovery_enabled;
    uint32_t max_exception_count;
    uint32_t exception_severity_level;
    uint64_t exception_handlers;
} riscv64_exception_context = {0};

// Exception statistics
static struct
{
    uint32_t total_exceptions;
    uint32_t instruction_misaligned;
    uint32_t instruction_access_fault;
    uint32_t illegal_instruction;
    uint32_t breakpoint;
    uint32_t load_address_misaligned;
    uint32_t load_access_fault;
    uint32_t store_address_misaligned;
    uint32_t store_access_fault;
    uint32_t environment_call;
    uint32_t instruction_page_fault;
    uint32_t load_page_fault;
    uint32_t store_page_fault;
    uint32_t exceptions_recovered;
    uint32_t exceptions_failed;
} riscv64_exception_stats = {0};

// Exception log
static struct
{
    uint32_t log_size;
    uint32_t log_index;
    uint32_t max_log_entries;
    struct riscv64_exception_entry *entries;
} riscv64_exception_log = {0};

// ============================================================================
// EXCEPTION INITIALIZATION
// ============================================================================

// Initialize exception management system
void riscv64_exception_init(void)
{
    if (riscv64_exception_context.initialized)
    {
        return;
    }

    // Initialize exception context with default values
    riscv64_exception_context.exception_handling_enabled = true;
    riscv64_exception_context.exception_logging_enabled = true;
    riscv64_exception_context.exception_recovery_enabled = true;
    riscv64_exception_context.max_exception_count = RISCV64_MAX_EXCEPTION_COUNT;
    riscv64_exception_context.exception_severity_level = RISCV64_EXCEPTION_SEVERITY_MEDIUM;
    riscv64_exception_context.exception_handlers = 0;
    riscv64_exception_context.initialized = true;

    // Clear exception statistics
    memset(&riscv64_exception_stats, 0, sizeof(riscv64_exception_stats));

    // Initialize exception log
    riscv64_exception_log.max_log_entries = RISCV64_MAX_EXCEPTION_LOG_ENTRIES;
    riscv64_exception_log.log_size = 0;
    riscv64_exception_log.log_index = 0;
    riscv64_exception_log.entries = NULL; // Will be allocated when needed

    printf("RISC-V64: Exception management system initialized\n");
    printf("  Exception Handling: %s\n", riscv64_exception_context.exception_handling_enabled ? "Enabled" : "Disabled");
    printf("  Exception Logging: %s\n", riscv64_exception_context.exception_logging_enabled ? "Enabled" : "Disabled");
    printf("  Exception Recovery: %s\n", riscv64_exception_context.exception_recovery_enabled ? "Enabled" : "Disabled");
    printf("  Max Exception Count: %u\n", riscv64_exception_context.max_exception_count);
    printf("  Exception Severity Level: %u\n", riscv64_exception_context.exception_severity_level);
}

// ============================================================================
// EXCEPTION CONTEXT FUNCTIONS
// ============================================================================

// Get exception context
struct riscv64_exception_context *riscv64_get_exception_context(void)
{
    if (!riscv64_exception_context.initialized)
    {
        return NULL;
    }

    return &riscv64_exception_context;
}

// Enable exception handling
void riscv64_exception_handling_enable(void)
{
    if (!riscv64_exception_context.initialized)
    {
        return;
    }

    riscv64_exception_context.exception_handling_enabled = true;

    printf("RISC-V64: Exception handling enabled\n");
}

// Disable exception handling
void riscv64_exception_handling_disable(void)
{
    if (!riscv64_exception_context.initialized)
    {
        return;
    }

    riscv64_exception_context.exception_handling_enabled = false;

    printf("RISC-V64: Exception handling disabled\n");
}

// Check if exception handling is enabled
bool riscv64_exception_handling_is_enabled(void)
{
    if (!riscv64_exception_context.initialized)
    {
        return false;
    }

    return riscv64_exception_context.exception_handling_enabled;
}

// Enable exception logging
void riscv64_exception_logging_enable(void)
{
    if (!riscv64_exception_context.initialized)
    {
        return;
    }

    riscv64_exception_context.exception_logging_enabled = true;

    printf("RISC-V64: Exception logging enabled\n");
}

// Disable exception logging
void riscv64_exception_logging_disable(void)
{
    if (!riscv64_exception_context.initialized)
    {
        return;
    }

    riscv64_exception_context.exception_logging_enabled = false;

    printf("RISC-V64: Exception logging disabled\n");
}

// Check if exception logging is enabled
bool riscv64_exception_logging_is_enabled(void)
{
    if (!riscv64_exception_context.initialized)
    {
        return false;
    }

    return riscv64_exception_context.exception_logging_enabled;
}

// Enable exception recovery
void riscv64_exception_recovery_enable(void)
{
    if (!riscv64_exception_context.initialized)
    {
        return;
    }

    riscv64_exception_context.exception_recovery_enabled = true;

    printf("RISC-V64: Exception recovery enabled\n");
}

// Disable exception recovery
void riscv64_exception_recovery_disable(void)
{
    if (!riscv64_exception_context.initialized)
    {
        return;
    }

    riscv64_exception_context.exception_recovery_enabled = false;

    printf("RISC-V64: Exception recovery disabled\n");
}

// Check if exception recovery is enabled
bool riscv64_exception_recovery_is_enabled(void)
{
    if (!riscv64_exception_context.initialized)
    {
        return false;
    }

    return riscv64_exception_context.exception_recovery_enabled;
}

// Set exception severity level
void riscv64_exception_set_severity_level(uint32_t level)
{
    if (!riscv64_exception_context.initialized)
    {
        return;
    }

    if (level <= RISCV64_EXCEPTION_SEVERITY_MAX)
    {
        riscv64_exception_context.exception_severity_level = level;
        printf("RISC-V64: Exception severity level set to %u\n", level);
    }
}

// Get exception severity level
uint32_t riscv64_exception_get_severity_level(void)
{
    if (!riscv64_exception_context.initialized)
    {
        return RISCV64_EXCEPTION_SEVERITY_NONE;
    }

    return riscv64_exception_context.exception_severity_level;
}

// ============================================================================
// EXCEPTION HANDLING
// ============================================================================

// Handle exception
void riscv64_exception_handle(uint32_t exception_number, uint64_t epc, uint64_t tval)
{
    if (!riscv64_exception_context.initialized)
    {
        return;
    }

    // Update exception statistics
    riscv64_exception_stats.total_exceptions++;

    // Log exception if logging is enabled
    if (riscv64_exception_context.exception_logging_enabled)
    {
        riscv64_exception_log_entry(exception_number, epc, tval);
    }

    // Handle exception based on type
    switch (exception_number)
    {
    case RISCV64_EXCEPTION_INSTRUCTION_ADDRESS_MISALIGNED:
        riscv64_exception_stats.instruction_misaligned++;
        riscv64_exception_handle_instruction_misaligned(epc, tval);
        break;
    case RISCV64_EXCEPTION_INSTRUCTION_ACCESS_FAULT:
        riscv64_exception_stats.instruction_access_fault++;
        riscv64_exception_handle_instruction_access_fault(epc, tval);
        break;
    case RISCV64_EXCEPTION_ILLEGAL_INSTRUCTION:
        riscv64_exception_stats.illegal_instruction++;
        riscv64_exception_handle_illegal_instruction(epc, tval);
        break;
    case RISCV64_EXCEPTION_BREAKPOINT:
        riscv64_exception_stats.breakpoint++;
        riscv64_exception_handle_breakpoint(epc, tval);
        break;
    case RISCV64_EXCEPTION_LOAD_ADDRESS_MISALIGNED:
        riscv64_exception_stats.load_address_misaligned++;
        riscv64_exception_handle_load_address_misaligned(epc, tval);
        break;
    case RISCV64_EXCEPTION_LOAD_ACCESS_FAULT:
        riscv64_exception_stats.load_access_fault++;
        riscv64_exception_handle_load_access_fault(epc, tval);
        break;
    case RISCV64_EXCEPTION_STORE_ADDRESS_MISALIGNED:
        riscv64_exception_stats.store_address_misaligned++;
        riscv64_exception_handle_store_address_misaligned(epc, tval);
        break;
    case RISCV64_EXCEPTION_STORE_ACCESS_FAULT:
        riscv64_exception_stats.store_access_fault++;
        riscv64_exception_handle_store_access_fault(epc, tval);
        break;
    case RISCV64_EXCEPTION_ENVIRONMENT_CALL:
        riscv64_exception_stats.environment_call++;
        riscv64_exception_handle_environment_call(epc, tval);
        break;
    case RISCV64_EXCEPTION_INSTRUCTION_PAGE_FAULT:
        riscv64_exception_stats.instruction_page_fault++;
        riscv64_exception_handle_instruction_page_fault(epc, tval);
        break;
    case RISCV64_EXCEPTION_LOAD_PAGE_FAULT:
        riscv64_exception_stats.load_page_fault++;
        riscv64_exception_handle_load_page_fault(epc, tval);
        break;
    case RISCV64_EXCEPTION_STORE_PAGE_FAULT:
        riscv64_exception_stats.store_page_fault++;
        riscv64_exception_handle_store_page_fault(epc, tval);
        break;
    default:
        printf("RISC-V64: Unknown exception %u at EPC 0x%llx, TVAL 0x%llx\n", exception_number, epc, tval);
        break;
    }
}

// ============================================================================
// EXCEPTION HANDLERS
// ============================================================================

// Handle instruction address misaligned exception
void riscv64_exception_handle_instruction_address_misaligned(uint64_t epc, uint64_t tval)
{
    printf("RISC-V64: Instruction address misaligned exception\n");
    printf("  EPC: 0x%llx\n", epc);
    printf("  TVAL: 0x%llx\n", tval);

    // Attempt to recover from misaligned instruction
    if (riscv64_exception_context.exception_recovery_enabled)
    {
        bool recovered = riscv64_exception_recover_instruction_misaligned(epc, tval);
        if (recovered)
        {
            riscv64_exception_stats.exceptions_recovered++;
            printf("RISC-V64: Instruction misaligned exception recovered\n");
        }
        else
        {
            riscv64_exception_stats.exceptions_failed++;
            printf("RISC-V64: Instruction misaligned exception recovery failed\n");
        }
    }
}

// Handle instruction access fault exception
void riscv64_exception_handle_instruction_access_fault(uint64_t epc, uint64_t tval)
{
    printf("RISC-V64: Instruction access fault exception\n");
    printf("  EPC: 0x%llx\n", epc);
    printf("  TVAL: 0x%llx\n", tval);

    // Attempt to recover from instruction access fault
    if (riscv64_exception_context.exception_recovery_enabled)
    {
        bool recovered = riscv64_exception_recover_instruction_access_fault(epc, tval);
        if (recovered)
        {
            riscv64_exception_stats.exceptions_recovered++;
            printf("RISC-V64: Instruction access fault exception recovered\n");
        }
        else
        {
            riscv64_exception_stats.exceptions_failed++;
            printf("RISC-V64: Instruction access fault exception recovery failed\n");
        }
    }
}

// Handle illegal instruction exception
void riscv64_exception_handle_illegal_instruction(uint64_t epc, uint64_t tval)
{
    printf("RISC-V64: Illegal instruction exception\n");
    printf("  EPC: 0x%llx\n", epc);
    printf("  TVAL: 0x%llx\n", tval);

    // Attempt to recover from illegal instruction
    if (riscv64_exception_context.exception_recovery_enabled)
    {
        bool recovered = riscv64_exception_recover_illegal_instruction(epc, tval);
        if (recovered)
        {
            riscv64_exception_stats.exceptions_recovered++;
            printf("RISC-V64: Illegal instruction exception recovered\n");
        }
        else
        {
            riscv64_exception_stats.exceptions_failed++;
            printf("RISC-V64: Illegal instruction exception recovery failed\n");
        }
    }
}

// Handle breakpoint exception
void riscv64_exception_handle_breakpoint(uint64_t epc, uint64_t tval)
{
    printf("RISC-V64: Breakpoint exception\n");
    printf("  EPC: 0x%llx\n", epc);
    printf("  TVAL: 0x%llx\n", tval);

    // Breakpoint exceptions are typically handled by debugger
    // For now, just continue execution
    printf("RISC-V64: Breakpoint exception handled, continuing execution\n");
}

// Handle load address misaligned exception
void riscv64_exception_handle_load_address_misaligned(uint64_t epc, uint64_t tval)
{
    printf("RISC-V64: Load address misaligned exception\n");
    printf("  EPC: 0x%llx\n", epc);
    printf("  TVAL: 0x%llx\n", tval);

    // Attempt to recover from load address misaligned
    if (riscv64_exception_context.exception_recovery_enabled)
    {
        bool recovered = riscv64_exception_recover_load_address_misaligned(epc, tval);
        if (recovered)
        {
            riscv64_exception_stats.exceptions_recovered++;
            printf("RISC-V64: Load address misaligned exception recovered\n");
        }
        else
        {
            riscv64_exception_stats.exceptions_failed++;
            printf("RISC-V64: Load address misaligned exception recovery failed\n");
        }
    }
}

// Handle load access fault exception
void riscv64_exception_handle_load_access_fault(uint64_t epc, uint64_t tval)
{
    printf("RISC-V64: Load access fault exception\n");
    printf("  EPC: 0x%llx\n", epc);
    printf("  TVAL: 0x%llx\n", tval);

    // Attempt to recover from load access fault
    if (riscv64_exception_context.exception_recovery_enabled)
    {
        bool recovered = riscv64_exception_recover_load_access_fault(epc, tval);
        if (recovered)
        {
            riscv64_exception_stats.exceptions_recovered++;
            printf("RISC-V64: Load access fault exception recovered\n");
        }
        else
        {
            riscv64_exception_stats.exceptions_failed++;
            printf("RISC-V64: Load access fault exception recovery failed\n");
        }
    }
}

// Handle store address misaligned exception
void riscv64_exception_handle_store_address_misaligned(uint64_t epc, uint64_t tval)
{
    printf("RISC-V64: Store address misaligned exception\n");
    printf("  EPC: 0x%llx\n", epc);
    printf("  TVAL: 0x%llx\n", tval);

    // Attempt to recover from store address misaligned
    if (riscv64_exception_context.exception_recovery_enabled)
    {
        bool recovered = riscv64_exception_recover_store_address_misaligned(epc, tval);
        if (recovered)
        {
            riscv64_exception_stats.exceptions_recovered++;
            printf("RISC-V64: Store address misaligned exception recovered\n");
        }
        else
        {
            riscv64_exception_stats.exceptions_failed++;
            printf("RISC-V64: Store address misaligned exception recovery failed\n");
        }
    }
}

// Handle store access fault exception
void riscv64_exception_handle_store_access_fault(uint64_t epc, uint64_t tval)
{
    printf("RISC-V64: Store access fault exception\n");
    printf("  EPC: 0x%llx\n", epc);
    printf("  TVAL: 0x%llx\n", tval);

    // Attempt to recover from store access fault
    if (riscv64_exception_context.exception_recovery_enabled)
    {
        bool recovered = riscv64_exception_recover_store_access_fault(epc, tval);
        if (recovered)
        {
            riscv64_exception_stats.exceptions_recovered++;
            printf("RISC-V64: Store access fault exception recovered\n");
        }
        else
        {
            riscv64_exception_stats.exceptions_failed++;
            printf("RISC-V64: Store access fault exception recovery failed\n");
        }
    }
}

// Handle environment call exception
void riscv64_exception_handle_environment_call(uint64_t epc, uint64_t tval)
{
    printf("RISC-V64: Environment call exception\n");
    printf("  EPC: 0x%llx\n", epc);
    printf("  TVAL: 0x%llx\n", tval);

    // Environment call exceptions are typically handled by the system
    // For now, just continue execution
    printf("RISC-V64: Environment call exception handled, continuing execution\n");
}

// Handle instruction page fault exception
void riscv64_exception_handle_instruction_page_fault(uint64_t epc, uint64_t tval)
{
    printf("RISC-V64: Instruction page fault exception\n");
    printf("  EPC: 0x%llx\n", epc);
    printf("  TVAL: 0x%llx\n", tval);

    // Attempt to recover from instruction page fault
    if (riscv64_exception_context.exception_recovery_enabled)
    {
        bool recovered = riscv64_exception_recover_instruction_page_fault(epc, tval);
        if (recovered)
        {
            riscv64_exception_stats.exceptions_recovered++;
            printf("RISC-V64: Instruction page fault exception recovered\n");
        }
        else
        {
            riscv64_exception_stats.exceptions_failed++;
            printf("RISC-V64: Instruction page fault exception recovery failed\n");
        }
    }
}

// Handle load page fault exception
void riscv64_exception_handle_load_page_fault(uint64_t epc, uint64_t tval)
{
    printf("RISC-V64: Load page fault exception\n");
    printf("  EPC: 0x%llx\n", epc);
    printf("  TVAL: 0x%llx\n", tval);

    // Attempt to recover from load page fault
    if (riscv64_exception_context.exception_recovery_enabled)
    {
        bool recovered = riscv64_exception_recover_load_page_fault(epc, tval);
        if (recovered)
        {
            riscv64_exception_stats.exceptions_recovered++;
            printf("RISC-V64: Load page fault exception recovered\n");
        }
        else
        {
            riscv64_exception_stats.exceptions_failed++;
            printf("RISC-V64: Load page fault exception recovery failed\n");
        }
    }
}

// Handle store page fault exception
void riscv64_exception_handle_store_page_fault(uint64_t epc, uint64_t tval)
{
    printf("RISC-V64: Store page fault exception\n");
    printf("  EPC: 0x%llx\n", epc);
    printf("  TVAL: 0x%llx\n", tval);

    // Attempt to recover from store page fault
    if (riscv64_exception_context.exception_recovery_enabled)
    {
        bool recovered = riscv64_exception_recover_store_page_fault(epc, tval);
        if (recovered)
        {
            riscv64_exception_stats.exceptions_recovered++;
            printf("RISC-V64: Store page fault exception recovered\n");
        }
        else
        {
            riscv64_exception_stats.exceptions_failed++;
            printf("RISC-V64: Store page fault exception recovery failed\n");
        }
    }
}

// ============================================================================
// EXCEPTION RECOVERY FUNCTIONS
// ============================================================================

// Recover from instruction misaligned exception
bool riscv64_exception_recover_instruction_misaligned(uint64_t epc, uint64_t tval)
{
    // For now, return false (placeholder implementation)
    // In a real system, this would involve fixing the instruction alignment
    return false;
}

// Recover from instruction access fault exception
bool riscv64_exception_recover_instruction_access_fault(uint64_t epc, uint64_t tval)
{
    // For now, return false (placeholder implementation)
    // In a real system, this would involve fixing the instruction access
    return false;
}

// Recover from illegal instruction exception
bool riscv64_exception_recover_illegal_instruction(uint64_t epc, uint64_t tval)
{
    // For now, return false (placeholder implementation)
    // In a real system, this would involve handling the illegal instruction
    return false;
}

// Recover from load address misaligned exception
bool riscv64_exception_recover_load_address_misaligned(uint64_t epc, uint64_t tval)
{
    // For now, return false (placeholder implementation)
    // In a real system, this would involve fixing the load address alignment
    return false;
}

// Recover from load access fault exception
bool riscv64_exception_recover_load_access_fault(uint64_t epc, uint64_t tval)
{
    // For now, return false (placeholder implementation)
    // In a real system, this would involve fixing the load access
    return false;
}

// Recover from store address misaligned exception
bool riscv64_exception_recover_store_address_misaligned(uint64_t epc, uint64_t tval)
{
    // For now, return false (placeholder implementation)
    // In a real system, this would involve fixing the store address alignment
    return false;
}

// Recover from store access fault exception
bool riscv64_exception_recover_store_access_fault(uint64_t epc, uint64_t tval)
{
    // For now, return false (placeholder implementation)
    // In a real system, this would involve fixing the store access
    return false;
}

// Recover from instruction page fault exception
bool riscv64_exception_recover_instruction_page_fault(uint64_t epc, uint64_t tval)
{
    // For now, return false (placeholder implementation)
    // In a real system, this would involve handling the instruction page fault
    return false;
}

// Recover from load page fault exception
bool riscv64_exception_recover_load_page_fault(uint64_t epc, uint64_t tval)
{
    // For now, return false (placeholder implementation)
    // In a real system, this would involve handling the load page fault
    return false;
}

// Recover from store page fault exception
bool riscv64_exception_recover_store_page_fault(uint64_t epc, uint64_t tval)
{
    // For now, return false (placeholder implementation)
    // In a real system, this would involve handling the store page fault
    return false;
}

// ============================================================================
// EXCEPTION LOGGING
// ============================================================================

// Log exception entry
void riscv64_exception_log_entry(uint32_t exception_number, uint64_t epc, uint64_t tval)
{
    if (!riscv64_exception_context.initialized || !riscv64_exception_context.exception_logging_enabled)
    {
        return;
    }

    // Allocate exception log if not already allocated
    if (riscv64_exception_log.entries == NULL)
    {
        riscv64_exception_log.entries = malloc(sizeof(struct riscv64_exception_entry) * riscv64_exception_log.max_log_entries);
        if (riscv64_exception_log.entries == NULL)
        {
            return; // Cannot log exception if memory allocation fails
        }
        memset(riscv64_exception_log.entries, 0, sizeof(struct riscv64_exception_entry) * riscv64_exception_log.max_log_entries);
    }

    // Add exception entry to log
    struct riscv64_exception_entry *entry = &riscv64_exception_log.entries[riscv64_exception_log.log_index];

    entry->timestamp = riscv64_timer_read_ns();
    entry->exception_number = exception_number;
    entry->epc = epc;
    entry->tval = tval;
    entry->cpu_id = riscv64_cpu_get_hart_id();
    entry->privilege_level = riscv64_cpu_get_mode();

    // Update log index (circular buffer)
    riscv64_exception_log.log_index = (riscv64_exception_log.log_index + 1) % riscv64_exception_log.max_log_entries;

    // Update log size
    if (riscv64_exception_log.log_size < riscv64_exception_log.max_log_entries)
    {
        riscv64_exception_log.log_size++;
    }
}

// Get exception log entry
struct riscv64_exception_entry *riscv64_exception_get_log_entry(uint32_t index)
{
    if (!riscv64_exception_context.initialized || !riscv64_exception_context.exception_logging_enabled)
    {
        return NULL;
    }

    if (riscv64_exception_log.entries == NULL || index >= riscv64_exception_log.log_size)
    {
        return NULL;
    }

    return &riscv64_exception_log.entries[index];
}

// Clear exception log
void riscv64_exception_clear_log(void)
{
    if (!riscv64_exception_context.initialized)
    {
        return;
    }

    if (riscv64_exception_log.entries != NULL)
    {
        memset(riscv64_exception_log.entries, 0, sizeof(struct riscv64_exception_entry) * riscv64_exception_log.max_log_entries);
    }

    riscv64_exception_log.log_size = 0;
    riscv64_exception_log.log_index = 0;

    printf("RISC-V64: Exception log cleared\n");
}

// ============================================================================
// EXCEPTION STATISTICS
// ============================================================================

// Get exception statistics
void riscv64_exception_get_stats(uint32_t *total_exceptions, uint32_t *instruction_misaligned,
                                 uint32_t *instruction_access_fault, uint32_t *illegal_instruction,
                                 uint32_t *breakpoint, uint32_t *load_address_misaligned,
                                 uint32_t *load_access_fault, uint32_t *store_address_misaligned,
                                 uint32_t *store_access_fault, uint32_t *environment_call,
                                 uint32_t *instruction_page_fault, uint32_t *load_page_fault,
                                 uint32_t *store_page_fault, uint32_t *exceptions_recovered,
                                 uint32_t *exceptions_failed)
{
    if (total_exceptions)
        *total_exceptions = riscv64_exception_stats.total_exceptions;
    if (instruction_misaligned)
        *instruction_misaligned = riscv64_exception_stats.instruction_misaligned;
    if (instruction_access_fault)
        *instruction_access_fault = riscv64_exception_stats.instruction_access_fault;
    if (illegal_instruction)
        *illegal_instruction = riscv64_exception_stats.illegal_instruction;
    if (breakpoint)
        *breakpoint = riscv64_exception_stats.breakpoint;
    if (load_address_misaligned)
        *load_address_misaligned = riscv64_exception_stats.load_address_misaligned;
    if (load_access_fault)
        *load_access_fault = riscv64_exception_stats.load_access_fault;
    if (store_address_misaligned)
        *store_address_misaligned = riscv64_exception_stats.store_address_misaligned;
    if (store_access_fault)
        *store_access_fault = riscv64_exception_stats.store_access_fault;
    if (environment_call)
        *environment_call = riscv64_exception_stats.environment_call;
    if (instruction_page_fault)
        *instruction_page_fault = riscv64_exception_stats.instruction_page_fault;
    if (load_page_fault)
        *load_page_fault = riscv64_exception_stats.load_page_fault;
    if (store_page_fault)
        *store_page_fault = riscv64_exception_stats.store_page_fault;
    if (exceptions_recovered)
        *exceptions_recovered = riscv64_exception_stats.exceptions_recovered;
    if (exceptions_failed)
        *exceptions_failed = riscv64_exception_stats.exceptions_failed;
}

// Reset exception statistics
void riscv64_exception_reset_stats(void)
{
    memset(&riscv64_exception_stats, 0, sizeof(riscv64_exception_stats));

    printf("RISC-V64: Exception statistics reset\n");
}

// ============================================================================
// EXCEPTION DEBUG FUNCTIONS
// ============================================================================

// Print exception status
void riscv64_exception_print_status(void)
{
    printf("RISC-V64: Exception Status\n");
    printf("  Initialized: %s\n", riscv64_exception_context.initialized ? "Yes" : "No");
    printf("  Exception Handling: %s\n", riscv64_exception_context.exception_handling_enabled ? "Enabled" : "Disabled");
    printf("  Exception Logging: %s\n", riscv64_exception_context.exception_logging_enabled ? "Enabled" : "Disabled");
    printf("  Exception Recovery: %s\n", riscv64_exception_context.exception_recovery_enabled ? "Enabled" : "Disabled");
    printf("  Max Exception Count: %u\n", riscv64_exception_context.max_exception_count);
    printf("  Exception Severity Level: %u\n", riscv64_exception_context.exception_severity_level);
    printf("  Exception Handlers: 0x%llx\n", riscv64_exception_context.exception_handlers);
}

// Print exception statistics
void riscv64_exception_print_statistics(void)
{
    printf("RISC-V64: Exception Statistics\n");
    printf("  Total Exceptions: %u\n", riscv64_exception_stats.total_exceptions);
    printf("  Instruction Misaligned: %u\n", riscv64_exception_stats.instruction_misaligned);
    printf("  Instruction Access Fault: %u\n", riscv64_exception_stats.instruction_access_fault);
    printf("  Illegal Instruction: %u\n", riscv64_exception_stats.illegal_instruction);
    printf("  Breakpoint: %u\n", riscv64_exception_stats.breakpoint);
    printf("  Load Address Misaligned: %u\n", riscv64_exception_stats.load_address_misaligned);
    printf("  Load Access Fault: %u\n", riscv64_exception_stats.load_access_fault);
    printf("  Store Address Misaligned: %u\n", riscv64_exception_stats.store_address_misaligned);
    printf("  Store Access Fault: %u\n", riscv64_exception_stats.store_access_fault);
    printf("  Environment Call: %u\n", riscv64_exception_stats.environment_call);
    printf("  Instruction Page Fault: %u\n", riscv64_exception_stats.instruction_page_fault);
    printf("  Load Page Fault: %u\n", riscv64_exception_stats.load_page_fault);
    printf("  Store Page Fault: %u\n", riscv64_exception_stats.store_page_fault);
    printf("  Exceptions Recovered: %u\n", riscv64_exception_stats.exceptions_recovered);
    printf("  Exceptions Failed: %u\n", riscv64_exception_stats.exceptions_failed);
}

// Print exception log
void riscv64_exception_print_log(void)
{
    if (!riscv64_exception_context.initialized || !riscv64_exception_context.exception_logging_enabled)
    {
        printf("RISC-V64: Exception logging not available\n");
        return;
    }

    printf("RISC-V64: Exception Log (%u entries)\n", riscv64_exception_log.log_size);

    if (riscv64_exception_log.entries == NULL || riscv64_exception_log.log_size == 0)
    {
        printf("  No exception entries\n");
        return;
    }

    for (uint32_t i = 0; i < riscv64_exception_log.log_size; i++)
    {
        struct riscv64_exception_entry *entry = &riscv64_exception_log.entries[i];

        printf("  Entry %u:\n", i);
        printf("    Timestamp: %llu ns\n", entry->timestamp);
        printf("    Exception Number: %u\n", entry->exception_number);
        printf("    EPC: 0x%llx\n", entry->epc);
        printf("    TVAL: 0x%llx\n", entry->tval);
        printf("    CPU ID: %u\n", entry->cpu_id);
        printf("    Privilege Level: %u\n", entry->privilege_level);
    }
}

// ============================================================================
// EXCEPTION CLEANUP
// ============================================================================

// Cleanup exception management system
void riscv64_exception_cleanup(void)
{
    if (!riscv64_exception_context.initialized)
    {
        return;
    }

    // Free exception log memory
    if (riscv64_exception_log.entries != NULL)
    {
        free(riscv64_exception_log.entries);
        riscv64_exception_log.entries = NULL;
    }

    // Clear contexts
    riscv64_exception_context.initialized = false;

    printf("RISC-V64: Exception management system cleaned up\n");
}
