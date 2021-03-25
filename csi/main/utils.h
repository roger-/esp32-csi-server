extern const char *TAG;

int sendall(const int sock, char *buffer, int buffer_len)
{
    ESP_LOGD(TAG, "sending %d bytes", buffer_len);

    int to_write = buffer_len;

    while (to_write > 0) {
        int written = send(sock, buffer + (buffer_len - to_write), to_write, 0);

        if (written < 0) {
            ESP_LOGE(TAG, "send: %d %s", sock, strerror(errno));
            return written;
        }

        to_write -= written;
    }

    return 0;
}
