#ifndef _MOTOR_H_
#define _MOTOR_H_


#include "driver/gpio.h"


#define DEFAULT_MOTOR_CLICK_WORKTIME 80     // 点击操作马达震动时长 ms

esp_err_t motor_init(gpio_num_t pin, uint32_t level);

void motor_on(void);
void motor_off(void);
void motor_reverse(void);
void motor_set(uint32_t level);

void motor_click(uint32_t motorWorkTime);

#endif
