#ifndef _ST7789_H_
#define _ST7789_H_

#include "lvgl.h"
#include "st7789_reg.h"


#define LCD_PIN_RST     11
#define LCD_PIN_BLK     45
#define LCD_PIN_DC      21

#define LCD_INVERT_COLORS   0
#define LCD_SOFTWARE_RST    0

#define ORIENTATION_PORTRAIT   0               // 竖向
#define ORIENTATION_PORTRAIT_INVERTED   1      // 竖向反转
#define ORIENTATION_LANDSCAPE   2               // 横向
#define ORIENTATION_LANDSCAPE_INVERTED   3      // 横向反转
#define LCD_ORIENTATION     ORIENTATION_LANDSCAPE



#ifndef LV_HOR_RES_MAX      //水平最大分辨率
    #define LV_HOR_RES_MAX    (240)
#endif

#ifndef LV_VER_RES_MAX      //垂直最大分辨率
    #define LV_VER_RES_MAX    (320)
#endif


#define CUSTOM_DISPLAY_BUFFER_SIZE 1
#if (CUSTOM_DISPLAY_BUFFER_SIZE)
    #define DISP_BUF_SIZE 1024*10
#else
    #define DISP_BUF_SIZE (LV_HOR_RES_MAX * 40)
#endif




/* ====================应用层API==================== */
void st7789_init(void);
void st7789_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);

// void st7789_enable_backlight(bool backlight);


#endif
