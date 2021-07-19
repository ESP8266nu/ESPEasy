#include "../Globals/Statistics.h"

#include <Arduino.h>

#ifndef BUILD_NO_RAM_TRACKER
uint32_t lowestRAM = 0;
String   lowestRAMfunction;
uint32_t lowestFreeStack = 0;
String   lowestFreeStackfunction;
#endif

uint8_t lastBootCause                           = BOOT_CAUSE_MANUAL_REBOOT;
unsigned long lastMixedSchedulerId_beforereboot = 0;

unsigned long idle_msec_per_sec = 0;
unsigned long elapsed10ps       = 0;
unsigned long elapsed10psU      = 0;
unsigned long elapsed50ps       = 0;
unsigned long loopCounter       = 0;
unsigned long loopCounterLast   = 0;
unsigned long loopCounterMax    = 1;
unsigned long lastLoopStart     = 0;
unsigned long shortestLoop      = 10000000;
unsigned long longestLoop       = 0;
unsigned long loopCounter_full  = 1;
float loop_usec_duration_total  = 0.0f;


unsigned long dailyResetCounter                   = 0;
volatile unsigned long sw_watchdog_callback_count = 0;


I2C_bus_state I2C_state = I2C_bus_state::OK;
unsigned long I2C_bus_cleared_count = 0;
