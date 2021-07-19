#include "../Helpers/Scheduler.h"

#include "../../ESPEasy_common.h"

#include "../../ESPEasy-Globals.h"

#include "../../_Plugin_Helper.h"

#include "../Commands/GPIO.h"
#include "../ControllerQueue/DelayQueueElements.h"
#include "../ESPEasyCore/ESPEasyGPIO.h"
#include "../ESPEasyCore/ESPEasyRules.h"
#include "../Globals/GlobalMapPortStatus.h"
#include "../Globals/RTC.h"
#include "../Helpers/DeepSleep.h"
#include "../Helpers/ESPEasyRTC.h"
#include "../Helpers/Networking.h"
#include "../Helpers/PeriodicalActions.h"
#include "../Helpers/PortStatus.h"


#define TIMER_ID_SHIFT       28   // Must be decreased as soon as timers below reach 15
#define SYSTEM_EVENT_QUEUE   0 // Not really a timer.
#define CONST_INTERVAL_TIMER 1
#define PLUGIN_TASK_TIMER    2
#define TASK_DEVICE_TIMER    3
#define GPIO_TIMER           4
#define PLUGIN_TIMER         5
#define RULES_TIMER          6
#define REBOOT_TIMER         15 // Used to show intended reboot


String ESPEasy_Scheduler::toString(ESPEasy_Scheduler::IntervalTimer_e timer) {
#ifdef BUILD_NO_DEBUG
  return String(static_cast<int>(timer));
#else
  switch (timer) {
    case IntervalTimer_e::TIMER_20MSEC:           return F("TIMER_20MSEC");
    case IntervalTimer_e::TIMER_100MSEC:          return F("TIMER_100MSEC");
    case IntervalTimer_e::TIMER_1SEC:             return F("TIMER_1SEC");
    case IntervalTimer_e::TIMER_30SEC:            return F("TIMER_30SEC");
    case IntervalTimer_e::TIMER_MQTT:             return F("TIMER_MQTT");
    case IntervalTimer_e::TIMER_STATISTICS:       return F("TIMER_STATISTICS");
    case IntervalTimer_e::TIMER_GRATUITOUS_ARP:   return F("TIMER_GRATUITOUS_ARP");
    case IntervalTimer_e::TIMER_MQTT_DELAY_QUEUE: return F("TIMER_MQTT_DELAY_QUEUE");
    case IntervalTimer_e::TIMER_C001_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C003_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C004_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C007_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C008_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C009_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C010_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C011_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C012_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C013_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C014_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C015_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C016_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C017_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C018_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C019_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C020_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C021_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C022_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C023_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C024_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C025_DELAY_QUEUE:
    {
      String res;
      res.reserve(24);
      res = F("TIMER_C0");
      const int id = static_cast<int>(timer) - static_cast<int>(IntervalTimer_e::TIMER_C001_DELAY_QUEUE) + 1;
      if (id < 10) { res += '0'; }
      res += id;
      res += F("_DELAY_QUEUE");
      return res;
    }
  }
  return F("unknown");
#endif
}

String ESPEasy_Scheduler::toString(ESPEasy_Scheduler::IntendedRebootReason_e reason) {
  switch(reason) {
    case IntendedRebootReason_e::DeepSleep:              return F("DeepSleep");
    case IntendedRebootReason_e::DelayedReboot:          return F("DelayedReboot");
    case IntendedRebootReason_e::ResetFactory:           return F("ResetFactory");
    case IntendedRebootReason_e::ResetFactoryPinActive:  return F("ResetFactoryPinActive");
    case IntendedRebootReason_e::ResetFactoryCommand:    return F("ResetFactoryCommand");
    case IntendedRebootReason_e::CommandReboot:          return F("CommandReboot");
    case IntendedRebootReason_e::RestoreSettings:        return F("RestoreSettings");
    case IntendedRebootReason_e::OTA_error:              return F("OTA_error");
    case IntendedRebootReason_e::ConnectionFailuresThreshold: return F("ConnectionFailuresThreshold");
  }
  return String(static_cast<int>(reason));
}

void ESPEasy_Scheduler::markIntendedReboot(ESPEasy_Scheduler::IntendedRebootReason_e reason) {
  const unsigned long mixed_id = getMixedId(REBOOT_TIMER, static_cast<unsigned long>(reason));
  RTC.lastMixedSchedulerId = mixed_id;
  saveToRTC();
}

/*********************************************************************************************\
* Generic Timer functions.
\*********************************************************************************************/
void ESPEasy_Scheduler::setNewTimerAt(unsigned long id, unsigned long timer) {
  START_TIMER;
  msecTimerHandler.registerAt(id, timer);
  STOP_TIMER(SET_NEW_TIMER);
}

// Mix timer type int with an ID describing the scheduled job.
unsigned long ESPEasy_Scheduler::getMixedId(unsigned long timerType, unsigned long id) {
  return (timerType << TIMER_ID_SHIFT) + id;
}

unsigned long ESPEasy_Scheduler::decodeSchedulerId(unsigned long mixed_id, unsigned long& timerType) {
  timerType = (mixed_id >> TIMER_ID_SHIFT);
  const unsigned long mask = (1 << TIMER_ID_SHIFT) - 1;

  return mixed_id & mask;
}

String ESPEasy_Scheduler::decodeSchedulerId(unsigned long mixed_id) {
  if (mixed_id == 0) {
    return F("Background Task");
  }
  unsigned long timerType = 0;
  const unsigned long id  = decodeSchedulerId(mixed_id, timerType);
  String idStr = String(id);
  String result = String(timerType);
#ifndef BUILD_NO_DEBUG
  result.reserve(64);
  switch (timerType) {
    case CONST_INTERVAL_TIMER:
      result = F("Const Interval: ");
      result +=  toString(static_cast<ESPEasy_Scheduler::IntervalTimer_e>(id));
      return result;
    case PLUGIN_TASK_TIMER:
    {
      result = F("PLUGIN_TIMER_IN: ");
      const deviceIndex_t deviceIndex = ((1 << 8) - 1) & id;
      if (validDeviceIndex(deviceIndex)) {
        idStr = getPluginNameFromDeviceIndex(deviceIndex);
      }
      result += idStr;
      return result;
    }
    case PLUGIN_TIMER:
    {
      result = F("PLUGIN_ONLY_TIMER_IN: ");
      const deviceIndex_t deviceIndex = ((1 << 8) - 1) & id;
      if (validDeviceIndex(deviceIndex)) {
        idStr = getPluginNameFromDeviceIndex(deviceIndex);
      }
      result += idStr;
      return result;
    }
    case TASK_DEVICE_TIMER:
    {
      result = F("PLUGIN_READ: Task ");
      // Id is taskIndex
      result += (id + 1);
      return result;
    }
    case GPIO_TIMER:
    {
      result = F("GPIO: ");
      uint8_t GPIOType = static_cast<uint8_t>((id) & 0xFF);
      uint8_t pinNumber = static_cast<uint8_t>((id >> 8) & 0xFF);
      uint8_t pinStateValue = static_cast<uint8_t>((id >> 16) & 0xFF);

      switch (GPIOType)
      {
        case GPIO_TYPE_INTERNAL:
          result += F("int");
          break;
        case GPIO_TYPE_MCP:
          result += F("MCP");
          break;
        case GPIO_TYPE_PCF:
          result += F("PCF");
          break;
        default:
          result += F("?");
          break;
      }
      result += F(" pin: ");
      result += pinNumber;
      result += F(" state: ");
      result += pinStateValue;
      return result;
    }
    case RULES_TIMER:
    {
      result = F("Rules#Timer=");
      const unsigned long mask  = (1 << TIMER_ID_SHIFT) - 1;
      const unsigned long timerID = id & mask;
      result += timerID;
      return result;
    }
    case REBOOT_TIMER:
    {
      result = F("Intended Reboot: ");
      result += toString(static_cast<ESPEasy_Scheduler::IntendedRebootReason_e>(id));
      return result;
    }
  }
#endif
  result += F(" timer, id: ");
  result += idStr;
  return result;
}

/*********************************************************************************************\
* Handle scheduled timers.
\*********************************************************************************************/
void ESPEasy_Scheduler::handle_schedule() {
  START_TIMER
  unsigned long timer    = 0;
  unsigned long mixed_id = 0;

  if (timePassedSince(last_system_event_run) < 500) {
    // Make sure system event queue will be looked at every now and then.
    mixed_id = msecTimerHandler.getNextId(timer);
  }

  if (RTC.lastMixedSchedulerId != mixed_id) {
    RTC.lastMixedSchedulerId = mixed_id;
    saveToRTC();
  }

  if (mixed_id == 0) {
    // No id ready to run right now.
    // Events are not that important to run immediately.
    // Make sure normal scheduled jobs run at higher priority.
    //backgroundtasks();
    process_system_event_queue();
    last_system_event_run = millis();
    STOP_TIMER(HANDLE_SCHEDULER_IDLE);
    return;
  }

  unsigned long timerType = 0;
  const unsigned long id  = decodeSchedulerId(mixed_id, timerType);

  delay(0); // See: https://github.com/letscontrolit/ESPEasy/issues/1818#issuecomment-425351328

  switch (timerType) {
    case CONST_INTERVAL_TIMER:
      process_interval_timer(static_cast<ESPEasy_Scheduler::IntervalTimer_e>(id), timer);
      break;
    case PLUGIN_TASK_TIMER:
      process_plugin_task_timer(id);
      break;
    case PLUGIN_TIMER:
      process_plugin_timer(id);
      break;
    case RULES_TIMER:
      process_rules_timer(id, timer);
      break;
    case TASK_DEVICE_TIMER:
      process_task_device_timer(id, timer);
      break;
    case GPIO_TIMER:
      process_gpio_timer(id);
      break;
  }
  STOP_TIMER(HANDLE_SCHEDULER_TASK);
}

/*********************************************************************************************\
* Interval Timer
* These timers set a new scheduled timer, based on the old value.
* This will make their interval as constant as possible.
\*********************************************************************************************/
void ESPEasy_Scheduler::setNextTimeInterval(unsigned long& timer, const unsigned long step) {
  timer += step;
  const long passed = timePassedSince(timer);

  if (passed < 0) {
    // Event has not yet happened, which is fine.
    return;
  }

  if (static_cast<unsigned long>(passed) > step) {
    // No need to keep running behind, start again.
    timer = millis() + step;
    return;
  }

  // Try to get in sync again.
  timer = millis() + (step - passed);
}

void ESPEasy_Scheduler::setIntervalTimer(IntervalTimer_e id) {
  setIntervalTimer(id, millis());
}

void ESPEasy_Scheduler::setIntervalTimerAt(IntervalTimer_e id, unsigned long newtimer) {
  setNewTimerAt(getMixedId(CONST_INTERVAL_TIMER, static_cast<unsigned long>(id)), newtimer);
}

void ESPEasy_Scheduler::setIntervalTimerOverride(IntervalTimer_e id, unsigned long msecFromNow) {
  unsigned long timer = millis();

  setNextTimeInterval(timer, msecFromNow);
  setNewTimerAt(getMixedId(CONST_INTERVAL_TIMER, static_cast<unsigned long>(id)), timer);
}

void ESPEasy_Scheduler::scheduleNextDelayQueue(IntervalTimer_e id, unsigned long nextTime) {
  if (nextTime != 0) {
    // Schedule for next process run.
    setIntervalTimerAt(id, nextTime);
  }
}

void ESPEasy_Scheduler::setIntervalTimer(IntervalTimer_e id, unsigned long lasttimer) {
  // Set the initial timers for the regular runs
  unsigned long interval = 0;

  switch (id) {
    case IntervalTimer_e::TIMER_20MSEC:         interval = 20; break;
    case IntervalTimer_e::TIMER_100MSEC:        interval = 100; break;
    case IntervalTimer_e::TIMER_1SEC:           interval = 1000; break;
    case IntervalTimer_e::TIMER_30SEC:
    case IntervalTimer_e::TIMER_STATISTICS:     interval = 30000; break;
    case IntervalTimer_e::TIMER_MQTT:           interval = timermqtt_interval; break;
    case IntervalTimer_e::TIMER_GRATUITOUS_ARP: interval = timer_gratuitous_arp_interval; break;

    // Fall-through for all DelayQueue, which are just the fall-back timers.
    // The timers for all delay queues will be set according to their own settings as long as there is something to process.
    case IntervalTimer_e::TIMER_MQTT_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C001_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C003_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C004_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C007_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C008_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C009_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C010_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C011_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C012_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C013_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C014_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C015_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C016_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C017_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C018_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C019_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C020_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C021_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C022_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C023_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C024_DELAY_QUEUE:
    case IntervalTimer_e::TIMER_C025_DELAY_QUEUE:
      // When extending this, search for EXTEND_CONTROLLER_IDS 
      // in the code to find all places that need to be updated too.
      interval = 1000; break;
  }
  unsigned long timer = lasttimer;

  setNextTimeInterval(timer, interval);
  setNewTimerAt(getMixedId(CONST_INTERVAL_TIMER, static_cast<unsigned long>(id)), timer);
}

void ESPEasy_Scheduler::sendGratuitousARP_now() {
  sendGratuitousARP();

  if (Settings.gratuitousARP()) {
    timer_gratuitous_arp_interval = 100;
    setIntervalTimer(ESPEasy_Scheduler::IntervalTimer_e::TIMER_GRATUITOUS_ARP);
  }
}

void ESPEasy_Scheduler::process_interval_timer(IntervalTimer_e id, unsigned long lasttimer) {
  // Set the interval timer now, it may be altered by the commands below.
  // This is the default next-run-time.
  setIntervalTimer(id, lasttimer);

  switch (id) {
    case IntervalTimer_e::TIMER_20MSEC:         run50TimesPerSecond(); break;
    case IntervalTimer_e::TIMER_100MSEC:

      if (!UseRTOSMultitasking) {
        run10TimesPerSecond();
      }
      break;
    case IntervalTimer_e::TIMER_1SEC:             runOncePerSecond();      break;
    case IntervalTimer_e::TIMER_30SEC:            runEach30Seconds();      break;
    case IntervalTimer_e::TIMER_MQTT:
#ifdef USES_MQTT
      runPeriodicalMQTT();
#endif // USES_MQTT
      break;
    case IntervalTimer_e::TIMER_STATISTICS:       logTimerStatistics();    break;
    case IntervalTimer_e::TIMER_GRATUITOUS_ARP:

      // Slowly increase the interval timer.
      timer_gratuitous_arp_interval = 2 * timer_gratuitous_arp_interval;

      if (timer_gratuitous_arp_interval > TIMER_GRATUITOUS_ARP_MAX) {
        timer_gratuitous_arp_interval = TIMER_GRATUITOUS_ARP_MAX;
      }

      if (Settings.gratuitousARP()) {
        sendGratuitousARP();
      }
      break;
    case IntervalTimer_e::TIMER_MQTT_DELAY_QUEUE:
#ifdef USES_MQTT
      processMQTTdelayQueue();
#endif // USES_MQTT
      break;
    case IntervalTimer_e::TIMER_C001_DELAY_QUEUE:
  #ifdef USES_C001
      process_c001_delay_queue();
  #endif // ifdef USES_C001
      break;
    case IntervalTimer_e::TIMER_C003_DELAY_QUEUE:
  #ifdef USES_C003
      process_c003_delay_queue();
  #endif // ifdef USES_C003
      break;
    case IntervalTimer_e::TIMER_C004_DELAY_QUEUE:
  #ifdef USES_C004
      process_c004_delay_queue();
  #endif // ifdef USES_C004
      break;
    case IntervalTimer_e::TIMER_C007_DELAY_QUEUE:
  #ifdef USES_C007
      process_c007_delay_queue();
  #endif // ifdef USES_C007
      break;
    case IntervalTimer_e::TIMER_C008_DELAY_QUEUE:
  #ifdef USES_C008
      process_c008_delay_queue();
  #endif // ifdef USES_C008
      break;
    case IntervalTimer_e::TIMER_C009_DELAY_QUEUE:
  #ifdef USES_C009
      process_c009_delay_queue();
  #endif // ifdef USES_C009
      break;
    case IntervalTimer_e::TIMER_C010_DELAY_QUEUE:
  #ifdef USES_C010
      process_c010_delay_queue();
  #endif // ifdef USES_C010
      break;
    case IntervalTimer_e::TIMER_C011_DELAY_QUEUE:
  #ifdef USES_C011
      process_c011_delay_queue();
  #endif // ifdef USES_C011
      break;
    case IntervalTimer_e::TIMER_C012_DELAY_QUEUE:
  #ifdef USES_C012
      process_c012_delay_queue();
  #endif // ifdef USES_C012
      break;

    case IntervalTimer_e::TIMER_C013_DELAY_QUEUE:
      /*
       #ifdef USES_C013
            process_c013_delay_queue();
       #endif
       */
      break;

    case IntervalTimer_e::TIMER_C014_DELAY_QUEUE:
      /*
       #ifdef USES_C014
            process_c014_delay_queue();
       #endif
       */
      break;

    case IntervalTimer_e::TIMER_C015_DELAY_QUEUE:
  #ifdef USES_C015
      process_c015_delay_queue();
  #endif // ifdef USES_C015
      break;
    case IntervalTimer_e::TIMER_C016_DELAY_QUEUE:
  #ifdef USES_C016
      process_c016_delay_queue();
  #endif // ifdef USES_C016
      break;

    case IntervalTimer_e::TIMER_C017_DELAY_QUEUE:
  #ifdef USES_C017
      process_c017_delay_queue();
  #endif // ifdef USES_C017
      break;

    case IntervalTimer_e::TIMER_C018_DELAY_QUEUE:
  #ifdef USES_C018
      process_c018_delay_queue();
  #endif // ifdef USES_C018
      break;

    case IntervalTimer_e::TIMER_C019_DELAY_QUEUE:
      /*
       #ifdef USES_C019
            process_c019_delay_queue();
       #endif
       */
      break;

    case IntervalTimer_e::TIMER_C020_DELAY_QUEUE:
      /*
       #ifdef USES_C020
            process_c020_delay_queue();
       #endif
       */
      break;

    case IntervalTimer_e::TIMER_C021_DELAY_QUEUE:
      /*
       #ifdef USES_C021
            process_c021_delay_queue();
       #endif
       */
      break;

    case IntervalTimer_e::TIMER_C022_DELAY_QUEUE:
      /*
       #ifdef USES_C022
            process_c022_delay_queue();
       #endif
       */
      break;

    case IntervalTimer_e::TIMER_C023_DELAY_QUEUE:
      /*
       #ifdef USES_C023
            process_c023_delay_queue();
       #endif
       */
      break;

    case IntervalTimer_e::TIMER_C024_DELAY_QUEUE:
      /*
       #ifdef USES_C024
            process_c024_delay_queue();
       #endif
       */
      break;

    case IntervalTimer_e::TIMER_C025_DELAY_QUEUE:
      /*
       #ifdef USES_C025
            process_c025_delay_queue();
       #endif
       */
      break;

    // When extending this, search for EXTEND_CONTROLLER_IDS 
    // in the code to find all places that need to be updated too.
  }
}

/*********************************************************************************************\
* Plugin Task Timer
\*********************************************************************************************/
unsigned long ESPEasy_Scheduler::createPluginTaskTimerId(deviceIndex_t deviceIndex, int Par1) {
  const unsigned long mask  = (1 << TIMER_ID_SHIFT) - 1;
  const unsigned long mixed = (Par1 << 8) + deviceIndex;

  return mixed & mask;
}

void ESPEasy_Scheduler::setPluginTaskTimer(unsigned long msecFromNow, taskIndex_t taskIndex, int Par1, int Par2, int Par3, int Par4, int Par5)
{
  // plugin number and par1 form a unique key that can be used to restart a timer
  // Use deviceIndex instead of pluginID, since the deviceIndex uses less bits.
  const deviceIndex_t deviceIndex = getDeviceIndex_from_TaskIndex(taskIndex);

  if (!validDeviceIndex(deviceIndex)) { return; }

  const unsigned long mixedTimerId = getMixedId(PLUGIN_TASK_TIMER, createPluginTaskTimerId(deviceIndex, Par1));

  systemTimerStruct timer_data;

  timer_data.TaskIndex       = taskIndex;
  timer_data.Par1            = Par1;
  timer_data.Par2            = Par2;
  timer_data.Par3            = Par3;
  timer_data.Par4            = Par4;
  timer_data.Par5            = Par5;
  systemTimers[mixedTimerId] = timer_data;
  setNewTimerAt(mixedTimerId, millis() + msecFromNow);
}

void ESPEasy_Scheduler::process_plugin_task_timer(unsigned long id) {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  START_TIMER;

  const unsigned long mixedTimerId = getMixedId(PLUGIN_TASK_TIMER, id);
  auto it                          = systemTimers.find(mixedTimerId);

  if (it == systemTimers.end()) { return; }

  const deviceIndex_t deviceIndex = getDeviceIndex_from_TaskIndex(it->second.TaskIndex);

  struct EventStruct TempEvent(it->second.TaskIndex);
  TempEvent.Par1         = it->second.Par1;
  TempEvent.Par2         = it->second.Par2;
  TempEvent.Par3         = it->second.Par3;
  TempEvent.Par4         = it->second.Par4;
  TempEvent.Par5         = it->second.Par5;

  // TD-er: Not sure if we have to keep original source for notifications.
  TempEvent.Source = EventValueSource::Enum::VALUE_SOURCE_SYSTEM;


  /*
     String log = F("proc_system_timer: Pluginid: ");
     log += deviceIndex;
     log += F(" taskIndex: ");
     log += it->second.TaskIndex;
     log += F(" sysTimerID: ");
     log += id;
     addLog(LOG_LEVEL_INFO, log);
   */
  systemTimers.erase(mixedTimerId);

  if (validDeviceIndex(deviceIndex)) {
    if (validUserVarIndex(TempEvent.BaseVarIndex)) {
      //checkDeviceVTypeForTask(&TempEvent);
      String dummy;
      Plugin_ptr[deviceIndex](PLUGIN_TIMER_IN, &TempEvent, dummy);
    }
  }
  STOP_TIMER(PROC_SYS_TIMER);
}

/*********************************************************************************************\
* Rules Timer
\*********************************************************************************************/
unsigned long ESPEasy_Scheduler::createRulesTimerId(unsigned int timerIndex) {
  const unsigned long mask  = (1 << TIMER_ID_SHIFT) - 1;
  const unsigned long mixed = timerIndex;

  return mixed & mask;
}

static bool checkRulesTimerIndex(unsigned int timerIndex) {
  if ((timerIndex > RULES_TIMER_MAX) || (timerIndex == 0)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("TIMER: invalid timer number ");
      log += timerIndex;
      addLog(LOG_LEVEL_ERROR, log);
    }
    return false;
  }
  return true;
}

bool ESPEasy_Scheduler::setRulesTimer(unsigned long msecFromNow, unsigned int timerIndex, int recurringCount) {
  if (!checkRulesTimerIndex(timerIndex)) { return false; }

  const unsigned long mixedTimerId = getMixedId(RULES_TIMER, createRulesTimerId(timerIndex));
  const systemTimerStruct timer_data(recurringCount, msecFromNow, timerIndex);

  systemTimers[mixedTimerId] = timer_data;
  setNewTimerAt(mixedTimerId, millis() + msecFromNow);
  return true;
}

void ESPEasy_Scheduler::process_rules_timer(unsigned long id, unsigned long lasttimer) {
  const unsigned long mixedTimerId = getMixedId(RULES_TIMER, id);

  auto it = systemTimers.find(mixedTimerId);

  if (it == systemTimers.end()) { return; }

  if (it->second.isPaused()) {
    // Timer is paused.
    // Must keep this timer 'active' in the scheduler.
    // However it does not need to be looked at frequently as the resume function re-schedules it when needed.
    // Look for its state every second.
    setNewTimerAt(mixedTimerId, millis() + 1000);
    return;
  }

  // Create a deep copy of the timer data as we may delete it from the map before sending the event.
  const int loopCount  = it->second.getLoopCount();
  const int timerIndex = it->second.getTimerIndex();

  // Reschedule before sending the event, as it may get rescheduled in handling the timer event.
  if (it->second.isRecurring()) {
    // Recurring timer
    unsigned long newTimer = lasttimer;
    setNextTimeInterval(newTimer, it->second.getInterval());
    setNewTimerAt(mixedTimerId, newTimer);
    it->second.markNextRecurring();
  } else {
    systemTimers.erase(mixedTimerId);
  }

  if (loopCount > 0) {
    // Should be executed
    if (Settings.UseRules) {
      String event = F("Rules#Timer=");
      event += timerIndex;

      // Add count as 2nd eventvalue
      event += ',';
      event += loopCount;
      rulesProcessing(event); // TD-er: Do not add to the eventQueue, but execute right now.
    }
  }
}

bool ESPEasy_Scheduler::pause_rules_timer(unsigned long timerIndex) {
  if (!checkRulesTimerIndex(timerIndex)) { return false; }
  const unsigned long mixedTimerId = getMixedId(RULES_TIMER, createRulesTimerId(timerIndex));
  auto it                          = systemTimers.find(mixedTimerId);

  if (it == systemTimers.end()) {
    addLog(LOG_LEVEL_INFO, F("TIMER: no timer set"));
    return false;
  }

  unsigned long timer;

  if (msecTimerHandler.getTimerForId(mixedTimerId, timer)) {
    if (it->second.isPaused()) {
      addLog(LOG_LEVEL_INFO, F("TIMER: already paused"));
    } else {
      // Store remainder of interval
      const long timeLeft = timePassedSince(timer) * -1;

      if (timeLeft > 0) {
        it->second.setRemainder(timeLeft);
        return true;
      }
    }
  }
  return false;
}

bool ESPEasy_Scheduler::resume_rules_timer(unsigned long timerIndex) {
  if (!checkRulesTimerIndex(timerIndex)) { return false; }
  const unsigned long mixedTimerId = getMixedId(RULES_TIMER, createRulesTimerId(timerIndex));
  auto it                          = systemTimers.find(mixedTimerId);

  if (it == systemTimers.end()) { return false; }

  unsigned long timer;

  if (msecTimerHandler.getTimerForId(mixedTimerId, timer)) {
    if (it->second.isPaused()) {
      // Reschedule timer with remainder of interval
      setNewTimerAt(mixedTimerId, millis() + it->second.getRemainder());
      it->second.setRemainder(0);
      return true;
    }
  }
  return false;
}

/*********************************************************************************************\
* Plugin Timer
\*********************************************************************************************/
unsigned long ESPEasy_Scheduler::createPluginTimerId(deviceIndex_t deviceIndex, int Par1) {
  const unsigned long mask  = (1 << TIMER_ID_SHIFT) - 1;
  const unsigned long mixed = (Par1 << 8) + deviceIndex;

  return mixed & mask;
}

void ESPEasy_Scheduler::setPluginTimer(unsigned long msecFromNow, pluginID_t pluginID, int Par1, int Par2, int Par3, int Par4, int Par5)
{
  // plugin number and par1 form a unique key that can be used to restart a timer
  // Use deviceIndex instead of pluginID, since the deviceIndex uses less bits.
  const deviceIndex_t deviceIndex = getDeviceIndex(pluginID);

  if (!validDeviceIndex(deviceIndex)) { return; }

  const unsigned long mixedTimerId = getMixedId(PLUGIN_TIMER, createPluginTimerId(deviceIndex, Par1));
  systemTimerStruct   timer_data;

  // PLUGIN_TIMER does not address a task, so don't set TaskIndex
  timer_data.Par1            = Par1;
  timer_data.Par2            = Par2;
  timer_data.Par3            = Par3;
  timer_data.Par4            = Par4;
  timer_data.Par5            = Par5;
  systemTimers[mixedTimerId] = timer_data;
  setNewTimerAt(mixedTimerId, millis() + msecFromNow);
}

void ESPEasy_Scheduler::process_plugin_timer(unsigned long id) {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  START_TIMER;
  const unsigned long mixedTimerId = getMixedId(PLUGIN_TIMER, id);
  auto it                          = systemTimers.find(mixedTimerId);

  if (it == systemTimers.end()) { return; }

  struct EventStruct TempEvent;
  // PLUGIN_TIMER does not address a task, so don't set TaskIndex

  // extract deviceID from timer id:
  const deviceIndex_t deviceIndex = ((1 << 8) - 1) & id;

  TempEvent.Par1 = it->second.Par1;
  TempEvent.Par2 = it->second.Par2;
  TempEvent.Par3 = it->second.Par3;
  TempEvent.Par4 = it->second.Par4;
  TempEvent.Par5 = it->second.Par5;

  // TD-er: Not sure if we have to keep original source for notifications.
  TempEvent.Source = EventValueSource::Enum::VALUE_SOURCE_SYSTEM;

  //  const deviceIndex_t deviceIndex = getDeviceIndex_from_TaskIndex(it->second.TaskIndex);

  /*
     String log = F("proc_system_timer: Pluginid: ");
     log += deviceIndex;
     log += F(" taskIndex: ");
     log += it->second.TaskIndex;
     log += F(" sysTimerID: ");
     log += id;
     addLog(LOG_LEVEL_INFO, log);
   */
  systemTimers.erase(mixedTimerId);

  if (validDeviceIndex(deviceIndex)) {
    String dummy;
    Plugin_ptr[deviceIndex](PLUGIN_ONLY_TIMER_IN, &TempEvent, dummy);
  }
  STOP_TIMER(PROC_SYS_TIMER);
}

/*********************************************************************************************\
* GPIO Timer
* Special timer to handle timed GPIO actions
\*********************************************************************************************/
unsigned long ESPEasy_Scheduler::createGPIOTimerId(uint8_t GPIOType, uint8_t pinNumber, int Par1) {
  const unsigned long mask = (1 << TIMER_ID_SHIFT) - 1;

  //  const unsigned long mixed = (Par1 << 8) + pinNumber;
  const unsigned long mixed = (Par1 << 16) + (pinNumber << 8) + GPIOType;

  return mixed & mask;
}

void ESPEasy_Scheduler::setGPIOTimer(unsigned long msecFromNow, pluginID_t pluginID, int Par1, int Par2, int Par3, int Par4, int Par5)
{
  uint8_t GPIOType = GPIO_TYPE_INVALID;

  switch (pluginID) {
    case PLUGIN_GPIO:
      GPIOType = GPIO_TYPE_INTERNAL;
      break;
    case PLUGIN_PCF:
      GPIOType = GPIO_TYPE_PCF;
      break;
    case PLUGIN_MCP:
      GPIOType = GPIO_TYPE_MCP;
      break;
  }

  if (GPIOType != GPIO_TYPE_INVALID) {
    // Par1 & Par2 & GPIOType form a unique key
    const unsigned long mixedTimerId = getMixedId(GPIO_TIMER, createGPIOTimerId(GPIOType, Par1, Par2));
    setNewTimerAt(mixedTimerId, millis() + msecFromNow);
  }
}


void ESPEasy_Scheduler::process_gpio_timer(unsigned long id) {
  uint8_t GPIOType = static_cast<uint8_t>((id) & 0xFF);
  uint8_t pinNumber = static_cast<uint8_t>((id >> 8) & 0xFF);
  uint8_t pinStateValue = static_cast<uint8_t>((id >> 16) & 0xFF);

  bool success = true;

  uint8_t pluginID;

  switch (GPIOType)
  {
    case GPIO_TYPE_INTERNAL:
      GPIO_Internal_Write(pinNumber, pinStateValue);
      pluginID=PLUGIN_GPIO;
      break;
    case GPIO_TYPE_MCP:
      GPIO_MCP_Write(pinNumber, pinStateValue);
      pluginID=PLUGIN_MCP;
      break;
    case GPIO_TYPE_PCF:
      GPIO_PCF_Write(pinNumber, pinStateValue);
      pluginID=PLUGIN_PCF;
      break;
    default:
      success=false;
  }
  if (success) {
    const uint32_t key = createKey(pluginID, pinNumber);
    // WARNING: operator [] creates an entry in the map if key does not exist
    portStatusStruct tempStatus = globalMapPortStatus[key];

    tempStatus.mode     = PIN_MODE_OUTPUT;
    tempStatus.command  = 1; //set to 1 in order to display the status in the PinStatus page

    if (tempStatus.state != pinStateValue) {
      tempStatus.state        = pinStateValue;
      tempStatus.output       = pinStateValue;
      tempStatus.forceEvent   = 1;
      tempStatus.forceMonitor = 1;
    }
    savePortStatus(key,tempStatus);
  }
}

/*********************************************************************************************\
* Task Device Timer
* This is the interval set in a plugin to get a new reading.
* These timers will re-schedule themselves as long as the plugin task is enabled.
* When the plugin task is initialized, a call to schedule_task_device_timer_at_init
* will bootstrap this sequence.
\*********************************************************************************************/
void ESPEasy_Scheduler::schedule_task_device_timer_at_init(unsigned long task_index) {
  unsigned long runAt = millis();

  if (!isDeepSleepEnabled()) {
    // Deepsleep is not enabled, add some offset based on the task index
    // to make sure not all are run at the same time.
    // This scheduled time may be overriden by the plugin's own init.
    runAt += (task_index * 37) + 100;
  } else {
    runAt += (task_index * 11) + 10;
  }
  schedule_task_device_timer(task_index, runAt);
}

// Typical use case is to run this when all needed connections are made.
void ESPEasy_Scheduler::schedule_all_task_device_timers() {
  for (taskIndex_t task = 0; task < TASKS_MAX; task++) {
    schedule_task_device_timer_at_init(task);
  }
}

void ESPEasy_Scheduler::schedule_task_device_timer(unsigned long task_index, unsigned long runAt) {
  /*
     String log = F("schedule_task_device_timer: task: ");
     log += task_index;
     log += F(" @ ");
     log += runAt;
     if (Settings.TaskDeviceEnabled[task_index]) {
      log += F(" (enabled)");
     }
     addLog(LOG_LEVEL_INFO, log);
   */

  if (!validTaskIndex(task_index)) { return; }

  const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(task_index);

  if (!validDeviceIndex(DeviceIndex)) { return; }

  // TD-er: Tasks without a timer or optional timer set to 0 should still be able to call PLUGIN_READ
  // For example to schedule a read from the PLUGIN_TEN_PER_SECOND when a new value is ready.

  /*
     if (!Device[DeviceIndex].TimerOption) { return; }

     if (Device[DeviceIndex].TimerOptional && (Settings.TaskDeviceTimer[task_index] == 0)) {
      return;
     }
   */

  if (Settings.TaskDeviceEnabled[task_index]) {
    setNewTimerAt(getMixedId(TASK_DEVICE_TIMER, task_index), runAt);
  }
}

void ESPEasy_Scheduler::reschedule_task_device_timer(unsigned long task_index, unsigned long lasttimer) {
  if (!validTaskIndex(task_index)) { return; }
  unsigned long newtimer = Settings.TaskDeviceTimer[task_index];

  if (newtimer != 0) {
    newtimer = lasttimer + (newtimer * 1000);
    schedule_task_device_timer(task_index, newtimer);
  }
}

void ESPEasy_Scheduler::process_task_device_timer(unsigned long task_index, unsigned long lasttimer) {
  if (!validTaskIndex(task_index)) { return; }
  reschedule_task_device_timer(task_index, lasttimer);
  START_TIMER;
  SensorSendTask(task_index);
  STOP_TIMER(SENSOR_SEND_TASK);
}

/*********************************************************************************************\
* System Event Timer
* Handling of these events will be asynchronous and being called from the loop().
* Thus only use these when the result is not needed immediately.
* Proper use case is calling from a callback function, since those cannot use yield() or delay()
\*********************************************************************************************/
void ESPEasy_Scheduler::schedule_plugin_task_event_timer(deviceIndex_t DeviceIndex, uint8_t Function, struct EventStruct &&event) {
  if (validDeviceIndex(DeviceIndex)) {
    schedule_event_timer(PluginPtrType::TaskPlugin, DeviceIndex, Function, std::move(event));
  }
}

void ESPEasy_Scheduler::schedule_mqtt_plugin_import_event_timer(deviceIndex_t   DeviceIndex,
                                                                taskIndex_t     TaskIndex,
                                                                uint8_t            Function,
                                                                char           *c_topic,
                                                                uint8_t           *b_payload,
                                                                unsigned int    length) {
  if (validDeviceIndex(DeviceIndex)) {
    const unsigned long mixedId = createSystemEventMixedId(PluginPtrType::TaskPlugin, DeviceIndex, static_cast<uint8_t>(Function));
    EventStruct event(TaskIndex);
    const size_t topic_length = strlen_P(c_topic);
    if (!(event.String1.reserve(topic_length) && event.String2.reserve(length))) {
      addLog(LOG_LEVEL_ERROR, F("MQTT : Out of Memory! Cannot process MQTT message"));
      return;
    }
    for (size_t i = 0; i < topic_length; ++i) {
      event.String1 += c_topic[i];
    }
    for (unsigned int i = 0; i < length; ++i) {
      const char c = static_cast<char>(*(b_payload + i));
      event.String2 += c;
    }
    // Emplace using move.
    // This makes sure the relatively large event will not be in memory twice.
    ScheduledEventQueue.emplace_back(mixedId, std::move(event));
  }
}

void ESPEasy_Scheduler::schedule_controller_event_timer(protocolIndex_t ProtocolIndex, uint8_t Function, struct EventStruct &&event) {
  if (validProtocolIndex(ProtocolIndex)) {
    schedule_event_timer(PluginPtrType::ControllerPlugin, ProtocolIndex, Function, std::move(event));
  }
}

unsigned long ESPEasy_Scheduler::createSystemEventMixedId(PluginPtrType ptr_type, uint16_t crc16) {
  unsigned long subId = static_cast<unsigned long>(ptr_type);

  subId = (subId << 16) + crc16;
  return getMixedId(SYSTEM_EVENT_QUEUE, subId);
}

unsigned long ESPEasy_Scheduler::createSystemEventMixedId(PluginPtrType ptr_type, uint8_t Index, uint8_t Function) {
  unsigned long subId = static_cast<unsigned long>(ptr_type);

  subId = (subId << 8) + Index;
  subId = (subId << 8) + Function;
  return getMixedId(SYSTEM_EVENT_QUEUE, subId);
}

void ESPEasy_Scheduler::schedule_mqtt_controller_event_timer(protocolIndex_t ProtocolIndex,
                                                             CPlugin::Function Function,
                                                             char           *c_topic,
                                                             uint8_t           *b_payload,
                                                             unsigned int    length) {
  if (validProtocolIndex(ProtocolIndex)) {
    // Emplace empty event in the queue first and the fill it.
    // This makes sure the relatively large event will not be in memory twice.
    const unsigned long mixedId = createSystemEventMixedId(PluginPtrType::ControllerPlugin, ProtocolIndex, static_cast<uint8_t>(Function));
    ScheduledEventQueue.emplace_back(mixedId, EventStruct());
    ScheduledEventQueue.back().event.String1 = c_topic;

    String& payload = ScheduledEventQueue.back().event.String2;
    if (!payload.reserve(length)) {
      addLog(LOG_LEVEL_ERROR, F("MQTT : Out of Memory! Cannot process MQTT message"));
    }

    for (unsigned int i = 0; i < length; ++i) {
      char c = static_cast<char>(*(b_payload + i));
      payload += c;
    }
  }
}

void ESPEasy_Scheduler::schedule_notification_event_timer(uint8_t NotificationProtocolIndex, NPlugin::Function Function, struct EventStruct &&event) {
  schedule_event_timer(PluginPtrType::NotificationPlugin, NotificationProtocolIndex, static_cast<uint8_t>(Function), std::move(event));
}

void ESPEasy_Scheduler::schedule_event_timer(PluginPtrType ptr_type, uint8_t Index, uint8_t Function, struct EventStruct &&event) {
  const unsigned long mixedId = createSystemEventMixedId(ptr_type, Index, Function);

  //  EventStructCommandWrapper eventWrapper(mixedId, *event);
  //  ScheduledEventQueue.push_back(eventWrapper);
  ScheduledEventQueue.emplace_back(mixedId, std::move(event));
}

void ESPEasy_Scheduler::process_system_event_queue() {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  if (ScheduledEventQueue.size() == 0) { return; }
  unsigned long id       = ScheduledEventQueue.front().id;
  uint8_t Function          = id & 0xFF;
  uint8_t Index             = (id >> 8) & 0xFF;
  PluginPtrType ptr_type = static_cast<PluginPtrType>((id >> 16) & 0xFF);

  // At this moment, the String is not being used in the plugin calls, so just supply a dummy String.
  // Also since these events will be processed asynchronous, the resulting
  //   output in the String is probably of no use elsewhere.
  // Else the line string could be used.
  String tmpString;

  switch (ptr_type) {
    case PluginPtrType::TaskPlugin:
      if (validDeviceIndex(Index)) {
        LoadTaskSettings(ScheduledEventQueue.front().event.TaskIndex);
        Plugin_ptr[Index](Function, &ScheduledEventQueue.front().event, tmpString);
      }
      break;
    case PluginPtrType::ControllerPlugin:
      CPluginCall(Index, static_cast<CPlugin::Function>(Function), &ScheduledEventQueue.front().event, tmpString);
      break;
    case PluginPtrType::NotificationPlugin:
      NPlugin_ptr[Index](static_cast<NPlugin::Function>(Function), &ScheduledEventQueue.front().event, tmpString);
      break;
  }
  ScheduledEventQueue.pop_front();
}

String ESPEasy_Scheduler::getQueueStats() {
  return msecTimerHandler.getQueueStats();
}

void ESPEasy_Scheduler::updateIdleTimeStats() {
  msecTimerHandler.updateIdleTimeStats();
}

float ESPEasy_Scheduler::getIdleTimePct() const {
  return msecTimerHandler.getIdleTimePct();
}

void ESPEasy_Scheduler::setEcoMode(bool enabled) {
  msecTimerHandler.setEcoMode(enabled);
}
