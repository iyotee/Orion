/*
 * Orion Operating System - Device Information and Management
 *
 * Device detection, enumeration, and information structures
 * for the Orion driver framework.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

/// Device information structure
#[derive(Debug, Clone)]
pub struct DeviceInfo {
    /// Device vendor ID
    pub vendor_id: u16,
    /// Device product/device ID
    pub device_id: u16,
    /// Device class
    pub class: u8,
    /// Device subclass
    pub subclass: u8,
    /// Device programming interface
    pub prog_if: u8,
    /// Device revision
    pub revision: u8,
    /// Bus type
    pub bus_type: BusType,
    /// Device location on bus
    pub location: DeviceLocation,
    /// Base address registers (for PCI devices)
    pub bars: [u64; 6],
    /// IRQ number
    pub irq: Option<u8>,
    /// Device name (if known)
    pub name: Option<&'static str>,
}

/// Bus type enumeration
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum BusType {
    /// PCI Express bus
    Pci,
    /// USB bus
    Usb,
    /// Platform device (integrated)
    Platform,
    /// Virtual device
    Virtual,
    /// ACPI device
    Acpi,
}

/// Device location on bus
#[derive(Debug, Clone, Copy)]
pub struct DeviceLocation {
    /// Bus number
    pub bus: u8,
    /// Device number
    pub device: u8,
    /// Function number
    pub function: u8,
}

impl DeviceInfo {
    /// Create a new device info structure
    pub fn new(vendor_id: u16, device_id: u16, bus_type: BusType) -> Self {
        Self {
            vendor_id,
            device_id,
            class: 0,
            subclass: 0,
            prog_if: 0,
            revision: 0,
            bus_type,
            location: DeviceLocation { bus: 0, device: 0, function: 0 },
            bars: [0; 6],
            irq: None,
            name: None,
        }
    }
    
    /// Check if this is a storage device
    pub fn is_storage_device(&self) -> bool {
        self.class == 0x01 // Mass storage controller
    }
    
    /// Check if this is a network device
    pub fn is_network_device(&self) -> bool {
        self.class == 0x02 // Network controller
    }
    
    /// Check if this is a display device
    pub fn is_display_device(&self) -> bool {
        self.class == 0x03 // Display controller
    }
    
    /// Check if this is a USB controller
    pub fn is_usb_controller(&self) -> bool {
        self.class == 0x0C && self.subclass == 0x03 // Serial bus controller, USB
    }
    
    /// Get device type as string
    pub fn device_type_str(&self) -> &'static str {
        match self.class {
            0x00 => "Unclassified",
            0x01 => "Mass Storage",
            0x02 => "Network",
            0x03 => "Display",
            0x04 => "Multimedia",
            0x05 => "Memory",
            0x06 => "Bridge",
            0x07 => "Communication",
            0x08 => "Generic System Peripheral",
            0x09 => "Input Device",
            0x0A => "Docking Station",
            0x0B => "Processor",
            0x0C => "Serial Bus",
            0x0D => "Wireless",
            0x0E => "Intelligent Controller",
            0x0F => "Satellite Communication",
            0x10 => "Encryption",
            0x11 => "Signal Processing",
            0x12 => "Processing Accelerator",
            0x13 => "Non-Essential Instrumentation",
            _ => "Unknown",
        }
    }
    
    /// Get vendor name if known
    pub fn vendor_name(&self) -> &'static str {
        match self.vendor_id {
            0x8086 => "Intel",
            0x1022 => "AMD",
            0x10DE => "NVIDIA",
            0x1002 => "ATI/AMD",
            0x15AD => "VMware",
            0x1AF4 => "VirtIO",
            0x1234 => "QEMU",
            0x80EE => "VirtualBox",
            _ => "Unknown Vendor",
        }
    }
}

impl Default for DeviceInfo {
    fn default() -> Self {
        Self::new(0, 0, BusType::Virtual)
    }
}

/// Device capabilities
bitflags::bitflags! {
    pub struct DeviceCapabilities: u32 {
        /// Device supports power management
        const POWER_MANAGEMENT = 1 << 0;
        /// Device supports MSI interrupts
        const MSI = 1 << 1;
        /// Device supports MSI-X interrupts
        const MSI_X = 1 << 2;
        /// Device supports 64-bit addressing
        const ADDRESS_64BIT = 1 << 3;
        /// Device supports DMA
        const DMA = 1 << 4;
        /// Device is hot-pluggable
        const HOT_PLUG = 1 << 5;
        /// Device supports advanced error reporting
        const ADVANCED_ERROR_REPORTING = 1 << 6;
    }
}

/// Resource type for device resources
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ResourceType {
    /// Memory resource
    Memory,
    /// I/O port resource
    IoPort,
    /// IRQ resource
    Irq,
    /// DMA channel resource
    Dma,
}

/// Device resource descriptor
#[derive(Debug, Clone, Copy)]
pub struct DeviceResource {
    /// Resource type
    pub resource_type: ResourceType,
    /// Start address/number
    pub start: u64,
    /// End address/number (inclusive)
    pub end: u64,
    /// Resource flags
    pub flags: u32,
}

impl DeviceResource {
    /// Create a new memory resource
    pub fn memory(start: u64, size: u64) -> Self {
        Self {
            resource_type: ResourceType::Memory,
            start,
            end: start + size - 1,
            flags: 0,
        }
    }
    
    /// Create a new I/O port resource
    pub fn io_port(start: u16, size: u16) -> Self {
        Self {
            resource_type: ResourceType::IoPort,
            start: start as u64,
            end: (start + size - 1) as u64,
            flags: 0,
        }
    }
    
    /// Create a new IRQ resource
    pub fn irq(irq_num: u8) -> Self {
        Self {
            resource_type: ResourceType::Irq,
            start: irq_num as u64,
            end: irq_num as u64,
            flags: 0,
        }
    }
    
    /// Get resource size
    pub fn size(&self) -> u64 {
        self.end - self.start + 1
    }
}
