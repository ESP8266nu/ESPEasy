#ifndef HELPERS_PERIODICALACTIONS_H
#define HELPERS_PERIODICALACTIONS_H

#include <Arduino.h>
#include "../../ESPEasy_common.h"

#include "../DataStructs/MessageRouteInfo.h"
#include "../Globals/CPlugins.h"
#include "../Helpers/Scheduler.h"

/*********************************************************************************************\
 * Tasks that run 50 times per second
\*********************************************************************************************/

void run50TimesPerSecond();

/*********************************************************************************************\
 * Tasks that run 10 times per second
\*********************************************************************************************/
void run10TimesPerSecond();


/*********************************************************************************************\
 * Tasks each second
\*********************************************************************************************/
void runOncePerSecond();

/*********************************************************************************************\
 * Tasks each 30 seconds
\*********************************************************************************************/
void runEach30Seconds();

#ifdef USES_MQTT

void scheduleNextMQTTdelayQueue();
void schedule_all_tasks_using_MQTT_controller();

void processMQTTdelayQueue();

bool processMQTT_message(controllerIndex_t controllerIndex,
                        const String    & topic,
                        const String    & payload,
                        bool retained,
                        const MessageRouteInfo_t* messageRouteInfo);

void updateMQTTclient_connected();

void runPeriodicalMQTT();

controllerIndex_t firstEnabledMQTT_ControllerIndex();


#endif //USES_MQTT


void logTimerStatistics();

void updateLoopStats_30sec(uint8_t loglevel);

/********************************************************************************************\
   Clean up all before going to sleep or reboot.
 \*********************************************************************************************/
void prepareShutdown(ESPEasy_Scheduler::IntendedRebootReason_e reason);



#endif // HELPERS_PERIODICALACTIONS_H