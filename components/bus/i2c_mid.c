/**
 * @file i2c_mid.c
 * @author Bosco's git (1270112821@qq.com)
 * @brief 提供i2c初始化函数，为i2c设备提供使用函数
 * @version 0.1
 * @date 2023-03-19
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "i2c_mid.h"
#include "i2c_bus.h"
// #include "spi_bus.h"
// #include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "esp_log.h"
#define TAG "bus_mid"




i2c_bus_handle_t i2c_handle = NULL;
i2c_bus_device_handle_t i2c_axp_handle = NULL;
i2c_bus_device_handle_t i2c_touch_handle = NULL;



esp_err_t g_i2c_init(){
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_PIN_SDA,
        .sda_pullup_en = I2C_PULLUP_SDA,
        .scl_io_num = I2C_PIN_SCL,
        .scl_pullup_en = I2C_PULLUP_SCL,
        .master.clk_speed = I2C_SPEED_HZ,
    };
    i2c_handle = i2c_bus_create(I2C_PORT, &conf);
    
    // uint8_t addr[3];
    // i2c_bus_scan(i2c_handle, addr, 3);
    // ESP_LOGI(TAG, "axp173 addr : 0x%02x", addr[0]);

    i2c_axp_handle = i2c_bus_device_create(i2c_handle, AXP_I2C_ADDR, I2C_SPEED_HZ);

    i2c_touch_handle = i2c_bus_device_create(i2c_handle, FT6236_I2C_ADDR, I2C_SPEED_HZ);
    assert(i2c_touch_handle != NULL);

    return ESP_OK;
}

esp_err_t axp_read_byte(uint8_t reg, uint8_t *data){
    return i2c_bus_read_bytes(i2c_axp_handle, reg, 1, data);
}

esp_err_t axp_read_bytes(uint8_t reg, uint8_t *data, size_t len){
    return i2c_bus_read_bytes(i2c_axp_handle, reg, len, data);
}

esp_err_t axp_write_byte(uint8_t reg, uint8_t data){
    return i2c_bus_write_bytes(i2c_axp_handle, reg, 1, &data);
}


esp_err_t ft6236_read_byte(uint8_t reg, uint8_t *data){
    return i2c_bus_read_byte(i2c_touch_handle, reg, data);
}

esp_err_t ft6236_read_bytes(uint8_t reg, uint8_t *data, size_t len){
    return i2c_bus_read_bytes(i2c_axp_handle, reg, len, data);
}