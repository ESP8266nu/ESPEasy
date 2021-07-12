#ifndef ESPEASY_CUSTOM_H
#define ESPEASY_CUSTOM_H

#include <Arduino.h>

/*
    To modify the stock configuration without changing the EspEasy.ino file :
    1) rename this file to "Custom.h" (It is ignored by Git)
    2) define your own settings below
    3) define USE_CUSTOM_H as a build flags. ie : export PLATFORMIO_BUILD_FLAGS="'-DUSE_CUSTOM_H'"
 */


/*
 #######################################################################################################
   Your Own Default Settings
 #######################################################################################################
    You can basically ovveride ALL macro defined in ESPEasy.ino.
    Don't forget to first #undef each existing #define that you add below.
    But since this Custom.h is included before other defines are made, you don't have to undef a lot of defines.
    Here are some examples:
 */

#ifdef BUILD_GIT
# undef BUILD_GIT
#endif // ifdef BUILD_GIT

#define BUILD_GIT           "My Build: "  __DATE__ " "  __TIME__


#define DEFAULT_NAME        "MyEspEasyDevice"                        // Enter your device friendly name
#define UNIT                0                                        // Unit Number
#define DEFAULT_DELAY       60                                       // Sleep Delay in seconds

// --- Wifi AP Mode (when your Wifi Network is not reachable) ----------------------------------------
#define DEFAULT_AP_IP       192, 168, 4, 1                           // Enter IP address (comma separated) for AP (config) mode
#define DEFAULT_AP_SUBNET   255, 255, 255, 0                         // Enter IP address (comma separated) for AP (config) mode
#define DEFAULT_AP_KEY      "configesp"                              // Enter network WPA key for AP (config) mode

// --- Wifi Client Mode -----------------------------------------------------------------------------
#define DEFAULT_SSID                         "MyHomeSSID"            // Enter your network SSID
#define DEFAULT_KEY                          "MySuperSecretPassword" // Enter your network WPA key
#define DEFAULT_SSID2                        ""                      // Enter your fallback network SSID
#define DEFAULT_KEY2                         ""                      // Enter your fallback network WPA key
#define DEFAULT_WIFI_INCLUDE_HIDDEN_SSID     false                   // Allow to connect to hidden SSID APs
#define DEFAULT_USE_STATIC_IP                false                   // (true|false) enabled or disabled static IP
#define DEFAULT_IP                           "192.168.0.50"          // Enter your IP address
#define DEFAULT_DNS                          "192.168.0.1"           // Enter your DNS
#define DEFAULT_GW                           "192.168.0.1"           // Enter your Gateway
#define DEFAULT_SUBNET                       "255.255.255.0"         // Enter your Subnet
#define DEFAULT_IPRANGE_LOW                  "0.0.0.0"               // Allowed IP range to access webserver
#define DEFAULT_IPRANGE_HIGH                 "255.255.255.255"       // Allowed IP range to access webserver
#define DEFAULT_IP_BLOCK_LEVEL               1                       // 0: ALL_ALLOWED  1: LOCAL_SUBNET_ALLOWED  2:
// ONLY_IP_RANGE_ALLOWED
#define DEFAULT_ADMIN_USERNAME               "admin"
#define DEFAULT_ADMIN_PASS                   ""

#define DEFAULT_WIFI_CONNECTION_TIMEOUT      10000 // minimum timeout in ms for WiFi to be connected.
#define DEFAULT_WIFI_FORCE_BG_MODE           false // when set, only allow to connect in 802.11B or G mode (not N)
#define DEFAULT_WIFI_RESTART_WIFI_CONN_LOST  false // Perform wifi off and on when connection was lost.
#define DEFAULT_ECO_MODE                     false // When set, make idle calls between executing tasks.
#define DEFAULT_WIFI_NONE_SLEEP              false // When set, the wifi will be set to no longer sleep (more power
// used and need reboot to reset mode)
#define DEFAULT_GRATUITOUS_ARP               false // When set, the node will send periodical gratuitous ARP
                                                   // packets to announce itself.
#define DEFAULT_TOLERANT_LAST_ARG_PARSE      false // When set, the last argument of some commands will be parsed to the end of the line
                                                   // See: https://github.com/letscontrolit/ESPEasy/issues/2724
#define DEFAULT_SEND_TO_HTTP_ACK             false // Wait for ack with SendToHttp command.

#define DEFAULT_AP_DONT_FORCE_SETUP          false // Allow optional usage of Sensor without WIFI avaiable // When set you can use the Sensor in AP-Mode without beeing forced to /setup
#define DEFAULT_DONT_ALLOW_START_AP          false // Usually the AP will be started when no WiFi is defined, or the defined one cannot be found. This flag may prevent it.

// --- Default Controller ------------------------------------------------------------------------------
#define DEFAULT_CONTROLLER   false                                          // true or false enabled or disabled, set 1st controller
                                                                            // defaults
#define DEFAULT_CONTROLLER_ENABLED true                                     // Enable default controller by default
#define DEFAULT_CONTROLLER_USER    ""                                       // Default controller user
#define DEFAULT_CONTROLLER_PASS    ""                                       // Default controller Password

// using a default template, you also need to set a DEFAULT PROTOCOL to a suitable MQTT protocol !
#define DEFAULT_PUB         "sensors/espeasy/%sysname%/%tskname%/%valname%" // Enter your pub
#define DEFAULT_SUB         "sensors/espeasy/%sysname%/#"                   // Enter your sub
#define DEFAULT_SERVER      "192.168.0.8"                                   // Enter your Server IP address
#define DEFAULT_SERVER_HOST ""                                              // Server hostname
#define DEFAULT_SERVER_USEDNS false                                         // true: Use hostname.  false: use IP
#define DEFAULT_USE_EXTD_CONTROLLER_CREDENTIALS   false                     // true: Allow longer user credentials for controllers

#define DEFAULT_PORT        8080                                            // Enter your Server port value

#define DEFAULT_PROTOCOL    0                                               // Protocol used for controller communications
                                                                            //   0 = Stand-alone (no controller set)
                                                                            //   1 = Domoticz HTTP
                                                                            //   2 = Domoticz MQTT
                                                                            //   3 = Nodo Telnet
                                                                            //   4 = ThingSpeak
                                                                            //   5 = Home Assistant (openHAB) MQTT
                                                                            //   6 = PiDome MQTT
                                                                            //   7 = EmonCMS
                                                                            //   8 = Generic HTTP
                                                                            //   9 = FHEM HTTP

#define DEFAULT_PIN_I2C_SDA                     4
#define DEFAULT_PIN_I2C_SCL                     5
#define DEFAULT_I2C_CLOCK_SPEED                 400000            // Use 100 kHz if working with old I2C chips

#define DEFAULT_SPI                             0                 //0=disabled 1=enabled and for ESP32 there is option 2 =HSPI

#define DEFAULT_PIN_STATUS_LED                  (-1)
#define DEFAULT_PIN_STATUS_LED_INVERSED         true

#define DEFAULT_PIN_RESET_BUTTON                (-1)


#define DEFAULT_USE_RULES                       false             // (true|false) Enable Rules?
#define DEFAULT_RULES_OLDENGINE                 true

#define DEFAULT_MQTT_RETAIN                     false             // (true|false) Retain MQTT messages?
#define DEFAULT_MQTT_DELAY                      100               // Time in milliseconds to retain MQTT messages
#define DEFAULT_MQTT_LWT_TOPIC                  ""                // Default lwt topic
#define DEFAULT_MQTT_LWT_CONNECT_MESSAGE        "Connected"       // Default lwt message
#define DEFAULT_MQTT_LWT_DISCONNECT_MESSAGE     "Connection Lost" // Default lwt message
#define DEFAULT_MQTT_USE_UNITNAME_AS_CLIENTID   0

#define DEFAULT_USE_NTP                         false             // (true|false) Use NTP Server
#define DEFAULT_NTP_HOST                        ""                // NTP Server Hostname
#define DEFAULT_TIME_ZONE                       0                 // Time Offset (in minutes)
#define DEFAULT_USE_DST                         false             // (true|false) Use Daily Time Saving

#define DEFAULT_LATITUDE                        0.0f              // Default Latitude  
#define DEFAULT_LONGITUDE                       0.0f              // Default Longitude

#define DEFAULT_SYSLOG_IP                       ""                // Syslog IP Address
#define DEFAULT_SYSLOG_LEVEL                    0                 // Syslog Log Level
#define DEFAULT_SERIAL_LOG_LEVEL                LOG_LEVEL_INFO    // Serial Log Level
#define DEFAULT_WEB_LOG_LEVEL                   LOG_LEVEL_INFO    // Web Log Level
#define DEFAULT_SD_LOG_LEVEL                    0                 // SD Card Log Level
#define DEFAULT_USE_SD_LOG                      false             // (true|false) Enable Logging to the SD card

#define DEFAULT_USE_SERIAL                      true              // (true|false) Enable Logging to the Serial Port
#define DEFAULT_SERIAL_BAUD                     115200            // Serial Port Baud Rate
#define DEFAULT_SYSLOG_FACILITY                 0                 // kern

#define DEFAULT_SYNC_UDP_PORT                   0                 // Used for ESPEasy p2p. (IANA registered port: 8266)


#define BUILD_NO_DEBUG


// Special SSID/key setup only to be used in custom builds.

// Deployment SSID will be used only when the configured SSIDs are not reachable and/or no credentials are set.
// This to make deployment of large number of nodes easier
#define CUSTOM_DEPLOYMENT_SSID                  ""                // Enter SSID not shown in UI, to be used on custom builds to ease deployment
#define CUSTOM_DEPLOYMENT_KEY                   ""                // Enter key not shown in UI, to be used on custom builds to ease deployment
#define CUSTOM_SUPPORT_SSID                     ""                // Enter SSID not shown in UI, to be used on custom builds to ease support
#define CUSTOM_SUPPORT_KEY                      ""                // Enter key not shown in UI, to be used on custom builds to ease support


// Emergency fallback SSID will only be attempted in the first 10 minutes after reboot.
// When found, the unit will connect to it and depending on the built in flag, it will either just connect to it, or clear set credentials.
// Use case: User connects to a public AP which does need to agree on an agreement page for the rules of conduct (e.g. open APs)
// This is seen as a valid connection, so the unit will not reconnect to another node and thus becomes inaccessible.
#define CUSTOM_EMERGENCY_FALLBACK_SSID          ""                // Enter SSID not shown in UI, to be used to regain access to the node
#define CUSTOM_EMERGENCY_FALLBACK_KEY           ""                // Enter key not shown in UI, to be used to regain access to the node

#define CUSTOM_EMERGENCY_FALLBACK_RESET_CREDENTIALS  false
#define CUSTOM_EMERGENCY_FALLBACK_START_AP           false

#define CUSTOM_EMERGENCY_FALLBACK_ALLOW_MINUTES_UPTIME 10

#define USES_SSDP


// #define USE_SETTINGS_ARCHIVE
// #define FEATURE_I2CMULTIPLEXER
// #define USE_TRIGONOMETRIC_FUNCTIONS_RULES

/*
 #######################################################################################################
   Defining web interface
 #######################################################################################################
 */

#define MENU_INDEX_MAIN_VISIBLE          true
/*
#define MENU_INDEX_CONFIG_VISIBLE        false
#define MENU_INDEX_CONTROLLERS_VISIBLE   false
#define MENU_INDEX_HARDWARE_VISIBLE      false
#define MENU_INDEX_DEVICES_VISIBLE       false
#define MENU_INDEX_RULES_VISIBLE         false
#define MENU_INDEX_NOTIFICATIONS_VISIBLE false
#define MENU_INDEX_TOOLS_VISIBLE         false
*/

#define MAIN_PAGE_SHOW_SYSINFO_BUTTON    true
#define MAIN_PAGE_SHOW_WiFi_SETUP_BUTTON true
#define MAIN_PAGE_SHOW_BASIC_INFO_NOT_LOGGED_IN false

#define MAIN_PAGE_SHOW_NODE_LIST_BUILD   true
#define MAIN_PAGE_SHOW_NODE_LIST_TYPE    true

#define SETUP_PAGE_SHOW_CONFIG_BUTTON    true


//#define WEBPAGE_TEMPLATE_HIDE_HELP_BUTTON


/*
 #######################################################################################################
   CSS / template
 #######################################################################################################
 */
/*
#define WEBPAGE_TEMPLATE_DEFAULT_HEADER "<header class='headermenu'><h1>ESP Easy Mega: {{title}}</h1><BR>"
#define WEBPAGE_TEMPLATE_DEFAULT_FOOTER "<footer><br><h6>Powered by <a href='http://www.letscontrolit.com' style='font-size: 15px; text-decoration: none'>Let's Control It</a> community</h6></footer></body></html>"
#define WEBPAGE_TEMPLATE_AP_HEADER      "<body><header class='apheader'><h1>Welcome to ESP Easy Mega AP</h1>"
#define WEBPAGE_TEMPLATE_HIDE_HELP_BUTTON
*/
// Embed Custom CSS in Custom.h:
/*
#define WEBSERVER_EMBED_CUSTOM_CSS
static const char DATA_ESPEASY_DEFAULT_MIN_CSS[] PROGMEM = {
...
,0};
*/


/*
 #######################################################################################################
   Special settings  (rendering settings incompatible with other builds)
 #######################################################################################################
 */

// #define USE_NON_STANDARD_24_TASKS

/*
 #######################################################################################################
   Your Own selection of plugins and controllers
 #######################################################################################################
 */

#define CONTROLLER_SET_NONE
#define NOTIFIER_SET_NONE
#define PLUGIN_SET_NONE


/*
 #######################################################################################################
 ###########     Plugins
 #######################################################################################################
 */

// #define USE_SERVO


// #define USES_P001   // Switch
// #define USES_P002   // ADC
// #define USES_P003   // Pulse
// #define USES_P004   // Dallas
// #define USES_P005   // DHT
// #define USES_P006   // BMP085
// #define USES_P007   // PCF8591
// #define USES_P008   // RFID
// #define USES_P009   // MCP

// #define USES_P010   // BH1750
// #define USES_P011   // PME
// #define USES_P012   // LCD
// #define USES_P013   // HCSR04
// #define USES_P014   // SI7021
// #define USES_P015   // TSL2561
// #define USES_P017   // PN532
// #define USES_P018   // Dust
// #define USES_P019   // PCF8574

// #define USES_P020   // Ser2Net
// #define USES_P021   // Level
// #define USES_P022   // PCA9685
// #define USES_P023   // OLED
// #define USES_P024   // MLX90614
// #define USES_P025   // ADS1115
// #define USES_P026   // SysInfo
// #define USES_P027   // INA219
// #define USES_P028   // BME280
// #define USES_P029   // Output

// #define USES_P031   // SHT1X
// #define USES_P032   // MS5611
// #define USES_P033   // Dummy
// #define USES_P034   // DHT12
// #define USES_P036   // FrameOLED
// #define USES_P037   // MQTTImport
// #define USES_P038   // NeoPixel
// #define USES_P039   // Environment - Thermocouple

// #define USES_P040   // RFID - ID12LA/RDM6300
// #define USES_P041   // NeoClock
// #define USES_P042   // Candle
// #define USES_P043   // ClkOutput
// #define USES_P044   // P1WifiGateway
// #define USES_P045   // MPU6050
// #define USES_P046   // VentusW266
// #define USES_P047   // I2C_soil_misture
// #define USES_P048   // Motoshield_v2
// #define USES_P049   // MHZ19

// #define USES_P050   // TCS34725 RGB Color Sensor with IR filter and White LED
// #define USES_P051   // AM2320
// #define USES_P052   // SenseAir
// #define USES_P053   // PMSx003
// #define USES_P054   // DMX512
// #define USES_P055   // Chiming
// #define USES_P056   // SDS011-Dust
// #define USES_P057   // HT16K33_LED
// #define USES_P058   // HT16K33_KeyPad
// #define USES_P059   // Encoder

// #define USES_P060   // MCP3221
// #define USES_P061   // Keypad
// #define USES_P062   // MPR121_KeyPad
// #define USES_P063   // TTP229_KeyPad
// #define USES_P064   // APDS9960 Gesture
// #define USES_P065   // DRF0299
// #define USES_P066   // VEML6040
// #define USES_P067   // HX711_Load_Cell
// #define USES_P068   // SHT3x
// #define USES_P069   // LM75A

// #define USES_P070   // NeoPixel_Clock
// #define USES_P071   // Kamstrup401
// #define USES_P072   // HDC1080
// #define USES_P073   // 7DG
// #define USES_P074   // TSL2561
// #define USES_P075   // Nextion
// #define USES_P076   // HWL8012   in POW r1
// #define USES_P077   // CSE7766   in POW R2
// #define USES_P078   // Eastron Modbus Energy meters
// #define USES_P079   // Wemos Motoshield

// #define USES_P080   // iButton Sensor  DS1990A
// #define USES_P081   // Cron
// #define USES_P082   // GPS
// #define USES_P083   // SGP30
// #define USES_P084   // VEML6070
// #define USES_P085   // AcuDC24x
// #define USES_P086   // Receiving values according Homie convention. Works together with C014 Homie controller
// #define USES_P087   // Serial Proxy
// #define USES_P088   // HeatpumpIR
// #define USES_P089   // Ping

// #define USES_P090   // CCS811
// #define USES_P091   // SerSwitch
// #define USES_P092   // DLbus
// #define USES_P093   // MitsubishiHP
// #define USES_P094   // CULReader
// #define USES_P095   // ILI9341
// #define USES_P096   // eInk
// #define USES_P097   // ESP32Touch
// #define USES_P098   // 
// #define USES_P099   // XPT2046 touchscreen

// #define USES_P100   // DS2423 counter
// #define USES_P101   // WakeOnLan
// #define USES_P102   // PZEM004Tv3
// #define USES_P103   // Atlas_EZO_pH
// #define USES_P104   // Atlas_EZO_EC
// #define USES_P105   // Atlas_EZO_ORP
// #define USES_P106   // BME680
// #define USES_P107   // Si1145
// #define USES_P109   // ThermoOLED
// #define USES_P110   // VL53L0X Time of Flight sensor
// #define USES_P111   // RF522 RFID reader
// #define USES_P113   // VL53L1X ToF
// #define USES_P115   // MAX1704x


// Special plugins needing IR library
// #define USES_P016   // IR
// #define P016_SEND_IR_TO_CONTROLLER false //IF true then the JSON replay solution is transmited back to the condroller.
// #define USES_P035   // IRTX
// #define P016_P035_Extended_AC // The following define is needed for extended decoding of A/C Messages and or using standardised 
                                 //common arguments for controlling all deeply supported A/C units
// #define P016_P035_USE_RAW_RAW2 //Use the RAW and RAW2 encodings, disabling it saves 3.7Kb
// #define USES_P088   // Heatpump IR
// #define USES_P108   // DDS238-x ZN Modbus energy meters


/*
 #######################################################################################################
 ###########     Controllers
 #######################################################################################################
 */


// #define USES_C001   // Domoticz HTTP
// #define USES_C002   // Domoticz MQTT
// #define USES_C003   // Nodo telnet
// #define USES_C004   // ThingSpeak
// #define USES_C005   // Home Assistant (openHAB) MQTT
// #define USES_C006   // PiDome MQTT
// #define USES_C007   // Emoncms
// #define USES_C008   // Generic HTTP
// #define USES_C009   // FHEM HTTP
// #define USES_C010   // Generic UDP
// #define USES_C011   // Generic HTTP Advanced
// #define USES_C012   // Blynk HTTP
// #define USES_C013   // ESPEasy P2P network
// #define USES_C014   // homie 3 & 4dev MQTT
// #define USES_C015   // Blynk
// #define USES_C016   // Cache controller
// #define USES_C017   // Zabbix
// #define USES_C018   // TTN/RN2483


/*
 #######################################################################################################
 ###########     Notifiers
 #######################################################################################################
 */


// #define USES_N001   // Email
// #define USES_N002   // Buzzer


#endif // ESPEASY_CUSTOM_H
