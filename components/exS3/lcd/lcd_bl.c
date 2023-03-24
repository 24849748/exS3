#include "lcd_bl.h"

#include "driver/ledc.h"
#include "soc/ledc_periph.h"
#include "exS3_conf.h"
#include "esp_log.h"

#define TAG "lcd_bl"


typedef struct{
    bool output_type;    // true: pwm; false: gpio
    int index;
} lcd_bl_t;

// typedef void * lcd_bl_handle_t;

void lcd_bl_init(gpio_num_t pin){
    if(!GPIO_IS_VALID_OUTPUT_GPIO(pin)){
        ESP_LOGE(TAG, "Invalid GPIO");
        return ;
    }

    #if (LCD_BL_MODE)
    const ledc_timer_config_t lcd_bl_timer = {
        .speed_mode = LCD_BL_PWM_SPEED_MODE,
        .bit_num = LCD_BL_PWM_DUTY_RES,
        .timer_num = LCD_BL_PWM_TIMER,
        .freq_hz = LCD_BL_PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };

    const ledc_channel_config_t ledc_bl_channel = {
        .gpio_num = pin, 
        .speed_mode = LCD_BL_PWM_SPEED_MODE,
        .channel = LCD_BL_PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LCD_BL_PWM_TIMER,
        .duty = 0,  // default duty; duty = ((2**duty_res)-1) * n%
        .hpoint = 0
    };

    ESP_ERROR_CHECK(ledc_timer_config(&lcd_bl_timer));
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_bl_channel));
    #else
    gpio_config_t conf = {
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL << pin),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&conf));
    gpio_set_level(pin, !LCD_BL_ACTIVE_LEVEL);
    #endif
}

void lcd_bl_deinit(void){
    #if (LCD_BL_MODE)
        ledc_stop(LEDC_LOW_SPEED_MODE, LCD_BL_PWM_CHANNEL, 0);
    #else
        gpio_reset_pin(LCD_PIN_BL);
    #endif
}

void lcd_bl_set(int brightness){
    #if (LCD_BL_MODE)
        if(brightness > 100) brightness = 100;
        if(brightness < 0) brightness = 0;
        ESP_LOGI(TAG, "Setting LCD backlight: %d%%", brightness);
        uint32_t duty_cycle = (1023 * brightness)/100;

        ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LCD_BL_PWM_CHANNEL, duty_cycle));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LCD_BL_PWM_CHANNEL));
    #else
        if(brightness > 1) brightness = 1;
        if(brightness < 0) brightness = 0;
        ESP_LOGI(TAG, "Setting LCD backlight: %d", brightness);
        gpio_set_level(LCD_PIN_BL, brightness);
    #endif
}