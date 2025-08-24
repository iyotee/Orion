# Orion Operating System - Main Build Configuration
#
# This is the main entry point for building ORION OS
# The actual build logic is in tools/build/
#
# Developed by Jeremy Noverraz (1988-2025)
# August 2025, Lausanne, Switzerland
#
# Copyright (c) 2024-2025 Orion OS Project
# License: MIT

cmake_minimum_required(VERSION 3.20)
project(OrionOS VERSION 0.1.0 LANGUAGES C CXX)

# Set C and C++ standards
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Architecture detection
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR WIN32)
    set(ORION_ARCH "x86_64")
    add_compile_definitions(ORION_ARCH_X86_64)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(ORION_ARCH "aarch64")
    add_compile_definitions(ORION_ARCH_AARCH64)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "riscv64")
    set(ORION_ARCH "riscv64")
    add_compile_definitions(ORION_ARCH_RISCV64)
else()
    set(ORION_ARCH "x86_64")  # Default to x86_64 for Windows
    add_compile_definitions(ORION_ARCH_X86_64)
endif()

message(STATUS "Building for architecture: ${ORION_ARCH}")

# ============================================================================
# PROJECT STRUCTURE
# ============================================================================
# kernel/     - C kernel (HAL, core, arch, mm, net)
# drivers/    - Rust drivers (network, GPU, USB, block)
# bootloader/ - Bootloader
# runtime/    - Rust runtime
# lib/        - Rust shared libraries
# servers/    - Rust servers (VFS, IO, POSIX, net)
# tools/      - Development tools and build system
# docs/       - Documentation
# tests/      - Tests and benchmarks
# config/     - Configuration files

# Include the main build system
add_subdirectory(tools/build)

# ============================================================================
# BUILD CONFIGURATION SUMMARY
# ============================================================================
message(STATUS "")
message(STATUS "=== ORION OS PROJECT STRUCTURE ===")
message(STATUS "Architecture: ${ORION_ARCH}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Kernel: C-based (HAL, core, arch, mm, net)")
message(STATUS "Drivers: Rust-based (network, GPU, USB, block)")
message(STATUS "Runtime: Rust-based")
message(STATUS "Build system: tools/build/")
message(STATUS "=====================================")
message(STATUS "")
message(STATUS "Available targets:")
message(STATUS "  all              - Build everything")
message(STATUS "  kernel           - Build kernel only")
message(STATUS "  drivers          - Build drivers only")
message(STATUS "  runtime          - Build runtime only")
message(STATUS "  clean-all        - Clean all build artifacts")
message(STATUS "")
