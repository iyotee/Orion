# Driver Development Guide

> **Comprehensive Guide for Developing Orion OS Drivers**  
> *Rust-based Userland Driver Framework*

## 🎯 Overview

Orion OS uses a **revolutionary approach** to device drivers: all drivers run in **userland** and are written in **Rust** with `#![no_std]` for maximum safety and performance. This architecture provides superior security, stability, and development experience compared to traditional kernel-mode drivers.

## 🏗️ Architecture

### 📊 Driver Framework Stack

```
┌─────────────────────────────────────────────────────────────┐
│                    USER SPACE                              │
├─────────────────────────────────────────────────────────────┤
│  Your Driver Binary                                        │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  impl NetworkDriver for MyDriver {                 │   │
│  │      fn send_packet(&mut self, data: &[u8]) { ... } │   │  
│  │      fn recv_packet(&mut self, buf: &mut [u8]) {...}│   │
│  │  }                                                  │   │
│  └─────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│  orion-driver Framework                                     │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  • Trait definitions (NetworkDriver, BlockDriver)  │   │
│  │  • MMIO safe abstractions                          │   │
│  │  • IPC communication with kernel                   │   │
│  │  • Device enumeration and probing                  │   │
│  │  • Error handling and logging                      │   │
│  └─────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│                    KERNEL SPACE                            │
├─────────────────────────────────────────────────────────────┤
│  Driver Manager                                             │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  • Device discovery and enumeration                │   │
│  │  • Driver loading and lifecycle management         │   │
│  │  • IPC routing and message passing                 │   │
│  │  • Hardware resource allocation                    │   │
│  │  • Security policy enforcement                     │   │
│  └─────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│                       HARDWARE                             │
└─────────────────────────────────────────────────────────────┘
```

## 🚀 Quick Start

### 📋 Prerequisites

```bash
# Install Rust with no_std target
rustup target add x86_64-unknown-none

# Install development tools
cargo install cargo-binutils
rustup component add llvm-tools-preview
```

### 🎬 Creating Your First Driver

1. **Generate driver project**:

```bash
cd drivers/
mkdir my-device && cd my-device
cargo init --name my-device-driver
```

2. **Configure Cargo.toml**:

```toml
[package]
name = "my-device-driver"
version = "1.0.0"
edition = "2021"
authors = ["Your Name <your.email@example.com>"]

[dependencies]
orion-driver = { path = "../../lib/orion-driver" }
bitflags = "2.0"

[[bin]]
name = "my-device"
path = "src/main.rs"
```

3. **Implement the driver**:

```rust
#![no_std]
#![no_main]

use orion_driver::{
    NetworkDriver, DeviceInfo, DriverError, DriverInfo, DriverResult, OrionDriver,
    LinkStatus, NetworkStats,
};

pub struct MyDeviceDriver {
    device: DeviceInfo,
    // Your driver state here
}

impl OrionDriver for MyDeviceDriver {
    fn probe(device: &DeviceInfo) -> DriverResult<bool> {
        // Check if this driver can handle the device
        Ok(device.vendor_id == 0x1234 && device.device_id == 0x5678)
    }
    
    fn init(device: DeviceInfo) -> DriverResult<Self> {
        // Initialize your device
        Ok(MyDeviceDriver { device })
    }
    
    fn handle_irq(&mut self) -> DriverResult<()> {
        // Handle device interrupts
        Ok(())
    }
    
    fn shutdown(&mut self) -> DriverResult<()> {
        // Clean shutdown
        Ok(())
    }
    
    fn info(&self) -> DriverInfo {
        DriverInfo {
            name: "My Device Driver",
            version: "1.0.0", 
            author: "Your Name",
            description: "Driver for my custom device",
        }
    }
}

impl NetworkDriver for MyDeviceDriver {
    // Implement network-specific methods
    fn send_packet(&mut self, packet: &[u8]) -> DriverResult<()> {
        // Send network packet
        Ok(())
    }
    
    fn recv_packet(&mut self, buffer: &mut [u8]) -> DriverResult<Option<usize>> {
        // Receive network packet
        Ok(None) // No packet available
    }
    
    fn mac_address(&self) -> [u8; 6] {
        [0x00, 0x11, 0x22, 0x33, 0x44, 0x55]
    }
    
    fn set_promiscuous(&mut self, enabled: bool) -> DriverResult<()> {
        // Enable/disable promiscuous mode
        Ok(())
    }
    
    fn link_status(&self) -> LinkStatus {
        LinkStatus::Up { speed_mbps: 1000, duplex: true }
    }
    
    fn statistics(&self) -> NetworkStats {
        NetworkStats::default()
    }
}

// Driver entry point and panic handler
include!("../../../lib/orion-driver/src/driver_main_template.rs");
```

## 🔧 Driver Types

### 🌐 Network Drivers

Implement the `NetworkDriver` trait for network interfaces:

```rust
pub trait NetworkDriver: OrionDriver {
    /// Send a network packet
    fn send_packet(&mut self, packet: &[u8]) -> DriverResult<()>;
    
    /// Receive a packet (non-blocking)  
    fn recv_packet(&mut self, buffer: &mut [u8]) -> DriverResult<Option<usize>>;
    
    /// Get MAC address
    fn mac_address(&self) -> [u8; 6];
    
    /// Set promiscuous mode
    fn set_promiscuous(&mut self, enabled: bool) -> DriverResult<()>;
    
    /// Get link status
    fn link_status(&self) -> LinkStatus;
    
    /// Get network statistics
    fn statistics(&self) -> NetworkStats;
}
```

**Example implementations:**
- **VirtIO-net**: High-performance virtualized networking
- **Intel e1000**: Enterprise-grade Ethernet controllers  
- **Realtek RTL8139**: Legacy Fast Ethernet support

### 💾 Block Drivers

Implement the `BlockDriver` trait for storage devices:

```rust
pub trait BlockDriver: OrionDriver {
    /// Read blocks from device
    fn read_blocks(&mut self, lba: u64, count: u32, buffer: &mut [u8]) -> DriverResult<()>;
    
    /// Write blocks to device
    fn write_blocks(&mut self, lba: u64, count: u32, buffer: &[u8]) -> DriverResult<()>;
    
    /// Get device size in blocks
    fn block_count(&self) -> u64;
    
    /// Get block size in bytes
    fn block_size(&self) -> u32;
    
    /// Flush write cache
    fn flush(&mut self) -> DriverResult<()>;
    
    /// Get device capabilities
    fn capabilities(&self) -> BlockCapabilities;
}
```

**Example implementations:**
- **VirtIO-block**: High-performance virtualized storage
- **NVMe**: Modern SSD controllers
- **AHCI**: SATA hard drives and SSDs

### 🎮 HID Drivers

Implement the `UsbDriver` trait for USB Human Interface Devices:

```rust
pub trait UsbDriver: OrionDriver {
    /// Send control transfer
    fn control_transfer(&mut self, setup: &UsbSetupPacket, data: &mut [u8]) -> DriverResult<usize>;
    
    /// Bulk transfer
    fn bulk_transfer(&mut self, endpoint: u8, data: &mut [u8]) -> DriverResult<usize>;
    
    /// Interrupt transfer
    fn interrupt_transfer(&mut self, endpoint: u8, data: &mut [u8]) -> DriverResult<usize>;
    
    /// Get device descriptor
    fn device_descriptor(&self) -> UsbDeviceDescriptor;
}
```

**Example implementations:**
- **USB HID**: Keyboards, mice, gamepads
- **USB Mass Storage**: Flash drives, external hard drives

### 🎨 Graphics Drivers

Implement the `GpuDriver` trait for graphics devices:

```rust
pub trait GpuDriver: OrionDriver {
    /// Initialize graphics mode
    fn init_graphics(&mut self, width: u32, height: u32, bpp: u8) -> DriverResult<()>;
    
    /// Get framebuffer information
    fn framebuffer_info(&self) -> FramebufferInfo;
    
    /// Set pixel at coordinates
    fn set_pixel(&mut self, x: u32, y: u32, color: u32) -> DriverResult<()>;
    
    /// Copy buffer to framebuffer  
    fn copy_buffer(&mut self, buffer: &[u8]) -> DriverResult<()>;
    
    /// Clear screen with color
    fn clear_screen(&mut self, color: u32) -> DriverResult<()>;
}
```

**Example implementations:**
- **Framebuffer**: Universal linear framebuffer support
- **VirtIO-GPU**: Modern virtualized graphics with 3D support

## 🛠️ Hardware Access

### 💾 Memory-Mapped I/O (MMIO)

Safe MMIO access using the `MmioAccessor`:

```rust
use orion_driver::{MmioAccessor, MmioPermissions};

// Create MMIO accessor for device registers
let mmio = unsafe {
    MmioAccessor::new(
        device.bars[0],    // Physical address from BAR
        4096,             // Size of MMIO region
        MmioPermissions::READ | MmioPermissions::WRITE | MmioPermissions::UNCACHED
    )
};

// Read/write device registers safely
let status = mmio.read_u32(STATUS_REGISTER_OFFSET)?;
mmio.write_u32(COMMAND_REGISTER_OFFSET, COMMAND_ENABLE)?;

// Use typed register accessors for convenience
let control_reg = mmio_register!(mmio, CONTROL_OFFSET, u32);
control_reg.set_bits(ENABLE_BIT | START_BIT)?;
```

### ⚡ Interrupt Handling

Handle device interrupts through the driver framework:

```rust
impl OrionDriver for MyDriver {
    fn handle_irq(&mut self) -> DriverResult<()> {
        // Read interrupt status register
        let status = self.mmio.read_u32(IRQ_STATUS_OFFSET)?;
        
        if status & IRQ_RX_COMPLETE != 0 {
            self.handle_rx_completion()?;
        }
        
        if status & IRQ_TX_COMPLETE != 0 {
            self.handle_tx_completion()?;
        }
        
        // Clear handled interrupts
        self.mmio.write_u32(IRQ_STATUS_OFFSET, status)?;
        
        Ok(())
    }
}
```

### 🔄 DMA and Buffer Management

Manage DMA buffers safely:

```rust
// Allocate DMA-capable buffer
let dma_buffer = DmaBuffer::new(4096)?;  // 4KB buffer

// Get physical address for device
let phys_addr = dma_buffer.physical_address();

// Program device with physical address
self.mmio.write_u64(DMA_BUFFER_ADDR_REG, phys_addr)?;
self.mmio.write_u32(DMA_BUFFER_SIZE_REG, 4096)?;

// Start DMA operation
self.mmio.write_u32(DMA_CONTROL_REG, DMA_START)?;
```

## 📡 Communication with Kernel

### 💬 IPC Message Handling

Drivers communicate with the kernel through structured IPC:

```rust
use orion_driver::{MessageLoop, ReceivedMessage, IoRequestType};

fn driver_main() {
    let mut message_loop = MessageLoop::new().unwrap();
    
    message_loop.run(
        "my-driver",
        "1.0.0", 
        &[VENDOR_ID],
        &[DEVICE_ID],
        |ipc, message| {
            match message {
                ReceivedMessage::ProbeDevice(probe_msg) => {
                    let can_handle = MyDriver::probe(&DeviceInfo::new(
                        probe_msg.vendor_id,
                        probe_msg.device_id, 
                        BusType::Pci
                    ))?;
                    ipc.send_probe_response(probe_msg.header.sequence, can_handle)
                }
                
                ReceivedMessage::IoRequest(io_msg) => {
                    let result = match io_msg.request_type {
                        IoRequestType::Read => self.handle_read(io_msg),
                        IoRequestType::Write => self.handle_write(io_msg),
                        IoRequestType::Ioctl => self.handle_ioctl(io_msg),
                        _ => Err(DriverError::Unsupported),
                    };
                    ipc.send_io_response(io_msg.header.sequence, result)
                }
                
                ReceivedMessage::Interrupt(device_handle) => {
                    self.handle_irq()?;
                    Ok(())
                }
                
                _ => Ok(()),
            }
        }
    ).unwrap();
}
```

## 🧪 Testing and Debugging

### ✅ Unit Testing

Write comprehensive unit tests for your driver:

```rust
#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_driver_probe() {
        let device = DeviceInfo::new(0x1234, 0x5678, BusType::Pci);
        assert!(MyDriver::probe(&device).unwrap());
        
        let wrong_device = DeviceInfo::new(0x0000, 0x0000, BusType::Pci);
        assert!(!MyDriver::probe(&wrong_device).unwrap());
    }
    
    #[test]
    fn test_packet_send() {
        let mut driver = create_test_driver();
        let packet = [0x01, 0x02, 0x03, 0x04];
        
        assert!(driver.send_packet(&packet).is_ok());
    }
}
```

### 🔍 QEMU Testing

Test your driver in QEMU with device emulation:

```bash
# Test network driver with virtio-net
qemu-system-x86_64 \
  -cdrom build/orion-os.iso \
  -netdev user,id=net0 \
  -device virtio-net-pci,netdev=net0 \
  -monitor stdio

# Test block driver with virtio-blk  
qemu-system-x86_64 \
  -cdrom build/orion-os.iso \
  -drive file=test-disk.img,if=virtio,format=raw \
  -monitor stdio

# Test USB devices
qemu-system-x86_64 \
  -cdrom build/orion-os.iso \
  -device qemu-xhci \
  -device usb-kbd \
  -device usb-mouse \
  -monitor stdio
```

### 📊 Performance Profiling

Profile your driver performance:

```rust
use orion_driver::profiling::*;

impl NetworkDriver for MyDriver {
    fn send_packet(&mut self, packet: &[u8]) -> DriverResult<()> {
        let _timer = ProfileTimer::new("send_packet");
        
        // Your implementation here
        
        Ok(())
    }
}
```

## 📝 Best Practices

### 🔒 Security

1. **Validate all inputs**: Never trust data from hardware or kernel
2. **Use safe MMIO**: Always use `MmioAccessor` for hardware access
3. **Minimize privileges**: Request only necessary capabilities
4. **Error handling**: Handle all error cases gracefully

### ⚡ Performance

1. **Minimize allocations**: Reuse buffers where possible
2. **Batch operations**: Group multiple operations together
3. **Use DMA**: For large data transfers, use DMA instead of programmed I/O
4. **Cache-aware**: Align data structures to cache lines

### 🧩 Code Quality

1. **Documentation**: Document all public APIs thoroughly
2. **Testing**: Write unit tests and integration tests
3. **Error types**: Use specific error types, not generic errors
4. **Logging**: Use structured logging for debugging

## 📚 Reference

### 🔗 Useful Traits

- **`OrionDriver`**: Base trait all drivers must implement
- **`NetworkDriver`**: For network interface devices
- **`BlockDriver`**: For storage devices  
- **`UsbDriver`**: For USB devices
- **`GpuDriver`**: For graphics devices

### 📋 Error Types

```rust
pub enum DriverError {
    DeviceNotFound,      // Device not present or not responding
    InitializationFailed, // Device initialization failed
    ResourceBusy,        // Resource already in use
    Unsupported,         // Operation not supported
    IpcError,           // Communication with kernel failed
    MemoryError,        // Memory allocation or access error
    Timeout,            // Operation timed out
    General,            // General error
}
```

### 🛡️ Safety Guidelines

1. **No unsafe code** outside of MMIO access
2. **Bounds checking** for all array accesses
3. **Integer overflow** protection with checked arithmetic
4. **Resource cleanup** in Drop implementations
5. **Panic handling** with custom panic handlers

---

This guide provides everything needed to develop high-quality, safe, and performant drivers for Orion OS. The Rust-based userland approach ensures memory safety while maintaining the performance characteristics needed for system-level programming.
