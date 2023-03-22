#ifndef _PWM_BL_H_
#define _PWM_BL_H_

#include <stdbool.h>
#include "driver/gpio.h"

/* typedef void * pwm_bl_handle_t;

typedef struct {
    bool pwm_control;
    bool output_invert;
    int gpio_num;
    
    int timer_idx;
    int channel_idx;
}pwm_bl_config_t;
 */

void pwm_bl_init(gpio_num_t pin, bool output_invert, bool pwm_control);
void pwm_bl_deinit(void);
void pwm_bl_set(int brightness_percent);

#if 0
pwm_bl_handle_t pwm_bl_create(const pwm_bl_config_t *config);

void pwm_bl_set(pwm_bl_handle_t bl, int brightness_percent);

void pwm_bl_delete(pwm_bl_handle_t bl);

#endif


#endif
