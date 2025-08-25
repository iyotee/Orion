# s390x Architecture CMakeLists.txt
# Build configuration for IBM s390x architecture support

set(S390X_ARCH_SOURCES
    arch.c
    test_s390x.c
)

set(S390X_ARCH_HEADERS
    arch.h
    config.h
)

# Create s390x architecture library
add_library(orion_arch_s390x STATIC ${S390X_ARCH_SOURCES})

# Set include directories
target_include_directories(orion_arch_s390x PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/kernel/core
    ${CMAKE_SOURCE_DIR}/include
)

# Set compiler flags for s390x
target_compile_options(orion_arch_s390x PRIVATE
    -m64
    -march=z13
    -mtune=z13
    -O2
    -Wall
    -Wextra
    -Werror
    -fno-stack-protector
    -fno-builtin
)

# Compiler definitions
target_compile_definitions(orion_arch_s390x PRIVATE
    ORION_ARCH_S390X=1
    S390X_ARCH=1
)

# Link libraries
target_link_libraries(orion_arch_s390x
    pthread
    m
    rt
)

# Install rules
install(TARGETS orion_arch_s390x
    ARCHIVE DESTINATION lib/orion
    LIBRARY DESTINATION lib/orion
)

install(FILES ${S390X_ARCH_HEADERS}
    DESTINATION include/orion/arch/s390x
)

# Test configuration
if(BUILD_TESTS)
    add_executable(test_s390x_arch test_s390x.c)
    target_link_libraries(test_s390x_arch
        orion_arch_s390x
        pthread
        m
        rt
    )
    
    add_test(NAME ARCH_s390x_Basic COMMAND test_s390x_arch basic)
    add_test(NAME ARCH_s390x_Memory COMMAND test_s390x_arch memory)
    add_test(NAME ARCH_s390x_System COMMAND test_s390x_arch system)
    add_test(NAME ARCH_s390x_Advanced COMMAND test_s390x_arch advanced)
    add_test(NAME ARCH_s390x_All COMMAND test_s390x_arch all)
endif()

# Documentation
if(BUILD_DOCS)
    add_custom_target(docs_s390x_arch
        COMMAND ${CMAKE_COMMAND} -E echo "Generating s390x architecture documentation..."
        COMMENT "s390x architecture documentation placeholder"
    )
endif()
