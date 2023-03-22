#include "anim.h"


/**
 * @brief x轴运动动画
 */
void anim_x_cb(void * var, int32_t v){
    lv_obj_set_x(var, v);
}
/**
 * @brief y轴运动动画
 */
void anim_y_cb(void * var, int32_t v){
    lv_obj_set_y(var, v);
    // lv_obj_set_style_opa(var,-v,0);
}
/**
 * @brief 不透明度动画
 */
void anim_opa_cb(void * var, int32_t v){
    lv_obj_set_style_opa(var, v, 0);
}

/**
 * @brief anim中隐藏obj回调
 */
void lv_obj_hide_anim_ready_cb(lv_anim_t * a){
    lv_obj_add_flag(a->var, LV_OBJ_FLAG_HIDDEN);
}
/**
 * @brief anim中显示obj回调
 */
void lv_obj_show_anim_ready_cb(lv_anim_t * a){
    if(lv_obj_has_flag(a->var, LV_OBJ_FLAG_HIDDEN)){
        lv_obj_clear_flag(a->var, LV_OBJ_FLAG_HIDDEN);
    }else {
        return;
    }
}


/**
 * @brief 通用动画设置
 * 
 * @param obj       需要添加动画的对象
 * @param start     开始值
 * @param end       结束值
 * @param duration  动画时长
 * @param delay     开始执行动画前的等待时长
 * @param exec_cb   anim_x_cb 或 anim_y_cb，执行动画运动的轴
 * @param ready_cb  动画ready完的回调函数
 * @param path_cb   内置路径动画函数
 *                  - lv_anim_path_linear/ease_in/ease_out/ease_in_out/overshoot/bounce/step
 * 
 */
void set_obj_anim(lv_obj_t *obj, int32_t start, int32_t end, uint32_t duration, uint32_t delay, lv_anim_exec_xcb_t exec_cb, lv_anim_ready_cb_t ready_cb, lv_anim_path_cb_t path_cb){
    lv_anim_t a;

    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, start, end);
    lv_anim_set_time(&a, duration);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)exec_cb);
    lv_anim_set_ready_cb(&a, ready_cb);
    lv_anim_set_path_cb(&a, path_cb);
    lv_anim_start(&a);
}


/* ==============预设的动画================= */

/**
 * @brief 沿x轴动画入场，默认动画时间400ms，ease_in_out，不透明度增加/淡入
 */
void anim_x_fade_in(lv_obj_t *obj, int32_t start, int32_t end, uint32_t delay){
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, start, end);
    lv_anim_set_time(&a, 400);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_exec_cb(&a, anim_x_cb);
    lv_anim_set_ready_cb(&a, lv_obj_show_anim_ready_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);

    // lv_anim_set_values(&a, LV_OPA_0, LV_OPA_COVER);
    // lv_anim_set_time(&a, 300);
    // lv_anim_set_exec_cb(&a, anim_opa_cb);
    // lv_anim_start(&a);
}

/**
 * @brief 沿x轴动画出场，默认动画时间400ms，ease_in_out，不透明度减少/淡出
 */
void anim_x_fade_out(lv_obj_t *obj, int32_t start, int32_t end, uint32_t delay){
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, start, end);
    lv_anim_set_time(&a, 400);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_exec_cb(&a, anim_x_cb);
    lv_anim_set_ready_cb(&a, lv_obj_hide_anim_ready_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);

    // lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_0);
    // lv_anim_set_time(&a, 300);
    // lv_anim_set_exec_cb(&a, anim_opa_cb);
    // lv_anim_start(&a);
}


/**
 * @brief 沿y轴动画入场，默认动画时间400ms，ease_in_out，不透明度增加/淡入
 */
void anim_y_fade_in(lv_obj_t *obj, int32_t start, int32_t end, uint32_t delay){
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, start, end);
    lv_anim_set_time(&a, 400);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_exec_cb(&a, anim_y_cb);
    lv_anim_set_ready_cb(&a, lv_obj_show_anim_ready_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);
    // lv_anim_set_values(&a, LV_OPA_0, LV_OPA_COVER);
    // lv_anim_set_time(&a, 300);
    // lv_anim_set_exec_cb(&a, anim_opa_cb);
    // lv_anim_start(&a);
}

/**
 * @brief 沿y轴动画出场，默认动画时间400ms，ease_in_out，不透明度减少/淡出
 */
void anim_y_fade_out(lv_obj_t *obj, int32_t start, int32_t end, uint32_t delay){
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, start, end);
    lv_anim_set_time(&a, 400);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_exec_cb(&a, anim_y_cb);
    lv_anim_set_ready_cb(&a, lv_obj_hide_anim_ready_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);
    // lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_0);
    // lv_anim_set_time(&a, 300);
    // lv_anim_set_exec_cb(&a, anim_opa_cb);
    // lv_anim_start(&a);
}


/**
 * @brief 不透明度增加动画，动画时间500ms，step
 */
void anim_step_in(lv_obj_t *obj, uint32_t delay){
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, 0, 255);
    lv_anim_set_time(&a, 500);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_exec_cb(&a, anim_opa_cb);
    lv_anim_set_ready_cb(&a, lv_obj_show_anim_ready_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_step);
    lv_anim_start(&a);
}
/**
 * @brief 不透明度减少动画，动画时间500ms，step
 */
void anim_step_out(lv_obj_t *obj, uint32_t delay){
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, 0, 255);
    lv_anim_set_time(&a, 500);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_exec_cb(&a, anim_opa_cb);
    lv_anim_set_ready_cb(&a, lv_obj_hide_anim_ready_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_step);
    lv_anim_start(&a);
}

/**
 * @brief notify bar进场动画
 */
void anim_y_liner_in(lv_obj_t *obj, int32_t start, int32_t end, uint32_t delay){
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, start, end);
    lv_anim_set_time(&a, 200);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_exec_cb(&a, anim_y_cb);
    // lv_anim_set_ready_cb(&a, lv_obj_show_anim_ready_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_start(&a);
}
/**
 * @brief notify bar出场动画
 */
void anim_y_liner_out(lv_obj_t *obj, int32_t start, int32_t end, uint32_t delay){
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, start, end);
    lv_anim_set_time(&a, 200);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_exec_cb(&a, anim_y_cb);
    // lv_anim_set_ready_cb(&a, lv_obj_hide_anim_ready_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_start(&a);
}


