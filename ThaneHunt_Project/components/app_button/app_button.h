#ifndef APP_BUTTON_H
#define APP_BUTTON_H

int init_user_led(void);
void user_led_turn_on(void);
void user_led_turn_off(void);
void user_led_toggle(void);

void button_thread_start(void);

void configure_gpio(void);

#endif // APP_BUTTON_H
