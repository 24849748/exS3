#include "rot_enc.h"

// #include "driver/pcnt.h"
#include "hal/pcnt_hal.h"

#include "esp_log.h"

#define TAG "hal_enc"

#define EC11_PCNT_DEFAULT_HIGH_LIMIT (100)
#define EC11_PCNT_DEFAULT_LOW_LIMIT  (-100)

typedef struct {
    // int diff;               //旋转方向 -1,0,1
    // int last_count;         //最后一次的值
    int accumu_count;       //总计数值
    int count;
    pcnt_unit_t pcnt_unit;  //pcnt单元
} encoder_t;


/**
 * @brief encoder pcnt 的溢出中断处理函数
 * 
 * @param arg 
 */
static void encoder_pcnt_overflow_handler(void *arg){
    encoder_t *ecd = (encoder_t *)arg;
    uint32_t status = 0;
    pcnt_get_event_status(ecd->pcnt_unit, &status);
    
    if(status & PCNT_EVT_H_LIM){
        ecd->accumu_count += EC11_PCNT_DEFAULT_HIGH_LIMIT;
    }else if(status & PCNT_EVT_L_LIM){
        ecd->accumu_count += EC11_PCNT_DEFAULT_LOW_LIMIT;
    }
}

esp_err_t encoder_set_count(encoder_handle_t ecd, int count){
    encoder_t *_ecd = (encoder_t *)ecd;
    _ecd->count = count;
    return ESP_OK;
}

/**
 * @brief 创建encoder设备，并初始化pcnt，返回操作句柄
 * 
 * @param uint  pcnt单元
 * @param PIN_A encoder引脚A
 * @param PIN_B encoder引脚B
 * @return encoder_handle_t encoder操作句柄
 */
encoder_handle_t encoder_create(pcnt_unit_t uint, int PIN_A, int PIN_B){
    encoder_t *ecd = (encoder_t *)calloc(1, sizeof(encoder_t));
    if(ecd == NULL){
        ESP_LOGE(TAG, "calloc ecd_t memory failed");
        return NULL;
    }

    ecd->pcnt_unit = uint;
    // ecd->diff = 0;
    // ecd->last_count = 0;

    pcnt_config_t dev_config = {
        .pulse_gpio_num = PIN_A,
        .ctrl_gpio_num = PIN_B,
        .channel = PCNT_CHANNEL_0,
        .unit = ecd->pcnt_unit,
        .pos_mode = PCNT_COUNT_DEC,
        .neg_mode = PCNT_COUNT_INC,
        .lctrl_mode = PCNT_MODE_REVERSE,
        .hctrl_mode = PCNT_MODE_KEEP,
        .counter_h_lim = EC11_PCNT_DEFAULT_HIGH_LIMIT,
        .counter_l_lim = EC11_PCNT_DEFAULT_LOW_LIMIT,
    };

    // ret = pcnt_unit_config(&dev_config);
    if(pcnt_unit_config(&dev_config) != ESP_OK){
        ESP_LOGE(TAG, "Config pcnt failed");
        free(ecd);
        return NULL;
    }

    dev_config.pulse_gpio_num = PIN_B;
    dev_config.ctrl_gpio_num = PIN_A;
    dev_config.channel = PCNT_CHANNEL_1;
    dev_config.pos_mode = PCNT_COUNT_INC;
    dev_config.neg_mode = PCNT_COUNT_DEC;
    // ret = pcnt_unit_config(&dev_config);
    if(pcnt_unit_config(&dev_config) != ESP_OK){
        ESP_LOGE(TAG, "Config pcnt failed");
        free(ecd);
        return NULL;
    }

    pcnt_counter_pause(ecd->pcnt_unit);
    pcnt_counter_clear(ecd->pcnt_unit);

    // pcnt_isr_service_install(0);
    if(pcnt_isr_service_install(0) != ESP_OK){
        ESP_LOGE(TAG, "Install ISR service failed");
        free(ecd);
        return NULL;
    }
    pcnt_isr_handler_add(ecd->pcnt_unit, encoder_pcnt_overflow_handler, ecd);

    //事件开启
    pcnt_event_enable(ecd->pcnt_unit, PCNT_EVT_H_LIM);
    pcnt_event_enable(ecd->pcnt_unit, PCNT_EVT_L_LIM);

    // pcnt_get_counter_value(ecd->pcnt_unit, &ecd->last_count);

    encoder_set_count(ecd, 50);

    return (encoder_handle_t)ecd;
}

/**
 * @brief 释放encoder内存资源
 * 
 * @param ecd encoder操作句柄
 * @return esp_err_t 
 */
esp_err_t encoder_delete(encoder_handle_t * ecd){
    if(ecd == NULL){
        return ESP_OK;
    }
    encoder_t *_ecd = (encoder_t *)(*ecd);
    free(_ecd);
    *ecd = NULL;
    return ESP_OK;
}

/**
 * @brief 设置encoder pcnt过滤器
 * 
 * @param ecd encoder操作句柄
 * @param max_glitch_us 
 * @return esp_err_t 
 */
esp_err_t encoder_set_glitch_fliter(encoder_handle_t ecd, uint32_t max_glitch_us){
    encoder_t *_ecd = (encoder_t *)ecd;

    esp_err_t ret;
    
    ret = pcnt_set_filter_value(_ecd->pcnt_unit, max_glitch_us * 80);
    if(ret != ESP_OK){
        ESP_LOGE(TAG, "set glitch filter failed");
        return ret;
    }

    if(max_glitch_us){
        pcnt_filter_enable(_ecd->pcnt_unit);
    } else {
        pcnt_filter_disable(_ecd->pcnt_unit);
    }

    return ESP_OK;
}

/**
 * @brief 恢复encoder pcnt计数器
 * 
 * @param ecd encoder操作句柄
 * @return esp_err_t 
 */
esp_err_t encoder_start(encoder_handle_t ecd){
    encoder_t *_ecd = (encoder_t *)ecd;
    return pcnt_counter_resume(_ecd->pcnt_unit);
}

/**
 * @brief 停止encoder pcnt计数器
 * 
 * @param ecd encoder操作句柄
 * @return esp_err_t 
 */
esp_err_t encoder_stop(encoder_handle_t ecd){
    encoder_t *_ecd = (encoder_t *)ecd;
    return pcnt_counter_pause(_ecd->pcnt_unit);
}

/**
 * @brief 读取编码器值
 * 
 * @param ecd encoder操作句柄
 * @return int 
 */
int encoder_get_counter_value(encoder_handle_t ecd){
    encoder_t *_ecd = (encoder_t *)ecd;
    int16_t read_count = 0;
    pcnt_get_counter_value(_ecd->pcnt_unit, &read_count);

    // if(read_count - _ecd->last_count < 0){
    //     _ecd->diff = -1;
    // }else if(read_count - _ecd->last_count > 0){
    //     _ecd->diff = 1;
    // }else if(read_count == _ecd->last_count){
    //     _ecd->diff = 0;
    // }

    // if(_ecd->diff == -1){
    //     _ecd->count -= 1;
    // }
    // if(_ecd->diff == 1){
    //     _ecd->count +=1;
    // }

    // if(_ecd->count > 100) _ecd->count = 100;
    // if(_ecd->count < 0) _ecd->count = 0;

    // return _ecd->count;
    return read_count + _ecd->accumu_count;
}
