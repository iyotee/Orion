#!/bin/bash

#!/bin/bash
##############################################################################
# Orion OS Build System
# 
# A modular build system for the Orion operating system kernel.
# Designed for academic research and professional development environments.
#
# Author: Orion OS Project
# License: MIT
# Version: 1.0.0
##############################################################################

set -e

# Build system configuration
readonly ORION_BUILD_VERSION="1.0.0"
readonly ORION_VERSION="1.0.0-alpha"
readonly PROJECT_ROOT="$(pwd)"

# Output formatting (minimal for performance)
print_header() {
    echo "Orion OS Build System v${ORION_BUILD_VERSION}"
    echo "Target: Orion Kernel v${ORION_VERSION}"
    echo "========================================"
}

print_status() {
    echo "[ INFO ] $1"
}

print_success() {
    echo "[ OK   ] $1"
}

print_error() {
    echo "[ ERR  ] $1" >&2
}

print_warning() {
    echo "[ WARN ] $1"
}

# Display usage information
show_help() {
    print_header
    echo ""
    echo "USAGE: $0 [COMMAND]"
    echo ""
    echo "COMMANDS:"
    echo "  clean          Clean all build artifacts"
    echo "  bootloader     Build the Orion UEFI bootloader"
    echo "  kernel         Build the Orion kernel"
    echo "  iso            Generate bootable UEFI ISO image"
    echo "  all            Complete build (bootloader + kernel + ISO)"
    echo "  test           Test ISO in QEMU emulator"
    echo "  install        Install build dependencies"
    echo "  status         Display build status"
    echo "  help           Show this help message"
    echo ""
    echo "EXAMPLES:"
    echo "  $0 all         # Complete build"
    echo "  $0 kernel      # Build kernel only"
    echo "  $0 test        # Test in QEMU"
    echo ""
}

# Clean build artifacts
clean_project() {
    print_status "Cleaning build artifacts..."
    
    # Remove build directories
    rm -rf build build-wsl
    rm -rf bootloader/build
    
    # Remove generated images
    rm -f *.iso *.img
    
    print_success "Build artifacts cleaned"
}

# Build UEFI bootloader
build_bootloader() {
    print_status "Building Orion UEFI bootloader..."
    
    cd bootloader || exit 1
    make clean >/dev/null 2>&1
    make all
    cd "${PROJECT_ROOT}" || exit 1
    
    if [ -f "bootloader/build/bootx64.efi" ]; then
        local size=$(stat -c%s "bootloader/build/bootx64.efi")
        print_success "Bootloader built successfully (${size} bytes)"
    else
        print_error "Bootloader build failed"
        exit 1
    fi
}

# Build kernel
build_kernel() {
    print_status "Building Orion kernel..."
    
    mkdir -p build
    cd build || exit 1
    cmake -DCMAKE_BUILD_TYPE=Debug -DORION_ARCH=x86_64 ..
    make kernel
    cd "${PROJECT_ROOT}" || exit 1
    
    if [ -f "build/kernel/orion-kernel.elf" ]; then
        local size=$(stat -c%s "build/kernel/orion-kernel.elf")
        print_success "Kernel built successfully (${size} bytes)"
    else
        print_error "Kernel build failed"
        exit 1
    fi
}

# Generate bootable ISO
build_iso() {
    print_status "Generating UEFI bootable ISO..."
    
    # Check prerequisites
    if [ ! -f "bootloader/build/bootx64.efi" ]; then
        print_warning "Bootloader not found, building..."
        build_bootloader
    fi
    
    if [ ! -f "build/kernel/orion-kernel.elf" ]; then
        print_warning "Kernel not found, building..."
        build_kernel
    fi
    
    # Generate ISO directly (unified implementation)
    local iso_output="build/orion-auto-boot.iso"
    local efi_img="build/efi-boot.img"
    local temp_dir="build/iso-temp"
    
    print_status "Creating UEFI boot image..."
    
    # Clean and prepare directories
    rm -rf "$temp_dir" "$efi_img"
    mkdir -p "$temp_dir/EFI/BOOT" "$temp_dir/boot"
    
    # Create FAT32 EFI system partition
    dd if=/dev/zero of="$efi_img" bs=1M count=50 2>/dev/null
    mkfs.fat -F 32 "$efi_img" >/dev/null 2>&1
    
    # Mount the FAT image and copy files (professional approach)
    sudo mkdir -p /mnt/orion-efi
    sudo mount "$efi_img" /mnt/orion-efi
    
    # Copy bootloader and kernel with proper structure
    sudo mkdir -p /mnt/orion-efi/EFI/BOOT
    sudo mkdir -p /mnt/orion-efi/boot
    sudo cp "bootloader/build/bootx64.efi" "/mnt/orion-efi/EFI/BOOT/bootx64.efi"
    sudo cp "build/kernel/orion-kernel.elf" "/mnt/orion-efi/boot/orion-kernel.elf"
    
    # Create professional boot configuration
    sudo sh -c 'echo "EFI\\BOOT\\bootx64.efi" > /mnt/orion-efi/startup.nsh'
    
    # Unmount cleanly
    sudo umount /mnt/orion-efi
    sudo rmdir /mnt/orion-efi
    
    # Create auto-boot script
    echo "EFI\\BOOT\\bootx64.efi" > "$temp_dir/startup.nsh"
    
    # Generate final ISO with professional settings
    print_status "Creating bootable ISO..."
    genisoimage -o "$iso_output" \
        -r -J -joliet-long \
        -b "$efi_img" \
        -no-emul-boot \
        -boot-load-size 4 \
        -boot-info-table \
        "$temp_dir" >/dev/null 2>&1
    
    # Cleanup
    rm -rf "$temp_dir" "$efi_img"
    
    if [ -f "$iso_output" ]; then
        local size=$(stat -c%s "$iso_output")
        print_success "Bootable ISO generated successfully (${size} bytes)"
        print_status "ISO location: $iso_output"
    else
        print_error "ISO generation failed"
        exit 1
    fi
}

# Test in QEMU emulator
test_iso() {
    print_status "Testing Orion ISO in QEMU..."
    
    if [ ! -f "build/orion-auto-boot.iso" ]; then
        print_warning "ISO not found, generating..."
        build_iso
    fi
    
    print_status "Starting QEMU emulator..."
    echo "Press Ctrl+C to stop QEMU"
    
    qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom build/orion-auto-boot.iso -m 512M -nographic
}

# Install build dependencies
install_deps() {
    print_status "Installing Orion build dependencies..."
    
    # Core development tools
    sudo apt-get update
    sudo apt-get install -y build-essential cmake gcc clang
    sudo apt-get install -y qemu-system-x86 ovmf
    sudo apt-get install -y xorriso dosfstools mtools
    sudo apt-get install -y gnu-efi
    
    print_success "Dependencies installed successfully"
}

# Display project status
show_status() {
    print_header
    echo ""
    echo "PROJECT STATUS:"
    echo ""
    
    # Check bootloader
    if [ -f "bootloader/build/bootx64.efi" ]; then
        local size=$(stat -c%s "bootloader/build/bootx64.efi")
        echo "[ OK   ] Bootloader: ${size} bytes"
    else
        echo "[ MISS ] Bootloader: Not built"
    fi
    
    # Check kernel
    if [ -f "build/kernel/orion-kernel.elf" ]; then
        local size=$(stat -c%s "build/kernel/orion-kernel.elf")
        echo "[ OK   ] Kernel: ${size} bytes"
    else
        echo "[ MISS ] Kernel: Not built"
    fi
    
    # Check ISO
    if [ -f "build/orion-auto-boot.iso" ]; then
        local size=$(stat -c%s "build/orion-auto-boot.iso")
        echo "[ OK   ] ISO image: ${size} bytes"
    else
        echo "[ MISS ] ISO image: Not generated"
    fi
    
    echo ""
    echo "FEATURES:"
    echo "[ OK   ] Orion Boot Protocol v1.0"
    echo "[ OK   ] x86_64 UEFI architecture"
    echo "[ OK   ] Academic-grade implementation"
    echo ""
}

# Complete build
build_all() {
    print_header
    print_status "Starting complete Orion build..."
    
    build_bootloader
    build_kernel
    build_iso
    
    print_success "Complete build finished successfully"
    echo ""
    show_status
}

# Command line argument parser
case "${1:-help}" in
    "clean")
        clean_project
        ;;
    "bootloader")
        build_bootloader
        ;;
    "kernel")
        build_kernel
        ;;
    "iso")
        build_iso
        ;;
    "all")
        build_all
        ;;
    "test")
        test_iso
        ;;
    "install")
        install_deps
        ;;
    "status")
        show_status
        ;;
    "help"|*)
        show_help
        ;;
esac
