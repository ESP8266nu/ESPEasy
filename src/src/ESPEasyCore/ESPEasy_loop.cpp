#include "../ESPEasyCore/ESPEasy_loop.h"


#include "../../ESPEasy-Globals.h"
#include "../DataStructs/TimingStats.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi_ProcessEvent.h"
#include "../ESPEasyCore/ESPEasy_backgroundtasks.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/EventQueue.h"
#include "../Globals/RTC.h"
#include "../Globals/Settings.h"
#include "../Globals/Statistics.h"
#include "../Helpers/DeepSleep.h"
#include "../Helpers/ESPEasyRTC.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/I2C_access.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/PeriodicalActions.h"

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
  #endif // ifdef USES_TIMING_STATS

  loop_usec_duration_total += usecSince;
  lastLoopStart             = micros();

  if ((usecSince <= 0) || (usecSince > 10000000)) {
    return; // No loop should take > 10 sec.
  }

  if (shortestLoop > static_cast<unsigned long>(usecSince)) {
    shortestLoop   = usecSince;
    loopCounterMax = 30 * 1000000 / usecSince;
  }

  if (longestLoop < static_cast<unsigned long>(usecSince)) {
    longestLoop = usecSince;
  }
}

/*********************************************************************************************\
* MAIN LOOP
\*********************************************************************************************/
void ESPEasy_loop()
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
    firstLoop               = false;
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

  {
    // Check I2C bus to see if it needs to be cleared.
    // See: http://www.forward.com.au/pfod/ArduinoProgramming/I2C_ClearBus/index.html
    const I2C_bus_state I2C_state_prev = I2C_state;  
    I2C_state = I2C_check_bus(Settings.Pin_i2c_scl, Settings.Pin_i2c_sda);
    switch (I2C_state) {
      case I2C_bus_state::BusCleared:
        // Log I2C bus cleared, update stats
        ++I2C_bus_cleared_count;
        addLog(LOG_LEVEL_ERROR, F("I2C  : Cleared I2C bus error state"));
        I2C_state = I2C_bus_state::OK;
        initI2C();
        break;
      case I2C_bus_state::SCL_Low:
        addLog(LOG_LEVEL_ERROR, F("I2C  : I2C bus error, SCL clock line held low"));
        break;
      case I2C_bus_state::SDA_Low_over_2_sec:
        addLog(LOG_LEVEL_ERROR, F("I2C  : I2C bus error, SCL clock line held low by slave clock stretch for >2 sec"));
        break;
      case I2C_bus_state::SDA_Low_20_clocks:
        addLog(LOG_LEVEL_ERROR, F("I2C  : I2C bus error, SDA data line held low"));
        break;
      case I2C_bus_state::ClearingProcessActive:
        if (I2C_state_prev != I2C_state) {
          addLog(LOG_LEVEL_ERROR, F("I2C  : I2C bus error, start clearing process"));
        }
        break;
      case I2C_bus_state::NotConfigured:
      case I2C_bus_state::OK:
        break;
    }
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
#endif // USES_MQTT
    // Now run all frequent tasks
    run50TimesPerSecond();
    run10TimesPerSecond();
    runEach30Seconds();
    runOncePerSecond();
  }

  // normal mode, run each task when its time
  else
  {
    if (!UseRTOSMultitasking) {
      // On ESP32 the schedule is executed on the 2nd core.
      Scheduler.handle_schedule();
    }
  }

  backgroundtasks();

  if (readyForSleep()) {
    prepare_deepSleep(Settings.Delay);

    // deepsleep will never return, its a special kind of reboot
  }
}
