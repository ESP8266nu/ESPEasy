#ifndef ESPEASY_COMMON_H
#define ESPEASY_COMMON_H

// *****************************************************************************************
// For Arduino IDE users:
// When building using Custom.h, uncomment the next line:
//#define USE_CUSTOM_H
// *****************************************************************************************

/*
    To modify the stock configuration without changing this repo file :
    - define USE_CUSTOM_H as a build flags. ie : export PLATFORMIO_BUILD_FLAGS="'-DUSE_CUSTOM_H'"
    - add a "Custom.h" file in this folder.

*/

#ifndef CORE_POST_2_5_0
  #define STR_HELPER(x) #x
  #define STR(x) STR_HELPER(x)
#endif

#ifdef __GCC__
#pragma GCC system_header
#endif


#include <stddef.h>
namespace std
{
  using ::ptrdiff_t;
  using ::size_t;
}

#include <stdint.h>
#include <Arduino.h>
#include <string.h>


#ifdef ESP8266
  # if !defined(ARDUINO_ESP8266_RELEASE_2_4_0) && !defined(ARDUINO_ESP8266_RELEASE_2_3_0)
    #  define SUPPORT_ARP
  # endif
#endif

#ifdef ESP32
# define SUPPORT_ARP
#endif

// User configuration
// Include Custom.h before ESPEasyDefaults.h. 
#ifdef USE_CUSTOM_H
// make the compiler show a warning to confirm that this file is inlcuded
//#warning "**** Using Settings from Custom.h File ***"
  #include "Custom.h"
#else 
  // Set as default
//  #define PLUGIN_BUILD_NORMAL
#endif

#include "src/CustomBuild/ESPEasyDefaults.h"
#include "src/DataStructs/NodeStruct.h"
#include "src/Globals/RamTracker.h"


#define FS_NO_GLOBALS
#if defined(ESP8266)
  #include "core_version.h"
  #define NODE_TYPE_ID      NODE_TYPE_ID_ESP_EASYM_STD
  #include <lwip/init.h>
  #ifndef LWIP_VERSION_MAJOR
    #error
  #endif
  #if LWIP_VERSION_MAJOR == 2
  //  #include <lwip/priv/tcp_priv.h>
  #else
    #include <lwip/tcp_impl.h>
  #endif
  #include <ESP8266WiFi.h>
  //#include <ESP8266Ping.h>
  #ifndef LWIP_OPEN_SRC
  #define LWIP_OPEN_SRC
  #endif
  #include "lwip/opt.h"
  #include "lwip/udp.h"
  #include "lwip/igmp.h"
  #include "include/UdpContext.h"
  #include "limits.h"
  extern "C" {
   #include "user_interface.h"
  }

  #define SMALLEST_OTA_IMAGE 276848 // smallest known 2-step OTA image
  #define MAX_SKETCH_SIZE 1044464   // 1020 kB - 16 bytes
  #define PIN_D_MAX        16
#endif
#if defined(ESP32)

  // Temp fix for a missing core_version.h within ESP Arduino core. Wait until they actually have different releases
  #define ARDUINO_ESP8266_RELEASE "2_4_0"

  #define NODE_TYPE_ID                        NODE_TYPE_ID_ESP_EASY32_STD
  #if ESP_IDF_VERSION_MAJOR < 3
    #define ICACHE_RAM_ATTR IRAM_ATTR
  #endif
  #include <WiFi.h>
//  #include  "esp32_ping.h"
  #if ESP_IDF_VERSION_MAJOR > 3
  #include <esp32/rom/rtc.h>
  #else
  #include <rom/rtc.h>
  #endif
  #include "esp_wifi.h" // Needed to call ESP-IDF functions like esp_wifi_....
  #define PIN_D_MAX        39
  #ifdef PLUGIN_BUILD_MAX_ESP32
  #define MAX_SKETCH_SIZE 4194304   // 0x400000 look at partitions in csv file
  #else // PLUGIN_BUILD_MAX_ESP32
  #define MAX_SKETCH_SIZE 1900544   // 0x1d0000 look at partitions in csv file
  #endif // PLUGIN_BUILD_MAX_ESP32
#endif

#include <WiFiUdp.h>
#include <Wire.h>
#include <SPI.h>
#include <FS.h>
#ifdef FEATURE_SD
#include <SD.h>
#else
using namespace fs;
#endif
#include <base64.h>


#ifdef USE_LITTLEFS
  #ifdef ESP32
    #include <LITTLEFS.h>
    #define ESPEASY_FS LITTLEFS
  #else
    #include <LittleFS.h>
    #define ESPEASY_FS LittleFS
  #endif
#else 
  #ifdef ESP32
    #include <SPIFFS.h>
  #endif
  #define ESPEASY_FS SPIFFS
#endif

// Include custom first, then build info. (one may want to set BUILD_GIT for example)
#include "src/CustomBuild/ESPEasy_buildinfo.h"
#include "src/CustomBuild/ESPEasyLimits.h"
#include "src/CustomBuild/define_plugin_sets.h"

#ifdef ESP32
#include <esp8266-compat.h>

#endif


#define ZERO_FILL(S)  memset((S), 0, sizeof(S))
#define ZERO_TERMINATE(S)  S[sizeof(S) - 1] = 0



String getUnknownString();

extern const String EMPTY_STRING;



/******************************************************************************\
 * Detect core versions *******************************************************
\******************************************************************************/

#ifndef ESP32
  #if defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)  || defined(ARDUINO_ESP8266_RELEASE_2_4_2)
    #ifndef CORE_2_4_X
      #define CORE_2_4_X
    #endif
  #endif

  #if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_0) || defined(ARDUINO_ESP8266_RELEASE_2_4_1)
    #ifndef CORE_PRE_2_4_2
      #define CORE_PRE_2_4_2
    #endif
  #endif

  #if defined(ARDUINO_ESP8266_RELEASE_2_3_0) || defined(CORE_2_4_X)
    #ifndef CORE_PRE_2_5_0
      #define CORE_PRE_2_5_0
    #endif
  #else
    #ifndef CORE_POST_2_5_0
      #define CORE_POST_2_5_0
    #endif
  #endif


  #ifdef FORCE_PRE_2_5_0
    #ifdef CORE_POST_2_5_0
      #undef CORE_POST_2_5_0
    #endif
  #endif
#endif // ESP32




// Enable FEATURE_ADC_VCC to measure supply voltage using the analog pin
// Please note that the TOUT pin has to be disconnected in this mode
// Use the "System Info" device to read the VCC value
#ifndef FEATURE_ADC_VCC
  #define FEATURE_ADC_VCC                  false
#endif

#ifndef ARDUINO_OTA_PORT
  #if defined(ESP32)
    #define ARDUINO_OTA_PORT  3232
  #else
    // Do not use port 8266 for OTA, since that's used for ESPeasy p2p
    #define ARDUINO_OTA_PORT  18266
  #endif
#endif

#if defined(ESP8266)
  //enable Arduino OTA updating.
  //Note: This adds around 10kb to the firmware size, and 1kb extra ram.
  // #define FEATURE_ARDUINO_OTA

  //enable mDNS mode (adds about 6kb ram and some bytes IRAM)
  // #define FEATURE_MDNS
#endif
#if defined(ESP32)
 //#define FEATURE_ARDUINO_OTA
 //#define FEATURE_MDNS
#endif


#endif // ESPEASY_COMMON_H