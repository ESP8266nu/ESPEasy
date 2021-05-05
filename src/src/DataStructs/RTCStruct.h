#ifndef DATASTRUCTS_RTC_STRUCTS_H
#define DATASTRUCTS_RTC_STRUCTS_H

#include "../../ESPEasy_common.h"

// this offsets are in blocks, bytes = blocks * 4
#define RTC_BASE_STRUCT 64
#define RTC_BASE_USERVAR 74
#define RTC_BASE_CACHE 124

#define RTC_CACHE_DATA_SIZE 240
#define CACHE_FILE_MAX_SIZE 24000

/*********************************************************************************************\
 * RTCStruct
\*********************************************************************************************/
//max 40 bytes: ( 74 - 64 ) * 4
struct RTCStruct
{
  RTCStruct() : ID1(0), ID2(0), lastWiFiChannel(0), factoryResetCounter(0),
                deepSleepState(0), bootFailedCount(0), flashDayCounter(0),
                lastWiFiSettingsIndex(0),
                flashCounter(0), bootCounter(0), lastMixedSchedulerId(0),
                repeatResetCount(0), repeatResetMarker(0),
                lastSysTime(0) {}

  void init() {
    ID1 = 0xAA;
    ID2 = 0x55;
    clearLastWiFi();
    factoryResetCounter = 0;
    deepSleepState = 0;
    bootFailedCount = 0;
    flashDayCounter = 0;
    lastWiFiSettingsIndex = 0;
    flashCounter = 0;
    bootCounter = 0;
    lastMixedSchedulerId = 0;
    repeatResetCount = 0;
    repeatResetMarker = 0;
    lastSysTime = 0;
  }

  void clearLastWiFi() {
    for (byte i = 0; i < 6; ++i) {
      lastBSSID[i] = 0;
    }
    lastWiFiChannel = 0;
    lastWiFiSettingsIndex = 0;
  }

  bool lastWiFi_set() const {
    return lastBSSID[0] != 0 && lastWiFiChannel != 0 && lastWiFiSettingsIndex != 0;
  }


  byte ID1;
  byte ID2;
  byte lastWiFiChannel;
  byte factoryResetCounter;
  byte deepSleepState;
  byte bootFailedCount;
  byte flashDayCounter;
  byte lastWiFiSettingsIndex;
  unsigned long flashCounter;
  unsigned long bootCounter;
  unsigned long lastMixedSchedulerId;
  uint8_t lastBSSID[6] = { 0 };
  byte repeatResetCount;
  byte repeatResetMarker;
  unsigned long lastSysTime;
};


#endif // DATASTRUCTS_RTC_STRUCTS_H
