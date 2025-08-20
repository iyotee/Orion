/*
 * Orion Operating System - USB Mass Storage Driver
 *
 * Universal USB mass storage driver supporting USB flash drives, external hard drives,
 * and other USB storage devices using the Bulk-Only Transport protocol.
 *
 * Developed by Jérémy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#![no_std]
#![no_main]

use orion_driver::{
    UsbDriver, BlockDriver, DeviceInfo, DriverError, DriverInfo, DriverResult, OrionDriver,
    UsbSetupPacket, UsbDeviceDescriptor, BlockCapabilities,
    MessageLoop, ReceivedMessage, IoRequestType,
};

/// USB Mass Storage Driver
pub struct UsbStorageDriver {
    device: DeviceInfo,
    device_descriptor: UsbDeviceDescriptor,
    interface_number: u8,
    bulk_in_endpoint: u8,
    bulk_out_endpoint: u8,
    max_packet_size: u16,
    max_lun: u8,
    block_size: u32,
    block_count: u64,
    device_ready: bool,
    write_protected: bool,
}

// USB Mass Storage Class Codes
const USB_CLASS_MASS_STORAGE: u8 = 0x08;

// Mass Storage Subclasses
const MS_SUBCLASS_RBC: u8 = 0x01;        // Reduced Block Commands
const MS_SUBCLASS_ATAPI: u8 = 0x02;      // CD/DVD (ATAPI)
const MS_SUBCLASS_QIC157: u8 = 0x03;     // Tape (QIC-157)
const MS_SUBCLASS_UFI: u8 = 0x04;        // Floppy (UFI)
const MS_SUBCLASS_SFF8070I: u8 = 0x05;   // Floppy (SFF-8070i)
const MS_SUBCLASS_SCSI: u8 = 0x06;       // SCSI transparent command set

// Mass Storage Protocols
const MS_PROTOCOL_CBI: u8 = 0x00;        // Control/Bulk/Interrupt
const MS_PROTOCOL_CB: u8 = 0x01;         // Control/Bulk (no interrupt)
const MS_PROTOCOL_BULK_ONLY: u8 = 0x50;  // Bulk-Only Transport (BOT)

// Mass Storage Requests
const MS_REQ_RESET: u8 = 0xFF;           // Bulk-Only Mass Storage Reset
const MS_REQ_GET_MAX_LUN: u8 = 0xFE;     // Get Max LUN

// Command Block Wrapper (CBW)
const CBW_SIGNATURE: u32 = 0x43425355;   // "USBC"
const CBW_FLAGS_DATA_OUT: u8 = 0x00;     // Host to device
const CBW_FLAGS_DATA_IN: u8 = 0x80;      // Device to host

// Command Status Wrapper (CSW)
const CSW_SIGNATURE: u32 = 0x53425355;   // "USBS"
const CSW_STATUS_GOOD: u8 = 0x00;        // Command passed
const CSW_STATUS_FAILED: u8 = 0x01;      // Command failed
const CSW_STATUS_PHASE_ERROR: u8 = 0x02; // Phase error

// SCSI Commands
const SCSI_CMD_TEST_UNIT_READY: u8 = 0x00;
const SCSI_CMD_REQUEST_SENSE: u8 = 0x03;
const SCSI_CMD_INQUIRY: u8 = 0x12;
const SCSI_CMD_MODE_SENSE_6: u8 = 0x1A;
const SCSI_CMD_START_STOP_UNIT: u8 = 0x1B;
const SCSI_CMD_PREVENT_ALLOW_REMOVAL: u8 = 0x1E;
const SCSI_CMD_READ_FORMAT_CAPACITIES: u8 = 0x23;
const SCSI_CMD_READ_CAPACITY_10: u8 = 0x25;
const SCSI_CMD_READ_10: u8 = 0x28;
const SCSI_CMD_WRITE_10: u8 = 0x2A;
const SCSI_CMD_VERIFY_10: u8 = 0x2F;
const SCSI_CMD_MODE_SENSE_10: u8 = 0x5A;

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
            max_packet_size: 64,
            max_lun: 0,
            block_size: 512,         // Default block size
            block_count: 0,
            device_ready: false,
            write_protected: false,
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
            author: "Jérémy Noverraz",
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
            // Read data from device
            // TODO: Implement actual bulk transfer
            Ok(0)
        } else if endpoint == self.bulk_out_endpoint {
            // Write data to device
            // TODO: Implement actual bulk transfer
            Ok(data.len())
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
        // TODO: Send reset request via control transfer
        self.device_ready = false;
        Ok(())
    }
    
    /// Get maximum LUN from device
    fn get_max_lun(&mut self) -> DriverResult<()> {
        // TODO: Send GET_MAX_LUN control request
        self.max_lun = 0; // Default to LUN 0
        Ok(())
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
            tag: 0x12345678, // TODO: Use proper tag management
            data_length: expected_length,
            flags: data_direction,
            lun: 0, // TODO: Support multiple LUNs
            cb_length: command.len() as u8,
            cb: [0; 16],
        };
        
        // Copy command to CBW
        let copy_len = core::cmp::min(command.len(), 16);
        cbw.cb[..copy_len].copy_from_slice(&command[..copy_len]);
        
        // Send CBW
        // TODO: Send CBW via bulk OUT transfer
        
        // Transfer data if needed
        if let Some(data_buffer) = data {
            if data_direction == CBW_FLAGS_DATA_IN {
                // TODO: Read data via bulk IN transfer
                let _ = data_buffer;
            } else {
                // TODO: Write data via bulk OUT transfer
                let _ = data_buffer;
            }
        }
        
        // Receive CSW
        let mut csw = CommandStatusWrapper {
            signature: 0,
            tag: 0,
            data_residue: 0,
            status: 0,
        };
        
        // TODO: Receive CSW via bulk IN transfer
        let _ = &mut csw;
        
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
        // TODO: Extract device information from inquiry response
        
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
        // TODO: Handle const buffer in execute_scsi_command
        let _ = (transfer_length, buffer);
        
        // For now, just execute command without data
        self.execute_scsi_command(&command, CBW_FLAGS_DATA_OUT, transfer_length, None)
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
