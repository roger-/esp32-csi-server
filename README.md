
# esp32-csi-server

Simple application to collect WiFi channel state information (CSI) from an ESP32 and forward it to a TCP client on another machine. Useful for logging data or remote processing (e.g. activity detection). Uses one or two ESP32s.

![diagram](https://user-images.githubusercontent.com/1389709/112679235-4f0a6880-8e42-11eb-82fb-43c5de895c32.png)

By default it operates in AP-STA mode and will create a private soft AP while connected to an external AP. Packets from devices connected via the soft AP will be used for CSI data (getting CSI data from the external AP is also possible). Also included is a mode to connect to the soft AP using a second ESP32, otherwise you can connected via phone, etc. and send pings to the soft AP (any app that generates WiFi traffic will work).

# Usage

1. Install ESP-IDF 4.1+ using [this guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/versions.html)
2. Clone this repo
3. Configure your ESP32 using the command `idf.py menuconfig`
4. Configure main/config.h to specify SSID, passwords and other configuration (AP-STA mode, etc.)
5. Build and flash using, e.g. `idf.py -p /dev/ttyUSB0 flash monitor`
6. Ensure a device is connected to the new soft AP and actively pinging it (e.g. configure and flash another ESP32 with AP-STA mode disabled)
7. Get CSI data via TCP, e.g. using `netcat 192.168.1.40 1000`

Note that some channel configurations are not currently parsed, so you will need to enable raw CSI support in config.h and [manually parse them](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/wifi.html#wi-fi-channel-state-information).

## Decoding
Output data is in JSON and actual CSI data is encoded using base64 and can be decoded using

```python
import base64

def decode(b64):
    csi = base64.b64decode(b64, validate=True)
    csi = np.frombuffer(csi, dtype='int8')
    csi = csi[1::2] + 1j * csi[:-1:2]
    
    return csi
```

To get data into a pandas DataFrame, you can using

```python
import json
import pandas as pd

js = (json.loads(line) for line in open(filename))
df = pd.json_normalize(js)
```

## Sample output

See also sample.json.

```json
{
  "mac": "7c:df:a1:08:e3:b2",
  "len": 384,
  "first_word_invalid": false,
  "ltf_merge_en": true,
  "channel_filter_en": false,
  "manu_scale": false,
  "shift": 0,
  "rssi": -51,
  "rate": 11,
  "sig_mode": 1,
  "mcs": 0,
  "cwb": 1,
  "smoothing": 1,
  "not_sounding": 1,
  "aggregation": 0,
  "stbc": 0,
  "fec_coding": 0,
  "sgi": 0,
  "noise_floor": -23.5,
  "ampdu_cnt": 0,
  "channel": 8,
  "secondary_channel": 2,
  "timestamp": 3973.219721,
  "ant": 0,
  "sig_len": 82,
  "rx_state": 0,
  "lltf": {
    "csi": "AAAAAAAAAAAAAAAAAPcA9vz3//X/+P74/fj9+fz5+/r6+Pj6+fr7+fz7+Pn2+/z7+P35/Pz6+Pz7+/b99vz3+gAA9f/1/PT99f/4/fb+9f73/vb+9v31/fn9+P33/vf99P7z/ff99vz4/Pf79/v5+vf5+vv9+QAAAAAAAAAAAAA=",
    "sc_ind_start": 0
  },
  "ht_ltf": {
    "csi": "AAAAAAAAAAAAAAAA8BbqFesU4hrgDN0K4gzjB9sK5QzlEecW3xfiFegJ3A3aGd0D5QnfEN8J2w3jDdkT0hLYG9wM3hTiCtsc8RHhEuAV6hTdFuge6xDpHPAY8CDsG+wg6yL3JO8Y9SLpG/8h+CP6JP0b/B72J/gc9ikCLwEnAAAAAAAAJQseASn0JwAi+iT8H/Qf+hr0HPcd8RLxIPIW8xXuDd0c5xbnD+oV4xfVFvYJ5BHpE+4W4g7lEdMN4gzVCd0K3AjhB9v83xDmCtcB2QnbBdoJ2gjaCeQI4QnaB9sF2gbWC+MO3hDgFN0V3wrnG+UZ8RjzAAAAAAAAAAAAAA==",
    "sc_ind_start": -64
  },
  "stbc_ht_ltf": {
    "csi": "",
    "sc_ind_start": 0
  }
}
```
