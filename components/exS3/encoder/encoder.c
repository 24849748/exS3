#include "encoder.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
// #include "freertos/queue.h"

#include "driver/pcnt.h"
#include "rot_enc.h"
// #include "encoder_button.h"
#include "button.h"
#include "lcd_bl.h"
#include "led.h"
#include "ft6236.h"

#include "exS3_conf.h"
#include "sdkconfig.h"      

#include "esp_log.h"
#define TAG "encoder"

#define ECD_DEBUG 1                 //开启debug 输出

static int32_t last_count;          /* 最后一次计数 */
static int32_t encoder_diff = 0;    /* 编码器转动方向 1-cw, -1-ccw, 0-default */

uint8_t last_brightness = 0;         // 避免重复设置亮度
uint8_t brightness = 50;             // 亮度

encoder_handle_t ecd;               /* encoder 设备句柄 */

void encoder_open_lcd(void){
    lcd_bl_set(brightness);
    ft6236_enable_read();
    encoder_start(ecd);
    led_endBlink();
}
void encoder_close_lcd(void){
    encoder_stop(ecd);
    ft6236_disable_read();
    lcd_bl_set(0);
    led_startBlink();
}

void encoder_init(void){    
    //编码器按键初始化
    button_init(encoder_open_lcd, encoder_close_lcd);

    //创建编码器句柄
    // ecd = encoder_create(ECD_PCNT_UNIT_NUM, PIN_ECD_A, PIN_ECD_B);
    ecd = encoder_create(ECD_PCNT_UNIT_NUM, ECD_PIN_A, ECD_PIN_B);
    
    //设置去抖值，1us
    encoder_set_glitch_fliter(ecd, 1);          //设置过滤
    encoder_start(ecd);         //开启encoder pcnt

    last_count = encoder_get_counter_value(ecd);    //获取初值

    lcd_bl_init(LCD_PIN_BL);
    lcd_bl_set(brightness);
}


void encoder_task(void *pvParameter){
    // ecd = (encoder_handle_t)pvParameter;
    TaskHandle_t GuiTaskHandle = (TaskHandle_t)pvParameter;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));

        button_scan();

        // int32_t dir = 0;
        int32_t count = encoder_get_counter_value(ecd);
        // ESP_LOGI(TAG, "count:%d",count);
        // if(count > 100) count = 100;
        // if(count < 0) count = 0;

        if(count - last_count < 0){
            encoder_diff = -1;
            brightness -= 5;
        }else if(count - last_count > 0){
            encoder_diff = 1;
            brightness += 5;
        }else if(count == last_count){
            encoder_diff = 0;
        }
        last_count = count;
        // ESP_LOGI(TAG, "diff:%d", encoder_diff);

        if(brightness > 100) brightness = 100;
        if(brightness < 5) brightness = 5;

        if(brightness != last_brightness){
            lcd_bl_set(brightness);
            last_brightness = brightness;
            xTaskNotify(GuiTaskHandle, (uint32_t)brightness, eSetValueWithOverwrite);
        }
    }
    // encoder_delete(ecd);
}







/**
 * @todo 
 *      使用nvs存储亮度（或没必要）
 * 
 */