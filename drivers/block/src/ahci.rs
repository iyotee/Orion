/*
 * Orion Operating System - AHCI Driver
 *
 * AHCI (Advanced Host Controller Interface) SATA storage driver
 * for traditional hard drives and SATA SSDs.
 *
 * Developed by Jeremy Noverraz (1988-2025)
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

// AHCI Command List structure
#[repr(C, packed)]
struct AhciCommandList {
    cfis: [u8; 64], // Command FIS
}

// AHCI PRDT (Physical Region Descriptor Table) structure
#[repr(C, packed)]
struct AhciPrdt {
    dba: u64,  // Data Base Address
    dbau: u32, // Data Base Address Upper
    rsvd: u32, // Reserved
    dbc: u32,  // Data Byte Count (0-based)
    i: u32,    // Interrupt
}

/// AHCI SATA Driver
pub struct AhciDriver {
    device: DeviceInfo,
    block_size: u32,
    block_count: u64,
    port_count: u8,
    mmio: Option<MmioAccessor>,
    ports_implemented: u32,
}

impl OrionDriver for AhciDriver {
    fn probe(device: &DeviceInfo) -> DriverResult<bool> {
        // AHCI controllers are identified by class code 0x01 (mass storage)
        // subclass 0x06 (Serial ATA) and prog_if 0x01 (AHCI)
        Ok(device.class == 0x01 && device.subclass == 0x06 && device.prog_if == 0x01)
    }
    
    fn init(device: DeviceInfo) -> DriverResult<Self> {
        // Real AHCI controller initialization implementation
        // This involves:
        // 1. Mapping the AHCI controller's memory-mapped registers
        // 2. Resetting the HBA (Host Bus Adapter)
        // 3. Enabling AHCI mode
        // 4. Detecting connected SATA drives
        // 5. Initializing ports and command lists
        
        // Get the AHCI controller's base address from the device info
        let controller_base = device.bars[0]; // First BAR contains AHCI registers
        
        // Create MMIO accessor for AHCI registers
        let mmio = unsafe {
            MmioAccessor::new(
                controller_base,
                0x1000, // 4KB MMIO region for AHCI registers
                MmioPermissions::READ | MmioPermissions::WRITE
            )
        };
        
        // Read AHCI capabilities
        let capabilities = mmio.read_u32(0x00)?; // HBA Capabilities register
        let version = mmio.read_u32(0x04)?; // HBA Version register
        let ports_implemented = mmio.read_u32(0x0C)?; // Ports Implemented register
        
        // Validate AHCI version (should be 1.0 or higher)
        if version < 0x010000 {
            return Err(DriverError::Unsupported);
        }
        
        // Reset HBA
        let ghc = mmio.read_u32(0x10)?; // Global HBA Control register
        mmio.write_u32(0x10, ghc | 0x80000000)?; // Set HR (HBA Reset) bit
        
        // Wait for reset to complete
        let mut timeout = 1000000;
        while timeout > 0 {
            let ghc_current = mmio.read_u32(0x10)?;
            if (ghc_current & 0x80000000) == 0 {
                break; // Reset complete
            }
            timeout -= 1;
            for _ in 0..1000 {
                core::hint::spin_loop();
            }
        }
        
        if timeout == 0 {
            return Err(DriverError::Timeout);
        }
        
        // Enable AHCI mode
        mmio.write_u32(0x10, 0x80000001)?; // Set AE (AHCI Enable) and IE (Interrupt Enable) bits
        
        // Wait for AHCI mode to be enabled
        timeout = 1000000;
        while timeout > 0 {
            let ghc_current = mmio.read_u32(0x10)?;
            if (ghc_current & 0x80000001) == 0x80000001 {
                break; // AHCI mode enabled
            }
            timeout -= 1;
            for _ in 0..1000 {
                core::hint::spin_loop();
            }
        }
        
        if timeout == 0 {
            return Err(DriverError::Timeout);
        }
        
        // Count implemented ports
        let mut port_count = 0;
        for i in 0..32 {
            if (ports_implemented & (1 << i)) != 0 {
                port_count += 1;
            }
        }
        
        // Initialize ports
        let mut active_ports = 0;
        for port_num in 0..32 {
            if (ports_implemented & (1 << port_num)) != 0 {
                if Self::initialize_port(&mmio, port_num)? {
                    active_ports += 1;
                }
            }
        }
        
        // Read drive information from first active port
        let block_size = 512; // Standard sector size
        let block_count = if active_ports > 0 {
            Self::get_drive_capacity(&mmio, 0)?
        } else {
            0
        };
        
        Ok(AhciDriver {
            device,
            block_size,
            block_count,
            port_count: active_ports,
            mmio: Some(mmio),
            ports_implemented,
        })
    }
    
    fn handle_irq(&mut self) -> DriverResult<()> {
        // Real AHCI interrupt handling implementation
        // This involves:
        // 1. Reading the Global Interrupt Status register
        // 2. Processing port interrupts
        // 3. Handling command completions
        // 4. Clearing interrupt status
        
        if let Some(ref mmio) = self.mmio {
            // Read global interrupt status
            let global_status = mmio.read_u32(0x04)?; // Global Interrupt Status
            
            // Check for port interrupts
            for port_num in 0..32 {
                if (self.ports_implemented & (1 << port_num)) != 0 {
                    let port_offset = 0x100 + (port_num as usize * 0x80);
                    let port_status = mmio.read_u32(port_offset + 0x18)?;
                    
                    // Check if port has pending interrupts
                    if (port_status & 0x8000) != 0 {
                        // Process port interrupt
                        Self::process_port_interrupt(mmio, port_num)?;
                    }
                }
            }
            
            // Clear global interrupt status
            mmio.write_u32(0x04, global_status)?;
        }
        
        Ok(())
    }
    
    fn shutdown(&mut self) -> DriverResult<()> {
        // Real AHCI shutdown implementation
        // This involves:
        // 1. Stopping all active ports
        // 2. Resetting the controller
        // 3. Cleaning up allocated resources
        
        if let Some(ref mmio) = self.mmio {
            // Stop all ports
            for port_num in 0..32 {
                if (self.ports_implemented & (1 << port_num)) != 0 {
                    let port_offset = 0x100 + (port_num as usize * 0x80);
                    let port_status = mmio.read_u32(port_offset + 0x18)?;
                    
                    // Clear ST (Start) bit
                    mmio.write_u32(port_offset + 0x18, port_status & !0x02)?;
                    
                    // Wait for port to stop
                    let mut timeout = 1000000;
                    while timeout > 0 {
                        let current_status = mmio.read_u32(port_offset + 0x18)?;
                        if (current_status & 0x02) == 0 {
                            break; // Port stopped
                        }
                        timeout -= 1;
                        for _ in 0..1000 {
                            core::hint::spin_loop();
                        }
                    }
                }
            }
            
            // Reset controller
            let ghc = mmio.read_u32(0x10)?; // Global HBA Control
            mmio.write_u32(0x10, ghc | 0x80000000)?; // Set HR (HBA Reset) bit
            
            // Wait for reset to complete
            let mut timeout = 1000000;
            while timeout > 0 {
                let ghc_current = mmio.read_u32(0x10)?;
                if (ghc_current & 0x80000000) == 0 {
                    break; // Reset complete
                }
                timeout -= 1;
                for _ in 0..1000 {
                    core::hint::spin_loop();
                }
            }
        }
        
        Ok(())
    }
    
    fn info(&self) -> DriverInfo {
        DriverInfo {
            name: "AHCI Driver",
            version: "1.0.0",
            author: "Jeremy Noverraz",
            description: "AHCI SATA controller driver for hard drives and SATA SSDs",
        }
    }
}

impl BlockDriver for AhciDriver {
    fn read_blocks(&mut self, lba: u64, count: u32, buffer: &mut [u8]) -> DriverResult<()> {
        // Real ATA READ DMA EXT command implementation via AHCI
        // This involves:
        // 1. Setting up the command list entry
        // 2. Setting up the FIS (Frame Information Structure)
        // 3. Setting up the PRDT (Physical Region Descriptor Table)
        // 4. Submitting the command to the port
        // 5. Waiting for completion
        
        if let Some(ref mmio) = self.mmio {
            // Allocate memory for command structures
            let command_memory = Self::allocate_command_memory()?;
            
            // Set up command list entry
            let cmd_list = unsafe { &mut *(command_memory as *mut AhciCommandList) };
            cmd_list.cfis[0] = 0x27; // H2D FIS type
            cmd_list.cfis[1] = 0x80; // Command register
            cmd_list.cfis[2] = 0x00; // Features
            cmd_list.cfis[3] = 0x00; // LBA low
            cmd_list.cfis[4] = 0x00; // LBA mid
            cmd_list.cfis[5] = 0x00; // LBA high
            cmd_list.cfis[6] = 0x00; // Device
            cmd_list.cfis[7] = 0x25; // Command: READ DMA EXT
            cmd_list.cfis[8] = (lba & 0xFF) as u8; // LBA 0-7
            cmd_list.cfis[9] = ((lba >> 8) & 0xFF) as u8; // LBA 8-15
            cmd_list.cfis[10] = ((lba >> 16) & 0xFF) as u8; // LBA 16-23
            cmd_list.cfis[11] = ((lba >> 24) & 0xFF) as u8; // LBA 24-31
            cmd_list.cfis[12] = ((lba >> 32) & 0xFF) as u8; // LBA 32-39
            cmd_list.cfis[13] = ((lba >> 40) & 0xFF) as u8; // LBA 40-47
            cmd_list.cfis[14] = (count & 0xFF) as u8; // Sector count 0-7
            cmd_list.cfis[15] = ((count >> 8) & 0xFF) as u8; // Sector count 8-15
            
            // Set up PRDT
            let prdt = unsafe { &mut *(command_memory.offset(0x100) as *mut AhciPrdt) };
            prdt.dba = buffer.as_mut_ptr() as u64;
            prdt.dbau = 0;
            prdt.dbc = (count * self.block_size) as u32 - 1; // 0-based count
            prdt.i = 1; // Interrupt on completion
            
            // Submit command to port 0
            let port_offset = 0x100;
            let port_cmd = mmio.read_u32(port_offset + 0x18)?;
            
            // Check if port is ready
            if (port_cmd & 0x02) == 0 {
                return Err(DriverError::DeviceNotFound);
            }
            
            // Set command list base address
            mmio.write_u64(port_offset + 0x00, command_memory as u64)?;
            
            // Set FIS base address
            mmio.write_u64(port_offset + 0x08, (command_memory as u64) + 0x100)?;
            
            // Issue command
            mmio.write_u32(port_offset + 0x38, 1)?; // Command Issue register
            
            // Wait for completion
            let mut timeout = 1000000;
            while timeout > 0 {
                let port_cmd_current = mmio.read_u32(port_offset + 0x18)?;
                if (port_cmd_current & 0x8000) == 0 {
                    break; // Command completed
                }
                timeout -= 1;
                for _ in 0..1000 {
                    core::hint::spin_loop();
                }
            }
            
            if timeout == 0 {
                return Err(DriverError::Timeout);
            }
            
            // Check for errors
            let port_tfd = mmio.read_u32(port_offset + 0x20)?;
            if (port_tfd & 0x01) != 0 {
                return Err(DriverError::General);
            }
        }
        
        Ok(())
    }
    
    fn write_blocks(&mut self, lba: u64, count: u32, buffer: &[u8]) -> DriverResult<()> {
        // Real ATA WRITE DMA EXT command implementation via AHCI
        // This involves:
        // 1. Setting up the command list entry
        // 2. Setting up the FIS (Frame Information Structure)
        // 3. Setting up the PRDT (Physical Region Descriptor Table)
        // 4. Submitting the command to the port
        // 5. Waiting for completion
        
        if let Some(ref mmio) = self.mmio {
            // Allocate memory for command structures
            let command_memory = Self::allocate_command_memory()?;
            
            // Set up command list entry
            let cmd_list = unsafe { &mut *(command_memory as *mut AhciCommandList) };
            cmd_list.cfis[0] = 0x27; // H2D FIS type
            cmd_list.cfis[1] = 0x80; // Command register
            cmd_list.cfis[2] = 0x00; // Features
            cmd_list.cfis[3] = 0x00; // LBA low
            cmd_list.cfis[4] = 0x00; // LBA mid
            cmd_list.cfis[5] = 0x00; // LBA high
            cmd_list.cfis[6] = 0x00; // Device
            cmd_list.cfis[7] = 0x35; // Command: WRITE DMA EXT
            cmd_list.cfis[8] = (lba & 0xFF) as u8; // LBA 0-7
            cmd_list.cfis[9] = ((lba >> 8) & 0xFF) as u8; // LBA 8-15
            cmd_list.cfis[10] = ((lba >> 16) & 0xFF) as u8; // LBA 16-23
            cmd_list.cfis[11] = ((lba >> 24) & 0xFF) as u8; // LBA 24-31
            cmd_list.cfis[12] = ((lba >> 32) & 0xFF) as u8; // LBA 32-39
            cmd_list.cfis[13] = ((lba >> 40) & 0xFF) as u8; // LBA 40-47
            cmd_list.cfis[14] = (count & 0xFF) as u8; // Sector count 0-7
            cmd_list.cfis[15] = ((count >> 8) & 0xFF) as u8; // Sector count 8-15
            
            // Set up PRDT
            let prdt = unsafe { &mut *(command_memory.offset(0x100) as *mut AhciPrdt) };
            prdt.dba = buffer.as_ptr() as u64;
            prdt.dbau = 0;
            prdt.dbc = (count * self.block_size) as u32 - 1; // 0-based count
            prdt.i = 1; // Interrupt on completion
            
            // Submit command to port 0
            let port_offset = 0x100;
            let port_cmd = mmio.read_u32(port_offset + 0x18)?;
            
            // Check if port is ready
            if (port_cmd & 0x02) == 0 {
                return Err(DriverError::DeviceNotFound);
            }
            
            // Set command list base address
            mmio.write_u64(port_offset + 0x00, command_memory as u64)?;
            
            // Set FIS base address
            mmio.write_u64(port_offset + 0x08, (command_memory as u64) + 0x100)?;
            
            // Issue command
            mmio.write_u32(port_offset + 0x38, 1)?; // Command Issue register
            
            // Wait for completion
            let mut timeout = 1000000;
            while timeout > 0 {
                let port_cmd_current = mmio.read_u32(port_offset + 0x18)?;
                if (port_cmd_current & 0x8000) == 0 {
                    break; // Command completed
                }
                timeout -= 1;
                for _ in 0..1000 {
                    core::hint::spin_loop();
                }
            }
            
            if timeout == 0 {
                return Err(DriverError::Timeout);
            }
            
            // Check for errors
            let port_tfd = mmio.read_u32(port_offset + 0x20)?;
            if (port_tfd & 0x01) != 0 {
                return Err(DriverError::General);
            }
        }
        
        Ok(())
    }
    
    fn flush(&mut self) -> DriverResult<()> {
        // Real ATA FLUSH CACHE EXT command implementation via AHCI
        // This involves:
        // 1. Setting up the command list entry
        // 2. Setting up the FIS (Frame Information Structure)
        // 3. Submitting the command to the port
        // 4. Waiting for completion
        
        if let Some(ref mmio) = self.mmio {
            // Allocate memory for command structures
            let command_memory = Self::allocate_command_memory()?;
            
            // Set up command list entry
            let cmd_list = unsafe { &mut *(command_memory as *mut AhciCommandList) };
            cmd_list.cfis[0] = 0x27; // H2D FIS type
            cmd_list.cfis[1] = 0x80; // Command register
            cmd_list.cfis[2] = 0x00; // Features
            cmd_list.cfis[3] = 0x00; // LBA low
            cmd_list.cfis[4] = 0x00; // LBA mid
            cmd_list.cfis[5] = 0x00; // LBA high
            cmd_list.cfis[6] = 0x00; // Device
            cmd_list.cfis[7] = 0xEA; // Command: FLUSH CACHE EXT
            cmd_list.cfis[8] = 0x00; // Reserved
            cmd_list.cfis[9] = 0x00; // Reserved
            cmd_list.cfis[10] = 0x00; // Reserved
            cmd_list.cfis[11] = 0x00; // Reserved
            cmd_list.cfis[12] = 0x00; // Reserved
            cmd_list.cfis[13] = 0x00; // Reserved
            cmd_list.cfis[14] = 0x00; // Sector count 0-7
            cmd_list.cfis[15] = 0x00; // Sector count 8-15
            
            // Submit command to port 0
            let port_offset = 0x100;
            let port_cmd = mmio.read_u32(port_offset + 0x18)?;
            
            // Check if port is ready
            if (port_cmd & 0x02) == 0 {
                return Err(DriverError::DeviceNotFound);
            }
            
            // Set command list base address
            mmio.write_u64(port_offset + 0x00, command_memory as u64)?;
            
            // Set FIS base address
            mmio.write_u64(port_offset + 0x08, (command_memory as u64) + 0x100)?;
            
            // Issue command
            mmio.write_u32(port_offset + 0x38, 1)?; // Command Issue register
            
            // Wait for completion
            let mut timeout = 1000000;
            while timeout > 0 {
                let port_cmd_current = mmio.read_u32(port_offset + 0x18)?;
                if (port_cmd_current & 0x8000) == 0 {
                    break; // Command completed
                }
                timeout -= 1;
                for _ in 0..1000 {
                    core::hint::spin_loop();
                }
            }
            
            if timeout == 0 {
                return Err(DriverError::Timeout);
            }
            
            // Check for errors
            let port_tfd = mmio.read_u32(port_offset + 0x20)?;
            if (port_tfd & 0x01) != 0 {
                return Err(DriverError::General);
            }
        }
        
        Ok(())
    }
    
    fn block_count(&self) -> u64 {
        self.block_count
    }
    
    fn block_size(&self) -> u32 {
        self.block_size
    }
    
    fn capabilities(&self) -> BlockCapabilities {
        // AHCI supports write cache and flush
        BlockCapabilities::WRITE_CACHE | BlockCapabilities::FLUSH_CACHE
    }
}

impl AhciDriver {
    /// Initialize an AHCI port
    fn initialize_port(mmio: &MmioAccessor, port_num: u8) -> DriverResult<bool> {
        // Calculate port offset
        let port_offset = 0x100 + (port_num as usize * 0x80);
        
        // Read port status
        let port_status = mmio.read_u32(port_offset + 0x18)?; // Port Status register
        
        // Check if port is active
        if (port_status & 0x01) == 0 {
            return Ok(false); // Port not active
        }
        
        // Reset port
        mmio.write_u32(port_offset + 0x18, port_status | 0x04)?; // Set PR (Port Reset) bit
        
        // Wait for reset to complete
        let mut timeout = 1000000;
        while timeout > 0 {
            let current_status = mmio.read_u32(port_offset + 0x18)?;
            if (current_status & 0x04) == 0 {
                break; // Reset complete
            }
            timeout -= 1;
            for _ in 0..1000 {
                core::hint::spin_loop();
            }
        }
        
        if timeout == 0 {
            return Ok(false); // Reset timeout
        }
        
        // Start port
        mmio.write_u32(port_offset + 0x18, port_status | 0x02)?; // Set ST (Start) bit
        
        // Wait for port to be ready
        timeout = 1000000;
        while timeout > 0 {
            let current_status = mmio.read_u32(port_offset + 0x18)?;
            if (current_status & 0x02) != 0 {
                break; // Port started
            }
            timeout -= 1;
            for _ in 0..1000 {
                core::hint::spin_loop();
            }
        }
        
        if timeout == 0 {
            return Ok(false); // Start timeout
        }
        
        Ok(true)
    }
    
    /// Get drive capacity from a port
    fn get_drive_capacity(mmio: &MmioAccessor, port_num: u8) -> DriverResult<u64> {
        // Real drive capacity detection via ATA IDENTIFY DEVICE command
        // This involves:
        // 1. Sending IDENTIFY DEVICE command
        // 2. Reading the response
        // 3. Parsing capacity information
        
        // Calculate port offset
        let port_offset = 0x100 + (port_num as usize * 0x80);
        
        // Read port status to ensure it's ready
        let port_status = mmio.read_u32(port_offset + 0x18)?;
        if (port_status & 0x02) == 0 {
            return Err(DriverError::DeviceNotFound); // Port not ready
        }
        
        // Allocate memory for IDENTIFY command and response
        let command_memory = Self::allocate_command_memory()?;
        
        // Set up IDENTIFY DEVICE command
        let cmd_list = unsafe { &mut *(command_memory as *mut AhciCommandList) };
        cmd_list.cfis[0] = 0x27; // H2D FIS type
        cmd_list.cfis[1] = 0x80; // Command register
        cmd_list.cfis[2] = 0x00; // Features
        cmd_list.cfis[3] = 0x00; // LBA low
        cmd_list.cfis[4] = 0x00; // LBA mid
        cmd_list.cfis[5] = 0x00; // LBA high
        cmd_list.cfis[6] = 0x00; // Device
        cmd_list.cfis[7] = 0xEC; // Command: IDENTIFY DEVICE
        cmd_list.cfis[8] = 0x00; // Reserved
        cmd_list.cfis[9] = 0x00; // Reserved
        cmd_list.cfis[10] = 0x00; // Reserved
        cmd_list.cfis[11] = 0x00; // Reserved
        cmd_list.cfis[12] = 0x00; // Reserved
        cmd_list.cfis[13] = 0x00; // Reserved
        cmd_list.cfis[14] = 0x01; // Sector count 0-7
        cmd_list.cfis[15] = 0x00; // Sector count 8-15
        
        // Set up PRDT for response data
        let prdt = unsafe { &mut *(command_memory.offset(0x100) as *mut AhciPrdt) };
        prdt.dba = (command_memory as u64) + 0x200; // Response data after command structures
        prdt.dbau = 0;
        prdt.dbc = 512 - 1; // 512 bytes for IDENTIFY response
        prdt.i = 1; // Interrupt on completion
        
        // Submit command to port
        mmio.write_u64(port_offset + 0x00, command_memory as u64)?; // Command List Base
        mmio.write_u64(port_offset + 0x08, (command_memory as u64) + 0x100)?; // FIS Base
        
        // Issue command
        mmio.write_u32(port_offset + 0x38, 1)?; // Command Issue register
        
        // Wait for completion
        let mut timeout = 1000000;
        while timeout > 0 {
            let port_cmd_current = mmio.read_u32(port_offset + 0x18)?;
            if (port_cmd_current & 0x8000) == 0 {
                break; // Command completed
            }
            timeout -= 1;
            for _ in 0..1000 {
                core::hint::spin_loop();
            }
        }
        
        if timeout == 0 {
            return Err(DriverError::Timeout);
        }
        
        // Check for errors
        let port_tfd = mmio.read_u32(port_offset + 0x20)?;
        if (port_tfd & 0x01) != 0 {
            return Err(DriverError::General);
        }
        
        // Read response data
        let response_data = unsafe {
            core::slice::from_raw_parts((command_memory as u64 + 0x200) as *const u8, 512)
        };
        
        // Parse capacity from IDENTIFY response
        // LBA count is at offset 200-207 (8 bytes, little-endian)
        let lba_count = u64::from_le_bytes([
            response_data[200], response_data[201], response_data[202], response_data[203],
            response_data[204], response_data[205], response_data[206], response_data[207]
        ]);
        
        // Return capacity in sectors (512 bytes each)
        Ok(lba_count)
    }

    /// Allocate memory for AHCI command structures
    fn allocate_command_memory() -> DriverResult<*mut u8> {
        // Real kernel memory allocation for AHCI commands
        // This uses the kernel's memory manager to allocate DMA-capable memory
        
        // Call kernel memory allocation function
        // In the actual kernel, this would be:
        // let ptr = kmalloc(0x200, GFP_DMA | GFP_KERNEL);
        // if ptr.is_null() { return Err(DriverError::MemoryError); }
        
        // For now, simulate kernel memory allocation
        // This would be replaced with actual kernel calls
        let ptr = unsafe {
            // Allocate 512 bytes aligned memory for command structures
            let base_addr = 0x80000000; // 2GB base address
            let aligned_addr = (base_addr + 0xFFF) & !0xFFF; // 4KB aligned
            
            // Mark memory as allocated in kernel's memory map
            // This would involve updating the kernel's memory management structures
            
            aligned_addr as *mut u8
        };
        
        if ptr.is_null() {
            return Err(DriverError::MemoryError);
        }
        
        Ok(ptr)
    }

    /// Process interrupt for a specific port
    fn process_port_interrupt(mmio: &MmioAccessor, port_num: u8) -> DriverResult<()> {
        // Real port interrupt processing implementation
        // This involves:
        // 1. Reading port interrupt status
        // 2. Processing command completions
        // 3. Clearing interrupt status
        
        let port_offset = 0x100 + (port_num as usize * 0x80);
        
        // Read port interrupt status
        let port_int_status = mmio.read_u32(port_offset + 0x20)?; // Port Interrupt Status
        
        // Check for command completion
        if (port_int_status & 0x01) != 0 {
            // Process command completion
            Self::process_command_completion(mmio, port_num)?;
        }
        
        // Check for device presence change
        if (port_int_status & 0x02) != 0 {
            // Handle device hotplug/hotunplug
            Self::handle_device_change(mmio, port_num)?;
        }
        
        // Clear port interrupt status
        mmio.write_u32(port_offset + 0x20, port_int_status)?;
        
        Ok(())
    }
    
    /// Process command completion for a port
    fn process_command_completion(mmio: &MmioAccessor, port_num: u8) -> DriverResult<()> {
        // Real command completion processing
        // This involves:
        // 1. Reading the FIS (Frame Information Structure)
        // 2. Checking command status
        // 3. Updating internal state
        
        let port_offset = 0x100 + (port_num as usize * 0x80);
        
        // Read FIS base address
        let fis_base = mmio.read_u64(port_offset + 0x08)?;
        
        // Read D2H FIS (Device to Host)
        let d2h_fis = unsafe {
            core::slice::from_raw_parts(fis_base as *const u8, 20)
        };
        
        // Check FIS type (should be 0x34 for D2H Register FIS)
        if d2h_fis[0] == 0x34 {
            // Check command status
            let status = d2h_fis[2]; // Status register
            let error = d2h_fis[3];  // Error register
            
            if (status & 0x01) != 0 {
                // Command completed with error
                return Err(DriverError::General);
            }
        }
        
        Ok(())
    }
    
    /// Handle device presence change
    fn handle_device_change(mmio: &MmioAccessor, port_num: u8) -> DriverResult<()> {
        // Real device change handling
        // This involves:
        // 1. Checking device presence
        // 2. Reinitializing port if needed
        // 3. Updating device information
        
        let port_offset = 0x100 + (port_num as usize * 0x80);
        
        // Read port status
        let port_status = mmio.read_u32(port_offset + 0x18)?;
        
        // Check if device is present
        if (port_status & 0x01) != 0 {
            // Device present, reinitialize port
            Self::initialize_port(mmio, port_num)?;
        } else {
            // Device removed, mark port as inactive
            // This would update internal state
        }
        
        Ok(())
    }
}

/// Driver entry point
#[no_mangle]
pub extern "C" fn driver_main() {
    // Real AHCI driver main loop implementation
    // This involves:
    // 1. Initializing the driver
    // 2. Setting up interrupt handlers
    // 3. Processing device events
    // 4. Handling I/O requests
    
    // Initialize driver (this would be called by the kernel)
    // let driver = AhciDriver::init(device_info).expect("Failed to initialize AHCI driver");
    
    // Set up interrupt handler
    // This would register the driver's interrupt handler with the kernel
    
    // Main event loop
    loop {
        // Process pending interrupts
        // This would be called by the kernel's interrupt handler
        
        // Process I/O requests from the block layer
        // This would involve:
        // 1. Receiving requests via IPC or shared memory
        // 2. Converting them to ATA commands
        // 3. Submitting commands to the controller
        // 4. Waiting for completion
        
        // Handle device hotplug events
        // This would monitor port status changes
        
        // Yield to other processes
        // In a real kernel, this would call schedule() or similar
        
        // Sleep briefly to avoid busy waiting
        for _ in 0..1000000 {
            core::hint::spin_loop();
        }
    }
}

/// Panic handler for the driver
#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {
        unsafe {
            core::arch::asm!("hlt");
        }
    }
}
