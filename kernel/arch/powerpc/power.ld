/*
 * ORION OS - POWER Architecture Linker Script
 *
 * Memory layout and section definitions for POWER architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

OUTPUT_FORMAT("elf64-powerpc")
OUTPUT_ARCH(powerpc:common64)
ENTRY(_start)

/* Memory layout */
MEMORY
{
    /* Kernel space */
    kernel_start (rwx) : ORIGIN = 0x0000000001000000, LENGTH = 16M
    
    /* Stack space */
    stack (rw) : ORIGIN = 0x0000000002000000, LENGTH = 1M
    
    /* Heap space */
    heap (rw) : ORIGIN = 0x0000000002100000, LENGTH = 15M
    
    /* Device memory */
    device_mem (rw) : ORIGIN = 0x4000000000000000, LENGTH = 1G
    
    /* High memory */
    high_mem (rw) : ORIGIN = 0x8000000000000000, LENGTH = 1T
}

/* Section definitions */
SECTIONS
{
    /* Boot section - loaded at 0x1000000 */
    .text.boot : {
        KEEP(*(.text.boot))
        *(.text.boot.*)
    } > kernel_start
    
    /* Main text section */
    .text : {
        *(.text)
        *(.text.*)
        *(.gnu.linkonce.t.*)
    } > kernel_start
    
    /* Read-only data */
    .rodata : {
        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r.*)
    } > kernel_start
    
    /* Data section */
    .data : {
        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d.*)
    } > kernel_start
    
    /* BSS section */
    .bss : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        __bss_end = .;
    } > kernel_start
    
    /* Stack section */
    .stack : {
        __stack_bottom = .;
        . += 1M;
        __stack_top = .;
    } > stack
    
    /* Heap section */
    .heap : {
        __heap_start = .;
        . += 15M;
        __heap_end = .;
    } > heap
    
    /* Device memory section */
    .device : {
        *(.device)
        *(.device.*)
    } > device_mem
    
    /* High memory section */
    .high : {
        *(.high)
        *(.high.*)
    } > high_mem
    
    /* Discard sections */
    /DISCARD/ : {
        *(.comment)
        *(.gnu.*)
        *(.note.*)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }
    
    /* Symbol definitions */
    __kernel_start = ADDR(.text.boot);
    __kernel_end = __bss_end;
    __kernel_size = __kernel_end - __kernel_start;
    
    __text_start = ADDR(.text);
    __text_end = ADDR(.text) + SIZEOF(.text);
    __text_size = SIZEOF(.text);
    
    __data_start = ADDR(.data);
    __data_end = ADDR(.data) + SIZEOF(.data);
    __data_size = SIZEOF(.data);
    
    __bss_size = __bss_end - __bss_start;
    
    __stack_size = 1M;
    __heap_size = 15M;
    
    /* Align sections */
    . = ALIGN(4096);
    __page_aligned_start = .;
}
