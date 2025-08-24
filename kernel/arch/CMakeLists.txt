# Orion OS - Kernel Architecture CMakeLists.txt
#
# Builds architecture-specific kernel components
#
# Developed by Jeremy Noverraz (1988-2025)
# August 2025, Lausanne, Switzerland

# Architecture-specific source files
if(ORION_ARCH STREQUAL "x86_64")
    set(ARCH_SOURCES
        x86_64/boot.S
        x86_64/arch_asm.S
        x86_64/entry.c
        x86_64/cpu.c
        x86_64/mmu.c
        x86_64/interrupts.c
        x86_64/interrupt_handlers.c
        x86_64/interrupt_stubs.c
        x86_64/timers.c
        x86_64/msvc_stubs.c
        x86_64/stubs.c
        x86_64/syscall_entry.S
    )
    set(ARCH_INCLUDES x86_64)
elseif(ORION_ARCH STREQUAL "aarch64")
    set(ARCH_SOURCES
        # aarch64/arch_aarch64.c  # Not yet implemented
    )
    set(ARCH_INCLUDES aarch64)
elseif(ORION_ARCH STREQUAL "riscv64")
    set(ARCH_SOURCES
        # riscv64/arch_riscv64.c  # Not yet implemented
    )
    set(ARCH_INCLUDES riscv64)
else()
    # Generic architecture for unsupported architectures
    set(ARCH_SOURCES
        # generic/arch_generic.c  # Not yet implemented
    )
    set(ARCH_INCLUDES generic)
endif()

# Create architecture object library
add_library(orion_arch OBJECT
    ${ARCH_SOURCES}
)

# Set include directories
target_include_directories(orion_arch PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/${ARCH_INCLUDES}
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/kernel/hal
    ${CMAKE_SOURCE_DIR}/kernel/core
    ${CMAKE_SOURCE_DIR}/include
)

# Compiler definitions
target_compile_definitions(orion_arch PRIVATE
    ORION_KERNEL_BUILD=1
    ORION_ARCH_BUILD=1
    ORION_ARCH_${ORION_ARCH}=1
)

# Compiler flags
if(WIN32)
    target_compile_options(orion_arch PRIVATE
        /W3
        /WX
        /GS-
        /O2
        /Zi
    )
else()
    target_compile_options(orion_arch PRIVATE
        -Wall
        -Wextra
        -Werror
        -fno-stack-protector
        -fno-builtin
        -O2
        -g
    )
endif()

# Link libraries
target_link_libraries(orion_arch
    pthread
    m
    rt
    dl
)

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION include/orion/arch
    FILES_MATCHING PATTERN "*.h"
)

# Architecture tests not yet implemented
message(STATUS "Architecture tests not yet implemented")
