#include "spi_bus.h"
#include <string.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "exS3_conf.h"

#include "esp_log.h"
#define TAG "spi_bus.h"

/* ================spi bus================== */

void spi_bus_init(spi_host_device_t host){
    const char *spi_names[] = {"SPI1_HOST","SPI2_HOST","SPI3_HOST"};    // ESP32S3

    ESP_LOGI(TAG, "Configuring SPI host %s", spi_names[host]);
    ESP_LOGI(TAG, "MISO pin: %d, MOSI pin: %d, SCLK pin: %d, IO2/WP pin: %d, IO3/HD pin: %d", SPI_PIN_MISO, SPI_PIN_MOSI, SPI_PIN_CLK, SPI_PIN_WP, SPI_PIN_HOLD);
    ESP_LOGI(TAG, "Max transfer size: %d (bytes)", SPI_BUS_MAX_TRANSFER_SZ);


    spi_bus_config_t buscfg = {
        .miso_io_num = SPI_PIN_MISO,
        .mosi_io_num = SPI_PIN_MOSI,
        .sclk_io_num = SPI_PIN_CLK,
        .quadwp_io_num = SPI_PIN_WP,
        .quadhd_io_num = SPI_PIN_HOLD,
        .max_transfer_sz = SPI_BUS_MAX_TRANSFER_SZ,
    };

    esp_err_t ret = spi_bus_initialize(host, &buscfg, SPI_DMA_CH_AUTO);
    assert(ret == ESP_OK);
}


void spi_bus_deinit(spi_host_device_t host) {
    spi_bus_free(host);
}

/* ==================end==================== */


#define SPI_TRANSACTION_POOL_SIZE   50
#define SPI_TRANSACTION_POLL_RESERVE_PERCENTAGE 10
#if SPI_TRANSACTION_POOL_SIZE >= SPI_TRANSACTION_POOL_RESERVE_PERCENTAGE
#define SPI_TRANSACTION_POOL_RESERVE (SPI_TRANSACTION_POOL_SIZE / SPI_TRANSACTION_POLL_RESERVE_PERCENTAGE)	
#else
#define SPI_TRANSACTION_POOL_RESERVE 1	/* defines minimum size */
#endif



// static void IRAM_ATTR spi_ready(spi_transaction_t *t);
static spi_device_handle_t st7789_handle;
static QueueHandle_t transaction_pool = NULL;




void spi_st7789_init(spi_host_device_t host, transaction_cb_t spi_ready_cb){
    ESP_LOGI(TAG, "Adding SPI [st7789] device");
    ESP_LOGI(TAG, "Clock speed: %dMHz, mode: %d, CS pin: %d", LCD_SPEED_HZ, LCD_SPI_MODE, LCD_PIN_CS);

    spi_device_interface_config_t conf = {
        .clock_speed_hz = (LCD_SPEED_HZ*1000*1000),
        .mode = LCD_SPI_MODE,
        .spics_io_num = LCD_PIN_CS,
        .input_delay_ns = 0,
        .queue_size = SPI_TRANSACTION_POOL_SIZE,
        .pre_cb = NULL,
        .post_cb = spi_ready_cb,
    };
    spi_bus_add_device(host, &conf, &st7789_handle);

    if(transaction_pool == NULL){
        transaction_pool = xQueueCreate(SPI_TRANSACTION_POOL_SIZE, sizeof(spi_transaction_ext_t*));
        assert(transaction_pool != NULL);
		for (size_t i = 0; i < SPI_TRANSACTION_POOL_SIZE; i++)
		{
			spi_transaction_ext_t* pTransaction = (spi_transaction_ext_t*)heap_caps_malloc(sizeof(spi_transaction_ext_t), MALLOC_CAP_DMA);
			assert(pTransaction != NULL);
			memset(pTransaction, 0, sizeof(spi_transaction_ext_t));
			xQueueSend(transaction_pool, &pTransaction, portMAX_DELAY);
		}
    }
}

void spi_st7789_wait_for_pending(void){
    spi_transaction_t *presult;
    while (uxQueueMessagesWaiting(transaction_pool) < SPI_TRANSACTION_POOL_SIZE) {
        if(spi_device_get_trans_result(st7789_handle, &presult, 1) == ESP_OK) {
            xQueueSend(transaction_pool, &presult, portMAX_DELAY);
        }
    }
    
}

void spi_st7789_remove_device(void) {
    spi_st7789_wait_for_pending();
    esp_err_t ret = spi_bus_remove_device(st7789_handle);
    assert(ret = ESP_OK);
}

void spi_st7789_release(void) {
    spi_device_release_bus(st7789_handle);
}

void spi_st7789_acquire(void) {
    esp_err_t ret = spi_device_acquire_bus(st7789_handle, portMAX_DELAY);
    assert(ret == ESP_OK);
}

void spi_st7789_transaction(const uint8_t *data, size_t len, spi_bus_send_flag_t flags, uint8_t * out, uint64_t addr, uint8_t dummy_bits){
    if(0 == len) return;

    spi_transaction_ext_t t = {0};

    t.base.length = len * 8;        // len单位为byte，这里换算为bit


    // 数据长度小于4个字节，直接放入tx_data发送出去，否则放入缓冲区tx_buffer
    if(len <= 4 && data != NULL) {
        t.base.flags = SPI_TRANS_USE_TXDATA;
        memcpy(t.base.tx_data, data, len);
    }else {
        t.base.tx_buffer = data;
    }

    // 如果接受到数据
    if(flags & SPI_BUS_RECEIVE) {
        assert(out!=NULL && (flags & (SPI_BUS_SEND_POLLING|SPI_BUS_SEND_SYNCHRONOUS)));
        t.base.rx_buffer = out;
    }
    
    // 设置发送地址长度，8/16/24/32bit
    if(flags & SPI_BUS_ADDRESS_8){
        t.address_bits = 8;
    }else if(flags & SPI_BUS_ADDRESS_16){
        t.address_bits = 16;
    }else if(flags & SPI_BUS_ADDRESS_16){
        t.address_bits = 24;
    }else if(flags & SPI_BUS_ADDRESS_16){
        t.address_bits = 32;
    }
    
    // 设置发送地址
    if(t.address_bits){
        t.base.addr = addr;
        t.base.flags |= SPI_TRANS_VARIABLE_ADDR;
    }

    // 设置user_data
    t.base.user = (void *)flags;

    // 等待/发送数据
    if(flags & SPI_BUS_SEND_POLLING){
        spi_st7789_wait_for_pending();
        spi_device_polling_transmit(st7789_handle, (spi_transaction_t *)&t);
    }else if(flags & SPI_BUS_SEND_SYNCHRONOUS){
        spi_st7789_wait_for_pending();
        spi_device_transmit(st7789_handle, (spi_transaction_t *)&t);
    }else {
        if(uxQueueMessagesWaiting(transaction_pool) == 0){
            spi_transaction_t *presult;
            while(uxQueueMessagesWaiting(transaction_pool) < SPI_TRANSACTION_POOL_RESERVE){
                if(spi_device_get_trans_result(st7789_handle, &presult,1)==ESP_OK){
                    xQueueSend(transaction_pool, &presult, portMAX_DELAY);
                }
            }
        }
        spi_transaction_ext_t *pTransaction = NULL;
		xQueueReceive(transaction_pool, &pTransaction, portMAX_DELAY);
        memcpy(pTransaction, &t, sizeof(t));
        if (spi_device_queue_trans(st7789_handle, (spi_transaction_t *) pTransaction, portMAX_DELAY) != ESP_OK) {
			xQueueSend(transaction_pool, &pTransaction, portMAX_DELAY);	/* send failed transaction back to the pool to be reused */
        }
    }
}


/**
 *  移植disp_spi
 *  除去disp_dirver
 *  在main.c中初始化 st7789 和 ft6236
 * 
 */
