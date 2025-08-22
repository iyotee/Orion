# Installation Guide

> **Complete installation guide for ORION OS across all supported architectures**

---

## 📋 **Prerequisites**

### **System Requirements**
- **Minimum RAM**: 2GB (4GB recommended)
- **Storage**: 10GB available space
- **Architecture**: x86_64, aarch64, riscv64 (others in development)
- **Boot Method**: UEFI (recommended) or Legacy BIOS

### **Required Tools**
- **Git**: For cloning the repository
- **Build Tools**: GCC/Clang, Make, CMake
- **Cross-compilation tools** (for target architectures)
- **QEMU**: For testing and development

---

## 🚀 **Quick Installation**

### **1. Clone the Repository**
```bash
git clone https://github.com/iyotee/Orion.git
cd Orion
```

### **2. Build the System**
```bash
# Build for current architecture
make build

# Build for specific architecture
make build ARCH=x86_64
make build ARCH=aarch64
make build ARCH=riscv64
```

### **3. Install to Disk**
```bash
# Interactive installation
make install

# Automated installation with configuration
make install CONFIG=minimal
make install CONFIG=desktop
make install CONFIG=server
```

---

## 🏗️ **Detailed Build Process**

### **Kernel Compilation**
```bash
# Navigate to kernel directory
cd kernel

# Configure build
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build kernel
cmake --build build --parallel

# Install kernel
sudo cmake --install build
```

### **Driver Framework**
```bash
# Navigate to drivers directory
cd drivers

# Build Rust drivers
cargo build --release

# Install drivers
cargo install --path .
```

### **System Tools**
```bash
# Build system utilities
cd tools
make all

# Install tools
sudo make install
```

---

## 🎯 **Architecture-Specific Installation**

### **x86_64 (Intel/AMD)**
```bash
# Optimized for Intel
make build ARCH=x86_64 CPU=intel

# Optimized for AMD
make build ARCH=x86_64 CPU=amd

# Generic x86_64
make build ARCH=x86_64 CPU=generic
```

### **aarch64 (ARM)**
```bash
# Apple Silicon optimization
make build ARCH=aarch64 CPU=apple

# ARM Cortex optimization
make build ARCH=aarch64 CPU=cortex

# Generic ARM64
make build ARCH=aarch64 CPU=generic
```

### **RISC-V**
```bash
# RV64GC with vector extensions
make build ARCH=riscv64 CPU=rv64gc

# Custom RISC-V configuration
make build ARCH=riscv64 CPU=custom
```

---

## 🔧 **Configuration Options**

### **Build Configurations**
```bash
# Minimal system (kernel + basic tools)
make build CONFIG=minimal

# Desktop system (GUI + applications)
make build CONFIG=desktop

# Server system (services + networking)
make build CONFIG=server

# Development system (tools + debugging)
make build CONFIG=dev
```

### **Security Options**
```bash
# Enable formal verification
make build SECURITY=formal

# Enable hardware capabilities
make build SECURITY=capabilities

# Enable post-quantum crypto
make build SECURITY=post-quantum
```

### **Performance Options**
```bash
# Optimize for performance
make build PERFORMANCE=high

# Optimize for size
make build PERFORMANCE=size

# Balanced optimization
make build PERFORMANCE=balanced
```

---

## 🐳 **Container Installation**

### **Docker Installation**
```bash
# Pull ORION OS container
docker pull orionos/orion:latest

# Run ORION OS container
docker run -it --privileged orionos/orion:latest

# Build custom container
docker build -t orionos/custom .
```

### **Podman Installation**
```bash
# Pull with Podman
podman pull orionos/orion:latest

# Run with Podman
podman run -it --privileged orionos/orion:latest
```

---

## 🔍 **Verification and Testing**

### **System Verification**
```bash
# Verify kernel installation
orion-verify kernel

# Verify driver installation
orion-verify drivers

# Verify system integrity
orion-verify system
```

### **Performance Testing**
```bash
# Run performance benchmarks
orion-benchmark all

# Compare with Linux
orion-benchmark compare-linux

# Architecture-specific tests
orion-benchmark arch=x86_64
```

### **Security Testing**
```bash
# Run security tests
orion-security-test all

# Verify capabilities
orion-security-test capabilities

# Test formal properties
orion-security-test formal
```

---

## 🚨 **Troubleshooting**

### **Common Issues**

#### **Build Failures**
```bash
# Clean build directory
make clean

# Update dependencies
make deps-update

# Check system requirements
make check-requirements
```

#### **Installation Errors**
```bash
# Check disk space
df -h

# Verify permissions
ls -la /usr/local/

# Check system compatibility
orion-check compatibility
```

#### **Boot Issues**
```bash
# Verify bootloader
orion-boot verify

# Check kernel parameters
orion-boot params

# Reset boot configuration
orion-boot reset
```

### **Getting Help**
- **Documentation**: Check this Wiki
- **Issues**: [GitHub Issues](https://github.com/iyotee/Orion/issues)
- **Discussions**: [GitHub Discussions](https://github.com/iyotee/Orion/discussions)
- **Community**: Join our development community

---

## 📚 **Next Steps**

After successful installation:

1. **[Quick Start Guide](Quick-Start)** - Get familiar with ORION OS
2. **[User Guide](User-Guide)** - Learn system usage
3. **[Development Guide](Development-Guide)** - Start contributing
4. **[Performance Tuning](Performance-Tuning)** - Optimize your system

---

## 🔄 **Updates and Maintenance**

### **System Updates**
```bash
# Update system packages
orion-update system

# Update kernel
orion-update kernel

# Update drivers
orion-update drivers
```

### **Rollback Options**
```bash
# List available versions
orion-rollback list

# Rollback to previous version
orion-rollback previous

# Rollback to specific version
orion-rollback version 1.2.3
```

---

*Last updated: December 2024*
