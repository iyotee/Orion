/*
 * Orion Operating System - x86_64 Interrupt Handlers
 *
 * C-language interrupt and exception handlers for x86_64 architecture.
 *
 * Developed by Jeremy Noverraz (1988-2025)
 * August 2025, Lausanne, Switzerland
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#include <orion/kernel.h>
#include <orion/types.h>
#include <orion/scheduler.h>
#include <orion/mm.h>

// Mouse event types
typedef enum
{
    MOUSE_EVENT_MOVE,
    MOUSE_EVENT_BUTTON_DOWN,
    MOUSE_EVENT_BUTTON_UP,
    MOUSE_EVENT_WHEEL
} mouse_event_type_t;

// Mouse event structure
typedef struct
{
    mouse_event_type_t type;
    int8_t dx;
    int8_t dy;
    uint8_t buttons;
    int8_t wheel_delta;
} mouse_event_t;

// Forward declarations
static void rtc_update_system_clock(void);
static bool rtc_check_alarms(void);
static void rtc_handle_alarm_events(void);
static char scancode_to_ascii(uint8_t scancode);
void windowing_system_send_mouse_event(mouse_event_t *event)
{
    // Basic mouse event handling for windowing system
    // This implementation provides a foundation for a full windowing system

    if (!event)
    {
        return; // Invalid event
    }

    // Log the mouse event for debugging
    kdebug("Mouse event: type=%u, x=%d, y=%d, buttons=0x%x",
           event->type, event->dx, event->dy, event->buttons);

    // Add event to windowing system queue
    // In a real implementation, this would:
    // - Add event to a lock-free queue
    // - Wake up windowing system thread
    // - Handle event based on current focus window

    // For now, implement basic event queuing
    extern bool windowing_system_queue_event(void *event_data, size_t event_size);
    extern void windowing_system_wakeup(void);

    if (windowing_system_queue_event(event, sizeof(mouse_event_t)))
    {
        // Event queued successfully, wake up windowing system
        windowing_system_wakeup();
        kdebug("Mouse event queued and windowing system woken up");
    }
    else
    {
        // Queue full or error, log warning
        kwarn("Failed to queue mouse event - queue may be full");
    }
}

// Page fault handling functions
bool demand_paging_handle_fault(uint64_t fault_addr, uint64_t error_code)
{
    // Basic demand paging implementation
    // This provides a foundation for full demand paging support

    // Check if the fault is for a valid memory region
    // In a real implementation, this would check the process's memory map

    // For now, implement basic demand paging logic
    bool present = !(error_code & 1);
    bool write = error_code & 2;
    bool user = error_code & 4;

    if (!present)
    {
        // Page not present - this is what demand paging handles
        kdebug("Demand paging: Page fault at 0x%llx (not present)",
               (unsigned long long)fault_addr);

        // Get current process's address space
        extern vm_space_t *vmm_get_current_space(void);
        vm_space_t *current_space = vmm_get_current_space();

        if (current_space)
        {
            // Try to allocate and map a new page
            extern int vmm_allocate_page(vm_space_t * space, uint64_t addr, uint32_t flags);
            int result = vmm_allocate_page(current_space, fault_addr,
                                           0x02 | 0x04); // PTE_WRITE | PTE_USER

            if (result == OR_OK)
            {
                kinfo("Demand paging: Successfully allocated page at 0x%llx",
                      (unsigned long long)fault_addr);
                return true; // Page fault resolved
            }
            else
            {
                kerror("Demand paging: Failed to allocate page at 0x%llx (error: %d)",
                       (unsigned long long)fault_addr, result);
            }
        }
        else
        {
            kerror("Demand paging: No current address space available");
        }
    }

    return false; // Not handled or failed
}

bool cow_handle_fault(uint64_t fault_addr, uint64_t error_code)
{
    // Check if this is a write fault
    bool write_fault = (error_code & 2) != 0;
    if (!write_fault)
    {
        return false; // Not a COW fault
    }

    // Get current process's address space
    extern vm_space_t *vmm_get_current_space(void);
    vm_space_t *current_space = vmm_get_current_space(); // Get current process space

    if (!current_space)
    {
        // Fallback to kernel space if no current process space
        extern vm_space_t *vmm_get_kernel_space(void);
        current_space = vmm_get_kernel_space();
        kdebug("cow_handle_fault: No current process space, using kernel space");
    }

    // Try to handle as COW fault
    int result = vmm_handle_cow_fault(current_space, fault_addr);
    if (result == OR_OK)
    {
        kdebug("cow_handle_fault: Successfully handled COW fault at 0x%p", (void *)fault_addr);
        return true;
    }

    kdebug("cow_handle_fault: Could not handle fault at 0x%p (error: %d)",
           (void *)fault_addr, result);
    return false;
}

// ========================================
// EXCEPTION HANDLERS
// ========================================

// Common exception handler
void isr_handler(uint64_t isr_num, uint64_t error_code, uint64_t rip,
                 uint64_t cs, uint64_t rflags, uint64_t rsp, uint64_t ss)
{
    kerror("Exception %llu at RIP=0x%llx, CS=0x%llx, RFLAGS=0x%llx",
           (unsigned long long)isr_num, (unsigned long long)rip,
           (unsigned long long)cs, (unsigned long long)rflags);

    if (error_code != 0xFFFFFFFFFFFFFFFF)
    {
        kerror("Error code: 0x%llx", (unsigned long long)error_code);
    }

    // Handle specific exceptions
    switch (isr_num)
    {
    case 0: // Divide by zero
        kerror("Divide by zero exception");
        break;
    case 6: // Invalid opcode
        kerror("Invalid opcode exception");
        break;
    case 8: // Double fault
        kerror("Double fault - system halted");
        arch_halt();
        break;
    case 13: // General protection fault
        kerror("General protection fault");
        break;
    case 14: // Page fault
        handle_page_fault(error_code, rip, rsp);
        break;
    default:
        kerror("Unhandled exception %llu", (unsigned long long)isr_num);
        break;
    }

    // For now, halt the system on critical exceptions
    if (isr_num >= 8 && isr_num <= 14)
    {
        kerror("Critical exception - system halted");
        arch_halt();
    }
}

// Common IRQ handler
void irq_handler(uint64_t irq_num)
{
    // Handle specific IRQs
    switch (irq_num)
    {
    case 0: // PIT timer
        handle_timer_interrupt();
        break;
    case 1: // Keyboard
        handle_keyboard_interrupt();
        break;
    case 8: // RTC
        handle_rtc_interrupt();
        break;
    case 12: // PS/2 Mouse
        handle_mouse_interrupt();
        break;
    default:
        kdebug("Unhandled IRQ %llu", (unsigned long long)irq_num);
        break;
    }

    // Send EOI to PIC or APIC
    send_eoi(irq_num);
}

// Page fault handler implementation
void handle_page_fault(uint64_t error_code, uint64_t rip, uint64_t rsp)
{
    uint64_t fault_addr = 0;
#ifdef _MSC_VER
    // MSVC implementation using MSVC intrinsic for reading CR2
    extern uint64_t read_cr2(void);
    fault_addr = read_cr2();
#else
    __asm__ volatile("mov %%cr2, %0" : "=r"(fault_addr));
#endif

    bool present = !(error_code & 1);
    bool write = error_code & 2;
    bool user = error_code & 4;
    bool reserved = error_code & 8;
    bool fetch = error_code & 16;

    kerror("Page fault at 0x%llx (RIP: 0x%llx, RSP: 0x%llx)",
           (unsigned long long)fault_addr, (unsigned long long)rip, (unsigned long long)rsp);

    if (present)
    {
        kerror("  Page not present");
    }
    if (write)
    {
        kerror("  Write access");
    }
    else
    {
        kerror("  Read access");
    }
    if (user)
    {
        kerror("  User mode access");
    }
    else
    {
        kerror("  Kernel mode access");
    }
    if (reserved)
    {
        kerror("  Reserved bit violation");
    }
    if (fetch)
    {
        kerror("  Instruction fetch");
    }

    // Try to handle page fault gracefully
    if (demand_paging_handle_fault(fault_addr, error_code))
    {
        return; // Page fault resolved
    }

    // Check for COW (Copy-on-Write) fault
    if (cow_handle_fault(fault_addr, error_code))
    {
        return; // COW fault resolved
    }

    // If we can't handle it, panic
    panic("Unhandled page fault");
}

// Timer interrupt handler
void handle_timer_interrupt(void)
{
    static uint64_t timer_ticks = 0;
    timer_ticks++;

    // Update system time
    arch_update_system_time();

    // Call scheduler tick for preemption
    if (timer_ticks % 10 == 0)
    { // Every ~10ms
        scheduler_tick();
    }

    // Update uptime every second (assuming 100Hz timer)
    if (timer_ticks % 100 == 0)
    {
        kdebug("System uptime: %llu seconds", (unsigned long long)(timer_ticks / 100));
    }
}

// Keyboard interrupt handler
void handle_keyboard_interrupt(void)
{
    uint8_t scancode = inb(0x60); // Read from keyboard controller

    // Basic scancode handling
    if (scancode & 0x80)
    {
        // Key release
        kdebug("Key released: 0x%x", scancode & 0x7F);
    }
    else
    {
        // Key press
        kdebug("Key pressed: 0x%x", scancode);

        // Handle special keys
        switch (scancode)
        {
        case 0x01: // ESC
            kprintf("ESC pressed - entering debug mode\n");
            break;
        case 0x1C: // Enter
            kprintf("\n");
            break;
        case 0x39: // Space
            kprintf(" ");
            break;
        default:
            // Translate scancode to ASCII
            char ascii = scancode_to_ascii(scancode);
            if (ascii)
            {
                kprintf("%c", ascii);
            }
            else
            {
                kprintf("?");
            }
            break;
        }
    }
}

// RTC interrupt handler
void handle_rtc_interrupt(void)
{
    // Read RTC status register C to clear interrupt
    outb(0x70, 0x0C);
    inb(0x71);

    kdebug("RTC interrupt");

    // Update system clock from RTC
    rtc_update_system_clock();

    // Check for RTC alarms
    if (rtc_check_alarms())
    {
        // Handle alarm events
        rtc_handle_alarm_events();
    }
}

// Mouse interrupt handler
void handle_mouse_interrupt(void)
{
    static uint8_t mouse_packet[3];
    static int packet_index = 0;

    uint8_t data = inb(0x60);
    mouse_packet[packet_index++] = data;

    if (packet_index >= 3)
    {
        // Complete packet received
        int8_t dx = (int8_t)mouse_packet[1];
        int8_t dy = (int8_t)mouse_packet[2];
        uint8_t buttons = mouse_packet[0];

        kdebug("Mouse: dx=%d, dy=%d, buttons=0x%x", dx, dy, buttons);

        // Send mouse events to windowing system
        mouse_event_t event = {
            .type = MOUSE_EVENT_MOVE,
            .dx = dx,
            .dy = dy,
            .buttons = buttons};
        windowing_system_send_mouse_event(&event);
        packet_index = 0;
    }
}

// RTC functions
static uint64_t system_time_seconds = 0; // Global system time in seconds since epoch
static uint32_t system_time_ticks = 0;   // Sub-second ticks

// Public functions to access system time
uint64_t arch_get_system_time(void)
{
    return system_time_seconds;
}

void arch_update_system_time(void)
{
    system_time_ticks++;
    if (system_time_ticks >= 100)
    { // Assuming 100Hz timer
        system_time_seconds++;
        system_time_ticks = 0;
    }
}

static void rtc_update_system_clock(void)
{
    // Read current RTC time registers
    uint8_t seconds, minutes, hours, day, month, year;

    // Read RTC registers (BCD format)
    outb(0x70, 0x00);
    seconds = inb(0x71);
    outb(0x70, 0x02);
    minutes = inb(0x71);
    outb(0x70, 0x04);
    hours = inb(0x71);
    outb(0x70, 0x07);
    day = inb(0x71);
    outb(0x70, 0x08);
    month = inb(0x71);
    outb(0x70, 0x09);
    year = inb(0x71);

    // Convert BCD to binary
    seconds = (seconds & 0x0F) + ((seconds >> 4) * 10);
    minutes = (minutes & 0x0F) + ((minutes >> 4) * 10);
    hours = (hours & 0x0F) + ((hours >> 4) * 10);
    day = (day & 0x0F) + ((day >> 4) * 10);
    month = (month & 0x0F) + ((month >> 4) * 10);
    year = (year & 0x0F) + ((year >> 4) * 10);

    // Convert to full year (assuming 20xx)
    year += 2000;

    // Simple Unix timestamp calculation (approximate)
    // This is a simplified version - real implementation would handle leap years properly
    uint64_t days_since_epoch = 0;

    // Calculate days since 1970
    for (uint32_t y = 1970; y < year; y++)
    {
        days_since_epoch += 365;
        if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0))
        {
            days_since_epoch++; // Leap year
        }
    }

    // Add days for months in current year
    uint32_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
    {
        days_in_month[1] = 29; // February in leap year
    }

    for (uint32_t m = 1; m < month; m++)
    {
        days_since_epoch += days_in_month[m - 1];
    }

    days_since_epoch += day - 1; // Days in current month (0-indexed)

    // Convert to seconds
    system_time_seconds = days_since_epoch * 86400 + hours * 3600 + minutes * 60 + seconds;

    kdebug("rtc_update_system_clock: Updated system time to %02u/%02u/%04u %02u:%02u:%02u (timestamp: %llu)",
           day, month, year, hours, minutes, seconds, (unsigned long long)system_time_seconds);
}

static bool rtc_check_alarms(void)
{
    // Read RTC status register to check for alarms
    outb(0x70, 0x0B); // Register B
    uint8_t reg_b = inb(0x71);

    // Check if alarms are enabled
    bool alarm_interrupt_enabled = (reg_b & 0x20) != 0;
    if (!alarm_interrupt_enabled)
    {
        return false;
    }

    // Read alarm registers
    outb(0x70, 0x01);
    uint8_t alarm_seconds = inb(0x71);
    outb(0x70, 0x03);
    uint8_t alarm_minutes = inb(0x71);
    outb(0x70, 0x05);
    uint8_t alarm_hours = inb(0x71);

    // Read current time
    outb(0x70, 0x00);
    uint8_t current_seconds = inb(0x71);
    outb(0x70, 0x02);
    uint8_t current_minutes = inb(0x71);
    outb(0x70, 0x04);
    uint8_t current_hours = inb(0x71);

    // Convert BCD to binary for comparison
    alarm_seconds = (alarm_seconds & 0x0F) + ((alarm_seconds >> 4) * 10);
    alarm_minutes = (alarm_minutes & 0x0F) + ((alarm_minutes >> 4) * 10);
    alarm_hours = (alarm_hours & 0x0F) + ((alarm_hours >> 4) * 10);

    current_seconds = (current_seconds & 0x0F) + ((current_seconds >> 4) * 10);
    current_minutes = (current_minutes & 0x0F) + ((current_minutes >> 4) * 10);
    current_hours = (current_hours & 0x0F) + ((current_hours >> 4) * 10);

    // Check if current time matches alarm time
    bool alarm_triggered = (current_hours == alarm_hours &&
                            current_minutes == alarm_minutes &&
                            current_seconds == alarm_seconds);

    if (alarm_triggered)
    {
        kinfo("rtc_check_alarms: Alarm triggered at %02u:%02u:%02u",
              alarm_hours, alarm_minutes, alarm_seconds);
    }

    return alarm_triggered;
}

static void rtc_handle_alarm_events(void)
{
    // Log the alarm event
    kinfo("rtc_handle_alarm_events: Processing RTC alarm");

    // Send signal to processes waiting for alarms
    // Basic signal delivery system implementation
    extern void scheduler_send_signal_to_all(uint32_t signal);
    extern void scheduler_wakeup_signal_waiters(uint32_t signal);

    // Send SIGALRM signal to all processes
    // In a real implementation, this would check which processes are waiting for alarms
    scheduler_send_signal_to_all(14); // SIGALRM = 14

    // Wake up processes waiting for alarm signals
    scheduler_wakeup_signal_waiters(14);

    // For now, just increment a global alarm counter
    static uint64_t alarm_count = 0;
    alarm_count++;

    kinfo("rtc_handle_alarm_events: Alarm #%llu processed, SIGALRM sent",
          (unsigned long long)alarm_count);

    // Clear the alarm interrupt flag
    outb(0x70, 0x0C);
    uint8_t reg_c = inb(0x71); // Reading register C clears the interrupt
    (void)reg_c;               // Suppress unused variable warning

    // Wake up any processes waiting on alarm events
    // Process wake-up mechanism implemented above via signal delivery
}

// Scancode to ASCII conversion table
static char scancode_to_ascii(uint8_t scancode)
{
    // Basic US QWERTY layout
    static const char ascii_table[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
        0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
        '*', 0, ' '};

    if (scancode < sizeof(ascii_table))
    {
        return ascii_table[scancode];
    }
    return 0;
}

// Send End of Interrupt signal
void send_eoi(uint64_t irq_num)
{
    if (irq_num >= 8)
    {
        // Send EOI to slave PIC
        outb(0xA0, 0x20);
    }
    // Send EOI to master PIC
    outb(0x20, 0x20);

    // Send APIC EOI if APIC is enabled
    extern bool apic_enabled;
    if (apic_enabled)
    {
        extern void apic_send_eoi(void);
        apic_send_eoi();
    }
}
