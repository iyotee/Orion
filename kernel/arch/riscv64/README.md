# ORION OS - RISC-V 64-bit Architecture Support

## Overview

This document describes the RISC-V 64-bit architecture support in the ORION operating system. The implementation provides comprehensive support for RISC-V 64-bit processors, including hardware abstraction, device drivers, extensions, and advanced features.

## Architecture Features

### Supported RISC-V Extensions

- **RV64I**: Base 64-bit integer instruction set
- **RV64M**: Multiply/divide instructions
- **RV64A**: Atomic memory operations
- **RV64F**: Single-precision floating-point
- **RV64D**: Double-precision floating-point
- **RV64C**: Compressed instructions
- **RVV**: Vector instructions (RISC-V Vector Extension)
- **RV64B**: Bit manipulation instructions
- **RV64H**: Hypervisor support
- **RV64P**: Packed SIMD operations

### Hardware Abstraction Layer (HAL)

The RISC-V HAL provides a unified interface for hardware operations:

- **CPU Feature Detection**: Automatic detection of supported extensions
- **Memory Management**: MMU initialization and page table management
- **Interrupt System**: Interrupt controller setup and management
- **Timer Management**: Hardware timer configuration and operation
- **Cache Control**: Cache initialization and management operations

### Device Drivers

Comprehensive device driver support for common RISC-V peripherals:

- **UART**: Serial communication (115200 bps default)
- **GPIO**: General-purpose input/output (32 pins)
- **I2C**: Inter-Integrated Circuit communication (100kHz default)
- **SPI**: Serial Peripheral Interface (1MHz default)
- **USB**: Universal Serial Bus (device mode)
- **Ethernet**: Network interface (10/100/1000 Mbps)
- **SD/MMC**: Storage interface (512-byte blocks)

## Implementation Details

### HAL Implementation

The HAL is implemented in `kernel/arch/hal/riscv64/`:

- `hal_riscv64.c`: Main HAL implementation
- `hal_riscv64.h`: HAL interface definitions

Key features:
- Automatic CPU feature detection via MISA CSR
- MMU setup with identity mapping for kernel space
- Interrupt system configuration with timer support
- Timer subsystem with nanosecond precision
- Cache management (platform-specific implementations)

### Device Driver Implementation

Device drivers are implemented in `kernel/arch/riscv64/devices.c`:

- **UART Driver**: FIFO-based with configurable baud rate
- **GPIO Driver**: Configurable pin direction and state
- **I2C Driver**: Master mode with start/stop conditions
- **SPI Driver**: Full-duplex with configurable clock
- **USB Driver**: Device mode with endpoint configuration
- **Ethernet Driver**: Packet-based with FIFO management
- **SD/MMC Driver**: Block-based with command interface

### Extension Support

RISC-V extensions are implemented in `kernel/arch/riscv64/extensions.c`:

- Automatic detection of available extensions
- Extension-specific initialization and configuration
- Utility functions for extension queries
- Comprehensive testing framework

### Advanced Features

Advanced features are implemented in `kernel/arch/riscv64/arch_advanced.c`:

- **Vector Extensions (RVV)**: Vector unit configuration and management
- **Cryptographic Acceleration**: Support for Zk, Zbkb, Zbkx extensions
- **Performance Monitoring**: Advanced PMU features and counters
- **Power Management**: Sleep modes and power state transitions
- **Virtualization**: Hypervisor mode setup and management

### Performance Monitoring Unit (PMU)

The PMU implementation in `kernel/arch/riscv64/pmu.c` provides:

- Hardware Performance Monitor (HPM) counter support
- Configurable event types for performance measurement
- Counter enable/disable and reset operations
- Basic counter access (cycle, instruction, time)
- Comprehensive statistics and monitoring

## Build Configuration

### CMake Configuration

The RISC-V architecture is configured via CMake:

```cmake
# Set compiler and flags for RISC-V 64-bit
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

# Compiler flags
set(RISCV64_CFLAGS
    -march=rv64gc
    -mabi=lp64d
    -fPIC
    -fPIE
    -Wall
    -Wextra
    -Werror
    -O2
    -g
    -DRISCV64_ARCH=1
    -D__riscv__=1
    -D__riscv_xlen=64
)
```

### Toolchain Requirements

- **Compiler**: `riscv64-linux-gnu-gcc`
- **Assembler**: `riscv64-linux-gnu-as`
- **Linker**: `riscv64-linux-gnu-ld`
- **Architecture**: RV64GC (64-bit with G and C extensions)

## Testing

### Test Framework

Comprehensive testing is provided in `kernel/arch/riscv64/test_riscv64.c`:

- **HAL Integration Tests**: HAL initialization and feature detection
- **Device Driver Tests**: All device driver functionality
- **Extension Support Tests**: RISC-V extension initialization
- **Advanced Features Tests**: Vector, crypto, PMU, power management
- **PMU Functionality Tests**: Performance counter operations
- **System Tests**: Interrupts, memory management, timers

### Running Tests

```bash
# Build and run RISC-V tests
cd kernel/arch/riscv64
make test_riscv64
./test_riscv64
```

## Memory Layout

### Virtual Address Space

- **Kernel Space**: 0x8000000000000000 - 0x8000000000000000 + 16GB
- **User Space**: 0x0000000000000000 - 0x7FFFFFFFFFFFFFFF (8EB)

### Page Sizes

- **Standard Pages**: 4KB (4096 bytes)
- **Huge Pages**: 2MB (2,097,152 bytes)
- **Giga Pages**: 1GB (1,073,741,824 bytes)

## Interrupt System

### Interrupt Numbers

- **Software Interrupt**: 3
- **Timer Interrupt**: 7
- **External Interrupt**: 11

### Exception Numbers

- **Instruction Misaligned**: 0
- **Instruction Access Fault**: 1
- **Illegal Instruction**: 2
- **Breakpoint**: 3
- **Load Address Misaligned**: 4
- **Load Access Fault**: 5
- **Store Address Misaligned**: 6
- **Store Access Fault**: 7
- **ECALL (User Mode)**: 8
- **ECALL (Supervisor Mode)**: 9
- **ECALL (Machine Mode)**: 11
- **Instruction Page Fault**: 12
- **Load Page Fault**: 13
- **Store Page Fault**: 15

## Power Management

### Supported Power States

- **Active**: Full operation mode
- **Idle**: Reduced power consumption
- **Sleep**: Deep sleep with wake-up capability
- **Hibernate**: Lowest power state

### Power Management Features

- Automatic power state transitions
- Configurable wake-up sources
- Power consumption monitoring
- Thermal management support

## Security Features

### Physical Memory Protection (PMP)

- Configurable memory regions
- Access control (read/write/execute)
- Locked and unlocked regions
- Machine mode access control

### Security Extensions

- **PMP**: Physical Memory Protection
- **PMA**: Physical Memory Attributes
- **Debug**: Debug interface security
- **Trace**: Secure trace capabilities

## Performance Optimization

### Cache Management

- L1 instruction and data cache support
- L2 unified cache support
- L3 cache support (if available)
- Cache line size optimization
- Cache coherency management

### Vector Optimization

- RVV vector extension support
- Vector length configuration
- Element width optimization
- Vector operation pipelining

## Debugging and Development

### Debug Features

- **GDB Support**: Full GDB debugging capabilities
- **Trace Support**: Instruction and data tracing
- **Performance Counters**: Detailed performance analysis
- **Error Reporting**: Comprehensive error handling

### Development Tools

- **Cross-compilation**: Full cross-compilation support
- **Emulation**: QEMU and other emulator support
- **Profiling**: Performance profiling tools
- **Testing**: Automated test framework

## Platform Support

### Supported Platforms

- **Generic RISC-V 64-bit**: Base implementation
- **SiFive**: Freedom U540, HiFive Unleashed
- **Microchip**: PolarFire SoC
- **NVIDIA**: Jetson Orin (RISC-V cores)
- **Custom Platforms**: Platform-specific adaptations

### Platform-Specific Features

- **Device Tree**: Automatic device detection
- **Platform Hooks**: Platform-specific initialization
- **Hardware Abstraction**: Platform-specific HAL extensions
- **Driver Support**: Platform-specific device drivers

## Future Enhancements

### Planned Features

- **RISC-V Vector Extension v1.0**: Full vector support
- **RISC-V Hypervisor Extension**: Enhanced virtualization
- **RISC-V Crypto Extensions**: Advanced cryptographic support
- **RISC-V Debug Extension**: Enhanced debugging capabilities
- **Multi-core Support**: SMP and NUMA optimization

### Performance Improvements

- **Vector Optimization**: Enhanced vector operation performance
- **Cache Optimization**: Advanced cache management
- **Power Optimization**: Dynamic power management
- **Memory Optimization**: Advanced memory management

## Contributing

### Development Guidelines

- Follow RISC-V specification compliance
- Maintain backward compatibility
- Implement comprehensive testing
- Document all public interfaces
- Follow ORION OS coding standards

### Testing Requirements

- All new features must include tests
- Tests must pass on supported platforms
- Performance benchmarks for optimizations
- Compatibility testing with RISC-V tools

## References

### RISC-V Specifications

- [RISC-V User-Level ISA Specification](https://riscv.org/specifications/)
- [RISC-V Privileged Architecture Specification](https://riscv.org/specifications/privileged-isa/)
- [RISC-V Vector Extension Specification](https://github.com/riscv/riscv-v-spec)

### ORION OS Documentation

- [ORION OS Architecture Overview](../README.md)
- [ORION OS Development Guide](../../../documentation/Development-Guide.md)
- [ORION OS Contributing Guidelines](../../../documentation/Contributing-Guidelines.md)

## License

This implementation is licensed under the MIT License. See the LICENSE file for details.

## Authors

- **Jeremy Noverraz** (1988-2025) - Primary implementation
- **ORION OS Project Team** - Contributions and testing

---

*Last updated: August 2025*
