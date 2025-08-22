/*
 * Orion Operating System - Generic Framebuffer GPU Driver
 *
 * Universal framebuffer driver supporting linear framebuffers from UEFI GOP,
 * VBE, and other sources. Provides basic 2D graphics operations.
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
    GpuDriver, DeviceInfo, DriverError, DriverInfo, DriverResult, OrionDriver,
    FramebufferInfo, MmioAccessor, MmioPermissions,
    MessageLoop, ReceivedMessage, IoRequestType,
};

/// Generic Framebuffer GPU Driver
pub struct FramebufferDriver {
    device: DeviceInfo,
    framebuffer_info: FramebufferInfo,
    framebuffer_accessor: Option<MmioAccessor>,
}

// Standard color format definitions
const COLOR_FORMAT_RGB: u8 = 0;
const COLOR_FORMAT_BGR: u8 = 1;
const COLOR_FORMAT_INDEXED: u8 = 2;

// Basic color constants (RGB888)
const COLOR_BLACK: u32 = 0x000000;
const COLOR_WHITE: u32 = 0xFFFFFF;
const COLOR_RED: u32 = 0xFF0000;
const COLOR_GREEN: u32 = 0x00FF00;
const COLOR_BLUE: u32 = 0x0000FF;
const COLOR_CYAN: u32 = 0x00FFFF;
const COLOR_MAGENTA: u32 = 0xFF00FF;
const COLOR_YELLOW: u32 = 0xFFFF00;
const COLOR_GRAY: u32 = 0x808080;
const COLOR_LIGHT_GRAY: u32 = 0xC0C0C0;
const COLOR_DARK_GRAY: u32 = 0x404040;

impl OrionDriver for FramebufferDriver {
    fn probe(device: &DeviceInfo) -> DriverResult<bool> {
        // Generic framebuffer can work with any device that provides a linear framebuffer
        // This includes UEFI GOP, VBE framebuffers, and simple display devices
        Ok(device.class == 0x03) // Display controller class
    }
    
    fn init(device: DeviceInfo) -> DriverResult<Self> {
        // For now, create a default framebuffer configuration
        // In a real implementation, this would be provided by the bootloader or BIOS
        let framebuffer_info = FramebufferInfo {
            width: 1024,
            height: 768,
            bpp: 32,
            pitch: 1024 * 4,
            memory_size: 1024 * 768 * 4,
            physical_address: device.bars[0], // Assume first BAR is framebuffer
        };
        
        // Create accessor for framebuffer memory
        let framebuffer_accessor = if framebuffer_info.physical_address != 0 {
            Some(unsafe {
                MmioAccessor::new(
                    framebuffer_info.physical_address,
                    framebuffer_info.memory_size,
                    MmioPermissions::READ | MmioPermissions::WRITE | MmioPermissions::WRITE_COMBINING
                )
            })
        } else {
            None
        };
        
        Ok(FramebufferDriver {
            device,
            framebuffer_info,
            framebuffer_accessor,
        })
    }
    
    fn handle_irq(&mut self) -> DriverResult<()> {
        // Framebuffer devices typically don't generate interrupts
        Ok(())
    }
    
    fn shutdown(&mut self) -> DriverResult<()> {
        // Clear the screen to black on shutdown
        if let Err(_) = self.clear_screen(COLOR_BLACK) {
            // Ignore errors during shutdown
        }
        Ok(())
    }
    
    fn info(&self) -> DriverInfo {
        DriverInfo {
            name: "Generic Framebuffer Driver",
            version: "1.0.0",
            author: "Jeremy Noverraz",
            description: "Universal framebuffer driver for linear display buffers",
        }
    }
}

impl GpuDriver for FramebufferDriver {
    fn init_graphics(&mut self, width: u32, height: u32, bpp: u8) -> DriverResult<()> {
        // For a generic framebuffer, we can't change the mode dynamically
        // But we can validate the requested mode against our current configuration
        if width == self.framebuffer_info.width &&
           height == self.framebuffer_info.height &&
           bpp == self.framebuffer_info.bpp {
            Ok(())
        } else {
            Err(DriverError::Unsupported)
        }
    }
    
    fn framebuffer_info(&self) -> FramebufferInfo {
        self.framebuffer_info
    }
    
    fn set_pixel(&mut self, x: u32, y: u32, color: u32) -> DriverResult<()> {
        if x >= self.framebuffer_info.width || y >= self.framebuffer_info.height {
            return Err(DriverError::MemoryError);
        }
        
        let accessor = self.framebuffer_accessor.as_ref()
            .ok_or(DriverError::DeviceNotFound)?;
        
        let bytes_per_pixel = (self.framebuffer_info.bpp / 8) as u32;
        let offset = (y * self.framebuffer_info.pitch + x * bytes_per_pixel) as usize;
        
        match self.framebuffer_info.bpp {
            32 => {
                // ARGB8888 or RGB888 with padding
                accessor.write_u32(offset, color)?;
            }
            24 => {
                // RGB888
                accessor.write_u8(offset, (color & 0xFF) as u8)?;           // B
                accessor.write_u8(offset + 1, ((color >> 8) & 0xFF) as u8)?; // G
                accessor.write_u8(offset + 2, ((color >> 16) & 0xFF) as u8)?; // R
            }
            16 => {
                // RGB565
                let r = ((color >> 16) & 0xFF) >> 3;
                let g = ((color >> 8) & 0xFF) >> 2;
                let b = (color & 0xFF) >> 3;
                let pixel = ((r << 11) | (g << 5) | b) as u16;
                accessor.write_u16(offset, pixel)?;
            }
            8 => {
                // Indexed color - use simple grayscale mapping
                let gray = ((color >> 16) & 0xFF) * 30 + ((color >> 8) & 0xFF) * 59 + (color & 0xFF) * 11;
                accessor.write_u8(offset, (gray / 100) as u8)?;
            }
            _ => {
                return Err(DriverError::Unsupported);
            }
        }
        
        Ok(())
    }
    
    fn copy_buffer(&mut self, buffer: &[u8]) -> DriverResult<()> {
        let accessor = self.framebuffer_accessor.as_ref()
            .ok_or(DriverError::DeviceNotFound)?;
        
        let required_size = self.framebuffer_info.memory_size;
        if buffer.len() != required_size {
            return Err(DriverError::MemoryError);
        }
        
        // Copy buffer to framebuffer
        accessor.write_buffer(0, buffer)?;
        
        Ok(())
    }
    
    fn clear_screen(&mut self, color: u32) -> DriverResult<()> {
        let width = self.framebuffer_info.width;
        let height = self.framebuffer_info.height;
        
        // For efficiency, clear line by line
        for y in 0..height {
            for x in 0..width {
                self.set_pixel(x, y, color)?;
            }
        }
        
        Ok(())
    }
}

impl FramebufferDriver {
    /// Draw a rectangle
    pub fn draw_rect(&mut self, x: u32, y: u32, width: u32, height: u32, color: u32) -> DriverResult<()> {
        for dy in 0..height {
            for dx in 0..width {
                if x + dx < self.framebuffer_info.width && y + dy < self.framebuffer_info.height {
                    self.set_pixel(x + dx, y + dy, color)?;
                }
            }
        }
        Ok(())
    }
    
    /// Draw a filled circle
    pub fn draw_circle(&mut self, center_x: u32, center_y: u32, radius: u32, color: u32) -> DriverResult<()> {
        let r_squared = (radius * radius) as i32;
        
        for y in 0..self.framebuffer_info.height {
            for x in 0..self.framebuffer_info.width {
                let dx = x as i32 - center_x as i32;
                let dy = y as i32 - center_y as i32;
                if dx * dx + dy * dy <= r_squared {
                    self.set_pixel(x, y, color)?;
                }
            }
        }
        Ok(())
    }
    
    /// Draw a line using Bresenham's algorithm
    pub fn draw_line(&mut self, x0: u32, y0: u32, x1: u32, y1: u32, color: u32) -> DriverResult<()> {
        let mut x0 = x0 as i32;
        let mut y0 = y0 as i32;
        let x1 = x1 as i32;
        let y1 = y1 as i32;
        
        let dx = (x1 - x0).abs();
        let dy = (y1 - y0).abs();
        let sx = if x0 < x1 { 1 } else { -1 };
        let sy = if y0 < y1 { 1 } else { -1 };
        let mut err = dx - dy;
        
        loop {
            if x0 >= 0 && x0 < self.framebuffer_info.width as i32 &&
               y0 >= 0 && y0 < self.framebuffer_info.height as i32 {
                self.set_pixel(x0 as u32, y0 as u32, color)?;
            }
            
            if x0 == x1 && y0 == y1 {
                break;
            }
            
            let e2 = 2 * err;
            if e2 > -dy {
                err -= dy;
                x0 += sx;
            }
            if e2 < dx {
                err += dx;
                y0 += sy;
            }
        }
        Ok(())
    }
    
    /// Draw a simple 8x8 character (basic bitmap font)
    pub fn draw_char(&mut self, x: u32, y: u32, ch: char, color: u32) -> DriverResult<()> {
        // Simple 8x8 bitmap font for basic characters
        let bitmap = get_char_bitmap(ch);
        
        for row in 0..8 {
            for col in 0..8 {
                if bitmap[row] & (1 << (7 - col)) != 0 {
                    if x + col < self.framebuffer_info.width && y + row < self.framebuffer_info.height {
                        self.set_pixel(x + col as u32, y + row as u32, color)?;
                    }
                }
            }
        }
        Ok(())
    }
    
    /// Draw a string using the basic font
    pub fn draw_string(&mut self, x: u32, y: u32, text: &str, color: u32) -> DriverResult<()> {
        for (i, ch) in text.chars().enumerate() {
            let char_x = x + (i as u32 * 8);
            if char_x >= self.framebuffer_info.width {
                break;
            }
            self.draw_char(char_x, y, ch, color)?;
        }
        Ok(())
    }
}

/// Get a simple 8x8 bitmap for a character
fn get_char_bitmap(ch: char) -> [u8; 8] {
    match ch {
        'A' => [0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00],
        'B' => [0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00],
        'C' => [0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00],
        'O' => [0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00],
        'R' => [0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x66, 0x00],
        'I' => [0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00],
        'N' => [0x66, 0x76, 0x7E, 0x6E, 0x66, 0x66, 0x66, 0x00],
        'S' => [0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00],
        ' ' => [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00],
        '0' => [0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00],
        '1' => [0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00],
        '.' => [0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00],
        _ => [0x7E, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7E, 0x00], // Default box
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
        "framebuffer",
        "1.0.0",
        &[0x1234], // Generic vendor ID
        &[0x1111], // Generic device ID
        |ipc, message| {
            match message {
                ReceivedMessage::ProbeDevice(probe_msg) => {
                    let can_handle = FramebufferDriver::probe(&DeviceInfo::new(
                        probe_msg.vendor_id, probe_msg.device_id, 
                        orion_driver::BusType::Platform
                    )).unwrap_or(false);
                    ipc.send_probe_response(probe_msg.header.sequence, can_handle)
                }
                
                ReceivedMessage::InitDevice(device_handle) => {
                    let _ = device_handle;
                    Ok(())
                }
                
                ReceivedMessage::IoRequest(io_msg) => {
                    let result = match io_msg.request_type {
                        IoRequestType::Read => {
                            // Read framebuffer info or pixel data
                            Ok(4) // Return size of framebuffer info
                        }
                        IoRequestType::Write => {
                            // Write pixel data or commands
                            Ok(io_msg.length as usize)
                        }
                        IoRequestType::Ioctl => {
                            // Graphics control operations
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
