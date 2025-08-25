/*
 * ORION OS - MIPS Extensions Management
 *
 * Extensions management for MIPS architecture
 * Handles NEON, VFP, crypto, SIMD, and other extensions
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include "arch.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// EXTENSIONS CONTEXT MANAGEMENT
// ============================================================================

static mips_extension_context_t mips_extension_context = {0};

void mips_extensions_init(void)
{
    memset(&mips_extension_context, 0, sizeof(mips_extension_context));

    // Set default extension settings
    mips_extension_context.extensions_enabled = false;
    mips_extension_context.supported_extensions = 0;
    mips_extension_context.active_extensions = 0;

    printf("MIPS: Extensions context initialized\n");
}

mips_extension_context_t *mips_extensions_get_context(void)
{
    return &mips_extension_context;
}

// ============================================================================
// EXTENSION DETECTION
// ============================================================================

void mips_extensions_detect(void)
{
    uint64_t config0 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG);
    uint64_t config1 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 1);
    uint64_t config2 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 2);
    uint64_t config3 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 3);

    // Detect supported extensions
    if (config1 & MIPS_CONFIG1_FP)
    {
        mips_extension_context.supported_extensions |= MIPS_EXTENSION_VFP;
    }

    if (config1 & MIPS_CONFIG1_DSP)
    {
        mips_extension_context.supported_extensions |= MIPS_EXTENSION_DSP;
    }

    if (config1 & MIPS_CONFIG1_MSA)
    {
        mips_extension_context.supported_extensions |= MIPS_EXTENSION_MSA;
    }

    if (config1 & MIPS_CONFIG1_VZ)
    {
        mips_extension_context.supported_extensions |= MIPS_EXTENSION_VZ;
    }

    if (config1 & MIPS_CONFIG1_CRC)
    {
        mips_extension_context.supported_extensions |= MIPS_EXTENSION_CRC;
    }

    if (config1 & MIPS_CONFIG1_GINV)
    {
        mips_extension_context.supported_extensions |= MIPS_EXTENSION_GINV;
    }

    if (config2 & MIPS_CONFIG2_NEON)
    {
        mips_extension_context.supported_extensions |= MIPS_EXTENSION_NEON;
    }

    if (config3 & MIPS_CONFIG3_CRYPTO)
    {
        mips_extension_context.supported_extensions |= MIPS_EXTENSION_CRYPTO;
    }

    if (config3 & MIPS_CONFIG3_SIMD)
    {
        mips_extension_context.supported_extensions |= MIPS_EXTENSION_SIMD;
    }

    printf("MIPS: Extensions detected: 0x%x\n", mips_extension_context.supported_extensions);
}

bool mips_extensions_supports(mips_extension_t extension)
{
    return (mips_extension_context.supported_extensions & (1 << extension)) != 0;
}

uint32_t mips_extensions_get_supported(void)
{
    return mips_extension_context.supported_extensions;
}

// ============================================================================
// EXTENSION ENABLING/DISABLING
// ============================================================================

void mips_extensions_enable_all(void)
{
    if (!mips_extension_context.extensions_enabled)
    {
        mips_extension_context.extensions_enabled = true;
        mips_extension_context.active_extensions = mips_extension_context.supported_extensions;

        // Enable all supported extensions
        for (int i = 0; i < 32; i++)
        {
            if (mips_extension_context.supported_extensions & (1 << i))
            {
                mips_extensions_enable_specific((mips_extension_t)i);
            }
        }

        printf("MIPS: All supported extensions enabled\n");
    }
}

void mips_extensions_disable_all(void)
{
    if (mips_extension_context.extensions_enabled)
    {
        // Disable all active extensions
        for (int i = 0; i < 32; i++)
        {
            if (mips_extension_context.active_extensions & (1 << i))
            {
                mips_extensions_disable_specific((mips_extension_t)i);
            }
        }

        mips_extension_context.extensions_enabled = false;
        mips_extension_context.active_extensions = 0;

        printf("MIPS: All extensions disabled\n");
    }
}

void mips_extensions_enable_specific(mips_extension_t extension)
{
    if (!mips_extensions_supports(extension))
    {
        printf("MIPS: Extension %d not supported\n", extension);
        return;
    }

    // Enable specific extension
    switch (extension)
    {
    case MIPS_EXTENSION_NEON:
        mips_extensions_enable_neon();
        break;
    case MIPS_EXTENSION_VFP:
        mips_extensions_enable_vfp();
        break;
    case MIPS_EXTENSION_CRYPTO:
        mips_extensions_enable_crypto();
        break;
    case MIPS_EXTENSION_SIMD:
        mips_extensions_enable_simd();
        break;
    default:
        printf("MIPS: Unknown extension: %d\n", extension);
        return;
    }

    mips_extension_context.active_extensions |= (1 << extension);
    printf("MIPS: Extension %d enabled\n", extension);
}

void mips_extensions_disable_specific(mips_extension_t extension)
{
    if (!mips_extensions_supports(extension))
    {
        return;
    }

    // Disable specific extension
    switch (extension)
    {
    case MIPS_EXTENSION_NEON:
        mips_extensions_disable_neon();
        break;
    case MIPS_EXTENSION_VFP:
        mips_extensions_disable_vfp();
        break;
    case MIPS_EXTENSION_CRYPTO:
        mips_extensions_disable_crypto();
        break;
    case MIPS_EXTENSION_SIMD:
        mips_extensions_disable_simd();
        break;
    default:
        printf("MIPS: Unknown extension: %d\n", extension);
        return;
    }

    mips_extension_context.active_extensions &= ~(1 << extension);
    printf("MIPS: Extension %d disabled\n", extension);
}

bool mips_extensions_is_enabled(mips_extension_t extension)
{
    return (mips_extension_context.active_extensions & (1 << extension)) != 0;
}

uint32_t mips_extensions_get_active(void)
{
    return mips_extension_context.active_extensions;
}

// ============================================================================
// NEON EXTENSION
// ============================================================================

void mips_extensions_enable_neon(void)
{
    if (!mips_extensions_supports(MIPS_EXTENSION_NEON))
    {
        printf("MIPS: NEON extension not supported\n");
        return;
    }

    // Enable NEON in CP0
    uint64_t config2 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 2);
    config2 |= MIPS_CONFIG2_NEON_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_CONFIG + 2, config2);

    printf("MIPS: NEON extension enabled\n");
}

void mips_extensions_disable_neon(void)
{
    if (!mips_extensions_supports(MIPS_EXTENSION_NEON))
    {
        return;
    }

    uint64_t config2 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 2);
    config2 &= ~MIPS_CONFIG2_NEON_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_CONFIG + 2, config2);

    printf("MIPS: NEON extension disabled\n");
}

bool mips_extensions_is_neon_enabled(void)
{
    if (!mips_extensions_supports(MIPS_EXTENSION_NEON))
    {
        return false;
    }

    uint64_t config2 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 2);
    return (config2 & MIPS_CONFIG2_NEON_ENABLE) != 0;
}

// ============================================================================
// VFP EXTENSION
// ============================================================================

void mips_extensions_enable_vfp(void)
{
    if (!mips_extensions_supports(MIPS_EXTENSION_VFP))
    {
        printf("MIPS: VFP extension not supported\n");
        return;
    }

    // Enable VFP in CP0
    uint64_t config1 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 1);
    config1 |= MIPS_CONFIG1_FP_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_CONFIG + 1, config1);

    printf("MIPS: VFP extension enabled\n");
}

void mips_extensions_disable_vfp(void)
{
    if (!mips_extensions_supports(MIPS_EXTENSION_VFP))
    {
        return;
    }

    uint64_t config1 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 1);
    config1 &= ~MIPS_CONFIG1_FP_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_CONFIG + 1, config1);

    printf("MIPS: VFP extension disabled\n");
}

bool mips_extensions_is_vfp_enabled(void)
{
    if (!mips_extensions_supports(MIPS_EXTENSION_VFP))
    {
        return false;
    }

    uint64_t config1 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 1);
    return (config1 & MIPS_CONFIG1_FP_ENABLE) != 0;
}

// ============================================================================
// CRYPTO EXTENSION
// ============================================================================

void mips_extensions_enable_crypto(void)
{
    if (!mips_extensions_supports(MIPS_EXTENSION_CRYPTO))
    {
        printf("MIPS: Crypto extension not supported\n");
        return;
    }

    // Enable crypto in CP0
    uint64_t config3 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 3);
    config3 |= MIPS_CONFIG3_CRYPTO_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_CONFIG + 3, config3);

    printf("MIPS: Crypto extension enabled\n");
}

void mips_extensions_disable_crypto(void)
{
    if (!mips_extensions_supports(MIPS_EXTENSION_CRYPTO))
    {
        return;
    }

    uint64_t config3 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 3);
    config3 &= ~MIPS_CONFIG3_CRYPTO_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_CONFIG + 3, config3);

    printf("MIPS: Crypto extension disabled\n");
}

bool mips_extensions_is_crypto_enabled(void)
{
    if (!mips_extensions_supports(MIPS_EXTENSION_CRYPTO))
    {
        return false;
    }

    uint64_t config3 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 3);
    return (config3 & MIPS_CONFIG3_CRYPTO_ENABLE) != 0;
}

// ============================================================================
// SIMD EXTENSION
// ============================================================================

void mips_extensions_enable_simd(void)
{
    if (!mips_extensions_supports(MIPS_EXTENSION_SIMD))
    {
        printf("MIPS: SIMD extension not supported\n");
        return;
    }

    // Enable SIMD in CP0
    uint64_t config3 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 3);
    config3 |= MIPS_CONFIG3_SIMD_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_CONFIG + 3, config3);

    printf("MIPS: SIMD extension enabled\n");
}

void mips_extensions_disable_simd(void)
{
    if (!mips_extensions_supports(MIPS_EXTENSION_SIMD))
    {
        return;
    }

    uint64_t config3 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 3);
    config3 &= ~MIPS_CONFIG3_SIMD_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_CONFIG + 3, config3);

    printf("MIPS: SIMD extension disabled\n");
}

bool mips_extensions_is_simd_enabled(void)
{
    if (!mips_extensions_supports(MIPS_EXTENSION_SIMD))
    {
        return false;
    }

    uint64_t config3 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 3);
    return (config3 & MIPS_CONFIG3_SIMD_ENABLE) != 0;
}

// ============================================================================
// DSP EXTENSION
// ============================================================================

void mips_extensions_enable_dsp(void)
{
    if (!mips_extensions_supports(MIPS_EXTENSION_DSP))
    {
        printf("MIPS: DSP extension not supported\n");
        return;
    }

    // Enable DSP in CP0
    uint64_t config1 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 1);
    config1 |= MIPS_CONFIG1_DSP_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_CONFIG + 1, config1);

    printf("MIPS: DSP extension enabled\n");
}

void mips_extensions_disable_dsp(void)
{
    if (!mips_extensions_supports(MIPS_EXTENSION_DSP))
    {
        return;
    }

    uint64_t config1 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 1);
    config1 &= ~MIPS_CONFIG1_DSP_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_CONFIG + 1, config1);

    printf("MIPS: DSP extension disabled\n");
}

bool mips_extensions_is_dsp_enabled(void)
{
    if (!mips_extensions_supports(MIPS_EXTENSION_DSP))
    {
        return false;
    }

    uint64_t config1 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 1);
    return (config1 & MIPS_CONFIG1_DSP_ENABLE) != 0;
}

// ============================================================================
// MSA EXTENSION
// ============================================================================

void mips_extensions_enable_msa(void)
{
    if (!mips_extensions_supports(MIPS_EXTENSION_MSA))
    {
        printf("MIPS: MSA extension not supported\n");
        return;
    }

    // Enable MSA in CP0
    uint64_t config1 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 1);
    config1 |= MIPS_CONFIG1_MSA_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_CONFIG + 1, config1);

    printf("MIPS: MSA extension enabled\n");
}

void mips_extensions_disable_msa(void)
{
    if (!mips_extensions_supports(MIPS_EXTENSION_MSA))
    {
        return;
    }

    uint64_t config1 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 1);
    config1 &= ~MIPS_CONFIG1_MSA_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_CONFIG + 1, config1);

    printf("MIPS: MSA extension disabled\n");
}

bool mips_extensions_is_msa_enabled(void)
{
    if (!mips_extensions_supports(MIPS_EXTENSION_MSA))
    {
        return false;
    }

    uint64_t config1 = mips_cpu_read_cp0_register(MIPS_CP0_CONFIG + 1);
    return (config1 & MIPS_CONFIG1_MSA_ENABLE) != 0;
}

// ============================================================================
// EXTENSION UTILITIES
// ============================================================================

void mips_extensions_print_supported(void)
{
    printf("MIPS: Supported Extensions:\n");
    printf("  NEON: %s\n", mips_extensions_supports(MIPS_EXTENSION_NEON) ? "yes" : "no");
    printf("  VFP: %s\n", mips_extensions_supports(MIPS_EXTENSION_VFP) ? "yes" : "no");
    printf("  Crypto: %s\n", mips_extensions_supports(MIPS_EXTENSION_CRYPTO) ? "yes" : "no");
    printf("  SIMD: %s\n", mips_extensions_supports(MIPS_EXTENSION_SIMD) ? "yes" : "no");
    printf("  DSP: %s\n", mips_extensions_supports(MIPS_EXTENSION_DSP) ? "yes" : "no");
    printf("  MSA: %s\n", mips_extensions_supports(MIPS_EXTENSION_MSA) ? "yes" : "no");
    printf("  VZ: %s\n", mips_extensions_supports(MIPS_EXTENSION_VZ) ? "yes" : "no");
    printf("  CRC: %s\n", mips_extensions_supports(MIPS_EXTENSION_CRC) ? "yes" : "no");
    printf("  GINV: %s\n", mips_extensions_supports(MIPS_EXTENSION_GINV) ? "yes" : "no");
}

void mips_extensions_print_active(void)
{
    printf("MIPS: Active Extensions:\n");
    printf("  NEON: %s\n", mips_extensions_is_enabled(MIPS_EXTENSION_NEON) ? "enabled" : "disabled");
    printf("  VFP: %s\n", mips_extensions_is_enabled(MIPS_EXTENSION_VFP) ? "enabled" : "disabled");
    printf("  Crypto: %s\n", mips_extensions_is_enabled(MIPS_EXTENSION_CRYPTO) ? "enabled" : "disabled");
    printf("  SIMD: %s\n", mips_extensions_is_enabled(MIPS_EXTENSION_SIMD) ? "enabled" : "disabled");
    printf("  DSP: %s\n", mips_extensions_is_enabled(MIPS_EXTENSION_DSP) ? "enabled" : "disabled");
    printf("  MSA: %s\n", mips_extensions_is_enabled(MIPS_EXTENSION_MSA) ? "enabled" : "disabled");
}

void mips_extensions_print_status(void)
{
    printf("MIPS: Extensions Status:\n");
    printf("  Extensions Enabled: %s\n", mips_extension_context.extensions_enabled ? "yes" : "no");
    printf("  Supported Extensions: 0x%x\n", mips_extension_context.supported_extensions);
    printf("  Active Extensions: 0x%x\n", mips_extension_context.active_extensions);

    mips_extensions_print_supported();
    mips_extensions_print_active();
}

// ============================================================================
// EXTENSION FEATURE CHECKS
// ============================================================================

bool mips_extensions_has_neon(void)
{
    return mips_extensions_supports(MIPS_EXTENSION_NEON) && mips_extensions_is_enabled(MIPS_EXTENSION_NEON);
}

bool mips_extensions_has_vfp(void)
{
    return mips_extensions_supports(MIPS_EXTENSION_VFP) && mips_extensions_is_enabled(MIPS_EXTENSION_VFP);
}

bool mips_extensions_has_crypto(void)
{
    return mips_extensions_supports(MIPS_EXTENSION_CRYPTO) && mips_extensions_is_enabled(MIPS_EXTENSION_CRYPTO);
}

bool mips_extensions_has_simd(void)
{
    return mips_extensions_supports(MIPS_EXTENSION_SIMD) && mips_extensions_is_enabled(MIPS_EXTENSION_SIMD);
}

bool mips_extensions_has_dsp(void)
{
    return mips_extensions_supports(MIPS_EXTENSION_DSP) && mips_extensions_is_enabled(MIPS_EXTENSION_DSP);
}

bool mips_extensions_has_msa(void)
{
    return mips_extensions_supports(MIPS_EXTENSION_MSA) && mips_extensions_is_enabled(MIPS_EXTENSION_MSA);
}

// ============================================================================
// EXTENSION INITIALIZATION
// ============================================================================

void mips_extensions_initialize_defaults(void)
{
    // Initialize extensions system
    mips_extensions_init();

    // Detect available extensions
    mips_extensions_detect();

    // Enable commonly used extensions by default
    if (mips_extensions_supports(MIPS_EXTENSION_VFP))
    {
        mips_extensions_enable_vfp();
    }

    if (mips_extensions_supports(MIPS_EXTENSION_DSP))
    {
        mips_extensions_enable_dsp();
    }

    if (mips_extensions_supports(MIPS_EXTENSION_CRC))
    {
        mips_extensions_enable_specific(MIPS_EXTENSION_CRC);
    }

    printf("MIPS: Extensions initialized with defaults\n");
}
