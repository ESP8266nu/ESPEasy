#ifndef DATATYPES_ESPEASYTIMESOURCE_H
#define DATATYPES_ESPEASYTIMESOURCE_H


#include <stdint.h>
#include <Arduino.h>

class String;

// Time Source type, sort by priority.
// Enum values are sent via NodeStruct, so only add new ones and don't change existing values
// typical time wander of an ESP module is 0.04 msec/sec, or roughly 3.5 seconds per 24h.
enum class timeSource_t : uint8_t {
  // External time source  (considered more reliable)
  GPS_PPS_time_source = 5,      // 1 - 10 msec accuracy
  GPS_time_source     = 10,     // 10 - 100 msec accuracy
  NTP_time_source     = 15,     // 20 - 100 msec accuracy

  // Manual override has higher priority because it is some kind of external sync
  Manual_set          = 20,     // Unknown accuracy

  // Sources which may drift over time due to lack of external synchronization.
  ESP_now_peer        = 40,     // < 5 msec accuracy between nodes, but time on the whole network may drift

  Restore_RTC_time_source = 50, // > 1 sec difference per reboot
  No_time_source          = 255 // No time set
};

const __FlashStringHelper* toString(timeSource_t timeSource);
bool isExternalTimeSource(timeSource_t timeSource);


#endif /* DATATYPES_ESPEASYTIMESOURCE_H */
