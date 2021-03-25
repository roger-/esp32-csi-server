extern const char *TAG;

static void task_ping_broadcast_server(void *pvParameters)
{
    struct sockaddr_in addr = {0};
    int rc;

    ESP_LOGI(TAG, "starting ping broadcast");

    while(1) {
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            ESP_LOGE(TAG, "socket: %d %s", sock, strerror(errno));
            goto END;
        }

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(GROUP_PING);
        addr.sin_port = htons(PORT_PING);

        char *message = "";

        while(1) {
            rc = sendto(sock, message, strlen(message), 0, (struct sockaddr *)&addr, sizeof(addr));
            if (rc < 0)
            {
                ESP_LOGE(TAG, "sendto: %d %s", rc, strerror(errno));
                goto END;
            }

            vTaskDelay(PING_PERIOD_MS / portTICK_RATE_MS);
        }

        END:
            close(sock);
            ESP_LOGI(TAG, "restarting ping broadcast");
            vTaskDelay(DELAY_TASK_RESTART_MS / portTICK_RATE_MS);
    }
    
    vTaskDelete(NULL);
}

void init_ping_broadcast_server() {
    xTaskCreatePinnedToCore(task_ping_broadcast_server, "ping server",
                        10000, (void *)NULL, 100, NULL, tskNO_AFFINITY);
}
