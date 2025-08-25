/*
 * ORION OS - POWER Architecture Vector Management
 *
 * Vector operations and management for POWER architecture
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

// ============================================================================
// VECTOR REGISTERS
// ============================================================================

// VSX registers (128-bit)
static __attribute__((aligned(16))) uint8_t power_vsx_regs[32][16];

// AltiVec registers (128-bit)
static __attribute__((aligned(16))) uint8_t power_altivec_regs[32][16];

// Vector context
typedef struct {
    bool vsx_enabled;
    bool altivec_enabled;
    bool vsx_256_enabled;
    bool vsx_512_enabled;
    uint32_t vsx_version;
    uint32_t altivec_version;
    uint64_t vector_flags;
} power_vector_context_t;

static power_vector_context_t power_vector;

// ============================================================================
// VECTOR INITIALIZATION
// ============================================================================

int power_vector_init(void)
{
    printf("POWER: Initializing vector support\n");
    
    // Clear vector context
    memset(&power_vector, 0, sizeof(power_vector));
    
    // Clear vector registers
    memset(power_vsx_regs, 0, sizeof(power_vsx_regs));
    memset(power_altivec_regs, 0, sizeof(power_altivec_regs));
    
    // Detect VSX support
    if (power_detect_vsx_support()) {
        power_vector.vsx_enabled = true;
        power_vector.vsx_version = power_get_vsx_version();
        printf("POWER: VSX version %u detected\n", power_vector.vsx_version);
        
        // Check for extended VSX support
        if (power_vector.vsx_version >= 2) {
            power_vector.vsx_256_enabled = true;
            printf("POWER: VSX 256-bit support enabled\n");
        }
        
        if (power_vector.vsx_version >= 3) {
            power_vector.vsx_512_enabled = true;
            printf("POWER: VSX 512-bit support enabled\n");
        }
    }
    
    // Detect AltiVec support
    if (power_detect_altivec_support()) {
        power_vector.altivec_enabled = true;
        power_vector.altivec_version = power_get_altivec_version();
        printf("POWER: AltiVec version %u detected\n", power_vector.altivec_version);
    }
    
    // Set vector flags
    if (power_vector.vsx_enabled) {
        power_vector.vector_flags |= POWER_VECTOR_VSX;
    }
    if (power_vector.altivec_enabled) {
        power_vector.vector_flags |= POWER_VECTOR_ALTIVEC;
    }
    if (power_vector.vsx_256_enabled) {
        power_vector.vector_flags |= POWER_VECTOR_VSX_256;
    }
    if (power_vector.vsx_512_enabled) {
        power_vector.vector_flags |= POWER_VECTOR_VSX_512;
    }
    
    printf("POWER: Vector support initialized successfully\n");
    return 0;
}

// ============================================================================
// VSX SUPPORT DETECTION
// ============================================================================

bool power_detect_vsx_support(void)
{
    // Check MSR.VSX bit
    uint64_t msr;
    __asm__ volatile("mfmsr %0" : "=r"(msr));
    
    if (msr & POWER_MSR_VSX) {
        return true;
    }
    
    // Try to enable VSX
    msr |= POWER_MSR_VSX;
    __asm__ volatile("mtmsr %0" : : "r"(msr));
    
    // Check if it was enabled
    __asm__ volatile("mfmsr %0" : "=r"(msr));
    return (msr & POWER_MSR_VSX) != 0;
}

uint32_t power_get_vsx_version(void)
{
    // Read PVR to determine POWER generation
    uint32_t pvr;
    __asm__ volatile("mfspr %0, 287" : "=r"(pvr));
    
    uint32_t generation = (pvr >> 16) & 0xFFFF;
    
    switch (generation) {
        case 0x004D: // POWER8
            return 1;
        case 0x004E: // POWER9
            return 2;
        case 0x0080: // POWER10
            return 3;
        default:
            return 1; // Default to version 1
    }
}

// ============================================================================
// ALTIVEC SUPPORT DETECTION
// ============================================================================

bool power_detect_altivec_support(void)
{
    // Check MSR.FP bit (AltiVec requires FP)
    uint64_t msr;
    __asm__ volatile("mfmsr %0" : "=r"(msr));
    
    if (msr & POWER_MSR_FP) {
        return true;
    }
    
    // Try to enable FP
    msr |= POWER_MSR_FP;
    __asm__ volatile("mtmsr %0" : : "r"(msr));
    
    // Check if it was enabled
    __asm__ volatile("mfmsr %0" : "=r"(msr));
    return (msr & POWER_MSR_FP) != 0;
}

uint32_t power_get_altivec_version(void)
{
    // AltiVec version is typically tied to POWER generation
    uint32_t pvr;
    __asm__ volatile("mfspr %0, 287" : "=r"(pvr));
    
    uint32_t generation = (pvr >> 16) & 0xFFFF;
    
    switch (generation) {
        case 0x004D: // POWER8
            return 2;
        case 0x004E: // POWER9
            return 3;
        case 0x0080: // POWER10
            return 4;
        default:
            return 1; // Default to version 1
    }
}

// ============================================================================
// VSX OPERATIONS
// ============================================================================

void power_vsx_load_128(uint32_t reg, const void *data)
{
    if (!power_vector.vsx_enabled || reg >= 32) {
        return;
    }
    
    memcpy(power_vsx_regs[reg], data, 16);
}

void power_vsx_store_128(uint32_t reg, void *data)
{
    if (!power_vector.vsx_enabled || reg >= 32) {
        return;
    }
    
    memcpy(data, power_vsx_regs[reg], 16);
}

void power_vsx_add_128(uint32_t dst, uint32_t src1, uint32_t src2)
{
    if (!power_vector.vsx_enabled || dst >= 32 || src1 >= 32 || src2 >= 32) {
        return;
    }
    
    // Load source registers
    uint64_t *s1 = (uint64_t *)power_vsx_regs[src1];
    uint64_t *s2 = (uint64_t *)power_vsx_regs[src2];
    uint64_t *d = (uint64_t *)power_vsx_regs[dst];
    
    // Perform 128-bit addition (two 64-bit operations)
    d[0] = s1[0] + s2[0];
    d[1] = s1[1] + s2[1];
}

void power_vsx_mul_128(uint32_t dst, uint32_t src1, uint32_t src2)
{
    if (!power_vector.vsx_enabled || dst >= 32 || src1 >= 32 || src2 >= 32) {
        return;
    }
    
    // Load source registers
    uint64_t *s1 = (uint64_t *)power_vsx_regs[src1];
    uint64_t *s2 = (uint64_t *)power_vsx_regs[src2];
    uint64_t *d = (uint64_t *)power_vsx_regs[dst];
    
    // Perform 128-bit multiplication (two 64-bit operations)
    d[0] = s1[0] * s2[0];
    d[1] = s1[1] * s2[1];
}

void power_vsx_xor_128(uint32_t dst, uint32_t src1, uint32_t src2)
{
    if (!power_vector.vsx_enabled || dst >= 32 || src1 >= 32 || src2 >= 32) {
        return;
    }
    
    // Load source registers
    uint64_t *s1 = (uint64_t *)power_vsx_regs[src1];
    uint64_t *s2 = (uint64_t *)power_vsx_regs[src2];
    uint64_t *d = (uint64_t *)power_vsx_regs[dst];
    
    // Perform 128-bit XOR (two 64-bit operations)
    d[0] = s1[0] ^ s2[0];
    d[1] = s1[1] ^ s2[1];
}

// ============================================================================
// ALTIVEC OPERATIONS
// ============================================================================

void power_altivec_load_128(uint32_t reg, const void *data)
{
    if (!power_vector.altivec_enabled || reg >= 32) {
        return;
    }
    
    memcpy(power_altivec_regs[reg], data, 16);
}

void power_altivec_store_128(uint32_t reg, void *data)
{
    if (!power_vector.altivec_enabled || reg >= 32) {
        return;
    }
    
    memcpy(data, power_altivec_regs[reg], 16);
}

void power_altivec_add_128(uint32_t dst, uint32_t src1, uint32_t src2)
{
    if (!power_vector.altivec_enabled || dst >= 32 || src1 >= 32 || src2 >= 32) {
        return;
    }
    
    // Load source registers
    uint64_t *s1 = (uint64_t *)power_altivec_regs[src1];
    uint64_t *s2 = (uint64_t *)power_altivec_regs[src2];
    uint64_t *d = (uint64_t *)power_altivec_regs[dst];
    
    // Perform 128-bit addition (two 64-bit operations)
    d[0] = s1[0] + s2[0];
    d[1] = s1[1] + s2[1];
}

void power_altivec_mul_128(uint32_t dst, uint32_t src1, uint32_t src2)
{
    if (!power_vector.altivec_enabled || dst >= 32 || src1 >= 32 || src2 >= 32) {
        return;
    }
    
    // Load source registers
    uint64_t *s1 = (uint64_t *)power_altivec_regs[src1];
    uint64_t *s2 = (uint64_t *)power_altivec_regs[src2];
    uint64_t *d = (uint64_t *)power_altivec_regs[dst];
    
    // Perform 128-bit multiplication (two 64-bit operations)
    d[0] = s1[0] * s2[0];
    d[1] = s1[1] * s2[1];
}

void power_altivec_xor_128(uint32_t dst, uint32_t src1, uint32_t src2)
{
    if (!power_vector.altivec_enabled || dst >= 32 || src1 >= 32 || src2 >= 32) {
        return;
    }
    
    // Load source registers
    uint64_t *s1 = (uint64_t *)power_altivec_regs[src1];
    uint64_t *s2 = (uint64_t *)power_altivec_regs[src2];
    uint64_t *d = (uint64_t *)power_altivec_regs[dst];
    
    // Perform 128-bit XOR (two 64-bit operations)
    d[0] = s1[0] ^ s2[0];
    d[1] = s1[1] ^ s2[1];
}

// ============================================================================
// ADVANCED VECTOR OPERATIONS
// ============================================================================

void power_vector_memset_128(void *ptr, uint8_t value, size_t size)
{
    if (!power_vector.vsx_enabled) {
        // Fallback to standard memset
        memset(ptr, value, size);
        return;
    }
    
    // Use VSX for 128-bit aligned operations
    uint8_t *p = (uint8_t *)ptr;
    size_t aligned_size = (size / 16) * 16;
    
    // Set 128-bit blocks using VSX
    for (size_t i = 0; i < aligned_size; i += 16) {
        power_vsx_load_128(0, &value);
        power_vsx_store_128(0, &p[i]);
    }
    
    // Handle remaining bytes
    for (size_t i = aligned_size; i < size; i++) {
        p[i] = value;
    }
}

void power_vector_memcpy_128(void *dst, const void *src, size_t size)
{
    if (!power_vector.vsx_enabled) {
        // Fallback to standard memcpy
        memcpy(dst, src, size);
        return;
    }
    
    // Use VSX for 128-bit aligned operations
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    size_t aligned_size = (size / 16) * 16;
    
    // Copy 128-bit blocks using VSX
    for (size_t i = 0; i < aligned_size; i += 16) {
        power_vsx_load_128(0, &s[i]);
        power_vsx_store_128(0, &d[i]);
    }
    
    // Handle remaining bytes
    for (size_t i = aligned_size; i < size; i++) {
        d[i] = s[i];
    }
}

// ============================================================================
// VECTOR BENCHMARKS
// ============================================================================

uint64_t power_vector_benchmark_add_128(uint32_t iterations)
{
    if (!power_vector.vsx_enabled) {
        return 0;
    }
    
    // Initialize test data
    uint64_t test_data1[2] = {0x1234567890ABCDEF, 0xFEDCBA0987654321};
    uint64_t test_data2[2] = {0x1111111111111111, 0x2222222222222222};
    
    power_vsx_load_128(0, test_data1);
    power_vsx_load_128(1, test_data2);
    
    uint64_t start_time = power_get_timer_value();
    
    for (uint32_t i = 0; i < iterations; i++) {
        power_vsx_add_128(2, 0, 1);
    }
    
    uint64_t end_time = power_get_timer_value();
    
    return end_time - start_time;
}

uint64_t power_vector_benchmark_mul_128(uint32_t iterations)
{
    if (!power_vector.vsx_enabled) {
        return 0;
    }
    
    // Initialize test data
    uint64_t test_data1[2] = {0x1234567890ABCDEF, 0xFEDCBA0987654321};
    uint64_t test_data2[2] = {0x1111111111111111, 0x2222222222222222};
    
    power_vsx_load_128(0, test_data1);
    power_vsx_load_128(1, test_data2);
    
    uint64_t start_time = power_get_timer_value();
    
    for (uint32_t i = 0; i < iterations; i++) {
        power_vsx_mul_128(2, 0, 1);
    }
    
    uint64_t end_time = power_get_timer_value();
    
    return end_time - start_time;
}

// ============================================================================
// VECTOR STATUS AND CONTROL
// ============================================================================

uint64_t power_vector_get_flags(void)
{
    return power_vector.vector_flags;
}

bool power_vector_is_vsx_enabled(void)
{
    return power_vector.vsx_enabled;
}

bool power_vector_is_altivec_enabled(void)
{
    return power_vector.altivec_enabled;
}

bool power_vector_is_vsx_256_enabled(void)
{
    return power_vector.vsx_256_enabled;
}

bool power_vector_is_vsx_512_enabled(void)
{
    return power_vector.vsx_512_enabled;
}

uint32_t power_vector_get_vsx_version(void)
{
    return power_vector.vsx_version;
}

uint32_t power_vector_get_altivec_version(void)
{
    return power_vector.altivec_version;
}

// ============================================================================
// VECTOR CLEANUP
// ============================================================================

void power_vector_cleanup(void)
{
    printf("POWER: Cleaning up vector support\n");
    
    // Clear vector registers
    memset(power_vsx_regs, 0, sizeof(power_vsx_regs));
    memset(power_altivec_regs, 0, sizeof(power_altivec_regs));
    
    // Clear vector context
    memset(&power_vector, 0, sizeof(power_vector));
    
    printf("POWER: Vector support cleaned up\n");
}
