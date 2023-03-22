#include "pwm_bl.h"

#include "driver/ledc.h"
#include "soc/ledc_periph.h"
#include <stdio.h>
#include "esp_log.h"

#define TAG "pwm_backlight"

typedef struct {
    bool pwm_control; // true: LEDC is used, false: GPIO is used
    int index;        // Either GPIO or LEDC channel
} pwm_bl_t;
pwm_bl_t bl;

void pwm_bl_init(gpio_num_t pin, bool output_invert, bool pwm_control){
    if(!GPIO_IS_VALID_OUTPUT_GPIO(pin)){
        ESP_LOGW(TAG, "Invalid GPIO");
        return;
    }

    if(pwm_control){
        bl.pwm_control = true;
        bl.index = 0;
        const ledc_channel_config_t LCD_backlight_channel = {
            .gpio_num = pin,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = 0,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = 0,
            .duty = 0,
            .hpoint = 0
        };
        const ledc_timer_config_t LCD_backlight_timer = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .bit_num = LEDC_TIMER_10_BIT,
            .timer_num = 0,
            .freq_hz = 5000,
            .clk_cfg = LEDC_AUTO_CLK
        };
        ESP_ERROR_CHECK(ledc_timer_config(&LCD_backlight_timer));
        ESP_ERROR_CHECK(ledc_channel_config(&LCD_backlight_channel));
        gpio_matrix_out(pin, ledc_periph_signal[LEDC_LOW_SPEED_MODE].sig_out0_idx + 0, output_invert, 0);
    }else {
        bl.index = pin;
        gpio_pad_select_gpio(pin);
        ESP_ERROR_CHECK(gpio_set_direction(pin, GPIO_MODE_OUTPUT));
        gpio_matrix_out(pin, SIG_GPIO_OUT_IDX, output_invert, false);
    }
}


void pwm_bl_set(int brightness_percent){
    if(brightness_percent > 100) brightness_percent = 100;
    if(brightness_percent < 0) brightness_percent = 0;

    ESP_LOGI(TAG, "Setting LCD backlight: %d%%", brightness_percent);

    if(bl.pwm_control){
        uint32_t duty_cycle = (1023 * brightness_percent) / 100; // LEDC resolution set to 10bits, thus: 100% = 1023
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, bl.index, duty_cycle));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, bl.index));
    }else {
        ESP_ERROR_CHECK(gpio_set_level(bl.index, brightness_percent));
    }
}

void pwm_bl_deinit(void){
    if(bl.pwm_control){
        ledc_stop(LEDC_LOW_SPEED_MODE, bl.index, 0);
    }else{
        gpio_reset_pin(bl.index);
    }
}


#if 0
pwm_bl_handle_t pwm_bl_create(const pwm_bl_config_t *config){
    if(config == NULL) return NULL;
    if(!GPIO_IS_VALID_OUTPUT_GPIO(config->gpio_num)){
        ESP_LOGW(TAG, "Invalid GPIO");
        return NULL;
    }

    pwm_bl_t * bl_dev = calloc(1,sizeof(pwm_bl_t));
    if(bl_dev == NULL){
        ESP_LOGW(TAG, "calloc backlight dev filed");
        return NULL;
    }

    if(config->pwm_control){
        bl_dev->pwm_control = true;
        bl_dev->index = config->channel_idx;
        const ledc_channel_config_t LCD_backlight_channel = {
            .gpio_num = config->gpio_num,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = config->channel_idx,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = config->timer_idx,
            .duty = 0,
            .hpoint = 0
        };
        const ledc_timer_config_t LCD_backlight_timer = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .bit_num = LEDC_TIMER_10_BIT,
            .timer_num = config->timer_idx,
            .freq_hz = 5000,
            .clk_cfg = LEDC_AUTO_CLK};

        ESP_ERROR_CHECK(ledc_timer_config(&LCD_backlight_timer));
        ESP_ERROR_CHECK(ledc_channel_config(&LCD_backlight_channel));
        gpio_matrix_out(config->gpio_num, ledc_periph_signal[LEDC_LOW_SPEED_MODE].sig_out0_idx + config->channel_idx, config->output_invert, 0);
    }else {
        bl_dev->index = config->gpio_num;
        gpio_pad_select_gpio(config->gpio_num);
        ESP_ERROR_CHECK(gpio_set_direction(config->gpio_num, GPIO_MODE_OUTPUT));
        gpio_matrix_out(config->gpio_num, SIG_GPIO_OUT_IDX, config->output_invert, false);
    }

    return (pwm_bl_handle_t)bl_dev;
}

void pwm_bl_set(pwm_bl_handle_t bl, int brightness_percent){
    if(bl == NULL) return;
    if(brightness_percent > 100) brightness_percent = 100;
    if(brightness_percent < 0) brightness_percent = 0;

    pwm_bl_t *bl_dev = (pwm_bl_t *) bl;
    ESP_LOGI(TAG, "Setting LCD backlight: %d%%", brightness_percent);

    if(bl_dev->pwm_control){
        uint32_t duty_cycle = (1023 * brightness_percent) / 100; // LEDC resolution set to 10bits, thus: 100% = 1023
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, bl_dev->index, duty_cycle));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, bl_dev->index));
    }else {
        ESP_ERROR_CHECK(gpio_set_level(bl_dev->index, brightness_percent));
    }
}

void pwm_bl_delete(pwm_bl_handle_t bl) {
    if (bl == NULL) return;

    pwm_bl_t *bckl_dev = (pwm_bl_t *) bl;
    if (bckl_dev->pwm_control) {
        ledc_stop(LEDC_LOW_SPEED_MODE, bckl_dev->index, 0);
    } else {
        gpio_reset_pin(bckl_dev->index);
    }
    free (bl);
}
#endif
