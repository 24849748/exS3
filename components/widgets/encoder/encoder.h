#ifndef _ENCODER_H_
#define _ENCODER_H_

//lvgl api 
#include "lvgl.h"


void encoder_init(void);
void encoder_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

void encoder_task(void *pvParameter);

// void create_encoder_task(void);

#endif
