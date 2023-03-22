#include "i2c_bus.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/i2c.h"

#define TAG "i2c_bus"

#define I2C_ACK_CHECK_EN            0x1
#define I2C_ACK_CHECK_DIS           0x0 

#define I2C_BUS_MS_TO_WAIT          20
#define I2C_BUS_TICKS_TO_WAIT       (I2C_BUS_MS_TO_WAIT/portTICK_RATE_MS)   //读写等待时间
#define I2C_BUS_MUTEX_TICKS_TO_WAIT (I2C_BUS_MS_TO_WAIT/portTICK_RATE_MS)   //上锁等待时间


i2c_config_t bus_conf;
SemaphoreHandle_t i2c_mutex = 0;
bool busIsInit;


/* check bus */
#define I2C_BUS_CHECK(a, str, ret) if(!(a)) { \
        ESP_LOGE(TAG,"%s:%d (%s):%s", __FILE__, __LINE__, __FUNCTION__, str); \
        return (ret); \
    }

/* bus init check */
#define I2C_BUS_INIT_CHECK(is_init, ret) if(!is_init) { \
        ESP_LOGE(TAG,"%s:%d (%s):i2c_bus has not inited", __FILE__, __LINE__, __FUNCTION__); \
        return (ret); \
    }

/* bus take mutex */
#define I2C_BUS_MUTEX_TAKE(mutex, ret) if (!xSemaphoreTake(mutex, I2C_BUS_MUTEX_TICKS_TO_WAIT)) { \
        ESP_LOGE(TAG, "i2c_bus take mutex timeout, max wait = %d ms", I2C_BUS_MUTEX_TICKS_TO_WAIT); \
        return (ret); \
    }

/* bus take mutex(more delay) */
#define I2C_BUS_MUTEX_TAKE_MAX_DELAY(mutex, ret) if (!xSemaphoreTake(mutex, portMAX_DELAY)) { \
        ESP_LOGE(TAG, "i2c_bus take mutex timeout, max wait = %d ms", portMAX_DELAY); \
        return (ret); \
    }

/* bus give mutex */
#define I2C_BUS_MUTEX_GIVE(mutex, ret) if (!xSemaphoreGive(mutex)) { \
        ESP_LOGE(TAG, "i2c_bus give mutex failed"); \
        return (ret); \
    }

static esp_err_t i2c_bus_read_reg8(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t *data, size_t len);
static esp_err_t i2c_bus_write_reg8(i2c_port_t port, uint8_t addr, uint8_t reg, const uint8_t *data, size_t len);

/********************* App Functions **********************/

//最原来
esp_err_t i2c_bus_init(i2c_port_t port){
    esp_err_t ret = ESP_OK;

    //通过mutex判断是否初始化    
    if(i2c_mutex == 0){
        i2c_mutex = xSemaphoreCreateMutex();

        bus_conf.clk_flags = 0;
        bus_conf.sda_io_num = I2C_PIN_SDA;
        bus_conf.scl_io_num = I2C_PIN_SCL;
        bus_conf.scl_pullup_en = I2C_PULLUP_SCL;
        bus_conf.sda_pullup_en = I2C_PULLUP_SDA;
        bus_conf.master.clk_speed = I2C_SPEED_FREQ_HZ;
        bus_conf.mode = I2C_MODE_MASTER;

        ret = i2c_param_config(port, &bus_conf);
        ret |= i2c_driver_install(port, bus_conf.mode, 0, 0, 0);
        if(ret != ESP_OK){
            ESP_LOGE(TAG, "Failed to init I2C port %d.",(int)port);
        }else {
            busIsInit = true;
            ESP_LOGI(TAG, "I2C port %d init over!(SDA: %d, SCL: %d, SPEED: %d Hz.)",port, bus_conf.sda_io_num, bus_conf.scl_io_num, bus_conf.master.clk_speed);
        }
    }
    return ret;
}

esp_err_t i2c_bus_deinit(i2c_port_t port){
    I2C_BUS_CHECK(busIsInit == true, "bus not inited", ESP_ERR_INVALID_STATE);
    vSemaphoreDelete(i2c_mutex);
    i2c_mutex = NULL;
    ESP_LOGI(TAG, "I2C port %d deinit", port);
    busIsInit = false;
    return i2c_driver_delete(port);
}



/* i2c read */
/**
 * @brief i2c读取多个字节
 * 
 * @param port  i2c总线端口号，I2C_NUM_0或I2C_NUM_1
 * @param addr  设备地址
 * @param reg   寄存器
 * @param data  读取后返回的数据
 * @param len   读取的长度
 * @return esp_err_t 
 */
esp_err_t i2c_bus_read_bytes(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t *data, size_t len){
    return i2c_bus_read_reg8(port, addr, reg, data, len);
}

/**
 * @brief i2c读取单个字节
 * 
 * @param port  i2c总线端口号，I2C_NUM_0或I2C_NUM_1
 * @param addr  设备地址
 * @param reg   寄存器
 * @param data  读取后返回的数据
 * @return esp_err_t 
 */
esp_err_t i2c_bus_read_byte(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t *data){
    return i2c_bus_read_reg8(port, addr, reg, data, 1);
}

/**
 * @brief i2c读取单个位
 * 
 * @param port      i2c总线端口号，I2C_NUM_0或I2C_NUM_1
 * @param addr      设备地址
 * @param reg       寄存器
 * @param bit_num   待读取的位
 * @param data      读取后返回的数据
 * @return esp_err_t 
 */
esp_err_t i2c_bus_read_bit(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t bit_num, uint8_t *data){
    uint8_t byte = 0;
    esp_err_t ret = i2c_bus_read_reg8(port, addr, reg, &byte, 1);
    
    *data = byte & (1<<bit_num);    //将1左移bit_num位，与读取到的byte做与或操作，并赋值给 *data ， 如果读取的bit为1， *data将不为0， 
    *data = (*data != 0) ? 1 : 0;
    return ret;
}

/**
 * @brief i2c读取多个位
 * 
 * @param port      i2c总线端口号，I2C_NUM_0或I2C_NUM_1
 * @param addr      设备地址
 * @param reg       寄存器
 * @param bit_start 开始读取bit的位置
 * @param bit_len   要读取bit的长度
 * @param data      读取后返回的数据
 * @return esp_err_t 
 */
esp_err_t i2c_bus_read_bits(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t bit_start, uint8_t bit_len, uint8_t *data){
    uint8_t byte = 0;
    esp_err_t ret = i2c_bus_read_reg8(port, addr, reg, &byte, 1);
    if(ret != ESP_OK){
        return ret;
    }

    uint8_t mask = ((1 << bit_len) - 1) << (bit_start - bit_len + 1);
    byte &= mask;   //取要读取的位置的bit数据
    byte >>= (bit_start - bit_len + 1);  //将数据移动到第一位  
    *data = byte; 
    return ret;

}

/* i2c write */
/**
 * @brief i2c写入多个字节
 * 
 * @param port  i2c总线端口号，I2C_NUM_0或I2C_NUM_1
 * @param addr  设备地址
 * @param reg   寄存器
 * @param data  待写入数据
 * @param len   待写入数据的长度
 * @return esp_err_t 
 */
esp_err_t i2c_bus_write_bytes(i2c_port_t port, uint8_t addr, uint8_t reg, const uint8_t *data, size_t len){
    return i2c_bus_write_reg8(port, addr, reg, data, len);
}

/**
 * @brief i2c写入单个字节
 * 
 * @param port  i2c总线端口号，I2C_NUM_0或I2C_NUM_1
 * @param addr  设备地址
 * @param reg   寄存器
 * @param data  待写入数据
 * @return esp_err_t 
 */
esp_err_t i2c_bus_write_byte(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t data){
    return i2c_bus_write_reg8(port, addr, reg, &data, 1);
}

/**
 * @brief i2c写入单个位
 * 
 * @param port  i2c总线端口号，I2C_NUM_0或I2C_NUM_1
 * @param addr  设备地址
 * @param reg   寄存器
 * @param bit_num 待写入的位
 * @param data  写入的数据 1/0
 * @return esp_err_t 
 */
esp_err_t i2c_bus_write_bit(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t bit_num, uint8_t data){
    uint8_t byte = 0;
    esp_err_t ret = i2c_bus_read_reg8(port, addr, reg, &byte, 1);
    if(ret != ESP_OK){
        return ret;
    }
    byte = (data != 0) ? (byte | (1 << bit_num)) : (byte & ~(1 << bit_num));

    return i2c_bus_write_byte(port, addr, reg, byte);
}

/**
 * @brief i2c写入多个位
 * 
 * @param port      i2c总线端口号，I2C_NUM_0或I2C_NUM_1
 * @param addr      设备地址
 * @param reg       寄存器
 * @param bit_start 开始写入bit的位置
 * @param bit_len   要写入bit的长度
 * @param data      待写入数据
 * @return esp_err_t 
 */
esp_err_t i2c_bus_write_bits(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t bit_start, uint8_t bit_len,uint8_t data){
    uint8_t byte = 0;
    esp_err_t ret = i2c_bus_read_reg8(port, addr, reg, &byte, 1);
    if(ret != ESP_OK){
        return ret;
    }

    uint8_t mask = ((1 << bit_len) - 1) << (bit_start - bit_len + 1);   
    data <<= (bit_start - bit_len + 1);     //把想要写入的数据移动到正确位置
    data &= mask;       //把data其他重要的数据置零
    byte &= ~(mask);    //把原来数据里要写入的数据位置清零
    byte |= data;       //合并两个数据
    return i2c_bus_write_reg8(port, addr, reg, &byte, 1);
}


/********************* Driver Functions **********************/
static esp_err_t i2c_bus_read_reg8(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t *data, size_t len){
    
    esp_err_t ret;
    // i2c_bus_init(port);

    I2C_BUS_INIT_CHECK(busIsInit, ESP_ERR_INVALID_STATE);
    /* 上锁 */
    I2C_BUS_MUTEX_TAKE(i2c_mutex, ESP_ERR_TIMEOUT);
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();   //创建cmd
    
    if(reg != NULL_I2C_REG_ADDR){
        i2c_master_start(cmd);
        /* send address */
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, I2C_ACK_CHECK_EN);
        /* send register */
        i2c_master_write_byte(cmd, reg, I2C_ACK_CHECK_EN);
    }   //判断是否有寄存器地址

    /* 读取 */
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, I2C_ACK_CHECK_EN);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(port, cmd , I2C_BUS_TICKS_TO_WAIT);  //发送cmd
    i2c_cmd_link_delete(cmd);   //删除命令

    /* 解锁 */
    I2C_BUS_MUTEX_GIVE(i2c_mutex, ESP_FAIL);
    return ret;
}

static esp_err_t i2c_bus_write_reg8(i2c_port_t port, uint8_t addr, uint8_t reg, const uint8_t *data, size_t len){
    
    esp_err_t ret;

    I2C_BUS_INIT_CHECK(busIsInit, ESP_ERR_INVALID_STATE);

    ESP_LOGV(TAG, "Writing port %d, addr 0x%03x, reg 0x%04x", port, addr, reg);
    // i2c_bus_init(port);

    /* 上锁 */
    I2C_BUS_MUTEX_TAKE(i2c_mutex, ESP_ERR_TIMEOUT);

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();   //创建cmd
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, I2C_ACK_CHECK_EN);

    //判断是否有寄存器地址
    if(reg != NULL_I2C_REG_ADDR){
        /* send register */
        i2c_master_write_byte(cmd, reg, I2C_ACK_CHECK_EN);
    }

    /* 读取 */
    i2c_master_write(cmd, (uint8_t *)data, len, I2C_ACK_CHECK_EN);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(port, cmd , I2C_BUS_TICKS_TO_WAIT);  //发送cmd
    i2c_cmd_link_delete(cmd);   //删除命令

    /* 解锁 */
    I2C_BUS_MUTEX_GIVE(i2c_mutex, ESP_FAIL);
    return ret;
}




