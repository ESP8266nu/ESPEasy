#include "../Helpers/ESPEasy_FactoryDefault.h"

#include "../../ESPEasy_common.h"
#include "../../_Plugin_Helper.h"

#include "../CustomBuild/StorageLayout.h"

#include "../DataStructs/ControllerSettingsStruct.h"
#include "../DataStructs/FactoryDefaultPref.h"
#include "../DataStructs/GpioFactorySettingsStruct.h"

#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/Serial.h"

#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/RTC.h"
#include "../Globals/ResetFactoryDefaultPref.h"
#include "../Globals/SecuritySettings.h"

#include "../Helpers/_CPlugin_Helper.h"
#include "../Helpers/ESPEasyRTC.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Misc.h"

/********************************************************************************************\
   Reset all settings to factory defaults
 \*********************************************************************************************/
void ResetFactory()
{
  #ifdef USE_CUSTOM_PROVISIONING
  if (ResetFactoryDefaultPreference.getPreference() == 0)
  {
    ResetFactoryDefaultPreference.setDeviceModel(static_cast<DeviceModel>(DEFAULT_FACTORY_DEFAULT_DEVICE_MODEL));
    ResetFactoryDefaultPreference.fetchRulesTXT(0, DEFAULT_PROVISIONING_FETCH_RULES1);
    ResetFactoryDefaultPreference.fetchRulesTXT(1, DEFAULT_PROVISIONING_FETCH_RULES2);
    ResetFactoryDefaultPreference.fetchRulesTXT(2, DEFAULT_PROVISIONING_FETCH_RULES3);
    ResetFactoryDefaultPreference.fetchRulesTXT(3, DEFAULT_PROVISIONING_FETCH_RULES4);
    ResetFactoryDefaultPreference.fetchNotificationDat(DEFAULT_PROVISIONING_FETCH_NOTIFICATIONS);
    ResetFactoryDefaultPreference.fetchSecurityDat(DEFAULT_PROVISIONING_FETCH_SECURITY);
    ResetFactoryDefaultPreference.fetchConfigDat(DEFAULT_PROVISIONING_FETCH_CONFIG);
    ResetFactoryDefaultPreference.fetchProvisioningDat(DEFAULT_PROVISIONING_FETCH_PROVISIONING);
    ResetFactoryDefaultPreference.saveURL(DEFAULT_PROVISIONING_SAVE_URL);
    ResetFactoryDefaultPreference.storeCredentials(DEFAULT_PROVISIONING_SAVE_CREDENTIALS);
    ResetFactoryDefaultPreference.allowFetchByCommand(DEFAULT_PROVISIONING_ALLOW_FETCH_COMMAND);
  }
  #endif


  const GpioFactorySettingsStruct gpio_settings(ResetFactoryDefaultPreference.getDeviceModel());
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("ResetFactory"));
  #endif

  // Direct Serial is allowed here, since this is only an emergency task.
  serialPrint(F("RESET: Resetting factory defaults... using "));
  serialPrint(getDeviceModelString(ResetFactoryDefaultPreference.getDeviceModel()));
  serialPrintln(F(" settings"));
  delay(1000);

  if (readFromRTC())
  {
    serialPrint(F("RESET: Warm boot, reset count: "));
    serialPrintln(String(RTC.factoryResetCounter));

    if (RTC.factoryResetCounter >= 3)
    {
      serialPrintln(F("RESET: Too many resets, protecting your flash memory (powercycle to solve this)"));
      return;
    }
  }
  else
  {
    serialPrintln(F("RESET: Cold boot"));
    initRTC();

    // TODO TD-er: Store set device model in RTC.
  }

  RTC.flashCounter = 0; // reset flashcounter, since we're already counting the number of factory-resets. we dont want to hit a flash-count
                        // limit during reset.
  RTC.factoryResetCounter++;
  saveToRTC();

  // always format on factory reset, in case of corrupt FS
  ESPEASY_FS.end();
  serialPrintln(F("RESET: formatting..."));
  ESPEASY_FS.format();
  serialPrintln(F("RESET: formatting done..."));

  if (!ESPEASY_FS.begin())
  {
    serialPrintln(F("RESET: FORMAT FS FAILED!"));
    return;
  }

#ifdef USE_CUSTOM_PROVISIONING
  {
    MakeProvisioningSettings(ProvisioningSettings);
    if (AllocatedProvisioningSettings()) {
      ProvisioningSettings.setUser(F(DEFAULT_PROVISIONING_USER));
      ProvisioningSettings.setPass(F(DEFAULT_PROVISIONING_PASS));
      ProvisioningSettings.setUrl(F(DEFAULT_PROVISIONING_URL));
      ProvisioningSettings.ResetFactoryDefaultPreference = ResetFactoryDefaultPreference.getPreference();
      saveProvisioningSettings(ProvisioningSettings);
    }
  }
#endif

  // pad files with extra zeros for future extensions
  InitFile(SettingsType::SettingsFileEnum::FILE_CONFIG_type);
  InitFile(SettingsType::SettingsFileEnum::FILE_SECURITY_type);
  #ifdef USES_NOTIFIER
  InitFile(SettingsType::SettingsFileEnum::FILE_NOTIFICATION_type);
  #endif

  String fname = getRulesFileName(0);
  InitFile(fname.c_str(), 0);

  Settings.clearMisc();

  if (!ResetFactoryDefaultPreference.keepNTP()) {
    Settings.clearTimeSettings();
    Settings.UseNTP = DEFAULT_USE_NTP;
    strcpy_P(Settings.NTPHost, PSTR(DEFAULT_NTP_HOST));
    Settings.TimeZone = DEFAULT_TIME_ZONE;
    Settings.DST      = DEFAULT_USE_DST;
  }

  if (!ResetFactoryDefaultPreference.keepNetwork()) {
    Settings.clearNetworkSettings();

    // TD-er Reset access control
    str2ip(F(DEFAULT_IPRANGE_LOW),  SecuritySettings.AllowedIPrangeLow);
    str2ip(F(DEFAULT_IPRANGE_HIGH), SecuritySettings.AllowedIPrangeHigh);
    SecuritySettings.IPblockLevel = DEFAULT_IP_BLOCK_LEVEL;

    #if DEFAULT_USE_STATIC_IP
    str2ip((char *)DEFAULT_IP,     Settings.IP);
    str2ip((char *)DEFAULT_DNS,    Settings.DNS);
    str2ip((char *)DEFAULT_GW,     Settings.Gateway);
    str2ip((char *)DEFAULT_SUBNET, Settings.Subnet);
    #endif // if DEFAULT_USE_STATIC_IP
    Settings.IncludeHiddenSSID(DEFAULT_WIFI_INCLUDE_HIDDEN_SSID);
  }

  Settings.clearNotifications();
  Settings.clearControllers();
  Settings.clearTasks();

  if (!ResetFactoryDefaultPreference.keepLogSettings()) {
    Settings.clearLogSettings();
    str2ip((char *)DEFAULT_SYSLOG_IP, Settings.Syslog_IP);

    setLogLevelFor(LOG_TO_SYSLOG, DEFAULT_SYSLOG_LEVEL);
    setLogLevelFor(LOG_TO_SERIAL, DEFAULT_SERIAL_LOG_LEVEL);
    setLogLevelFor(LOG_TO_WEBLOG, DEFAULT_WEB_LOG_LEVEL);
    setLogLevelFor(LOG_TO_SDCARD, DEFAULT_SD_LOG_LEVEL);
    Settings.SyslogFacility = DEFAULT_SYSLOG_FACILITY;
    Settings.UseValueLogger = DEFAULT_USE_SD_LOG;
  }

  if (!ResetFactoryDefaultPreference.keepUnitName()) {
    Settings.clearUnitNameSettings();
    Settings.Unit = UNIT;
    strcpy_P(Settings.Name, PSTR(DEFAULT_NAME));
    Settings.UDPPort = DEFAULT_SYNC_UDP_PORT;
  }

  if (!ResetFactoryDefaultPreference.keepWiFi()) {
    strcpy_P(SecuritySettings.WifiSSID,  PSTR(DEFAULT_SSID));
    strcpy_P(SecuritySettings.WifiKey,   PSTR(DEFAULT_KEY));
    strcpy_P(SecuritySettings.WifiSSID2, PSTR(DEFAULT_SSID2));
    strcpy_P(SecuritySettings.WifiKey2,  PSTR(DEFAULT_KEY2));
    strcpy_P(SecuritySettings.WifiAPKey, PSTR(DEFAULT_AP_KEY));
    SecuritySettings.WifiSSID2[0] = 0;
    SecuritySettings.WifiKey2[0]  = 0;
  }
  strcpy_P(SecuritySettings.Password, PSTR(DEFAULT_ADMIN_PASS));

  Settings.ResetFactoryDefaultPreference = ResetFactoryDefaultPreference.getPreference();

  // now we set all parameters that need to be non-zero as default value


  Settings.PID     = ESP_PROJECT_PID;
  Settings.Version = VERSION;
  Settings.Build   = BUILD;

  //  Settings.IP_Octet				 = DEFAULT_IP_OCTET;
  Settings.Delay                   = DEFAULT_DELAY;
  Settings.Pin_i2c_sda             = gpio_settings.i2c_sda;
  Settings.Pin_i2c_scl             = gpio_settings.i2c_scl;
  Settings.Pin_status_led          = gpio_settings.status_led;
  Settings.Pin_status_led_Inversed = DEFAULT_PIN_STATUS_LED_INVERSED;
  Settings.Pin_sd_cs               = -1;
  Settings.Pin_Reset               = DEFAULT_PIN_RESET_BUTTON;
  Settings.Protocol[0]             = DEFAULT_PROTOCOL;
  Settings.deepSleep_wakeTime      = false;
  Settings.CustomCSS               = false;
  Settings.InitSPI                 = DEFAULT_SPI;

  for (taskIndex_t x = 0; x < TASKS_MAX; x++)
  {
    Settings.TaskDevicePin1[x]         = -1;
    Settings.TaskDevicePin2[x]         = -1;
    Settings.TaskDevicePin3[x]         = -1;
    Settings.TaskDevicePin1PullUp[x]   = true;
    Settings.TaskDevicePin1Inversed[x] = false;

    for (controllerIndex_t y = 0; y < CONTROLLER_MAX; y++) {
      Settings.TaskDeviceSendData[y][x] = true;
    }
    Settings.TaskDeviceTimer[x] = Settings.Delay;
  }

  // advanced Settings
  Settings.UseRules                         = DEFAULT_USE_RULES;
  Settings.ControllerEnabled[0]             = DEFAULT_CONTROLLER_ENABLED;
  Settings.MQTTRetainFlag_unused            = DEFAULT_MQTT_RETAIN;
  Settings.MessageDelay_unused              = DEFAULT_MQTT_DELAY;
  Settings.MQTTUseUnitNameAsClientId_unused = DEFAULT_MQTT_USE_UNITNAME_AS_CLIENTID;

  // allow to set default latitude and longitude
  #ifdef DEFAULT_LATITUDE
  Settings.Latitude = DEFAULT_LATITUDE;
  #endif // ifdef DEFAULT_LATITUDE
  #ifdef DEFAULT_LONGITUDE
  Settings.Longitude = DEFAULT_LONGITUDE;
  #endif // ifdef DEFAULT_LONGITUDE

  Settings.UseSerial = DEFAULT_USE_SERIAL;
  Settings.BaudRate  = DEFAULT_SERIAL_BAUD;

  Settings.ETH_Phy_Addr   = gpio_settings.eth_phyaddr;
  Settings.ETH_Pin_mdc    = gpio_settings.eth_mdc;
  Settings.ETH_Pin_mdio   = gpio_settings.eth_mdio;
  Settings.ETH_Pin_power  = gpio_settings.eth_power;
  Settings.ETH_Phy_Type   = gpio_settings.eth_phytype;
  Settings.ETH_Clock_Mode = gpio_settings.eth_clock_mode;
  Settings.NetworkMedium  = gpio_settings.network_medium;

  /*
          Settings.GlobalSync						= DEFAULT_USE_GLOBAL_SYNC;

          Settings.IP_Octet						= DEFAULT_IP_OCTET;
          Settings.WDI2CAddress					= DEFAULT_WD_IC2_ADDRESS;
          Settings.UseSSDP						= DEFAULT_USE_SSDP;
          Settings.ConnectionFailuresThreshold	= DEFAULT_CON_FAIL_THRES;
          Settings.WireClockStretchLimit			= DEFAULT_I2C_CLOCK_LIMIT;
   */
  Settings.I2C_clockSpeed = DEFAULT_I2C_CLOCK_SPEED;

  Settings.JSONBoolWithoutQuotes(DEFAULT_JSON_BOOL_WITHOUT_QUOTES);
  Settings.EnableTimingStats(DEFAULT_ENABLE_TIMING_STATS);

#ifdef PLUGIN_DESCR
  strcpy_P(Settings.Name, PSTR(PLUGIN_DESCR));
#endif // ifdef PLUGIN_DESCR

#ifndef LIMIT_BUILD_SIZE
  addPredefinedPlugins(gpio_settings);
  addPredefinedRules(gpio_settings);
#endif

#if DEFAULT_CONTROLLER
  {
    // Place in a scope to have its memory freed ASAP
    MakeControllerSettings(ControllerSettings);

    if (AllocatedControllerSettings()) {
      safe_strncpy(ControllerSettings.Subscribe,            F(DEFAULT_SUB),            sizeof(ControllerSettings.Subscribe));
      safe_strncpy(ControllerSettings.Publish,              F(DEFAULT_PUB),            sizeof(ControllerSettings.Publish));
      safe_strncpy(ControllerSettings.MQTTLwtTopic,         F(DEFAULT_MQTT_LWT_TOPIC), sizeof(ControllerSettings.MQTTLwtTopic));
      safe_strncpy(ControllerSettings.LWTMessageConnect,    F(DEFAULT_MQTT_LWT_CONNECT_MESSAGE),
                   sizeof(ControllerSettings.LWTMessageConnect));
      safe_strncpy(ControllerSettings.LWTMessageDisconnect, F(DEFAULT_MQTT_LWT_DISCONNECT_MESSAGE),
                   sizeof(ControllerSettings.LWTMessageDisconnect));
      str2ip((char *)DEFAULT_SERVER, ControllerSettings.IP);
      ControllerSettings.setHostname(F(DEFAULT_SERVER_HOST));
      ControllerSettings.UseDNS = DEFAULT_SERVER_USEDNS;
      ControllerSettings.useExtendedCredentials(DEFAULT_USE_EXTD_CONTROLLER_CREDENTIALS);
      ControllerSettings.Port = DEFAULT_PORT;
      setControllerUser(0, ControllerSettings, F(DEFAULT_CONTROLLER_USER));
      setControllerPass(0, ControllerSettings, F(DEFAULT_CONTROLLER_PASS));

      SaveControllerSettings(0, ControllerSettings);
    }
  }
#endif // if DEFAULT_CONTROLLER

  SaveSettings();
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("ResetFactory2"));
  #endif
  serialPrintln(F("RESET: Successful, rebooting. (you might need to press the reset button if you've just flashed the firmware)"));

  // NOTE: this is a known ESP8266 bug, not our fault. :)
  delay(1000);
  WiFi.persistent(true);  // use SDK storage of SSID/WPA parameters
  WiFiEventData.intent_to_reboot = true;
  WifiDisconnect();       // this will store empty ssid/wpa into sdk storage
  WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
  reboot(ESPEasy_Scheduler::IntendedRebootReason_e::ResetFactory);
}


/*********************************************************************************************\
   Collect the stored preference for factory default
\*********************************************************************************************/
void applyFactoryDefaultPref() {
  // TODO TD-er: Store it in more places to make it more persistent
  Settings.ResetFactoryDefaultPreference = ResetFactoryDefaultPreference.getPreference();
}
