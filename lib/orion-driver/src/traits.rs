/*
 * Orion Operating System - Driver Traits
 *
 * Core traits that all Orion drivers must implement for consistent
 * driver architecture and type safety.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

use crate::{DeviceInfo, DriverError, DriverResult};

/// Main trait that all Orion drivers must implement
pub trait OrionDriver {
    /// Check if this driver can handle the device
    fn probe(device: &DeviceInfo) -> DriverResult<bool> 
    where 
        Self: Sized;
    
    /// Initialize the driver with the device
    fn init(device: DeviceInfo) -> DriverResult<Self> 
    where 
        Self: Sized;
    
    /// Handle an interrupt
    fn handle_irq(&mut self) -> DriverResult<()>;
    
    /// Shutdown the driver cleanly
    fn shutdown(&mut self) -> DriverResult<()>;
    
    /// Get driver information
    fn info(&self) -> DriverInfo;
}

/// Driver information structure
#[derive(Debug, Clone)]
pub struct DriverInfo {
    pub name: &'static str,
    pub version: &'static str,
    pub author: &'static str,
    pub description: &'static str,
}

/// Trait for block storage drivers
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
    fn flush(&mut self) -> DriverResult<()> {
        // Default implementation does nothing
        Ok(())
    }
    
    /// Get device capabilities
    fn capabilities(&self) -> BlockCapabilities {
        BlockCapabilities::empty()
    }
}

/// Block device capabilities
bitflags::bitflags! {
    pub struct BlockCapabilities: u32 {
        const READ_ONLY = 1 << 0;
        const WRITE_CACHE = 1 << 1;
        const FLUSH_CACHE = 1 << 2;
        const TRIM_SUPPORT = 1 << 3;
        const SECURE_ERASE = 1 << 4;
    }
}

/// Trait for network drivers
pub trait NetworkDriver: OrionDriver {
    /// Send a packet
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

/// Network link status
#[derive(Debug, Clone, Copy)]
pub enum LinkStatus {
    Down,
    Up { speed_mbps: u32, duplex: bool },
}

/// Network statistics
#[derive(Debug, Clone, Copy, Default)]
pub struct NetworkStats {
    pub rx_packets: u64,
    pub tx_packets: u64,
    pub rx_bytes: u64,
    pub tx_bytes: u64,
    pub rx_errors: u64,
    pub tx_errors: u64,
    pub rx_dropped: u64,
    pub tx_dropped: u64,
}

/// Trait for USB drivers
pub trait UsbDriver: OrionDriver {
    /// Send a control transfer
    fn control_transfer(&mut self, setup: &UsbSetupPacket, data: &mut [u8]) -> DriverResult<usize>;
    
    /// Bulk transfer
    fn bulk_transfer(&mut self, endpoint: u8, data: &mut [u8]) -> DriverResult<usize>;
    
    /// Interrupt transfer
    fn interrupt_transfer(&mut self, endpoint: u8, data: &mut [u8]) -> DriverResult<usize>;
    
    /// Get device descriptor
    fn device_descriptor(&self) -> UsbDeviceDescriptor;
}

/// USB setup packet
#[repr(C, packed)]
pub struct UsbSetupPacket {
    pub request_type: u8,
    pub request: u8,
    pub value: u16,
    pub index: u16,
    pub length: u16,
}

/// USB device descriptor
#[repr(C, packed)]
pub struct UsbDeviceDescriptor {
    pub length: u8,
    pub descriptor_type: u8,
    pub usb_version: u16,
    pub device_class: u8,
    pub device_subclass: u8,
    pub device_protocol: u8,
    pub max_packet_size: u8,
    pub vendor_id: u16,
    pub product_id: u16,
    pub device_version: u16,
    pub manufacturer_string: u8,
    pub product_string: u8,
    pub serial_string: u8,
    pub num_configurations: u8,
}

/// Trait for GPU drivers
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

/// Framebuffer information
#[derive(Debug, Clone, Copy)]
pub struct FramebufferInfo {
    pub width: u32,
    pub height: u32,
    pub bpp: u8,
    pub pitch: u32,
    pub memory_size: usize,
    pub physical_address: u64,
}
