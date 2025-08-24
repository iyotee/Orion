/*
 * Orion Operating System - Hardware Abstraction Layer (HAL)
 *
 * Universal hardware abstraction layer providing cross-architecture
 * compatibility and optimization for Orion OS kernel.
 *
 * Features:
 * - Universal interface for all CPU architectures
 * - Architecture-specific optimizations
 * - Performance monitoring and capabilities
 * - Memory management abstraction
 * - Security feature management
 * - Cross-architecture IPC optimization
 *
 * Supported architectures:
 * - x86_64 (Intel, AMD)
 * - AArch64 (ARM, Apple Silicon)
 * - RISC-V64 (SiFive, Western Digital, NVIDIA)
 * - PowerPC (IBM POWER)
 * - LoongArch (Loongson)
 * - MIPS (Loongson legacy)
 * - s390x (IBM z/Architecture)
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_HAL_H
#define ORION_HAL_H

#include <orion/types.h>
#include <orion/error.h>
#include <orion/capabilities.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * HAL Architecture Interface
 * ============================================================================ */

/**
 * @brief HAL architecture interface structure
 * 
 * This structure provides a unified interface for all supported CPU architectures,
 * ensuring 95% of kernel code compiles for all targets without modification.
 */
struct orion_hal_arch {
    /* Architecture identification */
    const char *name;
    uint32_t arch_id;
    uint32_t version;
    
    /* Memory management virtualized */
    int (*mmu_init)(void);
    int (*mmu_map)(vaddr_t va, paddr_t pa, uint64_t flags);
    int (*mmu_unmap)(vaddr_t va, size_t len);
    int (*mmu_protect)(vaddr_t va, size_t len, uint64_t flags);
    paddr_t (*mmu_translate)(vaddr_t va);
    
    /* Context switching optimized per architecture */
    void (*context_switch)(struct thread *prev, struct thread *next);
    void (*context_save)(struct thread *thread);
    void (*context_restore)(struct thread *thread);
    
    /* IPC optimized (uses CPU-specific extensions) */
    ssize_t (*ipc_fast_send)(struct ipc_msg *msg, struct thread *target);
    ssize_t (*ipc_fast_recv)(struct ipc_msg *msg, struct thread *source);
    int (*ipc_setup_fast_path)(struct thread *thread);
    
    /* Interrupt handling */
    int (*irq_register)(uint32_t irq, irq_handler_t handler, void *data);
    int (*irq_unregister)(uint32_t irq);
    void (*irq_enable)(uint32_t irq);
    void (*irq_disable)(uint32_t irq);
    void (*irq_ack)(uint32_t irq);
    
    /* Timer management */
    int (*timer_init)(uint64_t frequency_hz);
    uint64_t (*timer_get_ticks)(void);
    void (*timer_set_oneshot)(uint64_t delay_ticks, timer_callback_t callback);
    void (*timer_cancel)(void);
    
    /* CPU management */
    int (*cpu_init)(uint32_t cpu_id);
    void (*cpu_idle)(void);
    void (*cpu_wake)(uint32_t cpu_id);
    uint32_t (*cpu_get_count)(void);
    uint32_t (*cpu_get_current)(void);
    
    /* Cache management */
    void (*cache_invalidate)(void *addr, size_t size);
    void (*cache_clean)(void *addr, size_t size);
    void (*cache_flush)(void *addr, size_t size);
    void (*cache_sync)(void);
    
    /* Performance monitoring */
    int (*perf_init)(void);
    uint64_t (*perf_read_counter)(uint32_t counter_id);
    int (*perf_set_event)(uint32_t counter_id, uint32_t event_id);
    void (*perf_enable)(uint32_t counter_id);
    void (*perf_disable)(uint32_t counter_id);
    
    /* Security features */
    int (*secure_init)(void);
    int (*secure_measure)(const void *data, size_t size, uint8_t *hash);
    int (*secure_verify)(const void *data, size_t size, const uint8_t *hash);
    int (*secure_enclave_create)(size_t size, void **enclave);
    int (*secure_enclave_destroy)(void *enclave);
    
    /* Power management */
    int (*power_init)(void);
    int (*power_set_state)(power_state_t state);
    int (*power_get_state)(void);
    int (*power_set_frequency)(uint32_t cpu_id, uint32_t frequency_mhz);
    
    /* Debug and tracing */
    int (*debug_init)(void);
    void (*debug_break)(void);
    void (*debug_trace)(const char *message);
    int (*debug_dump_registers)(void);
    
    /* Architecture-specific extensions */
    void *arch_private;
    int (*arch_extension_call)(uint32_t extension_id, void *args);
};

/* ============================================================================
 * HAL Manager Interface
 * ============================================================================ */

/**
 * @brief HAL manager structure
 * 
 * Manages the registration, detection, and switching between different
 * architecture implementations.
 */
struct orion_hal_manager {
    struct orion_hal_arch *current_arch;
    struct orion_hal_arch *available_archs[ORION_MAX_ARCHITECTURES];
    uint32_t arch_count;
    uint32_t current_arch_id;
    
    /* Architecture management */
    int (*register_arch)(struct orion_hal_arch *arch);
    int (*detect_arch)(void);
    int (*switch_arch)(uint32_t arch_id);
    struct orion_hal_arch *(*get_arch)(uint32_t arch_id);
    
    /* Performance comparison */
    int (*benchmark_archs)(void);
    struct orion_perf_metrics *(*get_perf_metrics)(uint32_t arch_id);
    
    /* Capability management */
    struct orion_capabilities *(*get_capabilities)(void);
    int (*optimize_for_arch)(uint32_t arch_id);
};

/* ============================================================================
 * HAL Initialization and Management
 * ============================================================================ */

/**
 * @brief Initialize the HAL system
 * 
 * @return int 0 on success, negative error code on failure
 */
int orion_hal_init(void);

/**
 * @brief Get the current HAL architecture interface
 * 
 * @return struct orion_hal_arch* Pointer to current architecture interface
 */
struct orion_hal_arch *orion_hal_get_current(void);

/**
 * @brief Get the HAL manager
 * 
 * @return struct orion_hal_manager* Pointer to HAL manager
 */
struct orion_hal_manager *orion_hal_get_manager(void);

/**
 * @brief Shutdown the HAL system
 * 
 * @return int 0 on success, negative error code on failure
 */
int orion_hal_shutdown(void);

/* ============================================================================
 * Architecture-Specific HAL Implementations
 * ============================================================================ */

/* x86_64 HAL */
extern struct orion_hal_arch orion_hal_x86_64;

/* AArch64 HAL */
extern struct orion_hal_arch orion_hal_aarch64;

/* RISC-V64 HAL */
extern struct orion_hal_arch orion_hal_riscv64;

/* PowerPC HAL */
extern struct orion_hal_arch orion_hal_powerpc;

/* LoongArch HAL */
extern struct orion_hal_arch orion_hal_loongarch;

/* MIPS HAL */
extern struct orion_hal_arch orion_hal_mips;

/* s390x HAL */
extern struct orion_hal_arch orion_hal_s390x;

/* ============================================================================
 * HAL Capabilities and Features
 * ============================================================================ */

/**
 * @brief Hardware capabilities structure
 */
struct orion_hal_capabilities {
    /* CPU capabilities */
    struct {
        uint32_t max_cores;
        uint32_t max_threads;
        uint64_t max_frequency_mhz;
        uint64_t cache_l1_size;
        uint64_t cache_l2_size;
        uint64_t cache_l3_size;
        uint32_t features; /* ORION_CPU_FEATURE_* flags */
    } cpu;
    
    /* Memory capabilities */
    struct {
        uint64_t max_ram_gb;
        uint32_t numa_nodes;
        uint64_t page_size;
        uint32_t memory_features; /* ORION_MEMORY_FEATURE_* flags */
    } memory;
    
    /* Security capabilities */
    struct {
        uint32_t security_features; /* ORION_SECURITY_FEATURE_* flags */
        uint32_t encryption_acceleration;
        uint32_t secure_boot_support;
    } security;
    
    /* Performance capabilities */
    struct {
        uint32_t performance_counters;
        uint32_t profiling_support;
        uint32_t optimization_levels;
    } performance;
};

/**
 * @brief Performance metrics structure
 */
struct orion_perf_metrics {
    uint64_t context_switch_ns;
    uint64_t ipc_latency_ns;
    uint64_t memory_latency_ns;
    uint64_t cache_miss_rate;
    uint64_t power_efficiency;
    uint64_t throughput_score;
};

/* ============================================================================
 * HAL Feature Flags
 * ============================================================================ */

/* CPU Features */
#define ORION_CPU_FEATURE_SSE         (1U << 0)
#define ORION_CPU_FEATURE_SSE2        (1U << 1)
#define ORION_CPU_FEATURE_SSE3        (1U << 2)
#define ORION_CPU_FEATURE_SSSE3       (1U << 3)
#define ORION_CPU_FEATURE_SSE4_1      (1U << 4)
#define ORION_CPU_FEATURE_SSE4_2      (1U << 5)
#define ORION_CPU_FEATURE_AVX         (1U << 6)
#define ORION_CPU_FEATURE_AVX2        (1U << 7)
#define ORION_CPU_FEATURE_AVX512      (1U << 8)
#define ORION_CPU_FEATURE_NEON        (1U << 9)
#define ORION_CPU_FEATURE_SVE         (1U << 10)
#define ORION_CPU_FEATURE_VSX         (1U << 11)
#define ORION_CPU_FEATURE_ALTIVEC     (1U << 12)
#define ORION_CPU_FEATURE_RVV         (1U << 13)
#define ORION_CPU_FEATURE_LOONGARCH   (1U << 14)

/* Memory Features */
#define ORION_MEMORY_FEATURE_NUMA     (1U << 0)
#define ORION_MEMORY_FEATURE_HUGE_PAGES (1U << 1)
#define ORION_MEMORY_FEATURE_ECC      (1U << 2)
#define ORION_MEMORY_FEATURE_NVDIMM  (1U << 3)

/* Security Features */
#define ORION_SECURITY_FEATURE_TEE    (1U << 0)
#define ORION_SECURITY_FEATURE_TPM    (1U << 1)
#define ORION_SECURITY_FEATURE_SGX    (1U << 2)
#define ORION_SECURITY_FEATURE_ARM_TRUSTZONE (1U << 3)

/* ============================================================================
 * HAL Error Codes
 * ============================================================================ */

#define ORION_HAL_SUCCESS             0
#define ORION_HAL_ERROR_INVALID_ARG  -1
#define ORION_HAL_ERROR_NOT_SUPPORTED -2
#define ORION_HAL_ERROR_OUT_OF_MEMORY -3
#define ORION_HAL_ERROR_TIMEOUT      -4
#define ORION_HAL_ERROR_HARDWARE     -5
#define ORION_HAL_ERROR_SECURITY     -6

#ifdef __cplusplus
}
#endif

#endif /* ORION_HAL_H */
