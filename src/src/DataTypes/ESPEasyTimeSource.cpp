#include "ESPEasyTimeSource.h"

#include <Arduino.h>

#include "../../ESPEasy_common.h"


const __FlashStringHelper* toString(timeSource_t timeSource)
{
  switch (timeSource) {
    case timeSource_t::GPS_PPS_time_source:     return F("GPS PPS");
    case timeSource_t::GPS_time_source:         return F("GPS");
    case timeSource_t::NTP_time_source:         return F("NTP");
    case timeSource_t::Manual_set:              return F("Manual");
    case timeSource_t::ESP_now_peer:            return F(ESPEASY_NOW_NAME " peer");
    case timeSource_t::Restore_RTC_time_source: return F("RTC at boot");
    case timeSource_t::No_time_source:          return F("No time set");
  }
  return F("Unknown");
}

bool isExternalTimeSource(timeSource_t timeSource)
{
  switch (timeSource) {
    case timeSource_t::GPS_PPS_time_source:
    case timeSource_t::GPS_time_source:
    case timeSource_t::NTP_time_source:
    case timeSource_t::Manual_set:
      return true;
    default:
      return false;
  }
}
