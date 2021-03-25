# esp32-csi-server

Simple application to collect WiFi channel state information (CSI) from an ESP32 and forward it to a TCP client on another machine. Useful for logging data or remote processing (e.g. activity detection). Uses one or two ESP32s.

By default it operates in AP-STA mode and will create a private soft AP while connected to an external AP. Packets from devices connected via the soft AP will be used for CSI data (getting CSI data from the external AP is also possible). Also included is a mode to connect to the soft AP using a second ESP32, otherwise you can connected via phone, etc. and send pings to the soft AP.

# Usage

1. Install ESP-IDF 4.1+ using [this guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/versions.html)
2. Clone this repo
3. Configure your ESP32 using the command `esp-idf.py menuconfig`
4. Configure main/config.h to specify SSID, passwords and other configuration
5. Build and flash using, e.g. `idf.py -p /dev/ttyUSB0 flash monitor`
