#ifndef ORION_SYSCALLS_H
#define ORION_SYSCALLS_H

#include <orion/types.h>

// Numéros d'appels système Orion (< 60 total pour v1.0)

// Catégorie: Process/Thread (0-9)
#define SYS_EXIT 0          // Terminer processus
#define SYS_YIELD 1         // Céder CPU
#define SYS_PROC_CREATE 2   // Créer processus
#define SYS_THREAD_CREATE 3 // Créer thread
#define SYS_WAIT 4          // Attendre processus/thread
#define SYS_SIGNAL 5        // Envoyer signal
#define SYS_GETPID 6        // Obtenir PID
#define SYS_GETTID 7        // Obtenir TID

// Catégorie: Mémoire (10-19)
#define SYS_VM_MAP 10     // Mapper mémoire virtuelle
#define SYS_VM_UNMAP 11   // Démapper mémoire
#define SYS_VM_PROTECT 12 // Changer permissions
#define SYS_SHM_CREATE 13 // Créer mémoire partagée
#define SYS_SHM_ATTACH 14 // Attacher mémoire partagée
#define SYS_SHM_DETACH 15 // Détacher mémoire partagée
#define SYS_MADVISE 16    // Conseils mémoire

// Catégorie: IPC (20-29)
#define SYS_PORT_CREATE 20 // Créer port IPC
#define SYS_PORT_SEND 21   // Envoyer message
#define SYS_PORT_RECV 22   // Recevoir message
#define SYS_PORT_SHARE 23  // Partager port
#define SYS_MSG_FORWARD 24 // Transférer message

// Catégorie: Temps (30-34)
#define SYS_CLOCK_GET 30    // Lire horloge
#define SYS_TIMER_CREATE 31 // Créer timer
#define SYS_TIMER_START 32  // Démarrer timer
#define SYS_TIMER_STOP 33   // Arrêter timer
#define SYS_NANOSLEEP 34    // Dormir

// Catégorie: I/O abstraite (35-39)
#define SYS_IO_SUBMIT 35 // Soumettre I/O
#define SYS_IO_POLL 36   // Polling I/O
#define SYS_IO_CANCEL 37 // Annuler I/O

// Catégorie: Objets (40-44)
#define SYS_OBJ_INFO 40  // Info sur objet
#define SYS_OBJ_DUP 41   // Dupliquer handle
#define SYS_OBJ_CLOSE 42 // Fermer handle

// Catégorie: Sécurité (45-49)
#define SYS_CAP_GRANT 45    // Accorder capability
#define SYS_CAP_REVOKE 46   // Révoquer capability
#define SYS_CAP_QUERY 47    // Vérifier capability
#define SYS_SANDBOX_LOAD 48 // Charger sandbox
#define SYS_AUDIT_EMIT 49   // Émettre audit

// Catégorie: Divers (50-59)
#define SYS_INFO 50      // Info système
#define SYS_DBG_TRACE 51 // Debug trace
#define SYS_RANDOM 52    // Obtenir entropy

// Nombre maximum de syscalls
#define MAX_SYSCALLS 60

// Structures pour les appels système

// Structure pour sys_info
typedef struct
{
    char kernel_version[32];
    uint64_t boot_time;
    uint64_t current_time;
    uint64_t total_memory;
    uint64_t free_memory;
    uint32_t cpu_count;
    uint32_t process_count;
    uint32_t thread_count;
} or_system_info_t;

// Structure pour sys_vm_map
typedef struct
{
    uint64_t addr;    // Adresse virtuelle (0 = auto)
    size_t length;    // Taille
    uint32_t prot;    // Protection (READ|WRITE|EXEC)
    uint32_t flags;   // Flags (PRIVATE|SHARED|FIXED)
    or_cap_t backing; // Capability de l'objet backing (0 = anonymous)
    uint64_t offset;  // Offset dans l'objet backing
} or_vm_map_t;

// Structure pour messages IPC
typedef struct
{
    or_cap_t target_port; // Port de destination
    const void *data;     // Données à envoyer
    size_t data_size;     // Taille des données
    or_cap_t *caps;       // Capabilities à transférer
    size_t caps_count;    // Nombre de capabilities
    uint64_t timeout_ns;  // Timeout en nanosecondes
} or_msg_send_t;

typedef struct
{
    or_cap_t source_port; // Port source (output)
    void *buffer;         // Buffer pour recevoir
    size_t buffer_size;   // Taille du buffer
    or_cap_t *caps;       // Buffer pour capabilities reçues
    size_t caps_max;      // Nombre max de capabilities
    size_t caps_received; // Nombre de capabilities reçues (output)
    uint64_t timeout_ns;  // Timeout
} or_msg_recv_t;

// Flags pour vm_map
#define VM_PROT_READ (1 << 0)
#define VM_PROT_WRITE (1 << 1)
#define VM_PROT_EXEC (1 << 2)

#define VM_MAP_PRIVATE (1 << 0)
#define VM_MAP_SHARED (1 << 1)
#define VM_MAP_FIXED (1 << 2)
#define VM_MAP_ANONYMOUS (1 << 3)

// Convention d'appel System V x86_64:
// RDI, RSI, RDX, RCX, R8, R9 pour les 6 premiers arguments
// RAX = numéro syscall
// Retour dans RAX (int64_t)

// Wrapper pour appels depuis le code C (userland)
#ifdef _MSC_VER
// Pour MSVC, on utilise des stubs temporaires (sera remplacé par assembleur dédié)
static inline int64_t orion_syscall0(uint64_t num)
{
    (void)num;
    return -1;
}
static inline int64_t orion_syscall1(uint64_t num, uint64_t arg1)
{
    (void)num;
    (void)arg1;
    return -1;
}
static inline int64_t orion_syscall2(uint64_t num, uint64_t arg1, uint64_t arg2)
{
    (void)num;
    (void)arg1;
    (void)arg2;
    return -1;
}
static inline int64_t orion_syscall3(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
    (void)num;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    return -1;
}
#else
static inline int64_t orion_syscall0(uint64_t num)
{
    int64_t ret;
    __asm__ volatile(
        "syscall"
        : "=a"(ret)
        : "a"(num)
        : "rcx", "r11", "memory");
    return ret;
}

static inline int64_t orion_syscall1(uint64_t num, uint64_t arg1)
{
    int64_t ret;
    __asm__ volatile(
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(arg1)
        : "rcx", "r11", "memory");
    return ret;
}

static inline int64_t orion_syscall2(uint64_t num, uint64_t arg1, uint64_t arg2)
{
    int64_t ret;
    __asm__ volatile(
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(arg1), "S"(arg2)
        : "rcx", "r11", "memory");
    return ret;
}

static inline int64_t orion_syscall3(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3)
{
    int64_t ret;
    __asm__ volatile(
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3)
        : "rcx", "r11", "memory");
    return ret;
}
#endif

// Fonctions C pour utilisation dans le noyau
void syscalls_init(void);
int64_t syscall_handler(uint64_t syscall_num, uint64_t arg1, uint64_t arg2,
                        uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6);

// Handlers individuels (définis dans syscalls.c)
int64_t sys_info_impl(or_system_info_t *info);
int64_t sys_exit_impl(int exit_code);
int64_t sys_yield_impl(void);
int64_t sys_vm_map_impl(or_vm_map_t *map_params);
int64_t sys_port_create_impl(or_cap_t *out_port);
int64_t sys_port_send_impl(or_msg_send_t *msg);
int64_t sys_port_recv_impl(or_msg_recv_t *msg);

// Process/Thread syscalls
int64_t sys_proc_create_impl(const char *executable_path, char *const argv[], char *const envp[]);
int64_t sys_thread_create_impl(uint64_t entry_point, uint64_t stack_pointer, uint64_t arg);
int64_t sys_wait_impl(uint64_t pid, int *status, uint64_t options);
int64_t sys_signal_impl(uint64_t target_pid, uint32_t signal_num);
int64_t sys_getpid_impl(void);
int64_t sys_gettid_impl(void);

// Memory syscalls
int64_t sys_vm_unmap_impl(uint64_t addr, size_t length);
int64_t sys_vm_protect_impl(uint64_t addr, size_t length, uint32_t new_prot);
int64_t sys_shm_create_impl(size_t size, uint32_t flags);
int64_t sys_shm_attach_impl(or_cap_t shm_cap, uint64_t addr, uint32_t flags);
int64_t sys_shm_detach_impl(uint64_t addr);
int64_t sys_madvise_impl(uint64_t addr, size_t length, uint32_t advice);

// IPC syscalls
int64_t sys_port_share_impl(or_cap_t port, uint64_t target_pid);
int64_t sys_msg_forward_impl(or_cap_t source_port, or_cap_t dest_port);

// Time syscalls
int64_t sys_clock_get_impl(uint32_t clock_id, uint64_t *timestamp);
int64_t sys_timer_create_impl(uint32_t clock_id, uint64_t *timer_id);
int64_t sys_timer_start_impl(uint64_t timer_id, uint64_t timeout_ns, uint64_t interval_ns);
int64_t sys_timer_stop_impl(uint64_t timer_id);
int64_t sys_nanosleep_impl(uint64_t nanoseconds);

// I/O syscalls
int64_t sys_io_submit_impl(void *io_context, uint32_t nr, void *iocbpp);
int64_t sys_io_poll_impl(void *io_context, uint32_t min_nr, uint32_t max_nr, void *events, uint64_t timeout_ns);
int64_t sys_io_cancel_impl(void *io_context, void *iocb);

// Object syscalls
int64_t sys_obj_info_impl(or_cap_t cap, void *info_buffer, size_t buffer_size);
int64_t sys_obj_dup_impl(or_cap_t cap);
int64_t sys_obj_close_impl(or_cap_t cap);

// Security syscalls
int64_t sys_cap_grant_impl(uint64_t target_pid, or_cap_t cap, uint32_t permissions);
int64_t sys_cap_revoke_impl(uint64_t target_pid, or_cap_t cap);
int64_t sys_cap_query_impl(or_cap_t cap, uint32_t *permissions);
int64_t sys_sandbox_load_impl(const char *policy_path);
int64_t sys_audit_emit_impl(uint32_t event_type, const void *event_data, size_t data_size);

// Misc syscalls
int64_t sys_dbg_trace_impl(uint32_t trace_type, const void *trace_data, size_t data_size);
int64_t sys_random_impl(void *buffer, size_t size);

#endif // ORION_SYSCALLS_H
