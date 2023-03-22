#include "lv_common.h"

//--------------------background-------------------------//
lv_obj_t *bg_screen;

//----------------------style----------------------------//
lv_style_t style_btn_pressed;   // 按下白色边框style
lv_style_t style_btn;
lv_style_t style_btn_pr;        // 按下黑色背景

//----------------------func----------------------------//
LV_FONT_DECLARE(font_LMYY);





//--------------------background-------------------------//
void bg_page(void){
    lv_obj_t *scr = lv_scr_act();
    bg_screen = lv_obj_create(scr);
    lv_obj_remove_style_all(bg_screen);
    lv_obj_set_size(bg_screen, 240, 320);
    lv_obj_set_style_radius(bg_screen,2,0); //圆角设置
    lv_obj_set_style_bg_opa(bg_screen,LV_OPA_COVER,0);
    lv_obj_set_style_bg_color(bg_screen, BG_COLOR, 0);
    lv_obj_center(bg_screen);
}

//----------------------style----------------------------//


/* 初始化自定义button style */
void init_custom_btn_style(void){
    /* border,按下效果 */
    lv_style_init(&style_btn_pressed);
    lv_style_set_radius(&style_btn_pressed,7);
    lv_style_set_bg_color(&style_btn_pressed,lv_color_black());
    lv_style_set_bg_opa(&style_btn_pressed,LV_OPA_COVER);

    lv_style_set_border_color(&style_btn_pressed, lv_color_white());
    lv_style_set_border_opa(&style_btn_pressed, LV_OPA_COVER);
    // lv_style_set_border_side(&style_btn_border, LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_RIGHT);
    lv_style_set_border_side(&style_btn_pressed, LV_BORDER_SIDE_FULL);
    lv_style_set_border_width(&style_btn_pressed, 2);



    /* default style */
    lv_style_init(&style_btn);
    lv_style_set_radius(&style_btn, 7);             //背景圆角
    lv_style_set_bg_color(&style_btn, BTNBG_COLOR); //按钮背景颜色
    lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);  //无不透明

    lv_style_set_border_width(&style_btn, 0);       //无边框
    lv_style_set_outline_width(&style_btn, 0);      //无外框
    lv_style_set_shadow_width(&style_btn, 0);       //无阴影

    /* pressed style */
    lv_style_init(&style_btn_pr);
    lv_style_set_bg_color(&style_btn_pr, lv_color_black());     //纯黑背景
    // lv_style_set_bg_opa(&style_btn_pr, LV_OPA_COVER);
    // lv_style_set_border_color(&style_btn_pr, lv_color_white());
    // lv_style_set_border_opa(&style_btn_pr, LV_OPA_COVER);
    // lv_style_set_border_side(&style_btn_pr, LV_BORDER_SIDE_FULL);
    // lv_style_set_border_width(&style_btn_pr, 2);
}

//----------------------func-----------------------------//
// /* 创建图像按钮 */
// lv_obj_t * create_click_imgbtn(lv_obj_t * parent, lv_event_cb_t event_cb, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs){
//     lv_obj_t * obj = lv_img_create(parent);
//     lv_obj_add_style(obj, &style_btn_pressed, LV_STATE_PRESSED);
//     lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
//     lv_obj_add_event_cb(obj, event_cb, LV_EVENT_CLICKED, NULL);
//     lv_obj_align(obj, align, x_ofs, y_ofs);
//     return obj;
// }

// /**
//  * @brief 创建文本按钮
//  * @todo 设置位置、文本（颜色、字体）、回调函数
//  * @param parent 父对象
//  * @return 对象
//  */
// lv_obj_t * create_text_btn(lv_obj_t * parent){
//     lv_obj_t * obj = lv_btn_create(parent);

//     lv_obj_remove_style(obj, NULL, LV_PART_MAIN);
//     //按下风格
//     lv_obj_add_style(obj, &style_btn_pressed, LV_STATE_PRESSED);

//     //bg 颜色
//     lv_obj_set_style_bg_color(obj, TEXT_COLOR, LV_STATE_DEFAULT);
//     lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_STATE_DEFAULT);
//     lv_obj_set_style_radius(obj, 5, LV_STATE_DEFAULT);
//     lv_obj_set_size(obj, 60, 40);

//     return obj;
// }

/**
 * @brief 创建一个自定义风格的按钮, text需要自带颜色
 * 
 */
lv_obj_t * lv_create_button_c(lv_obj_t * parent, lv_coord_t size_w, lv_coord_t size_h, lv_align_t align, lv_coord_t x, lv_coord_t y, lv_event_cb_t cb, const char *text){
    lv_obj_t * btn_obj = lv_btn_create(parent);
    lv_obj_remove_style_all(btn_obj);

    lv_obj_add_style(btn_obj, &style_btn, LV_STATE_DEFAULT);
    lv_obj_add_style(btn_obj, &style_btn_pr, LV_STATE_PRESSED);

    lv_obj_set_size(btn_obj, size_w, size_h);
    lv_obj_align(btn_obj, align, x, y);

    lv_obj_add_event_cb(btn_obj, cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * info_label = lv_label_create(btn_obj);
    lv_label_set_recolor(info_label, true);
    lv_label_set_text(info_label, text);
    lv_obj_align(info_label, LV_ALIGN_TOP_LEFT, 5, 5);
    return btn_obj;
}


/**
 * @brief 创建左上角 返回 按钮
 *      - 需要指定返回主页面的回调函数
 * 
 * @param coord_y 
 * @param click_cb 点击后返回主页的回调函数
 * @return lv_obj_t* 
 */
lv_obj_t * create_return_button(lv_event_cb_t click_cb){
    
    /* 返回键 */
    lv_obj_t * obj = lv_img_create(bg_screen);
    lv_obj_set_style_translate_y(obj, 5, LV_STATE_PRESSED);
    lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 10, COORD_RETURN_Y);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(obj, click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_ext_click_area(obj, 20); // 拓展点击区域
    LV_IMG_DECLARE(logo_return);        // 使用返回图片
    lv_img_set_src(obj, &logo_return); 
    
    return obj;
}

/**
 * @brief 按钮样式1：白色文字左上，按下黑色背景，需要自定对齐位置（mainpage）
 * 
 * @param w     obj宽度
 * @param h     obj高度
 * @param cb    按钮回调函数
 * @param info  文字信息
 * @return lv_obj_t* 
 */
lv_obj_t * bosco_create_btn1(lv_coord_t w, lv_coord_t h, lv_event_cb_t cb, const char *text){
    lv_obj_t * btn_obj = lv_btn_create(bg_screen);
    lv_obj_remove_style_all(btn_obj);
    lv_obj_add_style(btn_obj, &style_btn, LV_STATE_DEFAULT);
    lv_obj_add_style(btn_obj, &style_btn_pr, LV_STATE_PRESSED);
    lv_obj_set_size(btn_obj, w, h);
    lv_obj_add_event_cb(btn_obj, cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * _text = lv_label_create(btn_obj);
    lv_obj_set_style_text_color(_text, lv_color_white(), 0);
    lv_label_set_text(_text, text);
    lv_obj_align(_text, LV_ALIGN_TOP_LEFT, 5, 5);
    return btn_obj;
}


/**
 * @brief 按钮样式2(LMYY字体)：文字居中、主题色，按下变大5px，需要自定对齐位置（acpage）
 * 
 * @param w     obj宽度
 * @param h     obj高度
 * @param cb    按钮回调函数
 * @param info  文字信息
 * @return lv_obj_t* 
 */
lv_obj_t * bosco_create_btn2(lv_coord_t w, lv_coord_t h, lv_event_cb_t cb, const char *info){
    lv_obj_t * obj = lv_btn_create(bg_screen);
    lv_obj_remove_style(obj, NULL, LV_PART_MAIN);
    lv_obj_add_style(obj, &style_btn, LV_STATE_DEFAULT);
    lv_obj_add_style(obj, &style_btn_pr, LV_STATE_PRESSED);
    lv_obj_set_style_width(obj, w+4, LV_STATE_PRESSED);
    lv_obj_set_style_height(obj, h+4, LV_STATE_PRESSED);

    lv_obj_set_size(obj, w, h);
    lv_obj_add_event_cb(obj, cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * text = lv_label_create(obj);
    lv_obj_set_style_text_color(text, lv_color_hex(0x88B2FB), 0);
    lv_label_set_text(text, info);
    lv_obj_set_style_text_font(text, &font_LMYY, 0);
    lv_obj_center(text);

    return obj;
}

/**
 * @brief 按钮样式3(LMYY字体)：文字居中、主题色，按下白色边框，需要自定对齐位置
 * 
 * @param w     obj宽度
 * @param h     obj高度
 * @param cb    按钮回调函数
 * @param info  文字信息
 * @return lv_obj_t* 
 */
lv_obj_t * bosco_create_btn3(lv_coord_t w, lv_coord_t h, lv_event_cb_t cb, const char *info){
    lv_obj_t * obj = lv_btn_create(bg_screen);
    lv_obj_remove_style(obj, NULL, LV_PART_MAIN);
    lv_obj_add_style(obj, &style_btn, LV_STATE_DEFAULT);
    lv_obj_add_style(obj, &style_btn_pr, LV_STATE_PRESSED);
    lv_obj_set_style_border_color(obj, lv_color_white(), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(obj, 2, LV_STATE_PRESSED);
    lv_obj_set_style_border_side(obj, LV_BORDER_SIDE_FULL, LV_STATE_PRESSED);
    lv_obj_set_style_border_opa(obj, LV_OPA_COVER, LV_STATE_PRESSED);

    lv_obj_set_size(obj, w, h);
    
    lv_obj_add_event_cb(obj, cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t * text = lv_label_create(obj);
    lv_obj_set_style_text_color(text, lv_color_hex(0x88B2FB), 0);
    lv_label_set_text(text, info);
    lv_obj_set_style_text_font(text, &font_LMYY, 0);
    lv_obj_center(text);

    return obj;
}

/**
 * @brief 按钮样式4(系统字体)：文字居中、主题色，按下变大5px，需要自定对齐位置（acpage）
 * 
 * @param w     obj宽度
 * @param h     obj高度
 * @param cb    按钮回调函数
 * @param info  文字信息
 * @return lv_obj_t* 
 */
lv_obj_t * bosco_create_btn4(lv_coord_t w, lv_coord_t h, lv_event_cb_t cb, const char *info){
    lv_obj_t * obj = lv_btn_create(bg_screen);
    lv_obj_remove_style(obj, NULL, LV_PART_MAIN);
    lv_obj_add_style(obj, &style_btn, LV_STATE_DEFAULT);
    lv_obj_add_style(obj, &style_btn_pr, LV_STATE_PRESSED);
    lv_obj_set_style_width(obj, w+2, LV_STATE_PRESSED);
    lv_obj_set_style_height(obj, h+2, LV_STATE_PRESSED);

    lv_obj_set_size(obj, w, h);
    lv_obj_add_event_cb(obj, cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * text = lv_label_create(obj);
    lv_obj_set_style_text_color(text, lv_color_hex(0x88B2FB), 0);
    lv_label_set_text(text, info);
    lv_obj_center(text);

    return obj;
}