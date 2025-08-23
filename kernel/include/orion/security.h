/*
 * Orion Operating System - Security System Header
 *
 * Security and access control system declarations.
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

#ifdef __cplusplus
extern "C"
{
#endif

    // Security functions
    bool security_check_syscall_allowed(uint64_t syscall, uint64_t pid);
    bool security_check_memory_access(uint64_t vaddr, uint64_t size, uint64_t flags);
    bool security_check_file_access(const char *path, uint64_t mode);

#ifdef __cplusplus
}
#endif

#endif // ORION_SECURITY_H
