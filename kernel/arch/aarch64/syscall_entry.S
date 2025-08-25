/*
 * Orion Operating System - aarch64 System Call Entry
 *
 * System call entry point and dispatcher for aarch64 architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "config.h"

// ============================================================================
// SYSTEM CALL NUMBERS
// ============================================================================

// Standard system calls
#define SYS_READ           0
#define SYS_WRITE          1
#define SYS_OPEN           2
#define SYS_CLOSE          3
#define SYS_FORK           4
#define SYS_EXEC           5
#define SYS_EXIT           6
#define SYS_WAIT           7
#define SYS_KILL           8
#define SYS_GETPID         9
#define SYS_SLEEP          10
#define SYS_YIELD          11
#define SYS_MMAP           12
#define SYS_MUNMAP         13
#define SYS_BRK            14
#define SYS_GETTIMEOFDAY   15
#define SYS_SETTIMEOFDAY   16
#define SYS_IOCTL          17
#define SYS_FCNTL          18
#define SYS_DUP            19
#define SYS_DUP2           20
#define SYS_PIPE           21
#define SYS_CHDIR          22
#define SYS_GETCWD         23
#define SYS_LINK           24
#define SYS_UNLINK         25
#define SYS_RENAME         26
#define SYS_MKDIR          27
#define SYS_RMDIR          28
#define SYS_READDIR        29
#define SYS_STAT           30
#define SYS_FSTAT          31
#define SYS_LSTAT          32
#define SYS_ACCESS         33
#define SYS_CHMOD          34
#define SYS_FCHMOD         35
#define SYS_CHOWN          36
#define SYS_FCHOWN         37
#define SYS_UMASK          38
#define SYS_GETUID         39
#define SYS_SETUID         40
#define SYS_GETGID         41
#define SYS_SETGID         42
#define SYS_GETEUID        43
#define SYS_GETEGID        44
#define SYS_SETREUID       45
#define SYS_SETREGID       46
#define SYS_GETGROUPS      47
#define SYS_SETGROUPS      48
#define SYS_SIGNAL         49
#define SYS_SIGACTION      50
#define SYS_SIGPROCMASK    51
#define SYS_SIGSUSPEND     52
#define SYS_SIGPENDING     53
#define SYS_SIGTIMEDWAIT   54
#define SYS_SIGWAITINFO    55
#define SYS_SIGQUEUE       56
#define SYS_SIGRETURN      57
#define SYS_SOCKET         58
#define SYS_BIND           59
#define SYS_CONNECT        60
#define SYS_LISTEN         61
#define SYS_ACCEPT         62
#define SYS_SEND           63
#define SYS_RECV           64
#define SYS_SENDTO         65
#define SYS_RECVFROM       66
#define SYS_SHUTDOWN       67
#define SYS_GETSOCKOPT     68
#define SYS_SETSOCKOPT     69
#define SYS_GETPEERNAME    70
#define SYS_GETSOCKNAME    71
#define SYS_SOCKETPAIR     72
#define SYS_EPOLL_CREATE   73
#define SYS_EPOLL_CTL      74
#define SYS_EPOLL_WAIT     75
#define SYS_EPOLL_PWAIT    76
#define SYS_INOTIFY_INIT   77
#define SYS_INOTIFY_ADD_WATCH 78
#define SYS_INOTIFY_RM_WATCH  79
#define SYS_TIMER_CREATE   80
#define SYS_TIMER_SETTIME  81
#define SYS_TIMER_GETTIME  82
#define SYS_TIMER_DELETE   83
#define SYS_CLOCK_GETTIME  84
#define SYS_CLOCK_SETTIME  85
#define SYS_CLOCK_GETRES   86
#define SYS_CLOCK_NANOSLEEP 87
#define SYS_GETRUSAGE      88
#define SYS_SETPRIORITY    89
#define SYS_GETPRIORITY    90
#define SYS_SCHED_SETSCHEDULER 91
#define SYS_SCHED_GETSCHEDULER 92
#define SYS_SCHED_SETPARAM 93
#define SYS_SCHED_GETPARAM 94
#define SYS_SCHED_SETAFFINITY 95
#define SYS_SCHED_GETAFFINITY 96
#define SYS_SCHED_YIELD    97
#define SYS_SCHED_GET_PRIORITY_MAX 98
#define SYS_SCHED_GET_PRIORITY_MIN 99
#define SYS_SCHED_RR_GET_INTERVAL 100

// ORION-specific system calls
#define SYS_ORION_GET_CAPABILITY    1000
#define SYS_ORION_SET_CAPABILITY    1001
#define SYS_ORION_CREATE_NAMESPACE  1002
#define SYS_ORION_DESTROY_NAMESPACE 1003
#define SYS_ORION_JOIN_NAMESPACE    1004
#define SYS_ORION_LEAVE_NAMESPACE   1005
#define SYS_ORION_CREATE_CHANNEL    1006
#define SYS_ORION_DESTROY_CHANNEL   1007
#define SYS_ORION_SEND_MESSAGE      1008
#define SYS_ORION_RECEIVE_MESSAGE   1009
#define SYS_ORION_CREATE_THREAD     1010
#define SYS_ORION_DESTROY_THREAD    1011
#define SYS_ORION_THREAD_YIELD      1012
#define SYS_ORION_THREAD_SLEEP      1013
#define SYS_ORION_THREAD_WAKE       1014
#define SYS_ORION_GET_THREAD_INFO   1015
#define SYS_ORION_SET_THREAD_PRIORITY 1016
#define SYS_ORION_GET_THREAD_PRIORITY 1017
#define SYS_ORION_CREATE_MUTEX      1018
#define SYS_ORION_DESTROY_MUTEX     1019
#define SYS_ORION_LOCK_MUTEX        1020
#define SYS_ORION_UNLOCK_MUTEX      1021
#define SYS_ORION_CREATE_SEMAPHORE  1022
#define SYS_ORION_DESTROY_SEMAPHORE 1023
#define SYS_ORION_WAIT_SEMAPHORE    1024
#define SYS_ORION_SIGNAL_SEMAPHORE  1025
#define SYS_ORION_CREATE_EVENT      1026
#define SYS_ORION_DESTROY_EVENT     1027
#define SYS_ORION_WAIT_EVENT        1028
#define SYS_ORION_SIGNAL_EVENT      1029
#define SYS_ORION_CREATE_TIMER      1030
#define SYS_ORION_DESTROY_TIMER     1031
#define SYS_ORION_START_TIMER       1032
#define SYS_ORION_STOP_TIMER        1033
#define SYS_ORION_GET_TIMER_INFO    1034
#define SYS_ORION_CREATE_MEMORY_POOL 1035
#define SYS_ORION_DESTROY_MEMORY_POOL 1036
#define SYS_ORION_ALLOCATE_MEMORY   1037
#define SYS_ORION_FREE_MEMORY       1038
#define SYS_ORION_GET_MEMORY_INFO   1039
#define SYS_ORION_CREATE_DEVICE     1040
#define SYS_ORION_DESTROY_DEVICE    1041
#define SYS_ORION_OPEN_DEVICE       1042
#define SYS_ORION_CLOSE_DEVICE      1043
#define SYS_ORION_READ_DEVICE       1044
#define SYS_ORION_WRITE_DEVICE      1045
#define SYS_ORION_IOCTL_DEVICE      1046
#define SYS_ORION_GET_DEVICE_INFO   1047
#define SYS_ORION_CREATE_FILE       1048
#define SYS_ORION_DESTROY_FILE      1049
#define SYS_ORION_OPEN_FILE         1050
#define SYS_ORION_CLOSE_FILE        1051
#define SYS_ORION_READ_FILE         1052
#define SYS_ORION_WRITE_FILE        1053
#define SYS_ORION_SEEK_FILE         1054
#define SYS_ORION_GET_FILE_INFO     1055
#define SYS_ORION_SET_FILE_INFO     1056
#define SYS_ORION_CREATE_DIRECTORY  1057
#define SYS_ORION_DESTROY_DIRECTORY 1058
#define SYS_ORION_READ_DIRECTORY    1059
#define SYS_ORION_GET_DIRECTORY_INFO 1060
#define SYS_ORION_CREATE_SYMBOLIC_LINK 1061
#define SYS_ORION_DESTROY_SYMBOLIC_LINK 1062
#define SYS_ORION_READ_SYMBOLIC_LINK 1063
#define SYS_ORION_CREATE_HARD_LINK  1064
#define SYS_ORION_DESTROY_HARD_LINK 1065
#define SYS_ORION_MOUNT_FILESYSTEM  1066
#define SYS_ORION_UNMOUNT_FILESYSTEM 1067
#define SYS_ORION_GET_FILESYSTEM_INFO 1068
#define SYS_ORION_SET_FILESYSTEM_INFO 1069
#define SYS_ORION_CREATE_NETWORK_INTERFACE 1070
#define SYS_ORION_DESTROY_NETWORK_INTERFACE 1071
#define SYS_ORION_GET_NETWORK_INTERFACE_INFO 1072
#define SYS_ORION_SET_NETWORK_INTERFACE_INFO 1073
#define SYS_ORION_CREATE_NETWORK_SOCKET 1074
#define SYS_ORION_DESTROY_NETWORK_SOCKET 1075
#define SYS_ORION_BIND_NETWORK_SOCKET 1076
#define SYS_ORION_CONNECT_NETWORK_SOCKET 1077
#define SYS_ORION_LISTEN_NETWORK_SOCKET 1078
#define SYS_ORION_ACCEPT_NETWORK_SOCKET 1079
#define SYS_ORION_SEND_NETWORK_MESSAGE 1080
#define SYS_ORION_RECEIVE_NETWORK_MESSAGE 1081
#define SYS_ORION_GET_NETWORK_STATISTICS 1082
#define SYS_ORION_SET_NETWORK_STATISTICS 1083
#define SYS_ORION_CREATE_PROCESS    1084
#define SYS_ORION_DESTROY_PROCESS   1085
#define SYS_ORION_GET_PROCESS_INFO  1086
#define SYS_ORION_SET_PROCESS_INFO  1087
#define SYS_ORION_CREATE_PROCESS_NAMESPACE 1088
#define SYS_ORION_DESTROY_PROCESS_NAMESPACE 1089
#define SYS_ORION_JOIN_PROCESS_NAMESPACE 1090
#define SYS_ORION_LEAVE_PROCESS_NAMESPACE 1091
#define SYS_ORION_CREATE_PROCESS_CHANNEL 1092
#define SYS_ORION_DESTROY_PROCESS_CHANNEL 1093
#define SYS_ORION_SEND_PROCESS_MESSAGE 1094
#define SYS_ORION_RECEIVE_PROCESS_MESSAGE 1095
#define SYS_ORION_GET_PROCESS_STATISTICS 1096
#define SYS_ORION_SET_PROCESS_STATISTICS 1097
#define SYS_ORION_CREATE_PROCESS_TIMER 1098
#define SYS_ORION_DESTROY_PROCESS_TIMER 1099

// ============================================================================
// SYSTEM CALL ENTRY POINT
// ============================================================================

.section .text.syscall, "ax"
.global aarch64_syscall_entry

aarch64_syscall_entry:
    // Save all registers that might be clobbered
    stp x0, x1, [sp, #-16]!
    stp x2, x3, [sp, #-16]!
    stp x4, x5, [sp, #-16]!
    stp x6, x7, [sp, #-16]!
    stp x8, x9, [sp, #-16]!
    stp x10, x11, [sp, #-16]!
    stp x12, x13, [sp, #-16]!
    stp x14, x15, [sp, #-16]!
    stp x16, x17, [sp, #-16]!
    stp x18, x19, [sp, #-16]!
    stp x20, x21, [sp, #-16]!
    stp x22, x23, [sp, #-16]!
    stp x24, x25, [sp, #-16]!
    stp x26, x27, [sp, #-16]!
    stp x28, x29, [sp, #-16]!
    stp x30, lr, [sp, #-16]!
    
    // Save system call number and arguments
    mov x9, x8      // System call number
    mov x10, x0     // First argument
    mov x11, x1     // Second argument
    mov x12, x2     // Third argument
    mov x13, x3     // Fourth argument
    mov x14, x4     // Fifth argument
    mov x15, x5     // Sixth argument
    
    // Check if system call number is valid
    cmp x9, #SYS_ORION_CREATE_PROCESS_TIMER
    b.hi aarch64_syscall_invalid
    
    // Dispatch to appropriate handler based on system call number
    adr x8, aarch64_syscall_table
    ldr x8, [x8, x9, lsl #3]  // Load handler address
    blr x8                     // Branch to handler
    
    // Restore registers
    ldp x30, lr, [sp], #16
    ldp x28, x29, [sp], #16
    ldp x26, x27, [sp], #16
    ldp x24, x25, [sp], #16
    ldp x22, x23, [sp], #16
    ldp x20, x21, [sp], #16
    ldp x18, x19, [sp], #16
    ldp x16, x17, [sp], #16
    ldp x14, x15, [sp], #16
    ldp x12, x13, [sp], #16
    ldp x10, x11, [sp], #16
    ldp x8, x9, [sp], #16
    ldp x6, x7, [sp], #16
    ldp x4, x5, [sp], #16
    ldp x2, x3, [sp], #16
    ldp x0, x1, [sp], #16
    
    // Return to user space
    eret

aarch64_syscall_invalid:
    // Invalid system call number
    mov x0, #-1     // Return error
    mov x1, #22     // EINVAL
    
    // Restore registers and return
    ldp x30, lr, [sp], #16
    ldp x28, x29, [sp], #16
    ldp x26, x27, [sp], #16
    ldp x24, x25, [sp], #16
    ldp x22, x23, [sp], #16
    ldp x20, x21, [sp], #16
    ldp x18, x19, [sp], #16
    ldp x16, x17, [sp], #16
    ldp x14, x15, [sp], #16
    ldp x12, x13, [sp], #16
    ldp x10, x11, [sp], #16
    ldp x8, x9, [sp], #16
    ldp x6, x7, [sp], #16
    ldp x4, x5, [sp], #16
    ldp x2, x3, [sp], #16
    ldp x0, x1, [sp], #16
    
    eret

// ============================================================================
// SYSTEM CALL HANDLERS
// ============================================================================

aarch64_syscall_read:
    // Read system call
    // x10 = fd, x11 = buf, x12 = count
    bl aarch64_syscall_read_impl
    ret

aarch64_syscall_write:
    // Write system call
    // x10 = fd, x11 = buf, x12 = count
    bl aarch64_syscall_write_impl
    ret

aarch64_syscall_open:
    // Open system call
    // x10 = pathname, x11 = flags, x12 = mode
    bl aarch64_syscall_open_impl
    ret

aarch64_syscall_close:
    // Close system call
    // x10 = fd
    bl aarch64_syscall_close_impl
    ret

aarch64_syscall_fork:
    // Fork system call
    bl aarch64_syscall_fork_impl
    ret

aarch64_syscall_exec:
    // Exec system call
    // x10 = pathname, x11 = argv, x12 = envp
    bl aarch64_syscall_exec_impl
    ret

aarch64_syscall_exit:
    // Exit system call
    // x10 = status
    bl aarch64_syscall_exit_impl
    ret

aarch64_syscall_wait:
    // Wait system call
    // x10 = pid, x11 = status, x12 = options
    bl aarch64_syscall_wait_impl
    ret

// ============================================================================
// ORION-SPECIFIC SYSTEM CALL HANDLERS
// ============================================================================

aarch64_syscall_orion_get_capability:
    // Get capability system call
    // x10 = capability_id
    bl aarch64_syscall_orion_get_capability_impl
    ret

aarch64_syscall_orion_set_capability:
    // Set capability system call
    // x10 = capability_id, x11 = value
    bl aarch64_syscall_orion_set_capability_impl
    ret

aarch64_syscall_orion_create_namespace:
    // Create namespace system call
    // x10 = namespace_type, x11 = name, x12 = flags
    bl aarch64_syscall_orion_create_namespace_impl
    ret

aarch64_syscall_orion_destroy_namespace:
    // Destroy namespace system call
    // x10 = namespace_id
    bl aarch64_syscall_orion_destroy_namespace_impl
    ret

aarch64_syscall_orion_create_channel:
    // Create channel system call
    // x10 = channel_type, x11 = flags
    bl aarch64_syscall_orion_create_channel_impl
    ret

aarch64_syscall_orion_destroy_channel:
    // Destroy channel system call
    // x10 = channel_id
    bl aarch64_syscall_orion_destroy_channel_impl
    ret

aarch64_syscall_orion_send_message:
    // Send message system call
    // x10 = channel_id, x11 = message, x12 = size
    bl aarch64_syscall_orion_send_message_impl
    ret

aarch64_syscall_orion_receive_message:
    // Receive message system call
    // x10 = channel_id, x11 = buffer, x12 = size
    bl aarch64_syscall_orion_receive_message_impl
    ret

// ============================================================================
// SYSTEM CALL TABLE
// ============================================================================

.section .rodata
.align 8

aarch64_syscall_table:
    // Standard system calls (0-100)
    .quad aarch64_syscall_read
    .quad aarch64_syscall_write
    .quad aarch64_syscall_open
    .quad aarch64_syscall_close
    .quad aarch64_syscall_fork
    .quad aarch64_syscall_exec
    .quad aarch64_syscall_exit
    .quad aarch64_syscall_wait
    .quad aarch64_syscall_invalid  // kill
    .quad aarch64_syscall_invalid  // getpid
    .quad aarch64_syscall_invalid  // sleep
    .quad aarch64_syscall_invalid  // yield
    .quad aarch64_syscall_invalid  // mmap
    .quad aarch64_syscall_invalid  // munmap
    .quad aarch64_syscall_invalid  // brk
    .quad aarch64_syscall_invalid  // gettimeofday
    .quad aarch64_syscall_invalid  // settimeofday
    .quad aarch64_syscall_invalid  // ioctl
    .quad aarch64_syscall_invalid  // fcntl
    .quad aarch64_syscall_invalid  // dup
    .quad aarch64_syscall_invalid  // dup2
    .quad aarch64_syscall_invalid  // pipe
    .quad aarch64_syscall_invalid  // chdir
    .quad aarch64_syscall_invalid  // getcwd
    .quad aarch64_syscall_invalid  // link
    .quad aarch64_syscall_invalid  // unlink
    .quad aarch64_syscall_invalid  // rename
    .quad aarch64_syscall_invalid  // mkdir
    .quad aarch64_syscall_invalid  // rmdir
    .quad aarch64_syscall_invalid  // readdir
    .quad aarch64_syscall_invalid  // stat
    .quad aarch64_syscall_invalid  // fstat
    .quad aarch64_syscall_invalid  // lstat
    .quad aarch64_syscall_invalid  // access
    .quad aarch64_syscall_invalid  // chmod
    .quad aarch64_syscall_invalid  // fchmod
    .quad aarch64_syscall_invalid  // chown
    .quad aarch64_syscall_invalid  // fchown
    .quad aarch64_syscall_invalid  // umask
    .quad aarch64_syscall_invalid  // getuid
    .quad aarch64_syscall_invalid  // setuid
    .quad aarch64_syscall_invalid  // getgid
    .quad aarch64_syscall_invalid  // setgid
    .quad aarch64_syscall_invalid  // geteuid
    .quad aarch64_syscall_invalid  // getegid
    .quad aarch64_syscall_invalid  // setreuid
    .quad aarch64_syscall_invalid  // setregid
    .quad aarch64_syscall_invalid  // getgroups
    .quad aarch64_syscall_invalid  // setgroups
    .quad aarch64_syscall_invalid  // signal
    .quad aarch64_syscall_invalid  // sigaction
    .quad aarch64_syscall_invalid  // sigprocmask
    .quad aarch64_syscall_invalid  // sigsuspend
    .quad aarch64_syscall_invalid  // sigpending
    .quad aarch64_syscall_invalid  // sigtimedwait
    .quad aarch64_syscall_invalid  // sigwaitinfo
    .quad aarch64_syscall_invalid  // sigqueue
    .quad aarch64_syscall_invalid  // sigreturn
    .quad aarch64_syscall_invalid  // socket
    .quad aarch64_syscall_invalid  // bind
    .quad aarch64_syscall_invalid  // connect
    .quad aarch64_syscall_invalid  // listen
    .quad aarch64_syscall_invalid  // accept
    .quad aarch64_syscall_invalid  // send
    .quad aarch64_syscall_invalid  // recv
    .quad aarch64_syscall_invalid  // sendto
    .quad aarch64_syscall_invalid  // recvfrom
    .quad aarch64_syscall_invalid  // shutdown
    .quad aarch64_syscall_invalid  // getsockopt
    .quad aarch64_syscall_invalid  // setsockopt
    .quad aarch64_syscall_invalid  // getpeername
    .quad aarch64_syscall_invalid  // getsockname
    .quad aarch64_syscall_invalid  // socketpair
    .quad aarch64_syscall_invalid  // epoll_create
    .quad aarch64_syscall_invalid  // epoll_ctl
    .quad aarch64_syscall_invalid  // epoll_wait
    .quad aarch64_syscall_invalid  // epoll_pwait
    .quad aarch64_syscall_invalid  // inotify_init
    .quad aarch64_syscall_invalid  // inotify_add_watch
    .quad aarch64_syscall_invalid  // inotify_rm_watch
    .quad aarch64_syscall_invalid  // timer_create
    .quad aarch64_syscall_invalid  // timer_settime
    .quad aarch64_syscall_invalid  // timer_gettime
    .quad aarch64_syscall_invalid  // timer_delete
    .quad aarch64_syscall_invalid  // clock_gettime
    .quad aarch64_syscall_invalid  // clock_settime
    .quad aarch64_syscall_invalid  // clock_getres
    .quad aarch64_syscall_invalid  // clock_nanosleep
    .quad aarch64_syscall_invalid  // getrusage
    .quad aarch64_syscall_invalid  // setpriority
    .quad aarch64_syscall_invalid  // getpriority
    .quad aarch64_syscall_invalid  // sched_setscheduler
    .quad aarch64_syscall_invalid  // sched_getscheduler
    .quad aarch64_syscall_invalid  // sched_setparam
    .quad aarch64_syscall_invalid  // sched_getparam
    .quad aarch64_syscall_invalid  // sched_setaffinity
    .quad aarch64_syscall_invalid  // sched_getaffinity
    .quad aarch64_syscall_invalid  // sched_yield
    .quad aarch64_syscall_invalid  // sched_get_priority_max
    .quad aarch64_syscall_invalid  // sched_get_priority_min
    .quad aarch64_syscall_invalid  // sched_rr_get_interval
    
    // ORION-specific system calls (1000-1099)
    .quad aarch64_syscall_orion_get_capability
    .quad aarch64_syscall_orion_set_capability
    .quad aarch64_syscall_orion_create_namespace
    .quad aarch64_syscall_orion_destroy_namespace
    .quad aarch64_syscall_invalid  // join_namespace
    .quad aarch64_syscall_invalid  // leave_namespace
    .quad aarch64_syscall_orion_create_channel
    .quad aarch64_syscall_orion_destroy_channel
    .quad aarch64_syscall_orion_send_message
    .quad aarch64_syscall_orion_receive_message
    .quad aarch64_syscall_invalid  // create_thread
    .quad aarch64_syscall_invalid  // destroy_thread
    .quad aarch64_syscall_invalid  // thread_yield
    .quad aarch64_syscall_invalid  // thread_sleep
    .quad aarch64_syscall_invalid  // thread_wake
    .quad aarch64_syscall_invalid  // get_thread_info
    .quad aarch64_syscall_invalid  // set_thread_priority
    .quad aarch64_syscall_invalid  // get_thread_priority
    .quad aarch64_syscall_invalid  // create_mutex
    .quad aarch64_syscall_invalid  // destroy_mutex
    .quad aarch64_syscall_invalid  // lock_mutex
    .quad aarch64_syscall_invalid  // unlock_mutex
    .quad aarch64_syscall_invalid  // create_semaphore
    .quad aarch64_syscall_invalid  // destroy_semaphore
    .quad aarch64_syscall_invalid  // wait_semaphore
    .quad aarch64_syscall_invalid  // signal_semaphore
    .quad aarch64_syscall_invalid  // create_event
    .quad aarch64_syscall_invalid  // destroy_event
    .quad aarch64_syscall_invalid  // wait_event
    .quad aarch64_syscall_invalid  // signal_event
    .quad aarch64_syscall_invalid  // create_timer
    .quad aarch64_syscall_invalid  // destroy_timer
    .quad aarch64_syscall_invalid  // start_timer
    .quad aarch64_syscall_invalid  // stop_timer
    .quad aarch64_syscall_invalid  // get_timer_info
    .quad aarch64_syscall_invalid  // create_memory_pool
    .quad aarch64_syscall_invalid  // destroy_memory_pool
    .quad aarch64_syscall_invalid  // allocate_memory
    .quad aarch64_syscall_invalid  // free_memory
    .quad aarch64_syscall_invalid  // get_memory_info
    .quad aarch64_syscall_invalid  // create_device
    .quad aarch64_syscall_invalid  // destroy_device
    .quad aarch64_syscall_invalid  // open_device
    .quad aarch64_syscall_invalid  // close_device
    .quad aarch64_syscall_invalid  // read_device
    .quad aarch64_syscall_invalid  // write_device
    .quad aarch64_syscall_invalid  // ioctl_device
    .quad aarch64_syscall_invalid  // get_device_info
    .quad aarch64_syscall_invalid  // create_file
    .quad aarch64_syscall_invalid  // destroy_file
    .quad aarch64_syscall_invalid  // open_file
    .quad aarch64_syscall_invalid  // close_file
    .quad aarch64_syscall_invalid  // read_file
    .quad aarch64_syscall_invalid  // write_file
    .quad aarch64_syscall_invalid  // seek_file
    .quad aarch64_syscall_invalid  // get_file_info
    .quad aarch64_syscall_invalid  // set_file_info
    .quad aarch64_syscall_invalid  // create_directory
    .quad aarch64_syscall_invalid  // destroy_directory
    .quad aarch64_syscall_invalid  // read_directory
    .quad aarch64_syscall_invalid  // get_directory_info
    .quad aarch64_syscall_invalid  // create_symbolic_link
    .quad aarch64_syscall_invalid  // destroy_symbolic_link
    .quad aarch64_syscall_invalid  // read_symbolic_link
    .quad aarch64_syscall_invalid  // create_hard_link
    .quad aarch64_syscall_invalid  // destroy_hard_link
    .quad aarch64_syscall_invalid  // mount_filesystem
    .quad aarch64_syscall_invalid  // unmount_filesystem
    .quad aarch64_syscall_invalid  // get_filesystem_info
    .quad aarch64_syscall_invalid  // set_filesystem_info
    .quad aarch64_syscall_invalid  // create_network_interface
    .quad aarch64_syscall_invalid  // destroy_network_interface
    .quad aarch64_syscall_invalid  // get_network_interface_info
    .quad aarch64_syscall_invalid  // set_network_interface_info
    .quad aarch64_syscall_invalid  // create_network_socket
    .quad aarch64_syscall_invalid  // destroy_network_socket
    .quad aarch64_syscall_invalid  // bind_network_socket
    .quad aarch64_syscall_invalid  // connect_network_socket
    .quad aarch64_syscall_invalid  // listen_network_socket
    .quad aarch64_syscall_invalid  // accept_network_socket
    .quad aarch64_syscall_invalid  // send_network_message
    .quad aarch64_syscall_invalid  // receive_network_message
    .quad aarch64_syscall_invalid  // get_network_statistics
    .quad aarch64_syscall_invalid  // set_network_statistics
    .quad aarch64_syscall_invalid  // create_process
    .quad aarch64_syscall_invalid  // destroy_process
    .quad aarch64_syscall_invalid  // get_process_info
    .quad aarch64_syscall_invalid  // set_process_info
    .quad aarch64_syscall_invalid  // create_process_namespace
    .quad aarch64_syscall_invalid  // destroy_process_namespace
    .quad aarch64_syscall_invalid  // join_process_namespace
    .quad aarch64_syscall_invalid  // leave_process_namespace
    .quad aarch64_syscall_invalid  // create_process_channel
    .quad aarch64_syscall_invalid  // destroy_process_channel
    .quad aarch64_syscall_invalid  // send_process_message
    .quad aarch64_syscall_invalid  // receive_process_message
    .quad aarch64_syscall_invalid  // get_process_statistics
    .quad aarch64_syscall_invalid  // set_process_statistics
    .quad aarch64_syscall_invalid  // create_process_timer
    .quad aarch64_syscall_invalid  // destroy_process_timer
