#ifndef _LED_H_
#define _LED_H_

#include "driver/gpio.h"

esp_err_t led_init(gpio_num_t pin, uint32_t level);

void led_on(void);
void led_off(void);
void led_set( uint32_t level);
void led_blink(void);

void led_startBlink(void);
void led_endBlink(void);


#endif
