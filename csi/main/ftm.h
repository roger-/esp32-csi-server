extern const char *TAG;

void init_ftm()
{
    ESP_LOGI(TAG, "starting FTM");

    wifi_ftm_initiator_cfg_t ftmi_cfg = {
        .frm_count = 2,
        .burst_period = 0,
        .channel = 8,
    };
    
    // wifi_get_channel(uint8_t *primary, wifi_second_chan_t *second

    //ftmi_cfg.channel = ap_record->primary;
    memcpy(ftmi_cfg.resp_mac, ap_mac, 6);

    ESP_ERROR_CHECK(esp_wifi_ftm_initiate_session(&ftmi_cfg));
}