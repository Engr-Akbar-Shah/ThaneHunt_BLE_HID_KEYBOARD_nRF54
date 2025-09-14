/*
Name : main

Description :
	Main entry point for the BLE HID keyboard application on Zephyr RTOS.
	This file coordinates initialization of GPIO buttons/LEDs, the HID
	service, and the Bluetooth stack. It optionally registers passkey
	authentication callbacks, starts the button handling thread, and
	manages system status indicators. The main loop provides LED activity
	feedback during advertising and simulates periodic battery service
	updates.

Date : 2025-09-14

Developer : Engineer Akbar Shah
*/

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "app_ble.h"
#include "app_button.h"
#include "app_hid.h"

#if CONFIG_IMU_LSM6DSO
#include "app_imu.h"
#endif

LOG_MODULE_REGISTER(MAIN);

int main(void)
{
	int err;

	LOG_INF("Starting BLE HIDS keyboard VERSION: [%s]\n\r", CONFIG_PROJECT_VERSION);

	init_user_buttons();

#if (CONFIG_ENABLE_PASS_KEY_AUTH)
	err = bt_register_auth_callbacks();
	if (err)
		return 0;
#endif
	hid_init();

	err = enable_bt();
	if (err)
		return 0;

	button_thread_start();

#if CONFIG_IMU_LSM6DSO
	err = imu_lsm6dso_init();
	if (err != 0)
		return 0;
#endif
	bool is_led_off = false;

	for (;;)
	{
		if (is_adv)
		{
			is_led_off = false;
			user_led_toggle();
		}
		else
		{
			if (!is_led_off)
			{
				is_led_off = true;
				user_led_turn_off();
			}
		}
#if CONFIG_IMU_LSM6DSO
		imu_readDisplay_raw_data();
#endif
		/* Battery level simulation */
		bas_notify();

		k_msleep(1000);
	}
}
