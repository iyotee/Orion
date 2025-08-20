# Orion OS x86_64 UEFI Boot Entry Point
# Professional implementation for academic environments

.code64
.section .boot, "ax"

# Orion Boot Protocol Header
.align 8
orion_boot_header:
    .long 0x0410494F        # Orion Boot Protocol magic ("ORIO")
    .long 0x00010000        # Protocol version 1.0.0
    .long orion_boot_header_end - orion_boot_header  # Header length
    .long 0x00000000        # Checksum (calculated by bootloader)
    
    # End marker
    .long 0x00000000        # End of header
orion_boot_header_end:

# Global symbols and external references
.global kernel_start
.global _stack_top
.extern kernel_main

# Global variables for UEFI parameters
.section .data
.global boot_efi_handle
.global boot_efi_system_table
boot_efi_handle:
    .quad 0
boot_efi_system_table:
    .quad 0





# Point d'entrée principal du noyau
kernel_start:
    # Disable interrupts immédiatement
    cli
    
    # Clear direction flag
    cld
    
    # Sauvegarder les paramètres UEFI (si présents)
    # RDI = EFI_HANDLE
    # RSI = EFI_SYSTEM_TABLE*
    mov %rdi, boot_efi_handle(%rip)
    mov %rsi, boot_efi_system_table(%rip)
    
    # Configurer la pile initiale
    lea _stack_top(%rip), %rsp
    mov %rsp, %rbp
    
    # Vérifier que nous sommes en long mode
    call verify_long_mode
    
    # Nettoyer BSS (données non-initialisées)
    call clear_bss
    
    # Configurer les segments de base
    call setup_segments
    
    # Initialiser le CPU
    call cpu_early_init
    
    # Sauter vers le code C
    call kernel_main
    
    # Si kernel_main retourne (ne devrait jamais arriver)
halt_loop:
    hlt
    jmp halt_loop

# Vérifier que nous sommes bien en long mode
verify_long_mode:
    # Lire EFER MSR
    mov $0xC0000080, %ecx   # MSR_EFER
    rdmsr
    
    # Vérifier LMA bit (bit 10)
    test $0x400, %eax
    jnz long_mode_ok
    
    # Pas en long mode - panic
    mov $not_long_mode_msg, %rsi
    call panic_early
    
long_mode_ok:
    ret

# Nettoyer la section BSS
clear_bss:
    lea _bss_start(%rip), %rdi
    lea _bss_end(%rip), %rcx
    sub %rdi, %rcx          # Taille BSS
    shr $3, %rcx            # Diviser par 8 (qwords)
    xor %rax, %rax          # Valeur 0
    rep stosq               # Remplir avec 0
    ret

# Configurer les segments de base (GDT minimale)
setup_segments:
    # Charger notre GDT minimale
    lgdt gdt_desc(%rip)
    
    # Recharger les segments
    mov $0x10, %ax          # Segment données noyau
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    
    # Long jump pour recharger CS
    pushq $0x08             # Segment code noyau
    lea code_segment_reloaded(%rip), %rax
    pushq %rax
    lretq
    
code_segment_reloaded:
    ret

# Initialisation CPU précoce
cpu_early_init:
    # Activer les features de sécurité dans CR4
    mov %cr4, %rax
    
    # Activer SMEP (bit 20) si disponible
    or $0x100000, %rax      # CR4_SMEP
    
    # Activer SMAP (bit 21) si disponible  
    or $0x200000, %rax      # CR4_SMAP
    
    # Activer UMIP (bit 11) si disponible
    or $0x800, %rax         # CR4_UMIP
    
    mov %rax, %cr4
    
    # Activer NX bit dans EFER si pas déjà fait
    mov $0xC0000080, %ecx   # MSR_EFER
    rdmsr
    or $0x800, %eax         # EFER_NXE
    wrmsr
    
    ret

# Panic précoce (avant que la console soit prête)
panic_early:
    # RSI = message
    # Pour l'instant, juste halter
    cli
panic_early_loop:
    hlt
    jmp panic_early_loop

# GDT minimale pour le boot
.align 16
gdt_start:
    # Descripteur null (0x00)
    .quad 0x0000000000000000
    
    # Segment code noyau 64-bit (0x08)
    .quad 0x00AF9A000000FFFF    # Base=0, Limite=4GB, DPL=0, Code64
    
    # Segment données noyau (0x10) 
    .quad 0x00CF92000000FFFF    # Base=0, Limite=4GB, DPL=0, Data
    
    # Segment code user 64-bit (0x18)
    .quad 0x00AFFA000000FFFF    # Base=0, Limite=4GB, DPL=3, Code64
    
    # Segment données user (0x20)
    .quad 0x00CFF2000000FFFF    # Base=0, Limite=4GB, DPL=3, Data
    
gdt_end:

gdt_desc:
    .word gdt_end - gdt_start - 1   # Limite
    .quad gdt_start                 # Base

# Messages d'erreur
not_long_mode_msg:
    .asciz "PANIC: CPU not in long mode"



# Pile d'interruption d'urgence (4KB)
.section .bss
.align 4096
emergency_stack_bottom:
    .space 4096
emergency_stack_top:
