# Orion tools and utilities

# Simple C tools
add_executable(orion-hexdump
    hexdump.c
)

add_executable(orion-klog
    klog.c
)

# These tools will mainly be in Rust via Cargo
# But we can have some simple C utilities

# Installation
install(TARGETS orion-hexdump orion-klog
    DESTINATION bin
    COMPONENT tools
)
