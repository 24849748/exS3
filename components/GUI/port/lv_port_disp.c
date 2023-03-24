/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"

#include "st7789.h"

#include "sdkconfig.h"
#include <stdbool.h>

#include "esp_log.h"
#include "esp_heap_caps.h"

#define TAG "lv_disp_port"



// static void disp_init(void);
// static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);


void lv_port_disp_init(void)
{ 
    // driver_disp_init(); //初始化spi屏幕
    st7789_init(); //需要先初始化bus，add device and send init cmd

    static lv_disp_draw_buf_t draw_buf_dsc;
    // static lv_color_t buf_2_1[LV_HOR_RES_MAX * 40];                        /*A buffer for 10 rows*/
    // static lv_color_t buf_2_2[LV_VER_RES_MAX * 40];                        /*An other buffer for 10 rows*/
    lv_color_t *buf_2_1 = heap_caps_malloc(LV_HOR_RES_MAX * 40 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf_2_1);
    lv_color_t *buf_2_2 = heap_caps_malloc(LV_VER_RES_MAX * 40 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf_2_2);

    lv_disp_draw_buf_init(&draw_buf_dsc, buf_2_1, buf_2_2, LV_HOR_RES_MAX * 40);   /*Initialize the display buffer*/


    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    static lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res = LV_VER_RES_MAX;

    /*Used to copy the buffer's content to the display*/
    // disp_drv.flush_cb = disp_flush;
    disp_drv.flush_cb = st7789_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
    
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


volatile bool disp_flush_enabled = true;
/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void){
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void){
    disp_flush_enabled = false;
}


// static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
// {
//     if(disp_flush_enabled) {
//         disp_driver_flush(disp_drv, area, color_p);
//     }

//     /*IMPORTANT!!!
//      *Inform the graphics library that you are ready with the flushing*/
    // lv_disp_flush_ready(disp_drv);
// }

