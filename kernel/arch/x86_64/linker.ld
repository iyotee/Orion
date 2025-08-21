/* Script de linkage simple pour Orion x86_64 */
OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(kernel_start)

SECTIONS {
    /* Code du noyau */
    .text : {
        *(.boot)
        *(.text)
        *(.text.*)
    }
    
    /* Données en lecture seule */
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }
    
    /* Données initialisées */
    .data : {
        *(.data)
        *(.data.*)
    }
    
    /* Données non-initialisées */
    .bss : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }
    
    /* Éliminer les sections non nécessaires */
    /DISCARD/ : {
        *(.note*)
        *(.comment)
        *(.eh_frame*)
    }
}
