/*
Name : app_hid

Description : 
    This module implements the Human Interface Device (HID) over GATT 
    service for Bluetooth Low Energy (BLE) using Zephyr RTOS. It manages 
    keyboard state, key press/release reporting, HID report map 
    initialization, protocol mode events, and output report handling 
    (e.g., Caps Lock). It works alongside the BLE module to enable full 
    HID keyboard functionality.

Date : 2025-09-14

Developer : Engineer Akbar Shah
*/

#ifndef APP_HID_H
#define APP_HID_H

struct keyboard_state;

void hid_init(void);
int connect_bt_hid(struct bt_conn *conn);
int disconnect_bt_hid(struct bt_conn *conn);
int key_report_con_send(const struct keyboard_state *state,
						bool boot_mode,
						struct bt_conn *conn);
int hid_buttons_release(const uint8_t *keys, size_t cnt);
int hid_buttons_press(const uint8_t *keys, size_t cnt);

#endif // APP_HID_H
