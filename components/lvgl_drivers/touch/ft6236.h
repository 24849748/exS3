#ifndef _FT6236_H_
#define _FT6236_H_

#include <stdint.h>
#include <stdbool.h>
#include "driver/i2c.h"


#include "lvgl.h"


#define FT6236_I2C_PORT I2C_NUM_0
#define FT6236_I2C_ADDR (0x38)

// typedef struct {
//     bool inited;
//     uint8_t addr;
//     i2c_port_t port;
// } ft6236_t;

typedef struct {
    int16_t last_x;
    int16_t last_y;
    lv_indev_state_t current_state;   //pressed or non
} ft6x36_touch_t;


void ft6236_init();

uint8_t ft6236_get_gesture_id();

void ft6236_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

void ft6236_enable_read(void);
void ft6236_disable_read(void);


#endif
