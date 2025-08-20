/*
 * Orion Operating System - Advanced Shell Interface
 *
 * A comprehensive, production-grade shell interface for the Orion kernel.
 * Designed for academic research, system debugging, and advanced OS development.
 *
 * Features:
 * - Command parsing and execution
 * - Built-in system commands
 * - Memory management utilities
 * - Process/thread inspection
 * - Performance profiling
 * - Security analysis tools
 * - Educational debugging features
 *
 * Copyright (c) 2024 Orion OS Project
 * License: MIT
 */

#ifndef ORION_SHELL_H
#define ORION_SHELL_H

#include <orion/types.h>
#include <orion/kernel.h>

// ====================================
// SHELL CONFIGURATION
// ====================================

#define ORION_SHELL_VERSION "1.0.0-advanced"
#define ORION_SHELL_PROMPT "orion$ "
#define ORION_SHELL_MAX_CMDLINE 512
#define ORION_SHELL_MAX_ARGS 32
#define ORION_SHELL_MAX_HISTORY 100
#define ORION_SHELL_MAX_ALIAS 50
#define ORION_SHELL_BUFFER_SIZE 4096

// ====================================
// COMMAND STRUCTURE
// ====================================

/**
 * Shell command handler function prototype
 */
typedef int (*shell_cmd_handler_t)(int argc, char **argv);

/**
 * Command descriptor structure
 */
struct shell_command
{
    const char *name;            // Command name
    const char *description;     // Brief description
    const char *usage;           // Usage syntax
    const char *help;            // Detailed help text
    shell_cmd_handler_t handler; // Command handler function
    uint32_t flags;              // Command flags (admin, debug, etc.)
    uint32_t min_args;           // Minimum required arguments
    uint32_t max_args;           // Maximum allowed arguments
};

/**
 * Command flags
 */
#define SHELL_CMD_FLAG_ADMIN 0x00000001        // Requires admin privileges
#define SHELL_CMD_FLAG_DEBUG 0x00000002        // Debug command
#define SHELL_CMD_FLAG_HIDDEN 0x00000004       // Hidden from help
#define SHELL_CMD_FLAG_DANGEROUS 0x00000008    // Potentially dangerous
#define SHELL_CMD_FLAG_EXPERIMENTAL 0x00000010 // Experimental feature

// ====================================
// SHELL STATE MANAGEMENT
// ====================================

/**
 * Shell execution context
 */
struct shell_context
{
    char *current_directory; // Current working directory
    uint32_t user_id;        // Current user ID
    uint32_t session_id;     // Shell session ID
    uint64_t start_time;     // Session start timestamp
    uint32_t command_count;  // Total commands executed
    uint32_t flags;          // Shell state flags
    void *private_data;      // Shell-specific data
};

/**
 * Shell history entry
 */
struct shell_history_entry
{
    char command[ORION_SHELL_MAX_CMDLINE];
    uint64_t timestamp;
    uint32_t exit_code;
    uint32_t execution_time_us;
};

/**
 * Command alias structure
 */
struct shell_alias
{
    char name[64];
    char expansion[256];
    uint32_t usage_count;
    uint64_t created_time;
};

/**
 * Shell state structure
 */
struct shell_state
{
    struct shell_context context;
    struct shell_history_entry history[ORION_SHELL_MAX_HISTORY];
    struct shell_alias aliases[ORION_SHELL_MAX_ALIAS];
    uint32_t history_count;
    uint32_t history_index;
    uint32_t alias_count;
    char input_buffer[ORION_SHELL_MAX_CMDLINE];
    char output_buffer[ORION_SHELL_BUFFER_SIZE];
    uint32_t cursor_position;
    uint32_t flags;
};

// ====================================
// CORE SHELL FUNCTIONS
// ====================================

/**
 * Initialize the Orion shell
 * @return 0 on success, negative error code on failure
 */
int shell_init(void);

/**
 * Start the interactive shell main loop
 * @param context Initial shell context (can be NULL)
 * @return 0 on normal exit, negative error code on failure
 */
int shell_run(struct shell_context *context);

/**
 * Execute a single command string
 * @param cmdline Command line to execute
 * @param context Shell context (can be NULL)
 * @return Command exit code
 */
int shell_execute(const char *cmdline, struct shell_context *context);

/**
 * Parse command line into arguments
 * @param cmdline Input command line
 * @param argc Output argument count
 * @param argv Output argument array (pre-allocated)
 * @return 0 on success, negative error code on failure
 */
int shell_parse_cmdline(const char *cmdline, int *argc, char **argv);

/**
 * Register a new shell command
 * @param cmd Command descriptor
 * @return 0 on success, negative error code on failure
 */
int shell_register_command(const struct shell_command *cmd);

/**
 * Unregister a shell command
 * @param name Command name to unregister
 * @return 0 on success, negative error code on failure
 */
int shell_unregister_command(const char *name);

/**
 * Find registered command by name
 * @param name Command name to find
 * @return Command descriptor or NULL if not found
 */
const struct shell_command *shell_find_command(const char *name);

// ====================================
// HISTORY AND ALIAS MANAGEMENT
// ====================================

/**
 * Add command to shell history
 * @param cmdline Command line
 * @param exit_code Command exit code
 * @param execution_time Execution time in microseconds
 */
void shell_add_history(const char *cmdline, int exit_code, uint32_t execution_time);

/**
 * Get command from history by index
 * @param index History index (0 = most recent)
 * @return History entry or NULL if invalid index
 */
const struct shell_history_entry *shell_get_history(uint32_t index);

/**
 * Clear shell history
 */
void shell_clear_history(void);

/**
 * Add or update command alias
 * @param name Alias name
 * @param expansion Alias expansion
 * @return 0 on success, negative error code on failure
 */
int shell_add_alias(const char *name, const char *expansion);

/**
 * Remove command alias
 * @param name Alias name to remove
 * @return 0 on success, negative error code on failure
 */
int shell_remove_alias(const char *name);

/**
 * Expand aliases in command line
 * @param cmdline Input/output command line buffer
 * @param buffer_size Size of command line buffer
 * @return 0 on success, negative error code on failure
 */
int shell_expand_aliases(char *cmdline, size_t buffer_size);

// ====================================
// OUTPUT AND FORMATTING
// ====================================

/**
 * Print formatted output to shell
 * @param format Printf-style format string
 * @param ... Format arguments
 * @return Number of characters printed
 */
int shell_printf(const char *format, ...);

/**
 * Print error message to shell
 * @param format Printf-style format string
 * @param ... Format arguments
 */
void shell_error(const char *format, ...);

/**
 * Print warning message to shell
 * @param format Printf-style format string
 * @param ... Format arguments
 */
void shell_warning(const char *format, ...);

/**
 * Print information message to shell
 * @param format Printf-style format string
 * @param ... Format arguments
 */
void shell_info(const char *format, ...);

/**
 * Print success message to shell
 * @param format Printf-style format string
 * @param ... Format arguments
 */
void shell_success(const char *format, ...);

/**
 * Print debug message to shell (if debug mode enabled)
 * @param format Printf-style format string
 * @param ... Format arguments
 */
void shell_debug(const char *format, ...);

// ====================================
// BUILT-IN COMMAND DECLARATIONS
// ====================================

// System Information Commands
int cmd_help(int argc, char **argv);
int cmd_version(int argc, char **argv);
int cmd_sysinfo(int argc, char **argv);
int cmd_uptime(int argc, char **argv);
int cmd_date(int argc, char **argv);

// Process Management Commands
int cmd_ps(int argc, char **argv);
int cmd_kill(int argc, char **argv);
int cmd_killall(int argc, char **argv);
int cmd_jobs(int argc, char **argv);
int cmd_top(int argc, char **argv);

// Memory Management Commands
int cmd_meminfo(int argc, char **argv);
int cmd_memtest(int argc, char **argv);
int cmd_memmap(int argc, char **argv);
int cmd_memdump(int argc, char **argv);
int cmd_malloc_stats(int argc, char **argv);

// File System Commands
int cmd_ls(int argc, char **argv);
int cmd_cd(int argc, char **argv);
int cmd_pwd(int argc, char **argv);
int cmd_mkdir(int argc, char **argv);
int cmd_rmdir(int argc, char **argv);
int cmd_rm(int argc, char **argv);
int cmd_cp(int argc, char **argv);
int cmd_mv(int argc, char **argv);
int cmd_cat(int argc, char **argv);
int cmd_head(int argc, char **argv);
int cmd_tail(int argc, char **argv);
int cmd_grep(int argc, char **argv);
int cmd_find(int argc, char **argv);
int cmd_df(int argc, char **argv);
int cmd_du(int argc, char **argv);

// Network Commands
int cmd_ping(int argc, char **argv);
int cmd_netstat(int argc, char **argv);
int cmd_ifconfig(int argc, char **argv);
int cmd_route(int argc, char **argv);

// System Control Commands
int cmd_reboot(int argc, char **argv);
int cmd_shutdown(int argc, char **argv);
int cmd_mount(int argc, char **argv);
int cmd_umount(int argc, char **argv);
int cmd_lsmod(int argc, char **argv);
int cmd_insmod(int argc, char **argv);
int cmd_rmmod(int argc, char **argv);

// Debug and Development Commands
int cmd_trace(int argc, char **argv);
int cmd_profile(int argc, char **argv);
int cmd_benchmark(int argc, char **argv);
int cmd_stress(int argc, char **argv);
int cmd_hexdump(int argc, char **argv);
int cmd_disasm(int argc, char **argv);
int cmd_gdb(int argc, char **argv);

// Shell Control Commands
int cmd_history(int argc, char **argv);
int cmd_alias(int argc, char **argv);
int cmd_unalias(int argc, char **argv);
int cmd_exit(int argc, char **argv);
int cmd_logout(int argc, char **argv);
int cmd_clear(int argc, char **argv);
int cmd_echo(int argc, char **argv);
int cmd_sleep(int argc, char **argv);

// Advanced System Commands
int cmd_lscpu(int argc, char **argv);
int cmd_lspci(int argc, char **argv);
int cmd_lsusb(int argc, char **argv);
int cmd_dmesg(int argc, char **argv);
int cmd_log(int argc, char **argv);
int cmd_sysctl(int argc, char **argv);
int cmd_capabilities(int argc, char **argv);
int cmd_security(int argc, char **argv);

// Educational Commands
int cmd_tutorial(int argc, char **argv);
int cmd_demo(int argc, char **argv);
int cmd_explain(int argc, char **argv);
int cmd_quiz(int argc, char **argv);

// ====================================
// SHELL UTILITIES
// ====================================

/**
 * Check if current user has administrative privileges
 * @return 1 if admin, 0 if not
 */
int shell_is_admin(void);

/**
 * Get current shell context
 * @return Current shell context or NULL
 */
struct shell_context *shell_get_context(void);

/**
 * Set shell context
 * @param context New shell context
 */
void shell_set_context(struct shell_context *context);

/**
 * Get shell state
 * @return Current shell state
 */
struct shell_state *shell_get_state(void);

/**
 * Parse integer argument with bounds checking
 * @param str String to parse
 * @param min Minimum allowed value
 * @param max Maximum allowed value
 * @param result Output parsed value
 * @return 0 on success, negative error code on failure
 */
int shell_parse_int(const char *str, int64_t min, int64_t max, int64_t *result);

/**
 * Parse size string (e.g., "1KB", "2MB", "1GB")
 * @param str String to parse
 * @param result Output size in bytes
 * @return 0 on success, negative error code on failure
 */
int shell_parse_size(const char *str, uint64_t *result);

/**
 * Format size for human-readable display
 * @param size Size in bytes
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @return Formatted string
 */
const char *shell_format_size(uint64_t size, char *buffer, size_t buffer_size);

/**
 * Format time duration for display
 * @param microseconds Duration in microseconds
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @return Formatted string
 */
const char *shell_format_time(uint64_t microseconds, char *buffer, size_t buffer_size);

// ====================================
// ERROR CODES
// ====================================

#define SHELL_ERROR_INVALID_COMMAND -1
#define SHELL_ERROR_INVALID_ARGS -2
#define SHELL_ERROR_PERMISSION_DENIED -3
#define SHELL_ERROR_NOT_FOUND -4
#define SHELL_ERROR_OUT_OF_MEMORY -5
#define SHELL_ERROR_IO_ERROR -6
#define SHELL_ERROR_INTERRUPTED -7
#define SHELL_ERROR_TIMEOUT -8
#define SHELL_ERROR_BUFFER_OVERFLOW -9
#define SHELL_ERROR_INVALID_FORMAT -10

#endif // ORION_SHELL_H
