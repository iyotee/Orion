/*
 * Orion Operating System - Time Functions Header
 *
 * Time-related functions for the Orion kernel.
 * Provides basic time operations without libc dependency.
 *
 * Copyright (c) 2024-2025 Orion OS Project
 * License: MIT
 */

#ifndef ORION_TIME_H
#define ORION_TIME_H

#include <orion/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Time structures
typedef struct {
    uint64_t seconds;
    uint32_t nanoseconds;
} orion_time_t;

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} orion_datetime_t;

// Time functions
uint64_t time_get_uptime(void);
uint64_t time_get_timestamp(void);
void time_get_datetime(orion_datetime_t *dt);

// Delay functions
void time_delay_ms(uint32_t milliseconds);
void time_delay_us(uint32_t microseconds);

// Timer functions
typedef void (*timer_callback_t)(void *data);
typedef struct timer timer_t;

timer_t *timer_create(uint64_t interval_ms, timer_callback_t callback, void *data);
void timer_destroy(timer_t *timer);
void timer_start(timer_t *timer);
void timer_stop(timer_t *timer);

#ifdef __cplusplus
}
#endif

#endif // ORION_TIME_H
