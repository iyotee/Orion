/*
 * ORION OS - POWER Architecture Header
 *
 * Main header file for POWER architecture support (POWER8, POWER9, POWER10)
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_ARCH_POWER_H
#define ORION_ARCH_POWER_H

#include "config.h"
#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// POWER SPECIFIC CONSTANTS
// ============================================================================

/* Privilege Levels */
#define POWER_MODE_USER 0x00
#define POWER_MODE_HYPERVISOR 0x01
#define POWER_MODE_KERNEL 0x02
#define POWER_MODE_MACHINE 0x03

/* SPR (Special Purpose Register) Numbers */
#define POWER_SPR_XER 1
#define POWER_SPR_LR 8
#define POWER_SPR_CTR 9
#define POWER_SPR_DSCR 17
#define POWER_SPR_DSISR 18
#define POWER_SPR_DAR 19
#define POWER_SPR_PPR 896
#define POWER_SPR_AMR 13
#define POWER_SPR_UAMOR 13
#define POWER_SPR_DAWR 61
#define POWER_SPR_DAWRX 62
#define POWER_SPR_CIABR 1015
#define POWER_SPR_DAWR1 61
#define POWER_SPR_DAWRX1 62

/* MSR (Machine State Register) Bits */
#define POWER_MSR_EE 0x8000000000000000ULL
#define POWER_MSR_PR 0x4000000000000000ULL
#define POWER_MSR_FP 0x2000000000000000ULL
#define POWER_MSR_ME 0x1000000000000000ULL
#define POWER_MSR_FE0 0x0800000000000000ULL
#define POWER_MSR_SE 0x0400000000000000ULL
#define POWER_MSR_BE 0x0200000000000000ULL
#define POWER_MSR_FE1 0x0100000000000000ULL
#define POWER_MSR_IR 0x0080000000000000ULL
#define POWER_MSR_DR 0x0040000000000000ULL
#define POWER_MSR_PMM 0x0020000000000000ULL
#define POWER_MSR_RI 0x0010000000000000ULL
#define POWER_MSR_LE 0x0008000000000000ULL

/* HID0 (Hardware Implementation Dependent Register 0) Bits */
#define POWER_HID0_EMCP 0x80000000
#define POWER_HID0_EBA 0x40000000
#define POWER_HID0_BCLK 0x20000000
#define POWER_HID0_ECLK 0x10000000
#define POWER_HID0_PAR 0x08000000
#define POWER_HID0_CFG 0x04000000
#define POWER_HID0_ABE 0x02000000
#define POWER_HID0_ABEEP 0x01000000
#define POWER_HID0_L10 0x00800000
#define POWER_HID0_L13 0x00400000
#define POWER_HID0_HIGH_BAT 0x00200000
#define POWER_HID0_ICFI 0x00100000
#define POWER_HID0_DCFI 0x00080000
#define POWER_HID0_SGE 0x00040000
#define POWER_HID0_DCE 0x00020000
#define POWER_HID0_ICE 0x00010000
#define POWER_HID0_DPM 0x00008000
#define POWER_HID0_DPE 0x00004000

// ============================================================================
// DATA TYPES
// ============================================================================

typedef uint64_t power_vaddr_t;
typedef uint64_t power_paddr_t;
typedef uint64_t power_size_t;
typedef uint64_t power_off_t;

typedef struct {
    uint64_t vsr[64];  /* VSX registers */
    uint64_t vr[32];   /* AltiVec registers */
    uint64_t fpr[32];  /* FPU registers */
    uint64_t gpr[32];  /* General purpose registers */
    uint64_t cr;       /* Condition register */
    uint64_t xer;      /* Fixed-point exception register */
    uint64_t lr;       /* Link register */
    uint64_t ctr;      /* Count register */
    uint64_t msr;      /* Machine state register */
    uint64_t pc;       /* Program counter */
    uint64_t sp;       /* Stack pointer */
} power_cpu_context_t;

typedef struct {
    uint64_t satp;     /* Page table base */
    uint64_t asid;     /* Address space ID */
    uint64_t tlb_entries[POWER_TLB_ENTRIES];
    uint64_t tlb_tags[POWER_TLB_ENTRIES];
} power_mmu_context_t;

typedef struct {
    uint64_t vector;   /* Interrupt vector */
    uint64_t priority; /* Interrupt priority */
    uint64_t source;   /* Interrupt source */
    uint64_t status;   /* Interrupt status */
} power_interrupt_context_t;

typedef struct {
    uint64_t tb_freq;  /* Time base frequency */
    uint64_t dec_freq; /* Decrementer frequency */
    uint64_t dec_value; /* Decrementer value */
    uint64_t dec_auto_reload; /* Auto-reload value */
} power_timer_config_t;

typedef struct {
    uint64_t l1i_size; /* L1 instruction cache size */
    uint64_t l1d_size; /* L1 data cache size */
    uint64_t l2_size;  /* L2 cache size */
    uint64_t l3_size;  /* L3 cache size */
    uint64_t l1i_line; /* L1 instruction cache line size */
    uint64_t l1d_line; /* L1 data cache line size */
    uint64_t l2_line;  /* L2 cache line size */
    uint64_t l3_line;  /* L3 cache line size */
} power_cache_config_t;

typedef struct {
    uint64_t vsx_len;  /* VSX vector length */
    uint64_t vsx_elen; /* VSX element size */
    uint64_t vsx_regs; /* VSX register count */
    uint64_t altivec_len; /* AltiVec vector length */
    uint64_t altivec_elen; /* AltiVec element size */
    uint64_t altivec_regs; /* AltiVec register count */
} power_vector_config_t;

typedef struct {
    uint64_t pcr;      /* Processor compatibility register */
    uint64_t pcr_mask; /* PCR mask */
    uint64_t pcr_shift; /* PCR shift */
    uint64_t pcr_val;  /* PCR value */
} power_security_config_t;

typedef struct {
    uint64_t pmc[8];   /* Performance monitor counters */
    uint64_t pmc_overflow[8]; /* PMC overflow flags */
    uint64_t pmc_control[8]; /* PMC control registers */
} power_pmu_config_t;

typedef struct {
    uint64_t power_mode; /* Current power mode */
    uint64_t sleep_level; /* Sleep level */
    uint64_t wakeup_source; /* Wakeup source */
    uint64_t power_gate; /* Power gate status */
} power_power_state_t;

typedef struct {
    uint64_t node_id;  /* NUMA node ID */
    uint64_t distance[POWER_MAX_NODES]; /* Distance matrix */
    uint64_t local_memory; /* Local memory size */
    uint64_t remote_memory; /* Remote memory size */
} power_numa_topology_t;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

/* CPU Management */
int power_cpu_init(void);
int power_cpu_detect_features(void);
int power_cpu_set_frequency(uint64_t freq_mhz);
int power_cpu_get_frequency(uint64_t *freq_mhz);
int power_cpu_set_affinity(uint64_t cpu_id, uint64_t affinity);
int power_cpu_get_affinity(uint64_t cpu_id, uint64_t *affinity);
int power_cpu_idle(void);
int power_cpu_wakeup(uint64_t cpu_id);
int power_cpu_shutdown(uint64_t cpu_id);

/* MMU Management */
int power_mmu_init(void);
int power_mmu_map_page(power_vaddr_t vaddr, power_paddr_t paddr, uint64_t flags);
int power_mmu_unmap_page(power_vaddr_t vaddr);
int power_mmu_get_page(power_vaddr_t vaddr, power_paddr_t *paddr, uint64_t *flags);
int power_mmu_flush_tlb(void);
int power_mmu_flush_tlb_range(power_vaddr_t start, power_vaddr_t end);
int power_mmu_set_asid(uint64_t asid);
int power_mmu_get_asid(uint64_t *asid);

/* Interrupt Management */
int power_interrupt_init(void);
int power_interrupt_register(uint64_t irq, void (*handler)(void *), void *data);
int power_interrupt_unregister(uint64_t irq);
int power_interrupt_enable(uint64_t irq);
int power_interrupt_disable(uint64_t irq);
int power_interrupt_ack(uint64_t irq);
int power_interrupt_set_priority(uint64_t irq, uint64_t priority);
int power_interrupt_get_priority(uint64_t irq, uint64_t *priority);

/* Timer Management */
int power_timer_init(void);
int power_timer_set_frequency(uint64_t freq_hz);
int power_timer_get_frequency(uint64_t *freq_hz);
int power_timer_set_period(uint64_t period_ns);
int power_timer_get_period(uint64_t *period_ns);
int power_timer_start(void);
int power_timer_stop(void);
int power_timer_reset(void);
uint64_t power_timer_read(void);
uint64_t power_timer_read_tb(void);

/* Cache Management */
int power_cache_init(void);
int power_cache_flush_l1i(void);
int power_cache_flush_l1d(void);
int power_cache_flush_l2(void);
int power_cache_flush_l3(void);
int power_cache_flush_all(void);
int power_cache_invalidate_l1i(void);
int power_cache_invalidate_l1d(void);
int power_cache_invalidate_l2(void);
int power_cache_invalidate_l3(void);
int power_cache_invalidate_all(void);
int power_cache_sync(void);

/* Vector Extensions */
int power_vector_init(void);
int power_vsx_init(void);
int power_altivec_init(void);
int power_vector_test(void);
int power_vsx_test(void);
int power_altivec_test(void);

/* Security Features */
int power_security_init(void);
int power_secure_boot_verify(void);
int power_memory_protection_enable(void);
int power_memory_protection_disable(void);
int power_aslr_enable(void);
int power_aslr_disable(void);
int power_htm_enable(void);
int power_htm_disable(void);

/* Performance Monitoring */
int power_pmu_init(void);
int power_pmu_start_counter(uint64_t counter, uint64_t event);
int power_pmu_stop_counter(uint64_t counter);
int power_pmu_read_counter(uint64_t counter, uint64_t *value);
int power_pmu_reset_counter(uint64_t counter);
int power_pmu_set_overflow_handler(uint64_t counter, void (*handler)(void *), void *data);
uint64_t power_pmu_read_cycle_counter(void);

/* Power Management */
int power_power_init(void);
int power_power_set_mode(uint64_t mode);
int power_power_get_mode(uint64_t *mode);
int power_power_sleep(uint64_t level);
int power_power_wakeup(uint64_t source);
int power_power_gate_enable(uint64_t gate);
int power_power_gate_disable(uint64_t gate);

/* NUMA Management */
int power_numa_init(void);
int power_numa_get_node_id(uint64_t *node_id);
int power_numa_get_distance(uint64_t node1, uint64_t node2, uint64_t *distance);
int power_numa_get_local_memory(uint64_t *size);
int power_numa_get_remote_memory(uint64_t *size);
int power_numa_set_memory_policy(uint64_t policy, uint64_t node_mask);

/* Main Initialization */
int power_arch_init(void);
int power_early_init(void);
int power_system_init(void);
int power_late_init(void);

/* Exception Handlers */
void power_exception_handler(uint64_t exception, power_cpu_context_t *context);
void power_interrupt_handler(uint64_t irq, power_cpu_context_t *context);
void power_machine_check_handler(power_cpu_context_t *context);
void power_data_storage_handler(power_cpu_context_t *context);
void power_instruction_storage_handler(power_cpu_context_t *context);
void power_program_handler(power_cpu_context_t *context);
void power_fp_unavailable_handler(power_cpu_context_t *context);
void power_decrementer_handler(power_cpu_context_t *context);
void power_system_call_handler(power_cpu_context_t *context);
void power_trace_handler(power_cpu_context_t *context);
void power_performance_handler(power_cpu_context_t *context);

/* Advanced Features */
int power_advanced_features_init(void);
int power_advanced_features_test(void);
void power_advanced_features_cleanup(void);

/* VSX Advanced */
int power_vsx_advanced_init(void);
int power_vsx_128_init(void);
int power_vsx_256_init(void);
int power_vsx_512_init(void);

/* AltiVec Advanced */
int power_altivec_advanced_init(void);
int power_altivec_128_init(void);
int power_altivec_256_init(void);

/* Cryptographic Acceleration */
int power_crypto_init(void);
bool power_crypto_aes_supported(void);
bool power_crypto_sha_supported(void);
bool power_crypto_sha3_supported(void);

/* Power Management Advanced */
int power_power_advanced_init(void);
uint32_t power_power_get_supported_states(void);

/* Virtualization Support */
int power_virtualization_init(void);
bool power_virtualization_is_supported(void);

/* MSVC Support */
#ifdef _MSC_VER
int power_msvc_optimize(void);
void power_msvc_intrinsics_init(void);
#endif

#endif // ORION_ARCH_POWER_H
