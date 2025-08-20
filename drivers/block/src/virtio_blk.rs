/*
 * Orion Operating System - VirtIO Block Driver
 *
 * VirtIO block device driver implementation for development and testing
 * in QEMU and other virtualized environments.
 *
 * Developed by Jérémy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#![no_std]
#![no_main]

use orion_driver::{
    BlockDriver, DeviceInfo, DriverError, DriverInfo, DriverResult, OrionDriver,
    BlockCapabilities, MmioAccessor, MmioPermissions,
};

/// VirtIO Block Device Driver
pub struct VirtioBlockDriver {
    device: DeviceInfo,
    mmio: MmioAccessor,
    block_size: u32,
    block_count: u64,
    features: u64,
}

// VirtIO register offsets
const VIRTIO_MMIO_MAGIC_VALUE: usize = 0x000;
const VIRTIO_MMIO_VERSION: usize = 0x004;
const VIRTIO_MMIO_DEVICE_ID: usize = 0x008;
const VIRTIO_MMIO_VENDOR_ID: usize = 0x00c;
const VIRTIO_MMIO_DEVICE_FEATURES: usize = 0x010;
const VIRTIO_MMIO_DRIVER_FEATURES: usize = 0x020;
const VIRTIO_MMIO_QUEUE_SEL: usize = 0x030;
const VIRTIO_MMIO_QUEUE_NUM_MAX: usize = 0x034;
const VIRTIO_MMIO_QUEUE_NUM: usize = 0x038;
const VIRTIO_MMIO_QUEUE_READY: usize = 0x044;
const VIRTIO_MMIO_QUEUE_NOTIFY: usize = 0x050;
const VIRTIO_MMIO_INTERRUPT_STATUS: usize = 0x060;
const VIRTIO_MMIO_INTERRUPT_ACK: usize = 0x064;
const VIRTIO_MMIO_STATUS: usize = 0x070;
const VIRTIO_MMIO_CONFIG: usize = 0x100;

// VirtIO status register bits
const VIRTIO_STATUS_ACKNOWLEDGE: u32 = 1;
const VIRTIO_STATUS_DRIVER: u32 = 2;
const VIRTIO_STATUS_DRIVER_OK: u32 = 4;
const VIRTIO_STATUS_FEATURES_OK: u32 = 8;
const VIRTIO_STATUS_FAILED: u32 = 128;

// VirtIO block features
const VIRTIO_BLK_F_SIZE_MAX: u64 = 1 << 1;
const VIRTIO_BLK_F_SEG_MAX: u64 = 1 << 2;
const VIRTIO_BLK_F_GEOMETRY: u64 = 1 << 4;
const VIRTIO_BLK_F_RO: u64 = 1 << 5;
const VIRTIO_BLK_F_BLK_SIZE: u64 = 1 << 6;
const VIRTIO_BLK_F_FLUSH: u64 = 1 << 9;

impl OrionDriver for VirtioBlockDriver {
    fn probe(device: &DeviceInfo) -> DriverResult<bool> {
        // VirtIO Vendor ID: 0x1AF4, Block Device ID: 0x1001
        Ok(device.vendor_id == 0x1AF4 && device.device_id == 0x1001)
    }
    
    fn init(device: DeviceInfo) -> DriverResult<Self> {
        // Create MMIO accessor for device registers
        let mmio = unsafe {
            MmioAccessor::new(
                device.bars[0], // First BAR contains MMIO registers
                4096,           // Standard VirtIO MMIO region size
                MmioPermissions::READ | MmioPermissions::WRITE | MmioPermissions::UNCACHED
            )
        };
        
        // Check VirtIO magic number
        let magic = mmio.read_u32(VIRTIO_MMIO_MAGIC_VALUE)?;
        if magic != 0x74726976 { // "virt" in little endian
            return Err(DriverError::DeviceNotFound);
        }
        
        // Check VirtIO version
        let version = mmio.read_u32(VIRTIO_MMIO_VERSION)?;
        if version != 2 {
            return Err(DriverError::Unsupported);
        }
        
        // Reset device
        mmio.write_u32(VIRTIO_MMIO_STATUS, 0)?;
        
        // Set ACKNOWLEDGE status bit
        mmio.write_u32(VIRTIO_MMIO_STATUS, VIRTIO_STATUS_ACKNOWLEDGE)?;
        
        // Set DRIVER status bit
        mmio.write_u32(VIRTIO_MMIO_STATUS, 
                      VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER)?;
        
        // Read device features
        let device_features = mmio.read_u32(VIRTIO_MMIO_DEVICE_FEATURES)?;
        
        // Select features we support
        let mut driver_features = 0u32;
        if device_features & (VIRTIO_BLK_F_BLK_SIZE as u32) != 0 {
            driver_features |= VIRTIO_BLK_F_BLK_SIZE as u32;
        }
        if device_features & (VIRTIO_BLK_F_FLUSH as u32) != 0 {
            driver_features |= VIRTIO_BLK_F_FLUSH as u32;
        }
        
        // Write driver features
        mmio.write_u32(VIRTIO_MMIO_DRIVER_FEATURES, driver_features)?;
        
        // Set FEATURES_OK status bit
        mmio.write_u32(VIRTIO_MMIO_STATUS,
                      VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | VIRTIO_STATUS_FEATURES_OK)?;
        
        // Check that device accepted our features
        let status = mmio.read_u32(VIRTIO_MMIO_STATUS)?;
        if status & VIRTIO_STATUS_FEATURES_OK == 0 {
            return Err(DriverError::InitializationFailed);
        }
        
        // Read device configuration
        let block_size = if driver_features & (VIRTIO_BLK_F_BLK_SIZE as u32) != 0 {
            mmio.read_u32(VIRTIO_MMIO_CONFIG + 20)?
        } else {
            512 // Default block size
        };
        
        let capacity = mmio.read_u64(VIRTIO_MMIO_CONFIG)?;
        let block_count = capacity;
        
        // TODO: Initialize virtqueues
        // For now, we'll skip queue setup and just provide basic info
        
        // Set DRIVER_OK status bit
        mmio.write_u32(VIRTIO_MMIO_STATUS,
                      VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | 
                      VIRTIO_STATUS_FEATURES_OK | VIRTIO_STATUS_DRIVER_OK)?;
        
        Ok(VirtioBlockDriver {
            device,
            mmio,
            block_size,
            block_count,
            features: driver_features as u64,
        })
    }
    
    fn handle_irq(&mut self) -> DriverResult<()> {
        // Read interrupt status
        let status = self.mmio.read_u32(VIRTIO_MMIO_INTERRUPT_STATUS)?;
        
        if status & 1 != 0 {
            // Queue interrupt - process completed requests
            // TODO: Process virtqueue completions
        }
        
        if status & 2 != 0 {
            // Configuration change interrupt
            // TODO: Handle configuration changes
        }
        
        // Acknowledge interrupts
        self.mmio.write_u32(VIRTIO_MMIO_INTERRUPT_ACK, status)?;
        
        Ok(())
    }
    
    fn shutdown(&mut self) -> DriverResult<()> {
        // Reset device status
        self.mmio.write_u32(VIRTIO_MMIO_STATUS, 0)?;
        Ok(())
    }
    
    fn info(&self) -> DriverInfo {
        DriverInfo {
            name: "VirtIO Block Driver",
            version: "1.0.0",
            author: "Jérémy Noverraz",
            description: "VirtIO block device driver for QEMU and virtualized environments",
        }
    }
}

impl BlockDriver for VirtioBlockDriver {
    fn read_blocks(&mut self, lba: u64, count: u32, buffer: &mut [u8]) -> DriverResult<()> {
        // Validate parameters
        if buffer.len() < (count * self.block_size) as usize {
            return Err(DriverError::MemoryError);
        }
        
        if lba + count as u64 > self.block_count {
            return Err(DriverError::MemoryError);
        }
        
        // TODO: Implement actual VirtIO queue-based I/O
        // For now, simulate successful read with placeholder data
        
        // Fill buffer with pattern for testing
        for (i, byte) in buffer.iter_mut().enumerate().take((count * self.block_size) as usize) {
            *byte = ((lba + i as u64) & 0xFF) as u8;
        }
        
        Ok(())
    }
    
    fn write_blocks(&mut self, lba: u64, count: u32, buffer: &[u8]) -> DriverResult<()> {
        // Validate parameters
        if buffer.len() < (count * self.block_size) as usize {
            return Err(DriverError::MemoryError);
        }
        
        if lba + count as u64 > self.block_count {
            return Err(DriverError::MemoryError);
        }
        
        // Check if device is read-only
        if self.features & VIRTIO_BLK_F_RO != 0 {
            return Err(DriverError::Unsupported);
        }
        
        // TODO: Implement actual VirtIO queue-based I/O
        // For now, simulate successful write
        
        let _ = buffer; // Silence unused warning
        Ok(())
    }
    
    fn block_count(&self) -> u64 {
        self.block_count
    }
    
    fn block_size(&self) -> u32 {
        self.block_size
    }
    
    fn flush(&mut self) -> DriverResult<()> {
        // Only flush if device supports it
        if self.features & VIRTIO_BLK_F_FLUSH != 0 {
            // TODO: Send flush command via VirtIO queue
            Ok(())
        } else {
            Err(DriverError::Unsupported)
        }
    }
    
    fn capabilities(&self) -> BlockCapabilities {
        let mut caps = BlockCapabilities::empty();
        
        if self.features & VIRTIO_BLK_F_RO != 0 {
            caps |= BlockCapabilities::READ_only();
        }
        
        if self.features & VIRTIO_BLK_F_FLUSH != 0 {
            caps |= BlockCapabilities::flush_cache();
        }
        
        caps
    }
}

/// Driver entry point
#[no_mangle]
pub extern "C" fn driver_main() {
    use orion_driver::{MessageLoop, ReceivedMessage};
    
    // Create message loop for kernel communication
    let mut message_loop = match MessageLoop::new() {
        Ok(loop_obj) => loop_obj,
        Err(_) => return, // Failed to initialize IPC
    };
    
    // Register driver and start message loop
    let result = message_loop.run(
        "virtio-blk",
        "1.0.0",
        &[0x1AF4], // VirtIO vendor ID
        &[0x1001], // Block device ID
        |ipc, message| {
            match message {
                ReceivedMessage::ProbeDevice(probe_msg) => {
                    // Check if we can handle this device
                    let can_handle = probe_msg.vendor_id == 0x1AF4 && 
                                   probe_msg.device_id == 0x1001;
                    ipc.send_probe_response(probe_msg.header.sequence, can_handle)
                }
                
                ReceivedMessage::InitDevice(device_handle) => {
                    // TODO: Initialize specific device instance
                    let _ = device_handle;
                    Ok(())
                }
                
                ReceivedMessage::IoRequest(io_msg) => {
                    // TODO: Handle I/O request
                    // For now, simulate successful operation
                    let result = match io_msg.request_type {
                        orion_driver::IoRequestType::Read => Ok(io_msg.length as usize),
                        orion_driver::IoRequestType::Write => Ok(io_msg.length as usize),
                        orion_driver::IoRequestType::Flush => Ok(0),
                        _ => Err(orion_driver::DriverError::Unsupported),
                    };
                    
                    ipc.send_io_response(io_msg.header.sequence, result)
                }
                
                ReceivedMessage::Interrupt(device_handle) => {
                    // TODO: Handle device interrupt
                    let _ = device_handle;
                    Ok(())
                }
                
                ReceivedMessage::Shutdown => {
                    // Clean shutdown requested
                    Ok(())
                }
                
                ReceivedMessage::Unknown => {
                    // Unknown message type, ignore
                    Ok(())
                }
            }
        }
    );
    
    // Log result (TODO: proper logging)
    let _ = result;
}

/// Panic handler for the driver
#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    // TODO: Report crash to driver supervisor via IPC
    // TODO: Implement proper crash reporting
    
    // For now, just halt
    loop {
        unsafe {
            core::arch::asm!("hlt");
        }
    }
}
