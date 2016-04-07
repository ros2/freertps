#ifndef LED_H
#define LED_H

// todo: extend this for multiple LEDs as well, maybe by using plurals, like
// leds_on(uint32_t mask), etc. etc.

void led_init(void);
void led_on(void);
void led_off(void);
void led_toggle(void);

#endif

