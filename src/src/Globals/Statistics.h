#ifndef GLOBALS_STATISTICS_H
#define GLOBALS_STATISTICS_H

#include <stdint.h>

#include "../../ESPEasy_common.h"

#include "../DataStructs/I2CTypes.h"

class String;


#define BOOT_CAUSE_MANUAL_REBOOT            0
#define BOOT_CAUSE_COLD_BOOT                1
#define BOOT_CAUSE_DEEP_SLEEP               2
#define BOOT_CAUSE_SOFT_RESTART             3
#define BOOT_CAUSE_EXT_WD                  10
#define BOOT_CAUSE_SW_WATCHDOG             11
#define BOOT_CAUSE_EXCEPTION               12
#define BOOT_CAUSE_POWER_UNSTABLE          20

#ifndef BUILD_NO_RAM_TRACKER
extern uint32_t lowestRAM;
extern String   lowestRAMfunction;
extern uint32_t lowestFreeStack;
extern String   lowestFreeStackfunction;
#endif

extern uint8_t lastBootCause;
extern unsigned long lastMixedSchedulerId_beforereboot;

extern unsigned long loopCounter;
extern unsigned long loopCounterLast;
extern unsigned long loopCounterMax;
extern unsigned long lastLoopStart;
extern unsigned long shortestLoop;
extern unsigned long longestLoop;
extern unsigned long loopCounter_full;
extern float loop_usec_duration_total;

extern unsigned long dailyResetCounter;
extern volatile unsigned long sw_watchdog_callback_count;


extern I2C_bus_state I2C_state;
extern unsigned long I2C_bus_cleared_count;


#endif // GLOBALS_STATISTICS_H
