/*
 * Orion Operating System - Driver IPC Communication
 *
 * Inter-process communication between userland drivers and the kernel.
 * Implements secure message passing for driver operations.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

use crate::{DriverError, DriverResult};

/// Maximum message size for driver IPC
pub const MAX_MESSAGE_SIZE: usize = 4096;

/// Driver IPC message types
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u32)]
pub enum MessageType {
    /// Driver registration request
    RegisterDriver = 1,
    /// Driver unregistration request
    UnregisterDriver = 2,
    /// Device probe request
    ProbeDevice = 3,
    /// Device initialization request
    InitDevice = 4,
    /// I/O request
    IoRequest = 5,
    /// Interrupt notification
    InterruptNotify = 6,
    /// Driver shutdown request
    ShutdownDriver = 7,
    /// Driver status query
    StatusQuery = 8,
    /// Response message
    Response = 0x8000,
}

/// IPC message header
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct MessageHeader {
    /// Message type
    pub msg_type: MessageType,
    /// Message length (including header)
    pub length: u32,
    /// Sequence number for matching requests/responses
    pub sequence: u32,
    /// Process ID of sender
    pub sender_pid: u32,
    /// Error code (for responses)
    pub error_code: u32,
}

/// Driver registration message
#[repr(C)]
#[derive(Debug, Clone)]
pub struct RegisterDriverMessage {
    /// Message header
    pub header: MessageHeader,
    /// Driver name
    pub driver_name: [u8; 64],
    /// Driver version
    pub driver_version: [u8; 16],
    /// Supported device vendor IDs
    pub vendor_ids: [u16; 8],
    /// Supported device IDs
    pub device_ids: [u16; 8],
    /// Driver capabilities
    pub capabilities: u32,
}

/// Device probe message
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct ProbeDeviceMessage {
    /// Message header
    pub header: MessageHeader,
    /// Device vendor ID
    pub vendor_id: u16,
    /// Device ID
    pub device_id: u16,
    /// Device class
    pub device_class: u8,
    /// Device subclass
    pub device_subclass: u8,
    /// Bus type
    pub bus_type: u8,
    /// Padding
    pub _padding: u8,
}

/// I/O request types
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum IoRequestType {
    /// Read operation
    Read = 1,
    /// Write operation
    Write = 2,
    /// I/O control operation
    Ioctl = 3,
    /// Flush operation
    Flush = 4,
}

/// I/O request message
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct IoRequestMessage {
    /// Message header
    pub header: MessageHeader,
    /// Request type
    pub request_type: IoRequestType,
    /// Device handle
    pub device_handle: u32,
    /// Offset/LBA for the operation
    pub offset: u64,
    /// Length of data to read/write
    pub length: u32,
    /// Buffer address (in driver's address space)
    pub buffer_addr: u64,
    /// Additional parameter (for ioctl)
    pub parameter: u64,
}

/// Driver IPC context
pub struct DriverIpcContext {
    /// Kernel IPC port capability
    kernel_port: u64,
    /// Driver's IPC port capability
    driver_port: u64,
    /// Next sequence number
    next_sequence: u32,
}

impl DriverIpcContext {
    /// Create a new IPC context
    pub fn new() -> DriverResult<Self> {
        // Create IPC ports with kernel
        // This establishes the communication channel between driver and kernel
        
        // For now, use placeholder port numbers
        // In a real implementation, these would be obtained from the kernel
        // via a system call or bootloader-provided information
        let kernel_port = 0x1000; // Kernel IPC port
        let driver_port = 0x2000; // Driver IPC port
        
        Ok(Self {
            kernel_port,
            driver_port,
            next_sequence: 1,
        })
    }
    
    /// Register driver with kernel
    pub fn register_driver(&mut self, 
                          name: &str, 
                          version: &str,
                          vendor_ids: &[u16],
                          device_ids: &[u16]) -> DriverResult<()> {
        let mut msg = RegisterDriverMessage {
            header: MessageHeader {
                msg_type: MessageType::RegisterDriver,
                length: core::mem::size_of::<RegisterDriverMessage>() as u32,
                sequence: self.next_sequence,
                sender_pid: self.get_current_pid(), // Get current PID
                error_code: 0,
            },
            driver_name: [0; 64],
            driver_version: [0; 16],
            vendor_ids: [0; 8],
            device_ids: [0; 8],
            capabilities: 0,
        };
        
        // Copy driver name
        let name_bytes = name.as_bytes();
        let copy_len = core::cmp::min(name_bytes.len(), 63);
        msg.driver_name[..copy_len].copy_from_slice(&name_bytes[..copy_len]);
        
        // Copy version
        let version_bytes = version.as_bytes();
        let copy_len = core::cmp::min(version_bytes.len(), 15);
        msg.driver_version[..copy_len].copy_from_slice(&version_bytes[..copy_len]);
        
        // Copy vendor and device IDs
        let vendor_copy_len = core::cmp::min(vendor_ids.len(), 8);
        msg.vendor_ids[..vendor_copy_len].copy_from_slice(&vendor_ids[..vendor_copy_len]);
        
        let device_copy_len = core::cmp::min(device_ids.len(), 8);
        msg.device_ids[..device_copy_len].copy_from_slice(&device_ids[..device_copy_len]);
        
        self.next_sequence += 1;
        
        // Send message to kernel
        self.send_message(&msg)?;
        
        // Wait for response
        self.wait_for_response(msg.header.sequence)?;
        
        Ok(())
    }
    
    /// Send probe response to kernel
    pub fn send_probe_response(&mut self, sequence: u32, can_handle: bool) -> DriverResult<()> {
        let response = MessageHeader {
            msg_type: MessageType::Response,
            length: core::mem::size_of::<MessageHeader>() as u32,
            sequence,
            sender_pid: self.get_current_pid(), // Get current PID
            error_code: if can_handle { 0 } else { 1 },
        };
        
        self.send_message(&response)
    }
    
    /// Send I/O response to kernel
    pub fn send_io_response(&mut self, sequence: u32, result: DriverResult<usize>) -> DriverResult<()> {
        let (error_code, length) = match result {
            Ok(len) => (0, len as u32),
            Err(DriverError::DeviceNotFound) => (1, 0),
            Err(DriverError::InitializationFailed) => (2, 0),
            Err(DriverError::ResourceBusy) => (3, 0),
            Err(DriverError::Unsupported) => (4, 0),
            Err(DriverError::IpcError) => (5, 0),
            Err(DriverError::MemoryError) => (6, 0),
            Err(DriverError::Timeout) => (7, 0),
            Err(DriverError::General) => (8, 0),
        };
        
        let response = IoRequestMessage {
            header: MessageHeader {
                msg_type: MessageType::Response,
                length: core::mem::size_of::<IoRequestMessage>() as u32,
                sequence,
                sender_pid: self.get_current_pid(), // Get current PID
                error_code,
            },
            request_type: IoRequestType::Read, // Not used in response
            device_handle: 0,
            offset: 0,
            length,
            buffer_addr: 0,
            parameter: 0,
        };
        
        self.send_message(&response)
    }
    
    /// Get current process ID for sender identification
    fn get_current_pid(&self) -> u32 {
        // In a real implementation, this would call a system call or
        // access kernel-provided information to get the current PID
        
        // For now, use a placeholder PID
        // This should be replaced with actual PID retrieval
        0x1234 // Placeholder PID
    }
    
    /// Receive next message from kernel
    pub fn receive_message(&mut self) -> DriverResult<ReceivedMessage> {
        // Implement actual IPC receive
        // This would typically involve:
        // 1. Checking if a message is available
        // 2. Reading the message from the IPC port
        // 3. Parsing the message header and body
        
        // For now, simulate message reception
        // In a real implementation, this would block until a message arrives
        
        // Simulate receiving a probe message
        if self.next_sequence % 10 == 0 {
            // Simulate device probe request
            let probe_msg = ProbeDeviceMessage {
                header: MessageHeader {
                    msg_type: MessageType::ProbeDevice,
                    length: core::mem::size_of::<ProbeDeviceMessage>() as u32,
                    sequence: self.next_sequence,
                    sender_pid: self.kernel_port as u32,
                    error_code: 0,
                },
                vendor_id: 0x8086, // Intel
                device_id: 0x1234, // Example device
                device_class: 0x03,       // Display controller
                device_subclass: 0x00,
                bus_type: 0x00,
                _padding: 0,
            };
            self.next_sequence += 1;
            return Ok(ReceivedMessage::ProbeDevice(probe_msg));
        }
        
        // Simulate I/O request
        if self.next_sequence % 15 == 0 {
            let io_msg = IoRequestMessage {
                header: MessageHeader {
                    msg_type: MessageType::IoRequest,
                    length: core::mem::size_of::<IoRequestMessage>() as u32,
                    sequence: self.next_sequence,
                    sender_pid: self.kernel_port as u32,
                    error_code: 0,
                },
                request_type: IoRequestType::Read,
                device_handle: 1,
                offset: 0,
                length: 512,
                buffer_addr: 0x1000,
                parameter: 0,
            };
            self.next_sequence += 1;
            return Ok(ReceivedMessage::IoRequest(io_msg));
        }
        
        // Simulate interrupt notification
        if self.next_sequence % 20 == 0 {
            self.next_sequence += 1;
            return Ok(ReceivedMessage::Interrupt(1));
        }
        
        // No message available
        Err(DriverError::Timeout)
    }
    
    /// Send a message to kernel
    fn send_message<T>(&self, message: &T) -> DriverResult<()> {
        // Implement actual IPC send
        // This would typically involve:
        // 1. Serializing the message
        // 2. Writing to the IPC port
        // 3. Ensuring message delivery
        
        // For now, simulate successful message sending
        // In a real implementation, this would actually send the message
        
        // Simulate message processing delay
        // This would be replaced with actual IPC communication
        
        Ok(())
    }
    
    /// Wait for response with specific sequence number
    fn wait_for_response(&mut self, sequence: u32) -> DriverResult<MessageHeader> {
        // Implement response waiting
        // This would typically involve:
        // 1. Waiting for a response message
        // 2. Checking if the response matches the expected sequence
        // 3. Handling timeout scenarios
        
        // For now, simulate response reception
        // In a real implementation, this would block until the response arrives
        
        // Simulate successful response
        let response = MessageHeader {
            msg_type: MessageType::Response,
            length: core::mem::size_of::<MessageHeader>() as u32,
            sequence,
            sender_pid: self.kernel_port as u32,
            error_code: 0, // Success
        };
        
        Ok(response)
    }
}

/// Received message wrapper
pub enum ReceivedMessage {
    /// Device probe request
    ProbeDevice(ProbeDeviceMessage),
    /// Device initialization request
    InitDevice(u32), // device handle
    /// I/O request
    IoRequest(IoRequestMessage),
    /// Interrupt notification
    Interrupt(u32), // device handle
    /// Shutdown request
    Shutdown,
    /// Unknown message
    Unknown,
}

/// Driver message loop helper
pub struct MessageLoop {
    /// IPC context
    ipc: DriverIpcContext,
    /// Whether the loop should continue
    running: bool,
}

impl MessageLoop {
    /// Create a new message loop
    pub fn new() -> DriverResult<Self> {
        Ok(Self {
            ipc: DriverIpcContext::new()?,
            running: true,
        })
    }
    
    /// Register driver and start message loop
    pub fn run<F>(&mut self, 
                  name: &str, 
                  version: &str,
                  vendor_ids: &[u16],
                  device_ids: &[u16],
                  mut handler: F) -> DriverResult<()> 
    where
        F: FnMut(&mut DriverIpcContext, ReceivedMessage) -> DriverResult<()>
    {
        // Register with kernel
        self.ipc.register_driver(name, version, vendor_ids, device_ids)?;
        
        // Message loop
        while self.running {
            match self.ipc.receive_message() {
                Ok(message) => {
                    if let Err(e) = handler(&mut self.ipc, message) {
                        // Log error properly
                        self.log_error("Message handler failed", &e);
                        break;
                    }
                }
                Err(DriverError::IpcError) => {
                    // Handle IPC error gracefully
                    self.log_error("IPC communication error", &DriverError::IpcError);
                    break;
                }
                Err(DriverError::Timeout) => {
                    // Timeout is normal, continue
                    continue;
                }
                Err(e) => {
                    // Log other errors and continue
                    self.log_error("Unexpected error in message loop", &e);
                    continue;
                }
            }
        }
        
        Ok(())
    }
    
    /// Stop the message loop
    pub fn stop(&mut self) {
        self.running = false;
    }
    
    /// Log error with context
    fn log_error(&self, context: &str, error: &DriverError) {
        // In a real implementation, this would send the error to the kernel
        // or write to a log file. For now, we'll just ignore it.
        
        // This could be enhanced with:
        // - Timestamp
        // - Driver name
        // - Error context
        // - Stack trace (if available)
        
        // For now, silently handle errors to avoid infinite loops
        let _ = (context, error);
    }
}
