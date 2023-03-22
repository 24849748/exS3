#ifndef _ROT_ENC_H_
#define _ROT_ENC_H_

#include "driver/pcnt.h"
#include "esp_err.h"


typedef void *encoder_handle_t;

// typedef struct {
//     // encoder_dev_t dev; /*!< Underlying device handle */
//     int phase_a_gpio_num;     /*!< Phase A GPIO number */
//     int phase_b_gpio_num;     /*!< Phase B GPIO number */
//     int flags;                /*!< Extra flags */
// } encoder_config_t;


encoder_handle_t encoder_create(pcnt_unit_t uint, int PIN_A, int PIN_B);

esp_err_t encoder_delete(encoder_handle_t * ecd);

esp_err_t encoder_set_glitch_fliter(encoder_handle_t ecd, uint32_t max_glitch_us);

esp_err_t encoder_start(encoder_handle_t ecd);

esp_err_t encoder_stop(encoder_handle_t ecd);

int encoder_get_counter_value(encoder_handle_t ecd);



#endif