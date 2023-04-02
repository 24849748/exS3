#include "exS3_wifi.h"

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_ip_addr.h"

#include "esp_log.h"
#define TAG "wifi"

#include "exS3_conf.h"


static esp_netif_t *netif = NULL;

esp_err_t init_nvs() {
    esp_err_t err = nvs_flash_init();
    if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND){
    /*第一次初始化失败, 检查错误, 处理错误, 重新初始化*/
        err |= nvs_flash_erase();
        err |= nvs_flash_init();
    }
    return err;
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGD(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    }
}

void wifi_init(){
    init_nvs();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());


    wifi_init_config_t wifi_conf = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_conf));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL, NULL));


    netif = esp_netif_create_default_wifi_sta();
    assert(netif);
    esp_netif_set_hostname(netif, WIFI_NAME);


    wifi_config_t conf = {
        .sta.ssid = WIFI_SSID,
        .sta.password = WIFI_PASSWORD,
        .sta.scan_method = WIFI_SCAN_METHOD,
        .sta.sort_method = WIFI_SORT_METHOD,
        .sta.threshold.rssi = WIFI_RSSI,
        .sta.threshold.authmode = WIFI_AUTHMODE,
    };
    // wifi_sta_config_t sta_conf = {
    //     .ssid = WIFI_SSID,
    //     .password = WIFI_PASSWORD,
    // };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &conf));
    ESP_ERROR_CHECK(esp_wifi_start());
}