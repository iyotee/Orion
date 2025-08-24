#ifndef EFI_H
#define EFI_H

#include <stdint.h>

// Basic UEFI types
typedef uint64_t UINT64;
typedef int64_t INT64;
typedef int32_t INT32;
typedef int16_t INT16;
typedef int8_t INT8;
typedef int64_t INTN;
typedef void VOID;
typedef VOID *EFI_STATUS;
typedef uint32_t UINT32;
typedef uint16_t UINT16;
typedef uint8_t UINT8;
typedef uint64_t UINTN;
typedef uint64_t EFI_PHYSICAL_ADDRESS;
typedef uint64_t EFI_VIRTUAL_ADDRESS;

// EFI status codes
#define EFI_SUCCESS 0
#define EFI_ERROR(x) ((EFI_STATUS)(0x8000000000000000ULL | (UINT64)(x)))
#define EFI_INVALID_PARAMETER EFI_ERROR(2)

// EFI memory types
#define EfiLoaderData 3

// EFI reset types
#define EfiResetCold 0

// EFI handle and table types (forward declarations)
typedef struct _EFI_HANDLE *EFI_HANDLE;
typedef struct _EFI_SYSTEM_TABLE EFI_SYSTEM_TABLE;
typedef struct _EFI_BOOT_SERVICES EFI_BOOT_SERVICES;
typedef struct _EFI_RUNTIME_SERVICES EFI_RUNTIME_SERVICES;
typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
typedef struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

// EFI input key structure
typedef struct
{
    UINT16 ScanCode;
    UINT16 UnicodeChar;
} EFI_INPUT_KEY;

// EFI system table structure
struct _EFI_SYSTEM_TABLE
{
    EFI_BOOT_SERVICES *BootServices;
    EFI_RUNTIME_SERVICES *RuntimeServices;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn;
    char *FirmwareVendor;
    UINT32 FirmwareRevision;
};

// EFI boot services structure
struct _EFI_BOOT_SERVICES
{
    EFI_STATUS (*AllocatePool)(UINT32 PoolType, UINTN Size, void **Buffer);
    EFI_STATUS (*WaitForEvent)(UINTN NumberOfEvents, void **Event, UINTN *Index);
    EFI_STATUS (*ResetSystem)(UINT32 ResetType, EFI_STATUS ResetStatus, UINTN DataSize, void *ResetData);
};

// EFI runtime services structure
struct _EFI_RUNTIME_SERVICES
{
    EFI_STATUS (*ResetSystem)(UINT32 ResetType, EFI_STATUS ResetStatus, UINTN DataSize, void *ResetData);
};

// EFI text output protocol
struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL
{
    EFI_STATUS (*ClearScreen)(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This);
};

// EFI text input protocol
struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL
{
    EFI_STATUS (*ReadKeyStroke)(EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This, EFI_INPUT_KEY *Key);
    void *WaitForKey;
};

// Scan codes
#define SCAN_UP 0x01
#define SCAN_DOWN 0x02

// Unicode characters
#define CHAR_CARRIAGE_RETURN 0x0D

// Print function (to be implemented)
void Print(const char *fmt, ...);

#endif // EFI_H
