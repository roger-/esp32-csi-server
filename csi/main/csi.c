#include <stdio.h>
#include <lwip/sockets.h>
#include <esp_log.h>
#include <string.h>
#include <errno.h>
#include "esp_wifi.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
//#include "esp_sntp.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "mbedtls/base64.h"
#include "config.h"
#include "csi.h"
#include "utils.h"
#include "wifi.h"
#include "ping.h"
#include "ftm.h"


const char *TAG = "CSI";

void init_flash()
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void app_main(void)
{
    init_flash();
    init_wifi();

    vTaskDelay(2000 / portTICK_RATE_MS);

    if(apsta_mode) {
        /*sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, "pool.ntp.org");
        sntp_init();*/

        init_csi_server();
        init_wifi_csi();        
    } else {
        //init_ftm();
        init_ping_broadcast_server();
    }
}
