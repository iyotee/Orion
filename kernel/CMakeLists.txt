# Construction du noyau Orion
set(KERNEL_SOURCES
    # Point d'entrée et fichiers assembleur
    arch/${ORION_ARCH}/boot.S
    arch/${ORION_ARCH}/arch_asm.S
    
    # Fichiers C de l'architecture
    arch/${ORION_ARCH}/entry.c
    arch/${ORION_ARCH}/cpu.c
    arch/${ORION_ARCH}/mmu.c
    arch/${ORION_ARCH}/interrupts.c
    arch/${ORION_ARCH}/interrupt_handlers.c
    arch/${ORION_ARCH}/interrupt_stubs.c
    arch/${ORION_ARCH}/timers.c
    arch/${ORION_ARCH}/msvc_stubs.c
    arch/${ORION_ARCH}/stubs.c
    
    # Cœur du noyau
    core/main.c
    core/boot.c
    core/scheduler.c
    core/syscalls.c
    core/ipc.c
    core/capabilities.c
    core/panic.c
    core/shell.c
    
    # Gestion mémoire
    mm/pmm.c
    mm/vmm.c
    mm/slab.c
    mm/heap.c
    
    # Système de fichiers
    fs/vfs.c
    fs/ramfs.c
    
    # Bibliothèques
    lib/string.c
)

# Headers du noyau
set(KERNEL_HEADERS
    include/orion/types.h
    include/orion/kernel.h
    include/orion/syscalls.h
    include/orion/mm.h
    include/orion/capabilities.h
    include/orion/ipc.h
    include/orion/scheduler.h
    include/orion/vfs.h
    include/orion/security.h
    arch/${ORION_ARCH}/include/arch.h
)

# Création de l'exécutable noyau
add_executable(kernel ${KERNEL_SOURCES} ${KERNEL_HEADERS})

# Include directories pour les headers
target_include_directories(kernel PRIVATE 
    include
    arch/${ORION_ARCH}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# Configuration de la compilation assembleur
enable_language(ASM)

# Définir que nous sommes dans le contexte kernel
target_compile_definitions(kernel PRIVATE ORION_KERNEL_BUILD=1)

# Application des flags de compilation selon le compilateur
if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(kernel PRIVATE ${KERNEL_CFLAGS})
    target_link_options(kernel PRIVATE ${KERNEL_LDFLAGS})
    
    # Flags spécifiques pour l'assembleur
    set_source_files_properties(arch/${ORION_ARCH}/boot.S PROPERTIES COMPILE_FLAGS "-fPIC" LANGUAGE ASM)
    set_source_files_properties(arch/${ORION_ARCH}/arch_asm.S PROPERTIES COMPILE_FLAGS "-fPIC" LANGUAGE ASM)
else()
    # MSVC flags pour Windows
    target_compile_options(kernel PRIVATE /W3 /WX- /Od /Zi)
    target_compile_definitions(kernel PRIVATE DEBUG _CRT_SECURE_NO_WARNINGS)
endif()

# Script de linkage spécifique à l'architecture (uniquement pour GCC/Clang)
if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/arch/${ORION_ARCH}/linker.ld")
    target_link_options(kernel PRIVATE -T ${LINKER_SCRIPT} -no-pie -nostdlib -nostartfiles)
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
