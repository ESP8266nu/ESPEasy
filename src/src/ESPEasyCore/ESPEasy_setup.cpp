#include "../ESPEasyCore/ESPEasy_setup.h"

#include "../../ESPEasy_fdwdecl.h" // Needed for PluginInit() and CPluginInit()

#include "../../ESPEasy-Globals.h"
#include "../../_Plugin_Helper.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyRules.h"
#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/Serial.h"
#include "../Globals/Cache.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/NetworkState.h"
#include "../Globals/RTC.h"
#include "../Globals/Statistics.h"
#include "../Globals/WiFi_AP_Candidates.h"
#include "../Helpers/DeepSleep.h"
#include "../Helpers/ESPEasyRTC.h"
#include "../Helpers/ESPEasy_FactoryDefault.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/ESPEasy_checks.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Memory.h"
#include "../Helpers/Misc.h"
#include "../Helpers/StringGenerator_System.h"
#include "../WebServer/WebServer.h"


#ifdef USE_RTOS_MULTITASKING
# include "../Helpers/Networking.h"
# include "../Helpers/PeriodicalActions.h"
#endif // ifdef USE_RTOS_MULTITASKING

#ifdef FEATURE_ARDUINO_OTA
# include "../Helpers/OTA.h"
#endif // ifdef FEATURE_ARDUINO_OTA


#ifdef USE_RTOS_MULTITASKING
void RTOS_TaskServers(void *parameter)
{
  while (true) {
    delay(100);
    web_server.handleClient();
    checkUDP();
  }
}

void RTOS_TaskSerial(void *parameter)
{
  while (true) {
    delay(100);
    serial();
  }
}

void RTOS_Task10ps(void *parameter)
{
  while (true) {
    delay(100);
    run10TimesPerSecond();
  }
}

void RTOS_HandleSchedule(void *parameter)
{
  while (true) {
    Scheduler.handle_schedule();
  }
}

#endif // ifdef USE_RTOS_MULTITASKING


/*********************************************************************************************\
* ISR call back function for handling the watchdog.
\*********************************************************************************************/
void sw_watchdog_callback(void *arg)
{
  yield(); // feed the WD
  ++sw_watchdog_callback_count;
}

/*********************************************************************************************\
* SETUP
\*********************************************************************************************/
void ESPEasy_setup()
{
#ifdef ESP8266_DISABLE_EXTRA4K
  disable_extra4k_at_link_time();
#endif // ifdef ESP8266_DISABLE_EXTRA4K
#ifdef PHASE_LOCKED_WAVEFORM
  enablePhaseLockedWaveform();
#endif // ifdef PHASE_LOCKED_WAVEFORM
#ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
#endif
  initWiFi();

  run_compiletime_checks();
#ifndef BUILD_NO_RAM_TRACKER
  lowestFreeStack = getFreeStackWatermark();
  lowestRAM       = FreeMem();
#endif // ifndef BUILD_NO_RAM_TRACKER
#ifdef ESP8266

  //  ets_isr_attach(8, sw_watchdog_callback, NULL);  // Set a callback for feeding the watchdog.
#endif // ifdef ESP8266


  // Read ADC at boot, before WiFi tries to connect.
  // see https://github.com/letscontrolit/ESPEasy/issues/2646
#if FEATURE_ADC_VCC
  vcc = ESP.getVcc() / 1000.0f;
#endif // if FEATURE_ADC_VCC
#ifdef ESP8266
  espeasy_analogRead(A0);
#endif // ifdef ESP8266

  initAnalogWrite();

  resetPluginTaskData();

  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("setup"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  Serial.begin(115200);

  // serialPrint("\n\n\nBOOOTTT\n\n\n");

  initLog();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("initLog()"));
  #endif


  if (SpiffsSectors() < 32)
  {
    serialPrintln(F("\nNo (or too small) FS area..\nSystem Halted\nPlease reflash with 128k FS minimum!"));

    while (true) {
      delay(1);
    }
  }

  emergencyReset();

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("\n\n\rINIT : Booting version: ");
    log += getValue(LabelType::GIT_BUILD);
    log += " (";
    log += getSystemLibraryString();
    log += ')';
    addLog(LOG_LEVEL_INFO, log);
    log  = F("INIT : Free RAM:");
    log += FreeMem();
    addLog(LOG_LEVEL_INFO, log);
  }

  readBootCause();

  {
    String log  = F("INIT : ");
    log += getLastBootCauseString();

    if (readFromRTC())
    {
      RTC.bootFailedCount++;
      RTC.bootCounter++;
      lastMixedSchedulerId_beforereboot = RTC.lastMixedSchedulerId;
      readUserVarFromRTC();

      if (RTC.deepSleepState != 1)
      {
        node_time.restoreLastKnownUnixTime(RTC.lastSysTime, RTC.deepSleepState);
      }

      log += F(" #");
      log += RTC.bootCounter;

      #ifndef BUILD_NO_DEBUG
      log += F(" Last Action before Reboot: ");
      log += ESPEasy_Scheduler::decodeSchedulerId(lastMixedSchedulerId_beforereboot);
      log += F(" Last systime: ");
      log += RTC.lastSysTime;
      #endif // ifndef BUILD_NO_DEBUG
    }

    // cold boot (RTC memory empty)
    else
    {
      initRTC();

      // cold boot situation
      if (lastBootCause == BOOT_CAUSE_MANUAL_REBOOT) { // only set this if not set earlier during boot stage.
        lastBootCause = BOOT_CAUSE_COLD_BOOT;
      }
      log = F("INIT : Cold Boot");
    }

    log += F(" - Restart Reason: ");
    log += getResetReasonString();

    RTC.deepSleepState = 0;
    saveToRTC();

    addLog(LOG_LEVEL_INFO, log);
  }
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("RTC init"));
  #endif

  fileSystemCheck();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("fileSystemCheck()"));
  #endif

  //  progMemMD5check();
  LoadSettings();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("LoadSettings()"));
  #endif

  Settings.UseRTOSMultitasking = false; // For now, disable it, we experience heap corruption.

  if ((RTC.bootFailedCount > 10) && (RTC.bootCounter > 10)) {
    uint8_t toDisable = RTC.bootFailedCount - 10;
    toDisable = disablePlugin(toDisable);

    if (toDisable != 0) {
      toDisable = disableController(toDisable);
    }

    if (toDisable != 0) {
      toDisable = disableNotification(toDisable);
    }
  }
  #ifdef HAS_ETHERNET

  // This ensures, that changing WIFI OR ETHERNET MODE happens properly only after reboot. Changing without reboot would not be a good idea.
  // This only works after LoadSettings();
  setNetworkMedium(Settings.NetworkMedium);
  #endif // ifdef HAS_ETHERNET

  if (active_network_medium == NetworkMedium_t::WIFI) {
    WiFi_AP_Candidates.load_knownCredentials();
    if (!WiFi_AP_Candidates.hasKnownCredentials()) {
      WiFiEventData.wifiSetup = true;
      RTC.clearLastWiFi(); // Must scan all channels
      // Wait until scan has finished to make sure as many as possible are found
      // We're still in the setup phase, so nothing else is taking resources of the ESP.
      WifiScan(false);
      WiFiEventData.lastScanMoment.clear();
    }

    // Always perform WiFi scan
    // It appears reconnecting from RTC may take just as long to be able to send first packet as performing a scan first and then connect.
    // Perhaps the WiFi radio needs some time to stabilize first?
    WifiScan(true);
  }
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("WifiScan()"));
  #endif


  //  setWifiMode(WIFI_STA);
  checkRuleSets();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("checkRuleSets()"));
  #endif


  // if different version, eeprom settings structure has changed. Full Reset needed
  // on a fresh ESP module eeprom values are set to 255. Version results into -1 (signed int)
  if ((Settings.Version != VERSION) || (Settings.PID != ESP_PROJECT_PID))
  {
    // Direct Serial is allowed here, since this is only an emergency task.
    serialPrint(F("\nPID:"));
    serialPrintln(String(Settings.PID));
    serialPrint(F("Version:"));
    serialPrintln(String(Settings.Version));
    serialPrintln(F("INIT : Incorrect PID or version!"));
    delay(1000);
    ResetFactory();
  }

  initSerial();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("initSerial()"));
  #endif


  if (Settings.Build != BUILD) {
    BuildFixes();
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log  = F("INIT : Free RAM:");
    log += FreeMem();
    addLog(LOG_LEVEL_INFO, log);
  }

  if (Settings.UseSerial && (Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE)) {
    Serial.setDebugOutput(true);
  }

  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("hardwareInit"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  hardwareInit();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("hardwareInit()"));
  #endif


  timermqtt_interval      = 250; // Interval for checking MQTT
  timerAwakeFromDeepSleep = millis();
  CPluginInit();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("CPluginInit()"));
  #endif
  #ifdef USES_NOTIFIER
  NPluginInit();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("NPluginInit()"));
  #endif
  #endif // ifdef USES_NOTIFIER

  PluginInit();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("PluginInit()"));
  #endif
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log  = F("INFO : Plugins: ");
    log += deviceCount + 1;
    log += ' ';
    log += getPluginDescriptionString();
    log += " (";
    log += getSystemLibraryString();
    log += ')';
    addLog(LOG_LEVEL_INFO, log);
  }

  if (deviceCount + 1 >= PLUGIN_MAX) {
    addLog(LOG_LEVEL_ERROR, String(F("Programming error! - Increase PLUGIN_MAX (")) + deviceCount + ')');
  }

  clearAllCaches();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("clearAllCaches()"));
  #endif

  if (Settings.UseRules && isDeepSleepEnabled())
  {
    String event = F("System#NoSleep=");
    event += Settings.deepSleep_wakeTime;
    rulesProcessing(event); // TD-er: Process events in the setup() now.
  }

  if (Settings.UseRules)
  {
    String event = F("System#Wake");
    rulesProcessing(event); // TD-er: Process events in the setup() now.
  }

  NetworkConnectRelaxed();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("NetworkConnectRelaxed()"));
  #endif

  setWebserverRunning(true);
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("setWebserverRunning()"));
  #endif


  #ifdef FEATURE_REPORTING
  ReportStatus();
  #endif // ifdef FEATURE_REPORTING

  #ifdef FEATURE_ARDUINO_OTA
  ArduinoOTAInit();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("ArduinoOTAInit()"));
  #endif
  #endif // ifdef FEATURE_ARDUINO_OTA

  if (node_time.systemTimePresent()) {
    node_time.initTime();
    #ifndef BUILD_NO_RAM_TRACKER
    logMemUsageAfter(F("node_time.initTime()"));
    #endif
  }

  if (Settings.UseRules)
  {
    String event = F("System#Boot");
    rulesProcessing(event); // TD-er: Process events in the setup() now.
    #ifndef BUILD_NO_RAM_TRACKER
    logMemUsageAfter(F("rulesProcessing(System#Boot)"));
    #endif
  }

  writeDefaultCSS();
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("writeDefaultCSS()"));
  #endif


  UseRTOSMultitasking = Settings.UseRTOSMultitasking;
  #ifdef USE_RTOS_MULTITASKING

  if (UseRTOSMultitasking) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("RTOS : Launching tasks");
      addLog(LOG_LEVEL_INFO, log);
    }
    xTaskCreatePinnedToCore(RTOS_TaskServers, "RTOS_TaskServers", 16384, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(RTOS_TaskSerial,  "RTOS_TaskSerial",  8192,  NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(RTOS_Task10ps,    "RTOS_Task10ps",    8192,  NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(
      RTOS_HandleSchedule,   /* Function to implement the task */
      "RTOS_HandleSchedule", /* Name of the task */
      16384,                 /* Stack size in words */
      NULL,                  /* Task input parameter */
      1,                     /* Priority of the task */
      NULL,                  /* Task handle. */
      1);                    /* Core where the task should run */
  }
  #endif // ifdef USE_RTOS_MULTITASKING

  // Start the interval timers at N msec from now.
  // Make sure to start them at some time after eachother,
  // since they will keep running at the same interval.
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_20MSEC,     5);    // timer for periodic actions 50 x per/sec
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_100MSEC,    66);   // timer for periodic actions 10 x per/sec
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_1SEC,       777);  // timer for periodic actions once per/sec
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_30SEC,      1333); // timer for watchdog once per 30 sec
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_MQTT,       88);   // timer for interaction with MQTT
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_STATISTICS, 2222);
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("Scheduler.setIntervalTimerOverride"));
  #endif

}
