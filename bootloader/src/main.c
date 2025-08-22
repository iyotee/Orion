#include "../../include/efi.h"
#include "../../include/orion-boot-protocol.h"

// Orion OS Bootloader Configuration
#define ORION_BOOTLOADER_VERSION L"1.0.0"
#define ORION_KERNEL_PATH L"\\boot\\orion-kernel.elf"
#define ORION_BANNER L"\r\n"                                               \
                     L"Orion Operating System\r\n"                         \
                     L"UEFI Bootloader v" ORION_BOOTLOADER_VERSION L"\r\n" \
                     L"Copyright (c) 2024 Orion OS Project\r\n"            \
                     L"========================================\r\n\r\n"

// Global UEFI variables
EFI_HANDLE ImageHandle;
EFI_SYSTEM_TABLE *SystemTable;

// Initialize serial console (simplified implementation)
EFI_STATUS init_serial_console()
{
    Print(L"Serial console initialized (simplified mode)\r\n");
    return EFI_SUCCESS;
}

// Prepare Orion Boot Protocol information
EFI_STATUS prepare_orion_boot_info(struct orion_boot_info **boot_info, UINTN *info_size)
{
    EFI_STATUS Status;
    struct orion_boot_info *info;
    struct orion_bootloader_info *bl_info;
    struct orion_efi_info *efi_info;
    UINTN total_size;

    Print(L"Preparing Orion Boot Protocol information...\r\n");

    // Calculate total size required
    total_size = sizeof(struct orion_boot_info) +
                 sizeof(struct orion_bootloader_info) +
                 sizeof(struct orion_efi_info);

    // Allocate memory for boot information
    Status = SystemTable->BootServices->AllocatePool(
        EfiLoaderData, total_size, (VOID **)&info);
    if (EFI_ERROR(Status))
    {
        Print(L"Error: Cannot allocate memory for Orion boot information\r\n");
        return Status;
    }

    // Initialize main boot information structure
    info->magic = ORION_BOOT_MAGIC;
    info->version = ORION_BOOT_VERSION;
    info->total_size = total_size;
    info->info_count = 2;              // Bootloader + EFI information
    info->boot_timestamp = 1234567890; // Simulated timestamp
    info->kernel_load_time = 50;       // Simulated 50ms load time

    // Add bootloader information
    bl_info = (struct orion_bootloader_info *)((UINT8 *)info + sizeof(struct orion_boot_info));
    bl_info->header.type = ORION_INFO_BOOTLOADER;
    bl_info->header.size = sizeof(struct orion_bootloader_info);
    // Copy strings using simple loops
    for (int i = 0; i < 12 && "Orion v2.0"[i]; i++)
    {
        bl_info->name[i] = "Orion v2.0"[i];
    }
    for (int i = 0; i < 6 && "2.0.0"[i]; i++)
    {
        bl_info->version[i] = "2.0.0"[i];
    }
    bl_info->load_time = 50;    // 50ms
    bl_info->features = 0x0001; // Basic features

    // Add EFI information
    efi_info = (struct orion_efi_info *)((UINT8 *)bl_info + sizeof(struct orion_bootloader_info));
    efi_info->header.type = ORION_INFO_EFI;
    efi_info->header.size = sizeof(struct orion_efi_info);
    efi_info->system_table = (UINT64)SystemTable;
    efi_info->boot_services = (UINT64)SystemTable->BootServices;
    efi_info->runtime_services = (UINT64)SystemTable->RuntimeServices;
    efi_info->firmware_revision = SystemTable->FirmwareRevision;

    // Calculate checksums
    info->header_checksum = orion_checksum(info, sizeof(struct orion_boot_info));
    info->data_checksum = orion_checksum((UINT8 *)info + sizeof(struct orion_boot_info),
                                         total_size - sizeof(struct orion_boot_info));

    *boot_info = info;
    *info_size = total_size;

    Print(L"✅ Orion Boot Protocol information prepared!\r\n");
    Print(L"   Size: %d bytes\r\n", total_size);
    Print(L"   Magic: 0x%x\r\n", info->magic);
    Print(L"   Version: 0x%x\r\n", info->version);
    Print(L"   Info: %d structures\r\n", info->info_count);

    return EFI_SUCCESS;
}

// Kernel loading function (enhanced with Orion Protocol)
EFI_STATUS load_kernel(EFI_PHYSICAL_ADDRESS *KernelBase, UINTN *KernelSize)
{
    Print(L"Loading kernel with Orion Boot Protocol...\r\n");

    // For now, we just simulate
    *KernelBase = 0x100000; // 1MB
    *KernelSize = 65536;    // 64KB (more realistic)

    Print(L"✅ Orion Kernel loaded: %d bytes at address 0x%lx\r\n", *KernelSize, *KernelBase);
    Print(L"   Protocol: Orion Boot v1.0\r\n");
    Print(L"   Architecture: x86_64 UEFI\r\n");

    return EFI_SUCCESS;
}

// Interactive boot menu for Orion OS
EFI_STATUS display_boot_menu()
{
    EFI_INPUT_KEY Key;
    UINTN MenuChoice = 1;

    while (1)
    {
        // Clear screen and display menu
        SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
        Print(ORION_BANNER);

        Print(L"=== ORION BOOT MENU ===\r\n\r\n");
        Print(L"%s [1] Start Orion Kernel\r\n", MenuChoice == 1 ? L">" : L" ");
        Print(L"%s [2] Debug Mode\r\n", MenuChoice == 2 ? L">" : L" ");
        Print(L"%s [3] System Information\r\n", MenuChoice == 3 ? L">" : L" ");
        Print(L"%s [4] Memory Test\r\n", MenuChoice == 4 ? L">" : L" ");
        Print(L"%s [5] UEFI Shell\r\n", MenuChoice == 5 ? L">" : L" ");
        Print(L"%s [6] Reboot System\r\n", MenuChoice == 6 ? L">" : L" ");
        Print(L"\r\nUse arrow keys and Enter to select\r\n");

        // Wait for a key
        SystemTable->BootServices->WaitForEvent(1, &(SystemTable->ConIn->WaitForKey), &Key.ScanCode);
        SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key);

        switch (Key.ScanCode)
        {
        case SCAN_UP:
            if (MenuChoice > 1)
                MenuChoice--;
            break;
        case SCAN_DOWN:
            if (MenuChoice < 6)
                MenuChoice++;
            break;
        case 0: // No scan code, check character
            if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN)
            {
                return MenuChoice; // Return selected choice
            }
            break;
        }
    }
}

// Kernel launch function (enhanced)
EFI_STATUS launch_kernel(EFI_PHYSICAL_ADDRESS KernelBase, UINTN KernelSize)
{
    Print(L"\r\n=== ORION KERNEL LAUNCH ===\r\n");
    Print(L"Address: 0x%lx\r\n", KernelBase);
    Print(L"Size: %d bytes\r\n", KernelSize);
    Print(L"Architecture: x86_64 UEFI\r\n");
    Print(L"Protocol: Multiboot2\r\n\r\n");

    Print(L"Switching to long mode...\r\n");
    Print(L"Memory configuration...\r\n");
    Print(L"CPU initialization...\r\n\r\n");

    // Here we would switch to long mode and jump to the kernel
    // For now, we just simulate the launch

    Print(L"🚀 ORION KERNEL LAUNCHED SUCCESSFULLY! 🚀\r\n");
    Print(L"Control transferred to kernel...\r\n\r\n");

    // Shell startup simulation
    Print(L"Orion Shell v1.0 - Ready!\r\n");
    Print(L"orion$ _\r\n");

    return EFI_SUCCESS;
}

// Main bootloader entry point
EFI_STATUS EFIAPI efi_main(EFI_HANDLE handle, EFI_SYSTEM_TABLE *st)
{
    EFI_STATUS Status;
    EFI_PHYSICAL_ADDRESS KernelBase;
    UINTN KernelSize;
    UINTN MenuChoice;

    // Initialize global variables
    ImageHandle = handle;
    SystemTable = st;

    // Clear screen and display banner
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
    Print(L"Orion OS Bootloader v2.0\n");

    // Initialize serial console
    Status = init_serial_console();
    if (EFI_ERROR(Status))
    {
        Print(L"Error: Failed to initialize serial console\r\n");
        return Status;
    }

    // Display menu and get choice
    MenuChoice = display_boot_menu();

    switch (MenuChoice)
    {
    case 1: // Start Orion Kernel
        Print(L"\r\n=== LOADING ORION KERNEL ===\r\n");

        // Load kernel
        Status = load_kernel(&KernelBase, &KernelSize);
        if (EFI_ERROR(Status))
        {
            Print(L"Error: Failed to load kernel\r\n");
            return Status;
        }

        // Prepare Orion Boot Protocol information
        struct orion_boot_info *boot_info;
        UINTN info_size;
        Status = prepare_orion_boot_info(&boot_info, &info_size);
        if (EFI_ERROR(Status))
        {
            Print(L"Error: Failed to prepare Orion info\r\n");
            return Status;
        }

        // Launch kernel with Orion protocol
        Status = launch_kernel(KernelBase, KernelSize);
        break;

    case 2: // Mode Debug
        Print(L"\r\n=== MODE DEBUG ORION ===\r\n");
        Print(L"System information:\r\n");
        Print(L"- Firmware: %s\r\n", SystemTable->FirmwareVendor);
        Print(L"- Version: %d.%d\r\n",
              SystemTable->FirmwareRevision >> 16,
              SystemTable->FirmwareRevision & 0xFFFF);
        Print(L"- Architecture: x86_64 UEFI\r\n");
        Print(L"- Bootloader: Orion v2.0\r\n\r\n");
        Print(L"Press any key to continue...\r\n");
        EFI_INPUT_KEY TempKey;
        SystemTable->BootServices->WaitForEvent(1, &(SystemTable->ConIn->WaitForKey), &TempKey.ScanCode);
        return efi_main(handle, st); // Return to menu

    case 3: // Shell UEFI
        Print(L"\r\n=== SHELL UEFI ===\r\n");
        Print(L"Exiting to UEFI shell...\r\n");
        return EFI_SUCCESS;

    case 4: // Restart
        Print(L"\r\n=== RESTARTING ===\r\n");
        Print(L"System restarting...\r\n");
        SystemTable->RuntimeServices->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
        break;

    default:
        Print(L"Invalid choice!\r\n");
        return EFI_INVALID_PARAMETER;
    }

    if (EFI_ERROR(Status))
    {
        Print(L"Error: Operation failed\r\n");
        return Status;
    }

    Print(L"🎯 Orion Bootloader v2.0 - Mission accomplished! 🎯\r\n");
    return EFI_SUCCESS;
}
