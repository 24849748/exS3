#include "button.h"

#include "widgets_conf.h"

#include "esp_timer.h"
#include "esp_log.h"

#define TAG "BUTTON"

#define ACTIVE_LEVEL    0   // 按键按下的电平

typedef struct {
    gpio_num_t pin;
    bool isPressing;            // 经过消抖判断后的状态
    
    int previousSteadyState;    // the previous steady state from the input pin, used to detect pressed and released event
	int lastSteadyState;        // the last steady state from the input pin
	int lastFlickerableState;   // the last flickerable state from the input pin
	int currentState;           // the current reading from the input pin


    int64_t debounceTime;       // 消抖时间
    int64_t lastDebounceTime;   // the last time the output pin was toggled

    void (*shortPress_cb)(void);    //
    void (*longPress_cb)(void);     //

    bool isShort;
    bool isLong;
    int64_t pressedTime;
    int64_t releasedTime;
} button_t;
button_t btn;

/**
 * @brief 设置button消抖时间
 * 
 * @param debounceTime 
 */
void button_set_debounceTime(int64_t debounceTime){
    btn.debounceTime = debounceTime;
}

/**
 * @brief 设置button事件回调函数
 * 
 * @param event 
 * @param event_cb 
 */
void button_set_event_cb(button_event_t event, button_event_cb_t event_cb){
    if(event == BTN_EVENT_SHORTPRESS){
        btn.shortPress_cb = event_cb;
    }else if(event == BTN_EVENT_LONGPRESS){
        btn.longPress_cb = event_cb;
    }
}

/**
 * @brief 获取button消抖后的状态
 * 
 * @return int 
 */
int button_get_state(void){
    return btn.lastSteadyState;
}

/**
 * @brief 获取button原始电平状态
 * 
 * @return int 
 */
int button_get_state_raw(void){
    return gpio_get_level(btn.pin);
}

/**
 * @brief 判断button是否按着
 * 
 * @return true 
 * @return false 
 */
bool button_isPressing(void){
    return btn.isPressing;
}

/**
 * @brief button是否按下
 * 
 * @return true 
 * @return false 
 */
bool button_isPressed(void){
    if(btn.previousSteadyState == 1 && btn.lastSteadyState == 0){
        return true;
    }else {
        return false;
    }
}

/**
 * @brief button是否释放
 * 
 * @return true 
 * @return false 
 */
bool button_isReleased(void){
    if(btn.previousSteadyState == 0 && btn.lastSteadyState == 1){
        return true;
    }else {
        return false;
    }
}




/**
 * @brief 按键扫描，需要在循环中调用
 * 
 */
void button_scan(void){
    btn.currentState = gpio_get_level(btn.pin);

    // 状态变化，更新状态、更新时间
    if(btn.currentState != btn.lastFlickerableState){       //如果currentState不等于lastFlickerableState
        btn.lastDebounceTime = esp_timer_get_time();            //更新上次状态变化时间，用于判断是否大于消抖间
        btn.lastFlickerableState = btn.currentState;        //更新currentState
    }

    // 消抖判断
    if((esp_timer_get_time() - btn.lastDebounceTime) >= btn.debounceTime){ //如果间隔时间大于消抖时间
        if(gpio_get_level(btn.pin) == ACTIVE_LEVEL){
            btn.isPressing = true;
        }else{
            btn.isPressing = false;
        }
        btn.previousSteadyState = btn.lastSteadyState; 
        btn.lastSteadyState = btn.currentState;             // 最后稳定状态 等于 现在的状态
        
    }

    if(btn.previousSteadyState == 1 && btn.lastSteadyState == 0){   // isPressed()
        btn.pressedTime = esp_timer_get_time();
        btn.isShort = true;
        btn.isLong = false;
    }
    if(btn.previousSteadyState == 0 && btn.lastSteadyState == 1){
        btn.isShort = false;
        btn.releasedTime = esp_timer_get_time();
        if((btn.releasedTime - btn.pressedTime) < SHORT_TIME){
            // ESP_LOGI(TAG, "short");
            btn.shortPress_cb();
        }
    }
    if(btn.isPressing == true && btn.isLong == false){
        if((esp_timer_get_time() - btn.pressedTime) > LONG_TIME){
            btn.isLong = true;
            // ESP_LOGI(TAG, "long");
            btn.longPress_cb();
        }
    }
}



// custom
void short_press_cb(void){
    ESP_LOGI(TAG, "short");
}
void long_press_cb(void){
    ESP_LOGI(TAG, "long");
}


/**
 * @brief 按键初始化
 * 
 */
void button_init(button_event_cb_t short_cb, button_event_cb_t long_cb){
    gpio_config_t config = {
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL << PIN_ECD_BTN),
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&config);

    
    btn.pin = PIN_ECD_BTN;
    btn.debounceTime = DEBOUNCE_TIME;
    
    btn.isPressing = false;
    btn.previousSteadyState = gpio_get_level(PIN_ECD_BTN);
    btn.lastSteadyState = gpio_get_level(PIN_ECD_BTN);
    btn.lastFlickerableState = gpio_get_level(PIN_ECD_BTN);

    btn.lastDebounceTime = 0;

    btn.isShort = false;
    btn.isLong = false;
    btn.pressedTime = 0;
    btn.releasedTime = 0;
    // btn.shortPress_cb = &short_press_cb;
    // encoder_btn_set_shortPress_event_cb(&short_press_cb);
    // btn.longPress_cb = &long_press_cb;
    // encoder_btn_set_longPress_event_cb(&long_press_cb);
    button_set_event_cb(BTN_EVENT_SHORTPRESS, short_cb);
    button_set_event_cb(BTN_EVENT_LONGPRESS, long_cb);
    

    ESP_LOGI(TAG, "Encoder Button init over");
}



/* 测试用例 */
static void button_task(void *pvParameter){
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(100));
        button_scan();
        // ESP_LOGI(TAG, "btn state:%d", encoder_btn_isPressing());
        // ESP_LOGI(TAG, "isReleased:%d", encoder_btn_isReleased());
    }
}

void create_button_task(void){
    xTaskCreatePinnedToCore(button_task, "encoder btn task", 2048, NULL, 4, NULL, 1);
}