/*
 * Orion Operating System - Storage Framework Core
 *
 * Core storage management functionality including:
 * - Storage device management
 * - Storage pool management
 * - Storage policies and strategies
 * - Basic storage operations
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

use core::{
    fmt,
    sync::atomic::{AtomicU64, AtomicU32, Ordering},
};
use alloc::{
    string::String,
    vec::Vec,
    collections::BTreeMap,
    boxed::Box,
};
use spin::RwLock;

// ========================================
// CORE STORAGE TYPES
// ========================================

/// Storage device types
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub enum DeviceType {
    HDD,           // Hard Disk Drive
    SSD,           // Solid State Drive
    NVMe,          // NVMe SSD
    RAM,           // RAM Disk
    Network,       // Network Storage
    Optical,       // Optical Drive
    Tape,          // Tape Drive
    Virtual,       // Virtual Device
}

/// Storage device states
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum DeviceState {
    Online,         // Device is online and operational
    Offline,        // Device is offline
    Degraded,       // Device is degraded but operational
    Failed,         // Device has failed
    Maintenance,    // Device is under maintenance
    Initializing,   // Device is initializing
}

/// Storage device information
#[derive(Debug, Clone)]
pub struct StorageDevice {
    pub id: String,
    pub name: String,
    pub device_type: DeviceType,
    pub state: DeviceState,
    pub capacity: u64,
    pub block_size: u32,
    pub read_speed: u64,      // MB/s
    pub write_speed: u64,     // MB/s
    pub latency: u64,         // microseconds
    pub health: u8,           // 0-100 percentage
    pub mount_point: Option<String>,
    pub metadata: BTreeMap<String, String>,
}

impl StorageDevice {
    pub fn new(
        id: String,
        name: String,
        device_type: DeviceType,
        capacity: u64,
        block_size: u32,
    ) -> Self {
        Self {
            id,
            name,
            device_type,
            state: DeviceState::Initializing,
            capacity,
            block_size,
            read_speed: 0,
            write_speed: 0,
            latency: 0,
            health: 100,
            mount_point: None,
            metadata: BTreeMap::new(),
        }
    }

    pub fn is_operational(&self) -> bool {
        matches!(self.state, DeviceState::Online | DeviceState::Degraded)
    }

    pub fn get_available_capacity(&self) -> u64 {
        // In a real implementation, this would check actual available space
        self.capacity
    }

    pub fn update_health(&mut self, health: u8) {
        self.health = health.clamp(0, 100);
        
        // Update state based on health
        if self.health < 20 {
            self.state = DeviceState::Failed;
        } else if self.health < 50 {
            self.state = DeviceState::Degraded;
        } else if self.health < 80 {
            self.state = DeviceState::Degraded;
        } else {
            self.state = DeviceState::Online;
        }
    }
}

// ========================================
// STORAGE POOL MANAGEMENT
// ========================================

/// Storage pool types
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum PoolType {
    Simple,         // Single device pool
    Mirrored,       // RAID 1 mirroring
    Striped,        // RAID 0 striping
    Parity,         // RAID 5/6 parity
    Hybrid,         // Mixed storage types
    Tiered,         // Multi-tier storage
}

/// Storage pool information
#[derive(Debug, Clone)]
pub struct StoragePool {
    pub id: String,
    pub name: String,
    pub pool_type: PoolType,
    pub devices: Vec<String>,  // Device IDs
    pub total_capacity: u64,
    pub used_capacity: u64,
    pub available_capacity: u64,
    pub redundancy_factor: u8,
    pub performance_tier: u8,  // 1-5, higher is faster
    pub metadata: BTreeMap<String, String>,
}

impl StoragePool {
    pub fn new(
        id: String,
        name: String,
        pool_type: PoolType,
        devices: Vec<String>,
    ) -> Self {
        Self {
            id,
            name,
            pool_type,
            devices,
            total_capacity: 0,
            used_capacity: 0,
            available_capacity: 0,
            redundancy_factor: 1,
            performance_tier: 1,
            metadata: BTreeMap::new(),
        }
    }

    pub fn calculate_capacity(&mut self, devices: &BTreeMap<String, StorageDevice>) {
        let mut total = 0u64;
        
        for device_id in &self.devices {
            if let Some(device) = devices.get(device_id) {
                total += device.capacity;
            }
        }
        
        self.total_capacity = total;
        
        // Adjust for redundancy
        match self.pool_type {
            PoolType::Mirrored => {
                self.available_capacity = total / self.redundancy_factor as u64;
            }
            PoolType::Striped => {
                self.available_capacity = total;
            }
            PoolType::Parity => {
                self.available_capacity = total - (total / (self.redundancy_factor as u64 + 1));
            }
            _ => {
                self.available_capacity = total;
            }
        }
    }

    pub fn get_utilization_percentage(&self) -> f64 {
        if self.total_capacity == 0 {
            0.0
        } else {
            (self.used_capacity as f64 / self.total_capacity as f64) * 100.0
        }
    }
}

// ========================================
// STORAGE POLICIES
// ========================================

/// Storage allocation policies
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum AllocationPolicy {
    FirstFit,       // First available space
    BestFit,        // Best fitting space
    WorstFit,       // Worst fitting space
    RoundRobin,     // Round-robin allocation
    Performance,    // Performance-based allocation
    Capacity,       // Capacity-based allocation
}

/// Storage placement policies
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum PlacementPolicy {
    Local,          // Local storage only
    Distributed,    // Distributed across devices
    Tiered,         // Tier-based placement
    Mirrored,       // Mirrored placement
    Striped,        // Striped placement
}

/// Storage policy configuration
#[derive(Debug, Clone)]
pub struct StoragePolicy {
    pub allocation_policy: AllocationPolicy,
    pub placement_policy: PlacementPolicy,
    pub redundancy_level: u8,
    pub performance_requirement: u8,  // 1-5
    pub capacity_requirement: u64,
    pub compression_enabled: bool,
    pub deduplication_enabled: bool,
    pub encryption_enabled: bool,
    pub cache_enabled: bool,
    pub cache_size: usize,
}

impl StoragePolicy {
    pub fn new() -> Self {
        Self {
            allocation_policy: AllocationPolicy::BestFit,
            placement_policy: PlacementPolicy::Local,
            redundancy_level: 1,
            performance_requirement: 3,
            capacity_requirement: 0,
            compression_enabled: true,
            deduplication_enabled: true,
            encryption_enabled: false,
            cache_enabled: true,
            cache_size: 1024 * 1024 * 1024, // 1GB
        }
    }

    pub fn high_performance() -> Self {
        Self {
            allocation_policy: AllocationPolicy::Performance,
            placement_policy: PlacementPolicy::Tiered,
            redundancy_level: 2,
            performance_requirement: 5,
            capacity_requirement: 0,
            compression_enabled: false,
            deduplication_enabled: false,
            encryption_enabled: false,
            cache_enabled: true,
            cache_size: 4 * 1024 * 1024 * 1024, // 4GB
        }
    }

    pub fn high_capacity() -> Self {
        Self {
            allocation_policy: AllocationPolicy::Capacity,
            placement_policy: PlacementPolicy::Distributed,
            redundancy_level: 3,
            performance_requirement: 2,
            capacity_requirement: 0,
            compression_enabled: true,
            deduplication_enabled: true,
            encryption_enabled: true,
            cache_enabled: false,
            cache_size: 0,
        }
    }
}

// ========================================
// STORAGE METRICS
// ========================================

/// Storage performance metrics
#[derive(Debug, Clone)]
pub struct StorageMetrics {
    pub read_operations: AtomicU64,
    pub write_operations: AtomicU64,
    pub total_bytes_read: AtomicU64,
    pub total_bytes_written: AtomicU64,
    pub read_latency: AtomicU64,      // Average in microseconds
    pub write_latency: AtomicU64,     // Average in microseconds
    pub throughput: AtomicU64,        // MB/s
    pub iops: AtomicU64,              // Operations per second
    pub error_count: AtomicU32,
    pub last_update: AtomicU64,
}

impl StorageMetrics {
    pub fn new() -> Self {
        Self {
            read_operations: AtomicU64::new(0),
            write_operations: AtomicU64::new(0),
            total_bytes_read: AtomicU64::new(0),
            total_bytes_written: AtomicU64::new(0),
            read_latency: AtomicU64::new(0),
            write_latency: AtomicU64::new(0),
            throughput: AtomicU64::new(0),
            iops: AtomicU64::new(0),
            error_count: AtomicU32::new(0),
            last_update: AtomicU64::new(0),
        }
    }

    pub fn record_read(&self, bytes: u64, latency: u64) {
        self.read_operations.fetch_add(1, Ordering::Relaxed);
        self.total_bytes_read.fetch_add(bytes, Ordering::Relaxed);
        
        // Update average latency
        let current = self.read_latency.load(Ordering::Relaxed);
        let count = self.read_operations.load(Ordering::Relaxed);
        if count > 0 {
            let new_avg = (current + latency) / 2;
            self.read_latency.store(new_avg, Ordering::Relaxed);
        }
        
        self.last_update.store(0, Ordering::Relaxed); // Placeholder for timestamp
    }

    pub fn record_write(&self, bytes: u64, latency: u64) {
        self.write_operations.fetch_add(1, Ordering::Relaxed);
        self.total_bytes_written.fetch_add(bytes, Ordering::Relaxed);
        
        // Update average latency
        let current = self.write_latency.load(Ordering::Relaxed);
        let count = self.write_operations.load(Ordering::Relaxed);
        if count > 0 {
            let new_avg = (current + latency) / 2;
            self.write_latency.store(new_avg, Ordering::Relaxed);
        }
        
        self.last_update.store(0, Ordering::Relaxed); // Placeholder for timestamp
    }

    pub fn record_error(&self) {
        self.error_count.fetch_add(1, Ordering::Relaxed);
    }

    pub fn get_read_throughput(&self) -> f64 {
        let bytes = self.total_bytes_read.load(Ordering::Relaxed);
        let operations = self.read_operations.load(Ordering::Relaxed);
        
        if operations > 0 {
            (bytes as f64 / operations as f64) / (1024.0 * 1024.0) // MB per operation
        } else {
            0.0
        }
    }

    pub fn get_write_throughput(&self) -> f64 {
        let bytes = self.total_bytes_written.load(Ordering::Relaxed);
        let operations = self.write_operations.load(Ordering::Relaxed);
        
        if operations > 0 {
            (bytes as f64 / operations as f64) / (1024.0 * 1024.0) // MB per operation
        } else {
            0.0
        }
    }
}

// ========================================
// STORAGE MANAGER
// ========================================

/// Main storage manager
pub struct StorageManager {
    devices: RwLock<BTreeMap<String, StorageDevice>>,
    pools: RwLock<BTreeMap<String, StoragePool>>,
    policies: RwLock<BTreeMap<String, StoragePolicy>>,
    metrics: RwLock<BTreeMap<String, StorageMetrics>>,
    default_policy: StoragePolicy,
}

impl StorageManager {
    pub fn new() -> Self {
        Self {
            devices: RwLock::new(BTreeMap::new()),
            pools: RwLock::new(BTreeMap::new()),
            policies: RwLock::new(BTreeMap::new()),
            metrics: RwLock::new(BTreeMap::new()),
            default_policy: StoragePolicy::new(),
        }
    }

    /// Add a storage device
    pub fn add_device(&self, device: StorageDevice) -> StorageResult<()> {
        let device_id = device.id.clone();
        
        // Add device
        self.devices.write().insert(device_id.clone(), device);
        
        // Create metrics for device
        let metrics = StorageMetrics::new();
        self.metrics.write().insert(device_id.clone(), metrics);
        
        // Update pools that use this device
        self.update_pools_capacity();
        
        Ok(())
    }

    /// Remove a storage device
    pub fn remove_device(&self, device_id: &str) -> StorageResult<()> {
        // Check if device is in use by any pools
        let pools = self.pools.read();
        for pool in pools.values() {
            if pool.devices.contains(&device_id.to_string()) {
                return Err(StorageError::DeviceInUse);
            }
        }
        
        // Remove device and metrics
        self.devices.write().remove(device_id);
        self.metrics.write().remove(device_id);
        
        // Update pools capacity
        self.update_pools_capacity();
        
        Ok(())
    }

    /// Create a storage pool
    pub fn create_pool(&self, pool: StoragePool) -> StorageResult<()> {
        let pool_id = pool.id.clone();
        
        // Validate devices exist
        let devices = self.devices.read();
        for device_id in &pool.devices {
            if !devices.contains_key(device_id) {
                return Err(StorageError::DeviceNotFound);
            }
        }
        
        // Add pool
        let mut pools = self.pools.write();
        pools.insert(pool_id.clone(), pool);
        
        // Update pool capacity
        if let Some(pool) = pools.get_mut(&pool_id) {
            pool.calculate_capacity(&devices);
        }
        
        Ok(())
    }

    /// Get device by ID
    pub fn get_device(&self, device_id: &str) -> Option<StorageDevice> {
        self.devices.read().get(device_id).cloned()
    }

    /// Get pool by ID
    pub fn get_pool(&self, pool_id: &str) -> Option<StoragePool> {
        self.pools.read().get(pool_id).cloned()
    }

    /// Get metrics for a device
    pub fn get_device_metrics(&self, device_id: &str) -> Option<StorageMetrics> {
        self.metrics.read().get(device_id).cloned()
    }

    /// Update pools capacity after device changes
    fn update_pools_capacity(&self) {
        let devices = self.devices.read();
        let mut pools = self.pools.write();
        
        for pool in pools.values_mut() {
            pool.calculate_capacity(&devices);
        }
    }

    /// Get all devices
    pub fn get_all_devices(&self) -> Vec<StorageDevice> {
        self.devices.read().values().cloned().collect()
    }

    /// Get all pools
    pub fn get_all_pools(&self) -> Vec<StoragePool> {
        self.pools.read().values().cloned().collect()
    }

    /// Get total storage capacity
    pub fn get_total_capacity(&self) -> u64 {
        self.devices.read().values().map(|d| d.capacity).sum()
    }

    /// Get available storage capacity
    pub fn get_available_capacity(&self) -> u64 {
        self.pools.read().values().map(|p| p.available_capacity).sum()
    }
}

// ========================================
// ERROR HANDLING
// ========================================

/// Storage operation errors
#[derive(Debug, Clone, PartialEq)]
pub enum StorageError {
    DeviceNotFound,
    DeviceInUse,
    PoolNotFound,
    InsufficientCapacity,
    InvalidOperation,
    DeviceOffline,
    OperationFailed,
    PermissionDenied,
}

impl fmt::Display for StorageError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            StorageError::DeviceNotFound => write!(f, "Storage device not found"),
            StorageError::DeviceInUse => write!(f, "Storage device is in use"),
            StorageError::PoolNotFound => write!(f, "Storage pool not found"),
            StorageError::InsufficientCapacity => write!(f, "Insufficient storage capacity"),
            StorageError::InvalidOperation => write!(f, "Invalid storage operation"),
            StorageError::DeviceOffline => write!(f, "Storage device is offline"),
            StorageError::OperationFailed => write!(f, "Storage operation failed"),
            StorageError::PermissionDenied => write!(f, "Permission denied"),
        }
    }
}

/// Result type for storage operations
pub type StorageResult<T> = Result<T, StorageError>;

// ========================================
// MODULE INITIALIZATION
// ========================================

/// Initialize the core storage module
pub fn initialize() -> StorageResult<()> {
    // Core module initialization logic
    Ok(())
}

/// Shutdown the core storage module
pub fn shutdown() -> StorageResult<()> {
    // Core module shutdown logic
    Ok(())
}
