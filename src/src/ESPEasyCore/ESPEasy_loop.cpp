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
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif
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
