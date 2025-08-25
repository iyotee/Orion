# Orion Operating System - aarch64 Architecture CMake Configuration
#
# Build configuration for aarch64 architecture
#
# Developed by Jeremy Noverraz (1988-2025)
# August 2025, Lausanne, Switzerland
#
# Copyright (c) 2024-2025 Orion OS Project
# License: MIT

# Set compiler and flags for aarch64
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Cross-compiler settings
if(DEFINED ENV{ORION_AARCH64_TOOLCHAIN})
    set(CMAKE_C_COMPILER $ENV{ORION_AARCH64_TOOLCHAIN}/aarch64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER $ENV{ORION_AARCH64_TOOLCHAIN}/aarch64-linux-gnu-as)
    set(CMAKE_LINKER $ENV{ORION_AARCH64_TOOLCHAIN}/aarch64-linux-gnu-ld)
else()
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER aarch64-linux-gnu-as)
    set(CMAKE_LINKER aarch64-linux-gnu-ld)
endif()

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a -mtune=generic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -fPIE")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -g")

# Assembler flags
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -march=armv8-a")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")

# Source files
set(AARCH64_SOURCES
    boot.S
    arch_asm.S
    arch.c
    interrupts.c
    interrupt_handlers.c
    timers.c
    entry.c
    syscall_entry.S
    test_aarch64.c
)

# Header files
set(AARCH64_HEADERS
    config.h
    arch.h
)

# Create aarch64 architecture library
add_library(orion_arch_aarch64 STATIC ${AARCH64_SOURCES})

# Set include directories
target_include_directories(orion_arch_aarch64 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../hal/common
)

# Set compile definitions
target_compile_definitions(orion_arch_aarch64 PRIVATE
    ORION_ARCH_AARCH64=1
    ORION_ARCH="aarch64"
)

# Set properties
set_target_properties(orion_arch_aarch64 PROPERTIES
    OUTPUT_NAME "orion_arch_aarch64"
    POSITION_INDEPENDENT_CODE ON
)

# Installation rules
install(TARGETS orion_arch_aarch64
    ARCHIVE DESTINATION lib/orion
    LIBRARY DESTINATION lib/orion
)

install(FILES ${AARCH64_HEADERS}
    DESTINATION include/orion/arch/aarch64
)

# Custom targets
add_custom_target(aarch64-doc
    COMMAND ${CMAKE_COMMAND} -E echo "Generating aarch64 documentation..."
    COMMENT "aarch64 architecture documentation"
)

add_custom_target(aarch64-test
    COMMAND ${CMAKE_COMMAND} -E echo "Running aarch64 tests..."
    COMMENT "aarch64 architecture tests"
    DEPENDS orion_arch_aarch64
)

# Add to global architecture list
if(NOT DEFINED ORION_ARCHITECTURES)
    set(ORION_ARCHITECTURES "")
endif()

list(APPEND ORION_ARCHITECTURES "aarch64")
set(ORION_ARCHITECTURES ${ORION_ARCHITECTURES} PARENT_SCOPE)
