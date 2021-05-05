#ifndef GLOBALS_STATISTICS_H
#define GLOBALS_STATISTICS_H

#include <stdint.h>

#include "../../ESPEasy_common.h"

class String;

#define BOOT_CAUSE_MANUAL_REBOOT            0
#define BOOT_CAUSE_COLD_BOOT                1
#define BOOT_CAUSE_DEEP_SLEEP               2
#define BOOT_CAUSE_EXT_WD                  10

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

extern volatile unsigned long sw_watchdog_callback_count;


#endif // GLOBALS_STATISTICS_H
