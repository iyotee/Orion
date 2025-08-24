/*
 * Orion Operating System - Network Drivers Library
 *
 * This module provides access to all network drivers and the network manager
 * for the Orion Operating System.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

// Export all network drivers
pub mod e1000;
pub mod e1000e;
pub mod rtl8139;
pub mod virtio_net;
pub mod network_manager;

// Re-export main driver types for easy access
pub use e1000::AdvancedE1000Driver;
pub use e1000e::EnhancedE1000EDriver;
pub use rtl8139::RTL8169Driver;
pub use virtio_net::VirtioNetDriver;
pub use network_manager::NetworkDriverManager;

// Re-export common types and structures
pub use network_manager::{
    NetworkInterface,
    AggregatedNetworkStats,
    NetworkConfiguration,
};

// Re-export driver traits
pub use orion_driver::{
    NetworkDriver,
    OrionDriver,
    DeviceInfo,
    DriverError,
    DriverResult,
    LinkStatus,
    NetworkStats,
    BusType,
    MmioAccessor,
    MmioPermissions,
    MessageLoop,
    ReceivedMessage,
    IpcInterface,
};

// Version information
pub const VERSION: &str = "2.0.0";
pub const AUTHOR: &str = "Jeremy Noverraz <jeremy@orion-os.dev>";
pub const DESCRIPTION: &str = "Complete network driver suite for Orion OS";

/// Get the version of the network drivers library
pub fn version() -> &'static str {
    VERSION
}

/// Get information about the network drivers library
pub fn info() -> &'static str {
    DESCRIPTION
}

/// Get the author information
pub fn author() -> &'static str {
    AUTHOR
}

/// Check if all required dependencies are available
pub fn check_dependencies() -> bool {
    // This would typically check for required system components
    // For now, return true as a placeholder
    true
}

/// Initialize the network drivers library
pub fn initialize() -> Result<(), &'static str> {
    // This would typically perform library initialization
    // For now, return Ok as a placeholder
    Ok(())
}

/// Get a list of all available network drivers
pub fn available_drivers() -> &'static [&'static str] {
    &[
        "e1000",
        "e1000e", 
        "rtl8169",
        "virtio_net",
    ]
}

/// Get a list of all available network driver features
pub fn available_features() -> &'static [&'static str] {
    &[
        "hardware_checksum_offload",
        "tcp_segmentation_offload",
        "jumbo_frames",
        "vlan_support",
        "qos_support",
        "power_management",
        "interrupt_coalescing",
        "multi_queue",
        "flow_control",
        "wake_on_lan",
    ]
}

/// Get driver information for a specific driver
pub fn get_driver_info(driver_name: &str) -> Option<&'static str> {
    match driver_name {
        "e1000" => Some("Intel e1000 Series Gigabit Ethernet Driver"),
        "e1000e" => Some("Intel e1000e Enhanced Gigabit Ethernet Driver"),
        "rtl8169" => Some("Realtek RTL8169 Gigabit Ethernet Driver"),
        "virtio_net" => Some("VirtIO Network Driver"),
        _ => None,
    }
}

/// Get driver version for a specific driver
pub fn get_driver_version(driver_name: &str) -> Option<&'static str> {
    match driver_name {
        "e1000" | "e1000e" | "rtl8169" | "virtio_net" => Some("2.0.0"),
        _ => None,
    }
}

/// Check if a driver supports a specific feature
pub fn driver_supports_feature(driver_name: &str, feature: &str) -> bool {
    match (driver_name, feature) {
        // All drivers support basic features
        (_, "hardware_checksum_offload") => true,
        (_, "tcp_segmentation_offload") => true,
        (_, "vlan_support") => true,
        (_, "qos_support") => true,
        (_, "power_management") => true,
        (_, "interrupt_coalescing") => true,
        (_, "flow_control") => true,
        
        // Specific feature support
        ("e1000" | "e1000e", "jumbo_frames") => true,
        ("rtl8169", "jumbo_frames") => true,
        ("virtio_net", "jumbo_frames") => true,
        
        ("e1000" | "e1000e", "multi_queue") => true,
        ("virtio_net", "multi_queue") => true,
        
        ("e1000" | "e1000e", "wake_on_lan") => true,
        ("rtl8169", "wake_on_lan") => true,
        
        _ => false,
    }
}

/// Get the maximum supported speed for a driver
pub fn get_max_speed(driver_name: &str) -> Option<u32> {
    match driver_name {
        "e1000" | "e1000e" | "rtl8169" => Some(1000), // 1 Gbps
        "virtio_net" => Some(10000), // 10 Gbps (depending on hypervisor)
        _ => None,
    }
}

/// Get the maximum supported MTU for a driver
pub fn get_max_mtu(driver_name: &str) -> Option<u16> {
    match driver_name {
        "e1000" | "e1000e" | "rtl8169" | "virtio_net" => Some(9000), // 9KB jumbo frames
        _ => None,
    }
}

/// Get the default ring size for a driver
pub fn get_default_ring_size(driver_name: &str) -> Option<usize> {
    match driver_name {
        "e1000" | "e1000e" | "rtl8169" | "virtio_net" => Some(256),
        _ => None,
    }
}

/// Get the maximum ring size for a driver
pub fn get_max_ring_size(driver_name: &str) -> Option<usize> {
    match driver_name {
        "e1000" | "e1000e" | "rtl8169" | "virtio_net" => Some(1024),
        _ => None,
    }
}

/// Get the default buffer size for a driver
pub fn get_default_buffer_size(driver_name: &str) -> Option<usize> {
    match driver_name {
        "e1000" | "e1000e" | "rtl8169" | "virtio_net" => Some(2048),
        _ => None,
    }
}

/// Get the maximum buffer size for a driver
pub fn get_max_buffer_size(driver_name: &str) -> Option<usize> {
    match driver_name {
        "e1000" | "e1000e" | "rtl8169" | "virtio_net" => Some(16384),
        _ => None,
    }
}

/// Get the interrupt coalescing settings for a driver
pub fn get_interrupt_coalescing_settings(driver_name: &str) -> Option<(u32, u32)> {
    match driver_name {
        "e1000" | "e1000e" | "rtl8169" | "virtio_net" => Some((100, 1000)), // (packets, microseconds)
        _ => None,
    }
}

/// Get the power management capabilities for a driver
pub fn get_power_management_capabilities(driver_name: &str) -> Option<&'static [&'static str]> {
    match driver_name {
        "e1000" | "e1000e" => Some(&["D0", "D1", "D2", "D3hot", "D3cold"]),
        "rtl8169" => Some(&["D0", "D1", "D3hot"]),
        "virtio_net" => Some(&["D0", "D3hot"]),
        _ => None,
    }
}

/// Get the supported link speeds for a driver
pub fn get_supported_link_speeds(driver_name: &str) -> Option<&'static [u32]> {
    match driver_name {
        "e1000" | "e1000e" | "rtl8169" => Some(&[10, 100, 1000]), // 10 Mbps, 100 Mbps, 1 Gbps
        "virtio_net" => Some(&[1000, 10000]), // 1 Gbps, 10 Gbps
        _ => None,
    }
}

/// Get the supported duplex modes for a driver
pub fn get_supported_duplex_modes(driver_name: &str) -> Option<&'static [&'static str]> {
    match driver_name {
        "e1000" | "e1000e" | "rtl8169" | "virtio_net" => Some(&["half", "full"]),
        _ => None,
    }
}

/// Get the supported VLAN features for a driver
pub fn get_supported_vlan_features(driver_name: &str) -> Option<&'static [&'static str]> {
    match driver_name {
        "e1000" | "e1000e" | "rtl8169" | "virtio_net" => Some(&["vlan_tagging", "vlan_filtering", "vlan_stripping"]),
        _ => None,
    }
}

/// Get the supported QoS features for a driver
pub fn get_supported_qos_features(driver_name: &str) -> Option<&'static [&'static str]> {
    match driver_name {
        "e1000" | "e1000e" | "rtl8169" | "virtio_net" => Some(&["priority_queuing", "traffic_shaping", "congestion_management"]),
        _ => None,
    }
}

/// Get the supported security features for a driver
pub fn get_supported_security_features(driver_name: &str) -> Option<&'static [&'static str]> {
    match driver_name {
        "e1000" | "e1000e" | "rtl8169" | "virtio_net" => Some(&["mac_filtering", "promiscuous_mode", "vlan_isolation"]),
        _ => None,
    }
}

/// Get the supported diagnostic features for a driver
pub fn get_supported_diagnostic_features(driver_name: &str) -> Option<&'static [&'static str]> {
    match driver_name {
        "e1000" | "e1000e" | "rtl8169" | "virtio_net" => Some(&["link_status", "statistics", "error_counting", "performance_monitoring"]),
        _ => None,
    }
}

/// Get the supported management features for a driver
pub fn get_supported_management_features(driver_name: &str) -> Option<&'static [&'static str]> {
    match driver_name {
        "e1000" | "e1000e" | "rtl8169" | "virtio_net" => Some(&["hot_plugging", "firmware_update", "configuration_management", "remote_management"]),
        _ => None,
    }
}

/// Get comprehensive driver information
pub fn get_comprehensive_driver_info(driver_name: &str) -> Option<String> {
    let mut info = String::new();
    
    if let Some(desc) = get_driver_info(driver_name) {
        info.push_str(&format!("Driver: {}\n", desc));
    }
    
    if let Some(version) = get_driver_version(driver_name) {
        info.push_str(&format!("Version: {}\n", version));
    }
    
    if let Some(speed) = get_max_speed(driver_name) {
        info.push_str(&format!("Max Speed: {} Mbps\n", speed));
    }
    
    if let Some(mtu) = get_max_mtu(driver_name) {
        info.push_str(&format!("Max MTU: {}\n", mtu));
    }
    
    if let Some(ring_size) = get_default_ring_size(driver_name) {
        info.push_str(&format!("Default Ring Size: {}\n", ring_size));
    }
    
    if let Some(buffer_size) = get_default_buffer_size(driver_name) {
        info.push_str(&format!("Default Buffer Size: {}\n", buffer_size));
    }
    
    if let Some(speeds) = get_supported_link_speeds(driver_name) {
        info.push_str(&format!("Supported Speeds: {:?} Mbps\n", speeds));
    }
    
    if let Some(duplex_modes) = get_supported_duplex_modes(driver_name) {
        info.push_str(&format!("Supported Duplex Modes: {:?}\n", duplex_modes));
    }
    
    if let Some(features) = get_supported_vlan_features(driver_name) {
        info.push_str(&format!("VLAN Features: {:?}\n", features));
    }
    
    if let Some(features) = get_supported_qos_features(driver_name) {
        info.push_str(&format!("QoS Features: {:?}\n", features));
    }
    
    if let Some(features) = get_supported_security_features(driver_name) {
        info.push_str(&format!("Security Features: {:?}\n", features));
    }
    
    if Some(info.is_empty()) == Some(false) {
        Some(info)
    } else {
        None
    }
}

/// Get a summary of all available drivers
pub fn get_drivers_summary() -> String {
    let mut summary = String::new();
    summary.push_str("=== Orion OS Network Drivers Summary ===\n\n");
    
    for driver in available_drivers() {
        if let Some(info) = get_driver_info(driver) {
            summary.push_str(&format!("{}: {}\n", driver, info));
        }
        if let Some(version) = get_driver_version(driver) {
            summary.push_str(&format!("  Version: {}\n", version));
        }
        if let Some(speed) = get_max_speed(driver) {
            summary.push_str(&format!("  Max Speed: {} Mbps\n", speed));
        }
        summary.push_str("\n");
    }
    
    summary.push_str(&format!("Total Drivers: {}\n", available_drivers().len()));
    summary.push_str(&format!("Library Version: {}\n", VERSION));
    summary.push_str(&format!("Author: {}\n", AUTHOR));
    
    summary
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_version() {
        assert_eq!(version(), "2.0.0");
    }
    
    #[test]
    fn test_available_drivers() {
        let drivers = available_drivers();
        assert!(drivers.contains(&"e1000"));
        assert!(drivers.contains(&"e1000e"));
        assert!(drivers.contains(&"rtl8169"));
        assert!(drivers.contains(&"virtio_net"));
    }
    
    #[test]
    fn test_driver_info() {
        assert!(get_driver_info("e1000").is_some());
        assert!(get_driver_info("nonexistent").is_none());
    }
    
    #[test]
    fn test_driver_version() {
        assert_eq!(get_driver_version("e1000"), Some("2.0.0"));
        assert_eq!(get_driver_version("nonexistent"), None);
    }
    
    #[test]
    fn test_max_speed() {
        assert_eq!(get_max_speed("e1000"), Some(1000));
        assert_eq!(get_max_speed("virtio_net"), Some(10000));
        assert_eq!(get_max_speed("nonexistent"), None);
    }
    
    #[test]
    fn test_max_mtu() {
        assert_eq!(get_max_mtu("e1000"), Some(9000));
        assert_eq!(get_max_mtu("nonexistent"), None);
    }
    
    #[test]
    fn test_feature_support() {
        assert!(driver_supports_feature("e1000", "hardware_checksum_offload"));
        assert!(driver_supports_feature("e1000", "jumbo_frames"));
        assert!(!driver_supports_feature("e1000", "nonexistent_feature"));
    }
}
