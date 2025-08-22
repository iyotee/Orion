# Quick Start Guide

> **Get up and running with ORION OS in minutes**

---

## ⚡ **5-Minute Setup**

### **1. System Check**
```bash
# Check system compatibility
orion-check system

# Verify architecture support
orion-check architecture

# Check available resources
orion-check resources
```

### **2. Quick Build**
```bash
# Clone and build
git clone https://github.com/iyotee/Orion.git
cd Orion
make quick-build
```

### **3. Test in QEMU**
```bash
# Run in emulator
make run-qemu

# Or with specific architecture
make run-qemu ARCH=x86_64
make run-qemu ARCH=aarch64
```

---

## 🚀 **First Boot Experience**

### **Welcome Screen**
- **Language Selection**: Choose your preferred language
- **Architecture Detection**: Automatic hardware detection
- **Performance Profile**: Select optimization level

### **Initial Configuration**
```bash
# Set system hostname
orion-config hostname my-orion-system

# Configure network
orion-config network auto

# Set timezone
orion-config timezone Europe/Paris
```

### **User Account Setup**
```bash
# Create admin user
orion-user create admin --admin

# Set password
orion-user password admin

# Create regular user
orion-user create user1
```

---

## 🖥️ **Basic System Usage**

### **Command Line Interface**
```bash
# System information
orion-info system
orion-info architecture
orion-info performance

# Process management
orion-ps
orion-top
orion-kill <pid>

# File operations
orion-ls
orion-cp <src> <dst>
orion-mv <src> <dst>
```

### **Package Management**
```bash
# Install packages
orion-pkg install <package>

# Update system
orion-pkg update
orion-pkg upgrade

# Search packages
orion-pkg search <query>

# Remove packages
orion-pkg remove <package>
```

### **System Services**
```bash
# Start service
orion-service start <service>

# Stop service
orion-service stop <service>

# Check status
orion-service status <service>

# Enable at boot
orion-service enable <service>
```

---

## 🔧 **Essential Configuration**

### **Performance Tuning**
```bash
# Auto-optimize for current hardware
orion-optimize auto

# Manual optimization
orion-optimize cpu
orion-optimize memory
orion-optimize storage

# Check optimization results
orion-optimize status
```

### **Security Setup**
```bash
# Enable security features
orion-security enable capabilities
orion-security enable firewall
orion-security enable encryption

# Check security status
orion-security status
orion-security audit
```

### **Network Configuration**
```bash
# Configure network interface
orion-network configure eth0

# Set up wireless
orion-network wifi-scan
orion-network wifi-connect <ssid>

# Check connectivity
orion-network test
```

---

## 📱 **Desktop Environment**

### **GUI Setup**
```bash
# Install desktop environment
orion-pkg install desktop-minimal
orion-pkg install desktop-full

# Start GUI
orion-gui start

# Configure display
orion-gui configure
```

### **Application Installation**
```bash
# Install common applications
orion-pkg install browser
orion-pkg install office
orion-pkg install media

# Install development tools
orion-pkg install dev-tools
orion-pkg install ide
```

---

## 🐳 **Container and Virtualization**

### **Docker Support**
```bash
# Install Docker
orion-pkg install docker

# Run containers
docker run -it ubuntu:latest
docker run -d nginx:latest

# Manage containers
docker ps
docker images
```

### **Virtual Machines**
```bash
# Install QEMU
orion-pkg install qemu

# Create VM
orion-vm create ubuntu-vm --os ubuntu --ram 4G

# Start VM
orion-vm start ubuntu-vm

# Manage VMs
orion-vm list
orion-vm stop ubuntu-vm
```

---

## 🔍 **Monitoring and Debugging**

### **System Monitoring**
```bash
# Real-time monitoring
orion-monitor system
orion-monitor performance
orion-monitor network

# Log analysis
orion-log system
orion-log security
orion-log performance
```

### **Performance Analysis**
```bash
# Run benchmarks
orion-benchmark cpu
orion-benchmark memory
orion-benchmark storage

# Compare with Linux
orion-benchmark compare-linux
```

### **Troubleshooting**
```bash
# System diagnostics
orion-diagnose system
orion-diagnose network
orion-diagnose performance

# Generate report
orion-diagnose report
```

---

## 📚 **Learning Resources**

### **Built-in Help**
```bash
# Command help
orion-help <command>

# System manual
orion-manual

# Tutorials
orion-tutorial basic
orion-tutorial advanced
```

### **Online Resources**
- **[User Guide](User-Guide)** - Complete user manual
- **[API Reference](API-Reference)** - System interfaces
- **[Development Guide](Development-Guide)** - Contributing to ORION OS
- **[Community Forum](https://github.com/iyotee/Orion/discussions)** - Get help and share

---

## 🎯 **Next Steps**

### **For End Users**
1. **Explore the system** with built-in tools
2. **Install applications** you need
3. **Customize your environment** for productivity
4. **Join the community** for support and tips

### **For Developers**
1. **Study the architecture** with technical docs
2. **Experiment with APIs** and system calls
3. **Contribute code** to improve ORION OS
4. **Report bugs** and suggest features

### **For System Administrators**
1. **Configure services** for your environment
2. **Set up monitoring** and alerting
3. **Implement security policies** and access control
4. **Plan deployment** strategies

---

## 🆘 **Getting Help**

### **Immediate Support**
```bash
# Built-in help system
orion-help

# System diagnostics
orion-diagnose

# Community support
orion-support
```

### **Community Resources**
- **GitHub Issues**: [Report bugs](https://github.com/iyotee/Orion/issues)
- **GitHub Discussions**: [Ask questions](https://github.com/iyotee/Orion/discussions)
- **Documentation**: [Complete guides](https://github.com/iyotee/Orion/wiki)
- **Code Examples**: [Sample applications](https://github.com/iyotee/Orion/examples)

---

*Welcome to ORION OS - The Universal Operating System!*
*Last updated: December 2024*
