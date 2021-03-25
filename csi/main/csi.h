extern const char *TAG;
QueueHandle_t queue;
extern uint8_t ap_mac[6];

int sendall(const int sock, char *buffer, int buffer_len);

typedef struct {
    wifi_csi_info_t x;
    int8_t buf[612];
    uint16_t len;
} static_wifi_csi_info_t;

int is_ap_mac(uint8_t *mac)
{
    for(int i = 0; i < 6; ++i) {
        if(mac[i] != ap_mac[i])
            return 0;
    }

    return 1;
}

#define buff_printf(...) assert(len_json < len_buff_json); len_json += snprintf(buff_json + len_json, len_buff_json - len_json, __VA_ARGS__)
#define boolstr(expr) ((expr ? "true" : "false"))

int csi_to_json(static_wifi_csi_info_t *csi_info, char *buff_json, int len_buff_json)
{
    int retval; 
    int len_json = 0;

    buff_printf("{"); 
    buff_printf( 
            "\"mac\": \"" MACSTR "\","
            "\"len\": %d,"
            "\"first_word_invalid\": %s",
            MAC2STR(csi_info->x.mac),
            csi_info->x.len,
            boolstr(csi_info->x.first_word_invalid));

    buff_printf( 
            ",\"ltf_merge_en\": %s"
            ",\"channel_filter_en\": %s"
            ",\"manu_scale\": %s"
            ",\"shift\": %d",
            boolstr(configuration_csi.ltf_merge_en),
            boolstr(configuration_csi.channel_filter_en),
            boolstr(configuration_csi.manu_scale),
            configuration_csi.shift);

    buff_printf(  
            ",\"rssi\": %d,"
            "\"rate\": %d,"
            "\"sig_mode\": %d,"
            "\"mcs\": %d,"
            "\"cwb\": %d,"
            "\"smoothing\": %d,"
            "\"not_sounding\": %d,"
            "\"aggregation\": %d,"
            "\"stbc\": %d,"
            "\"fec_coding\": %d,"
            "\"sgi\": %d,"
            "\"noise_floor\": %.2f,"
            "\"ampdu_cnt\": %d,"
            "\"channel\": %d,"
            "\"secondary_channel\": %d,"
            "\"timestamp\": %.6f,"
            "\"ant\": %d,"
            "\"sig_len\": %d,"
            "\"rx_state\": %d",
            csi_info->x.rx_ctrl.rssi,
            csi_info->x.rx_ctrl.rate,
            csi_info->x.rx_ctrl.sig_mode,
            csi_info->x.rx_ctrl.mcs,
            csi_info->x.rx_ctrl.cwb,
            csi_info->x.rx_ctrl.smoothing,
            csi_info->x.rx_ctrl.not_sounding,
            csi_info->x.rx_ctrl.aggregation,
            csi_info->x.rx_ctrl.stbc,
            csi_info->x.rx_ctrl.fec_coding,
            csi_info->x.rx_ctrl.sgi,
            csi_info->x.rx_ctrl.noise_floor / 4.0,
            csi_info->x.rx_ctrl.ampdu_cnt,
            csi_info->x.rx_ctrl.channel,
            csi_info->x.rx_ctrl.secondary_channel,
            csi_info->x.rx_ctrl.timestamp / 1e6,
            csi_info->x.rx_ctrl.ant,
            csi_info->x.rx_ctrl.sig_len,
            csi_info->x.rx_ctrl.rx_state);

    const enum {NONE, ABOVE, BELOW} sec_chan = csi_info->x.rx_ctrl.secondary_channel;
    const bool is_ht   = csi_info->x.rx_ctrl.sig_mode;
    const int chan_bw  = csi_info->x.rx_ctrl.cwb == 0 ? 20 : 40;
    const bool is_stbc = csi_info->x.rx_ctrl.stbc; 
     
    int8_t buff_csi[300]; 

    if(csi_info->buf[0] != 0) {
        ESP_LOGI(TAG, "json - non-zero guardband at t=%d", csi_info->x.rx_ctrl.timestamp);
    }

    // raw CSI
    if(raw_csi) {
        // convert CSI buffer to base64 string and store in JSON buffer
        buff_printf(",\"csi_raw\": \"");

        size_t len_b64;
        retval = mbedtls_base64_encode((unsigned char *)(buff_json + len_json), len_buff_json - len_json, &len_b64, (const unsigned char *)csi_info->buf, csi_info->len);
        if(retval != 0) {
            ESP_LOGE(TAG, "base64 error");
            return retval;
        }
        len_json += len_b64;

        buff_printf("\"");
    } 

    // LLTF
    if(1) {
        size_t len_csi = 0;
        int csi_first_ind = 0;

        if(sec_chan == NONE) { // none
            memcpy(buff_csi + len_csi, csi_info->buf + 64, 64); len_csi += 64;
            memcpy(buff_csi + len_csi, csi_info->buf + 0, 64);  len_csi += 64;
            csi_first_ind = -32;
        } else if(sec_chan == BELOW) { // below
            memcpy(buff_csi + len_csi, csi_info->buf + 0, 128); len_csi += 128;
            csi_first_ind = 0;
        } else if(sec_chan == ABOVE) { // above
            memcpy(buff_csi + len_csi, csi_info->buf + 0, 128); len_csi += 128;
            csi_first_ind = -64;
        }

        size_t offset_csi = 0;
        if(csi_info->x.first_word_invalid) {
            ESP_LOGI(TAG, "invalid CSI data, skipping first 4 bytes");
            offset_csi    += 4;
            len_csi       -= 4;
            csi_first_ind += 2;
        }

        // convert CSI buffer to base64 string and store in JSON buffer
        buff_printf(",\"lltf\": {\"csi\":\"");

        size_t len_b64;
        retval = mbedtls_base64_encode((unsigned char *)(buff_json + len_json), len_buff_json - len_json, &len_b64, (const unsigned char *)(buff_csi + offset_csi), len_csi);
        if(retval != 0) {
            ESP_LOGE(TAG, "base64 error");
            return retval;
        }
        len_json += len_b64;

        buff_printf("\",\"sc_ind_start\": %d}", csi_first_ind);
    }

    // HT-LTF
    if(1) {
        size_t len_csi = 0;
        int csi_first_ind = 0;

        if(sec_chan == NONE && is_ht == true) { 
            memcpy(buff_csi + len_csi, csi_info->buf + 3 * 64, 64); len_csi += 64;
            memcpy(buff_csi + len_csi, csi_info->buf + 2 * 64, 64); len_csi += 64;
            csi_first_ind = -32;
        } else if(sec_chan == BELOW && is_ht == true && chan_bw == 20 && is_stbc == false) {
            memcpy(buff_csi + len_csi, csi_info->buf + 2 * 64, 128); len_csi += 128;
            csi_first_ind = 0;
        } else if(sec_chan == BELOW && is_ht == true && chan_bw == 20 && is_stbc == true) { 
            memcpy(buff_csi + len_csi, csi_info->buf + 2 * 64, 63 * 2); len_csi += 63 * 2;
            csi_first_ind = 0;
        } else if(sec_chan == BELOW && is_ht == true && chan_bw == 40 && is_stbc == false) { 
            memcpy(buff_csi + len_csi, csi_info->buf + 2 * 2 * 64, 64 * 2); len_csi += 64 * 2;
            memcpy(buff_csi + len_csi, csi_info->buf + 2 * 1 * 64, 64 * 2); len_csi += 64 * 2;
            csi_first_ind = -64;
        } else if(sec_chan == BELOW && is_ht == true && chan_bw == 40 && is_stbc == true) { 
            memcpy(buff_csi + len_csi, csi_info->buf + 2 * 64 + 2 * 61, 60); len_csi += 60;
            memcpy(buff_csi + len_csi, csi_info->buf +          2 * 64, 61); len_csi += 61;
            csi_first_ind = -60;
        } // TODO: above

        // convert CSI buffer to base64 string and store in JSON buffer
        buff_printf(",\"ht_ltf\": {\"csi\":\"");

        size_t len_b64;
        retval = mbedtls_base64_encode((unsigned char *)(buff_json + len_json), len_buff_json - len_json, &len_b64, (const unsigned char *)buff_csi, len_csi);
        if(retval != 0) {
            ESP_LOGE(TAG, "base64 error");
            return retval;
        }
        len_json += len_b64;

        buff_printf("\",\"sc_ind_start\": %d}", csi_first_ind);
    } 

    // STBC-HT-LTF
    if(1) {
        size_t len_csi = 0;
        int csi_first_ind = 0;

        if(sec_chan == NONE && is_ht == true && is_stbc == true) { 
            memcpy(buff_csi + len_csi, csi_info->buf + 5 * 64, 64); len_csi += 64;
            memcpy(buff_csi + len_csi, csi_info->buf + 4 * 64, 64); len_csi += 64;
            csi_first_ind = -32;
        } // TODO: below, above

        // convert CSI buffer to base64 string and store in JSON buffer
        buff_printf(",\"stbc_ht_ltf\": {\"csi\":\"");

        size_t len_b64;
        retval = mbedtls_base64_encode((unsigned char *)(buff_json + len_json), len_buff_json - len_json, &len_b64, (const unsigned char *)buff_csi, len_csi);
        if(retval != 0) {
            ESP_LOGE(TAG, "base64 error");
            return retval;
        }
        len_json += len_b64;

        buff_printf("\",\"sc_ind_start\": %d}", csi_first_ind);
    } 

    buff_printf("}\n");

    return len_json;
}

static void task_csi_server(void *pvParameters)
{
	struct sockaddr_in clientAddress;
	struct sockaddr_in serverAddress;   

    ESP_LOGI(TAG, "starting TCP server");

    while(1) {
        // Create a socket that we will listen upon.
        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock < 0) {
            ESP_LOGE(TAG, "socket: %d %s", sock, strerror(errno));
            goto END;
        }

        // Bind our server socket to a port.
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddress.sin_port = htons(PORT_CSI);
        int rc  = bind(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
        if (rc < 0) {
            ESP_LOGE(TAG, "bind: %d %s", rc, strerror(errno));
            goto END;
        }

        // Flag the socket as listening for new connections.
        rc = listen(sock, 5);
        if (rc < 0) {
            ESP_LOGE(TAG, "listen: %d %s", rc, strerror(errno));
            goto END;
        }

        while (1) {
            // Listen for a new client connection.
            socklen_t clientAddressLength = sizeof(clientAddress);
            int clientSock = accept(sock, (struct sockaddr *)&clientAddress, &clientAddressLength);
            if (clientSock < 0) {
                ESP_LOGE(TAG, "accept: %d %s", clientSock, strerror(errno));
                continue;
            }

            ESP_LOGI(TAG, "client connected");
            xQueueReset(queue);

            static_wifi_csi_info_t csi_info;

            while(1) {         
                int got_valid = xQueueReceive(queue, &csi_info, portMAX_DELAY);

                if(!got_valid) {
                    ESP_LOGI(TAG, "queue empty?\n");
                    continue;
                } 

                if(csi_info.buf[0] != 0) {
                    ESP_LOGI(TAG, "server - non-zero guardband at t=%d", csi_info.x.rx_ctrl.timestamp);
                }

                char buff_json[3000] = {0};
                int len_json = csi_to_json(&csi_info, (char *)buff_json, sizeof(buff_json));

                if(len_json < 0)
                    continue;

                if(sendall(clientSock, buff_json, len_json) < 0)
                    break;
            }

            close(clientSock);
        }

        END:
            close(sock);
            ESP_LOGI(TAG, "restarting TCP server");
            vTaskDelay(DELAY_TASK_RESTART_MS / portTICK_RATE_MS);
    }
    
    vTaskDelete(NULL);
}

void cb_wifi_csi(void *ctx, wifi_csi_info_t *data)
{
    //int size = 0;

    if(ignore_ap_packets && is_ap_mac((uint8_t *)&(data->mac)))
        return;

    if(data->buf[0] != 0) {
        ESP_LOGI(TAG, "cb - non-zero guardband at t=%d", data->rx_ctrl.timestamp);
    }

    // copy data to new struct
    static_wifi_csi_info_t csi;

    csi.x = *data;
    csi.x.buf = NULL;
    memcpy(csi.buf, data->buf, data->len);
    csi.len = data->len;

    /*size = uxQueueMessagesWaiting(queue);
    ESP_LOGD(TAG, "queue count: %d", size);

    int is_client_connected = (xEventGroupGetBits(events) & BIT_CLIENT_CONNECTED); //portMAX_DELAY 

    if(!is_client_connected)
        return;*/
        
    if(xQueueSendToBack(queue, (void *)&csi, (TickType_t)0) != pdPASS) {
        ESP_LOGD(TAG, "queue error - full?");
    }
}

void init_csi_server()
{
    queue = xQueueCreate(50, sizeof(static_wifi_csi_info_t));

    if(queue == NULL){
        ESP_LOGE(TAG, "queue error\n");
    }

    xTaskCreatePinnedToCore(task_csi_server, "csi server",
                            10000, (void *)NULL, 100, NULL, tskNO_AFFINITY);
}

void init_wifi_csi()
{
    ESP_LOGI(TAG, "starting CSI collection");

    /*wifi_csi_config_t configuration_csi = {0};
    configuration_csi.lltf_en = 1;
    configuration_csi.htltf_en = 1;
    configuration_csi.stbc_htltf2_en = 1;
    configuration_csi.ltf_merge_en = 0; //1
    configuration_csi.channel_filter_en = 0;
    configuration_csi.manu_scale = 0; //0
    configuration_csi.shift = 0;*/

    ESP_ERROR_CHECK(esp_wifi_set_csi_config(&configuration_csi));
    ESP_ERROR_CHECK(esp_wifi_set_csi_rx_cb(&cb_wifi_csi, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_csi(1));
}