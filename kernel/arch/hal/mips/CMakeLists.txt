# MIPS HAL CMakeLists.txt
# Build configuration for MIPS Hardware Abstraction Layer

set(MIPS_HAL_SOURCES
    hal_mips.c
)

set(MIPS_HAL_HEADERS
    hal_mips.h
    hal_mips_common.h
)

# Create MIPS HAL library
add_library(orion_hal_mips STATIC ${MIPS_HAL_SOURCES})

# Set include directories
target_include_directories(orion_hal_mips PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../mips
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_SOURCE_DIR}/kernel/core
)

# Set compiler flags for MIPS
target_compile_options(orion_hal_mips PRIVATE
    -march=mips64
    -mtune=mips64r2
    -O2
    -Wall
    -Wextra
    -Werror
)

# Link with MIPS architecture library
target_link_libraries(orion_hal_mips
    orion_arch_mips
)

# Install headers
install(FILES ${MIPS_HAL_HEADERS}
    DESTINATION include/orion/hal/mips
)

# Set properties
set_target_properties(orion_hal_mips PROPERTIES
    OUTPUT_NAME "orion_hal_mips"
    POSITION_INDEPENDENT_CODE ON
)

# Add to parent scope
set(MIPS_HAL_LIBRARY orion_hal_mips PARENT_SCOPE)
