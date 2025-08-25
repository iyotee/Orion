# ORION OS - POWER Architecture CMake Configuration
#
# Build configuration for POWER architecture support
#
# Developed by Jeremy Noverraz (1988-2025)
# August 2025, Lausanne, Switzerland
#
# Copyright (c) 2024-2025 Orion OS Project
# License: MIT

# POWER Architecture Sources
set(POWER_SOURCES
    arch.c
    arch_advanced.c
    entry.c
    entry_asm.S
    interrupts.c
    mmu.c
    timer.c
    cache.c
    exceptions.c
    vectors.c
    devices.c
    performance.c
    security.c
    virtualization.c
              )

# POWER Architecture Headers
set(POWER_HEADERS
    arch.h
    config.h
    boot_config.h
)

# POWER Architecture Assembly Files
set(POWER_ASM_SOURCES
    # boot.S
    # interrupts.S
    # syscall_entry.S
)

# Create POWER architecture library
add_library(orion_arch_power STATIC ${POWER_SOURCES} ${POWER_ASM_SOURCES})

# Set target properties
set_target_properties(orion_arch_power PROPERTIES
    OUTPUT_NAME "orion_arch_power"
    POSITION_INDEPENDENT_CODE ON
)

# Set compiler flags for POWER
target_compile_options(orion_arch_power PRIVATE
    -mcpu=power8
    -mtune=power8
    -maltivec
    -mvsx
    -mhard-float
    -m64
    -O2
    -Wall
    -Wextra
    -Werror
    -std=c99
)

# Set assembler flags for POWER
if(POWER_ASM_SOURCES)
    set_source_files_properties(${POWER_ASM_SOURCES} PROPERTIES
        COMPILE_FLAGS "-mcpu=power8 -maltivec -mvsx -m64"
    )
endif()

# Set linker flags for POWER
target_link_options(orion_arch_power PRIVATE
    -Wl,--gc-sections
    -Wl,--as-needed
)

# Include directories
target_include_directories(orion_arch_power PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# Compiler definitions
target_compile_definitions(orion_arch_power PRIVATE
    ORION_ARCH_POWER
    POWER_ARCH
    _GNU_SOURCE
)

# Link libraries
target_link_libraries(orion_arch_power
    orion_core
    orion_common
)

# Installation
install(TARGETS orion_arch_power
    ARCHIVE DESTINATION lib/orion
    LIBRARY DESTINATION lib/orion
    RUNTIME DESTINATION bin/orion
)

install(FILES ${POWER_HEADERS}
    DESTINATION include/orion/arch/power
)

# Custom targets
add_custom_target(test_power
    COMMAND ${CMAKE_COMMAND} -E echo "Running POWER architecture tests..."
    COMMAND $<TARGET_FILE:orion_arch_power>
    DEPENDS orion_arch_power
    COMMENT "Testing POWER architecture support"
)

add_custom_target(doc_power
    COMMAND ${CMAKE_COMMAND} -E echo "Generating POWER architecture documentation..."
    COMMAND ${CMAKE_COMMAND} -E echo "Documentation generation not implemented yet"
    COMMENT "Generating POWER architecture documentation"
)

# POWER-specific test executable
add_executable(power_test ${POWER_SOURCES})

target_compile_options(power_test PRIVATE
    -mcpu=power8
    -mtune=power8
    -maltivec
    -mvsx
    -mhard-float
    -m64
    -O2
    -Wall
    -Wextra
    -std=c99
)

target_include_directories(power_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(power_test PRIVATE
    ORION_ARCH_POWER
    POWER_ARCH
    _GNU_SOURCE
    TEST_MODE
)

# POWER architecture information
message(STATUS "POWER Architecture Configuration:")
message(STATUS "  Sources: ${POWER_SOURCES}")
message(STATUS "  Headers: ${POWER_HEADERS}")
message(STATUS "  Assembly: ${POWER_ASM_SOURCES}")
message(STATUS "  Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  Target: power8")
message(STATUS "  Features: AltiVec, VSX, 64-bit")
