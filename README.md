<div align="center">

# <img src="orion_logo.png" alt="Orion OS Logo" width="200"/>

# Orion Operating System

> **A Modern Hybrid Micro-Kernel Operating System**  
> *Developed by Jérémy Noverraz (1988-2025)*  
> *August 2025, Lausanne, Switzerland*

</div>

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://img.shields.io/badge/Build-Passing-green.svg)]()
[![Version](https://img.shields.io/badge/Version-1.0.0--alpha-blue.svg)]()
[![Architecture](https://img.shields.io/badge/Architecture-x86__64-red.svg)]()

## 🌟 Overview

Orion OS is a modern, security-focused operating system built from the ground up with performance and reliability in mind. It combines the best aspects of monolithic and microkernel architectures in a hybrid approach, featuring a minimal Trusted Computing Base (TCB) and comprehensive security mechanisms.

### 🎯 Key Features

- **🔒 Security-First Design**: Capability-based access control, KASLR, W^X enforcement, CFI
- **⚡ High Performance**: CFS scheduler, lock-free IPC, zero-copy operations
- **🛡️ Memory Safety**: Comprehensive MMU with 4-level paging, heap protection, guard pages
- **🌐 Modern Hardware Support**: Complete driver ecosystem in Rust for safety
- **🔧 Developer-Friendly**: Clean APIs, comprehensive documentation, extensive testing

## 🏗️ Architecture

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

### 🧩 Components

#### **Kernel Core (C11 Freestanding)**
- **Memory Management**: PMM, VMM, Slab allocator, Heap allocator
- **Process Management**: CFS scheduler with Red-Black trees
- **Security**: Capability system, hardware security features
- **IPC**: Lock-free inter-process communication
- **System Calls**: 60 stable syscalls with binary compatibility

#### **Drivers (Rust #![no_std])**
- **Network**: VirtIO-net, Intel e1000, Realtek RTL8139
- **Graphics**: Framebuffer, VirtIO-GPU with 2D/3D acceleration
- **USB**: HID (keyboard/mouse), Mass Storage (USB drives)
- **Block**: VirtIO-block, NVMe, AHCI SATA

## 🚀 Quick Start

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential clang cmake python3 xorriso qemu-system-x86

# Arch Linux
sudo pacman -S base-devel clang cmake python xorriso qemu

# Rust (for drivers)
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
rustup target add x86_64-unknown-none
```

### Building Orion OS

```bash
# Clone the repository
git clone https://github.com/your-org/orion-os.git
cd orion-os

# Build everything
./tools/orion-build.sh all

# Or build components separately
./tools/orion-build.sh bootloader    # Build UEFI bootloader
./tools/orion-build.sh kernel        # Build kernel
./tools/orion-build.sh drivers       # Build Rust drivers
./tools/orion-build.sh iso           # Generate bootable ISO
```

### Testing in QEMU

```bash
# Boot the generated ISO
qemu-system-x86_64 -M q35 -m 4G -smp 4 \
  -drive if=pflash,format=raw,readonly=on,file=/usr/share/ovmf/OVMF_CODE.fd \
  -drive if=pflash,format=raw,file=/usr/share/ovmf/OVMF_VARS.fd \
  -cdrom build/orion-os.iso \
  -netdev user,id=net0 -device virtio-net-pci,netdev=net0 \
  -device virtio-gpu-pci \
  -enable-kvm
```

## 📚 Documentation

### 📖 Essential Documentation
- **[Architecture Guide](docs/ARCHITECTURE.md)** - System design and hybrid micro-kernel architecture
- **[Installation Guide](docs/INSTALLATION.md)** - Complete build and setup instructions
- **[Development Guide](docs/DEVELOPMENT.md)** - Contributing guidelines and coding standards
- **[API Reference](docs/API.md)** - Complete system calls and kernel API documentation

## 🔧 Development

### 🏗️ Build System

Orion uses a unified Bash-based build system:

```bash
./tools/orion-build.sh help

Commands:
  clean      Clean all build artifacts
  bootloader Build the Orion UEFI bootloader
  kernel     Build the Orion kernel
  iso        Generate bootable UEFI ISO image
  all        Complete build (bootloader + kernel + ISO)
  test       Test ISO in QEMU emulator
  install    Install build dependencies
  status     Display build status
  help       Show help message
```

### 🧪 Testing

```bash
# Run all tests
python3 tools/test.py --all

# Run specific test categories
python3 tools/test.py --suite build      # Build system tests
python3 tools/test.py --suite kernel     # Kernel tests
python3 tools/test.py --suite qemu       # QEMU boot tests
```

### 📊 Performance Metrics

| Component | Metric | Target | Current |
|-----------|--------|--------|---------|
| Syscall Latency | avg/max | <100ns/<500ns | ✅ |
| Context Switch | time | <1μs | ✅ |
| IPC Throughput | msg/sec | >1M | ✅ |
| Memory Allocation | time | <10μs | ✅ |
| Boot Time | UEFI→Shell | <3s | ✅ |

## 🛡️ Security

### 🔐 Security Features

- **Capability-Based Access Control**: Fine-grained permissions without UID/GID
- **Hardware Security**: SMEP, SMAP, UMIP, KASLR, CFI enforcement
- **Memory Protection**: W^X enforcement, stack canaries, guard pages
- **Intrusion Detection**: Real-time threat detection and response
- **Audit Logging**: Comprehensive security event tracking

### 🚨 Security Reporting

Please report security vulnerabilities to: `security@orion-os.dev`

We follow responsible disclosure practices and will acknowledge security reports within 48 hours.

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

### 📋 Development Process

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

### 📏 Code Standards

- **C Code**: C11 standard, clang-format style
- **Rust Code**: Stable Rust, #![no_std], cargo fmt
- **Documentation**: Comprehensive inline documentation
- **Testing**: Unit tests required for new features

## 🗺️ Roadmap

### 📅 Version 1.0 (Current)
- ✅ Hybrid micro-kernel architecture
- ✅ Complete memory management
- ✅ CFS scheduler with SMP support
- ✅ Capability-based security
- ✅ Rust driver framework
- ✅ Basic networking and storage

### 📅 Version 1.1 (Q4 2025)
- 🔄 Audio subsystem (AC97, HDA)
- 🔄 WiFi and Bluetooth support
- 🔄 Advanced power management
- 🔄 Container support
- 🔄 Performance optimization

### 📅 Version 2.0 (2026)
- 🔄 Multi-architecture support (ARM64, RISC-V)
- 🔄 Distributed computing features
- 🔄 Advanced graphics (Vulkan)
- 🔄 Real-time capabilities
- 🔄 Commercial certification

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **MINIX**: Inspiration for microkernel design
- **Linux**: Reference for driver interfaces
- **seL4**: Security and verification concepts
- **Rust Community**: Safe systems programming
- **UEFI Specification**: Modern boot standards

## 📞 Contact

- **Author**: Jérémy Noverraz
- **Email**: jeremy@orion-os.dev
- **Location**: Lausanne, Switzerland
- **GitHub**: [OrionOS Project](https://github.com/orion-os)

---

> *"Building the future of operating systems, one line of code at a time."*

**Made with ❤️ in Switzerland 🇨🇭**