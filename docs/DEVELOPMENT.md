# Orion OS Development Guide

## Project Structure

```
Orion/
├── kernel/                    # C11 freestanding kernel
│   ├── core/                 # Core kernel subsystems
│   ├── mm/                   # Memory management
│   ├── arch/x86_64/          # Architecture-specific code
│   └── include/orion/        # Kernel headers
├── drivers/                  # Rust #![no_std] drivers
│   ├── block/               # Block device drivers
│   ├── net/                 # Network drivers
│   ├── gpu/                 # Graphics drivers
│   └── usb/                 # USB drivers
├── lib/orion-driver/         # Rust driver framework
├── bootloader/              # UEFI bootloader
├── tools/                   # Build and development tools
└── docs/                    # Documentation
```

## Development Setup

### Code Style
- **C Code**: K&R style, 4 spaces, descriptive names
- **Rust Code**: Standard Rust formatting with `rustfmt`
- **Comments**: English only, academic style
- **Headers**: Include copyright and location info

### Testing
```bash
# Run all tests
./tools/test.py --all

# Specific test suites
./tools/test.py --suite kernel     # Kernel tests
./tools/test.py --suite qemu       # QEMU boot tests
```

## Core Subsystems

### Memory Management (`kernel/mm/`)
- **PMM**: Physical memory manager (buddy allocator)
- **VMM**: Virtual memory manager (4-level paging)
- **Heap**: Kernel heap allocator (`kmalloc`/`kfree`)
- **Slab**: Object caching allocator

### Process Management (`kernel/core/scheduler.c`)
- **CFS Scheduler**: Completely Fair Scheduler with Red-Black trees
- **Process/Thread**: Full POSIX-compatible process model
- **Context Switching**: Architecture-specific assembly

### Security (`kernel/core/capabilities.c`)
- **Capabilities**: Unforgeable access tokens
- **Hardware Security**: KASLR, W^X, stack canaries
- **IDS**: Intrusion detection system

### IPC (`kernel/core/ipc.c`)
- **Lock-free Queues**: High-performance message passing
- **Zero-copy**: Page remapping for large messages
- **Shared Memory**: Secure shared memory pools

## Driver Development

### Rust Driver Framework
Located in `lib/orion-driver/`, provides:
- Safe MMIO abstractions
- IPC communication with kernel
- Device enumeration and probing
- Error handling and logging

### Creating a New Driver
1. Add to appropriate `drivers/` subdirectory
2. Implement required traits from `orion-driver`
3. Add to build system in `Cargo.toml`
4. Test with QEMU device emulation

## Contributing

### Code Review Process
1. Fork the repository
2. Create feature branch
3. Implement changes with tests
4. Submit pull request
5. Address review feedback

### Commit Guidelines
- Use descriptive commit messages
- Include component prefix (e.g., "kernel/mm:", "drivers/net:")
- Reference issues when applicable
- Sign commits with academic information

### Quality Standards
- All code must compile without warnings
- New features require documentation
- Test coverage for critical paths
- Academic-grade code quality

---
*For API documentation, see API.md*
