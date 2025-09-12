
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <bluetooth/services/hids.h>
#include <zephyr/logging/log.h>
#include <dk_buttons_and_leds.h>

#include "app_ble.h"
#include "app_button.h"
#include "app_hid.h"
#include "app_keycodes.h"

LOG_MODULE_REGISTER(APP_BUTTON, CONFIG_BUTTON_LOG_LEVEL);

#define USER_BUTTON_NODE DT_NODELABEL(button_0)
#define USER_BUTTON_PIN DT_GPIO_PIN(USER_BUTTON_NODE, gpios)

#define KEY_TEXT_MASK BIT(USER_BUTTON_PIN)
#define KEY_SHIFT_MASK DK_BTN2_MSK
#define KEY_ADV_MASK DK_BTN4_MSK

#define KEY_PAIRING_ACCEPT DK_BTN1_MSK
#define KEY_PAIRING_REJECT DK_BTN2_MSK

#define BUTTON_THREAD_STACK_SIZE 2048
#define BUTTON_THREAD_PRIO 5

#define BUTTON_DEBOUNCE_MS 10

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(USER_BUTTON_NODE, gpios);
static struct gpio_callback button_cb;

static struct k_thread button_thread_data;
static k_tid_t button_thread_tid;
K_THREAD_STACK_DEFINE(button_thread_stack, BUTTON_THREAD_STACK_SIZE);

K_MSGQ_DEFINE(Button_queue, sizeof(bool), 16, 4);

static void button_work_handler(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(button_work, button_work_handler);

uint32_t BUTTON_PIN = 0;

void num_comp_reply(bool accept)
{
	struct pairing_data_mitm pairing_data;
	struct bt_conn *conn;

	if (k_msgq_get(&mitm_queue, &pairing_data, K_NO_WAIT) != 0)
	{
		return;
	}

	conn = pairing_data.conn;
#if (CONFIG_ENABLE_PASS_KEY_AUTH)
	if (accept)
	{
		bt_conn_auth_passkey_confirm(conn);
		printk("Numeric Match, conn %p\n", conn);
	}
	else
	{
		bt_conn_auth_cancel(conn);
		printk("Numeric Reject, conn %p\n", conn);
	}
#endif
	bt_conn_unref(pairing_data.conn);

	if (k_msgq_num_used_get(&mitm_queue))
	{
		k_work_submit(&pairing_work);
	}
}

static void button_text_changed(bool down, uint8_t *chr)
{
	if (down)
	{
		(void)hid_buttons_press(chr, 1);
	}
	else
	{
		(void)hid_buttons_release(chr, 1);
	}
}

static void button_shift_changed(bool down)
{
	static const uint8_t shift_key = HID_KEY_LSHIFT;

	if (down)
	{
		(void)hid_buttons_press(&shift_key, 1);
	}
	else
	{
		(void)hid_buttons_release(&shift_key, 1);
	}
}

static void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(cb);
	BUTTON_PIN = pins;
	k_work_reschedule(&button_work, K_MSEC(BUTTON_DEBOUNCE_MS));
}

static void button_work_handler(struct k_work *work)
{
	bool gp_state = gpio_pin_get_dt(&button);
	(void)k_msgq_put(&Button_queue, &gp_state, K_NO_WAIT);
}

static void button_thread_fn(void *p1, void *p2, void *p3)
{
	bool ev;

	for (;;)
	{
		k_msgq_get(&Button_queue, &ev, K_FOREVER);

		if (BUTTON_PIN & KEY_TEXT_MASK)
		{
			static uint8_t key = HID_KEY_H;
			button_text_changed(ev, &key);
		}
		else if (BUTTON_PIN & KEY_SHIFT_MASK)
		{
			button_shift_changed(ev);
		}
	}
}

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

void configure_gpio(void)
{
	int ret;

	if (!device_is_ready(button.port))
	{
		printk("Error: button device not ready\n");
		return;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret)
	{
		printk("Error %d: failed to configure button pin\n", ret);
		return;
	}

	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH);
	if (ret)
	{
		printk("Error %d: failed to configure interrupt\n", ret);
		return;
	}

	gpio_init_callback(&button_cb, button_isr, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb);

	/* LEDs init (unchanged) */
	int err = dk_leds_init();
	if (err)
	{
		printk("Cannot init LEDs (err: %d)\n", err);
	}

	/* Start the consumer thread */
	button_thread_start();

	printk("Button configured: port=%s pin=%u active_%s, debounce=%dms\n",
		   button.port->name, button.pin,
		   (button.dt_flags & GPIO_ACTIVE_LOW) ? "low" : "high",
		   BUTTON_DEBOUNCE_MS);
}