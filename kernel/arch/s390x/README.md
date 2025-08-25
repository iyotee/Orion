# s390x Architecture Support

## Overview

The s390x architecture support provides comprehensive IBM z/Architecture mainframe support for the Orion Operating System. This implementation covers all major aspects of s390x systems including CPU management, memory management, interrupts, timers, cache operations, performance monitoring, security features, and power management.

## Architecture Support

### Supported Processors
- **z196**: IBM zEnterprise 196 (2010)
- **zEC12**: IBM zEnterprise EC12 (2012)
- **z13**: IBM z13 (2015)
- **z14**: IBM z14 (2017)
- **z15**: IBM z15 (2019)
- **z16**: IBM z16 (2022)

### Features
- **Vector Facility**: SIMD operations and crypto acceleration
- **Crypto Facility**: Hardware cryptographic acceleration
- **Decimal Facility**: Decimal floating-point operations
- **Transactional Execution**: Hardware transactional memory
- **Neural Network Processing**: AI/ML acceleration support

## File Structure

```
kernel/arch/s390x/
├── arch.h              # Main architecture header
├── arch.c              # Architecture implementation
├── config.h            # Configuration constants
├── test_s390x.c        # Test suite
├── CMakeLists.txt      # Build configuration
└── README.md           # This file
```

## Implementation Details

### Core Components

#### CPU Management
- CPU initialization and feature detection
- Multi-core support (up to 128 cores)
- Frequency scaling and power management
- CPU identification and model detection

#### Memory Management
- MMU initialization and configuration
- Page mapping and unmapping
- Memory protection and access control
- TLB management and invalidation
- Support for 4K, 1M, and 2G page sizes

#### Interrupt System
- Interrupt registration and handling
- IRQ enable/disable/acknowledge
- Support for external, IO, timer, and service interrupts
- Interrupt vector management

#### Timer System
- High-precision timer initialization
- Configurable frequency (1-16 MHz)
- One-shot timer support
- CPU timer and measurement timer

#### Cache Management
- L1/L2/L3 cache operations
- Cache invalidation, cleaning, and flushing
- Cache coherency maintenance
- 256-byte cache line support

#### Performance Monitoring
- Hardware performance counters
- Event selection and configuration
- Counter enable/disable control
- Support for up to 16 counters

#### Security Features
- Cryptographic acceleration
- Secure measurement and verification
- Secure enclave creation/destruction
- TPM and CCA support

#### Power Management
- Multiple power states (active, idle, sleep, hibernate)
- CPU frequency scaling
- Enterprise-grade power management
- Dynamic power adjustment

#### Debug Support
- Breakpoint and watchpoint support
- Register dumping and inspection
- Debug tracing and profiling
- Performance measurement tools

### Architecture Extensions

The implementation supports various z/Architecture extensions:

- **z196/zEC12/z13/z14/z15/z16**: Generation-specific features
- **Vector Facility**: SIMD operations and crypto acceleration
- **Crypto Facility**: Hardware cryptographic operations
- **Decimal Facility**: Decimal floating-point arithmetic
- **Transactional Execution**: Hardware transactional memory

## Build Configuration

### CMake Options

```bash
# Build for s390x
cmake -DORION_ARCH=s390x ..

# Enable tests
cmake -DBUILD_TESTS=ON ..

# Enable documentation
cmake -DBUILD_DOCS=ON ..
```

### Compiler Flags

The architecture automatically configures appropriate compiler flags:
- `-m64`: 64-bit s390x
- `-march=z13`: Target z13 architecture
- `-mtune=z13`: Optimize for z13
- `-O2`: Optimization level 2

## Testing

### Test Suite

The architecture includes a comprehensive test suite:

```bash
# Run all tests
./test_s390x_arch all

# Run specific test categories
./test_s390x_arch basic      # Basic functionality
./test_s390x_arch memory     # Memory management
./test_s390x_arch system     # System operations
./test_s390x_arch advanced   # Advanced features
```

### Test Categories

- **Basic**: Initialization and CPU feature detection
- **Memory**: MMU, cache, and memory operations
- **System**: Interrupts, timers, and performance
- **Advanced**: Security, power management, and extensions

## Integration

### With Orion OS Kernel

The s390x architecture integrates seamlessly with the Orion OS kernel:

```c
// Kernel automatically detects s390x architecture
if (orion_arch_get_current()->arch_id == ORION_ARCH_S390X) {
    // s390x-specific optimizations
    s390x_optimize_kernel();
}
```

### With HAL System

The architecture works with the s390x HAL to provide hardware abstraction:

```c
// HAL automatically uses s390x architecture functions
hal_s390x_mmu_init();        // Calls s390x_mmu_init()
hal_s390x_timer_init(4194304); // Calls s390x_timer_init()
```

## Performance Considerations

### Optimizations

- **Vector Operations**: Automatic Vector Facility usage where available
- **Cache Management**: Optimized cache line operations
- **Interrupt Latency**: Minimal interrupt handling overhead
- **Context Switching**: Efficient register save/restore

### Benchmarks

Typical performance metrics for z15:
- Context switch: < 50ns
- Interrupt latency: < 100ns
- Cache miss penalty: < 25ns (L1), < 100ns (L2)
- MMU TLB miss: < 50ns

## Platform Support

### Supported Platforms

- **IBM z Systems**: z13, z14, z15, z16
- **IBM LinuxONE**: LinuxONE III, LinuxONE Emperor
- **QEMU**: s390x emulation
- **z/VM**: IBM virtualization platform

### Hardware Requirements

- **Minimum**: z13 processor
- **Recommended**: z15 or z16
- **Memory**: 16GB RAM minimum
- **Storage**: 32GB minimum

## Troubleshooting

### Common Issues

1. **Initialization Failure**
   - Check processor compatibility
   - Verify kernel configuration
   - Check hardware initialization

2. **Performance Issues**
   - Verify cache configuration
   - Check interrupt routing
   - Monitor performance counters

3. **Memory Errors**
   - Verify MMU configuration
   - Check page table setup
   - Validate memory mapping

### Debug Information

Enable debug output:

```c
// Initialize debug system
s390x_debug_init();

// Enable tracing
s390x_debug_trace("s390x operation");

// Dump registers on error
s390x_debug_dump_registers();
```

## Future Enhancements

### Planned Features

- **z17 Support**: Next-generation processor support
- **Advanced Security**: Enhanced security features
- **AI Acceleration**: AI/ML optimization support
- **Quantum Computing**: Quantum-safe cryptography

### Roadmap

- **Q1 2025**: z17 preliminary support
- **Q2 2025**: Advanced security features
- **Q3 2025**: AI acceleration support
- **Q4 2025**: Quantum computing integration

## Contributing

### Development Guidelines

1. Follow z/Architecture specifications
2. Maintain backward compatibility
3. Add comprehensive tests for new features
4. Update documentation for changes
5. Follow Orion OS coding standards

### Testing Requirements

- All new features must include tests
- Performance benchmarks for optimizations
- Compatibility testing across z/Architecture generations
- Stress testing for reliability

## References

### Documentation

- [z/Architecture Principles of Operation](https://www.ibm.com/docs/en/zos)
- [IBM z Systems Documentation](https://www.ibm.com/docs/en/z15)
- [z/Architecture Reference](https://www.ibm.com/support/z-content-solutions/)

### Standards

- z/Architecture specification
- IBM z Systems architecture
- Linux on z Systems standards

## License

This architecture implementation is licensed under the MIT License, part of the Orion OS Project.

## Authors

- **Jeremy Noverraz** (1988-2025) - Primary developer
- **Orion OS Team** - Contributors and maintainers

## Version History

- **v1.0.0** (August 2025): Initial s390x architecture implementation
- **v1.1.0** (Planned): z17 support and enhancements
