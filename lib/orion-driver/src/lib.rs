/*
 * Orion Operating System - Rust Driver Framework
 *
 * Secure userland driver framework for the Orion hybrid micro-kernel.
 * Provides traits and utilities for developing safe and efficient
 * device drivers in Rust stable #![no_std].
 *
 * Developed by Jérémy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#![no_std]
#![forbid(unsafe_code)] // Except in specific HAL modules when necessary

pub mod device;
pub mod ipc;
pub mod mmio;
pub mod traits;

pub use device::*;
pub use ipc::*;
pub use mmio::*;
pub use traits::*;

/// Driver framework version
pub const DRIVER_FRAMEWORK_VERSION: &str = "0.1.0";

/// Standard result type for driver operations
pub type DriverResult<T> = Result<T, DriverError>;

/// Driver error types
#[derive(Debug, Clone, Copy)]
pub enum DriverError {
    /// Device not found or incompatible
    DeviceNotFound,
    /// Initialization failed
    InitializationFailed,
    /// Resource already in use
    ResourceBusy,
    /// Operation not supported
    Unsupported,
    /// IPC communication error
    IpcError,
    /// Memory/MMIO access error
    MemoryError,
    /// Operation timeout
    Timeout,
    /// General error
    General,
}
