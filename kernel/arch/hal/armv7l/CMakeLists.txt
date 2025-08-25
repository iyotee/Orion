# ARMv7l HAL CMakeLists.txt
# Build configuration for ARMv7l Hardware Abstraction Layer

set(ARMV7L_HAL_SOURCES
    hal_armv7l.c
)

set(ARMV7L_HAL_HEADERS
    hal_armv7l.h
    hal_armv7l_common.h
)

# Create ARMv7l HAL library
add_library(orion_hal_armv7l STATIC ${ARMV7L_HAL_SOURCES})

# Set include directories
target_include_directories(orion_hal_armv7l PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../armv7l
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_SOURCE_DIR}/kernel/core
)

# Set compiler flags for ARMv7l
target_compile_options(orion_hal_armv7l PRIVATE
    -march=armv7-a
    -mtune=cortex-a7
    -mfpu=neon-vfpv4
    -mfloat-abi=hard
    -O2
    -Wall
    -Wextra
    -Werror
)

# Link with ARMv7l architecture library
target_link_libraries(orion_hal_armv7l
    orion_arch_armv7l
)

# Install headers
install(FILES ${ARMV7L_HAL_HEADERS}
    DESTINATION include/orion/hal/armv7l
)

# Set properties
set_target_properties(orion_hal_armv7l PROPERTIES
    OUTPUT_NAME "orion_hal_armv7l"
    POSITION_INDEPENDENT_CODE ON
)

# Add to parent scope
set(ARMV7L_HAL_LIBRARY orion_hal_armv7l PARENT_SCOPE)
