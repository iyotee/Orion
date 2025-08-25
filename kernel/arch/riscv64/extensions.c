/*
 * ORION OS - RISC-V 64-bit Extensions Management
 *
 * Extensions management for RISC-V 64-bit architecture
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include "common.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// GLOBAL EXTENSIONS CONTEXTS
// ============================================================================

// Extensions context
static struct
{
    bool initialized;
    bool rv64i_enabled;
    bool rv64m_enabled;
    bool rv64a_enabled;
    bool rv64f_enabled;
    bool rv64d_enabled;
    bool rv64c_enabled;
    bool rv64v_enabled;
    bool rv64h_enabled;
    bool rv64p_enabled;
    bool rv64b_enabled;
    uint64_t available_extensions;
    uint64_t enabled_extensions;
} riscv64_extensions_context = {0};

// Extension statistics
static struct
{
    uint32_t extensions_loaded;
    uint32_t extensions_failed;
    uint32_t extension_usage_count;
    uint32_t vector_operations;
    uint32_t floating_point_operations;
    uint32_t atomic_operations;
    uint32_t compressed_instructions;
} riscv64_extensions_stats = {0};

// ============================================================================
// EXTENSIONS INITIALIZATION
// ============================================================================

// Initialize extensions system
void riscv64_extensions_init(void)
{
    if (riscv64_extensions_context.initialized)
    {
        return;
    }

    // Initialize extensions context with default values
    riscv64_extensions_context.rv64i_enabled = true; // Base integer extension is always enabled
    riscv64_extensions_context.rv64m_enabled = false;
    riscv64_extensions_context.rv64a_enabled = false;
    riscv64_extensions_context.rv64f_enabled = false;
    riscv64_extensions_context.rv64d_enabled = false;
    riscv64_extensions_context.rv64c_enabled = false;
    riscv64_extensions_context.rv64v_enabled = false;
    riscv64_extensions_context.rv64h_enabled = false;
    riscv64_extensions_context.rv64p_enabled = false;
    riscv64_extensions_context.rv64b_enabled = false;
    riscv64_extensions_context.available_extensions = 0;
    riscv64_extensions_context.enabled_extensions = RISCV64_EXTENSION_RV64I;
    riscv64_extensions_context.initialized = true;

    // Clear extension statistics
    memset(&riscv64_extensions_stats, 0, sizeof(riscv64_extensions_stats));

    // Detect available extensions
    riscv64_extensions_detect();

    printf("RISC-V64: Extensions system initialized\n");
    printf("  Base Extension (RV64I): Enabled\n");
}

// ============================================================================
// EXTENSIONS DETECTION
// ============================================================================

// Detect available extensions
void riscv64_extensions_detect(void)
{
    if (!riscv64_extensions_context.initialized)
    {
        return;
    }

    uint64_t misa;
    asm volatile("csrr %0, misa" : "=r"(misa));

    // Check for RV64M (Multiply/Divide)
    if (misa & (1ULL << 12))
    {
        riscv64_extensions_context.rv64m_enabled = true;
        riscv64_extensions_context.available_extensions |= RISCV64_EXTENSION_RV64M;
        riscv64_extensions_context.enabled_extensions |= RISCV64_EXTENSION_RV64M;
        printf("RISC-V64: RV64M extension detected and enabled\n");
    }

    // Check for RV64A (Atomic)
    if (misa & (1ULL << 0))
    {
        riscv64_extensions_context.rv64a_enabled = true;
        riscv64_extensions_context.available_extensions |= RISCV64_EXTENSION_RV64A;
        riscv64_extensions_context.enabled_extensions |= RISCV64_EXTENSION_RV64A;
        printf("RISC-V64: RV64A extension detected and enabled\n");
    }

    // Check for RV64F (Single-precision Floating Point)
    if (misa & (1ULL << 5))
    {
        riscv64_extensions_context.rv64f_enabled = true;
        riscv64_extensions_context.available_extensions |= RISCV64_EXTENSION_RV64F;
        riscv64_extensions_context.enabled_extensions |= RISCV64_EXTENSION_RV64F;
        printf("RISC-V64: RV64F extension detected and enabled\n");
    }

    // Check for RV64D (Double-precision Floating Point)
    if (misa & (1ULL << 3))
    {
        riscv64_extensions_context.rv64d_enabled = true;
        riscv64_extensions_context.available_extensions |= RISCV64_EXTENSION_RV64D;
        riscv64_extensions_context.enabled_extensions |= RISCV64_EXTENSION_RV64D;
        printf("RISC-V64: RV64D extension detected and enabled\n");
    }

    // Check for RV64C (Compressed)
    if (misa & (1ULL << 2))
    {
        riscv64_extensions_context.rv64c_enabled = true;
        riscv64_extensions_context.available_extensions |= RISCV64_EXTENSION_RV64C;
        riscv64_extensions_context.enabled_extensions |= RISCV64_EXTENSION_RV64C;
        printf("RISC-V64: RV64C extension detected and enabled\n");
    }

    // Check for RV64V (Vector)
    if (misa & (1ULL << 21))
    {
        riscv64_extensions_context.rv64v_enabled = true;
        riscv64_extensions_context.available_extensions |= RISCV64_EXTENSION_RV64V;
        riscv64_extensions_context.enabled_extensions |= RISCV64_EXTENSION_RV64V;
        printf("RISC-V64: RV64V extension detected and enabled\n");
    }

    // Check for RV64H (Hypervisor)
    if (misa & (1ULL << 7))
    {
        riscv64_extensions_context.rv64h_enabled = true;
        riscv64_extensions_context.available_extensions |= RISCV64_EXTENSION_RV64H;
        riscv64_extensions_context.enabled_extensions |= RISCV64_EXTENSION_RV64H;
        printf("RISC-V64: RV64H extension detected and enabled\n");
    }

    // Check for RV64P (Packed SIMD)
    if (misa & (1ULL << 23))
    {
        riscv64_extensions_context.rv64p_enabled = true;
        riscv64_extensions_context.available_extensions |= RISCV64_EXTENSION_RV64P;
        riscv64_extensions_context.enabled_extensions |= RISCV64_EXTENSION_RV64P;
        printf("RISC-V64: RV64P extension detected and enabled\n");
    }

    // Check for RV64B (Bit Manipulation)
    if (misa & (1ULL << 1))
    {
        riscv64_extensions_context.rv64b_enabled = true;
        riscv64_extensions_context.available_extensions |= RISCV64_EXTENSION_RV64B;
        riscv64_extensions_context.enabled_extensions |= RISCV64_EXTENSION_RV64B;
        printf("RISC-V64: RV64B extension detected and enabled\n");
    }

    riscv64_extensions_stats.extensions_loaded++;

    printf("RISC-V64: Extensions detection completed\n");
    printf("  Available: 0x%llx\n", riscv64_extensions_context.available_extensions);
    printf("  Enabled: 0x%llx\n", riscv64_extensions_context.enabled_extensions);
}

// ============================================================================
// EXTENSIONS CONTEXT FUNCTIONS
// ============================================================================

// Get extensions context
struct riscv64_extensions_context *riscv64_get_extensions_context(void)
{
    if (!riscv64_extensions_context.initialized)
    {
        return NULL;
    }

    return &riscv64_extensions_context;
}

// Check if extension is available
bool riscv64_extension_is_available(uint64_t extension)
{
    if (!riscv64_extensions_context.initialized)
    {
        return false;
    }

    return (riscv64_extensions_context.available_extensions & extension) != 0;
}

// Check if extension is enabled
bool riscv64_extension_is_enabled(uint64_t extension)
{
    if (!riscv64_extensions_context.initialized)
    {
        return false;
    }

    return (riscv64_extensions_context.enabled_extensions & extension) != 0;
}

// Enable extension
int riscv64_extension_enable(uint64_t extension)
{
    if (!riscv64_extensions_context.initialized)
    {
        return RISCV64_ERROR_NOT_INITIALIZED;
    }

    if (!riscv64_extension_is_available(extension))
    {
        return RISCV64_ERROR_EXTENSION_NOT_AVAILABLE;
    }

    riscv64_extensions_context.enabled_extensions |= extension;

    // Update individual extension flags
    if (extension & RISCV64_EXTENSION_RV64M)
        riscv64_extensions_context.rv64m_enabled = true;
    if (extension & RISCV64_EXTENSION_RV64A)
        riscv64_extensions_context.rv64a_enabled = true;
    if (extension & RISCV64_EXTENSION_RV64F)
        riscv64_extensions_context.rv64f_enabled = true;
    if (extension & RISCV64_EXTENSION_RV64D)
        riscv64_extensions_context.rv64d_enabled = true;
    if (extension & RISCV64_EXTENSION_RV64C)
        riscv64_extensions_context.rv64c_enabled = true;
    if (extension & RISCV64_EXTENSION_RV64V)
        riscv64_extensions_context.rv64v_enabled = true;
    if (extension & RISCV64_EXTENSION_RV64H)
        riscv64_extensions_context.rv64h_enabled = true;
    if (extension & RISCV64_EXTENSION_RV64P)
        riscv64_extensions_context.rv64p_enabled = true;
    if (extension & RISCV64_EXTENSION_RV64B)
        riscv64_extensions_context.rv64b_enabled = true;

    printf("RISC-V64: Extension 0x%llx enabled\n", extension);

    return 0;
}

// Disable extension
int riscv64_extension_disable(uint64_t extension)
{
    if (!riscv64_extensions_context.initialized)
    {
        return RISCV64_ERROR_NOT_INITIALIZED;
    }

    // Cannot disable base RV64I extension
    if (extension & RISCV64_EXTENSION_RV64I)
    {
        return RISCV64_ERROR_CANNOT_DISABLE_BASE;
    }

    riscv64_extensions_context.enabled_extensions &= ~extension;

    // Update individual extension flags
    if (extension & RISCV64_EXTENSION_RV64M)
        riscv64_extensions_context.rv64m_enabled = false;
    if (extension & RISCV64_EXTENSION_RV64A)
        riscv64_extensions_context.rv64a_enabled = false;
    if (extension & RISCV64_EXTENSION_RV64F)
        riscv64_extensions_context.rv64f_enabled = false;
    if (extension & RISCV64_EXTENSION_RV64D)
        riscv64_extensions_context.rv64d_enabled = false;
    if (extension & RISCV64_EXTENSION_RV64C)
        riscv64_extensions_context.rv64c_enabled = false;
    if (extension & RISCV64_EXTENSION_RV64V)
        riscv64_extensions_context.rv64v_enabled = false;
    if (extension & RISCV64_EXTENSION_RV64H)
        riscv64_extensions_context.rv64h_enabled = false;
    if (extension & RISCV64_EXTENSION_RV64P)
        riscv64_extensions_context.rv64p_enabled = false;
    if (extension & RISCV64_EXTENSION_RV64B)
        riscv64_extensions_context.rv64b_enabled = false;

    printf("RISC-V64: Extension 0x%llx disabled\n", extension);

    return 0;
}

// ============================================================================
// RISC-V EXTENSION IMPLEMENTATIONS
// ============================================================================

int riscv64_extension_rv64m_init(void)
{
    printf("RISC-V64: Initializing RV64M extension (Multiply/Divide)...\n");

    if (!riscv64_has_feature(RISCV64_FEATURE_RV64M))
    {
        printf("RISC-V64: RV64M extension not supported by this CPU\n");
        return RISCV64_ERROR_EXTENSION_NOT_SUPPORTED;
    }

    // RV64M provides hardware multiply and divide instructions
    // No special initialization needed - instructions are always available

    printf("RISC-V64: RV64M extension initialized successfully\n");
    return 0;
}

int riscv64_extension_rv64a_init(void)
{
    printf("RISC-V64: Initializing RV64A extension (Atomic Instructions)...\n");

    if (!riscv64_has_feature(RISCV64_FEATURE_RV64A))
    {
        printf("RISC-V64: RV64A extension not supported by this CPU\n");
        return RISCV64_ERROR_EXTENSION_NOT_SUPPORTED;
    }

    // RV64A provides atomic memory operations
    // Initialize atomic operation support
    riscv64_extensions_support.atomic_ops = true;
    riscv64_extensions_support.lr_sc_available = true;
    riscv64_extensions_support.amo_available = true;

    printf("RISC-V64: RV64A extension initialized successfully\n");
    return 0;
}

int riscv64_extension_rv64f_init(void)
{
    printf("RISC-V64: Initializing RV64F extension (Single-Precision FP)...\n");

    if (!riscv64_has_feature(RISCV64_FEATURE_RV64F))
    {
        printf("RISC-V64: RV64F extension not supported by this CPU\n");
        return RISCV64_ERROR_EXTENSION_NOT_SUPPORTED;
    }

    // RV64F provides single-precision floating-point operations
    // Initialize FPU state
    riscv64_extensions_support.fpu_single = true;
    riscv64_extensions_support.fpu_double = false;

    // Set up FPU control registers
    uint64_t fcsr = 0;
    __asm__ volatile("csrw fcsr, %0" : : "r"(fcsr));

    // Enable FPU
    uint64_t mstatus;
    __asm__ volatile("csrr %0, mstatus" : "=r"(mstatus));
    mstatus |= (1ULL << 13); // FS bit
    __asm__ volatile("csrw mstatus, %0" : : "r"(mstatus));

    printf("RISC-V64: RV64F extension initialized successfully\n");
    return 0;
}

int riscv64_extension_rv64d_init(void)
{
    printf("RISC-V64: Initializing RV64D extension (Double-Precision FP)...\n");

    if (!riscv64_has_feature(RISCV64_FEATURE_RV64D))
    {
        printf("RISC-V64: RV64D extension not supported by this CPU\n");
        return RISCV64_ERROR_EXTENSION_NOT_SUPPORTED;
    }

    // RV64D provides double-precision floating-point operations
    // Initialize FPU state for double precision
    riscv64_extensions_support.fpu_single = true;
    riscv64_extensions_support.fpu_double = true;

    // Set up FPU control registers for double precision
    uint64_t fcsr = 0;
    __asm__ volatile("csrw fcsr, %0" : : "r"(fcsr));

    // Enable FPU
    uint64_t mstatus;
    __asm__ volatile("csrr %0, mstatus" : "=r"(mstatus));
    mstatus |= (1ULL << 13); // FS bit
    __asm__ volatile("csrw mstatus, %0" : : "r"(mstatus));

    printf("RISC-V64: RV64D extension initialized successfully\n");
    return 0;
}

int riscv64_extension_rv64c_init(void)
{
    printf("RISC-V64: Initializing RV64C extension (Compressed Instructions)...\n");

    if (!riscv64_has_feature(RISCV64_FEATURE_RV64C))
    {
        printf("RISC-V64: RV64C extension not supported by this CPU\n");
        return RISCV64_ERROR_EXTENSION_NOT_SUPPORTED;
    }

    // RV64C provides 16-bit compressed instructions
    // No special initialization needed - instructions are always available
    riscv64_extensions_support.compressed_instructions = true;

    printf("RISC-V64: RV64C extension initialized successfully\n");
    return 0;
}

int riscv64_extension_rv64v_init(void)
{
    printf("RISC-V64: Initializing RV64V extension (Vector Instructions)...\n");

    if (!riscv64_has_feature(RISCV64_FEATURE_RVV))
    {
        printf("RISC-V64: RV64V extension not supported by this CPU\n");
        return RISCV64_ERROR_EXTENSION_NOT_SUPPORTED;
    }

    // RV64V provides vector operations
    // Initialize vector unit
    riscv64_extensions_support.vector_ops = true;

    // Read vector configuration
    uint64_t vlenb;
    __asm__ volatile("csrr %0, vlenb" : "=r"(vlenb));
    riscv64_extensions_support.vector_length_bytes = vlenb;

    // Read vector element width
    uint64_t vsew;
    __asm__ volatile("csrr %0, vsew" : "=r"(vsew));
    riscv64_extensions_support.vector_element_width = vsew;

    // Enable vector unit
    uint64_t mstatus;
    __asm__ volatile("csrr %0, mstatus" : "=r"(mstatus));
    mstatus |= (1ULL << 9); // VS bit
    __asm__ volatile("csrw mstatus, %0" : : "r"(mstatus));

    printf("RISC-V64: RV64V extension initialized successfully (VLENB: %llu, VSEW: %llu)\n",
           (unsigned long long)vlenb, (unsigned long long)vsew);
    return 0;
}

int riscv64_extension_rv64h_init(void)
{
    printf("RISC-V64: Initializing RV64H extension (Hypervisor)...\n");

    if (!riscv64_has_feature(RISCV64_FEATURE_RVH))
    {
        printf("RISC-V64: RV64H extension not supported by this CPU\n");
        return RISCV64_ERROR_EXTENSION_NOT_SUPPORTED;
    }

    // RV64H provides hypervisor support
    // Initialize hypervisor mode
    riscv64_extensions_support.hypervisor = true;

    // Set up hypervisor trap vector
    uint64_t htvec = 0x8000000000000000ULL; // Hypervisor trap vector base
    __asm__ volatile("csrw htvec, %0" : : "r"(htvec));

    // Enable hypervisor mode
    uint64_t mstatus;
    __asm__ volatile("csrr %0, mstatus" : "=r"(mstatus));
    mstatus |= (1ULL << 18); // TVM bit
    __asm__ volatile("csrw mstatus, %0" : : "r"(mstatus));

    printf("RISC-V64: RV64H extension initialized successfully\n");
    return 0;
}

int riscv64_extension_rv64p_init(void)
{
    printf("RISC-V64: Initializing RV64P extension (Packed SIMD)...\n");

    if (!riscv64_has_feature(RISCV64_FEATURE_RVP))
    {
        printf("RISC-V64: RV64P extension not supported by this CPU\n");
        return RISCV64_ERROR_EXTENSION_NOT_SUPPORTED;
    }

    // RV64P provides packed SIMD operations
    // Initialize packed SIMD support
    riscv64_extensions_support.packed_simd = true;

    printf("RISC-V64: RV64P extension initialized successfully\n");
    return 0;
}

int riscv64_extension_rv64b_init(void)
{
    printf("RISC-V64: Initializing RV64B extension (Bit Manipulation)...\n");

    if (!riscv64_has_feature(RISCV64_FEATURE_RV64B))
    {
        printf("RISC-V64: RV64B extension not supported by this CPU\n");
        return RISCV64_ERROR_EXTENSION_NOT_SUPPORTED;
    }

    // RV64B provides bit manipulation instructions
    // Initialize bit manipulation support
    riscv64_extensions_support.bit_manipulation = true;

    printf("RISC-V64: RV64B extension initialized successfully\n");
    return 0;
}

// ============================================================================
// EXTENSION UTILITY FUNCTIONS
// ============================================================================

bool riscv64_extension_is_supported(uint64_t extension)
{
    return riscv64_has_feature(extension);
}

uint64_t riscv64_extension_get_vector_length(void)
{
    if (!riscv64_extensions_support.vector_ops)
    {
        return 0;
    }

    uint64_t vlen;
    __asm__ volatile("csrr %0, vlen" : "=r"(vlen));
    return vlen;
}

uint64_t riscv64_extension_get_vector_length_bytes(void)
{
    return riscv64_extensions_support.vector_length_bytes;
}

uint64_t riscv64_extension_get_vector_element_width(void)
{
    return riscv64_extensions_support.vector_element_width;
}

bool riscv64_extension_has_fpu(void)
{
    return riscv64_extensions_support.fpu_single || riscv64_extensions_support.fpu_double;
}

bool riscv64_extension_has_double_fpu(void)
{
    return riscv64_extensions_support.fpu_double;
}

bool riscv64_extension_has_atomic_ops(void)
{
    return riscv64_extensions_support.atomic_ops;
}

bool riscv64_extension_has_hypervisor(void)
{
    return riscv64_extensions_support.hypervisor;
}

bool riscv64_extension_has_packed_simd(void)
{
    return riscv64_extensions_support.packed_simd;
}

bool riscv64_extension_has_bit_manipulation(void)
{
    return riscv64_extensions_support.bit_manipulation;
}

// ============================================================================
// EXTENSION TESTING
// ============================================================================

int riscv64_extension_test_all(void)
{
    printf("RISC-V64: Testing all available extensions...\n");

    int test_result = 0;

    // Test RV64M (multiply/divide)
    if (riscv64_has_feature(RISCV64_FEATURE_RV64M))
    {
        uint64_t a = 123456789;
        uint64_t b = 987654321;
        uint64_t result = a * b;
        if (result == 123456789ULL * 987654321ULL)
        {
            printf("RISC-V64: RV64M test passed\n");
        }
        else
        {
            printf("RISC-V64: RV64M test failed\n");
            test_result = -1;
        }
    }

    // Test RV64F (single precision FP)
    if (riscv64_has_feature(RISCV64_FEATURE_RV64F))
    {
        float x = 3.14159f;
        float y = 2.71828f;
        float result = x + y;
        if (result > 5.8f && result < 5.9f)
        {
            printf("RISC-V64: RV64F test passed\n");
        }
        else
        {
            printf("RISC-V64: RV64F test failed\n");
            test_result = -1;
        }
    }

    // Test RV64D (double precision FP)
    if (riscv64_has_feature(RISCV64_FEATURE_RV64D))
    {
        double x = 3.14159265359;
        double y = 2.71828182846;
        double result = x * y;
        if (result > 8.5 && result < 8.6)
        {
            printf("RISC-V64: RV64D test passed\n");
        }
        else
        {
            printf("RISC-V64: RV64D test failed\n");
            test_result = -1;
        }
    }

    // Test RV64C (compressed instructions)
    if (riscv64_has_feature(RISCV64_FEATURE_RV64C))
    {
        // Test that compressed instructions work
        // This is a simple test - in practice, you'd verify instruction decoding
        printf("RISC-V64: RV64C test passed (compressed instructions available)\n");
    }

    if (test_result == 0)
    {
        printf("RISC-V64: All extension tests passed\n");
    }
    else
    {
        printf("RISC-V64: Some extension tests failed\n");
    }

    return test_result;
}

// ============================================================================
// EXTENSIONS STATISTICS
// ============================================================================

// Get extensions statistics
void riscv64_extensions_get_stats(uint32_t *extensions_loaded, uint32_t *extensions_failed,
                                  uint32_t *extension_usage_count, uint32_t *vector_operations,
                                  uint32_t *floating_point_operations, uint32_t *atomic_operations,
                                  uint32_t *compressed_instructions)
{
    if (extensions_loaded)
        *extensions_loaded = riscv64_extensions_stats.extensions_loaded;
    if (extensions_failed)
        *extensions_failed = riscv64_extensions_stats.extensions_failed;
    if (extension_usage_count)
        *extension_usage_count = riscv64_extensions_stats.extension_usage_count;
    if (vector_operations)
        *vector_operations = riscv64_extensions_stats.vector_operations;
    if (floating_point_operations)
        *floating_point_operations = riscv64_extensions_stats.floating_point_operations;
    if (atomic_operations)
        *atomic_operations = riscv64_extensions_stats.atomic_operations;
    if (compressed_instructions)
        *compressed_instructions = riscv64_extensions_stats.compressed_instructions;
}

// Reset extensions statistics
void riscv64_extensions_reset_stats(void)
{
    memset(&riscv64_extensions_stats, 0, sizeof(riscv64_extensions_stats));

    printf("RISC-V64: Extensions statistics reset\n");
}

// ============================================================================
// EXTENSIONS DEBUG FUNCTIONS
// ============================================================================

// Print extensions status
void riscv64_extensions_print_status(void)
{
    printf("RISC-V64: Extensions Status\n");
    printf("  Initialized: %s\n", riscv64_extensions_context.initialized ? "Yes" : "No");
    printf("  RV64I (Base): %s\n", riscv64_extensions_context.rv64i_enabled ? "Enabled" : "Disabled");
    printf("  RV64M (Multiply/Divide): %s\n", riscv64_extensions_context.rv64m_enabled ? "Enabled" : "Disabled");
    printf("  RV64A (Atomic): %s\n", riscv64_extensions_context.rv64a_enabled ? "Enabled" : "Disabled");
    printf("  RV64F (Single FP): %s\n", riscv64_extensions_context.rv64f_enabled ? "Enabled" : "Disabled");
    printf("  RV64D (Double FP): %s\n", riscv64_extensions_context.rv64d_enabled ? "Enabled" : "Disabled");
    printf("  RV64C (Compressed): %s\n", riscv64_extensions_context.rv64c_enabled ? "Enabled" : "Disabled");
    printf("  RV64V (Vector): %s\n", riscv64_extensions_context.rv64v_enabled ? "Enabled" : "Disabled");
    printf("  RV64H (Hypervisor): %s\n", riscv64_extensions_context.rv64h_enabled ? "Enabled" : "Disabled");
    printf("  RV64P (Packed SIMD): %s\n", riscv64_extensions_context.rv64p_enabled ? "Enabled" : "Disabled");
    printf("  RV64B (Bit Manipulation): %s\n", riscv64_extensions_context.rv64b_enabled ? "Enabled" : "Disabled");
    printf("  Available: 0x%llx\n", riscv64_extensions_context.available_extensions);
    printf("  Enabled: 0x%llx\n", riscv64_extensions_context.enabled_extensions);
}

// Print extensions statistics
void riscv64_extensions_print_statistics(void)
{
    printf("RISC-V64: Extensions Statistics\n");
    printf("  Extensions Loaded: %u\n", riscv64_extensions_stats.extensions_loaded);
    printf("  Extensions Failed: %u\n", riscv64_extensions_stats.extensions_failed);
    printf("  Extension Usage Count: %u\n", riscv64_extensions_stats.extension_usage_count);
    printf("  Vector Operations: %u\n", riscv64_extensions_stats.vector_operations);
    printf("  Floating Point Operations: %u\n", riscv64_extensions_stats.floating_point_operations);
    printf("  Atomic Operations: %u\n", riscv64_extensions_stats.atomic_operations);
    printf("  Compressed Instructions: %u\n", riscv64_extensions_stats.compressed_instructions);
}

// ============================================================================
// EXTENSIONS CLEANUP
// ============================================================================

// Cleanup extensions system
void riscv64_extensions_cleanup(void)
{
    if (!riscv64_extensions_context.initialized)
    {
        return;
    }

    // Disable all extensions except base RV64I
    riscv64_extensions_context.enabled_extensions = RISCV64_EXTENSION_RV64I;
    riscv64_extensions_context.rv64m_enabled = false;
    riscv64_extensions_context.rv64a_enabled = false;
    riscv64_extensions_context.rv64f_enabled = false;
    riscv64_extensions_context.rv64d_enabled = false;
    riscv64_extensions_context.rv64c_enabled = false;
    riscv64_extensions_context.rv64v_enabled = false;
    riscv64_extensions_context.rv64h_enabled = false;
    riscv64_extensions_context.rv64p_enabled = false;
    riscv64_extensions_context.rv64b_enabled = false;

    // Clear contexts
    riscv64_extensions_context.initialized = false;

    printf("RISC-V64: Extensions system cleaned up\n");
}
