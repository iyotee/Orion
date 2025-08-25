/*
 * ORION OS - RISC-V 64-bit Interrupt Handlers
 *
 * C-level implementations for RISC-V 64-bit exception and interrupt handlers
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// GLOBAL ARRAYS
// ============================================================================

// Exception and interrupt counters
static uint32_t riscv64_exception_counters[16] = {0};
static uint32_t riscv64_interrupt_counters[1024] = {0};
static uint32_t riscv64_syscall_counters[64] = {0};

// System call table
static void (*riscv64_syscall_table[64])(void) = {0};

// ============================================================================
// EXCEPTION HANDLERS
// ============================================================================

// Instruction misaligned exception
void riscv64_instruction_misaligned_handler(uint64_t cause, uint64_t epc, uint64_t tval)
{
    riscv64_exception_counters[RISCV64_EXC_INSTRUCTION_MISALIGNED]++;
    printf("RISC-V64: Instruction misaligned exception at PC 0x%llx, tval 0x%llx\n", epc, tval);
    
    // Handle misaligned instruction
    // This could involve emulation or termination
    riscv64_handle_instruction_misaligned(epc, tval);
}

// Instruction access fault
void riscv64_instruction_access_fault_handler(uint64_t cause, uint64_t epc, uint64_t tval)
{
    riscv64_exception_counters[RISCV64_EXC_INSTRUCTION_ACCESS_FAULT]++;
    printf("RISC-V64: Instruction access fault at PC 0x%llx, tval 0x%llx\n", epc, tval);
    
    // Handle instruction access fault
    riscv64_handle_instruction_access_fault(epc, tval);
}

// Illegal instruction exception
void riscv64_illegal_instruction_handler(uint64_t cause, uint64_t epc, uint64_t tval)
{
    riscv64_exception_counters[RISCV64_EXC_ILLEGAL_INSTRUCTION]++;
    printf("RISC-V64: Illegal instruction exception at PC 0x%llx, tval 0x%llx\n", epc, tval);
    
    // Handle illegal instruction
    riscv64_handle_illegal_instruction(epc, tval);
}

// Breakpoint exception
void riscv64_breakpoint_handler(uint64_t cause, uint64_t epc, uint64_t tval)
{
    riscv64_exception_counters[RISCV64_EXC_BREAKPOINT]++;
    printf("RISC-V64: Breakpoint exception at PC 0x%llx, tval 0x%llx\n", epc, tval);
    
    // Handle breakpoint
    riscv64_handle_breakpoint(epc, tval);
}

// Load address misaligned exception
void riscv64_load_address_misaligned_handler(uint64_t cause, uint64_t epc, uint64_t tval)
{
    riscv64_exception_counters[RISCV64_EXC_LOAD_ADDRESS_MISALIGNED]++;
    printf("RISC-V64: Load address misaligned exception at PC 0x%llx, tval 0x%llx\n", epc, tval);
    
    // Handle misaligned load
    riscv64_handle_load_misaligned(epc, tval);
}

// Load access fault
void riscv64_load_access_fault_handler(uint64_t cause, uint64_t epc, uint64_t tval)
{
    riscv64_exception_counters[RISCV64_EXC_LOAD_ACCESS_FAULT]++;
    printf("RISC-V64: Load access fault at PC 0x%llx, tval 0x%llx\n", epc, tval);
    
    // Handle load access fault
    riscv64_handle_load_access_fault(epc, tval);
}

// Store address misaligned exception
void riscv64_store_address_misaligned_handler(uint64_t cause, uint64_t epc, uint64_t tval)
{
    riscv64_exception_counters[RISCV64_EXC_STORE_ADDRESS_MISALIGNED]++;
    printf("RISC-V64: Store address misaligned exception at PC 0x%llx, tval 0x%llx\n", epc, tval);
    
    // Handle misaligned store
    riscv64_handle_store_misaligned(epc, tval);
}

// Store access fault
void riscv64_store_access_fault_handler(uint64_t cause, uint64_t epc, uint64_t tval)
{
    riscv64_exception_counters[RISCV64_EXC_STORE_ACCESS_FAULT]++;
    printf("RISC-V64: Store access fault at PC 0x%llx, tval 0x%llx\n", epc, tval);
    
    // Handle store access fault
    riscv64_handle_store_access_fault(epc, tval);
}

// User mode ECALL
void riscv64_ecall_u_handler(uint64_t cause, uint64_t epc, uint64_t tval)
{
    riscv64_exception_counters[RISCV64_EXC_ECALL_U]++;
    printf("RISC-V64: User mode ECALL at PC 0x%llx\n", epc);
    
    // Handle user mode system call
    riscv64_handle_user_ecall(epc);
}

// Supervisor mode ECALL
void riscv64_ecall_s_handler(uint64_t cause, uint64_t epc, uint64_t tval)
{
    riscv64_exception_counters[RISCV64_EXC_ECALL_S]++;
    printf("RISC-V64: Supervisor mode ECALL at PC 0x%llx\n", epc);
    
    // Handle supervisor mode system call
    riscv64_handle_supervisor_ecall(epc);
}

// Machine mode ECALL
void riscv64_ecall_m_handler(uint64_t cause, uint64_t epc, uint64_t tval)
{
    riscv64_exception_counters[RISCV64_EXC_ECALL_M]++;
    printf("RISC-V64: Machine mode ECALL at PC 0x%llx\n", epc);
    
    // Handle machine mode system call
    riscv64_handle_machine_ecall(epc);
}

// Instruction page fault
void riscv64_instruction_page_fault_handler(uint64_t cause, uint64_t epc, uint64_t tval)
{
    riscv64_exception_counters[RISCV64_EXC_INSTRUCTION_PAGE_FAULT]++;
    printf("RISC-V64: Instruction page fault at PC 0x%llx, tval 0x%llx\n", epc, tval);
    
    // Handle instruction page fault
    riscv64_handle_instruction_page_fault(epc, tval);
}

// Load page fault
void riscv64_load_page_fault_handler(uint64_t cause, uint64_t epc, uint64_t tval)
{
    riscv64_exception_counters[RISCV64_EXC_LOAD_PAGE_FAULT]++;
    printf("RISC-V64: Load page fault at PC 0x%llx, tval 0x%llx\n", epc, tval);
    
    // Handle load page fault
    riscv64_handle_load_page_fault(epc, tval);
}

// Store page fault
void riscv64_store_page_fault_handler(uint64_t cause, uint64_t epc, uint64_t tval)
{
    riscv64_exception_counters[RISCV64_EXC_STORE_PAGE_FAULT]++;
    printf("RISC-V64: Store page fault at PC 0x%llx, tval 0x%llx\n", epc, tval);
    
    // Handle store page fault
    riscv64_handle_store_page_fault(epc, tval);
}

// ============================================================================
// INTERRUPT HANDLERS
// ============================================================================

// Software interrupt handler
void riscv64_software_interrupt_handler(void)
{
    riscv64_interrupt_counters[RISCV64_IRQ_SOFTWARE]++;
    printf("RISC-V64: Software interrupt received\n");
    
    // Handle software interrupt
    riscv64_handle_software_interrupt();
}

// Timer interrupt handler
void riscv64_timer_interrupt_handler(void)
{
    riscv64_interrupt_counters[RISCV64_IRQ_TIMER]++;
    
    // Handle timer interrupt
    riscv64_handle_timer_interrupt();
}

// External interrupt handler
void riscv64_external_interrupt_handler(void)
{
    riscv64_interrupt_counters[RISCV64_IRQ_EXTERNAL]++;
    printf("RISC-V64: External interrupt received\n");
    
    // Handle external interrupt
    riscv64_handle_external_interrupt();
}

// ============================================================================
// SYSTEM CALL DISPATCHER
// ============================================================================

// System call dispatcher
void riscv64_syscall_dispatcher(void)
{
    // Get system call number from a7 register
    uint64_t syscall_number;
    asm volatile("mv %0, a7" : "=r"(syscall_number));
    
    if (syscall_number >= 64) {
        printf("RISC-V64: Invalid system call number %llu\n", syscall_number);
        return;
    }
    
    riscv64_syscall_counters[syscall_number]++;
    
    // Call appropriate system call handler
    void (*handler)(void) = riscv64_syscall_table[syscall_number];
    if (handler) {
        handler();
    } else {
        printf("RISC-V64: Unimplemented system call %llu\n", syscall_number);
    }
}

// Fast system call handler
void riscv64_fast_syscall_handler(void)
{
    // Get system call number from a7 register
    uint64_t syscall_number;
    asm volatile("mv %0, a7" : "=r"(syscall_number));
    
    if (syscall_number >= 64) {
        return;
    }
    
    riscv64_syscall_counters[syscall_number]++;
    
    // Call appropriate system call handler
    void (*handler)(void) = riscv64_syscall_table[syscall_number];
    if (handler) {
        handler();
    }
}

// ============================================================================
// SYSTEM CALL IMPLEMENTATIONS
// ============================================================================

// Exit system call
void riscv64_syscall_exit(void)
{
    int exit_code;
    asm volatile("mv %0, a0" : "=r"(exit_code));
    
    printf("RISC-V64: Process exit with code %d\n", exit_code);
    riscv64_handle_process_exit(exit_code);
}

// Read system call
void riscv64_syscall_read(void)
{
    int fd;
    void *buf;
    size_t count;
    
    asm volatile("mv %0, a0" : "=r"(fd));
    asm volatile("mv %0, a1" : "=r"(buf));
    asm volatile("mv %0, a2" : "=r"(count));
    
    ssize_t result = riscv64_handle_read(fd, buf, count);
    
    // Set return value in a0
    asm volatile("mv a0, %0" : : "r"(result));
}

// Write system call
void riscv64_syscall_write(void)
{
    int fd;
    const void *buf;
    size_t count;
    
    asm volatile("mv %0, a0" : "=r"(fd));
    asm volatile("mv %0, a1" : "=r"(buf));
    asm volatile("mv %0, a2" : "=r"(count));
    
    ssize_t result = riscv64_handle_write(fd, buf, count);
    
    // Set return value in a0
    asm volatile("mv a0, %0" : : "r"(result));
}

// Open system call
void riscv64_syscall_open(void)
{
    const char *pathname;
    int flags;
    mode_t mode;
    
    asm volatile("mv %0, a0" : "=r"(pathname));
    asm volatile("mv %0, a1" : "=r"(flags));
    asm volatile("mv %0, a2" : "=r"(mode));
    
    int result = riscv64_handle_open(pathname, flags, mode);
    
    // Set return value in a0
    asm volatile("mv a0, %0" : : "r"(result));
}

// Close system call
void riscv64_syscall_close(void)
{
    int fd;
    asm volatile("mv %0, a0" : "=r"(fd));
    
    int result = riscv64_handle_close(fd);
    
    // Set return value in a0
    asm volatile("mv a0, %0" : : "r"(result));
}

// Fork system call
void riscv64_syscall_fork(void)
{
    pid_t result = riscv64_handle_fork();
    
    // Set return value in a0
    asm volatile("mv a0, %0" : : "r"(result));
}

// Exec system call
void riscv64_syscall_exec(void)
{
    const char *pathname;
    char *const argv[];
    char *const envp[];
    
    asm volatile("mv %0, a0" : "=r"(pathname));
    asm volatile("mv %0, a1" : "=r"(argv));
    asm volatile("mv %0, a2" : "=r"(envp));
    
    int result = riscv64_handle_exec(pathname, argv, envp);
    
    // Set return value in a0
    asm volatile("mv a0, %0" : : "r"(result));
}

// Wait system call
void riscv64_syscall_wait(void)
{
    int *status;
    asm volatile("mv %0, a0" : "=r"(status));
    
    pid_t result = riscv64_handle_wait(status);
    
    // Set return value in a0
    asm volatile("mv a0, %0" : : "r"(result));
}

// Mmap system call
void riscv64_syscall_mmap(void)
{
    void *addr;
    size_t length;
    int prot;
    int flags;
    int fd;
    off_t offset;
    
    asm volatile("mv %0, a0" : "=r"(addr));
    asm volatile("mv %0, a1" : "=r"(length));
    asm volatile("mv %0, a2" : "=r"(prot));
    asm volatile("mv %0, a3" : "=r"(flags));
    asm volatile("mv %0, a4" : "=r"(fd));
    asm volatile("mv %0, a5" : "=r"(offset));
    
    void *result = riscv64_handle_mmap(addr, length, prot, flags, fd, offset);
    
    // Set return value in a0
    asm volatile("mv a0, %0" : : "r"(result));
}

// Munmap system call
void riscv64_syscall_munmap(void)
{
    void *addr;
    size_t length;
    
    asm volatile("mv %0, a0" : "=r"(addr));
    asm volatile("mv %0, a1" : "=r"(length));
    
    int result = riscv64_handle_munmap(addr, length);
    
    // Set return value in a0
    asm volatile("mv a0, %0" : : "r"(result));
}

// Socket system call
void riscv64_syscall_socket(void)
{
    int domain;
    int type;
    int protocol;
    
    asm volatile("mv %0, a0" : "=r"(domain));
    asm volatile("mv %0, a1" : "=r"(type));
    asm volatile("mv %0, a2" : "=r"(protocol));
    
    int result = riscv64_handle_socket(domain, type, protocol);
    
    // Set return value in a0
    asm volatile("mv a0, %0" : : "r"(result));
}

// Bind system call
void riscv64_syscall_bind(void)
{
    int sockfd;
    const struct sockaddr *addr;
    socklen_t addrlen;
    
    asm volatile("mv %0, a0" : "=r"(sockfd));
    asm volatile("mv %0, a1" : "=r"(addr));
    asm volatile("mv %0, a2" : "=r"(addrlen));
    
    int result = riscv64_handle_bind(sockfd, addr, addrlen);
    
    // Set return value in a0
    asm volatile("mv a0, %0" : : "r"(result));
}

// Listen system call
void riscv64_syscall_listen(void)
{
    int sockfd;
    int backlog;
    
    asm volatile("mv %0, a0" : "=r"(sockfd));
    asm volatile("mv %0, a1" : "=r"(backlog));
    
    int result = riscv64_handle_listen(sockfd, backlog);
    
    // Set return value in a0
    asm volatile("mv a0, %0" : : "r"(result));
}

// Accept system call
void riscv64_syscall_accept(void)
{
    int sockfd;
    struct sockaddr *addr;
    socklen_t *addrlen;
    
    asm volatile("mv %0, a0" : "=r"(sockfd));
    asm volatile("mv %0, a1" : "=r"(addr));
    asm volatile("mv %0, a2" : "=r"(addrlen));
    
    int result = riscv64_handle_accept(sockfd, addr, addrlen);
    
    // Set return value in a0
    asm volatile("mv a0, %0" : : "r"(result));
}

// Connect system call
void riscv64_syscall_connect(void)
{
    int sockfd;
    const struct sockaddr *addr;
    socklen_t addrlen;
    
    asm volatile("mv %0, a0" : "=r"(sockfd));
    asm volatile("mv %0, a1" : "=r"(addr));
    asm volatile("mv %0, a2" : "=r"(addrlen));
    
    int result = riscv64_handle_connect(sockfd, addr, addrlen);
    
    // Set return value in a0
    asm volatile("mv a0, %0" : : "r"(result));
}

// Send system call
void riscv64_syscall_send(void)
{
    int sockfd;
    const void *buf;
    size_t len;
    int flags;
    
    asm volatile("mv %0, a0" : "=r"(sockfd));
    asm volatile("mv %0, a1" : "=r"(buf));
    asm volatile("mv %0, a2" : "=r"(len));
    asm volatile("mv %0, a3" : "=r"(flags));
    
    ssize_t result = riscv64_handle_send(sockfd, buf, len, flags);
    
    // Set return value in a0
    asm volatile("mv a0, %0" : : "r"(result));
}

// Recv system call
void riscv64_syscall_recv(void)
{
    int sockfd;
    void *buf;
    size_t len;
    int flags;
    
    asm volatile("mv %0, a0" : "=r"(sockfd));
    asm volatile("mv %0, a1" : "=r"(buf));
    asm volatile("mv %0, a2" : "=r"(len));
    asm volatile("mv %0, a3" : "=r"(flags));
    
    ssize_t result = riscv64_handle_recv(sockfd, buf, len, flags);
    
    // Set return value in a0
    asm volatile("mv a0, %0" : : "r"(result));
}

// ============================================================================
// EXCEPTION HANDLING FUNCTIONS
// ============================================================================

// Handle instruction misaligned
void riscv64_handle_instruction_misaligned(uint64_t epc, uint64_t tval)
{
    // Implementation for handling misaligned instructions
    // This could involve emulation or process termination
    printf("RISC-V64: Handling instruction misaligned at 0x%llx\n", epc);
}

// Handle instruction access fault
void riscv64_handle_instruction_access_fault(uint64_t epc, uint64_t tval)
{
    // Implementation for handling instruction access faults
    printf("RISC-V64: Handling instruction access fault at 0x%llx\n", epc);
}

// Handle illegal instruction
void riscv64_handle_illegal_instruction(uint64_t epc, uint64_t tval)
{
    // Implementation for handling illegal instructions
    printf("RISC-V64: Handling illegal instruction at 0x%llx\n", epc);
}

// Handle breakpoint
void riscv64_handle_breakpoint(uint64_t epc, uint64_t tval)
{
    // Implementation for handling breakpoints
    printf("RISC-V64: Handling breakpoint at 0x%llx\n", epc);
}

// Handle load misaligned
void riscv64_handle_load_misaligned(uint64_t epc, uint64_t tval)
{
    // Implementation for handling misaligned loads
    printf("RISC-V64: Handling load misaligned at 0x%llx\n", epc);
}

// Handle load access fault
void riscv64_handle_load_access_fault(uint64_t epc, uint64_t tval)
{
    // Implementation for handling load access faults
    printf("RISC-V64: Handling load access fault at 0x%llx\n", epc);
}

// Handle store misaligned
void riscv64_handle_store_misaligned(uint64_t epc, uint64_t tval)
{
    // Implementation for handling misaligned stores
    printf("RISC-V64: Handling store misaligned at 0x%llx\n", epc);
}

// Handle store access fault
void riscv64_handle_store_access_fault(uint64_t epc, uint64_t tval)
{
    // Implementation for handling store access faults
    printf("RISC-V64: Handling store access fault at 0x%llx\n", epc);
}

// Handle user ECALL
void riscv64_handle_user_ecall(uint64_t epc)
{
    // Implementation for handling user mode system calls
    printf("RISC-V64: Handling user mode ECALL at 0x%llx\n", epc);
}

// Handle supervisor ECALL
void riscv64_handle_supervisor_ecall(uint64_t epc)
{
    // Implementation for handling supervisor mode system calls
    printf("RISC-V64: Handling supervisor mode ECALL at 0x%llx\n", epc);
}

// Handle machine ECALL
void riscv64_handle_machine_ecall(uint64_t epc)
{
    // Implementation for handling machine mode system calls
    printf("RISC-V64: Handling machine mode ECALL at 0x%llx\n", epc);
}

// Handle instruction page fault
void riscv64_handle_instruction_page_fault(uint64_t epc, uint64_t tval)
{
    // Implementation for handling instruction page faults
    printf("RISC-V64: Handling instruction page fault at 0x%llx\n", epc);
}

// Handle load page fault
void riscv64_handle_load_page_fault(uint64_t epc, uint64_t tval)
{
    // Implementation for handling load page faults
    printf("RISC-V64: Handling load page fault at 0x%llx\n", epc);
}

// Handle store page fault
void riscv64_handle_store_page_fault(uint64_t epc, uint64_t tval)
{
    // Implementation for handling store page faults
    printf("RISC-V64: Handling store page fault at 0x%llx\n", epc);
}

// ============================================================================
// INTERRUPT HANDLING FUNCTIONS
// ============================================================================

// Handle software interrupt
void riscv64_handle_software_interrupt(void)
{
    // Implementation for handling software interrupts
    printf("RISC-V64: Handling software interrupt\n");
}

// Handle timer interrupt
void riscv64_handle_timer_interrupt(void)
{
    // Implementation for handling timer interrupts
    // This is typically handled by the timer subsystem
}

// Handle external interrupt
void riscv64_handle_external_interrupt(void)
{
    // Implementation for handling external interrupts
    printf("RISC-V64: Handling external interrupt\n");
}

// ============================================================================
// SYSTEM CALL HANDLING FUNCTIONS
// ============================================================================

// Handle process exit
void riscv64_handle_process_exit(int exit_code)
{
    // Implementation for handling process exit
    printf("RISC-V64: Handling process exit with code %d\n", exit_code);
}

// Handle read operation
ssize_t riscv64_handle_read(int fd, void *buf, size_t count)
{
    // Implementation for handling read system call
    printf("RISC-V64: Handling read from fd %d, count %zu\n", fd, count);
    return -1; // Placeholder
}

// Handle write operation
ssize_t riscv64_handle_write(int fd, const void *buf, size_t count)
{
    // Implementation for handling write system call
    printf("RISC-V64: Handling write to fd %d, count %zu\n", fd, count);
    return -1; // Placeholder
}

// Handle open operation
int riscv64_handle_open(const char *pathname, int flags, mode_t mode)
{
    // Implementation for handling open system call
    printf("RISC-V64: Handling open of %s with flags %d\n", pathname, flags);
    return -1; // Placeholder
}

// Handle close operation
int riscv64_handle_close(int fd)
{
    // Implementation for handling close system call
    printf("RISC-V64: Handling close of fd %d\n", fd);
    return -1; // Placeholder
}

// Handle fork operation
pid_t riscv64_handle_fork(void)
{
    // Implementation for handling fork system call
    printf("RISC-V64: Handling fork\n");
    return -1; // Placeholder
}

// Handle exec operation
int riscv64_handle_exec(const char *pathname, char *const argv[], char *const envp[])
{
    // Implementation for handling exec system call
    printf("RISC-V64: Handling exec of %s\n", pathname);
    return -1; // Placeholder
}

// Handle wait operation
pid_t riscv64_handle_wait(int *status)
{
    // Implementation for handling wait system call
    printf("RISC-V64: Handling wait\n");
    return -1; // Placeholder
}

// Handle mmap operation
void *riscv64_handle_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    // Implementation for handling mmap system call
    printf("RISC-V64: Handling mmap of length %zu\n", length);
    return MAP_FAILED; // Placeholder
}

// Handle munmap operation
int riscv64_handle_munmap(void *addr, size_t length)
{
    // Implementation for handling munmap system call
    printf("RISC-V64: Handling munmap of length %zu\n", length);
    return -1; // Placeholder
}

// Handle socket operation
int riscv64_handle_socket(int domain, int type, int protocol)
{
    // Implementation for handling socket system call
    printf("RISC-V64: Handling socket creation\n");
    return -1; // Placeholder
}

// Handle bind operation
int riscv64_handle_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    // Implementation for handling bind system call
    printf("RISC-V64: Handling bind\n");
    return -1; // Placeholder
}

// Handle listen operation
int riscv64_handle_listen(int sockfd, int backlog)
{
    // Implementation for handling listen system call
    printf("RISC-V64: Handling listen\n");
    return -1; // Placeholder
}

// Handle accept operation
int riscv64_handle_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    // Implementation for handling accept system call
    printf("RISC-V64: Handling accept\n");
    return -1; // Placeholder
}

// Handle connect operation
int riscv64_handle_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    // Implementation for handling connect system call
    printf("RISC-V64: Handling connect\n");
    return -1; // Placeholder
}

// Handle send operation
ssize_t riscv64_handle_send(int sockfd, const void *buf, size_t len, int flags)
{
    // Implementation for handling send system call
    printf("RISC-V64: Handling send of length %zu\n", len);
    return -1; // Placeholder
}

// Handle recv operation
ssize_t riscv64_handle_recv(int sockfd, void *buf, size_t len, int flags)
{
    // Implementation for handling recv system call
    printf("RISC-V64: Handling recv of length %zu\n", len);
    return -1; // Placeholder
}

// ============================================================================
// STATISTICS FUNCTIONS
// ============================================================================

// Get exception counter
uint32_t riscv64_get_exception_counter(uint32_t exception)
{
    if (exception >= 16) {
        return 0;
    }
    return riscv64_exception_counters[exception];
}

// Get interrupt counter
uint32_t riscv64_get_interrupt_counter(uint32_t irq)
{
    if (irq >= 1024) {
        return 0;
    }
    return riscv64_interrupt_counters[irq];
}

// Get system call counter
uint32_t riscv64_get_syscall_counter(uint32_t syscall)
{
    if (syscall >= 64) {
        return 0;
    }
    return riscv64_syscall_counters[syscall];
}

// Print all statistics
void riscv64_print_interrupt_statistics(void)
{
    printf("RISC-V64: Interrupt Statistics\n");
    printf("  Exception Counters:\n");
    for (int i = 0; i < 16; i++) {
        if (riscv64_exception_counters[i] > 0) {
            printf("    Exception %d: %u\n", i, riscv64_exception_counters[i]);
        }
    }
    
    printf("  System Call Counters:\n");
    for (int i = 0; i < 64; i++) {
        if (riscv64_syscall_counters[i] > 0) {
            printf("    Syscall %d: %u\n", i, riscv64_syscall_counters[i]);
        }
    }
}
