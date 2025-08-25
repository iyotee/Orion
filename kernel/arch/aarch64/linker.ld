/*
 * Orion Operating System - aarch64 Linker Script
 *
 * Memory layout and section definitions for aarch64 architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_start)

/* Memory layout for aarch64 */
MEMORY {
    KERNEL_TEXT (rx) : ORIGIN = 0x40000000, LENGTH = 64M
    KERNEL_DATA (rw) : ORIGIN = 0x44000000, LENGTH = 64M
    KERNEL_BSS (rw) : ORIGIN = 0x48000000, LENGTH = 64M
    KERNEL_HEAP (rw) : ORIGIN = 0x4C000000, LENGTH = 128M
    KERNEL_STACK (rw) : ORIGIN = 0x54000000, LENGTH = 16M
    DEVICE_MEM (rw) : ORIGIN = 0x60000000, LENGTH = 256M
    RESERVED (rw) : ORIGIN = 0x70000000, LENGTH = 256M
}

SECTIONS {
    /* Boot section */
    .text.boot : {
        *(.text.boot)
    } > KERNEL_TEXT
    
    /* Exception vectors */
    .text.vectors : {
        *(.text.vectors)
    } > KERNEL_TEXT
    
    /* Main text section */
    .text : {
        *(.text)
        *(.text.*)
    } > KERNEL_TEXT
    
    /* Read-only data */
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } > KERNEL_TEXT
    
    /* Data section */
    .data : {
        *(.data)
        *(.data.*)
    } > KERNEL_DATA
    
    /* BSS section */
    .bss : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    } > KERNEL_BSS
    
    /* Stack */
    .stack : {
        . = ALIGN(16);
        aarch64_boot_stack_top = .;
        . += 4096;
    } > KERNEL_STACK
    
    /* Symbols for C code */
    __kernel_start = ADDR(.text.boot);
    __kernel_end = __bss_end;
    __kernel_size = __kernel_end - __kernel_start;
}
