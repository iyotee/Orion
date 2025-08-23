/*
 * Orion Driver Framework - Core Traits
 *
 * Defines the core traits that all Orion OS drivers must implement.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

use crate::ipc::{ReceivedMessage, IpcInterface};
use crate::error::DriverResult;
use crate::types::{DeviceInfo, UsbDeviceDescriptor};

/// Base trait for all Orion OS drivers
pub trait OrionDriver: Send + Sync {
    /// Probe if driver can handle a device
    fn probe(device: &DeviceInfo) -> DriverResult<bool> where Self: Sized;
    
    /// Get driver information
    fn get_info(&self) -> &'static str;
    
    /// Get driver version
    fn get_version(&self) -> &'static str;
    
    /// Check if driver can handle a specific device
    fn can_handle(&self, vendor_id: u16, device_id: u16) -> bool;
    
    /// Initialize the driver
    fn init(&mut self, device: DeviceInfo) -> DriverResult<()>;
    
    /// Handle IRQ/interrupt
    fn handle_irq(&mut self) -> DriverResult<()>;
    
    /// Shutdown the driver
    fn shutdown(&mut self) -> DriverResult<()>;
    
    /// Handle incoming messages
    fn handle_message(&mut self, message: ReceivedMessage, ipc: &mut dyn IpcInterface) -> DriverResult<()>;
}

/// USB device driver trait
pub trait UsbDriver: OrionDriver {
    /// Handle USB setup packets
    fn handle_setup_packet(&mut self, packet: &[u8]) -> DriverResult<()>;
    
    /// Handle USB data transfer
    fn handle_data_transfer(&mut self, endpoint: u8, data: &[u8]) -> DriverResult<usize>;
    
    /// Handle USB interrupts
    fn handle_interrupt(&mut self, endpoint: u8) -> DriverResult<()>;
    
    /// Control transfer
    fn control_transfer(&mut self, setup: &[u8], data: &mut [u8]) -> DriverResult<usize>;
    
    /// Bulk transfer
    fn bulk_transfer(&mut self, endpoint: u8, data: &mut [u8]) -> DriverResult<usize>;
    
    /// Interrupt transfer
    fn interrupt_transfer(&mut self, endpoint: u8, data: &mut [u8]) -> DriverResult<usize>;
    
    /// Get device descriptor
    fn device_descriptor(&self) -> UsbDeviceDescriptor;
}

/// Block device driver trait
pub trait BlockDriver: OrionDriver {
    /// Read data from block device
    fn read_blocks(&mut self, lba: u64, count: u32, buffer: &mut [u8]) -> DriverResult<usize>;
    
    /// Write data to block device
    fn write_blocks(&mut self, lba: u64, count: u32, buffer: &[u8]) -> DriverResult<usize>;
    
    /// Get device capacity in blocks
    fn get_capacity(&self) -> DriverResult<u64>;
    
    /// Get block size
    fn get_block_size(&self) -> DriverResult<u32>;
}

/// Network device driver trait
pub trait NetworkDriver: OrionDriver {
    /// Send packet
    fn send_packet(&mut self, data: &[u8]) -> DriverResult<usize>;
    
    /// Receive packet
    fn receive_packet(&mut self, buffer: &mut [u8]) -> DriverResult<usize>;
    
    /// Get MAC address
    fn get_mac_address(&self) -> DriverResult<[u8; 6]>;
    
    /// Set MAC address
    fn set_mac_address(&mut self, mac: [u8; 6]) -> DriverResult<()>;
}

/// Graphics driver trait
pub trait GraphicsDriver: OrionDriver {
    /// Set display mode
    fn set_mode(&mut self, width: u32, height: u32, depth: u32) -> DriverResult<()>;
    
    /// Get framebuffer address
    fn get_framebuffer(&self) -> DriverResult<*mut u8>;
    
    /// Get framebuffer size
    fn get_framebuffer_size(&self) -> DriverResult<usize>;
    
    /// Handle graphics commands
    fn handle_command(&mut self, command: &[u8]) -> DriverResult<()>;
}

/// Driver information trait
pub trait DriverInfo {
    /// Get driver name
    fn name(&self) -> &'static str;
    
    /// Get driver version
    fn version(&self) -> &'static str;
    
    /// Get supported vendor IDs
    fn supported_vendors(&self) -> &[u16];
    
    /// Get supported device IDs
    fn supported_devices(&self) -> &[u16];
    
    /// Get driver capabilities
    fn capabilities(&self) -> &[&'static str];
}
