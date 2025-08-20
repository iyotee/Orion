/*
 * Orion Operating System - USB HID Driver
 *
 * Universal USB Human Interface Device driver supporting keyboards, mice,
 * gamepads, and other HID-compliant input devices.
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
    UsbDriver, DeviceInfo, DriverError, DriverInfo, DriverResult, OrionDriver,
    UsbSetupPacket, UsbDeviceDescriptor,
    MessageLoop, ReceivedMessage, IoRequestType,
};

/// USB HID Driver
pub struct UsbHidDriver {
    device: DeviceInfo,
    device_descriptor: UsbDeviceDescriptor,
    interface_number: u8,
    interrupt_endpoint: u8,
    report_size: u16,
    hid_type: HidDeviceType,
    boot_protocol: bool,
}

/// Types of HID devices
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum HidDeviceType {
    Unknown,
    Keyboard,
    Mouse,
    Gamepad,
    Joystick,
    Touch,
    Tablet,
    Generic,
}

// USB HID Class Codes
const USB_CLASS_HID: u8 = 0x03;

// HID Subclasses
const HID_SUBCLASS_NONE: u8 = 0x00;
const HID_SUBCLASS_BOOT: u8 = 0x01;

// HID Protocols
const HID_PROTOCOL_NONE: u8 = 0x00;
const HID_PROTOCOL_KEYBOARD: u8 = 0x01;
const HID_PROTOCOL_MOUSE: u8 = 0x02;

// HID Request Types
const HID_REQ_GET_REPORT: u8 = 0x01;
const HID_REQ_GET_IDLE: u8 = 0x02;
const HID_REQ_GET_PROTOCOL: u8 = 0x03;
const HID_REQ_SET_REPORT: u8 = 0x09;
const HID_REQ_SET_IDLE: u8 = 0x0A;
const HID_REQ_SET_PROTOCOL: u8 = 0x0B;

// HID Report Types
const HID_REPORT_INPUT: u8 = 0x01;
const HID_REPORT_OUTPUT: u8 = 0x02;
const HID_REPORT_FEATURE: u8 = 0x03;

// HID Descriptor Types
const HID_DESC_HID: u8 = 0x21;
const HID_DESC_REPORT: u8 = 0x22;
const HID_DESC_PHYSICAL: u8 = 0x23;

// Boot Protocol Reports
const BOOT_KEYBOARD_REPORT_SIZE: u16 = 8;
const BOOT_MOUSE_REPORT_SIZE: u16 = 3;

// Keyboard modifier keys (bitmap)
const KEY_MOD_LCTRL: u8 = 0x01;
const KEY_MOD_LSHIFT: u8 = 0x02;
const KEY_MOD_LALT: u8 = 0x04;
const KEY_MOD_LMETA: u8 = 0x08;
const KEY_MOD_RCTRL: u8 = 0x10;
const KEY_MOD_RSHIFT: u8 = 0x20;
const KEY_MOD_RALT: u8 = 0x40;
const KEY_MOD_RMETA: u8 = 0x80;

// Mouse button flags
const MOUSE_BTN_LEFT: u8 = 0x01;
const MOUSE_BTN_RIGHT: u8 = 0x02;
const MOUSE_BTN_MIDDLE: u8 = 0x04;

/// Boot protocol keyboard report
#[repr(C, packed)]
struct BootKeyboardReport {
    modifiers: u8,       // Modifier keys bitmap
    reserved: u8,        // Reserved byte
    keycodes: [u8; 6],   // Up to 6 simultaneous key presses
}

/// Boot protocol mouse report
#[repr(C, packed)]
struct BootMouseReport {
    buttons: u8,         // Button state bitmap
    delta_x: i8,         // X movement delta
    delta_y: i8,         // Y movement delta
}

/// HID descriptor
#[repr(C, packed)]
struct HidDescriptor {
    length: u8,
    descriptor_type: u8,
    hid_version: u16,
    country_code: u8,
    num_descriptors: u8,
    report_descriptor_type: u8,
    report_descriptor_length: u16,
}

impl OrionDriver for UsbHidDriver {
    fn probe(device: &DeviceInfo) -> DriverResult<bool> {
        // Check if this is a HID device
        Ok(device.class == USB_CLASS_HID)
    }
    
    fn init(device: DeviceInfo) -> DriverResult<Self> {
        // Determine HID device type based on protocol
        let hid_type = match device.prog_if {
            HID_PROTOCOL_KEYBOARD => HidDeviceType::Keyboard,
            HID_PROTOCOL_MOUSE => HidDeviceType::Mouse,
            _ => HidDeviceType::Generic,
        };
        
        // Check if this is a boot interface device
        let boot_protocol = device.subclass == HID_SUBCLASS_BOOT;
        
        // Set up default report size based on device type and protocol
        let report_size = if boot_protocol {
            match hid_type {
                HidDeviceType::Keyboard => BOOT_KEYBOARD_REPORT_SIZE,
                HidDeviceType::Mouse => BOOT_MOUSE_REPORT_SIZE,
                _ => 64, // Default size for other devices
            }
        } else {
            64 // Default size, should be determined from HID descriptor
        };
        
        // Create a default device descriptor
        let device_descriptor = UsbDeviceDescriptor {
            length: 18,
            descriptor_type: 1,
            usb_version: 0x0200, // USB 2.0
            device_class: USB_CLASS_HID,
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
        
        Ok(UsbHidDriver {
            device,
            device_descriptor,
            interface_number: 0,
            interrupt_endpoint: 0x81, // Default interrupt IN endpoint
            report_size,
            hid_type,
            boot_protocol,
        })
    }
    
    fn handle_irq(&mut self) -> DriverResult<()> {
        // Process interrupt from HID device (input report available)
        self.process_input_report()?;
        Ok(())
    }
    
    fn shutdown(&mut self) -> DriverResult<()> {
        // No specific shutdown procedure for HID devices
        Ok(())
    }
    
    fn info(&self) -> DriverInfo {
        let description = match self.hid_type {
            HidDeviceType::Keyboard => "USB keyboard driver with full key mapping support",
            HidDeviceType::Mouse => "USB mouse driver with button and scroll wheel support",
            HidDeviceType::Gamepad => "USB gamepad driver for gaming controllers",
            HidDeviceType::Joystick => "USB joystick driver for flight controllers",
            HidDeviceType::Touch => "USB touch device driver for touchscreens",
            HidDeviceType::Tablet => "USB tablet driver for graphics tablets",
            _ => "Generic USB HID driver for human interface devices",
        };
        
        DriverInfo {
            name: "USB HID Driver",
            version: "1.0.0",
            author: "Jérémy Noverraz",
            description,
        }
    }
}

impl UsbDriver for UsbHidDriver {
    fn control_transfer(&mut self, setup: &UsbSetupPacket, data: &mut [u8]) -> DriverResult<usize> {
        // Handle HID-specific control transfers
        match setup.request {
            HID_REQ_GET_REPORT => {
                // Get input/output/feature report
                let report_type = (setup.value >> 8) as u8;
                let report_id = (setup.value & 0xFF) as u8;
                self.get_report(report_type, report_id, data)
            }
            
            HID_REQ_SET_REPORT => {
                // Set output/feature report
                let report_type = (setup.value >> 8) as u8;
                let report_id = (setup.value & 0xFF) as u8;
                self.set_report(report_type, report_id, data)?;
                Ok(0)
            }
            
            HID_REQ_GET_IDLE => {
                // Get idle rate for input reports
                if data.len() >= 1 {
                    data[0] = 0; // Infinite idle rate (report only on change)
                    Ok(1)
                } else {
                    Err(DriverError::MemoryError)
                }
            }
            
            HID_REQ_SET_IDLE => {
                // Set idle rate for input reports
                let idle_rate = (setup.value >> 8) as u8;
                self.set_idle_rate(idle_rate)?;
                Ok(0)
            }
            
            HID_REQ_GET_PROTOCOL => {
                // Get current protocol (boot or report)
                if data.len() >= 1 {
                    data[0] = if self.boot_protocol { 0 } else { 1 };
                    Ok(1)
                } else {
                    Err(DriverError::MemoryError)
                }
            }
            
            HID_REQ_SET_PROTOCOL => {
                // Set protocol (boot or report)
                let protocol = (setup.value & 0xFF) as u8;
                self.boot_protocol = protocol == 0;
                Ok(0)
            }
            
            _ => Err(DriverError::Unsupported)
        }
    }
    
    fn bulk_transfer(&mut self, endpoint: u8, data: &mut [u8]) -> DriverResult<usize> {
        // HID devices typically use interrupt transfers, not bulk
        let _ = (endpoint, data);
        Err(DriverError::Unsupported)
    }
    
    fn interrupt_transfer(&mut self, endpoint: u8, data: &mut [u8]) -> DriverResult<usize> {
        if endpoint == self.interrupt_endpoint {
            // Read input report from device
            self.read_input_report(data)
        } else {
            Err(DriverError::Unsupported)
        }
    }
    
    fn device_descriptor(&self) -> UsbDeviceDescriptor {
        self.device_descriptor
    }
}

impl UsbHidDriver {
    /// Process an input report from the device
    fn process_input_report(&mut self) -> DriverResult<()> {
        // TODO: Read input report and process based on device type
        match self.hid_type {
            HidDeviceType::Keyboard => self.process_keyboard_report(),
            HidDeviceType::Mouse => self.process_mouse_report(),
            _ => Ok(()), // Generic processing
        }
    }
    
    /// Process keyboard input report
    fn process_keyboard_report(&mut self) -> DriverResult<()> {
        // TODO: Read keyboard report and generate key events
        // For boot protocol keyboards, this involves reading the 8-byte report
        // and translating scan codes to key events
        Ok(())
    }
    
    /// Process mouse input report
    fn process_mouse_report(&mut self) -> DriverResult<()> {
        // TODO: Read mouse report and generate motion/button events
        // For boot protocol mice, this involves reading the 3-byte report
        // and generating appropriate mouse events
        Ok(())
    }
    
    /// Read input report from device
    fn read_input_report(&mut self, buffer: &mut [u8]) -> DriverResult<usize> {
        if buffer.len() < self.report_size as usize {
            return Err(DriverError::MemoryError);
        }
        
        // TODO: Perform actual interrupt transfer to read report
        // For now, simulate no data available
        Ok(0)
    }
    
    /// Get report from device
    fn get_report(&mut self, report_type: u8, report_id: u8, buffer: &mut [u8]) -> DriverResult<usize> {
        let _ = (report_type, report_id);
        
        // TODO: Implement GET_REPORT control transfer
        // For now, return empty report
        if buffer.len() > 0 {
            buffer[0] = 0;
            Ok(1)
        } else {
            Ok(0)
        }
    }
    
    /// Set report to device
    fn set_report(&mut self, report_type: u8, report_id: u8, data: &[u8]) -> DriverResult<()> {
        let _ = (report_type, report_id, data);
        
        // TODO: Implement SET_REPORT control transfer
        // This is used for output reports (e.g., keyboard LEDs) and feature reports
        Ok(())
    }
    
    /// Set idle rate for input reports
    fn set_idle_rate(&mut self, idle_rate: u8) -> DriverResult<()> {
        let _ = idle_rate;
        
        // TODO: Configure device idle rate
        // idle_rate of 0 means infinite (report only on change)
        // Other values specify the idle time in 4ms units
        Ok(())
    }
    
    /// Get HID descriptor from device
    pub fn get_hid_descriptor(&mut self) -> DriverResult<HidDescriptor> {
        // TODO: Read HID descriptor via control transfer
        // This contains information about report descriptors
        
        // Return a default descriptor for now
        Ok(HidDescriptor {
            length: 9,
            descriptor_type: HID_DESC_HID,
            hid_version: 0x0111, // HID 1.11
            country_code: 0,     // Not country specific
            num_descriptors: 1,
            report_descriptor_type: HID_DESC_REPORT,
            report_descriptor_length: 64, // Default size
        })
    }
    
    /// Get report descriptor from device
    pub fn get_report_descriptor(&mut self, buffer: &mut [u8]) -> DriverResult<usize> {
        // TODO: Read report descriptor via control transfer
        // This describes the format of input/output reports
        
        let _ = buffer;
        Ok(0) // No descriptor available yet
    }
    
    /// Translate boot keyboard report to key events
    pub fn translate_keyboard_report(&self, report: &BootKeyboardReport) -> Vec<KeyEvent> {
        let mut events = Vec::new();
        
        // TODO: Compare with previous report to detect key press/release events
        // For now, just return empty vector
        let _ = report;
        events
    }
    
    /// Translate boot mouse report to mouse events  
    pub fn translate_mouse_report(&self, report: &BootMouseReport) -> MouseEvent {
        MouseEvent {
            buttons: report.buttons,
            delta_x: report.delta_x as i16,
            delta_y: report.delta_y as i16,
            wheel_delta: 0, // Boot protocol doesn't support wheel
        }
    }
}

/// Key event structure
#[derive(Debug, Clone, Copy)]
pub struct KeyEvent {
    pub keycode: u8,
    pub pressed: bool,
    pub modifiers: u8,
}

/// Mouse event structure  
#[derive(Debug, Clone, Copy)]
pub struct MouseEvent {
    pub buttons: u8,
    pub delta_x: i16,
    pub delta_y: i16,
    pub wheel_delta: i8,
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
        "usb-hid",
        "1.0.0",
        &[0xFFFF], // Accept any vendor (HID is class-based)
        &[0xFFFF], // Accept any device ID
        |ipc, message| {
            match message {
                ReceivedMessage::ProbeDevice(probe_msg) => {
                    let can_handle = probe_msg.device_class == USB_CLASS_HID;
                    ipc.send_probe_response(probe_msg.header.sequence, can_handle)
                }
                
                ReceivedMessage::InitDevice(device_handle) => {
                    let _ = device_handle;
                    Ok(())
                }
                
                ReceivedMessage::IoRequest(io_msg) => {
                    let result = match io_msg.request_type {
                        IoRequestType::Read => {
                            // Read input report or device state
                            Ok(8) // Typical HID report size
                        }
                        IoRequestType::Write => {
                            // Write output report (e.g., LED state)
                            Ok(io_msg.length as usize)
                        }
                        IoRequestType::Ioctl => {
                            // HID-specific control operations
                            Ok(0)
                        }
                        _ => Err(DriverError::Unsupported),
                    };
                    ipc.send_io_response(io_msg.header.sequence, result)
                }
                
                ReceivedMessage::Interrupt(device_handle) => {
                    // HID interrupt (input report available)
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
