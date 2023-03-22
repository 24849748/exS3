#include "start_page.h"
#include "main_page.h"

#include "lv_common.h"
#include "anim.h"
/* #define */

#define COORD_GIF_X 15
#define COORD_GIF_Y -35

#define START_TIME  1000     // load界面执行时间

LV_IMG_DECLARE(loadimg1);
LV_IMG_DECLARE(loadimg2);
LV_IMG_DECLARE(loadimg3);
LV_IMG_DECLARE(loadimg4);
const lv_img_dsc_t *dsc_imgs_load[4] = {
    &loadimg4,
    &loadimg3,
    &loadimg2,
    &loadimg1,
};


/* 回调函数 */
static void anim_barValue_cb(void * obj, int32_t value){
    lv_bar_set_value(obj, value, LV_ANIM_ON);
}
void main_page_in_ready_cb(lv_anim_t * a){
    main_page_anim_in(0);
}

// void main_page_in_timer_cb(lv_timer_t *timer){
//     main_page_anim_in(0);
//     lv_timer_del(timer);
// }

/*************************
 *      绘制页面内容
 *************************/

void start_page(void){
    
    lv_obj_t * loadBar;
    lv_obj_t * loadLabel; 
    lv_obj_t * loadGif; 

    /* gif 动图 */
    loadGif = lv_animimg_create(bg_screen);
    lv_obj_align(loadGif, LV_ALIGN_LEFT_MID, COORD_GIF_X, COORD_GIF_Y);
    lv_animimg_set_src(loadGif,  (lv_img_dsc_t**)dsc_imgs_load, 4);
    lv_animimg_set_duration(loadGif, 250);
    lv_animimg_set_repeat_count(loadGif, LV_ANIM_REPEAT_INFINITE);
    lv_animimg_start(loadGif);

    /* Loading文本 */
    loadLabel = lv_label_create(bg_screen);
    lv_obj_set_style_text_color(loadLabel, lv_color_white(), 0);
    lv_label_set_text(loadLabel, "Loading...");
    lv_obj_align(loadLabel, LV_ALIGN_CENTER, 0, 20);

    /* 进度条 */
    loadBar = lv_bar_create(bg_screen);
    lv_obj_set_size(loadBar, 200, 7);       // 进度条大小
    lv_obj_center(loadBar);                 // 进度条位置
    lv_bar_set_range(loadBar, 5, 95);      // 进度条范围    
    lv_obj_set_style_bg_color(loadBar, lv_color_hex(0xfdd86d), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(loadBar, lv_color_hex(0xa9edb9), LV_PART_MAIN);

    /* 进度条动画 */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, loadBar);
    lv_anim_set_exec_cb(&a, anim_barValue_cb);
    lv_anim_set_time(&a, START_TIME);
    lv_anim_set_values(&a, 5, 95);
    lv_anim_set_delay(&a, 500);
    lv_anim_start(&a);

    lv_anim_set_var(&a, loadGif);
    lv_anim_set_exec_cb(&a, anim_x_cb);
    lv_anim_set_time(&a, START_TIME);
    lv_anim_set_values(&a, COORD_GIF_X, 185);
    lv_anim_set_delay(&a, 500);
    lv_anim_start(&a);

    lv_anim_set_var(&a, loadLabel);
    lv_anim_set_exec_cb(&a, NULL);
    lv_anim_set_time(&a, 1);
    lv_anim_set_delay(&a, START_TIME+450);
    lv_anim_set_ready_cb(&a, main_page_in_ready_cb);
    lv_anim_start(&a);

    lv_obj_del_delayed(loadGif, START_TIME+500);
    lv_obj_del_delayed(loadBar, START_TIME+500);
    lv_obj_del_delayed(loadLabel, START_TIME+500);

    // lv_timer_t * start_timer = lv_timer_create(main_page_in_timer_cb, START_TIME+500, NULL);
    // lv_timer_ready(start_timer);
}
