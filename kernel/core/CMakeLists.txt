# Orion OS - Kernel Core CMakeLists.txt
#
# Builds the core kernel components: scheduler, memory management, IPC, etc.
#
# Developed by Jeremy Noverraz (1988-2025)
# August 2025, Lausanne, Switzerland

# Core kernel source files
set(KERNEL_CORE_SOURCES
    main.c
    boot.c
    scheduler.c
    scheduler_apple_silicon.c
    ipc.c
    capabilities.c
    init_process.c
    process.c
    thread.c
    servers.c
    klog.c
    panic.c
    stubs.c
    syscalls.c
    string.c
    shell.c
    klog_test.c
)

# Create kernel core object library
add_library(orion_core OBJECT
    ${KERNEL_CORE_SOURCES}
)

# Set include directories
target_include_directories(orion_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/kernel/hal
    ${CMAKE_SOURCE_DIR}/kernel/hal/common
    ${CMAKE_SOURCE_DIR}/kernel/hal/${ORION_ARCH}
    ${CMAKE_SOURCE_DIR}/include
)

# Compiler definitions
target_compile_definitions(orion_core PRIVATE
    ORION_KERNEL_BUILD=1
    ORION_CORE_BUILD=1
    ORION_ARCH_${ORION_ARCH}=1
)

# Compiler flags
if(WIN32)
    target_compile_options(orion_core PRIVATE
        /W3
        /WX
        /GS-
        /O2
        /Zi
    )
else()
    target_compile_options(orion_core PRIVATE
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
target_link_libraries(orion_core
    pthread
    m
    rt
    dl
)

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION include/orion/core
    FILES_MATCHING PATTERN "*.h"
)

# Create core test executable
if(BUILD_TESTING)
    add_executable(core_test
        tests/core_test.c
        tests/scheduler_test.c
        tests/memory_test.c
        tests/ipc_test.c
        tests/process_test.c
        tests/thread_test.c
        tests/sync_test.c
        tests/time_test.c
        tests/interrupt_test.c
        tests/syscall_test.c
        tests/capability_test.c
        tests/security_test.c
    )
    
    target_link_libraries(core_test
        orion_core
        orion_hal
        pthread
        m
        rt
        dl
    )
    
    add_test(NAME CoreTest COMMAND core_test)
endif()

# Create performance benchmark executable
if(BUILD_TESTING)
    add_executable(core_benchmark
        benchmarks/scheduler_benchmark.c
        benchmarks/memory_benchmark.c
        benchmarks/ipc_benchmark.c
        benchmarks/process_benchmark.c
        benchmarks/thread_benchmark.c
        benchmarks/sync_benchmark.c
        benchmarks/time_benchmark.c
        benchmarks/interrupt_benchmark.c
        benchmarks/syscall_benchmark.c
        benchmarks/capability_benchmark.c
        benchmarks/security_benchmark.c
    )
    
    target_link_libraries(core_benchmark
        orion_core
        orion_hal
        pthread
        m
        rt
        dl
    )
    
    add_test(NAME CoreBenchmark COMMAND core_benchmark)
endif()
