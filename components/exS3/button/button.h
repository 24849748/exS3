#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define DEBOUNCE_TIME   8*1000          //消抖时间8ms
#define SHORT_TIME      800*1000        //短按时间阈值800ms
#define LONG_TIME       2000*1000       //长按时间阈值1s

typedef enum {
    BTN_EVENT_SHORTPRESS,
    BTN_EVENT_LONGPRESS,
} button_event_t;
typedef void * button_event_cb_t;

void button_init(button_event_cb_t short_cb, button_event_cb_t long_cb);
void button_scan(void);

void button_set_debounceTime(int64_t debounceTime);
void button_set_event_cb(button_event_t event, button_event_cb_t event_cb);
int button_get_state(void);
int button_get_state_raw(void);
bool button_isPressing(void);

bool button_isPressed(void);
bool button_isReleased(void);


void create_button_task(void);

#endif
