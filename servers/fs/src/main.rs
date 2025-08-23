/*
 * Orion Operating System - File System Server
 *
 * High-performance file system server using the new Rust VFS.
 * Faster and more secure than the previous C implementation.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#![no_std]
#![no_main]

extern crate alloc;

use orion_ipc::IpcChannel;
use orion_cap::Capability;

// Global allocator for the server
use linked_list_allocator::LockedHeap;

#[global_allocator]
static ALLOCATOR: LockedHeap = LockedHeap::empty();

mod vfs;

use vfs::{VirtualFileSystem, FileSystemType, FileType};

struct FileSystemServer {
    vfs: VirtualFileSystem,
    ipc_channel: IpcChannel,
    capabilities: Capability,
}

impl FileSystemServer {
    fn new() -> Self {
        let mut server = Self {
            vfs: VirtualFileSystem::new(),
            ipc_channel: IpcChannel::new(),
            capabilities: Capability::new(),
        };

        // Initialize with a RAM filesystem at root
        server.initialize_root_fs();

        server
    }

    fn initialize_root_fs(&mut self) {
        // Mount a RAM filesystem at root
        if let Err(_e) = self.vfs.mount("/", FileSystemType::RamFS, "ram0", "defaults") {
            // TODO: Log error
        }

        // Create basic directory structure
        if let Err(_e) = self.vfs.create("/tmp", FileType::Directory) {
            // TODO: Log error
        }

        if let Err(_e) = self.vfs.create("/var", FileType::Directory) {
            // TODO: Log error
        }

        if let Err(_e) = self.vfs.create("/home", FileType::Directory) {
            // TODO: Log error
        }
    }

    fn run(&mut self) {
        // TODO: Implement main server loop
        // - Handle IPC messages
        // - Process file system requests
        // - Manage mount points
        // - Handle errors and recovery
    }
}

fn main() {
    let mut server = FileSystemServer::new();
    
    // TODO: Initialize IPC and start serving
    server.run();
}

#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    loop {
        unsafe {
            core::arch::asm!("hlt");
        }
    }
}
