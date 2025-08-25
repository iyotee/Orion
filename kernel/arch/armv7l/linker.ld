/*
 * ORION OS - ARMv7l Linker Script
 * 
 * This file defines the memory layout and sections for ARMv7l architecture.
 */

/* Memory layout for ARMv7l */
MEMORY
{
    /* RAM - typically starts at 0x80000000 for ARMv7l */
    RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 128M
    
    /* ROM/Flash - typically starts at 0x00000000 */
    ROM (rx) : ORIGIN = 0x00000000, LENGTH = 16M
    
    /* Stack space - allocated at the end of RAM */
    STACK (rw) : ORIGIN = 0x87F00000, LENGTH = 1M
}

/* Entry point */
ENTRY(_start)

/* Output format */
OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)

/* Section definitions */
SECTIONS
{
    /* Boot section - must be at the beginning */
    .text.boot : ALIGN(4K)
    {
        KEEP(*(.text.boot))
    } >ROM
    
    /* Exception vectors - must be aligned to 4KB */
    .text.vectors : ALIGN(4K)
    {
        KEEP(*(.text.vectors))
    } >ROM
    
    /* Main text section */
    .text : ALIGN(4K)
    {
        *(.text)
        *(.text.*)
        *(.gnu.linkonce.t.*)
        
        /* C++ constructors */
        KEEP(*(.init))
        KEEP(*(.fini))
        
        /* Exception unwinding */
        KEEP(*(.eh_frame))
        KEEP(*(.eh_frame_hdr))
        KEEP(*(.eh_frame_entry))
        KEEP(*(.gcc_except_table))
        KEEP(*(.gcc_except_table.*))
    } >ROM
    
    /* Read-only data */
    .rodata : ALIGN(4K)
    {
        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r.*)
        
        /* C++ type info */
        KEEP(*(.gnu.linkonce.r.*))
        KEEP(*(.gnu.linkonce.r.*))
    } >ROM
    
    /* ARM exception index table */
    .ARM.exidx : ALIGN(4K)
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        *(.gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } >ROM
    
    /* ARM unwind tables */
    .ARM.extab : ALIGN(4K)
    {
        *(.ARM.extab*)
        *(.gnu.linkonce.armextab.*)
    } >ROM
    
    /* Data section - copied to RAM at startup */
    .data : ALIGN(4K)
    {
        __data_start = .;
        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d.*)
        
        /* C++ static constructors */
        KEEP(*(.ctors))
        KEEP(*(.dtors))
        
        /* C++ global constructors */
        KEEP(*(.init_array))
        KEEP(*(.fini_array))
        
        __data_end = .;
    } >RAM AT>ROM
    
    /* BSS section - zeroed at startup */
    .bss : ALIGN(4K)
    {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        __bss_end = .;
    } >RAM
    
    /* Stack section */
    .stack : ALIGN(4K)
    {
        __stack_start = .;
        *(.stack)
        . = ALIGN(4K);
        __stack_end = .;
    } >STACK
    
    /* Heap section */
    .heap : ALIGN(4K)
    {
        __heap_start = .;
        *(.heap)
        . = ALIGN(4K);
        __heap_end = .;
    } >RAM
    
    /* ARMv7l specific sections */
    
    /* NEON/VFP data */
    .neon_data : ALIGN(16)
    {
        *(.neon_data)
        *(.neon_data.*)
    } >RAM
    
    /* NEON/VFP text */
    .neon_text : ALIGN(16)
    {
        *(.neon_text)
        *(.neon_text.*)
    } >ROM
    
    /* TrustZone secure data */
    .secure_data : ALIGN(4K)
    {
        __secure_data_start = .;
        *(.secure_data)
        *(.secure_data.*)
        __secure_data_end = .;
    } >RAM
    
    /* TrustZone secure text */
    .secure_text : ALIGN(4K)
    {
        __secure_text_start = .;
        *(.secure_text)
        *(.secure_text.*)
        __secure_text_end = .;
    } >ROM
    
    /* Performance monitoring data */
    .perf_data : ALIGN(4K)
    {
        __perf_data_start = .;
        *(.perf_data)
        *(.perf_data.*)
        __perf_data_end = .;
    } >RAM
    
    /* Debug information */
    .debug : ALIGN(4K)
    {
        *(.debug)
        *(.debug.*)
        *(.debug_abbrev)
        *(.debug_aranges)
        *(.debug_frame)
        *(.debug_info)
        *(.debug_line)
        *(.debug_loc)
        *(.debug_macinfo)
        *(.debug_pubnames)
        *(.debug_ranges)
        *(.debug_str)
    } >ROM
    
    /* ARM specific debug sections */
    .ARM.attributes : ALIGN(4K)
    {
        KEEP(*(.ARM.attributes))
    } >ROM
    
    /* Comment sections - can be discarded */
    .comment : ALIGN(4K)
    {
        *(.comment)
        *(.comment.*)
    } >ROM
    
    /* Note sections - can be discarded */
    .note : ALIGN(4K)
    {
        *(.note)
        *(.note.*)
        *(.note.gnu.build-id)
    } >ROM
    
    /* Discard unused sections */
    /DISCARD/ :
    {
        *(.interp)
        *(.hash)
        *(.dynsym)
        *(.dynstr)
        *(.gnu.version)
        *(.gnu.version_d)
        *(.gnu.version_r)
        *(.rel)
        *(.rel.*)
        *(.rela)
        *(.rela.*)
        *(.plt)
        *(.got)
        *(.got.plt)
        *(.dynamic)
        *(.dynsym)
        *(.dynstr)
        *(.gnu.hash)
        *(.gnu.version)
        *(.gnu.version_d)
        *(.gnu.version_r)
        *(.gnu.version_r)
        *(.gnu.version_d)
        *(.gnu.version)
        *(.gnu.hash)
        *(.dynstr)
        *(.dynsym)
        *(.dynamic)
        *(.got.plt)
        *(.got)
        *(.plt)
        *(.rela.*)
        *(.rela)
        *(.rel.*)
        *(.rel)
        *(.gnu.version_r)
        *(.gnu.version_d)
        *(.gnu.version)
        *(.dynstr)
        *(.dynsym)
        *(.hash)
        *(.interp)
    }
    
    /* Symbol definitions */
    __rom_start = ORIGIN(ROM);
    __rom_end = ORIGIN(ROM) + LENGTH(ROM);
    __ram_start = ORIGIN(RAM);
    __ram_end = ORIGIN(RAM) + LENGTH(RAM);
    __stack_start = ORIGIN(STACK);
    __stack_end = ORIGIN(STACK) + LENGTH(STACK);
    
    /* ARMv7l specific symbols */
    __armv7l_start = .;
    __armv7l_end = .;
    
    /* End of image */
    __image_end = .;
    
    /* Ensure proper alignment */
    . = ALIGN(4K);
}
