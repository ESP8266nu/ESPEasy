# ESP-NOW Arduino library for ESP8266 and ESP32

**WifiEspNow** is an Arduino library for ESP-NOW, a connectionless WiFi communication protocol defined by Espressif.
Refer to [ESP-NOW reference](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/network/esp_now.html) for more information about how ESP-NOW works and its limitations.

[`WifiEspNow`](src/WifiEspNow.h) is a simple wrapper of ESP-NOW functions in ESP-IDF.
On ESP8266, it supports unicast only.
On ESP32, it supports both unicast and multicast.

[`WifiEspNowBroadcast`](src/WifiEspNowBroadcast.h) implements *pseudo* broadcast over ESP-NOW.
Each device advertises a specific WiFi SSID, and discovers each other through BSSID scanning.
Then, messages are transmitted separately toward every peer via ESP-NOW unicast.
This is my custom protocol, which differs from `WifiEspNow` multicast.
