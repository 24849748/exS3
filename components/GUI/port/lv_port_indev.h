#ifndef LV_PORT_INDEV_H
#define LV_PORT_INDEV_H


/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/
#define LV_INDEV_USE_ENCODER    0
#define LV_INDEV_USE_BUTTON    0
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_port_indev_init(void);

#if LV_INDEV_USE_ENCODER
lv_group_t * lv_get_encoder_group(void);
#endif

/**********************
 *      MACROS
 **********************/


#endif
