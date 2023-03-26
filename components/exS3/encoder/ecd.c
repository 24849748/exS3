#include "ecd.h"

#include "exS3_conf.h"
#include "esp_log.h"

#define TAG "ecd"

//#define ECD_DEBUG

#define EVENT_QUEUE_LENGTH 1

#define TABLE_ROWS 7

#define DIR_NONE 0x0   // No complete step yet.
#define DIR_CW   0x10  // Clockwise step.
#define DIR_CCW  0x20  // Anti-clockwise step.


// Create the half-step state table (emits a code at 00 and 11)
#define R_START       0x0
#define H_CCW_BEGIN   0x1
#define H_CW_BEGIN    0x2
#define H_START_M     0x3
#define H_CW_BEGIN_M  0x4
#define H_CCW_BEGIN_M 0x5
static uint8_t _ttable_half[TABLE_ROWS][TABLE_COLS] = {
    // 00                  01              10            11                   // BA
    {H_START_M,            H_CW_BEGIN,     H_CCW_BEGIN,  R_START},            // R_START (00)
    {H_START_M | DIR_CCW,  R_START,        H_CCW_BEGIN,  R_START},            // H_CCW_BEGIN
    {H_START_M | DIR_CW,   H_CW_BEGIN,     R_START,      R_START},            // H_CW_BEGIN
    {H_START_M,            H_CCW_BEGIN_M,  H_CW_BEGIN_M, R_START},            // H_START_M (11)
    {H_START_M,            H_START_M,      H_CW_BEGIN_M, R_START | DIR_CW},   // H_CW_BEGIN_M
    {H_START_M,            H_CCW_BEGIN_M,  H_START_M,    R_START | DIR_CCW},  // H_CCW_BEGIN_M
};

// Create the full-step state table (emits a code at 00 only)
#define F_CW_FINAL  0x1
#define F_CW_BEGIN  0x2
#define F_CW_NEXT   0x3
#define F_CCW_BEGIN 0x4
#define F_CCW_FINAL 0x5
#define F_CCW_NEXT  0x6
static uint8_t _ttable_full[TABLE_ROWS][TABLE_COLS] = {
    // 00        01           10           11                  // BA
    {R_START,    F_CW_BEGIN,  F_CCW_BEGIN, R_START},           // R_START
    {F_CW_NEXT,  R_START,     F_CW_FINAL,  R_START | DIR_CW},  // F_CW_FINAL
    {F_CW_NEXT,  F_CW_BEGIN,  R_START,     R_START},           // F_CW_BEGIN
    {F_CW_NEXT,  F_CW_BEGIN,  F_CW_FINAL,  R_START},           // F_CW_NEXT
    {F_CCW_NEXT, R_START,     F_CCW_BEGIN, R_START},           // F_CCW_BEGIN
    {F_CCW_NEXT, F_CCW_FINAL, R_START,     R_START | DIR_CCW}, // F_CCW_FINAL
    {F_CCW_NEXT, F_CCW_FINAL, F_CCW_BEGIN, R_START},           // F_CCW_NEXT
};


#define _MERGE_DIR_POS(pos, dir)    ((pos<<8) | dir)
#define _GET_POS(merge)             (merge >> 8)
#define _GET_DIR(merge)             (merge & 0xff)

// table_row_t * table;
// uint8_t table_state;
// volatile ecd_state_t state;
// QueueHandle_t ecd_queue = NULL;

static uint8_t _process(ecd_t *ecd) {
    uint8_t event = 0;
    
    uint8_t pin_state = 0;

    if(!ecd->flip){
        pin_state = (gpio_get_level(ecd->pin_b) << 1) | gpio_get_level(ecd->pin_a);
    }else{
        pin_state = (gpio_get_level(ecd->pin_a) << 1) | gpio_get_level(ecd->pin_b);
    }
        // Determine new state from the pins and state table.
#ifdef ECD_DEBUG
        uint8_t old_state = table_state;
#endif

    ecd->table_state = ecd->table[ecd->table_state & 0xf][pin_state];
    // Return emit bits, i.e. the generated event.
    event = ecd->table_state & 0x30;

#ifdef ECD_DEBUG
        ESP_EARLY_LOGD(TAG, "BA %d%d, state 0x%02x, new state 0x%02x, event 0x%02x",
                    pin_state >> 1, pin_state & 1, old_state, table_state, event);
#endif

    return event;
}

static void _isr_rotenc(void * args) {
    ecd_t * ecd = (ecd_t *)args;

    uint8_t event = _process(ecd);
    bool send_event = false;
    if(!ecd->read_enable){
        return;
    }
    switch (event) {
    case DIR_CW:
        ++ecd->pos;
        ecd->dir = CW;
        send_event = true;
        break;
    case DIR_CCW:
        --ecd->pos;
        ecd->dir = CCW;
        send_event = true;
        break;
    default:
        break;
    }
    // int32_t tmp = (ecd->pos << 8) | ecd->dir;
    // int32_t tmp = _MERGE_DIR_POS(ecd->pos, ecd->dir);
    int8_t tmp = ecd->dir;
    if (send_event && ecd->queue) {
        BaseType_t task_woken = pdFALSE;
        xQueueOverwriteFromISR(ecd->queue, &tmp, &task_woken);
        if (task_woken) {
            portYIELD_FROM_ISR();
        }
        ecd->dir = NO_SET;  // reset direction
    }
}


ecd_t *ecd_create(gpio_num_t pin_a, gpio_num_t pin_b, bool half, bool flip)
{
    ecd_t * ecd = (ecd_t *)malloc(sizeof(ecd_t));
    if(!ecd){
        ESP_LOGE(TAG, "ecd_t malloc failed");
        return NULL;
    }

    ecd->pin_a = pin_a;
    ecd->pin_b = pin_b,
    ecd->table = half ? &_ttable_half[0] : &_ttable_full[0];
    ecd->table_state = R_START;
    ecd->flip = flip;
    ecd->pos = 0;
    ecd->dir = NO_SET;
    ecd->read_enable = true;

    ecd->queue = xQueueCreate(EVENT_QUEUE_LENGTH, sizeof(int32_t));
    if(ecd->queue == NULL){
        ESP_LOGE(TAG, "Encoder queue created failed");
        free(ecd);
        return NULL;
    }

    // configure GPIOs
    gpio_config_t conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << ECD_PIN_A) | (1ULL << ECD_PIN_B),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&conf));

    gpio_install_isr_service(0);

    gpio_isr_handler_add(ECD_PIN_A, _isr_rotenc, (void *)ecd);
    gpio_isr_handler_add(ECD_PIN_B, _isr_rotenc, (void *)ecd);

    return ecd;
}

esp_err_t ecd_deinit(ecd_t *ecd)
{
    // ecd_t *ecd = (ecd_t *)ecd_handle;
    if(ecd){
        gpio_isr_handler_remove(ecd->pin_a);
        gpio_isr_handler_remove(ecd->pin_b);
    }
    return ESP_OK;
}

int32_t ecd_get_position(ecd_t * ecd)
{
    // ecd_t *ecd = (ecd_t *)ecd_handle;
    return ecd->pos;
}

int8_t ecd_get_direction(ecd_t * ecd)
{
    // ecd_t *ecd = (ecd_t *)ecd_handle;
    return ecd->dir;
}

esp_err_t ecd_reset(ecd_t * ecd)
{
    // ecd_t *ecd = (ecd_t *)ecd_handle;
    ecd->pos = 0;
    ecd->dir = NO_SET;
    return ESP_OK;
}

QueueHandle_t ecd_get_queue_handle(ecd_t * ecd)
{
    // ecd_t *ecd = (ecd_t *)ecd_handle;
    return ecd->queue;
}



/*********************
 *     high level
 *********************/

#define ENABLE_HALF_STEPS false  
#define RESET_AT          1      // Set to a positive non-zero number to reset the position if this value is exceeded
#define FLIP_DIRECTION    false  


// 需要reset，不能省略
void ecd_task(void *arg){
    ecd_t * ecd = (ecd_t *)arg;
    // ecd_init(ECD_USE_HALF_STEPS);
    // QueueHandle_t ecd_queue = ecd_get_queue_handle(ecd);

    while(1){
        int8_t recv_data;
        // int32_t tpos;
        int8_t tdir;
        // 等事件传进来
        if(xQueueReceive(ecd->queue, &recv_data, pdMS_TO_TICKS(200)) == pdTRUE) {
            // tpos = _GET_POS(recv_data);
            // tdir = _GET_DIR(recv_data);
            // ESP_LOGI(TAG, "Event: position %d, direction %s", tpos, tdir ? (tdir == CW ? "CW" : "CCW") : "NOT_SET");
            tdir = recv_data;
            ESP_LOGI(TAG, "direction %d", tdir);
            // old_dir = tdir;
        }else { // 轮询
            // tpos = ecd_get_position(ecd);
            tdir = ecd_get_direction(ecd);
            ESP_LOGI(TAG, "direction %d", tdir);
            // ESP_LOGI(TAG, "Event: position %d, direction %s", tpos, tdir ? (tdir == CW ? "CW" : "CCW") : "NOT_SET");
            // tpos = ecd_get_position(ecd);
            // if(RESET_AT && (tpos >= RESET_AT || tpos <= -RESET_AT)){
            //     ESP_LOGI(TAG, "Reset");
            //     ecd_reset(ecd);
            // }
        }

    }
    ESP_LOGE(TAG, "Queue receive failed");
    ecd_deinit(ecd);
}



// backup
#if 0

static uint8_t _process(ecd_t * ecd) {
    uint8_t event = 0;
    if (ecd != NULL) {
        // Get state of input pins.
// #if !ECD_FLIPS_DIR
        uint8_t pin_state = (gpio_get_level(ecd->pin_b) << 1) | gpio_get_level(ecd->pin_a);
// #else
//        uint8_t pin_state = (gpio_get_level(ecd->pin_a) << 1) | gpio_get_level(ecd->pin_b);
// #endif
        // Determine new state from the pins and state table.
#ifdef ECD_DEBUG
        uint8_t old_state = ecd->table_state;
#endif
        ecd->table_state = ecd->table[ecd->table_state & 0xf][pin_state];

        // Return emit bits, i.e. the generated event.
        event = ecd->table_state & 0x30;
#ifdef ECD_DEBUG
        ESP_EARLY_LOGD(TAG, "BA %d%d, state 0x%02x, new state 0x%02x, event 0x%02x",
                    pin_state >> 1, pin_state & 1, old_state, ecd->table_state, event);
#endif
    }
    return event;
}

static void _isr_rotenc(void * args) {
    ecd_t * ecd = (ecd_t *)args;
    uint8_t event = _process(ecd);
    bool send_event = false;

    switch (event) {
    case DIR_CW:
        ++ecd->state.position;
        ecd->state.direction = DIR_CLOCKWISE;
        send_event = true;
        break;
    case DIR_CCW:
        --ecd->state.position;
        ecd->state.direction = DIR_COUNTER_CLOCKWISE;
        send_event = true;
        break;
    default:
        break;
    }

    if (send_event && ecd->queue) {
        ecd_event_t queue_event =
        {
            .state =
            {
                .position = ecd->state.position,
                .direction = ecd->state.direction,
            },
        };
        BaseType_t task_woken = pdFALSE;
        xQueueOverwriteFromISR(ecd->queue, &queue_event, &task_woken);
        if (task_woken) {
            portYIELD_FROM_ISR();
        }
    }
}



esp_err_t ecd_init(ecd_t * ecd, gpio_num_t pin_a, gpio_num_t pin_b, bool half)
{
    esp_err_t err = ESP_OK;

    // ecd_t *ecd = (ecd_t*)malloc(sizeof(ecd_t));
    if(ecd == NULL){
        ESP_LOGE(TAG, "ecd is NULL");
        err = ESP_ERR_INVALID_ARG;
        return err;
    }

#if !ECD_FLIPS_DIR
    ecd->pin_a = pin_a;
    ecd->pin_b = pin_b;
#else
    ecd->pin_a = pin_b;
    ecd->pin_b = pin_a;
#endif

    ecd->table = half ? &_ttable_half[0] : &_ttable_full[0];
    ecd->table_state = R_START;
    ecd->state.position = 0;
    ecd->state.direction = DIR_NOT_SET;

    // configure GPIOs
    gpio_config_t conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << pin_a) | (1ULL << pin_b),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&conf));


    gpio_install_isr_service(0);
    // install interrupt handlers
    gpio_isr_handler_add(ecd->pin_a, _isr_rotenc, ecd);
    gpio_isr_handler_add(ecd->pin_b, _isr_rotenc, ecd);
    


    ecd->queue = xQueueCreate(EVENT_QUEUE_LENGTH, sizeof(ecd_event_t));
    if(ecd->queue == NULL){
        ESP_LOGE(TAG, "Encoder queue created failed");
        return err;
    }

    return err;
}

esp_err_t ecd_uninit(ecd_t * ecd)
{
    // ecd_t *ecd = (ecd_t *)ecd_hanlde;
    esp_err_t err = ESP_OK;
    if (ecd) {
        gpio_isr_handler_remove(ecd->pin_a);
        gpio_isr_handler_remove(ecd->pin_b);
    }else {
        ESP_LOGE(TAG, "ecd is NULL");
        err = ESP_ERR_INVALID_ARG;
    }
    return err;
}

#if 0
esp_err_t ecd_enable_half_steps(ecd_t * ecd, bool enable)
{
    esp_err_t err = ESP_OK;
    if (ecd) {
        ecd->table = enable ? &_ttable_half[0] : &_ttable_full[0];
        ecd->table_state = R_START;
    }else {
        ESP_LOGE(TAG, "ecd is NULL");
        err = ESP_ERR_INVALID_ARG;
    }
    return err;
}

esp_err_t ecd_flip_direction(ecd_t * ecd)
{
    esp_err_t err = ESP_OK;
    if (ecd){
        gpio_num_t temp = ecd->pin_a;
        ecd->pin_a = ecd->pin_b;
        ecd->pin_b = temp;
    }else {
        ESP_LOGE(TAG, "ecd is NULL");
        err = ESP_ERR_INVALID_ARG;
    }
    return err;
}




QueueHandle_t ecd_create_queue(void)
{
    return xQueueCreate(EVENT_QUEUE_LENGTH, sizeof(ecd_event_t));
}

esp_err_t ecd_set_queue(ecd_t * ecd, QueueHandle_t queue)
{
    esp_err_t err = ESP_OK;
    if (ecd) {
        ecd->queue = queue;
    }else {
        ESP_LOGE(TAG, "ecd is NULL");
        err = ESP_ERR_INVALID_ARG;
    }
    return err;
}
#endif



esp_err_t ecd_get_state(const ecd_t * ecd, ecd_state_t * state)
{
    esp_err_t err = ESP_OK;
    if (ecd && state) {
        // make a snapshot of the state
        state->position = ecd->state.position;
        state->direction = ecd->state.direction;
    }else {
        ESP_LOGE(TAG, "ecd and/or state is NULL");
        err = ESP_ERR_INVALID_ARG;
    }
    return err;
}

esp_err_t ecd_reset(ecd_t * ecd)
{
    esp_err_t err = ESP_OK;
    if (ecd) {
        ecd->state.position = 0;
        ecd->state.direction = DIR_NOT_SET;
    }else {
        ESP_LOGE(TAG, "ecd is NULL");
        err = ESP_ERR_INVALID_ARG;
    }
    return err;
}

QueueHandle_t ecd_get_queue_handle(const ecd_t * ecd)
{
    return ecd->queue;
}


#endif