cmake_minimum_required(VERSION 3.20)
project(Orion LANGUAGES C ASM)

# General Configuration
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Target architecture support
set(ORION_ARCH "x86_64" CACHE STRING "Target architecture (x86_64 or aarch64)")
set_property(CACHE ORION_ARCH PROPERTY STRINGS "x86_64" "aarch64")

# Linux/GCC configuration
set(CMAKE_ASM_COMPILER as)
set(CMAKE_LINKER ld)

# GCC compilation flags for freestanding kernel
set(KERNEL_CFLAGS
    -std=c11
    -ffreestanding
    -fno-builtin
    -fno-stack-protector
    -fno-pie
    -mno-red-zone
    -mcmodel=kernel
    -Wall
    -Wextra
    -Werror
    -O2
    -g
    -DDEBUG
    -DORION_KERNEL_BUILD=1
    -DORION_ARCH_${ORION_ARCH}=1
)

# Assembly flags
set(KERNEL_ASFLAGS
    --64
    -g
)

# Linker flags
set(KERNEL_LDFLAGS
    -melf_x86_64
    -T${CMAKE_CURRENT_SOURCE_DIR}/kernel/arch/${ORION_ARCH}/linker.ld
    -nostdlib
    -znoexecstack
    -zrelro
    -znow
)

# Set compiler flags
set(CMAKE_C_FLAGS "${KERNEL_CFLAGS}")
set(CMAKE_ASM_FLAGS "${KERNEL_ASFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${KERNEL_LDFLAGS}")

# Include directories
include_directories(kernel/include)

# Subprojects
add_subdirectory(kernel)
add_subdirectory(tools)

# Main targets
add_custom_target(all-orion
    DEPENDS kernel
    COMMENT "Complete Orion build"
)

# Target for QEMU x86_64
add_custom_target(qemu-x86_64
    COMMAND qemu-system-x86_64 
        -machine q35
        -cpu qemu64
        -smp 2
        -m 512M
        -serial stdio
        -no-reboot
        -no-shutdown
        -kernel $<TARGET_FILE:kernel>
    DEPENDS kernel
    COMMENT "Running Orion in QEMU x86_64"
)

# Target for generating UEFI ISO
add_custom_target(iso
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/build-iso.sh $<TARGET_FILE:kernel> ${CMAKE_BINARY_DIR}/orion.iso
    DEPENDS kernel
    COMMENT "Generating bootable UEFI ISO"
)

# Tests
enable_testing()
# add_subdirectory(tests OPTIONAL)
