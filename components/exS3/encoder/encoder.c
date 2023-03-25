/**
 * @file 使用新的encoder
 * @author Bosco's git (1270112821@qq.com)
 * @brief lvgl的bar组件没有key特性，实现encoder调节屏幕亮度的思路比较复杂，再者还有
 *      触屏输入，encoder输入的必要性不大，因此单独创建一个任务来实现现有功能。
 *      encoder作为LCD屏幕调节组件，需要实现调节LCD屏幕背光，长短按实现“锁屏/解锁”功能，
 *      需要 or 被需要的模块如下：
 *          - lcd_bl        : 
 *          - notify_bar    : 
 *          - ft6236        : 关闭触屏输入
 *          - led           : 区别锁屏与关机 
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "encoder.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "exS3_conf.h"

#include "ecd.h"
#include "button.h"
#include "lcd_bl.h"
#include "ft6236.h"
#include "led.h"

int brightness = 0;    // todo: 将默认亮度存储到nvs里, init时从nvs读取
int lastbrightness = 0;
bool encoder_read_enable;
ecd_t * ecd;

static void btn_open_lcd_cb(void){
    lcd_bl_set(brightness);
    ft6236_enable_read();
    encoder_read_enable = true;
    led_endBlink();
}

static void btn_close_lcd_cb(void){
    encoder_read_enable = false;
    ft6236_disable_read();
    lcd_bl_set(0);
    led_startBlink();
}

void encoder_open_lcd(void){
    lcd_bl_set(brightness);
}

void encoder_init(void){
    ecd = ecd_create(ECD_PIN_A, ECD_PIN_B, false, true);

    button_init(btn_open_lcd_cb, btn_close_lcd_cb);

    brightness = LCD_DEFAULT_BRIGHTNESS;
    lastbrightness = brightness;
    encoder_read_enable = true;

    // 初始化并关闭背光，在lv_task创建好obj后开启
    lcd_bl_init(LCD_PIN_BL);
    lcd_bl_set(0);
}

void encoder_task(void *pvParameter){
    TaskHandle_t GuiTaskHandle = (TaskHandle_t)pvParameter;
    // ecd_t * ecd = (ecd_t *)pvParameter;

    int8_t dir;
    while (1)
    {
        // ecd_btn_scan();
        button_scan();
        // 接受encoder的方向数据
        if(xQueueReceive(ecd->queue, &dir, pdMS_TO_TICKS(200)) == pdTRUE)
        {
            if(dir == CW && encoder_read_enable){
                brightness+=5;
                if(brightness > 100) brightness = 100;
            }else if(dir == CCW && encoder_read_enable){
                brightness-=5;
                if(brightness < 5) brightness = 5;
            }
        }
        // 如果brightness有变化，设置屏幕亮度，并更新lastbrightness，并发送给notify_bar
        if (brightness != lastbrightness)
        {
            lcd_bl_set(brightness);
            lastbrightness = brightness;
            xTaskNotify(GuiTaskHandle, (uint32_t)brightness, eSetValueWithoutOverwrite);
        }

        /* button事件处理 */
        // 已在button的回调函数里实现
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
}