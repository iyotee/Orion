/*
 * ORION OS - MIPS Features Management
 *
 * Features management for MIPS architecture
 * Handles MIPS extensions, advanced features, and capability detection
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
// FEATURES CONTEXT MANAGEMENT
// ============================================================================

static mips_features_context_t mips_features_context = {0};

void mips_features_init(void)
{
    memset(&mips_features_context, 0, sizeof(mips_features_context));

    // Initialize features context
    mips_features_context.features_initialized = false;
    mips_features_context.extension_count = 0;
    mips_features_context.advanced_features_enabled = false;
    mips_features_context.optimization_level = MIPS_OPTIMIZATION_LEVEL_BALANCED;
    mips_features_context.security_features_enabled = true;
    mips_features_context.virtualization_features_enabled = false;

    // Initialize extension support
    for (int i = 0; i < MIPS_MAX_EXTENSIONS; i++)
    {
        mips_features_context.extension_support[i] = false;
        mips_features_context.extension_enabled[i] = false;
    }

    printf("MIPS: Features management initialized\n");
}

mips_features_context_t *mips_features_get_context(void)
{
    return &mips_features_context;
}

// ============================================================================
// MIPS EXTENSIONS DETECTION
// ============================================================================

void mips_features_detect_extensions(void)
{
    printf("MIPS: Detecting MIPS extensions...\n");

    // Detect MIPS16
    mips_features_context.extension_support[MIPS_EXTENSION_MIPS16] = mips_features_detect_mips16();
    printf("MIPS: MIPS16 support: %s\n",
           mips_features_context.extension_support[MIPS_EXTENSION_MIPS16] ? "yes" : "no");

    // Detect microMIPS
    mips_features_context.extension_support[MIPS_EXTENSION_MICROMIPS] = mips_features_detect_micromips();
    printf("MIPS: microMIPS support: %s\n",
           mips_features_context.extension_support[MIPS_EXTENSION_MICROMIPS] ? "yes" : "no");

    // Detect DSP
    mips_features_context.extension_support[MIPS_EXTENSION_DSP] = mips_features_detect_dsp();
    printf("MIPS: DSP support: %s\n",
           mips_features_context.extension_support[MIPS_EXTENSION_DSP] ? "yes" : "no");

    // Detect MSA
    mips_features_context.extension_support[MIPS_EXTENSION_MSA] = mips_features_detect_msa();
    printf("MIPS: MSA support: %s\n",
           mips_features_context.extension_support[MIPS_EXTENSION_MSA] ? "yes" : "no");

    // Detect VZ (Virtualization)
    mips_features_context.extension_support[MIPS_EXTENSION_VZ] = mips_features_detect_vz();
    printf("MIPS: VZ support: %s\n",
           mips_features_context.extension_support[MIPS_EXTENSION_VZ] ? "yes" : "no");

    // Detect CRC
    mips_features_context.extension_support[MIPS_EXTENSION_CRC] = mips_features_detect_crc();
    printf("MIPS: CRC support: %s\n",
           mips_features_context.extension_support[MIPS_EXTENSION_CRC] ? "yes" : "no");

    // Detect GINV
    mips_features_context.extension_support[MIPS_EXTENSION_GINV] = mips_features_detect_ginv();
    printf("MIPS: GINV support: %s\n",
           mips_features_context.extension_support[MIPS_EXTENSION_GINV] ? "yes" : "no");

    // Count supported extensions
    mips_features_context.extension_count = 0;
    for (int i = 0; i < MIPS_MAX_EXTENSIONS; i++)
    {
        if (mips_features_context.extension_support[i])
        {
            mips_features_context.extension_count++;
        }
    }

    printf("MIPS: Detected %u supported extensions\n", mips_features_context.extension_count);
}

bool mips_features_detect_mips16(void)
{
    // Check for MIPS16 support in CPU configuration
    uint32_t config = mips_cpu_read_config_register();
    return (config & MIPS_CONFIG_MIPS16) != 0;
}

bool mips_features_detect_micromips(void)
{
    // Check for microMIPS support in CPU configuration
    uint32_t config = mips_cpu_read_config_register();
    return (config & MIPS_CONFIG_MICROMIPS) != 0;
}

bool mips_features_detect_dsp(void)
{
    // Check for DSP support in CPU configuration
    uint32_t config = mips_cpu_read_config_register();
    return (config & MIPS_CONFIG_DSP) != 0;
}

bool mips_features_detect_msa(void)
{
    // Check for MSA support in CPU configuration
    uint32_t config = mips_cpu_read_config_register();
    return (config & MIPS_CONFIG_MSA) != 0;
}

bool mips_features_detect_vz(void)
{
    // Check for VZ support in CPU configuration
    uint32_t config = mips_cpu_read_config_register();
    return (config & MIPS_CONFIG_VZ) != 0;
}

bool mips_features_detect_crc(void)
{
    // Check for CRC support in CPU configuration
    uint32_t config = mips_cpu_read_config_register();
    return (config & MIPS_CONFIG_CRC) != 0;
}

bool mips_features_detect_ginv(void)
{
    // Check for GINV support in CPU configuration
    uint32_t config = mips_cpu_read_config_register();
    return (config & MIPS_CONFIG_GINV) != 0;
}

// ============================================================================
// EXTENSION MANAGEMENT
// ============================================================================

bool mips_features_enable_extension(mips_extension_t extension)
{
    if (extension >= MIPS_MAX_EXTENSIONS)
    {
        printf("MIPS: Invalid extension %u\n", extension);
        return false;
    }

    if (!mips_features_context.extension_support[extension])
    {
        printf("MIPS: Extension %u not supported\n", extension);
        return false;
    }

    if (mips_features_context.extension_enabled[extension])
    {
        printf("MIPS: Extension %u already enabled\n", extension);
        return true;
    }

    // Enable specific extension
    switch (extension)
    {
    case MIPS_EXTENSION_MIPS16:
        mips_features_enable_mips16();
        break;
    case MIPS_EXTENSION_MICROMIPS:
        mips_features_enable_micromips();
        break;
    case MIPS_EXTENSION_DSP:
        mips_features_enable_dsp();
        break;
    case MIPS_EXTENSION_MSA:
        mips_features_enable_msa();
        break;
    case MIPS_EXTENSION_VZ:
        mips_features_enable_vz();
        break;
    case MIPS_EXTENSION_CRC:
        mips_features_enable_crc();
        break;
    case MIPS_EXTENSION_GINV:
        mips_features_enable_ginv();
        break;
    default:
        printf("MIPS: Unknown extension %u\n", extension);
        return false;
    }

    mips_features_context.extension_enabled[extension] = true;
    printf("MIPS: Extension %u enabled\n", extension);
    return true;
}

void mips_features_disable_extension(mips_extension_t extension)
{
    if (extension >= MIPS_MAX_EXTENSIONS)
    {
        printf("MIPS: Invalid extension %u\n", extension);
        return;
    }

    if (!mips_features_context.extension_enabled[extension])
    {
        printf("MIPS: Extension %u not enabled\n", extension);
        return;
    }

    // Disable specific extension
    switch (extension)
    {
    case MIPS_EXTENSION_MIPS16:
        mips_features_disable_mips16();
        break;
    case MIPS_EXTENSION_MICROMIPS:
        mips_features_disable_micromips();
        break;
    case MIPS_EXTENSION_DSP:
        mips_features_disable_dsp();
        break;
    case MIPS_EXTENSION_MSA:
        mips_features_disable_msa();
        break;
    case MIPS_EXTENSION_VZ:
        mips_features_disable_vz();
        break;
    case MIPS_EXTENSION_CRC:
        mips_features_disable_crc();
        break;
    case MIPS_EXTENSION_GINV:
        mips_features_disable_ginv();
        break;
    default:
        printf("MIPS: Unknown extension %u\n", extension);
        return;
    }

    mips_features_context.extension_enabled[extension] = false;
    printf("MIPS: Extension %u disabled\n", extension);
}

bool mips_features_is_extension_supported(mips_extension_t extension)
{
    if (extension >= MIPS_MAX_EXTENSIONS)
    {
        return false;
    }

    return mips_features_context.extension_support[extension];
}

bool mips_features_is_extension_enabled(mips_extension_t extension)
{
    if (extension >= MIPS_MAX_EXTENSIONS)
    {
        return false;
    }

    return mips_features_context.extension_enabled[extension];
}

// ============================================================================
// SPECIFIC EXTENSION IMPLEMENTATIONS
// ============================================================================

void mips_features_enable_mips16(void)
{
    printf("MIPS: Enabling MIPS16 extension\n");

    // Set MIPS16 mode in CPU
    uint32_t config = mips_cpu_read_config_register();
    config |= MIPS_CONFIG_MIPS16;
    mips_cpu_write_config_register(config);

    // Initialize MIPS16 decoder
    mips_features_init_mips16_decoder();
}

void mips_features_disable_mips16(void)
{
    printf("MIPS: Disabling MIPS16 extension\n");

    // Clear MIPS16 mode in CPU
    uint32_t config = mips_cpu_read_config_register();
    config &= ~MIPS_CONFIG_MIPS16;
    mips_cpu_write_config_register(config);
}

void mips_features_enable_micromips(void)
{
    printf("MIPS: Enabling microMIPS extension\n");

    // Set microMIPS mode in CPU
    uint32_t config = mips_cpu_read_config_register();
    config |= MIPS_CONFIG_MICROMIPS;
    mips_cpu_write_config_register(config);

    // Initialize microMIPS decoder
    mips_features_init_micromips_decoder();
}

void mips_features_disable_micromips(void)
{
    printf("MIPS: Disabling microMIPS extension\n");

    // Clear microMIPS mode in CPU
    uint32_t config = mips_cpu_read_config_register();
    config &= ~MIPS_CONFIG_MICROMIPS;
    mips_cpu_write_config_register(config);
}

void mips_features_enable_dsp(void)
{
    printf("MIPS: Enabling DSP extension\n");

    // Set DSP mode in CPU
    uint32_t config = mips_cpu_read_config_register();
    config |= MIPS_CONFIG_DSP;
    mips_cpu_write_config_register(config);

    // Initialize DSP unit
    mips_features_init_dsp_unit();
}

void mips_features_disable_dsp(void)
{
    printf("MIPS: Disabling DSP extension\n");

    // Clear DSP mode in CPU
    uint32_t config = mips_cpu_read_config_register();
    config &= ~MIPS_CONFIG_DSP;
    mips_cpu_write_config_register(config);
}

void mips_features_enable_msa(void)
{
    printf("MIPS: Enabling MSA extension\n");

    // Set MSA mode in CPU
    uint32_t config = mips_cpu_read_config_register();
    config |= MIPS_CONFIG_MSA;
    mips_cpu_write_config_register(config);

    // Initialize MSA unit
    mips_features_init_msa_unit();
}

void mips_features_disable_msa(void)
{
    printf("MIPS: Disabling MSA extension\n");

    // Clear MSA mode in CPU
    uint32_t config = mips_cpu_read_config_register();
    config &= ~MIPS_CONFIG_MSA;
    mips_cpu_write_config_register(config);
}

void mips_features_enable_vz(void)
{
    printf("MIPS: Enabling VZ extension\n");

    // Set VZ mode in CPU
    uint32_t config = mips_cpu_read_config_register();
    config |= MIPS_CONFIG_VZ;
    mips_cpu_write_config_register(config);

    // Initialize virtualization unit
    mips_features_init_virtualization_unit();
}

void mips_features_disable_vz(void)
{
    printf("MIPS: Disabling VZ extension\n");

    // Clear VZ mode in CPU
    uint32_t config = mips_cpu_read_config_register();
    config &= ~MIPS_CONFIG_VZ;
    mips_cpu_write_config_register(config);
}

void mips_features_enable_crc(void)
{
    printf("MIPS: Enabling CRC extension\n");

    // Set CRC mode in CPU
    uint32_t config = mips_cpu_read_config_register();
    config |= MIPS_CONFIG_CRC;
    mips_cpu_write_config_register(config);

    // Initialize CRC unit
    mips_features_init_crc_unit();
}

void mips_features_disable_crc(void)
{
    printf("MIPS: Disabling CRC extension\n");

    // Clear CRC mode in CPU
    uint32_t config = mips_cpu_read_config_register();
    config &= ~MIPS_CONFIG_CRC;
    mips_cpu_write_config_register(config);
}

void mips_features_enable_ginv(void)
{
    printf("MIPS: Enabling GINV extension\n");

    // Set GINV mode in CPU
    uint32_t config = mips_cpu_read_config_register();
    config |= MIPS_CONFIG_GINV;
    mips_cpu_write_config_register(config);

    // Initialize GINV unit
    mips_features_init_ginv_unit();
}

void mips_features_disable_ginv(void)
{
    printf("MIPS: Disabling GINV extension\n");

    // Clear GINV mode in CPU
    uint32_t config = mips_cpu_read_config_register();
    config &= ~MIPS_CONFIG_GINV;
    mips_cpu_write_config_register(config);
}

// ============================================================================
// EXTENSION INITIALIZATION
// ============================================================================

void mips_features_init_mips16_decoder(void)
{
    printf("MIPS: Initializing MIPS16 decoder\n");
    // MIPS16 decoder initialization code
}

void mips_features_init_micromips_decoder(void)
{
    printf("MIPS: Initializing microMIPS decoder\n");
    // microMIPS decoder initialization code
}

void mips_features_init_dsp_unit(void)
{
    printf("MIPS: Initializing DSP unit\n");
    // DSP unit initialization code
}

void mips_features_init_msa_unit(void)
{
    printf("MIPS: Initializing MSA unit\n");
    // MSA unit initialization code
}

void mips_features_init_virtualization_unit(void)
{
    printf("MIPS: Initializing virtualization unit\n");
    // Virtualization unit initialization code
}

void mips_features_init_crc_unit(void)
{
    printf("MIPS: Initializing CRC unit\n");
    // CRC unit initialization code
}

void mips_features_init_ginv_unit(void)
{
    printf("MIPS: Initializing GINV unit\n");
    // GINV unit initialization code
}

// ============================================================================
// ADVANCED FEATURES
// ============================================================================

void mips_features_enable_advanced_features(void)
{
    printf("MIPS: Enabling advanced features\n");

    mips_features_context.advanced_features_enabled = true;

    // Enable performance optimizations
    mips_features_enable_performance_optimizations();

    // Enable security features
    if (mips_features_context.security_features_enabled)
    {
        mips_features_enable_security_features();
    }

    // Enable virtualization features
    if (mips_features_context.virtualization_features_enabled)
    {
        mips_features_enable_virtualization_features();
    }
}

void mips_features_disable_advanced_features(void)
{
    printf("MIPS: Disabling advanced features\n");

    mips_features_context.advanced_features_enabled = false;

    // Disable performance optimizations
    mips_features_disable_performance_optimizations();

    // Disable security features
    mips_features_disable_security_features();

    // Disable virtualization features
    mips_features_disable_virtualization_features();
}

void mips_features_enable_performance_optimizations(void)
{
    printf("MIPS: Enabling performance optimizations\n");

    // Enable branch prediction
    mips_cpu_enable_branch_prediction();

    // Enable out-of-order execution
    mips_cpu_enable_out_of_order_execution();

    // Enable speculative execution
    mips_cpu_enable_speculative_execution();
}

void mips_features_disable_performance_optimizations(void)
{
    printf("MIPS: Disabling performance optimizations\n");

    // Disable branch prediction
    mips_cpu_disable_branch_prediction();

    // Disable out-of-order execution
    mips_cpu_disable_out_of_order_execution();

    // Disable speculative execution
    mips_cpu_disable_speculative_execution();
}

void mips_features_enable_security_features(void)
{
    printf("MIPS: Enabling security features\n");

    // Enable memory protection
    mips_memory_enable_protection();

    // Enable address space randomization
    mips_memory_enable_aslr();

    // Enable stack protection
    mips_memory_enable_stack_protection();
}

void mips_features_disable_security_features(void)
{
    printf("MIPS: Disabling security features\n");

    // Disable memory protection
    mips_memory_disable_protection();

    // Disable address space randomization
    mips_memory_disable_aslr();

    // Disable stack protection
    mips_memory_disable_stack_protection();
}

void mips_features_enable_virtualization_features(void)
{
    printf("MIPS: Enabling virtualization features\n");

    // Enable hardware virtualization
    mips_cpu_enable_hardware_virtualization();

    // Enable nested paging
    mips_mmu_enable_nested_paging();

    // Enable virtual machine extensions
    mips_cpu_enable_vm_extensions();
}

void mips_features_disable_virtualization_features(void)
{
    printf("MIPS: Disabling virtualization features\n");

    // Disable hardware virtualization
    mips_cpu_disable_hardware_virtualization();

    // Disable nested paging
    mips_mmu_disable_nested_paging();

    // Disable virtual machine extensions
    mips_cpu_disable_vm_extensions();
}

// ============================================================================
// OPTIMIZATION MANAGEMENT
// ============================================================================

void mips_features_set_optimization_level(mips_optimization_level_t level)
{
    printf("MIPS: Setting optimization level to %u\n", level);

    mips_features_context.optimization_level = level;

    switch (level)
    {
    case MIPS_OPTIMIZATION_LEVEL_NONE:
        mips_features_disable_all_optimizations();
        break;
    case MIPS_OPTIMIZATION_LEVEL_BASIC:
        mips_features_enable_basic_optimizations();
        break;
    case MIPS_OPTIMIZATION_LEVEL_BALANCED:
        mips_features_enable_balanced_optimizations();
        break;
    case MIPS_OPTIMIZATION_LEVEL_AGGRESSIVE:
        mips_features_enable_aggressive_optimizations();
        break;
    case MIPS_OPTIMIZATION_LEVEL_MAXIMUM:
        mips_features_enable_maximum_optimizations();
        break;
    default:
        printf("MIPS: Unknown optimization level %u\n", level);
        break;
    }
}

void mips_features_enable_basic_optimizations(void)
{
    printf("MIPS: Enabling basic optimizations\n");

    // Basic instruction scheduling
    mips_cpu_enable_basic_scheduling();

    // Basic register allocation
    mips_cpu_enable_basic_register_allocation();
}

void mips_features_enable_balanced_optimizations(void)
{
    printf("MIPS: Enabling balanced optimizations\n");

    // Enable basic optimizations
    mips_features_enable_basic_optimizations();

    // Moderate instruction scheduling
    mips_cpu_enable_moderate_scheduling();

    // Moderate register allocation
    mips_cpu_enable_moderate_register_allocation();
}

void mips_features_enable_aggressive_optimizations(void)
{
    printf("MIPS: Enabling aggressive optimizations\n");

    // Enable balanced optimizations
    mips_features_enable_balanced_optimizations();

    // Aggressive instruction scheduling
    mips_cpu_enable_aggressive_scheduling();

    // Aggressive register allocation
    mips_cpu_enable_aggressive_register_allocation();

    // Enable loop optimizations
    mips_cpu_enable_loop_optimizations();
}

void mips_features_enable_maximum_optimizations(void)
{
    printf("MIPS: Enabling maximum optimizations\n");

    // Enable aggressive optimizations
    mips_features_enable_aggressive_optimizations();

    // Maximum instruction scheduling
    mips_cpu_enable_maximum_scheduling();

    // Maximum register allocation
    mips_cpu_enable_maximum_register_allocation();

    // Enable all available optimizations
    mips_cpu_enable_all_optimizations();
}

void mips_features_disable_all_optimizations(void)
{
    printf("MIPS: Disabling all optimizations\n");

    // Disable all CPU optimizations
    mips_cpu_disable_all_optimizations();
}

// ============================================================================
// FEATURES UTILITIES
// ============================================================================

void mips_features_print_status(void)
{
    printf("MIPS: Features Status:\n");
    printf("  Features Initialized: %s\n", mips_features_context.features_initialized ? "yes" : "no");
    printf("  Extension Count: %u\n", mips_features_context.extension_count);
    printf("  Advanced Features: %s\n", mips_features_context.advanced_features_enabled ? "enabled" : "disabled");
    printf("  Optimization Level: %u\n", mips_features_context.optimization_level);
    printf("  Security Features: %s\n", mips_features_context.security_features_enabled ? "enabled" : "disabled");
    printf("  Virtualization Features: %s\n", mips_features_context.virtualization_features_enabled ? "enabled" : "disabled");

    printf("  Extension Support:\n");
    for (int i = 0; i < MIPS_MAX_EXTENSIONS; i++)
    {
        if (mips_features_context.extension_support[i])
        {
            const char *status = mips_features_context.extension_enabled[i] ? "enabled" : "disabled";
            printf("    Extension %u: supported, %s\n", i, status);
        }
    }
}

void mips_features_enable_all_supported_extensions(void)
{
    printf("MIPS: Enabling all supported extensions\n");

    for (int i = 0; i < MIPS_MAX_EXTENSIONS; i++)
    {
        if (mips_features_context.extension_support[i] && !mips_features_context.extension_enabled[i])
        {
            mips_features_enable_extension(i);
        }
    }
}

void mips_features_disable_all_extensions(void)
{
    printf("MIPS: Disabling all extensions\n");

    for (int i = 0; i < MIPS_MAX_EXTENSIONS; i++)
    {
        if (mips_features_context.extension_enabled[i])
        {
            mips_features_disable_extension(i);
        }
    }
}

void mips_features_cleanup(void)
{
    printf("MIPS: Cleaning up features system\n");

    // Disable all extensions
    mips_features_disable_all_extensions();

    // Disable advanced features
    mips_features_disable_advanced_features();

    // Reset optimization level
    mips_features_set_optimization_level(MIPS_OPTIMIZATION_LEVEL_BALANCED);

    mips_features_context.features_initialized = false;
    mips_features_context.extension_count = 0;
    mips_features_context.advanced_features_enabled = false;

    printf("MIPS: Features cleanup completed\n");
}
