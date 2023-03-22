#include "ft6236.h"
#include "ft6236_def.h"
#include "i2c_bus.h"

#include "esp_log.h"
#define TAG "FT6236"

// static ft6236_t ft6236;
static ft6x36_touch_t touch_inputs = { -1, -1, LV_INDEV_STATE_REL};    // -1 coordinates to designate it was never touched

bool ft6236_inited = false;
bool touch_enable = true;

void ft6236_enable_read(void){
    touch_enable = true;
}
void ft6236_disable_read(void){
    touch_enable = false;
}

/**
 * @brief ft6236读取寄存器数据，使用前必须先运行 ft6236_init() 函数
 * 
 * @param reg   待读取的寄存器
 * @param data  读取后存储的数据
 */
static esp_err_t ft6236_read_byte(uint8_t reg, uint8_t *data){
    return i2c_bus_read_byte(FT6236_I2C_PORT, FT6236_I2C_ADDR, reg, data);
}
static esp_err_t ft6236_read_bytes(uint8_t reg, uint8_t *data, size_t len){
    return i2c_bus_read_bytes(FT6236_I2C_PORT, FT6236_I2C_ADDR, reg, data, len);
}


/**
 * @brief 初始化ft6236
 * 
 * @param dev_addr  FT6236 ID地址
 * @param port      FT6236 所在I2C端口
 */
void ft6236_init(){
    ft6236_inited = true;

    uint8_t t_data;
    esp_err_t ret;
    ESP_LOGI(TAG, "Begin to init ft6236");

    ret = ft6236_read_byte(FT6X36_PANEL_ID_REG, &t_data);
    if (ret != ESP_OK){
        ESP_LOGE(TAG, "Error reading from device: %s", esp_err_to_name(ret));
    }
    ESP_LOGI(TAG, "\tDevice ID: 0x%02x", t_data);

    ft6236_read_byte(FT6X36_CHIPSELECT_REG, &t_data);
    ESP_LOGI(TAG, "\tChip ID: 0x%02x", t_data);
    
    ft6236_read_byte(FT6X36_DEV_MODE_REG, &t_data);
    ESP_LOGI(TAG, "\tChip ID: 0x%02x", t_data);
    
    ft6236_read_byte(FT6X36_FIRMWARE_ID_REG, &t_data);
    ESP_LOGI(TAG, "\tChip ID: 0x%02x", t_data);
    
    ft6236_read_byte(FT6X36_RELEASECODE_REG, &t_data);
    ESP_LOGI(TAG, "\tChip ID: 0x%02x", t_data);

    ESP_LOGI(TAG, "Init ft6236 over");
}

/**
 * @brief ft6236读取坐标，作为lv_port_indev的回调
 * 
 * @param drv 
 * @param data 
 * @return true 
 * @return false 
 */
void ft6236_read(lv_indev_drv_t *drv, lv_indev_data_t *data){
    if (!ft6236_inited) {
        ESP_LOGE(TAG, "Init first!");
        return;
    }

    if(!touch_enable){
        data->point.x = -1;
        data->point.y = -1;
        data->state = LV_INDEV_STATE_REL;

        return;
    }

    uint8_t data_buf[5];        // 1 byte status, 2 bytes X, 2 bytes Y
    esp_err_t ret = ft6236_read_bytes(FT6X36_TD_STAT_REG, &data_buf[0], 5);
    // esp_err_t ret = i2c_bus_read_bytes(ft6236.port, ft6236.addr,FT6X36_TD_STAT_REG, &data_buf[0],5);
    if(ret != ESP_OK){
        ESP_LOGE(TAG, "Error talking to touch IC: %s", esp_err_to_name(ret));
    }

    
    //读取到的触摸点数量
    uint8_t touch_pnt_cnt = data_buf[0];

    //忽略无触摸和多点触摸，只允许单点触摸
    if (ret != ESP_OK || touch_pnt_cnt != 1) {
        if ( touch_inputs.current_state != LV_INDEV_STATE_REL){
            touch_inputs.current_state = LV_INDEV_STATE_REL;
        } 

        //赋值并返回
        data->point.x = touch_inputs.last_x;
        data->point.y = touch_inputs.last_y;
        data->state = touch_inputs.current_state;
        
        return;
    }


    touch_inputs.current_state = LV_INDEV_STATE_PR;
    touch_inputs.last_x = ((data_buf[1] & FT6X36_MSB_MASK) << 8) | (data_buf[2] & FT6X36_LSB_MASK);
    touch_inputs.last_y = ((data_buf[3] & FT6X36_MSB_MASK) << 8) | (data_buf[4] & FT6X36_LSB_MASK);

#if LV_FT6X36_SWAPXY
    int16_t swap_buf = touch_inputs.last_x;
    touch_inputs.last_x = touch_inputs.last_y;
    touch_inputs.last_y = swap_buf;
#endif
#if LV_FT6X36_INVERT_X
    touch_inputs.last_x =  LV_HOR_RES - touch_inputs.last_x;
#endif
#if LV_FT6X36_INVERT_Y
    touch_inputs.last_y = LV_VER_RES - touch_inputs.last_y;
#endif

    //赋值
    data->point.x = touch_inputs.last_x;
    data->point.y = touch_inputs.last_y;
    data->state = touch_inputs.current_state;
    ESP_LOGD(TAG, "X=%u Y=%u", data->point.x, data->point.y);
    
    return;
}



/**
  * @brief  Read the FT6x36 gesture ID. Initialize first!
  * @param  dev_addr: I2C FT6x36 Slave address.
  * @retval The gesture ID or 0x00 in case of failure
  */

/**
 * @brief 读取ft6236手势ID
 * 
 * @return uint8_t 返回读取到的手势ID
 */
uint8_t ft6236_get_gesture_id() {
    if (!ft6236_inited) {
        ESP_LOGE(TAG, "Init first!");
        return 0x00;
    }

    uint8_t data_buf;
    esp_err_t ret;
    
    if ((ret = ft6236_read_byte(FT6X36_GEST_ID_REG, &data_buf) != ESP_OK))
        ESP_LOGE(TAG, "Error reading from device: %s", esp_err_to_name(ret));
    return data_buf;
}
