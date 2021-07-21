
#ifndef DATASTRUCTS_SETTINGSSTRUCT_H
#define DATASTRUCTS_SETTINGSSTRUCT_H

#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataTypes/EthernetParameters.h"
#include "../DataTypes/NetworkMedium.h"
#include "../Globals/Plugins.h"
#include "../../ESPEasy_common.h"

//we disable SPI if not defined
#ifndef DEFAULT_SPI
 #define DEFAULT_SPI 0
#endif


// FIXME TD-er: Move this PinBootState to DataTypes folder

// State is stored, so don't change order
enum class PinBootState {
  Default_state  = 0,
  Output_low     = 1,
  Output_high    = 2,
  Input_pullup   = 3,
  Input_pulldown = 4,  // Only on ESP32 and GPIO16 on ESP82xx
  Input          = 5,

  // Options for later:
  // ANALOG (only on ESP32)
  // WAKEUP_PULLUP (only on ESP8266)
  // WAKEUP_PULLDOWN (only on ESP8266)
  // SPECIAL
  // FUNCTION_0 (only on ESP8266)
  // FUNCTION_1
  // FUNCTION_2
  // FUNCTION_3
  // FUNCTION_4
  // FUNCTION_5 (only on ESP32)
  // FUNCTION_6 (only on ESP32)

};


/*********************************************************************************************\
 * SettingsStruct
\*********************************************************************************************/
template<unsigned int N_TASKS>
class SettingsStruct_tmpl
{
  public:

  SettingsStruct_tmpl();

  // VariousBits1 defaults to 0, keep in mind when adding bit lookups.
  bool appendUnitToHostname() const;
  void appendUnitToHostname(bool value);

  bool uniqueMQTTclientIdReconnect_unused() const;
  void uniqueMQTTclientIdReconnect_unused(bool value);

  bool OldRulesEngine() const;
  void OldRulesEngine(bool value);

  bool ForceWiFi_bg_mode() const;
  void ForceWiFi_bg_mode(bool value);

  bool WiFiRestart_connection_lost() const;
  void WiFiRestart_connection_lost(bool value);

  bool EcoPowerMode() const;
  void EcoPowerMode(bool value);

  bool WifiNoneSleep() const;
  void WifiNoneSleep(bool value);

  // Enable send gratuitous ARP by default, so invert the values (default = 0)
  bool gratuitousARP() const;
  void gratuitousARP(bool value);

  // Be a bit more tolerant when parsing the last argument of a command.
  // See: https://github.com/letscontrolit/ESPEasy/issues/2724
  bool TolerantLastArgParse() const;
  void TolerantLastArgParse(bool value);

  // SendToHttp command does not wait for ack, with this flag it does wait.
  bool SendToHttp_ack() const;
  void SendToHttp_ack(bool value);

  // Enable/disable ESPEasyNow protocol
  bool UseESPEasyNow() const;
  void UseESPEasyNow(bool value);

  // Whether to try to connect to a hidden SSID network
  bool IncludeHiddenSSID() const;
  void IncludeHiddenSSID(bool value);

  // When sending, the TX power may be boosted to max TX power.
  bool UseMaxTXpowerForSending() const;
  void UseMaxTXpowerForSending(bool value);

  // When set you can use the Sensor in AP-Mode without beeing forced to /setup
  bool ApDontForceSetup() const;
  void ApDontForceSetup(bool value);

  // Perform periodical WiFi scans so that in case of a WiFi disconnect a node may reconnect to a better AP
  bool PeriodicalScanWiFi() const;
  void PeriodicalScanWiFi(bool value);

  // When outputting JSON bools use quoted values (on, backward compatible) or use official JSON true/false unquoted
  bool JSONBoolWithoutQuotes() const;
  void JSONBoolWithoutQuotes(bool value);
  
  // Enable timing statistics (may consume a few kB of RAM)
  bool EnableTimingStats() const;
  void EnableTimingStats(bool value);


  // Flag indicating whether all task values should be sent in a single event or one event per task value (default behavior)
  bool CombineTaskValues_SingleEvent(taskIndex_t taskIndex) const;
  void CombineTaskValues_SingleEvent(taskIndex_t taskIndex, bool value);

  bool DoNotStartAP() const;
  void DoNotStartAP(bool value);

  bool UseAlternativeDeepSleep() const;
  void UseAlternativeDeepSleep(bool value);

  bool UseLastWiFiFromRTC() const;
  void UseLastWiFiFromRTC(bool value);

  void validate();

  bool networkSettingsEmpty() const;

  void clearNetworkSettings();

  void clearTimeSettings();

  void clearNotifications();

  void clearControllers();

  void clearTasks();

  void clearLogSettings();

  void clearUnitNameSettings();

  void clearMisc();

  void clearAll();

  void clearTask(taskIndex_t task);

  // Return hostname + unit when selected to add unit.
  String getHostname() const;

  // Return hostname with explicit set append unit.
  String getHostname(bool appendUnit) const;

  PinBootState getPinBootState(uint8_t gpio_pin) const;
  void setPinBootState(uint8_t gpio_pin, PinBootState state);

  bool getSPI_pins(int8_t spi_gpios[3]) const;

  // Return true when pin is one of the SPI pins and SPI is enabled
  bool isSPI_pin(int8_t pin) const;

  // Return true when pin is one of the configured I2C pins.
  bool isI2C_pin(int8_t pin) const;

  // Return true when pin is one of the fixed Ethernet pins and Ethernet is enabled
  bool isEthernetPin(int8_t pin) const;

  // Return true when pin is one of the optional Ethernet pins and Ethernet is enabled
  bool isEthernetPinOptional(int8_t pin) const;

  // Access to TaskDevicePin1 ... TaskDevicePin3
  // @param pinnr 1 = TaskDevicePin1, ..., 3 = TaskDevicePin3
  int8_t getTaskDevicePin(taskIndex_t taskIndex, uint8_t pinnr) const;

  float getWiFi_TX_power() const;
  void setWiFi_TX_power(float dBm);


  unsigned long PID;
  int           Version;
  int16_t       Build;
  uint8_t          IP[4];
  uint8_t          Gateway[4];
  uint8_t          Subnet[4];
  uint8_t          DNS[4];
  uint8_t          IP_Octet;
  uint8_t          Unit;
  char          Name[26];
  char          NTPHost[64];
  // FIXME TD-er: Issue #2690
  unsigned long Delay;              // Sleep time in seconds
  int8_t        Pin_i2c_sda;
  int8_t        Pin_i2c_scl;
  int8_t        Pin_status_led;
  int8_t        Pin_sd_cs;
  int8_t        PinBootStates[17];  // Only use getPinBootState and setPinBootState as multiple pins are packed for ESP32
  uint8_t          Syslog_IP[4];
  unsigned int  UDPPort;
  uint8_t          SyslogLevel;
  uint8_t          SerialLogLevel;
  uint8_t          WebLogLevel;
  uint8_t          SDLogLevel;
  unsigned long BaudRate;
  unsigned long MessageDelay_unused;  // MQTT settings now moved to the controller settings.
  uint8_t          deepSleep_wakeTime;   // 0 = Sleep Disabled, else time awake from sleep in seconds
  boolean       CustomCSS;
  boolean       DST;
  uint8_t          WDI2CAddress;
  boolean       UseRules;
  boolean       UseSerial;
  boolean       UseSSDP;
  boolean       UseNTP;
  unsigned long WireClockStretchLimit;
  boolean       GlobalSync;
  unsigned long ConnectionFailuresThreshold;
  int16_t       TimeZone;
  boolean       MQTTRetainFlag_unused;
  uint8_t          InitSPI; //0 = disabled, 1= enabled but for ESP32 there is option 2= SPI2 
  // FIXME TD-er: Must change to cpluginID_t, but then also another check must be added since changing the pluginID_t will also render settings incompatible
  uint8_t          Protocol[CONTROLLER_MAX];
  uint8_t          Notification[NOTIFICATION_MAX]; //notifications, point to a NPLUGIN id
  // FIXME TD-er: Must change to pluginID_t, but then also another check must be added since changing the pluginID_t will also render settings incompatible
  uint8_t          TaskDeviceNumber[N_TASKS]; // The "plugin number" set at as task (e.g. 4 for P004_dallas)
  unsigned int  OLD_TaskDeviceID[N_TASKS];  //UNUSED: this can be removed
  union {
    struct {
      int8_t        TaskDevicePin1[N_TASKS];
      int8_t        TaskDevicePin2[N_TASKS];
      int8_t        TaskDevicePin3[N_TASKS];
      uint8_t          TaskDevicePort[N_TASKS];
    };
    int8_t        TaskDevicePin[4][N_TASKS];
  };
  boolean       TaskDevicePin1PullUp[N_TASKS];
  int16_t       TaskDevicePluginConfig[N_TASKS][PLUGIN_CONFIGVAR_MAX];
  boolean       TaskDevicePin1Inversed[N_TASKS];
  float         TaskDevicePluginConfigFloat[N_TASKS][PLUGIN_CONFIGFLOATVAR_MAX];
  long          TaskDevicePluginConfigLong[N_TASKS][PLUGIN_CONFIGLONGVAR_MAX];
  uint8_t          TaskDeviceSendDataFlags[N_TASKS];
  uint8_t          OLD_TaskDeviceGlobalSync[N_TASKS];
  uint8_t          TaskDeviceDataFeed[N_TASKS];    // When set to 0, only read local connected sensorsfeeds
  unsigned long TaskDeviceTimer[N_TASKS];
  boolean       TaskDeviceEnabled[N_TASKS];
  boolean       ControllerEnabled[CONTROLLER_MAX];
  boolean       NotificationEnabled[NOTIFICATION_MAX];
  unsigned int  TaskDeviceID[CONTROLLER_MAX][N_TASKS];        // IDX number (mainly used by Domoticz)
  boolean       TaskDeviceSendData[CONTROLLER_MAX][N_TASKS];
  boolean       Pin_status_led_Inversed;
  boolean       deepSleepOnFail;
  boolean       UseValueLogger;
  boolean       ArduinoOTAEnable;
  uint16_t      DST_Start;
  uint16_t      DST_End;
  boolean       UseRTOSMultitasking;
  int8_t        Pin_Reset;
  uint8_t          SyslogFacility;
  uint32_t      StructSize;  // Forced to be 32 bit, to make sure alignment is clear.
  boolean       MQTTUseUnitNameAsClientId_unused;

  //its safe to extend this struct, up to several bytes, default values in config are 0
  //look in misc.ino how config.dat is used because also other stuff is stored in it at different offsets.
  //TODO: document config.dat somewhere here
  float         Latitude;
  float         Longitude;
  uint32_t      VariousBits1;
  uint32_t      ResetFactoryDefaultPreference; // Do not clear this one in the clearAll()
  uint32_t      I2C_clockSpeed;
  uint16_t      WebserverPort;
  uint16_t      SyslogPort;

  // FIXME @TD-er: As discussed in #1292, the CRC for the settings is now disabled.
  // make sure crc is the last value in the struct
  // Try to extend settings to make the checksum 4-uint8_t aligned.
//  uint8_t       ProgmemMd5[16]; // crc of the binary that last saved the struct to file.
//  uint8_t       md5[16];
  uint8_t       ETH_Phy_Addr;
  int8_t        ETH_Pin_mdc;
  int8_t        ETH_Pin_mdio;
  int8_t        ETH_Pin_power;
  EthPhyType_t   ETH_Phy_Type;
  EthClockMode_t ETH_Clock_Mode;
  uint8_t          ETH_IP[4];
  uint8_t          ETH_Gateway[4];
  uint8_t          ETH_Subnet[4];
  uint8_t          ETH_DNS[4];
  NetworkMedium_t NetworkMedium;
  int8_t        I2C_Multiplexer_Type;
  int8_t        I2C_Multiplexer_Addr;
  int8_t        I2C_Multiplexer_Channel[N_TASKS];
  uint8_t       I2C_Flags[N_TASKS];
  uint32_t      I2C_clockSpeed_Slow;
  uint8_t       I2C_Multiplexer_ResetPin;

  #ifdef ESP32
  int8_t        PinBootStates_ESP32[24]; // pins 17 ... 39
  #endif
  uint8_t       WiFi_TX_power = 70; // 70 = 17.5dBm. unit: 0.25 dBm
  int8_t        WiFi_sensitivity_margin = 3;  // Margin in dBm on top of sensitivity.
  uint8_t       NumberExtraWiFiScans = 0;
  int8_t        ForceESPEasyNOWchannel = 0;
};

/*
SettingsStruct* SettingsStruct_ptr = new (std::nothrow) SettingsStruct;
SettingsStruct& Settings = *SettingsStruct_ptr;
*/



typedef SettingsStruct_tmpl<TASKS_MAX> SettingsStruct;

#endif // DATASTRUCTS_SETTINGSSTRUCT_H
