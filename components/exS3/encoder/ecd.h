#ifndef _ECD_H_
#define _ECD_H_

#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_err.h"
#include "driver/gpio.h"


#define ECD_USE_HALF_STEPS  false
#define ECD_FLIPS_DIR       true


#define TABLE_COLS 4


typedef uint8_t table_row_t[TABLE_COLS];

// typedef int32_t ecd_position_t;


/* direction */
#define NO_SET  0
#define CW      1
#define CCW     -1

typedef struct {
    int32_t position;
    int8_t dir; 
} ecd_state_t;


typedef struct {
    gpio_num_t pin_a;                       ///< GPIO for Signal A from the rotary encoder device
    gpio_num_t pin_b;                       ///< GPIO for Signal B from the rotary encoder device
    QueueHandle_t queue;                    ///< Handle for event queue, created by ::rotary_encoder_create_queue
    const table_row_t * table;              ///< Pointer to active state transition table
    uint8_t table_state;                    ///< Internal state
    // volatile ecd_state_t state;             ///< Device state
    bool flip;
    volatile int32_t pos;
    volatile int8_t dir;
} ecd_t;

// typedef void * ecd_handle_t;



ecd_t * ecd_create(gpio_num_t pin_a, gpio_num_t pin_b, bool half, bool flip);

esp_err_t ecd_deinit(ecd_t * ecd);

int32_t ecd_get_position(ecd_t * ecd);

int8_t ecd_get_direction(ecd_t * ecd);

esp_err_t ecd_reset(ecd_t * ecd);

QueueHandle_t ecd_get_queue_handle(ecd_t * ecd);



/*********************
 *     high level
 *********************/

void ecd_task(void *arg);






#endif