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

 When found stable enought, the maintainer (and only him) will choose to move it to TESTING or STABLE
*/




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

#ifdef PLUGIN_BUILD_NORMAL_IR
    #define PLUGIN_BUILD_NORMAL     // add stable
    #define PLUGIN_BUILD_IR
#endif

#ifdef PLUGIN_BUILD_DEV
    #define  PLUGIN_SET_EXPERIMENTAL
    #define  CONTROLLER_SET_EXPERIMENTAL
    #define  NOTIFIER_SET_EXPERIMENTAL
    #define  PLUGIN_BUILD_TESTING   // add testing
#endif

#ifdef PLUGIN_BUILD_TESTING
    #define  PLUGIN_SET_TESTING
    #define  CONTROLLER_SET_TESTING
    #define  NOTIFIER_SET_TESTING
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
#endif

#ifdef PLUGIN_BUILD_MINIMAL_OTA
    #define PLUGIN_DESCR  "Minimal 1M OTA"

    #define CONTROLLER_SET_NONE

    #define BUILD_MINIMAL_OTA
    #define BUILD_NO_DEBUG

    #define USES_C001   // Domoticz HTTP
    #define USES_C002   // Domoticz MQTT
    #define USES_C005   // OpenHAB MQTT
//    #define USES_C006   // PiDome MQTT
    #define USES_C008   // Generic HTTP
    #define USES_C009   // FHEM HTTP
//    #define USES_C010   // Generic UDP
    #define USES_C013   // ESPEasy P2P network

//    #define NOTIFIER_SET_STABLE
    #define NOTIFIER_SET_NONE

    #define PLUGIN_SET_NONE

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
#endif


#ifndef BUILD_MINIMAL_OTA
  #ifndef WEBSERVER_TIMINGSTATS
    #define WEBSERVER_TIMINGSTATS
  #endif
  #ifndef WEBSERVER_SYSVARS
    #define WEBSERVER_SYSVARS
  #endif
  #ifndef WEBSERVER_NEW_UI
    #define WEBSERVER_NEW_UI
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
    #define USES_P035      // IRTX
#endif


/******************************************************************************\
 * Devices ********************************************************************
\******************************************************************************/

// Itead ----------------------------
#ifdef PLUGIN_SET_SONOFF_BASIC
    #define PLUGIN_DESCR  "Sonoff Basic"

    #define PLUGIN_SET_ONLY_SWITCH
#endif

#ifdef PLUGIN_SET_SONOFF_TH1x
    #define PLUGIN_DESCR  "Sonoff TH10/TH16"

    #define PLUGIN_SET_ONLY_SWITCH
    #define PLUGIN_SET_ONLY_TEMP_HUM
#endif

#ifdef PLUGIN_SET_SONOFF_POW
    #define PLUGIN_DESCR  "Sonoff POW R1/R2"

    #define PLUGIN_SET_ONLY_SWITCH
    #define USES_P076   // HWL8012   in POW r1
    // Needs CSE7766 Energy sensor, via Serial RXD 4800 baud 8E1 (GPIO1), TXD (GPIO3)
    #define USES_P077	  // CSE7766   in POW R2
    #define USES_P081   // Cron
#endif

#ifdef PLUGIN_SET_SONOFF_S2x
    #define PLUGIN_DESCR  "Sonoff S20/22/26"

    #define PLUGIN_SET_ONLY_SWITCH
#endif

#ifdef PLUGIN_SET_SONOFF_4CH
    #define PLUGIN_DESCR  "Sonoff 4CH"
    #define PLUGIN_SET_ONLY_SWITCH
#endif

#ifdef PLUGIN_SET_SONOFF_TOUCH
    #define PLUGIN_DESCR  "Sonoff Touch"
    #define PLUGIN_SET_ONLY_SWITCH
#endif

// Shelly ----------------------------
#ifdef PLUGIN_SET_SHELLY_1
    #define PLUGIN_DESCR  "Shelly 1"

    #define PLUGIN_SET_ONLY_SWITCH
    #define CONTROLLER_SET_STABLE
    #define NOTIFIER_SET_STABLE
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
    #define USES_P036   // FrameOLED
#endif

#ifdef PLUGIN_SET_EASY_OLED2
    #define PLUGIN_SET_ONLY_SWITCH
    #define USES_P023   // OLED
#endif

#ifdef PLUGIN_SET_EASY_RELAY
    #define PLUGIN_SET_ONLY_SWITCH
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
    #define USES_P036   // FrameOLED
    #define USES_P027   // INA219
    #define USES_P028   // BME280
#endif

#ifdef PLUGIN_SET_TEST_ESP32
    #define PLUGIN_DESCR  "TEST ESP32"
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
    #ifndef USES_P028
        #define USES_P028   // BME280
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




// STABLE #####################################
#ifdef PLUGIN_SET_STABLE
    #define USE_SERVO

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

    #define USES_P030   // BMP280
    #define USES_P031   // SHT1X
    #define USES_P032   // MS5611
    #define USES_P033   // Dummy
    #define USES_P034   // DHT12
//    #define USES_P035   // IRTX
    #define USES_P036   // FrameOLED
    #define USES_P037   // MQTTImport
    #define USES_P038   // NeoPixel
    #define USES_P039   // ID12

    #define USES_P041   // NeoClock
    #define USES_P042   // Candle
    #define USES_P043   // ClkOutput
    #define USES_P044   // P1WifiGateway

    #define USES_P049   // MHZ19

    #define USES_P052   // SenseAir
    #define USES_P056   // SDS011-Dust
    #define USES_P059   // Encoder

    #define USES_P063   // TTP229_KeyPad
#endif


#ifdef CONTROLLER_SET_STABLE
    #define USES_C001   // Domoticz HTTP
    #define USES_C002   // Domoticz MQTT
    #define USES_C003   // Nodo telnet
    #define USES_C004   // ThingSpeak
    #define USES_C005   // OpenHAB MQTT
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
    #define USES_P045   // MPU6050
    #define USES_P047   // I2C_soil_misture
    #define USES_P048   // Motoshield_v2

    #define USES_P051   // AM2320

    #define USES_P053   // PMSx003
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
    #define USES_P067   // HX711_Load_Cell
    #define USES_P068   // SHT3x
    #define USES_P069   // LM75A

    #define USES_P070   // NeoPixel_Clock
    #define USES_P071   // Kamstrup401
    #define USES_P072   // HDC1080
    #define USES_P073   // 7DG
    #define USES_P074   // TSL2561
    #define USES_P075   // Nextion
    #define USES_P076   // HWL8012   in POW r1
    // Needs CSE7766 Energy sensor, via Serial RXD 4800 baud 8E1 (GPIO1), TXD (GPIO3)
    #define USES_P077	  // CSE7766   in POW R2
    #define USES_P078   // Eastron Modbus Energy meters
    #define USES_P079   // Wemos Motoshield
    #define USES_P080   // iButton Sensor  DS1990A
    #define USES_P081   // Cron
    #define USES_P082   // GPS
    #define USES_P083   // MQ-5 gas sensor
#endif


#ifdef CONTROLLER_SET_TESTING
    #define USES_C011   // Generic HTTP Advanced
    #define USES_C012   // Blynk HTTP
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
    #define USES_P100	// SRF01
	   #define USES_P101	// NeoClock       (MERGED?)
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
	#define USES_P115	// HeatpumpIR
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
#endif


#ifdef NOTIFIER_SET_EXPERIMENTAL
#endif



/******************************************************************************\
 * Remove incompatible plugins ************************************************
\******************************************************************************/
#ifdef PLUGIN_SET_TEST_ESP32
  #undef USES_P010   // BH1750          (doesn't work yet on ESP32)
//  #undef USES_P049   // MHZ19           (doesn't work yet on ESP32)

//  #undef USES_P052   // SenseAir        (doesn't work yet on ESP32)
//  #undef USES_P053   // PMSx003

//  #undef USES_P056   // SDS011-Dust     (doesn't work yet on ESP32)
//  #undef USES_P065   // DRF0299
//  #undef USES_P071   // Kamstrup401
  #undef USES_P075   // Nextion
//  #undef USES_P078   // Eastron Modbus Energy meters (doesn't work yet on ESP32)
//  #undef USES_P082   // GPS

  #ifdef USE_SERVO
    #undef USE_SERVO
  #endif

  #ifdef USES_P083
    // analogRead yields different values for ESP32
    #undef USES_P083   // MQ-5 gas sensor
  #endif
#endif


#ifdef ARDUINO_ESP8266_RELEASE_2_3_0
  #ifdef USES_P081
    #undef USES_P081   // Cron
  #endif


#endif


/******************************************************************************\
 * Libraries dependencies *****************************************************
\******************************************************************************/
#if defined(USES_P049) || defined(USES_P052) || defined(USES_P053) || defined(USES_P056) || defined(USES_P071) || defined(USES_P075) || defined(USES_P082)
// At least one plugin uses serial.
#else
  // No plugin uses serial, so make sure software serial is not included.
  #define DISABLE_SOFTWARE_SERIAL
#endif

/*
#if defined(USES_P00x) || defined(USES_P00y)
#include <the_required_lib.h>
#endif
*/
