/*
 * ORION OS - RISC-V 64-bit Linker Script
 *
 * Memory layout and section placement for RISC-V 64-bit kernel
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
    /* Kernel space - 16GB starting at 0x8000000000000000 */
    kernel_ram (rwx) : ORIGIN = 0x8000000000000000, LENGTH = 0x400000000
    
    /* User space - 8EB starting at 0x0000000000000000 */
    user_ram (rwx) : ORIGIN = 0x0000000000000000, LENGTH = 0x8000000000000000
    
    /* Device memory - 1GB starting at 0x1000000000000000 */
    device_mem (rw) : ORIGIN = 0x1000000000000000, LENGTH = 0x40000000
}

/* Entry point */
ENTRY(_start)

/* Output format and architecture */
OUTPUT_FORMAT("elf64-littleriscv")
OUTPUT_ARCH(riscv:rv64)

/* Sections */
SECTIONS
{
    /* Boot section at the beginning */
    .text.boot : {
        KEEP(*(.text.boot))
    } > kernel_ram
    
    /* Main text section */
    .text : {
        *(.text)
        *(.text.*)
        *(.gnu.linkonce.t.*)
    } > kernel_ram
    
    /* Read-only data */
    .rodata : {
        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r.*)
    } > kernel_ram
    
    /* Data section */
    .data : {
        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d.*)
    } > kernel_ram
    
    /* BSS section */
    .bss : {
        *(.bss)
        *(.bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
    } > kernel_ram
    
    /* Stack section */
    .stack : {
        . = ALIGN(16);
        _stack_bottom = .;
        . += 16384;  /* 16KB stack */
        _stack_top = .;
    } > kernel_ram
    
    /* Kernel heap */
    .heap : {
        . = ALIGN(4096);
        _heap_start = .;
        . += 0x1000000;  /* 16MB heap */
        _heap_end = .;
    } > kernel_ram
    
    /* Page tables */
    .page_tables : {
        . = ALIGN(4096);
        _page_tables_start = .;
        . += 0x100000;  /* 1MB for page tables */
        _page_tables_end = .;
    } > kernel_ram
    
    /* Exception vectors */
    .exception_vectors : {
        . = ALIGN(4096);
        _exception_vectors_start = .;
        *(.exception_vectors)
        . = ALIGN(4096);
        _exception_vectors_end = .;
    } > kernel_ram
    
    /* Interrupt handlers */
    .interrupt_handlers : {
        . = ALIGN(4096);
        _interrupt_handlers_start = .;
        *(.interrupt_handlers)
        . = ALIGN(4096);
        _interrupt_handlers_end = .;
    } > kernel_ram
    
    /* System call table */
    .syscall_table : {
        . = ALIGN(8);
        _syscall_table_start = .;
        *(.syscall_table)
        . = ALIGN(8);
        _syscall_table_end = .;
    } > kernel_ram
    
    /* Debug symbols */
    .debug_info : {
        *(.debug_info)
    } > kernel_ram
    
    .debug_abbrev : {
        *(.debug_abbrev)
    } > kernel_ram
    
    .debug_line : {
        *(.debug_line)
    } > kernel_ram
    
    .debug_str : {
        *(.debug_str)
    } > kernel_ram
    
    /* Discard unused sections */
    /DISCARD/ : {
        *(.comment)
        *(.gnu.*)
        *(.note.*)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }
    
    /* Symbol definitions */
    _kernel_start = ADDR(.text.boot);
    _kernel_end = .;
    _kernel_size = _kernel_end - _kernel_start;
    
    /* Memory layout info */
    _kernel_base = 0x8000000000000000;
    _kernel_size_total = 0x400000000;
    _user_base = 0x0000000000000000;
    _user_size = 0x8000000000000000;
    _device_base = 0x1000000000000000;
    _device_size = 0x40000000;
}
