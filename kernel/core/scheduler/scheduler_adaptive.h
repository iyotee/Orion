/*
 * Orion Operating System - Intelligent Adaptive Scheduler
 *
 * Enhanced scheduler with per-architecture optimizations:
 * - Apple Silicon: unified memory, P/E cores, Neural Engine
 * - AMD: Infinity Fabric, SME/SEV, Precision Boost
 * - Intel: Thread Director, TSX, advanced virtualization
 * - Universal: adaptive policies and performance monitoring
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_SCHEDULER_ADAPTIVE_H
#define ORION_SCHEDULER_ADAPTIVE_H

#include <orion/types.h>
#include <orion/kernel.h>
#include <orion/structures.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* ============================================================================
     * Architecture-Specific Scheduler Features
     * ============================================================================ */

    /**
     * @brief Architecture-specific scheduler policies
     */
    typedef enum
    {
        SCHED_POLICY_UNIVERSAL,     // Generic policy for all architectures
        SCHED_POLICY_APPLE_SILICON, // Apple Silicon optimizations
        SCHED_POLICY_AMD,           // AMD optimizations
        SCHED_POLICY_INTEL,         // Intel optimizations
        SCHED_POLICY_ARM64,         // ARM64 optimizations
        SCHED_POLICY_RISCV,         // RISC-V optimizations
        SCHED_POLICY_POWERPC,       // PowerPC optimizations
        SCHED_POLICY_LOONGARCH,     // LoongArch optimizations
        SCHED_POLICY_MIPS,          // MIPS optimizations
        SCHED_POLICY_S390X,         // S390x optimizations
    } sched_policy_t;

    /**
     * @brief CPU core types for heterogeneous architectures
     */
    typedef enum
    {
        CORE_TYPE_UNKNOWN,     // Unknown core type
        CORE_TYPE_PERFORMANCE, // Performance core (P-core)
        CORE_TYPE_EFFICIENCY,  // Efficiency core (E-core)
        CORE_TYPE_HYBRID,      // Hybrid core (can switch modes)
        CORE_TYPE_NEURAL,      // Neural/ML acceleration core
        CORE_TYPE_SECURITY,    // Security/trusted core
    } core_type_t;

    /**
     * @brief Thread affinity preferences
     */
    typedef enum
    {
        AFFINITY_NONE,        // No specific affinity
        AFFINITY_PERFORMANCE, // Prefer performance cores
        AFFINITY_EFFICIENCY,  // Prefer efficiency cores
        AFFINITY_NEURAL,      // Prefer neural cores
        AFFINITY_SECURITY,    // Prefer security cores
        AFFINITY_NUMA_LOCAL,  // Prefer local NUMA node
        AFFINITY_CACHE_LOCAL, // Prefer cache-local cores
    } affinity_preference_t;

    /**
     * @brief Architecture-specific CPU information
     */
    typedef struct
    {
        uint32_t cpu_id;              // CPU ID
        core_type_t core_type;        // Core type
        uint32_t numa_node;           // NUMA node
        uint32_t cache_level;         // Cache level (L1, L2, L3)
        uint64_t base_frequency;      // Base frequency in Hz
        uint64_t max_frequency;       // Maximum frequency in Hz
        uint32_t power_efficiency;    // Power efficiency rating (0-100)
        bool supports_hyperthreading; // Hyper-threading support
        bool supports_turbo;          // Turbo boost support
        uint64_t cache_size;          // Cache size in bytes
        uint32_t vector_width;        // SIMD vector width
        bool supports_avx;            // AVX support
        bool supports_neon;           // NEON support
        bool supports_sve;            // SVE support
    } arch_cpu_info_t;

    /**
     * @brief Enhanced thread structure with architecture optimizations
     */
    typedef struct
    {
        // Basic thread information
        thread_t *thread; // Pointer to base thread

        // Architecture-specific optimizations
        sched_policy_t policy;          // Scheduler policy
        affinity_preference_t affinity; // Affinity preference
        uint32_t preferred_cpu;         // Preferred CPU ID
        uint32_t last_cpu;              // Last CPU this thread ran on

        // Performance characteristics
        uint64_t avg_runtime;        // Average runtime per quantum
        uint64_t avg_sleep_time;     // Average sleep time
        uint32_t cache_misses;       // Cache miss count
        uint32_t branch_mispredicts; // Branch misprediction count

        // Power management
        uint32_t power_profile;   // Power profile (0-100)
        bool prefers_efficiency;  // Prefer efficiency cores
        uint64_t energy_consumed; // Energy consumed in Joules

        // Machine learning hints
        bool is_ml_workload;     // Machine learning workload
        bool uses_neural_engine; // Uses neural engine
        uint32_t ml_priority;    // ML workload priority

        // Security requirements
        bool requires_secure_core; // Requires security core
        bool uses_trustzone;       // Uses TrustZone
        uint32_t security_level;   // Security level (0-100)
    } adaptive_thread_t;

    /**
     * @brief Enhanced runqueue with architecture optimizations
     */
    typedef struct
    {
        // Base runqueue information
        cpu_runqueue_t *base_rq; // Base runqueue

        // Architecture-specific optimizations
        sched_policy_t policy; // Scheduler policy for this CPU
        core_type_t core_type; // Core type
        uint32_t numa_node;    // NUMA node

        // Performance monitoring
        uint64_t total_runtime;    // Total runtime on this CPU
        uint64_t total_sleep_time; // Total sleep time
        uint32_t context_switches; // Context switch count
        uint32_t migrations;       // Thread migration count

        // Power management
        uint64_t energy_consumed;   // Energy consumed on this CPU
        uint32_t current_frequency; // Current CPU frequency
        uint32_t power_state;       // Current power state

        // Cache and memory statistics
        uint32_t cache_hits;       // Cache hit count
        uint32_t cache_misses;     // Cache miss count
        uint64_t memory_bandwidth; // Memory bandwidth usage
    } adaptive_runqueue_t;

    /* ============================================================================
     * Architecture-Specific Scheduler Functions
     * ============================================================================ */

    /**
     * @brief Initialize the adaptive scheduler
     * @return OR_OK on success, error code on failure
     */
    int adaptive_scheduler_init(void);

    /**
     * @brief Detect architecture and set appropriate scheduler policy
     * @return OR_OK on success, error code on failure
     */
    int adaptive_scheduler_detect_architecture(void);

    /**
     * @brief Get architecture-specific scheduler policy
     * @return Scheduler policy for current architecture
     */
    sched_policy_t adaptive_scheduler_get_policy(void);

    /**
     * @brief Set scheduler policy for specific architecture
     * @param policy Scheduler policy to set
     * @return OR_OK on success, error code on failure
     */
    int adaptive_scheduler_set_policy(sched_policy_t policy);

    /**
     * @brief Get CPU information for architecture optimizations
     * @param cpu_id CPU ID
     * @param info Output CPU information
     * @return OR_OK on success, error code on failure
     */
    int adaptive_scheduler_get_cpu_info(uint32_t cpu_id, arch_cpu_info_t *info);

    /**
     * @brief Create adaptive thread with architecture optimizations
     * @param thread Base thread
     * @param policy Scheduler policy
     * @param affinity Affinity preference
     * @return Adaptive thread or NULL on failure
     */
    adaptive_thread_t *adaptive_scheduler_create_thread(thread_t *thread,
                                                        sched_policy_t policy,
                                                        affinity_preference_t affinity);

    /**
     * @brief Destroy adaptive thread
     * @param at Adaptive thread to destroy
     */
    void adaptive_scheduler_destroy_thread(adaptive_thread_t *at);

    /**
     * @brief Schedule thread with architecture optimizations
     * @param at Adaptive thread to schedule
     * @return OR_OK on success, error code on failure
     */
    int adaptive_scheduler_schedule_thread(adaptive_thread_t *at);

    /**
     * @brief Get optimal CPU for thread based on architecture
     * @param at Adaptive thread
     * @return Optimal CPU ID or -1 on failure
     */
    int adaptive_scheduler_get_optimal_cpu(adaptive_thread_t *at);

    /**
     * @brief Migrate thread to optimal CPU
     * @param at Adaptive thread
     * @param target_cpu Target CPU ID
     * @return OR_OK on success, error code on failure
     */
    int adaptive_scheduler_migrate_thread(adaptive_thread_t *at, uint32_t target_cpu);

    /**
     * @brief Update thread performance characteristics
     * @param at Adaptive thread
     * @param runtime Runtime in nanoseconds
     * @param sleep_time Sleep time in nanoseconds
     * @param cache_misses Cache miss count
     * @param branch_mispredicts Branch misprediction count
     */
    void adaptive_scheduler_update_performance(adaptive_thread_t *at,
                                               uint64_t runtime,
                                               uint64_t sleep_time,
                                               uint32_t cache_misses,
                                               uint32_t branch_mispredicts);

    /**
     * @brief Get scheduler statistics
     * @param stats Output statistics structure
     * @return OR_OK on success, error code on failure
     */
    int adaptive_scheduler_get_stats(void *stats);

    /* ============================================================================
     * Apple Silicon Specific Functions
     * ============================================================================ */

    /**
     * @brief Initialize Apple Silicon scheduler optimizations
     * @return OR_OK on success, error code on failure
     */
    int apple_silicon_scheduler_init(void);

    /**
     * @brief Detect Apple Silicon P/E core configuration
     * @return OR_OK on success, error code on failure
     */
    int apple_silicon_detect_cores(void);

    /**
     * @brief Schedule thread on optimal Apple Silicon core
     * @param at Adaptive thread
     * @return OR_OK on success, error code on failure
     */
    int apple_silicon_schedule_thread(adaptive_thread_t *at);

    /**
     * @brief Use Neural Engine for ML workloads
     * @param at Adaptive thread
     * @return OR_OK on success, error code on failure
     */
    int apple_silicon_use_neural_engine(adaptive_thread_t *at);

    /* ============================================================================
     * AMD Specific Functions
     * ============================================================================ */

    /**
     * @brief Initialize AMD scheduler optimizations
     * @return OR_OK on success, error code on failure
     */
    int amd_scheduler_init(void);

    /**
     * @brief Detect AMD Infinity Fabric topology
     * @return OR_OK on success, error code on failure
     */
    int amd_detect_infinity_fabric(void);

    /**
     * @brief Optimize for AMD Precision Boost
     * @param at Adaptive thread
     * @return OR_OK on success, error code on failure
     */
    int amd_optimize_precision_boost(adaptive_thread_t *at);

    /**
     * @brief Use AMD SME/SEV for secure workloads
     * @param at Adaptive thread
     * @return OR_OK on success, error code on failure
     */
    int amd_use_sme_sev(adaptive_thread_t *at);

    /* ============================================================================
     * Intel Specific Functions
     * ============================================================================ */

    /**
     * @brief Initialize Intel scheduler optimizations
     * @return OR_OK on success, error code on failure
     */
    int intel_scheduler_init(void);

    /**
     * @brief Detect Intel Thread Director capabilities
     * @return OR_OK on success, error code on failure
     */
    int intel_detect_thread_director(void);

    /**
     * @brief Use Intel Thread Director for optimal scheduling
     * @param at Adaptive thread
     * @return OR_OK on success, error code on failure
     */
    int intel_use_thread_director(adaptive_thread_t *at);

    /**
     * @brief Optimize for Intel TSX transactions
     * @param at Adaptive thread
     * @return OR_OK on success, error code on failure
     */
    int intel_optimize_tsx(adaptive_thread_t *at);

#ifdef __cplusplus
}
#endif

#endif /* ORION_SCHEDULER_ADAPTIVE_H */
