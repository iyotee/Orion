# Orion Operating System - RISC-V 64-bit Architecture CMake Configuration
#
# Build configuration for RISC-V 64-bit architecture
#
# Developed by Jeremy Noverraz (1988-2025)
# August 2025, Lausanne, Switzerland
#
# Copyright (c) 2024-2025 Orion OS Project
# License: MIT

# Set compiler and flags for RISC-V 64-bit
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

# Cross-compiler settings
if(DEFINED ENV{ORION_RISCV64_TOOLCHAIN})
    set(CMAKE_C_COMPILER $ENV{ORION_RISCV64_TOOLCHAIN}/riscv64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER $ENV{ORION_RISCV64_TOOLCHAIN}/riscv64-linux-gnu-as)
    set(CMAKE_LINKER $ENV{ORION_RISCV64_TOOLCHAIN}/riscv64-linux-gnu-ld)
else()
    set(CMAKE_C_COMPILER riscv64-linux-gnu-gcc)
    set(CMAKE_ASM_COMPILER riscv64-linux-gnu-as)
    set(CMAKE_LINKER riscv64-linux-gnu-ld)
endif()

# Compiler flags
set(RISCV64_CFLAGS
    -march=rv64gc
    -mabi=lp64d
    -fPIC
    -fPIE
    -Wall
    -Wextra
    -Werror
    -O2
    -g
    -DRISCV64_ARCH=1
    -D__riscv__=1
    -D__riscv_xlen=64
)

# Assembler flags
set(RISCV64_ASFLAGS
    -march=rv64gc
    -mabi=lp64d
)

# Set architecture-specific linker flags
set(RISCV64_LDFLAGS
    -melf64lriscv
    -no-pie
    -nostdlib
    -static
)

# Architecture source files
set(RISCV64_SOURCES
    arch.c
    arch_advanced.c
    boot.S
    interrupts.c
    interrupt_handlers.c
    timers.c
    cpu.c
    mmu.c
    cache.c
    security.c
    performance.c
    extensions.c
    error.c
    stubs.c
    interrupt_stubs.c
    devices.c
    exceptions.c
    power.c
    numa.c
    pmu.c
    entry.c
    syscall_entry.S
    linker.ld
    test_riscv64.c
)

# Architecture header files
set(RISCV64_HEADERS
    arch.h
    config.h
    common.h
)

# Create RISC-V architecture library
add_library(orion_arch_riscv64 STATIC ${RISCV64_SOURCES})

# Set compiler flags for the library
target_compile_options(orion_arch_riscv64 PRIVATE ${RISCV64_CFLAGS})
target_compile_options(orion_arch_riscv64 PRIVATE -DRISCV64_ARCH)

# Set assembler flags for assembly files
set_source_files_properties(boot.S PROPERTIES
    COMPILE_FLAGS "${RISCV64_ASFLAGS}"
)

set_source_files_properties(syscall_entry.S PROPERTIES
    COMPILE_FLAGS "${RISCV64_ASFLAGS}"
)

# Set include directories
target_include_directories(orion_arch_riscv64 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/kernel/arch/hal/common
    ${CMAKE_SOURCE_DIR}/kernel/core
)

# Set compile definitions
target_compile_definitions(orion_arch_riscv64 PRIVATE
    ORION_ARCH_RISCV64=1
    RISCV64_ARCH=1
    __riscv__=1
    __riscv_xlen=64
)

# Link with HAL common library
target_link_libraries(orion_arch_riscv64
    orion_hal_common
)

# Install rules
install(TARGETS orion_arch_riscv64
    LIBRARY DESTINATION lib/orion/arch
    ARCHIVE DESTINATION lib/orion/arch
)

install(FILES ${RISCV64_HEADERS}
    DESTINATION include/orion/arch/riscv64
)

# Add custom target for RISC-V-specific tests
add_custom_target(test_riscv64
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose --output-on-failure
    DEPENDS orion_arch_riscv64
    COMMENT "Running RISC-V architecture tests"
)

# Add custom target for RISC-V documentation
add_custom_target(doc_riscv64
    COMMAND ${CMAKE_COMMAND} -E echo "Generating RISC-V documentation..."
    COMMAND ${CMAKE_COMMAND} -E echo "RISC-V architecture support documentation"
    COMMENT "Generating RISC-V documentation"
)

# Set architecture-specific variables for parent CMake
set(ORION_ARCH_RISCV64_SOURCES ${RISCV64_SOURCES} PARENT_SCOPE)
set(ORION_ARCH_RISCV64_HEADERS ${RISCV64_HEADERS} PARENT_SCOPE)
set(ORION_ARCH_RISCV64_LIBRARY orion_arch_riscv64 PARENT_SCOPE)
set(ORION_ARCH_RISCV64_CFLAGS ${RISCV64_CFLAGS} PARENT_SCOPE)
set(ORION_ARCH_RISCV64_LDFLAGS ${RISCV64_LDFLAGS} PARENT_SCOPE)

# Print configuration information
message(STATUS "RISC-V 64-bit architecture support configured")
message(STATUS "  Compiler flags: ${RISCV64_CFLAGS}")
message(STATUS "  Assembler flags: ${RISCV64_ASFLAGS}")
message(STATUS "  Linker flags: ${RISCV64_LDFLAGS}")
message(STATUS "  Source files: ${RISCV64_SOURCES}")
message(STATUS "  Header files: ${RISCV64_HEADERS}")
