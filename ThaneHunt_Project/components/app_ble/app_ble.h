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

struct pairing_data_mitm
{
	struct bt_conn *conn;
	unsigned int passkey;
};

struct conn_mode
{
	struct bt_conn *conn;
	bool in_boot_mode;
};

extern struct conn_mode conn_mode[CONFIG_BT_HIDS_MAX_CLIENT_COUNT];

extern struct k_msgq mitm_queue;
extern struct k_work pairing_work;
extern volatile bool is_adv;

void advertising_start(void);
void pairing_process();

void connected(struct bt_conn *conn, uint8_t err);
void disconnected(struct bt_conn *conn, uint8_t reason);
int enable_bt(void);
void bas_notify(void);

#if (CONFIG_ENABLE_PASS_KEY_AUTH)
int bt_register_auth_callbacks(void);
#endif

#endif // APP_BLE_H
