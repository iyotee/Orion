/*
 * Orion Operating System - Memory-Mapped I/O (MMIO)
 *
 * Safe abstractions for memory-mapped I/O operations in userland drivers.
 * Provides controlled access to device registers and memory.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

// Allow unsafe code in this module as it's necessary for MMIO operations
#![allow(unsafe_code)]

use crate::{DriverError, DriverResult};
use core::marker::PhantomData;
use core::ptr::{read_volatile, write_volatile};

/// MMIO region descriptor
#[derive(Debug, Clone, Copy)]
pub struct MmioRegion {
    /// Base physical address
    pub base_addr: u64,
    /// Size in bytes
    pub size: usize,
    /// Virtual address (mapped by kernel)
    pub virt_addr: u64,
    /// Access permissions
    pub permissions: MmioPermissions,
}

/// MMIO access permissions
bitflags::bitflags! {
    #[derive(Debug, Clone, Copy)]
    pub struct MmioPermissions: u32 {
        /// Read access
        const READ = 1 << 0;
        /// Write access
        const WRITE = 1 << 1;
        /// Execute access (rare for MMIO)
        const EXECUTE = 1 << 2;
        /// Uncached access
        const UNCACHED = 1 << 3;
        /// Write-combining access
        const WRITE_COMBINING = 1 << 4;
    }
}

/// Safe MMIO accessor
pub struct MmioAccessor {
    /// Base virtual address
    base_addr: u64,
    /// Size of the region
    size: usize,
    /// Access permissions
    permissions: MmioPermissions,
}

impl MmioAccessor {
    /// Create a new MMIO accessor
    /// 
    /// # Safety
    /// 
    /// The caller must ensure that:
    /// - The base address is valid and mapped
    /// - The size is correct
    /// - The permissions match the actual mapping
    pub unsafe fn new(base_addr: u64, size: usize, permissions: MmioPermissions) -> Self {
        Self {
            base_addr,
            size,
            permissions,
        }
    }
    
    /// Create MMIO accessor from region
    pub fn from_region(region: &MmioRegion) -> Self {
        Self {
            base_addr: region.virt_addr,
            size: region.size,
            permissions: region.permissions.clone(),
        }
    }
    
    /// Check if address is within bounds
    fn check_bounds(&self, offset: usize, size: usize) -> DriverResult<()> {
        if offset + size > self.size {
            return Err(DriverError::MemoryError);
        }
        Ok(())
    }
    
    /// Read 8-bit value from offset
    pub fn read_u8(&self, offset: usize) -> DriverResult<u8> {
        if !self.permissions.contains(MmioPermissions::READ) {
            return Err(DriverError::MemoryError);
        }
        
        self.check_bounds(offset, 1)?;
        
        let addr = (self.base_addr + offset as u64) as *const u8;
        Ok(unsafe { read_volatile(addr) })
    }
    
    /// Read 16-bit value from offset
    pub fn read_u16(&self, offset: usize) -> DriverResult<u16> {
        if !self.permissions.contains(MmioPermissions::READ) {
            return Err(DriverError::MemoryError);
        }
        
        self.check_bounds(offset, 2)?;
        
        let addr = (self.base_addr + offset as u64) as *const u16;
        Ok(unsafe { read_volatile(addr) })
    }
    
    /// Read 32-bit value from offset
    pub fn read_u32(&self, offset: usize) -> DriverResult<u32> {
        if !self.permissions.contains(MmioPermissions::READ) {
            return Err(DriverError::MemoryError);
        }
        
        self.check_bounds(offset, 4)?;
        
        let addr = (self.base_addr + offset as u64) as *const u32;
        Ok(unsafe { read_volatile(addr) })
    }
    
    /// Read 64-bit value from offset
    pub fn read_u64(&self, offset: usize) -> DriverResult<u64> {
        if !self.permissions.contains(MmioPermissions::READ) {
            return Err(DriverError::MemoryError);
        }
        
        self.check_bounds(offset, 8)?;
        
        let addr = (self.base_addr + offset as u64) as *const u64;
        Ok(unsafe { read_volatile(addr) })
    }
    
    /// Write 8-bit value to offset
    pub fn write_u8(&self, offset: usize, value: u8) -> DriverResult<()> {
        if !self.permissions.contains(MmioPermissions::WRITE) {
            return Err(DriverError::MemoryError);
        }
        
        self.check_bounds(offset, 1)?;
        
        let addr = (self.base_addr + offset as u64) as *mut u8;
        unsafe { write_volatile(addr, value) };
        Ok(())
    }
    
    /// Write 16-bit value to offset
    pub fn write_u16(&self, offset: usize, value: u16) -> DriverResult<()> {
        if !self.permissions.contains(MmioPermissions::WRITE) {
            return Err(DriverError::MemoryError);
        }
        
        self.check_bounds(offset, 2)?;
        
        let addr = (self.base_addr + offset as u64) as *mut u16;
        unsafe { write_volatile(addr, value) };
        Ok(())
    }
    
    /// Write 32-bit value to offset
    pub fn write_u32(&self, offset: usize, value: u32) -> DriverResult<()> {
        if !self.permissions.contains(MmioPermissions::WRITE) {
            return Err(DriverError::MemoryError);
        }
        
        self.check_bounds(offset, 4)?;
        
        let addr = (self.base_addr + offset as u64) as *mut u32;
        unsafe { write_volatile(addr, value) };
        Ok(())
    }
    
    /// Write 64-bit value to offset
    pub fn write_u64(&self, offset: usize, value: u64) -> DriverResult<()> {
        if !self.permissions.contains(MmioPermissions::WRITE) {
            return Err(DriverError::MemoryError);
        }
        
        self.check_bounds(offset, 8)?;
        
        let addr = (self.base_addr + offset as u64) as *mut u64;
        unsafe { write_volatile(addr, value) };
        Ok(())
    }
    
    /// Read buffer from MMIO region
    pub fn read_buffer(&self, offset: usize, buffer: &mut [u8]) -> DriverResult<()> {
        if !self.permissions.contains(MmioPermissions::READ) {
            return Err(DriverError::MemoryError);
        }
        
        self.check_bounds(offset, buffer.len())?;
        
        for (i, byte) in buffer.iter_mut().enumerate() {
            let addr = (self.base_addr + (offset + i) as u64) as *const u8;
            *byte = unsafe { read_volatile(addr) };
        }
        
        Ok(())
    }
    
    /// Write buffer to MMIO region
    pub fn write_buffer(&self, offset: usize, buffer: &[u8]) -> DriverResult<()> {
        if !self.permissions.contains(MmioPermissions::WRITE) {
            return Err(DriverError::MemoryError);
        }
        
        self.check_bounds(offset, buffer.len())?;
        
        for (i, &byte) in buffer.iter().enumerate() {
            let addr = (self.base_addr + (offset + i) as u64) as *mut u8;
            unsafe { write_volatile(addr, byte) };
        }
        
        Ok(())
    }
    
    /// Memory barrier (ensure all previous MMIO operations complete)
    pub fn memory_barrier(&self) {
        // Use architecture-specific memory barrier
        // For x86_64, we use mfence instruction
        // For other architectures, fall back to atomic fence
        
        #[cfg(target_arch = "x86_64")]
        unsafe {
            core::arch::asm!("mfence");
        }
        
        #[cfg(not(target_arch = "x86_64"))]
        {
            core::sync::atomic::fence(core::sync::atomic::Ordering::SeqCst);
        }
    }
    
    /// Get base address
    pub fn base_addr(&self) -> u64 {
        self.base_addr
    }
    
    /// Get region size
    pub fn size(&self) -> usize {
        self.size
    }
    
    /// Get permissions
    pub fn permissions(&self) -> MmioPermissions {
        self.permissions.clone()
    }
}

/// Typed MMIO register accessor
pub struct MmioRegister<T> {
    /// MMIO accessor
    accessor: MmioAccessor,
    /// Register offset
    offset: usize,
    /// Phantom data for type safety
    _phantom: PhantomData<T>,
}

impl<T> MmioRegister<T> {
    /// Create a new typed register accessor
    pub fn new(accessor: MmioAccessor, offset: usize) -> Self {
        Self {
            accessor,
            offset,
            _phantom: PhantomData,
        }
    }
}

impl MmioRegister<u8> {
    /// Read register value
    pub fn read(&self) -> DriverResult<u8> {
        self.accessor.read_u8(self.offset)
    }
    
    /// Write register value
    pub fn write(&self, value: u8) -> DriverResult<()> {
        self.accessor.write_u8(self.offset, value)
    }
}

impl MmioRegister<u16> {
    /// Read register value
    pub fn read(&self) -> DriverResult<u16> {
        self.accessor.read_u16(self.offset)
    }
    
    /// Write register value
    pub fn write(&self, value: u16) -> DriverResult<()> {
        self.accessor.write_u16(self.offset, value)
    }
}

impl MmioRegister<u32> {
    /// Read register value
    pub fn read(&self) -> DriverResult<u32> {
        self.accessor.read_u32(self.offset)
    }
    
    /// Write register value
    pub fn write(&self, value: u32) -> DriverResult<()> {
        self.accessor.write_u32(self.offset, value)
    }
    
    /// Update register with mask
    pub fn update<F>(&self, f: F) -> DriverResult<()> 
    where
        F: FnOnce(u32) -> u32
    {
        let current = self.read()?;
        let new_value = f(current);
        self.write(new_value)
    }
    
    /// Set specific bits
    pub fn set_bits(&self, mask: u32) -> DriverResult<()> {
        self.update(|v| v | mask)
    }
    
    /// Clear specific bits
    pub fn clear_bits(&self, mask: u32) -> DriverResult<()> {
        self.update(|v| v & !mask)
    }
    
    /// Toggle specific bits
    pub fn toggle_bits(&self, mask: u32) -> DriverResult<()> {
        self.update(|v| v ^ mask)
    }
}

impl MmioRegister<u64> {
    /// Read register value
    pub fn read(&self) -> DriverResult<u64> {
        self.accessor.read_u64(self.offset)
    }
    
    /// Write register value
    pub fn write(&self, value: u64) -> DriverResult<()> {
        self.accessor.write_u64(self.offset, value)
    }
}

/// Helper macros for common MMIO patterns
#[macro_export]
macro_rules! mmio_register {
    ($accessor:expr, $offset:expr, $type:ty) => {
        MmioRegister::<$type>::new($accessor, $offset)
    };
}

#[macro_export]
macro_rules! mmio_read {
    ($accessor:expr, $offset:expr, u8) => {
        $accessor.read_u8($offset)
    };
    ($accessor:expr, $offset:expr, u16) => {
        $accessor.read_u16($offset)
    };
    ($accessor:expr, $offset:expr, u32) => {
        $accessor.read_u32($offset)
    };
    ($accessor:expr, $offset:expr, u64) => {
        $accessor.read_u64($offset)
    };
}

#[macro_export]
macro_rules! mmio_write {
    ($accessor:expr, $offset:expr, $value:expr, u8) => {
        $accessor.write_u8($offset, $value)
    };
    ($accessor:expr, $offset:expr, $value:expr, u16) => {
        $accessor.write_u16($offset, $value)
    };
    ($accessor:expr, $offset:expr, $value:expr, u32) => {
        $accessor.write_u32($offset, $value)
    };
    ($accessor:expr, $offset:expr, $value:expr, u64) => {
        $accessor.write_u64($offset, $value)
    };
}
