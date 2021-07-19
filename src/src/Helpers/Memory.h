#ifndef HELPERS_MEMORY_H
#define HELPERS_MEMORY_H

#include <Arduino.h>

#include "../../ESPEasy_common.h"

/*********************************************************************************************\
   Memory management
\*********************************************************************************************/


// For keeping track of 'cont' stack
// See: https://github.com/esp8266/Arduino/issues/2557
//      https://github.com/esp8266/Arduino/issues/5148#issuecomment-424329183
//      https://github.com/letscontrolit/ESPEasy/issues/1824
#ifdef ESP32

// FIXME TD-er: For ESP32 you need to provide the task number, or NULL to get from the calling task.
uint32_t getCurrentFreeStack();

uint32_t getFreeStackWatermark();

// FIXME TD-er: Must check if these functions are also needed for ESP32.
bool     canYield();

#else // ifdef ESP32

extern "C" {
# include <cont.h>
extern cont_t *g_pcont;
}

uint32_t getCurrentFreeStack();

uint32_t getFreeStackWatermark();

bool     canYield();

bool     allocatedOnStack(const void *address);

#endif // ESP32

/********************************************************************************************\
   Get free system mem
 \*********************************************************************************************/
unsigned long FreeMem();

#ifdef USE_SECOND_HEAP
unsigned long FreeMem2ndHeap();
#endif

unsigned long getMaxFreeBlock();


#endif