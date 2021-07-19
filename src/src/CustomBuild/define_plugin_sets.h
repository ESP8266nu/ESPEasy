#ifndef DEFINE_PLUGIN_SETS_H
#define DEFINE_PLUGIN_SETS_H

#include "../../ESPEasy_common.h"

/*
#################################################
 This is the place where plugins are registered
#################################################
To create/register a plugin, you have to :
- find an available number, ie 777.
- Create your own plugin, ie as "_P777_myfunction.ino"
- be sure it starts with ""#ifdef USES_P777", and ends with "#endif"
- then register it into the PLUGIN_SET_EXPERIMENTAL block (see below)
 #ifdef PLUGIN_SET_EXPERIMENTAL
     #define USES_P777   // MYsuperPlugin
 #endif
 - you can from now on test it by compiling using the PLUGIN_BUILD_DEV flag
 either by adding "-DPLUGIN_BUILD_DEV" when compiling, or by momentarly
 adding "#define PLUGIN_BUILD_DEV" at the top of the ESPEasy.ino file
 - You will then have to push a PR including your plugin + the corret line (#define USES_P777) added to this file
 When found stable enough, the maintainer (and only him) will choose to move it to TESTING or STABLE
*/

//#define FEATURE_SD

/******************************************************************************\
 * WebServer pages   **********************************************************
\******************************************************************************/
// FIXME TD-er: Make useful selections for these pages to be included. (e.g. view only)

#ifndef WEBSERVER_CUSTOM_BUILD_DEFINED
    #ifndef WEBSERVER_TIMINGSTATS
        #define WEBSERVER_TIMINGSTATS
    #endif
    #ifndef WEBSERVER_SYSVARS
        #define WEBSERVER_SYSVARS
    #endif
    #ifndef WEBSERVER_NEW_UI
    //    #define WEBSERVER_NEW_UI
    #endif
    #ifndef WEBSERVER_I2C_SCANNER
        #define WEBSERVER_I2C_SCANNER
    #endif
    #ifndef WEBSERVER_FAVICON
        #define WEBSERVER_FAVICON
    #endif
    #ifndef WEBSERVER_CSS
        #define WEBSERVER_CSS
    #endif
    #ifndef WEBSERVER_INCLUDE_JS
        #define WEBSERVER_INCLUDE_JS
    #endif
    #ifndef WEBSERVER_LOG
        #define WEBSERVER_LOG
    #endif
    #ifndef WEBSERVER_GITHUB_COPY
        #define WEBSERVER_GITHUB_COPY
    #endif
    #ifndef WEBSERVER_ROOT
        #define WEBSERVER_ROOT
    #endif
    #ifndef WEBSERVER_ADVANCED
        #define WEBSERVER_ADVANCED
    #endif
    #ifndef WEBSERVER_CONFIG
        #define WEBSERVER_CONFIG
    #endif
    #ifndef WEBSERVER_CONTROL
        #define WEBSERVER_CONTROL
    #endif
    #ifndef WEBSERVER_CONTROLLERS
        #define WEBSERVER_CONTROLLERS
    #endif
    #ifndef WEBSERVER_CUSTOM
        #define WEBSERVER_CUSTOM
    #endif
    #ifndef WEBSERVER_DEVICES
        #define WEBSERVER_DEVICES
    #endif
    #ifndef WEBSERVER_DOWNLOAD
        #define WEBSERVER_DOWNLOAD
    #endif
    #ifndef WEBSERVER_FACTORY_RESET
        #define WEBSERVER_FACTORY_RESET
    #endif
    #ifndef WEBSERVER_FILELIST
        #define WEBSERVER_FILELIST
    #endif
    #ifndef WEBSERVER_HARDWARE
        #define WEBSERVER_HARDWARE
    #endif
    #ifndef WEBSERVER_PINSTATES
        #define WEBSERVER_PINSTATES
    #endif
    #ifndef WEBSERVER_RULES
        #define WEBSERVER_RULES
    #endif
    #ifndef WEBSERVER_SETUP
        #define WEBSERVER_SETUP
    #endif
    #ifndef WEBSERVER_SYSINFO
        #define WEBSERVER_SYSINFO
    #endif
    #ifndef WEBSERVER_TOOLS
        #define WEBSERVER_TOOLS
    #endif
    #ifndef WEBSERVER_UPLOAD
        #define WEBSERVER_UPLOAD
    #endif
    #ifndef WEBSERVER_WIFI_SCANNER
        #define WEBSERVER_WIFI_SCANNER
    #endif
    #ifndef WEBSERVER_NEW_RULES
//        #define WEBSERVER_NEW_RULES
    #endif
#endif

#ifndef USE_CUSTOM_H
    #ifndef USES_SSDP
        #define USES_SSDP
    #endif
    #ifndef USES_TIMING_STATS
        #define USES_TIMING_STATS
    #endif
    #ifndef FEATURE_I2CMULTIPLEXER
        #define FEATURE_I2CMULTIPLEXER
    #endif
    #ifndef USE_TRIGONOMETRIC_FUNCTIONS_RULES
        #define USE_TRIGONOMETRIC_FUNCTIONS_RULES
    #endif
#endif



#ifdef MEMORY_ANALYSIS
  #ifdef MQTT_ONLY
    #define USES_C002   // Domoticz MQTT
    #define USES_C005   // Home Assistant (openHAB) MQTT
    #define USES_C006   // PiDome MQTT
    #define USES_C014   // homie 3 & 4dev MQTT
    #define USES_P037   // MQTTImport
  #endif
#endif


/******************************************************************************\
 * Available options **********************************************************
\******************************************************************************/
#if defined(CORE_POST_2_5_0) && !defined(MEMORY_ANALYSIS) && !defined(USE_CUSTOM_H)
    #ifndef USE_SETTINGS_ARCHIVE
    // FIXME TD-er: Disabled for now, to reduce binary size
//        #define USE_SETTINGS_ARCHIVE
    #endif // USE_SETTINGS_ARCHIVE
#endif

#if defined(USE_SETTINGS_ARCHIVE) && defined(FORCE_PRE_2_5_0)
  #undef USE_SETTINGS_ARCHIVE
#endif


/******************************************************************************\
 * BUILD Configs **************************************************************
\******************************************************************************/

// IR library is large, so make a separate build including stable plugins and IR.
#ifdef PLUGIN_BUILD_DEV_IR
    #define PLUGIN_BUILD_DEV       // add dev
    #define PLUGIN_BUILD_IR
#endif

#ifdef PLUGIN_BUILD_TESTING_IR
    #define PLUGIN_BUILD_TESTING   // add testing
    #define PLUGIN_BUILD_IR
#endif

#ifdef PLUGIN_BUILD_MINIMAL_IR
    #ifndef USES_DOMOTICZ
        #define USES_DOMOTICZ
    #endif
    #ifndef USES_FHEM
        #define USES_FHEM
    #endif
    #ifndef USES_HOMEASSISTANT_OPENHAB
        #define USES_HOMEASSISTANT_OPENHAB
    #endif

    #define PLUGIN_BUILD_MINIMAL_OTA
    #define PLUGIN_DESCR  "Minimal, IR"
    #define PLUGIN_BUILD_IR
#endif

#ifdef PLUGIN_BUILD_MINIMAL_IRext
    #ifndef USES_DOMOTICZ
        #define USES_DOMOTICZ
    #endif
    #ifndef USES_FHEM
        #define USES_FHEM
    #endif
    #ifndef USES_HOMEASSISTANT_OPENHAB
        #define USES_HOMEASSISTANT_OPENHAB
    #endif

    #define PLUGIN_BUILD_MINIMAL_OTA
    #define PLUGIN_DESCR  "Minimal, IR with AC"
    #define PLUGIN_BUILD_IR_EXTENDED
#endif

#ifdef PLUGIN_BUILD_NORMAL_IR
    #define PLUGIN_BUILD_NORMAL     // add stable
    #define PLUGIN_DESCR  "Normal, IR"
    #define PLUGIN_BUILD_IR
#endif

#ifdef PLUGIN_BUILD_NORMAL_IRext
    #define PLUGIN_BUILD_NORMAL     // add stable
    #define PLUGIN_DESCR  "Normal, IR with AC"
    #define PLUGIN_BUILD_IR_EXTENDED
#endif

#ifdef PLUGIN_BUILD_DEV
    #define  PLUGIN_SET_EXPERIMENTAL
    #define  CONTROLLER_SET_EXPERIMENTAL
    #define  NOTIFIER_SET_EXPERIMENTAL
    #define  PLUGIN_BUILD_TESTING   // add testing
#endif

#ifdef PLUGIN_BUILD_TESTING
    #if !defined(PLUGIN_BUILD_TESTING_B) && !defined(PLUGIN_BUILD_TESTING_C) && !defined(PLUGIN_BUILD_TESTING_D)
      #define PLUGIN_DESCR  "TEST_A"
      #define PLUGIN_SET_TESTING_A
    #endif
    #define PLUGIN_SET_TESTING
    #define CONTROLLER_SET_TESTING
    #define NOTIFIER_SET_TESTING
    #define PLUGIN_BUILD_NORMAL     // add stable
#endif

#ifdef PLUGIN_BUILD_TESTING_B
    #define PLUGIN_DESCR  "TEST_B"
    #define PLUGIN_SET_TESTING
    #define PLUGIN_SET_TESTING_B
    #define CONTROLLER_SET_TESTING
    #define NOTIFIER_SET_TESTING
    #define PLUGIN_BUILD_NORMAL     // add stable
#endif

#ifdef PLUGIN_BUILD_TESTING_C
    #define PLUGIN_DESCR  "TEST_C"
    #define PLUGIN_SET_TESTING
    #define PLUGIN_SET_TESTING_C
    #define CONTROLLER_SET_TESTING
    #define NOTIFIER_SET_TESTING
    #define PLUGIN_BUILD_NORMAL     // add stable
#endif

#ifdef PLUGIN_BUILD_TESTING_D
    #define PLUGIN_DESCR  "TEST_D"
    #define PLUGIN_SET_TESTING
    #define PLUGIN_SET_TESTING_D
    #define CONTROLLER_SET_TESTING
    #define NOTIFIER_SET_TESTING
    #define PLUGIN_BUILD_NORMAL     // add stable
#endif

#ifndef PLUGIN_BUILD_CUSTOM
    #ifndef PLUGIN_BUILD_NORMAL
        #define PLUGIN_BUILD_NORMAL // defaults to stable, if not custom
    #endif
#endif

#ifdef PLUGIN_BUILD_NORMAL
    #define  PLUGIN_SET_STABLE
    #define  CONTROLLER_SET_STABLE
    #define  NOTIFIER_SET_STABLE

    #ifndef FEATURE_I2CMULTIPLEXER
        #define FEATURE_I2CMULTIPLEXER
    #endif
    #ifndef USE_TRIGONOMETRIC_FUNCTIONS_RULES
        #define USE_TRIGONOMETRIC_FUNCTIONS_RULES
    #endif
    #define KEEP_TRIGONOMETRIC_FUNCTIONS_RULES
#endif

#ifdef USES_FHEM
    #define USES_C009   // FHEM HTTP
#endif

#ifdef USES_HOMEASSISTANT_OPENHAB
    #define USES_C005   // Home Assistant (openHAB) MQTT
#endif

#ifdef PLUGIN_BUILD_MINIMAL_OTA
    #ifndef PLUGIN_DESCR
      #define PLUGIN_DESCR  "Minimal 1M OTA"
    #endif

    #define CONTROLLER_SET_NONE

    #define BUILD_MINIMAL_OTA
    #ifndef BUILD_NO_DEBUG
      #define BUILD_NO_DEBUG
    #endif

//    #define USES_C001   // Domoticz HTTP
//    #define USES_C002   // Domoticz MQTT
//    #define USES_C005   // Home Assistant (openHAB) MQTT
//    #define USES_C006   // PiDome MQTT
    #define USES_C008   // Generic HTTP
//    #define USES_C009   // FHEM HTTP
//    #define USES_C010   // Generic UDP
    #define USES_C013   // ESPEasy P2P network

//    #define NOTIFIER_SET_STABLE
    #define NOTIFIER_SET_NONE

    #define PLUGIN_SET_NONE

    #ifdef USE_SETTINGS_ARCHIVE
        #undef USE_SETTINGS_ARCHIVE
    #endif // USE_SETTINGS_ARCHIVE

    #ifdef USES_TIMING_STATS
        #undef USES_TIMING_STATS
    #endif

    #ifndef USES_P001
        #define USES_P001   // switch
    #endif
    #ifndef USES_P026
      #define USES_P026   // SysInfo
    #endif
    #ifndef USES_P033
      #define USES_P033   // Dummy
    #endif
    #ifndef USES_P037
//        #define USES_P037   // MQTTImport
    #endif

    #ifndef USES_P004
//        #define USES_P004   // Dallas
    #endif
    #ifndef USES_P005
//        #define USES_P005   // DHT
    #endif

    #ifdef USE_SERVO
      #undef USE_SERVO
    #endif
    #ifdef USE_RTTTL
      #undef USE_RTTTL
    #endif
#endif


// Strip out parts not needed for either MINIMAL_OTA and MEMORY_ANALYSIS
#if defined(BUILD_MINIMAL_OTA) || defined(MEMORY_ANALYSIS)
    #ifndef WEBSERVER_CUSTOM_BUILD_DEFINED
        #ifdef WEBSERVER_TIMINGSTATS
            #undef WEBSERVER_TIMINGSTATS
        #endif
        #ifdef WEBSERVER_SYSVARS
            #undef WEBSERVER_SYSVARS
        #endif
        #ifdef WEBSERVER_NEW_UI
            #undef WEBSERVER_NEW_UI
        #endif
        #ifdef WEBSERVER_I2C_SCANNER
            #undef WEBSERVER_I2C_SCANNER
        #endif
        #ifdef WEBSERVER_FAVICON
            #undef WEBSERVER_FAVICON
        #endif
        #ifdef WEBSERVER_CSS
            #undef WEBSERVER_CSS
        #endif
        #ifdef WEBSERVER_INCLUDE_JS
            #undef WEBSERVER_INCLUDE_JS
        #endif
        #ifdef WEBSERVER_LOG
            #undef WEBSERVER_LOG
        #endif
        #ifdef WEBSERVER_GITHUB_COPY
            #undef WEBSERVER_GITHUB_COPY
        #endif
        #ifdef WEBSERVER_PINSTATES
            #undef WEBSERVER_PINSTATES
        #endif
        #ifdef WEBSERVER_WIFI_SCANNER
            #undef WEBSERVER_WIFI_SCANNER
        #endif
        #ifdef WEBSERVER_CUSTOM
            #undef WEBSERVER_CUSTOM
        #endif
        #ifdef WEBSERVER_NEW_RULES
            #undef WEBSERVER_NEW_RULES
        #endif


    #endif // WEBSERVER_CUSTOM_BUILD_DEFINED

    #ifndef LIMIT_BUILD_SIZE
        #define LIMIT_BUILD_SIZE
    #endif
    #ifdef KEEP_TRIGONOMETRIC_FUNCTIONS_RULES
        #undef KEEP_TRIGONOMETRIC_FUNCTIONS_RULES
    #endif
    #ifndef NOTIFIER_SET_NONE
        #define NOTIFIER_SET_NONE
    #endif
#endif



#ifdef BUILD_NO_DEBUG
    #ifdef WEBSERVER_RULES_DEBUG
        #undef WEBSERVER_RULES_DEBUG
    #endif
#endif


/******************************************************************************\
 * IR plugins *****************************************************************
\******************************************************************************/
// See lib\IRremoteESP8266\src\IRremoteESP8266.h
// Disable all settings like these when not needed:
// #define DECODE_TOSHIBA_AC      true
// #define SEND_TOSHIBA_AC        true
#ifdef PLUGIN_BUILD_IR
    #define PLUGIN_DESCR  "IR"
    #define USES_P016      // IR
    #define P016_SEND_IR_TO_CONTROLLER false //IF true then the JSON replay solution is transmited back to the condroller.
    #define USES_P035      // IRTX
    #define P016_P035_USE_RAW_RAW2 //Use the RAW and RAW2 encodings, disabling it saves 3.7Kb
#endif

#ifdef PLUGIN_BUILD_IR_EXTENDED
    #ifndef PLUGIN_DESCR
        #define PLUGIN_DESCR  "IR Extended"
    #endif // PLUGIN_DESCR
    #define USES_P016      // IR
    #define P016_SEND_IR_TO_CONTROLLER false //IF true then the JSON replay solution is transmited back to the condroller.
    #define USES_P035      // IRTX
    // The following define is needed for extended decoding of A/C Messages and or using standardised common arguments for controlling all deeply supported A/C units
    #define P016_P035_Extended_AC
    #define P016_P035_USE_RAW_RAW2 //Use the RAW and RAW2 encodings, disabling it saves 3.7Kb
    #define USES_P088      // ToniA IR plugin
    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
    #define USES_P029      // Output - Domoticz MQTT Helper
    #define PLUGIN_SET_ONLY_TEMP_HUM
#endif

#ifdef PLUGIN_BUILD_IR_EXTENDED_NO_RX
    #ifndef PLUGIN_DESCR
        #define PLUGIN_DESCR  "IR Extended, no IR RX"
    #endif // PLUGIN_DESCR
    #define USES_P035      // IRTX
    // The following define is needed for extended decoding of A/C Messages and or using standardised common arguments for controlling all deeply supported A/C units
    #define P016_P035_Extended_AC
    #define P016_P035_USE_RAW_RAW2 //Use the RAW and RAW2 encodings, disabling it saves 3.7Kb
    #define USES_P088      //ToniA IR plugin
#endif

/******************************************************************************\
 * Devices ********************************************************************
\******************************************************************************/

// Itead ----------------------------
#ifdef PLUGIN_SET_SONOFF_BASIC
    #define PLUGIN_DESCR  "Sonoff Basic"

    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
#endif

#ifdef PLUGIN_SET_SONOFF_TH1x
    #define PLUGIN_DESCR  "Sonoff TH10/TH16"

    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
    #define PLUGIN_SET_ONLY_TEMP_HUM
#endif

#ifdef PLUGIN_SET_SONOFF_POW
    #ifndef PLUGIN_DESCR
        #define PLUGIN_DESCR  "Sonoff POW R1/R2"
    #endif

    #define CONTROLLER_SET_STABLE
    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
    #define USES_P076   // HWL8012   in POW r1
    // Needs CSE7766 Energy sensor, via Serial RXD 4800 baud 8E1 (GPIO1), TXD (GPIO3)
    #define USES_P077	  // CSE7766   in POW R2
    #define USES_P081   // Cron
#endif

#ifdef PLUGIN_SET_SONOFF_S2x
    #define PLUGIN_DESCR  "Sonoff S20/22/26"

    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
#endif

#ifdef PLUGIN_SET_SONOFF_4CH
    #define PLUGIN_DESCR  "Sonoff 4CH"
    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
#endif

#ifdef PLUGIN_SET_SONOFF_TOUCH
    #define PLUGIN_DESCR  "Sonoff Touch"
    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
#endif

// Shelly ----------------------------
#ifdef PLUGIN_SET_SHELLY_1
    #define PLUGIN_DESCR  "Shelly 1"

    #define PLUGIN_SET_ONLY_SWITCH
    #define CONTROLLER_SET_STABLE
    #define NOTIFIER_SET_STABLE
    #define USES_P004   // DS18B20
#endif

#ifdef PLUGIN_SET_SHELLY_PLUG_S
    #define PLUGIN_DESCR  "Shelly PLUG-S"

    #define PLUGIN_SET_ONLY_SWITCH
    #define CONTROLLER_SET_STABLE
    #define NOTIFIER_SET_STABLE
    #define USES_P076   // HWL8012   in POW r1
    #define USES_P081   // Cron
#endif

// Easy ----------------------------
#ifdef PLUGIN_SET_EASY_TEMP
    #define PLUGIN_DESCR  "Temp Hum"
    #define PLUGIN_SET_ONLY_TEMP_HUM
#endif

#ifdef PLUGIN_SET_EASY_CARBON
    #define PLUGIN_DESCR  "Carbon"
    #define PLUGIN_SET_NONE
    #define USES_P052   // SenseAir
#endif

/*
#ifdef PLUGIN_SET_EASY_NEXTION
    #define PLUGIN_SET_ONLY_SWITCH
    //#define USES_Pxxx   // Nextion
#endif
*/

#ifdef PLUGIN_SET_EASY_OLED1
    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
    #define USES_P036   // FrameOLED
#endif

#ifdef PLUGIN_SET_EASY_OLED2
    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
    #define USES_P023   // OLED
#endif

#ifdef PLUGIN_SET_EASY_RELAY
    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
#endif

// LedStrips ----------------------------
#ifdef PLUGIN_SET_H801
    #define PLUGIN_SET_ONLY_LEDSTRIP
#endif

#ifdef PLUGIN_SET_MAGICHOME
    #define PLUGIN_SET_ONLY_LEDSTRIP
#endif

#ifdef PLUGIN_SET_MAGICHOME_IR
    #define PLUGIN_SET_ONLY_LEDSTRIP
    #define USES_P016      // IR
#endif


// Generic ESP32 -----------------------------
#ifdef PLUGIN_SET_GENERIC_ESP32
    #define PLUGIN_DESCR  "Generic ESP32"

    #ifndef ESP32
        #define ESP32
    #endif
    #ifdef ESP8266
        #undef ESP8266
    #endif
    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
    #define USES_P036   // FrameOLED
    #define USES_P027   // INA219
    #define USES_P028   // BME280
#endif

#ifdef PLUGIN_SET_TEST_ESP32
    #if !defined(PLUGIN_SET_TEST_B_ESP32) && !defined(PLUGIN_SET_TEST_C_ESP32) && !defined(PLUGIN_SET_TEST_D_ESP32)
      #define PLUGIN_DESCR  "TEST_A ESP32"
      #define  PLUGIN_SET_TESTING_A
    #endif
    #ifndef ESP32
        #define ESP32
    #endif
    #ifdef ESP8266
        #undef ESP8266
    #endif
//    #define PLUGIN_SET_ONLY_SWITCH

    #define  PLUGIN_SET_TESTING
    #define  CONTROLLER_SET_STABLE
    #define  NOTIFIER_SET_STABLE
    #define  PLUGIN_SET_STABLE     // add stable
    // See also PLUGIN_SET_TEST_ESP32 section at end,
    // where incompatible plugins will be disabled.
    // TODO : Check compatibility of plugins for ESP32 board.
#endif

#ifdef PLUGIN_SET_TEST_B_ESP32
    #define PLUGIN_DESCR  "TEST_B ESP32"
    #ifndef ESP32
        #define ESP32
    #endif
    #ifdef ESP8266
        #undef ESP8266
    #endif
//    #define PLUGIN_SET_ONLY_SWITCH

    #define  PLUGIN_SET_TESTING
    #define  PLUGIN_SET_TESTING_B
    #define  CONTROLLER_SET_STABLE
    #define  NOTIFIER_SET_STABLE
    #define  PLUGIN_SET_STABLE     // add stable
    // See also PLUGIN_SET_TEST_ESP32 section at end,
    // where incompatible plugins will be disabled.
    // TODO : Check compatibility of plugins for ESP32 board.
#endif

#ifdef PLUGIN_SET_TEST_C_ESP32
    #define PLUGIN_DESCR  "TEST_C ESP32"
    #ifndef ESP32
        #define ESP32
    #endif
    #ifdef ESP8266
        #undef ESP8266
    #endif
//    #define PLUGIN_SET_ONLY_SWITCH

    #define  PLUGIN_SET_TESTING
    #define  PLUGIN_SET_TESTING_C
    #define  CONTROLLER_SET_STABLE
    #define  NOTIFIER_SET_STABLE
    #define  PLUGIN_SET_STABLE     // add stable
    // See also PLUGIN_SET_TEST_ESP32 section at end,
    // where incompatible plugins will be disabled.
    // TODO : Check compatibility of plugins for ESP32 board.
#endif

#ifdef PLUGIN_SET_TEST_D_ESP32
    #define PLUGIN_DESCR  "TEST_D ESP32"
    #ifndef ESP32
        #define ESP32
    #endif
    #ifdef ESP8266
        #undef ESP8266
    #endif
//    #define PLUGIN_SET_ONLY_SWITCH

    #define  PLUGIN_SET_TESTING
    #define  PLUGIN_SET_TESTING_D
    #define  CONTROLLER_SET_STABLE
    #define  NOTIFIER_SET_STABLE
    #define  PLUGIN_SET_STABLE     // add stable
    // See also PLUGIN_SET_TEST_ESP32 section at end,
    // where incompatible plugins will be disabled.
    // TODO : Check compatibility of plugins for ESP32 board.
#endif

#ifdef PLUGIN_BUILD_MAX_ESP32
    #define PLUGIN_DESCR  "MAX ESP32"
    #ifndef ESP32
        #define ESP32
    #endif
    #ifdef ESP8266
        #undef ESP8266
    #endif

    #define PLUGIN_SET_MAX
    #define CONTROLLER_SET_ALL
    #define NOTIFIER_SET_ALL
    #ifndef PLUGIN_ENERGY_COLLECTION
        #define PLUGIN_ENERGY_COLLECTION
    #endif
    #ifndef PLUGIN_DISPLAY_COLLECTION
        #define PLUGIN_DISPLAY_COLLECTION
    #endif
    // See also PLUGIN_SET_MAX section at end, to include any disabled plugins from other definitions
    // See also PLUGIN_SET_TEST_ESP32 section at end,
    // where incompatible plugins will be disabled.
    // TODO : Check compatibility of plugins for ESP32 board.
#endif


// Generic ------------------------------------
#ifdef PLUGIN_SET_GENERIC_1M
    #define PLUGIN_SET_NONE
    // TODO : small list of common plugins to fit in 1M
#endif

// Ventus W266 --------------------------------
#ifdef PLUGIN_SET_VENTUS_W266
    #define PLUGIN_SET_ONLY_SWITCH
    #define PLUGIN_BUILD_DISABLED
    #define USES_P046      // TESTING	Hardware	P046_VentusW266.ino
#endif


#ifdef PLUGIN_SET_LC_TECH_RELAY_X2
    #define CONTROLLER_SET_STABLE
    #define PLUGIN_SET_ONLY_SWITCH
    #define NOTIFIER_SET_STABLE
    #define USES_P026    // Sysinfo
    #define USES_P029    // Domoticz MQTT Helper
    #define USES_P033    // Dummy
    #define USES_P037    // MQTT import
    #define USES_P081    // Cron
    #define USES_P091    // Ser Switch
#endif



/******************************************************************************\
 * "ONLY" shorcuts ************************************************************
\******************************************************************************/
#ifdef PLUGIN_SET_ONLY_SWITCH
    #ifndef PLUGIN_SET_NONE
        #define PLUGIN_SET_NONE
    #endif
    #ifndef USES_P001
        #define USES_P001   // switch
    #endif
    #ifndef USES_P003
//        #define USES_P003   // pulse
    #endif
    #ifndef USES_P026
      #define USES_P026   // SysInfo
    #endif
    #ifndef USES_P033
      #define USES_P033   // Dummy
    #endif
    #ifndef USES_P037
        #define USES_P037   // MQTTImport
    #endif
#endif

#ifdef PLUGIN_SET_ONLY_TEMP_HUM
    #ifndef PLUGIN_SET_NONE
        #define PLUGIN_SET_NONE
    #endif
    #ifndef USES_P004
        #define USES_P004   // Dallas
    #endif
    #ifndef USES_P005
        #define USES_P005   // DHT
    #endif
    #ifndef USES_P014
        #define USES_P014   // SI7021
    #endif
    #ifndef USES_P028
        #define USES_P028   // BME280
    #endif
    #ifndef USES_P034
        #define USES_P034   // DHT12
    #endif
#endif

#ifdef PLUGIN_SET_ONLY_LEDSTRIP
    #ifndef PLUGIN_SET_NONE
        #define PLUGIN_SET_NONE
    #endif
    #ifndef USES_P141
        #define USES_P141   // LedStrip
    #endif
    #ifndef USES_P037
        #define USES_P037   // MQTTImport
    #endif
#endif






/******************************************************************************\
 * Main Families **************************************************************
\******************************************************************************/

// NONE #####################################
#ifdef PLUGIN_SET_NONE
    #ifdef PLUGIN_SET_STABLE
        #undef PLUGIN_SET_STABLE
    #endif
    #ifdef PLUGIN_SET_TESTING
        #undef PLUGIN_SET_TESTING
    #endif
    #ifdef PLUGIN_SET_TESTING_A
        #undef PLUGIN_SET_TESTING_A
    #endif
    #ifdef PLUGIN_SET_TESTING_B
        #undef PLUGIN_SET_TESTING_B
    #endif
    #ifdef PLUGIN_SET_TESTING_C
        #undef PLUGIN_SET_TESTING_C
    #endif
    #ifdef PLUGIN_SET_TESTING_D
        #undef PLUGIN_SET_TESTING_D
    #endif
    #ifdef PLUGIN_SET_EXPERIMENTAL
        #undef PLUGIN_SET_EXPERIMENTAL
    #endif
#endif


#ifdef CONTROLLER_SET_NONE
    #ifdef CONTROLLER_SET_STABLE
        #undef CONTROLLER_SET_STABLE
    #endif
    #ifdef CONTROLLER_SET_TESTING
        #undef CONTROLLER_SET_TESTING
    #endif
    #ifdef CONTROLLER_SET_EXPERIMENTAL
        #undef CONTROLLER_SET_EXPERIMENTAL
    #endif
#endif


#ifdef NOTIFIER_SET_NONE
    #ifdef NOTIFIER_SET_STABLE
        #undef NOTIFIER_SET_STABLE
    #endif
    #ifdef NOTIFIER_SET_TESTING
        #undef NOTIFIER_SET_TESTING
    #endif
    #ifdef NOTIFIER_SET_EXPERIMENTAL
        #undef NOTIFIER_SET_EXPERIMENTAL
    #endif
#endif

// ALL ###########################################
#ifdef PLUGIN_SET_ALL
    #ifndef PLUGIN_SET_STABLE
        #define PLUGIN_SET_STABLE
    #endif
    #ifndef PLUGIN_SET_TESTING
        #define PLUGIN_SET_TESTING
    #endif
    // #ifndef PLUGIN_SET_TESTING_A
    //     #define PLUGIN_SET_TESTING_A
    // #endif
    #ifndef PLUGIN_SET_EXPERIMENTAL
        #define PLUGIN_SET_EXPERIMENTAL
    #endif
#endif


#ifdef CONTROLLER_SET_ALL
    #ifndef CONTROLLER_SET_STABLE
        #define CONTROLLER_SET_STABLE
    #endif
    #ifndef CONTROLLER_SET_TESTING
        #define CONTROLLER_SET_TESTING
    #endif
    #ifndef CONTROLLER_SET_EXPERIMENTAL
        #define CONTROLLER_SET_EXPERIMENTAL
    #endif
#endif


#ifdef NOTIFIER_SET_ALL
    #ifndef NOTIFIER_SET_STABLE
        #define NOTIFIER_SET_STABLE
    #endif
    #ifndef NOTIFIER_SET_TESTING
        #define NOTIFIER_SET_TESTING
    #endif
    #ifndef NOTIFIER_SET_EXPERIMENTAL
        #define NOTIFIER_SET_EXPERIMENTAL
    #endif
#endif

// MAX ###########################################
#ifdef PLUGIN_SET_MAX
    #ifndef PLUGIN_SET_STABLE
        #define PLUGIN_SET_STABLE
    #endif
    #ifndef PLUGIN_SET_TESTING
        #define PLUGIN_SET_TESTING
    #endif
    #ifndef PLUGIN_SET_TESTING_A
        #define PLUGIN_SET_TESTING_A
    #endif
    #ifndef PLUGIN_SET_TESTING_B
        #define PLUGIN_SET_TESTING_B
    #endif
    #ifndef PLUGIN_SET_TESTING_C
        #define PLUGIN_SET_TESTING_C
    #endif
    #ifndef PLUGIN_SET_TESTING_D
        #define PLUGIN_SET_TESTING_D
    #endif
    // #ifndef PLUGIN_SET_EXPERIMENTAL
    //     #define PLUGIN_SET_EXPERIMENTAL
    // #endif
#endif




// STABLE #####################################
#ifdef PLUGIN_SET_STABLE
    #ifndef DONT_USE_SERVO
        #define USE_SERVO
    #endif
    #define USE_RTTTL

    #define USES_P001   // Switch
    #define USES_P002   // ADC
    #define USES_P003   // Pulse
    #define USES_P004   // Dallas
    #define USES_P005   // DHT
    #define USES_P006   // BMP085
    #define USES_P007   // PCF8591
    #define USES_P008   // RFID
    #define USES_P009   // MCP

    #define USES_P010   // BH1750
    #define USES_P011   // PME
    #define USES_P012   // LCD
    #define USES_P013   // HCSR04
    #define USES_P014   // SI7021
    #define USES_P015   // TSL2561
//    #define USES_P016   // IR
    #define USES_P017   // PN532
    #define USES_P018   // Dust
    #define USES_P019   // PCF8574

    #define USES_P020   // Ser2Net
    #define USES_P021   // Level
    #define USES_P022   // PCA9685
    #define USES_P023   // OLED
    #define USES_P024   // MLX90614
    #define USES_P025   // ADS1115
    #define USES_P026   // SysInfo
    #define USES_P027   // INA219
    #define USES_P028   // BME280
    #define USES_P029   // Output

//    #define USES_P030   // BMP280   (Made obsolete, now BME280 can handle both)
    #define USES_P031   // SHT1X
    #define USES_P032   // MS5611
    #define USES_P033   // Dummy
    #define USES_P034   // DHT12
//    #define USES_P035   // IRTX
    #define USES_P036   // FrameOLED
    #define USES_P037   // MQTTImport
    #define USES_P038   // NeoPixel
    #define USES_P039   // Environment - Thermocouple

    #define USES_P040   // RFID - ID12LA/RDM6300
    // FIXME TD-er: Disabled NeoClock and Candle plugin to make builds fit in max bin size.
//    #define USES_P041   // NeoClock
//    #define USES_P042   // Candle
    #define USES_P043   // ClkOutput
    #define USES_P044   // P1WifiGateway

    #define USES_P049   // MHZ19

    #define USES_P052   // SenseAir
    #define USES_P053   // PMSx003

    #define USES_P056   // SDS011-Dust
    #define USES_P059   // Encoder

    #define USES_P063   // TTP229_KeyPad
    #define USES_P073   // 7DG
    #define USES_P079   // Wemos Motoshield
#endif


#ifdef CONTROLLER_SET_STABLE
    #define USES_C001   // Domoticz HTTP
    #define USES_C002   // Domoticz MQTT
    #define USES_C003   // Nodo telnet
    #define USES_C004   // ThingSpeak
    #define USES_C005   // Home Assistant (openHAB) MQTT
    #define USES_C006   // PiDome MQTT
    #define USES_C007   // Emoncms
    #define USES_C008   // Generic HTTP
    #define USES_C009   // FHEM HTTP
    #define USES_C010   // Generic UDP
    #define USES_C013   // ESPEasy P2P network
#endif


#ifdef NOTIFIER_SET_STABLE
    #define USES_N001   // Email
    #define USES_N002   // Buzzer

    #ifdef NOTIFIER_SET_NONE
      #undef NOTIFIER_SET_NONE
    #endif
#endif



// TESTING #####################################
#ifdef PLUGIN_SET_TESTING
  #ifndef PLUGIN_SET_MAX
    #ifndef LIMIT_BUILD_SIZE
      #define LIMIT_BUILD_SIZE
    #endif
    #ifndef NOTIFIER_SET_NONE
      #define NOTIFIER_SET_NONE
    #endif
  #endif // PLUGIN_SET_MAX

    #define USES_P045   // MPU6050
    #define USES_P047   // I2C_soil_misture
    #define USES_P048   // Motoshield_v2

    #define USES_P050   // TCS34725
    #define USES_P051   // AM2320
    #define USES_P054   // DMX512
    #define USES_P055   // Chiming
    #define USES_P057   // HT16K33_LED
    #define USES_P058   // HT16K33_KeyPad

    #define USES_P060   // MCP3221
    #define USES_P061   // Keypad
    #define USES_P062   // MPR121_KeyPad

    #define USES_P064   // APDS9960
    #define USES_P065   // DRF0299
    #define USES_P066   // VEML6040

    #define USES_P075   // Nextion
    //#define USES_P076   // HWL8012   in POW r1
    // Needs CSE7766 Energy sensor, via Serial RXD 4800 baud 8E1 (GPIO1), TXD (GPIO3)
    //#define USES_P077	  // CSE7766   in POW R2
    //#define USES_P078   // Eastron Modbus Energy meters
    #define USES_P081   // Cron
    #define USES_P082   // GPS
    #define USES_P089   // Ping
#endif

#ifdef PLUGIN_SET_TESTING_A

    #define USES_P067   // HX711_Load_Cell
    #define USES_P068   // SHT3x

    #define USES_P070   // NeoPixel_Clock
    #define USES_P071   // Kamstrup401
    #define USES_P072   // HDC1080
    #define USES_P074   // TSL2561

    #define USES_P080   // iButton Sensor  DS1990A
    #define USES_P083   // SGP30
    #define USES_P084   // VEML6070
    #define USES_P086   // Receiving values according Homie convention. Works together with C014 Homie controller

    #define USES_P090   // CCS811 TVOC/eCO2 Sensor

    //#define USES_P095  // TFT ILI9341
    //#define USES_P096  // eInk   (Needs lib_deps = Adafruit GFX Library, LOLIN_EPD )
    #define USES_P097   // Touch (ESP32)
    //#define USES_P099   // XPT2046 Touchscreen
#endif

#ifdef PLUGIN_SET_TESTING_B
    #define USES_P069   // LM75A

    #define USES_P100   // Pulse Counter - DS2423
    #define USES_P101   // Wake On Lan
    #define USES_P103   // Atlas Scientific EZO Sensors (pH, ORP, EZO)
    #define USES_P106   // BME680
    #define USES_P107   // SI1145 UV index
    #define USES_P108   // DDS238-x ZN MODBUS energy meter (was P224 in the Playground)

    #define USES_P110   // VL53L0X Time of Flight sensor
    #define USES_P113   // VL53L1X ToF
#endif

#ifdef PLUGIN_SET_TESTING_C
    #define USES_P085   // AcuDC24x
    #define USES_P087   // Serial Proxy

    #define USES_P091	// SerSwitch
    #define USES_P092   // DL-Bus

    #define USES_P111   // RC522 RFID reader
#endif

#ifdef PLUGIN_SET_TESTING_D
    #define USES_P093   // Mitsubishi Heat Pump
    #define USES_P094  // CUL Reader
    #define USES_P114  // VEML6075 UVA/UVB sensor
    #define USES_P115  // Fuel Gauge MAX1704x
#endif


// Collection of all energy related plugins.
#ifdef PLUGIN_ENERGY_COLLECTION
   #ifndef USES_P025
     #define USES_P025   // ADS1115
   #endif
   #ifndef USES_P027
     #define USES_P027   // INA219
   #endif
   #ifndef USES_P076 
     #define USES_P076   // HWL8012   in POW r1
   #endif
   #ifndef USES_P077 
     // Needs CSE7766 Energy sensor, via Serial RXD 4800 baud 8E1 (GPIO1), TXD (GPIO3)
     #define USES_P077	  // CSE7766   in POW R2
   #endif
   #ifndef USES_P078 
     #define USES_P078   // Eastron Modbus Energy meters
   #endif
   #ifndef USES_P085
     #define USES_P085   // AcuDC24x
   #endif
   #ifndef USES_P093
     #define USES_P093   // Mitsubishi Heat Pump
   #endif
   #ifndef USES_P102
     #define USES_P102   // PZEM-004Tv30
   #endif
   #ifndef USES_P108 
     #define USES_P108   // DDS238-x ZN MODBUS energy meter (was P224 in the Playground)
   #endif
   #ifndef USES_P115
     #define USES_P115   // Fuel Gauge MAX1704x
   #endif
#endif

// Collection of all display plugins. (also NeoPixel)
#ifdef PLUGIN_DISPLAY_COLLECTION
   #ifndef USES_P012
     #define USES_P012   // LCD
   #endif
   #ifndef USES_P023
    #define USES_P023   // OLED
   #endif
   #ifndef USES_P036 
    #define USES_P036   // FrameOLED
   #endif
   #ifndef USES_P038 
    #define USES_P038   // NeoPixel
   #endif
   #ifndef USES_P041 
    #define USES_P041   // NeoClock
   #endif
   #ifndef USES_P042 
    #define USES_P042   // Candle
   #endif
   #ifndef USES_P057 
    #define USES_P057   // HT16K33_LED
   #endif
   #ifndef USES_P070 
    #define USES_P070   // NeoPixel_Clock
   #endif
   #ifndef USES_P075 
    #define USES_P075   // Nextion
   #endif
   #ifndef USES_P095 
    #define USES_P095  // TFT ILI9341
   #endif
   #ifndef USES_P096 
    #define USES_P096  // eInk   (Needs lib_deps = Adafruit GFX Library, LOLIN_EPD )
   #endif
   #ifndef USES_P099
    #define USES_P099   // XPT2046 Touchscreen
   #endif
#endif


#ifdef CONTROLLER_SET_TESTING
    #define USES_C011   // Generic HTTP Advanced
    #define USES_C012   // Blynk HTTP
    #define USES_C014   // homie 3 & 4dev MQTT
    //#define USES_C015   // Blynk
    #define USES_C017   // Zabbix
    // #define USES_C018 // TTN RN2483
#endif


#ifdef NOTIFIER_SET_TESTING
#endif


// EXPERIMENTAL (playground) #######################
#ifdef PLUGIN_SET_EXPERIMENTAL
    #define USES_P046   // VentusW266
    #define USES_P050   // TCS34725 RGB Color Sensor with IR filter and White LED
    #define USES_P064   // APDS9960 Gesture
    #define USES_P077	// CSE7766   Was P134 on Playground


    // [copied from Playground as of 6 March 2018]
    // It needs some cleanup as some are ALSO in the main repo,
    // thus they should have been removed from the Playground repo
    // #define USES_P100	// Was SRF01, now Pulse Counter - DS2423 [Testing]
	// #define USES_P101	// Was NeoClock, now Wake On Lan [Testing]
	#define USES_P102	// Nodo
	#define USES_P103	// Event
	#define USES_P104	// SRF02
	#define USES_P105	// RGBW
	#define USES_P106	// IRTX
	#define USES_P107	// Email_Demo
	#define USES_P108	// WOL
	#define USES_P109	// RESOL_DeltaSol_Pro
	   #define USES_P110	// P1WifiGateway      (MERGED?)
	#define USES_P111	// RF
	   //#define USES_P111	// SenseAir     (MERGED?)
	#define USES_P112	// Power
	//#define USES_P112	// RFTX
	#define USES_P113	// SI1145
	#define USES_P114	// DSM501
	//#define USES_P115	// HeatpumpIR - P088 in the main repo.
	#define USES_P116	// ID12
	#define USES_P117	// LW12FC
	//#define USES_P117	// Neopixels
	//#define USES_P117	// Nextion
	#define USES_P118	// CCS811
	#define USES_P119	// BME680
	#define USES_P120	// Thermocouple
	#define USES_P121	// Candle
	   #define USES_P122	// NeoPixel       (MERGED?)
	      #define USES_P123	// NeoPixel_Clock  (MERGED?)
	#define USES_P124	// NeoPixelBusFX
	//#define USES_P124	// Ventus_W266_RFM69
	#define USES_P125	// ArduCAM
	#define USES_P127	// Teleinfo
	#define USES_P130	// VEML6075
	#define USES_P131	// SHT3X
	#define USES_P133	// VL53L0X
	#define USES_P141	// LedStrip
	#define USES_P142	// RGB-Strip
	#define USES_P143	// AnyonePresent
	#define USES_P144	// RC-Switch-TX
	#define USES_P145	// Itho
	#define USES_P149	// MHZ19
	#define USES_P150	// SDM120C
	#define USES_P151	// CISA
	#define USES_P153	// MAX44009
	#define USES_P162	// MPL3115A2
	#define USES_P163	// DS1631
	#define USES_P165	// SerSwitch
	#define USES_P166	// WiFiMan
	#define USES_P167	// ADS1015
	#define USES_P170	// HLW8012
	#define USES_P171	// PZEM-004T
	#define USES_P180	// Mux
	#define USES_P181	// TempHumidity_SHT2x
	#define USES_P182	// MT681
	#define USES_P199	// RF443_KaKu
	#define USES_P202	// ADC_ACcurrentSensor
	   #define USES_P205	// FrameOLED      (MERGED?)
	#define USES_P209	// IFTTTMaker
	   #define USES_P210	// MQTTImport     (MERGED?)
	#define USES_P211	// MPU6050
	#define USES_P212	// MY9291
	#define USES_P213	// VEML6070
#endif


#ifdef CONTROLLER_SET_EXPERIMENTAL
  //#define USES_C016   // Cache controller
  //#define USES_C018   // TTN/RN2483
#endif


#ifdef NOTIFIER_SET_EXPERIMENTAL
#endif


// Maximized build definition for an ESP(32) with 16MB Flash and 4MB sketch partition
// Add all plugins, controllers and features that don't fit in the TESTING set
#ifdef PLUGIN_SET_MAX
  // Features
  #ifndef USE_SERVO
    #define USE_SERVO
  #endif
  #ifndef USE_RTTTL
    #define USE_RTTTL
  #endif
  #ifndef USE_SETTINGS_ARCHIVE
    #define USE_SETTINGS_ARCHIVE
  #endif
  #ifndef FEATURE_SD
    #define FEATURE_SD
  #endif

  // Plugins
  #ifndef USES_P016
//    #define USES_P016   // IR
  #endif
  #ifndef USES_P035
//    #define USES_P035   // IRTX
  #endif
  #ifndef USES_P041
    #define USES_P041   // NeoClock
  #endif
  #ifndef USES_P042
    #define USES_P042   // Candle
  #endif
  #ifndef USES_P087
    #define USES_P087   // Serial Proxy
  #endif
  #ifndef USES_P094
    #define USES_P094  // CUL Reader
  #endif
  #ifndef USES_P095
    #define USES_P095  // TFT ILI9341
  #endif
  #ifndef USES_P096
    #define USES_P096  // eInk   (Needs lib_deps = Adafruit GFX Library, LOLIN_EPD )
  #endif
  #ifndef USES_P098
    #define USES_P098   // ESPEasy-NOW Receiver
  #endif
  #ifndef USES_P099
    #define USES_P099   // XPT2046 Touchscreen
  #endif
  #ifndef USES_P102
    #define USES_P102   // PZEM004Tv3
  #endif
  #ifndef USES_P103
    #define USES_P103   // Atlas EZO pH
  #endif
  #ifndef USES_P104
    #define USES_P104   // Atlas EZO EC
  #endif
  #ifndef USES_P105
    #define USES_P105   // Atlas EZO_ORP
  #endif
  #ifndef USES_P108
    #define USES_P108   // DDS238-x ZN MODBUS energy meter (was P224 in the Playground)
  #endif
  #ifndef USES_P109
    #define USES_P109   // ThermOLED
  #endif
  #ifndef USES_P110
    #define USES_P110   // VL53L0X
  #endif
  #ifndef USES_P111
    #define USES_P111   // RC522 RFID reader
  #endif
  #ifndef USES_P112
    #define USES_P112   // AS7256x
  #endif
  #ifndef USES_P113
    #define USES_P113   // VL53L1X
  #endif
  #ifndef USES_P114
    #define USES_P114   // VEML6075 UVA/UVB sensor
  #endif
  #ifndef USES_P115
    #define USES_P115   // 
  #endif
  #ifndef USES_P116
    #define USES_P116   // 
  #endif
  #ifndef USES_P117
    #define USES_P117   // 
  #endif
  #ifndef USES_P118
    #define USES_P118   // 
  #endif
  #ifndef USES_P119
    #define USES_P119   // 
  #endif
  #ifndef USES_P120
    #define USES_P120   // 
  #endif
  #ifndef USES_P121
    #define USES_P121   // 
  #endif
  #ifndef USES_P122
    #define USES_P122   // 
  #endif
  #ifndef USES_P123
    #define USES_P123   // 
  #endif
  #ifndef USES_P124
    #define USES_P124   // 
  #endif
  #ifndef USES_P125
    #define USES_P125   // 
  #endif

  // Controllers
  #ifndef USES_C015
    #ifndef ESP32
      #define USES_C015   // Blynk (?doesn't compile on ESP32?)
    #endif
  #endif
  #ifndef USES_C016
    #define USES_C016   // Cache controller
  #endif
  #ifndef USES_C018
    #define USES_C018 // TTN RN2483
  #endif

  // Notifiers

#endif // PLUGIN_SET_MAX


/******************************************************************************\
 * Remove incompatible plugins ************************************************
\******************************************************************************/
#ifdef ESP32
//  #undef USES_P010   // BH1750          (doesn't work yet on ESP32)
//  #undef USES_P049   // MHZ19           (doesn't work yet on ESP32)

//  #undef USES_P052   // SenseAir        (doesn't work yet on ESP32)
//  #undef USES_P053   // PMSx003

//  #undef USES_P056   // SDS011-Dust     (doesn't work yet on ESP32)
//  #undef USES_P065   // DRF0299
//  #undef USES_P071   // Kamstrup401
//  #undef USES_P075   // Nextion
//  #undef USES_P078   // Eastron Modbus Energy meters (doesn't work yet on ESP32)
//  #undef USES_P082   // GPS
#endif


#ifdef ARDUINO_ESP8266_RELEASE_2_3_0
  #ifdef USES_P081
    #undef USES_P081   // Cron
  #endif


#endif


/******************************************************************************\
 * Libraries dependencies *****************************************************
\******************************************************************************/
#if defined(USES_P020) || defined(USES_P049) || defined(USES_P052) || defined(USES_P053) || defined(USES_P056) || defined(USES_P071) || defined(USES_P075) || defined(USES_P078) || defined(USES_P082) || defined(USES_P085) || defined(USES_P087) || defined(USES_P094) || defined(USES_P102) || defined(USES_P108) || defined(USES_C018)
  // At least one plugin uses serial.
  #ifndef PLUGIN_USES_SERIAL
    #define PLUGIN_USES_SERIAL
  #endif
#else
  // No plugin uses serial, so make sure software serial is not included.
  #define DISABLE_SOFTWARE_SERIAL
#endif


/*
#if defined(USES_P00x) || defined(USES_P00y)
#include <the_required_lib.h>
#endif
*/


#if defined(USES_C018)
  #define USES_PACKED_RAW_DATA
#endif

#if defined(USES_P085) || defined (USES_P052) || defined(USES_P078) || defined(USES_P108)
  // FIXME TD-er: Is this correct? Those plugins use Modbus_RTU.
  #define USES_MODBUS
#endif

#if defined(USES_C001) || defined (USES_C002) || defined(USES_P029)
  #ifndef USES_DOMOTICZ
    #define USES_DOMOTICZ
  #endif
#endif

#ifdef USES_DOMOTICZ  // Move Domoticz enabling logic together
    #ifndef USES_C001
      #define USES_C001   // Domoticz HTTP
    #endif
    #ifndef USES_C002
      #define USES_C002   // Domoticz MQTT
    #endif
    #ifndef USES_P029
      #define USES_P029   // Output
    #endif
#endif


// Disable Homie plugin for now in the dev build to make it fit.
#if defined(PLUGIN_BUILD_DEV) && defined(USES_C014)
  #undef USES_C014
#endif

// VCC builds need a bit more, disable timing stats to make it fit.
#if defined(FEATURE_ADC_VCC) && !defined(PLUGIN_SET_MAX)
  #ifndef LIMIT_BUILD_SIZE
    #define LIMIT_BUILD_SIZE
  #endif
  #ifndef NOTIFIER_SET_NONE
    #define NOTIFIER_SET_NONE
  #endif

#endif


// Due to size restrictions, disable a few plugins/controllers for 1M builds
#ifdef SIZE_1M
  #ifdef USES_C003
    #undef USES_C003
  #endif
  #ifdef USES_C016
    #undef USES_C016  // Cache controller
  #endif
  #ifdef FEATURE_SD
    #undef FEATURE_SD  // Unlikely on 1M units
  #endif
  #ifndef LIMIT_BUILD_SIZE
    #define LIMIT_BUILD_SIZE
  #endif
#endif

// Disable some diagnostic parts to make builds fit.
#ifdef LIMIT_BUILD_SIZE
  #ifdef WEBSERVER_TIMINGSTATS
    #undef WEBSERVER_TIMINGSTATS
  #endif

  // Do not include large blobs but fetch them from CDN
  #ifndef WEBSERVER_USE_CDN_JS_CSS
    #define WEBSERVER_USE_CDN_JS_CSS
  #endif

  #ifndef BUILD_NO_DEBUG
    #define BUILD_NO_DEBUG
  #endif
  #ifndef BUILD_NO_SPECIAL_CHARACTERS_STRINGCONVERTER
    #define BUILD_NO_SPECIAL_CHARACTERS_STRINGCONVERTER
  #endif
  #ifdef FEATURE_I2CMULTIPLEXER
    #undef FEATURE_I2CMULTIPLEXER
  #endif
  #ifdef USE_SETTINGS_ARCHIVE
    #undef USE_SETTINGS_ARCHIVE
  #endif

  #ifdef USE_SERVO
    #undef USE_SERVO
  #endif
  #ifdef USE_RTTTL
    #undef USE_RTTTL
  #endif
  #ifdef USES_BLYNK
    #undef USES_BLYNK
  #endif
  #ifndef PLUGIN_SET_TESTING
    #ifdef USES_P076
      #undef USES_P076   // HWL8012   in POW r1
    #endif
    #ifdef USES_P093
      #undef USES_P093   // Mitsubishi Heat Pump
    #endif
    #ifdef USES_P100 // Pulse Counter - DS2423
      #undef USES_P100
    #endif
  #endif
  #ifdef USES_C012
    #undef USES_C012 // Blynk
  #endif
  #ifdef USES_C015
    #undef USES_C015 // Blynk
  #endif
  #ifdef USES_C016
    #undef USES_C016 // Cache controller
  #endif
  #ifdef USES_C017 // Zabbix
    #undef USES_C017
  #endif
  #ifdef USES_C018
    #undef USES_C018 // LoRa TTN - RN2483/RN2903
  #endif
  #if defined(USE_TRIGONOMETRIC_FUNCTIONS_RULES) && !defined(KEEP_TRIGONOMETRIC_FUNCTIONS_RULES)
    #undef USE_TRIGONOMETRIC_FUNCTIONS_RULES
  #endif
  #ifdef USES_SSDP
    #undef USES_SSDP
  #endif

#endif

// Timing stats page needs timing stats
#if defined(WEBSERVER_TIMINGSTATS) && !defined(USES_TIMING_STATS)
  #define USES_TIMING_STATS
#endif

// If timing stats page is not included, there is no need in collecting the stats
#if !defined(WEBSERVER_TIMINGSTATS) && defined(USES_TIMING_STATS)
  #undef USES_TIMING_STATS
#endif


#ifdef BUILD_NO_DEBUG
  #ifndef BUILD_NO_DIAGNOSTIC_COMMANDS
    #define BUILD_NO_DIAGNOSTIC_COMMANDS
  #endif
  #ifndef BUILD_NO_RAM_TRACKER
    #define BUILD_NO_RAM_TRACKER
  #endif
#endif

  // Do not include large blobs but fetch them from CDN
#ifdef WEBSERVER_USE_CDN_JS_CSS
  #ifdef WEBSERVER_FAVICON
    #ifndef WEBSERVER_FAVICON_CDN
      #define WEBSERVER_FAVICON_CDN
    #endif
  #endif
  #ifdef WEBSERVER_CSS
    #undef WEBSERVER_CSS
  #endif
  #ifdef WEBSERVER_INCLUDE_JS
    #undef WEBSERVER_INCLUDE_JS
  #endif
#endif

#if defined(USES_C002) || defined (USES_C005) || defined(USES_C006) || defined(USES_C014) || defined(USES_P037)
  #define USES_MQTT
#endif

#if defined(USES_C012) || defined (USES_C015)
  #define USES_BLYNK
#endif

// Specific notifier plugins may be enabled via Custom.h, regardless
// whether NOTIFIER_SET_NONE is defined
#if defined(USES_N001) || defined(USES_N002)
  #ifndef USES_NOTIFIER
    #define USES_NOTIFIER
  #endif
#endif


#ifdef USES_MQTT
// MQTT_MAX_PACKET_SIZE : Maximum packet size
#ifndef MQTT_MAX_PACKET_SIZE
  #define MQTT_MAX_PACKET_SIZE 1024 // Is also used in PubSubClient
#endif
#endif //USES_MQTT


// It may have gotten undefined to fit a build. Make sure the Blynk controllers are not defined
#ifndef USES_BLYNK
  #ifdef USES_C012
    #undef USES_C012
  #endif
  #ifdef USES_C015
    #undef USES_C015
  #endif
#endif

#ifdef FEATURE_ARDUINO_OTA
  #ifndef FEATURE_MDNS
    #define FEATURE_MDNS
  #endif
#endif

#ifdef FEATURE_MDNS
  #ifndef FEATURE_DNS_SERVER
    #define FEATURE_DNS_SERVER
  #endif
#endif

#ifdef WEBSERVER_SETUP
  #ifndef PLUGIN_BUILD_MINIMAL_OTA
    #ifndef FEATURE_DNS_SERVER
      #define FEATURE_DNS_SERVER
    #endif
  #endif
#endif

// Here we can re-enable specific features in the TESTING sets as we have created some space there by splitting them up
#if defined(TESTING_USE_RTTTL) && (defined(PLUGIN_SET_TESTING_A) || defined(PLUGIN_SET_TESTING_B) || defined(PLUGIN_SET_TESTING_C) || defined(PLUGIN_SET_TESTING_D))
  #ifndef USE_RTTTL
    #define USE_RTTTL
  #endif
#endif

#endif // DEFINE_PLUGIN_SETS_H
