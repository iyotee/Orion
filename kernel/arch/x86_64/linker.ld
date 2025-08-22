/* Simple linker script for Orion x86_64 */
OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(kernel_start)

SECTIONS {
    /* Kernel code */
    .text : {
        *(.boot)
        *(.text)
        *(.text.*)
    }
    
    /* Read-only data */
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }
    
    /* Initialized data */
    .data : {
        *(.data)
        *(.data.*)
    }
    
    /* Uninitialized data */
    .bss : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }
    
    /* Remove unnecessary sections */
    /DISCARD/ : {
        *(.note*)
        *(.comment)
        *(.eh_frame*)
    }
}
