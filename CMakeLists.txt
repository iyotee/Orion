cmake_minimum_required(VERSION 3.20)
project(Orion LANGUAGES C ASM)

# Configuration générale
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Support des architectures cibles
set(ORION_ARCH "x86_64" CACHE STRING "Architecture cible (x86_64 ou aarch64)")
set_property(CACHE ORION_ARCH PROPERTY STRINGS "x86_64" "aarch64")

# Configuration du compilateur (Clang requis)
set(CMAKE_C_COMPILER clang)
set(CMAKE_ASM_COMPILER clang)
set(CMAKE_LINKER lld)

# Flags de compilation pour le noyau freestanding
set(KERNEL_CFLAGS
    -ffreestanding
    -fno-stack-protector
    -fno-stack-check
    -fno-lto
    -fno-PIE
    -fno-PIC
    -mno-80387
    -mno-mmx
    -mno-3dnow
    -mno-sse
    -mno-sse2
    -mno-red-zone
    -mcmodel=large
    -Wall
    -Wextra
    -Werror
    -pedantic
    -Wno-unused-parameter
)

# Flags spécifiques à l'architecture
if(ORION_ARCH STREQUAL "x86_64")
    list(APPEND KERNEL_CFLAGS
        -target x86_64-unknown-none
        -mno-red-zone
        -mcmodel=large
    )
    set(KERNEL_LDFLAGS
        -target x86_64-unknown-none
        -nostdlib
        -Wl,-n
        -Wl,--gc-sections
        -Wl,-no-pie
        -static
    )
elseif(ORION_ARCH STREQUAL "aarch64")
    list(APPEND KERNEL_CFLAGS
        -target aarch64-unknown-none
    )
    set(KERNEL_LDFLAGS
        -target aarch64-unknown-none
        -nostdlib
        -Wl,--gc-sections
    )
endif()

# Configuration Debug/Release
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND KERNEL_CFLAGS -O0 -g3 -DDEBUG)
else()
    list(APPEND KERNEL_CFLAGS -O2 -DNDEBUG)
endif()

# Répertoires d'inclusion
include_directories(kernel/include)

# Sous-projets
add_subdirectory(kernel)
add_subdirectory(tools)

# Cibles principales
add_custom_target(all-orion
    DEPENDS kernel
    COMMENT "Construction complète d'Orion"
)

# Cible pour QEMU x86_64
add_custom_target(qemu-x86_64
    COMMAND qemu-system-x86_64 
        -machine q35
        -cpu qemu64
        -smp 2
        -m 512M
        -serial stdio
        -no-reboot
        -no-shutdown
        -kernel $<TARGET_FILE:kernel>
    DEPENDS kernel
    COMMENT "Exécution d'Orion dans QEMU x86_64"
)

# Cible pour générer l'ISO UEFI
add_custom_target(iso
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/build-iso.sh $<TARGET_FILE:kernel> ${CMAKE_BINARY_DIR}/orion.iso
    DEPENDS kernel
    COMMENT "Génération de l'ISO UEFI bootable"
)

# Tests
enable_testing()
# add_subdirectory(tests OPTIONAL)
