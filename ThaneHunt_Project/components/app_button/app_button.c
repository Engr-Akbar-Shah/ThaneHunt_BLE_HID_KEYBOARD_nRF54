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

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <bluetooth/services/hids.h>
#include <zephyr/logging/log.h>

#include "app_ble.h"
#include "app_button.h"
#include "app_hid.h"
#include "app_keycodes.h"
#include "app_sleep.h"

LOG_MODULE_REGISTER(APP_BUTTON);

#define USER_LED_NODE DT_NODELABEL(led_0)
#define USER_BUTTON_NODE DT_NODELABEL(button_0)
#define USER_BUTTON_PIN DT_GPIO_PIN(USER_BUTTON_NODE, gpios)
#define KEY_TEXT_MASK BIT(USER_BUTTON_PIN)
#define BUTTON_THREAD_STACK_SIZE 2048
#define BUTTON_THREAD_PRIO 0
#define BUTTON_DEBOUNCE_MS 10

static struct gpio_callback button_cb;

static volatile uint32_t BUTTON_PIN;
static volatile bool LATCH_RESET_BUTTON;

static k_tid_t button_thread_tid;
static struct k_thread button_thread_data;

static void button_work_handler(struct k_work *work);

K_MSGQ_DEFINE(Button_queue, sizeof(bool), 16, 4);
K_WORK_DELAYABLE_DEFINE(button_work, button_work_handler);
K_THREAD_STACK_DEFINE(button_thread_stack, BUTTON_THREAD_STACK_SIZE);

static const struct gpio_dt_spec user_led = GPIO_DT_SPEC_GET(USER_LED_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(USER_BUTTON_NODE, gpios);

/*
Function : button_text_changed

Description :
	Sends a single HID key press or release based on the button state.

Parameter :
	down : true for press, false for release
	chr  : Pointer to the HID key code to send

Return :
	void

Example Call :
	uint8_t key = HID_KEY_H;
	button_text_changed(true, &key);
*/
static void button_text_changed(bool down, uint8_t *chr)
{
	LOG_DBG("Button %s: sending HID key 0x%02X", down ? "pressed" : "released", *chr);
	if (down)
	{
		(void)hid_buttons_press(chr, 1);
	}
	else
	{
		(void)hid_buttons_release(chr, 1);
	}
}

/*
Function : onButton_reset_send_spaceBar

Description :
	Sends a quick SPACE key tap (press + release). Used after a wakeup latch
	indicates the button triggered system resume.

Parameter :
	None

Return :
	void

Example Call :
	onButton_reset_send_spaceBar();
*/
static void onButton_reset_send_spaceBar(void)
{
	static const uint8_t key = HID_KEY_SPACE;
	(void)hid_buttons_press(&key, 1);
	(void)hid_buttons_release(&key, 1);
	LOG_DBG("Sent SPACE key tap after wakeup");
}

/*
Function : button_isr

Description :
	GPIO interrupt service routine for the button. Captures the pin bitmap,
	then reschedules a debounced work handler.

Parameter :
	dev  : GPIO device pointer (unused)
	cb   : Callback structure pointer (unused)
	pins : Bitmask of pins that triggered the interrupt

Return :
	void

Example Call :
	egistered via gpio_init_callback(...)
*/
static void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(cb);
	BUTTON_PIN = pins;
	k_work_reschedule(&button_work, K_MSEC(BUTTON_DEBOUNCE_MS));
}

/*
Function : button_work_handler

Description :
	Debounce work handler. Reads the current button GPIO level and enqueues
	it to the Button_queue for the consumer thread.

Parameter :
	work : Pointer to the work item

Return :
	void

Example Call :
	scheduled by button_isr via k_work_reschedule(...)
*/
static void button_work_handler(struct k_work *work)
{
	bool gp_state = gpio_pin_get_dt(&button);
	(void)k_msgq_put(&Button_queue, &gp_state, K_NO_WAIT);
}

/*
Function : init_user_led

Description :
	Initializes the user LED GPIO and configures it as output.

Parameter :
	None

Return :
	int : 0 on success, negative errno on failure

Example Call :
	int err = init_user_led();
*/
int init_user_led(void)
{
	int err;
	if (!device_is_ready(user_led.port))
	{
		return -ENODEV;
	}

	err = gpio_pin_configure_dt(&user_led, GPIO_OUTPUT);
	if (err)
	{
		LOG_ERR("Error %d: failed to configure LED pin\n", err);
		return err;
	}
	return err;
}

/*
Function : user_led_turn_on

Description :
	Sets the user LED pin high (turns LED on).

Parameter :
	None

Return :
	void

Example Call :
	user_led_turn_on();
*/
void user_led_turn_on(void)
{
	if (!gpio_pin_set_dt(&user_led, 1))
	{
		LOG_INF("User LED Turn ON\n\r");
	}
}

/*
Function : user_led_turn_off

Description :
	Sets the user LED pin low (turns LED off).

Parameter :
	None

Return :
	void

Example Call :
	user_led_turn_off();
*/
void user_led_turn_off(void)
{
	if (!gpio_pin_set_dt(&user_led, 0))
	{
		LOG_INF("User LED Turn OFF\n\r");
	}
}

/*
Function : user_led_toggle

Description :
	Toggles the user LED pin state.

Parameter :
	None

Return :
	void

Example Call :
	user_led_toggle();
*/
void user_led_toggle(void)
{
	gpio_pin_toggle_dt(&user_led);
	LOG_DBG("User LED Toggled\n\r");
}

/*
Function : init_user_buttons

Description :
	Configures the button GPIO as input with interrupts on both edges, installs
	the ISR callback, initializes the user LED, and starts the button consumer
	thread. Logs configuration details.

Parameter :
	None

Return :
	void

Example Call :
	init_user_buttons();
*/
void init_user_buttons(void)
{
	int ret;

	if (!device_is_ready(button.port))
	{
		LOG_ERR("Error: button device not ready\n");
		return;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret)
	{
		LOG_ERR("Error %d: failed to configure button pin\n", ret);
		return;
	}

	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH);
	if (ret)
	{
		LOG_ERR("Error %d: failed to configure interrupt\n", ret);
		return;
	}

	gpio_init_callback(&button_cb, button_isr, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb);

	/* LEDs init (unchanged) */
	int err = init_user_led();
	if (err)
	{
		LOG_ERR("Cannot init LEDs (err: %d)\n", err);
	}

	/* Start the consumer thread */
	button_thread_start();

	LOG_DBG("Button configured: port=%s pin=%u active_%s, debounce=%dms\n",
			button.port->name, button.pin,
			(button.dt_flags & GPIO_ACTIVE_LOW) ? "low" : "high",
			BUTTON_DEBOUNCE_MS);
}

/*
Function : button_thread_fn

Description :
	Button consumer thread. Starts an idle timer, waits for BLE connection,
	optionally sends a SPACE tap if the wake latch indicates button resume,
	then processes debounced button events from the queue. For each event it
	restarts the idle timer and sends the mapped HID key press/release.

Parameter :
	p1 : Unused (NULL expected)
	p2 : Unused (NULL expected)
	p3 : Unused (NULL expected)

Return :
	void

Example Call :
	spawned by button_thread_start()
*/
static void button_thread_fn(void *p1, void *p2, void *p3)
{
	bool ev;

	start_idle_timer();
	while (isBle_connected == false)
	{
		k_sleep(K_MSEC(100));
	}
	if (LATCH_RESET_BUTTON)
	{
		LATCH_RESET_BUTTON = false;
		onButton_reset_send_spaceBar();
	}

	for (;;)
	{
		k_msgq_get(&Button_queue, &ev, K_FOREVER);
		if (isBle_connected == false)
		{
			continue; // ignore button presses when not connected
		}
		/* Any activity -> restart idle timer */
		reset_idle_timer();

		if (BUTTON_PIN & KEY_TEXT_MASK)
		{
			static uint8_t key = HID_KEY_H;
			button_text_changed(ev, &key);
		}
	}
}

/*
Function : button_thread_start

Description :
	Creates and starts the button thread if not already running, sets its
	stack/priority, and assigns a debug-friendly thread name.

Parameter :
	None

Return :
	void

Example Call :
	button_thread_start();
*/
void button_thread_start(void)
{
	if (button_thread_tid)
	{
		return; /* already started */
	}

	button_thread_tid = k_thread_create(
		&button_thread_data,
		button_thread_stack,
		K_THREAD_STACK_SIZEOF(button_thread_stack),
		button_thread_fn,
		NULL, NULL, NULL,
		BUTTON_THREAD_PRIO, 0, K_NO_WAIT);

	k_thread_name_set(button_thread_tid, "btn_thread");
}

/*
Function : read_latch_register

Description :
	Reads and logs the GPIO LATCH register to determine if the button caused
	the wakeup from system off. Sets LATCH_RESET_BUTTON if the button bit is
	found, and clears the corresponding latch flags.

Parameter :
	None

Return :
	int : 0 always

Example Call :
	read_latch_register();
*/
int read_latch_register(void)
{
	uint32_t lat = NRF_P0->LATCH; /* snapshot */
	if (lat)
	{
		if (lat & BIT(USER_BUTTON_PIN))
		{
			LATCH_RESET_BUTTON = true;
			LOG_INF("Button (P1.%u) was the wakeup source", USER_BUTTON_PIN);
		}
		/* Clear only the bits that were set */
		NRF_P0->LATCH = lat; /* write-1-to-clear */
	}
	/* If your board also uses P1, read/clear NRF_P1->LATCH similarly. */
	return 0;
}

/*
SYS_INIT(read_latch_register, PRE_KERNEL_1, 0);

Description :
	Registers the read_latch_register function to run automatically at system
	startup during the PRE_KERNEL_1 initialization stage with priority 0.
	This ensures the GPIO latch state is checked and cleared early in the
	boot process, before most kernel services are started.
*/
SYS_INIT(read_latch_register, APPLICATION, 0);