#ifndef STRING_PROVIDER_TYPES_H
#define STRING_PROVIDER_TYPES_H

#include "../../ESPEasy_common.h"




struct LabelType;

// enum LabelType::Enum : short;

struct LabelType {
  enum Enum : short {
    UNIT_NR,
    UNIT_NAME,
    HOST_NAME,

    LOCAL_TIME,
    UPTIME,
    LOAD_PCT,            // 15.10
    LOOP_COUNT,          // 400
    CPU_ECO_MODE,        // true
    WIFI_TX_MAX_PWR,     // Unit: 0.25 dBm, 0 = use default (do not set)
    WIFI_CUR_TX_PWR,     // Unit dBm of current WiFi TX power.
    WIFI_SENS_MARGIN,    // Margin in dB on top of sensitivity
    WIFI_SEND_AT_MAX_TX_PWR,
    WIFI_NR_EXTRA_SCANS,
    WIFI_PERIODICAL_SCAN,
    WIFI_USE_LAST_CONN_FROM_RTC,

    FREE_MEM,            // 9876
    FREE_STACK,          // 3456
#if defined(CORE_POST_2_5_0) || defined(ESP32)
    HEAP_MAX_FREE_BLOCK, // 7654
#endif // if defined(CORE_POST_2_5_0) || defined(ESP32)
#if defined(CORE_POST_2_5_0)
    HEAP_FRAGMENTATION,  // 12
#endif // if defined(CORE_POST_2_5_0)

#ifdef ESP32
    HEAP_SIZE,
    HEAP_MIN_FREE,
    #ifdef ESP32_ENABLE_PSRAM
    PSRAM_SIZE,
    PSRAM_FREE,
    PSRAM_MIN_FREE,
    PSRAM_MAX_FREE_BLOCK,
    #endif // ESP32_ENABLE_PSRAM
#endif // ifdef ESP32

    JSON_BOOL_QUOTES,
    ENABLE_TIMING_STATISTICS,

    BOOT_TYPE,               // Cold boot
    BOOT_COUNT,              // 0
    RESET_REASON,            // Software/System restart
    DEEP_SLEEP_ALTERNATIVE_CALL,
    LAST_TASK_BEFORE_REBOOT, // Last scheduled task.
    SW_WD_COUNT,

    WIFI_CONNECTION,         // 802.11G
    WIFI_RSSI,               // -67
    IP_CONFIG,               // DHCP
    IP_CONFIG_STATIC,
    IP_CONFIG_DYNAMIC,
    IP_ADDRESS,              // 192.168.1.123
    IP_SUBNET,               // 255.255.255.0
    IP_ADDRESS_SUBNET,       // 192.168.1.123 / 255.255.255.0
    GATEWAY,                 // 192.168.1.1
    CLIENT_IP,               // 192.168.1.67
  #ifdef FEATURE_MDNS
    M_DNS,                   // breadboard.local
  #endif // ifdef FEATURE_MDNS
    DNS,                     // 192.168.1.1 / (IP unset)
    DNS_1,
    DNS_2,
    ALLOWED_IP_RANGE,        // 192.168.1.0 - 192.168.1.255
    STA_MAC,                 // EC:FA:BC:0E:AE:5B
    AP_MAC,                  // EE:FA:BC:0E:AE:5B
    SSID,                    // mynetwork
    BSSID,
    CHANNEL,                 // 1
    ENCRYPTION_TYPE_STA,     // WPA2
    CONNECTED,               // 1h16m
    CONNECTED_MSEC,          // 1h16m
    LAST_DISCONNECT_REASON,  // 200
    LAST_DISC_REASON_STR,    // Beacon timeout
    NUMBER_RECONNECTS,       // 5
    WIFI_STORED_SSID1,
    WIFI_STORED_SSID2,

    FORCE_WIFI_BG,
    RESTART_WIFI_LOST_CONN,
    FORCE_WIFI_NOSLEEP,
    PERIODICAL_GRAT_ARP,
    CONNECTION_FAIL_THRESH,

    BUILD_DESC,
    GIT_BUILD,
    SYSTEM_LIBRARIES,
    PLUGIN_COUNT,
    PLUGIN_DESCRIPTION,
    BUILD_TIME,
    BINARY_FILENAME,
    BUILD_PLATFORM,
    BUILD_DESCRIPTION,
    GIT_HEAD,


    SYSLOG_LOG_LEVEL,
    SERIAL_LOG_LEVEL,
    WEB_LOG_LEVEL,
#ifdef FEATURE_SD
    SD_LOG_LEVEL,
#endif // ifdef FEATURE_SD

    ESP_CHIP_ID,
    ESP_CHIP_FREQ,
    ESP_CHIP_MODEL,
    ESP_CHIP_REVISION,
    ESP_CHIP_CORES,
    ESP_BOARD_NAME,

    FLASH_CHIP_ID,
    FLASH_CHIP_REAL_SIZE,
    FLASH_IDE_SIZE,
    FLASH_IDE_SPEED,
    FLASH_IDE_MODE,
    FLASH_WRITE_COUNT,
    SKETCH_SIZE,
    SKETCH_FREE,
    FS_SIZE,
    FS_FREE,
    MAX_OTA_SKETCH_SIZE,
    OTA_2STEP,
    OTA_POSSIBLE,
#ifdef HAS_ETHERNET
    ETH_IP_ADDRESS,
    ETH_IP_SUBNET,
    ETH_IP_ADDRESS_SUBNET,
    ETH_IP_GATEWAY,
    ETH_IP_DNS,
    ETH_MAC,
    ETH_DUPLEX,
    ETH_SPEED,
    ETH_STATE,
    ETH_SPEED_STATE,
    ETH_CONNECTED,
#endif // ifdef HAS_ETHERNET
    ETH_WIFI_MODE,
    SUNRISE,
    SUNSET,
    ISNTP,
    UPTIME_MS,
    TIMEZONE_OFFSET,
    LATITUDE,
    LONGITUDE,
    SUNRISE_S,
    SUNSET_S,
    SUNRISE_M,
    SUNSET_M,


    MAX_LABEL  // Keep as last
  };
};


#ifdef HAS_ETHERNET
String getEthSpeed();

String getEthLinkSpeedState();
#endif // ifdef HAS_ETHERNET

String getInternalLabel(LabelType::Enum label,
                        char            replaceSpace = '_');
const __FlashStringHelper * getLabel(LabelType::Enum label);
String getValue(LabelType::Enum label);
String getExtendedValue(LabelType::Enum label);


struct FileType {
  enum Enum : short {
    CONFIG_DAT,
    SECURITY_DAT,
    RULES_TXT,
    NOTIFICATION_DAT
  };
};


String getFileName(FileType::Enum filetype);
String getFileName(FileType::Enum filetype,
                   unsigned int   filenr);

// filenr = 0...3 for files rules1.txt ... rules4.txt
String getRulesFileName(unsigned int filenr);
void   addDownloadFiletypeCheckbox(FileType::Enum filetype,
                                   unsigned int   filenr = 0);
void   storeDownloadFiletypeCheckbox(FileType::Enum filetype,
                                     unsigned int   filenr = 0);
bool   tryDownloadFileType(const String & url,
                           const String & user,
                           const String & pass,
                           FileType::Enum filetype,
                           unsigned int   filenr = 0);


#endif // STRING_PROVIDER_TYPES_H