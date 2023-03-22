#include "axp173.h"
#include <string.h>
#include "i2c_bus.h"
// #include "i2c_mid.h"
#include "esp_err.h"
#include "esp_log.h"

#define TAG "axp"

#define AXP_GET_REG(x)  (x >> 8)

#define AXP_GET_BITNUM(x)   (x & 0x0f)      // 
// 检查输入电压范围
#define AXP_CHECK_VOLT(volt, min, max, ret)   if(volt < min || volt > max){           \
                                                    ESP_LOGI(TAG, "Invalid voltage!");  \
                                                    return ret;                         \
                                                }                                       
// 将读取的数据存入info结构体
#define AXP_READ_STATUS(info, t, bit_num)    info = ((t & (1<<bit_num))? 1 : 0)


static esp_err_t axp_read_byte(uint8_t reg, uint8_t *data){
    return i2c_bus_read_bytes(AXP_I2C_PORT, AXP_I2C_ADDR, reg, data, 1);
}
static esp_err_t axp_read_bytes(uint8_t reg, uint8_t *data, size_t len){
    return i2c_bus_read_bytes(AXP_I2C_PORT, AXP_I2C_ADDR, reg, data, len);
}
static esp_err_t axp_write_byte(uint8_t reg, uint8_t data){
    return i2c_bus_write_bytes(AXP_I2C_PORT, AXP_I2C_ADDR, reg, &data, 1);
}




esp_err_t axp_init() {
    ESP_LOGI(TAG, "Init axp173 ...");

    /* 电源输出控制 */
    axp_write_byte(0x12, axp_op_generate_byte(true, false, false, true, true, true));
    
    axp_en_ctrl(EN_COLUMB, true);       // 库仑计使能
    axp_en_ctrl(EN_ADC_INTER_TEMP, true);   // 内部温度ADC使能
    axp_en_ctrl(EN_PEK_SHUTDOWN, true); //长按PEK关机使能
    axp_write_byte(AXP173_PEK, 0b01011100);     // 关机时间：4s，开机时间：512ms，：长按键时间：1.5s

    /* set voltage */
    axp_set_volt(DC1_SET_VOLT, 3300);   
    axp_set_volt(LDO2_SET_VOLT, 3300);
    axp_set_volt(LDO3_SET_VOLT, 3300);
    axp_set_volt(LDO4_SET_VOLT, 3300);

    /* PEK 按键参数设置 */
    // axp_pek_setting(1,1,1);
    
    axp_info_t info;
    axp_read_info(&info);
    axp_show_info(&info);

    ESP_LOGI(TAG, "Init axp173 over.");
    return ESP_OK;
}

/**
 * @brief 使能类设置，包括电源输出使能，ADC使能等
 * 
 * @param command 包括reg和bit_num信息的传参，格式: EN_OP_***; EN_***; EN_ADC_***
 * @param enable  true or false
 * @return esp_err_t 
 */
esp_err_t axp_en_ctrl(uint32_t command, bool enable) {
    esp_err_t ret = ESP_OK;
    uint8_t reg = AXP_GET_REG(command);
    uint8_t bit_num = AXP_GET_BITNUM(command);
    uint8_t tmp;

    ret = axp_read_byte(reg, &tmp);
    if(enable){
        tmp |= (1<<bit_num);    // set bit
    }else{
        tmp &= ~(1<<bit_num);   // clear bit
    }
    ret = axp_write_byte(reg, tmp);

    return ret;
}

/**
 * @brief 电压设置
 * 
 * @param channel 待设置的电源通道, 格式: ***_SET_VOLT
 * @param volt 待设置电压, 单位mV
 * @return esp_err_t 
 */
esp_err_t axp_set_volt(uint32_t channel, int volt) {
    esp_err_t ret = ESP_OK;
    uint8_t reg = AXP_GET_REG(channel);
    uint8_t tmp = 0;

    switch (channel)
    {
    case DC2_SET_VOLT:
        AXP_CHECK_VOLT(volt, 700, 2275, ESP_FAIL);
        tmp = (volt-700)/25;
        ret = axp_write_byte(reg, tmp);
        break;

    case DC1_SET_VOLT:
        AXP_CHECK_VOLT(volt, 700, 3500, ESP_FAIL);
        tmp = (volt-700)/25;
        ret = axp_write_byte(reg, tmp);
        break;

    case LDO4_SET_VOLT:
        AXP_CHECK_VOLT(volt, 700, 3500, ESP_FAIL);
        tmp = (volt-700)/25;
        ret = axp_write_byte(reg, tmp);
        break;

    case LDO2_SET_VOLT:
        AXP_CHECK_VOLT(volt, 1800, 3300, ESP_FAIL);
        ret = axp_read_byte(reg, &tmp);
        tmp &= ~(0b11110000);
        tmp |= ((volt-1800)/100 << 4);
        ret = axp_write_byte(reg, tmp);
        break;
    
    case LDO3_SET_VOLT:
        AXP_CHECK_VOLT(volt, 1800, 3300, ESP_FAIL);
        ret = axp_read_byte(reg, &tmp);
        tmp &= ~(0b00001111);
        tmp |= ((volt-1800)/100);
        ret = axp_write_byte(reg, tmp);
        break;

    case VHOLD_SET_VOLT:
        AXP_CHECK_VOLT(volt, 4000, 4700, ESP_FAIL);
        ret = axp_read_byte(reg, &tmp);
        tmp &= ~(0b00111000);           // 清除要写入的位区域
        tmp |= (((volt-4000)/100) << 3);    // setbit写入新的数据
        ret = axp_write_byte(reg, tmp);
        break;

    case VOFF_SET_VOLT:
        AXP_CHECK_VOLT(volt, 2600, 3300, ESP_FAIL);
        ret = axp_read_byte(reg, &tmp);
        tmp &= ~(0b00000111);
        tmp |= ((volt-2600)/100);
        ret = axp_write_byte(reg, tmp);
        break;
    }

    return ret;
}

/**
 * @brief 读取AXP173常用信息, 并存储在 info结构体里
 * 
 * @param info 
 * @return esp_err_t 
 */
esp_err_t axp_read_info(axp_info_t *info) {
    ESP_LOGI(TAG, "Reading axp info.");
    uint8_t tmp;
    axp_read_byte(0x00, &tmp);
    // ESP_LOGI(TAG, "0x00 tmp: %d", tmp);
    AXP_READ_STATUS(info->acin_exist, tmp, 7);
    AXP_READ_STATUS(info->vbus_exist, tmp, 5);

    AXP_READ_STATUS(info->bat_current_dir, tmp, 2);


    axp_read_byte(0x01, &tmp);
    // ESP_LOGI(TAG, "0x01 tmp: %d", tmp);
    AXP_READ_STATUS(info->axp_over_temp, tmp, 7);
    AXP_READ_STATUS(info->charge_idct, tmp, 6);
    AXP_READ_STATUS(info->bat_exist, tmp, 5);


    axp_read_byte(0x12, &tmp);
    // ESP_LOGI(TAG, "0x12 tmp: %d", tmp);
    AXP_READ_STATUS(info->exten, tmp, 6);
    AXP_READ_STATUS(info->dcdc2, tmp, 4);
    AXP_READ_STATUS(info->ldo3, tmp, 3);
    AXP_READ_STATUS(info->ldo2, tmp, 2);
    AXP_READ_STATUS(info->ldo4, tmp, 1);
    AXP_READ_STATUS(info->dcdc1, tmp, 0);


    // axp_show_info(info);
    
    ESP_LOGI(TAG, "End to read axp info.");
    return ESP_OK;
}

/**
 * @brief 读取ADC寄存器数据，并转换单位
 * 
 * @param dc 格式: DATA_***_***
 * @param buffer 返回的数据
 * @return esp_err_t 
 */
esp_err_t axp_read_adc_data(uint32_t dc, float *buffer){
    esp_err_t ret = ESP_OK;
    uint8_t reg = AXP_GET_REG(dc);
    uint8_t len = (dc & 0xff);
    uint8_t data[len];  // row data
    ESP_ERROR_CHECK(axp_read_bytes(reg, data, len));
    float step;     // sen = step/1000
    switch (dc)
    {
    case DATA_ACIN_VOLT:
        step = 1.7;
        *buffer = (((data[0]<<4) + data[1]) * step)/1000;
        break;
    case DATA_ACIN_CURRENT:
        step = 0.625;
        *buffer = (((data[0]<<4) + data[1]) * step)/1000;
        break;
    case DATA_VBUS_VOLT:
        step = 1.7;
        *buffer = (((data[0]<<4) + data[1]) * step)/1000;
        break;
    case DATA_VBUS_CURRENT:
        step = 0.375;
        *buffer = (((data[0]<<4) + data[1]) * step)/1000;
        break;
    case DATA_INTEL_TEMP:
        step = 0.1;
        *buffer = ((((data[0]<<4) + data[1]) * step)/1000)-144.7;
        break;
    case DATA_TS_ADC:
        step = 0.8;
        *buffer = (((data[0]<<4) + data[1]) * step)/1000;
        break;
    case DATA_BAT_POWER:
        step = 1.1/2;
        *buffer = (((data[0]<<16) + (data[1]<<8) + data[2]) * step)/1000;
        break;
    case DATA_BAT_VOLT:
        step = 1.1;
        *buffer = (((data[0]<<4) + data[1]) * step)/1000;
        break;
    case DATA_BAT_DISCHARGE_CURRENT:
        step = 0.5;
        *buffer = (((data[0]<<5) + data[1]) * step)/1000;
        break;
    case DATA_BAT_CHARGE_CURRENT:
        step = 0.5;
        *buffer = (((data[0]<<5) + data[1]) * step)/1000;
        break;
    case DATA_APS_VOLT:
        step = 1.4;
        *buffer = (((data[0]<<4) + data[1]) * step)/1000;
        break;
    // case DATA_COLUMB_CHARGE:
    // case DATA_COLUMB_DISCHARGE:
        // break;
    }

    return ret;
}


/**
 * @brief [abandon],PEK 按键参数设置 , '{}' is default
 * 
 * @param boot_time         [0]:128ms _ {1}:512ms _ [2]:1s _ [3]:2s
 * @param longpress_time    [0]:1s _ {1}:1.5s _ [2]:2s _ [3]:2.5s
 * @param shutdown_time     [0]:4s _ {1}:6s _ [2]:8s _ [3]:10s
 * @return esp_err_t 
 */
esp_err_t axp_pek_setting(uint8_t boot_time, uint8_t longpress_time, uint8_t shutdown_time) {
    uint8_t tmp;
    tmp = axp_read_byte(AXP173_PEK, &tmp);
    tmp &= ~(0b11110011);
    tmp |= ((boot_time<<6) | (longpress_time<<4) | shutdown_time);

    return axp_write_byte(AXP173_PEK, tmp);
}

/**
 * @brief ADC采样速率设置
 * 
 * @param rate      [0]:25Hz _ [1]:50Hz _ [2]:100Hz _ [3]:200Hz
 * @return esp_err_t 
 */
esp_err_t axp_set_adc_sample_rate(uint8_t rate){
    uint8_t tmp;
    axp_read_byte(0x84, &tmp);
    tmp &= ~(0b11000000);
    tmp |= (rate << 6);
    return axp_write_byte(0x84, tmp);
}

/**
 * @brief [abandon], TS 管脚设置, '{}' is default
 * 
 * @param op_current    输出电流：      [0]:20uA _ [1]:40uA _ [2]:60uA _ {3}:80uA
 * @param op_way        电流输出方式：  [0]:关闭 _ [1]:充电时输出电流 _ {2}:ADC采样时 _ [3]:一直打开
 * @param function      管脚功能：      {0}:电池温度监测 _ [1]:外部独立ADC
 * @return esp_err_t 
 */
esp_err_t axp_ts_setting(uint8_t op_current, uint8_t op_way, uint8_t function){
    uint8_t tmp;
    tmp = axp_read_byte(0x84, &tmp);
    tmp &= ~(0b00110111);
    tmp |= ((op_current << 4 | (function << 2) | op_way));
    
    return axp_write_byte(0x84, tmp);
}

/**
 * @brief 暂停库仑计
 * 
 * @return esp_err_t 
 */
esp_err_t axp_columb_pause(void){
    uint8_t tmp;
    axp_read_byte(0xb8, &tmp);
    tmp |= (1<<6);
    return axp_write_byte(0xb8, tmp);
}
/**
 * @brief 清除库仑计
 * 
 * @return esp_err_t 
 */
esp_err_t axp_columb_clear(void){
    uint8_t tmp;
    axp_read_byte(0xb8, &tmp);
    tmp |= (1<<5);
    return axp_write_byte(0xb8, tmp);
}
/* 未测试 */
esp_err_t axp_read_columb_data(float *buffer){
    uint8_t tmp[4];
    uint32_t columb_in, columb_out;
    esp_err_t ret = ESP_OK;
    ret = axp_read_bytes(0xb0, tmp, 4);
    columb_in = (tmp[0]<<24) + (tmp[1]<<16) + (tmp[2]<<8) + tmp[3];
    ret = axp_read_bytes(0xb4, tmp, 4);
    columb_out = (tmp[0]<<24) + (tmp[1]<<16) + (tmp[2]<<8) + tmp[3];
    ESP_LOGI(TAG,"columb in: %d, out: %d", columb_in, columb_out);
    *buffer = 32768 * (columb_in - columb_out) / 3600 / 25;
    return ret;
}

void axp_show_info(axp_info_t *info){
    ESP_LOGI(TAG, "========axp173 info=========");
    ESP_LOGI(TAG, "acin_exist: %d\t| vbus_exist: %d\t\t\t| bat_exist: %d",
        info->acin_exist, info->vbus_exist, info->bat_exist);
    ESP_LOGI(TAG, "temp: %s\t| charge_idct: %s\t| bat_current_dir: %s",
        info->axp_over_temp ? "over temp" : "normal",
        info->charge_idct ? "charging" : "discharge",
        info->bat_current_dir ? "in" : "out");

    ESP_LOGI(TAG, "EXTEN: %d\t\t| DCDC1: %d\t\t\t| DCDC2: %d",
        info->exten, info->dcdc1, info->dcdc2);
    ESP_LOGI(TAG, "LDO2: %d\t\t| LDO3: %d\t\t\t| LDO4: %d",
        info->ldo2, info->ldo3, info->ldo4);
}

uint8_t axp_op_generate_byte(bool dc1, bool dc2, bool exten, bool ldo2, bool ldo3, bool ldo4){
    uint8_t ret = 0;
    if(dc1) ret |= (1<<0);
    if(dc2) ret |= (1<<4);
    if(ldo2) ret |= (1<<2);
    if(ldo3) ret |= (1<<3);
    if(ldo4) ret |= (1<<1);
    return ret;
}
