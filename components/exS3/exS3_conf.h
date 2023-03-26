#ifndef _EXS3_CONF_H_
#define _EXS3_CONF_H_

/* exS3板级组件配置：电源管理、屏幕、触摸，及其他组件 */


/*********************
 *       I2C
 *********************/
#define I2C_PIN_SDA         48
#define I2C_PIN_SCL         47
#define I2C_PULLUP_SCL      0
#define I2C_PULLUP_SDA      0
#define I2C_SPEED_FREQ_HZ   200000
#define I2C_PORT            I2C_NUM_0


/*********************
 *       SPI
 *********************/
#define SPI_PIN_MISO    (-1)    // SPI master in slave out引脚(=spi_q)
#define SPI_PIN_MOSI    10      // SPI master out slave in引脚(=spi_d)
#define SPI_PIN_CLK     13      // SPI clock引脚
#define SPI_PIN_WP      (-1)    // SPI 写保护引脚
#define SPI_PIN_HOLD    (-1)    // SPI HOLD引脚

#define SPI_BUS_MAX_TRANSFER_SZ   (8192*2)
#define SPI_HOST_ID     SPI3_HOST

#define LCD_PIN_RST     11
// #define LCD_PIN_BLK     45
#define LCD_PIN_CS      12
#define LCD_SPI_MODE    2   // ?
#define LCD_SPEED_HZ    40
// #define LCD_SPEED_HZ    (LCD_CLOCK_HZ * 1000 * 1000)  //40MHz
// #define LCD_INVERT_COLOR    1
// #define LCD_ORIENTATION 2

#define LCD_DEFAULT_BRIGHTNESS  50
#define LCD_BRIGHTNESS_STEP     4

/*********************
 *    TOUCH: FT6236
 *********************/
#define FT6236_I2C_PORT I2C_PORT
#define FT6236_I2C_ADDR (0x38)

/*********************
 *    POWER: AXP173
 *********************/

#define AXP_PIN_IRQ     39
#define AXP_I2C_PORT    I2C_PORT
#define AXP_I2C_ADDR    (0x34)


/*********************
 *    LCD: ST7789
 *********************/
#define LCD_PIN_RST     11
#define LCD_PIN_BL      45
#define LCD_PIN_DC      21

#define LCD_INVERT_COLORS   1
#define LCD_SOFTWARE_RST    0

#define ORIENTATION_PORTRAIT   0               // 竖向
#define ORIENTATION_PORTRAIT_INVERTED   1      // 竖向反转
#define ORIENTATION_LANDSCAPE   2               // 横向
#define ORIENTATION_LANDSCAPE_INVERTED   3      // 横向反转
#define LCD_ORIENTATION     ORIENTATION_LANDSCAPE


#ifndef LV_HOR_RES_MAX      //水平最大分辨率
    #define LV_HOR_RES_MAX    (240)
#endif

#ifndef LV_VER_RES_MAX      //垂直最大分辨率
    #define LV_VER_RES_MAX    (320)
#endif


#define CUSTOM_DISPLAY_BUFFER_SIZE 1
#if (CUSTOM_DISPLAY_BUFFER_SIZE)
    #define DISP_BUF_SIZE 1024*10
#else
    #define DISP_BUF_SIZE (LV_HOR_RES_MAX * 40)
#endif



#define LCD_BL_MODE 1   // 1:pwm mode; 0:gpio mode
#if (LCD_BL_MODE)
    #define LCD_BL_PWM_TIMER    LEDC_TIMER_0
    #define LCD_BL_PWM_SPEED_MODE     LEDC_LOW_SPEED_MODE
    #define LCD_BL_PWM_CHANNEL  LEDC_CHANNEL_0
    #define LCD_BL_PWM_DUTY_RES LEDC_TIMER_10_BIT
    #define LCD_BL_PWM_DUTY     (0)
    #define LCD_BL_PWM_FREQ     (5000)  // 5kHz
#else
    #define LCD_BL_ACTIVE_LEVEL 0
#endif


/*********************
 *      Unity
 *********************/

#define LED_PIN     1

#define MOTOR_PIN   38

#define ECD_PIN_A   8
#define ECD_PIN_B   6
#define ECD_PIN_BTN 15

#define ECD_PCNT_UNIT_NUM    PCNT_UNIT_0

#define ECD_BTN_ACTIVE_LEVEL    0
// button
#define DEBOUNCE_TIME   8*1000          //消抖时间8ms
#define SHORT_TIME      800*1000        //短按时间阈值800ms
#define LONG_TIME       2000*1000       //长按时间阈值1s

// 
#define DEFAULT_MOTOR_CLICK_WORKTIME 80     // 点击操作马达震动时长 ms


#endif