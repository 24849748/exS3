#include "exS3_sntp.h"
#include "time.h"

#include "esp_log.h"
#define TAG "sntp"

static struct tm timeinfo = {0};

// sntp通知函数
static void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

/*
void sntp_main(){
    time_t now;

    pS3_init_sntp();
    // 等待sntp初始化完成
    do {
        vTaskDelay(pdMS_TO_TICKS(100));
    }while(sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET);

    sntp_stop();

    while (1)
    {
        time(&now); //
        tzset();    // 更新c库运行数据
        localtime_r(&now, &timeinfo);

        ESP_LOGI(TAG, "%4d-%02d-%02d %02d:%02d:%02d week:%d",timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_wday);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
*/

/*********************
 *       
 *********************/

/**
 * @brief 初始化sntp服务，需要联网前提
 * 
 */
void sntp_initalize(void){
    ESP_LOGI(TAG, "Initialize sntp...");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);    // 设置单播模式
    sntp_setservername(0, "ntp1.aliyun.com");
    sntp_setservername(1, "210.72.145.44");
    sntp_setservername(2, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);  // 设置一个回调函数，通知时间同步过程
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);   // 设置立刻同步模式

    sntp_init();    // 定期同步，默认时间一小时
}

/**
 * @brief 打印时间信息
 * 
 * @param timeinfo  传入的时间结构体 
 */
void sntp_print_time(const struct tm *timeinfo){
    // char strftime_buf[64];
    // strftime(strftime_buf, sizeof(strftime_buf), "%c", timeinfo);
    // ESP_LOGI(TAG, "Sntp Time: %s", strftime_buf);

    ESP_LOGI(TAG, "%4d-%02d-%02d %02d:%02d:%02d week:%d", timeinfo->tm_year + 1900, timeinfo->tm_mon +1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_wday);
}



/**
 * @brief 用户手动同步时间，时间分辨率微秒
 * 
 * @param tv 
 */
void sntp_user_sync_time_us(struct timeval *tv){
    settimeofday(tv, NULL);
    ESP_LOGI(TAG, "Time is synchronized from custom code");
    sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
}


/*********************
 *       
 *********************/

void sntp_obtain_time(void){
    sntp_servermode_dhcp(1);    // 如果有接受来自dhcp的ntp服务

    time_t now;
     /* 判断sntp是否处于同步状态 */
    int retry = 0;
    const int retry_count = 10;
    while(sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count){
        ESP_LOGW(TAG, "Waiting for system to be set...(%d/%d)", retry, retry_count);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    time(&now); // 更新系统现在时间
    localtime_r(&now, &timeinfo);   // 格式转换
}


bool sntp_check_time(time_t *time){
    struct tm timeinfo;
    localtime_r(time, &timeinfo);
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGW(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        return false;
    }else {
        return true;
    }
}

#define TIME_CHECK(year)        if(year < (2023-1900)){ \
                                    ESP_LOGW(TAG, "Time is not set yet.");  \
                                    }\


#if 0
void sntp_demo(){
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }

    char strftime_buf[64];

    // Set timezone to China Standard Time
    setenv("TZ", "CST-8", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);

    if (sntp_get_sync_mode() == SNTP_SYNC_MODE_SMOOTH) {
        struct timeval outdelta;
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_IN_PROGRESS) {
            adjtime(NULL, &outdelta);
            ESP_LOGI(TAG, "Waiting for adjusting time ... outdelta = %li sec: %li ms: %li us",
                        (long)outdelta.tv_sec,
                        outdelta.tv_usec/1000,
                        outdelta.tv_usec%1000);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }
}


#endif


/*
待优化initialize流程
*/