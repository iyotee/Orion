# PowerPC HAL CMakeLists.txt
# Build configuration for PowerPC Hardware Abstraction Layer

set(POWERPC_HAL_SOURCES
    hal_powerpc.c
    test_hal_powerpc.c
    integration_test.c
)

set(POWERPC_HAL_HEADERS
    hal_powerpc.h
    hal_powerpc_common.h
)

# Create PowerPC HAL library
add_library(orion_hal_powerpc STATIC ${POWERPC_HAL_SOURCES})

# Set include directories
target_include_directories(orion_hal_powerpc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../powerpc
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_SOURCE_DIR}/kernel/core
)

# Set compiler flags for PowerPC
target_compile_options(orion_hal_powerpc PRIVATE
    -m64
    -mcpu=power8
    -mtune=power8
    -O2
    -Wall
    -Wextra
    -Werror
)

# Link with PowerPC architecture library
target_link_libraries(orion_hal_powerpc
    orion_arch_powerpc
)

# Install rules
install(TARGETS orion_hal_powerpc
    ARCHIVE DESTINATION lib/orion
    LIBRARY DESTINATION lib/orion
)

install(FILES ${POWERPC_HAL_HEADERS}
    DESTINATION include/orion/hal/powerpc
)

# Test configuration
if(BUILD_TESTS)
    add_executable(test_hal_powerpc test_hal_powerpc.c)
    target_link_libraries(test_hal_powerpc
        orion_hal_powerpc
        orion_arch_powerpc
    )
    
    add_test(NAME HAL_PowerPC_Basic COMMAND test_hal_powerpc)
    add_test(NAME HAL_PowerPC_Memory COMMAND test_hal_powerpc memory)
    add_test(NAME HAL_PowerPC_System COMMAND test_hal_powerpc system)
    add_test(NAME HAL_PowerPC_Advanced COMMAND test_hal_powerpc advanced)
    add_test(NAME HAL_PowerPC_All COMMAND test_hal_powerpc all)
endif()

# Documentation
if(BUILD_DOCS)
    add_custom_target(docs_powerpc_hal
        COMMAND ${CMAKE_COMMAND} -E echo "Generating PowerPC HAL documentation..."
        COMMENT "PowerPC HAL documentation placeholder"
    )
endif()
