# ORION OS - s390x Architecture Support (IBM Mainframe)

## Overview

s390x (System/390 Extended) is IBM's 64-bit mainframe architecture designed for enterprise computing, transaction processing, and high-availability systems. ORION OS provides comprehensive support for IBM z/Architecture mainframes with enterprise-grade reliability and performance.

## Supported IBM Mainframe Systems

### **IBM z15 (2019)**
- **Process**: 14nm FinFET
- **CPUs**: Up to 190 configurable processors
- **Cores**: 12-20 cores per processor
- **Frequency**: 4.5-5.2 GHz
- **Memory**: Up to 40TB RAM
- **I/O**: FICON Express16S, 10GbE, InfiniBand
- **Power**: 3.5-4.5 kW per drawer
- **Reliability**: 99.99999% uptime (5 minutes downtime per year)

### **IBM z16 (2022)**
- **Process**: 7nm FinFET
- **CPUs**: Up to 200 configurable processors
- **Cores**: 15-25 cores per processor
- **Frequency**: 5.0-5.5 GHz
- **Memory**: Up to 50TB RAM
- **I/O**: FICON Express32S, 25GbE, InfiniBand EDR
- **Power**: 3.0-4.0 kW per drawer
- **AI Acceleration**: Integrated AI inference acceleration

### **IBM z14 (2017)**
- **Process**: 14nm FinFET
- **CPUs**: Up to 170 configurable processors
- **Cores**: 10-18 cores per processor
- **Frequency**: 4.0-5.0 GHz
- **Memory**: Up to 32TB RAM
- **I/O**: FICON Express16S, 10GbE
- **Power**: 4.0-5.0 kW per drawer

## Architecture-Specific Features

### **z/Architecture Extensions**
```c
// z/Architecture feature detection
struct s390x_arch_features {
    bool has_z196;                    // z196 architecture
    bool has_zEC12;                   // zEC12 architecture
    bool has_z13;                     // z13 architecture
    bool has_z14;                     // z14 architecture
    bool has_z15;                     // z15 architecture
    bool has_z16;                     // z16 architecture
    bool has_vector_facility;         // Vector facility
    bool has_crypto_facility;         // Crypto facility
    bool has_decimal_facility;        // Decimal floating point
    bool has_transactional_execution; // Transactional execution
};

// Architecture feature management
int s390x_arch_detect_features(struct s390x_arch_features *features);
bool s390x_arch_has_extension(uint32_t extension_id);
```

### **Vector Facility (SIMD)**
```c
// z/Architecture vector facility
struct s390x_vector_facility {
    uint32_t vector_length;           // Vector length in bytes
    uint32_t vector_registers;        // Number of vector registers
    bool vector_decimal;              // Decimal vector operations
    bool vector_string;               // String vector operations
    bool vector_crypto;               // Crypto vector operations
};

// Vector operations
int s390x_vector_init(void);
void s390x_vector_add(uint64_t *dest, const uint64_t *src1, const uint64_t *src2, size_t len);
void s390x_vector_multiply(uint64_t *dest, const uint64_t *src1, const uint64_t *src2, size_t len);
void s390x_vector_crypto_encrypt(void *dest, const void *src, const void *key, size_t len);
```

### **Crypto Facility**
```c
// z/Architecture crypto facility
struct s390x_crypto_facility {
    bool aes_support;                 // AES encryption support
    bool sha_support;                 // SHA hash support
    bool rsa_support;                 // RSA encryption support
    bool ecc_support;                 // Elliptic curve support
    bool quantum_safe;                // Quantum-safe algorithms
    uint32_t crypto_engines;          // Number of crypto engines
};

// Crypto operations
int s390x_crypto_init(void);
int s390x_aes_encrypt(const void *key, const void *input, void *output, size_t len);
int s390x_sha_hash(const void *input, void *output, size_t len, uint32_t algorithm);
int s390x_rsa_encrypt(const void *key, const void *input, void *output, size_t len);
```

## Memory Management

### **z/Architecture Memory Model**
```c
// s390x memory management
struct s390x_memory_config {
    uint64_t real_memory_size;        // Real memory size
    uint64_t virtual_memory_size;     // Virtual memory size
    uint32_t page_sizes[8];           // Supported page sizes
    bool storage_key_protection;      // Storage key protection
    bool access_register_mode;        // Access register mode
    uint32_t max_asid;                // Maximum ASID
};

// Memory management functions
int s390x_memory_init(void);
int s390x_memory_map(vaddr_t va, paddr_t pa, uint64_t flags);
int s390x_memory_unmap(vaddr_t va, size_t len);
int s390x_memory_set_storage_key(paddr_t pa, uint8_t key);
```

### **Storage Keys and Protection**
```c
// s390x storage key protection
struct s390x_storage_key {
    uint8_t access_key;               // Access key (0-15)
    uint8_t fetch_protection;         // Fetch protection
    uint8_t reference_bit;            // Reference bit
    uint8_t change_bit;               // Change bit
    bool storage_alteration;          // Storage alteration allowed
};

// Storage key management
int s390x_set_storage_key(paddr_t pa, struct s390x_storage_key *key);
int s390x_get_storage_key(paddr_t pa, struct s390x_storage_key *key);
int s390x_clear_storage_key(paddr_t pa);
```

## I/O and Channel Subsystem

### **Channel Subsystem (CSS)**
```c
// s390x channel subsystem
struct s390x_channel_subsystem {
    uint32_t max_channels;            // Maximum number of channels
    uint32_t max_subchannels;         // Maximum number of subchannels
    bool multiple_subchannel_sets;    // Multiple subchannel sets
    uint32_t channel_paths;           // Number of channel paths
    bool dynamic_channel_reconfiguration; // Dynamic channel reconfiguration
};

// Channel management
int s390x_css_init(void);
int s390x_channel_configure(uint32_t channel_id, uint32_t config);
int s390x_subchannel_create(uint32_t subchannel_id, uint32_t device_type);
int s390x_channel_path_configure(uint32_t path_id, uint32_t config);
```

### **FICON and Network I/O**
```c
// s390x FICON configuration
struct s390x_ficon_config {
    uint32_t speed_gbps;              // FICON speed in Gbps
    uint32_t max_frame_size;          // Maximum frame size
    bool encryption;                   // Link encryption
    bool compression;                  // Data compression
    uint32_t buffer_credits;          // Buffer credits
};

// FICON management
int s390x_ficon_init(uint32_t channel_id);
int s390x_ficon_set_speed(uint32_t channel_id, uint32_t speed);
int s390x_ficon_enable_encryption(uint32_t channel_id);
int s390x_ficon_enable_compression(uint32_t channel_id);
```

## Transaction Processing

### **Transactional Execution (TX)**
```c
// s390x transactional execution
struct s390x_transactional_exec {
    bool tx_supported;                // Transactional execution support
    uint32_t tx_nesting_depth;        // Maximum nesting depth
    bool tx_restricted;               // Restricted transactional execution
    bool tx_constrained;              // Constrained transactional execution
    uint32_t tx_abort_codes;          // Transaction abort codes
};

// Transactional execution functions
int s390x_tx_begin(void);
int s390x_tx_end(void);
int s390x_tx_abort(uint32_t reason);
bool s390x_tx_in_transaction(void);
int s390x_tx_nest_level(void);
```

### **High Availability Features**
```c
// s390x high availability
struct s390x_high_availability {
    bool parallel_sysplex;            // Parallel sysplex support
    bool coupling_facility;           // Coupling facility
    bool cross_system_extension;      // Cross-system extension
    bool dynamic_partitioning;        // Dynamic partitioning
    uint32_t sysplex_id;              // Sysplex identifier
};

// High availability functions
int s390x_ha_init(void);
int s390x_sysplex_join(uint32_t sysplex_id);
int s390x_coupling_connect(uint32_t facility_id);
int s390x_partition_create(uint32_t partition_id);
```

## Performance Monitoring

### **CPU Measurement Facility (CPUMF)**
```c
// s390x CPU measurement facility
struct s390x_cpumf_config {
    uint32_t counter_sets;            // Number of counter sets
    uint32_t counters_per_set;        // Counters per set
    bool sampling_support;            // Sampling support
    bool diagnostic_sampling;         // Diagnostic sampling
    uint32_t max_sampling_rate;       // Maximum sampling rate
};

// CPUMF functions
int s390x_cpumf_init(void);
int s390x_cpumf_set_event(uint32_t counter_set, uint32_t counter, uint32_t event);
uint64_t s390x_cpumf_read_counter(uint32_t counter_set, uint32_t counter);
int s390x_cpumf_start_sampling(uint32_t rate);
int s390x_cpumf_stop_sampling(void);
```

### **System Activity Measurement**
```c
// s390x system activity measurement
struct s390x_sam_config {
    bool cpu_utilization;             // CPU utilization measurement
    bool memory_utilization;          // Memory utilization measurement
    bool i_o_utilization;             // I/O utilization measurement
    bool network_utilization;         // Network utilization measurement
    uint32_t measurement_interval;    // Measurement interval in ms
};

// System activity measurement
int s390x_sam_init(void);
int s390x_sam_start_measurement(struct s390x_sam_config *config);
int s390x_sam_get_measurements(struct s390x_sam_data *data);
int s390x_sam_stop_measurement(void);
```

## Enterprise Features

### **LPAR (Logical Partitioning)**
```c
// s390x logical partitioning
struct s390x_lpar_config {
    uint32_t lpar_id;                 // LPAR identifier
    uint32_t cpu_count;               // Number of CPUs
    uint64_t memory_size;             // Memory size
    uint32_t channel_paths;           // Channel paths
    bool dedicated_cpus;               // Dedicated CPUs
    bool shared_cpus;                  // Shared CPUs
};

// LPAR management
int s390x_lpar_create(struct s390x_lpar_config *config);
int s390x_lpar_destroy(uint32_t lpar_id);
int s390x_lpar_start(uint32_t lpar_id);
int s390x_lpar_stop(uint32_t lpar_id);
int s390x_lpar_reconfigure(uint32_t lpar_id, struct s390x_lpar_config *config);
```

### **z/VM and Virtualization**
```c
// s390x virtualization support
struct s390x_virtualization {
    bool z_vm_support;                // z/VM support
    bool kvm_support;                 // KVM support
    uint32_t max_guests;              // Maximum number of guests
    bool live_migration;              // Live migration support
    bool memory_overcommit;           // Memory overcommit support
};

// Virtualization functions
int s390x_virtualization_init(void);
int s390x_guest_create(uint32_t guest_id, struct s390x_guest_config *config);
int s390x_guest_destroy(uint32_t guest_id);
int s390x_guest_start(uint32_t guest_id);
int s390x_guest_stop(uint32_t guest_id);
```

## Security Features

### **Secure Service Container (SSC)**
```c
// s390x secure service container
struct s390x_ssc_config {
    bool ssc_supported;               // SSC support available
    uint32_t ssc_version;             // SSC version
    uint64_t attestation_key;         // Attestation key
    bool secure_boot;                 // Secure boot support
    bool measured_boot;               // Measured boot support
};

// SSC functions
int s390x_ssc_init(void);
int s390x_ssc_create_container(size_t size, void **container);
int s390x_ssc_destroy_container(void *container);
int s390x_ssc_enter_container(void *container);
int s390x_ssc_exit_container(void);
```

### **Hardware Security Module (HSM)**
```c
// s390x hardware security module
struct s390x_hsm_config {
    bool hsm_supported;               // HSM support available
    uint32_t hsm_type;                // HSM type
    uint32_t key_slots;               // Number of key slots
    bool key_management;              // Key management support
    bool crypto_acceleration;         // Crypto acceleration
};

// HSM functions
int s390x_hsm_init(void);
int s390x_hsm_generate_key(uint32_t slot_id, uint32_t key_type, uint32_t key_size);
int s390x_hsm_import_key(uint32_t slot_id, const void *key_data, size_t key_size);
int s390x_hsm_export_key(uint32_t slot_id, void *key_data, size_t *key_size);
int s390x_hsm_delete_key(uint32_t slot_id);
```

## Performance Targets

### **IBM z15 Performance Targets**
- **Syscall latency**: < 200ns (median), < 500ns (99th percentile)
- **Context switch**: < 400ns same-core, < 1200ns cross-core
- **IPC throughput**: > 18M msg/s, latency < 180ns
- **Memory bandwidth**: ≥ 98% of STREAM benchmark native
- **Transaction processing**: > 1M TPS (transactions per second)

### **IBM z16 Performance Targets**
- **Syscall latency**: < 150ns (median), < 400ns (99th percentile)
- **Context switch**: < 300ns same-core, < 1000ns cross-core
- **IPC throughput**: > 25M msg/s, latency < 120ns
- **Memory bandwidth**: ≥ 99% of STREAM benchmark native
- **Transaction processing**: > 1.5M TPS (transactions per second)

## Development and Testing

### **QEMU Emulation**
```bash
# Run ORION OS on s390x emulation
qemu-system-s390x \
    -M s390-ccw-virtio \
    -cpu max \
    -m 8G \
    -kernel orion-kernel-s390x.img \
    -append "console=hvc0 root=/dev/vda2" \
    -serial stdio \
    -display gtk
```

### **Cross-Compilation**
```bash
# Cross-compile for s390x
make ARCH=s390x BOARD=z15 \
    CROSS_COMPILE=s390x-linux-gnu- \
    CONFIG_S390X_OPTIMIZATIONS=y \
    CONFIG_S390X_ENTERPRISE=y \
    CONFIG_S390X_TRANSACTIONAL_EXECUTION=y
```

### **Hardware Testing**
```bash
# Test s390x-specific features
orion-test --board=z15 --test=transactional-execution
orion-test --board=z15 --test=storage-keys
orion-test --board=z15 --test=channel-subsystem
orion-test --board=z15 --test=crypto-facility
```

## Configuration

### **Device Tree**
```dts
// s390x device tree configuration
/dts-v1/;

/ {
    compatible = "ibm,s390x";
    model = "IBM z15";
    
    cpus {
        #address-cells = <1>;
        #size-cells = <0>;
        
        PowerPC,z15@0 {
            device_type = "cpu";
            reg = <0>;
            compatible = "ibm,z15";
            ibm,chip-id = <0>;
            ibm,partition-id = <0>;
        };
    };
    
    memory@0 {
        device_type = "memory";
        reg = <0x0 0x0 0x0 0x80000000>;
        ibm,chip-id = <0>;
    };
    
    channel-subsystem {
        compatible = "ibm,channel-subsystem";
        #address-cells = <1>;
        #size-cells = <0>;
        
        channel@0 {
            compatible = "ibm,ficon-channel";
            reg = <0>;
            ibm,channel-type = "ficon";
        };
    };
};
```

### **Kernel Configuration**
```bash
# s390x-specific kernel configuration
CONFIG_S390=y
CONFIG_S390X=y
CONFIG_Z15=y
CONFIG_S390X_TRANSACTIONAL_EXECUTION=y
CONFIG_S390X_STORAGE_KEYS=y
CONFIG_S390X_CHANNEL_SUBSYSTEM=y
CONFIG_S390X_CRYPTO_FACILITY=y
CONFIG_S390X_VECTOR_FACILITY=y
CONFIG_S390X_CPU_MEASUREMENT_FACILITY=y
CONFIG_S390X_SECURE_SERVICE_CONTAINER=y
```

## Enterprise Integration

### **IBM z/OS Compatibility**
```c
// z/OS compatibility layer
struct zos_compatibility {
    bool zos_syscalls;                // z/OS system call emulation
    bool zos_libraries;               // z/OS library compatibility
    bool zos_filesystem;              // z/OS filesystem support
    bool zos_jcl;                     // JCL job control language
    uint32_t zos_version;             // z/OS version emulated
};

// z/OS compatibility functions
int zos_compatibility_init(void);
int zos_syscall_emulate(uint32_t syscall, void *args);
int zos_library_load(const char *library_name);
int zos_jcl_parse(const char *jcl_script);
```

### **IBM CICS and IMS Support**
```c
// IBM transaction processing support
struct ibm_transaction_processing {
    bool cics_support;                // CICS transaction support
    bool ims_support;                 // IMS transaction support
    bool db2_support;                 // DB2 database support
    bool mq_support;                  // MQ messaging support
    uint32_t max_transactions;        // Maximum concurrent transactions
};

// Transaction processing functions
int ibm_cics_init(void);
int ibm_cics_start_transaction(const char *transaction_id);
int ibm_cics_commit_transaction(uint32_t transaction_handle);
int ibm_cics_rollback_transaction(uint32_t transaction_handle);
```

## Troubleshooting

### **Common Issues**
1. **Channel subsystem**: Use `orion-monitor --channels` to check channel status
2. **Storage keys**: Verify with `orion-memory --storage-keys`
3. **Transactional execution**: Check with `orion-cpu --tx-support`
4. **Crypto facility**: Test with `orion-crypto --test`

### **Performance Tuning**
```bash
# Optimize for specific s390x model
orion-optimize --board=z16 --profile=enterprise
orion-optimize --board=z15 --profile=high-performance
orion-optimize --board=z14 --profile=legacy
```

---

**Note**: This s390x support is specifically designed for IBM mainframe systems, providing enterprise-grade reliability, performance, and compatibility with z/OS applications.
