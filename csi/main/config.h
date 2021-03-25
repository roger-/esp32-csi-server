const int PORT_CSI = 1000;
const int PORT_PING = 2222; 
const char *GROUP_PING = "224.1.1.1";

wifi_csi_config_t configuration_csi = {
    .lltf_en = 1,
    .htltf_en = 1,
    .stbc_htltf2_en = 1,
    .ltf_merge_en = 1,
    .channel_filter_en = 0,
    .manu_scale = 0,
    .shift = 0,
};

#define STA_WIFI_SSID "CHANGEME"
#define STA_WIFI_PASS "CHANGEME"

#define AP_WIFI_SSID "CHANGEME"
#define AP_WIFI_PASS "CHANGEME"

#define PING_PERIOD_MS 50
#define DELAY_TASK_RESTART_MS 1000

static const int apsta_mode = 1;
static const int raw_csi = 0;
static const int ignore_ap_packets = 1;