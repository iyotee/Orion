// Gestionnaire d'appels système Orion
#include <orion/kernel.h>
#include <orion/types.h>
#include <orion/syscalls.h>

// Déclarations des fonctions manquantes (stubs)
extern void thread_exit(int exit_code);
extern or_cap_t ipc_port_create(uint64_t pid);
extern int ipc_send_message(or_cap_t port, void* data, uint64_t size, uint64_t timeout_ns);
extern int ipc_recv_message(or_cap_t port, void* buffer, uint64_t size, uint64_t timeout_ns);

// Table des appels système
typedef int64_t (*syscall_handler_t)(uint64_t arg1, uint64_t arg2, 
                                    uint64_t arg3, uint64_t arg4,
                                    uint64_t arg5, uint64_t arg6);

static syscall_handler_t syscall_table[MAX_SYSCALLS] = {
    // Process/Thread
    [SYS_EXIT]          = (syscall_handler_t)sys_exit_impl,
    [SYS_YIELD]         = (syscall_handler_t)sys_yield_impl,
    [SYS_PROC_CREATE]   = (syscall_handler_t)sys_proc_create_impl,
    [SYS_THREAD_CREATE] = (syscall_handler_t)sys_thread_create_impl,
    [SYS_WAIT]          = (syscall_handler_t)sys_wait_impl,
    [SYS_SIGNAL]        = (syscall_handler_t)sys_signal_impl,
    [SYS_GETPID]        = (syscall_handler_t)sys_getpid_impl,
    [SYS_GETTID]        = (syscall_handler_t)sys_gettid_impl,
    
    // Mémoire
    [SYS_VM_MAP]        = (syscall_handler_t)sys_vm_map_impl,
    [SYS_VM_UNMAP]      = (syscall_handler_t)sys_vm_unmap_impl,
    [SYS_VM_PROTECT]    = (syscall_handler_t)sys_vm_protect_impl,
    [SYS_SHM_CREATE]    = (syscall_handler_t)sys_shm_create_impl,
    [SYS_SHM_ATTACH]    = (syscall_handler_t)sys_shm_attach_impl,
    [SYS_SHM_DETACH]    = (syscall_handler_t)sys_shm_detach_impl,
    [SYS_MADVISE]       = (syscall_handler_t)sys_madvise_impl,
    
    // IPC
    [SYS_PORT_CREATE]   = (syscall_handler_t)sys_port_create_impl,
    [SYS_PORT_SEND]     = (syscall_handler_t)sys_port_send_impl,
    [SYS_PORT_RECV]     = (syscall_handler_t)sys_port_recv_impl,
    [SYS_PORT_SHARE]    = (syscall_handler_t)sys_port_share_impl,
    [SYS_MSG_FORWARD]   = (syscall_handler_t)sys_msg_forward_impl,
    
    // Temps
    [SYS_CLOCK_GET]     = (syscall_handler_t)sys_clock_get_impl,
    [SYS_TIMER_CREATE]  = (syscall_handler_t)sys_timer_create_impl,
    [SYS_TIMER_START]   = (syscall_handler_t)sys_timer_start_impl,
    [SYS_TIMER_STOP]    = (syscall_handler_t)sys_timer_stop_impl,
    [SYS_NANOSLEEP]     = (syscall_handler_t)sys_nanosleep_impl,
    
    // I/O
    [SYS_IO_SUBMIT]     = (syscall_handler_t)sys_io_submit_impl,
    [SYS_IO_POLL]       = (syscall_handler_t)sys_io_poll_impl,
    [SYS_IO_CANCEL]     = (syscall_handler_t)sys_io_cancel_impl,
    
    // Objets
    [SYS_OBJ_INFO]      = (syscall_handler_t)sys_obj_info_impl,
    [SYS_OBJ_DUP]       = (syscall_handler_t)sys_obj_dup_impl,
    [SYS_OBJ_CLOSE]     = (syscall_handler_t)sys_obj_close_impl,
    
    // Sécurité
    [SYS_CAP_GRANT]     = (syscall_handler_t)sys_cap_grant_impl,
    [SYS_CAP_REVOKE]    = (syscall_handler_t)sys_cap_revoke_impl,
    [SYS_CAP_QUERY]     = (syscall_handler_t)sys_cap_query_impl,
    [SYS_SANDBOX_LOAD]  = (syscall_handler_t)sys_sandbox_load_impl,
    [SYS_AUDIT_EMIT]    = (syscall_handler_t)sys_audit_emit_impl,
    
    // Divers
    [SYS_INFO]          = (syscall_handler_t)sys_info_impl,
    [SYS_DBG_TRACE]     = (syscall_handler_t)sys_dbg_trace_impl,
    [SYS_RANDOM]        = (syscall_handler_t)sys_random_impl
};

// Handler principal des syscalls (appelé depuis l'assembleur)
int64_t syscall_handler(uint64_t syscall_num, uint64_t arg1, uint64_t arg2,
                       uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6) {
    
    kdebug("Syscall %llu called", (unsigned long long)syscall_num);
    
    // Vérifier numéro syscall valide
    if (syscall_num >= MAX_SYSCALLS) {
        kerror("Invalid syscall number: %llu", (unsigned long long)syscall_num);
        return -OR_ENOSYS;
    }
    
    // Vérifier handler existe
    syscall_handler_t handler = syscall_table[syscall_num];
    if (!handler) {
        kerror("Unimplemented syscall: %llu", (unsigned long long)syscall_num);
        return -OR_ENOSYS;
    }
    
    // Appeler le handler
    return handler(arg1, arg2, arg3, arg4, arg5, arg6);
}

// Implémentations des syscalls

// sys_info - obtenir infos système
int64_t sys_info_impl(or_system_info_t* info)
{
    if (!info) {
        return -OR_EINVAL;
    }
    
    kdebug("sys_info called");
    
    // Vérifier que l'adresse userland est valide
    if (!mmu_is_valid_addr((uint64_t)info)) {
        return -OR_EFAULT;
    }
    
    // Copier les informations système de façon sécurisée
    or_system_info_t kernel_info = {0};
    
    // Copier la version du kernel
    const char* version = ORION_VERSION_STR;
    for (int i = 0; i < 31 && version[i]; i++) {
        kernel_info.kernel_version[i] = version[i];
    }
    kernel_info.kernel_version[31] = '\0';
    
    // Obtenir les statistiques mémoire
    uint64_t total_mem, free_mem, used_mem;
    pmm_get_stats(&total_mem, &free_mem, &used_mem);
    
    kernel_info.boot_time = arch_get_boot_time();
    kernel_info.current_time = arch_get_timestamp();
    kernel_info.total_memory = total_mem * PAGE_SIZE;
    kernel_info.free_memory = free_mem * PAGE_SIZE;
    kernel_info.cpu_count = arch_get_cpu_count();
    kernel_info.process_count = scheduler_get_process_count();
    kernel_info.thread_count = scheduler_get_thread_count();
    
    // Copie sécurisée vers userland (simulation)
    *info = kernel_info;
    
    return OR_OK;
}

// sys_exit - terminer processus
int64_t sys_exit_impl(int exit_code) {
    kinfo("Process exit with code %d", exit_code);
    
    // Nettoyer le processus courant
    process_t* current_process = scheduler_get_current_process();
    if (current_process) {
        // Fermer tous les handles ouverts
        for (int i = 0; i < MAX_HANDLES; i++) {
            if (current_process->handles[i].type != HANDLE_TYPE_NONE) {
                sys_obj_close_impl(i);
            }
        }
        
        // Libérer l'espace d'adressage
        if (current_process->vm_space && !current_process->vm_space->is_kernel) {
            vmm_destroy_space(current_process->vm_space);
        }
        
        // Marquer le processus comme zombie
        current_process->state = PROC_STATE_ZOMBIE;
        current_process->exit_code = exit_code;
        
        // Réveiller le parent s'il attend
        if (current_process->parent) {
            scheduler_wakeup_process(current_process->parent);
        }
    }
    
    thread_exit(exit_code);
    
    // Ne devrait jamais arriver
    return OR_OK;
}

// sys_yield - céder CPU
int64_t sys_yield_impl(void) {
    kdebug("sys_yield called");
    
    sched_yield();
    
    return OR_OK;
}

// sys_vm_map - mapper mémoire virtuelle
int64_t sys_vm_map_impl(or_vm_map_t* map_params) {
    if (!map_params) {
        return -OR_EINVAL;
    }
    
    kdebug("sys_vm_map called: addr=0x%p, len=%llu, prot=0x%x, flags=0x%x",
           (void*)map_params->addr,
           (unsigned long long)map_params->length,
           map_params->prot,
           map_params->flags);
    
    // Valider les paramètres
    if (map_params->length == 0 || !IS_ALIGNED(map_params->length, PAGE_SIZE)) {
        return -OR_EINVAL;
    }
    
    if (map_params->addr && !IS_ALIGNED(map_params->addr, PAGE_SIZE)) {
        return -OR_EINVAL;
    }
    
    // Obtenir l'espace d'adressage du processus courant
    process_t* current_process = scheduler_get_current_process();
    if (!current_process || !current_process->vm_space) {
        return -OR_EINVAL;
    }
    
    // Convertir les flags de protection
    uint64_t vm_flags = 0;
    if (map_params->prot & VM_PROT_READ) vm_flags |= VM_FLAG_READ;
    if (map_params->prot & VM_PROT_WRITE) vm_flags |= VM_FLAG_WRITE;
    if (map_params->prot & VM_PROT_EXEC) vm_flags |= VM_FLAG_EXEC;
    if (!current_process->vm_space->is_kernel) vm_flags |= VM_FLAG_USER;
    
    // Calculer le nombre de pages nécessaires
    size_t pages_needed = map_params->length / PAGE_SIZE;
    
    uint64_t vaddr;
    if (map_params->flags & VM_MAP_FIXED && map_params->addr) {
        // Adresse fixe demandée
        vaddr = map_params->addr;
        
        // Vérifier que l'espace est libre
        for (size_t i = 0; i < pages_needed; i++) {
            if (mmu_is_valid_addr(vaddr + i * PAGE_SIZE)) {
                return -OR_EINVAL; // Déjà mappé
            }
        }
        
        // Mapper les pages une par une
        for (size_t i = 0; i < pages_needed; i++) {
            uint64_t page_vaddr = vaddr + i * PAGE_SIZE;
            uint64_t page_paddr = pmm_alloc_page();
            
            if (!page_paddr) {
                // Nettoyer les pages déjà mappées
                for (size_t j = 0; j < i; j++) {
                    vmm_unmap_page(current_process->vm_space, vaddr + j * PAGE_SIZE);
                }
                return -OR_ENOMEM;
            }
            
            if (vmm_map_page(current_process->vm_space, page_vaddr, page_paddr, vm_flags) != OR_OK) {
                pmm_free_page(page_paddr);
                // Nettoyer les pages déjà mappées
                for (size_t j = 0; j < i; j++) {
                    vmm_unmap_page(current_process->vm_space, vaddr + j * PAGE_SIZE);
                }
                return -OR_ENOMEM;
            }
        }
    } else {
        // Allocation automatique d'adresse
        vaddr = vmm_alloc_pages(current_process->vm_space, pages_needed, vm_flags);
        if (!vaddr) {
            return -OR_ENOMEM;
        }
    }
    
    kdebug("sys_vm_map: mapped %llu pages at 0x%p",
           (unsigned long long)pages_needed, (void*)vaddr);
    
    return (int64_t)vaddr;
}

// sys_port_create - créer port IPC
int64_t sys_port_create_impl(or_cap_t* out_port) {
    if (!out_port) {
        return -OR_EINVAL;
    }
    
    // Obtenir PID du processus appelant
    process_t* current_process = scheduler_get_current_process();
    if (!current_process) {
        return -OR_EINVAL;
    }
    
    or_cap_t port = ipc_port_create(current_process->pid);
    if (port == 0) {
        return -OR_ENOMEM;
    }
    
    // Vérifier adresse userland et copier de façon sécurisée
    if (!mmu_is_valid_addr((uint64_t)out_port)) {
        return -OR_EFAULT;
    }
    
    *out_port = port;
    
    kdebug("Created IPC port %llu", (unsigned long long)port);
    return OR_OK;
}

// sys_port_send - envoyer message IPC
int64_t sys_port_send_impl(or_msg_send_t* msg) {
    if (!msg) {
        return -OR_EINVAL;
    }
    
    kdebug("sys_port_send called: port=%llu, size=%llu",
           (unsigned long long)msg->target_port,
           (unsigned long long)msg->data_size);
    
    return ipc_send_message(msg->target_port, (void*)msg->data, 
                           msg->data_size, msg->timeout_ns);
}

// sys_port_recv - recevoir message IPC
int64_t sys_port_recv_impl(or_msg_recv_t* msg) {
    if (!msg) {
        return -OR_EINVAL;
    }
    
    kdebug("sys_port_recv called");
    
    return ipc_recv_message(msg->source_port, msg->buffer,
                           msg->buffer_size, msg->timeout_ns);
}

// ========================================
// NOUVELLES IMPLÉMENTATIONS DE SYSCALLS
// ========================================

// sys_proc_create - créer un nouveau processus
int64_t sys_proc_create_impl(const char* executable_path, char* const argv[], char* const envp[]) {
    if (!executable_path) {
        return -OR_EINVAL;
    }
    
    kdebug("sys_proc_create called: path=%s", executable_path);
    
    // Créer un nouveau processus
    process_t* new_process = scheduler_create_process();
    if (!new_process) {
        return -OR_ENOMEM;
    }
    
    // Charger l'exécutable ELF
    int result = elf_load_process(new_process, executable_path);
    if (result != OR_OK) {
        scheduler_destroy_process(new_process);
        return result;
    }
    
    // Configurer les arguments et variables d'environnement
    result = process_setup_args(new_process, argv, envp);
    if (result != OR_OK) {
        scheduler_destroy_process(new_process);
        return result;
    }
    
    // Ajouter au scheduler
    scheduler_add_process(new_process);
    
    kdebug("Created process PID %llu", (unsigned long long)new_process->pid);
    return (int64_t)new_process->pid;
}

// sys_thread_create - créer un nouveau thread
int64_t sys_thread_create_impl(uint64_t entry_point, uint64_t stack_pointer, uint64_t arg) {
    kdebug("sys_thread_create called: entry=0x%p, stack=0x%p", 
           (void*)entry_point, (void*)stack_pointer);
    
    process_t* current_process = scheduler_get_current_process();
    if (!current_process) {
        return -OR_EINVAL;
    }
    
    thread_t* new_thread = scheduler_create_thread(current_process, entry_point, stack_pointer, arg);
    if (!new_thread) {
        return -OR_ENOMEM;
    }
    
    kdebug("Created thread TID %llu", (unsigned long long)new_thread->tid);
    return (int64_t)new_thread->tid;
}

// sys_wait - attendre la fin d'un processus/thread
int64_t sys_wait_impl(uint64_t pid, int* status, uint64_t options) {
    kdebug("sys_wait called: pid=%llu", (unsigned long long)pid);
    
    process_t* current_process = scheduler_get_current_process();
    if (!current_process) {
        return -OR_EINVAL;
    }
    
    process_t* target = scheduler_find_process(pid);
    if (!target || target->parent != current_process) {
        return -OR_EINVAL; // Pas notre enfant
    }
    
    // Attendre que le processus devienne zombie
    while (target->state != PROC_STATE_ZOMBIE) {
        scheduler_block_current_process();
        sched_yield();
    }
    
    // Récupérer le code de sortie
    if (status && mmu_is_valid_addr((uint64_t)status)) {
        *status = target->exit_code;
    }
    
    uint64_t waited_pid = target->pid;
    
    // Nettoyer le processus zombie
    scheduler_destroy_process(target);
    
    return (int64_t)waited_pid;
}

// sys_signal - envoyer un signal
int64_t sys_signal_impl(uint64_t target_pid, uint32_t signal_num) {
    kdebug("sys_signal called: target=%llu, signal=%u", 
           (unsigned long long)target_pid, signal_num);
    
    process_t* target = scheduler_find_process(target_pid);
    if (!target) {
        return -OR_ENOENT;
    }
    
    return signal_send(target, signal_num);
}

// sys_getpid - obtenir PID du processus courant
int64_t sys_getpid_impl(void) {
    process_t* current_process = scheduler_get_current_process();
    return current_process ? (int64_t)current_process->pid : -OR_EINVAL;
}

// sys_gettid - obtenir TID du thread courant  
int64_t sys_gettid_impl(void) {
    thread_t* current_thread = scheduler_get_current_thread();
    return current_thread ? (int64_t)current_thread->tid : -OR_EINVAL;
}

// sys_vm_unmap - démapper des pages de mémoire virtuelle
int64_t sys_vm_unmap_impl(uint64_t addr, size_t length) {
    if (!IS_ALIGNED(addr, PAGE_SIZE) || !IS_ALIGNED(length, PAGE_SIZE) || length == 0) {
        return -OR_EINVAL;
    }
    
    process_t* current_process = scheduler_get_current_process();
    if (!current_process || !current_process->vm_space) {
        return -OR_EINVAL;
    }
    
    size_t pages = length / PAGE_SIZE;
    vmm_free_pages(current_process->vm_space, addr, pages);
    
    kdebug("sys_vm_unmap: unmapped %llu pages at 0x%p", 
           (unsigned long long)pages, (void*)addr);
    
    return OR_OK;
}

// sys_vm_protect - changer les permissions de pages mémoire
int64_t sys_vm_protect_impl(uint64_t addr, size_t length, uint32_t new_prot) {
    if (!IS_ALIGNED(addr, PAGE_SIZE) || !IS_ALIGNED(length, PAGE_SIZE) || length == 0) {
        return -OR_EINVAL;
    }
    
    process_t* current_process = scheduler_get_current_process();
    if (!current_process || !current_process->vm_space) {
        return -OR_EINVAL;
    }
    
    // Convertir les permissions
    uint64_t vm_flags = 0;
    if (new_prot & VM_PROT_READ) vm_flags |= VM_FLAG_READ;
    if (new_prot & VM_PROT_WRITE) vm_flags |= VM_FLAG_WRITE;
    if (new_prot & VM_PROT_EXEC) vm_flags |= VM_FLAG_EXEC;
    if (!current_process->vm_space->is_kernel) vm_flags |= VM_FLAG_USER;
    
    size_t pages = length / PAGE_SIZE;
    for (size_t i = 0; i < pages; i++) {
        uint64_t page_addr = addr + i * PAGE_SIZE;
        if (vmm_protect_page(current_process->vm_space, page_addr, vm_flags) != OR_OK) {
            return -OR_EINVAL;
        }
    }
    
    return OR_OK;
}

// Stubs pour les syscalls restants (à implémenter plus tard)
int64_t sys_shm_create_impl(size_t size, uint32_t flags) {
    (void)size; (void)flags;
    return -OR_ENOSYS;
}

int64_t sys_shm_attach_impl(or_cap_t shm_cap, uint64_t addr, uint32_t flags) {
    (void)shm_cap; (void)addr; (void)flags;
    return -OR_ENOSYS;
}

int64_t sys_shm_detach_impl(uint64_t addr) {
    (void)addr;
    return -OR_ENOSYS;
}

int64_t sys_madvise_impl(uint64_t addr, size_t length, uint32_t advice) {
    (void)addr; (void)length; (void)advice;
    return -OR_ENOSYS;
}

int64_t sys_port_share_impl(or_cap_t port, uint64_t target_pid) {
    (void)port; (void)target_pid;
    return -OR_ENOSYS;
}

int64_t sys_msg_forward_impl(or_cap_t source_port, or_cap_t dest_port) {
    (void)source_port; (void)dest_port;
    return -OR_ENOSYS;
}

int64_t sys_clock_get_impl(uint32_t clock_id, uint64_t* timestamp) {
    if (!timestamp || !mmu_is_valid_addr((uint64_t)timestamp)) {
        return -OR_EFAULT;
    }
    
    *timestamp = arch_get_timestamp();
    return OR_OK;
}

int64_t sys_timer_create_impl(uint32_t clock_id, uint64_t* timer_id) {
    (void)clock_id; (void)timer_id;
    return -OR_ENOSYS;
}

int64_t sys_timer_start_impl(uint64_t timer_id, uint64_t timeout_ns, uint64_t interval_ns) {
    (void)timer_id; (void)timeout_ns; (void)interval_ns;
    return -OR_ENOSYS;
}

int64_t sys_timer_stop_impl(uint64_t timer_id) {
    (void)timer_id;
    return -OR_ENOSYS;
}

int64_t sys_nanosleep_impl(uint64_t nanoseconds) {
    scheduler_sleep_ns(nanoseconds);
    return OR_OK;
}

int64_t sys_io_submit_impl(void* io_context, uint32_t nr, void* iocbpp) {
    (void)io_context; (void)nr; (void)iocbpp;
    return -OR_ENOSYS;
}

int64_t sys_io_poll_impl(void* io_context, uint32_t min_nr, uint32_t max_nr, void* events, uint64_t timeout_ns) {
    (void)io_context; (void)min_nr; (void)max_nr; (void)events; (void)timeout_ns;
    return -OR_ENOSYS;
}

int64_t sys_io_cancel_impl(void* io_context, void* iocb) {
    (void)io_context; (void)iocb;
    return -OR_ENOSYS;
}

int64_t sys_obj_info_impl(or_cap_t cap, void* info_buffer, size_t buffer_size) {
    (void)cap; (void)info_buffer; (void)buffer_size;
    return -OR_ENOSYS;
}

int64_t sys_obj_dup_impl(or_cap_t cap) {
    (void)cap;
    return -OR_ENOSYS;
}

int64_t sys_obj_close_impl(or_cap_t cap) {
    process_t* current_process = scheduler_get_current_process();
    if (!current_process || cap >= MAX_HANDLES) {
        return -OR_EINVAL;
    }
    
    if (current_process->handles[cap].type == HANDLE_TYPE_NONE) {
        return -OR_EINVAL; // Déjà fermé
    }
    
    // Nettoyer le handle selon son type
    handle_cleanup(&current_process->handles[cap]);
    current_process->handles[cap].type = HANDLE_TYPE_NONE;
    
    return OR_OK;
}

int64_t sys_cap_grant_impl(uint64_t target_pid, or_cap_t cap, uint32_t permissions) {
    (void)target_pid; (void)cap; (void)permissions;
    return -OR_ENOSYS;
}

int64_t sys_cap_revoke_impl(uint64_t target_pid, or_cap_t cap) {
    (void)target_pid; (void)cap;
    return -OR_ENOSYS;
}

int64_t sys_cap_query_impl(or_cap_t cap, uint32_t* permissions) {
    (void)cap; (void)permissions;
    return -OR_ENOSYS;
}

int64_t sys_sandbox_load_impl(const char* policy_path) {
    (void)policy_path;
    return -OR_ENOSYS;
}

int64_t sys_audit_emit_impl(uint32_t event_type, const void* event_data, size_t data_size) {
    (void)event_type; (void)event_data; (void)data_size;
    return -OR_ENOSYS;
}

int64_t sys_dbg_trace_impl(uint32_t trace_type, const void* trace_data, size_t data_size) {
    (void)trace_type; (void)trace_data; (void)data_size;
    return -OR_ENOSYS;
}

int64_t sys_random_impl(void* buffer, size_t size) {
    if (!buffer || !mmu_is_valid_addr((uint64_t)buffer) || size == 0) {
        return -OR_EINVAL;
    }
    
    // Générateur pseudo-aléatoire simple pour commencer
    static uint64_t rand_state = 0x123456789ABCDEF0ULL;
    
    uint8_t* bytes = (uint8_t*)buffer;
    for (size_t i = 0; i < size; i++) {
        rand_state = rand_state * 1103515245 + 12345;
        bytes[i] = (uint8_t)(rand_state >> 32);
    }
    
    return (int64_t)size;
}

// Initialiser le système d'appels système
void syscalls_init(void) {
    kinfo("Initializing system call interface");
    
    // Configurer l'instruction SYSCALL/SYSRET (x86_64)
    arch_setup_syscall_interface();
    
    kinfo("Syscall interface initialized (%d max syscalls)", MAX_SYSCALLS);
}
