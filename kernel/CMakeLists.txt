# Orion Kernel Build
set(KERNEL_SOURCES
    # Entry point and assembly files
    arch/${ORION_ARCH}/boot.S
    arch/${ORION_ARCH}/arch_asm.S

    # Architecture C files
    arch/${ORION_ARCH}/entry.c
    arch/${ORION_ARCH}/cpu.c
    arch/${ORION_ARCH}/mmu.c
    arch/${ORION_ARCH}/interrupts.c
    arch/${ORION_ARCH}/interrupt_handlers.c
    arch/${ORION_ARCH}/interrupt_stubs.c
    arch/${ORION_ARCH}/timers.c

    # Kernel core
    core/main.c
    core/boot.c
    core/scheduler.c
    core/syscalls.c
    core/ipc.c
    core/capabilities.c
    core/panic.c
    core/shell.c
    core/init_process.c
    core/thread.c
    core/process.c
    core/servers.c
    core/klog.c
    core/string.c
    core/stubs.c

    # Memory management
    mm/heap.c
    mm/pmm.c
    mm/slab.c
    mm/vmm.c

    # File system
    fs/ramfs.c
    fs/vfs.c
)

# Create kernel executable
add_executable(kernel ${KERNEL_SOURCES})

# Set target properties
set_target_properties(kernel PROPERTIES
    OUTPUT_NAME "orion_kernel"
    SUFFIX ".elf"
)

# Include directories
target_include_directories(kernel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/${ORION_ARCH}/include
)

# Link with custom linker script
set_target_properties(kernel PROPERTIES
    LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS}"
)
