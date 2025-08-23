/*
 * Orion Operating System - Storage Framework
 *
 * Unified storage management framework providing advanced features:
 * - Multi-layer caching with intelligent eviction policies
 * - Compression and deduplication optimization
 * - Encryption and security management
 * - Performance monitoring and adaptive optimization
 * - Hardware acceleration support
 * - Distributed storage coordination
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#![no_std]

extern crate alloc;

// ========================================
// CORE STORAGE FRAMEWORK MODULES
// ========================================

pub mod core;
pub mod cache;
pub mod optimization;
pub mod security;
pub mod monitoring;
pub mod coordination;
pub mod drivers;

// ========================================
// RE-EXPORTS FOR EASY ACCESS
// ========================================

// Core storage types
pub use core::{
    StorageManager,
    StorageDevice,
    StoragePool,
    StoragePolicy,
    StorageMetrics,
    StorageError,
    StorageResult,
};

// Cache management
pub use cache::{
    CacheManager,
    CachePolicy,
    CacheLevel,
    CacheMetrics,
    CacheStrategy,
};

// Optimization engine
pub use optimization::{
    OptimizationEngine,
    OptimizationStrategy,
    CompressionOptimizer,
    DeduplicationOptimizer,
    PerformanceOptimizer,
};

// Security features
pub use security::{
    SecurityManager,
    EncryptionProvider,
    AccessControl,
    AuditLogger,
};

// Performance monitoring
pub use monitoring::{
    PerformanceMonitor,
    MetricsCollector,
    AlertManager,
    PerformanceReport,
};

// Distributed coordination
pub use coordination::{
    CoordinationManager,
    ClusterManager,
    ReplicationManager,
    LoadBalancer,
};

// Storage drivers
pub use drivers::{
    DriverManager,
    DriverInterface,
    DriverMetrics,
    DriverRegistry,
};

// ========================================
// FRAMEWORK CONFIGURATION
// ========================================

/// Framework version information
pub const VERSION: &str = "1.0.0";

/// Maximum number of storage devices supported
pub const MAX_STORAGE_DEVICES: usize = 256;

/// Maximum number of storage pools
pub const MAX_STORAGE_POOLS: usize = 64;

/// Maximum cache levels supported
pub const MAX_CACHE_LEVELS: usize = 4;

/// Default cache size (1GB)
pub const DEFAULT_CACHE_SIZE: usize = 1024 * 1024 * 1024;

/// Default compression ratio threshold
pub const DEFAULT_COMPRESSION_RATIO: f64 = 0.8;

/// Default deduplication threshold
pub const DEFAULT_DEDUPLICATION_THRESHOLD: usize = 4096;

// ========================================
// FRAMEWORK INITIALIZATION
// ========================================

/// Initialize the storage framework
pub fn initialize_framework() -> StorageResult<()> {
    // Initialize core components
    core::initialize()?;
    
    // Initialize cache system
    cache::initialize()?;
    
    // Initialize optimization engine
    optimization::initialize()?;
    
    // Initialize security manager
    security::initialize()?;
    
    // Initialize monitoring system
    monitoring::initialize()?;
    
    // Initialize coordination system
    coordination::initialize()?;
    
    // Initialize driver system
    drivers::initialize()?;
    
    Ok(())
}

/// Shutdown the storage framework
pub fn shutdown_framework() -> StorageResult<()> {
    // Shutdown in reverse order
    drivers::shutdown()?;
    coordination::shutdown()?;
    monitoring::shutdown()?;
    security::shutdown()?;
    optimization::shutdown()?;
    cache::shutdown()?;
    core::shutdown()?;
    
    Ok(())
}

// ========================================
// ERROR HANDLING
// ========================================

/// Storage framework error types
#[derive(Debug, Clone, PartialEq)]
pub enum StorageFrameworkError {
    InitializationFailed,
    ComponentNotFound,
    ConfigurationError,
    ResourceExhausted,
    OperationFailed,
    SecurityViolation,
    CoordinationError,
}

impl core::fmt::Display for StorageFrameworkError {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        match self {
            StorageFrameworkError::InitializationFailed => write!(f, "Framework initialization failed"),
            StorageFrameworkError::ComponentNotFound => write!(f, "Storage component not found"),
            StorageFrameworkError::ConfigurationError => write!(f, "Configuration error"),
            StorageFrameworkError::ResourceExhausted => write!(f, "Resource exhausted"),
            StorageFrameworkError::OperationFailed => write!(f, "Operation failed"),
            StorageFrameworkError::SecurityViolation => write!(f, "Security violation"),
            StorageFrameworkError::CoordinationError => write!(f, "Coordination error"),
        }
    }
}

/// Result type for storage framework operations
pub type StorageFrameworkResult<T> = core::result::Result<T, StorageFrameworkError>;

// ========================================
// FEATURE FLAGS
// ========================================

/// Check if compression is enabled
pub fn compression_enabled() -> bool {
    cfg!(feature = "compression")
}

/// Check if deduplication is enabled
pub fn deduplication_enabled() -> bool {
    cfg!(feature = "deduplication")
}

/// Check if encryption is enabled
pub fn encryption_enabled() -> bool {
    cfg!(feature = "encryption")
}

/// Check if caching is enabled
pub fn caching_enabled() -> bool {
    cfg!(feature = "caching")
}

/// Check if performance monitoring is enabled
pub fn performance_monitoring_enabled() -> bool {
    cfg!(feature = "performance-monitoring")
}

/// Check if adaptive optimization is enabled
pub fn adaptive_optimization_enabled() -> bool {
    cfg!(feature = "adaptive-optimization")
}

/// Check if hardware acceleration is enabled
pub fn hardware_acceleration_enabled() -> bool {
    cfg!(feature = "hardware-acceleration")
}
