/*
 * Orion Operating System - System Limits
 *
 * This header contains all system-wide limits and constants
 * to ensure consistency across the kernel.
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_LIMITS_H
#define ORION_LIMITS_H

#ifdef __cplusplus
extern "C"
{
#endif

    // ========================================
    // PROCESS AND THREAD LIMITS
    // ========================================

#define MAX_PROCESSES 65536          // Maximum number of processes
#define MAX_THREADS 1048576          // Maximum number of threads
#define MAX_THREADS_PER_PROCESS 1024 // Maximum threads per process
#define MAX_PROCESS_NAME_LENGTH 64   // Maximum process name length
#define MAX_THREAD_NAME_LENGTH 32    // Maximum thread name length

    // ========================================
    // MEMORY LIMITS
    // ========================================

#define MAX_HANDLES 256                    // Maximum handles per process
#define MAX_MEMORY_REGIONS 1024            // Maximum memory regions per process
#define MAX_STACK_SIZE (1024 * 1024)       // Maximum stack size (1MB)
#define MAX_HEAP_SIZE (1024 * 1024 * 1024) // Maximum heap size (1GB)

    // ========================================
    // CPU AND SMP LIMITS
    // ========================================

#define MAX_CPUS 64                            // Maximum number of CPUs
#define MAX_CPU_AFFINITY 0xFFFFFFFFFFFFFFFFULL // All CPUs

    // ========================================
    // IPC AND COMMUNICATION LIMITS
    // ========================================

#define IPC_MAX_MESSAGE_SIZE (64 * 1024) // 64KB max per message
#define IPC_MAX_CAPABILITIES 16          // Max capabilities per message
#define MAX_IPC_PORTS 1024               // Maximum IPC ports per process
#define MAX_IPC_CONNECTIONS 256          // Maximum IPC connections per port

    // ========================================
    // FILE SYSTEM LIMITS
    // ========================================

#define MAX_FILES 1024          // Maximum open files per process
#define MAX_PORTS 1024          // Maximum network ports
#define MAX_SERVERS 32          // Maximum system servers
#define MAX_DEVICES 256         // Maximum devices
#define MAX_MOUNTS 64           // Maximum mount points
#define MAX_SOCKETS 1024        // Maximum network sockets
#define MAX_PATH_LENGTH 4096    // Maximum file path length
#define MAX_FILENAME_LENGTH 255 // Maximum filename length

    // ========================================
    // SECURITY AND CAPABILITIES LIMITS
    // ========================================

#define MAX_CAPABILITIES 4294967296ULL // Maximum capability count
#define MAX_SECURITY_CONTEXTS 1024     // Maximum security contexts
#define MAX_AUDIT_ENTRIES 10000        // Maximum audit log entries
#define MAX_SIGNAL_HANDLERS 32         // Maximum signal handlers per process

    // ========================================
    // NETWORK LIMITS
    // ========================================

#define MAX_NETWORK_INTERFACES 16    // Maximum network interfaces
#define MAX_NETWORK_CONNECTIONS 1024 // Maximum network connections
#define MAX_PACKET_SIZE 65536        // Maximum network packet size

    // ========================================
    // DEVICE AND DRIVER LIMITS
    // ========================================

#define MAX_DEVICE_DRIVERS 256 // Maximum device drivers
#define MAX_DEVICE_QUEUES 64   // Maximum device queues
#define MAX_IRQ_HANDLERS 256   // Maximum IRQ handlers

    // ========================================
    // TIMER AND SCHEDULER LIMITS
    // ========================================

#define MAX_TIMERS 1024                                     // Maximum timers per process
#define MAX_SCHEDULER_PRIORITIES 40                         // Priority levels (-20 to +19)
#define MAX_SLEEP_TIME (365 * 24 * 60 * 60 * 1000000000ULL) // 1 year in nanoseconds

    // ========================================
    // DEBUG AND LOGGING LIMITS
    // ========================================

#define MAX_LOG_ENTRIES 100000       // Maximum log entries
#define MAX_DEBUG_LEVELS 8           // Maximum debug levels
#define MAX_ERROR_MESSAGE_LENGTH 512 // Maximum error message length

    // ========================================
    // BOOT AND INITIALIZATION LIMITS
    // ========================================

#define MAX_BOOT_MODULES 64  // Maximum boot modules
#define MAX_ACPI_TABLES 32   // Maximum ACPI tables
#define MAX_EFI_HANDLES 1024 // Maximum EFI handles

    // ========================================
    // PCI AND HARDWARE LIMITS
    // ========================================

#define PCI_MAX_DEVICES 256 // Maximum PCI devices
#define PCI_MAX_FUNCTIONS 8 // Maximum PCI functions per device
#define PCI_MAX_BARS 6      // Maximum PCI BARs per device
#define PCI_MAX_LAT 0x3F    // Maximum PCI latency

    // ========================================
    // VIRTUALIZATION LIMITS
    // ========================================

#define MAX_VM_INSTANCES 64                          // Maximum virtual machine instances
#define MAX_VM_MEMORY (1024ULL * 1024 * 1024 * 1024) // 1TB per VM
#define MAX_VM_CPUS 32                               // Maximum CPUs per VM

#ifdef __cplusplus
}
#endif

#endif // ORION_LIMITS_H
