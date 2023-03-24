#ifndef _I2C_BUS_H_
#define _I2C_BUS_H_

#include "driver/i2c.h"


esp_err_t i2c_bus_init(i2c_port_t port);
esp_err_t i2c_bus_deinit(i2c_port_t port);

esp_err_t i2c_bus_read_bytes(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t *data, size_t len);
esp_err_t i2c_bus_read_byte(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t *data);
esp_err_t i2c_bus_read_bit(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t bit_num, uint8_t *data);
esp_err_t i2c_bus_read_bits(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t bit_start, uint8_t bit_len, uint8_t *data);

esp_err_t i2c_bus_write_bytes(i2c_port_t port, uint8_t addr, uint8_t reg, const uint8_t *data, size_t len);
esp_err_t i2c_bus_write_byte(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t data);
esp_err_t i2c_bus_write_bit(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t bit_num, uint8_t data);
esp_err_t i2c_bus_write_bits(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t bit_start, uint8_t bit_len, uint8_t data);





#endif

