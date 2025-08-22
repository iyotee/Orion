/*
 * Orion Operating System - System Constants
 *
 * Complete system constants and error codes.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_CONSTANTS_H
#define ORION_CONSTANTS_H

// ========================================
// SUCCESS AND ERROR CODES
// ========================================

#define OR_OK 0
#define OR_ERROR -1
#define OR_EINVAL -2
#define OR_ENOMEM -3
#define OR_ENOENT -4
#define OR_EEXIST -5
#define OR_EBUSY -6
#define OR_EAGAIN -7
#define OR_ETIMEDOUT -8
#define OR_EPERM -9
#define OR_EACCES -10
#define OR_ENOSPC -11
#define OR_EOVERFLOW -12
#define OR_ENODATA -13
#define OR_ESERVER -14

// ========================================
// PROCESS STATES
// ========================================

#define PROCESS_STATE_NEW 0
#define PROCESS_STATE_READY 1
#define PROCESS_STATE_RUNNING 2
#define PROCESS_STATE_BLOCKED 3
#define PROCESS_STATE_TERMINATED 4
#define PROCESS_STATE_ZOMBIE 5
#define PROCESS_STATE_SLEEPING 6
#define PROCESS_STATE_STOPPED 7

// ========================================
// THREAD STATES
// ========================================

#define THREAD_STATE_NEW 0
#define THREAD_STATE_READY 1
#define THREAD_STATE_RUNNING 2
#define THREAD_STATE_BLOCKED 3
#define THREAD_STATE_SLEEPING 4
#define THREAD_STATE_TERMINATED 5
#define THREAD_STATE_STOPPED 6

// ========================================
// PROCESS FLAGS
// ========================================

#define PROCESS_FLAG_SYSTEM 0x00000001
#define PROCESS_FLAG_USER 0x00000002
#define PROCESS_FLAG_PRIVILEGED 0x00000004
#define PROCESS_FLAG_SANDBOXED 0x00000008
#define PROCESS_FLAG_DEBUGGABLE 0x00000010
#define PROCESS_FLAG_SIGNALABLE 0x00000020

// ========================================
// PROCESS PRIORITIES
// ========================================

#define PROCESS_PRIORITY_LOW 0
#define PROCESS_PRIORITY_NORMAL 1
#define PROCESS_PRIORITY_HIGH 2
#define PROCESS_PRIORITY_CRITICAL 3
#define PROCESS_PRIORITY_REALTIME 4

// ========================================
// THREAD PRIORITIES (NICE VALUES)
// ========================================

#define THREAD_PRIORITY_MIN -20
#define THREAD_PRIORITY_MAX 19
#define THREAD_PRIORITY_DEFAULT 0

// ========================================
// MEMORY FLAGS
// ========================================

#define VM_FLAG_NONE 0x00000000
#define VM_FLAG_READ 0x00000001
#define VM_FLAG_WRITE 0x00000002
#define VM_FLAG_EXEC 0x00000004
#define VM_FLAG_USER 0x00000008
#define VM_FLAG_KERNEL 0x00000010
#define VM_FLAG_COW 0x00000020
#define VM_FLAG_SHARED 0x00000040
#define VM_FLAG_NOCACHE 0x00000080
#define VM_FLAG_WRITETHROUGH 0x00000100

// ========================================
// PAGE FLAGS
// ========================================

#define PAGE_PRESENT 0x00000001
#define PAGE_WRITE 0x00000002
#define PAGE_USER 0x00000004
#define PAGE_WRITETHROUGH 0x00000008
#define PAGE_NOCACHE 0x00000010
#define PAGE_ACCESSED 0x00000020
#define PAGE_DIRTY 0x00000040
#define PAGE_PAT 0x00000080
#define PAGE_GLOBAL 0x00000100
#define PAGE_NX 0x8000000000000000ULL

// ========================================
// INTERRUPT FLAGS
// ========================================

#define INT_FLAG_NONE 0x00000000
#define INT_FLAG_PRESENT 0x00000001
#define INT_FLAG_RING0 0x00000000
#define INT_FLAG_RING1 0x00000020
#define INT_FLAG_RING2 0x00000040
#define INT_FLAG_RING3 0x00000060
#define INT_FLAG_TRAP 0x00000000
#define INT_FLAG_INTERRUPT 0x00000080
#define INT_FLAG_32BIT 0x00000000
#define INT_FLAG_16BIT 0x00000400

// ========================================
// SCHEDULER FLAGS
// ========================================

#define SCHED_FLAG_NONE 0x00000000
#define SCHED_FLAG_PREEMPTIBLE 0x00000001
#define SCHED_FLAG_RT 0x00000002
#define SCHED_FLAG_IDLE 0x00000004
#define SCHED_FLAG_BATCH 0x00000008

// ========================================
// IPC FLAGS
// ========================================

#define IPC_FLAG_NONE 0x00000000
#define IPC_FLAG_NONBLOCK 0x00000001
#define IPC_FLAG_TIMEOUT 0x00000002
#define IPC_FLAG_PRIORITY 0x00000004
#define IPC_FLAG_RELIABLE 0x00000008

// ========================================
// CAPABILITY TYPES
// ========================================

#define CAP_TYPE_NONE 0
#define CAP_TYPE_READ 1
#define CAP_TYPE_WRITE 2
#define CAP_TYPE_EXEC 3
#define CAP_TYPE_CREATE 4
#define CAP_TYPE_DELETE 5
#define CAP_TYPE_GRANT 6
#define CAP_TYPE_REVOKE 7
#define CAP_TYPE_IPC_PORT 8
#define CAP_TYPE_IPC_SEND 9
#define CAP_TYPE_IPC_RECV 10
#define CAP_TYPE_IPC_CREATE 11
#define CAP_TYPE_IPC_DESTROY 12

// ========================================
// SECURITY FLAGS
// ========================================

#define SEC_FLAG_NONE 0x00000000
#define SEC_FLAG_SMEP 0x00000001
#define SEC_FLAG_SMAP 0x00000002
#define SEC_FLAG_UMIP 0x00000004
#define SEC_FLAG_NX 0x00000008
#define SEC_FLAG_ASLR 0x00000010
#define SEC_FLAG_CFI 0x00000020

// ========================================
// TIMER FLAGS
// ========================================

#define TIMER_FLAG_NONE 0x00000000
#define TIMER_FLAG_PERIODIC 0x00000001
#define TIMER_FLAG_ONE_SHOT 0x00000002
#define TIMER_FLAG_ABSOLUTE 0x00000004
#define TIMER_FLAG_RELATIVE 0x00000008

// ========================================
// DEVICE FLAGS
// ========================================

#define DEV_FLAG_NONE 0x00000000
#define DEV_FLAG_READABLE 0x00000001
#define DEV_FLAG_WRITABLE 0x00000002
#define DEV_FLAG_SEEKABLE 0x00000004
#define DEV_FLAG_BLOCK 0x00000008
#define DEV_FLAG_CHAR 0x00000010
#define DEV_FLAG_NETWORK 0x00000020
#define DEV_FLAG_STORAGE 0x00000040

// ========================================
// FILE SYSTEM FLAGS
// ========================================

#define FS_FLAG_NONE 0x00000000
#define FS_FLAG_READ_ONLY 0x00000001
#define FS_FLAG_HIDDEN 0x00000002
#define FS_FLAG_SYSTEM 0x00000004
#define FS_FLAG_ARCHIVE 0x00000008
#define FS_FLAG_COMPRESSED 0x00000010
#define FS_FLAG_ENCRYPTED 0x00000020

// ========================================
// NETWORK FLAGS
// ========================================

#define NET_FLAG_NONE 0x00000000
#define NET_FLAG_BROADCAST 0x00000001
#define NET_FLAG_MULTICAST 0x00000002
#define NET_FLAG_LOOPBACK 0x00000004
#define NET_FLAG_POINTTOPOINT 0x00000008
#define NET_FLAG_UP 0x00000010
#define NET_FLAG_RUNNING 0x00000020

// ========================================
// SYSTEM LIMITS
// ========================================

// MAX_PROCESSES and MAX_THREADS are defined in kernel.h - no duplication
#include <orion/limits.h>

// ========================================
// MEMORY LIMITS
// ========================================

#define PAGE_SIZE 4096
#define PAGE_SHIFT 12
#define PAGE_MASK 0xFFFFFFFFFFFFF000ULL

#define KERNEL_STACK_SIZE (16 * 1024)  // 16KB
#define USER_STACK_SIZE (64 * 1024)    // 64KB
#define SERVER_STACK_SIZE (128 * 1024) // 128KB

#define KERNEL_HEAP_SIZE (1024 * 1024)       // 1MB
#define USER_HEAP_INITIAL_SIZE (1024 * 1024) // 1MB

// ========================================
// TIME CONSTANTS
// ========================================

#define NS_PER_SEC 1000000000ULL
#define NS_PER_MS 1000000ULL
#define NS_PER_US 1000ULL

#define MS_PER_SEC 1000ULL
#define US_PER_SEC 1000000ULL

// ========================================
// ARCHITECTURE CONSTANTS
// ========================================

#define CPUID_VENDOR_AMD "AuthenticAMD"
#define CPUID_VENDOR_INTEL "GenuineIntel"
#define CPUID_VENDOR_VIA "CentaurHauls"

#define X86_CR0_PE 0x00000001
#define X86_CR0_MP 0x00000002
#define X86_CR0_EM 0x00000004
#define X86_CR0_TS 0x00000008
#define X86_CR0_ET 0x00000010
#define X86_CR0_NE 0x00000020
#define X86_CR0_WP 0x00010000
#define X86_CR0_AM 0x00040000
#define X86_CR0_NW 0x20000000
#define X86_CR0_CD 0x40000000
#define X86_CR0_PG 0x80000000

#define X86_CR4_VME 0x00000001
#define X86_CR4_PVI 0x00000002
#define X86_CR4_TSD 0x00000004
#define X86_CR4_DE 0x00000008
#define X86_CR4_PSE 0x00000010
#define X86_CR4_PAE 0x00000020
#define X86_CR4_MCE 0x00000040
#define X86_CR4_PGE 0x00000080
#define X86_CR4_PCE 0x00000100
#define X86_CR4_OSFXSR 0x00000200
#define X86_CR4_OSXMMEXCPT 0x00000400
#define X86_CR4_LA57 0x00000800
#define X86_CR4_VMXE 0x00002000
#define X86_CR4_SMXE 0x00004000
#define X86_CR4_FSGSBASE 0x00010000
#define X86_CR4_PCIDE 0x00020000
#define X86_CR4_OSXSAVE 0x00040000
#define X86_CR4_SMEP 0x00100000
#define X86_CR4_SMAP 0x00200000
#define X86_CR4_PKE 0x00400000

// ========================================
// APIC CONSTANTS
// ========================================

#define APIC_ID 0x20
#define APIC_VER 0x30
#define APIC_TPR 0x80
#define APIC_APR 0x90
#define APIC_PPR 0xA0
#define APIC_EOI 0xB0
#define APIC_RRR 0xC0
#define APIC_LDR 0xD0
#define APIC_DFR 0xE0
#define APIC_SIVR 0xF0
#define APIC_ISR0 0x100
#define APIC_ISR1 0x110
#define APIC_ISR2 0x120
#define APIC_ISR3 0x130
#define APIC_ISR4 0x140
#define APIC_ISR5 0x150
#define APIC_ISR6 0x160
#define APIC_ISR7 0x170
#define APIC_TMR0 0x180
#define APIC_TMR1 0x190
#define APIC_TMR2 0x1A0
#define APIC_TMR3 0x1B0
#define APIC_TMR4 0x1C0
#define APIC_TMR5 0x1D0
#define APIC_TMR6 0x1E0
#define APIC_TMR7 0x1F0
#define APIC_IRR0 0x200
#define APIC_IRR1 0x210
#define APIC_IRR2 0x220
#define APIC_IRR3 0x230
#define APIC_IRR4 0x240
#define APIC_IRR5 0x250
#define APIC_IRR6 0x260
#define APIC_IRR7 0x270
#define APIC_ESR 0x280
#define APIC_LVT_CMCI 0x2F0
#define APIC_ICR 0x300
#define APIC_LVT_TIMER 0x320
#define APIC_LVT_THERMAL 0x330
#define APIC_LVT_PERF 0x340
#define APIC_LVT_LINT0 0x350
#define APIC_LVT_LINT1 0x360
#define APIC_LVT_ERROR 0x370
#define APIC_TIMER_ICR 0x380
#define APIC_TIMER_CCR 0x390
#define APIC_TIMER_DCR 0x3E0

// ========================================
// IOAPIC CONSTANTS
// ========================================

#define IOAPIC_ID 0x00
#define IOAPIC_VER 0x01
#define IOAPIC_ARB 0x02
#define IOAPIC_REDIR_TBL 0x10

// ========================================
// PCI CONSTANTS
// ========================================

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define PCI_VENDOR_ID 0x00
#define PCI_DEVICE_ID 0x02
#define PCI_COMMAND 0x04
#define PCI_STATUS 0x06
#define PCI_REVISION_ID 0x08
#define PCI_CLASS_CODE 0x09
#define PCI_SUBCLASS 0x0A
#define PCI_PROG_IF 0x0B
#define PCI_CACHE_LINE_SIZE 0x0C
#define PCI_LATENCY_TIMER 0x0D
#define PCI_HEADER_TYPE 0x0E
#define PCI_BIST 0x0F
#define PCI_BAR0 0x10
#define PCI_BAR1 0x14
#define PCI_BAR2 0x18
#define PCI_BAR3 0x1C
#define PCI_BAR4 0x20
#define PCI_BAR5 0x24
#define PCI_CARDBUS_CIS 0x28
#define PCI_SUBSYS_VENDOR_ID 0x2C
#define PCI_SUBSYS_ID 0x2E
#define PCI_EXPANSION_ROM 0x30
#define PCI_CAP_PTR 0x34
#define PCI_RESERVED1 0x35
#define PCI_RESERVED2 0x36
#define PCI_RESERVED3 0x37
#define PCI_INTERRUPT_LINE 0x3C
#define PCI_INTERRUPT_PIN 0x3D
#define PCI_MIN_GNT 0x3E
#define PCI_MAX_LAT 0x3F

// ========================================
// USB CONSTANTS
// ========================================

#define USB_CLASS_HID 0x03
#define USB_CLASS_MASS_STORAGE 0x08
#define USB_CLASS_HUB 0x09
#define USB_CLASS_VENDOR 0xFF

#define USB_SUBCLASS_HID_BOOT 0x01
#define USB_SUBCLASS_MASS_STORAGE_SCSI 0x06

#define USB_PROTOCOL_HID_KEYBOARD 0x01
#define USB_PROTOCOL_HID_MOUSE 0x02
#define USB_PROTOCOL_MASS_STORAGE_BOT 0x50

// ========================================
// VIRTIO CONSTANTS
// ========================================

#define VIRTIO_MMIO_MAGIC_VALUE 0x000
#define VIRTIO_MMIO_VERSION 0x004
#define VIRTIO_MMIO_DEVICE_ID 0x008
#define VIRTIO_MMIO_VENDOR_ID 0x00C
#define VIRTIO_MMIO_DEVICE_FEATURES 0x010
#define VIRTIO_MMIO_DRIVER_FEATURES 0x020
#define VIRTIO_MMIO_QUEUE_SEL 0x030
#define VIRTIO_MMIO_QUEUE_NUM_MAX 0x034
#define VIRTIO_MMIO_QUEUE_NUM 0x038
#define VIRTIO_MMIO_QUEUE_ALIGN 0x03C
#define VIRTIO_MMIO_QUEUE_PFN 0x040
#define VIRTIO_MMIO_QUEUE_NOTIFY 0x050
#define VIRTIO_MMIO_INTERRUPT_STATUS 0x060
#define VIRTIO_MMIO_INTERRUPT_ACK 0x064
#define VIRTIO_MMIO_STATUS 0x070

#define VIRTIO_STATUS_ACKNOWLEDGE 1
#define VIRTIO_STATUS_DRIVER 2
#define VIRTIO_STATUS_DRIVER_OK 4
#define VIRTIO_STATUS_FEATURES_OK 8
#define VIRTIO_STATUS_DEVICE_NEEDS_RESET 64
#define VIRTIO_STATUS_FAILED 128

// ========================================
// ACPI CONSTANTS
// ========================================

#define ACPI_SIGNATURE_RSDP "RSD PTR "
#define ACPI_SIGNATURE_RSDT "RSDT"
#define ACPI_SIGNATURE_XSDT "XSDT"
#define ACPI_SIGNATURE_FADT "FACP"
#define ACPI_SIGNATURE_DSDT "DSDT"
#define ACPI_SIGNATURE_SSDT "SSDT"
#define ACPI_SIGNATURE_MADT "APIC"
#define ACPI_SIGNATURE_MCFG "MCFG"
#define ACPI_SIGNATURE_HPET "HPET"
#define ACPI_SIGNATURE_ECDT "ECDT"
#define ACPI_SIGNATURE_SRAT "SRAT"
#define ACPI_SIGNATURE_SLIT "SLIT"

// ========================================
// EFI CONSTANTS
// ========================================

#define EFI_SUCCESS 0
#define EFI_LOAD_ERROR 0x8000000000000001ULL
#define EFI_INVALID_PARAMETER 0x8000000000000002ULL
#define EFI_UNSUPPORTED 0x8000000000000003ULL
#define EFI_BAD_BUFFER_SIZE 0x8000000000000004ULL
#define EFI_BUFFER_TOO_SMALL 0x8000000000000005ULL
#define EFI_NOT_READY 0x8000000000000006ULL
#define EFI_DEVICE_ERROR 0x8000000000000007ULL
#define EFI_WRITE_PROTECTED 0x8000000000000008ULL
#define EFI_OUT_OF_RESOURCES 0x8000000000000009ULL
#define EFI_VOLUME_CORRUPTED 0x800000000000000AULL
#define EFI_VOLUME_FULL 0x800000000000000BULL
#define EFI_NO_MEDIA 0x800000000000000CULL
#define EFI_MEDIA_CHANGED 0x800000000000000DULL
#define EFI_NOT_FOUND 0x800000000000000EULL
#define EFI_ACCESS_DENIED 0x800000000000000FULL
#define EFI_NO_RESPONSE 0x8000000000000010ULL
#define EFI_NO_MAPPING 0x8000000000000011ULL
#define EFI_TIMEOUT 0x8000000000000012ULL
#define EFI_NOT_STARTED 0x8000000000000013ULL
#define EFI_ALREADY_STARTED 0x8000000000000014ULL
#define EFI_ABORTED 0x8000000000000015ULL
#define EFI_ICMP_ERROR 0x8000000000000016ULL
#define EFI_TFTP_ERROR 0x8000000000000017ULL
#define EFI_PROTOCOL_ERROR 0x8000000000000018ULL
#define EFI_INCOMPATIBLE_VERSION 0x8000000000000019ULL
#define EFI_SECURITY_VIOLATION 0x800000000000001AULL
#define EFI_CRC_ERROR 0x800000000000001BULL
#define EFI_END_OF_MEDIA 0x800000000000001CULL
#define EFI_END_OF_FILE 0x800000000000001DULL
#define EFI_INVALID_LANGUAGE 0x800000000000001EULL
#define EFI_COMPROMISED_DATA 0x800000000000001FULL
#define EFI_IP_ADDRESS_CONFLICT 0x8000000000000020ULL
#define EFI_HTTP_ERROR 0x8000000000000021ULL

#endif // ORION_CONSTANTS_H
