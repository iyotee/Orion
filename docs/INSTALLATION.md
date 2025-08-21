# Orion OS Installation Guide

## Prerequisites

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential clang cmake python3 xorriso qemu-system-x86
```

### Arch Linux
```bash
sudo pacman -S base-devel clang cmake python xorriso qemu
```

### Rust (for drivers)
```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
rustup target add x86_64-unknown-none
```

## Building Orion OS

### Clone Repository
```bash
git clone https://github.com/iyotee/Orion.git
cd Orion
```

### Build System
Use the unified build script located in `tools/`:

```bash
# Build everything
./tools/orion-build.sh all

# Build components separately
./tools/orion-build.sh bootloader    # Build UEFI bootloader
./tools/orion-build.sh kernel        # Build kernel
./tools/orion-build.sh drivers       # Build Rust drivers
./tools/orion-build.sh iso           # Generate bootable ISO
```

### Testing in QEMU

```bash
# Boot the generated ISO
qemu-system-x86_64 -M q35 -m 4G -smp 4 \
  -drive if=pflash,format=raw,readonly=on,file=/usr/share/ovmf/OVMF_CODE.fd \
  -drive if=pflash,format=raw,file=/usr/share/ovmf/OVMF_VARS.fd \
  -cdrom build/orion-os.iso \
  -netdev user,id=net0 -device virtio-net-pci,netdev=net0 \
  -device virtio-gpu-pci
```

## Build Output

After successful compilation:
- **Bootloader**: `build/bootloader/orion-bootloader.efi`
- **Kernel**: `build/kernel/orion-kernel.elf`
- **Drivers**: `build/drivers/`
- **ISO Image**: `build/orion-os.iso`

## Troubleshooting

### Common Issues
1. **Missing OVMF**: Install OVMF UEFI firmware for QEMU
2. **Clang not found**: Ensure Clang/LLVM is properly installed
3. **Rust targets**: Run `rustup target add x86_64-unknown-none`

### Debug Build
For debugging with symbols:
```bash
./tools/orion-build.sh kernel --debug
```

---
*For development setup, see DEVELOPMENT.md*
