/*
 * Orion Operating System - Driver Framework Library
 *
 * Common driver framework providing traits, structures, and utilities
 * for all Orion OS device drivers.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#![no_std]

extern crate alloc;

pub mod prelude;
pub mod traits;
pub mod types;
pub mod ipc;
pub mod error;

pub use traits::*;
pub use types::*;
pub use ipc::*;
pub use error::*;
