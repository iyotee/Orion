# Orion OS Development Guide

## Table of Contents

1. [Development Environment Setup](#1-development-environment-setup)
2. [Project Structure](#2-project-structure)
3. [Build System](#3-build-system)
4. [Coding Standards](#4-coding-standards)
5. [Testing Framework](#5-testing-framework)
6. [Debugging Techniques](#6-debugging-techniques)
7. [Contributing Guidelines](#7-contributing-guidelines)

## 1. Development Environment Setup

### 1.1 Prerequisites

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake clang
sudo apt-get install -y qemu-system-x86 ovmf
sudo apt-get install -y xorriso dosfstools mtools
sudo apt-get install -y gnu-efi rustc cargo
```

#### macOS
```bash
brew install llvm cmake qemu
rustup install stable
```

### 1.2 Environment Variables

```bash
export ORION_ROOT=/path/to/orion
export PATH=$ORION_ROOT/tools:$PATH
```

### 1.3 IDE Configuration

#### VS Code
Recommended extensions:
- C/C++ Extension Pack
- Rust Analyzer
- CMake Tools
- Assembly Language Support

## 2. Project Structure

```
orion/
├── bootloader/           # UEFI bootloader
│   ├── src/             # Bootloader source code
│   └── orion-boot-protocol.h
├── kernel/              # Kernel implementation
│   ├── arch/            # Architecture-specific code
│   │   ├── x86_64/      # x86_64 implementation
│   │   └── aarch64/     # ARM64 implementation
│   ├── core/            # Core kernel functionality
│   ├── mm/              # Memory management
│   └── include/         # Kernel headers
├── drivers/             # Device drivers (Rust)
├── servers/             # User-space servers (Rust)
├── lib/                 # Shared libraries
├── tools/               # Development utilities
├── docs/                # Documentation
└── orion-build.sh       # Unified build system
```

### 2.1 Module Organization

Each kernel module follows this structure:
```
module/
├── module.c             # Implementation
├── module_internal.h    # Private headers
└── tests/               # Unit tests
```

## 3. Build System

### 3.1 Unified Build Script

The `orion-build.sh` script provides all build functionality:

```bash
# Complete build (recommended)
./orion-build.sh all

# Individual components
./orion-build.sh bootloader
./orion-build.sh kernel
./orion-build.sh drivers
./orion-build.sh iso

# Testing and validation
./orion-build.sh test
./orion-build.sh status
```

### 3.2 CMake Configuration

Kernel build configuration:
```cmake
# Debug build (default)
cmake -DCMAKE_BUILD_TYPE=Debug -DORION_ARCH=x86_64 ..

# Release build
cmake -DCMAKE_BUILD_TYPE=Release -DORION_ARCH=x86_64 ..
```

### 3.3 Cross-compilation

For ARM64 targets:
```bash
./orion-build.sh kernel ARCH=aarch64
```

## 4. Coding Standards

### 4.1 C Code Style

#### Naming Conventions
- Functions: `snake_case`
- Variables: `snake_case`
- Constants: `UPPER_CASE`
- Types: `snake_case_t`

#### Example
```c
#include <orion/kernel.h>

// Function documentation
/**
 * Initialize the process scheduler
 * @param max_processes Maximum number of processes
 * @return 0 on success, negative error code on failure
 */
int scheduler_init(size_t max_processes) {
    if (max_processes == 0) {
        return -EINVAL;
    }
    
    // Implementation...
    return 0;
}
```

### 4.2 Rust Code Style

Follow standard Rust conventions:
- Use `cargo fmt` for formatting
- Run `cargo clippy` for linting
- Document all public APIs

#### Example
```rust
//! Orion device driver framework
//! 
//! This module provides the core traits and utilities for implementing
//! device drivers in user space.

use orion_driver::{Driver, DriverError, Device};

/// A generic block device driver
pub struct BlockDriver {
    device_id: u32,
    block_size: usize,
}

impl Driver for BlockDriver {
    fn init(&mut self) -> Result<(), DriverError> {
        // Initialize hardware
        Ok(())
    }
}
```

### 4.3 Assembly Code Style

```assembly
# Function prologue documentation
# Orion kernel entry point from UEFI bootloader
# 
# Parameters:
#   %rdi - Boot information structure pointer
#   %rsi - System table pointer
kernel_start:
    # Save boot parameters
    movq %rdi, boot_info_ptr(%rip)
    movq %rsi, system_table_ptr(%rip)
    
    # Continue initialization...
```

## 5. Testing Framework

### 5.1 Unit Testing

#### C Unit Tests
Use the built-in testing framework:

```c
#include <orion/test.h>

TEST(scheduler, init_valid_params) {
    int result = scheduler_init(1000);
    ASSERT_EQ(result, 0);
}

TEST(scheduler, init_invalid_params) {
    int result = scheduler_init(0);
    ASSERT_EQ(result, -EINVAL);
}
```

#### Rust Unit Tests
```rust
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_driver_init() {
        let mut driver = BlockDriver::new(0, 512);
        assert!(driver.init().is_ok());
    }
}
```

### 5.2 Integration Testing

Run integration tests with:
```bash
./orion-build.sh test
```

### 5.3 QEMU Testing

Test the complete system:
```bash
# Boot in QEMU with debugging
./orion-build.sh test

# With GDB debugging
./orion-build.sh test-debug
```

## 6. Debugging Techniques

### 6.1 QEMU + GDB Setup

```bash
# Terminal 1: Start QEMU with GDB server
qemu-system-x86_64 -s -S -bios /usr/share/ovmf/OVMF.fd \
                   -cdrom build/orion.iso -nographic

# Terminal 2: Connect GDB
gdb build/kernel/orion-kernel.elf
(gdb) target remote :1234
(gdb) continue
```

### 6.2 Kernel Debug Output

Use the kernel logging system:
```c
#include <orion/debug.h>

void some_function(void) {
    KDEBUG("Function entry: %s", __func__);
    
    if (error_condition) {
        KERROR("Critical error in %s", __func__);
    }
    
    KINFO("Operation completed successfully");
}
```

### 6.3 Serial Console

Enable serial output for debugging:
```c
// In kernel initialization
serial_init(SERIAL_PORT_COM1, 115200);
kprintf("Kernel initialized successfully\n");
```

## 7. Contributing Guidelines

### 7.1 Development Workflow

1. **Fork** the repository
2. **Create** a feature branch
3. **Implement** changes with tests
4. **Test** thoroughly
5. **Submit** pull request

### 7.2 Commit Standards

Use conventional commit format:
```
type(scope): description

[optional body]

[optional footer]
```

Examples:
```
feat(mm): add slab allocator implementation
fix(scheduler): resolve race condition in task switching
docs(api): update system call documentation
```

### 7.3 Code Review Process

- All changes require review
- Tests must pass
- Documentation must be updated
- Performance impact considered

### 7.4 Academic Contributions

For academic use:
- Include research context
- Document experimental results
- Provide benchmark data
- Reference related work

## 8. Performance Analysis

### 8.1 Benchmarking

Use the built-in benchmark suite:
```bash
./orion-build.sh benchmark
```

### 8.2 Profiling

Profile system calls:
```c
#include <orion/profiler.h>

PROFILE_START("syscall_handler");
int result = handle_syscall(args);
PROFILE_END("syscall_handler");
```

### 8.3 Performance Targets

Maintain these performance characteristics:
- System call latency: < 150ns median
- Context switch: < 250ns
- IPC message: < 300ns latency

## 9. Documentation Standards

### 9.1 Code Documentation

- Document all public APIs
- Include usage examples
- Explain complex algorithms
- Note performance characteristics

### 9.2 Architecture Documentation

- Maintain design decisions (ADRs)
- Update diagrams regularly
- Document interfaces
- Explain security model

---

*For questions or support, please refer to the project documentation or contact the development team.*
