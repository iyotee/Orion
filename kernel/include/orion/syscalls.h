#ifndef ORION_SYSCALLS_H
#define ORION_SYSCALLS_H

#include <orion/types.h>

// Orion system call numbers (< 60 total for v1.0)

// Category: Process/Thread (0-9)
#define SYS_EXIT 0          // Terminate process
#define SYS_YIELD 1         // Yield CPU
#define SYS_PROC_CREATE 2   // Create process
#define SYS_THREAD_CREATE 3 // Create thread
#define SYS_WAIT 4          // Wait for process/thread
#define SYS_SIGNAL 5        // Send signal
#define SYS_GETPID 6        // Get PID
#define SYS_GETTID 7        // Get TID

// Category: Memory (10-19)
#define SYS_VM_MAP 10     // Map virtual memory
#define SYS_VM_UNMAP 11   // Unmap memory
#define SYS_VM_PROTECT 12 // Change permissions
#define SYS_SHM_CREATE 13 // Create shared memory
#define SYS_SHM_ATTACH 14 // Attach shared memory
#define SYS_SHM_DETACH 15 // Detach shared memory
#define SYS_MADVISE 16    // Memory advice

// Category: IPC (20-29)
#define SYS_PORT_CREATE 20 // Create IPC port
#define SYS_PORT_SEND 21   // Send message
#define SYS_PORT_RECV 22   // Receive message
#define SYS_PORT_SHARE 23  // Share port
#define SYS_MSG_FORWARD 24 // Forward message

// Category: Time (30-34)
#define SYS_CLOCK_GET 30    // Read clock
#define SYS_TIMER_CREATE 31 // Create timer
#define SYS_TIMER_START 32  // Start timer
#define SYS_TIMER_STOP 33   // Stop timer
#define SYS_NANOSLEEP 34    // Sleep

// Category: Abstract I/O (35-39)
#define SYS_OPEN 35      // Open file
#define SYS_IO_SUBMIT 36 // Submit I/O
#define SYS_IO_POLL 37   // Poll I/O
#define SYS_IO_CANCEL 38 // Cancel I/O

// Category: Objects (40-44)
#define SYS_OBJ_INFO 40  // Object info
#define SYS_OBJ_DUP 41   // Duplicate handle
#define SYS_OBJ_CLOSE 42 // Close handle

// Category: Security (45-49)
#define SYS_CAP_GRANT 45    // Grant capability
#define SYS_CAP_REVOKE 46   // Revoke capability
#define SYS_CAP_QUERY 47    // Check capability
#define SYS_SANDBOX_LOAD 48 // Load sandbox
#define SYS_AUDIT_EMIT 49   // Emit audit

// Category: Miscellaneous (50-59)
#define SYS_INFO 50      // System info
#define SYS_DBG_TRACE 51 // Debug trace
#define SYS_RANDOM 52    // Get entropy

// Maximum number of system calls
#define MAX_SYSCALLS 60

// Structures for system calls

// Structures defined in types.h
// or_system_info_t, or_vm_map_t, or_msg_send_t, or_msg_recv_t

// Flags for vm_map
#define VM_PROT_READ (1 << 0)
#define VM_PROT_WRITE (1 << 1)
#define VM_PROT_EXEC (1 << 2)

#define VM_MAP_PRIVATE (1 << 0)
#define VM_MAP_SHARED (1 << 1)
#define VM_MAP_FIXED (1 << 2)
#define VM_MAP_ANONYMOUS (1 << 3)

// System V x86_64 calling convention:
// RDI, RSI, RDX, RCX, R8, R9 for first 6 arguments
// RAX = system call number
// Return in RAX (int64_t)

// Wrapper for calls from C code (userland)
#ifdef _MSC_VER
// For MSVC, we use temporary stubs (will be replaced by dedicated assembly)
static inline int64_t orion_syscall0(uint64_t num)
{
    (void)num;
    return -1;
}
static inline int64_t orion_syscall1(uint64_t num, uint64_t arg1)
{
    (void)num;
    (void)arg1;
    return -1;
}
static inline int64_t orion_syscall2(uint64_t num, uint64_t arg1, uint64_t arg2)
{
    (void)num;
    (void)arg1;
    (void)arg2;
    return -1;
}
static inline int64_t orion_syscall3(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
    (void)num;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    return -1;
}
#else
static inline int64_t orion_syscall0(uint64_t num)
{
    int64_t ret;
    __asm__ volatile(
        "syscall"
        : "=a"(ret)
        : "a"(num)
        : "rcx", "r11", "memory");
    return ret;
}

static inline int64_t orion_syscall1(uint64_t num, uint64_t arg1)
{
    int64_t ret;
    __asm__ volatile(
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(arg1)
        : "rcx", "r11", "memory");
    return ret;
}

static inline int64_t orion_syscall2(uint64_t num, uint64_t arg1, uint64_t arg2)
{
    int64_t ret;
    __asm__ volatile(
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(arg1), "S"(arg2)
        : "rcx", "r11", "memory");
    return ret;
}

static inline int64_t orion_syscall3(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
    int64_t ret;
    __asm__ volatile(
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3)
        : "rcx", "r11", "memory");
    return ret;
}
#endif

// Functions for use in the kernel
void syscalls_init(void);
int64_t syscall_handler(uint64_t syscall_num, uint64_t arg1, uint64_t arg2,
                        uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);

// Individual handlers (defined in syscalls.c)
int64_t sys_info_impl(or_system_info_t *info);
int64_t sys_exit_impl(int exit_code);
int64_t sys_yield_impl(void);
int64_t sys_vm_map_impl(or_vm_map_t *map_params);
int64_t sys_port_create_impl(or_cap_t *out_port);
int64_t sys_port_send_impl(or_msg_send_t *msg);
int64_t sys_port_recv_impl(or_msg_recv_t *msg);

// Process/Thread syscalls
int64_t sys_proc_create_impl(const char *executable_path, char *const argv[], char *const envp[]);
int64_t sys_thread_create_impl(uint64_t entry_point, uint64_t stack_pointer, uint64_t arg);
int64_t sys_wait_impl(uint64_t pid, int *status, uint64_t options);
int64_t sys_signal_impl(uint64_t target_pid, uint32_t signal_num);
int64_t sys_getpid_impl(void);
int64_t sys_gettid_impl(void);

// Memory syscalls
int64_t sys_vm_unmap_impl(uint64_t addr, size_t length);
int64_t sys_vm_protect_impl(uint64_t addr, size_t length, uint32_t new_prot);
int64_t sys_shm_create_impl(size_t size, uint32_t flags);
int64_t sys_shm_attach_impl(or_cap_t shm_cap, uint64_t addr, uint32_t flags);
int64_t sys_shm_detach_impl(uint64_t addr);
int64_t sys_madvise_impl(uint64_t addr, size_t length, uint32_t advice);

// IPC syscalls
int64_t sys_port_share_impl(or_cap_t port, uint64_t target_pid);
int64_t sys_msg_forward_impl(or_cap_t source_port, or_cap_t dest_port);

// Time syscalls
int64_t sys_clock_get_impl(uint32_t clock_id, uint64_t *timestamp);
int64_t sys_timer_create_impl(uint32_t clock_id, uint64_t *timer_id);
int64_t sys_timer_start_impl(uint64_t timer_id, uint64_t timeout_ns, uint64_t interval_ns);
int64_t sys_timer_stop_impl(uint64_t timer_id);
int64_t sys_nanosleep_impl(uint64_t nanoseconds);

// I/O syscalls
int64_t sys_io_submit_impl(void *io_context, uint32_t nr, void *iocbpp);
int64_t sys_io_poll_impl(void *io_context, uint32_t min_nr, uint32_t max_nr, void *events, uint64_t timeout_ns);
int64_t sys_io_cancel_impl(void *io_context, void *iocb);

// Object syscalls
int64_t sys_obj_info_impl(or_cap_t cap, void *info_buffer, size_t buffer_size);
int64_t sys_obj_dup_impl(or_cap_t cap);
int64_t sys_obj_close_impl(or_cap_t cap);

// Security syscalls
int64_t sys_cap_grant_impl(uint64_t target_pid, or_cap_t cap, uint32_t permissions);
int64_t sys_cap_revoke_impl(uint64_t target_pid, or_cap_t cap);
int64_t sys_cap_query_impl(or_cap_t cap, uint32_t *permissions);
int64_t sys_sandbox_load_impl(const char *policy_path);
int64_t sys_audit_emit_impl(uint32_t event_type, const void *event_data, size_t data_size);

// Misc syscalls
int64_t sys_dbg_trace_impl(uint32_t trace_type, const void *trace_data, size_t data_size);
int64_t sys_random_impl(void *buffer, size_t size);

#endif // ORION_SYSCALLS_H
