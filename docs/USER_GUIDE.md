# Orion OS User Guide

> **Installation and User Manual**  
> *Comprehensive guide to install, configure, and use Orion OS*

---

## Getting Started

**Orion OS** is a modern, secure operating system designed for performance and reliability. This guide will help you get Orion OS up and running on your system.

---

## 🔧 **Installation Guide**

### **System Requirements**

#### **Minimum Requirements**
- **Processor**: 64-bit x86_64 processor (Intel/AMD)
- **Memory**: 4GB RAM minimum, 8GB recommended
- **Storage**: 20GB available disk space
- **Graphics**: Any graphics card with framebuffer support
- **Network**: Ethernet or wireless network adapter

#### **Recommended Requirements**
- **Processor**: Multi-core 64-bit processor
- **Memory**: 16GB RAM or more
- **Storage**: 100GB SSD for optimal performance
- **Graphics**: Modern GPU with hardware acceleration
- **Network**: Gigabit Ethernet or WiFi 6

### **Installation Methods**

#### **1. QEMU Virtual Machine (Recommended for Testing)**
```bash
# Install QEMU
sudo apt install qemu-system-x86  # Ubuntu/Debian
brew install qemu                  # macOS
choco install qemu                 # Windows

# Download Orion OS image
wget https://github.com/iyotee/Orion/releases/latest/download/orion-os.img

# Run Orion OS in QEMU
qemu-system-x86_64 \
  -m 4G \
  -smp 4 \
  -drive file=orion-os.img,format=raw \
  -display gtk \
  -enable-kvm
```

#### **2. Bare Metal Installation**
```bash
# Download bootable ISO
wget https://github.com/iyotee/Orion/releases/latest/download/orion-os.iso

# Create bootable USB drive
sudo dd if=orion-os.iso of=/dev/sdX bs=4M status=progress

# Boot from USB drive
# Follow on-screen installation wizard
```

#### **3. Docker Container (Development)**
```dockerfile
FROM ubuntu:22.04
RUN apt update && apt install -y qemu-system-x86
COPY orion-os.img /orion-os.img
CMD ["qemu-system-x86_64", "-m", "4G", "-drive", "file=/orion-os.img,format=raw"]
```

### **Build from Source**

#### **Prerequisites**
```bash
# Install build tools
sudo apt install build-essential cmake git rustc cargo

# Clone repository
git clone https://github.com/iyotee/Orion.git
cd Orion

# Build system
./tools/orion-build.sh
```

---

## 🚀 **First Boot Experience**

### **Boot Process**

#### **1. UEFI Boot**
- **Secure Boot**: Disabled by default for development
- **Boot Order**: UEFI hard disk first
- **Boot Options**: Press F12 for boot menu

#### **2. Kernel Initialization**
- **Hardware Detection**: Automatic device detection
- **Memory Setup**: Physical and virtual memory initialization
- **Driver Loading**: Automatic driver discovery and loading
- **Service Startup**: System services initialization

#### **3. User Interface**
- **Console Mode**: Text-based interface by default
- **Shell Access**: Orion Shell (osh) command line
- **Network Setup**: Automatic network configuration

### **Initial Configuration**

#### **System Setup Wizard**
```bash
# Run initial setup
sudo orion-setup

# Configure network
orion-network --configure

# Set timezone
orion-timezone --set Europe/Zurich

# Create user account
orion-user --add username
```

#### **Network Configuration**
```bash
# View network interfaces
orion-network --list

# Configure WiFi
orion-network --wifi --ssid "NetworkName" --password "Password"

# Configure static IP
orion-network --static --interface eth0 --ip 192.168.1.100 --gateway 192.168.1.1
```

---

## 💻 **User Interface & Shell**

### **Orion Shell (osh)**

#### **Basic Commands**
```bash
# File operations
ls -la                    # List files with details
cp source dest            # Copy files
mv oldname newname        # Rename files
rm filename               # Delete files

# Directory operations
mkdir dirname             # Create directory
cd dirname                # Change directory
pwd                       # Print working directory
rmdir dirname             # Remove directory

# System information
ps aux                    # List processes
top                       # System monitor
df -h                     # Disk usage
free -h                   # Memory usage
```

#### **Advanced Commands**
```bash
# Process management
kill -9 pid              # Force kill process
nice -n 10 command       # Run with priority
renice 10 pid            # Change process priority

# Network tools
ping hostname             # Test connectivity
netstat -tuln            # Network connections
traceroute hostname       # Network path

# System administration
sudo command              # Run as superuser
orion-update              # System update
orion-backup              # System backup
```

### **File Management**

#### **File System Structure**
```
/
├── bin/                  # System binaries
├── boot/                 # Boot files
├── dev/                  # Device files
├── etc/                  # Configuration files
├── home/                 # User home directories
├── lib/                  # System libraries
├── media/                # Removable media
├── mnt/                  # Mount points
├── opt/                  # Optional software
├── proc/                 # Process information
├── root/                 # Root user home
├── run/                  # Runtime data
├── sbin/                 # System binaries
├── srv/                  # Service data
├── tmp/                  # Temporary files
├── usr/                  # User programs
└── var/                  # Variable data
```

#### **File Permissions**
```bash
# View permissions
ls -la filename

# Change permissions
chmod 755 filename        # rwxr-xr-x
chmod +x filename         # Add execute permission
chmod -w filename         # Remove write permission

# Change ownership
chown user:group filename
chown user filename       # Change user only
chgrp group filename      # Change group only
```

---

## 🔐 **Security & User Management**

### **User Accounts**

#### **User Types**
- **Root User**: Superuser with full system access
- **Regular Users**: Limited access users
- **System Users**: Service accounts
- **Guest Users**: Temporary access accounts

#### **User Management**
```bash
# Create user
orion-user --add username --fullname "Full Name"

# Set password
passwd username

# Modify user
orion-user --modify username --shell /bin/bash

# Delete user
orion-user --delete username

# List users
orion-user --list
```

#### **Group Management**
```bash
# Create group
orion-group --add groupname

# Add user to group
orion-group --adduser groupname username

# Remove user from group
orion-group --deluser groupname username

# List groups
orion-group --list
```

### **File Security**

#### **Access Control**
- **User Permissions**: Owner, group, others
- **Capability System**: Fine-grained access control
- **File Attributes**: Immutable, append-only, secure deletion
- **Encryption**: File-level encryption support

#### **Security Best Practices**
```bash
# Secure file permissions
chmod 600 ~/.ssh/id_rsa    # Private key
chmod 644 ~/.ssh/id_rsa.pub # Public key
chmod 700 ~/.ssh            # SSH directory

# Set file attributes
chattr +i filename          # Make immutable
chattr +a filename          # Append only
chattr +s filename          # Secure deletion
```

---

## 🌐 **Network & Connectivity**

### **Network Configuration**

#### **Network Interfaces**
```bash
# View interfaces
orion-network --list

# Enable interface
orion-network --up eth0

# Disable interface
orion-network --down eth0

# Configure interface
orion-network --configure eth0
```

#### **WiFi Configuration**
```bash
# Scan for networks
orion-network --scan

# Connect to network
orion-network --wifi --ssid "NetworkName" --password "Password"

# View connection status
orion-network --status

# Disconnect
orion-network --disconnect
```

#### **Network Services**
```bash
# Start network service
orion-service --start network

# Stop network service
orion-service --stop network

# Restart network service
orion-service --restart network

# View service status
orion-service --status network
```

### **Internet Services**

#### **Web Browser**
```bash
# Launch web browser
orion-browser

# Command line browser
lynx https://example.com
w3m https://example.com
```

#### **Email Client**
```bash
# Launch email client
orion-mail

# Command line email
mutt
pine
```

---

## 🖥️ **Graphics & Display**

### **Display Configuration**

#### **Display Settings**
```bash
# View display info
orion-display --info

# Change resolution
orion-display --resolution 1920x1080

# Change refresh rate
orion-display --refresh 60

# Rotate display
orion-display --rotate 90
```

#### **Multi-Monitor Setup**
```bash
# Detect displays
orion-display --detect

# Configure multi-monitor
orion-display --multi --primary 1 --secondary 2

# Mirror displays
orion-display --mirror

# Extend displays
orion-display --extend
```

### **Graphics Applications**

#### **Image Viewers**
```bash
# Launch image viewer
orion-image

# Command line tools
feh image.jpg              # Image viewer
convert image.jpg image.png # Image conversion
```

#### **Video Players**
```bash
# Launch video player
orion-video

# Command line player
mpv video.mp4
mplayer video.mp4
```

---

## 📦 **Software Management**

### **Package Management**

#### **Orion Package Manager**
```bash
# Update package list
orion-update

# Install package
orion-install package-name

# Remove package
orion-remove package-name

# Search packages
orion-search keyword

# List installed packages
orion-list --installed
```

#### **Software Repositories**
```bash
# Add repository
orion-repo --add name url

# Remove repository
orion-repo --remove name

# List repositories
orion-repo --list

# Update repositories
orion-repo --update
```

### **Development Tools**

#### **Programming Languages**
```bash
# C/C++ development
orion-install gcc g++ make cmake

# Python development
orion-install python3 pip3

# Rust development
orion-install rustc cargo

# Node.js development
orion-install nodejs npm
```

#### **Development IDEs**
```bash
# Text editors
orion-install vim emacs nano

# IDEs
orion-install vscode eclipse

# Version control
orion-install git svn
```

---

## 🔧 **System Administration**

### **System Monitoring**

#### **Performance Monitoring**
```bash
# System monitor
top
htop
iotop

# Resource usage
free -h                   # Memory
df -h                     # Disk
iostat                    # I/O statistics
```

#### **Process Management**
```bash
# View processes
ps aux
ps -ef

# Kill processes
kill pid
killall process-name
pkill pattern

# Process priority
nice -n 10 command
renice 10 pid
```

### **System Maintenance**

#### **Updates & Upgrades**
```bash
# Check for updates
orion-update --check

# Install updates
orion-update --install

# System upgrade
orion-upgrade

# Kernel update
orion-kernel --update
```

#### **Backup & Recovery**
```bash
# Create backup
orion-backup --create --full

# Restore backup
orion-backup --restore backup-file

# Schedule backups
orion-backup --schedule --daily

# List backups
orion-backup --list
```

---

## 🚨 **Troubleshooting**

### **Common Issues**

#### **Boot Problems**
```bash
# Boot in safe mode
orion-boot --safe

# Boot with verbose output
orion-boot --verbose

# Boot from previous kernel
orion-boot --previous

# Reset boot configuration
orion-boot --reset
```

#### **Network Issues**
```bash
# Reset network configuration
orion-network --reset

# Test connectivity
ping 8.8.8.8
nslookup google.com

# Check network logs
orion-log --network
```

#### **Performance Issues**
```bash
# Check system load
uptime
top

# Check memory usage
free -h
vmstat

# Check disk I/O
iostat
iotop
```

### **Getting Help**

#### **System Logs**
```bash
# View system logs
orion-log --system

# View error logs
orion-log --error

# View security logs
orion-log --security

# View application logs
orion-log --application
```

#### **Support Resources**
- **Documentation**: `/usr/share/doc/orion-os/`
- **Online Help**: [GitHub Wiki](https://github.com/iyotee/Orion/wiki)
- **Community**: [GitHub Discussions](https://github.com/iyotee/Orion/discussions)
- **Issues**: [GitHub Issues](https://github.com/iyotee/Orion/issues)

---

## 📚 **Advanced Usage**

### **Command Line Mastery**

#### **Shell Scripting**
```bash
#!/bin/bash
# Simple backup script
BACKUP_DIR="/backup/$(date +%Y%m%d)"
mkdir -p "$BACKUP_DIR"
cp -r /home "$BACKUP_DIR/"
echo "Backup completed: $BACKUP_DIR"
```

#### **Automation**
```bash
# Cron jobs
crontab -e

# Example: Daily backup at 2 AM
0 2 * * * /usr/local/bin/backup.sh

# Example: Weekly system update
0 3 * * 0 orion-update
```

### **Customization**

#### **Shell Configuration**
```bash
# Edit shell configuration
nano ~/.bashrc

# Add custom aliases
alias ll='ls -la'
alias update='orion-update'
alias backup='orion-backup --create'

# Reload configuration
source ~/.bashrc
```

#### **System Configuration**
```bash
# Edit system configuration
sudo nano /etc/orion/orion.conf

# Customize boot parameters
sudo nano /etc/orion/boot.conf

# Configure services
sudo nano /etc/orion/services.conf
```

---

## 📞 **Support and Contact**

### **Getting Help**
- **Documentation**: This guide and online resources
- **Community**: GitHub Discussions and Issues
- **Email**: [jeremy.noverraz@proton.me](mailto:jeremy.noverraz@proton.me)
- **Location**: Lausanne, Switzerland

### **Contributing**
- **Report Bugs**: Use GitHub Issues
- **Suggest Features**: Use GitHub Discussions
- **Submit Code**: Use GitHub Pull Requests
- **Improve Documentation**: Edit documentation files


