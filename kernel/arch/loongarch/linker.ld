/*
 * ORION OS - LoongArch Linker Script
 * 
 * This file defines the memory layout and sections for the LoongArch kernel.
 */

/* Memory layout for LoongArch */
MEMORY {
    /* Kernel text section - executable and read-only */
    KERNEL_TEXT (rx) : ORIGIN = 0x9000000000000000, LENGTH = 0x10000000  /* 256MB */
    
    /* Kernel data section - read-write */
    KERNEL_DATA (rw) : ORIGIN = 0x9000000010000000, LENGTH = 0x10000000  /* 256MB */
    
    /* Kernel BSS section - zero-initialized data */
    KERNEL_BSS (rw) : ORIGIN = 0x9000000020000000, LENGTH = 0x10000000  /* 256MB */
    
    /* Kernel heap section - dynamic memory allocation */
    KERNEL_HEAP (rw) : ORIGIN = 0x9000000030000000, LENGTH = 0x10000000  /* 256MB */
    
    /* Kernel stack section - per-CPU stacks */
    KERNEL_STACK (rw) : ORIGIN = 0x9000000040000000, LENGTH = 0x10000000  /* 256MB */
    
    /* Device memory - MMIO regions */
    DEVICE_MEM (rw) : ORIGIN = 0x9000000050000000, LENGTH = 0x10000000  /* 256MB */
    
    /* Reserved memory - for future use */
    RESERVED (rw) : ORIGIN = 0x9000000060000000, LENGTH = 0x10000000  /* 256MB */
}

/* Entry point */
ENTRY(_start)

/* Output format */
OUTPUT_FORMAT("elf64-loongarch")
OUTPUT_ARCH(loongarch64)

/* Section definitions */
SECTIONS {
    /* Kernel text section */
    .text : {
        /* Exception vectors must be at the beginning */
        KEEP(*(.text.vectors))
        
        /* Main kernel text */
        *(.text)
        *(.text.*)
        
        /* Keep function names for debugging */
        KEEP(*(.text.unlikely))
        KEEP(*(.text.hot))
        
        /* Align to page boundary */
        . = ALIGN(0x1000);
    } > KERNEL_TEXT
    
    /* Read-only data section */
    .rodata : {
        *(.rodata)
        *(.rodata.*)
        
        /* String constants */
        *(.rodata.str1.8)
        *(.rodata.str1.1)
        
        /* Align to cache line */
        . = ALIGN(64);
    } > KERNEL_TEXT
    
    /* Exception handling data */
    .eh_frame : {
        KEEP(*(.eh_frame))
        KEEP(*(.eh_frame.*))
    } > KERNEL_TEXT
    
    /* Debug information */
    .debug_info : {
        KEEP(*(.debug_info))
        KEEP(*(.debug_info.*))
    } > KERNEL_TEXT
    
    .debug_abbrev : {
        KEEP(*(.debug_abbrev))
        KEEP(*(.debug_abbrev.*))
    } > KERNEL_TEXT
    
    .debug_line : {
        KEEP(*(.debug_line))
        KEEP(*(.debug_line.*))
    } > KERNEL_TEXT
    
    .debug_frame : {
        KEEP(*(.debug_frame))
        KEEP(*(.debug_frame.*))
    } > KERNEL_TEXT
    
    .debug_str : {
        KEEP(*(.debug_str))
        KEEP(*(.debug_str.*))
    } > KERNEL_TEXT
    
    .debug_loc : {
        KEEP(*(.debug_loc))
        KEEP(*(.debug_loc.*))
    } > KERNEL_TEXT
    
    .debug_ranges : {
        KEEP(*(.debug_ranges))
        KEEP(*(.debug_ranges.*))
    } > KERNEL_TEXT
    
    /* Kernel data section */
    .data : {
        /* Initialized data */
        *(.data)
        *(.data.*)
        
        /* Global constructors and destructors */
        KEEP(*(.init_array))
        KEEP(*(.fini_array))
        
        /* Align to cache line */
        . = ALIGN(64);
    } > KERNEL_DATA AT> KERNEL_TEXT
    
    /* Kernel BSS section */
    .bss : {
        /* Zero-initialized data */
        *(.bss)
        *(.bss.*)
        
        /* Common symbols */
        *(COMMON)
        
        /* Align to page boundary */
        . = ALIGN(0x1000);
        
        /* Kernel BSS end marker */
        _kernel_bss_end = .;
    } > KERNEL_BSS
    
    /* Kernel heap section */
    .heap : {
        /* Heap start marker */
        _kernel_heap_start = .;
        
        /* Reserve space for heap */
        . = . + 0x10000000;  /* 256MB */
        
        /* Heap end marker */
        _kernel_heap_end = .;
    } > KERNEL_HEAP
    
    /* Kernel stack section */
    .stack : {
        /* Stack start marker */
        _kernel_stack_start = .;
        
        /* Reserve space for stacks */
        . = . + 0x10000000;  /* 256MB */
        
        /* Stack end marker */
        _kernel_stack_end = .;
    } > KERNEL_STACK
    
    /* Device memory section */
    .device : {
        /* MMIO regions */
        *(.device)
        *(.device.*)
        
        /* Align to page boundary */
        . = ALIGN(0x1000);
    } > DEVICE_MEM
    
    /* Reserved section */
    .reserved : {
        /* Reserved memory */
        *(.reserved)
        *(.reserved.*)
        
        /* Align to page boundary */
        . = ALIGN(0x1000);
    } > RESERVED
    
    /* Discard unused sections */
    /DISCARD/ : {
        *(.comment)
        *(.gnu.warning)
        *(.note)
        *(.note.*)
        *(.gnu.build-id)
        *(.gnu.hash)
        *(.hash)
        *(.dynsym)
        *(.dynstr)
        *(.dynamic)
        *(.interp)
        *(.plt)
        *(.plt.*)
        *(.got)
        *(.got.*)
        *(.rela.*)
        *(.iplt)
        *(.igot)
        *(.igot.*)
    }
    
    /* Symbol definitions */
    _kernel_start = LOADADDR(.text);
    _kernel_end = LOADADDR(.text) + SIZEOF(.text) + SIZEOF(.rodata) + SIZEOF(.data);
    
    /* Memory layout information */
    _kernel_text_start = ADDR(.text);
    _kernel_text_end = ADDR(.text) + SIZEOF(.text);
    _kernel_rodata_start = ADDR(.rodata);
    _kernel_rodata_end = ADDR(.rodata) + SIZEOF(.rodata);
    _kernel_data_start = ADDR(.data);
    _kernel_data_end = ADDR(.data) + SIZEOF(.data);
    _kernel_bss_start = ADDR(.bss);
    _kernel_heap_start = ADDR(.heap);
    _kernel_stack_start = ADDR(.stack);
    _kernel_device_start = ADDR(.device);
    _kernel_reserved_start = ADDR(.reserved);
    
    /* Page alignment information */
    _kernel_text_pages = (_kernel_text_end - _kernel_text_start + 0xFFF) >> 12;
    _kernel_rodata_pages = (_kernel_rodata_end - _kernel_rodata_start + 0xFFF) >> 12;
    _kernel_data_pages = (_kernel_data_end - _kernel_data_start + 0xFFF) >> 12;
    _kernel_bss_pages = (_kernel_bss_end - _kernel_bss_start + 0xFFF) >> 12;
    
    /* Cache line alignment information */
    _kernel_text_cache_lines = (_kernel_text_end - _kernel_text_start + 63) >> 6;
    _kernel_rodata_cache_lines = (_kernel_rodata_end - _kernel_rodata_start + 63) >> 6;
    _kernel_data_cache_lines = (_kernel_data_end - _kernel_data_start + 63) >> 6;
    
    /* Architecture-specific symbols */
    PROVIDE(__loongarch64 = 1);
    PROVIDE(__loongarch = 1);
    PROVIDE(__orion_kernel = 1);
    
    /* LoongArch-specific symbols */
    PROVIDE(__loongarch_arch = 1);
    PROVIDE(__loongarch_64bit = 1);
    PROVIDE(__loongarch_lp64d = 1);
    
    /* Vector extension symbols */
    PROVIDE(__loongarch_lsx = 1);
    PROVIDE(__loongarch_lasx = 1);
    
    /* Crypto extension symbols */
    PROVIDE(__loongarch_crypto = 1);
    
    /* Virtualization symbols */
    PROVIDE(__loongarch_virt = 1);
    
    /* NUMA symbols */
    PROVIDE(__loongarch_numa = 1);
    
    /* Binary translation symbols */
    PROVIDE(__loongarch_lbt = 1);
    
    /* End of kernel */
    _end = .;
}
