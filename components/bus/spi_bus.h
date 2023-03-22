#ifndef _SPI_BUS_H_
#define _SPI_BUS_H_

#include <stdint.h>
#include <stdbool.h>
#include "driver/spi_master.h"

// #define SPI_PIN_MISO    14
// #define SPI_PIN_MOSI    10
// #define SPI_PIN_CLK     13
// #define SPI_PIN_WP      (-1)
// #define SPI_PIN_HOLD    (-1)
// #define SPI_MAX_TRANSFER_SZ     (8*1024*2)
// #define SPI_HOST_ID     SPI3_HOST

// #define TFT_PIN_CS      12
// #define TFT_PIN_RST     11
// #define TFT_PIN_BLK     45
// #define TFT_SPI_MODE    2   // ?
// #define TFT_SPEED_HZ    (40 * 1000 * 1000)

#define SPI_PIN_MISO    (-1)    // SPI master in slave out引脚(=spi_q)
#define SPI_PIN_MOSI    10      // SPI master out slave in引脚(=spi_d)
#define SPI_PIN_CLK     13      // SPI clock引脚
#define SPI_PIN_WP      (-1)    // SPI 写保护引脚
#define SPI_PIN_HOLD    (-1)    // SPI HOLD引脚

#define SPI_BUS_MAX_TRANSFER_SZ   (8192*2)
#define SPI_HOST_ID     SPI3_HOST

#define LCD_PIN_RST     11
#define LCD_PIN_BLK     45
#define LCD_PIN_CS      12
#define LCD_SPI_MODE    2   // ?
#define LCD_SPEED_HZ    40
// #define LCD_SPEED_HZ    (LCD_CLOCK_HZ * 1000 * 1000)  //40MHz
// #define LCD_INVERT_COLOR    1
// #define LCD_ORIENTATION 2



typedef enum {
    SPI_BUS_SEND_QUEUED         = 0x00000000,
    SPI_BUS_SEND_POLLING        = 0x00000001,
    SPI_BUS_SEND_SYNCHRONOUS    = 0x00000002,
    SPI_BUS_SIGNAL_FLUSH        = 0x00000004,
    SPI_BUS_RECEIVE             = 0x00000008,
    SPI_BUS_CMD_8               = 0x00000010, /* Reserved */
    SPI_BUS_CMD_16              = 0x00000020, /* Reserved */
    SPI_BUS_ADDRESS_8           = 0x00000040, 
    SPI_BUS_ADDRESS_16          = 0x00000080, 
    SPI_BUS_ADDRESS_24          = 0x00000100, 
    SPI_BUS_ADDRESS_32          = 0x00000200, 
    SPI_BUS_MODE_DIO            = 0x00000400, 
    SPI_BUS_MODE_QIO            = 0x00000800, 
    SPI_BUS_MODE_DIOQIO_ADDR    = 0x00001000, 
	SPI_BUS_VARIABLE_DUMMY		= 0x00002000,
} spi_bus_send_flag_t;


// spi bus
void spi_bus_init(spi_host_device_t host);
void spi_bus_deinit(spi_host_device_t host);


// spi device
void spi_st7789_init(spi_host_device_t host, transaction_cb_t spi_ready_cb);
void spi_st7789_wait_for_pending(void);
void spi_st7789_remove_device(void);
void spi_st7789_release(void);
void spi_st7789_acquire(void);
void spi_st7789_transaction(const uint8_t *data, size_t len, spi_bus_send_flag_t flags, uint8_t * out, uint64_t addr, uint8_t dummy_bits);



#endif
