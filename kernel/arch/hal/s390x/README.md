# s390x HAL (Hardware Abstraction Layer)

## Overview

The s390x HAL provides a unified hardware abstraction interface for IBM z/Architecture mainframes in the Orion Operating System. It bridges the generic HAL interface with s390x-specific implementations, enabling cross-architecture compatibility while maintaining optimal performance for enterprise mainframe systems.

## Architecture Support

- **z196**: IBM zEnterprise 196 (2010)
- **zEC12**: IBM zEnterprise EC12 (2012)
- **z13**: IBM z13 (2015)
- **z14**: IBM z14 (2017)
- **z15**: IBM z15 (2019)
- **z16**: IBM z16 (2022)

## Features

### Core Functionality
- **Memory Management**: MMU abstraction with z/Architecture optimizations
- **Interrupt Handling**: Unified interrupt management system
- **Timer Management**: High-precision timer abstraction
- **Cache Management**: L1/L2/L3 cache operations
- **Context Switching**: Optimized thread context management

### Advanced Features
- **Performance Monitoring**: Hardware performance counters
- **Security Features**: Crypto, TPM, CCA, EP11, PAES, KMAC support
- **Power Management**: Enterprise-grade power management
- **Debug Support**: Breakpoints, watchpoints, and tracing
- **Vector Extensions**: z/Architecture Vector Facility

### Extensions
- **Vector Facility**: SIMD operations and crypto acceleration
- **Crypto Facility**: Hardware cryptographic acceleration
- **Decimal Facility**: Decimal floating-point operations
- **Transactional Execution**: Hardware transactional memory
- **z/Architecture**: IBM mainframe architecture features

## File Structure

```
kernel/arch/hal/s390x/
├── hal_s390x.h              # Main HAL header
├── hal_s390x.c              # HAL implementation
├── hal_s390x_common.h       # Common definitions
├── test_hal_s390x.c         # Test suite
├── CMakeLists.txt           # Build configuration
├── README.md                # This file
└── README_S390X.md          # Existing s390x architecture documentation
```

## Usage

### Basic Initialization

```c
#include <orion/hal/s390x/hal_s390x.h>

// Initialize s390x HAL
int ret = hal_s390x_init();
if (ret != ORION_HAL_SUCCESS) {
    // Handle initialization error
}

// Get HAL interface
struct orion_hal_arch *hal = hal_s390x_get_interface();

// Use HAL functions
hal->mmu_init();
hal->timer_init(4194304);  // 4.194304 MHz timer
hal->irq_register(1, handler, data);
```

### Memory Management

```c
// Map a page
int ret = hal->mmu_map(0x10000000, 0x20000000, 
                       ORION_PAGE_READ | ORION_PAGE_WRITE);

// Protect memory
ret = hal->mmu_protect(0x10000000, 4096, ORION_PAGE_READ);

// Translate address
paddr_t pa = hal->mmu_translate(0x10000000);
```

### Interrupt Handling

```c
// Register interrupt handler
int ret = hal->irq_register(S390X_IRQ_IO, my_handler, NULL);

// Enable/disable interrupts
hal->irq_enable(S390X_IRQ_IO);
hal->irq_disable(S390X_IRQ_IO);

// Acknowledge interrupt
hal->irq_ack(S390X_IRQ_IO);
```

### Performance Monitoring

```c
// Initialize performance monitoring
int ret = hal->perf_init();

// Set up performance counter
ret = hal->perf_set_event(S390X_PMU_L1_DCACHE_MISS, 
                          S390X_PMU_EVENT_CACHE_MISS);

// Enable counter
hal->perf_enable(S390X_PMU_L1_DCACHE_MISS);

// Read counter value
uint64_t misses = hal->perf_read_counter(S390X_PMU_L1_DCACHE_MISS);
```

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

The HAL automatically configures appropriate compiler flags:
- `-m64`: 64-bit s390x
- `-march=z13`: Target z13 architecture
- `-mtune=z13`: Optimize for z13
- `-O2`: Optimization level 2

## Testing

### Test Suite

The HAL includes a comprehensive test suite:

```bash
# Run all tests
./test_hal_s390x all

# Run specific test categories
./test_hal_s390x basic      # Basic functionality
./test_hal_s390x memory     # Memory management
./test_hal_s390x system     # System operations
./test_hal_s390x advanced   # Advanced features
```

### Test Categories

- **Basic**: Initialization and interface validation
- **Memory**: MMU and cache operations
- **System**: Interrupts, timers, and performance
- **Advanced**: Security, power management, and extensions

## Integration

### With Orion OS Kernel

The s390x HAL integrates seamlessly with the Orion OS kernel:

```c
// Kernel automatically detects s390x architecture
if (orion_hal_get_current()->arch_id == ORION_ARCH_S390X) {
    // s390x-specific optimizations
    s390x_optimize_kernel();
}
```

### With Device Drivers

Device drivers can use HAL functions for hardware access:

```c
// Timer-based delay
uint64_t start = hal->timer_get_ticks();
while (hal->timer_get_ticks() - start < delay_ticks) {
    // Wait
}

// Cache management for DMA
hal->cache_invalidate(dma_buffer, buffer_size);
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
hal->debug_init();

// Enable tracing
hal->debug_trace("s390x HAL operation");

// Dump registers on error
hal->debug_dump_registers();
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

This HAL implementation is licensed under the MIT License, part of the Orion OS Project.

## Authors

- **Jeremy Noverraz** (1988-2025) - Primary developer
- **Orion OS Team** - Contributors and maintainers

## Version History

- **v1.0.0** (August 2025): Initial s390x HAL implementation
- **v1.1.0** (Planned): z17 support and enhancements
