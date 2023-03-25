#include "lv_task.h"

// #include <stdbool.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "esp_freertos_hooks.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_log.h"

#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"

#include "encoder.h"

/* GUI page */
#include "main_page.h"
#include "fan_page.h"
#include "ac_page.h"
#include "info_page.h"
#include "ctrl_page.h"
#include "start_page.h"
#include "lv_common.h"
#include "notify_bar.h"

#define TAG "lv_task"
#define LV_TICK_PERIOD_MS 1

TaskHandle_t GuiTaskHandle;
SemaphoreHandle_t xGuiMutex;

static void lv_tick_task(void *arg){
    (void)arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

void guiTask(void *pvParameter){
    (void)pvParameter;
    xGuiMutex = xSemaphoreCreateMutex();
    //lvgl init
    lv_init();

    //lvgl display init
    lv_port_disp_init();
    
    //lvgl indev init
    lv_port_indev_init();


    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui",
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));
    
    /* ========== GUI ============ */
    //init
    bg_page();
    init_custom_btn_style();

    //create obj
    main_page_create_obj();
    info_page_create_obj();
    ac_page_create_obj();
    fan_page_create_obj();
    ctrl_page_create_obj();
    create_brightnessBar();
    
    create_notify_timer();

    /* 开启屏幕背光并进入开机界面 */
    encoder_open_lcd();
    start_page();
    /* ============================ */

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        if(pdTRUE == xSemaphoreTake(xGuiMutex, portMAX_DELAY)){
            lv_task_handler();
            xSemaphoreGive(xGuiMutex);
        }
    }
    vTaskDelete(NULL);
}
