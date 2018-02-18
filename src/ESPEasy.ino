#ifdef CONTINUOUS_INTEGRATION
#pragma GCC diagnostic error "-Wall"
#else
#pragma GCC diagnostic warning "-Wall"
#endif

/****************************************************************************************************************************\
 * Arduino project "ESP Easy" © Copyright www.letscontrolit.com
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You received a copy of the GNU General Public License along with this program in file 'License.txt'.
 *
 * IDE download    : https://www.arduino.cc/en/Main/Software
 * ESP8266 Package : https://github.com/esp8266/Arduino
 *
 * Source Code     : https://github.com/ESP8266nu/ESPEasy
 * Support         : http://www.letscontrolit.com
 * Discussion      : http://www.letscontrolit.com/forum/
 *
 * Additional information about licensing can be found at : http://www.gnu.org/licenses
\*************************************************************************************************************************/

// This file incorporates work covered by the following copyright and permission notice:

/****************************************************************************************************************************\
* Arduino project "Nodo" © Copyright 2010..2015 Paul Tonkes
*
* This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
* You received a copy of the GNU General Public License along with this program in file 'License.txt'.
*
* Voor toelichting op de licentievoorwaarden zie    : http://www.gnu.org/licenses
* Uitgebreide documentatie is te vinden op          : http://www.nodo-domotica.nl
* Compiler voor deze programmacode te downloaden op : http://arduino.cc
\*************************************************************************************************************************/

//   Simple Arduino sketch for ESP module, supporting:
//   =================================================================================
//   Simple switch inputs and direct GPIO output control to drive relais, mosfets, etc
//   Analog input (ESP-7/12 only)
//   Pulse counters
//   Dallas OneWire DS18b20 temperature sensors
//   DHT11/22/12 humidity sensors
//   BMP085 I2C Barometric Pressure sensor
//   PCF8591 4 port Analog to Digital converter (I2C)
//   RFID Wiegand-26 reader
//   MCP23017 I2C IO Expanders
//   BH1750 I2C Luminosity sensor
//   Arduino Pro Mini with IO extender sketch, connected through I2C
//   LCD I2C display 4x20 chars
//   HC-SR04 Ultrasonic distance sensor
//   SI7021 I2C temperature/humidity sensors
//   TSL2561 I2C Luminosity sensor
//   TSOP4838 IR receiver
//   PN532 RFID reader
//   Sharp GP2Y10 dust sensor
//   PCF8574 I2C IO Expanders
//   PCA9685 I2C 16 channel PWM driver
//   OLED I2C display with SSD1306 driver
//   MLX90614 I2C IR temperature sensor
//   ADS1115 I2C ADC
//   INA219 I2C voltage/current sensor
//   BME280 I2C temp/hum/baro sensor
//   MSP5611 I2C temp/baro sensor
//   BMP280 I2C Barometric Pressure sensor
//   SHT1X temperature/humidity sensors
//   Ser2Net server

// ********************************************************************************
//   User specific configuration
// ********************************************************************************

// Set default configuration settings if you want (not mandatory)
// You can always change these during runtime and save to eeprom
// After loading firmware, issue a 'reset' command to load the defaults.

// --- Basic Config Settings ------------------------------------------------------------------------
#define DEFAULT_NAME        "ESP_Easy"			// Enter your device friendly name
#define UNIT				0					// Unit Number
#define DEFAULT_DELAY       60                  // Sleep Delay in seconds

// --- Wifi AP Mode (when your Wifi Network is not reachable) ----------------------------------------
#define DEFAULT_AP_IP      	192,168,4,1         // Enter IP address (comma separated) for AP (config) mode
#define DEFAULT_AP_KEY      "configesp"         // Enter network WPA key for AP (config) mode

// --- Wifi Client Mode -----------------------------------------------------------------------------
#define DEFAULT_SSID        "ssid"              // Enter your Wifi network SSID
#define DEFAULT_KEY         "wpakey"            // Enter your Wifi network WPA key

#define DEFAULT_USE_STATIC_IP   false           // (true|false) enabled or disabled static IP
#define DEFAULT_IP          "192.168.0.50"      // Enter your IP address
#define DEFAULT_DNS         "192.168.0.1"       // Enter your DNS
#define DEFAULT_GW          "192.168.0.1"       // Enter your Gateway
#define DEFAULT_SUBNET      "255.255.255.0"     // Enter your Subnet

// --- Default Controller ------------------------------------------------------------------------------
#define DEFAULT_CONTROLLER   false              // true or false enabled or disabled, set 1st controller defaults
// using a default template, you also need to set a DEFAULT PROTOCOL to a suitable MQTT protocol !
#define DEFAULT_PUB         "sensors/espeasy/%sysname%/%tskname%/%valname%" // Enter your pub
#define DEFAULT_SUB         "sensors/espeasy/%sysname%/#" // Enter your sub
#define DEFAULT_SERVER      "192.168.0.8"       // Enter your Server IP address
#define DEFAULT_PORT        8080                // Enter your Server port value

#define DEFAULT_PROTOCOL    1                   // Protocol used for controller communications
//   1 = Domoticz HTTP
//   2 = Domoticz MQTT
//   3 = Nodo Telnet
//   4 = ThingSpeak
//   5 = OpenHAB MQTT
//   6 = PiDome MQTT
//   7 = EmonCMS
//   8 = Generic HTTP
//   9 = FHEM HTTP


// --- Advanced Settings ---------------------------------------------------------------------------------
#define DEFAULT_USE_RULES			false	// (true|false) Enable Rules?

#define DEFAULT_MQTT_RETAIN			false	// (true|false) Retain MQTT messages?
#define DEFAULT_MQTT_DELAY			1000	// Time in milliseconds to retain MQTT messages

#define DEFAULT_USE_NTP				false	// (true|false) Use NTP Server
#define DEFAULT_NTP_HOST			""		// NTP Server Hostname
#define DEFAULT_TIME_ZONE			0		// Time Offset (in minutes)
#define DEFAULT_USE_DST				false	// (true|false) Use Daily Time Saving

#define LOG_TO_SERIAL         1
#define LOG_TO_SYSLOG         2
#define LOG_TO_WEBLOG         3
#define LOG_TO_SDCARD         4
#define DEFAULT_SYSLOG_IP			""				// Syslog IP Address
#define DEFAULT_SYSLOG_LEVEL		0				// Syslog Log Level
#define DEFAULT_SERIAL_LOG_LEVEL	LOG_LEVEL_INFO	// Serial Log Level
#define DEFAULT_WEB_LOG_LEVEL		LOG_LEVEL_INFO	// Web Log Level
#define DEFAULT_SD_LOG_LEVEL		0				// SD Card Log Level
#define DEFAULT_USE_SD_LOG			false			// (true|false) Enable Logging to the SD card

#define DEFAULT_USE_SERIAL			true	// (true|false) Enable Logging to the Serial Port
#define DEFAULT_SERIAL_BAUD			115200	// Serial Port Baud Rate

/*
// --- Experimental Advanced Settings (NOT ACTIVES at this time) ------------------------------------

#define DEFAULT_USE_GLOBAL_SYNC			false		// (true|false)
#define DEFAULT_SYNC_UDP_PORT			0			//

#define DEFAULT_IP_OCTET				0			//
#define DEFAULT_WD_IC2_ADDRESS			0			//
#define DEFAULT_USE_SSDP			false		// (true|false)
#define DEFAULT_CON_FAIL_THRES			0			//
#define DEFAULT_I2C_CLOCK_LIMIT			0			//
*/


// Enable FEATURE_ADC_VCC to measure supply voltage using the analog pin
// Please note that the TOUT pin has to be disconnected in this mode
// Use the "System Info" device to read the VCC value
#define FEATURE_ADC_VCC                  false


//enable Arduino OTA updating.
//Note: This adds around 10kb to the firmware size, and 1kb extra ram.
// #define FEATURE_ARDUINO_OTA

//enable mDNS mode (adds about 6kb ram and some bytes IRAM)
// #define FEATURE_MDNS


//enable reporting status to ESPEasy developers.
//this informs us of crashes and stability issues.
// not finished yet!
// #define FEATURE_REPORTING

//Select which plugin sets you want to build.
//These are normally automaticly set via the Platformio build environment.
//If you use ArduinoIDE you might need to uncomment some of them, depending on your needs
//If you dont select any, a version with a minimal number of plugins will be biult for 512k versions.
//(512k is NOT finsihed or tested yet as of v2.0.0-dev6)

//build all the normal stable plugins (on by default)
#define PLUGIN_BUILD_NORMAL

//build all plugins that are in test stadium
//#define PLUGIN_BUILD_TESTING

//build all plugins that still are being developed and are broken or incomplete
//#define PLUGIN_BUILD_DEV

// ********************************************************************************
//   DO NOT CHANGE ANYTHING BELOW THIS LINE
// ********************************************************************************
#include "core_version.h"
#ifndef ARDUINO_ESP8266_RELEASE_2_3_0
#error ESPEasy v2.0 only support Arduino core 2.3.0. (Use the ESPEasy development branch to fix this)
#endif

#define ESP_PROJECT_PID           2016110801L
#define VERSION                             2 // config file version (not ESPEasy version). increase if you make incompatible changes to config system.
#define BUILD                           20000 // git version 2.0.0
#define BUILD_NOTES                 " - Mega"

#ifndef BUILD_GIT
#define BUILD_GIT "(custom)"
#endif

#define MAX_FLASHWRITES_PER_DAY           100 // per 24 hour window

#define NODE_TYPE_ID_ESP_EASY_STD           1
#define NODE_TYPE_ID_ESP_EASYM_STD         17
#define NODE_TYPE_ID_ESP_EASY32_STD        33
#define NODE_TYPE_ID_ARDUINO_EASY_STD      65
#define NODE_TYPE_ID_NANO_EASY_STD         81
#define NODE_TYPE_ID                        NODE_TYPE_ID_ESP_EASYM_STD

#define PLUGIN_INIT_ALL                     1
#define PLUGIN_INIT                         2
#define PLUGIN_READ                         3
#define PLUGIN_ONCE_A_SECOND                4
#define PLUGIN_TEN_PER_SECOND               5
#define PLUGIN_DEVICE_ADD                   6
#define PLUGIN_EVENTLIST_ADD                7
#define PLUGIN_WEBFORM_SAVE                 8
#define PLUGIN_WEBFORM_LOAD                 9
#define PLUGIN_WEBFORM_SHOW_VALUES         10
#define PLUGIN_GET_DEVICENAME              11
#define PLUGIN_GET_DEVICEVALUENAMES        12
#define PLUGIN_WRITE                       13
#define PLUGIN_EVENT_OUT                   14
#define PLUGIN_WEBFORM_SHOW_CONFIG         15
#define PLUGIN_SERIAL_IN                   16
#define PLUGIN_UDP_IN                      17
#define PLUGIN_CLOCK_IN                    18
#define PLUGIN_TIMER_IN                    19
#define PLUGIN_FIFTY_PER_SECOND            20
#define PLUGIN_SET_CONFIG                  21
#define PLUGIN_GET_DEVICEGPIONAMES         22
#define PLUGIN_EXIT                        23
#define PLUGIN_GET_CONFIG                  24

#define CPLUGIN_PROTOCOL_ADD                1
#define CPLUGIN_PROTOCOL_TEMPLATE           2
#define CPLUGIN_PROTOCOL_SEND               3
#define CPLUGIN_PROTOCOL_RECV               4
#define CPLUGIN_GET_DEVICENAME              5
#define CPLUGIN_WEBFORM_SAVE                6
#define CPLUGIN_WEBFORM_LOAD                7
#define CPLUGIN_GET_PROTOCOL_DISPLAY_NAME   8

#define CONTROLLER_HOSTNAME                 1
#define CONTROLLER_IP                       2
#define CONTROLLER_PORT                     3
#define CONTROLLER_USER                     4
#define CONTROLLER_PASS                     5
#define CONTROLLER_SUBSCRIBE                6
#define CONTROLLER_PUBLISH                  7

#define NPLUGIN_PROTOCOL_ADD                1
#define NPLUGIN_GET_DEVICENAME              2
#define NPLUGIN_WEBFORM_SAVE                3
#define NPLUGIN_WEBFORM_LOAD                4
#define NPLUGIN_WRITE                       5
#define NPLUGIN_NOTIFY                      6
#define NPLUGIN_NOT_FOUND                 255


#define LOG_LEVEL_ERROR                     1
#define LOG_LEVEL_INFO                      2
#define LOG_LEVEL_DEBUG                     3
#define LOG_LEVEL_DEBUG_MORE                4
#define LOG_LEVEL_DEBUG_DEV                 9 // use for testing/debugging only, not for regular use

#define CMD_REBOOT                         89
#define CMD_WIFI_DISCONNECT               135

#if defined(PLUGIN_BUILD_TESTING) || defined(PLUGIN_BUILD_DEV)
  #define DEVICES_MAX                      72
#else
  #define DEVICES_MAX                      64
#endif
#define TASKS_MAX                          12 // max 12!
#define CONTROLLER_MAX                      3 // max 4!
#define NOTIFICATION_MAX                    3 // max 4!
#define VARS_PER_TASK                       4
#define PLUGIN_MAX                DEVICES_MAX
#define PLUGIN_CONFIGVAR_MAX                8
#define PLUGIN_CONFIGFLOATVAR_MAX           4
#define PLUGIN_CONFIGLONGVAR_MAX            4
#define PLUGIN_EXTRACONFIGVAR_MAX          16
#define CPLUGIN_MAX                        16
#define NPLUGIN_MAX                         4
#define UNIT_MAX                           32 // Only relevant for UDP unicast message 'sweeps' and the nodelist.
#define RULES_TIMER_MAX                     8
#define SYSTEM_TIMER_MAX                    8
#define SYSTEM_CMD_TIMER_MAX                2
#define PINSTATE_TABLE_MAX                 32
#define RULES_MAX_SIZE                   2048
#define RULES_MAX_NESTING_LEVEL             3
#define RULESETS_MAX                        4

#define PIN_MODE_UNDEFINED                  0
#define PIN_MODE_INPUT                      1
#define PIN_MODE_OUTPUT                     2
#define PIN_MODE_PWM                        3
#define PIN_MODE_SERVO                      4

#define SEARCH_PIN_STATE                 true
#define NO_SEARCH_PIN_STATE             false

#define DEVICE_TYPE_SINGLE                  1  // connected through 1 datapin
#define DEVICE_TYPE_DUAL                    2  // connected through 2 datapins
#define DEVICE_TYPE_TRIPLE                  3  // connected through 3 datapins
#define DEVICE_TYPE_ANALOG                 10  // AIN/tout pin
#define DEVICE_TYPE_I2C                    20  // connected through I2C
#define DEVICE_TYPE_DUMMY                  99  // Dummy device, has no physical connection

#define SENSOR_TYPE_SINGLE                  1
#define SENSOR_TYPE_TEMP_HUM                2
#define SENSOR_TYPE_TEMP_BARO               3
#define SENSOR_TYPE_TEMP_HUM_BARO           4
#define SENSOR_TYPE_DUAL                    5
#define SENSOR_TYPE_TRIPLE                  6
#define SENSOR_TYPE_QUAD                    7
#define SENSOR_TYPE_SWITCH                 10
#define SENSOR_TYPE_DIMMER                 11
#define SENSOR_TYPE_LONG                   20
#define SENSOR_TYPE_WIND                   21

#define VALUE_SOURCE_SYSTEM                 1
#define VALUE_SOURCE_SERIAL                 2
#define VALUE_SOURCE_HTTP                   3
#define VALUE_SOURCE_MQTT                   4
#define VALUE_SOURCE_UDP                    5

#define BOOT_CAUSE_MANUAL_REBOOT            0
#define BOOT_CAUSE_COLD_BOOT                1
#define BOOT_CAUSE_DEEP_SLEEP               2
#define BOOT_CAUSE_EXT_WD                  10

#define DAT_TASKS_SIZE                   2048
#define DAT_TASKS_CUSTOM_OFFSET          1024
#define DAT_CUSTOM_CONTROLLER_SIZE       1024
#define DAT_CONTROLLER_SIZE              1024
#define DAT_NOTIFICATION_SIZE            1024

#define DAT_OFFSET_TASKS                 4096  // each task = 2k, (1024 basic + 1024 bytes custom), 12 max
#define DAT_OFFSET_CONTROLLER           28672  // each controller = 1k, 4 max
#define DAT_OFFSET_CUSTOM_CONTROLLER    32768  // each custom controller config = 1k, 4 max.


/*
	To modify the stock configuration without changing this repo file :
    - define USE_CUSTOM_H as a build flags. ie : export PLATFORMIO_BUILD_FLAGS="'-DUSE_CUSTOM_H'"
	- add a "Custom.h" file in this folder.

*/
#ifdef USE_CUSTOM_H
#include "Custom.h"
#endif

#include "ESPEasyTimeTypes.h"
#include "lwip/tcp_impl.h"
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#ifdef FEATURE_MDNS
#include <ESP8266mDNS.h>
#endif

#include <Wire.h>
#include <SPI.h>
#include <PubSubClient.h>
// #include <ArduinoJson.h>
// #include <LiquidCrystal_I2C.h>
#include <Servo.h>
#define FS_NO_GLOBALS
#include <FS.h>
#include <SD.h>
#include <ESP8266HTTPUpdateServer.h>
ESP8266HTTPUpdateServer httpUpdater(true);
#include <base64.h>
#if FEATURE_ADC_VCC
ADC_MODE(ADC_VCC);
#endif
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


#ifdef FEATURE_ARDUINO_OTA
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
bool ArduinoOTAtriggered=false;
#endif


// Setup DNS, only used if the ESP has no valid WiFi config
const byte DNS_PORT = 53;
IPAddress apIP(DEFAULT_AP_IP);
DNSServer dnsServer;
#ifdef FEATURE_MDNS
MDNSResponder mdns;
#endif

// MQTT client
WiFiClient mqtt;
PubSubClient MQTTclient(mqtt);
bool MQTTclient_should_reconnect = true;

// WebServer
ESP8266WebServer WebServer(80);

// udp protocol stuff (syslog, global sync, node info list, ntp time)
WiFiUDP portUDP;

bool WiFiConnected(uint32_t timeout_ms);

extern "C" {
#include "spi_flash.h"
}
extern "C" uint32_t _SPIFFS_start;
extern "C" uint32_t _SPIFFS_end;
extern "C" uint32_t _SPIFFS_page;
extern "C" uint32_t _SPIFFS_block;

struct SecurityStruct
{
  SecurityStruct() {
    memset(WifiSSID, 0, sizeof(WifiSSID));
    memset(WifiKey, 0, sizeof(WifiKey));
    memset(WifiSSID2, 0, sizeof(WifiSSID2));
    memset(WifiKey2, 0, sizeof(WifiKey2));
    memset(WifiAPKey, 0, sizeof(WifiAPKey));
    for (byte i = 0; i < CONTROLLER_MAX; ++i) {
      memset(ControllerUser[i], 0, sizeof(ControllerUser[i]));
      memset(ControllerPassword[i], 0, sizeof(ControllerPassword[i]));
    }
    memset(Password, 0, sizeof(Password));
  }
  char          WifiSSID[32];
  char          WifiKey[64];
  char          WifiSSID2[32];
  char          WifiKey2[64];
  char          WifiAPKey[64];
  char          ControllerUser[CONTROLLER_MAX][26];
  char          ControllerPassword[CONTROLLER_MAX][64];
  char          Password[26];
  //its safe to extend this struct, up to 4096 bytes, default values in config are 0
} SecuritySettings;

struct SettingsStruct
{
  SettingsStruct() :
    PID(0), Version(0), Build(0), IP_Octet(0), Unit(0), Delay(0),
    Pin_i2c_sda(-1), Pin_i2c_scl(-1), Pin_status_led(-1), Pin_sd_cs(-1),
    UDPPort(0), SyslogLevel(0), SerialLogLevel(0), WebLogLevel(0), SDLogLevel(0),
    BaudRate(0), MessageDelay(0), deepSleep(0),
    CustomCSS(false), DST(false), WDI2CAddress(0),
    UseRules(false), UseSerial(false), UseSSDP(false), UseNTP(false),
    WireClockStretchLimit(0), GlobalSync(false), ConnectionFailuresThreshold(0),
    TimeZone(0), MQTTRetainFlag(false), InitSPI(false),
    Pin_status_led_Inversed(false), deepSleepOnFail(false), UseValueLogger(false),
    DST_Start(0), DST_End(0)
    {
      for (byte i = 0; i < CONTROLLER_MAX; ++i) {
        Protocol[i] = 0;
        ControllerEnabled[i] = false;
        for (byte task = 0; task < TASKS_MAX; ++task) {
          TaskDeviceID[i][task] = 0;
          TaskDeviceSendData[i][task] = false;
        }
      }
      for (byte task = 0; task < TASKS_MAX; ++task) {
        TaskDeviceNumber[task] = 0;
        OLD_TaskDeviceID[task] = 0;
        TaskDevicePin1PullUp[task] = false;
        for (byte cv = 0; cv < PLUGIN_CONFIGVAR_MAX; ++cv) {
          TaskDevicePluginConfig[task][cv] = 0;
        }
        TaskDevicePin1Inversed[task] = false;
        for (byte cv = 0; cv < PLUGIN_CONFIGFLOATVAR_MAX; ++cv) {
          TaskDevicePluginConfigFloat[task][cv] = 0.0;
        }
        for (byte cv = 0; cv < PLUGIN_CONFIGLONGVAR_MAX; ++cv) {
          TaskDevicePluginConfigLong[task][cv] = 0;
        }
        OLD_TaskDeviceSendData[task] = false;
        TaskDeviceGlobalSync[task] = false;
        TaskDeviceDataFeed[task] = 0;
        TaskDeviceTimer[task] = 0;
        TaskDeviceEnabled[task] = false;
      }
    }

  unsigned long PID;
  int           Version;
  int16_t       Build;
  byte          IP[4];
  byte          Gateway[4];
  byte          Subnet[4];
  byte          DNS[4];
  byte          IP_Octet;
  byte          Unit;
  char          Name[26];
  char          NTPHost[64];
  unsigned long Delay;
  int8_t        Pin_i2c_sda;
  int8_t        Pin_i2c_scl;
  int8_t        Pin_status_led;
  int8_t        Pin_sd_cs;
  int8_t        PinBootStates[17];
  byte          Syslog_IP[4];
  unsigned int  UDPPort;
  byte          SyslogLevel;
  byte          SerialLogLevel;
  byte          WebLogLevel;
  byte          SDLogLevel;
  unsigned long BaudRate;
  unsigned long MessageDelay;
  byte          deepSleep;
  boolean       CustomCSS;
  boolean       DST;
  byte          WDI2CAddress;
  boolean       UseRules;
  boolean       UseSerial;
  boolean       UseSSDP;
  boolean       UseNTP;
  unsigned long WireClockStretchLimit;
  boolean       GlobalSync;
  unsigned long ConnectionFailuresThreshold;
  int16_t       TimeZone;
  boolean       MQTTRetainFlag;
  boolean       InitSPI;
  byte          Protocol[CONTROLLER_MAX];
  byte          Notification[NOTIFICATION_MAX]; //notifications, point to a NPLUGIN id
  byte          TaskDeviceNumber[TASKS_MAX];
  unsigned int  OLD_TaskDeviceID[TASKS_MAX];
  union {
    struct {
      int8_t        TaskDevicePin1[TASKS_MAX];
      int8_t        TaskDevicePin2[TASKS_MAX];
      int8_t        TaskDevicePin3[TASKS_MAX];
      byte          TaskDevicePort[TASKS_MAX];
    };
    int8_t        TaskDevicePin[4][TASKS_MAX];
  };
  boolean       TaskDevicePin1PullUp[TASKS_MAX];
  int16_t       TaskDevicePluginConfig[TASKS_MAX][PLUGIN_CONFIGVAR_MAX];
  boolean       TaskDevicePin1Inversed[TASKS_MAX];
  float         TaskDevicePluginConfigFloat[TASKS_MAX][PLUGIN_CONFIGFLOATVAR_MAX];
  long          TaskDevicePluginConfigLong[TASKS_MAX][PLUGIN_CONFIGLONGVAR_MAX];
  boolean       OLD_TaskDeviceSendData[TASKS_MAX];
  boolean       TaskDeviceGlobalSync[TASKS_MAX];
  byte          TaskDeviceDataFeed[TASKS_MAX];
  unsigned long TaskDeviceTimer[TASKS_MAX];
  boolean       TaskDeviceEnabled[TASKS_MAX];
  boolean       ControllerEnabled[CONTROLLER_MAX];
  boolean       NotificationEnabled[NOTIFICATION_MAX];
  unsigned int  TaskDeviceID[CONTROLLER_MAX][TASKS_MAX];
  boolean       TaskDeviceSendData[CONTROLLER_MAX][TASKS_MAX];
  boolean       Pin_status_led_Inversed;
  boolean       deepSleepOnFail;
  boolean       UseValueLogger;
  uint16_t      DST_Start;
  uint16_t      DST_End;
  //its safe to extend this struct, up to several bytes, default values in config are 0
  //look in misc.ino how config.dat is used because also other stuff is stored in it at different offsets.
  //TODO: document config.dat somewhere here
} Settings;

struct ControllerSettingsStruct
{
  ControllerSettingsStruct() : UseDNS(false), Port(0) {
    memset(HostName, 0, sizeof(HostName));
    memset(Publish, 0, sizeof(Publish));
    memset(Subscribe, 0, sizeof(Subscribe));
  }
  boolean       UseDNS;
  byte          IP[4];
  unsigned int  Port;
  char          HostName[65];
  char          Publish[129];
  char          Subscribe[129];

  IPAddress getIP() const {
    IPAddress host(IP[0], IP[1], IP[2], IP[3]);
    return host;
  }

  String getHost() const {
    if (UseDNS) {
      return HostName;
    }
    return getIP().toString();
  }

  boolean connectToHost(WiFiClient &client) {
    if (!WiFiConnected(100)) {
      return false; // Not connected, so no use in wasting time to connect to a host.
    }
    if (UseDNS) {
      return client.connect(HostName, Port);
    }
    return client.connect(getIP(), Port);
  }

  int beginPacket(WiFiUDP &client) {
    if (!WiFiConnected(100)) {
      return 0; // Not connected, so no use in wasting time to connect to a host.
    }
    if (UseDNS) {
      return client.beginPacket(HostName, Port);
    }
    return client.beginPacket(getIP(), Port);
  }

  String getHostPortString() const {
    String result = getHost();
    result += ":";
    result += Port;
    return result;
  }
};

struct NotificationSettingsStruct
{
  NotificationSettingsStruct() : Port(0), Pin1(0), Pin2(0) {
    memset(Server,   0, sizeof(Server));
    memset(Domain,   0, sizeof(Domain));
    memset(Sender,   0, sizeof(Sender));
    memset(Receiver, 0, sizeof(Receiver));
    memset(Subject,  0, sizeof(Subject));
    memset(Body,     0, sizeof(Body));
    memset(User,     0, sizeof(User));
    memset(Pass,     0, sizeof(Pass));
  }

  char          Server[65];
  unsigned int  Port;
  char          Domain[65];
  char          Sender[65];
  char          Receiver[65];
  char          Subject[129];
  char          Body[513];
  byte          Pin1;
  byte          Pin2;
  char          User[49];
  char          Pass[33];
  //its safe to extend this struct, up to 4096 bytes, default values in config are 0
};

struct ExtraTaskSettingsStruct
{
  ExtraTaskSettingsStruct() : TaskIndex(0) {
    TaskDeviceName[0] = 0;
    for (byte i = 0; i < VARS_PER_TASK; ++i) {
      for (byte j = 0; j < 41; ++j) {
        TaskDeviceFormula[i][j] = 0;
        TaskDeviceValueNames[i][j] = 0;
        TaskDeviceValueDecimals[i] = 0;
      }
    }
    for (byte i = 0; i < PLUGIN_EXTRACONFIGVAR_MAX; ++i) {
      TaskDevicePluginConfigLong[i] = 0;
      TaskDevicePluginConfig[i] = 0;
    }
  }

  byte    TaskIndex;
  char    TaskDeviceName[41];
  char    TaskDeviceFormula[VARS_PER_TASK][41];
  char    TaskDeviceValueNames[VARS_PER_TASK][41];
  long    TaskDevicePluginConfigLong[PLUGIN_EXTRACONFIGVAR_MAX];
  byte    TaskDeviceValueDecimals[VARS_PER_TASK];
  int16_t TaskDevicePluginConfig[PLUGIN_EXTRACONFIGVAR_MAX];
} ExtraTaskSettings;

struct EventStruct
{
  EventStruct() :
    Source(0), TaskIndex(0), ControllerIndex(0), ProtocolIndex(0), NotificationIndex(0),
    BaseVarIndex(0), idx(0), sensorType(0), Par1(0), Par2(0), Par3(0), Par4(0), Par5(0),
    OriginTaskIndex(0), Data(NULL) {}
  byte Source;
  byte TaskIndex; // index position in TaskSettings array, 0-11
  byte ControllerIndex; // index position in Settings.Controller, 0-3
  byte ProtocolIndex; // index position in protocol array, depending on which controller plugins are loaded.
  byte NotificationIndex; // index position in Settings.Notification, 0-3
  //Edwin: Not needed, and wasnt used. We can determine the protocol index with getNotificationProtocolIndex(NotificationIndex)
  // byte NotificationProtocolIndex; // index position in notification array, depending on which controller plugins are loaded.
  byte BaseVarIndex;
  int idx;
  byte sensorType;
  int Par1;
  int Par2;
  int Par3;
  int Par4;
  int Par5;
  byte OriginTaskIndex;
  String String1;
  String String2;
  String String3;
  byte *Data;
};

#define LOG_STRUCT_MESSAGE_SIZE 128
#define LOG_STRUCT_MESSAGE_LINES 20

struct LogStruct {
    LogStruct() : write_idx(0), read_idx(0) {
      for (int i = 0; i < LOG_STRUCT_MESSAGE_LINES; ++i) {
        memset(Message[i], 0, LOG_STRUCT_MESSAGE_SIZE);
        timeStamp[i] = 0;
      }
    }

    void add(const char *line) {
      write_idx = (write_idx + 1) % LOG_STRUCT_MESSAGE_LINES;
      if (write_idx == read_idx) {
        // Buffer full, move read_idx to overwrite oldest entry.
        read_idx = (read_idx + 1) % LOG_STRUCT_MESSAGE_LINES;
      }
      timeStamp[write_idx] = millis();
      strncpy(Message[write_idx], line, LOG_STRUCT_MESSAGE_SIZE-1);
    }

    // Read the next item and append it to the given string.
    // Returns whether new lines are available.
    bool get(String& output, const String& lineEnd) {
      if (!isEmpty()) {
        read_idx = (read_idx + 1) % LOG_STRUCT_MESSAGE_LINES;
        output += formatLine(read_idx, lineEnd);
      }
      return !isEmpty();
    }

    bool getAll(String& output, const String& lineEnd) {
      int tmpread((write_idx + 1) % LOG_STRUCT_MESSAGE_LINES);
      bool someAdded = false;
      while (tmpread != write_idx) {
        if (timeStamp[tmpread] != 0) {
          output += formatLine(tmpread, lineEnd);
          someAdded = true;
        }
        tmpread = (tmpread + 1)% LOG_STRUCT_MESSAGE_LINES;
      }
      return someAdded;
    }

    bool isEmpty() {
      return (write_idx == read_idx);
    }

  private:
    String formatLine(int index, const String& lineEnd) {
      String output;
      output += timeStamp[index];
      output += " : ";
      output += Message[index];
      output += lineEnd;
      return output;
    }


    int write_idx;
    int read_idx;
    unsigned long timeStamp[LOG_STRUCT_MESSAGE_LINES];
    char Message[LOG_STRUCT_MESSAGE_LINES][LOG_STRUCT_MESSAGE_SIZE];

} Logging;

struct DeviceStruct
{
  DeviceStruct() :
    Number(0), Type(0), VType(0), Ports(0),
    PullUpOption(false), InverseLogicOption(false), FormulaOption(false),
    ValueCount(0), Custom(false), SendDataOption(false), GlobalSyncOption(false),
    TimerOption(false), TimerOptional(false), DecimalsOnly(false) {}
  byte Number;
  byte Type;
  byte VType;
  byte Ports;
  boolean PullUpOption;
  boolean InverseLogicOption;
  boolean FormulaOption;
  byte ValueCount;
  boolean Custom;
  boolean SendDataOption;
  boolean GlobalSyncOption;
  boolean TimerOption;
  boolean TimerOptional;
  boolean DecimalsOnly;
} Device[DEVICES_MAX + 1]; // 1 more because first device is empty device

struct ProtocolStruct
{
  ProtocolStruct() :
    Number(0), usesMQTT(false), usesAccount(false), usesPassword(false),
    defaultPort(0), usesTemplate(false), usesID(false) {}
  byte Number;
  boolean usesMQTT;
  boolean usesAccount;
  boolean usesPassword;
  int defaultPort;
  boolean usesTemplate;
  boolean usesID;
} Protocol[CPLUGIN_MAX];

struct NotificationStruct
{
  NotificationStruct() :
    Number(0), usesMessaging(false), usesGPIO(0) {}
  byte Number;
  boolean usesMessaging;
  byte usesGPIO;
} Notification[NPLUGIN_MAX];

struct NodeStruct
{
  NodeStruct() :
    age(0), build(0), nodeName(NULL), nodeType(0)
    {
      for (byte i = 0; i < 4; ++i) ip[i] = 0;
    }
  byte ip[4];
  byte age;
  uint16_t build;
  char* nodeName;
  byte nodeType;
} Nodes[UNIT_MAX];

struct systemTimerStruct
{
  systemTimerStruct() :
    timer(0), plugin(0), Par1(0), Par2(0), Par3(0) {}

  unsigned long timer;
  byte plugin;
  byte Par1;
  byte Par2;
  byte Par3;
} systemTimers[SYSTEM_TIMER_MAX];

struct systemCMDTimerStruct
{
  systemCMDTimerStruct() : timer(0) {}
  unsigned long timer;
  String action;
} systemCMDTimers[SYSTEM_CMD_TIMER_MAX];

struct pinStatesStruct
{
  byte plugin;
  byte index;
  byte mode;
  uint16_t value;
} pinStates[PINSTATE_TABLE_MAX];


// this offsets are in blocks, bytes = blocks * 4
#define RTC_BASE_STRUCT 64
#define RTC_BASE_USERVAR 74

//max 40 bytes: ( 74 - 64 ) * 4
struct RTCStruct
{
  byte ID1;
  byte ID2;
  boolean unused1;
  byte factoryResetCounter;
  byte deepSleepState;
  byte unused2;
  byte flashDayCounter;
  unsigned long flashCounter;
  unsigned long bootCounter;
} RTC;


int deviceCount = -1;
int protocolCount = -1;
int notificationCount = -1;

boolean printToWeb = false;
String printWebString = "";
boolean printToWebJSON = false;

float UserVar[VARS_PER_TASK * TASKS_MAX];
unsigned long RulesTimer[RULES_TIMER_MAX];

unsigned long timerSensor[TASKS_MAX];
unsigned long timer100ms;
unsigned long timer20ms;
unsigned long timer1s;
unsigned long timerwd;
unsigned long timermqtt;
unsigned long lastSend;
unsigned int NC_Count = 0;
unsigned int C_Count = 0;
byte cmd_within_mainloop = 0;
unsigned long connectionFailures;
unsigned long wdcounter = 0;
unsigned long timerAPoff = 0;

#if FEATURE_ADC_VCC
float vcc = -1.0;
#endif

boolean WebLoggedIn = false;
int WebLoggedInTimer = 300;

boolean (*Plugin_ptr[PLUGIN_MAX])(byte, struct EventStruct*, String&);
byte Plugin_id[PLUGIN_MAX];

boolean (*CPlugin_ptr[CPLUGIN_MAX])(byte, struct EventStruct*, String&);
byte CPlugin_id[CPLUGIN_MAX];

boolean (*NPlugin_ptr[NPLUGIN_MAX])(byte, struct EventStruct*, String&);
byte NPlugin_id[NPLUGIN_MAX];

String dummyString = "";

byte lastBootCause = BOOT_CAUSE_MANUAL_REBOOT;

boolean wifiSetup = false;
boolean wifiSetupConnect = false;

unsigned long start = 0;
unsigned long elapsed = 0;
unsigned long loopCounter = 0;
unsigned long loopCounterLast = 0;
unsigned long loopCounterMax = 1;

unsigned long dailyResetCounter = 0;

String eventBuffer = "";

uint16_t lowestRAM = 0;
String lowestRAMfunction = "";

bool shouldReboot=false;

// Blynk_get prototype
boolean Blynk_get(const String& command, byte controllerIndex,float *data = NULL );

int firstEnabledBlynkController() {
  for (byte i = 0; i < CONTROLLER_MAX; ++i) {
    byte ProtocolIndex = getProtocolIndex(Settings.Protocol[i]);
    if (Protocol[ProtocolIndex].Number == 12 && Settings.ControllerEnabled[i]) {
      return i;
    }
  }
  return -1;
}

/*********************************************************************************************\
 * SETUP
\*********************************************************************************************/
void setup()
{

  lowestRAM = FreeMem();

  Serial.begin(115200);
  // Serial.print("\n\n\nBOOOTTT\n\n\n");

  initLog();


  if (SpiffsSectors() < 32)
  {
    Serial.println(F("\nNo (or too small) SPIFFS area..\nSystem Halted\nPlease reflash with 128k SPIFFS minimum!"));
    while (true)
      delay(1);
  }

  emergencyReset();

  String log = F("\n\n\rINIT : Booting version: ");
  log += BUILD_GIT;
  addLog(LOG_LEVEL_INFO, log);


  //warm boot
  if (readFromRTC())
  {
    RTC.bootCounter++;
    readUserVarFromRTC();

    if (RTC.deepSleepState == 1)
    {
      log = F("INIT : Rebooted from deepsleep #");
      lastBootCause=BOOT_CAUSE_DEEP_SLEEP;
    }
    else
      log = F("INIT : Warm boot #");

    log += RTC.bootCounter;

  }
  //cold boot (RTC memory empty)
  else
  {
    initRTC();

    // cold boot situation
    if (lastBootCause == BOOT_CAUSE_MANUAL_REBOOT) // only set this if not set earlier during boot stage.
      lastBootCause = BOOT_CAUSE_COLD_BOOT;
    log = F("INIT : Cold Boot");
  }

  RTC.deepSleepState=0;
  saveToRTC();

  addLog(LOG_LEVEL_INFO, log);




  fileSystemCheck();
  LoadSettings();

  if (strcasecmp(SecuritySettings.WifiSSID, "ssid") == 0)
    wifiSetup = true;

  ExtraTaskSettings.TaskIndex = 255; // make sure this is an unused nr to prevent cache load on boot

  // if different version, eeprom settings structure has changed. Full Reset needed
  // on a fresh ESP module eeprom values are set to 255. Version results into -1 (signed int)
  if (Settings.Version != VERSION || Settings.PID != ESP_PROJECT_PID)
  {
    // Direct Serial is allowed here, since this is only an emergency task.
    Serial.print(F("\nPID:"));
    Serial.println(Settings.PID);
    Serial.print(F("Version:"));
    Serial.println(Settings.Version);
    Serial.println(F("INIT : Incorrect PID or version!"));
    delay(1000);
    ResetFactory();
  }

  if (Settings.UseSerial)
  {
    //make sure previous serial buffers are flushed before resetting baudrate
    Serial.flush();
    Serial.begin(Settings.BaudRate);
  }

  if (Settings.Build != BUILD)
    BuildFixes();


  log = F("INIT : Free RAM:");
  log += FreeMem();
  addLog(LOG_LEVEL_INFO, log);

  if (Settings.UseSerial && Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE)
    Serial.setDebugOutput(true);

  hardwareInit();

  WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
  WifiAPconfig();

  if (Settings.deepSleep)
  {
    //only one attempt in deepsleep, to conserve battery
    if (!WifiConnect(1))
    {
        if (Settings.deepSleepOnFail)
        {
          addLog(LOG_LEVEL_ERROR, F("SLEEP: Connection failed, going back to sleep."));
          deepSleep(Settings.Delay);
        }
    }
  }
  else
    // 3 connect attempts
    WifiConnect(3);

  #ifdef FEATURE_REPORTING
  ReportStatus();
  #endif

  //After booting, we want all the tasks to run without delaying more than neccesary.
  //Plugins that need an initial startup delay need to overwrite their initial timerSensor value in PLUGIN_INIT
  //They should also check if we returned from deep sleep so that they can skip the delay in that case.
  for (byte x = 0; x < TASKS_MAX; x++)
    if (Settings.TaskDeviceTimer[x] !=0)
      timerSensor[x] = millis() + (x * Settings.MessageDelay);

  timer100ms = 0; // timer for periodic actions 10 x per/sec
  timer1s = 0; // timer for periodic actions once per/sec
  timerwd = 0; // timer for watchdog once per 30 sec
  timermqtt = 0; // Timer for the MQTT keep alive loop.

  PluginInit();
  CPluginInit();
  NPluginInit();

  WebServerInit();

  #ifdef FEATURE_ARDUINO_OTA
  ArduinoOTAInit();
  #endif

  // setup UDP
  if (Settings.UDPPort != 0)
    portUDP.begin(Settings.UDPPort);

/*
  // Setup MQTT Client
  // ToDo TD-er: Controller index is forced to the first enabled MQTT controller.
  int enabledMqttController = firstEnabledMQTTController();
  if (enabledMqttController >= 0) {
    MQTTConnect(enabledMqttController);
  }
*/

  sendSysInfoUDP(3);

  if (Settings.UseNTP)
    initTime();

#if FEATURE_ADC_VCC
  vcc = ESP.getVcc() / 1000.0;
#endif

  // Start DNS, only used if the ESP has no valid WiFi config
  // It will reply with it's own address on all DNS requests
  // (captive portal concept)
  if (wifiSetup)
    dnsServer.start(DNS_PORT, "*", apIP);

  if (Settings.UseRules)
  {
    String event = F("System#Boot");
    rulesProcessing(event);
  }

  writeDefaultCSS();

}

int firstEnabledMQTTController() {
  for (byte i = 0; i < CONTROLLER_MAX; ++i) {
    byte ProtocolIndex = getProtocolIndex(Settings.Protocol[i]);
    if (Protocol[ProtocolIndex].usesMQTT && Settings.ControllerEnabled[i]) {
      return i;
    }
  }
  return -1;
}

bool getControllerProtocolDisplayName(byte ProtocolIndex, byte parameterIdx, String& protoDisplayName) {
  EventStruct tmpEvent;
  tmpEvent.idx=parameterIdx;
  return CPlugin_ptr[ProtocolIndex](CPLUGIN_GET_PROTOCOL_DISPLAY_NAME, &tmpEvent, protoDisplayName);
}

/*********************************************************************************************\
 * MAIN LOOP
\*********************************************************************************************/
void loop()
{
  loopCounter++;

  if (wifiSetupConnect)
  {
    // try to connect for setup wizard
    WifiConnect(1);
    wifiSetupConnect = false;
  }

  // Deep sleep mode, just run all tasks one time and go back to sleep as fast as possible
  if (isDeepSleepEnabled())
  {
      run50TimesPerSecond();
      run10TimesPerSecond();
      runEach30Seconds();
      runOncePerSecond();
      deepSleep(Settings.Delay);
      //deepsleep will never return, its a special kind of reboot
  }
  //normal mode, run each task when its time
  else
  {

    if (timeOutReached(timer20ms))
      run50TimesPerSecond();

    if (timeOutReached(timer100ms))
      run10TimesPerSecond();

    if (timeOutReached(timerwd))
      runEach30Seconds();

    if (timeOutReached(timer1s))
      runOncePerSecond();

    if (timeOutReached(timermqtt)) {
      // MQTT_KEEPALIVE = 15 seconds.
      timermqtt = millis() + 250;
      //dont do this in backgroundtasks(), otherwise causes crashes. (https://github.com/letscontrolit/ESPEasy/issues/683)
      int enabledMqttController = firstEnabledMQTTController();
      if (enabledMqttController >= 0) {
        if (!MQTTclient.loop()) {
          if (!MQTTCheck(enabledMqttController)) {
            // Check failed, no need to retry it immediately.
            timermqtt = millis() + 500;
          }
        }
      }
    }
  }

  backgroundtasks();

}


/*********************************************************************************************\
 * Tasks that run 50 times per second
\*********************************************************************************************/

void run50TimesPerSecond()
{
  timer20ms = millis() + 20;
  PluginCall(PLUGIN_FIFTY_PER_SECOND, 0, dummyString);

  // statusLED(false);
}

/*********************************************************************************************\
 * Tasks that run 10 times per second
\*********************************************************************************************/
void run10TimesPerSecond()
{
  start = micros();
  timer100ms = millis() + 100;
  PluginCall(PLUGIN_TEN_PER_SECOND, 0, dummyString);
  if (Settings.UseRules && eventBuffer.length() > 0)
  {
    rulesProcessing(eventBuffer);
    eventBuffer = "";
  }
  elapsed = micros() - start;
}


/*********************************************************************************************\
 * Tasks each second
\*********************************************************************************************/
void runOncePerSecond()
{
  dailyResetCounter++;
  if (dailyResetCounter > 86400) // 1 day elapsed... //86400
  {
    RTC.flashDayCounter=0;
    saveToRTC();
    dailyResetCounter=0;
    String log = F("SYS  : Reset 24h counters");
    addLog(LOG_LEVEL_INFO, log);
  }

  timer1s = millis() + 1000;

  checkSensors();

  if (Settings.ConnectionFailuresThreshold)
    if (connectionFailures > Settings.ConnectionFailuresThreshold)
      delayedReboot(60);

  if (cmd_within_mainloop != 0)
  {
    switch (cmd_within_mainloop)
    {
      case CMD_WIFI_DISCONNECT:
        {
          WifiDisconnect();
          break;
        }
      case CMD_REBOOT:
        {
          ESP.reset();
          break;
        }
    }
    cmd_within_mainloop = 0;
  }

  // clock events
  if (Settings.UseNTP)
    checkTime();

  unsigned long timer = micros();
  PluginCall(PLUGIN_ONCE_A_SECOND, 0, dummyString);

  checkSystemTimers();

  if (Settings.UseRules)
    rulesTimers();

  timer = micros() - timer;

  if (SecuritySettings.Password[0] != 0)
  {
    if (WebLoggedIn)
      WebLoggedInTimer++;
    if (WebLoggedInTimer > 300)
      WebLoggedIn = false;
  }

  // I2C Watchdog feed
  if (Settings.WDI2CAddress != 0)
  {
    Wire.beginTransmission(Settings.WDI2CAddress);
    Wire.write(0xA5);
    Wire.endTransmission();
  }

  if (Settings.SerialLogLevel == 5)
  {
    Serial.print(F("10 ps:"));
    Serial.print(elapsed);
    Serial.print(F(" uS  1 ps:"));
    Serial.println(timer);
  }

  if (timerAPoff != 0 && timeOutReached(timerAPoff))
  {
    timerAPoff = 0;
    WifiAPMode(false);
  }
}

/*********************************************************************************************\
 * Tasks each 30 seconds
\*********************************************************************************************/
void runEach30Seconds()
{
  wdcounter++;
  timerwd = millis() + 30000;
  char str[60];
  str[0] = 0;
  sprintf_P(str, PSTR("Uptime %u ConnectFailures %u FreeMem %u"), wdcounter / 2, connectionFailures, FreeMem());
  String log = F("WD   : ");
  log += str;
  addLog(LOG_LEVEL_INFO, log);
  sendSysInfoUDP(1);
  refreshNodeList();
  int enabledMqttController = firstEnabledMQTTController();
  if (enabledMqttController >= 0) {
    MQTTCheck(enabledMqttController);
  }
  if (Settings.UseSSDP)
    SSDP_update();
#if FEATURE_ADC_VCC
  vcc = ESP.getVcc() / 1000.0;
#endif
  loopCounterLast = loopCounter;
  loopCounter = 0;
  if (loopCounterLast > loopCounterMax)
    loopCounterMax = loopCounterLast;

  WifiCheck();

  #ifdef FEATURE_REPORTING
  ReportStatus();
  #endif

}


/*********************************************************************************************\
 * Check sensor timers
\*********************************************************************************************/
void checkSensors()
{
  bool isDeepSleep = isDeepSleepEnabled();
  //check all the devices and only run the sendtask if its time, or we if we used deep sleep mode
  for (byte x = 0; x < TASKS_MAX; x++)
  {
    if (
        (Settings.TaskDeviceTimer[x] != 0) &&
        (isDeepSleep || timeOutReached(timerSensor[x]))
    )
    {
      timerSensor[x] = millis() + Settings.TaskDeviceTimer[x] * 1000;
      if (timerSensor[x] == 0) // small fix if result is 0, else timer will be stopped...
        timerSensor[x] = 1;
      SensorSendTask(x);
    }
  }
  saveUserVarToRTC();
}


/*********************************************************************************************\
 * send all sensordata
\*********************************************************************************************/
// void SensorSendAll()
// {
//   for (byte x = 0; x < TASKS_MAX; x++)
//   {
//     SensorSendTask(x);
//   }
// }


/*********************************************************************************************\
 * send specific sensor task data
\*********************************************************************************************/
void SensorSendTask(byte TaskIndex)
{
  if (Settings.TaskDeviceEnabled[TaskIndex])
  {
    byte varIndex = TaskIndex * VARS_PER_TASK;

    boolean success = false;
    byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[TaskIndex]);
    LoadTaskSettings(TaskIndex);

    struct EventStruct TempEvent;
    TempEvent.TaskIndex = TaskIndex;
    TempEvent.BaseVarIndex = varIndex;
    // TempEvent.idx = Settings.TaskDeviceID[TaskIndex]; todo check
    TempEvent.sensorType = Device[DeviceIndex].VType;

    float preValue[VARS_PER_TASK]; // store values before change, in case we need it in the formula
    for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
      preValue[varNr] = UserVar[varIndex + varNr];

    if(Settings.TaskDeviceDataFeed[TaskIndex] == 0)  // only read local connected sensorsfeeds
      success = PluginCall(PLUGIN_READ, &TempEvent, dummyString);
    else
      success = true;

    if (success)
    {
      for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
      {
        if (ExtraTaskSettings.TaskDeviceFormula[varNr][0] != 0)
        {
          String spreValue = String(preValue[varNr]);
          String formula = ExtraTaskSettings.TaskDeviceFormula[varNr];
          float value = UserVar[varIndex + varNr];
          float result = 0;
          String svalue = String(value);
          formula.replace(F("%pvalue%"), spreValue);
          formula.replace(F("%value%"), svalue);
          byte error = Calculate(formula.c_str(), &result);
          if (error == 0)
            UserVar[varIndex + varNr] = result;
        }
      }
    }
    // Call sendData regardless the success parameter.
    sendData(&TempEvent);
  }
}


/*********************************************************************************************\
 * set global system timer
\*********************************************************************************************/
void setSystemTimer(unsigned long timer, byte plugin, byte Par1, byte Par2, byte Par3)
{
  // plugin number and par1 form a unique key that can be used to restart a timer
  // first check if a timer is not already running for this request
  boolean reUse = false;
  for (byte x = 0; x < SYSTEM_TIMER_MAX; x++)
    if (systemTimers[x].timer != 0)
    {
      if ((systemTimers[x].plugin == plugin) && (systemTimers[x].Par1 == Par1))
      {
        systemTimers[x].timer = millis() + timer;
        reUse = true;
        break;
      }
    }

  if (!reUse)
  {
    // find a new free timer slot...
    for (byte x = 0; x < SYSTEM_TIMER_MAX; x++)
      if (systemTimers[x].timer == 0)
      {
        systemTimers[x].timer = millis() + timer;
        systemTimers[x].plugin = plugin;
        systemTimers[x].Par1 = Par1;
        systemTimers[x].Par2 = Par2;
        systemTimers[x].Par3 = Par3;
        break;
      }
  }
}


//EDWIN: this function seems to be unused?
/*********************************************************************************************\
 * set global system command timer
\*********************************************************************************************/
void setSystemCMDTimer(unsigned long timer, String& action)
{
  for (byte x = 0; x < SYSTEM_CMD_TIMER_MAX; x++)
    if (systemCMDTimers[x].timer == 0)
    {
      systemCMDTimers[x].timer = millis() + timer;
      systemCMDTimers[x].action = action;
      break;
    }
}


/*********************************************************************************************\
 * check global system timers
\*********************************************************************************************/
void checkSystemTimers()
{
  for (byte x = 0; x < SYSTEM_TIMER_MAX; x++)
    if (systemTimers[x].timer != 0)
    {
      if (timeOutReached(systemTimers[x].timer))
      {
        struct EventStruct TempEvent;
        TempEvent.Par1 = systemTimers[x].Par1;
        TempEvent.Par2 = systemTimers[x].Par2;
        TempEvent.Par3 = systemTimers[x].Par3;
        for (byte y = 0; y < PLUGIN_MAX; y++)
          if (Plugin_id[y] == systemTimers[x].plugin)
            Plugin_ptr[y](PLUGIN_TIMER_IN, &TempEvent, dummyString);
        systemTimers[x].timer = 0;
      }
    }

  for (byte x = 0; x < SYSTEM_CMD_TIMER_MAX; x++)
    if (systemCMDTimers[x].timer != 0)
      if (timeOutReached(systemCMDTimers[x].timer))
      {
        struct EventStruct TempEvent;
        parseCommandString(&TempEvent, systemCMDTimers[x].action);
        if (!PluginCall(PLUGIN_WRITE, &TempEvent, systemCMDTimers[x].action))
          ExecuteCommand(VALUE_SOURCE_SYSTEM, systemCMDTimers[x].action.c_str());
        systemCMDTimers[x].timer = 0;
        systemCMDTimers[x].action = "";
      }
}


/*********************************************************************************************\
 * run background tasks
\*********************************************************************************************/
bool runningBackgroundTasks=false;
void backgroundtasks()
{
  //always start with a yield
  yield();

  //prevent recursion!
  if (runningBackgroundTasks)
  {
    return;
  }
  runningBackgroundTasks=true;

  tcpCleanup();

  if (Settings.UseSerial)
    if (Serial.available())
      if (!PluginCall(PLUGIN_SERIAL_IN, 0, dummyString))
        serial();

  // process DNS, only used if the ESP has no valid WiFi config
  if (wifiSetup)
    dnsServer.processNextRequest();

  WebServer.handleClient();

  checkUDP();

  #ifdef FEATURE_ARDUINO_OTA
  ArduinoOTA.handle();

  //once OTA is triggered, only handle that and dont do other stuff. (otherwise it fails)
  while (ArduinoOTAtriggered)
  {
    yield();
    ArduinoOTA.handle();
  }

  #endif

  yield();

  statusLED(false);

  runningBackgroundTasks=false;
}
