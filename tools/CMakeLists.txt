# Outils et utilitaires Orion

# Outils simples en C
add_executable(orion-hexdump
    hexdump.c
)

add_executable(orion-klog
    klog.c
)

# Ces outils seront principalement en Rust via Cargo
# Mais on peut avoir quelques utilitaires C simples

# Installation
install(TARGETS orion-hexdump orion-klog
    DESTINATION bin
    COMPONENT tools
)
