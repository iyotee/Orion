/*
 * Orion Operating System - Kernel Logging System Test
 *
 * Test file for the kernel logging system to verify compilation
 * and basic functionality.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/klog.h>

// Define missing constants for testing
#define KLOG_CAT_MEMORY KLOG_CAT_KERNEL
#define KLOG_CAT_PROCESS KLOG_CAT_KERNEL

// Test function to verify logging system compilation
void test_kernel_logging(void)
{
    // Test basic logging functions
    klog_info(KLOG_CAT_KERNEL, "Testing kernel logging system");
    klog_warning(KLOG_CAT_DRIVER, "This is a test warning message");
    klog_err(KLOG_CAT_MEMORY, "This is a test error message");
    klog_debug(KLOG_CAT_PROCESS, "This is a test debug message");

    // Test driver logging macros
    driver_log("Driver test message");
    driver_error("Driver error test");
    driver_warning("Driver warning test");
    driver_debug("Driver debug test");

    // Test emergency logging
    klog_emergency("Emergency test message");

    // Test log level changes
    klog_set_level(KLOG_DEBUG);
    uint8_t level = klog_get_level();

    // Test buffer operations
    int buffer_id = klog_add_buffer(KLOG_CAT_NETWORK);
    if (buffer_id >= 0)
    {
        klog_remove_buffer(buffer_id);
    }

    // Test log flushing
    klog_flush();
}
