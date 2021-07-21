#include "../Helpers/PeriodicalActions.h"

#include "../../ESPEasy_common.h"

#include "../../ESPEasy-Globals.h"

#include "../ControllerQueue/DelayQueueElements.h"
#include "../ControllerQueue/MQTT_queue_element.h"
#include "../DataStructs/TimingStats.h"
#include "../DataTypes/ESPEasy_plugin_functions.h"
#include "../ESPEasyCore/Controller.h"
#include "../ESPEasyCore/ESPEasyGPIO.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/ESPEasyRules.h"
#include "../ESPEasyCore/Serial.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/EventQueue.h"
#include "../Globals/MainLoopCommand.h"
#include "../Globals/MQTT.h"
#include "../Globals/NetworkState.h"
#include "../Globals/RTC.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Services.h"
#include "../Globals/Settings.h"
#include "../Globals/Statistics.h"
#include "../Globals/ESPEasy_now_handler.h"
#include "../Globals/SendData_DuplicateChecker.h"
#include "../Helpers/ESPEasyRTC.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Memory.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Network.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringGenerator_System.h"
#include "../Helpers/StringGenerator_WiFi.h"
#include "../Helpers/StringProvider.h"

#ifdef USES_C015
#include "../../ESPEasy_fdwdecl.h"
#endif



#define PLUGIN_ID_MQTT_IMPORT         37


/*********************************************************************************************\
 * Tasks that run 50 times per second
\*********************************************************************************************/

void run50TimesPerSecond() {
  String dummy;
  {
    START_TIMER;
    PluginCall(PLUGIN_FIFTY_PER_SECOND, 0, dummy);
    STOP_TIMER(PLUGIN_CALL_50PS);
  }
  {
    START_TIMER;
    CPluginCall(CPlugin::Function::CPLUGIN_FIFTY_PER_SECOND, 0, dummy);
    STOP_TIMER(CPLUGIN_CALL_50PS);
  }
  #ifdef USES_ESPEASY_NOW
  {
    if (ESPEasy_now_handler.loop()) {
      // FIXME TD-er: Must check if enabled, etc.
    }
    START_TIMER;
    SendData_DuplicateChecker.loop();
    STOP_TIMER(ESPEASY_NOW_DEDUP_LOOP);
  }
  #endif

}

/*********************************************************************************************\
 * Tasks that run 10 times per second
\*********************************************************************************************/
void run10TimesPerSecond() {
  String dummy;
  //@giig19767g: WARNING: Monitor10xSec must run before PLUGIN_TEN_PER_SECOND
  {
    START_TIMER;
    GPIO_Monitor10xSec();
    STOP_TIMER(PLUGIN_CALL_10PSU);
  }
  {
    START_TIMER;
    PluginCall(PLUGIN_TEN_PER_SECOND, 0, dummy);
    STOP_TIMER(PLUGIN_CALL_10PS);
  }
  {
    START_TIMER;
//    PluginCall(PLUGIN_UNCONDITIONAL_POLL, 0, dummyString);
    PluginCall(PLUGIN_MONITOR, 0, dummy);
    STOP_TIMER(PLUGIN_CALL_10PSU);
  }
  {
    START_TIMER;
    CPluginCall(CPlugin::Function::CPLUGIN_TEN_PER_SECOND, 0, dummy);
    STOP_TIMER(CPLUGIN_CALL_10PS);
  }
  processNextEvent();
  
  #ifdef USES_C015
  if (NetworkConnected())
      Blynk_Run_c015();
  #endif
  #ifndef USE_RTOS_MULTITASKING
    web_server.handleClient();
  #endif
}


/*********************************************************************************************\
 * Tasks each second
\*********************************************************************************************/
void runOncePerSecond()
{
  START_TIMER;
  updateLogLevelCache();
  dailyResetCounter++;
  if (dailyResetCounter > 86400) // 1 day elapsed... //86400
  {
    RTC.flashDayCounter=0;
    saveToRTC();
    dailyResetCounter=0;
    addLog(LOG_LEVEL_INFO, F("SYS  : Reset 24h counters"));
  }

  if (Settings.ConnectionFailuresThreshold)
    if (WiFiEventData.connectionFailures > Settings.ConnectionFailuresThreshold)
      delayedReboot(60, ESPEasy_Scheduler::IntendedRebootReason_e::DelayedReboot);

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
          reboot(ESPEasy_Scheduler::IntendedRebootReason_e::CommandReboot);
          break;
        }
    }
    cmd_within_mainloop = 0;
  }
  // clock events
  if (node_time.reportNewMinute()) {
    String dummy;
    PluginCall(PLUGIN_CLOCK_IN, 0, dummy);
    if (Settings.UseRules)
    {
      String event;
      event.reserve(21);
      event  = F("Clock#Time=");
      event += node_time.weekday_str();
      event += ",";

      if (node_time.hour() < 10) {
        event += '0';
      }
      event += node_time.hour();
      event += ":";

      if (node_time.minute() < 10) {
        event += '0';
      }
      event += node_time.minute();
      // TD-er: Do not add to the eventQueue, but execute right now.
      rulesProcessing(event);
    }
  }

//  unsigned long start = micros();
  String dummy;
  PluginCall(PLUGIN_ONCE_A_SECOND, 0, dummy);
//  unsigned long elapsed = micros() - start;


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

  checkResetFactoryPin();
  STOP_TIMER(PLUGIN_CALL_1PS);
}

/*********************************************************************************************\
 * Tasks each 30 seconds
\*********************************************************************************************/
void runEach30Seconds()
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAMtoLog();
  #endif
  wdcounter++;
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(80);
    log = F("WD   : Uptime ");
    log += getUptimeMinutes();
    log += F(" ConnectFailures ");
    log += WiFiEventData.connectionFailures;
    log += F(" FreeMem ");
    log += FreeMem();
    #ifdef HAS_ETHERNET
    if(active_network_medium == NetworkMedium_t::Ethernet) {
      log += F( " EthSpeedState ");
      log += getValue(LabelType::ETH_SPEED_STATE);
    } else {
      log += F(" WiFiStatus ");
      log += ArduinoWifiStatusToString(WiFi.status());
    }
    #else
    log += F(" WiFiStatus ");
    log += ArduinoWifiStatusToString(WiFi.status());
    #endif
    log += F(" ESPeasy internal wifi status: ");
    log += ESPeasyWifiStatusToString();

//    log += F(" ListenInterval ");
//    log += WiFi.getListenInterval();
    addLog(LOG_LEVEL_INFO, log);
  }
  WiFiScanPeriodical();
  sendSysInfoUDP(1);
  refreshNodeList();

  // sending $stats to homie controller
  CPluginCall(CPlugin::Function::CPLUGIN_INTERVAL, 0);

  #if defined(ESP8266)
  #ifdef USES_SSDP
  if (Settings.UseSSDP)
    SSDP_update();

  #endif // USES_SSDP
  #endif
#if FEATURE_ADC_VCC
  if (!WiFiEventData.wifiConnectInProgress) {
    vcc = ESP.getVcc() / 1000.0f;
  }
#endif

  #ifdef FEATURE_REPORTING
  ReportStatus();
  #endif

}

#ifdef USES_MQTT


void scheduleNextMQTTdelayQueue() {
  if (MQTTDelayHandler != nullptr) {
    Scheduler.scheduleNextDelayQueue(ESPEasy_Scheduler::IntervalTimer_e::TIMER_MQTT_DELAY_QUEUE, MQTTDelayHandler->getNextScheduleTime());
  }
}

void schedule_all_tasks_using_MQTT_controller() {
  controllerIndex_t ControllerIndex = firstEnabledMQTT_ControllerIndex();

  if (!validControllerIndex(ControllerIndex)) { return; }

  deviceIndex_t DeviceIndex = getDeviceIndex(PLUGIN_ID_MQTT_IMPORT); // Check if P037_MQTTimport is present in the build
  if (validDeviceIndex(DeviceIndex)) {
    for (taskIndex_t task = 0; task < TASKS_MAX; task++) {
      if (Settings.TaskDeviceNumber[task] == PLUGIN_ID_MQTT_IMPORT) {
        // Schedule a call to each MQTT import plugin to notify the broker connection state
        EventStruct event(task);
        event.Par1 = MQTTclient_connected ? 1 : 0;
        Scheduler.schedule_plugin_task_event_timer(DeviceIndex, PLUGIN_MQTT_CONNECTION_STATE, std::move(event));
      }
    }
  }

  for (taskIndex_t task = 0; task < TASKS_MAX; task++) {
    if (Settings.TaskDeviceSendData[ControllerIndex][task] &&
        Settings.ControllerEnabled[ControllerIndex] &&
        Settings.Protocol[ControllerIndex])
    {
      Scheduler.schedule_task_device_timer_at_init(task);
    }
  }
}

void processMQTTdelayQueue() {
  if (MQTTDelayHandler == nullptr) {
    return;
  }

  runPeriodicalMQTT(); // Update MQTT connected state.
  #ifndef USES_ESPEASY_NOW
  // When using ESPEasy_NOW we may still send MQTT messages even when we're not connected.
  // For all other situations no need to continue.
  if (!MQTTclient_connected) {
    scheduleNextMQTTdelayQueue();
    return;
  }
  #endif

  START_TIMER;
  MQTT_queue_element *element(MQTTDelayHandler->getNext());

  if (element == nullptr) { return; }

#ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;
    log.reserve(30 + element->_topic.length() + element->_payload.length());
    log += F("processMQTTdelayQueue: ");
    log += element->_topic;
    log += " ";
    log += element->_payload.substring(0,64);
    addLog(LOG_LEVEL_DEBUG, log);
  }
#endif

  bool processed = false;
  MessageRouteInfo_t messageRouteInfo;
  if (element->getMessageRouteInfo() != nullptr) {
    messageRouteInfo = *(element->getMessageRouteInfo());
  }
  messageRouteInfo.appendUnit(Settings.Unit);

  #ifdef USES_ESPEASY_NOW
  if (element->_topic.startsWith(F("traceroute/")) || element->_topic.indexOf(F("/traceroute/")) != -1) {
    // Special debug feature for ESPEasy-NOW to perform a traceroute of packets.
    // The message is prepended by each unit number handling the message.
    const String replacement = getValue(LabelType::UNIT_NR);
    String message;
    message.reserve(replacement.length() + 1 + element->_payload.length());
    message  = replacement;
    message += ';';
    message += element->_payload;

    processed = processMQTT_message(element->controller_idx, element->_topic, message, element->_retained, &messageRouteInfo);
  } else 
  #endif
  {
    processed = processMQTT_message(element->controller_idx, element->_topic, element->_payload, element->_retained, &messageRouteInfo);
  }


  MQTTDelayHandler->markProcessed(processed);
  if (processed) {
    statusLED(true);
  } else {
#ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = F("MQTT : process MQTT queue not published, ");
      log += MQTTDelayHandler->sendQueue.size();
      log += F(" items left in queue");
      addLog(LOG_LEVEL_DEBUG, log);
    }
#endif // ifndef BUILD_NO_DEBUG
  }
  Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_MQTT, 10); // Make sure the MQTT is being processed as soon as possible.
  scheduleNextMQTTdelayQueue();
  STOP_TIMER(MQTT_DELAY_QUEUE);
}

bool processMQTT_message(controllerIndex_t controllerIndex,
                        const String    & topic,
                        const String    & payload,
                        bool retained,
                        const MessageRouteInfo_t* messageRouteInfo) 
{
  bool processed = false;

  #ifdef USES_ESPEASY_NOW
  if (!MQTTclient_connected) {
    processed = ESPEasy_now_handler.sendToMQTT(controllerIndex, topic, payload, messageRouteInfo);
  }
  #endif

  if (!processed) {
    if (MQTTclient.publish(topic.c_str(), payload.c_str(), retained)) {
      // FIXME TD-er: Must check if connected via WiFi or Ethernet
      if (WiFiEventData.connectionFailures > 0) {
        --WiFiEventData.connectionFailures;
      }
//#ifndef BUILD_NO_DEBUG
      if (loglevelActiveFor(LOG_LEVEL_DEBUG) && messageRouteInfo != nullptr) {
        String log = F("MQTT : published from mesh: ");
        log += messageRouteInfo->toString();
        addLog(LOG_LEVEL_DEBUG, log);
      }
//#endif // ifndef BUILD_NO_DEBUG
      processed = true;
    }
  }
  return processed;
}

void updateMQTTclient_connected() {
  if (MQTTclient_connected != MQTTclient.connected()) {
    MQTTclient_connected = !MQTTclient_connected;
    if (!MQTTclient_connected) {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        String connectionError = F("MQTT : Connection lost, state: ");
        connectionError += getMQTT_state();
        addLog(LOG_LEVEL_ERROR, connectionError);
      }
      MQTTclient_must_send_LWT_connected = false;
    } else {
      // Now schedule all tasks using the MQTT controller.
      schedule_all_tasks_using_MQTT_controller();
    }
    if (Settings.UseRules) {
      if (MQTTclient_connected) {
        eventQueue.add(F("MQTT#Connected"));
      } else {
        eventQueue.add(F("MQTT#Disconnected"));
      }
    }
  }
  if (!MQTTclient_connected) {
    // As suggested here: https://github.com/letscontrolit/ESPEasy/issues/1356
    if (timermqtt_interval < 30000) {
      timermqtt_interval += 5000;
    }
  } else {
    timermqtt_interval = 250;
  }
  Scheduler.setIntervalTimer(ESPEasy_Scheduler::IntervalTimer_e::TIMER_MQTT);
  scheduleNextMQTTdelayQueue();
}

void runPeriodicalMQTT() {
  // MQTT_KEEPALIVE = 15 seconds.
  if (!NetworkConnected(10)) {
    updateMQTTclient_connected();
    return;
  }
  //dont do this in backgroundtasks(), otherwise causes crashes. (https://github.com/letscontrolit/ESPEasy/issues/683)
  controllerIndex_t enabledMqttController = firstEnabledMQTT_ControllerIndex();
  if (validControllerIndex(enabledMqttController)) {
    if (!MQTTclient.loop()) {
      updateMQTTclient_connected();
      if (MQTTCheck(enabledMqttController)) {
        updateMQTTclient_connected();
      }
    }
  } else {
    if (MQTTclient.connected()) {
      MQTTclient.disconnect();
      updateMQTTclient_connected();
    }
  }
}

// FIXME TD-er: Must move to a more logical part of the code
controllerIndex_t firstEnabledMQTT_ControllerIndex() {
  for (controllerIndex_t i = 0; i < CONTROLLER_MAX; ++i) {
    protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(i);
    if (validProtocolIndex(ProtocolIndex)) {
      if (Protocol[ProtocolIndex].usesMQTT && Settings.ControllerEnabled[i]) {
        return i;
      }
    }
  }
  return INVALID_CONTROLLER_INDEX;
}


#endif //USES_MQTT



void logTimerStatistics() {
  static bool firstRun = true;
  if (firstRun) {
    Scheduler.setIntervalTimerOverride(ESPEasy_Scheduler::IntervalTimer_e::TIMER_STATISTICS, 1000);
    firstRun = false;
  }

  uint8_t loglevel = LOG_LEVEL_DEBUG;
  updateLoopStats_30sec(loglevel);
#ifndef BUILD_NO_DEBUG
//  logStatistics(loglevel, true);
  if (loglevelActiveFor(loglevel)) {
    String queueLog = F("Scheduler stats: (called/tasks/max_length/idle%) ");
    queueLog += Scheduler.getQueueStats();
    addLog(loglevel, queueLog);
  }
#endif
}

void updateLoopStats_30sec(uint8_t loglevel) {
  loopCounterLast = loopCounter;
  loopCounter = 0;
  if (loopCounterLast > loopCounterMax)
    loopCounterMax = loopCounterLast;

  Scheduler.updateIdleTimeStats();

#ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(loglevel)) {
    String log = F("LoopStats: shortestLoop: ");
    log += shortestLoop;
    log += F(" longestLoop: ");
    log += longestLoop;
    log += F(" avgLoopDuration: ");
    log += loop_usec_duration_total / loopCounter_full;
    log += F(" loopCounterMax: ");
    log += loopCounterMax;
    log += F(" loopCounterLast: ");
    log += loopCounterLast;
    addLog(loglevel, log);
  }
#endif
  loop_usec_duration_total = 0;
  loopCounter_full = 1;
}


/********************************************************************************************\
   Clean up all before going to sleep or reboot.
 \*********************************************************************************************/
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


void prepareShutdown(ESPEasy_Scheduler::IntendedRebootReason_e reason)
{
#ifdef USES_MQTT
  runPeriodicalMQTT(); // Flush outstanding MQTT messages
#endif // USES_MQTT
  process_serialWriteBuffer();
  flushAndDisconnectAllClients();
  saveUserVarToRTC();
  setWifiMode(WIFI_OFF);
  ESPEASY_FS.end();
  delay(100); // give the node time to flush all before reboot or sleep
  node_time.now();
  Scheduler.markIntendedReboot(reason);
  saveToRTC();
}


