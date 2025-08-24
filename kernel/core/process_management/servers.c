/*
 * Orion Operating System - System Servers
 *
 * Complete system server initialization and management.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/kernel.h>
#include <orion/types.h>
#include <orion/mm.h>
#include <orion/scheduler.h>
#include <orion/ipc.h>

// ========================================
// CONSTANTS AND DEFINITIONS
// ========================================

#define MAX_SERVERS 32
#define SERVER_STACK_SIZE (128 * 1024) // 128KB server stack

// Server states
#define SERVER_STATE_INACTIVE 0
#define SERVER_STATE_STARTING 1
#define SERVER_STATE_RUNNING 2
#define SERVER_STATE_STOPPING 3
#define SERVER_STATE_ERROR 4

// Server types
#define SERVER_TYPE_FS 1
#define SERVER_TYPE_NETWORK 2
#define SERVER_TYPE_DEVICE 3
#define SERVER_TYPE_IPC 4

// ========================================
// DATA STRUCTURES
// ========================================

typedef struct system_server
{
    uint32_t id;
    uint32_t type;
    uint32_t state;
    char name[32];
    process_t *process;
    thread_t *main_thread;
    uint64_t start_time;
    uint64_t uptime;
    uint64_t message_count;
    uint64_t error_count;
} system_server_t;

// ========================================
// GLOBAL VARIABLES
// ========================================

static system_server_t g_servers[MAX_SERVERS];
static uint32_t g_server_count = 0;
static spinlock_t g_server_lock = SPINLOCK_INIT;

// ========================================
// SERVER MANAGEMENT FUNCTIONS
// ========================================

/**
 * Start file system server
 *
 * @return 0 on success, negative error code on failure
 */
int start_fs_server(void)
{
    kinfo("Starting file system server...");

    // Check if server already exists
    for (uint32_t i = 0; i < g_server_count; i++)
    {
        if (g_servers[i].type == SERVER_TYPE_FS && g_servers[i].state == SERVER_STATE_RUNNING)
        {
            kinfo("File system server already running");
            return OR_OK;
        }
    }

    // Create server process
    process_t *fs_process = process_create("fs_server", 0, 0);
    if (!fs_process)
    {
        kerror("Failed to create file system server process");
        return -1;
    }

    // Set server properties
    fs_process->state = PROCESS_STATE_READY;

    // Create main server thread
    thread_t *fs_thread = thread_create(fs_process, "fs_main", fs_server_main, NULL);
    if (!fs_thread)
    {
        kerror("Failed to create file system server thread");
        process_destroy(fs_process);
        return -1;
    }

    // Start the server
    if (process_start(fs_process) != OR_OK)
    {
        kerror("Failed to start file system server");
        thread_destroy(fs_thread);
        process_destroy(fs_process);
        return -1;
    }

    // Register server
    spinlock_lock(&g_server_lock);

    if (g_server_count < MAX_SERVERS)
    {
        system_server_t *server = &g_servers[g_server_count];
        server->id = g_server_count;
        server->type = SERVER_TYPE_FS;
        server->state = SERVER_STATE_RUNNING;
        strcpy(server->name, "fs_server");
        server->process = fs_process;
        server->main_thread = fs_thread;
        server->start_time = arch_get_timestamp();
        server->uptime = 0;
        server->message_count = 0;
        server->error_count = 0;

        g_server_count++;

        spinlock_unlock(&g_server_lock);

        kinfo("File system server started successfully (PID: %llu)", (unsigned long long)fs_process->pid);
        return OR_OK;
    }

    spinlock_unlock(&g_server_lock);

    kerror("No free server slots available");
    return -1;
}

/**
 * Start network server
 *
 * @return 0 on success, negative error code on failure
 */
int start_network_server(void)
{
    kinfo("Starting network server...");

    // Check if server already exists
    for (uint32_t i = 0; i < g_server_count; i++)
    {
        if (g_servers[i].type == SERVER_TYPE_NETWORK && g_servers[i].state == SERVER_STATE_RUNNING)
        {
            kinfo("Network server already running");
            return OR_OK;
        }
    }

    // Create server process
    process_t *net_process = process_create("net_server", 0, 0);
    if (!net_process)
    {
        kerror("Failed to create network server process");
        return -1;
    }

    // Set server properties
    net_process->state = PROCESS_STATE_READY;

    // Create main server thread
    thread_t *net_thread = thread_create(net_process, "net_main", net_server_main, NULL);
    if (!net_thread)
    {
        kerror("Failed to create network server thread");
        process_destroy(net_process);
        return -1;
    }

    // Start the server
    if (process_start(net_process) != OR_OK)
    {
        kerror("Failed to start network server");
        thread_destroy(net_thread);
        process_destroy(net_process);
        return -1;
    }

    // Register server
    spinlock_lock(&g_server_lock);

    if (g_server_count < MAX_SERVERS)
    {
        system_server_t *server = &g_servers[g_server_count];
        server->id = g_server_count;
        server->type = SERVER_TYPE_NETWORK;
        server->state = SERVER_STATE_RUNNING;
        strcpy(server->name, "net_server");
        server->process = net_process;
        server->main_thread = net_thread;
        server->start_time = arch_get_timestamp();
        server->uptime = 0;
        server->message_count = 0;
        server->error_count = 0;

        g_server_count++;

        spinlock_unlock(&g_server_lock);

        kinfo("Network server started successfully (PID: %llu)", (unsigned long long)net_process->pid);
        return OR_OK;
    }

    spinlock_unlock(&g_server_lock);

    kerror("No free server slots available");
    return -1;
}

/**
 * Start device manager
 *
 * @return 0 on success, negative error code on failure
 */
int start_device_manager(void)
{
    kinfo("Starting device manager...");

    // Check if server already exists
    for (uint32_t i = 0; i < g_server_count; i++)
    {
        if (g_servers[i].type == SERVER_TYPE_DEVICE && g_servers[i].state == SERVER_STATE_RUNNING)
        {
            kinfo("Device manager already running");
            return OR_OK;
        }
    }

    // Create server process
    process_t *dev_process = process_create("dev_manager", 0, 0);
    if (!dev_process)
    {
        kerror("Failed to create device manager process");
        return -1;
    }

    // Set server properties
    dev_process->state = PROCESS_STATE_READY;

    // Create main server thread
    thread_t *dev_thread = thread_create(dev_process, "dev_main", device_manager_main, NULL);
    if (!dev_thread)
    {
        kerror("Failed to create device manager thread");
        process_destroy(dev_process);
        return -1;
    }

    // Start the server
    if (process_start(dev_process) != OR_OK)
    {
        kerror("Failed to start device manager");
        thread_destroy(dev_thread);
        process_destroy(dev_process);
        return -1;
    }

    // Register server
    spinlock_lock(&g_server_lock);

    if (g_server_count < MAX_SERVERS)
    {
        system_server_t *server = &g_servers[g_server_count];
        server->id = g_server_count;
        server->type = SERVER_TYPE_DEVICE;
        server->state = SERVER_STATE_RUNNING;
        strcpy(server->name, "dev_manager");
        server->process = dev_process;
        server->main_thread = dev_thread;
        server->start_time = arch_get_timestamp();
        server->uptime = 0;
        server->message_count = 0;
        server->error_count = 0;

        g_server_count++;

        spinlock_unlock(&g_server_lock);

        kinfo("Device manager started successfully (PID: %llu)", (unsigned long long)dev_process->pid);
        return OR_OK;
    }

    spinlock_unlock(&g_server_lock);

    kerror("No free server slots available");
    return -1;
}

/**
 * Start IPC server
 *
 * @return 0 on success, negative error code on failure
 */
int start_ipc_server(void)
{
    kinfo("Starting IPC server...");

    // Check if server already exists
    for (uint32_t i = 0; i < g_server_count; i++)
    {
        if (g_servers[i].type == SERVER_TYPE_IPC && g_servers[i].state == SERVER_STATE_RUNNING)
        {
            kinfo("IPC server already running");
            return OR_OK;
        }
    }

    // Create server process
    process_t *ipc_process = process_create("ipc_server", 0, 0);
    if (!ipc_process)
    {
        kerror("Failed to create IPC server process");
        return -1;
    }

    // Set server properties
    ipc_process->state = PROCESS_STATE_READY;

    // Create main server thread
    thread_t *ipc_thread = thread_create(ipc_process, "ipc_main", ipc_server_main, NULL);
    if (!ipc_thread)
    {
        kerror("Failed to create IPC server thread");
        process_destroy(ipc_process);
        return -1;
    }

    // Start the server
    if (process_start(ipc_process) != OR_OK)
    {
        kerror("Failed to start IPC server");
        thread_destroy(ipc_thread);
        process_destroy(ipc_process);
        return -1;
    }

    // Register server
    spinlock_lock(&g_server_lock);

    if (g_server_count < MAX_SERVERS)
    {
        system_server_t *server = &g_servers[g_server_count];
        server->id = g_server_count;
        server->type = SERVER_TYPE_IPC;
        server->state = SERVER_STATE_RUNNING;
        strcpy(server->name, "ipc_server");
        server->process = ipc_process;
        server->main_thread = ipc_thread;
        server->start_time = arch_get_timestamp();
        server->uptime = 0;
        server->message_count = 0;
        server->error_count = 0;

        g_server_count++;

        spinlock_unlock(&g_server_lock);

        kinfo("IPC server started successfully (PID: %llu)", (unsigned long long)ipc_process->pid);
        return OR_OK;
    }

    spinlock_unlock(&g_server_lock);

    kerror("No free server slots available");
    return -1;
}

// ========================================
// SERVER MAIN FUNCTIONS (STUBS FOR NOW)
// ========================================

/**
 * File system server main function
 *
 * @param arg Server argument (unused)
 */
void fs_server_main(void *arg)
{
    (void)arg;

    kinfo("File system server main loop started");

    // TODO: Implement actual file system server logic
    // This would include:
    // - Mounting file systems
    // - Handling file operations
    // - Managing file descriptors
    // - IPC communication with clients

    while (1)
    {
        // Process file system requests
        // Handle IPC messages
        // Manage file system state

        // Sleep for a bit to avoid busy waiting
        arch_delay_ms(10);
    }
}

/**
 * Network server main function
 *
 * @param arg Server argument (unused)
 */
void net_server_main(void *arg)
{
    (void)arg;

    kinfo("Network server main loop started");

    // TODO: Implement actual network server logic
    // This would include:
    // - Network interface management
    // - TCP/UDP protocol handling
    // - Socket management
    // - IPC communication with clients

    while (1)
    {
        // Process network requests
        // Handle IPC messages
        // Manage network state

        // Sleep for a bit to avoid busy waiting
        arch_delay_ms(10);
    }
}

/**
 * Device manager main function
 *
 * @param arg Server argument (unused)
 */
void device_manager_main(void *arg)
{
    (void)arg;

    kinfo("Device manager main loop started");

    // TODO: Implement actual device manager logic
    // This would include:
    // - Device enumeration
    // - Driver management
    // - Device hotplug handling
    // - IPC communication with clients

    while (1)
    {
        // Process device requests
        // Handle IPC messages
        // Manage device state

        // Sleep for a bit to avoid busy waiting
        arch_delay_ms(10);
    }
}

/**
 * IPC server main function
 *
 * @param arg Server argument (unused)
 */
void ipc_server_main(void *arg)
{
    (void)arg;

    kinfo("IPC server main loop started");

    // TODO: Implement actual IPC server logic
    // This would include:
    // - Port management
    // - Message routing
    // - Capability management
    // - IPC protocol handling

    while (1)
    {
        // Process IPC requests
        // Handle port operations
        // Manage message queues

        // Sleep for a bit to avoid busy waiting
        arch_delay_ms(10);
    }
}

// ========================================
// SERVER UTILITY FUNCTIONS
// ========================================

/**
 * Get server count
 *
 * @return Number of active servers
 */
uint32_t server_get_count(void)
{
    return g_server_count;
}

/**
 * Get server by type
 *
 * @param type Server type to find
 * @return Pointer to server or NULL if not found
 */
system_server_t *server_get_by_type(uint32_t type)
{
    for (uint32_t i = 0; i < g_server_count; i++)
    {
        if (g_servers[i].type == type && g_servers[i].state == SERVER_STATE_RUNNING)
        {
            return &g_servers[i];
        }
    }
    return NULL;
}

/**
 * Stop a server
 *
 * @param type Server type to stop
 * @return 0 on success, negative error code on failure
 */
int server_stop(uint32_t type)
{
    system_server_t *server = server_get_by_type(type);
    if (!server)
    {
        return -OR_ENOENT;
    }

    kinfo("Stopping server: %s", server->name);

    // Mark server as stopping
    server->state = SERVER_STATE_STOPPING;

    // Stop the process
    if (server->process)
    {
        // TODO: Implement proper process termination
        // For now, just mark as terminated
        server->process->state = PROCESS_STATE_TERMINATED;
    }

    // Update server state
    server->state = SERVER_STATE_INACTIVE;

    kinfo("Server %s stopped", server->name);

    return OR_OK;
}

/**
 * Initialize server management system
 */
void server_init(void)
{
    kinfo("Initializing server management system");

    // Initialize server table
    memset(g_servers, 0, sizeof(g_servers));
    g_server_count = 0;

    // Initialize locks
    spinlock_init(&g_server_lock);

    kinfo("Server management system initialized (max servers: %d)", MAX_SERVERS);
}
