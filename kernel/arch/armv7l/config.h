/*
 * ORION OS - ARMv7l Architecture Configuration
 *
 * Configuration for ARMv7l architecture (32-bit ARM)
 * Supports all Raspberry Pi models: Pi 1, Pi 2, Pi Zero, Pi Zero W
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_ARMV7L_CONFIG_H
#define ORION_ARMV7L_CONFIG_H

// ============================================================================
// ARCHITECTURE IDENTIFICATION
// ============================================================================

#define ORION_ARCH_ARMV7L
#define ORION_ARCH_NAME "ARMv7l"
#define ORION_ARCH_BITS 32
#define ORION_ARCH_ENDIANNESS ORION_LITTLE_ENDIAN

// ============================================================================
// PROCESSOR SUPPORT
// ============================================================================

// ARMv6 (Raspberry Pi 1, Pi Zero)
#define ARM_SUPPORT_ARMv6
#define ARM_SUPPORT_ARMv6K
#define ARM_SUPPORT_ARMv6Z

// ARMv7 (Raspberry Pi 2)
#define ARM_SUPPORT_ARMv7
#define ARM_SUPPORT_ARMv7A
#define ARM_SUPPORT_ARMv7R
#define ARM_SUPPORT_ARMv7M

// ARM Cortex-A Series
#define ARM_SUPPORT_CORTEX_A5
#define ARM_SUPPORT_CORTEX_A7
#define ARM_SUPPORT_CORTEX_A8
#define ARM_SUPPORT_CORTEX_A9
#define ARM_SUPPORT_CORTEX_A15
#define ARM_SUPPORT_CORTEX_A17

// ARM Cortex-M Series
#define ARM_SUPPORT_CORTEX_M0
#define ARM_SUPPORT_CORTEX_M1
#define ARM_SUPPORT_CORTEX_M3
#define ARM_SUPPORT_CORTEX_M4
#define ARM_SUPPORT_CORTEX_M7

// ============================================================================
// FEATURE SUPPORT
// ============================================================================

// ARM Extensions
#define ARM_SUPPORT_THUMB
#define ARM_SUPPORT_THUMB2
#define ARM_SUPPORT_JAZELLE
#define ARM_SUPPORT_ARM_TRUSTZONE
#define ARM_SUPPORT_ARM_NEON
#define ARM_SUPPORT_ARM_VFP
#define ARM_SUPPORT_ARM_VFPv3
#define ARM_SUPPORT_ARM_VFPv4
#define ARM_SUPPORT_ARM_NEON_FP16

// Memory Management
#define ARM_SUPPORT_MMU
#define ARM_SUPPORT_MPU
#define ARM_SUPPORT_TLB
#define ARM_SUPPORT_CACHE
#define ARM_SUPPORT_L1_CACHE
#define ARM_SUPPORT_L2_CACHE
#define ARM_SUPPORT_L3_CACHE

// Interrupts and Exceptions
#define ARM_SUPPORT_IRQ
#define ARM_SUPPORT_FIQ
#define ARM_SUPPORT_SWI
#define ARM_SUPPORT_ABORT
#define ARM_SUPPORT_UNDEF
#define ARM_SUPPORT_RESET
#define ARM_SUPPORT_VECTOR_TABLE

// ============================================================================
// RASPBERRY PI SPECIFIC SUPPORT
// ============================================================================

// Raspberry Pi Models
#define RASPBERRY_PI_1
#define RASPBERRY_PI_2
#define RASPBERRY_PI_ZERO
#define RASPBERRY_PI_ZERO_W

// Broadcom SoC Support
#define BCM_SUPPORT_BCM2835 // Pi 1, Pi Zero
#define BCM_SUPPORT_BCM2836 // Pi 2
#define BCM_SUPPORT_BCM2837 // Pi 3 (64-bit, but 32-bit mode supported)

// Hardware Peripherals
#define RASPBERRY_PI_GPIO
#define RASPBERRY_PI_UART
#define RASPBERRY_PI_SPI
#define RASPBERRY_PI_I2C
#define RASPBERRY_PI_PWM
#define RASPBERRY_PI_ADC
#define RASPBERRY_PI_SDIO
#define RASPBERRY_PI_USB
#define RASPBERRY_PI_ETHERNET
#define RASPBERRY_PI_WIFI
#define RASPBERRY_PI_BLUETOOTH

// ============================================================================
// MEMORY CONFIGURATION
// ============================================================================

// Memory Layout
#define ARM_MEMORY_BASE 0x00000000
#define ARM_MEMORY_SIZE (512 * 1024 * 1024) // 512MB typical for Pi
#define ARM_STACK_SIZE (64 * 1024)          // 64KB stack
#define ARM_HEAP_SIZE (128 * 1024 * 1024)   // 128MB heap

// Page Sizes
#define ARM_PAGE_SIZE_4KB
#define ARM_PAGE_SIZE_64KB
#define ARM_PAGE_SIZE_1MB
#define ARM_PAGE_SIZE_16MB

// Cache Configuration
#define ARM_CACHE_LINE_SIZE 32
#define ARM_L1_CACHE_SIZE (16 * 1024)  // 16KB L1
#define ARM_L2_CACHE_SIZE (512 * 1024) // 512KB L2

// ============================================================================
// TIMING AND CLOCKING
// ============================================================================

// Clock Frequencies
#define ARM_CPU_FREQ_MIN 600000000      // 600MHz
#define ARM_CPU_FREQ_MAX 1400000000     // 1.4GHz
#define ARM_CPU_FREQ_DEFAULT 1000000000 // 1GHz

// Timer Configuration
#define ARM_TIMER_FREQ 1000000 // 1MHz timer
#define ARM_SYSTICK_FREQ 1000  // 1kHz system tick

// ============================================================================
// SECURITY FEATURES
// ============================================================================

// ARM Security Extensions
#define ARM_SUPPORT_TRUSTZONE
#define ARM_SUPPORT_SECURE_MONITOR
#define ARM_SUPPORT_CRYPTO_EXTENSIONS
#define ARM_SUPPORT_RANDOM_NUMBER_GENERATOR

// Memory Protection
#define ARM_SUPPORT_MPU
#define ARM_SUPPORT_PXN
#define ARM_SUPPORT_UXN
#define ARM_SUPPORT_SCTLR

// ============================================================================
// DEBUG AND DEVELOPMENT
// ============================================================================

// Debug Support
#define ARM_SUPPORT_DEBUG
#define ARM_SUPPORT_BREAKPOINTS
#define ARM_SUPPORT_WATCHPOINTS
#define ARM_SUPPORT_ETM
#define ARM_SUPPORT_CTI

// Performance Monitoring
#define ARM_SUPPORT_PMU
#define ARM_SUPPORT_CYCLES_COUNTER
#define ARM_SUPPORT_INSTRUCTIONS_COUNTER
#define ARM_SUPPORT_CACHE_MISS_COUNTER

// ============================================================================
// COMPILER AND BUILD CONFIGURATION
// ============================================================================

// Compiler Flags
#define ARM_COMPILER_GCC
#define ARM_COMPILER_CLANG
#define ARM_COMPILER_ARMCC

// Optimization Flags
#define ARM_OPTIMIZATION_O2
#define ARM_OPTIMIZATION_OS
#define ARM_OPTIMIZATION_FAST_MATH

// Assembly Support
#define ARM_ASSEMBLY_GAS
#define ARM_ASSEMBLY_ARMASM
#define ARM_ASSEMBLY_LLVM

// ============================================================================
// SYSTEM CONFIGURATION
// ============================================================================

// Boot Configuration
#define ARM_BOOT_UART
#define ARM_BOOT_GPIO
#define ARM_BOOT_LED
#define ARM_BOOT_DTB

// Device Tree Support
#define ARM_SUPPORT_DEVICE_TREE
#define ARM_DEVICE_TREE_BLOB
#define ARM_DEVICE_TREE_SOURCE

// Bootloader Support
#define ARM_BOOTLOADER_U_BOOT
#define ARM_BOOTLOADER_RASPBERRY_PI
#define ARM_BOOTLOADER_GRUB

// ============================================================================
// NETWORKING SUPPORT
// ============================================================================

// Network Interfaces
#define ARM_NETWORK_ETHERNET
#define ARM_NETWORK_WIFI
#define ARM_NETWORK_BLUETOOTH
#define ARM_NETWORK_USB_ETHERNET

// Network Protocols
#define ARM_NETWORK_TCP_IP
#define ARM_NETWORK_UDP
#define ARM_NETWORK_ICMP
#define ARM_NETWORK_DHCP

// ============================================================================
// STORAGE SUPPORT
// ============================================================================

// Storage Interfaces
#define ARM_STORAGE_SD_CARD
#define ARM_STORAGE_USB_STORAGE
#define ARM_STORAGE_NETWORK_STORAGE
#define ARM_STORAGE_FLASH

// File Systems
#define ARM_FILESYSTEM_FAT32
#define ARM_FILESYSTEM_EXT4
#define ARM_FILESYSTEM_NTFS
#define ARM_FILESYSTEM_BTRFS

// ============================================================================
// MULTIMEDIA SUPPORT
// ============================================================================

// Video Support
#define ARM_VIDEO_OPENGL_ES
#define ARM_VIDEO_VULKAN
#define ARM_VIDEO_HARDWARE_ACCELERATION
#define ARM_VIDEO_H264_DECODE
#define ARM_VIDEO_H265_DECODE

// Audio Support
#define ARM_AUDIO_ALSA
#define ARM_AUDIO_PULSEAUDIO
#define ARM_AUDIO_HARDWARE_MIXER
#define ARM_AUDIO_I2S
#define ARM_AUDIO_SPDIF

// ============================================================================
// POWER MANAGEMENT
// ============================================================================

// Power States
#define ARM_POWER_STATE_ACTIVE
#define ARM_POWER_STATE_IDLE
#define ARM_POWER_STATE_SLEEP
#define ARM_POWER_STATE_HIBERNATE

// Power Management Features
#define ARM_POWER_FREQUENCY_SCALING
#define ARM_POWER_VOLTAGE_SCALING
#define ARM_POWER_CORE_OFFLOADING
#define ARM_POWER_DYNAMIC_POWER_GATING

// ============================================================================
// VIRTUALIZATION SUPPORT
// ============================================================================

// Hypervisor Support
#define ARM_VIRTUALIZATION_HYPERVISOR
#define ARM_VIRTUALIZATION_KVM
#define ARM_VIRTUALIZATION_XEN
#define ARM_VIRTUALIZATION_VIRTUALBOX

// Virtual Machine Features
#define ARM_VIRTUALIZATION_VM_CREATION
#define ARM_VIRTUALIZATION_VM_CONTROL
#define ARM_VIRTUALIZATION_VM_MIGRATION
#define ARM_VIRTUALIZATION_VM_SNAPSHOTS

// ============================================================================
// REAL-TIME SUPPORT
// ============================================================================

// Real-Time Features
#define ARM_REALTIME_PRIORITY_SCHEDULING
#define ARM_REALTIME_TIMERS
#define ARM_REALTIME_INTERRUPTS
#define ARM_REALTIME_MEMORY_ALLOCATION

// Real-Time Operating System Support
#define ARM_REALTIME_FREERTOS
#define ARM_REALTIME_VXWORKS
#define ARM_REALTIME_QNX
#define ARM_REALTIME_RTEMS

// ============================================================================
// DEVELOPMENT TOOLS
// ============================================================================

// Development Environment
#define ARM_DEVELOPMENT_ECLIPSE
#define ARM_DEVELOPMENT_VISUAL_STUDIO
#define ARM_DEVELOPMENT_VS_CODE
#define ARM_DEVELOPMENT_ARM_DS

// Debugging Tools
#define ARM_DEBUG_GDB
#define ARM_DEBUG_LLDB
#define ARM_DEBUG_OPENOCD
#define ARM_DEBUG_JTAG

// ============================================================================
// TESTING AND VALIDATION
// ============================================================================

// Testing Framework
#define ARM_TESTING_UNITY
#define ARM_TESTING_CMOCKA
#define ARM_TESTING_GOOGLE_TEST
#define ARM_TESTING_CATCH2

// Validation Tools
#define ARM_VALIDATION_ARM_ARCHITECTURE_VALIDATOR
#define ARM_VALIDATION_ARM_COMPLIANCE_SUITE
#define ARM_VALIDATION_ARM_ACLE

// ============================================================================
// PERFORMANCE OPTIMIZATION
// ============================================================================

// Compiler Optimizations
#define ARM_OPTIMIZATION_INLINE_FUNCTIONS
#define ARM_OPTIMIZATION_LOOP_UNROLLING
#define ARM_OPTIMIZATION_VECTORIZATION
#define ARM_OPTIMIZATION_BRANCH_PREDICTION

// Runtime Optimizations
#define ARM_OPTIMIZATION_CACHE_ALIGNMENT
#define ARM_OPTIMIZATION_MEMORY_POOLS
#define ARM_OPTIMIZATION_LOCK_FREE_DATA_STRUCTURES
#define ARM_OPTIMIZATION_ATOMIC_OPERATIONS

// ============================================================================
// COMPATIBILITY AND STANDARDS
// ============================================================================

// Standards Compliance
#define ARM_COMPLIANCE_ARM_ARCHITECTURE_REFERENCE_MANUAL
#define ARM_COMPLIANCE_ARM_ARCHITECTURE_PROCEDURE_CALL_STANDARD
#define ARM_COMPLIANCE_ARM_ARCHITECTURE_SYSTEM_CALLS
#define ARM_COMPLIANCE_ARM_ARCHITECTURE_MEMORY_MODEL

// ABI Compatibility
#define ARM_ABI_EABI
#define ARM_ABI_GNUEABI
#define ARM_ABI_ARMCC
#define ARM_ABI_LLVM

// ============================================================================
// FUTURE EXTENSIONS
// ============================================================================

// ARMv8 32-bit Mode Support
#define ARM_FUTURE_ARMv8_AARCH32
#define ARM_FUTURE_ARMv8_1_AARCH32
#define ARM_FUTURE_ARMv8_2_AARCH32

// Advanced Features
#define ARM_FUTURE_ARM_NEON_FP16
#define ARM_FUTURE_ARM_CRYPTO_EXTENSIONS
#define ARM_FUTURE_ARM_RANDOM_NUMBER_GENERATOR
#define ARM_FUTURE_ARM_CRC_EXTENSIONS

#endif // ORION_ARMV7L_CONFIG_H
