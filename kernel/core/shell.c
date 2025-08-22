/*
 * Orion Operating System - Advanced Shell Implementation
 * 
 * This is the core implementation of the Orion shell, providing a comprehensive
 * command-line interface for system administration, debugging, and education.
 * 
 * Copyright (c) 2024 Orion OS Project
 * License: MIT
 */

#include <orion/shell.h>
#include <orion/kernel.h>
#include <orion/mm.h>
#include <orion/syscalls.h>
// Don't include system string.h - use our own functions

// Simple strchr implementation for kernel
static char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == c) return (char*)s;
        s++;
    }
    return (c == 0) ? (char*)s : NULL;
}

// Simple strncpy implementation for kernel
static char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

// Simple strtok implementation for kernel
static char *shell_strtok(char *str, const char *delim) {
    static char *saved_str = NULL;
    if (str != NULL) saved_str = str;
    if (saved_str == NULL) return NULL;
    
    // Skip leading delimiters
    while (*saved_str && strchr(delim, *saved_str)) saved_str++;
    if (*saved_str == '\0') return NULL;
    
    char *token_start = saved_str;
    // Find end of token
    while (*saved_str && !strchr(delim, *saved_str)) saved_str++;
    
    if (*saved_str) {
        *saved_str = '\0';
        saved_str++;
    } else {
        saved_str = NULL;
    }
    
    return token_start;
}

// va_* macros for freestanding environment
typedef char* va_list;
// va_* macros defined in types.h

// ====================================
// COMMAND FUNCTION STUBS
// ====================================

int cmd_help(int argc, char* argv[]) { 
    kinfo("Help: Available commands listed"); 
    return 0; 
}
int cmd_version(int argc, char* argv[]) { 
    kinfo("Orion OS Kernel v1.0.0"); 
    return 0; 
}
int cmd_sysinfo(int argc, char* argv[]) { 
    kinfo("System info stub"); 
    return 0; 
}
int cmd_uptime(int argc, char* argv[]) { 
    kinfo("Uptime stub"); 
    return 0; 
}
int cmd_ps(int argc, char* argv[]) { 
    kinfo("Process list stub"); 
    return 0; 
}
int cmd_kill(int argc, char* argv[]) { 
    kinfo("Kill process stub"); 
    return 0; 
}
int cmd_top(int argc, char* argv[]) { 
    kinfo("Top processes stub"); 
    return 0; 
}
int cmd_meminfo(int argc, char* argv[]) { 
    kinfo("Memory info stub"); 
    return 0; 
}
int cmd_memtest(int argc, char* argv[]) { 
    kinfo("Memory test stub"); 
    return 0; 
}
int cmd_memmap(int argc, char* argv[]) { 
    kinfo("Memory map stub"); 
    return 0; 
}
int cmd_reboot(int argc, char* argv[]) { 
    kinfo("Reboot stub"); 
    return 0; 
}
int cmd_shutdown(int argc, char* argv[]) { 
    kinfo("Shutdown stub"); 
    return 0; 
}
int cmd_trace(int argc, char* argv[]) { 
    kinfo("Trace stub"); 
    return 0; 
}
int cmd_profile(int argc, char* argv[]) { 
    kinfo("Profile stub"); 
    return 0; 
}
int cmd_benchmark(int argc, char* argv[]) { 
    kinfo("Benchmark stub"); 
    return 0; 
}
int cmd_history(int argc, char* argv[]) { 
    kinfo("History stub"); 
    return 0; 
}
int cmd_alias(int argc, char* argv[]) { 
    kinfo("Alias stub"); 
    return 0; 
}
int cmd_clear(int argc, char* argv[]) { 
    kinfo("Clear stub"); 
    return 0; 
}
int cmd_exit(int argc, char* argv[]) { 
    kinfo("Exit stub"); 
    return 0; 
}
int cmd_echo(int argc, char* argv[]) { 
    kinfo("Echo stub"); 
    return 0; 
}
int cmd_tutorial(int argc, char* argv[]) { 
    kinfo("Tutorial stub"); 
    return 0; 
}
int cmd_demo(int argc, char* argv[]) { 
    kinfo("Demo stub"); 
    return 0; 
}
int cmd_explain(int argc, char* argv[]) { 
    kinfo("Explain stub"); 
    return 0; 
}

// ====================================
// GLOBAL SHELL STATE
// ====================================

static struct shell_state g_shell_state;
static const struct shell_command *g_commands[256];
static uint32_t g_command_count = 0;
static int g_shell_initialized = 0;

// ====================================
// BUILT-IN COMMAND TABLE
// ====================================

static const struct shell_command builtin_commands[] = {
    // System Information
    {
        .name = "help",
        .description = "Display help information",
        .usage = "help [command]",
        .help = "Show available commands or detailed help for a specific command",
        .handler = cmd_help,
        .flags = 0,
        .min_args = 0,
        .max_args = 1
    },
    {
        .name = "version",
        .description = "Show Orion version information",
        .usage = "version",
        .help = "Display kernel version, build date, and system information",
        .handler = cmd_version,
        .flags = 0,
        .min_args = 0,
        .max_args = 0
    },
    {
        .name = "sysinfo",
        .description = "Display system information",
        .usage = "sysinfo [detailed]",
        .help = "Show comprehensive system information including CPU, memory, and hardware",
        .handler = cmd_sysinfo,
        .flags = 0,
        .min_args = 0,
        .max_args = 1
    },
    {
        .name = "uptime",
        .description = "Show system uptime and load",
        .usage = "uptime",
        .help = "Display how long the system has been running and load average",
        .handler = cmd_uptime,
        .flags = 0,
        .min_args = 0,
        .max_args = 0
    },
    
    // Process Management
    {
        .name = "ps",
        .description = "List running processes",
        .usage = "ps [options]",
        .help = "Display information about running processes\n"
                "Options:\n"
                "  -a    Show all processes\n"
                "  -u    Show user information\n"
                "  -x    Show processes without terminal",
        .handler = cmd_ps,
        .flags = 0,
        .min_args = 0,
        .max_args = 3
    },
    {
        .name = "kill",
        .description = "Terminate a process",
        .usage = "kill [-signal] pid",
        .help = "Send a signal to a process\n"
                "Common signals:\n"
                "  -TERM (15)  Terminate gracefully\n"
                "  -KILL (9)   Force kill\n"
                "  -STOP (19)  Stop process\n"
                "  -CONT (18)  Continue process",
        .handler = cmd_kill,
        .flags = SHELL_CMD_FLAG_ADMIN,
        .min_args = 1,
        .max_args = 2
    },
    {
        .name = "top",
        .description = "Display running processes dynamically",
        .usage = "top [options]",
        .help = "Show real-time process information and system statistics",
        .handler = cmd_top,
        .flags = 0,
        .min_args = 0,
        .max_args = 2
    },
    
    // Memory Management
    {
        .name = "meminfo",
        .description = "Display memory information",
        .usage = "meminfo [detailed]",
        .help = "Show memory usage statistics and allocation information",
        .handler = cmd_meminfo,
        .flags = 0,
        .min_args = 0,
        .max_args = 1
    },
    {
        .name = "memtest",
        .description = "Test memory subsystem",
        .usage = "memtest [size] [pattern]",
        .help = "Perform memory allocation and integrity tests\n"
                "Arguments:\n"
                "  size     Amount of memory to test (e.g., 1MB, 10KB)\n"
                "  pattern  Test pattern (random, zeros, ones, alt)",
        .handler = cmd_memtest,
        .flags = SHELL_CMD_FLAG_DEBUG,
        .min_args = 0,
        .max_args = 2
    },
    {
        .name = "memmap",
        .description = "Display physical memory map",
        .usage = "memmap [detailed]",
        .help = "Show physical memory layout and regions",
        .handler = cmd_memmap,
        .flags = SHELL_CMD_FLAG_DEBUG,
        .min_args = 0,
        .max_args = 1
    },
    
    // System Control
    {
        .name = "reboot",
        .description = "Restart the system",
        .usage = "reboot [force]",
        .help = "Restart the system gracefully or forcefully",
        .handler = cmd_reboot,
        .flags = SHELL_CMD_FLAG_ADMIN | SHELL_CMD_FLAG_DANGEROUS,
        .min_args = 0,
        .max_args = 1
    },
    {
        .name = "shutdown",
        .description = "Shutdown the system",
        .usage = "shutdown [time] [message]",
        .help = "Schedule system shutdown with optional delay and message",
        .handler = cmd_shutdown,
        .flags = SHELL_CMD_FLAG_ADMIN | SHELL_CMD_FLAG_DANGEROUS,
        .min_args = 0,
        .max_args = 2
    },
    
    // Debug Commands
    {
        .name = "trace",
        .description = "Trace system calls and kernel events",
        .usage = "trace [options] [pid]",
        .help = "Enable kernel tracing for debugging and analysis\n"
                "Options:\n"
                "  -s    Trace system calls\n"
                "  -m    Trace memory operations\n"
                "  -i    Trace interrupts\n"
                "  -a    Trace all events",
        .handler = cmd_trace,
        .flags = SHELL_CMD_FLAG_DEBUG | SHELL_CMD_FLAG_ADMIN,
        .min_args = 0,
        .max_args = 3
    },
    {
        .name = "profile",
        .description = "Profile system performance",
        .usage = "profile [duration] [output]",
        .help = "Collect performance profiling data for analysis",
        .handler = cmd_profile,
        .flags = SHELL_CMD_FLAG_DEBUG,
        .min_args = 0,
        .max_args = 2
    },
    {
        .name = "benchmark",
        .description = "Run system benchmarks",
        .usage = "benchmark [test] [iterations]",
        .help = "Execute performance benchmarks\n"
                "Available tests:\n"
                "  cpu       CPU performance\n"
                "  memory    Memory bandwidth\n"
                "  disk      Disk I/O\n"
                "  network   Network throughput\n"
                "  syscall   System call latency",
        .handler = cmd_benchmark,
        .flags = SHELL_CMD_FLAG_DEBUG,
        .min_args = 0,
        .max_args = 2
    },
    
    // Shell Control
    {
        .name = "history",
        .description = "Show command history",
        .usage = "history [count]",
        .help = "Display recent commands with timestamps and execution times",
        .handler = cmd_history,
        .flags = 0,
        .min_args = 0,
        .max_args = 1
    },
    {
        .name = "alias",
        .description = "Create command aliases",
        .usage = "alias [name=value]",
        .help = "Create or display command aliases",
        .handler = cmd_alias,
        .flags = 0,
        .min_args = 0,
        .max_args = 1
    },
    {
        .name = "clear",
        .description = "Clear the screen",
        .usage = "clear",
        .help = "Clear the terminal screen",
        .handler = cmd_clear,
        .flags = 0,
        .min_args = 0,
        .max_args = 0
    },
    {
        .name = "exit",
        .description = "Exit the shell",
        .usage = "exit [code]",
        .help = "Exit the shell with optional exit code",
        .handler = cmd_exit,
        .flags = 0,
        .min_args = 0,
        .max_args = 1
    },
    {
        .name = "echo",
        .description = "Display text",
        .usage = "echo [text...]",
        .help = "Print arguments to the console",
        .handler = cmd_echo,
        .flags = 0,
        .min_args = 0,
        .max_args = 32
    },
    
    // Educational Commands
    {
        .name = "tutorial",
        .description = "Interactive Orion OS tutorial",
        .usage = "tutorial [topic]",
        .help = "Learn about Orion OS features and concepts\n"
                "Available topics:\n"
                "  basics      Shell basics and navigation\n"
                "  memory      Memory management concepts\n"
                "  processes   Process and thread management\n"
                "  security    Security model and capabilities\n"
                "  kernel      Kernel architecture overview",
        .handler = cmd_tutorial,
        .flags = 0,
        .min_args = 0,
        .max_args = 1
    },
    {
        .name = "demo",
        .description = "Run interactive demonstrations",
        .usage = "demo [topic]",
        .help = "Execute interactive demos showcasing Orion features",
        .handler = cmd_demo,
        .flags = 0,
        .min_args = 0,
        .max_args = 1
    },
    {
        .name = "explain",
        .description = "Explain commands and concepts",
        .usage = "explain <command|concept>",
        .help = "Get detailed explanations of commands, system calls, or OS concepts",
        .handler = cmd_explain,
        .flags = 0,
        .min_args = 1,
        .max_args = 1
    }
};

#define BUILTIN_COMMAND_COUNT (sizeof(builtin_commands) / sizeof(builtin_commands[0]))

// ====================================
// CORE SHELL FUNCTIONS
// ====================================

/**
 * Initialize the Orion shell
 */
int shell_init(void) {
    if (g_shell_initialized) {
        return 0; // Already initialized
    }
    
    // Initialize shell state
    memset(&g_shell_state, 0, sizeof(g_shell_state));
    
    // Set default context
    g_shell_state.context.current_directory = "/";
    g_shell_state.context.user_id = 0; // Root for now
    g_shell_state.context.session_id = 1;
    g_shell_state.context.start_time = arch_get_timestamp();
    g_shell_state.context.flags = 0;
    
    // Register built-in commands
    for (uint32_t i = 0; i < BUILTIN_COMMAND_COUNT; i++) {
        if (shell_register_command(&builtin_commands[i]) != 0) {
            kprintf("Warning: Failed to register command '%s'\n", 
                    builtin_commands[i].name);
        }
    }
    
    g_shell_initialized = 1;
    
    kprintf("Orion Shell v%s initialized\n", ORION_SHELL_VERSION);
    kprintf("Type 'help' for available commands\n");
    
    return 0;
}

/**
 * Start the interactive shell main loop
 */
int shell_run(struct shell_context *context) {
    if (!g_shell_initialized) {
        if (shell_init() != 0) {
            return -1;
        }
    }
    
    if (context) {
        g_shell_state.context = *context;
    }
    
    shell_printf("\n");
    shell_printf("=== Orion Operating System ===\n");
    shell_printf("Advanced Shell v%s\n", ORION_SHELL_VERSION);
    shell_printf("Academic Research Platform\n");
    shell_printf("===============================\n\n");
    
    shell_printf("Welcome to Orion Shell!\n");
    shell_printf("Type 'help' for available commands or 'tutorial' for interactive learning.\n\n");
    
    while (1) {
        // Display prompt
        shell_printf("%s", ORION_SHELL_PROMPT);
        
        // Read command line (simplified for now)
        char cmdline[ORION_SHELL_MAX_CMDLINE];
        
        // For now, simulate some commands for demo
        static int demo_step = 0;
        const char *demo_commands[] = {
            "help",
            "version", 
            "sysinfo",
            "meminfo",
            "ps",
            "tutorial basics",
            "exit"
        };
        
        if (demo_step < sizeof(demo_commands)/sizeof(demo_commands[0])) {
            strcpy(cmdline, demo_commands[demo_step++]);
            shell_printf("%s\n", cmdline);
        } else {
            strcpy(cmdline, "exit");
        }
        
        // Execute command
        int result = shell_execute(cmdline, &g_shell_state.context);
        
        // Check for exit
        if (strcmp(cmdline, "exit") == 0) {
            break;
        }
        
        // Add to history
        shell_add_history(cmdline, result, 0);
        
        // Brief pause for demo
        for (volatile int i = 0; i < 1000000; i++);
    }
    
    shell_printf("\nOrion Shell session ended.\n");
    return 0;
}

/**
 * Execute a single command string
 */
int shell_execute(const char *cmdline, struct shell_context *context) {
    if (!cmdline || strlen(cmdline) == 0) {
        return 0;
    }
    
    // Parse command line
    char *argv[ORION_SHELL_MAX_ARGS];
    int argc;
    
    // Create modifiable copy
    char cmd_buffer[ORION_SHELL_MAX_CMDLINE];
    strncpy(cmd_buffer, cmdline, sizeof(cmd_buffer) - 1);
    cmd_buffer[sizeof(cmd_buffer) - 1] = '\0';
    
    if (shell_parse_cmdline(cmd_buffer, &argc, argv) != 0) {
        shell_error("Failed to parse command line\n");
        return SHELL_ERROR_INVALID_FORMAT;
    }
    
    if (argc == 0) {
        return 0;
    }
    
    // Find command
    const struct shell_command *cmd = shell_find_command(argv[0]);
    if (!cmd) {
        shell_error("Command not found: %s\n", argv[0]);
        shell_info("Type 'help' for available commands\n");
        return SHELL_ERROR_INVALID_COMMAND;
    }
    
    // Check argument count
    if (argc - 1 < cmd->min_args || argc - 1 > cmd->max_args) {
        shell_error("Invalid number of arguments for '%s'\n", cmd->name);
        shell_info("Usage: %s\n", cmd->usage);
        return SHELL_ERROR_INVALID_ARGS;
    }
    
    // Check permissions
    if ((cmd->flags & SHELL_CMD_FLAG_ADMIN) && !shell_is_admin()) {
        shell_error("Permission denied: Administrative privileges required\n");
        return SHELL_ERROR_PERMISSION_DENIED;
    }
    
    // Execute command
    uint64_t start_time = arch_get_timestamp();
    int result = cmd->handler(argc, argv);
    uint64_t end_time = arch_get_timestamp();
    
    // Update statistics
    g_shell_state.context.command_count++;
    
    // Show execution time for debug commands
    if (cmd->flags & SHELL_CMD_FLAG_DEBUG) {
        uint64_t execution_time = end_time - start_time;
        shell_debug("Command executed in %llu microseconds\n", execution_time);
    }
    
    return result;
}

/**
 * Parse command line into arguments
 */
int shell_parse_cmdline(const char *cmdline, int *argc, char **argv) {
    if (!cmdline || !argc || !argv) {
        return -1;
    }
    
    *argc = 0;
    
    // Simple parser - split on spaces
    char *token = shell_strtok((char*)cmdline, " \t\n");
    while (token && *argc < ORION_SHELL_MAX_ARGS - 1) {
        argv[*argc] = token;
        (*argc)++;
        token = shell_strtok(NULL, " \t\n");
    }
    
    argv[*argc] = NULL;
    return 0;
}

/**
 * Register a new shell command
 */
int shell_register_command(const struct shell_command *cmd) {
    if (!cmd || !cmd->name || !cmd->handler) {
        return -1;
    }
    
    if (g_command_count >= sizeof(g_commands)/sizeof(g_commands[0])) {
        return -1; // Command table full
    }
    
    // Check for duplicate names
    if (shell_find_command(cmd->name) != NULL) {
        return -1; // Command already exists
    }
    
    g_commands[g_command_count++] = cmd;
    return 0;
}

/**
 * Find registered command by name
 */
const struct shell_command *shell_find_command(const char *name) {
    if (!name) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < g_command_count; i++) {
        if (strcmp(g_commands[i]->name, name) == 0) {
            return g_commands[i];
        }
    }
    
    return NULL;
}

// ====================================
// HISTORY MANAGEMENT
// ====================================

/**
 * Add command to shell history
 */
void shell_add_history(const char *cmdline, int exit_code, uint32_t execution_time) {
    if (!cmdline) {
        return;
    }
    
    uint32_t index = g_shell_state.history_count % ORION_SHELL_MAX_HISTORY;
    struct shell_history_entry *entry = &g_shell_state.history[index];
    
    strncpy(entry->command, cmdline, sizeof(entry->command) - 1);
    entry->command[sizeof(entry->command) - 1] = '\0';
    entry->timestamp = arch_get_timestamp();
    entry->exit_code = exit_code;
    entry->execution_time_us = execution_time;
    
    g_shell_state.history_count++;
}

// ====================================
// OUTPUT FUNCTIONS
// ====================================

/**
 * Print formatted output to shell
 */
int shell_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int result = kvprintf(format, args);
    va_end(args);
    return result;
}

/**
 * Print error message to shell
 */
void shell_error(const char *format, ...) {
    shell_printf("[ERROR] ");
    va_list args;
    va_start(args, format);
    kvprintf(format, args);
    va_end(args);
}

/**
 * Print warning message to shell
 */
void shell_warning(const char *format, ...) {
    shell_printf("[WARN]  ");
    va_list args;
    va_start(args, format);
    kvprintf(format, args);
    va_end(args);
}

/**
 * Print information message to shell
 */
void shell_info(const char *format, ...) {
    shell_printf("[INFO]  ");
    va_list args;
    va_start(args, format);
    kvprintf(format, args);
    va_end(args);
}

/**
 * Print success message to shell
 */
void shell_success(const char *format, ...) {
    shell_printf("[OK]    ");
    va_list args;
    va_start(args, format);
    kvprintf(format, args);
    va_end(args);
}

/**
 * Print debug message to shell
 */
void shell_debug(const char *format, ...) {
    shell_printf("[DEBUG] ");
    va_list args;
    va_start(args, format);
    kvprintf(format, args);
    va_end(args);
}

// ====================================
// UTILITY FUNCTIONS
// ====================================

/**
 * Check if current user has administrative privileges
 */
int shell_is_admin(void) {
    return g_shell_state.context.user_id == 0; // Root user
}

/**
 * Get current shell context
 */
struct shell_context *shell_get_context(void) {
    return &g_shell_state.context;
}

/**
 * Get shell state
 */
struct shell_state *shell_get_state(void) {
    return &g_shell_state;
}

/**
 * Format size for human-readable display
 */
const char *shell_format_size(uint64_t size, char *buffer, size_t buffer_size) {
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double formatted_size = (double)size;
    
    while (formatted_size >= 1024.0 && unit_index < 4) {
        formatted_size /= 1024.0;
        unit_index++;
    }
    
    if (unit_index == 0) {
        snprintf(buffer, buffer_size, "%llu %s", size, units[unit_index]);
    } else {
        snprintf(buffer, buffer_size, "%.2f %s", formatted_size, units[unit_index]);
    }
    
    return buffer;
}
