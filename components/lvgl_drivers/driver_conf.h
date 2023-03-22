#ifndef DRIVER_CONF_H
#define DRIVER_CONF_H


/* ---------------屏幕和spi------------------ */

/* disp_driver.c */
#define SPI_TFT_CLOCK_SPEED_HZ (40*1000*1000) //spi clock speed = 40MHz
#define SPI_TFT_SPI_MODE (2)

//引脚
#define DISP_SPI_CS     12
#define DISP_SPI_IO2    (-1)
#define DISP_SPI_IO3    (-1)
#define DISP_SPI_MISO   (-1)
#define DISP_SPI_INPUT_DELAY_NS (0)
#define DISP_SPI_MOSI   10
#define DISP_SPI_CLK    13

#define TFT_SPI_HOST SPI3_HOST

/* st7789.c */
#define ST7789_INVERT_COLORS    1  //颜色反转
#define LV_DISP_ORIENTATION     2  //画面旋转，0--portratit; 1--portratit inverted; 2--landscape; 3--landscape inverted

//画面偏移
#define LV_TFT_DISP_OFFSETS 0
#if (LV_TFT_DISP_OFFSETS)
    #define LV_TFT_DISP_X_OFFSETS 
    #define LV_TFT_DISP_Y_OFFSETS
#endif

//非spi引脚
#define ST7789_DC   21
// #define ST7789_SOFT_RST 0
#define ST7789_RST  11
#define ST7789_BLK  45

/* lvgl_helpers.h */
//水平最大分辨率
#ifndef LV_HOR_RES_MAX  
    #define LV_HOR_RES_MAX    (240)
#endif
//垂直最大分辨率
#ifndef LV_VER_RES_MAX  
    #define LV_VER_RES_MAX    (320)
#endif

#define CUSTOM_DISPLAY_BUFFER_SIZE 1
#if (CUSTOM_DISPLAY_BUFFER_SIZE)
    #define DISP_BUF_SIZE 8192
#else
    #define DISP_BUF_SIZE (LV_HOR_RES_MAX * 40)
#endif


#define SPI_BUS_MAX_TRANSFER_SZ (DISP_BUF_SIZE * 2)




/* ---------------触屏和i2c------------------ */

/* 是否将坐标发送到队列 */
// #define LV_FT6X36_COORDINATES_QUEUE 0

/* I2C端口总线 */
#define LV_I2C_TOUCH_PORT   I2C_NUM_0

/* 坐标交换 */
#define LV_FT6X36_SWAPXY    0
#define LV_FT6X36_INVERT_X  0
#define LV_FT6X36_INVERT_Y  0

// #define PIN_I2C_SCL         47
// #define PIN_I2C_SDA         48
// #define I2C_PULLUP_EN       0
// #define I2C_SPEED_FREQ_HZ   200000  //范围 100000 ~ 500000

// #define RW_TIMEOUT          20      //读写操作的超时时间，不包括等待锁定的时间：范围：10~1000
// #define LOCK_TIMEOUT        50      //旧锁被覆盖的时间   10~1000

#endif
