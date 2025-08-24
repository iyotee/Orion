/*
 * Orion Operating System - System Calls Header
 *
 * System call interface declarations.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_SYSCALLS_H
#define ORION_SYSCALLS_H

#include <orion/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

// System call numbers
#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_OPEN 2
#define SYS_CLOSE 3
#define SYS_EXEC 4
#define SYS_EXIT 5
#define SYS_FORK 6
#define SYS_GETPID 7

    // System call functions
    void syscall_init(void);
    void syscall_handler(uint64_t syscall_num, uint64_t arg1, uint64_t arg2, uint64_t arg3);

#ifdef __cplusplus
}
#endif

#endif // ORION_SYSCALLS_H
