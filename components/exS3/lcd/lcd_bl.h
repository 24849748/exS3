#ifndef _LCD_BL_H_
#define _LCD_BL_H_

#include "driver/gpio.h"

void lcd_bl_init(gpio_num_t pin);
void lcd_bl_deinit(void);
void lcd_bl_set(int brightness);

#endif