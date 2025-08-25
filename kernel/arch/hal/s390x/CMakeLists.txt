# s390x HAL CMakeLists.txt
# Build configuration for s390x Hardware Abstraction Layer

set(S390X_HAL_SOURCES
    hal_s390x.c
    test_hal_s390x.c
)

set(S390X_HAL_HEADERS
    hal_s390x.h
    hal_s390x_common.h
)

# Create s390x HAL library
add_library(orion_hal_s390x STATIC ${S390X_HAL_SOURCES})

# Set include directories
target_include_directories(orion_hal_s390x PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../s390x
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_SOURCE_DIR}/kernel/core
)

# Set compiler flags for s390x
target_compile_options(orion_hal_s390x PRIVATE
    -m64
    -march=z13
    -mtune=z13
    -O2
    -Wall
    -Wextra
    -Werror
)

# Link with s390x architecture library
target_link_libraries(orion_hal_s390x
    orion_arch_s390x
)

# Install rules
install(TARGETS orion_hal_s390x
    ARCHIVE DESTINATION lib/orion
    LIBRARY DESTINATION lib/orion
)

install(FILES ${S390X_HAL_HEADERS}
    DESTINATION include/orion/hal/s390x
)

# Test configuration
if(BUILD_TESTS)
    add_executable(test_hal_s390x test_hal_s390x.c)
    target_link_libraries(test_hal_s390x
        orion_hal_s390x
        orion_arch_s390x
    )
    
    add_test(NAME HAL_s390x_Basic COMMAND test_hal_s390x)
    add_test(NAME HAL_s390x_Memory COMMAND test_hal_s390x memory)
    add_test(NAME HAL_s390x_System COMMAND test_hal_s390x system)
    add_test(NAME HAL_s390x_Advanced COMMAND test_hal_s390x advanced)
    add_test(NAME HAL_s390x_All COMMAND test_hal_s390x all)
endif()

# Documentation
if(BUILD_DOCS)
    add_custom_target(docs_s390x_hal
        COMMAND ${CMAKE_COMMAND} -E echo "Generating s390x HAL documentation..."
        COMMENT "s390x HAL documentation placeholder"
    )
endif()
