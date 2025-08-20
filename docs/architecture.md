# Orion OS Architecture

## Overview

Orion OS is a modern hybrid micro-kernel operating system designed for performance, security, and maintainability. It combines the best aspects of monolithic and microkernel architectures.

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    USER SPACE                               │
├─────────────────────────────────────────────────────────────┤
│  Applications  │  Libraries  │  Shell  │  Rust Drivers     │
├─────────────────────────────────────────────────────────────┤
│                    KERNEL SPACE                             │
├─────────────────────────────────────────────────────────────┤
│  VFS  │  Network  │  Security  │  IPC  │  Scheduler  │ MM  │
├─────────────────────────────────────────────────────────────┤
│              Hybrid Micro-Kernel Core (C11)                │
├─────────────────────────────────────────────────────────────┤
│                    Hardware Layer                           │
└─────────────────────────────────────────────────────────────┘
```

## Core Components

### Kernel Core (C11 Freestanding)
- **Memory Management**: PMM, VMM, Slab allocator, Heap allocator
- **Process Management**: CFS scheduler with Red-Black trees
- **Security**: Capability-based access control
- **IPC**: Lock-free inter-process communication
- **System Calls**: 60 stable syscalls with binary compatibility

### Drivers (Rust #![no_std])
- **Network**: VirtIO-net, Intel e1000, Realtek RTL8139
- **Graphics**: Framebuffer, VirtIO-GPU
- **USB**: HID, Mass Storage
- **Block**: VirtIO-block, NVMe, AHCI

## Key Design Principles

1. **Security First**: Capability-based access control, hardware security features
2. **Performance**: Lock-free data structures, zero-copy operations
3. **Maintainability**: Clean separation between kernel and drivers
4. **Academic Quality**: Professional documentation and code standards

## Target Platforms

- **Primary**: x86_64 (UEFI)
- **Secondary**: aarch64 (UEFI)
- **Virtualization**: QEMU, VMware support

---
*Developed by Jérémy Noverraz (1988-2025)*  
*August 2025, Lausanne, Switzerland*
