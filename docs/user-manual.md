# Orion OS User Manual

> **Complete User Guide for Orion Operating System**  
> *Modern Operating System - User Documentation*

## 🚀 Getting Started

### 💿 Installation

#### **Requirements**
- **CPU**: x86_64 with UEFI support
- **Memory**: Minimum 4GB RAM, recommended 8GB+
- **Storage**: 500MB free space for installation
- **Boot**: UEFI-capable system (Legacy BIOS not supported)

#### **Quick Installation**

1. **Download** the latest Orion OS ISO from releases
2. **Create bootable media**:
   ```bash
   # Linux/macOS
   dd if=orion-os.iso of=/dev/sdX bs=4M status=progress
   
   # Windows (use Rufus or similar tool)
   ```
3. **Boot from USB/DVD** and follow the installation wizard
4. **Configure** your system during first boot

### 🎬 First Boot

When you first boot Orion OS, you'll see:

```
   ╔══════════════════════════════════════════╗
   ║                                          ║
   ║         🌟 ORION OPERATING SYSTEM        ║
   ║                                          ║
   ║      Modern • Secure • High-Performance  ║
   ║                                          ║
   ║         Press Enter to continue          ║
   ║                                          ║
   ╚══════════════════════════════════════════╝
```

## 🖥️ Desktop Environment

### 🏠 Orion Shell

Orion OS features a modern shell environment that combines the power of a command-line interface with graphical elements.

#### **Shell Prompt**
```bash
orion@hostname:~$ 
```

#### **Basic Navigation**
```bash
# List files and directories
ls                    # Basic listing
ls -la               # Detailed listing with hidden files
ls --color           # Colored output

# Change directory
cd /path/to/directory # Change to specific directory
cd ..                # Go up one directory
cd ~                 # Go to home directory
cd -                 # Go to previous directory

# Show current directory
pwd

# Create directories
mkdir my-folder
mkdir -p path/to/nested/folders

# Remove files and directories
rm file.txt          # Remove file
rm -r folder/        # Remove directory recursively
rmdir empty-folder   # Remove empty directory
```

#### **File Operations**
```bash
# Copy files
cp source.txt destination.txt
cp -r source-folder/ destination-folder/

# Move/rename files
mv old-name.txt new-name.txt
mv file.txt /path/to/new/location/

# Create files
touch new-file.txt
echo "Hello World" > hello.txt

# View file contents
cat file.txt         # Display entire file
less file.txt        # View file with pagination
head file.txt        # Show first 10 lines
tail file.txt        # Show last 10 lines
tail -f logfile      # Follow file updates
```

### 📊 System Information

```bash
# System information
orion-info           # Complete system overview
orion-info --cpu     # CPU information
orion-info --memory  # Memory usage
orion-info --storage # Storage devices

# Process management
ps                   # List running processes
ps -aux             # Detailed process list
top                 # Interactive process monitor
htop                # Enhanced process monitor (if available)

# System resources
free -h             # Memory usage (human-readable)
df -h               # Disk usage
uptime              # System uptime and load
```

### 🌐 Network Management

```bash
# Network interfaces
ifconfig            # Show network interfaces
ip addr show        # Modern alternative to ifconfig

# Network connectivity
ping hostname       # Test connectivity
traceroute hostname # Trace network path
netstat -tuln      # Show listening ports

# WiFi management (if available)
wifi scan           # Scan for networks
wifi connect SSID   # Connect to network
wifi status         # Connection status
```

## 🔧 System Administration

### 👤 User Management

```bash
# User operations
whoami              # Current username
id                  # User and group IDs
groups              # Show user groups

# Process control
kill PID            # Terminate process by ID
killall process     # Terminate by process name
jobs                # Show background jobs
fg %1               # Bring job to foreground
bg %1               # Send job to background
```

### 🔐 Security

#### **Capability System**

Orion OS uses a modern capability-based security model instead of traditional permissions:

```bash
# List your capabilities
cap-list

# Check capability for a resource
cap-check CAP_FILE_READ /path/to/file

# Grant capability (if you have CAP_GRANT)
cap-grant USER CAP_NET_BIND_SERVICE

# Revoke capability
cap-revoke USER CAP_FILE_WRITE
```

#### **Security Levels**
- **PUBLIC**: No special clearance required
- **CONFIDENTIAL**: Basic security clearance  
- **SECRET**: Advanced security clearance
- **TOP_SECRET**: Highest security clearance

### 📦 Package Management

```bash
# Search for packages
pkg search package-name

# Install packages
pkg install package-name

# Update system
pkg update          # Update package database
pkg upgrade         # Upgrade all packages

# Remove packages
pkg remove package-name

# List installed packages
pkg list
```

## 🛠️ Development Environment

### 🔨 Compiling Code

Orion OS includes modern development tools:

```bash
# C/C++ development
clang hello.c -o hello        # Compile C program
clang++ hello.cpp -o hello    # Compile C++ program

# Rust development
cargo new my-project          # Create new Rust project
cargo build                   # Build project
cargo run                     # Build and run

# Python development
python3 script.py            # Run Python script
pip3 install package         # Install Python package

# Assembly development
nasm -f elf64 hello.asm      # Assemble x86_64 code
ld hello.o -o hello          # Link object file
```

### 🧪 Debugging

```bash
# Debuggers
gdb program          # GNU Debugger
lldb program         # LLVM Debugger

# System tracing
strace program       # Trace system calls
ltrace program       # Trace library calls

# Performance profiling
perf record program  # Record performance data
perf report          # Analyze performance data
```

## 🎮 Entertainment

### 🎵 Multimedia

```bash
# Audio (when audio drivers are available)
alsamixer           # Audio mixer
aplay sound.wav     # Play audio file

# Graphics
display image.png   # Display image
```

### 🎯 Games

```bash
# Terminal games
tetris              # Classic Tetris
snake               # Snake game
chess               # Chess game (if available)
```

## ⚙️ Configuration

### 🔧 System Configuration

Configuration files are located in `/etc/`:

```bash
# Network configuration
/etc/network/interfaces     # Network interface config
/etc/resolv.conf           # DNS configuration

# System configuration
/etc/orion/kernel.conf     # Kernel parameters
/etc/orion/security.conf   # Security policies
/etc/orion/drivers.conf    # Driver configuration
```

### 👥 User Configuration

User-specific configuration in `~/.config/`:

```bash
# Shell configuration
~/.bashrc              # Bash shell configuration
~/.profile             # Login shell configuration

# Application configuration
~/.config/orion/       # Orion-specific user settings
```

## 🆘 Troubleshooting

### 🐛 Common Issues

#### **System Won't Boot**
1. Check UEFI settings are correct
2. Verify boot media integrity
3. Check hardware compatibility
4. Try safe mode: press F8 during boot

#### **Network Not Working**
1. Check cable connection
2. Verify driver loading: `lsmod | grep net`
3. Check interface status: `ifconfig`
4. Restart network service: `systemctl restart networking`

#### **Performance Issues**
1. Check system resources: `top`
2. Check disk space: `df -h`
3. Monitor memory usage: `free -h`
4. Check for background processes

#### **Graphics Issues**
1. Check display driver: `lsmod | grep gpu`
2. Verify resolution settings
3. Try different graphics modes
4. Check cable connections

### 📋 Log Files

System logs are located in `/var/log/`:

```bash
# System logs
tail -f /var/log/kernel.log    # Kernel messages
tail -f /var/log/system.log    # System events
tail -f /var/log/security.log  # Security events
tail -f /var/log/driver.log    # Driver messages

# Application logs
tail -f /var/log/application.log
```

### 🔍 Diagnostic Tools

```bash
# Hardware information
lshw                # List hardware
lscpu               # CPU information
lsmem               # Memory information
lsblk               # Block devices
lsusb               # USB devices
lspci               # PCI devices

# System diagnostics
dmesg               # Kernel ring buffer
journalctl          # System journal
systemctl status    # Service status
```

## 📞 Getting Help

### 💬 Built-in Help

```bash
# Command help
command --help      # Show command help
man command         # Manual page
info command        # Info documentation

# System help
orion-help          # Orion-specific help
orion-help topics   # List help topics
orion-help network  # Network help
```

### 🌐 Online Resources

- **Official Website**: https://orion-os.dev
- **Documentation**: https://docs.orion-os.dev
- **Forums**: https://forum.orion-os.dev
- **Bug Reports**: https://github.com/orion-os/orion/issues
- **Discord**: https://discord.gg/orion-os

### 📧 Support

- **General Support**: support@orion-os.dev
- **Security Issues**: security@orion-os.dev
- **Development**: dev@orion-os.dev

## 🎯 Tips and Tricks

### ⚡ Productivity Tips

1. **Use tab completion**: Press Tab to auto-complete commands and paths
2. **Command history**: Use ↑/↓ arrows to navigate command history
3. **Search history**: Press Ctrl+R to search command history
4. **Multiple terminals**: Open multiple terminal windows with Ctrl+Alt+T
5. **Copy/paste**: Use Ctrl+Shift+C/V in terminal

### 🔥 Advanced Features

```bash
# Process substitution
diff <(ls dir1) <(ls dir2)

# Command pipelines
ps aux | grep python | awk '{print $2}' | xargs kill

# Background jobs
long-running-command &
nohup command > output.log 2>&1 &

# Environment variables
export MY_VAR="value"
echo $MY_VAR
```

### 🎨 Customization

```bash
# Aliases
alias ll='ls -la'
alias grep='grep --color=auto'
alias ..='cd ..'

# Custom prompt
export PS1='\u@\h:\w\$ '

# Functions
function mkcd() {
    mkdir -p "$1" && cd "$1"
}
```

## 📖 Keyboard Shortcuts

### ⌨️ Terminal Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+C` | Interrupt current command |
| `Ctrl+Z` | Suspend current command |
| `Ctrl+D` | Exit shell/logout |
| `Ctrl+L` | Clear screen |
| `Ctrl+A` | Move to beginning of line |
| `Ctrl+E` | Move to end of line |
| `Ctrl+U` | Delete from cursor to beginning |
| `Ctrl+K` | Delete from cursor to end |
| `Ctrl+R` | Search command history |
| `Alt+.` | Insert last argument |

### 🖥️ System Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+Alt+T` | Open terminal |
| `Alt+Tab` | Switch between applications |
| `Ctrl+Alt+Del` | System manager |
| `Ctrl+Alt+F1-F6` | Switch to virtual console |
| `Ctrl+Alt+F7` | Return to GUI |

---

This manual covers the essential aspects of using Orion OS. For advanced topics and development information, refer to the technical documentation and developer guides.
