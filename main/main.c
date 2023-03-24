#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


// #include "i2c_mid.h"
#include "i2c_bus.h"
#include "spi_bus.h"
#include "led.h"
#include "axp173.h"
#include "motor.h"
#include "encoder.h"
#include "lv_task.h"

#include "exS3_conf.h"

#include "esp_log.h"
#define TAG "main"

TaskHandle_t GuiTaskHandle;

void app_main(void)
{   
    i2c_bus_init(I2C_PORT);
    spi_bus_init(SPI_HOST_ID);
    
    axp_init();

    led_init(LED_PIN, 0);
    motor_init(MOTOR_PIN, 0);
    
    encoder_init();
    
    // lv_create_task();    
    xTaskCreatePinnedToCore(guiTask, "gui", 4096 * 2, NULL, 0, &GuiTaskHandle, 1);
    xTaskCreatePinnedToCore(encoder_task, "encoderTask", 4096, (void *)GuiTaskHandle, 4, NULL, 1);
    
    motor_click(200);
    // motor_on(MOTOR_PIN);
    // vTaskDelay(pdMS_TO_TICKS(200));
    // motor_off(MOTOR_PIN);

    // rtos_debug();
}


/**
 *  WiFi ：wifi:state wrong txa_flags=9
 *  ntp get不到正确时间
 */


/** 启动流程 
 *  1.初始化总线（spi、i2c）
 *  2.初始化GPIO（led、motor、button、encoder）
 *  3.初始化存储设备（nvs）
 *  4.连接wifi
 *  5.初始化 lv_port_disp、lv_port_indev（屏幕、输入设备）
 *  6.lvgl_init
 *  7.创建lvgl任务
 *  8.创建其他任务
 *  9.
 *  10.
 */


/**
 * @todo
 *      sd卡 相册
 *      开机加载界面
 *      屏幕亮度调节显示组件
 */


/* 快速运行IDF环境命令
idf.py -p /dev/cu.usbserial-1420 flash monitor
*/