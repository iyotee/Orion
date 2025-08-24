/*
 * Orion Operating System - I/O Server
 *
 * Input/Output management and device control server for Orion OS.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#![no_std]
#![no_main]

use orion_ipc::IpcChannel;
use orion_cap::Capability;

fn main() {
    let _channel = IpcChannel::new();
    let _capability = Capability::new();
    // TODO: Implement I/O management server
}

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {
        unsafe {
            core::arch::asm!("hlt");
        }
    }
}
