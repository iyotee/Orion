/*
 * ORION OS - MIPS Architecture Interrupt Handlers
 *
 * C-level interrupt handlers for MIPS architecture (MIPS32, MIPS64)
 * Implements exception and interrupt handling logic
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include <stdio.h>
#include <string.h>

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// Exception handler data
static void *mips_exception_data[MIPS_MAX_EXCEPTIONS] = {0};

// Exception counters
static uint64_t mips_exception_counters[MIPS_MAX_EXCEPTIONS] = {0};

// Interrupt counters
static uint64_t mips_interrupt_counters[MIPS_MAX_IRQS] = {0};

// System call table
static void (*mips_syscall_handlers[MIPS_MAX_SYSCALLS])(void) = {0};

// ============================================================================
// EXCEPTION HANDLERS
// ============================================================================

void mips_handle_undefined_instruction(void)
{
    mips_exception_counters[MIPS_EXCEPTION_UNDEFINED]++;
    
    printf("MIPS: Undefined instruction exception\n");
    
    // Get exception information
    uint64_t epc = mips_cpu_read_cp0_epc();
    uint64_t cause = mips_cpu_read_cp0_cause();
    
    printf("MIPS: EPC: 0x%llx, Cause: 0x%llx\n", (unsigned long long)epc, (unsigned long long)cause);
    
    // Handle undefined instruction (could implement emulation)
    // For now, just skip to next instruction
    mips_cpu_write_cp0_epc(epc + 4);
}

void mips_handle_software_interrupt(void)
{
    mips_exception_counters[MIPS_EXCEPTION_SOFTWARE]++;
    
    printf("MIPS: Software interrupt exception\n");
    
    // Get exception information
    uint64_t epc = mips_cpu_read_cp0_epc();
    uint64_t cause = mips_cpu_read_cp0_cause();
    
    printf("MIPS: EPC: 0x%llx, Cause: 0x%llx\n", (unsigned long long)epc, (unsigned long long)cause);
    
    // Handle software interrupt (system call)
    mips_handle_system_call();
}

void mips_handle_prefetch_abort(void)
{
    mips_exception_counters[MIPS_EXCEPTION_PREFETCH]++;
    
    printf("MIPS: Prefetch abort exception\n");
    
    // Get exception information
    uint64_t epc = mips_cpu_read_cp0_epc();
    uint64_t badvaddr = mips_cpu_read_cp0_register(MIPS_CP0_BADVADDR);
    
    printf("MIPS: EPC: 0x%llx, BadVAddr: 0x%llx\n", (unsigned long long)epc, (unsigned long long)badvaddr);
    
    // Handle prefetch abort (instruction fetch error)
    // Could implement page fault handling here
}

void mips_handle_data_abort(void)
{
    mips_exception_counters[MIPS_EXCEPTION_DATA]++;
    
    printf("MIPS: Data abort exception\n");
    
    // Get exception information
    uint64_t epc = mips_cpu_read_cp0_epc();
    uint64_t badvaddr = mips_cpu_read_cp0_register(MIPS_CP0_BADVADDR);
    
    printf("MIPS: EPC: 0x%llx, BadVAddr: 0x%llx\n", (unsigned long long)epc, (unsigned long long)badvaddr);
    
    // Handle data abort (data access error)
    // Could implement page fault handling here
}

void mips_handle_reserved_exception(void)
{
    mips_exception_counters[MIPS_EXCEPTION_RESERVED]++;
    
    printf("MIPS: Reserved exception\n");
    
    // Get exception information
    uint64_t epc = mips_cpu_read_cp0_epc();
    uint64_t cause = mips_cpu_read_cp0_cause();
    
    printf("MIPS: EPC: 0x%llx, Cause: 0x%llx\n", (unsigned long long)epc, (unsigned long long)cause);
    
    // Handle reserved exception
    // This should not happen in normal operation
}

void mips_handle_irq(void)
{
    printf("MIPS: IRQ exception\n");
    
    // Get exception information
    uint64_t epc = mips_cpu_read_cp0_epc();
    uint64_t cause = mips_cpu_read_cp0_cause();
    uint64_t status = mips_cpu_read_cp0_status();
    
    printf("MIPS: EPC: 0x%llx, Cause: 0x%llx, Status: 0x%llx\n", 
           (unsigned long long)epc, (unsigned long long)cause, (unsigned long long)status);
    
    // Check which interrupt is pending
    uint64_t pending = cause & status & MIPS_STATUS_IP;
    
    if (pending & MIPS_STATUS_IP0) {
        mips_interrupt_counters[0]++;
        mips_handle_irq_0();
    }
    if (pending & MIPS_STATUS_IP1) {
        mips_interrupt_counters[1]++;
        mips_handle_irq_1();
    }
    if (pending & MIPS_STATUS_IP2) {
        mips_interrupt_counters[2]++;
        mips_handle_irq_2();
    }
    if (pending & MIPS_STATUS_IP3) {
        mips_interrupt_counters[3]++;
        mips_handle_irq_3();
    }
    if (pending & MIPS_STATUS_IP4) {
        mips_interrupt_counters[4]++;
        mips_handle_irq_4();
    }
    if (pending & MIPS_STATUS_IP5) {
        mips_interrupt_counters[5]++;
        mips_handle_irq_5();
    }
    if (pending & MIPS_STATUS_IP6) {
        mips_interrupt_counters[6]++;
        mips_handle_irq_6();
    }
    if (pending & MIPS_STATUS_IP7) {
        mips_interrupt_counters[7]++;
        mips_handle_timer_interrupt();
    }
}

void mips_handle_fiq(void)
{
    mips_exception_counters[MIPS_EXCEPTION_FIQ]++;
    
    printf("MIPS: FIQ exception\n");
    
    // Get exception information
    uint64_t epc = mips_cpu_read_cp0_epc();
    
    printf("MIPS: EPC: 0x%llx\n", (unsigned long long)epc);
    
    // Handle FIQ (Fast Interrupt Request)
    // This is typically used for high-priority, time-critical interrupts
}

// ============================================================================
// INTERRUPT HANDLERS
// ============================================================================

void mips_handle_irq_0(void)
{
    printf("MIPS: Handling IRQ 0\n");
    // Handle specific IRQ 0
}

void mips_handle_irq_1(void)
{
    printf("MIPS: Handling IRQ 1\n");
    // Handle specific IRQ 1
}

void mips_handle_irq_2(void)
{
    printf("MIPS: Handling IRQ 2\n");
    // Handle specific IRQ 2
}

void mips_handle_irq_3(void)
{
    printf("MIPS: Handling IRQ 3\n");
    // Handle specific IRQ 3
}

void mips_handle_irq_4(void)
{
    printf("MIPS: Handling IRQ 4\n");
    // Handle specific IRQ 4
}

void mips_handle_irq_5(void)
{
    printf("MIPS: Handling IRQ 5\n");
    // Handle specific IRQ 5
}

void mips_handle_irq_6(void)
{
    printf("MIPS: Handling IRQ 6\n");
    // Handle specific IRQ 6
}

void mips_handle_timer_interrupt(void)
{
    printf("MIPS: Handling timer interrupt\n");
    
    // Update system timer
    static uint64_t system_tick = 0;
    system_tick++;
    
    // Acknowledge timer interrupt
    mips_timer_interrupt_acknowledge();
    
    // Call timer callback if registered
    // This would be implemented based on the timer system
}

// ============================================================================
// SYSTEM CALL HANDLING
// ============================================================================

void mips_handle_system_call(void)
{
    printf("MIPS: Handling system call\n");
    
    // Get system call number from register
    // In MIPS, this would typically be in $v0 or $a0
    // For now, we'll use a placeholder
    
    uint32_t syscall_number = 0; // Placeholder
    
    if (syscall_number < MIPS_MAX_SYSCALLS && mips_syscall_handlers[syscall_number])
    {
        mips_syscall_handlers[syscall_number]();
    }
    else
    {
        printf("MIPS: Unknown system call: %u\n", syscall_number);
    }
}

void mips_execute_system_call(uint32_t syscall_number)
{
    if (syscall_number < MIPS_MAX_SYSCALLS && mips_syscall_handlers[syscall_number])
    {
        mips_syscall_handlers[syscall_number]();
    }
    else
    {
        printf("MIPS: Unknown system call: %u\n", syscall_number);
    }
}

// ============================================================================
// SYSTEM CALL HANDLER REGISTRATION
// ============================================================================

int mips_register_syscall_handler(uint32_t syscall_number, void (*handler)(void))
{
    if (syscall_number < MIPS_MAX_SYSCALLS && handler)
    {
        mips_syscall_handlers[syscall_number] = handler;
        printf("MIPS: System call handler registered for syscall %u\n", syscall_number);
        return 0; // Success
    }
    return -1; // Error
}

void mips_unregister_syscall_handler(uint32_t syscall_number)
{
    if (syscall_number < MIPS_MAX_SYSCALLS)
    {
        mips_syscall_handlers[syscall_number] = NULL;
        printf("MIPS: System call handler unregistered for syscall %u\n", syscall_number);
    }
}

// ============================================================================
// PLACEHOLDER SYSTEM CALL IMPLEMENTATIONS
// ============================================================================

void mips_syscall_exit(void)
{
    printf("MIPS: System call: exit\n");
    // Implement process exit
}

void mips_syscall_read(void)
{
    printf("MIPS: System call: read\n");
    // Implement file read
}

void mips_syscall_write(void)
{
    printf("MIPS: System call: write\n");
    // Implement file write
}

void mips_syscall_open(void)
{
    printf("MIPS: System call: open\n");
    // Implement file open
}

void mips_syscall_close(void)
{
    printf("MIPS: System call: close\n");
    // Implement file close
}

void mips_syscall_fork(void)
{
    printf("MIPS: System call: fork\n");
    // Implement process fork
}

void mips_syscall_exec(void)
{
    printf("MIPS: System call: exec\n");
    // Implement process exec
}

void mips_syscall_wait(void)
{
    printf("MIPS: System call: wait\n");
    // Implement process wait
}

void mips_syscall_kill(void)
{
    printf("MIPS: System call: kill\n");
    // Implement process kill
}

void mips_syscall_getpid(void)
{
    printf("MIPS: System call: getpid\n");
    // Implement get process ID
}

void mips_syscall_sleep(void)
{
    printf("MIPS: System call: sleep\n");
    // Implement sleep
}

void mips_syscall_time(void)
{
    printf("MIPS: System call: time\n");
    // Implement get time
}

void mips_syscall_brk(void)
{
    printf("MIPS: System call: brk\n");
    // Implement memory allocation
}

void mips_syscall_mmap(void)
{
    printf("MIPS: System call: mmap\n");
    // Implement memory mapping
}

void mips_syscall_munmap(void)
{
    printf("MIPS: System call: munmap\n");
    // Implement memory unmapping
}

void mips_syscall_socket(void)
{
    printf("MIPS: System call: socket\n");
    // Implement socket creation
}

void mips_syscall_bind(void)
{
    printf("MIPS: System call: bind\n");
    // Implement socket binding
}

void mips_syscall_listen(void)
{
    printf("MIPS: System call: listen\n");
    // Implement socket listening
}

void mips_syscall_accept(void)
{
    printf("MIPS: System call: accept\n");
    // Implement socket acceptance
}

void mips_syscall_connect(void)
{
    printf("MIPS: System call: connect\n");
    // Implement socket connection
}

void mips_syscall_send(void)
{
    printf("MIPS: System call: send\n");
    // Implement socket send
}

void mips_syscall_recv(void)
{
    printf("MIPS: System call: recv\n");
    // Implement socket receive
}

// ============================================================================
// EXCEPTION AND INTERRUPT STATISTICS
// ============================================================================

uint64_t mips_get_exception_counter(uint32_t exception_type)
{
    if (exception_type < MIPS_MAX_EXCEPTIONS)
    {
        return mips_exception_counters[exception_type];
    }
    return 0;
}

uint64_t mips_get_interrupt_counter(uint32_t irq_number)
{
    if (irq_number < MIPS_MAX_IRQS)
    {
        return mips_interrupt_counters[irq_number];
    }
    return 0;
}

void mips_reset_exception_counters(void)
{
    memset(mips_exception_counters, 0, sizeof(mips_exception_counters));
    printf("MIPS: Exception counters reset\n");
}

void mips_reset_interrupt_counters(void)
{
    memset(mips_interrupt_counters, 0, sizeof(mips_interrupt_counters));
    printf("MIPS: Interrupt counters reset\n");
}

// ============================================================================
// ERROR HANDLING
// ============================================================================

void mips_handle_error(uint32_t error_code, const char *error_message)
{
    printf("MIPS: ERROR %u: %s\n", error_code, error_message);
    
    // Log error
    // Send error to error handler
    // Potentially restart system
    
    if (error_code >= 0x1000)
    {
        // Critical error, restart system
        printf("MIPS: Critical error detected, restarting system\n");
        mips_reset();
    }
}

void mips_handle_panic(const char *panic_message)
{
    printf("MIPS: PANIC: %s\n", panic_message);
    
    // Disable interrupts
    mips_cpu_disable_interrupts();
    
    // Dump system state
    printf("MIPS: System state dump:\n");
    printf("  CPU ID: %u\n", mips_cpu_get_id());
    printf("  CPU Revision: %u\n", mips_cpu_get_revision());
    printf("  CPU Frequency: %llu Hz\n", (unsigned long long)mips_cpu_get_frequency());
    
    // Halt system
    printf("MIPS: System halted due to panic\n");
    mips_cpu_halt();
}
