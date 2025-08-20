# Construction du noyau Orion
set(KERNEL_SOURCES
    # Point d'entrée
    arch/${ORION_ARCH}/boot.S
    arch/${ORION_ARCH}/entry.c
    arch/${ORION_ARCH}/cpu.c
    arch/${ORION_ARCH}/mmu.c
    arch/${ORION_ARCH}/interrupts.c
    arch/${ORION_ARCH}/msvc_stubs.c
    
    # Cœur du noyau
    core/main.c
    core/boot.c
    core/scheduler.c
    core/syscalls.c
    core/ipc.c
    core/capabilities.c
    core/panic.c
    
    # Gestion mémoire
    mm/pmm.c
    mm/vmm.c
    mm/slab.c
    mm/heap.c
    
    # Bibliothèques
    lib/string.c
)

# Headers du noyau
set(KERNEL_HEADERS
    include/orion/types.h
    include/orion/kernel.h
    include/orion/boot.h
    include/orion/syscalls.h
    include/orion/mm.h
    include/orion/capabilities.h
    include/orion/ipc.h
    arch/${ORION_ARCH}/include/arch.h
)

# Création de l'exécutable noyau
add_executable(kernel ${KERNEL_SOURCES} ${KERNEL_HEADERS})

# Include directories pour les headers
target_include_directories(kernel PRIVATE 
    include
    arch/${ORION_ARCH}/include
)

# Application des flags de compilation selon le compilateur
if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(kernel PRIVATE ${KERNEL_CFLAGS} -Wno-gnu-zero-variadic-macro-arguments)
    target_link_options(kernel PRIVATE ${KERNEL_LDFLAGS})
else()
    # MSVC flags pour Windows
    target_compile_options(kernel PRIVATE /W3 /WX- /Od /Zi)
    target_compile_definitions(kernel PRIVATE DEBUG _CRT_SECURE_NO_WARNINGS)
endif()

# Script de linkage spécifique à l'architecture (uniquement pour GCC/Clang)
if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/arch/${ORION_ARCH}/linker.ld")
    target_link_options(kernel PRIVATE -T ${LINKER_SCRIPT})
endif()

# Propriétés de l'exécutable
set_target_properties(kernel PROPERTIES
    OUTPUT_NAME "orion-kernel"
    SUFFIX ".elf"
)

# Installation
install(TARGETS kernel
    DESTINATION bin
    COMPONENT kernel
)
