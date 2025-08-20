/* Script de linkage pour Orion x86_64 */
OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(kernel_start)

/* Adresse virtuelle du noyau en high memory */
KERNEL_VMA = 0xFFFFFFFF80000000;

/* Adresse physique de chargement (1MB) */
KERNEL_LMA = 0x100000;

SECTIONS {
    /* Section de boot et point d'entrée */
    . = KERNEL_LMA;
    
    .boot : AT(ADDR(.boot)) {
        KEEP(*(.boot))
        . = ALIGN(4096);
    }
    
    /* Transition vers l'espace virtuel haut */
    . = KERNEL_VMA + (. - KERNEL_LMA);
    
    /* Code du noyau */
    .text : AT(ADDR(.text) - KERNEL_VMA) {
        _text_start = .;
        *(.text)
        *(.text.*)
        . = ALIGN(4096);
        _text_end = .;
    }
    
    /* Données en lecture seule */
    .rodata : AT(ADDR(.rodata) - KERNEL_VMA) {
        _rodata_start = .;
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(4096);
        _rodata_end = .;
    }
    
    /* Données initialisées */
    .data : AT(ADDR(.data) - KERNEL_VMA) {
        _data_start = .;
        *(.data)
        *(.data.*)
        . = ALIGN(4096);
        _data_end = .;
    }
    
    /* Données non-initialisées */
    .bss : AT(ADDR(.bss) - KERNEL_VMA) {
        _bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4096);
        _bss_end = .;
    }
    
    /* Pile initiale du noyau (64KB) */
    .stack : AT(ADDR(.stack) - KERNEL_VMA) {
        . = ALIGN(4096);
        _stack_bottom = .;
        . += 0x10000; /* 64KB */
        _stack_top = .;
    }
    
    /* Fin du noyau */
    _kernel_end = .;
    
    /* Symboles utiles */
    _kernel_size = _kernel_end - KERNEL_VMA;
    _kernel_phys_start = KERNEL_LMA;
    _kernel_phys_end = _kernel_phys_start + _kernel_size;
    
    /* Éliminer les sections non nécessaires */
    /DISCARD/ : {
        *(.note*)
        *(.comment)
        *(.eh_frame*)
    }
}
