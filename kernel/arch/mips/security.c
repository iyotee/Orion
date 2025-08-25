/*
 * ORION OS - MIPS Security Management
 *
 * Security management for MIPS architecture
 * Handles security features, virtualization, and access control
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
// SECURITY CONTEXT MANAGEMENT
// ============================================================================

static mips_security_context_t mips_security_context = {0};
static mips_virtualization_context_t mips_virtualization_context = {0};

void mips_security_init(void)
{
    memset(&mips_security_context, 0, sizeof(mips_security_context));
    memset(&mips_virtualization_context, 0, sizeof(mips_virtualization_context));

    // Set default security settings
    mips_security_context.secure_mode = false;
    mips_security_context.virtualization_enabled = false;
    mips_security_context.debug_enabled = false;
    mips_security_context.security_level = 0;

    // Set default virtualization settings
    mips_virtualization_context.virtualization_enabled = false;
    mips_virtualization_context.guest_id = 0;
    mips_virtualization_context.host_id = 0;
    mips_virtualization_context.nested_virtualization = false;

    printf("MIPS: Security contexts initialized\n");
}

mips_security_context_t *mips_security_get_context(void)
{
    return &mips_security_context;
}

mips_virtualization_context_t *mips_virtualization_get_context(void)
{
    return &mips_virtualization_context;
}

// ============================================================================
// SECURITY MODE MANAGEMENT
// ============================================================================

void mips_security_enable_secure_mode(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_SECURITY))
    {
        printf("MIPS: Security features not supported\n");
        return;
    }

    // Enable secure mode in CP0
    uint64_t security = mips_cpu_read_cp0_register(MIPS_CP0_SECURITY);
    security |= MIPS_SECURITY_SECURE_MODE;
    mips_cpu_write_cp0_register(MIPS_CP0_SECURITY, security);

    mips_security_context.secure_mode = true;
    mips_security_context.security_level = 1;

    printf("MIPS: Secure mode enabled\n");
}

void mips_security_disable_secure_mode(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_SECURITY))
    {
        return;
    }

    uint64_t security = mips_cpu_read_cp0_register(MIPS_CP0_SECURITY);
    security &= ~MIPS_SECURITY_SECURE_MODE;
    mips_cpu_write_cp0_register(MIPS_CP0_SECURITY, security);

    mips_security_context.secure_mode = false;
    mips_security_context.security_level = 0;

    printf("MIPS: Secure mode disabled\n");
}

bool mips_security_is_secure_mode(void)
{
    return mips_security_context.secure_mode;
}

void mips_security_set_security_level(uint32_t level)
{
    if (level > MIPS_MAX_SECURITY_LEVEL)
    {
        printf("MIPS: Invalid security level: %d\n", level);
        return;
    }

    mips_security_context.security_level = level;

    // Update CP0 security register
    uint64_t security = mips_cpu_read_cp0_register(MIPS_CP0_SECURITY);
    security = (security & ~MIPS_SECURITY_LEVEL_MASK) | (level << MIPS_SECURITY_LEVEL_SHIFT);
    mips_cpu_write_cp0_register(MIPS_CP0_SECURITY, security);

    printf("MIPS: Security level set to %d\n", level);
}

uint32_t mips_security_get_security_level(void)
{
    return mips_security_context.security_level;
}

// ============================================================================
// VIRTUALIZATION MANAGEMENT
// ============================================================================

void mips_virtualization_enable(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_VZ))
    {
        printf("MIPS: Virtualization not supported\n");
        return;
    }

    // Enable virtualization in CP0
    uint64_t vz = mips_cpu_read_cp0_register(MIPS_CP0_VZ);
    vz |= MIPS_VZ_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_VZ, vz);

    mips_virtualization_context.virtualization_enabled = true;
    mips_security_context.virtualization_enabled = true;

    printf("MIPS: Virtualization enabled\n");
}

void mips_virtualization_disable(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_VZ))
    {
        return;
    }

    uint64_t vz = mips_cpu_read_cp0_register(MIPS_CP0_VZ);
    vz &= ~MIPS_VZ_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_VZ, vz);

    mips_virtualization_context.virtualization_enabled = false;
    mips_security_context.virtualization_enabled = false;

    printf("MIPS: Virtualization disabled\n");
}

bool mips_virtualization_is_enabled(void)
{
    return mips_virtualization_context.virtualization_enabled;
}

void mips_virtualization_set_guest_id(uint32_t guest_id)
{
    if (!mips_virtualization_context.virtualization_enabled)
    {
        printf("MIPS: Virtualization not enabled\n");
        return;
    }

    mips_virtualization_context.guest_id = guest_id;

    // Update CP0 guest ID register
    mips_cpu_write_cp0_register(MIPS_CP0_GUESTID, guest_id);

    printf("MIPS: Guest ID set to %d\n", guest_id);
}

uint32_t mips_virtualization_get_guest_id(void)
{
    return mips_virtualization_context.guest_id;
}

void mips_virtualization_set_host_id(uint32_t host_id)
{
    if (!mips_virtualization_context.virtualization_enabled)
    {
        printf("MIPS: Virtualization not enabled\n");
        return;
    }

    mips_virtualization_context.host_id = host_id;

    // Update CP0 host ID register
    mips_cpu_write_cp0_register(MIPS_CP0_HOSTID, host_id);

    printf("MIPS: Host ID set to %d\n", host_id);
}

uint32_t mips_virtualization_get_host_id(void)
{
    return mips_virtualization_context.host_id;
}

void mips_virtualization_enter_guest_mode(void)
{
    if (!mips_virtualization_context.virtualization_enabled)
    {
        printf("MIPS: Virtualization not enabled\n");
        return;
    }

    // Set guest mode in CP0
    uint64_t vz = mips_cpu_read_cp0_register(MIPS_CP0_VZ);
    vz |= MIPS_VZ_GUEST_MODE;
    mips_cpu_write_cp0_register(MIPS_CP0_VZ, vz);

    printf("MIPS: Entered guest mode\n");
}

void mips_virtualization_exit_guest_mode(void)
{
    if (!mips_virtualization_context.virtualization_enabled)
    {
        return;
    }

    uint64_t vz = mips_cpu_read_cp0_register(MIPS_CP0_VZ);
    vz &= ~MIPS_VZ_GUEST_MODE;
    mips_cpu_write_cp0_register(MIPS_CP0_VZ, vz);

    printf("MIPS: Exited guest mode\n");
}

bool mips_virtualization_is_guest_mode(void)
{
    if (!mips_virtualization_context.virtualization_enabled)
    {
        return false;
    }

    uint64_t vz = mips_cpu_read_cp0_register(MIPS_CP0_VZ);
    return (vz & MIPS_VZ_GUEST_MODE) != 0;
}

// ============================================================================
// DEBUG MANAGEMENT
// ============================================================================

void mips_security_enable_debug(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_DEBUG))
    {
        printf("MIPS: Debug features not supported\n");
        return;
    }

    // Enable debug mode in CP0
    uint64_t debug = mips_cpu_read_cp0_register(MIPS_CP0_DEBUG);
    debug |= MIPS_DEBUG_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_DEBUG, debug);

    mips_security_context.debug_enabled = true;

    printf("MIPS: Debug mode enabled\n");
}

void mips_security_disable_debug(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_DEBUG))
    {
        return;
    }

    uint64_t debug = mips_cpu_read_cp0_register(MIPS_CP0_DEBUG);
    debug &= ~MIPS_DEBUG_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_DEBUG, debug);

    mips_security_context.debug_enabled = false;

    printf("MIPS: Debug mode disabled\n");
}

bool mips_security_is_debug_enabled(void)
{
    return mips_security_context.debug_enabled;
}

// ============================================================================
// ACCESS CONTROL
// ============================================================================

bool mips_security_check_access(mips_addr_t addr, uint32_t access_flags, uint32_t security_level)
{
    // Check if address is accessible at given security level
    if (security_level > mips_security_context.security_level)
    {
        return false;
    }

    // Check memory protection
    if (!mips_memory_is_accessible(addr, access_flags))
    {
        return false;
    }

    // Check virtualization restrictions
    if (mips_virtualization_context.virtualization_enabled &&
        mips_virtualization_is_guest_mode())
    {
        // Guest mode restrictions
        if (addr >= MIPS_GUEST_MEMORY_LIMIT)
        {
            return false;
        }
    }

    return true;
}

bool mips_security_check_privilege(mips_cpu_mode_t mode, uint32_t required_level)
{
    // Check if current mode has required privilege level
    if (mode == MIPS_MODE_KERNEL)
    {
        return true; // Kernel mode has full access
    }

    if (mode == MIPS_MODE_SUPERVISOR)
    {
        return required_level <= 1; // Supervisor mode has limited access
    }

    if (mode == MIPS_MODE_USER)
    {
        return required_level == 0; // User mode has minimal access
    }

    return false;
}

// ============================================================================
// SECURITY FEATURES
// ============================================================================

void mips_security_enable_encryption(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_CRYPTO))
    {
        printf("MIPS: Crypto features not supported\n");
        return;
    }

    // Enable crypto unit
    uint64_t crypto = mips_cpu_read_cp0_register(MIPS_CP0_CRYPTO);
    crypto |= MIPS_CRYPTO_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_CRYPTO, crypto);

    printf("MIPS: Encryption enabled\n");
}

void mips_security_disable_encryption(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_CRYPTO))
    {
        return;
    }

    uint64_t crypto = mips_cpu_read_cp0_register(MIPS_CP0_CRYPTO);
    crypto &= ~MIPS_CRYPTO_ENABLE;
    mips_cpu_write_cp0_register(MIPS_CP0_CRYPTO, crypto);

    printf("MIPS: Encryption disabled\n");
}

bool mips_security_is_encryption_enabled(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_CRYPTO))
    {
        return false;
    }

    uint64_t crypto = mips_cpu_read_cp0_register(MIPS_CP0_CRYPTO);
    return (crypto & MIPS_CRYPTO_ENABLE) != 0;
}

void mips_security_enable_secure_boot(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_SECURITY))
    {
        printf("MIPS: Secure boot not supported\n");
        return;
    }

    // Enable secure boot
    uint64_t security = mips_cpu_read_cp0_register(MIPS_CP0_SECURITY);
    security |= MIPS_SECURITY_SECURE_BOOT;
    mips_cpu_write_cp0_register(MIPS_CP0_SECURITY, security);

    printf("MIPS: Secure boot enabled\n");
}

void mips_security_disable_secure_boot(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_SECURITY))
    {
        return;
    }

    uint64_t security = mips_cpu_read_cp0_register(MIPS_CP0_SECURITY);
    security &= ~MIPS_SECURITY_SECURE_BOOT;
    mips_cpu_write_cp0_register(MIPS_CP0_SECURITY, security);

    printf("MIPS: Secure boot disabled\n");
}

bool mips_security_is_secure_boot_enabled(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_SECURITY))
    {
        return false;
    }

    uint64_t security = mips_cpu_read_cp0_register(MIPS_CP0_SECURITY);
    return (security & MIPS_SECURITY_SECURE_BOOT) != 0;
}

// ============================================================================
// SECURITY MONITORING
// ============================================================================

void mips_security_enable_monitoring(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_SECURITY))
    {
        printf("MIPS: Security monitoring not supported\n");
        return;
    }

    // Enable security monitoring
    uint64_t security = mips_cpu_read_cp0_register(MIPS_CP0_SECURITY);
    security |= MIPS_SECURITY_MONITORING;
    mips_cpu_write_cp0_register(MIPS_CP0_SECURITY, security);

    printf("MIPS: Security monitoring enabled\n");
}

void mips_security_disable_monitoring(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_SECURITY))
    {
        return;
    }

    uint64_t security = mips_cpu_read_cp0_register(MIPS_CP0_SECURITY);
    security &= ~MIPS_SECURITY_MONITORING;
    mips_cpu_write_cp0_register(MIPS_CP0_SECURITY, security);

    printf("MIPS: Security monitoring disabled\n");
}

bool mips_security_is_monitoring_enabled(void)
{
    if (!mips_cpu_supports_feature(MIPS_FEATURE_SECURITY))
    {
        return false;
    }

    uint64_t security = mips_cpu_read_cp0_register(MIPS_CP0_SECURITY);
    return (security & MIPS_SECURITY_MONITORING) != 0;
}

// ============================================================================
// SECURITY STATISTICS
// ============================================================================

void mips_security_print_statistics(void)
{
    printf("MIPS: Security Statistics:\n");
    printf("  Secure Mode: %s\n", mips_security_context.secure_mode ? "enabled" : "disabled");
    printf("  Security Level: %d\n", mips_security_context.security_level);
    printf("  Virtualization: %s\n", mips_virtualization_context.virtualization_enabled ? "enabled" : "disabled");
    printf("  Debug Mode: %s\n", mips_security_context.debug_enabled ? "enabled" : "disabled");
    printf("  Guest ID: %d\n", mips_virtualization_context.guest_id);
    printf("  Host ID: %d\n", mips_virtualization_context.host_id);
    printf("  Guest Mode: %s\n", mips_virtualization_is_guest_mode() ? "active" : "inactive");
    printf("  Encryption: %s\n", mips_security_is_encryption_enabled() ? "enabled" : "disabled");
    printf("  Secure Boot: %s\n", mips_security_is_secure_boot_enabled() ? "enabled" : "disabled");
    printf("  Monitoring: %s\n", mips_security_is_monitoring_enabled() ? "enabled" : "disabled");
}

// ============================================================================
// SECURITY UTILITY FUNCTIONS
// ============================================================================

uint32_t mips_security_calculate_checksum(const void *data, size_t size)
{
    uint32_t checksum = 0;
    const uint8_t *bytes = (const uint8_t *)data;

    for (size_t i = 0; i < size; i++)
    {
        checksum += bytes[i];
    }

    return checksum;
}

bool mips_security_verify_checksum(const void *data, size_t size, uint32_t expected_checksum)
{
    uint32_t calculated_checksum = mips_security_calculate_checksum(data, size);
    return calculated_checksum == expected_checksum;
}

void mips_security_secure_memory_region(mips_addr_t start, mips_size_t size)
{
    // Mark memory region as secure
    mips_memory_protect(start, size, MIPS_ENTRYLO_READ | MIPS_ENTRYLO_WRITE);

    printf("MIPS: Memory region secured: 0x%llx - 0x%llx\n", start, start + size);
}

void mips_security_unsecure_memory_region(mips_addr_t start, mips_size_t size)
{
    // Mark memory region as unsecure
    mips_memory_protect(start, size, MIPS_ENTRYLO_READ | MIPS_ENTRYLO_WRITE | MIPS_ENTRYLO_EXECUTE);

    printf("MIPS: Memory region unsecured: 0x%llx - 0x%llx\n", start, start + size);
}
