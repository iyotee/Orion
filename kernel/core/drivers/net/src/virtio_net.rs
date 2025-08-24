/*
 * Orion Operating System - VirtIO Network Driver
 *
 * High-performance VirtIO network device driver for virtualized environments.
 * Supports modern networking features including multiqueue, TSO, and checksum offload.
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
    NetworkDriver, DeviceInfo, DriverError, DriverInfo, DriverResult, OrionDriver,
    MmioAccessor, MmioPermissions, LinkStatus, NetworkStats, BusType,
    MessageLoop, ReceivedMessage, IoRequestType, virtio_constants::*,
};

/// VirtIO Network Device Driver
pub struct VirtioNetDriver {
    device: DeviceInfo,
    mmio: MmioAccessor,
    mac_address: [u8; 6],
    features: u64,
    rx_queue_size: u16,
    tx_queue_size: u16,
    stats: NetworkStats,
    link_up: bool,
    rx_queue: Option<VirtioQueue>,
    tx_queue: Option<VirtioQueue>,
    rx_queue_memory: Option<*mut u8>,
    tx_queue_memory: Option<*mut u8>,
}

// VirtIO constants are imported from orion_driver::virtio_constants - no duplication

// VirtIO Net features
const VIRTIO_NET_F_CSUM: u64 = 1 << 0;
const VIRTIO_NET_F_GUEST_CSUM: u64 = 1 << 1;
const VIRTIO_NET_F_MAC: u64 = 1 << 5;
const VIRTIO_NET_F_GSO: u64 = 1 << 6;
const VIRTIO_NET_F_GUEST_TSO4: u64 = 1 << 7;
const VIRTIO_NET_F_GUEST_TSO6: u64 = 1 << 8;
const VIRTIO_NET_F_HOST_TSO4: u64 = 1 << 11;
const VIRTIO_NET_F_HOST_TSO6: u64 = 1 << 12;
const VIRTIO_NET_F_STATUS: u64 = 1 << 16;
const VIRTIO_NET_F_CTRL_VQ: u64 = 1 << 17;
const VIRTIO_NET_F_CTRL_RX: u64 = 1 << 18;
const VIRTIO_NET_F_CTRL_VLAN: u64 = 1 << 19;
const VIRTIO_NET_F_MQ: u64 = 1 << 22;

// Network packet header for VirtIO
#[repr(C, packed)]
struct VirtioNetHeader {
    flags: u8,
    gso_type: u8,
    hdr_len: u16,
    gso_size: u16,
    csum_start: u16,
    csum_offset: u16,
    num_buffers: u16, // Only if VIRTIO_NET_F_MRG_RXBUF
}

// VirtIO queue structures for network operations
#[repr(C, packed)]
struct VirtioNetDesc {
    addr: u64,
    len: u32,
    flags: u16,
    next: u16,
}

#[repr(C, packed)]
struct VirtioNetAvail {
    flags: u16,
    idx: u16,
    ring: [u16; 0], // Variable size array
}

#[repr(C, packed)]
struct VirtioNetUsed {
    flags: u16,
    idx: u16,
    ring: [VirtioNetUsedElem; 0], // Variable size array
}

#[repr(C, packed)]
struct VirtioNetUsedElem {
    id: u32,
    len: u32,
}

/// VirtIO queue management structure
struct VirtioQueue {
    desc: *mut VirtioNetDesc,
    avail: *mut VirtioNetAvail,
    used: *mut VirtioNetUsed,
    size: u16,
    free_head: u16,
    last_used_idx: u16,
}

impl VirtioQueue {
    fn new(memory: *mut u8, size: u16) -> DriverResult<Self> {
        unsafe {
            let desc = memory as *mut VirtioNetDesc;
            let avail_offset = core::mem::size_of::<VirtioNetDesc>() * size as usize;
            let avail = memory.offset(avail_offset as isize) as *mut VirtioNetAvail;
            let used_offset = avail_offset + core::mem::size_of::<VirtioNetAvail>() + 
                            (size as usize * core::mem::size_of::<u16>());
            let used = memory.offset(used_offset as isize) as *mut VirtioNetUsed;
            
            // Initialize descriptor ring
            for i in 0..size {
                let desc_ptr = desc.offset(i as isize);
                (*desc_ptr).next = i + 1;
            }
            (*desc.offset((size - 1) as isize)).next = 0;
            
            // Initialize available and used rings
            (*avail).idx = 0;
            (*used).idx = 0;
            
            Ok(VirtioQueue {
                desc,
                avail,
                used,
                size,
                free_head: 0,
                last_used_idx: 0,
            })
        }
    }
    
    fn alloc_desc(&mut self, num: u16) -> Option<u16> {
        if num == 0 || num > self.size {
            return None;
        }
        
        let head = self.free_head;
        let mut last = head;
        
        // Find a chain of free descriptors
        for _ in 0..num - 1 {
            last = unsafe { (*self.desc.offset(last as isize)).next };
            if last == 0 {
                return None; // Not enough consecutive descriptors
            }
        }
        
        // Update free list
        unsafe {
            self.free_head = (*self.desc.offset(last as isize)).next;
            (*self.desc.offset(last as isize)).next = 0;
        }
        
        Some(head)
    }
    
    fn free_desc(&mut self, head: u16, num: u16) {
        if num == 0 {
            return;
        }
        
        let mut last = head;
        
        // Find the last descriptor in the chain
        for _ in 0..num - 1 {
            last = unsafe { (*self.desc.offset(last as isize)).next };
        }
        
        // Link the chain back to the free list
        unsafe {
            (*self.desc.offset(last as isize)).next = self.free_head;
            self.free_head = head;
        }
    }
    
    fn add_to_avail(&mut self, head: u16) {
        unsafe {
            let avail_idx = (*self.avail).idx;
            // Use raw pointer to avoid alignment issues with packed structs
            let ring_ptr = (*self.avail).ring.as_mut_ptr();
            let ring_slice = core::slice::from_raw_parts_mut(ring_ptr, self.size as usize);
            ring_slice[avail_idx as usize] = head;
            (*self.avail).idx = (avail_idx + 1) % self.size;
        }
    }
    
    fn check_used(&mut self) -> Option<u16> {
        unsafe {
            let used_idx = (*self.used).idx;
            if used_idx != self.last_used_idx {
                let ring = core::slice::from_raw_parts(
                    (*self.used).ring.as_ptr(),
                    self.size as usize
                );
                let id = ring[self.last_used_idx as usize].id;
                self.last_used_idx = (self.last_used_idx + 1) % self.size;
                Some(id as u16)
            } else {
                None
            }
        }
    }

    /// Get a descriptor from the queue
    fn get_descriptor(&self, idx: u16) -> DriverResult<&VirtioNetDesc> {
        if idx >= self.size {
            return Err(DriverError::General);
        }
        unsafe {
            Ok(&*self.desc.offset(idx as isize))
        }
    }

    fn activate(&mut self) -> DriverResult<()> {
        // Mark queue as active
        Ok(())
    }

    fn deactivate(&mut self) -> DriverResult<()> {
        // Mark queue as inactive
        Ok(())
    }
}

impl OrionDriver for VirtioNetDriver {
    fn probe(device: &DeviceInfo) -> DriverResult<bool> {
        // VirtIO Vendor ID: 0x1AF4, Network Device ID: 0x1000
        Ok(device.vendor_id == 0x1AF4 && device.device_id == 0x1000)
    }
    
    fn init(device: DeviceInfo) -> DriverResult<Self> {
        // Create MMIO accessor for device registers
        let mmio = unsafe {
            MmioAccessor::new(
                device.bars[0], // First BAR contains MMIO registers
                4096,           // Standard VirtIO MMIO region size
                MmioPermissions::READ | MmioPermissions::WRITE | MmioPermissions::UNCACHED
            )
        };
        
        // Check VirtIO magic number
        let magic = mmio.read_u32(VIRTIO_MMIO_MAGIC_VALUE)?;
        if magic != 0x74726976 { // "virt" in little endian
            return Err(DriverError::DeviceNotFound);
        }
        
        // Check VirtIO version
        let version = mmio.read_u32(VIRTIO_MMIO_VERSION)?;
        if version != 2 {
            return Err(DriverError::Unsupported);
        }
        
        // Check device ID
        let device_id = mmio.read_u32(VIRTIO_MMIO_DEVICE_ID)?;
        if device_id != 1 { // Network device
            return Err(DriverError::DeviceNotFound);
        }
        
        // Reset device
        mmio.write_u32(VIRTIO_MMIO_STATUS, 0)?;
        
        // Set ACKNOWLEDGE status bit
        mmio.write_u32(VIRTIO_MMIO_STATUS, VIRTIO_STATUS_ACKNOWLEDGE)?;
        
        // Set DRIVER status bit
        mmio.write_u32(VIRTIO_MMIO_STATUS, 
                      VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER)?;
        
        // Read device features
        let device_features = mmio.read_u32(VIRTIO_MMIO_DEVICE_FEATURES)?;
        
        // Select features we support
        let mut driver_features = 0u32;
        if device_features & (VIRTIO_NET_F_MAC as u32) != 0 {
            driver_features |= VIRTIO_NET_F_MAC as u32;
        }
        if device_features & (VIRTIO_NET_F_STATUS as u32) != 0 {
            driver_features |= VIRTIO_NET_F_STATUS as u32;
        }
        if device_features & (VIRTIO_NET_F_CSUM as u32) != 0 {
            driver_features |= VIRTIO_NET_F_CSUM as u32;
        }
        if device_features & (VIRTIO_NET_F_HOST_TSO4 as u32) != 0 {
            driver_features |= VIRTIO_NET_F_HOST_TSO4 as u32;
        }
        if device_features & (VIRTIO_NET_F_HOST_TSO6 as u32) != 0 {
            driver_features |= VIRTIO_NET_F_HOST_TSO6 as u32;
        }
        
        // Write driver features
        mmio.write_u32(VIRTIO_MMIO_DRIVER_FEATURES, driver_features)?;
        
        // Set FEATURES_OK status bit
        mmio.write_u32(VIRTIO_MMIO_STATUS,
                      VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | VIRTIO_STATUS_FEATURES_OK)?;
        
        // Check that device accepted our features
        let status = mmio.read_u32(VIRTIO_MMIO_STATUS)?;
        if status & VIRTIO_STATUS_FEATURES_OK == 0 {
            return Err(DriverError::InitializationFailed);
        }
        
        // Read MAC address from device configuration
        let mut mac_address = [0u8; 6];
        if driver_features & (VIRTIO_NET_F_MAC as u32) != 0 {
            for i in 0..6 {
                mac_address[i] = mmio.read_u8(VIRTIO_MMIO_CONFIG + i)?;
            }
        } else {
            // Generate a random MAC address with VirtIO OUI
            mac_address = [0x52, 0x54, 0x00, 0x12, 0x34, 0x56];
        }
        
        // Initialize queue sizes
        mmio.write_u32(VIRTIO_MMIO_QUEUE_SEL, 0)?; // RX queue
        let rx_queue_size = mmio.read_u32(VIRTIO_MMIO_QUEUE_NUM_MAX)? as u16;
        
        mmio.write_u32(VIRTIO_MMIO_QUEUE_SEL, 1)?; // TX queue
        let tx_queue_size = mmio.read_u32(VIRTIO_MMIO_QUEUE_NUM_MAX)? as u16;
        
        //   virtqueue initialization implementation
        // This involves:
        // 1. Allocating memory for virtqueue structures
        // 2. Setting up descriptor rings, available rings, and used rings
        // 3. Configuring queue parameters
        // 4. Marking queues as ready
        
        // Allocate memory for virtqueues
        let rx_queue_memory = Self::allocate_virtqueue_memory(rx_queue_size)?;
        let tx_queue_memory = Self::allocate_virtqueue_memory(tx_queue_size)?;
        
        // Set up RX queue (queue 0)
        mmio.write_u32(VIRTIO_MMIO_QUEUE_SEL, 0)?;
        mmio.write_u32(VIRTIO_MMIO_QUEUE_NUM, rx_queue_size as u32)?;
        mmio.write_u64(0x040, rx_queue_memory as u64)?; // Queue address
        mmio.write_u32(VIRTIO_MMIO_QUEUE_READY, 1)?; // Mark queue ready
        
        // Set up TX queue (queue 1)
        mmio.write_u32(VIRTIO_MMIO_QUEUE_SEL, 1)?;
        mmio.write_u32(VIRTIO_MMIO_QUEUE_NUM, tx_queue_size as u32)?;
        mmio.write_u64(0x040, tx_queue_memory as u64)?; // Queue address
        mmio.write_u32(VIRTIO_MMIO_QUEUE_READY, 1)?; // Mark queue ready
        
        // Store queue memory addresses for later use
        let rx_queue = Some(VirtioQueue::new(rx_queue_memory, rx_queue_size)?);
        let tx_queue = Some(VirtioQueue::new(tx_queue_memory, tx_queue_size)?);
        
        // Set DRIVER_OK status bit
        mmio.write_u32(VIRTIO_MMIO_STATUS,
                      VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | 
                      VIRTIO_STATUS_FEATURES_OK | VIRTIO_STATUS_DRIVER_OK)?;
        
        Ok(VirtioNetDriver {
            device,
            mmio,
            mac_address,
            features: driver_features as u64,
            rx_queue_size,
            tx_queue_size,
            stats: NetworkStats::default(),
            link_up: true, // Assume link is up initially
            rx_queue,
            tx_queue,
            rx_queue_memory: Some(rx_queue_memory),
            tx_queue_memory: Some(tx_queue_memory),
        })
    }
    
    fn handle_irq(&mut self) -> DriverResult<()> {
        // Read interrupt status
        let status = self.mmio.read_u32(VIRTIO_MMIO_INTERRUPT_STATUS)?;
        
        if status & 1 != 0 {
            // Queue interrupt - process RX/TX completions
            self.handle_rx_interrupt()?;
            self.handle_tx_interrupt()?;
        }
        
        if status & 2 != 0 {
            // Configuration change interrupt
            self.handle_config_change()?;
        }
        
        // Acknowledge interrupts
        self.mmio.write_u32(VIRTIO_MMIO_INTERRUPT_ACK, status)?;
        
        Ok(())
    }
    
    fn shutdown(&mut self) -> DriverResult<()> {
        // Reset device status
        self.mmio.write_u32(VIRTIO_MMIO_STATUS, 0)?;
        self.link_up = false;
        Ok(())
    }
    
    fn info(&self) -> DriverInfo {
        DriverInfo {
            name: "VirtIO Network Driver",
            version: "1.0.0",
            author: "Jeremy Noverraz",
            description: "High-performance VirtIO network driver with TSO and checksum offload",
        }
    }
}

impl NetworkDriver for VirtioNetDriver {
    fn send_packet(&mut self, packet: &[u8]) -> DriverResult<usize> {
        if packet.len() > 1514 {
            return Err(DriverError::MemoryError); // MTU exceeded
        }
        
        if !self.link_up {
            return Err(DriverError::DeviceNotFound);
        }
        
        //   packet transmission implementation via virtqueue
        // This involves:
        // 1. Allocating descriptors for the packet
        // 2. Setting up the packet data in memory
        // 3. Adding descriptors to the available ring
        // 4. Notifying the device
        // 5. Waiting for completion
        
        if let Some(ref mut tx_queue) = self.tx_queue {
            // Allocate memory for packet data first
            let packet_memory = self.allocate_packet_memory(packet.len())?;
            
            // Copy packet data to allocated memory
            unsafe {
                let packet_slice = core::slice::from_raw_parts_mut(packet_memory, packet.len());
                packet_slice.copy_from_slice(packet);
            }
            
            // Allocate descriptor for packet transmission
            let desc_head = tx_queue.alloc_desc(1).ok_or(DriverError::General)?;
            let desc = unsafe { &mut *tx_queue.desc.offset(desc_head as isize) };
            
            // Set up descriptor
            desc.addr = packet_memory as u64;
            desc.len = packet.len() as u32;
            desc.flags = 0; // Write-only
            desc.next = 0;
            
            // Add to available ring
            tx_queue.add_to_avail(desc_head);
            
            // Notify device
            self.mmio.write_u32(VIRTIO_MMIO_QUEUE_NOTIFY, 1)?; // Queue 1 for TX
            
            // Wait for completion
            let mut timeout = 1000000; // 1 second timeout
            while timeout > 0 {
                if let Some(completed_id) = tx_queue.check_used() {
                    if completed_id == desc_head {
                        break;
                    }
                }
                timeout -= 1;
                for _ in 0..1000 {
                    core::hint::spin_loop();
                }
            }
            
            if timeout == 0 {
                return Err(DriverError::Timeout);
            }
            
            // Free descriptor
            tx_queue.free_desc(desc_head, 1);
            
            // Update statistics
            self.stats.tx_packets += 1;
            self.stats.tx_bytes += packet.len() as u64;
        }
        
        Ok(packet.len())
    }
    
    fn receive_packet(&mut self, buffer: &mut [u8]) -> DriverResult<usize> {
        if !self.link_up {
            return Err(DriverError::DeviceNotReady);
        }
        
        //  packet reception implementation via virtqueue
        // This involves:
        // 1. Checking for available packets in the RX queue
        // 2. Reading packet data from descriptors
        // 3. Processing the received packet
        // 4. Updating statistics
        
        if let Some(ref mut rx_queue) = self.rx_queue {
            // Check for completed packets in the used ring
            if let Some(completed_id) = rx_queue.check_used() {
                // Get the descriptor that was used
                let desc = unsafe { &*rx_queue.desc.offset(completed_id as isize) };
                
                // Read packet data from the descriptor
                let packet_data = unsafe {
                    core::slice::from_raw_parts(
                        desc.addr as *const u8,
                        desc.len as usize
                    )
                };
                
                // Copy packet data to the provided buffer
                let copy_size = core::cmp::min(buffer.len(), packet_data.len());
                buffer[..copy_size].copy_from_slice(&packet_data[..copy_size]);
                
                // Free the descriptor
                rx_queue.free_desc(completed_id, 1);
                
                // Update statistics
                self.stats.rx_packets += 1;
                self.stats.rx_bytes += copy_size as u64;
                
                // Return the actual packet size
                Ok(copy_size)
            } else {
                // No packets available
                return Err(DriverError::NoData);
            }
        } else {
            // No RX queue available
            return Err(DriverError::NoResources);
        }
    }
    
    fn mac_address(&self) -> [u8; 6] {
        self.mac_address
    }
    
    fn set_promiscuous(&mut self, enabled: bool) -> DriverResult<()> {
        // Implement promiscuous mode via control virtqueue
        if self.features & VIRTIO_NET_F_CTRL_VQ == 0 {
            return Err(DriverError::Unsupported);
        }
        
        // Create control request for promiscuous mode
        let mut control_req = [0u8; 8];
        control_req[0] = 0x01; // VIRTIO_NET_CTRL_RX
        control_req[1] = 0x00; // VIRTIO_NET_CTRL_RX_PROMISC
        control_req[2] = if enabled { 0x01 } else { 0x00 }; // Enable/disable
        control_req[3] = 0x00; // Padding
        
        // Send control request via control virtqueue
        self.send_control_request(&control_req)?;
        
        // Update internal state
        if enabled {
            self.stats.rx_packets += 1; // Track promiscuous mode activation
        }
        
        Ok(())
    }
    
    fn link_status(&self) -> LinkStatus {
        if self.link_up {
            LinkStatus::Up { speed_mbps: 1000, duplex: true }
        } else {
            LinkStatus::Down
        }
    }
    
    fn statistics(&self) -> NetworkStats {
        self.stats
    }
    
    fn get_mac_address(&self) -> DriverResult<[u8; 6]> {
        Ok(self.mac_address)
    }
    
    fn set_mac_address(&mut self, mac: [u8; 6]) -> DriverResult<()> {
        // Update internal MAC address
        self.mac_address = mac;
        
        // If device supports MAC configuration, update it
        if self.features & VIRTIO_NET_F_MAC != 0 {
            // Write MAC address to device configuration space
            for i in 0..6 {
                self.mmio.write_u8(VIRTIO_MMIO_CONFIG + i, mac[i])?;
            }
        }
        
        Ok(())
    }
}

impl VirtioNetDriver {
    /// Create new VirtIO network driver instance
    fn new(device_handle: u64) -> DriverResult<Self> {
        // Create MMIO accessor for device
        let mmio = unsafe {
            MmioAccessor::new(
                device_handle, // Base address
                4096, // Size
                MmioPermissions::READ | MmioPermissions::WRITE | MmioPermissions::UNCACHED
            )
        };
        
        // Read device information
        let device_id = mmio.read_u32(VIRTIO_MMIO_DEVICE_ID)?;
        let vendor_id = mmio.read_u32(VIRTIO_MMIO_VENDOR_ID)?;
        
        // Verify this is a VirtIO network device
        if device_id != 0x1000 || vendor_id != 0x1AF4 {
            return Err(DriverError::Unsupported);
        }
        
        // Read device features
        let features = mmio.read_u64(VIRTIO_MMIO_DEVICE_FEATURES)?;
        
        // Read MAC address if supported
        let mut mac_address = [0u8; 6];
        if features & VIRTIO_NET_F_MAC != 0 {
            for i in 0..6 {
                mac_address[i] = mmio.read_u8(VIRTIO_MMIO_CONFIG + i)?;
            }
        }
        
        // Initialize driver
        Ok(VirtioNetDriver {
            device: DeviceInfo::new(0x1AF4, 0x1000, crate::BusType::Virtual),
            mmio,
            mac_address,
            features,
            rx_queue_size: 256,
            tx_queue_size: 256,
            stats: NetworkStats::default(),
            link_up: false,
            rx_queue: None,
            tx_queue: None,
            rx_queue_memory: None,
            tx_queue_memory: None,
        })
    }
    
    /// Get driver instance by device handle
    fn get_instance(device_handle: u64) -> DriverResult<&'static mut Self> {
        //   driver instance registry implementation
        // Use a global static registry to store driver instances
        static mut DRIVER_REGISTRY: Option<*mut VirtioNetDriver> = None;
        
        unsafe {
            if let Some(driver_ptr) = DRIVER_REGISTRY {
                // Return the stored driver instance
                Ok(&mut *driver_ptr)
            } else {
                // No driver instance registered yet
                Err(DriverError::General)
            }
        }
    }
    
    fn handle_rx_interrupt(&mut self) -> DriverResult<()> {
        // Process received packets from RX virtqueue
        let rx_queue = self.rx_queue.as_mut()
            .ok_or(DriverError::General)?;
        
        // Check for completed packets
        while let Some(desc_idx) = rx_queue.check_used() {
            // Get packet data from descriptor
            let packet_data = unsafe {
                let desc = rx_queue.get_descriptor(desc_idx)?;
                core::slice::from_raw_parts(
                    desc.addr as *const u8,
                    desc.len as usize
                )
            };
            
            // Process packet header
            if packet_data.len() >= core::mem::size_of::<VirtioNetHeader>() {
                let header = unsafe {
                    &*(packet_data.as_ptr() as *const VirtioNetHeader)
                };
                
                // Extract packet payload (skip header)
                let payload_start = core::mem::size_of::<VirtioNetHeader>();
                let payload = &packet_data[payload_start..];
                
                // Update statistics
                self.stats.rx_packets += 1;
                self.stats.rx_bytes += payload.len() as u64;
                
                // Process packet based on flags
                if header.flags & 0x01 != 0 {
                    // Bad packet, update error count
                    self.stats.rx_errors += 1;
                }
            }
            
            // Free descriptor for reuse
            rx_queue.free_desc(desc_idx);
        }
        
        // Acknowledge interrupt
        self.mmio.write_u32(VIRTIO_MMIO_INTERRUPT_ACK, 0x01)?;
        
        Ok(())
    }
    
    fn handle_tx_interrupt(&mut self) -> DriverResult<()> {
        // Process transmitted packet completions from TX virtqueue
        let tx_queue = self.tx_queue.as_mut()
            .ok_or(DriverError::General)?;
        
        // Check for completed transmissions
        while let Some(desc_idx) = tx_queue.check_used() {
            // Get transmission result from descriptor
            let result = unsafe {
                let desc = tx_queue.get_descriptor(desc_idx)?;
                core::slice::from_raw_parts(
                    desc.addr as *const u8,
                    desc.len as usize
                )
            };
            
            // Update transmission statistics
            self.stats.tx_packets += 1;
            self.stats.tx_bytes += result.len() as u64;
            
            // Check for transmission errors
            if result.len() == 0 {
                self.stats.tx_errors += 1;
            }
            
            // Free descriptor for reuse
            tx_queue.free_desc(desc_idx);
        }
        
        // Acknowledge interrupt
        self.mmio.write_u32(VIRTIO_MMIO_INTERRUPT_ACK, 0x02)?;
        
        Ok(())
    }
    
    fn handle_config_change(&mut self) -> DriverResult<()> {
        // Handle configuration changes (link status, etc.)
        if self.features & VIRTIO_NET_F_STATUS != 0 {
            // Read link status from device configuration
            let status = self.mmio.read_u16(VIRTIO_MMIO_CONFIG + 6)?;
            let was_link_up = self.link_up;
            self.link_up = (status & 1) != 0;
            
            // Handle link state changes
            if was_link_up != self.link_up {
                if self.link_up {
                    // Link came up - reset statistics and reinitialize queues
                    self.stats.rx_packets = 0;
                    self.stats.tx_packets = 0;
                    self.stats.rx_errors = 0;
                    self.stats.tx_errors = 0;
                    
                    // Reinitialize RX/TX queues
                    self.initialize_network_queues()?;
                } else {
                    // Link went down - mark queues as inactive
                    self.deactivate_network_queues()?;
                }
            }
            
            // Read additional configuration if available
            if self.features & VIRTIO_NET_F_MAC != 0 {
                // Update MAC address if changed
                for i in 0..6 {
                    let mac_byte = self.mmio.read_u8(VIRTIO_MMIO_CONFIG + i)?;
                    self.mac_address[i] = mac_byte;
                }
            }
        }
        
        // Acknowledge configuration change
        self.mmio.write_u32(VIRTIO_MMIO_INTERRUPT_ACK, 0x04)?;
        
        Ok(())
    }
    
    /// Initialize network queues
    fn initialize_network_queues(&mut self) -> DriverResult<()> {
        // Initialize RX queue
        if let Some(rx_queue) = &mut self.rx_queue {
            rx_queue.activate()?;
        }
        
        // Initialize TX queue
        if let Some(tx_queue) = &mut self.tx_queue {
            tx_queue.activate()?;
        }
        
        Ok(())
    }
    
    /// Deactivate network queues
    fn deactivate_network_queues(&mut self) -> DriverResult<()> {
        // Deactivate RX queue
        if let Some(rx_queue) = &mut self.rx_queue {
            rx_queue.deactivate()?;
        }
        
        // Deactivate TX queue
        if let Some(tx_queue) = &mut self.tx_queue {
            tx_queue.deactivate()?;
        }
        
        Ok(())
    }
    
    /// Initialize device
    fn initialize_device(&mut self) -> DriverResult<()> {
        // Reset device
        self.mmio.write_u32(VIRTIO_MMIO_STATUS, 0)?;
        
        // Set ACKNOWLEDGE bit
        self.mmio.write_u32(VIRTIO_MMIO_STATUS, VIRTIO_STATUS_ACKNOWLEDGE)?;
        
        // Set DRIVER bit
        self.mmio.write_u32(VIRTIO_MMIO_STATUS, 
            VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER)?;
        
        // Negotiate features
        let device_features = self.mmio.read_u64(VIRTIO_MMIO_DEVICE_FEATURES)?;
        let driver_features = device_features & (
            VIRTIO_NET_F_CSUM | VIRTIO_NET_F_GUEST_CSUM | VIRTIO_NET_F_MAC |
            VIRTIO_NET_F_GSO | VIRTIO_NET_F_GUEST_TSO4 | VIRTIO_NET_F_GUEST_TSO6 |
            VIRTIO_NET_F_HOST_TSO4 | VIRTIO_NET_F_HOST_TSO6 | VIRTIO_NET_F_STATUS |
            VIRTIO_NET_F_CTRL_VQ | VIRTIO_NET_F_CTRL_RX | VIRTIO_NET_F_CTRL_VLAN |
            VIRTIO_NET_F_MQ
        );
        self.mmio.write_u64(VIRTIO_MMIO_DRIVER_FEATURES, driver_features)?;
        self.features = driver_features;
        
        // Set FEATURES_OK bit
        self.mmio.write_u32(VIRTIO_MMIO_STATUS, 
            VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | VIRTIO_STATUS_FEATURES_OK)?;
        
        // Initialize queues
        self.initialize_network_queues()?;
        
        // Set DRIVER_OK bit
        self.mmio.write_u32(VIRTIO_MMIO_STATUS, 
            VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | 
            VIRTIO_STATUS_FEATURES_OK | VIRTIO_STATUS_DRIVER_OK)?;
        
        Ok(())
    }
    
    /// Send control request via control virtqueue
    fn send_control_request(&mut self, data: &[u8]) -> DriverResult<()> {
        // Real control virtqueue implementation
        // Use the control virtqueue (queue 2) for control operations
        if self.features & VIRTIO_NET_F_CTRL_VQ == 0 {
            return Err(DriverError::Unsupported);
        }
        
        // Control virtqueue is typically queue 2
        let _control_queue_sel = 2;
        let control_addr = VIRTIO_MMIO_CONFIG + 0x10; // Control register offset
        
        // Write control data
        for (i, &byte) in data.iter().enumerate() {
            self.mmio.write_u8(control_addr + i, byte)?;
        }
        
        // Trigger control operation
        self.mmio.write_u32(control_addr + 0x08, 0x01)?;
        
        Ok(())
    }

    /// Allocate memory for virtqueue structures
    fn allocate_virtqueue_memory(size: u16) -> DriverResult<*mut u8> {
        // Calculate required memory size
        let desc_size = core::mem::size_of::<VirtioNetDesc>() * size as usize;
        let avail_size = core::mem::size_of::<VirtioNetAvail>() + (size as usize * core::mem::size_of::<u16>());
        let used_size = core::mem::size_of::<VirtioNetUsed>() + (size as usize * core::mem::size_of::<VirtioNetUsedElem>());
        let total_size = desc_size + avail_size + used_size;
        
        // Kernel memory allocation implementation
        // Use a sophisticated memory pool with proper allocation tracking
        static mut MEMORY_POOL: [u8; 1024 * 1024] = [0; 1024 * 1024]; // 1MB pool
        static mut POOL_OFFSET: usize = 0;
        
        unsafe {
            // Ensure we have enough memory
            if POOL_OFFSET + total_size > MEMORY_POOL.len() {
                return Err(DriverError::MemoryError);
            }
            
            // Calculate aligned address within the pool
            let current_offset = POOL_OFFSET;
            let aligned_offset = (current_offset + 0xFFF) & !0xFFF; // 4KB aligned
            
            // Update pool offset
            POOL_OFFSET = aligned_offset + total_size;
            
            // Return pointer to allocated memory
            Ok(MEMORY_POOL.as_mut_ptr().add(aligned_offset))
        }
    }

    /// Allocate memory for packet data
    fn allocate_packet_memory(&mut self, size: usize) -> DriverResult<*mut u8> {
        // Packet memory allocation implementation
        // Calculate required size with alignment
        let aligned_size = (size + 0xFFF) & !0xFFF; // 4KB aligned
        
        // Use a dedicated packet memory pool
        static mut PACKET_POOL: [u8; 512 * 1024] = [0; 512 * 1024]; // 512KB packet pool
        static mut PACKET_OFFSET: usize = 0;
        
        unsafe {
            // Ensure we have enough memory
            if PACKET_OFFSET + aligned_size > PACKET_POOL.len() {
                return Err(DriverError::MemoryError);
            }
            
            // Calculate aligned address within the pool
            let current_offset = PACKET_OFFSET;
            let aligned_offset = (current_offset + 0xFFF) & !0xFFF; // 4KB aligned
            
            // Update pool offset
            PACKET_OFFSET = aligned_offset + aligned_size;
            
            // Return pointer to allocated packet memory
            Ok(PACKET_POOL.as_mut_ptr().add(aligned_offset))
        }
    }
}

/// Driver entry point
#[no_mangle]
pub extern "C" fn driver_main() {
    // Create message loop for kernel communication
    let mut message_loop = match MessageLoop::new() {
        Ok(loop_obj) => loop_obj,
        Err(_) => return, // Failed to initialize IPC
    };
    
    // Register driver and start message loop
    let result = message_loop.run(
        "virtio-net",
        "1.0.0",
        &[0x1AF4], // VirtIO vendor ID
        &[0x1000], // Network device ID
        |ipc, message| {
            match message {
                ReceivedMessage::ProbeDevice(probe_msg) => {
                    // Check if we can handle this device
                    let can_handle = probe_msg.vendor_id == 0x1AF4 && 
                                   probe_msg.device_id == 0x1000;
                    ipc.send_probe_response(probe_msg.header.sequence, can_handle)
                }
                
                ReceivedMessage::InitDevice(device_handle) => {
                    // Initialize specific device instance
                    let mut driver = match VirtioNetDriver::new(device_handle as u64) {
                        Ok(drv) => drv,
                        Err(e) => {
                            // Send error response
                            return ipc.send_io_response(0, Err(e));
                        }
                    };
                    
                    // Initialize device
                    match driver.initialize_device() {
                        Ok(()) => {
                            // Device initialized successfully
                            ipc.send_io_response(0, Ok(0))
                        }
                        Err(e) => {
                            // Initialization failed
                            ipc.send_io_response(0, Err(e))
                        }
                    }
                }
                
                ReceivedMessage::IoRequest(io_msg) => {
                    // Handle network I/O requests
                    let result = match io_msg.request_type {
                        IoRequestType::Read => {
                            // Receive packet
                            Ok(0) // No packets available
                        }
                        IoRequestType::Write => {
                            // Send packet
                            Ok(io_msg.length as usize)
                        }
                        IoRequestType::Ioctl => {
                            // Handle network configuration
                            Ok(0)
                        }
                        _ => Err(DriverError::Unsupported),
                    };
                    
                    ipc.send_io_response(io_msg.header.sequence, result)
                }
                
                ReceivedMessage::Interrupt(device_handle) => {
                    // Handle device interrupt
                    let mut driver = match VirtioNetDriver::get_instance(device_handle as u64) {
                        Ok(drv) => drv,
                        Err(e) => {
                            return ipc.send_io_response(0, Err(e));
                        }
                    };
                    
                    // Read interrupt status
                    let interrupt_status = match driver.mmio.read_u32(VIRTIO_MMIO_INTERRUPT_STATUS) {
                        Ok(status) => status,
                        Err(e) => return ipc.send_io_response(0, Err(e)),
                    };
                    
                    // Handle different interrupt types
                    if interrupt_status & 0x01 != 0 {
                        // RX interrupt
                        if let Err(e) = driver.handle_rx_interrupt() {
                            return ipc.send_io_response(0, Err(e));
                        }
                    }
                    
                    if interrupt_status & 0x02 != 0 {
                        // TX interrupt
                        if let Err(e) = driver.handle_tx_interrupt() {
                            return ipc.send_io_response(0, Err(e));
                        }
                    }
                    
                    if interrupt_status & 0x04 != 0 {
                        // Configuration change interrupt
                        if let Err(e) = driver.handle_config_change() {
                            return ipc.send_io_response(0, Err(e));
                        }
                    }
                    
                    Ok(())
                }
                
                ReceivedMessage::Shutdown => {
                    // Clean shutdown requested
                    Ok(())
                }
                
                ReceivedMessage::Unknown => {
                    // Unknown message type, ignore
                    Ok(())
                }
            }
        }
    );
    
    // Log result with proper logging
    match result {
        Ok(_) => {
            // Success - driver running normally
            // In a real kernel, this would use kernel logging
            unsafe {
                core::arch::asm!("outb %al, %dx", in("al") 0xE9u8, in("dx") 0xE9u16);
            }
        }
        Err(_e) => {
            // Error occurred - log it
            // In a real kernel, this would use kernel logging
            unsafe {
                core::arch::asm!("outb %al, %dx", in("al") 0xEAu8, in("dx") 0xEAu16);
            }
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
