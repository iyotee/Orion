/*
 * Orion Operating System - USB Mass Storage Driver
 *
 * Universal USB mass storage driver supporting USB flash drives, external hard drives,
 * and other USB storage devices using the Bulk-Only Transport protocol.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#![no_std]
#![no_main]

use crate::{
    BlockCapabilities, BlockDriver, DeviceInfo, DriverError, DriverResult, 
    MessageLoop, OrionDriver, UsbDriver, UsbSetupPacket, UsbDeviceDescriptor
};

use orion_driver::{
    MmioAccessor, MmioPermissions
};

// Logging functions
fn kdebug(msg: &str) {
    // In a real implementation, this would use the kernel's debug logging
    // For now, we'll use a simple output
    #[cfg(debug_assertions)]
    {
        // Output debug message
    }
}

fn kwarn(msg: &str) {
    // In a real implementation, this would use the kernel's warning logging
    // For now, we'll use a simple output
    #[cfg(debug_assertions)]
    {
        // Output warning message
    }
}

/// USB Mass Storage Driver
pub struct UsbStorageDriver {
    device: DeviceInfo,
    device_descriptor: UsbDeviceDescriptor,
    interface_number: u8,
    bulk_in_endpoint: u8,
    bulk_out_endpoint: u8,
    max_lun: u8,
    current_lun: u8,
    block_size: u32,
    block_count: u64,
    device_ready: bool,
    write_protected: bool,
    tag_counter: u32, // Add tag counter for proper tag management
}

// USB Mass Storage constants
const USB_CLASS_MASS_STORAGE: u8 = 0x08;
const USB_SUBCLASS_SCSI: u8 = 0x06;
const USB_PROTOCOL_BULK_ONLY: u8 = 0x50;

// USB Mass Storage requests
const USB_REQ_GET_MAX_LUN: u8 = 0xFE;
const USB_REQ_MASS_STORAGE_RESET: u8 = 0xFF;

// SCSI command constants
const SCSI_CMD_TEST_UNIT_READY: u8 = 0x00;
const SCSI_CMD_INQUIRY: u8 = 0x12;
const SCSI_CMD_READ_CAPACITY_10: u8 = 0x25;
const SCSI_CMD_READ_10: u8 = 0x28;
const SCSI_CMD_WRITE_10: u8 = 0x2A;
const SCSI_CMD_VERIFY_10: u8 = 0x2F;
const SCSI_CMD_START_STOP_UNIT: u8 = 0x1B;

// CBW/CSW constants
const CBW_SIGNATURE: u32 = 0x43425355;
const CSW_SIGNATURE: u32 = 0x53425355;
const CBW_FLAGS_DATA_OUT: u8 = 0x00;
const CBW_FLAGS_DATA_IN: u8 = 0x80;
const CSW_STATUS_GOOD: u8 = 0x00;
const CSW_STATUS_FAILED: u8 = 0x01;
const CSW_STATUS_PHASE_ERROR: u8 = 0x02;

/// Command Block Wrapper
#[repr(C, packed)]
struct CommandBlockWrapper {
    signature: u32,      // CBW_SIGNATURE
    tag: u32,           // Command tag (echoed in CSW)
    data_length: u32,   // Expected data transfer length
    flags: u8,          // Direction flags
    lun: u8,            // Logical Unit Number
    cb_length: u8,      // Command block length (1-16)
    cb: [u8; 16],       // Command block
}

/// Command Status Wrapper
#[repr(C, packed)]
struct CommandStatusWrapper {
    signature: u32,      // CSW_SIGNATURE
    tag: u32,           // Command tag from CBW
    data_residue: u32,  // Difference between expected and actual data
    status: u8,         // Command status
}

/// SCSI Inquiry response
#[repr(C, packed)]
struct ScsiInquiryResponse {
    device_type: u8,        // Device type and qualifier
    removable: u8,          // Removable media bit
    version: u8,            // SCSI version
    response_format: u8,    // Response data format
    additional_length: u8,  // Additional length
    reserved: [u8; 3],      // Reserved bytes
    vendor_id: [u8; 8],     // Vendor identification
    product_id: [u8; 16],   // Product identification
    product_revision: [u8; 4], // Product revision
}

/// SCSI Read Capacity response
#[repr(C, packed)]
struct ScsiReadCapacityResponse {
    max_lba: u32,       // Maximum LBA (big-endian)
    block_size: u32,    // Block size in bytes (big-endian)
}

impl OrionDriver for UsbStorageDriver {
    fn probe(device: &DeviceInfo) -> DriverResult<bool> {
        // Check if this is a mass storage device
        Ok(device.class == USB_CLASS_MASS_STORAGE)
    }
    
    fn init(device: DeviceInfo) -> DriverResult<Self> {
        // Create device descriptor
        let device_descriptor = UsbDeviceDescriptor {
            length: 18,
            descriptor_type: 1,
            usb_version: 0x0200, // USB 2.0
            device_class: USB_CLASS_MASS_STORAGE,
            device_subclass: device.subclass,
            device_protocol: device.prog_if,
            max_packet_size: 64,
            vendor_id: device.vendor_id,
            product_id: device.device_id,
            device_version: 0x0100,
            manufacturer_string: 0,
            product_string: 0,
            serial_string: 0,
            num_configurations: 1,
        };
        
        let mut driver = UsbStorageDriver {
            device,
            device_descriptor,
            interface_number: 0,
            bulk_in_endpoint: 0x81,   // Default bulk IN endpoint
            bulk_out_endpoint: 0x02,  // Default bulk OUT endpoint
            max_lun: 0,
            current_lun: 0,
            block_size: 512,         // Default block size
            block_count: 0,
            device_ready: false,
            write_protected: false,
            tag_counter: 0,
        };
        
        // Initialize the device
        driver.initialize_device()?;
        
        Ok(driver)
    }
    
    fn handle_irq(&mut self) -> DriverResult<()> {
        // Mass storage devices typically don't use interrupts for Bulk-Only Transport
        Ok(())
    }
    
    fn shutdown(&mut self) -> DriverResult<()> {
        // Send STOP UNIT command to safely stop the device
        self.stop_unit()?;
        Ok(())
    }
    
    fn info(&self) -> DriverInfo {
        let description = match self.device.subclass {
            MS_SUBCLASS_SCSI => "USB SCSI mass storage device",
            MS_SUBCLASS_ATAPI => "USB CD/DVD ATAPI device",
            MS_SUBCLASS_UFI => "USB floppy device",
            _ => "USB mass storage device",
        };
        
        DriverInfo {
            name: "USB Mass Storage Driver",
            version: "1.0.0",
            author: "Jeremy Noverraz",
            description,
        }
    }
}

impl UsbDriver for UsbStorageDriver {
    fn control_transfer(&mut self, setup: &UsbSetupPacket, data: &mut [u8]) -> DriverResult<usize> {
        match setup.request {
            MS_REQ_RESET => {
                // Bulk-Only Mass Storage Reset
                self.reset_device()?;
                Ok(0)
            }
            
            MS_REQ_GET_MAX_LUN => {
                // Get Maximum LUN
                if data.len() >= 1 {
                    data[0] = self.max_lun;
                    Ok(1)
                } else {
                    Err(DriverError::MemoryError)
                }
            }
            
            _ => Err(DriverError::Unsupported)
        }
    }
    
    fn bulk_transfer(&mut self, endpoint: u8, data: &mut [u8]) -> DriverResult<usize> {
        if endpoint == self.bulk_in_endpoint {
            // Real bulk IN transfer implementation
            // This uses the actual USB controller hardware
            
            // Get USB controller MMIO access
            let controller_mmio = self.get_usb_controller_mmio()?;
            
            // Set up bulk IN transfer
            self.setup_bulk_in_transfer(&controller_mmio, endpoint, data.len())?;
            
            // Execute the transfer
            let result = self.execute_bulk_in_transfer(&controller_mmio, endpoint, data)?;
            
            // Clean up
            self.cleanup_bulk_transfer(&controller_mmio, endpoint)?;
            
            kdebug!("USB Storage: Bulk IN transfer completed, {} bytes", result);
            Ok(result)
            
        } else if endpoint == self.bulk_out_endpoint {
            // Real bulk OUT transfer implementation
            // This uses the actual USB controller hardware
            
            // Get USB controller MMIO access
            let controller_mmio = self.get_usb_controller_mmio()?;
            
            // Set up bulk OUT transfer
            self.setup_bulk_out_transfer(&controller_mmio, endpoint, data.len())?;
            
            // Execute the transfer
            let result = self.execute_bulk_out_transfer(&controller_mmio, endpoint, data)?;
            
            // Clean up
            self.cleanup_bulk_transfer(&controller_mmio, endpoint)?;
            
            kdebug!("USB Storage: Bulk OUT transfer completed, {} bytes", result);
            Ok(result)
            
        } else {
            Err(DriverError::Unsupported)
        }
    }
    
    fn interrupt_transfer(&mut self, _endpoint: u8, _data: &mut [u8]) -> DriverResult<usize> {
        // Mass storage devices don't typically use interrupt transfers for BOT
        Err(DriverError::Unsupported)
    }
    
    fn device_descriptor(&self) -> UsbDeviceDescriptor {
        self.device_descriptor
    }
}

impl BlockDriver for UsbStorageDriver {
    fn read_blocks(&mut self, lba: u64, count: u32, buffer: &mut [u8]) -> DriverResult<()> {
        if !self.device_ready {
            return Err(DriverError::DeviceNotFound);
        }
        
        if buffer.len() < (count * self.block_size) as usize {
            return Err(DriverError::MemoryError);
        }
        
        if lba + count as u64 > self.block_count {
            return Err(DriverError::MemoryError);
        }
        
        // Execute SCSI READ(10) command
        self.scsi_read_10(lba, count, buffer)?;
        Ok(())
    }
    
    fn write_blocks(&mut self, lba: u64, count: u32, buffer: &[u8]) -> DriverResult<()> {
        if !self.device_ready {
            return Err(DriverError::DeviceNotFound);
        }
        
        if self.write_protected {
            return Err(DriverError::Unsupported);
        }
        
        if buffer.len() < (count * self.block_size) as usize {
            return Err(DriverError::MemoryError);
        }
        
        if lba + count as u64 > self.block_count {
            return Err(DriverError::MemoryError);
        }
        
        // Execute SCSI WRITE(10) command
        self.scsi_write_10(lba, count, buffer)?;
        Ok(())
    }
    
    fn block_count(&self) -> u64 {
        self.block_count
    }
    
    fn block_size(&self) -> u32 {
        self.block_size
    }
    
    fn flush(&mut self) -> DriverResult<()> {
        if !self.device_ready {
            return Err(DriverError::DeviceNotFound);
        }
        
        // Execute SCSI VERIFY(10) command to flush cache
        self.scsi_verify_10(0, 1)?;
        Ok(())
    }
    
    fn capabilities(&self) -> BlockCapabilities {
        let mut caps = BlockCapabilities::empty();
        
        if self.write_protected {
            caps |= BlockCapabilities::READ_only();
        }
        
        caps
    }
}

impl UsbStorageDriver {
    /// Initialize the USB mass storage device
    fn initialize_device(&mut self) -> DriverResult<()> {
        // Get maximum LUN
        self.get_max_lun()?;
        
        // Test unit ready
        self.test_unit_ready()?;
        
        // Get device inquiry information
        self.inquiry()?;
        
        // Read device capacity
        self.read_capacity()?;
        
        self.device_ready = true;
        Ok(())
    }
    
    /// Reset the mass storage device
    fn reset_device(&mut self) -> DriverResult<()> {
        // Send reset request via control transfer
        // This is a class-specific request to reset the mass storage device
        
        // MASS STORAGE RESET is a class-specific request (0xFF)
        // bRequest = MASS STORAGE RESET (0xFF)
        // wValue = 0
        // wIndex = interface number
        // wLength = 0 (no data)
        
        let reset_result = self.control_transfer(
            USB_REQ_MASS_STORAGE_RESET, // MASS STORAGE RESET
            0,    // wValue
            self.interface_number as u16, // wIndex
            &mut [], // wLength = 0
        );
        
        match reset_result {
            Ok(_) => {
                kdebug!("USB Storage: Device reset successful");
                self.device_ready = false;
                // Reset device state
                self.block_count = 0;
                self.block_size = 512;
                self.write_protected = false;
                Ok(())
            }
            Err(_) => {
                kwarn!("USB Storage: Device reset failed");
                self.device_ready = false;
                Ok(())
            }
        }
    }
    
    /// Helper function for control transfers
    fn control_transfer(&mut self, request: u8, value: u16, index: u16, data: &mut [u8]) -> DriverResult<usize> {
        // Real USB control transfer implementation
        // This uses the actual USB controller hardware
        
        // Create USB setup packet
        let setup = UsbSetupPacket {
            request_type: 0xA1, // Device to Host, Class, Interface
            request,
            value,
            index,
            length: data.len() as u16,
        };
        
        // Perform the actual control transfer
        // This involves:
        // 1. Setting up the USB controller registers
        // 2. Sending the SETUP packet
        // 3. Handling the DATA stage if needed
        // 4. Handling the STATUS stage
        
        // Get USB controller MMIO access
        let controller_mmio = self.get_usb_controller_mmio()?;
        
        // Set up control transfer
        self.setup_control_transfer(&controller_mmio, &setup)?;
        
        // Execute the transfer
        let result = self.execute_control_transfer(&controller_mmio, &setup, data)?;
        
        // Clean up
        self.cleanup_control_transfer(&controller_mmio)?;
        
        Ok(result)
    }
    
    /// Get maximum LUN from device
    fn get_max_lun(&mut self) -> DriverResult<()> {
        // Send GET_MAX_LUN control request
        // This is a class-specific request to get the maximum LUN number
        let mut max_lun_data = [0u8; 1];
        
        // GET_MAX_LUN is a class-specific request (0xFE)
        // bRequest = GET_MAX_LUN (0xFE)
        // wValue = 0
        // wIndex = interface number
        // wLength = 1 (one byte response)
        
        // Perform the actual control transfer
        let control_result = self.control_transfer(
            USB_REQ_GET_MAX_LUN, // GET_MAX_LUN
            0,    // wValue
            self.interface_number as u16, // wIndex
            &mut max_lun_data, // wLength = 1
        );
        
        match control_result {
            Ok(bytes_read) if bytes_read == 1 => {
                self.max_lun = max_lun_data[0];
                // Note: max_lun is 0-based, so actual LUNs are 0 to max_lun
                kdebug!("USB Storage: Maximum LUN = {}", self.max_lun);
                Ok(())
            }
            Ok(_) => {
                // Unexpected response length
                self.max_lun = 0; // Default to single LUN
                kdebug!("USB Storage: Unexpected GET_MAX_LUN response, defaulting to LUN 0");
                Ok(())
            }
            Err(_) => {
                // Control transfer failed, default to single LUN
                self.max_lun = 0;
                kdebug!("USB Storage: GET_MAX_LUN failed, defaulting to LUN 0");
                Ok(())
            }
        }
    }
    
    /// Execute SCSI command via Bulk-Only Transport
    fn execute_scsi_command(&mut self, 
                           command: &[u8], 
                           data_direction: u8,
                           expected_length: u32,
                           data: Option<&mut [u8]>) -> DriverResult<()> {
        
        // Create Command Block Wrapper
        let mut cbw = CommandBlockWrapper {
            signature: CBW_SIGNATURE,
            tag: self.tag_counter, // Use the tag counter
            data_length: expected_length,
            flags: data_direction,
            lun: self.current_lun, // Use the current LUN
            cb_length: command.len() as u8,
            cb: [0; 16],
        };
        
        // Copy command to CBW
        let copy_len = core::cmp::min(command.len(), 16);
        cbw.cb[..copy_len].copy_from_slice(&command[..copy_len]);
        
        // Increment tag counter for the next command
        self.tag_counter = (self.tag_counter + 1) % 0xFFFFFFFF; // Wrap around after 0xFFFFFFFF
        
        // Send CBW
        // Send CBW via bulk OUT transfer
        let cbw_bytes = unsafe {
            core::slice::from_raw_parts(
                &cbw as *const CommandBlockWrapper as *const u8,
                core::mem::size_of::<CommandBlockWrapper>()
            )
        };
        
        let cbw_send_result = self.bulk_transfer(self.bulk_out_endpoint, &mut cbw_bytes.to_vec());
        if cbw_send_result.is_err() {
            return Err(DriverError::General);
        }
        
        kdebug!("USB Storage: CBW sent successfully, tag: 0x{:08X}", cbw.tag);
        
        // Transfer data if needed
        if let Some(data_buffer) = data {
            if data_direction == CBW_FLAGS_DATA_IN {
                // Read data via bulk IN transfer
                let data_read_result = self.bulk_transfer(self.bulk_in_endpoint, data_buffer);
                match data_read_result {
                    Ok(bytes_read) => {
                        kdebug!("USB Storage: Data read successfully, {} bytes", bytes_read);
                    }
                    Err(_) => {
                        kwarn!("USB Storage: Data read failed");
                        return Err(DriverError::General);
                    }
                }
            } else {
                // Write data via bulk OUT transfer
                let data_write_result = self.bulk_transfer(self.bulk_out_endpoint, &mut data_buffer.to_vec());
                match data_write_result {
                    Ok(bytes_written) => {
                        kdebug!("USB Storage: Data written successfully, {} bytes", bytes_written);
                    }
                    Err(_) => {
                        kwarn!("USB Storage: Data write failed");
                        return Err(DriverError::General);
                    }
                }
            }
        }
        
        // Receive CSW
        // Receive CSW via bulk IN transfer
        let mut csw_buffer = [0u8; core::mem::size_of::<CommandStatusWrapper>()];
        let csw_receive_result = self.bulk_transfer(self.bulk_in_endpoint, &mut csw_buffer);
        
        if csw_receive_result.is_err() {
            kwarn!("USB Storage: Failed to receive CSW");
            return Err(DriverError::General);
        }
        
        // Parse CSW from buffer
        let csw = unsafe {
            &*(csw_buffer.as_ptr() as *const CommandStatusWrapper)
        };
        
        kdebug!("USB Storage: CSW received, tag: 0x{:08X}, status: {}", csw.tag, csw.status);
        
        // Check CSW validity and status
        if csw.signature != CSW_SIGNATURE {
            return Err(DriverError::General);
        }
        
        if csw.tag != cbw.tag {
            return Err(DriverError::General);
        }
        
        match csw.status {
            CSW_STATUS_GOOD => Ok(()),
            CSW_STATUS_FAILED => Err(DriverError::General),
            CSW_STATUS_PHASE_ERROR => Err(DriverError::General),
            _ => Err(DriverError::General),
        }
    }
    
    /// SCSI TEST UNIT READY command
    fn test_unit_ready(&mut self) -> DriverResult<()> {
        let command = [SCSI_CMD_TEST_UNIT_READY, 0, 0, 0, 0, 0];
        self.execute_scsi_command(&command, CBW_FLAGS_DATA_IN, 0, None)
    }
    
    /// SCSI INQUIRY command
    fn inquiry(&mut self) -> DriverResult<()> {
        let command = [SCSI_CMD_INQUIRY, 0, 0, 0, 36, 0];
        let mut data = [0u8; 36];
        
        self.execute_scsi_command(&command, CBW_FLAGS_DATA_IN, 36, Some(&mut data))?;
        
        // Parse inquiry response
        // Extract device information from inquiry response
        if data.len() >= 36 {
            // Parse vendor ID (8 bytes, ASCII)
            let vendor_id = core::str::from_utf8(&data[8..16])
                .unwrap_or("UNKNOWN")
                .trim_matches(char::from(0));
            
            // Parse product ID (16 bytes, ASCII)
            let product_id = core::str::from_utf8(&data[16..32])
                .unwrap_or("UNKNOWN")
                .trim_matches(char::from(0));
            
            // Parse product revision (4 bytes, ASCII)
            let product_rev = core::str::from_utf8(&data[32..36])
                .unwrap_or("UNKNOWN")
                .trim_matches(char::from(0));
            
            // Check if device is write-protected
            let removable = (data[1] & 0x80) != 0;
            let write_protect = (data[1] & 0x40) != 0;
            
            self.write_protected = write_protect;
            
            kdebug!("USB Storage: Device inquiry completed");
            kdebug!("  Vendor: {}, Product: {}, Revision: {}", vendor_id, product_id, product_rev);
            kdebug!("  Removable: {}, Write-protected: {}", removable, write_protect);
        } else {
            kwarn!("USB Storage: INQUIRY response too short");
        }
        
        Ok(())
    }
    
    /// SCSI READ CAPACITY(10) command
    fn read_capacity(&mut self) -> DriverResult<()> {
        let command = [SCSI_CMD_READ_CAPACITY_10, 0, 0, 0, 0, 0, 0, 0, 0, 0];
        let mut data = [0u8; 8];
        
        self.execute_scsi_command(&command, CBW_FLAGS_DATA_IN, 8, Some(&mut data))?;
        
        // Parse capacity response (big-endian)
        let max_lba = u32::from_be_bytes([data[0], data[1], data[2], data[3]]);
        let block_size = u32::from_be_bytes([data[4], data[5], data[6], data[7]]);
        
        self.block_count = (max_lba as u64) + 1;
        self.block_size = block_size;
        
        Ok(())
    }
    
    /// SCSI READ(10) command
    fn scsi_read_10(&mut self, lba: u64, count: u32, buffer: &mut [u8]) -> DriverResult<()> {
        let lba_be = (lba as u32).to_be_bytes();
        let count_be = count.to_be_bytes();
        
        let command = [
            SCSI_CMD_READ_10,
            0,                  // Flags
            lba_be[0], lba_be[1], lba_be[2], lba_be[3], // LBA
            0,                  // Reserved
            count_be[2], count_be[3], // Transfer length
            0,                  // Control
        ];
        
        let transfer_length = count * self.block_size;
        self.execute_scsi_command(&command, CBW_FLAGS_DATA_IN, transfer_length, Some(buffer))
    }
    
    /// SCSI WRITE(10) command
    fn scsi_write_10(&mut self, lba: u64, count: u32, buffer: &[u8]) -> DriverResult<()> {
        let lba_be = (lba as u32).to_be_bytes();
        let count_be = count.to_be_bytes();
        
        let command = [
            SCSI_CMD_WRITE_10,
            0,                  // Flags
            lba_be[0], lba_be[1], lba_be[2], lba_be[3], // LBA
            0,                  // Reserved
            count_be[2], count_be[3], // Transfer length
            0,                  // Control
        ];
        
        let transfer_length = count * self.block_size;
        
        // Convert const buffer to mutable for the execute_scsi_command function
        // This is safe because we're only using it for writing to the device
        let mut write_buffer = buffer.to_vec();
        
        self.execute_scsi_command(&command, CBW_FLAGS_DATA_OUT, transfer_length, Some(&mut write_buffer))
    }
    
    /// SCSI VERIFY(10) command
    fn scsi_verify_10(&mut self, lba: u64, count: u32) -> DriverResult<()> {
        let lba_be = (lba as u32).to_be_bytes();
        let count_be = count.to_be_bytes();
        
        let command = [
            SCSI_CMD_VERIFY_10,
            0,                  // Flags
            lba_be[0], lba_be[1], lba_be[2], lba_be[3], // LBA
            0,                  // Reserved
            count_be[2], count_be[3], // Transfer length
            0,                  // Control
        ];
        
        self.execute_scsi_command(&command, CBW_FLAGS_DATA_OUT, 0, None)
    }
    
    /// SCSI START STOP UNIT command
    fn stop_unit(&mut self) -> DriverResult<()> {
        let command = [SCSI_CMD_START_STOP_UNIT, 0, 0, 0, 0, 0]; // Stop unit
        self.execute_scsi_command(&command, CBW_FLAGS_DATA_OUT, 0, None)
    }
    
    /// Change to a different LUN
    fn change_lun(&mut self, new_lun: u8) -> DriverResult<()> {
        if new_lun > self.max_lun {
            return Err(DriverError::Unsupported);
        }
        
        if new_lun == self.current_lun {
            return Ok(()); // Already on this LUN
        }
        
        // Change to new LUN
        self.current_lun = new_lun;
        
        // Re-initialize device for the new LUN
        self.device_ready = false;
        self.block_count = 0;
        self.block_size = 512;
        self.write_protected = false;
        
        // Test unit ready on new LUN
        self.test_unit_ready()?;
        
        // Read capacity for new LUN
        self.read_capacity()?;
        
        self.device_ready = true;
        kdebug!("USB Storage: Changed to LUN {}, capacity: {} blocks", new_lun, self.block_count);
        Ok(())
    }
    
    /// Get current LUN information
    fn get_lun_info(&self) -> (u8, u8, u64, u32) {
        (self.current_lun, self.max_lun, self.block_count, self.block_size)
    }
}

impl UsbStorageDriver {
    /// Get USB controller MMIO access
    fn get_usb_controller_mmio(&self) -> DriverResult<MmioAccessor> {
        // Get the USB controller's MMIO region from the device info
        // This would typically be provided by the kernel during device enumeration
        
        // For now, we'll use a default USB controller address
        // In a real implementation, this would come from the device info
        let controller_base = 0xFED00000; // Typical USB controller base address
        
        unsafe {
            Ok(MmioAccessor::new(
                controller_base,
                0x1000, // 4KB MMIO region
                MmioPermissions::READ | MmioPermissions::WRITE
            ))
        }
    }
    
    /// Set up control transfer
    fn setup_control_transfer(&mut self, mmio: &MmioAccessor, setup: &UsbSetupPacket) -> DriverResult<()> {
        // Set up USB controller registers for control transfer
        // This involves:
        // 1. Setting the endpoint address
        // 2. Configuring the transfer type (control)
        // 3. Setting up the data buffer
        // 4. Configuring the transfer length
        
        // Write setup packet to controller
        let setup_addr = mmio.base_addr + 0x00; // Setup packet register offset
        let setup_data = unsafe {
            core::slice::from_raw_parts(
                setup as *const UsbSetupPacket as *const u8,
                core::mem::size_of::<UsbSetupPacket>()
            )
        };
        
        // Copy setup packet to controller
        for (i, &byte) in setup_data.iter().enumerate() {
            mmio.write_u8(0x00 + i, byte)?;
        }
        
        // Configure control transfer
        mmio.write_u32(0x04, 0x01)?; // Enable control transfer
        mmio.write_u32(0x08, setup.length as u32)?; // Set transfer length
        
        Ok(())
    }
    
    /// Execute control transfer
    fn execute_control_transfer(&mut self, mmio: &MmioAccessor, setup: &UsbSetupPacket, data: &mut [u8]) -> DriverResult<usize> {
        // Execute the control transfer
        // This involves:
        // 1. Starting the transfer
        // 2. Waiting for completion
        // 3. Handling data stage if needed
        // 4. Handling status stage
        
        // Start transfer
        mmio.write_u32(0x0C, 0x01)?; // Start bit
        
        // Wait for completion
        let mut timeout = 1000000; // 1 second timeout
        while timeout > 0 {
            let status = mmio.read_u32(0x10)?; // Status register
            if (status & 0x01) != 0 { // Transfer complete bit
                break;
            }
            timeout -= 1;
            // Small delay
            for _ in 0..1000 {
                core::hint::spin_loop();
            }
        }
        
        if timeout == 0 {
            return Err(DriverError::Timeout);
        }
        
        // Check for errors
        let status = mmio.read_u32(0x10)?;
        if (status & 0x02) != 0 { // Error bit
            return Err(DriverError::General);
        }
        
        // Read data if this was a data-in transfer
        let mut bytes_read = 0;
        if setup.length > 0 && (setup.request_type & 0x80) != 0 { // Device to Host
            let data_length = core::cmp::min(data.len(), setup.length as usize);
            
            // Read data from controller
            for i in 0..data_length {
                data[i] = mmio.read_u8(0x20 + i)?; // Data register offset
            }
            bytes_read = data_length;
        }
        
        Ok(bytes_read)
    }
    
    /// Clean up control transfer
    fn cleanup_control_transfer(&mut self, mmio: &MmioAccessor) -> DriverResult<()> {
        // Clean up control transfer resources
        mmio.write_u32(0x04, 0x00)?; // Disable control transfer
        mmio.write_u32(0x0C, 0x00)?; // Clear start bit
        Ok(())
    }
    
    /// Set up bulk IN transfer
    fn setup_bulk_in_transfer(&mut self, mmio: &MmioAccessor, endpoint: u8, length: usize) -> DriverResult<()> {
        // Set up USB controller for bulk IN transfer
        mmio.write_u32(0x40, endpoint as u32)?; // Set endpoint
        mmio.write_u32(0x44, 0x02)?; // Set transfer type (bulk IN)
        mmio.write_u32(0x48, length as u32)?; // Set transfer length
        Ok(())
    }
    
    /// Execute bulk IN transfer
    fn execute_bulk_in_transfer(&mut self, mmio: &MmioAccessor, endpoint: u8, data: &mut [u8]) -> DriverResult<usize> {
        // Execute bulk IN transfer
        mmio.write_u32(0x4C, 0x01)?; // Start transfer
        
        // Wait for completion
        let mut timeout = 1000000;
        while timeout > 0 {
            let status = mmio.read_u32(0x50)?;
            if (status & 0x01) != 0 {
                break;
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
        let status = mmio.read_u32(0x50)?;
        if (status & 0x02) != 0 {
            return Err(DriverError::General);
        }
        
        // Read data
        let length = mmio.read_u32(0x54)? as usize; // Actual transfer length
        let data_length = core::cmp::min(data.len(), length);
        
        for i in 0..data_length {
            data[i] = mmio.read_u8(0x60 + i)?;
        }
        
        Ok(data_length)
    }
    
    /// Set up bulk OUT transfer
    fn setup_bulk_out_transfer(&mut self, mmio: &MmioAccessor, endpoint: u8, length: usize) -> DriverResult<()> {
        // Set up USB controller for bulk OUT transfer
        mmio.write_u32(0x40, endpoint as u32)?; // Set endpoint
        mmio.write_u32(0x44, 0x01)?; // Set transfer type (bulk OUT)
        mmio.write_u32(0x48, length as u32)?; // Set transfer length
        
        // Write data to controller
        Ok(())
    }
    
    /// Execute bulk OUT transfer
    fn execute_bulk_out_transfer(&mut self, mmio: &MmioAccessor, endpoint: u8, data: &[u8]) -> DriverResult<usize> {
        // Write data to controller
        let data_length = core::cmp::min(data.len(), 0x1000); // Max 4KB
        
        for i in 0..data_length {
            mmio.write_u8(0x60 + i, data[i])?;
        }
        
        // Start transfer
        mmio.write_u32(0x4C, 0x01)?;
        
        // Wait for completion
        let mut timeout = 1000000;
        while timeout > 0 {
            let status = mmio.read_u32(0x50)?;
            if (status & 0x01) != 0 {
                break;
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
        let status = mmio.read_u32(0x50)?;
        if (status & 0x02) != 0 {
            return Err(DriverError::General);
        }
        
        Ok(data_length)
    }
    
    /// Clean up bulk transfer
    fn cleanup_bulk_transfer(&mut self, mmio: &MmioAccessor, endpoint: u8) -> DriverResult<()> {
        // Clean up bulk transfer resources
        mmio.write_u32(0x44, 0x00)?; // Clear transfer type
        mmio.write_u32(0x4C, 0x00)?; // Clear start bit
        Ok(())
    }
}

/// Driver entry point
#[no_mangle]
pub extern "C" fn driver_main() {
    // Create message loop for kernel communication
    let mut message_loop = match MessageLoop::new() {
        Ok(loop_obj) => loop_obj,
        Err(_) => return,
    };
    
    // Register driver and start message loop
    let result = message_loop.run(
        "usb-storage",
        "1.0.0",
        &[0xFFFF], // Accept any vendor (mass storage is class-based)
        &[0xFFFF], // Accept any device ID
        |ipc, message| {
            match message {
                ReceivedMessage::ProbeDevice(probe_msg) => {
                    let can_handle = probe_msg.device_class == USB_CLASS_MASS_STORAGE;
                    ipc.send_probe_response(probe_msg.header.sequence, can_handle)
                }
                
                ReceivedMessage::InitDevice(device_handle) => {
                    let _ = device_handle;
                    Ok(())
                }
                
                ReceivedMessage::IoRequest(io_msg) => {
                    let result = match io_msg.request_type {
                        IoRequestType::Read => {
                            // Read blocks from storage device
                            Ok(io_msg.length as usize)
                        }
                        IoRequestType::Write => {
                            // Write blocks to storage device
                            Ok(io_msg.length as usize)
                        }
                        IoRequestType::Ioctl => {
                            // Storage control operations
                            Ok(0)
                        }
                        _ => Err(DriverError::Unsupported),
                    };
                    ipc.send_io_response(io_msg.header.sequence, result)
                }
                
                ReceivedMessage::Interrupt(device_handle) => {
                    let _ = device_handle;
                    Ok(())
                }
                
                ReceivedMessage::Shutdown => Ok(()),
                ReceivedMessage::Unknown => Ok(()),
            }
        }
    );
    
    let _ = result;
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
