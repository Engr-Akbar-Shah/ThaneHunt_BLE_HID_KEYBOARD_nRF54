/*
Name : app_button

Description : 
    Button/LED and idle-power management for a BLE HID device on Zephyr RTOS.
    Configures a GPIO button with interrupt + debounced work queue, maps button
    events to HID key reports, manages an activity timer that disconnects BLE
    and enters system-off, and controls a user LED.

Date : 2025-09-14

Developer : Engineer Akbar Shah
*/


#ifndef APP_BUTTON_H
#define APP_BUTTON_H

int read_latch_register(void);

int init_user_led(void);
void user_led_turn_on(void);
void user_led_turn_off(void);
void user_led_toggle(void);
void button_thread_start(void);
void init_user_buttons(void);

#endif // APP_BUTTON_H
