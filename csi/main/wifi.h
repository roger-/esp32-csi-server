extern const char *TAG;
uint8_t ap_mac[6] = {0};

int wifi_retries = 0;

//static EventGroupHandle_t events;

const int BIT_WIFI_CONNECTED   = BIT0;
const int BIT_CLIENT_CONNECTED = BIT1;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    ip_event_got_ip_t* ip_event;
    wifi_event_ftm_report_t *ftm_event;

    uint8_t l_Mac[6];
    char mac[20] = {0};

    wifi_ap_record_t ap_rec;

    if(event_base == WIFI_EVENT) {
        switch(event_id) {
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "WiFi connected");

                esp_wifi_get_mac(ESP_IF_WIFI_STA, l_Mac);
                sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", l_Mac[0], l_Mac[1], l_Mac[2], l_Mac[3], l_Mac[4], l_Mac[5]);
                ESP_LOGI(TAG, "STA MAC: %s", mac);

                /*esp_wifi_get_mac(ESP_IF_WIFI_AP, l_Mac);
                sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", l_Mac[0], l_Mac[1], l_Mac[2], l_Mac[3], l_Mac[4], l_Mac[5]);
                ESP_LOGI(TAG, "WiFi AP MAC: %s", mac);*/

                esp_wifi_sta_get_ap_info(&ap_rec);
                memcpy(&ap_mac, &ap_rec.bssid, sizeof(ap_rec.bssid));

                sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", ap_rec.bssid[0], ap_rec.bssid[1], ap_rec.bssid[2], ap_rec.bssid[3], ap_rec.bssid[4], ap_rec.bssid[5]);
                ESP_LOGI(TAG, "AP MAC: %s", mac);

                break;

            case WIFI_EVENT_FTM_REPORT:
                ftm_event = (wifi_event_ftm_report_t *) event_data;

                if (ftm_event->status == FTM_STATUS_SUCCESS) {
                    ESP_LOGI(TAG, "Estimated RTT - %d nSec, Estimated Distance - %d.%02d meters", ftm_event->rtt_est,
                        ftm_event->dist_est / 100, ftm_event->dist_est % 100);
                } else {
                    ESP_LOGI(TAG, "FTM procedure with Peer("MACSTR") failed! (Status - %d)",
                            MAC2STR(ftm_event->peer_mac), ftm_event->status);
                }
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "WiFi reconnection attempt %d", ++wifi_retries);
                ESP_ERROR_CHECK(esp_wifi_connect());                
                
                break;

            case WIFI_EVENT_AP_STACONNECTED:
                ip_event = (ip_event_got_ip_t*) event_data;
                ESP_LOGI(TAG, "client connected with ip:" IPSTR, IP2STR(&ip_event->ip_info.ip));
                break;

            default:
                break;
        }
    }
    
    if(event_base == IP_EVENT) {
        switch(event_id) {
            case IP_EVENT_STA_GOT_IP:
                ip_event = (ip_event_got_ip_t*) event_data;
                ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&ip_event->ip_info.ip));
                break;
            default:
                break;
        }
    }
}


void init_wifi_apsta()
{
    wifi_config_t wifi_config_ap = {
        .ap = {
            .ssid = AP_WIFI_SSID,
            .password = AP_WIFI_PASS,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .max_connection = 4,
        },
    };

    wifi_config_t wifi_config_sta = {
        .sta = {
            .ssid = STA_WIFI_SSID,
            .password = STA_WIFI_PASS,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config_sta));

    ESP_ERROR_CHECK(esp_wifi_set_bandwidth(ESP_IF_WIFI_AP, WIFI_BW_HT40));
    ESP_ERROR_CHECK(esp_wifi_set_bandwidth(ESP_IF_WIFI_STA, WIFI_BW_HT40));
}

void init_wifi_sta()
{
    wifi_config_t wifi_config_sta = {
        .sta = {
            .ssid = AP_WIFI_SSID,
            .password = AP_WIFI_PASS,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config_sta));
    ESP_ERROR_CHECK(esp_wifi_set_bandwidth(ESP_IF_WIFI_STA, WIFI_BW_HT40));
}

void init_wifi()
{
    ESP_LOGI(TAG, "starting WiFi");

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();
    if(apsta_mode) {
        esp_netif_t* wifi_ap = esp_netif_create_default_wifi_ap(); 
        esp_netif_ip_info_t ip_info;

        IP4_ADDR(&ip_info.ip, 192, 168, 4, 1);
        IP4_ADDR(&ip_info.gw, 192, 168, 4, 1);
        IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);

        esp_netif_dhcps_stop(wifi_ap);
        esp_netif_set_ip_info(wifi_ap, &ip_info);
        esp_netif_dhcps_start(wifi_ap); 
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    if(apsta_mode) {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
        init_wifi_apsta();
    } else {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        init_wifi_sta();
    }

    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    
    //esp_wifi_set_storage(WIFI_STORAGE_RAM);
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_wifi_connect());

    //ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    //ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));

}
