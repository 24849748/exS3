#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "esp_log.h"

#define TAG "led"

static TimerHandle_t led_timer = NULL;

static void led_timer_cb(TimerHandle_t xTimer){
    led_blink(PIN_LED);
}


/**
 * @brief led 初始化
 * 
 * @param pin 
 * @param level 默认电平
 * @return esp_err_t 
 */
esp_err_t led_init(gpio_num_t pin, uint32_t default_level){
    gpio_config_t conf = {
        .intr_type = GPIO_PIN_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pin_bit_mask = (1ULL << pin),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    esp_err_t ret = gpio_config(&conf);
    if(ret != ESP_OK){
        ESP_LOGE(TAG, "led configurate failed!");
        return ESP_FAIL;
    }

    ret = gpio_set_level(pin, default_level);
    if(ret!=ESP_OK){
        ESP_LOGE(TAG, "led set failed!");
        return ESP_FAIL;
    }

    led_timer = xTimerCreate("led_blink", (pdMS_TO_TICKS(1500)), pdTRUE, NULL, led_timer_cb);

    return ESP_OK;
}

/**
 * @brief LED闪一次
 * 
 * @param pin 
 */
void led_blink(gpio_num_t pin){
    uint8_t level = gpio_get_level(pin);
    gpio_set_level(pin, !level);
}

void led_set(gpio_num_t pin, uint32_t level){
    gpio_set_level(pin, level);
}


void led_on(gpio_num_t pin){
    gpio_set_level(pin, 1);
}
void led_off(gpio_num_t pin){
    gpio_set_level(pin, 0);
}


void led_startBlink(void){
    xTimerStart(led_timer, 0);
}

void led_endBlink(void){
    xTimerStop(led_timer, 0);
    led_off(PIN_LED);
}