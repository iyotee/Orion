/*
 * ORION OS - LoongArch Architecture Support
 *
 * This file defines LoongArch-specific constants, structures, and function
 * declarations for the ORION kernel.
 */

#ifndef ORION_ARCH_LOONGARCH_H
#define ORION_ARCH_LOONGARCH_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

/* LoongArch Architecture Identification */
#define LOONGARCH_ARCH_ID 0x14C0
#define LOONGARCH_ARCH_NAME "LoongArch64"
#define LOONGARCH_ARCH_VERSION 0x0100

/* CPU Register Definitions */
#define LOONGARCH_REG_ZERO 0 /* Always zero */
#define LOONGARCH_REG_RA 1   /* Return address */
#define LOONGARCH_REG_TP 2   /* Thread pointer */
#define LOONGARCH_REG_SP 3   /* Stack pointer */
#define LOONGARCH_REG_A0 4   /* Argument 0 */
#define LOONGARCH_REG_A1 5   /* Argument 1 */
#define LOONGARCH_REG_A2 6   /* Argument 2 */
#define LOONGARCH_REG_A3 7   /* Argument 3 */
#define LOONGARCH_REG_A4 8   /* Argument 4 */
#define LOONGARCH_REG_A5 9   /* Argument 5 */
#define LOONGARCH_REG_A6 10  /* Argument 6 */
#define LOONGARCH_REG_A7 11  /* Argument 7 */
#define LOONGARCH_REG_T0 12  /* Temporary 0 */
#define LOONGARCH_REG_T1 13  /* Temporary 1 */
#define LOONGARCH_REG_T2 14  /* Temporary 2 */
#define LOONGARCH_REG_T3 15  /* Temporary 3 */
#define LOONGARCH_REG_T4 16  /* Temporary 4 */
#define LOONGARCH_REG_T5 17  /* Temporary 5 */
#define LOONGARCH_REG_T6 18  /* Temporary 6 */
#define LOONGARCH_REG_T7 19  /* Temporary 7 */
#define LOONGARCH_REG_T8 20  /* Temporary 8 */
#define LOONGARCH_REG_S0 23  /* Saved 0 */
#define LOONGARCH_REG_S1 24  /* Saved 1 */
#define LOONGARCH_REG_S2 25  /* Saved 2 */
#define LOONGARCH_REG_S3 26  /* Saved 3 */
#define LOONGARCH_REG_S4 27  /* Saved 4 */
#define LOONGARCH_REG_S5 28  /* Saved 5 */
#define LOONGARCH_REG_S6 29  /* Saved 6 */
#define LOONGARCH_REG_S7 30  /* Saved 7 */
#define LOONGARCH_REG_S8 31  /* Saved 8 */

/* MMU Constants */
#define LOONGARCH_PAGE_SHIFT 12
#define LOONGARCH_PAGE_SIZE (1UL << LOONGARCH_PAGE_SHIFT)
#define LOONGARCH_PAGE_MASK (~(LOONGARCH_PAGE_SIZE - 1))

#define LOONGARCH_HUGE_PAGE_SHIFT 21
#define LOONGARCH_HUGE_PAGE_SIZE (1UL << LOONGARCH_HUGE_PAGE_SHIFT)
#define LOONGARCH_HUGE_PAGE_MASK (~(LOONGARCH_HUGE_PAGE_SIZE - 1))

#define LOONGARCH_GIGA_PAGE_SHIFT 30
#define LOONGARCH_GIGA_PAGE_SIZE (1UL << LOONGARCH_GIGA_PAGE_SHIFT)
#define LOONGARCH_GIGA_PAGE_MASK (~(LOONGARCH_GIGA_PAGE_SIZE - 1))

/* Page Table Entry Flags */
#define LOONGARCH_PTE_VALID (1UL << 0)        /* Valid */
#define LOONGARCH_PTE_DIRTY (1UL << 1)        /* Dirty */
#define LOONGARCH_PTE_PRESENT (1UL << 2)      /* Present */
#define LOONGARCH_PTE_WRITE (1UL << 3)        /* Writable */
#define LOONGARCH_PTE_USER (1UL << 4)         /* User accessible */
#define LOONGARCH_PTE_EXEC (1UL << 5)         /* Executable */
#define LOONGARCH_PTE_GLOBAL (1UL << 6)       /* Global */
#define LOONGARCH_PTE_NOCACHE (1UL << 7)      /* No cache */
#define LOONGARCH_PTE_WRITETHROUGH (1UL << 8) /* Write-through */
#define LOONGARCH_PTE_SPECIAL (1UL << 9)      /* Special */
#define LOONGARCH_PTE_HUGE (1UL << 10)        /* Huge page */
#define LOONGARCH_PTE_PROTNONE (1UL << 11)    /* Protection none */

/* Virtual Address Space */
#define LOONGARCH_KERNEL_BASE 0x9000000000000000UL
#define LOONGARCH_KERNEL_SIZE 0x400000000UL /* 16GB */
#define LOONGARCH_USER_BASE 0x0000000000000000UL
#define LOONGARCH_USER_SIZE 0x8000000000000000UL /* 8EB */

/* Interrupt/Exception Definitions */
#define LOONGARCH_INT_SIP0 0 /* Software interrupt 0 */
#define LOONGARCH_INT_SIP1 1 /* Software interrupt 1 */
#define LOONGARCH_INT_IP0 2  /* Hardware interrupt 0 */
#define LOONGARCH_INT_IP1 3  /* Hardware interrupt 1 */
#define LOONGARCH_INT_IP2 4  /* Hardware interrupt 2 */
#define LOONGARCH_INT_IP3 5  /* Hardware interrupt 3 */
#define LOONGARCH_INT_IP4 6  /* Hardware interrupt 4 */
#define LOONGARCH_INT_IP5 7  /* Hardware interrupt 5 */
#define LOONGARCH_INT_IP6 8  /* Hardware interrupt 6 */
#define LOONGARCH_INT_IP7 9  /* Hardware interrupt 7 */

#define LOONGARCH_EXC_RESET 0        /* Reset */
#define LOONGARCH_EXC_TLB_REFILL 1   /* TLB refill */
#define LOONGARCH_EXC_TLB_INVALID 2  /* TLB invalid */
#define LOONGARCH_EXC_TLB_MODIFIED 3 /* TLB modified */
#define LOONGARCH_EXC_TLB_LOAD 4     /* TLB load */
#define LOONGARCH_EXC_TLB_STORE 5    /* TLB store */
#define LOONGARCH_EXC_ADDR_ERR 6     /* Address error */
#define LOONGARCH_EXC_ADDR_ERR_ST 7  /* Address error store */
#define LOONGARCH_EXC_SYS_CALL 8     /* System call */
#define LOONGARCH_EXC_BREAK 9        /* Break */
#define LOONGARCH_EXC_RESERVED 10    /* Reserved */
#define LOONGARCH_EXC_FLOATING 11    /* Floating point */
#define LOONGARCH_EXC_LOAD 12        /* Load */
#define LOONGARCH_EXC_STORE 13       /* Store */
#define LOONGARCH_EXC_LOAD_GUEST 14  /* Load guest */
#define LOONGARCH_EXC_STORE_GUEST 15 /* Store guest */

/* CPU Features */
#define LOONGARCH_FEATURE_LSX (1UL << 0)    /* LSX vector extension */
#define LOONGARCH_FEATURE_LASX (1UL << 1)   /* LASX vector extension */
#define LOONGARCH_FEATURE_CRYPTO (1UL << 2) /* Crypto acceleration */
#define LOONGARCH_FEATURE_VIRT (1UL << 3)   /* Virtualization */
#define LOONGARCH_FEATURE_HT (1UL << 4)     /* Hyper-threading */
#define LOONGARCH_FEATURE_NUMA (1UL << 5)   /* NUMA support */
#define LOONGARCH_FEATURE_LBT (1UL << 6)    /* Loongson Binary Translation */
#define LOONGARCH_FEATURE_LSX2 (1UL << 7)   /* LSX2 extension */
#define LOONGARCH_FEATURE_LASX2 (1UL << 8)  /* LASX2 extension */

/* Timer/Clock Definitions */
#define LOONGARCH_TIMER_FREQ 1000000000 /* 1GHz default */
#define LOONGARCH_TIMER_IRQ 2           /* Timer interrupt number */

/* Cache and Memory Barriers */
#define LOONGARCH_CACHE_LINE_SIZE 64
#define LOONGARCH_CACHE_LINE_MASK (LOONGARCH_CACHE_LINE_SIZE - 1)

/* Memory Barriers */
static inline void loongarch_mb(void)
{
    __asm__ __volatile__("dbar 0" : : : "memory");
}

static inline void loongarch_rmb(void)
{
    __asm__ __volatile__("dbar 0" : : : "memory");
}

static inline void loongarch_wmb(void)
{
    __asm__ __volatile__("dbar 0" : : : "memory");
}

/* CPU Information Structure */
struct loongarch_cpu_info
{
    uint32_t vendor_id;
    uint32_t cpu_id;
    uint32_t revision;
    uint64_t features;
    uint32_t cache_line_size;
    uint32_t l1i_cache_size;
    uint32_t l1d_cache_size;
    uint32_t l2_cache_size;
    uint32_t l3_cache_size;
    uint32_t num_cores;
    uint32_t num_threads;
    uint32_t max_freq_mhz;
    char model_name[64];
};

/* MMU Context Structure */
struct loongarch_mmu_context
{
    uint64_t pgd;         /* Page Global Directory */
    uint64_t asid;        /* Address Space ID */
    uint64_t tlb_entries; /* TLB entry count */
    uint64_t flags;       /* MMU flags */
};

/* Interrupt Context Structure */
struct loongarch_interrupt_context
{
    uint64_t regs[32];         /* General registers */
    uint64_t csr_era;          /* Exception return address */
    uint64_t csr_badvaddr;     /* Bad virtual address */
    uint64_t csr_crmd;         /* Current mode */
    uint64_t csr_prmd;         /* Previous mode */
    uint64_t csr_euen;         /* Extended unit enable */
    uint64_t csr_ecfg;         /* Exception configuration */
    uint64_t csr_estat;        /* Exception status */
    uint64_t csr_ticlr;        /* Timer interrupt clear */
    uint64_t csr_tcfg;         /* Timer configuration */
    uint64_t csr_tval;         /* Timer value */
    uint64_t csr_tick;         /* Timer tick */
    uint64_t csr_tlbrentry;    /* TLB refill entry */
    uint64_t csr_dmw0;         /* Direct mapping window 0 */
    uint64_t csr_dmw1;         /* Direct mapping window 1 */
    uint64_t csr_pgdl;         /* Page directory low */
    uint64_t csr_pgdh;         /* Page directory high */
    uint64_t csr_pgd;          /* Page directory */
    uint64_t csr_pwctl0;       /* Page window control 0 */
    uint64_t csr_pwctl1;       /* Page window control 1 */
    uint64_t csr_stlbps;       /* STLB page size */
    uint64_t csr_rvacfg;       /* Reduced virtual address config */
    uint64_t csr_ptbase;       /* Page table base */
    uint64_t csr_ptwidth;      /* Page table width */
    uint64_t csr_stlbpgsize;   /* STLB page size */
    uint64_t csr_rvacfg2;      /* Reduced virtual address config 2 */
    uint64_t csr_ptbase2;      /* Page table base 2 */
    uint64_t csr_ptwidth2;     /* Page table width 2 */
    uint64_t csr_stlbpgsize2;  /* STLB page size 2 */
    uint64_t csr_rvacfg3;      /* Reduced virtual address config 3 */
    uint64_t csr_ptbase3;      /* Page table base 3 */
    uint64_t csr_ptwidth3;     /* Page table width 3 */
    uint64_t csr_stlbpgsize3;  /* STLB page size 3 */
    uint64_t csr_rvacfg4;      /* Reduced virtual address config 4 */
    uint64_t csr_ptbase4;      /* Page table base 4 */
    uint64_t csr_ptwidth4;     /* Page table width 4 */
    uint64_t csr_stlbpgsize4;  /* STLB page size 4 */
    uint64_t csr_rvacfg5;      /* Reduced virtual address config 5 */
    uint64_t csr_ptbase5;      /* Page table base 5 */
    uint64_t csr_ptwidth5;     /* Page table width 5 */
    uint64_t csr_stlbpgsize5;  /* STLB page size 5 */
    uint64_t csr_rvacfg6;      /* Reduced virtual address config 6 */
    uint64_t csr_ptbase6;      /* Page table base 6 */
    uint64_t csr_ptwidth6;     /* Page table width 6 */
    uint64_t csr_stlbpgsize6;  /* STLB page size 6 */
    uint64_t csr_rvacfg7;      /* Reduced virtual address config 7 */
    uint64_t csr_ptbase7;      /* Page table base 7 */
    uint64_t csr_ptwidth7;     /* Page table width 7 */
    uint64_t csr_stlbpgsize7;  /* STLB page size 7 */
    uint64_t csr_rvacfg8;      /* Reduced virtual address config 8 */
    uint64_t csr_ptbase8;      /* Page table base 8 */
    uint64_t csr_ptwidth8;     /* Page table width 8 */
    uint64_t csr_stlbpgsize8;  /* STLB page size 8 */
    uint64_t csr_rvacfg9;      /* Reduced virtual address config 9 */
    uint64_t csr_ptbase9;      /* Page table base 9 */
    uint64_t csr_ptwidth9;     /* Page table width 9 */
    uint64_t csr_stlbpgsize9;  /* STLB page size 9 */
    uint64_t csr_rvacfg10;     /* Reduced virtual address config 10 */
    uint64_t csr_ptbase10;     /* Page table base 10 */
    uint64_t csr_ptwidth10;    /* Page table width 10 */
    uint64_t csr_stlbpgsize10; /* STLB page size 10 */
    uint64_t csr_rvacfg11;     /* Reduced virtual address config 11 */
    uint64_t csr_ptbase11;     /* Page table base 11 */
    uint64_t csr_ptwidth11;    /* Page table width 11 */
    uint64_t csr_stlbpgsize11; /* STLB page size 11 */
    uint64_t csr_rvacfg12;     /* Reduced virtual address config 12 */
    uint64_t csr_ptbase12;     /* Page table base 12 */
    uint64_t csr_ptwidth12;    /* Page table width 12 */
    uint64_t csr_stlbpgsize12; /* STLB page size 12 */
    uint64_t csr_rvacfg13;     /* Reduced virtual address config 13 */
    uint64_t csr_ptbase13;     /* Page table base 13 */
    uint64_t csr_ptwidth13;    /* Page table width 13 */
    uint64_t csr_stlbpgsize13; /* STLB page size 13 */
    uint64_t csr_rvacfg14;     /* Reduced virtual address config 14 */
    uint64_t csr_ptbase14;     /* Page table base 14 */
    uint64_t csr_ptwidth14;    /* Page table width 14 */
    uint64_t csr_stlbpgsize14; /* STLB page size 14 */
    uint64_t csr_rvacfg15;     /* Reduced virtual address config 15 */
    uint64_t csr_ptbase15;     /* Page table base 15 */
    uint64_t csr_ptwidth15;    /* Page table width 15 */
    uint64_t csr_stlbpgsize15; /* STLB page size 15 */
};

/* Architecture-specific Types */
typedef uint64_t loongarch_vaddr_t;
typedef uint64_t loongarch_paddr_t;
typedef uint64_t loongarch_size_t;
typedef uint64_t loongarch_off_t;

/* Function Declarations */

/* CPU Management */
int loongarch_cpu_init(void);
int loongarch_cpu_detect(struct loongarch_cpu_info *info);
void loongarch_cpu_idle(void);
void loongarch_cpu_reset(void);

/* MMU Management */
int loongarch_mmu_init(void);
int loongarch_mmu_map(loongarch_vaddr_t va, loongarch_paddr_t pa, uint64_t flags);
int loongarch_mmu_unmap(loongarch_vaddr_t va, loongarch_size_t len);
int loongarch_mmu_protect(loongarch_vaddr_t va, loongarch_size_t len, uint64_t flags);
void loongarch_mmu_invalidate_tlb(void);
void loongarch_mmu_invalidate_tlb_page(loongarch_vaddr_t va);
int loongarch_mmu_create_context(struct loongarch_mmu_context *ctx);
void loongarch_mmu_switch_context(struct loongarch_mmu_context *ctx);
void loongarch_mmu_destroy_context(struct loongarch_mmu_context *ctx);

/* Interrupt Management */
int loongarch_interrupt_init(void);
int loongarch_interrupt_enable(uint32_t irq);
int loongarch_interrupt_disable(uint32_t irq);
int loongarch_interrupt_set_handler(uint32_t irq, void (*handler)(void));
void loongarch_interrupt_handler(struct loongarch_interrupt_context *ctx);
void loongarch_interrupt_return(struct loongarch_interrupt_context *ctx);

/* Timer Management */
int loongarch_timer_init(void);
uint64_t loongarch_timer_read(void);
int loongarch_timer_set_oneshot(uint64_t deadline);
int loongarch_timer_set_periodic(uint64_t period);
void loongarch_timer_interrupt_handler(void);

/* Cache Management */
void loongarch_cache_invalidate_all(void);
void loongarch_cache_invalidate_range(loongarch_vaddr_t va, loongarch_size_t len);
void loongarch_cache_clean_range(loongarch_vaddr_t va, loongarch_size_t len);
void loongarch_cache_flush_range(loongarch_vaddr_t va, loongarch_size_t len);

/* Vector Extensions */
int loongarch_lsx_init(void);
int loongarch_lasx_init(void);
bool loongarch_lsx_supported(void);
bool loongarch_lasx_supported(void);

/* Crypto Acceleration */
int loongarch_crypto_init(void);
bool loongarch_crypto_supported(void);

/* Power Management */
int loongarch_power_init(void);
void loongarch_power_suspend(void);
void loongarch_power_resume(void);

/* Debug Support */
int loongarch_debug_init(void);
void loongarch_debug_break(void);
void loongarch_debug_print(const char *msg);

/* Boot Support */
void loongarch_boot_init(void);
void loongarch_boot_secondary(uint32_t cpu_id, void (*entry)(void));
void loongarch_boot_finalize(void);

/* Architecture-specific Utilities */
uint64_t loongarch_read_csr(uint32_t csr);
void loongarch_write_csr(uint32_t csr, uint64_t value);
uint64_t loongarch_read_msr(uint32_t msr);
void loongarch_write_msr(uint32_t msr, uint64_t value);

/* Performance Monitoring */
int loongarch_pmu_init(void);
uint64_t loongarch_pmu_read_counter(uint32_t counter);
int loongarch_pmu_set_counter(uint32_t counter, uint64_t value);
uint64_t loongarch_pmu_read_cycle_counter(void);

/* NUMA Support */
int loongarch_numa_init(void);
uint32_t loongarch_numa_get_node_id(loongarch_paddr_t pa);
uint32_t loongarch_numa_get_distance(uint32_t node1, uint32_t node2);
int loongarch_numa_optimize(void);

/* Virtualization Support */
int loongarch_virt_init(void);
bool loongarch_virt_supported(void);
int loongarch_virt_create_vm(void);
int loongarch_virt_destroy_vm(int vm_id);
int loongarch_virtualization_init(void);

/* Loongson Binary Translation */
int loongarch_lbt_init(void);
bool loongarch_lbt_supported(void);
int loongarch_lbt_translate(void *code, size_t size);

/* Advanced Features */
int loongarch_advanced_features_init(void);
int loongarch_advanced_features_test(void);
void loongarch_advanced_features_cleanup(void);

/* Vector Extensions */
int loongarch_lsx_init(void);
int loongarch_lasx_init(void);

/* Cryptographic Acceleration */
int loongarch_crypto_init(void);

/* Power Management */
int loongarch_power_management_init(void);

/* MSVC Support */
#ifdef _MSC_VER
int loongarch_msvc_optimize(void);
void loongarch_msvc_intrinsics_init(void);
#endif

/* Error Handling */
void loongarch_panic(const char *msg);
void loongarch_error_handler(uint32_t error_code, struct loongarch_interrupt_context *ctx);

/* Architecture-specific Constants */
#define LOONGARCH_MAX_IRQS 256
#define LOONGARCH_MAX_CPUS 256
#define LOONGARCH_MAX_NODES 16
#define LOONGARCH_MAX_VMS 64
#define LOONGARCH_MAX_PMU_COUNTERS 8

/* Architecture-specific Macros */
#define LOONGARCH_ALIGN_UP(addr, align) \
    (((addr) + (align) - 1) & ~((align) - 1))

#define LOONGARCH_ALIGN_DOWN(addr, align) \
    ((addr) & ~((align) - 1))

#define LOONGARCH_IS_ALIGNED(addr, align) \
    (((addr) & ((align) - 1)) == 0)

#define LOONGARCH_PAGE_ALIGN(addr) \
    LOONGARCH_ALIGN_UP(addr, LOONGARCH_PAGE_SIZE)

#define LOONGARCH_PAGE_ALIGN_DOWN(addr) \
    LOONGARCH_ALIGN_DOWN(addr, LOONGARCH_PAGE_SIZE)

#define LOONGARCH_HUGE_PAGE_ALIGN(addr) \
    LOONGARCH_ALIGN_UP(addr, LOONGARCH_HUGE_PAGE_SIZE)

#define LOONGARCH_GIGA_PAGE_ALIGN(addr) \
    LOONGARCH_ALIGN_UP(addr, LOONGARCH_GIGA_PAGE_SIZE)

/* Architecture-specific Inline Functions */
static inline bool loongarch_is_kernel_address(loongarch_vaddr_t va)
{
    return (va >= LOONGARCH_KERNEL_BASE);
}

static inline bool loongarch_is_user_address(loongarch_vaddr_t va)
{
    return (va < LOONGARCH_USER_SIZE);
}

static inline bool loongarch_is_valid_address(loongarch_vaddr_t va)
{
    return (va < LOONGARCH_USER_SIZE || va >= LOONGARCH_KERNEL_BASE);
}

static inline loongarch_paddr_t loongarch_virt_to_phys(loongarch_vaddr_t va)
{
    if (loongarch_is_kernel_address(va))
    {
        return (loongarch_paddr_t)(va - LOONGARCH_KERNEL_BASE);
    }
    return (loongarch_paddr_t)va;
}

static inline loongarch_vaddr_t loongarch_phys_to_virt(loongarch_paddr_t pa)
{
    return (loongarch_vaddr_t)(pa + LOONGARCH_KERNEL_BASE);
}

/* Architecture-specific Assembly Functions */
void loongarch_context_switch(struct loongarch_mmu_context *prev, struct loongarch_mmu_context *next);
void loongarch_enter_user_mode(void *entry, void *stack, uint64_t arg);
void loongarch_return_from_interrupt(struct loongarch_interrupt_context *ctx);
void loongarch_flush_tlb_all(void);
void loongarch_flush_tlb_page(loongarch_vaddr_t va);
void loongarch_flush_icache_all(void);
void loongarch_flush_dcache_all(void);
void loongarch_sync_icache(loongarch_vaddr_t va, loongarch_size_t len);
void loongarch_sync_dcache(loongarch_vaddr_t va, loongarch_size_t len);

#endif /* ORION_ARCH_LOONGARCH_H */
