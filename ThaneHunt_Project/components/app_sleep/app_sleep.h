/*
Name : app_sleep

Description :
    Power management helper module for the BLE HID application on Zephyr RTOS.
    Provides an idle timer and work handler to disconnect BLE and transition
    the system into deep sleep (system-off) after inactivity. Also exposes
    utility functions to start and reset the idle timer.

Date : 2025-09-14

Developer : Engineer Akbar Shah
*/

#ifndef APP_SLEEP_H
#define APP_SLEEP_H

void PRINT_RESET_CAUSE();

void start_idle_timer(void);
void reset_idle_timer(void);

#endif // APP_SLEEP_H
