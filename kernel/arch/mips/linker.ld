/*
 * ORION OS - MIPS Architecture Linker Script
 *
 * Linker script for MIPS architecture (MIPS32, MIPS64)
 * Defines memory layout and section placement
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

/* Memory regions */
MEMORY
{
    /* RAM: Main system memory */
    RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 256M
    
    /* ROM: Boot and read-only data */
    ROM (rx) : ORIGIN = 0xBFC00000, LENGTH = 4M
    
    /* STACK: Stack memory */
    STACK (rw) : ORIGIN = 0x90000000, LENGTH = 64K
}

/* Entry point */
ENTRY(_start)

/* Output format */
OUTPUT_FORMAT("elf64-bigmips")
OUTPUT_ARCH(mips:isa64r2)

/* Sections */
SECTIONS
{
    /* Boot section - loaded at ROM address */
    .text.boot : {
        KEEP(*(.text.boot))
        *(.text.boot.*)
    } >ROM AT>ROM
    
    /* Exception vectors - must be aligned */
    .text.vectors : {
        KEEP(*(.text.vectors))
        *(.text.vectors.*)
    } >ROM AT>ROM
    
    /* Main text section */
    .text : {
        *(.text)
        *(.text.*)
        *(.gnu.linkonce.t.*)
    } >RAM AT>RAM
    
    /* Read-only data */
    .rodata : {
        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r.*)
    } >RAM AT>RAM
    
    /* Data section */
    .data : {
        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d.*)
    } >RAM AT>RAM
    
    /* BSS section */
    .bss : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        __bss_end = .;
    } >RAM AT>RAM
    
    /* Stack section */
    .stack : {
        __stack_start = .;
        . += 64K;
        __stack_end = .;
    } >STACK AT>STACK
    
    /* Heap section */
    .heap : {
        __heap_start = .;
        . += 16M;
        __heap_end = .;
    } >RAM AT>RAM
    
    /* NEON data section (if applicable) */
    .neon_data : {
        *(.neon_data)
        *(.neon_data.*)
    } >RAM AT>RAM
    
    /* Secure data section */
    .secure_data : {
        *(.secure_data)
        *(.secure_data.*)
    } >RAM AT>RAM
    
    /* Performance data section */
    .perf_data : {
        *(.perf_data)
        *(.perf_data.*)
    } >RAM AT>RAM
    
    /* Debug section */
    .debug : {
        *(.debug)
        *(.debug.*)
    } >RAM AT>RAM
    
    /* Discard sections */
    /DISCARD/ : {
        *(.comment)
        *(.gnu.*)
        *(.note.*)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }
    
    /* Symbols for C code */
    __kernel_start = ADDR(.text.boot);
    __kernel_end = __bss_end;
    __kernel_size = __kernel_end - __kernel_start;
    
    __ram_start = ORIGIN(RAM);
    __ram_end = ORIGIN(RAM) + LENGTH(RAM);
    __ram_size = LENGTH(RAM);
    
    __rom_start = ORIGIN(ROM);
    __rom_end = ORIGIN(ROM) + LENGTH(ROM);
    __rom_size = LENGTH(ROM);
    
    __stack_start = ADDR(.stack);
    __stack_end = ADDR(.stack) + SIZEOF(.stack);
    __stack_size = SIZEOF(.stack);
    
    __heap_start = ADDR(.heap);
    __heap_end = ADDR(.heap) + SIZEOF(.heap);
    __heap_size = SIZEOF(.heap);
}
