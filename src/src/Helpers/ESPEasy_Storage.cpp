#include "../Helpers/ESPEasy_Storage.h"

#include "../../ESPEasy_common.h"

#include "../CustomBuild/StorageLayout.h"

#include "../DataStructs/TimingStats.h"

#include "../DataTypes/ESPEasyFileType.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/Serial.h"

#include "../Globals/CRCValues.h"
#include "../Globals/Cache.h"
#include "../Globals/Device.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/EventQueue.h"
#include "../Globals/ExtraTaskSettings.h"
#include "../Globals/Plugins.h"
#include "../Globals/RTC.h"
#include "../Globals/ResetFactoryDefaultPref.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Settings.h"

#include "../Helpers/ESPEasyRTC.h"
#include "../Helpers/ESPEasy_FactoryDefault.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/FS_Helper.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Memory.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/PeriodicalActions.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"

#include "ESPEasy_checks.h"

#ifdef ESP32
#include <MD5Builder.h>
#include <esp_partition.h>
#endif

#ifdef ESP32
String patch_fname(const String& fname) {
  if (fname.startsWith(F("/"))) {
    return fname;
  }
  return String(F("/")) + fname;
}
#endif
#ifdef ESP8266
#define patch_fname(F) (F)
#endif

/********************************************************************************************\
   file system error handling
   Look here for error # reference: https://github.com/pellepl/spiffs/blob/master/src/spiffs.h
 \*********************************************************************************************/
String FileError(int line, const char *fname)
{
  String err = F("FS   : Error while reading/writing ");

  err += fname;
  err += F(" in ");
  err += line;
  addLog(LOG_LEVEL_ERROR, err);
  return err;
}

/********************************************************************************************\
   Keep track of number of flash writes.
 \*********************************************************************************************/
void flashCount()
{
  if (RTC.flashDayCounter <= MAX_FLASHWRITES_PER_DAY) {
    RTC.flashDayCounter++;
  }
  RTC.flashCounter++;
  saveToRTC();
}

String flashGuard()
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("flashGuard"));
  #endif

  if (RTC.flashDayCounter > MAX_FLASHWRITES_PER_DAY)
  {
    String log = F("FS   : Daily flash write rate exceeded! (powercycle to reset this)");
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }
  flashCount();
  return String();
}

// use this in function that can return an error string. it automaticly returns with an error string if there where too many flash writes.
#define FLASH_GUARD() { String flashErr = flashGuard(); \
                        if (flashErr.length()) return flashErr; }


String appendLineToFile(const String& fname, const String& line) {
  return appendToFile(fname, reinterpret_cast<const uint8_t *>(line.c_str()), line.length());
}

String appendToFile(const String& fname, const uint8_t *data, unsigned int size) {
  fs::File f = tryOpenFile(fname, "a+");

  SPIFFS_CHECK(f,                   fname.c_str());
  SPIFFS_CHECK(f.write(data, size), fname.c_str());
  f.close();
  return "";
}

bool fileExists(const String& fname) {
  const String patched_fname = patch_fname(fname);
  auto search = Cache.fileExistsMap.find(patched_fname);
  if (search != Cache.fileExistsMap.end()) {
    return search->second;
  }
  bool res = ESPEASY_FS.exists(patched_fname);
  Cache.fileExistsMap[patched_fname] = res;
  return res;
}

fs::File tryOpenFile(const String& fname, const String& mode) {
  START_TIMER;
  fs::File f;
  if (fname.isEmpty() || fname.equals(F("/"))) {
    return f;
  }

  bool exists = fileExists(fname);

  if (!exists) {
    if (mode == F("r")) {
      return f;
    }
    Cache.fileExistsMap.clear();
  }
  f = ESPEASY_FS.open(patch_fname(fname), mode.c_str());
  STOP_TIMER(TRY_OPEN_FILE);
  return f;
}

bool tryRenameFile(const String& fname_old, const String& fname_new) {
  Cache.fileExistsMap.clear();
  if (fileExists(fname_old) && !fileExists(fname_new)) {
    clearAllCaches();
    return ESPEASY_FS.rename(patch_fname(fname_old), patch_fname(fname_new));
  }
  return false;
}

bool tryDeleteFile(const String& fname) {
  if (fname.length() > 0)
  {
    bool res = ESPEASY_FS.remove(patch_fname(fname));
    clearAllCaches();

    // A call to GarbageCollection() will at most erase a single block. (e.g. 8k block size)
    // A deleted file may have covered more than a single block, so try to clear multiple blocks.
    uint8_t retries = 3;

    while (retries > 0 && GarbageCollection()) {
      --retries;
    }
    return res;
  }
  return false;
}

/********************************************************************************************\
   Fix stuff to clear out differences between releases
 \*********************************************************************************************/
String BuildFixes()
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("BuildFixes"));
  #endif
  serialPrintln(F("\nBuild changed!"));

  if (Settings.Build < 145)
  {
    InitFile(SettingsType::SettingsFileEnum::FILE_NOTIFICATION_type);
  }

  if (Settings.Build < 20101)
  {
    serialPrintln(F("Fix reset Pin"));
    Settings.Pin_Reset = -1;
  }

  if (Settings.Build < 20102) {
    // Settings were 'mangled' by using older version
    // Have to patch settings to make sure no bogus data is being used.
    serialPrintln(F("Fix settings with uninitalized data or corrupted by switching between versions"));
    Settings.UseRTOSMultitasking       = false;
    Settings.Pin_Reset                 = -1;
    Settings.SyslogFacility            = DEFAULT_SYSLOG_FACILITY;
    Settings.MQTTUseUnitNameAsClientId_unused = DEFAULT_MQTT_USE_UNITNAME_AS_CLIENTID;
    Settings.StructSize                = sizeof(Settings);
  }

  if (Settings.Build < 20103) {
    Settings.ResetFactoryDefaultPreference = 0;
    Settings.OldRulesEngine(DEFAULT_RULES_OLDENGINE);
  }
  if (Settings.Build < 20105) {
    Settings.I2C_clockSpeed = DEFAULT_I2C_CLOCK_SPEED;
  }
  if (Settings.Build <= 20106) {
    // ClientID is now defined in the controller settings.
    #ifdef USES_MQTT
    controllerIndex_t controller_idx = firstEnabledMQTT_ControllerIndex();
    if (validControllerIndex(controller_idx)) {
      MakeControllerSettings(ControllerSettings);
      if (AllocatedControllerSettings()) {
        LoadControllerSettings(controller_idx, ControllerSettings);

        String clientid;
        if (Settings.MQTTUseUnitNameAsClientId_unused) {
          clientid = F("%sysname%");
          if (Settings.appendUnitToHostname()) {
            clientid += F("_%unit%");
          }
        }
        else {
          clientid  = F("ESPClient_%mac%");
        }
        safe_strncpy(ControllerSettings.ClientID, clientid, sizeof(ControllerSettings.ClientID));

        ControllerSettings.mqtt_uniqueMQTTclientIdReconnect(Settings.uniqueMQTTclientIdReconnect_unused());
        ControllerSettings.mqtt_retainFlag(Settings.MQTTRetainFlag_unused);
        SaveControllerSettings(controller_idx, ControllerSettings);
      }
    }
    #endif // USES_MQTT
  }
  if (Settings.Build < 20107) {
    Settings.WebserverPort = 80;
  }
  if (Settings.Build < 20108) {
    Settings.ETH_Phy_Addr   = DEFAULT_ETH_PHY_ADDR;
    Settings.ETH_Pin_mdc    = DEFAULT_ETH_PIN_MDC;
    Settings.ETH_Pin_mdio   = DEFAULT_ETH_PIN_MDIO;
    Settings.ETH_Pin_power  = DEFAULT_ETH_PIN_POWER;
    Settings.ETH_Phy_Type   = DEFAULT_ETH_PHY_TYPE;
    Settings.ETH_Clock_Mode = DEFAULT_ETH_CLOCK_MODE;
    Settings.NetworkMedium  = DEFAULT_NETWORK_MEDIUM;
  }
  if (Settings.Build < 20109) {
    Settings.SyslogPort = 514;
  }
  if (Settings.Build < 20110) {
    Settings.I2C_clockSpeed_Slow = DEFAULT_I2C_CLOCK_SPEED_SLOW;
    Settings.I2C_Multiplexer_Type = I2C_MULTIPLEXER_NONE;
    Settings.I2C_Multiplexer_Addr = -1;
    for (taskIndex_t x = 0; x < TASKS_MAX; x++) {
      Settings.I2C_Multiplexer_Channel[x] = -1;
    }
    Settings.I2C_Multiplexer_ResetPin = -1;
  }
  if (Settings.Build < 20111) {
    #ifdef ESP32
    constexpr uint8_t maxStatesesp32 = sizeof(Settings.PinBootStates_ESP32) / sizeof(Settings.PinBootStates_ESP32[0]);
    for (uint8_t i = 0; i < maxStatesesp32; ++i) {
      Settings.PinBootStates_ESP32[i] = 0;
    }
    #endif
  }
  if (Settings.Build < 20112) {
    Settings.WiFi_TX_power = 70; // 70 = 17.5dBm. unit: 0.25 dBm
    Settings.WiFi_sensitivity_margin = 3; // Margin in dBm on top of sensitivity.
  }
  if (Settings.Build < 20113) {
    Settings.NumberExtraWiFiScans = 0;
  }
  if (Settings.Build < 20114) {
    // P003_Pulse was always using the pull-up, now it is a setting.
    for (taskIndex_t taskIndex = 0; taskIndex < TASKS_MAX; ++taskIndex) {
      if (Settings.TaskDeviceNumber[taskIndex] == 3) {
        Settings.TaskDevicePin1PullUp[taskIndex] = true;
      }
    }
    // Disable periodical scanning as it does cause lots of strange issues.
    Settings.PeriodicalScanWiFi(false);
  }

  Settings.Build = BUILD;
  return SaveSettings();
}

/********************************************************************************************\
   Mount FS and check config.dat
 \*********************************************************************************************/
void fileSystemCheck()
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("fileSystemCheck"));
  #endif
  addLog(LOG_LEVEL_INFO, F("FS   : Mounting..."));

  if (ESPEASY_FS.begin())
  {
    clearAllCaches();
    #if defined(ESP8266)
    fs::FSInfo fs_info;
    ESPEASY_FS.info(fs_info);

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("FS   : Mount successful, used ");
      log = log + fs_info.usedBytes;
      log = log + F(" bytes of ");
      log = log + fs_info.totalBytes;
      addLog(LOG_LEVEL_INFO, log);
    }

    // Run garbage collection before any file is open.
    uint8_t retries = 3;

    while (retries > 0 && GarbageCollection()) {
      --retries;
    }
    #endif // if defined(ESP8266)

    fs::File f = tryOpenFile(SettingsType::getSettingsFileName(SettingsType::Enum::BasicSettings_Type).c_str(), "r");

    if (!f)
    {
      ResetFactory();
    }

    if (f) { f.close(); }
  }
  else
  {
    String log = F("FS   : Mount failed");
    serialPrintln(log);
    addLog(LOG_LEVEL_ERROR, log);
    ResetFactory();
  }
}

/********************************************************************************************\
   Garbage collection
 \*********************************************************************************************/
bool GarbageCollection() {
  #ifdef CORE_POST_2_6_0

  // Perform garbage collection
  START_TIMER;

  if (ESPEASY_FS.gc()) {
    addLog(LOG_LEVEL_INFO, F("FS   : Success garbage collection"));
    STOP_TIMER(FS_GC_SUCCESS);
    return true;
  }
  STOP_TIMER(FS_GC_FAIL);
  return false;
  #else // ifdef CORE_POST_2_6_0

  // Not supported, so nothing was removed.
  return false;
  #endif // ifdef CORE_POST_2_6_0
}

/********************************************************************************************\
   Save settings to file system
 \*********************************************************************************************/
String SaveSettings()
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveSettings"));
  #endif
  String     err;
  {
    Settings.StructSize = sizeof(Settings);

    // FIXME @TD-er: As discussed in #1292, the CRC for the settings is now disabled.

    /*
      MD5Builder md5;
      uint8_t    tmp_md5[16] = { 0 };
      memcpy( Settings.ProgmemMd5, CRCValues.runTimeMD5, 16);
      md5.begin();
      md5.add((uint8_t *)&Settings, sizeof(Settings)-16);
      md5.calculate();
      md5.getBytes(tmp_md5);
      if (memcmp(tmp_md5, Settings.md5, 16) != 0) {
        // Settings have changed, save to file.
        memcpy(Settings.md5, tmp_md5, 16);
    */
    Settings.validate();
    err = SaveToFile(SettingsType::getSettingsFileName(SettingsType::Enum::BasicSettings_Type).c_str(), 0, (uint8_t *)&Settings, sizeof(Settings));
  }

  if (err.length()) {
    return err;
  }

  // Must check this after saving, or else it is not possible to fix multiple
  // issues which can only corrected on different pages.
  if (!SettingsCheck(err)) { return err; }

  //  }

  err = SaveSecuritySettings();
  return err;
}

String SaveSecuritySettings() {
  MD5Builder md5;
  uint8_t    tmp_md5[16] = { 0 };
  String     err;

  SecuritySettings.validate();
  memcpy(SecuritySettings.ProgmemMd5, CRCValues.runTimeMD5, 16);
  md5.begin();
  md5.add((uint8_t *)&SecuritySettings, sizeof(SecuritySettings) - 16);
  md5.calculate();
  md5.getBytes(tmp_md5);

  if (memcmp(tmp_md5, SecuritySettings.md5, 16) != 0) {
    // Settings have changed, save to file.
    memcpy(SecuritySettings.md5, tmp_md5, 16);
    err = SaveToFile(getFileName(FileType::SECURITY_DAT).c_str(), 0, (uint8_t *)&SecuritySettings, sizeof(SecuritySettings));

    if (WifiIsAP(WiFi.getMode())) {
      // Security settings are saved, may be update of WiFi settings or hostname.
      WiFiEventData.wifiSetupConnect         = true;
      WiFiEventData.wifiConnectAttemptNeeded = true;
    }
  }
  ExtendedControllerCredentials.save();
  afterloadSettings();
  return err;
}

void afterloadSettings() {
  ExtraTaskSettings.clear(); // make sure these will not contain old settings.

  // Load ResetFactoryDefaultPreference from provisioning.dat if available.
  uint32_t pref_temp = Settings.ResetFactoryDefaultPreference;
  #ifdef USE_CUSTOM_PROVISIONING
  if (fileExists(getFileName(FileType::PROVISIONING_DAT))) {
    MakeProvisioningSettings(ProvisioningSettings);
    if (AllocatedProvisioningSettings()) {
      loadProvisioningSettings(ProvisioningSettings);
      if (ProvisioningSettings.matchingFlashSize()) {
        pref_temp = ProvisioningSettings.ResetFactoryDefaultPreference.getPreference();
      }
    }
  }
  #endif

  // TODO TD-er: Try to get the information from more locations to make it more persistent
  // Maybe EEPROM location?


  ResetFactoryDefaultPreference_struct pref(pref_temp);
  if (modelMatchingFlashSize(pref.getDeviceModel())) {
    ResetFactoryDefaultPreference = pref_temp;
  }
  Scheduler.setEcoMode(Settings.EcoPowerMode());

  if (!Settings.UseRules) {
    eventQueue.clear();
  }
  CheckRunningServices(); // To update changes in hostname.
}

/********************************************************************************************\
   Load settings from file system
 \*********************************************************************************************/
String LoadSettings()
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadSettings"));
  #endif
  String  err;
  uint8_t calculatedMd5[16];
  MD5Builder md5;

  err = LoadFromFile(SettingsType::getSettingsFileName(SettingsType::Enum::BasicSettings_Type).c_str(), 0, (uint8_t *)&Settings, sizeof(SettingsStruct));

  if (err.length()) {
    return err;
  }
  Settings.validate();

  // FIXME @TD-er: As discussed in #1292, the CRC for the settings is now disabled.

  /*
     if (Settings.StructSize > 16) {
      md5.begin();
      md5.add((uint8_t *)&Settings, Settings.StructSize -16);
      md5.calculate();
      md5.getBytes(calculatedMd5);
     }
     if (memcmp (calculatedMd5, Settings.md5,16)==0){
      addLog(LOG_LEVEL_INFO,  F("CRC  : Settings CRC           ...OK"));
      if (memcmp(Settings.ProgmemMd5, CRCValues.runTimeMD5, 16)!=0)
        addLog(LOG_LEVEL_INFO, F("CRC  : binary has changed since last save of Settings"));
     }
     else{
      addLog(LOG_LEVEL_ERROR, F("CRC  : Settings CRC           ...FAIL"));
     }
   */

  err = LoadFromFile(getFileName(FileType::SECURITY_DAT).c_str(), 0, (uint8_t *)&SecuritySettings, sizeof(SecurityStruct));
  md5.begin();
  md5.add((uint8_t *)&SecuritySettings, sizeof(SecuritySettings) - 16);
  md5.calculate();
  md5.getBytes(calculatedMd5);

  if (memcmp(calculatedMd5, SecuritySettings.md5, 16) == 0) {
    addLog(LOG_LEVEL_INFO, F("CRC  : SecuritySettings CRC   ...OK "));

    if (memcmp(SecuritySettings.ProgmemMd5, CRCValues.runTimeMD5, 16) != 0) {
      addLog(LOG_LEVEL_INFO, F("CRC  : binary has changed since last save of Settings"));
    }
  }
  else {
    addLog(LOG_LEVEL_ERROR, F("CRC  : SecuritySettings CRC   ...FAIL"));
  }

  ExtendedControllerCredentials.load();

  //  setupStaticIPconfig();
  // FIXME TD-er: Must check if static/dynamic IP was changed and trigger a reconnect? Or is a reboot better when changing those settings?
  afterloadSettings();
  SecuritySettings.validate();
  return err;
}



/********************************************************************************************\
   Disable Plugin, based on bootFailedCount
 \*********************************************************************************************/
uint8_t disablePlugin(uint8_t bootFailedCount) {
  for (taskIndex_t i = 0; i < TASKS_MAX && bootFailedCount > 0; ++i) {
    if (Settings.TaskDeviceEnabled[i]) {
      --bootFailedCount;

      if (bootFailedCount == 0) {
        Settings.TaskDeviceEnabled[i] = false;
      }
    }
  }
  return bootFailedCount;
}

/********************************************************************************************\
   Disable Controller, based on bootFailedCount
 \*********************************************************************************************/
uint8_t disableController(uint8_t bootFailedCount) {
  for (controllerIndex_t i = 0; i < CONTROLLER_MAX && bootFailedCount > 0; ++i) {
    if (Settings.ControllerEnabled[i]) {
      --bootFailedCount;

      if (bootFailedCount == 0) {
        Settings.ControllerEnabled[i] = false;
      }
    }
  }
  return bootFailedCount;
}

/********************************************************************************************\
   Disable Notification, based on bootFailedCount
 \*********************************************************************************************/
uint8_t disableNotification(uint8_t bootFailedCount) {
  for (uint8_t i = 0; i < NOTIFICATION_MAX && bootFailedCount > 0; ++i) {
    if (Settings.NotificationEnabled[i]) {
      --bootFailedCount;

      if (bootFailedCount == 0) {
        Settings.NotificationEnabled[i] = false;
      }
    }
  }
  return bootFailedCount;
}


bool getAndLogSettingsParameters(bool read, SettingsType::Enum settingsType, int index, int& offset, int& max_size) {
#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
    String log = read ? F("Read") : F("Write");
    log += F(" settings: ");
    log += SettingsType::getSettingsTypeString(settingsType);
    log += F(" index: ");
    log += index;
    addLog(LOG_LEVEL_DEBUG_DEV, log);
  }
#endif // ifndef BUILD_NO_DEBUG
  return SettingsType::getSettingsParameters(settingsType, index, offset, max_size);
}


/********************************************************************************************\
   Load array of Strings from Custom settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String LoadStringArray(SettingsType::Enum settingsType, int index, String strings[], uint16_t nrStrings, uint16_t maxStringLength)
{
  int offset, max_size;
  if (!SettingsType::getSettingsParameters(settingsType, index, offset, max_size))
  {
    #ifndef BUILD_NO_DEBUG
    return F("Invalid index for custom settings");
    #else
    return F("Save error");
    #endif
  }

  const uint32_t bufferSize = 128;

  // FIXME TD-er: For now stack allocated, may need to be heap allocated?
  if (maxStringLength >= bufferSize) { return F("Max 128 chars allowed"); }

  char buffer[bufferSize];

  String   result;
  uint32_t readPos       = 0;
  uint32_t nextStringPos = 0;
  uint32_t stringCount   = 0;
  String   tmpString;
  tmpString.reserve(bufferSize);

  {
    while (stringCount < nrStrings && static_cast<int>(readPos) < max_size) {
      const uint32_t readSize = std::min(bufferSize, max_size - readPos);
      result += LoadFromFile(settingsType,
                            index,
                            (uint8_t *)&buffer,
                            readSize,
                            readPos);

      for (uint32_t i = 0; i < readSize && stringCount < nrStrings; ++i) {
        const uint32_t curPos = readPos + i;

        if (curPos >= nextStringPos) {
          if (buffer[i] == 0) {
            if (maxStringLength != 0) {
              // Specific string length, so we have to set the next string position.
              nextStringPos += maxStringLength;
            }
            strings[stringCount] = tmpString;
            tmpString            = "";
            tmpString.reserve(readSize);
            ++stringCount;
          } else {
            tmpString += buffer[i];
          }
        }
      }
      readPos += bufferSize;
    }
  }

  if ((!tmpString.isEmpty()) && (stringCount < nrStrings)) {
    result              += F("Incomplete custom settings for index ");
    result              += (index + 1);
    strings[stringCount] = tmpString;
  }
  return result;
}

/********************************************************************************************\
   Save array of Strings from Custom settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String SaveStringArray(SettingsType::Enum settingsType, int index, const String strings[], uint16_t nrStrings, uint16_t maxStringLength)
{
  int offset, max_size;
  if (!SettingsType::getSettingsParameters(settingsType, index, offset, max_size))
  {
    #ifndef BUILD_NO_DEBUG
    return F("Invalid index for custom settings");
    #else
    return F("Save error");
    #endif
  }

  const uint16_t bufferSize = 128;

  // FIXME TD-er: For now stack allocated, may need to be heap allocated?
  uint8_t buffer[bufferSize];

  String   result;
  int      writePos        = 0;
  uint16_t stringCount     = 0;
  uint16_t stringReadPos   = 0;
  uint16_t nextStringPos   = 0;
  uint16_t curStringLength = 0;

  if (maxStringLength != 0) {
    // Specified string length, check given strings
    for (int i = 0; i < nrStrings; ++i) {
      if (strings[i].length() >= maxStringLength) {
        result += getCustomTaskSettingsError(i);
      }
    }
  }

  while (stringCount < nrStrings && writePos < max_size) {
    ZERO_FILL(buffer);

    for (int i = 0; i < bufferSize && stringCount < nrStrings; ++i) {
      if (stringReadPos == 0) {
        // We're at the start of a string
        curStringLength = strings[stringCount].length();

        if (maxStringLength != 0) {
          if (curStringLength >= maxStringLength) {
            curStringLength = maxStringLength - 1;
          }
        }
      }

      uint16_t curPos = writePos + i;

      if (curPos >= nextStringPos) {
        if (stringReadPos < curStringLength) {
          buffer[i] = strings[stringCount][stringReadPos];
          ++stringReadPos;
        } else {
          buffer[i]     = 0;
          stringReadPos = 0;
          ++stringCount;

          if (maxStringLength == 0) {
            nextStringPos += curStringLength + 1;
          } else {
            nextStringPos += maxStringLength;
          }
        }
      }
    }

    // Buffer is filled, now write to flash
    // As we write in parts, only count as single write.
    if (RTC.flashDayCounter > 0) {
      RTC.flashDayCounter--;
    }
    result   += SaveToFile(settingsType, index, &(buffer[0]), bufferSize, writePos);
    writePos += bufferSize;
  }

  if ((writePos >= max_size) && (stringCount < nrStrings)) {
    result += F("Error: Not all strings fit in custom settings.");
  }
  return result;
}



/********************************************************************************************\
   Save Task settings to file system
 \*********************************************************************************************/
String SaveTaskSettings(taskIndex_t TaskIndex)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveTaskSettings"));
  #endif

  if (ExtraTaskSettings.TaskIndex != TaskIndex) {
    #ifndef BUILD_NO_DEBUG
    return F("SaveTaskSettings taskIndex does not match");
    #else
    return F("Save error");
    #endif
  }
  String err = SaveToFile(SettingsType::Enum::TaskSettings_Type,
                          TaskIndex,
                          (uint8_t *)&ExtraTaskSettings,
                          sizeof(struct ExtraTaskSettingsStruct));

  if (err.isEmpty()) {
    err = checkTaskSettings(TaskIndex);
  }
  return err;
}

/********************************************************************************************\
   Load Task settings from file system
 \*********************************************************************************************/
String LoadTaskSettings(taskIndex_t TaskIndex)
{
  if (ExtraTaskSettings.TaskIndex == TaskIndex) {
    return String(); // already loaded
  }
  if (!validTaskIndex(TaskIndex)) {
    ExtraTaskSettings.clear();
    return String(); // Un-initialized task index.
  }
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadTaskSettings"));
  #endif

  START_TIMER
  ExtraTaskSettings.clear();
  const String result = LoadFromFile(SettingsType::Enum::TaskSettings_Type, TaskIndex, (uint8_t *)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct));

  // After loading, some settings may need patching.
  ExtraTaskSettings.TaskIndex = TaskIndex; // Needed when an empty task was requested

  const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(TaskIndex);
  if (validDeviceIndex(DeviceIndex)) {
    if (!Device[DeviceIndex].configurableDecimals()) {
      // Nr of decimals cannot be configured, so set them to 0 just to be sure.
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0;
      }      
    }
  }

  if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0) {
    // if field set empty, reload defaults
    struct EventStruct TempEvent(TaskIndex);
    String tmp;

    // the plugin call should populate ExtraTaskSettings with its default values.
    PluginCall(PLUGIN_GET_DEVICEVALUENAMES, &TempEvent, tmp);
  }
  ExtraTaskSettings.validate();
  STOP_TIMER(LOAD_TASK_SETTINGS);

  return result;
}

/********************************************************************************************\
   Save Custom Task settings to file system
 \*********************************************************************************************/
String SaveCustomTaskSettings(taskIndex_t TaskIndex, uint8_t *memAddress, int datasize)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveCustomTaskSettings"));
  #endif
  return SaveToFile(SettingsType::Enum::CustomTaskSettings_Type, TaskIndex, memAddress, datasize);
}

/********************************************************************************************\
   Save array of Strings to Custom Task settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String SaveCustomTaskSettings(taskIndex_t TaskIndex, String strings[], uint16_t nrStrings, uint16_t maxStringLength)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveCustomTaskSettings"));
  #endif
  return SaveStringArray(
    SettingsType::Enum::CustomTaskSettings_Type, TaskIndex,
    strings, nrStrings, maxStringLength);
}

String getCustomTaskSettingsError(uint8_t varNr) {
  String error = F("Error: Text too long for line ");

  error += varNr + 1;
  error += '\n';
  return error;
}

/********************************************************************************************\
   Clear custom task settings
 \*********************************************************************************************/
String ClearCustomTaskSettings(taskIndex_t TaskIndex)
{
  // addLog(LOG_LEVEL_DEBUG, F("Clearing custom task settings"));
  return ClearInFile(SettingsType::Enum::CustomTaskSettings_Type, TaskIndex);
}

/********************************************************************************************\
   Load Custom Task settings from file system
 \*********************************************************************************************/
String LoadCustomTaskSettings(taskIndex_t TaskIndex, uint8_t *memAddress, int datasize)
{
  START_TIMER;
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadCustomTaskSettings"));
  #endif
  String result = LoadFromFile(SettingsType::Enum::CustomTaskSettings_Type, TaskIndex, memAddress, datasize);
  STOP_TIMER(LOAD_CUSTOM_TASK_STATS);
  return result;
}

/********************************************************************************************\
   Load array of Strings from Custom Task settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String LoadCustomTaskSettings(taskIndex_t TaskIndex, String strings[], uint16_t nrStrings, uint16_t maxStringLength)
{
  START_TIMER;
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadCustomTaskSettings"));
  #endif
  String result = LoadStringArray(SettingsType::Enum::CustomTaskSettings_Type,
                           TaskIndex,
                           strings, nrStrings, maxStringLength);
  STOP_TIMER(LOAD_CUSTOM_TASK_STATS);
  return result;
}

/********************************************************************************************\
   Save Controller settings to file system
 \*********************************************************************************************/
String SaveControllerSettings(controllerIndex_t ControllerIndex, ControllerSettingsStruct& controller_settings)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveControllerSettings"));
  #endif
  controller_settings.validate(); // Make sure the saved controller settings have proper values.
  return SaveToFile(SettingsType::Enum::ControllerSettings_Type, ControllerIndex,
                    (uint8_t *)&controller_settings, sizeof(controller_settings));
}

/********************************************************************************************\
   Load Controller settings to file system
 \*********************************************************************************************/
String LoadControllerSettings(controllerIndex_t ControllerIndex, ControllerSettingsStruct& controller_settings) {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadControllerSettings"));
  #endif
  String result =
    LoadFromFile(SettingsType::Enum::ControllerSettings_Type, ControllerIndex,
                 (uint8_t *)&controller_settings, sizeof(controller_settings));
  controller_settings.validate(); // Make sure the loaded controller settings have proper values.
  return result;
}

/********************************************************************************************\
   Clear Custom Controller settings
 \*********************************************************************************************/
String ClearCustomControllerSettings(controllerIndex_t ControllerIndex)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("ClearCustomControllerSettings"));
  #endif

  // addLog(LOG_LEVEL_DEBUG, F("Clearing custom controller settings"));
  return ClearInFile(SettingsType::Enum::CustomControllerSettings_Type, ControllerIndex);
}

/********************************************************************************************\
   Save Custom Controller settings to file system
 \*********************************************************************************************/
String SaveCustomControllerSettings(controllerIndex_t ControllerIndex, uint8_t *memAddress, int datasize)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveCustomControllerSettings"));
  #endif
  return SaveToFile(SettingsType::Enum::CustomControllerSettings_Type, ControllerIndex, memAddress, datasize);
}

/********************************************************************************************\
   Load Custom Controller settings to file system
 \*********************************************************************************************/
String LoadCustomControllerSettings(controllerIndex_t ControllerIndex, uint8_t *memAddress, int datasize)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadCustomControllerSettings"));
  #endif
  return LoadFromFile(SettingsType::Enum::CustomControllerSettings_Type, ControllerIndex, memAddress, datasize);
}


#ifdef USE_CUSTOM_PROVISIONING
/********************************************************************************************\
   Save Provisioning Settings
 \*********************************************************************************************/
String saveProvisioningSettings(ProvisioningStruct& ProvisioningSettings)
{
  MD5Builder md5;
  uint8_t    tmp_md5[16] = { 0 };
  String     err;

  ProvisioningSettings.validate();
  memcpy(ProvisioningSettings.ProgmemMd5, CRCValues.runTimeMD5, 16);
  md5.begin();
  md5.add((uint8_t *)&ProvisioningSettings + 16, sizeof(ProvisioningSettings) - 16);
  md5.calculate();
  md5.getBytes(tmp_md5);

  if (memcmp(tmp_md5, ProvisioningSettings.md5, 16) != 0) {
    // Settings have changed, save to file.
    memcpy(ProvisioningSettings.md5, tmp_md5, 16);
    err = SaveToFile_trunc(getFileName(FileType::PROVISIONING_DAT).c_str(), 0, (uint8_t *)&ProvisioningSettings, sizeof(ProvisioningStruct));
  }
  return err;
}

/********************************************************************************************\
   Load Provisioning Settings
 \*********************************************************************************************/
String loadProvisioningSettings(ProvisioningStruct& ProvisioningSettings)
{
  uint8_t calculatedMd5[16] = { 0 };
  MD5Builder md5;

  String err = LoadFromFile(getFileName(FileType::PROVISIONING_DAT).c_str(), 0, (uint8_t *)&ProvisioningSettings, sizeof(ProvisioningStruct));
  md5.begin();
  md5.add(((uint8_t *)&ProvisioningSettings) + 16, sizeof(ProvisioningSettings) - 16);
  md5.calculate();
  md5.getBytes(calculatedMd5);

  if (memcmp(calculatedMd5, ProvisioningSettings.md5, 16) == 0) {
    addLog(LOG_LEVEL_INFO, F("CRC  : ProvisioningSettings CRC   ...OK "));

    if (memcmp(ProvisioningSettings.ProgmemMd5, CRCValues.runTimeMD5, 16) != 0) {
      addLog(LOG_LEVEL_INFO, F("CRC  : binary has changed since last save of Settings"));
    }
  }
  else {
    addLog(LOG_LEVEL_ERROR, F("CRC  : ProvisioningSettings CRC   ...FAIL"));
  }
  ProvisioningSettings.validate();
  return err;
}

#endif

/********************************************************************************************\
   Save Controller settings to file system
 \*********************************************************************************************/
String SaveNotificationSettings(int NotificationIndex, uint8_t *memAddress, int datasize)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveNotificationSettings"));
  #endif
  return SaveToFile(SettingsType::Enum::NotificationSettings_Type, NotificationIndex, memAddress, datasize);
}

/********************************************************************************************\
   Load Controller settings to file system
 \*********************************************************************************************/
String LoadNotificationSettings(int NotificationIndex, uint8_t *memAddress, int datasize)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadNotificationSettings"));
  #endif
  return LoadFromFile(SettingsType::Enum::NotificationSettings_Type, NotificationIndex, memAddress, datasize);
}

/********************************************************************************************\
   Init a file with zeros on file system
 \*********************************************************************************************/
String InitFile(const String& fname, int datasize)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("InitFile"));
  #endif
  FLASH_GUARD();

  fs::File f = tryOpenFile(fname, "w");

  if (f) {
    for (int x = 0; x < datasize; x++)
    {
      // See https://github.com/esp8266/Arduino/commit/b1da9eda467cc935307d553692fdde2e670db258#r32622483
      uint8_t zero_value = 0;
      SPIFFS_CHECK(f.write(&zero_value, 1), fname.c_str());
    }
    f.close();
  }

  // OK
  return String();
}

String InitFile(SettingsType::Enum settingsType)
{
  return InitFile(SettingsType::getSettingsFile(settingsType));
}

String InitFile(SettingsType::SettingsFileEnum file_type)
{
  return InitFile(SettingsType::getSettingsFileName(file_type), 
                  SettingsType::getInitFileSize(file_type));
}

/********************************************************************************************\
   Save data into config file on file system
 \*********************************************************************************************/
String SaveToFile(const char *fname, int index, const uint8_t *memAddress, int datasize)
{
  return doSaveToFile(fname, index, memAddress, datasize, "r+");
}

String SaveToFile_trunc(const char *fname, int index, const uint8_t *memAddress, int datasize)
{
  return doSaveToFile(fname, index, memAddress, datasize, "w+");
}

// See for mode description: https://github.com/esp8266/Arduino/blob/master/doc/filesystem.rst
String doSaveToFile(const char *fname, int index, const uint8_t *memAddress, int datasize, const char *mode)
{
#ifndef BUILD_NO_DEBUG
#ifndef ESP32

  if (allocatedOnStack(memAddress)) {
    String log = F("SaveToFile: ");
    log += fname;
    log += F(" ERROR, Data allocated on stack");
    addLog(LOG_LEVEL_ERROR, log);

    //    return log;  // FIXME TD-er: Should this be considered a breaking error?
  }
#endif // ifndef ESP32
#endif

  if (index < 0) {
    #ifndef BUILD_NO_DEBUG
    String log = F("SaveToFile: ");
    log += fname;
    log += F(" ERROR, invalid position in file");
    #else
    String log = F("Save error");
    #endif
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }
  START_TIMER;
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SaveToFile"));
  #endif
  FLASH_GUARD();
  
  #ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("SaveToFile: free stack: ");
    log += getCurrentFreeStack();
    addLog(LOG_LEVEL_INFO, log);
  }
  #endif
  delay(1);
  unsigned long timer = millis() + 50;
  fs::File f          = tryOpenFile(fname, mode);

  if (f) {
    clearAllCaches();
    SPIFFS_CHECK(f,                          fname);
    SPIFFS_CHECK(f.seek(index, fs::SeekSet), fname);
    const uint8_t *pointerToByteToSave = memAddress;

    for (int x = 0; x < datasize; x++)
    {
      // See https://github.com/esp8266/Arduino/commit/b1da9eda467cc935307d553692fdde2e670db258#r32622483
      uint8_t byteToSave = *pointerToByteToSave;
      SPIFFS_CHECK(f.write(&byteToSave, 1), fname);
      pointerToByteToSave++;

      if (x % 256 == 0) {
        // one page written, do some background tasks
        timer = millis() + 50;
        delay(0);
      }

      if (timeOutReached(timer)) {
        timer += 50;
        delay(0);
      }
    }
    f.close();
    #ifndef BUILD_NO_DEBUG
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      log.reserve(48);
      log += F("FILE : Saved ");
      log += fname;
      log += F(" offset: ");
      log += index;
      log += F(" size: ");
      log += datasize;
      addLog(LOG_LEVEL_INFO, log);
    }
    #endif
  } else {
    #ifndef BUILD_NO_DEBUG
    String log = F("SaveToFile: ");
    log += fname;
    log += F(" ERROR, Cannot save to file");
    #else
    String log = F("Save error");
    #endif

    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }
  STOP_TIMER(SAVEFILE_STATS);
  #ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("SaveToFile: free stack after: ");
    log += getCurrentFreeStack();
    addLog(LOG_LEVEL_INFO, log);
  }
  #endif

  // OK
  return String();
}

/********************************************************************************************\
   Clear a certain area in a file (set to 0)
 \*********************************************************************************************/
String ClearInFile(const char *fname, int index, int datasize)
{
  if (index < 0) {
    #ifndef BUILD_NO_DEBUG
    String log = F("ClearInFile: ");
    log += fname;
    log += F(" ERROR, invalid position in file");
    #else
    String log = F("Save error");
    #endif

    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }

  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("ClearInFile"));
  #endif
  FLASH_GUARD();

  fs::File f = tryOpenFile(fname, "r+");

  if (f) {
    SPIFFS_CHECK(f.seek(index, fs::SeekSet), fname);

    for (int x = 0; x < datasize; x++)
    {
      // See https://github.com/esp8266/Arduino/commit/b1da9eda467cc935307d553692fdde2e670db258#r32622483
      uint8_t zero_value = 0;
      SPIFFS_CHECK(f.write(&zero_value, 1), fname);
    }
    f.close();
  } else {
    #ifndef BUILD_NO_DEBUG
    String log = F("ClearInFile: ");
    log += fname;
    log += F(" ERROR, Cannot save to file");
    #else
    String log = F("Save error");
    #endif
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }

  // OK
  return String();
}

/********************************************************************************************\
   Load data from config file on file system
 \*********************************************************************************************/
String LoadFromFile(const char *fname, int offset, uint8_t *memAddress, int datasize)
{
  if (offset < 0) {
    #ifndef BUILD_NO_DEBUG
    String log = F("LoadFromFile: ");
    log += fname;
    log += F(" ERROR, invalid position in file");
    #else
    String log = F("Load error");
    #endif
    addLog(LOG_LEVEL_ERROR, log);
    return log;
  }
  delay(0);
  START_TIMER;
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("LoadFromFile"));
  #endif
  fs::File f = tryOpenFile(fname, "r");
  SPIFFS_CHECK(f,                            fname);
  SPIFFS_CHECK(f.seek(offset, fs::SeekSet),  fname);
  SPIFFS_CHECK(f.read(memAddress, datasize), fname);
  f.close();

  STOP_TIMER(LOADFILE_STATS);
  delay(0);

  return String();
}

/********************************************************************************************\
   Wrapper functions to handle errors in accessing settings
 \*********************************************************************************************/
String getSettingsFileIndexRangeError(bool read, SettingsType::Enum settingsType, int index) {
  if (settingsType >= SettingsType::Enum::SettingsType_MAX) {
    String error = F("Unknown settingsType: ");
    error += static_cast<int>(settingsType);
    return error;
  }
  String error = read ? F("Load") : F("Save");
  #ifndef BUILD_NO_DEBUG
  error += SettingsType::getSettingsTypeString(settingsType);
  error += F(" index out of range: ");
  error += index;
  #else
  error += F(" error");
  #endif
  return error;
}

String getSettingsFileDatasizeError(bool read, SettingsType::Enum settingsType, int index, int datasize, int max_size) {
  String error = read ? F("Load") : F("Save");
  #ifndef BUILD_NO_DEBUG
  error += SettingsType::getSettingsTypeString(settingsType);
  error += '(';
  error += index;
  error += F(") datasize(");
  error += datasize;
  error += F(") > max_size(");
  error += max_size;
  error += ')';
  #else
  error += F(" error");
  #endif
  
  return error;
}

String LoadFromFile(SettingsType::Enum settingsType, int index, uint8_t *memAddress, int datasize, int offset_in_block) {
  bool read = true;
  int  offset, max_size;

  if (!getAndLogSettingsParameters(read, settingsType, index, offset, max_size)) {
    return getSettingsFileIndexRangeError(read, settingsType, index);
  }

  if ((datasize + offset_in_block) > max_size) {
    return getSettingsFileDatasizeError(read, settingsType, index, datasize, max_size);
  }
  String fname = SettingsType::getSettingsFileName(settingsType);
  return LoadFromFile(fname.c_str(), (offset + offset_in_block), memAddress, datasize);
}

String LoadFromFile(SettingsType::Enum settingsType, int index, uint8_t *memAddress, int datasize) {
  return LoadFromFile(settingsType, index, memAddress, datasize, 0);
}

String SaveToFile(SettingsType::Enum settingsType, int index, uint8_t *memAddress, int datasize) {
  return SaveToFile(settingsType, index, memAddress, datasize, 0);
}

String SaveToFile(SettingsType::Enum settingsType, int index, uint8_t *memAddress, int datasize, int posInBlock) {
  bool read = false;
  int  offset, max_size;

  if (!getAndLogSettingsParameters(read, settingsType, index, offset, max_size)) {
    return getSettingsFileIndexRangeError(read, settingsType, index);
  }

  if ((datasize > max_size) || ((posInBlock + datasize) > max_size)) {
    return getSettingsFileDatasizeError(read, settingsType, index, datasize, max_size);
  }
  String fname = SettingsType::getSettingsFileName(settingsType);
  if (!fileExists(fname)) {
    InitFile(settingsType);
  }
  return SaveToFile(fname.c_str(), offset + posInBlock, memAddress, datasize);
}

String ClearInFile(SettingsType::Enum settingsType, int index) {
  bool read = false;
  int  offset, max_size;

  if (!getAndLogSettingsParameters(read, settingsType, index, offset, max_size)) {
    return getSettingsFileIndexRangeError(read, settingsType, index);
  }
  String fname = SettingsType::getSettingsFileName(settingsType);
  return ClearInFile(fname.c_str(), offset, max_size);
}

/********************************************************************************************\
   Check file system area settings
 \*********************************************************************************************/
int SpiffsSectors()
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("SpiffsSectors"));
  #endif
  #if defined(ESP8266)
    # ifdef CORE_POST_2_6_0
  uint32_t _sectorStart = ((uint32_t)&_FS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint32_t _sectorEnd   = ((uint32_t)&_FS_end - 0x40200000) / SPI_FLASH_SEC_SIZE;
    # else // ifdef CORE_POST_2_6_0
  uint32_t _sectorStart = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint32_t _sectorEnd   = ((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE;
    # endif // ifdef CORE_POST_2_6_0

  return _sectorEnd - _sectorStart;
  #endif // if defined(ESP8266)
  #if defined(ESP32)
  return 32;
  #endif // if defined(ESP32)
}

size_t SpiffsUsedBytes() {
  size_t result = 1; // Do not output 0, this may be used in divisions.

  #ifdef ESP32
  result = ESPEASY_FS.usedBytes();
  #endif // ifdef ESP32
  #ifdef ESP8266
  fs::FSInfo fs_info;
  ESPEASY_FS.info(fs_info);
  result = fs_info.usedBytes;
  #endif // ifdef ESP8266
  return result;
}

size_t SpiffsTotalBytes() {
  size_t result = 1; // Do not output 0, this may be used in divisions.

  #ifdef ESP32
  result = ESPEASY_FS.totalBytes();
  #endif // ifdef ESP32
  #ifdef ESP8266
  fs::FSInfo fs_info;
  ESPEASY_FS.info(fs_info);
  result = fs_info.totalBytes;
  #endif // ifdef ESP8266
  return result;
}

size_t SpiffsBlocksize() {
  size_t result = 8192; // Some default viable for most 1 MB file systems

  #ifdef ESP32
  result = 8192;        // Just assume 8k, since we cannot query it
  #endif // ifdef ESP32
  #ifdef ESP8266
  fs::FSInfo fs_info;
  ESPEASY_FS.info(fs_info);
  result = fs_info.blockSize;
  #endif // ifdef ESP8266
  return result;
}

size_t SpiffsPagesize() {
  size_t result = 256; // Most common

  #ifdef ESP32
  result = 256;        // Just assume 256, since we cannot query it
  #endif // ifdef ESP32
  #ifdef ESP8266
  fs::FSInfo fs_info;
  ESPEASY_FS.info(fs_info);
  result = fs_info.pageSize;
  #endif // ifdef ESP8266
  return result;
}

size_t SpiffsFreeSpace() {
  int freeSpace = SpiffsTotalBytes() - SpiffsUsedBytes();

  if (freeSpace < static_cast<int>(2 * SpiffsBlocksize())) {
    // Not enough free space left to store anything
    // There needs to be minimum of 2 free blocks.
    return 0;
  }
  return freeSpace - 2 * SpiffsBlocksize();
}

bool SpiffsFull() {
  return SpiffsFreeSpace() == 0;
}

/********************************************************************************************\
   Handling cached data
 \*********************************************************************************************/
String createCacheFilename(unsigned int count) {
  String fname;

  fname.reserve(16);
  #ifdef ESP32
  fname = '/';
  #endif // ifdef ESP32
  fname += F("cache_");
  fname += String(count);
  fname += F(".bin");
  return fname;
}

// Match string with an integer between '_' and ".bin"
int getCacheFileCountFromFilename(const String& fname) {
  int startpos = fname.indexOf('_');

  if (startpos < 0) { return -1; }
  int endpos = fname.indexOf(F(".bin"));

  if (endpos < 0) { return -1; }

  //  String digits = fname.substring(startpos + 1, endpos);
  int result;

  if (validIntFromString(fname.substring(startpos + 1, endpos), result)) {
    return result;
  }
  return -1;
}

// Look into the filesystem to see if there are any cache files present on the filesystem
// Return true if any found.
bool getCacheFileCounters(uint16_t& lowest, uint16_t& highest, size_t& filesizeHighest) {
  lowest          = 65535;
  highest         = 0;
  filesizeHighest = 0;
#ifdef ESP8266
  Dir dir = ESPEASY_FS.openDir(F("cache"));

  while (dir.next()) {
    String filename = dir.fileName();
    int    count    = getCacheFileCountFromFilename(filename);

    if (count >= 0) {
      if (lowest > count) {
        lowest = count;
      }

      if (highest < count) {
        highest         = count;
        filesizeHighest = dir.fileSize();
      }
    }
  }
#endif // ESP8266
#ifdef ESP32
  File root = ESPEASY_FS.open(F("/"));
  File file = root.openNextFile();

  while (file)
  {
    if (!file.isDirectory()) {
      const String fname(file.name());
      if (fname.startsWith(F("/cache")) || fname.startsWith(F("cache"))) {
        int count = getCacheFileCountFromFilename(fname);

        if (count >= 0) {
          if (lowest > count) {
            lowest = count;
          }

          if (highest < count) {
            highest         = count;
            filesizeHighest = file.size();
          }
        } else {
          addLog(LOG_LEVEL_INFO, String(F("RTC  : Cannot get count from: ")) + fname);
        }
      }
    }
    file = root.openNextFile();
  }
#endif // ESP32

  if (lowest <= highest) {
    return true;
  }
  lowest  = 0;
  highest = 0;
  return false;
}

/********************************************************************************************\
   Get partition table information
 \*********************************************************************************************/
#ifdef ESP32
String getPartitionType(uint8_t pType, uint8_t pSubType) {
  esp_partition_type_t partitionType       = static_cast<esp_partition_type_t>(pType);
  esp_partition_subtype_t partitionSubType = static_cast<esp_partition_subtype_t>(pSubType);

  if (partitionType == ESP_PARTITION_TYPE_APP) {
    if ((partitionSubType >= ESP_PARTITION_SUBTYPE_APP_OTA_MIN) &&
        (partitionSubType < ESP_PARTITION_SUBTYPE_APP_OTA_MAX)) {
      String result = F("OTA partition ");
      result += (partitionSubType - ESP_PARTITION_SUBTYPE_APP_OTA_MIN);
      return result;
    }

    switch (partitionSubType) {
      case ESP_PARTITION_SUBTYPE_APP_FACTORY: return F("Factory app");
      case ESP_PARTITION_SUBTYPE_APP_TEST:    return F("Test app");
      default: break;
    }
  }

  if (partitionType == ESP_PARTITION_TYPE_DATA) {
    switch (partitionSubType) {
      case ESP_PARTITION_SUBTYPE_DATA_OTA:      return F("OTA selection");
      case ESP_PARTITION_SUBTYPE_DATA_PHY:      return F("PHY init data");
      case ESP_PARTITION_SUBTYPE_DATA_NVS:      return F("NVS");
      case ESP_PARTITION_SUBTYPE_DATA_COREDUMP: return F("COREDUMP");
      case ESP_PARTITION_SUBTYPE_DATA_ESPHTTPD: return F("ESPHTTPD");
      case ESP_PARTITION_SUBTYPE_DATA_FAT:      return F("FAT");
      case ESP_PARTITION_SUBTYPE_DATA_SPIFFS:   return F("SPIFFS");
      default: break;
    }
  }
  String result = F("Unknown(");
  result += partitionSubType;
  result += ')';
  return result;
}

String getPartitionTableHeader(const String& itemSep, const String& lineEnd) {
  String result;

  result += F("Address");
  result += itemSep;
  result += F("Size");
  result += itemSep;
  result += F("Label");
  result += itemSep;
  result += F("Partition Type");
  result += itemSep;
  result += F("Encrypted");
  result += lineEnd;
  return result;
}

String getPartitionTable(uint8_t pType, const String& itemSep, const String& lineEnd) {
  esp_partition_type_t partitionType = static_cast<esp_partition_type_t>(pType);
  String result;
  esp_partition_iterator_t _mypartiterator = esp_partition_find(partitionType, ESP_PARTITION_SUBTYPE_ANY, NULL);

  if (_mypartiterator) {
    do {
      const esp_partition_t *_mypart = esp_partition_get(_mypartiterator);
      result += formatToHex(_mypart->address);
      result += itemSep;
      result += formatToHex_decimal(_mypart->size, 1024);
      result += itemSep;
      result += _mypart->label;
      result += itemSep;
      result += getPartitionType(_mypart->type, _mypart->subtype);
      result += itemSep;
      result += (_mypart->encrypted ? F("Yes") : F("-"));
      result += lineEnd;
    } while ((_mypartiterator = esp_partition_next(_mypartiterator)) != NULL);
  }
  esp_partition_iterator_release(_mypartiterator);
  return result;
}

#endif // ifdef ESP32

#ifdef USE_DOWNLOAD
String downloadFileType(const String& url, const String& user, const String& pass, FileType::Enum filetype, unsigned int filenr)
{
  if (!getDownloadFiletypeChecked(filetype, filenr)) {
    // Not selected, so not downloaded
    return F("Not Allowed");
  }

  String filename = getFileName(filetype, filenr);
  String fullUrl;

  fullUrl.reserve(url.length() + filename.length() + 1); // May need to add an extra slash
  fullUrl = url;
  fullUrl = parseTemplate(fullUrl, true);                // URL encode

  // URLEncode may also encode the '/' into "%2f"
  // FIXME TD-er: Can this really occur?
  fullUrl.replace(F("%2f"), F("/"));

  while (filename.startsWith(F("/"))) {
    filename = filename.substring(1);
  }

  if (!fullUrl.endsWith(F("/"))) {
    fullUrl += F("/");
  }
  fullUrl += filename;

  String error;

  if (ResetFactoryDefaultPreference.deleteFirst()) {
    if (fileExists(filename) && !tryDeleteFile(filename)) {
      return F("Could not delete existing file");
    }

    if (!downloadFile(fullUrl, filename, user, pass, error)) {
      return error;
    }
  } else {
    if (fileExists(filename)) {
      String filename_bak = filename;
      filename_bak += F("_bak");
      if (fileExists(filename_bak)) {
        return F("Could not rename to _bak");
      }

      // Must download it to a tmp file.
      String tmpfile = filename;
      tmpfile += F("_tmp");

      if (!downloadFile(fullUrl, tmpfile, user, pass, error)) {
        return error;
      }

      if (fileExists(filename) && !tryRenameFile(filename, filename_bak)) {
        return F("Could not rename to _bak");
      } else {
        // File does not exist (anymore)
        if (!tryRenameFile(tmpfile, filename)) {
          error = F("Could not rename tmp file");

          if (tryRenameFile(filename_bak, filename)) {
            error += F("... reverted");
          } else {
            error += F(" Not reverted!");
          }
          return error;
        }
      }
    } else {
      if (!downloadFile(fullUrl, filename, user, pass, error)) {
        return error;
      }
    }
  }
  return error;
}

#endif // ifdef USE_DOWNLOAD

#ifdef USE_CUSTOM_PROVISIONING

String downloadFileType(FileType::Enum filetype, unsigned int filenr)
{
  if (!ResetFactoryDefaultPreference.allowFetchByCommand()) {
    return F("Not Allowed");
  }
  String url, user, pass;

  {
    MakeProvisioningSettings(ProvisioningSettings);

    if (AllocatedProvisioningSettings()) {
      loadProvisioningSettings(ProvisioningSettings);
      url  = ProvisioningSettings.url;
      user = ProvisioningSettings.user;
      pass = ProvisioningSettings.pass;
    }
  }
  return downloadFileType(url, user, pass, filetype, filenr);
}

#endif // ifdef USE_CUSTOM_PROVISIONING
