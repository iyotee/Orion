#ifndef ORION_MULTI_ISA_H
#define ORION_MULTI_ISA_H

#include <orion/types.h>

// ============================================================================
// MULTI-ISA ARCHITECTURE SUPPORT
// ============================================================================

// Supported Instruction Set Architectures
typedef enum
{
    ISA_X86_64,  // Intel, AMD, VIA, Zhaoxin (DÉJÀ IMPLÉMENTÉ)
    ISA_ARM64,   // Apple M1-M4, Qualcomm, MediaTek, Samsung, HiSilicon, NXP, Ampere, Fujitsu
    ISA_RISCV64, // SiFive, Raspberry Pi, IoT, serveurs
    ISA_ARM32,   // ARMv7, ARMv8 32-bit, embarqué
    ISA_MIPS64,  // Loongson, marché chinois
    ISA_POWER64, // IBM, mainframes, HPC
    ISA_ZARCH    // IBM mainframes, entreprise
} isa_type_t;

// Architecture information structure
typedef struct
{
    isa_type_t type;
    const char *vendor;
    const char *model;
    uint64_t features;
    uint64_t capabilities;
    uint64_t version;
    uint64_t revision;
    void *arch_specific;
} isa_info_t;

// Architecture detection result
typedef struct
{
    isa_type_t detected_isa;
    const char *vendor_string;
    const char *model_string;
    uint64_t capabilities;
    bool is_supported;
    const char *notes;
} arch_detection_result_t;

// ============================================================================
// ARM64 SPECIFIC CONSTANTS
// ============================================================================

// ARM64 Exception Levels
#define ARM64_EL0 0 // User mode
#define ARM64_EL1 1 // Kernel mode
#define ARM64_EL2 2 // Hypervisor mode
#define ARM64_EL3 3 // Secure monitor mode

// ARM64 MMU Granule Sizes
#define ARM64_GRANULE_4KB (4 * 1024)
#define ARM64_GRANULE_16KB (16 * 1024)
#define ARM64_GRANULE_64KB (64 * 1024)

// ARM64 System Registers
#define ARM64_ID_AA64ISAR0_EL1 0x00000000
#define ARM64_ID_AA64ISAR1_EL1 0x00000001
#define ARM64_ID_AA64MMFR0_EL1 0x00000002
#define ARM64_ID_AA64PFR0_EL1 0x00000003
#define ARM64_ID_AA64PFR1_EL1 0x00000004

// ARM64 Feature Masks
#define ARM64_FEATURE_NEON 0x00000001
#define ARM64_FEATURE_CRYPTO 0x00000002
#define ARM64_FEATURE_FP 0x00000004
#define ARM64_FEATURE_SIMD 0x00000008
#define ARM64_FEATURE_SVE 0x00000010
#define ARM64_FEATURE_SVE2 0x00000020

// ARM64 Timer Registers
#define ARM64_CNTFRQ_EL0 0x00000000
#define ARM64_CNTP_TVAL_EL0 0x00000001
#define ARM64_CNTP_CTL_EL0 0x00000002
#define ARM64_CNTP_CVAL_EL0 0x00000003

// ARM64 Timer Control Bits
#define ARM64_CNTP_CTL_ENABLE 0x00000001
#define ARM64_CNTP_CTL_IMASK 0x00000002
#define ARM64_CNTP_CTL_ISTATUS 0x00000004

// ============================================================================
// RISC-V SPECIFIC CONSTANTS
// ============================================================================

// RISC-V Privilege Levels
#define RISCV_MODE_M 3 // Machine mode
#define RISCV_MODE_S 1 // Supervisor mode
#define RISCV_MODE_U 0 // User mode

// RISC-V CSR Numbers
#define RISCV_CSR_MHARTID 0xF14
#define RISCV_CSR_MSTATUS 0x300
#define RISCV_CSR_MIE 0x304
#define RISCV_CSR_MIP 0x344
#define RISCV_CSR_MIDELEG 0x303
#define RISCV_CSR_MEDELEG 0x302
#define RISCV_CSR_MCOUNTEREN 0x306
#define RISCV_CSR_STVEC 0x105
#define RISCV_CSR_SCOUNTEREN 0x106
#define RISCV_CSR_SENVCFG 0x10A

// RISC-V Interrupt Enable Bits
#define RISCV_MIE_MTIE 0x00000080
#define RISCV_MIE_MSIE 0x00000008
#define RISCV_MIE_MEIE 0x00000008

// RISC-V MMU Modes
#define RISCV_MMU_SV39 0x00000008
#define RISCV_MMU_SV48 0x00000009
#define RISCV_MMU_SV57 0x0000000A

// ============================================================================
// MIPS64 SPECIFIC CONSTANTS
// ============================================================================

// MIPS64 CP0 Register Numbers
#define MIPS64_CP0_PRID 0x00000000
#define MIPS64_CP0_CONFIG 0x00000001
#define MIPS64_CP0_CONFIG1 0x00000002
#define MIPS64_CP0_CONFIG2 0x00000003
#define MIPS64_CP0_CONFIG3 0x00000004
#define MIPS64_CP0_CONFIG4 0x00000005
#define MIPS64_CP0_CONFIG5 0x00000006
#define MIPS64_CP0_CONFIG6 0x00000007
#define MIPS64_CP0_EBASE 0x00000008
#define MIPS64_CP0_STATUS 0x0000000C

// MIPS64 Status Register Bits
#define MIPS64_STATUS_IE 0x00000001
#define MIPS64_STATUS_IM 0x0000FF00
#define MIPS64_STATUS_BEV 0x00400000
#define MIPS64_STATUS_ERL 0x00000004
#define MIPS64_STATUS_EXL 0x00000002

// MIPS64 TLB Entry Format
#define MIPS64_TLB_PRESENT 0x00000001
#define MIPS64_TLB_WRITE 0x00000002
#define MIPS64_TLB_EXEC 0x00000004
#define MIPS64_TLB_USER 0x00000008
#define MIPS64_TLB_GLOBAL 0x00000010
#define MIPS64_TLB_VALID 0x00000020

// ============================================================================
// POWER64 SPECIFIC CONSTANTS
// ============================================================================

// POWER64 SPR Numbers
#define POWER64_SPR_PVR 0x00000000
#define POWER64_SPR_PCR 0x00000001
#define POWER64_SPR_DSCR 0x00000002
#define POWER64_SPR_TAR 0x00000003
#define POWER64_SPR_EBBHR 0x00000004
#define POWER64_SPR_EBBRR 0x00000005
#define POWER64_SPR_HSRR0 0x00000006
#define POWER64_SPR_HSRR1 0x00000007

// POWER64 MSR Bits
#define POWER64_MSR_EE 0x0000000000008000
#define POWER64_MSR_ME 0x0000000000001000
#define POWER64_MSR_IR 0x0000000000000200
#define POWER64_MSR_DR 0x0000000000000400
#define POWER64_MSR_LE 0x0000000000000001

// POWER64 Interrupt Types
#define POWER64_IRQ_EXTERNAL 0x00000001
#define POWER64_IRQ_DECREMENTER 0x00000002
#define POWER64_IRQ_HV_DECREMENTER 0x00000004
#define POWER64_IRQ_PERFORMANCE 0x00000008

// ============================================================================
// MULTI-ISA FEATURE FLAGS
// ============================================================================

// Common Features
#define ISA_FEATURE_MMU 0x0000000000000001
#define ISA_FEATURE_FPU 0x0000000000000002
#define ISA_FEATURE_SIMD 0x0000000000000004
#define ISA_FEATURE_VIRTUALIZATION 0x0000000000000008
#define ISA_FEATURE_SECURITY 0x0000000000000010
#define ISA_FEATURE_CRYPTO 0x0000000000000020
#define ISA_FEATURE_COMPRESSION 0x0000000000000040
#define ISA_FEATURE_VECTOR 0x0000000000000080

// Advanced Features
#define ISA_FEATURE_AI_ACCELERATION 0x0000000000000100
#define ISA_FEATURE_NEURAL_ENGINE 0x0000000000000200
#define ISA_FEATURE_RAY_TRACING 0x0000000000000400
#define ISA_FEATURE_5G_MODEM 0x0000000000000800
#define ISA_FEATURE_SECURE_ENCLAVE 0x0000000000001000

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

// Architecture detection
arch_detection_result_t isa_detect(void);
bool isa_supports_feature(isa_type_t isa, uint64_t feature);
void *isa_get_arch_specific(isa_type_t isa);

// Architecture-specific initialization
int arm64_init(void);
int riscv_init(void);
int mips64_init(void);
int power64_init(void);

// Architecture-specific capabilities
uint64_t arm64_detect_capabilities(void);
uint64_t riscv_detect_capabilities(void);
uint64_t mips64_detect_capabilities(void);
uint64_t power64_detect_capabilities(void);

// Cross-architecture compatibility
bool isa_is_compatible(isa_type_t isa1, isa_type_t isa2);
uint64_t isa_get_common_features(isa_type_t isa1, isa_type_t isa2);

// ============================================================================
// INLINE FUNCTIONS
// ============================================================================

// Check if current architecture supports a feature
static inline bool current_isa_supports_feature(uint64_t feature)
{
    extern isa_info_t g_current_isa;
    return (g_current_isa.capabilities & feature) != 0;
}

// Get current architecture type
static inline isa_type_t get_current_isa_type(void)
{
    extern isa_info_t g_current_isa;
    return g_current_isa.type;
}

// Check if running on specific architecture
static inline bool is_running_on(isa_type_t isa)
{
    return get_current_isa_type() == isa;
}

// ============================================================================
// MACROS
// ============================================================================

// Architecture-specific code compilation
#ifdef __x86_64__
#define ISA_X86_64_CODE(code) code
#define ISA_ARM64_CODE(code)
#define ISA_RISCV_CODE(code)
#define ISA_MIPS64_CODE(code)
#define ISA_POWER64_CODE(code)
#elif defined(__aarch64__)
#define ISA_X86_64_CODE(code)
#define ISA_ARM64_CODE(code) code
#define ISA_RISCV_CODE(code)
#define ISA_MIPS64_CODE(code)
#define ISA_POWER64_CODE(code)
#elif defined(__riscv)
#define ISA_X86_64_CODE(code)
#define ISA_ARM64_CODE(code)
#define ISA_RISCV_CODE(code) code
#define ISA_MIPS64_CODE(code)
#define ISA_POWER64_CODE(code)
#elif defined(__mips64)
#define ISA_X86_64_CODE(code)
#define ISA_ARM64_CODE(code)
#define ISA_RISCV_CODE(code)
#define ISA_MIPS64_CODE(code) code
#define ISA_POWER64_CODE(code)
#elif defined(__powerpc64__)
#define ISA_X86_64_CODE(code)
#define ISA_ARM64_CODE(code)
#define ISA_RISCV_CODE(code)
#define ISA_MIPS64_CODE(code)
#define ISA_POWER64_CODE(code) code
#else
#define ISA_X86_64_CODE(code)
#define ISA_ARM64_CODE(code)
#define ISA_RISCV_CODE(code)
#define ISA_MIPS64_CODE(code)
#define ISA_POWER64_CODE(code)
#endif

// Feature detection macros
#define HAS_MMU() current_isa_supports_feature(ISA_FEATURE_MMU)
#define HAS_FPU() current_isa_supports_feature(ISA_FEATURE_FPU)
#define HAS_SIMD() current_isa_supports_feature(ISA_FEATURE_SIMD)
#define HAS_VIRTUALIZATION() current_isa_supports_feature(ISA_FEATURE_VIRTUALIZATION)
#define HAS_SECURITY() current_isa_supports_feature(ISA_FEATURE_SECURITY)
#define HAS_CRYPTO() current_isa_supports_feature(ISA_FEATURE_CRYPTO)

// Architecture-specific macros
#define IS_X86_64() is_running_on(ISA_X86_64)
#define IS_ARM64() is_running_on(ISA_ARM64)
#define IS_RISCV() is_running_on(ISA_RISCV64)
#define IS_MIPS64() is_running_on(ISA_MIPS64)
#define IS_POWER64() is_running_on(ISA_POWER64)

#endif // ORION_MULTI_ISA_H
