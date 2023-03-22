#include "info_page.h"
#include "main_page.h"
#include "anim.h"
#include "lv_common.h"

#include "axp173.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "motor.h"

#include "esp_log.h"

#define TAG "info_page"

/* 坐标信息,对齐左上角LV_ALIGN_TOP_LEFT */
#define INFO_RETURN_Y       15   

#define INFO_LINE_X         25     //info line x坐标
#define FirstLineINFO_Y     60     //第一行 info y坐标
#define LastLineINFO_Y      FirstLineINFO_Y + 130 + 20    //最后一行 info y坐标

static lv_obj_t * info_return;      //左上角返回键
static lv_obj_t * info_line;        //信息栏左边

static lv_obj_t * axp_info;         //axp173相关信息
static lv_obj_t * axp_info_power;
static lv_obj_t * axp_info_percentage;         //axp173相关信息
static lv_obj_t * axp_info_volt;         //axp173相关信息

static lv_obj_t * sys_info;         //系统相关信息
static lv_obj_t * sys_info_flash;
static lv_obj_t * sys_info_heap;

static lv_timer_t * info_timer;

// static uint32_t info_power,info_bat,info_flash,info_heap;
// static float info_volt;



/* 按钮回调函数 */
static void return_mainpage_cb(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "return main page");
        info_page_anim_out(200);
        main_page_anim_in(330);
    }
}



/*************************
 *      绘制页面内容
 *************************/

/* info 页面返回按键 */
void create_info_return_btn(void){
    /* 返回键 */
    info_return = create_return_button(return_mainpage_cb);
}

/* info左边线条 */
void create_info_line(void){

    static lv_point_t points[] = { 
        {INFO_LINE_X, FirstLineINFO_Y},     //line起点
        {INFO_LINE_X, LastLineINFO_Y} };    //line终点
    static lv_style_t styleline;
    lv_style_init(&styleline);
    lv_style_set_line_color(&styleline, lv_color_white());
    lv_style_set_line_width(&styleline, 3);

    info_line = lv_line_create(bg_screen);
    lv_line_set_points(info_line, points, 2);
    lv_obj_add_style(info_line, &styleline, 0);
    lv_obj_align(info_line, LV_ALIGN_TOP_LEFT, 0, 0);
}

/**
 * @brief 快速创建一个info label对象，并设置文字内容、对齐左上角，内容需要重新设定
 */
lv_obj_t * lv_create_info_label(lv_obj_t *parent, lv_coord_t y){
    lv_obj_t * obj = lv_label_create(parent);
    // lv_label_set_text(obj, text);
    lv_label_set_recolor(obj,true);
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, INFO_LINE_X+10, y);
    lv_obj_set_style_text_color(obj, lv_color_white(), 0);
    return obj;
}

/**
 * @brief 更新一次text info
 * 
 */
void info_axp_update(void){
    float bat_volt, inter_temp, vbus_volt, vbus_current, charge_current, discharge_current;
    axp_info_t info;
    axp_read_info(&info);

    axp_read_adc_data(DATA_BAT_VOLT, &bat_volt);
    axp_read_adc_data(DATA_INTEL_TEMP, &inter_temp);

    axp_read_adc_data(DATA_VBUS_VOLT, &vbus_volt);
    axp_read_adc_data(DATA_VBUS_CURRENT, &vbus_current);

    axp_read_adc_data(DATA_BAT_CHARGE_CURRENT, &charge_current);
    axp_read_adc_data(DATA_BAT_DISCHARGE_CURRENT, &discharge_current);

    /* 充电指示 + 电压 */
    if(info.axp_over_temp == 0){
        if(info.charge_idct == 0){
            lv_label_set_text_fmt(axp_info, "AXP173 : %.2f V", bat_volt);  //无充电无过温
        }else if(info.charge_idct == 1){
            lv_label_set_text_fmt(axp_info, "#50ec7b AXP173# : %.2f V", bat_volt);  //绿色充电状态
        }
    }else if(info.axp_over_temp == 1){
        lv_label_set_text_fmt(axp_info, "#dd001b AXP173# : %.2f V", bat_volt);  //红色过温警告
    }

    lv_label_set_text_fmt(axp_info_power, "Temp : %.1f *C", inter_temp);


    // /* 电池百分比 */
    // int32_t bat_pct = axp_get_bat_pct();
    // if(bat_pct <= 20){
    //     lv_label_set_text_fmt(axp_info_percentage,LV_SYMBOL_BATTERY_EMPTY " %d %%", bat_pct);
    // }else if(bat_pct > 20 && bat_pct <= 40){
    //     lv_label_set_text_fmt(axp_info_percentage,LV_SYMBOL_BATTERY_1 " %d %%", bat_pct);
    // }else if(bat_pct > 40 && bat_pct <= 60){
    //     lv_label_set_text_fmt(axp_info_percentage,LV_SYMBOL_BATTERY_2 " %d %%", bat_pct);
    // }
    // else if(bat_pct > 60 && bat_pct <= 80){
    //     lv_label_set_text_fmt(axp_info_percentage,LV_SYMBOL_BATTERY_3 " %d %%", bat_pct);
    // }else if(bat_pct > 80 && bat_pct <= 100){
    //     lv_label_set_text_fmt(axp_info_percentage,LV_SYMBOL_BATTERY_FULL " %d %%", bat_pct);
    // }else{
    //     lv_label_set_text_fmt(axp_info_percentage, " %d %%", bat_pct);
    // }
    
    lv_label_set_text_fmt(axp_info_percentage, " %.2fV %.1fmA", vbus_volt, vbus_current);

    /* 电流 */
    lv_label_set_text_fmt(axp_info_volt, "#50ec7b InC:%.1fmA# OutC:%.1fmA", charge_current, discharge_current);
}

/**
 * @brief 创建 axp info 文字对象
 *      包括： 充电状态 电池电压/百分比 当前功率
 */
void create_axp_info(void){

    axp_info = lv_create_info_label(bg_screen, FirstLineINFO_Y);
    axp_info_power = lv_create_info_label(bg_screen, FirstLineINFO_Y+20);
    axp_info_percentage = lv_create_info_label(bg_screen, FirstLineINFO_Y+40);
    axp_info_volt = lv_create_info_label(bg_screen, FirstLineINFO_Y+60);
    
    // lv_label_set_text(axp_info, "#ffffff AXP173");
    // lv_label_set_text(axp_info_power, "power");
    // lv_label_set_text(axp_info_percentage, " %d %%");
    // lv_label_set_text(axp_info_volt, "Volt");

    
    info_axp_update();
}

/**
 * @brief 创建 sys info 文字对象
 *      包括： cpu型号  flash大小  剩余堆容量  mac地址
 */
void create_sys_info(void){
    // sys_info = lv_label_create(bg_screen);
    // lv_label_set_recolor(sys_info, true);
    // lv_label_set_text(sys_info, "#ffffff system : esp32S3\n#ffffff flash: 123 Mb\n#ffffff free heap size: 51 bytes");
    // lv_obj_align(sys_info, LV_ALIGN_TOP_LEFT, INFO_LINE_X + 10, 160);
    // info_flash = 512;
    // info_heap = 6162;

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    sys_info = lv_create_info_label(bg_screen, FirstLineINFO_Y+90);
    sys_info_flash = lv_create_info_label(bg_screen, FirstLineINFO_Y+110);
    sys_info_heap = lv_create_info_label(bg_screen, FirstLineINFO_Y+130);

    lv_label_set_text(sys_info, LV_SYMBOL_HOME " ESP32 S3");
    // lv_label_set_text_fmt(sys_info_flash, "#ffffff flash: %d mb", info_flash);
    // lv_label_set_text_fmt(sys_info_heap, "#ffffff free heap: %d bytes", esp_get_minimum_free_heap_size());
    lv_label_set_text_fmt(sys_info_flash, "Flash: %dMB", spi_flash_get_chip_size() / (1024 * 1024));
    lv_label_set_text_fmt(sys_info_heap, "Heap: %d bytes", esp_get_minimum_free_heap_size());
}


/* axp173、系统资源 定时器更新函数 */
void axp_sys_timer_update(lv_timer_t *timer){

    info_axp_update();
}

void create_axp_sys_timer(void){
    info_timer = lv_timer_create(axp_sys_timer_update, 1000, NULL);
}

void get_esp_sys_info(){
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
}


/*************************
 *        页面动画
 *************************/

/* info页面动画 */
void info_page_anim_in(uint32_t delay){
    anim_y_fade_in(info_return, -50, INFO_RETURN_Y, delay);
    anim_y_fade_in(info_line, -100, 0, delay);

    anim_x_fade_in(axp_info, -50, 40, delay+50);
    anim_x_fade_in(axp_info_power, -50, 40, delay+100);
    anim_x_fade_in(axp_info_percentage, -50, 40, delay+150);
    anim_x_fade_in(axp_info_volt, -50, 40, delay+200);
    
    anim_x_fade_in(sys_info, -50, 40, delay+250);
    anim_x_fade_in(sys_info_flash, -50, 40, delay+300);
    anim_x_fade_in(sys_info_heap, -50, 40, delay+350);
    
    create_axp_sys_timer();

}
void info_page_anim_out(uint32_t delay){
    anim_y_fade_out(info_return, INFO_RETURN_Y, -50, delay);
    anim_y_fade_out(info_line, 0, 100, delay);

    anim_x_fade_out(axp_info, 40, 100, delay+10);
    anim_x_fade_out(axp_info_power, 40, 100, delay+30);
    anim_x_fade_out(axp_info_percentage, 40, 100, delay+50);
    anim_x_fade_out(axp_info_volt, 40, 100, delay+70);

    anim_x_fade_out(sys_info, 40, 100, delay+90);
    anim_x_fade_out(sys_info_flash, 40, 100, delay+110);
    anim_x_fade_out(sys_info_heap, 40, 100, delay+130);

    lv_timer_del(info_timer);

    motor_click(DEFAULT_MOTOR_CLICK_WORKTIME);
}

/*************************
 *        页面API
 *************************/

void info_page_hide_obj(void){
    lv_obj_add_flag(info_return, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(info_line, LV_OBJ_FLAG_HIDDEN);
    
    lv_obj_add_flag(axp_info, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(axp_info_volt, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(axp_info_power, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(axp_info_percentage, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_flag(sys_info, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(sys_info_flash, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(sys_info_heap, LV_OBJ_FLAG_HIDDEN);
}

void info_page_create_obj(void){
    create_info_return_btn();
    create_info_line();
    create_axp_info();
    create_sys_info();

    info_page_hide_obj();
}




/**
 * axp可显示信息
 *  充电状态、电池存在状态指示 ，电池电压、电量
 *  内部温度 【温度 + 过温指示】
 *  
 *  电池库伦计充、放电数据，功率    
 *  
 *  电量百分比、电池冲放电电流（currentPath或两个都显示）
 * 
 */


/* 
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

void app_main(void)
{
    printf("Hello world!\n");

    //Print chip information 
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}

 */
