#include "st7789.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// #include "driver/spi_master.h"
#include "spi_bus.h"
#include "st7789_reg.h"
// #include "lcd_bl.h"

#include "driver/gpio.h"
#include "esp_log.h"
#define TAG "st7789.h"


typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes;  //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;




static void IRAM_ATTR spi_ready(spi_transaction_t *trans);

static void st7789_send_cmd(uint8_t cmd);
static void st7789_send_data(void * data, uint16_t length);
static void st7789_send_color(void * data, size_t length);
static void st7789_set_orientation(uint8_t orientation);
// static void st7789_enable_backlight(bool backlight);



void st7789_init(void){

    ESP_LOGI(TAG, "Display buffer size: %d", DISP_BUF_SIZE);
    ESP_LOGI(TAG, "HOR_MAX: %d, VER_MAX: %d",LV_HOR_RES_MAX, LV_VER_RES_MAX);

    ESP_LOGI(TAG, "Initializing SPI master for display");
    
    // st7789_spi_add_device(ST7789_SPI_HOST);     // 在spi bus上添加st7789设备
    spi_st7789_init(SPI_HOST_ID, spi_ready);

    lcd_init_cmd_t st7789_init_cmds[] = {
        {0xCF, {0x00, 0x83, 0X30}, 3},
        {0xED, {0x64, 0x03, 0X12, 0X81}, 4},
        {ST7789_PWCTRL2, {0x85, 0x01, 0x79}, 3},
        {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
        {0xF7, {0x20}, 1},
        {0xEA, {0x00, 0x00}, 2},
        {ST7789_LCMCTRL, {0x26}, 1},
        {ST7789_IDSET, {0x11}, 1},
        {ST7789_VCMOFSET, {0x35, 0x3E}, 2},
        {ST7789_CABCCTRL, {0xBE}, 1},
        {ST7789_MADCTL, {0x00}, 1}, // Set to 0x28 if your display is flipped
        {ST7789_COLMOD, {0x55}, 1},

#if LCD_INVERT_COLORS == 1
		{ST7789_INVON, {0}, 0}, // set inverted mode
#else
 		{ST7789_INVOFF, {0}, 0}, // set non-inverted mode
#endif

        {ST7789_RGBCTRL, {0x00, 0x1B}, 2},
        {0xF2, {0x08}, 1},
        {ST7789_GAMSET, {0x01}, 1},
        {ST7789_PVGAMCTRL, {0xD0, 0x00, 0x02, 0x07, 0x0A, 0x28, 0x32, 0x44, 0x42, 0x06, 0x0E, 0x12, 0x14, 0x17}, 14},
        {ST7789_NVGAMCTRL, {0xD0, 0x00, 0x02, 0x07, 0x0A, 0x28, 0x31, 0x54, 0x47, 0x0E, 0x1C, 0x17, 0x1B, 0x1E}, 14},
        {ST7789_CASET, {0x00, 0x00, 0x00, 0xEF}, 4},
        {ST7789_RASET, {0x00, 0x00, 0x01, 0x3f}, 4},
        {ST7789_RAMWR, {0}, 0},
        {ST7789_GCTRL, {0x07}, 1},
        {0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},
        {ST7789_SLPOUT, {0}, 0x80},
        {ST7789_DISPON, {0}, 0x80},
        {0, {0}, 0xff},
    };


    //Initialize non-SPI GPIOs
    gpio_pad_select_gpio(LCD_PIN_DC);
    gpio_set_direction(LCD_PIN_DC, GPIO_MODE_OUTPUT);

    //背光初始化在encoder组件中
    // gpio_pad_select_gpio(ST7789_PIN_BLK);
    // gpio_set_direction(ST7789_PIN_BLK, GPIO_MODE_OUTPUT);

    // st7789_enable_backlight(false);

#if !LCD_SOFTWARE_RST
    gpio_pad_select_gpio(LCD_PIN_RST);
    gpio_set_direction(LCD_PIN_RST, GPIO_MODE_OUTPUT);

    gpio_set_level(LCD_PIN_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(LCD_PIN_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);

#elif LCD_SOFTWARE_RST
    st7789_send_cmd(ST7789_SWRESET);
#endif

    

    ESP_LOGI(TAG, "ST7789 initialization.");

    //Send all the commands
    uint16_t cmd = 0;
    while (st7789_init_cmds[cmd].databytes!=0xff) {
        st7789_send_cmd(st7789_init_cmds[cmd].cmd);
        st7789_send_data(st7789_init_cmds[cmd].data, st7789_init_cmds[cmd].databytes&0x1F);
        if (st7789_init_cmds[cmd].databytes & 0x80) {
                vTaskDelay(100 / portTICK_RATE_MS);
        }
        cmd++;
    }

    st7789_set_orientation(LCD_ORIENTATION);
    // st7789_enable_backlight(true);
    // lcd_bl_set(LCD_DEFAULT_BRIGHTNESS);
}


/* The ST7789 display controller can drive up to 320*240 displays, when using a 240*240 or 240*135
 * displays there's a gap of 80px or 40/52/53px respectively. 52px or 53x offset depends on display orientation.
 * We need to edit the coordinates to take into account those gaps, this is not necessary in all orientations. */
void st7789_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map) {
    uint8_t data[4] = {0};

    uint16_t offsetx1 = area->x1;
    uint16_t offsetx2 = area->x2;
    uint16_t offsety1 = area->y1;
    uint16_t offsety2 = area->y2;

#if (LV_TFT_DISP_OFFSETS)
    offsetx1 += LV_TFT_DISP_X_OFFSETS;
    offsetx2 += LV_TFT_DISP_X_OFFSETS;
    offsety1 += LV_TFT_DISP_Y_OFFSETS;
    offsety2 += LV_TFT_DISP_Y_OFFSETS;

#elif (LV_HOR_RES_MAX == 240) && (LV_VER_RES_MAX == 240)
    #if (LV_DISP_ORIENTATION == 0)          //PORTRAIT
        offsetx1 += 80;
        offsetx2 += 80;
    #elif (LV_DISP_ORIENTATION == 3)        //LANDSCAPE_INVERTED
        offsety1 += 80;
        offsety2 += 80;
    #endif
#elif (LV_HOR_RES_MAX == 240) && (LV_VER_RES_MAX == 135)
    #if (LV_DISP_ORIENTATION == 0) || \     //PORTRAIT
        (LV_DISP_ORIENTATION == 1)          //PORTRAIT_INVERTED
        offsetx1 += 40;
        offsetx2 += 40;
        offsety1 += 53;
        offsety2 += 53;
    #endif
#elif (LV_HOR_RES_MAX == 135) && (LV_VER_RES_MAX == 240)
    #if (LV_DISP_ORIENTATION == 2) || \     //LANDSCAPE
        (LV_DISP_ORIENTATION == 3)          //LANDSCAPE_INVERTED
        offsetx1 += 52;
        offsetx2 += 52;
        offsety1 += 40;
        offsety2 += 40;
    #endif
#endif

    /*Column addresses*/
    st7789_send_cmd(ST7789_CASET);
    data[0] = (offsetx1 >> 8) & 0xFF;
    data[1] = offsetx1 & 0xFF;
    data[2] = (offsetx2 >> 8) & 0xFF;
    data[3] = offsetx2 & 0xFF;
    st7789_send_data(data, 4);

    /*Page addresses*/
    st7789_send_cmd(ST7789_RASET);
    data[0] = (offsety1 >> 8) & 0xFF;
    data[1] = offsety1 & 0xFF;
    data[2] = (offsety2 >> 8) & 0xFF;
    data[3] = offsety2 & 0xFF;
    st7789_send_data(data, 4);

    /*Memory write*/
    st7789_send_cmd(ST7789_RAMWR);

    size_t size = (size_t)lv_area_get_width(area) * (size_t)lv_area_get_height(area);

    st7789_send_color((void*)color_map, size * 2);
}




/**
 * @brief ST7789发送控制命令
 * 
 * @param cmd 
 */
void st7789_send_cmd(uint8_t cmd) {
    spi_st7789_wait_for_pending();
    gpio_set_level(LCD_PIN_DC, 0);
    spi_st7789_transaction(&cmd, 1, SPI_BUS_SEND_POLLING, NULL, 0, 0);
}

/**
 * @brief ST7789发送数据
 * 
 * @param data 
 * @param length 
 */
void st7789_send_data(void * data, uint16_t length) {
    spi_st7789_wait_for_pending();
    gpio_set_level(LCD_PIN_DC, 1);
    spi_st7789_transaction(data, length, SPI_BUS_SEND_POLLING, NULL, 0, 0);
}

/**
 * @brief 设置st7789颜色
 * 
 * @param data 
 * @param length 
 */
void st7789_send_color(void * data, size_t length) {
    spi_st7789_wait_for_pending();
    gpio_set_level(LCD_PIN_DC, 1);
    spi_st7789_transaction(data, length, SPI_BUS_SEND_QUEUED|SPI_BUS_SIGNAL_FLUSH, NULL, 0, 0);
}

/**
 * @brief 设置st7789显示方向
 * 
 * @param orientation 
 */
void st7789_set_orientation(uint8_t orientation) {
    // ESP_ASSERT(orientation < 4);
    assert(orientation < 4);

    const char *orientation_str[] = {
        "PORTRAIT", "PORTRAIT_INVERTED", "LANDSCAPE", "LANDSCAPE_INVERTED"
    };

    ESP_LOGI(TAG, "Display orientation: %s", orientation_str[orientation]);

    uint8_t data[] = {0xC0, 0x00, 0x60, 0xA0};

    ESP_LOGI(TAG, "0x36 command value: 0x%02X", data[orientation]);
    
    st7789_send_cmd(ST7789_MADCTL);
    st7789_send_data((void *) &data[orientation], 1);
}

/**
 * @brief st7789开启背光
 * 
 * @param backlight 
 */
void st7789_enable_backlight(bool backlight) {
    ESP_LOGI(TAG,"%s backlight.\n", backlight ? "Enabling" : "Disabling");
    uint32_t tmp = 0;
    tmp = backlight ? 1 : 0;

    gpio_set_level(LCD_PIN_BL, tmp);
}




static void IRAM_ATTR spi_ready(spi_transaction_t *trans){
    spi_bus_send_flag_t flags = (spi_bus_send_flag_t) trans->user;

    if (flags & SPI_BUS_SIGNAL_FLUSH) {
        lv_disp_t * disp = NULL;
        disp = _lv_refr_get_disp_refreshing();
        lv_disp_flush_ready(disp->driver);
    }
}