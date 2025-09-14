/*
Name : app_ble

Description : This module implements Bluetooth Low Energy (BLE) functionality for
			  HID-enabled devices using Zephyr RTOS. It handles BLE initialization,
			  advertising, connection management, authentication, and Battery Service
			  notifications to support HID profiles such as keyboards or mice.

Date : 2025-09-14

Developer : Engineer Akbar Shah
*/

#ifndef APP_BLE_H
#define APP_BLE_H

#define KEY_PRESS_MAX 6 /* Maximum number of non-control keys \
						 * pressed simultaneously             \
						 */
/* Number of bytes in key report
 *
 * 1B - control keys
 * 1B - reserved
 * rest - non-control keys
 */
#define INPUT_REPORT_KEYS_MAX_LEN (1 + 1 + KEY_PRESS_MAX)

#define OUTPUT_REPORT_MAX_LEN 1
#define SCAN_CODE_POS 2
#define KEYS_MAX_LEN (INPUT_REPORT_KEYS_MAX_LEN - \
					  SCAN_CODE_POS)

struct conn_mode
{
	struct bt_conn *conn;
	bool in_boot_mode;
};

extern struct conn_mode conn_mode[CONFIG_BT_HIDS_MAX_CLIENT_COUNT];

extern volatile bool is_adv;
extern volatile bool isBle_connected;

void connected(struct bt_conn *conn, uint8_t err);
void disconnected(struct bt_conn *conn, uint8_t reason);
int enable_bt(void);
void bas_notify(void);
int ble_disconnect_safe(void);

#if (CONFIG_ENABLE_PASS_KEY_AUTH)
int bt_register_auth_callbacks(void);
#endif

#endif // APP_BLE_H
