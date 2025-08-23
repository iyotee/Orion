/*
 * Orion Driver Framework - IPC Module
 *
 * Inter-Process Communication interface for driver-kernel communication.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

use crate::types::{DeviceHandle, IoRequestType};
use crate::error::{DriverResult, DriverError};
use alloc::vec::Vec;
use alloc::string::String;
use alloc::string::ToString;

/// Message header for all IPC messages
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct MessageHeader {
    pub sequence: u64,
    pub message_type: MessageType,
    pub length: u32,
}

/// Message types for driver-kernel communication
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MessageType {
    Probe = 1,
    Init = 2,
    IoRequest = 3,
    Interrupt = 4,
    Shutdown = 5,
    Status = 6,
    Error = 7,
}

/// Messages received by drivers from the kernel
#[derive(Debug, Clone)]
pub enum ReceivedMessage {
    /// Probe message to check if driver can handle device
    ProbeDevice(ProbeMessage),
    
    /// Initialize device message
    InitDevice(DeviceHandle),
    
    /// I/O request message
    IoRequest(IoMessage),
    
    /// Interrupt notification
    Interrupt(DeviceHandle),
    
    /// Shutdown request
    Shutdown,
    
    /// Unknown message type
    Unknown,
}

/// Probe message for device detection
#[derive(Debug, Clone)]
pub struct ProbeMessage {
    pub header: MessageHeader,
    pub vendor_id: u16,
    pub device_id: u16,
    pub device_class: u8,
    pub device_subclass: u8,
    pub device_protocol: u8,
}

/// I/O request message
#[derive(Debug, Clone)]
pub struct IoMessage {
    pub header: MessageHeader,
    pub request_type: IoRequestType,
    pub device_handle: DeviceHandle,
    pub offset: u64,
    pub length: u32,
    pub data: Vec<u8>,
}

/// Interrupt message
#[derive(Debug, Clone)]
pub struct InterruptMessage {
    pub header: MessageHeader,
    pub device_handle: DeviceHandle,
    pub interrupt_type: u8,
    pub data: Vec<u8>,
}

/// IPC interface for driver-kernel communication
pub trait IpcInterface {
    /// Send probe response
    fn send_probe_response(&mut self, sequence: u64, can_handle: bool) -> DriverResult<()>;
    
    /// Send initialization response
    fn send_init_response(&mut self, sequence: u64, result: DriverResult<()>) -> DriverResult<()>;
    
    /// Send I/O response
    fn send_io_response(&mut self, sequence: u64, result: DriverResult<usize>) -> DriverResult<()>;
    
    /// Send interrupt response
    fn send_interrupt_response(&mut self, sequence: u64, result: DriverResult<()>) -> DriverResult<()>;
    
    /// Send status update
    fn send_status(&mut self, device_handle: DeviceHandle, status: u32) -> DriverResult<()>;
    
    /// Send error notification
    fn send_error(&mut self, device_handle: DeviceHandle, error: DriverError) -> DriverResult<()>;
}

/// Message loop for driver operation
pub struct MessageLoop {
    driver_name: String,
    driver_version: String,
    supported_vendors: Vec<u16>,
    supported_devices: Vec<u16>,
}

impl MessageLoop {
    /// Create a new message loop
    pub fn new() -> DriverResult<Self> {
        Ok(Self {
            driver_name: String::new(),
            driver_version: String::new(),
            supported_vendors: Vec::new(),
            supported_devices: Vec::new(),
        })
    }
    
    /// Run the message loop
    pub fn run<F>(
        &mut self,
        name: &str,
        version: &str,
        vendors: &[u16],
        devices: &[u16],
        message_handler: F,
    ) -> DriverResult<()>
    where
        F: FnMut(&mut dyn IpcInterface, ReceivedMessage) -> DriverResult<()>,
    {
        self.driver_name = name.to_string();
        self.driver_version = version.to_string();
        self.supported_vendors = vendors.to_vec();
        self.supported_devices = devices.to_vec();
        
        // TODO: Implement actual message loop with kernel communication
        // For now, just return success
        Ok(())
    }
}

/// Mock IPC interface for testing
pub struct MockIpcInterface;

impl IpcInterface for MockIpcInterface {
    fn send_probe_response(&mut self, _sequence: u64, _can_handle: bool) -> DriverResult<()> {
        Ok(())
    }
    
    fn send_init_response(&mut self, _sequence: u64, _result: DriverResult<()>) -> DriverResult<()> {
        Ok(())
    }
    
    fn send_io_response(&mut self, _sequence: u64, _result: DriverResult<usize>) -> DriverResult<()> {
        Ok(())
    }
    
    fn send_interrupt_response(&mut self, _sequence: u64, _result: DriverResult<()>) -> DriverResult<()> {
        Ok(())
    }
    
    fn send_status(&mut self, _device_handle: DeviceHandle, _status: u32) -> DriverResult<()> {
        Ok(())
    }
    
    fn send_error(&mut self, _device_handle: DeviceHandle, _error: DriverError) -> DriverResult<()> {
        Ok(())
    }
}
