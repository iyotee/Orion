# ORION OS - LoongArch Architecture CMake Configuration
#
# This file configures the build for LoongArch architecture support.

# Set architecture-specific compiler flags
set(LOONGARCH_CFLAGS
    -march=loongarch64
    -mabi=lp64d
    -fno-stack-protector
    -fno-builtin
    -nostdinc
    -nostdlib
    -fno-pie
    -fno-pic
    -mno-relax
)

# Set architecture-specific assembler flags
set(LOONGARCH_ASFLAGS
    -march=loongarch64
    -mabi=lp64d
    -fno-stack-protector
    -fno-builtin
    -nostdinc
    -nostdlib
    -fno-pie
    -fno-pic
    -mno-relax
)

# Set architecture-specific linker flags
set(LOONGARCH_LDFLAGS
    -melf64loongarch
    -no-pie
    -nostdlib
    -static
    -T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
)

# Architecture source files
set(LOONGARCH_SOURCES
    arch.c
    arch_asm.S
    arch_advanced.c
    boot.S
    interrupts.c
    interrupt_handlers.c
    timers.c
    entry.c
    syscall_entry.S
    test_loongarch.c
)

# Architecture header files
set(LOONGARCH_HEADERS
    arch.h
    config.h
)

# Create LoongArch architecture library
add_library(orion_arch_loongarch STATIC ${LOONGARCH_SOURCES})

# Set compiler flags for the library
target_compile_options(orion_arch_loongarch PRIVATE ${LOONGARCH_CFLAGS})
target_compile_options(orion_arch_loongarch PRIVATE -DLOONGARCH_ARCH)

# Set assembler flags for assembly files
set_source_files_properties(arch_asm.S PROPERTIES
    COMPILE_FLAGS "${LOONGARCH_ASFLAGS}"
)

# Set include directories
target_include_directories(orion_arch_loongarch PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/kernel/arch/hal/common
    ${CMAKE_SOURCE_DIR}/kernel/core
)

# Set compile definitions
target_compile_definitions(orion_arch_loongarch PRIVATE
    ORION_ARCH_LOONGARCH=1
    LOONGARCH_ARCH=1
    __LOONGARCH__=1
)

# Link with HAL common library
target_link_libraries(orion_arch_loongarch
    orion_hal_common
)

# Install rules
install(TARGETS orion_arch_loongarch
    LIBRARY DESTINATION lib/orion/arch
    ARCHIVE DESTINATION lib/orion/arch
)

install(FILES ${LOONGARCH_HEADERS}
    DESTINATION include/orion/arch/loongarch
)

# Add custom target for LoongArch-specific tests
add_custom_target(test_loongarch
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose --output-on-failure
    DEPENDS orion_arch_loongarch
    COMMENT "Running LoongArch architecture tests"
)

# Add custom target for LoongArch documentation
add_custom_target(doc_loongarch
    COMMAND ${CMAKE_COMMAND} -E echo "Generating LoongArch documentation..."
    COMMAND ${CMAKE_COMMAND} -E echo "LoongArch architecture support documentation"
    COMMENT "Generating LoongArch documentation"
)

# Set architecture-specific variables for parent CMake
set(ORION_ARCH_LOONGARCH_SOURCES ${LOONGARCH_SOURCES} PARENT_SCOPE)
set(ORION_ARCH_LOONGARCH_HEADERS ${LOONGARCH_HEADERS} PARENT_SCOPE)
set(ORION_ARCH_LOONGARCH_LIBRARY orion_arch_loongarch PARENT_SCOPE)
set(ORION_ARCH_LOONGARCH_CFLAGS ${LOONGARCH_CFLAGS} PARENT_SCOPE)
set(ORION_ARCH_LOONGARCH_LDFLAGS ${LOONGARCH_LDFLAGS} PARENT_SCOPE)

# Print configuration information
message(STATUS "LoongArch architecture support configured")
message(STATUS "  Compiler flags: ${LOONGARCH_CFLAGS}")
message(STATUS "  Assembler flags: ${LOONGARCH_ASFLAGS}")
message(STATUS "  Linker flags: ${LOONGARCH_LDFLAGS}")
message(STATUS "  Source files: ${LOONGARCH_SOURCES}")
message(STATUS "  Header files: ${LOONGARCH_HEADERS}")
