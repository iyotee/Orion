/*
 * ORION OS - ARMv7l Interrupt Handlers
 * 
 * This file implements the actual interrupt and exception handlers for ARMv7l,
 * including IRQ, FIQ, and exception vector handling.
 */

#include "arch.h"
#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// EXTERNAL SYMBOLS
// ============================================================================

extern void arm_exception_vector_table(void);
extern void arm_irq_handler_wrapper(void);
extern void arm_fiq_handler_wrapper(void);
extern void arm_undefined_handler_wrapper(void);
extern void arm_prefetch_abort_handler_wrapper(void);
extern void arm_data_abort_handler_wrapper(void);
extern void arm_reserved_handler_wrapper(void);

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

static arm_exception_handler_t g_exception_handlers[ARM_EXCEPTION_MAX];
static void *g_exception_data[ARM_EXCEPTION_MAX];

static uint32_t g_irq_count = 0;
static uint32_t g_fiq_count = 0;
static uint32_t g_exception_counts[ARM_EXCEPTION_MAX] = {0};

// ============================================================================
// EXCEPTION HANDLER REGISTRATION
// ============================================================================

int arm_register_exception_handler(arm_exception_type_t type, arm_exception_handler_t handler, void *data)
{
    if (type >= ARM_EXCEPTION_MAX || !handler) {
        return -1;
    }

    g_exception_handlers[type] = handler;
    g_exception_data[type] = data;
    return 0;
}

int arm_unregister_exception_handler(arm_exception_type_t type)
{
    if (type >= ARM_EXCEPTION_MAX) {
        return -1;
    }

    g_exception_handlers[type] = NULL;
    g_exception_data[type] = NULL;
    return 0;
}

// ============================================================================
// EXCEPTION HANDLER IMPLEMENTATIONS
// ============================================================================

void arm_undefined_instruction_handler(arm_cpu_context_t *context)
{
    g_exception_counts[ARM_EXCEPTION_UNDEFINED]++;

    if (g_exception_handlers[ARM_EXCEPTION_UNDEFINED]) {
        g_exception_handlers[ARM_EXCEPTION_UNDEFINED](context, g_exception_data[ARM_EXCEPTION_UNDEFINED]);
    } else {
        // Default handling - log and continue
        arm_log_exception("Undefined instruction", context);
    }
}

void arm_software_interrupt_handler(arm_cpu_context_t *context)
{
    g_exception_counts[ARM_EXCEPTION_SWI]++;

    if (g_exception_handlers[ARM_EXCEPTION_SWI]) {
        g_exception_handlers[ARM_EXCEPTION_SWI](context, g_exception_data[ARM_EXCEPTION_SWI]);
    } else {
        // Default handling - system call
        arm_handle_system_call(context);
    }
}

void arm_prefetch_abort_handler(arm_cpu_context_t *context)
{
    g_exception_counts[ARM_EXCEPTION_PREFETCH_ABORT]++;

    if (g_exception_handlers[ARM_EXCEPTION_PREFETCH_ABORT]) {
        g_exception_handlers[ARM_EXCEPTION_PREFETCH_ABORT](context, g_exception_data[ARM_EXCEPTION_PREFETCH_ABORT]);
    } else {
        // Default handling - log and terminate
        arm_log_exception("Prefetch abort", context);
        arm_handle_fatal_error(context);
    }
}

void arm_data_abort_handler(arm_cpu_context_t *context)
{
    g_exception_counts[ARM_EXCEPTION_DATA_ABORT]++;

    if (g_exception_handlers[ARM_EXCEPTION_DATA_ABORT]) {
        g_exception_handlers[ARM_EXCEPTION_DATA_ABORT](context, g_exception_data[ARM_EXCEPTION_DATA_ABORT]);
    } else {
        // Default handling - log and terminate
        arm_log_exception("Data abort", context);
        arm_handle_fatal_error(context);
    }
}

void arm_reserved_handler(arm_cpu_context_t *context)
{
    g_exception_counts[ARM_EXCEPTION_RESERVED]++;

    if (g_exception_handlers[ARM_EXCEPTION_RESERVED]) {
        g_exception_handlers[ARM_EXCEPTION_RESERVED](context, g_exception_data[ARM_EXCEPTION_RESERVED]);
    } else {
        // Default handling - log and halt
        arm_log_exception("Reserved exception", context);
        arm_halt_system();
    }
}

// ============================================================================
// INTERRUPT HANDLER IMPLEMENTATIONS
// ============================================================================

void arm_irq_handler(arm_cpu_context_t *context)
{
    g_irq_count++;

    // Get the IRQ number from the interrupt controller
    uint32_t irq_number = arm_get_active_irq();
    
    if (irq_number != ARM_INVALID_IRQ) {
        // Call the registered handler
        arm_irq_handler_t handler = arm_get_irq_handler(irq_number);
        if (handler) {
            handler(irq_number, context);
        }
        
        // Acknowledge the interrupt
        arm_irq_acknowledge(irq_number);
    }
}

void arm_fiq_handler(arm_cpu_context_t *context)
{
    g_fiq_count++;

    // FIQ has a single global handler
    arm_fiq_handler_t handler = arm_get_fiq_handler();
    if (handler) {
        handler(context);
    }
    
    // FIQ acknowledgment is typically automatic
    arm_fiq_acknowledge();
}

// ============================================================================
// SYSTEM CALL HANDLING
// ============================================================================

void arm_handle_system_call(arm_cpu_context_t *context)
{
    // Extract system call number and parameters
    uint32_t syscall_number = context->r7;  // ARM syscall convention
    uint32_t arg1 = context->r0;
    uint32_t arg2 = context->r1;
    uint32_t arg3 = context->r2;
    uint32_t arg4 = context->r3;
    uint32_t arg5 = context->r4;
    uint32_t arg6 = context->r5;

    // Handle the system call
    int result = arm_execute_system_call(syscall_number, arg1, arg2, arg3, arg4, arg5, arg6);
    
    // Store result in r0
    context->r0 = result;
}

int arm_execute_system_call(uint32_t syscall_number, uint32_t arg1, uint32_t arg2, uint32_t arg3, 
                           uint32_t arg4, uint32_t arg5, uint32_t arg6)
{
    // Basic system call implementation
    // This should be expanded based on the actual system call interface
    (void)arg4;
    (void)arg5;
    (void)arg6;

    switch (syscall_number) {
        case ARM_SYSCALL_READ:
            return arm_syscall_read(arg1, arg2, arg3);
        case ARM_SYSCALL_WRITE:
            return arm_syscall_write(arg1, arg2, arg3);
        case ARM_SYSCALL_OPEN:
            return arm_syscall_open(arg1, arg2, arg3);
        case ARM_SYSCALL_CLOSE:
            return arm_syscall_close(arg1);
        case ARM_SYSCALL_EXIT:
            arm_syscall_exit(arg1);
            return 0;
        case ARM_SYSCALL_FORK:
            return arm_syscall_fork();
        case ARM_SYSCALL_EXECVE:
            return arm_syscall_execve(arg1, arg2, arg3);
        case ARM_SYSCALL_WAITPID:
            return arm_syscall_waitpid(arg1, arg2, arg3);
        case ARM_SYSCALL_KILL:
            return arm_syscall_kill(arg1, arg2);
        case ARM_SYSCALL_GETPID:
            return arm_syscall_getpid();
        case ARM_SYSCALL_SLEEP:
            return arm_syscall_sleep(arg1);
        default:
            return -1;  // Unknown system call
    }
}

// ============================================================================
// SYSTEM CALL IMPLEMENTATIONS
// ============================================================================

int arm_syscall_read(int fd, void *buf, size_t count)
{
    // Basic read implementation
    (void)fd;
    (void)buf;
    (void)count;
    return 0;
}

int arm_syscall_write(int fd, const void *buf, size_t count)
{
    // Basic write implementation
    (void)fd;
    (void)buf;
    (void)count;
    return 0;
}

int arm_syscall_open(const char *pathname, int flags, mode_t mode)
{
    // Basic open implementation
    (void)pathname;
    (void)flags;
    (void)mode;
    return 0;
}

int arm_syscall_close(int fd)
{
    // Basic close implementation
    (void)fd;
    return 0;
}

void arm_syscall_exit(int status)
{
    // Basic exit implementation
    (void)status;
    // Should terminate the current process
}

int arm_syscall_fork(void)
{
    // Basic fork implementation
    return 0;
}

int arm_syscall_execve(const char *filename, char *const argv[], char *const envp[])
{
    // Basic execve implementation
    (void)filename;
    (void)argv;
    (void)envp;
    return 0;
}

int arm_syscall_waitpid(pid_t pid, int *status, int options)
{
    // Basic waitpid implementation
    (void)pid;
    (void)status;
    (void)options;
    return 0;
}

int arm_syscall_kill(pid_t pid, int sig)
{
    // Basic kill implementation
    (void)pid;
    (void)sig;
    return 0;
}

pid_t arm_syscall_getpid(void)
{
    // Basic getpid implementation
    return 0;
}

int arm_syscall_sleep(unsigned int seconds)
{
    // Basic sleep implementation
    (void)seconds;
    return 0;
}

// ============================================================================
// ERROR HANDLING
// ============================================================================

void arm_handle_fatal_error(arm_cpu_context_t *context)
{
    // Handle fatal errors (aborts, etc.)
    // This should trigger system recovery or shutdown
    (void)context;
    
    // For now, just halt the system
    arm_halt_system();
}

void arm_halt_system(void)
{
    // Halt the system
    while (1) {
        __asm__ volatile("wfi");
    }
}

// ============================================================================
// LOGGING AND DEBUGGING
// ============================================================================

void arm_log_exception(const char *message, arm_cpu_context_t *context)
{
    // Log exception information
    // This should be implemented based on the logging system
    (void)message;
    (void)context;
}

// ============================================================================
// INTERRUPT CONTROLLER INTERFACE
// ============================================================================

uint32_t arm_get_active_irq(void)
{
    // Get the currently active IRQ number
    // This is architecture-specific
    return ARM_INVALID_IRQ;
}

arm_irq_handler_t arm_get_irq_handler(uint32_t irq_number)
{
    // Get the registered handler for an IRQ
    // This should interface with the interrupt management system
    (void)irq_number;
    return NULL;
}

arm_fiq_handler_t arm_get_fiq_handler(void)
{
    // Get the registered FIQ handler
    // This should interface with the interrupt management system
    return NULL;
}

// ============================================================================
// STATISTICS AND MONITORING
// ============================================================================

uint32_t arm_get_exception_count(arm_exception_type_t type)
{
    if (type >= ARM_EXCEPTION_MAX) {
        return 0;
    }
    return g_exception_counts[type];
}

void arm_reset_exception_counts(void)
{
    memset(g_exception_counts, 0, sizeof(g_exception_counts));
    g_irq_count = 0;
    g_fiq_count = 0;
}

// ============================================================================
// TESTING AND VALIDATION
// ============================================================================

void arm_test_exception_handlers(void)
{
    // Test exception handler functionality
    // This can be implemented for testing purposes
}

void arm_test_interrupt_handlers(void)
{
    // Test interrupt handler functionality
    // This can be implemented for testing purposes
}
