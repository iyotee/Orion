# ORION OS - MIPS Architecture CMake Configuration
#
# CMake configuration for MIPS architecture (MIPS32, MIPS64)
# Supports all MIPS processors: R3000, R4000, R5000, R10000, etc.
#
# Developed by Jeremy Noverraz (1988-2025)
# August 2025, Lausanne, Switzerland
#
# Copyright (c) 2024-2025 Orion OS Project
# License: MIT

# Set MIPS architecture
set(ARCH_NAME "mips")
set(ARCH_DESCRIPTION "MIPS Architecture (MIPS32, MIPS64)")

# MIPS-specific compiler flags
set(MIPS_CFLAGS
    -march=mips64r2
    -mabi=64
    -mhard-float
    -mno-shared
    -mno-abicalls
    -fno-pic
    -fno-pie
    -fno-stack-protector
    -fno-omit-frame-pointer
    -Wall
    -Wextra
    -Werror
    -std=c99
)

# MIPS-specific linker flags
set(MIPS_LDFLAGS
    -melf64btsmip
    -static
    -nostdlib
    -nostartfiles
    -nodefaultlibs
    -Wl,--gc-sections
    -Wl,--strip-all
)

# MIPS source files
set(MIPS_SOURCES
    arch.c
    entry.c
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
    system.c
    drivers.c
    utils.c
    diagnostics.c
    features.c
    validation.c
    test_mips.c
)

# MIPS header files
set(MIPS_HEADERS
    arch.h
    config.h
)

# MIPS assembly files
set(MIPS_ASM_SOURCES
    syscall_entry.S
    boot.S
    arch_asm.S
)

# MIPS linker script
set(MIPS_LINKER_SCRIPT "linker.ld")

# Create MIPS library target
add_library(${ARCH_NAME} STATIC ${MIPS_SOURCES} ${MIPS_ASM_SOURCES})

# Set target properties
set_target_properties(${ARCH_NAME} PROPERTIES
    COMPILE_FLAGS "${MIPS_CFLAGS}"
    LINK_FLAGS "${MIPS_LDFLAGS}"
    OUTPUT_NAME "${ARCH_NAME}"
    ARCHIVE_OUTPUT_NAME "${ARCH_NAME}"
)

# Set include directories
target_include_directories(${ARCH_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

# Set compile definitions
target_compile_definitions(${ARCH_NAME} PRIVATE
    MIPS_ARCH=1
    MIPS64=1
    ORION_OS=1
    ORION_ARCH_MIPS=1
)

# Link against system libraries
target_link_libraries(${ARCH_NAME} PRIVATE
    # Add system libraries here if needed
)

# Install rules
install(TARGETS ${ARCH_NAME}
    ARCHIVE DESTINATION lib/${ARCH_NAME}
    LIBRARY DESTINATION lib/${ARCH_NAME}
    RUNTIME DESTINATION bin/${ARCH_NAME}
)

install(FILES ${MIPS_HEADERS}
    DESTINATION include/${ARCH_NAME}
)

# MIPS-specific test target
if(BUILD_TESTS)
    add_executable(${ARCH_NAME}_test test_${ARCH_NAME}.c)
    set_target_properties(${ARCH_NAME}_test PROPERTIES
        COMPILE_FLAGS "${MIPS_CFLAGS}"
        LINK_FLAGS "${MIPS_LDFLAGS}"
        OUTPUT_NAME "${ARCH_NAME}_test"
    )
    
    target_include_directories(${ARCH_NAME}_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
    )
    
    target_link_libraries(${ARCH_NAME}_test PRIVATE ${ARCH_NAME})
    
    # Add test to CTest
    add_test(NAME ${ARCH_NAME}_test COMMAND ${ARCH_NAME}_test)
endif()

# MIPS-specific documentation
if(BUILD_DOCS)
    add_custom_target(${ARCH_NAME}_docs
        COMMAND ${CMAKE_COMMAND} -E echo "Generating MIPS architecture documentation..."
        COMMENT "Generating MIPS architecture documentation"
    )
endif()

# MIPS-specific install target
add_custom_target(${ARCH_NAME}_install
    COMMAND ${CMAKE_COMMAND} -E echo "Installing MIPS architecture..."
    COMMENT "Installing MIPS architecture"
    DEPENDS ${ARCH_NAME}
)

# MIPS-specific clean target
add_custom_target(${ARCH_NAME}_clean
    COMMAND ${CMAKE_COMMAND} -E echo "Cleaning MIPS architecture..."
    COMMENT "Cleaning MIPS architecture"
)

# Print MIPS configuration
message(STATUS "MIPS Architecture Configuration:")
message(STATUS "  Architecture: ${ARCH_NAME}")
message(STATUS "  Description: ${ARCH_DESCRIPTION}")
message(STATUS "  Sources: ${MIPS_SOURCES}")
message(STATUS "  Headers: ${MIPS_HEADERS}")
message(STATUS "  Assembly: ${MIPS_ASM_SOURCES}")
message(STATUS "  Linker Script: ${MIPS_LINKER_SCRIPT}")
message(STATUS "  C Flags: ${MIPS_CFLAGS}")
message(STATUS "  LD Flags: ${MIPS_LDFLAGS}")
message(STATUS "  Build Tests: ${BUILD_TESTS}")
message(STATUS "  Build Docs: ${BUILD_DOCS}")
