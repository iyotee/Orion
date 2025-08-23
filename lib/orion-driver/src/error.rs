/*
 * Orion Driver Framework - Error Handling
 *
 * Error types and result handling for driver operations.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

use core::fmt;

/// Result type for driver operations
pub type DriverResult<T> = Result<T, DriverError>;

/// Driver error types
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DriverError {
    /// General error
    General,
    
    /// Not supported
    Unsupported,
    
    /// Invalid parameter
    InvalidParameter,
    
    /// Device not found
    DeviceNotFound,
    
    /// Device busy
    DeviceBusy,
    
    /// Timeout
    Timeout,
    
    /// I/O error
    IoError,
    
    /// Out of memory
    OutOfMemory,
    
    /// Permission denied
    PermissionDenied,
    
    /// Already exists
    AlreadyExists,
    
    /// Not initialized
    NotInitialized,
    
    /// Invalid state
    InvalidState,
    
    /// Buffer too small
    BufferTooSmall,
    
    /// Protocol error
    ProtocolError,
    
    /// Hardware error
    HardwareError,
    
    /// Driver specific error
    DriverSpecific(u32),
}

impl fmt::Display for DriverError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            DriverError::General => write!(f, "General error"),
            DriverError::Unsupported => write!(f, "Operation not supported"),
            DriverError::InvalidParameter => write!(f, "Invalid parameter"),
            DriverError::DeviceNotFound => write!(f, "Device not found"),
            DriverError::DeviceBusy => write!(f, "Device busy"),
            DriverError::Timeout => write!(f, "Operation timeout"),
            DriverError::IoError => write!(f, "I/O error"),
            DriverError::OutOfMemory => write!(f, "Out of memory"),
            DriverError::PermissionDenied => write!(f, "Permission denied"),
            DriverError::AlreadyExists => write!(f, "Already exists"),
            DriverError::NotInitialized => write!(f, "Not initialized"),
            DriverError::InvalidState => write!(f, "Invalid state"),
            DriverError::BufferTooSmall => write!(f, "Buffer too small"),
            DriverError::ProtocolError => write!(f, "Protocol error"),
            DriverError::HardwareError => write!(f, "Hardware error"),
            DriverError::DriverSpecific(code) => write!(f, "Driver specific error: {}", code),
        }
    }
}



impl From<core::convert::Infallible> for DriverError {
    fn from(_: core::convert::Infallible) -> Self {
        DriverError::General
    }
}

/// Error kind for categorization
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ErrorKind {
    /// Fatal error - driver cannot continue
    Fatal,
    
    /// Recoverable error - driver can retry
    Recoverable,
    
    /// Temporary error - driver should wait and retry
    Temporary,
    
    /// User error - invalid user input
    User,
    
    /// System error - system resource issue
    System,
}

impl DriverError {
    /// Get the error kind
    pub fn kind(&self) -> ErrorKind {
        match self {
            DriverError::General => ErrorKind::System,
            DriverError::Unsupported => ErrorKind::User,
            DriverError::InvalidParameter => ErrorKind::User,
            DriverError::DeviceNotFound => ErrorKind::System,
            DriverError::DeviceBusy => ErrorKind::Temporary,
            DriverError::Timeout => ErrorKind::Temporary,
            DriverError::IoError => ErrorKind::Recoverable,
            DriverError::OutOfMemory => ErrorKind::System,
            DriverError::PermissionDenied => ErrorKind::User,
            DriverError::AlreadyExists => ErrorKind::User,
            DriverError::NotInitialized => ErrorKind::System,
            DriverError::InvalidState => ErrorKind::System,
            DriverError::BufferTooSmall => ErrorKind::User,
            DriverError::ProtocolError => ErrorKind::Recoverable,
            DriverError::HardwareError => ErrorKind::Fatal,
            DriverError::DriverSpecific(_) => ErrorKind::System,
        }
    }
    
    /// Check if error is fatal
    pub fn is_fatal(&self) -> bool {
        matches!(self.kind(), ErrorKind::Fatal)
    }
    
    /// Check if error is recoverable
    pub fn is_recoverable(&self) -> bool {
        matches!(self.kind(), ErrorKind::Recoverable)
    }
    
    /// Check if error is temporary
    pub fn is_temporary(&self) -> bool {
        matches!(self.kind(), ErrorKind::Temporary)
    }
}
