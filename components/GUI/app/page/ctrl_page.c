#include "ctrl_page.h"
#include "main_page.h"
#include "anim.h"
#include "lv_common.h"
#include "esp_log.h"

#include "led.h"
#include "motor.h"

#define TAG "ctrl_page"

#define CTRL_RETURN_Y 15

static lv_obj_t * ctrl_return;
static lv_obj_t * btn_led;
static lv_obj_t * btn_motor;

bool LEDstatus = false;
bool MOTORstatus = false;


/* 按钮回调函数 */
static void return_mainpage_cb(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "return main page");
        ctrl_page_anim_out(200);
        main_page_anim_in(200);
    }
}
static void btn_led_cb(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        LEDstatus = !LEDstatus;
        led_set(PIN_LED, LEDstatus);
        ESP_LOGI(TAG, "LED:%s\n",LEDstatus?"open":"close");
    }
}
static void btn_motor_cb(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        MOTORstatus = !MOTORstatus;
        motor_set(PIN_MOTOR, MOTORstatus);
        ESP_LOGI(TAG, "Motor:%s\n",MOTORstatus?"open":"close");
    }
}


/*************************
 *      绘制页面内容
 *************************/


/* info 页面返回按键 */
void create_ctrl_return_btn(void){
    /* 返回键 */
    ctrl_return = create_return_button(return_mainpage_cb);
}

void create_ctrl_page_btn(void){
    
    // btn_led = lv_obj_create(bg_screen);
    // lv_obj_remove_style_all(btn_led);
    
    // lv_obj_add_style(btn_led, &style_btn, LV_STATE_DEFAULT);
    // lv_obj_add_style(btn_led, &style_btn_pr, LV_STATE_PRESSED);

    // lv_obj_set_size(btn_led, 90, 60);
    // lv_obj_align(btn_led, LV_ALIGN_RIGHT_MID, -20, 10);
    // lv_obj_add_event_cb(btn_led, btn_led, LV_EVENT_CLICKED, NULL);

    // lv_obj_t * ac_label = lv_label_create(btn_led);
    // lv_label_set_recolor(ac_label, true);
    // lv_label_set_text(ac_label, "#ffffff Led");
    // lv_obj_align(ac_label, LV_ALIGN_TOP_LEFT, 5, 5);

    btn_led = lv_create_button_c(bg_screen, 90, 60, LV_ALIGN_LEFT_MID, 20, 10, btn_led_cb, "#ffffff Led");
    lv_obj_remove_style(btn_led, &style_btn_pr, LV_STATE_PRESSED);
    lv_obj_add_flag(btn_led, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_style(btn_led, &style_btn_pr, LV_STATE_CHECKED);
    btn_motor = lv_create_button_c(bg_screen, 90, 60, LV_ALIGN_RIGHT_MID, -20, 10, btn_motor_cb, "#ffffff Motor");
    lv_obj_remove_style(btn_motor, &style_btn_pr, LV_STATE_PRESSED);
    lv_obj_add_flag(btn_motor, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_style(btn_motor, &style_btn_pr, LV_STATE_CHECKED);
}


/*************************
 *        页面动画
 *************************/

void ctrl_page_anim_in(uint32_t delay){
    anim_y_fade_in(ctrl_return, -50, CTRL_RETURN_Y, delay);
    anim_x_fade_in(btn_led, -100, 20, delay);
    anim_x_fade_in(btn_motor, 100, -20, delay);
}

void ctrl_page_anim_out(uint32_t delay){
    anim_y_fade_out(ctrl_return, CTRL_RETURN_Y, -50, delay);
    anim_x_fade_out(btn_led, 20, -100, delay);
    anim_x_fade_out(btn_motor, -20, 100, delay);

    motor_click(DEFAULT_MOTOR_CLICK_WORKTIME);
}



/*************************
 *        页面API
 *************************/


void ctrl_page_hide_obj(void){
    lv_obj_add_flag(ctrl_return, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(btn_led, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(btn_motor, LV_OBJ_FLAG_HIDDEN);
}

void ctrl_page_create_obj(void){
    create_ctrl_return_btn();
    create_ctrl_page_btn();

    ctrl_page_hide_obj();
}
