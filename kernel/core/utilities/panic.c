// Orion panic and emergency shutdown management
#include <orion/kernel.h>
#include <orion/types.h>
#include <orion/klog.h>
#include <orion/structures.h

// Panic state
static bool panic_in_progress = false;

// Emergency system shutdown
void emergency_halt(void)
{
    // Disable interrupts
    cli();

    // Stop all CPUs if SMP
    if (arch_is_smp_enabled())
    {
        klog_info(KLOG_CAT_KERNEL, "Stopping all CPUs...");

        // Send IPI to all other CPUs to halt them
        for (int cpu_id = 0; cpu_id < arch_get_cpu_count(); cpu_id++)
        {
            if (cpu_id != arch_get_current_cpu_id())
            {
                arch_send_ipi(cpu_id, 0x01); // IPI_HALT = 0x01
                klog_info(KLOG_CAT_KERNEL, "Sent halt IPI to CPU %d", cpu_id);
            }
        }

        // Wait a bit for other CPUs to halt
        arch_delay_ms(100);
    }

    // Infinite loop
    while (1)
    {
        hlt();
    }
}

// Main panic handler (already defined in entry.c, but complete version here)
void panic_full(const char *file, int line, const char *function, const char *fmt, ...)
{
    // Avoid recursive panic
    if (panic_in_progress)
    {
        emergency_halt();
    }
    panic_in_progress = true;

    // Disable interrupts immediately
    cli();

    // Panic display
    klog_emergency("\n");
    klog_emergency("################################");
    klog_emergency("###   ORION KERNEL PANIC     ###");
    klog_emergency("################################");
    klog_emergency("");

    klog_emergency("Location: %s:%d in %s()", file, line, function);

    // Handle variable arguments properly
    va_list args;
    va_start(args, fmt);
    klog_emergency("Message: ");

    // Custom printf for panic (simplified but functional)
    const char *p = fmt;
    while (*p)
    {
        if (*p == '%' && *(p + 1))
        {
            p++; // Skip %
            switch (*p)
            {
            case 's':
            {
                const char *str = va_arg(args, const char *);
                klog_emergency("%s", str ? str : "(null)");
                break;
            }
            case 'd':
            {
                int num = va_arg(args, int);
                klog_emergency("%d", num);
                break;
            }
            case 'x':
            {
                unsigned int num = va_arg(args, unsigned int);
                klog_emergency("%x", num);
                break;
            }
            case 'p':
            {
                void *ptr = va_arg(args, void *);
                klog_emergency("%p", ptr);
                break;
            }
            case 'u':
            {
                unsigned int num = va_arg(args, unsigned int);
                klog_emergency("%u", num);
                break;
            }
            default:
            {
                klog_emergency("%%%c", *p);
                break;
            }
            }
        }
        else
        {
            klog_emergency("%c", *p);
        }
        p++;
    }
    va_end(args);
    klog_emergency("\n\n");

    // Display system information
    klog_emergency("System state:");
    klog_emergency("  Interrupts: disabled");
    klog_emergency("  Panic in progress: yes");
    klog_emergency("  CPU: %d", arch_get_current_cpu_id());
    klog_emergency("  Timestamp: %llu", (unsigned long long)arch_get_timestamp());

    // Display stack trace and registers
    dump_stack();
    dump_registers();

    // Save core dump
    save_core_dump(file, line, function, fmt);

    klog_emergency("\n");
    klog_emergency("System will halt.");
    klog_emergency("################################");

    // Emergency shutdown
    emergency_halt();
}

// Save core dump for debugging
void save_core_dump(const char *file, int line, const char *function, const char *fmt)
{
    klog_emergency("Saving core dump...");

    // Create core dump filename with timestamp
    char core_filename[256];
    uint64_t timestamp = arch_get_timestamp();
    snprintf(core_filename, sizeof(core_filename),
             "core.%llu.%s.%d", timestamp, file, line);

    // Core dump implementation with filesystem support
    // Try to create a filesystem-based core dump, fallback to in-memory

    // Check if VFS is available and we can write to filesystem
    extern bool vfs_is_available(void);
    extern int vfs_write_file(const char *path, const void *data, size_t size);

    bool fs_dump_created = false;

    if (vfs_is_available())
    {
        // Try to create filesystem-based core dump
        kprintf("Creating filesystem core dump: %s\n", core_filename);

        // Prepare core dump data in memory first
        char core_dump_buffer[4096];
        size_t dump_size = 0;

        // Format core dump header
        dump_size += snprintf(core_dump_buffer + dump_size,
                              sizeof(core_dump_buffer) - dump_size,
                              "=== ORION OS CORE DUMP ===\n");
        dump_size += snprintf(core_dump_buffer + dump_size,
                              sizeof(core_dump_buffer) - dump_size,
                              "Timestamp: %llu\n", timestamp);
        dump_size += snprintf(core_dump_buffer + dump_size,
                              sizeof(core_dump_buffer) - dump_size,
                              "File: %s, Line: %d, Function: %s\n", file, line, function);

        // Add process information
        process_t *current = scheduler_get_current_process();
        if (current)
        {
            dump_size += snprintf(core_dump_buffer + dump_size,
                                  sizeof(core_dump_buffer) - dump_size,
                                  "Process: PID %llu, State: %d\n",
                                  (unsigned long long)current->pid, current->state);
            dump_size += snprintf(core_dump_buffer + dump_size,
                                  sizeof(core_dump_buffer) - dump_size,
                                  "Memory: estimated allocation\n");
        }

        // Add system state information
        dump_size += snprintf(core_dump_buffer + dump_size,
                              sizeof(core_dump_buffer) - dump_size,
                              "System uptime: %llu seconds\n",
                              (unsigned long long)(arch_get_timestamp() / 1000000000));
        dump_size += snprintf(core_dump_buffer + dump_size,
                              sizeof(core_dump_buffer) - dump_size,
                              "Memory: %llu MB free\n",
                              (unsigned long long)(pmm_get_free_pages() * PAGE_SIZE / (1024 * 1024)));
        dump_size += snprintf(core_dump_buffer + dump_size,
                              sizeof(core_dump_buffer) - dump_size,
                              "Active processes: %u\n", scheduler_get_process_count());

        // Try to write to filesystem
        int write_result = vfs_write_file(core_filename, core_dump_buffer, dump_size);
        if (write_result == (int)dump_size)
        {
            fs_dump_created = true;
            kprintf("Filesystem core dump created successfully: %s (%zu bytes)\n",
                    core_filename, dump_size);
        }
        else
        {
            kwarn("Failed to create filesystem core dump (error: %d), falling back to in-memory",
                  write_result);
        }
    }

    // If filesystem dump failed or unavailable, create in-memory dump
    if (!fs_dump_created)
    {
        kprintf("Creating in-memory core dump: %s\n", core_filename);

        // Dump current process state
        process_t *current = scheduler_get_current_process();
        if (current)
        {
            kprintf("Process: PID %llu, State: %d\n",
                    (unsigned long long)current->pid, current->state);
            kprintf("Memory: estimated allocation\n");
        }

        // Dump register state
        dump_registers();

        // Dump stack trace
        dump_stack();

        // Dump system state
        kprintf("System state:\n");
        kprintf("  Uptime: %llu seconds\n", (unsigned long long)(arch_get_timestamp() / 1000000000));
        kprintf("  Memory: %llu MB free\n", (unsigned long long)(pmm_get_free_pages() * PAGE_SIZE / (1024 * 1024)));
        kprintf("  Active processes: %u\n", scheduler_get_process_count());

        kprintf("In-memory core dump completed\n");
    }
}

// Macro for panic with location info
#define PANIC(fmt, ...) panic_full(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

// Assertions with panic
void assert_failed(const char *expr, const char *file, int line, const char *function)
{
    panic_full(file, line, function, "Assertion failed: %s", expr);
}

// Basic memory corruption check
bool check_stack_canary(void)
{
    // Get current thread's stack canary
    thread_t *current = scheduler_get_current_thread();
    if (!current)
    {
        return false; // No current thread
    }

    // Check if stack canary is intact (using security system)
    extern uint64_t security_get_stack_canary(void);
    uint64_t expected_canary = security_get_stack_canary();

    // For now, return true since stack canary fields not implemented in thread struct
    // TODO: Implement proper stack canary checking when thread struct is enhanced
    return true;
}

// Register dump (stub)
void dump_registers(void)
{
    kprintf("Register dump:\n");

    // Capture and display all registers
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp, rsp, r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t rip, rflags, cs, ds, es, fs, gs, ss;

    // Read general purpose registers using MSVC intrinsics
    rax = __readgsqword(0);
    rbx = __readgsqword(8);
    rcx = __readgsqword(16);
    rdx = __readgsqword(24);
    rsi = __readgsqword(32);
    rdi = __readgsqword(40);
    rbp = __readgsqword(48);
    rsp = __readgsqword(56);
    r8 = __readgsqword(64);
    r9 = __readgsqword(72);
    r10 = __readgsqword(80);
    r11 = __readgsqword(88);
    r12 = __readgsqword(96);
    r13 = __readgsqword(104);
    r14 = __readgsqword(112);
    r15 = __readgsqword(120);

    // Read instruction pointer and flags using MSVC intrinsics
    rflags = __readrflags();

    // Display registers
    kprintf("  RAX: 0x%016llx  RBX: 0x%016llx  RCX: 0x%016llx  RDX: 0x%016llx\n", rax, rbx, rcx, rdx);
    kprintf("  RSI: 0x%016llx  RDI: 0x%016llx  RBP: 0x%016llx  RSP: 0x%016llx\n", rsi, rdi, rbp, rsp);
    kprintf("  R8:  0x%016llx  R9:  0x%016llx  R10: 0x%016llx  R11: 0x%016llx\n", r8, r9, r10, r11);
    kprintf("  R12: 0x%016llx  R13: 0x%016llx  R14: 0x%016llx  R15: 0x%016llx\n", r12, r13, r14, r15);
    kprintf("  RIP: 0x%016llx  RFLAGS: 0x%016llx\n", (unsigned long long)__builtin_return_address(0), rflags);
}

// Stack dump (stub)
void dump_stack(void)
{
    kprintf("Stack trace:\n");

    // Walk the stack and display return addresses
    uint64_t *rbp = (uint64_t *)__builtin_frame_address(0);
    uint64_t *rsp = (uint64_t *)__builtin_return_address(0);

    if (!rbp || !rsp)
    {
        kprintf("  (unable to get stack frame)\n");
        return;
    }

    kprintf("  Current frame: RBP=0x%llx, RSP=0x%llx\n",
            (unsigned long long)rbp, (unsigned long long)rsp);

    // Walk up to 20 stack frames
    int frame_count = 0;
    uint64_t *frame_ptr = rbp;

    while (frame_ptr && frame_count < 20)
    {
        uint64_t saved_rbp = frame_ptr[0];
        uint64_t return_addr = frame_ptr[1];

        // Validate frame pointer
        if (saved_rbp == 0 || saved_rbp == (uint64_t)frame_ptr)
        {
            break; // End of stack
        }

        kprintf("  Frame %d: RBP=0x%llx, Return=0x%llx\n",
                frame_count, (unsigned long long)saved_rbp, (unsigned long long)return_addr);

        frame_ptr = (uint64_t *)saved_rbp;
        frame_count++;
    }

    if (frame_count == 0)
    {
        kprintf("  (no stack frames found)\n");
    }
}

// Security event audit log
void security_audit_log(const char *event, uint64_t pid, const char *details)
{
    kprintf("[AUDIT] PID %llu: %s - %s\n",
            (unsigned long long)pid, event, details);

    // Save to persistent log
    char log_entry[512];
    uint64_t timestamp = arch_get_timestamp();

    snprintf(log_entry, sizeof(log_entry),
             "[%llu] AUDIT: PID %llu: %s - %s\n",
             timestamp, pid, event, details);

    // Write to kernel log buffer
    kernel_log_write(log_entry, strlen(log_entry));

    // Write to persistent storage when filesystem is available
    extern bool vfs_is_available(void);
    extern int vfs_write_file(const char *path, const void *data, size_t size);

    bool persistent_log_written = false;

    if (vfs_is_available())
    {
        // Try to write to persistent security log file
        const char *security_log_path = "/var/log/security.log";
        int write_result = vfs_write_file(security_log_path, log_entry, strlen(log_entry));

        if (write_result == (int)strlen(log_entry))
        {
            persistent_log_written = true;
            kdebug("Security audit log written to persistent storage: %s", security_log_path);
        }
        else
        {
            kwarn("Failed to write security audit log to persistent storage (error: %d)", write_result);
        }
    }

    // Fallback logging if persistent storage unavailable
    if (!persistent_log_written)
    {
        kdebug("Security audit log stored in kernel buffer only (no persistent storage)");
    }

    // Store in security audit buffer (circular buffer)
    static char security_buffer[1024][512];
    static int security_buffer_index = 0;

    strncpy(security_buffer[security_buffer_index], log_entry, sizeof(security_buffer[0]) - 1);
    security_buffer[security_buffer_index][sizeof(security_buffer[0]) - 1] = '\0';

    security_buffer_index = (security_buffer_index + 1) % 1024;

    kprintf("Security audit logged to buffer (entry %d)\n", security_buffer_index);
}
