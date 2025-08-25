/*
 * ORION OS - RISC-V 64-bit NUMA Management
 *
 * NUMA topology management for RISC-V 64-bit architecture
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
// GLOBAL NUMA CONTEXTS
// ============================================================================

// NUMA topology context
static struct
{
    bool initialized;
    bool numa_enabled;
    bool numa_optimization_enabled;
    bool numa_monitoring_enabled;
    uint32_t numa_node_count;
    uint32_t current_numa_node;
    uint32_t preferred_numa_node;
    uint64_t numa_topology_version;
} riscv64_numa_context = {0};

// NUMA node information
static struct
{
    uint32_t node_id;
    uint32_t cpu_count;
    uint32_t memory_size_mb;
    uint32_t memory_bandwidth;
    uint32_t cpu_affinity;
    uint64_t local_memory_start;
    uint64_t local_memory_end;
    bool is_local;
} riscv64_numa_nodes[RISCV64_MAX_NUMA_NODES] = {0};

// NUMA statistics
static struct
{
    uint32_t total_numa_operations;
    uint32_t numa_memory_allocations;
    uint32_t numa_cpu_migrations;
    uint32_t numa_cache_misses;
    uint32_t numa_bandwidth_usage;
    uint64_t total_numa_latency;
    uint64_t average_numa_latency;
} riscv64_numa_stats = {0};

// NUMA configuration
static struct
{
    uint32_t numa_policy;
    uint32_t numa_balance_threshold;
    uint32_t numa_migration_delay;
    bool numa_auto_balancing;
    bool numa_memory_interleaving;
} riscv64_numa_config = {0};

// ============================================================================
// NUMA INITIALIZATION
// ============================================================================

// Initialize NUMA management system
void riscv64_numa_init(void)
{
    if (riscv64_numa_context.initialized)
    {
        return;
    }

    // Initialize NUMA context with default values
    riscv64_numa_context.numa_enabled = true;
    riscv64_numa_context.numa_optimization_enabled = true;
    riscv64_numa_context.numa_monitoring_enabled = true;
    riscv64_numa_context.numa_node_count = 1; // Default to single node
    riscv64_numa_context.current_numa_node = 0;
    riscv64_numa_context.preferred_numa_node = 0;
    riscv64_numa_context.numa_topology_version = 1;
    riscv64_numa_context.initialized = true;

    // Initialize NUMA configuration
    riscv64_numa_config.numa_policy = RISCV64_NUMA_POLICY_LOCAL;
    riscv64_numa_config.numa_balance_threshold = RISCV64_NUMA_BALANCE_THRESHOLD;
    riscv64_numa_config.numa_migration_delay = RISCV64_NUMA_MIGRATION_DELAY;
    riscv64_numa_config.numa_auto_balancing = true;
    riscv64_numa_config.numa_memory_interleaving = false;

    // Clear NUMA statistics
    memset(&riscv64_numa_stats, 0, sizeof(riscv64_numa_stats));

    // Initialize default NUMA node
    riscv64_numa_nodes[0].node_id = 0;
    riscv64_numa_nodes[0].cpu_count = 1;
    riscv64_numa_nodes[0].memory_size_mb = 1024;   // Default 1GB
    riscv64_numa_nodes[0].memory_bandwidth = 1000; // Default 1GB/s
    riscv64_numa_nodes[0].cpu_affinity = 0;
    riscv64_numa_nodes[0].local_memory_start = 0x80000000; // Default start address
    riscv64_numa_nodes[0].local_memory_end = 0x84000000;   // Default end address
    riscv64_numa_nodes[0].is_local = true;

    printf("RISC-V64: NUMA management system initialized\n");
    printf("  NUMA Enabled: %s\n", riscv64_numa_context.numa_enabled ? "Yes" : "No");
    printf("  NUMA Optimization: %s\n", riscv64_numa_context.numa_optimization_enabled ? "Enabled" : "Disabled");
    printf("  NUMA Monitoring: %s\n", riscv64_numa_context.numa_monitoring_enabled ? "Enabled" : "Disabled");
    printf("  NUMA Node Count: %u\n", riscv64_numa_context.numa_node_count);
    printf("  Current NUMA Node: %u\n", riscv64_numa_context.current_numa_node);
}

// ============================================================================
// NUMA CONTEXT FUNCTIONS
// ============================================================================

// Get NUMA management context
struct riscv64_numa_context *riscv64_get_numa_context(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return NULL;
    }

    return &riscv64_numa_context;
}

// Enable NUMA management
void riscv64_numa_enable(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return;
    }

    riscv64_numa_context.numa_enabled = true;

    printf("RISC-V64: NUMA management enabled\n");
}

// Disable NUMA management
void riscv64_numa_disable(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return;
    }

    riscv64_numa_context.numa_enabled = false;

    printf("RISC-V64: NUMA management disabled\n");
}

// Check if NUMA management is enabled
bool riscv64_numa_is_enabled(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return false;
    }

    return riscv64_numa_context.numa_enabled;
}

// Enable NUMA optimization
void riscv64_numa_optimization_enable(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return;
    }

    riscv64_numa_context.numa_optimization_enabled = true;

    printf("RISC-V64: NUMA optimization enabled\n");
}

// Disable NUMA optimization
void riscv64_numa_optimization_disable(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return;
    }

    riscv64_numa_context.numa_optimization_enabled = false;

    printf("RISC-V64: NUMA optimization disabled\n");
}

// Check if NUMA optimization is enabled
bool riscv64_numa_optimization_is_enabled(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return false;
    }

    return riscv64_numa_context.numa_optimization_enabled;
}

// Enable NUMA monitoring
void riscv64_numa_monitoring_enable(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return;
    }

    riscv64_numa_context.numa_monitoring_enabled = true;

    printf("RISC-V64: NUMA monitoring enabled\n");
}

// Disable NUMA monitoring
void riscv64_numa_monitoring_disable(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return;
    }

    riscv64_numa_context.numa_monitoring_enabled = false;

    printf("RISC-V64: NUMA monitoring disabled\n");
}

// Check if NUMA monitoring is enabled
bool riscv64_numa_monitoring_is_enabled(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return false;
    }

    return riscv64_numa_context.numa_monitoring_enabled;
}

// ============================================================================
// NUMA TOPOLOGY MANAGEMENT
// ============================================================================

// Detect NUMA topology
int riscv64_numa_detect_topology(void)
{
    if (!riscv64_numa_context.initialized || !riscv64_numa_context.numa_enabled)
    {
        return RISCV64_ERROR_NOT_ENABLED;
    }

    printf("RISC-V64: Detecting NUMA topology\n");

    // For now, assume single node topology
    // In a real system, this would detect actual hardware topology
    riscv64_numa_context.numa_node_count = 1;
    riscv64_numa_context.current_numa_node = 0;
    riscv64_numa_context.preferred_numa_node = 0;

    // Update topology version
    riscv64_numa_context.numa_topology_version++;

    printf("RISC-V64: NUMA topology detected: %u nodes\n", riscv64_numa_context.numa_node_count);

    return RISCV64_SUCCESS;
}

// Get NUMA node count
uint32_t riscv64_numa_get_node_count(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return 0;
    }

    return riscv64_numa_context.numa_node_count;
}

// Get current NUMA node
uint32_t riscv64_numa_get_current_node(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return RISCV64_INVALID_NUMA_NODE;
    }

    return riscv64_numa_context.current_numa_node;
}

// Set current NUMA node
int riscv64_numa_set_current_node(uint32_t node_id)
{
    if (!riscv64_numa_context.initialized)
    {
        return RISCV64_ERROR_NOT_INITIALIZED;
    }

    if (node_id >= riscv64_numa_context.numa_node_count)
    {
        return RISCV64_ERROR_INVALID_PARAMETER;
    }

    riscv64_numa_context.current_numa_node = node_id;

    printf("RISC-V64: Current NUMA node set to %u\n", node_id);

    return RISCV64_SUCCESS;
}

// Get preferred NUMA node
uint32_t riscv64_numa_get_preferred_node(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return RISCV64_INVALID_NUMA_NODE;
    }

    return riscv64_numa_context.preferred_numa_node;
}

// Set preferred NUMA node
int riscv64_numa_set_preferred_node(uint32_t node_id)
{
    if (!riscv64_numa_context.initialized)
    {
        return RISCV64_ERROR_NOT_INITIALIZED;
    }

    if (node_id >= riscv64_numa_context.numa_node_count)
    {
        return RISCV64_ERROR_INVALID_PARAMETER;
    }

    riscv64_numa_context.preferred_numa_node = node_id;

    printf("RISC-V64: Preferred NUMA node set to %u\n", node_id);

    return RISCV64_SUCCESS;
}

// Get NUMA topology version
uint64_t riscv64_numa_get_topology_version(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return 0;
    }

    return riscv64_numa_context.numa_topology_version;
}

// ============================================================================
// NUMA NODE OPERATIONS
// ============================================================================

// Get NUMA node information
struct riscv64_numa_node *riscv64_numa_get_node_info(uint32_t node_id)
{
    if (!riscv64_numa_context.initialized)
    {
        return NULL;
    }

    if (node_id >= riscv64_numa_context.numa_node_count)
    {
        return NULL;
    }

    return &riscv64_numa_nodes[node_id];
}

// Get NUMA node by CPU ID
uint32_t riscv64_numa_get_node_by_cpu(uint32_t cpu_id)
{
    if (!riscv64_numa_context.initialized)
    {
        return RISCV64_INVALID_NUMA_NODE;
    }

    // For now, assume all CPUs belong to node 0
    // In a real system, this would check CPU affinity
    return 0;
}

// Get NUMA node by memory address
uint32_t riscv64_numa_get_node_by_address(uint64_t address)
{
    if (!riscv64_numa_context.initialized)
    {
        return RISCV64_INVALID_NUMA_NODE;
    }

    // Check which node the address belongs to
    for (uint32_t i = 0; i < riscv64_numa_context.numa_node_count; i++)
    {
        if (address >= riscv64_numa_nodes[i].local_memory_start &&
            address < riscv64_numa_nodes[i].local_memory_end)
        {
            return i;
        }
    }

    // Address not found in any node
    return RISCV64_INVALID_NUMA_NODE;
}

// Check if NUMA node is local
bool riscv64_numa_is_local_node(uint32_t node_id)
{
    if (!riscv64_numa_context.initialized)
    {
        return false;
    }

    if (node_id >= riscv64_numa_context.numa_node_count)
    {
        return false;
    }

    return (node_id == riscv64_numa_context.current_numa_node);
}

// Get local NUMA node
uint32_t riscv64_numa_get_local_node(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return RISCV64_INVALID_NUMA_NODE;
    }

    return riscv64_numa_context.current_numa_node;
}

// ============================================================================
// NUMA MEMORY MANAGEMENT
// ============================================================================

// Allocate memory on specific NUMA node
void *riscv64_numa_alloc_on_node(size_t size, uint32_t node_id)
{
    if (!riscv64_numa_context.initialized || !riscv64_numa_context.numa_enabled)
    {
        return NULL;
    }

    if (node_id >= riscv64_numa_context.numa_node_count)
    {
        return NULL;
    }

    // For now, use standard malloc
    // In a real system, this would allocate from the specific NUMA node
    void *ptr = malloc(size);

    if (ptr != NULL)
    {
        riscv64_numa_stats.numa_memory_allocations++;
        riscv64_numa_stats.total_numa_operations++;

        printf("RISC-V64: Allocated %zu bytes on NUMA node %u\n", size, node_id);
    }

    return ptr;
}

// Allocate memory on local NUMA node
void *riscv64_numa_alloc_local(size_t size)
{
    return riscv64_numa_alloc_on_node(size, riscv64_numa_context.current_numa_node);
}

// Allocate memory with interleaving across NUMA nodes
void *riscv64_numa_alloc_interleaved(size_t size)
{
    if (!riscv64_numa_context.initialized || !riscv64_numa_context.numa_enabled)
    {
        return NULL;
    }

    if (!riscv64_numa_config.numa_memory_interleaving)
    {
        return riscv64_numa_alloc_local(size);
    }

    // For now, use standard malloc
    // In a real system, this would interleave memory across nodes
    void *ptr = malloc(size);

    if (ptr != NULL)
    {
        riscv64_numa_stats.numa_memory_allocations++;
        riscv64_numa_stats.total_numa_operations++;

        printf("RISC-V64: Allocated %zu bytes with NUMA interleaving\n", size);
    }

    return ptr;
}

// Free NUMA memory
void riscv64_numa_free(void *ptr)
{
    if (ptr != NULL)
    {
        free(ptr);
        printf("RISC-V64: Freed NUMA memory\n");
    }
}

// Get NUMA memory statistics
void riscv64_numa_get_memory_stats(uint32_t *allocations, uint64_t *total_size)
{
    if (allocations)
        *allocations = riscv64_numa_stats.numa_memory_allocations;
    if (total_size)
        *total_size = 0; // Would track actual allocated size
}

// ============================================================================
// NUMA CPU MANAGEMENT
// ============================================================================

// Migrate CPU to specific NUMA node
int riscv64_numa_migrate_cpu(uint32_t cpu_id, uint32_t node_id)
{
    if (!riscv64_numa_context.initialized || !riscv64_numa_context.numa_enabled)
    {
        return RISCV64_ERROR_NOT_ENABLED;
    }

    if (node_id >= riscv64_numa_context.numa_node_count)
    {
        return RISCV64_ERROR_INVALID_PARAMETER;
    }

    // For now, just update statistics
    // In a real system, this would actually migrate the CPU
    riscv64_numa_stats.numa_cpu_migrations++;
    riscv64_numa_stats.total_numa_operations++;

    printf("RISC-V64: Migrated CPU %u to NUMA node %u\n", cpu_id, node_id);

    return RISCV64_SUCCESS;
}

// Get CPU NUMA affinity
uint32_t riscv64_numa_get_cpu_affinity(uint32_t cpu_id)
{
    if (!riscv64_numa_context.initialized)
    {
        return RISCV64_INVALID_NUMA_NODE;
    }

    // For now, assume all CPUs have affinity to node 0
    // In a real system, this would return actual CPU affinity
    return 0;
}

// Set CPU NUMA affinity
int riscv64_numa_set_cpu_affinity(uint32_t cpu_id, uint32_t node_id)
{
    if (!riscv64_numa_context.initialized)
    {
        return RISCV64_ERROR_NOT_INITIALIZED;
    }

    if (node_id >= riscv64_numa_context.numa_node_count)
    {
        return RISCV64_ERROR_INVALID_PARAMETER;
    }

    // For now, just update statistics
    // In a real system, this would set actual CPU affinity
    printf("RISC-V64: Set CPU %u affinity to NUMA node %u\n", cpu_id, node_id);

    return RISCV64_SUCCESS;
}

// ============================================================================
// NUMA OPTIMIZATION
// ============================================================================

// Optimize NUMA placement
int riscv64_numa_optimize_placement(void)
{
    if (!riscv64_numa_context.initialized || !riscv64_numa_context.numa_optimization_enabled)
    {
        return RISCV64_ERROR_NOT_ENABLED;
    }

    printf("RISC-V64: Optimizing NUMA placement\n");

    // For now, just update statistics
    // In a real system, this would perform actual optimization
    riscv64_numa_stats.total_numa_operations++;

    return RISCV64_SUCCESS;
}

// Balance NUMA load
int riscv64_numa_balance_load(void)
{
    if (!riscv64_numa_context.initialized || !riscv64_numa_context.numa_optimization_enabled)
    {
        return RISCV64_ERROR_NOT_ENABLED;
    }

    printf("RISC-V64: Balancing NUMA load\n");

    // For now, just update statistics
    // In a real system, this would perform actual load balancing
    riscv64_numa_stats.total_numa_operations++;

    return RISCV64_SUCCESS;
}

// Check if NUMA balancing is needed
bool riscv64_numa_balancing_needed(void)
{
    if (!riscv64_numa_context.initialized || !riscv64_numa_context.numa_auto_balancing)
    {
        return false;
    }

    // For now, always return false
    // In a real system, this would check actual load imbalance
    return false;
}

// ============================================================================
// NUMA MONITORING
// ============================================================================

// Get NUMA cache miss rate
uint32_t riscv64_numa_get_cache_miss_rate(void)
{
    if (!riscv64_numa_context.initialized || !riscv64_numa_context.numa_monitoring_enabled)
    {
        return 0;
    }

    // For now, return a placeholder value
    // In a real system, this would read from performance counters
    return 5; // 5% cache miss rate
}

// Get NUMA bandwidth usage
uint32_t riscv64_numa_get_bandwidth_usage(void)
{
    if (!riscv64_numa_context.initialized || !riscv64_numa_context.numa_monitoring_enabled)
    {
        return 0;
    }

    // For now, return a placeholder value
    // In a real system, this would read from bandwidth counters
    return 60; // 60% bandwidth usage
}

// Get NUMA latency
uint64_t riscv64_numa_get_latency(uint32_t from_node, uint32_t to_node)
{
    if (!riscv64_numa_context.initialized || !riscv64_numa_context.numa_monitoring_enabled)
    {
        return 0;
    }

    if (from_node >= riscv64_numa_context.numa_node_count ||
        to_node >= riscv64_numa_context.numa_node_count)
    {
        return 0;
    }

    // For now, return placeholder values
    // In a real system, this would measure actual latency
    if (from_node == to_node)
    {
        return 100; // Local access: 100ns
    }
    else
    {
        return 500; // Remote access: 500ns
    }
}

// Update NUMA statistics
void riscv64_numa_update_stats(void)
{
    if (!riscv64_numa_context.initialized || !riscv64_numa_context.numa_monitoring_enabled)
    {
        return;
    }

    // Update cache miss statistics
    uint32_t cache_miss_rate = riscv64_numa_get_cache_miss_rate();
    if (cache_miss_rate > riscv64_numa_config.numa_balance_threshold)
    {
        riscv64_numa_stats.numa_cache_misses++;
    }

    // Update bandwidth statistics
    uint32_t bandwidth_usage = riscv64_numa_get_bandwidth_usage();
    if (bandwidth_usage > 80)
    { // High bandwidth usage
        riscv64_numa_stats.numa_bandwidth_usage++;
    }

    // Update latency statistics
    uint64_t total_latency = 0;
    uint32_t latency_count = 0;

    for (uint32_t i = 0; i < riscv64_numa_context.numa_node_count; i++)
    {
        for (uint32_t j = 0; j < riscv64_numa_context.numa_node_count; j++)
        {
            uint64_t latency = riscv64_numa_get_latency(i, j);
            total_latency += latency;
            latency_count++;
        }
    }

    if (latency_count > 0)
    {
        riscv64_numa_stats.total_numa_latency = total_latency;
        riscv64_numa_stats.average_numa_latency = total_latency / latency_count;
    }
}

// ============================================================================
// NUMA CONFIGURATION
// ============================================================================

// Set NUMA policy
void riscv64_numa_set_policy(uint32_t policy)
{
    if (!riscv64_numa_context.initialized)
    {
        return;
    }

    if (policy < RISCV64_NUMA_POLICY_MAX)
    {
        riscv64_numa_config.numa_policy = policy;
        printf("RISC-V64: NUMA policy set to %u\n", policy);
    }
}

// Get NUMA policy
uint32_t riscv64_numa_get_policy(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return RISCV64_NUMA_POLICY_DEFAULT;
    }

    return riscv64_numa_config.numa_policy;
}

// Set NUMA balance threshold
void riscv64_numa_set_balance_threshold(uint32_t threshold)
{
    if (!riscv64_numa_context.initialized)
    {
        return;
    }

    riscv64_numa_config.numa_balance_threshold = threshold;
    printf("RISC-V64: NUMA balance threshold set to %u\n", threshold);
}

// Get NUMA balance threshold
uint32_t riscv64_numa_get_balance_threshold(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return 0;
    }

    return riscv64_numa_config.numa_balance_threshold;
}

// Enable auto NUMA balancing
void riscv64_numa_auto_balancing_enable(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return;
    }

    riscv64_numa_config.numa_auto_balancing = true;
    printf("RISC-V64: Auto NUMA balancing enabled\n");
}

// Disable auto NUMA balancing
void riscv64_numa_auto_balancing_disable(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return;
    }

    riscv64_numa_config.numa_auto_balancing = false;
    printf("RISC-V64: Auto NUMA balancing disabled\n");
}

// Check if auto NUMA balancing is enabled
bool riscv64_numa_auto_balancing_is_enabled(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return false;
    }

    return riscv64_numa_config.numa_auto_balancing;
}

// Enable NUMA memory interleaving
void riscv64_numa_memory_interleaving_enable(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return;
    }

    riscv64_numa_config.numa_memory_interleaving = true;
    printf("RISC-V64: NUMA memory interleaving enabled\n");
}

// Disable NUMA memory interleaving
void riscv64_numa_memory_interleaving_disable(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return;
    }

    riscv64_numa_config.numa_memory_interleaving = false;
    printf("RISC-V64: NUMA memory interleaving disabled\n");
}

// Check if NUMA memory interleaving is enabled
bool riscv64_numa_memory_interleaving_is_enabled(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return false;
    }

    return riscv64_numa_config.numa_memory_interleaving;
}

// ============================================================================
// NUMA STATISTICS
// ============================================================================

// Get NUMA statistics
void riscv64_numa_get_stats(uint32_t *total_operations, uint32_t *memory_allocations,
                            uint32_t *cpu_migrations, uint32_t *cache_misses,
                            uint32_t *bandwidth_usage, uint64_t *total_latency,
                            uint64_t *average_latency)
{
    if (total_operations)
        *total_operations = riscv64_numa_stats.total_numa_operations;
    if (memory_allocations)
        *memory_allocations = riscv64_numa_stats.numa_memory_allocations;
    if (cpu_migrations)
        *cpu_migrations = riscv64_numa_stats.numa_cpu_migrations;
    if (cache_misses)
        *cache_misses = riscv64_numa_stats.numa_cache_misses;
    if (bandwidth_usage)
        *bandwidth_usage = riscv64_numa_stats.numa_bandwidth_usage;
    if (total_latency)
        *total_latency = riscv64_numa_stats.total_numa_latency;
    if (average_latency)
        *average_latency = riscv64_numa_stats.average_numa_latency;
}

// Reset NUMA statistics
void riscv64_numa_reset_stats(void)
{
    memset(&riscv64_numa_stats, 0, sizeof(riscv64_numa_stats));

    printf("RISC-V64: NUMA statistics reset\n");
}

// ============================================================================
// NUMA DEBUG FUNCTIONS
// ============================================================================

// Print NUMA status
void riscv64_numa_print_status(void)
{
    printf("RISC-V64: NUMA Status\n");
    printf("  Initialized: %s\n", riscv64_numa_context.initialized ? "Yes" : "No");
    printf("  NUMA Enabled: %s\n", riscv64_numa_context.numa_enabled ? "Yes" : "No");
    printf("  NUMA Optimization: %s\n", riscv64_numa_context.numa_optimization_enabled ? "Enabled" : "Disabled");
    printf("  NUMA Monitoring: %s\n", riscv64_numa_context.numa_monitoring_enabled ? "Enabled" : "Disabled");
    printf("  NUMA Node Count: %u\n", riscv64_numa_context.numa_node_count);
    printf("  Current NUMA Node: %u\n", riscv64_numa_context.current_numa_node);
    printf("  Preferred NUMA Node: %u\n", riscv64_numa_context.preferred_numa_node);
    printf("  Topology Version: %llu\n", riscv64_numa_context.numa_topology_version);
}

// Print NUMA configuration
void riscv64_numa_print_configuration(void)
{
    printf("RISC-V64: NUMA Configuration\n");
    printf("  NUMA Policy: %u\n", riscv64_numa_config.numa_policy);
    printf("  Balance Threshold: %u\n", riscv64_numa_config.numa_balance_threshold);
    printf("  Migration Delay: %u\n", riscv64_numa_config.numa_migration_delay);
    printf("  Auto Balancing: %s\n", riscv64_numa_config.numa_auto_balancing ? "Enabled" : "Disabled");
    printf("  Memory Interleaving: %s\n", riscv64_numa_config.numa_memory_interleaving ? "Enabled" : "Disabled");
}

// Print NUMA statistics
void riscv64_numa_print_statistics(void)
{
    printf("RISC-V64: NUMA Statistics\n");
    printf("  Total Operations: %u\n", riscv64_numa_stats.total_numa_operations);
    printf("  Memory Allocations: %u\n", riscv64_numa_stats.numa_memory_allocations);
    printf("  CPU Migrations: %u\n", riscv64_numa_stats.numa_cpu_migrations);
    printf("  Cache Misses: %u\n", riscv64_numa_stats.numa_cache_misses);
    printf("  Bandwidth Usage: %u\n", riscv64_numa_stats.numa_bandwidth_usage);
    printf("  Total Latency: %llu\n", riscv64_numa_stats.total_numa_latency);
    printf("  Average Latency: %llu\n", riscv64_numa_stats.average_numa_latency);
}

// Print NUMA topology
void riscv64_numa_print_topology(void)
{
    printf("RISC-V64: NUMA Topology\n");
    printf("  Node Count: %u\n", riscv64_numa_context.numa_node_count);

    for (uint32_t i = 0; i < riscv64_numa_context.numa_node_count; i++)
    {
        printf("  Node %u:\n", i);
        printf("    CPU Count: %u\n", riscv64_numa_nodes[i].cpu_count);
        printf("    Memory Size: %u MB\n", riscv64_numa_nodes[i].memory_size_mb);
        printf("    Memory Bandwidth: %u MB/s\n", riscv64_numa_nodes[i].memory_bandwidth);
        printf("    Local Memory: 0x%llx - 0x%llx\n",
               riscv64_numa_nodes[i].local_memory_start,
               riscv64_numa_nodes[i].local_memory_end);
        printf("    Is Local: %s\n", riscv64_numa_nodes[i].is_local ? "Yes" : "No");
    }
}

// ============================================================================
// NUMA CLEANUP
// ============================================================================

// Cleanup NUMA management system
void riscv64_numa_cleanup(void)
{
    if (!riscv64_numa_context.initialized)
    {
        return;
    }

    // Clear contexts
    riscv64_numa_context.initialized = false;

    printf("RISC-V64: NUMA management system cleaned up\n");
}
