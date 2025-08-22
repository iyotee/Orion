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
.extern kernel_main

# Global variables for UEFI parameters
.section .data
.global boot_efi_handle
.global boot_efi_system_table
boot_efi_handle:
    .quad 0
boot_efi_system_table:
    .quad 0





# Main kernel entry point
kernel_start:
    # Disable interrupts immediately
    cli
    
    # Clear direction flag
    cld
    
    # Save UEFI parameters (if present)
    # RDI = EFI_HANDLE
    # RSI = EFI_SYSTEM_TABLE*
    mov %rdi, boot_efi_handle(%rip)
    mov %rsi, boot_efi_system_table(%rip)
    
    # Configure initial stack (use temporary stack)
    lea emergency_stack_top(%rip), %rsp
    mov %rsp, %rbp
    
    # Check that we are in long mode
    call verify_long_mode
    
    # Clear BSS (uninitialized data)
    call clear_bss
    
    # Configure basic segments
    call setup_segments
    
    # Initialize CPU
    call cpu_early_init
    
    # Jump to C code
    call kernel_main
    
    # If kernel_main returns (should never happen)
halt_loop:
    hlt
    jmp halt_loop

# Verify that we are indeed in long mode
verify_long_mode:
    # Read EFER MSR
    mov $0xC0000080, %ecx   # MSR_EFER
    rdmsr
    
    # Check LMA bit (bit 10)
    test $0x400, %eax
    jnz long_mode_ok
    
    # Not in long mode - panic
    mov $not_long_mode_msg, %rsi
    call panic_early
    
long_mode_ok:
    ret

# Clear BSS section (simplified for now)
clear_bss:
    # For now, no complex BSS cleanup
    ret

# Configure basic segments (minimal GDT)
setup_segments:
    # Load our minimal GDT
    lgdt gdt_desc(%rip)
    
    # Reload segments
    mov $0x10, %ax          # Kernel data segment
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    
    # Long jump to reload CS
    pushq $0x08             # Kernel code segment
    lea code_segment_reloaded(%rip), %rax
    pushq %rax
    lretq
    
code_segment_reloaded:
    ret

# Early CPU initialization
cpu_early_init:
    # Enable security features in CR4
    mov %cr4, %rax
    
    # Enable SMEP (bit 20) if available
    or $0x100000, %rax      # CR4_SMEP
    
    # Enable SMAP (bit 21) if available  
    or $0x200000, %rax      # CR4_SMAP
    
    # Enable UMIP (bit 11) if available
    or $0x800, %rax         # CR4_UMIP
    
    mov %rax, %cr4
    
    # Enable NX bit in EFER if not already done
    mov $0xC0000080, %ecx   # MSR_EFER
    rdmsr
    or $0x800, %eax         # EFER_NXE
    wrmsr
    
    ret

# Early panic (before console is ready)
panic_early:
    # RSI = message
    # For now, just halt
    cli
panic_early_loop:
    hlt
    jmp panic_early_loop

# Minimal GDT for boot
.align 16
gdt_start:
    # Null descriptor (0x00)
    .quad 0x0000000000000000
    
    # 64-bit kernel code segment (0x08)
    .quad 0x00AF9A000000FFFF    # Base=0, Limit=4GB, DPL=0, Code64
    
    # Kernel data segment (0x10) 
    .quad 0x00CF92000000FFFF    # Base=0, Limit=4GB, DPL=0, Data
    
    # 64-bit user code segment (0x18)
    .quad 0x00AFFA000000FFFF    # Base=0, Limit=4GB, DPL=3, Code64
    
    # User data segment (0x20)
    .quad 0x00CFF2000000FFFF    # Base=0, Limit=4GB, DPL=3, Data
    
gdt_end:

gdt_desc:
    .word gdt_end - gdt_start - 1   # Limit
    .quad gdt_start                 # Base

# Error messages
not_long_mode_msg:
    .asciz "PANIC: CPU not in long mode"



# Emergency interrupt stack (4KB)
.section .bss
.align 4096
emergency_stack_bottom:
    .space 4096
emergency_stack_top:
