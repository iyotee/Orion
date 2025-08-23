/*
 * Orion Driver Framework - Prelude Module
 *
 * Re-exports all essential components for easy driver development.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

// Core traits
pub use crate::traits::{
    OrionDriver,
    UsbDriver,
    BlockDriver,
    NetworkDriver,
    GraphicsDriver,
};

// Common types
pub use crate::types::{
    DeviceInfo,
    UsbDeviceDescriptor, UsbConfigurationDescriptor, UsbInterfaceDescriptor, UsbEndpointDescriptor,
    MouseEvent, KeyEvent, KeyCode, HidReportDescriptor, HidReportType,
    StorageGeometry, CommandBlockWrapper, CommandStatusWrapper,
    MacAddress, IpAddress,
    DisplayMode, ColorFormat, FramebufferInfo,
};

// IPC and messaging
pub use crate::ipc::{
    MessageLoop,
    ReceivedMessage,
    IpcInterface,
    MessageHeader,
    ProbeMessage,
    IoMessage,
    InterruptMessage,
};

// Error handling
pub use crate::error::{
    DriverError,
    DriverResult,
    ErrorKind,
};

// Re-export common dependencies
pub use bitflags::bitflags;
pub use spin::Mutex;
pub use spin::RwLock;
