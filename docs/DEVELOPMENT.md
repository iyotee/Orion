# Orion OS Development Guide

> **Technical Development Documentation**  
> *Comprehensive guide for system development and contribution*

---

## Project Overview

**Orion OS** is a hybrid microkernel operating system designed for performance, security, and maintainability.

---

## Project Structure

```
Orion/
├── kernel/                   # C11 freestanding kernel
│   ├── core/                 # Core kernel subsystems
│   ├── mm/                   # Memory management
│   ├── arch/x86_64/          # Architecture-specific code
│   └── include/orion/        # Kernel headers
├── drivers/                  # Rust #![no_std] drivers
│   ├── block/                # Block device drivers
│   ├── net/                  # Network drivers
│   ├── gpu/                  # Graphics drivers
│   └── usb/                  # USB drivers
├── lib/orion-driver/         # Rust driver framework
├── bootloader/               # UEFI bootloader
├── tools/                    # Build and development tools
└── docs/                     # Documentation
```

---

## Development Setup

### Prerequisites
- Linux/macOS/Windows with WSL2
- GCC 9+ or Clang 12+ for C compilation
- Rust 1.70+ for driver development
- CMake 3.20+ for build system
- QEMU 6.0+ for testing

### Code Style
- **C Code**: K&R style, 4 spaces, descriptive names
- **Rust Code**: Standard Rust formatting with `rustfmt`
- **Comments**: English only, technical documentation style
- **Headers**: Include appropriate copyright notices

---

## Testing & Quality Assurance

### Testing Framework
```bash
# Run all tests
./tools/test.py --all

# Specific test suites
./tools/test.py --suite kernel     # Kernel tests
./tools/test.py --suite qemu       # QEMU boot tests
./tools/test.py --suite drivers    # Driver tests
./tools/test.py --suite security   # Security tests
```

### Quality Standards
- All code must compile without warnings
- New features require documentation
- Test coverage for critical paths
- Professional code quality
- Security audit for all new features

---

## Core Subsystems

### Memory Management (`kernel/mm/`)

#### **Physical Memory Manager (PMM)**
- **Buddy Allocator**: Efficient page allocation with O(log n) complexity
- **Bitmap Management**: Fast free page search and allocation
- **NUMA Support**: Multi-node memory management

#### **Virtual Memory Manager (VMM)**
- **4-Level Paging**: Modern x86_64 page table structure
- **Copy-on-Write (COW)**: Memory sharing with lazy allocation
- **Demand Paging**: Automatic page allocation on access
- **Permission Checking**: Advanced memory protection
- **TLB Management**: Efficient translation lookaside buffer

#### **Memory Allocators**
- **Kernel Heap**: `kmalloc`/`kfree` with slab optimization
- **Slab Allocator**: Object caching for performance
- **Pool Allocator**: Fixed-size allocation pools

### Process Management (`kernel/core/scheduler.c`)

#### **CFS Scheduler**
- **Completely Fair Scheduler**: Red-Black tree implementation
- **SMP Support**: Multi-processor scheduling with IPI
- **Load Balancing**: Dynamic load distribution
- **Priority Management**: Real-time and normal priorities

#### **Process Model**
- **POSIX Compatibility**: Full process/thread model
- **ELF Loading**: Dynamic binary loading
- **Context Switching**: Architecture-specific assembly
- **Resource Management**: File descriptors, memory, capabilities

### Security System (`kernel/core/capabilities.c`)

#### **Capability Model**
- **Unforgeable Tokens**: Cryptographic capability system
- **Fine-grained Access Control**: Per-resource permissions
- **Capability Delegation**: Secure privilege transfer
- **Revocation System**: Immediate access removal

#### **Hardware Security**
- **KASLR**: Kernel Address Space Layout Randomization
- **W^X**: Write XOR Execute memory protection
- **Stack Canaries**: Buffer overflow protection
- **SMEP/SMAP**: Supervisor Mode Execution Prevention

#### **Security Monitoring**
- **Intrusion Detection**: Real-time security monitoring
- **Audit Logging**: Comprehensive security events
- **Core Dumps**: Crash analysis and debugging
- **Violation Reporting**: Security incident handling

### IPC System (`kernel/core/ipc.c`)

#### **High-Performance Communication**
- **Lock-free Queues**: Zero-lock message passing
- **Zero-copy**: Page remapping for large messages
- **Shared Memory**: Secure memory pools
- **Port Management**: Dynamic port creation/destruction

---

## Driver Development

### Rust Driver Framework

Located in `lib/orion-driver/`, provides:

#### Core Features
- **Safe MMIO Abstractions**: Memory-mapped I/O safety
- **IPC Communication**: Kernel communication protocols
- **Device Enumeration**: Automatic device detection
- **Error Handling**: Comprehensive error management
- **Logging System**: Structured logging and debugging

#### Driver Traits
```rust
pub trait Driver {
    fn init(&mut self) -> Result<(), DriverError>;
    fn shutdown(&mut self) -> Result<(), DriverError>;
    fn handle_interrupt(&mut self) -> Result<(), DriverError>;
}
```

### Creating a New Driver

#### 1. Project Structure
```bash
drivers/your_device/
├── Cargo.toml
├── src/
│   ├── main.rs
│   ├── device.rs
│   └── protocol.rs
└── tests/
```

#### 2. Implementation Steps
1. Add to appropriate `drivers/` subdirectory
2. Implement required traits from `orion-driver`
3. Add to build system in `Cargo.toml`
4. Test with QEMU device emulation
5. Add comprehensive error handling

#### 3. Testing & Validation
- Unit Tests: Component-level testing
- Integration Tests: Full driver testing
- Performance Tests: Benchmarking and optimization
- Security Tests: Vulnerability assessment

---

## API Reference

### System Calls (60+ functions)

#### Process Management
```c
// Process creation and management
int sys_proc_create(const char* path, char** argv, char** envp);
int sys_wait(pid_t pid, int* status);
pid_t sys_getpid(void);
pid_t sys_gettid(void);
int sys_exit(int code);
```

#### Memory Management
```c
// Virtual memory operations
int sys_vm_map(void* addr, size_t size, int prot, int flags);
int sys_vm_unmap(void* addr, size_t size);
int sys_vm_protect(void* addr, size_t size, int prot);
```

#### I/O Operations
```c
// File operations
int sys_open(const char* path, int flags, mode_t mode);
int sys_read(int fd, void* buf, size_t count);
int sys_write(int fd, const void* buf, size_t count);
int sys_close(int fd);
```

#### IPC Communication
```c
// Inter-process communication
int sys_port_create(void);
int sys_msg_send(int port, const void* msg, size_t size);
int sys_msg_recv(int port, void* buf, size_t size);
```

#### Time and Timing
```c
// Time management
int sys_clock_get(clockid_t type, struct timespec* time);
int sys_nanosleep(const struct timespec* req, struct timespec* rem);
```

### Kernel APIs

#### Memory Management APIs
```c
// Physical Memory Manager
void* pmm_alloc_page(void);
void pmm_free_page(void* page);
size_t pmm_get_free_pages(void);

// Virtual Memory Manager
int vmm_map_page(vm_space_t* space, uint64_t vaddr, uint64_t paddr, uint32_t flags);
int vmm_unmap_page(vm_space_t* space, uint64_t vaddr);
int vmm_mark_cow(vm_space_t* space, uint64_t vaddr);
int vmm_handle_cow_fault(vm_space_t* space, uint64_t vaddr);

// Heap Allocator
void* kmalloc(size_t size);
void kfree(void* ptr);
void* krealloc(void* ptr, size_t size);
```

#### Process Management APIs
```c
// Scheduler APIs
process_t* scheduler_create_process(const char* name);
thread_t* scheduler_create_thread(process_t* proc, void* entry);
void sched_yield(void);
void scheduler_tick(void);

// Process Management
int process_setup_args(process_t* proc, char** argv, char** envp);
int elf_load_process(process_t* proc, const char* path);
process_t* scheduler_find_process(pid_t pid);
```

#### Security APIs
```c
// Capability Management
cap_id_t cap_create(cap_rights_t rights, cap_type_t type);
bool cap_check_rights(cap_id_t cap, cap_rights_t required);
int cap_grant(cap_id_t source, uint64_t target_pid, cap_rights_t rights);
int cap_revoke(uint64_t target_pid, cap_rights_t rights);

// Security Functions
bool security_is_address_valid(uint64_t addr, size_t size);
void security_report_violation(const char* desc);
void security_audit_log(const char* event, const char* details);
```

---

## Current Implementation

### Kernel Core
The kernel implements a hybrid microkernel architecture with the following components:
- **CPU Management**: APIC support, SMP, security features
- **Memory Management**: Advanced VMM with COW, demand paging
- **Scheduler**: CFS implementation with SMP support
- **Interrupt Handling**: Page faults, RTC, device interrupts
- **Security System**: Capability-based access control
- **IPC System**: Lock-free communication with zero-copy support

### Driver Framework
The Rust driver framework provides:
- **Framework**: IPC, MMIO, driver traits, error handling
- **USB HID Driver**: Input processing, transfer management
- **VirtIO GPU Driver**: Graphics operations, resource management
- **USB Storage Driver**: Bulk transfers, SCSI commands
- **Network Drivers**: Packet I/O, queue management
- **Block Drivers**: Controller initialization, I/O operations

### Future Development
- **Multi-ISA Architecture**: ARM64, RISC-V, MIPS64, POWER64 support
- **Enhanced Graphics**: Advanced GPU acceleration
- **Network Stack**: Complete TCP/IP implementation
- **File Systems**: Advanced file system support

---

## Contributing

### Code Review Process
1. Fork the repository
2. Create feature branch with descriptive name
3. Implement changes with comprehensive tests
4. Submit pull request with detailed description
5. Address review feedback promptly

### Commit Guidelines
- Use descriptive commit messages
- Include component prefix (e.g., "kernel/mm:", "drivers/net:")
- Reference issues when applicable
- Follow established commit conventions

### Quality Standards
- All code must compile without warnings
- New features require documentation
- Test coverage for critical paths
- Professional code quality
- Security review for all changes

---

## Debugging & Troubleshooting

### Kernel Logging
```c
// Log levels
KLOG_INFO("System initialized successfully");
KLOG_WARN("Memory pressure detected");
KLOG_ERROR("Failed to allocate memory");
KLOG_DEBUG("Processing interrupt %d", irq);
```

### Core Dumps
- Automatic generation on kernel panic
- Memory state preservation for analysis
- Stack trace generation with symbols
- Register state capture for debugging

### Performance Profiling
- Scheduler statistics and load metrics
- Memory usage tracking and allocation patterns
- Interrupt frequency and timing analysis
- Driver performance metrics

---

## Support

### Project Resources
- **Issues**: [GitHub Issues](https://github.com/iyotee/Orion/issues)
- **Discussions**: [GitHub Discussions](https://github.com/iyotee/Orion/discussions)
- **Wiki**: [Documentation Wiki](https://github.com/iyotee/Orion/wiki)

### Development Support
- Review existing documentation
- Search GitHub issues for known problems
- Consult the development community
- Follow established contribution guidelines
