#ifndef _SPI_BUS_H_
#define _SPI_BUS_H_

#include <stdint.h>
#include <stdbool.h>
#include "driver/spi_master.h"



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
