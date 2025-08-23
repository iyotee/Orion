/*
 * Orion Operating System - Driver Framework Types
 *
 * Common data structures and types for all Orion OS device drivers.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

use alloc::vec::Vec;

// ========================================
// BASIC DRIVER TYPES
// ========================================

/// Device handle for kernel communication
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct DeviceHandle {
    pub id: u64,
    pub device_type: DeviceType,
}

impl DeviceHandle {
    pub fn new(id: u64, device_type: DeviceType) -> Self {
        Self { id, device_type }
    }
}

/// Device type enumeration
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum DeviceType {
    Unknown,
    Usb,
    Pci,
    Isa,
    Virtio,
    Block,
    Network,
    Graphics,
    Audio,
    Input,
}

/// I/O request types
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum IoRequestType {
    Read,
    Write,
    Ioctl,
    Flush,
    Sync,
}

/// Device information structure
#[derive(Debug, Clone)]
pub struct DeviceInfo {
    pub vendor_id: u16,
    pub device_id: u16,
    pub class: u8,
    pub subclass: u8,
    pub protocol: u8,
    pub revision: u8,
    pub bus: u8,
    pub device: u8,
    pub function: u8,
}

impl DeviceInfo {
    pub fn new(vendor_id: u16, device_id: u16, class: u8, subclass: u8, protocol: u8) -> Self {
        Self {
            vendor_id,
            device_id,
            class,
            subclass,
            protocol,
            revision: 0,
            bus: 0,
            device: 0,
            function: 0,
        }
    }
}

/// Driver information structure
#[derive(Debug, Clone)]
pub struct DriverInfo {
    pub name: &'static str,
    pub version: &'static str,
    pub description: &'static str,
    pub author: &'static str,
    pub license: &'static str,
}

impl DriverInfo {
    pub fn new(name: &'static str, version: &'static str, description: &'static str) -> Self {
        Self {
            name,
            version,
            description,
            author: "Jeremy Noverraz (1988-2025)",
            license: "MIT",
        }
    }
}

// ========================================
// USB-SPECIFIC TYPES
// ========================================

/// USB device descriptor
#[derive(Debug, Clone)]
pub struct UsbDeviceDescriptor {
    pub length: u8,
    pub descriptor_type: u8,
    pub usb_version: u16,
    pub device_class: u8,
    pub device_subclass: u8,
    pub device_protocol: u8,
    pub max_packet_size: u8,
    pub vendor_id: u16,
    pub product_id: u16,
    pub device_version: u16,
    pub manufacturer_index: u8,
    pub product_index: u8,
    pub serial_number_index: u8,
    pub num_configurations: u8,
}

/// USB configuration descriptor
#[derive(Debug, Clone)]
pub struct UsbConfigurationDescriptor {
    pub length: u8,
    pub descriptor_type: u8,
    pub total_length: u16,
    pub num_interfaces: u8,
    pub configuration_value: u8,
    pub configuration_index: u8,
    pub attributes: u8,
    pub max_power: u8,
}

/// USB interface descriptor
#[derive(Debug, Clone)]
pub struct UsbInterfaceDescriptor {
    pub length: u8,
    pub descriptor_type: u8,
    pub interface_number: u8,
    pub alternate_setting: u8,
    pub num_endpoints: u8,
    pub interface_class: u8,
    pub interface_subclass: u8,
    pub interface_protocol: u8,
    pub interface_index: u8,
}

/// USB endpoint descriptor
#[derive(Debug, Clone)]
pub struct UsbEndpointDescriptor {
    pub length: u8,
    pub descriptor_type: u8,
    pub endpoint_address: u8,
    pub attributes: u8,
    pub max_packet_size: u16,
    pub interval: u8,
}

// ========================================
// HID-SPECIFIC TYPES
// ========================================

/// HID report descriptor
#[derive(Debug, Clone)]
pub struct HidReportDescriptor {
    pub data: Vec<u8>,
    pub length: usize,
}

/// HID report types
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum HidReportType {
    Input,
    Output,
    Feature,
}

/// Mouse event structure
#[derive(Debug, Clone)]
pub struct MouseEvent {
    pub delta_x: i16,
    pub delta_y: i16,
    pub buttons: u8,
    pub wheel: i8,
}

impl MouseEvent {
    pub fn new(delta_x: i16, delta_y: i16, buttons: u8) -> Self {
        Self {
            delta_x,
            delta_y,
            buttons,
            wheel: 0,
        }
    }
}

/// Keyboard event structure
#[derive(Debug, Clone)]
pub struct KeyEvent {
    pub key_code: KeyCode,
    pub pressed: bool,
    pub modifiers: u8,
}

/// Key codes enumeration
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum KeyCode {
    // Basic keys
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Key0, Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8, Key9,
    
    // Function keys
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    
    // Special keys
    Escape, Tab, CapsLock, Shift, Ctrl, Alt, Meta, Space, Enter, Backspace, Delete,
    Home, End, PageUp, PageDown, Insert, PrintScreen, ScrollLock, Pause,
    
    // Arrow keys
    Up, Down, Left, Right,
    
    // Other
    Unknown,
}

// ========================================
// STORAGE-SPECIFIC TYPES
// ========================================

/// Storage device geometry
#[derive(Debug, Clone)]
pub struct StorageGeometry {
    pub cylinders: u32,
    pub heads: u32,
    pub sectors_per_track: u32,
    pub bytes_per_sector: u32,
    pub total_sectors: u64,
}

/// Storage command block wrapper (USB mass storage)
#[derive(Debug, Clone)]
pub struct CommandBlockWrapper {
    pub signature: u32,
    pub tag: u32,
    pub data_transfer_length: u32,
    pub flags: u8,
    pub lun: u8,
    pub command_length: u8,
    pub command_data: [u8; 16],
}

/// Storage command status wrapper (USB mass storage)
#[derive(Debug, Clone)]
pub struct CommandStatusWrapper {
    pub signature: u32,
    pub tag: u32,
    pub data_residue: u32,
    pub status: u8,
}

// ========================================
// NETWORK-SPECIFIC TYPES
// ========================================

/// MAC address
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct MacAddress {
    pub bytes: [u8; 6],
}

impl MacAddress {
    pub fn new(bytes: [u8; 6]) -> Self {
        Self { bytes }
    }
    
    pub fn broadcast() -> Self {
        Self { bytes: [0xFF; 6] }
    }
    
    pub fn zero() -> Self {
        Self { bytes: [0x00; 6] }
    }
}

/// IP address (IPv4)
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct IpAddress {
    pub bytes: [u8; 4],
}

impl IpAddress {
    pub fn new(bytes: [u8; 4]) -> Self {
        Self { bytes }
    }
    
    pub fn localhost() -> Self {
        Self { bytes: [127, 0, 0, 1] }
    }
    
    pub fn broadcast() -> Self {
        Self { bytes: [255, 255, 255, 255] }
    }
}

// ========================================
// GRAPHICS-SPECIFIC TYPES
// ========================================

/// Display mode
#[derive(Debug, Clone)]
pub struct DisplayMode {
    pub width: u32,
    pub height: u32,
    pub bits_per_pixel: u8,
    pub refresh_rate: u32,
}

/// Color format
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ColorFormat {
    RGB888,
    BGR888,
    RGB565,
    RGBA8888,
    BGRA8888,
}

/// Framebuffer information
#[derive(Debug, Clone)]
pub struct FramebufferInfo {
    pub address: usize,
    pub size: usize,
    pub width: u32,
    pub height: u32,
    pub pitch: u32,
    pub bits_per_pixel: u8,
    pub color_format: ColorFormat,
}
