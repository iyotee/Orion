# Orion OS API Documentation

## System Calls

Orion OS provides 60 stable system calls organized into logical groups:

### Process Management
- `sys_exit(int code)` - Terminate current process
- `sys_proc_create(path, argv, envp)` - Create new process
- `sys_wait(pid, status)` - Wait for process completion
- `sys_getpid()` - Get current process ID
- `sys_gettid()` - Get current thread ID

### Memory Management
- `sys_vm_map(addr, size, prot, flags)` - Map virtual memory
- `sys_vm_unmap(addr, size)` - Unmap virtual memory
- `sys_vm_protect(addr, size, prot)` - Change memory protection

### I/O Operations
- `sys_read(fd, buf, count)` - Read from file descriptor
- `sys_write(fd, buf, count)` - Write to file descriptor
- `sys_open(path, flags, mode)` - Open file
- `sys_close(fd)` - Close file descriptor

### IPC (Inter-Process Communication)
- `sys_port_create()` - Create IPC port
- `sys_msg_send(port, msg, size)` - Send message
- `sys_msg_recv(port, buf, size)` - Receive message

### Time and Timing
- `sys_clock_get(type, time)` - Get system time
- `sys_nanosleep(req, rem)` - Sleep for specified time

## Kernel APIs

### Memory Management APIs
```c
// Physical Memory Manager
void* pmm_alloc_page(void);
void pmm_free_page(void* page);

// Virtual Memory Manager
int vmm_map_page(vm_space_t* space, uint64_t vaddr, uint64_t paddr, uint32_t flags);
int vmm_unmap_page(vm_space_t* space, uint64_t vaddr);

// Heap Allocator
void* kmalloc(size_t size);
void kfree(void* ptr);
void* krealloc(void* ptr, size_t size);
```

### Process Management APIs
```c
// Scheduler APIs
process_t* scheduler_create_process(const char* name);
thread_t* scheduler_create_thread(process_t* proc, void* entry);
void sched_yield(void);

// Process Management
int process_setup_args(process_t* proc, char** argv, char** envp);
int elf_load_process(process_t* proc, const char* path);
```

### Security APIs
```c
// Capability Management
cap_id_t cap_create(cap_rights_t rights, cap_type_t type);
bool cap_check_rights(cap_id_t cap, cap_rights_t required);
int cap_grant(cap_id_t source, uint64_t target_pid, cap_rights_t rights);
int cap_revoke(uint64_t target_pid, cap_rights_t rights);

// Security Functions
bool security_is_address_valid(uint64_t addr, size_t size);
void security_report_violation(const char* desc);
```

## Driver Framework APIs

### Rust Driver Traits
```rust
// Main driver trait
pub trait OrionDriver {
    fn probe(device: &DeviceInfo) -> DriverResult<bool>;
    fn init(device: DeviceInfo) -> DriverResult<Self>;
    fn handle_irq(&mut self) -> DriverResult<()>;
    fn shutdown(&mut self) -> DriverResult<()>;
}

// Block device trait
pub trait BlockDriver: OrionDriver {
    fn read_blocks(&mut self, lba: u64, blocks: u32, buffer: &mut [u8]) -> DriverResult<()>;
    fn write_blocks(&mut self, lba: u64, blocks: u32, buffer: &[u8]) -> DriverResult<()>;
    fn flush(&mut self) -> DriverResult<()>;
}

// Network device trait
pub trait NetworkDriver: OrionDriver {
    fn send_packet(&mut self, packet: &[u8]) -> DriverResult<()>;
    fn receive_packet(&mut self, buffer: &mut [u8]) -> DriverResult<usize>;
    fn link_status(&self) -> LinkStatus;
}
```

### MMIO Operations
```rust
// Safe memory-mapped I/O
pub struct MmioAccessor {
    base: usize,
    size: usize,
}

impl MmioAccessor {
    pub fn read_u32(&self, offset: usize) -> u32;
    pub fn write_u32(&self, offset: usize, value: u32);
    pub fn read_u64(&self, offset: usize) -> u64;
    pub fn write_u64(&self, offset: usize, value: u64);
}
```

## Error Codes

### System Call Errors
- `ORION_OK` (0) - Success
- `ORION_EINVAL` (-1) - Invalid argument
- `ORION_ENOMEM` (-2) - Out of memory
- `ORION_EPERM` (-3) - Permission denied
- `ORION_ENOENT` (-4) - No such file or directory

### Driver Errors
```rust
pub enum DriverError {
    DeviceNotFound,
    InitializationFailed,
    ResourceBusy,
    Unsupported,
    IpcError,
    MemoryError,
    Timeout,
    General,
}
```

## Boot Protocol

Orion uses a custom boot protocol between the bootloader and kernel:

### Boot Information Structure
```c
typedef struct orion_boot_info {
    uint32_t magic;                    // ORION_BOOT_MAGIC
    uint32_t version;                  // Protocol version
    
    orion_memory_info_t memory;        // Memory map
    orion_module_info_t modules;       // Loaded modules
    orion_bootloader_info_t bootloader;// Bootloader info
    orion_efi_info_t efi;             // EFI system table
    
    uint32_t checksum;                 // Structure checksum
} orion_boot_info_t;
```

---
*Complete API reference with examples available in kernel headers*
