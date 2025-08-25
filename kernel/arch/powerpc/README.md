# ORION OS - POWER Architecture Support

This directory contains the POWER architecture implementation for ORION OS, supporting POWER8, POWER9, and POWER10 processors.

## Architecture Overview

POWER (Performance Optimization With Enhanced RISC) is IBM's RISC architecture family, designed for high-performance computing and enterprise workloads.

### Supported Processors
- **POWER8**: 2013, 22nm, up to 12 cores
- **POWER9**: 2017, 14nm, up to 24 cores  
- **POWER10**: 2021, 7nm, up to 15 cores

### Key Features
- 64-bit architecture
- Big-endian byte order
- VSX (Vector Scalar eXtension)
- AltiVec vector instructions
- Hardware transactional memory (HTM)
- Decimal floating point (DFP)
- XIVE interrupt controller
- Advanced power management

## File Structure

```
kernel/arch/power/
├── arch.h                 # Main architecture header
├── config.h               # Configuration constants
├── boot_config.h          # Boot configuration
├── arch.c                 # Core architecture implementation
├── arch_advanced.c        # Advanced features
├── entry.c                # System entry point
├── entry_asm.S            # Assembly bootstrap code
├── interrupts.c           # Interrupt management
├── mmu.c                  # Memory management unit
├── timer.c                # Timer and time base
├── cache.c                # Cache operations
├── test_power.c           # Test suite
├── CMakeLists.txt         # Build configuration
├── power.ld               # Linker script
└── README.md              # This file
```

## Implementation Details

### CPU Management
- PVR (Processor Version Register) detection
- MSR (Machine State Register) control
- HID0 (Hardware Implementation Dependent) configuration
- CPU frequency and affinity management

### Memory Management
- 3-level page table structure
- TLB (Translation Lookaside Buffer) management
- ASID (Address Space ID) support
- Page protection and mapping

### Interrupt System
- XIVE (eXternal Interrupt Virtualization Engine) support
- Traditional interrupt controller fallback
- Priority-based interrupt handling
- CPU affinity and trigger mode configuration

### Timer System
- Decrementer register (SPR 22)
- Time Base (TBU/TBL registers)
- Configurable frequency and precision
- One-shot, periodic, and auto-reload modes

### Cache Management
- L1 instruction and data caches
- L2 and L3 unified caches
- Cache line operations (flush, invalidate)
- Prefetching support

### Vector Extensions
- VSX (Vector Scalar eXtension) 128/256/512-bit
- AltiVec 128-bit vector operations
- Cryptographic acceleration
- Hardware transactional memory

## Building

### Prerequisites
- GCC with POWER support
- CMake 3.10+
- Cross-compilation toolchain (for non-POWER hosts)

### Compilation
```bash
cd kernel/arch/power
mkdir build && cd build
cmake ..
make
```

### Compiler Flags
- `-mcpu=power8`: Target POWER8 architecture
- `-maltivec`: Enable AltiVec instructions
- `-mvsx`: Enable VSX instructions
- `-mhard-float`: Enable hardware floating point
- `-m64`: 64-bit mode

## Testing

The test suite covers:
- CPU feature detection
- MMU operations
- Interrupt handling
- Timer functionality
- Cache operations
- Vector operations
- Advanced features

Run tests with:
```bash
./power_test
```

## Boot Process

1. **Assembly Bootstrap** (`entry_asm.S`)
   - Set up initial stack pointer
   - Clear BSS section
   - Initialize basic registers
   - Call C bootstrap function

2. **C Bootstrap** (`entry.c`)
   - Initialize all subsystems
   - Set up device drivers
   - Initialize file system
   - Start user space

3. **System Initialization**
   - CPU and feature detection
   - MMU and memory setup
   - Interrupt controller configuration
   - Timer and cache initialization

## Memory Layout

```
0x0000000001000000 - 0x0000000001FFFFFF: Kernel (16MB)
0x0000000002000000 - 0x00000000020FFFFF: Stack (1MB)
0x0000000002100000 - 0x0000000002FFFFFF: Heap (15MB)
0x4000000000000000 - 0x400000003FFFFFFF: Device Memory (1GB)
0x8000000000000000 - 0xFFFFFFFFFFFFFFFF: High Memory (1TB)
```

## Device Support

### Peripherals
- UART (serial console)
- GPIO (general purpose I/O)
- I2C (inter-integrated circuit)
- SPI (serial peripheral interface)
- USB 2.0
- Ethernet (1Gbps)
- SD/MMC card interface

### Interrupt Mapping
- IRQ 0: Timer
- IRQ 1: UART
- IRQ 2: GPIO
- IRQ 3: I2C
- IRQ 4: SPI
- IRQ 5: USB
- IRQ 6: Ethernet
- IRQ 7: SD/MMC

## Performance Considerations

### Cache Optimization
- Align data structures to cache line boundaries
- Use cache prefetching for predictable access patterns
- Minimize cache misses in critical paths

### Vector Operations
- Utilize VSX and AltiVec for data-parallel workloads
- Align vector data to 16-byte boundaries
- Use appropriate vector length for data size

### Power Management
- Implement CPU frequency scaling
- Use appropriate sleep states
- Monitor power consumption

## Debugging

### Debug Features
- Performance monitoring counters (PMC)
- Hardware breakpoints
- Watchpoint support
- Exception handling

### Debug Tools
- GDB with POWER support
- QEMU POWER emulation
- IBM PowerVM for virtualization

## Future Enhancements

- POWER11 support
- Advanced vector extensions
- Enhanced security features
- Improved power management
- Virtualization optimizations

## References

- [POWER ISA v3.1](https://openpowerfoundation.org/specifications/isa/)
- [POWER9 User Manual](https://ibm.ent.box.com/s/2hvhw5vcpn)
- [POWER10 Technical Overview](https://www.ibm.com/downloads/cas/EPYNMXK0X)

## License

MIT License - see LICENSE file for details.

## Authors

Developed by Jeremy Noverraz (1988-2025)
Lausanne, Switzerland
