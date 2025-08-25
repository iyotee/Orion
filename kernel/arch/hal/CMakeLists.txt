# Orion OS - HAL (Hardware Abstraction Layer) CMakeLists.txt
#
# Builds the Hardware Abstraction Layer components for multiple architectures
#
# Developed by Jeremy Noverraz (1988-2025)
# August 2025, Lausanne, Switzerland

# Common HAL components
set(HAL_COMMON_SOURCES
    common/hal_common.c
    hal_integration.c
    test_hal.c
)

# Architecture-specific HAL components
if(ORION_ARCH STREQUAL "x86_64")
    set(HAL_ARCH_SOURCES
        x86_64/hal_x86_64.c
    )
    set(HAL_ARCH_INCLUDES x86_64)
elseif(ORION_ARCH STREQUAL "aarch64")
    set(HAL_ARCH_SOURCES
        aarch64/hal_aarch64.c
    )
    set(HAL_ARCH_INCLUDES aarch64)
elseif(ORION_ARCH STREQUAL "riscv64")
    set(HAL_ARCH_SOURCES
        riscv64/hal_riscv64.c
    )
    set(HAL_ARCH_INCLUDES riscv64)
elseif(ORION_ARCH STREQUAL "powerpc")
    set(HAL_ARCH_SOURCES
        powerpc/hal_powerpc.c
    )
    set(HAL_ARCH_INCLUDES powerpc)
elseif(ORION_ARCH STREQUAL "s390x")
    set(HAL_ARCH_SOURCES
        s390x/hal_s390x.c
    )
    set(HAL_ARCH_INCLUDES s390x)
elseif(ORION_ARCH STREQUAL "mips")
    set(HAL_ARCH_SOURCES
        mips/hal_mips.c
    )
    set(HAL_ARCH_INCLUDES mips)
elseif(ORION_ARCH STREQUAL "armv7l")
    set(HAL_ARCH_SOURCES
        armv7l/hal_armv7l.c
    )
    set(HAL_ARCH_INCLUDES armv7l)
else()
    # Generic HAL for unsupported architectures
    set(HAL_ARCH_SOURCES
        # generic/hal_generic.c  # Not yet implemented
    )
    set(HAL_ARCH_INCLUDES generic)
endif()

# Create HAL object library
add_library(orion_hal OBJECT
    ${HAL_COMMON_SOURCES}
    ${HAL_ARCH_SOURCES}
)

# Set include directories
target_include_directories(orion_hal PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/${HAL_ARCH_INCLUDES}
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/include
)

# Compiler definitions
target_compile_definitions(orion_hal PRIVATE
    ORION_HAL_BUILD=1
    ORION_ARCH_${ORION_ARCH}=1
)

# Compiler flags
if(WIN32)
    target_compile_options(orion_hal PRIVATE
        /W3
        /WX
        /GS-
    )
else()
    target_compile_options(orion_hal PRIVATE
        -Wall
        -Wextra
        -Werror
        -fno-stack-protector
        -fno-builtin
    )
endif()

# Link libraries
target_link_libraries(orion_hal
    pthread
    m
    rt
)

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION include/orion/hal
    FILES_MATCHING PATTERN "*.h"
)

# Create HAL test executable
if(BUILD_TESTING)
    add_executable(hal_test
        tests/hal_test.c
    )
    
    target_link_libraries(hal_test
        orion_hal
        pthread
        m
        rt
    )
    
    add_test(NAME HALTest COMMAND hal_test)
endif()
