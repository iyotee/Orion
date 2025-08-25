# ORION OS - ARMv7l Architecture CMake Configuration
#
# Build configuration for ARMv7l architecture (32-bit ARM)
# Supports all Raspberry Pi models: Pi 1, Pi 2, Pi Zero, Pi Zero W
#
# Developed by Jeremy Noverraz (1988-2025)
# August 2025, Lausanne, Switzerland
#
# Copyright (c) 2024-2025 Orion OS Project
# License: MIT

# ============================================================================
# ARCHITECTURE CONFIGURATION
# ============================================================================

set(ORION_ARCH "armv7l")
set(ORION_ARCH_NAME "ARMv7l")
set(ORION_ARCH_BITS 32)
set(ORION_ARCH_ENDIANNESS "little")

# ============================================================================
# COMPILER FLAGS
# ============================================================================

# ARM-specific compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv7-a -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-stack-protector -fno-builtin -fno-pie -fno-pic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -std=c99")

# Assembly flags
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -march=armv7-a -mtune=cortex-a7 -mfpu=neon-vfpv4")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -fno-stack-protector -fno-builtin")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -nostartfiles -nodefaultlibs")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -Wl,--strip-all")

# ============================================================================
# SOURCE FILES
# ============================================================================

# C source files
set(ARM_SOURCES
    arch.c
    entry.c
    interrupts.c
    interrupt_handlers.c
    timers.c
    test_armv7l.c
)

# Assembly source files
set(ARM_ASM_SOURCES
    entry_asm.S
    syscall_entry.S
    boot.S
    arch_asm.S
)

# Header files
set(ARM_HEADERS
    config.h
    arch.h
)

# Linker script
set(ARM_LINKER_SCRIPT
    linker.ld
)

# ============================================================================
# TARGET DEFINITIONS
# ============================================================================

# Create ARMv7l architecture library
add_library(orion_armv7l STATIC ${ARM_SOURCES} ${ARM_ASM_SOURCES})

# Set target properties
set_target_properties(orion_armv7l PROPERTIES
    COMPILE_FLAGS "${CMAKE_C_FLAGS}"
    OUTPUT_NAME "orion_armv7l"
    POSITION_INDEPENDENT_CODE OFF
)

# Set include directories
target_include_directories(orion_armv7l PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/kernel/arch/include
)

# ============================================================================
# COMPILER-SPECIFIC SETTINGS
# ============================================================================

# GCC-specific settings
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(orion_armv7l PRIVATE
        -fno-stack-protector
        -fno-builtin
        -fno-pie
        -fno-pic
        -nostdinc
        -nostdlib
        -fno-common
        -fno-ident
        -fno-asynchronous-unwind-tables
        -fno-unwind-tables
        -fno-stack-check
        -fno-strict-overflow
        -fno-delete-null-pointer-checks
        -fno-var-tracking
        -fno-var-tracking-assignments
        -fno-ipa-sra
        -fno-ipa-cp
        -fno-ipa-cp-clone
        -fno-ipa-bit-cp
        -fno-ipa-vrp
        -fno-ipa-sra
        -fno-ipa-cp
        -fno-ipa-cp-clone
        -fno-ipa-bit-cp
        -fno-ipa-vrp
        -fno-ipa-sra
        -fno-ipa-cp
        -fno-ipa-cp-clone
        -fno-ipa-bit-cp
        -fno-ipa-vrp
    )
endif()

# Clang-specific settings
if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(orion_armv7l PRIVATE
        -fno-stack-protector
        -fno-builtin
        -fno-pie
        -fno-pic
        -nostdinc
        -nostdlib
        -fno-common
        -fno-ident
        -fno-asynchronous-unwind-tables
        -fno-unwind-tables
        -fno-stack-check
        -fno-strict-overflow
        -fno-delete-null-pointer-checks
        -fno-var-tracking
        -fno-var-tracking-assignments
    )
endif()

# ============================================================================
# ASSEMBLY COMPILATION
# ============================================================================

# Set assembly file properties
set_source_files_properties(${ARM_ASM_SOURCES} PROPERTIES
    LANGUAGE ASM
    COMPILE_FLAGS "${CMAKE_ASM_FLAGS}"
)

# ============================================================================
# DEPENDENCIES
# ============================================================================

# Link with system libraries if needed
target_link_libraries(orion_armv7l
    # Add any required system libraries here
)

# ============================================================================
# INSTALLATION
# ============================================================================

# Install the library
install(TARGETS orion_armv7l
    LIBRARY DESTINATION lib/orion/arch/armv7l
    ARCHIVE DESTINATION lib/orion/arch/armv7l
    RUNTIME DESTINATION bin/orion/arch/armv7l
)

# Install headers
install(FILES ${ARM_HEADERS}
    DESTINATION include/orion/arch/armv7l
)

# ============================================================================
# TESTING
# ============================================================================

# Enable testing if requested
if(BUILD_TESTING)
    enable_testing()
    
    # Add test targets here if needed
    # add_executable(armv7l_test test_armv7l.c)
    # target_link_libraries(armv7l_test orion_armv7l)
    # add_test(NAME ARMv7l_Test COMMAND armv7l_test)
endif()

# ============================================================================
# CUSTOM TARGETS
# ============================================================================

# Create a custom target for ARMv7l specific operations
add_custom_target(armv7l_clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/armv7l
    COMMENT "Cleaning ARMv7l build artifacts"
)

# Create a custom target for ARMv7l documentation
add_custom_target(armv7l_docs
    COMMAND ${CMAKE_COMMAND} -E echo "ARMv7l Architecture Documentation"
    COMMAND ${CMAKE_COMMAND} -E echo "================================"
    COMMAND ${CMAKE_COMMAND} -E echo "Architecture: ${ORION_ARCH_NAME}"
    COMMAND ${CMAKE_COMMAND} -E echo "Bits: ${ORION_ARCH_BITS}"
    COMMAND ${CMAKE_COMMAND} -E echo "Endianness: ${ORION_ARCH_ENDIANNESS}"
    COMMAND ${CMAKE_COMMAND} -E echo "Supported Models:"
    COMMAND ${CMAKE_COMMAND} -E echo "  - Raspberry Pi 1 (ARMv6)"
    COMMAND ${CMAKE_COMMAND} -E echo "  - Raspberry Pi 2 (ARMv7)"
    COMMAND ${CMAKE_COMMAND} -E echo "  - Raspberry Pi Zero (ARMv6)"
    COMMAND ${CMAKE_COMMAND} -E echo "  - Raspberry Pi Zero W (ARMv6)"
    COMMENT "Generating ARMv7l documentation"
)

# ============================================================================
# OUTPUT CONFIGURATION
# ============================================================================

# Set output directory
set_target_properties(orion_armv7l PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/arch/armv7l
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/arch/armv7l
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/arch/armv7l
)

# ============================================================================
# BUILD INFORMATION
# ============================================================================

# Print build information
message(STATUS "ORION OS ARMv7l Architecture Configuration:")
message(STATUS "  Architecture: ${ORION_ARCH_NAME}")
message(STATUS "  Bits: ${ORION_ARCH_BITS}")
message(STATUS "  Endianness: ${ORION_ARCH_ENDIANNESS}")
message(STATUS "  Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "  C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "  ASM Flags: ${CMAKE_ASM_FLAGS}")
message(STATUS "  Sources: ${ARM_SOURCES}")
message(STATUS "  Assembly: ${ARM_ASM_SOURCES}")
message(STATUS "  Headers: ${ARM_HEADERS}")

# ============================================================================
# END OF FILE
# ============================================================================
