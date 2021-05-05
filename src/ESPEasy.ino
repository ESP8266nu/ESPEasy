
#include <Arduino.h>

#ifdef CONTINUOUS_INTEGRATION
#pragma GCC diagnostic error "-Wall"
#else
#pragma GCC diagnostic warning "-Wall"
#endif

// Include this as first, to make sure all defines are active during the entire compile.
// See: https://www.letscontrolit.com/forum/viewtopic.php?f=4&t=7980
// If Custom.h build from Arduino IDE is needed, uncomment #define USE_CUSTOM_H in ESPEasy_common.h
#include "ESPEasy_common.h"

#ifdef USE_CUSTOM_H
// make the compiler show a warning to confirm that this file is inlcuded
  #warning "**** Using Settings from Custom.h File ***"
#endif



// Needed due to preprocessor issues.
#ifdef PLUGIN_SET_GENERIC_ESP32
  #ifndef ESP32
    #define ESP32
  #endif
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
//   Simple switch inputs and direct GPIO output control to drive relays, mosfets, etc
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
//   DL-Bus (Technische Alternative)

// Define globals before plugin sets to allow a personal override of the selected plugins
#include "ESPEasy-Globals.h"
// Must be included after all the defines, since it is using TASKS_MAX
#include "_Plugin_Helper.h"
// Plugin helper needs the defined controller sets, thus include after 'define_plugin_sets.h'
#include "src/Helpers/_CPlugin_Helper.h"


#include "src/DataStructs/ControllerSettingsStruct.h"
#include "src/DataStructs/ESPEasy_EventStruct.h"
#include "src/DataStructs/PortStatusStruct.h"
#include "src/DataStructs/ProtocolStruct.h"
#include "src/DataStructs/RTCStruct.h"
#include "src/DataStructs/SystemTimerStruct.h"
#include "src/DataStructs/TimingStats.h"
#include "src/DataStructs/tcp_cleanup.h"

#include "src/DataTypes/DeviceModel.h"
#include "src/DataTypes/SettingsType.h"

#include "src/ESPEasyCore/ESPEasy_Log.h"
#include "src/ESPEasyCore/ESPEasyNetwork.h"
#include "src/ESPEasyCore/ESPEasyRules.h"
#include "src/ESPEasyCore/ESPEasyWifi.h"
#include "src/ESPEasyCore/ESPEasyWifi_ProcessEvent.h"
#include "src/ESPEasyCore/Serial.h"

#include "src/Globals/Cache.h"
#include "src/Globals/CPlugins.h"
#include "src/Globals/Device.h"
#include "src/Globals/ESPEasyWiFiEvent.h"
#include "src/Globals/ESPEasy_Scheduler.h"
#include "src/Globals/ESPEasy_time.h"
#include "src/Globals/EventQueue.h"
#include "src/Globals/ExtraTaskSettings.h"
#include "src/Globals/GlobalMapPortStatus.h"
#include "src/Globals/MQTT.h"
#include "src/Globals/NetworkState.h"
#include "src/Globals/Plugins.h"
#include "src/Globals/Protocol.h"
#include "src/Globals/RTC.h"
#include "src/Globals/RamTracker.h"
#include "src/Globals/SecuritySettings.h"
#include "src/Globals/Services.h"
#include "src/Globals/Settings.h"
#include "src/Globals/Statistics.h"
#include "src/Globals/WiFi_AP_Candidates.h"

#include "src/Helpers/DeepSleep.h"
#include "src/Helpers/ESPEasyRTC.h"
#include "src/Helpers/ESPEasy_FactoryDefault.h"
#include "src/Helpers/ESPEasy_Storage.h"
#include "src/Helpers/ESPEasy_checks.h"
#include "src/Helpers/Hardware.h"
#include "src/Helpers/Memory.h"
#include "src/Helpers/Misc.h"
#include "src/Helpers/Network.h"
#include "src/Helpers/Networking.h"
#include "src/Helpers/OTA.h"
#include "src/Helpers/PeriodicalActions.h"
#include "src/Helpers/Scheduler.h"
#include "src/Helpers/StringGenerator_System.h"
#include "src/Helpers/RepeatResetDetect.h"

#include "src/WebServer/WebServer.h"

#ifdef PHASE_LOCKED_WAVEFORM
#include <core_esp8266_waveform.h>
#endif

#if FEATURE_ADC_VCC
ADC_MODE(ADC_VCC);
#endif


#ifdef CORE_POST_2_5_0
void preinit();
#endif

#ifdef CORE_POST_2_5_0
/*********************************************************************************************\
 * Pre-init
\*********************************************************************************************/
void preinit() {
  // Global WiFi constructors are not called yet
  // (global class instances like WiFi, Serial... are not yet initialized)..
  // No global object methods or C++ exceptions can be called in here!
  //The below is a static class method, which is similar to a function, so it's ok.
  ESP8266WiFiClass::preinitWiFiOff();
  system_phy_set_powerup_option(RF_NO_CAL);

}
#endif

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
void setup()
{
#ifdef ESP8266_DISABLE_EXTRA4K
  disable_extra4k_at_link_time();
#endif
#ifdef PHASE_LOCKED_WAVEFORM
  enablePhaseLockedWaveform();
#endif
  initWiFi();
  
  run_compiletime_checks();
#ifndef BUILD_NO_RAM_TRACKER
  lowestFreeStack = getFreeStackWatermark();
  lowestRAM = FreeMem();
#endif
#ifndef ESP32
//  ets_isr_attach(8, sw_watchdog_callback, NULL);  // Set a callback for feeding the watchdog.
#endif


  // Read ADC at boot, before WiFi tries to connect.
  // see https://github.com/letscontrolit/ESPEasy/issues/2646
#if FEATURE_ADC_VCC
  vcc = ESP.getVcc() / 1000.0f;
#endif
#ifdef ESP8266
  espeasy_analogRead(A0);
#endif

  initAnalogWrite();

  resetPluginTaskData();

  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("setup"));
  #endif

  Serial.begin(115200);
  // serialPrint("\n\n\nBOOOTTT\n\n\n");

  initLog();

  if (SpiffsSectors() < 32)
  {
    serialPrintln(F("\nNo (or too small) FS area..\nSystem Halted\nPlease reflash with 128k FS minimum!"));
    while (true)
      delay(1);
  }

  emergencyReset();

  String log = F("\n\n\rINIT : Booting version: ");
  log += F(BUILD_GIT);
  log += " (";
  log += getSystemLibraryString();
  log += ')';
  addLog(LOG_LEVEL_INFO, log);
  log = F("INIT : Free RAM:");
  log += FreeMem();
  addLog(LOG_LEVEL_INFO, log);

  #ifdef ESP8266
  // Our ESP32 code does not yet support RTC, so separate this in code for ESP8266 and ESP32
  if (readFromRTC())
  {
    RTC.bootFailedCount++;
    RTC.bootCounter++;
    lastMixedSchedulerId_beforereboot = RTC.lastMixedSchedulerId;
    readUserVarFromRTC();

    if (RTC.deepSleepState == 1)
    {
      log = F("INIT : Rebooted from deepsleep #");
      lastBootCause = BOOT_CAUSE_DEEP_SLEEP;
    }
    else {
      RepeatResetDetect::onWarmBoot(RTC.repeatResetCount, RTC.repeatResetMarker);
      node_time.restoreLastKnownUnixTime(RTC.lastSysTime, RTC.deepSleepState);
      log = F("INIT : Warm boot #");
    }

    log += RTC.bootCounter;
    #ifndef BUILD_NO_DEBUG
    log += F(" Last Action before Reboot: ");
    log += ESPEasy_Scheduler::decodeSchedulerId(lastMixedSchedulerId_beforereboot);
    log += F(" Last systime: ");
    log += RTC.lastSysTime;
    log += F(" repeatReset #");
    log += RTC.repeatResetCount;
    #endif
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
  #endif // ESP8266

  #ifdef ESP32
  if (rtc_get_reset_reason( (RESET_REASON) 0) == DEEPSLEEP_RESET) {
    log = F("INIT : Rebooted from deepsleep #");
    lastBootCause = BOOT_CAUSE_DEEP_SLEEP;
  } else {
    // cold boot situation
    if (lastBootCause == BOOT_CAUSE_MANUAL_REBOOT) // only set this if not set earlier during boot stage.
      lastBootCause = BOOT_CAUSE_COLD_BOOT;
    log = F("INIT : Cold Boot");
  }

  #endif // ESP32

  log += F(" - Restart Reason: ");
  log += getResetReasonString();

  RTC.deepSleepState=0;
  RepeatResetDetect::markBoot(RTC.repeatResetMarker);
  saveToRTC();

  addLog(LOG_LEVEL_INFO, log);

  if (RepeatResetDetect::testTrigger(RTC.repeatResetCount)) {
    log = F("Factory reset on repeat reset #");
    log += RTC.repeatResetCount;
    addLog(LOG_LEVEL_INFO, log);
    ResetFactory();
  }

  fileSystemCheck();
//  progMemMD5check();
  LoadSettings();

  Settings.UseRTOSMultitasking = false; // For now, disable it, we experience heap corruption.
  if (RTC.bootFailedCount > 10 && RTC.bootCounter > 10) {
    byte toDisable = RTC.bootFailedCount - 10;
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
  #endif
  if (active_network_medium == NetworkMedium_t::WIFI) {
    if (!WiFi_AP_Candidates.hasKnownCredentials()) {
      WiFiEventData.wifiSetup = true;
      RTC.clearLastWiFi(); // Must scan all channels
      // Wait until scan has finished to make sure as many as possible are found
      // We're still in the setup phase, so nothing else is taking resources of the ESP.
      WifiScan(false); 
      WiFiEventData.lastScanMoment.clear();
    }
    // Start an extra async scan so we can continue, but we may find more APs by scanning twice.
    WifiScan(true); 
  }

//  setWifiMode(WIFI_STA);
  checkRuleSets();

  // if different version, eeprom settings structure has changed. Full Reset needed
  // on a fresh ESP module eeprom values are set to 255. Version results into -1 (signed int)
  if (Settings.Version != VERSION || Settings.PID != ESP_PROJECT_PID)
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

  if (Settings.Build != BUILD) {
    BuildFixes();
  }


  log = F("INIT : Free RAM:");
  log += FreeMem();
  addLog(LOG_LEVEL_INFO, log);

  if (Settings.UseSerial && Settings.SerialLogLevel >= LOG_LEVEL_DEBUG_MORE)
    Serial.setDebugOutput(true);
  
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("hardwareInit"));
  #endif
  hardwareInit();

  timermqtt_interval = 250; // Interval for checking MQTT
  timerAwakeFromDeepSleep = millis();
  CPluginInit();
  #ifdef USES_NOTIFIER
  NPluginInit();
  #endif
  PluginInit();
  log = F("INFO : Plugins: ");
  log += deviceCount + 1;
  log += ' ';
  log += getPluginDescriptionString();
  log += " (";
  log += getSystemLibraryString();
  log += ')';
  addLog(LOG_LEVEL_INFO, log);

  if (deviceCount + 1 >= PLUGIN_MAX) {
    addLog(LOG_LEVEL_ERROR, String(F("Programming error! - Increase PLUGIN_MAX (")) + deviceCount + ')');
  }

  clearAllCaches();

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

  setWebserverRunning(true);

  #ifdef FEATURE_REPORTING
  ReportStatus();
  #endif

  #ifdef FEATURE_ARDUINO_OTA
  ArduinoOTAInit();
  #endif

  if (node_time.systemTimePresent()) {
    node_time.initTime();
  }

  if (Settings.UseRules)
  {
    String event = F("System#Boot");
    rulesProcessing(event); // TD-er: Process events in the setup() now.
  }

  writeDefaultCSS();

  UseRTOSMultitasking = Settings.UseRTOSMultitasking;
  #ifdef USE_RTOS_MULTITASKING
    if(UseRTOSMultitasking){
      log = F("RTOS : Launching tasks");
      addLog(LOG_LEVEL_INFO, log);
      xTaskCreatePinnedToCore(RTOS_TaskServers, "RTOS_TaskServers", 16384, NULL, 1, NULL, 1);
      xTaskCreatePinnedToCore(RTOS_TaskSerial, "RTOS_TaskSerial", 8192, NULL, 1, NULL, 1);
      xTaskCreatePinnedToCore(RTOS_Task10ps, "RTOS_Task10ps", 8192, NULL, 1, NULL, 1);
      xTaskCreatePinnedToCore(
                    RTOS_HandleSchedule,   /* Function to implement the task */
                    "RTOS_HandleSchedule", /* Name of the task */
                    16384,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    1,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    1);         /* Core where the task should run */
    }
  #endif

  // Start the interval timers at N msec from now.
  // Make sure to start them at some time after eachother,
  // since they will keep running at the same interval.
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_20MSEC,  5); // timer for periodic actions 50 x per/sec
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_100MSEC, 66); // timer for periodic actions 10 x per/sec
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_1SEC,    777); // timer for periodic actions once per/sec
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_30SEC,   1333); // timer for watchdog once per 30 sec
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_MQTT,    88); // timer for interaction with MQTT
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_STATISTICS, 2222);
}

#ifdef USE_RTOS_MULTITASKING
void RTOS_TaskServers( void * parameter )
{
 while (true){
  delay(100);
  web_server.handleClient();
  checkUDP();
 }
}

void RTOS_TaskSerial( void * parameter )
{
  while (true){
    delay(100);
    serial();
  }
}

void RTOS_Task10ps( void * parameter )
{
 while (true){
    delay(100);
    run10TimesPerSecond();
 }
}

void RTOS_HandleSchedule( void * parameter )
{
 while (true){
    Scheduler.handle_schedule();
 }
}

#endif


void updateLoopStats() {
  ++loopCounter;
  ++loopCounter_full;
  if (lastLoopStart == 0) {
    lastLoopStart = micros();
    return;
  }
  const long usecSince = usecPassedSince(lastLoopStart);
  #ifdef USES_TIMING_STATS
  miscStats[LOOP_STATS].add(usecSince);
  #endif

  loop_usec_duration_total += usecSince;
  lastLoopStart = micros();
  if (usecSince <= 0 || usecSince > 10000000)
    return; // No loop should take > 10 sec.
  if (shortestLoop > static_cast<unsigned long>(usecSince)) {
    shortestLoop = usecSince;
    loopCounterMax = 30 * 1000000 / usecSince;
  }
  if (longestLoop < static_cast<unsigned long>(usecSince))
    longestLoop = usecSince;
}


float getCPUload() {
  return 100.0f - Scheduler.getIdleTimePct();
}

int getLoopCountPerSec() {
  return loopCounterLast / 30;
}

int getUptimeMinutes() {
  return wdcounter / 2;
}



/*********************************************************************************************\
 * MAIN LOOP
\*********************************************************************************************/
void loop()
{
  /*
  //FIXME TD-er: No idea what this does.
  if(MainLoopCall_ptr)
      MainLoopCall_ptr();
  */
  dummyString = String(); // Fixme TD-er  Make sure this global variable doesn't keep memory allocated.

  updateLoopStats();

  handle_unprocessedNetworkEvents();

  bool firstLoopConnectionsEstablished = NetworkConnected() && firstLoop;
  if (firstLoopConnectionsEstablished) {
     addLog(LOG_LEVEL_INFO, F("firstLoopConnectionsEstablished"));
     firstLoop = false;
     timerAwakeFromDeepSleep = millis(); // Allow to run for "awake" number of seconds, now we have wifi.
     // schedule_all_task_device_timers(); // Disabled for now, since we are now using queues for controllers.
     if (Settings.UseRules && isDeepSleepEnabled())
     {
        String event = F("System#NoSleep=");
        event += Settings.deepSleep_wakeTime;
        eventQueue.addMove(std::move(event));
     }


     RTC.bootFailedCount = 0;
     saveToRTC();
     sendSysInfoUDP(1);
  }
  // Work around for nodes that do not have WiFi connection for a long time and may reboot after N unsuccessful connect attempts
  if (getUptimeMinutes() > 2) {
    // Apparently the uptime is already a few minutes. Let's consider it a successful boot.
     RTC.bootFailedCount = 0;
     saveToRTC();
  }

  // Deep sleep mode, just run all tasks one (more) time and go back to sleep as fast as possible
  if ((firstLoopConnectionsEstablished || readyForSleep()) && isDeepSleepEnabled())
  {
#ifdef USES_MQTT
      runPeriodicalMQTT();
#endif //USES_MQTT
      // Now run all frequent tasks
      run50TimesPerSecond();
      run10TimesPerSecond();
      runEach30Seconds();
      runOncePerSecond();
  }
  //normal mode, run each task when its time
  else
  {
    if (!UseRTOSMultitasking) {
      // On ESP32 the schedule is executed on the 2nd core.
      Scheduler.handle_schedule();
    }
  }

  backgroundtasks();

  if (readyForSleep()){
    prepare_deepSleep(Settings.Delay);
    //deepsleep will never return, its a special kind of reboot
  }
}

void flushAndDisconnectAllClients() {
  if (anyControllerEnabled()) {
#ifdef USES_MQTT
    bool mqttControllerEnabled = validControllerIndex(firstEnabledMQTT_ControllerIndex());
#endif //USES_MQTT
    unsigned long timer = millis() + 1000;
    while (!timeOutReached(timer)) {
      // call to all controllers (delay queue) to flush all data.
      CPluginCall(CPlugin::Function::CPLUGIN_FLUSH, 0);
#ifdef USES_MQTT      
      if (mqttControllerEnabled && MQTTclient.connected()) {
        MQTTclient.loop();
      }
#endif //USES_MQTT
    }
#ifdef USES_MQTT
    if (mqttControllerEnabled && MQTTclient.connected()) {
      MQTTclient.disconnect();
      updateMQTTclient_connected();
    }
#endif //USES_MQTT
    saveToRTC();
    delay(100); // Flush anything in the network buffers.
  }
  process_serialWriteBuffer();
}









/*********************************************************************************************\
 * run background tasks
\*********************************************************************************************/
bool runningBackgroundTasks=false;
void backgroundtasks()
{
  //checkRAM(F("backgroundtasks"));
  //always start with a yield
  delay(0);
/*
  // Remove this watchdog feed for now.
  // See https://github.com/letscontrolit/ESPEasy/issues/1722#issuecomment-419659193

  #ifdef ESP32
  // Have to find a similar function to call ESP32's esp_task_wdt_feed();
  #else
  ESP.wdtFeed();
  #endif
*/

  //prevent recursion!
  if (runningBackgroundTasks)
  {
    return;
  }
  START_TIMER
  const bool networkConnected = NetworkConnected();
  runningBackgroundTasks=true;

  /*
  // Not needed anymore, see: https://arduino-esp8266.readthedocs.io/en/latest/faq/readme.html#how-to-clear-tcp-pcbs-in-time-wait-state
  if (networkConnected) {
    #if defined(ESP8266)
      tcpCleanup();
    #endif
  }
  */

  process_serialWriteBuffer();
  if(!UseRTOSMultitasking){
    serial();
    if (webserverRunning) {
      web_server.handleClient();
    }
    if (networkConnected) {
      checkUDP();
    }
  }

  #ifdef FEATURE_DNS_SERVER
  // process DNS, only used if the ESP has no valid WiFi config
  if (dnsServerActive) {
    dnsServer.processNextRequest();
  }
  #endif

  #ifdef FEATURE_ARDUINO_OTA
  if(Settings.ArduinoOTAEnable && networkConnected)
    ArduinoOTA.handle();

  //once OTA is triggered, only handle that and dont do other stuff. (otherwise it fails)
  while (ArduinoOTAtriggered)
  {
    delay(0);
    if (NetworkConnected()) {
      ArduinoOTA.handle();
    }
  }

  #endif

  #ifdef FEATURE_MDNS
  // Allow MDNS processing
  if (networkConnected) {
    #ifdef ESP8266
    // ESP32 does not have an update() function
    MDNS.update();
    #endif
  }
  #endif

  delay(0);

  statusLED(false);

  runningBackgroundTasks=false;
  STOP_TIMER(BACKGROUND_TASKS);
}