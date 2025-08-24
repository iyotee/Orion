/*
 * Orion Operating System - Realtek RTL8139 Network Driver
 *
 * Driver for the popular Realtek RTL8139 Fast Ethernet controller.
 * Widely used in older systems and virtualized environments.
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
    MmioAccessor, MmioPermissions, LinkStatus, NetworkStats,
    MessageLoop, ReceivedMessage, IoRequestType,
};

/// Realtek RTL8139 Network Driver
pub struct Rtl8139Driver {
    device: DeviceInfo,
    mmio: MmioAccessor,
    mac_address: [u8; 6],
    rx_buffer_size: usize,
    tx_buffer_count: usize,
    current_tx_buffer: usize,
    stats: NetworkStats,
    link_up: bool,
}

// RTL8139 register offsets
const RTL8139_MAC0: usize = 0x00;        // Ethernet hardware address
const RTL8139_MAR0: usize = 0x08;        // Multicast filter
const RTL8139_TXSTATUS0: usize = 0x10;   // Transmit status (4 32bit regs)
const RTL8139_TXADDR0: usize = 0x20;     // Tx descriptors (also 4 32bit)
const RTL8139_RXBUF: usize = 0x30;       // Receive buffer start address
const RTL8139_RXEARLYCNT: usize = 0x34;  // Early Rx byte count
const RTL8139_RXEARLYSTATUS: usize = 0x36; // Early Rx status
const RTL8139_CHIPCMD: usize = 0x37;     // Command register
const RTL8139_RXBUFTAIL: usize = 0x38;   // Current address of packet read (queue tail)
const RTL8139_RXBUFHEAD: usize = 0x3A;   // Current buffer address (queue head)
const RTL8139_INTRMASK: usize = 0x3C;    // Interrupt mask
const RTL8139_INTRSTATUS: usize = 0x3E;  // Interrupt status
const RTL8139_TXCONFIG: usize = 0x40;    // Tx configuration
const RTL8139_RXCONFIG: usize = 0x44;    // Rx configuration
const RTL8139_TIMER: usize = 0x48;       // A general purpose counter
const RTL8139_RXMISSED: usize = 0x4C;    // 24 bits valid, write clears
const RTL8139_CFG9346: usize = 0x50;     // 93C46 command register
const RTL8139_CONFIG0: usize = 0x51;     // Configuration reg 0
const RTL8139_CONFIG1: usize = 0x52;     // Configuration reg 1
const RTL8139_FLASHREG: usize = 0x54;    // Communication with Flash ROM
const RTL8139_MEDIASTAT: usize = 0x58;   // Media status register
const RTL8139_CONFIG3: usize = 0x59;     // Configuration reg 3
const RTL8139_CONFIG4: usize = 0x5A;     // Configuration reg 4
const RTL8139_HLTCLK: usize = 0x5B;      // Halt clock
const RTL8139_MULINT: usize = 0x5C;      // Multiple interrupt
const RTL8139_RERID: usize = 0x5E;       // PCI revision ID
const RTL8139_TXSUMMARY: usize = 0x60;   // Tx command/status summary
const RTL8139_BASICMODECONTROL: usize = 0x62; // Basic mode control register
const RTL8139_BASICMODESTATUS: usize = 0x64;  // Basic mode status register
const RTL8139_NWAYADVERT: usize = 0x66;  // Auto-negotiation advertisement reg
const RTL8139_NWAYLPAR: usize = 0x68;    // Auto-negotiation link partner reg
const RTL8139_NWAYEXPANSION: usize = 0x6A; // Auto-negotiation expansion reg

// Command register bits
const RTL8139_CMD_RESET: u8 = 0x10;      // Reset bit
const RTL8139_CMD_RX_ENABLE: u8 = 0x08;  // Receiver enable
const RTL8139_CMD_TX_ENABLE: u8 = 0x04;  // Transmitter enable
const RTL8139_CMD_RX_BUF_EMPTY: u8 = 0x01; // Rx buffer empty

// Interrupt register bits
const RTL8139_INT_PCIERR: u16 = 0x8000;  // PCI Bus error
const RTL8139_INT_PCS_TIMEOUT: u16 = 0x4000; // PCS timeout
const RTL8139_INT_RXFIFO_OVERFLOW: u16 = 0x0040; // Rx FIFO overflow
const RTL8139_INT_RXFIFO_UNDERRUN: u16 = 0x0020; // Packet underrun/link change
const RTL8139_INT_RXBUF_OVERFLOW: u16 = 0x0010;  // Rx buffer overflow
const RTL8139_INT_TX_ERR: u16 = 0x0008;  // Tx error
const RTL8139_INT_TX_OK: u16 = 0x0004;   // Tx okay
const RTL8139_INT_RX_ERR: u16 = 0x0002;  // Rx error
const RTL8139_INT_RX_OK: u16 = 0x0001;   // Rx okay

// Tx config register bits
const RTL8139_TX_CARRIER_LOST: u32 = 0x80000000; // Carrier sense lost
const RTL8139_TX_ABORT: u32 = 0x40000000;        // Transmission aborted
const RTL8139_TX_OUT_OF_WINDOW: u32 = 0x20000000; // Out of window collision
const RTL8139_TX_STATUS_OK: u32 = 0x00008000;    // Status ok: a good packet transmitted
const RTL8139_TX_UNDERRUN: u32 = 0x00004000;     // Transmit FIFO underrun
const RTL8139_TX_OWN: u32 = 0x00002000;          // Own bit
const RTL8139_TX_FIFO_THRESH_SHIFT: u32 = 11;    // FIFO threshold shift
const RTL8139_TX_SIZE_MASK: u32 = 0x00001FFF;    // Descriptor size mask

// Rx config register bits
const RTL8139_RX_CONFIG_FTH_SHIFT: u32 = 13;     // FIFO threshold shift
const RTL8139_RX_CONFIG_RBLEN_SHIFT: u32 = 11;   // Rx buffer length shift
const RTL8139_RX_CONFIG_MXDMA_SHIFT: u32 = 8;    // Max DMA burst shift
const RTL8139_RX_CONFIG_WRAP: u32 = 0x80;        // Rx buffer wrap
const RTL8139_RX_CONFIG_AER: u32 = 0x20;         // Accept error packets
const RTL8139_RX_CONFIG_AR: u32 = 0x10;          // Accept runt packets
const RTL8139_RX_CONFIG_AB: u32 = 0x08;          // Accept broadcast packets
const RTL8139_RX_CONFIG_AM: u32 = 0x04;          // Accept multicast packets
const RTL8139_RX_CONFIG_APM: u32 = 0x02;         // Accept physical match packets
const RTL8139_RX_CONFIG_AAP: u32 = 0x01;         // Accept all packets

// 93C46 config register bits
const RTL8139_CFG9346_LOCK: u8 = 0x00;     // Lock config registers
const RTL8139_CFG9346_UNLOCK: u8 = 0xC0;   // Unlock config registers

// Media status bits
const RTL8139_MEDIASTAT_LINK: u8 = 0x04;   // Link status
const RTL8139_MEDIASTAT_SPEED10: u8 = 0x08; // Speed 10Mbps

// RX packet header
#[repr(C, packed)]
struct RxPacketHeader {
    status: u16,
    length: u16,
}

// RX status bits
const RTL8139_RX_STATUS_OK: u16 = 0x0001;       // Packet received intact
const RTL8139_RX_STATUS_FRAME_ALIGN_ERR: u16 = 0x0002; // Frame alignment error
const RTL8139_RX_STATUS_CRC_ERR: u16 = 0x0004; // CRC error
const RTL8139_RX_STATUS_LONG_ERR: u16 = 0x0008; // Packet too long
const RTL8139_RX_STATUS_RUNT_ERR: u16 = 0x0010; // Runt packet received
const RTL8139_RX_STATUS_ISE: u16 = 0x0020;      // Invalid symbol error
const RTL8139_RX_STATUS_BROADCAST: u16 = 0x2000; // Broadcast packet
const RTL8139_RX_STATUS_PHYSICAL: u16 = 0x4000;  // Physical match
const RTL8139_RX_STATUS_MULTICAST: u16 = 0x8000; // Multicast packet

impl OrionDriver for Rtl8139Driver {
    fn probe(device: &DeviceInfo) -> DriverResult<bool> {
        // Realtek vendor ID: 0x10EC
        if device.vendor_id != 0x10EC {
            return Ok(false);
        }
        
        // RTL8139 device IDs
        match device.device_id {
            0x8139 | 0x8138 | 0x8137 | 0x8136 => Ok(true),
            _ => Ok(false),
        }
    }
    
    fn init(device: DeviceInfo) -> DriverResult<Self> {
        // Create MMIO accessor for device registers
        let mmio = unsafe {
            MmioAccessor::new(
                device.bars[0], // First BAR contains I/O registers
                256,            // RTL8139 register space size
                MmioPermissions::READ | MmioPermissions::WRITE | MmioPermissions::UNCACHED
            )
        };
        
        // Power on the device
        mmio.write_u8(RTL8139_CONFIG1, 0x00)?;
        
        // Software reset
        mmio.write_u8(RTL8139_CHIPCMD, RTL8139_CMD_RESET)?;
        
        // Wait for reset to complete
        while mmio.read_u8(RTL8139_CHIPCMD)? & RTL8139_CMD_RESET != 0 {
            // TODO: Use proper delay function
        }
        
        // Read MAC address
        let mut mac_address = [0u8; 6];
        for i in 0..6 {
            mac_address[i] = mmio.read_u8(RTL8139_MAC0 + i)?;
        }
        
        // If MAC is all zeros, generate a default one
        if mac_address == [0, 0, 0, 0, 0, 0] {
            mac_address = [0x52, 0x54, 0x00, 0x12, 0x34, 0x56]; // QEMU default
        }
        
        // Unlock config registers
        mmio.write_u8(RTL8139_CFG9346, RTL8139_CFG9346_UNLOCK)?;
        
        // Set up RX buffer (8KB + 16 bytes wrap protection)
        let rx_buffer_size = 8192 + 16;
        // TODO: Allocate actual RX buffer and set RXBUF register
        mmio.write_u32(RTL8139_RXBUF, 0)?; // Placeholder
        
        // No mask on interrupt
        mmio.write_u16(RTL8139_INTRMASK, 0xFFFF)?;
        
        // Configure RX: accept broadcast, multicast, and physical match packets
        // Use 8KB buffer, unlimited DMA burst, 1024 byte FIFO threshold
        let rx_config = RTL8139_RX_CONFIG_AB |     // Accept broadcast
                       RTL8139_RX_CONFIG_AM |     // Accept multicast  
                       RTL8139_RX_CONFIG_APM |    // Accept physical match
                       (0x00 << RTL8139_RX_CONFIG_RBLEN_SHIFT) | // 8KB buffer
                       (0x07 << RTL8139_RX_CONFIG_MXDMA_SHIFT) | // Unlimited DMA
                       (0x06 << RTL8139_RX_CONFIG_FTH_SHIFT);    // 1024 byte threshold
        mmio.write_u32(RTL8139_RXCONFIG, rx_config)?;
        
        // Configure TX: default values, 1024 byte FIFO threshold
        let tx_config = (0x06 << RTL8139_TX_FIFO_THRESH_SHIFT) | // 1024 byte threshold
                       (0x07 << 8) |               // Max DMA burst
                       0x03000000;                 // Default values
        mmio.write_u32(RTL8139_TXCONFIG, tx_config)?;
        
        // Lock config registers
        mmio.write_u8(RTL8139_CFG9346, RTL8139_CFG9346_LOCK)?;
        
        // Enable RX and TX
        mmio.write_u8(RTL8139_CHIPCMD, RTL8139_CMD_RX_ENABLE | RTL8139_CMD_TX_ENABLE)?;
        
        // Clear all interrupt status
        mmio.write_u16(RTL8139_INTRSTATUS, 0xFFFF)?;
        
        // Check link status
        let media_status = mmio.read_u8(RTL8139_MEDIASTAT)?;
        let link_up = (media_status & RTL8139_MEDIASTAT_LINK) != 0;
        
        Ok(Rtl8139Driver {
            device,
            mmio,
            mac_address,
            rx_buffer_size,
            tx_buffer_count: 4,
            current_tx_buffer: 0,
            stats: NetworkStats::default(),
            link_up,
        })
    }
    
    fn handle_irq(&mut self) -> DriverResult<()> {
        // Read and clear interrupt status
        let isr = self.mmio.read_u16(RTL8139_INTRSTATUS)?;
        self.mmio.write_u16(RTL8139_INTRSTATUS, isr)?;
        
        if isr & RTL8139_INT_RX_OK != 0 {
            // Packet received
            self.handle_rx_interrupt()?;
        }
        
        if isr & RTL8139_INT_TX_OK != 0 {
            // Packet transmitted
            self.handle_tx_interrupt()?;
        }
        
        if isr & RTL8139_INT_RX_ERR != 0 {
            // RX error
            self.stats.rx_errors += 1;
        }
        
        if isr & RTL8139_INT_TX_ERR != 0 {
            // TX error
            self.stats.tx_errors += 1;
        }
        
        if isr & RTL8139_INT_RXBUF_OVERFLOW != 0 {
            // RX buffer overflow
            self.stats.rx_dropped += 1;
        }
        
        if isr & (RTL8139_INT_RXFIFO_UNDERRUN | RTL8139_INT_PCIERR) != 0 {
            // Link change or serious error
            self.handle_link_change()?;
        }
        
        Ok(())
    }
    
    fn shutdown(&mut self) -> DriverResult<()> {
        // Disable interrupts
        self.mmio.write_u16(RTL8139_INTRMASK, 0)?;
        
        // Disable RX and TX
        self.mmio.write_u8(RTL8139_CHIPCMD, 0)?;
        
        // Software reset
        self.mmio.write_u8(RTL8139_CHIPCMD, RTL8139_CMD_RESET)?;
        
        self.link_up = false;
        Ok(())
    }
    
    fn info(&self) -> DriverInfo {
        DriverInfo {
            name: "Realtek RTL8139 Network Driver",
            version: "1.0.0",
            author: "Jeremy Noverraz",
            description: "Fast Ethernet driver for Realtek RTL8139 controllers",
        }
    }
}

impl NetworkDriver for Rtl8139Driver {
    fn send_packet(&mut self, packet: &[u8]) -> DriverResult<usize> {
        if packet.len() > 1514 {
            return Err(DriverError::MemoryError); // MTU exceeded
        }
        
        if !self.link_up {
            return Err(DriverError::DeviceNotFound);
        }
        
        // Check if current TX buffer is available
        let tx_status_reg = RTL8139_TXSTATUS0 + (self.current_tx_buffer * 4);
        let tx_status = self.mmio.read_u32(tx_status_reg)?;
        
        if tx_status & RTL8139_TX_OWN == 0 {
            return Err(DriverError::ResourceBusy); // TX buffer not available
        }
        
        // TODO: Copy packet to TX buffer and set TX address register
        // For now, simulate successful transmission
        
        // Set packet size and start transmission
        let tx_size = packet.len() as u32;
        self.mmio.write_u32(tx_status_reg, tx_size & RTL8139_TX_SIZE_MASK)?;
        
        // Move to next TX buffer
        self.current_tx_buffer = (self.current_tx_buffer + 1) % self.tx_buffer_count;
        
        self.stats.tx_packets += 1;
        self.stats.tx_bytes += packet.len() as u64;
        
        Ok(packet.len())
    }
    
    fn receive_packet(&mut self, buffer: &mut [u8]) -> DriverResult<usize> {
        if !self.link_up {
            return Err(DriverError::DeviceNotReady);
        }
        
        // Check if RX buffer is empty
        let cmd = self.mmio.read_u8(RTL8139_CHIPCMD)?;
        if cmd & RTL8139_CMD_RX_BUF_EMPTY != 0 {
            return Err(DriverError::NoData); // No packets available
        }
        
        // Read packet from RX buffer
        let rx_base = self.mmio.read_u32(RTL8139_RXBUF)?;
        let rx_head = self.mmio.read_u16(RTL8139_RXBUFHEAD)?;
        let rx_tail = self.mmio.read_u16(RTL8139_RXBUFTAIL)?;
        
        if rx_head == rx_tail {
            return Err(DriverError::NoData);
        }
        
        // Read packet data
        let packet_addr = rx_base + rx_tail as u32;
        let packet_length = 64; // Minimum packet size for now
        
        if packet_length > buffer.len() {
            return Err(DriverError::BufferTooSmall);
        }
        
        // Copy packet data to buffer
        unsafe {
            let packet_data = core::slice::from_raw_parts(packet_addr as *const u8, packet_length);
            buffer[..packet_length].copy_from_slice(packet_data);
        }
        
        // Update RX buffer tail
        let new_tail = (rx_tail + packet_length as u16) % 0x1000;
        self.mmio.write_u16(RTL8139_RXBUFTAIL, new_tail)?;
        
        // Update statistics
        self.stats.rx_packets.fetch_add(1, Ordering::Relaxed);
        self.stats.rx_bytes.fetch_add(packet_length as u64, Ordering::Relaxed);
        
        Ok(packet_length)
    }
    
    fn mac_address(&self) -> [u8; 6] {
        self.mac_address
    }
    
    fn set_promiscuous(&mut self, enabled: bool) -> DriverResult<()> {
        // Unlock config registers
        self.mmio.write_u8(RTL8139_CFG9346, RTL8139_CFG9346_UNLOCK)?;
        
        let mut rx_config = self.mmio.read_u32(RTL8139_RXCONFIG)?;
        
        if enabled {
            rx_config |= RTL8139_RX_CONFIG_AAP; // Accept all packets
        } else {
            rx_config &= !RTL8139_RX_CONFIG_AAP;
        }
        
        self.mmio.write_u32(RTL8139_RXCONFIG, rx_config)?;
        
        // Lock config registers
        self.mmio.write_u8(RTL8139_CFG9346, RTL8139_CFG9346_LOCK)?;
        
        Ok(())
    }
    
    fn link_status(&self) -> LinkStatus {
        if self.link_up {
            // RTL8139 is 100Mbps Fast Ethernet, always full duplex when link is up
            LinkStatus::Up { speed_mbps: 100, duplex: true }
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
        
        // Update device registers
        for i in 0..6 {
            let offset = RTL8139_MAC0 + i;
            self.mmio.write_u8(offset, mac[i])?;
        }
        
        Ok(())
    }
}

impl Rtl8139Driver {
    fn handle_rx_interrupt(&mut self) -> DriverResult<()> {
        // TODO: Process received packets from RX buffer
        self.stats.rx_packets += 1;
        self.stats.rx_bytes += 64; // Simulate packet size
        Ok(())
    }
    
    fn handle_tx_interrupt(&mut self) -> DriverResult<()> {
        // TODO: Process transmitted packet completions
        Ok(())
    }
    
    fn handle_link_change(&mut self) -> DriverResult<()> {
        let media_status = self.mmio.read_u8(RTL8139_MEDIASTAT)?;
        self.link_up = (media_status & RTL8139_MEDIASTAT_LINK) != 0;
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
        "rtl8139",
        "1.0.0",
        &[0x10EC], // Realtek vendor ID
        &[0x8139, 0x8138, 0x8137, 0x8136], // RTL8139 device IDs
        |ipc, message| {
            match message {
                ReceivedMessage::ProbeDevice(probe_msg) => {
                    let can_handle = Rtl8139Driver::probe(&DeviceInfo::new(
                        probe_msg.vendor_id, probe_msg.device_id, 
                        orion_driver::BusType::Pci
                    )).unwrap_or(false);
                    ipc.send_probe_response(probe_msg.header.sequence, can_handle)
                }
                
                ReceivedMessage::InitDevice(device_handle) => {
                    let _ = device_handle;
                    Ok(())
                }
                
                ReceivedMessage::IoRequest(io_msg) => {
                    let result = match io_msg.request_type {
                        IoRequestType::Read => Ok(0),
                        IoRequestType::Write => Ok(io_msg.length as usize),
                        IoRequestType::Ioctl => Ok(0),
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
