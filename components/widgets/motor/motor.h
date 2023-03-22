#ifndef _MOTOR_H_
#define _MOTOR_H_


#include "driver/gpio.h"
#include "widgets_conf.h"


#define DEFAULT_MOTOR_CLICK_WORKTIME 80     // 点击操作马达震动时长 ms

esp_err_t motor_init(gpio_num_t pin, uint32_t level);

void motor_on(gpio_num_t pin);
void motor_off(gpio_num_t pin);
void motor_reverse(gpio_num_t pin);
void motor_set(gpio_num_t pin, uint32_t level);

void motor_click(uint32_t motorWorkTime);

#endif
