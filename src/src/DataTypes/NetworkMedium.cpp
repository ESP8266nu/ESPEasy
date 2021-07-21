#include "NetworkMedium.h"

#include "../../ESPEasy_common.h"

bool isValid(NetworkMedium_t medium) {
  switch (medium) {
    case NetworkMedium_t::WIFI:
    case NetworkMedium_t::Ethernet:
      return true;
    case NetworkMedium_t::ESPEasyNOW_only:
      #ifdef USES_ESPEASY_NOW
      return true;
      #else
      return false;
      #endif

    case NetworkMedium_t::NotSet:
      return false;

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return false;
}

const __FlashStringHelper * toString(NetworkMedium_t medium) {
  switch (medium) {
    case NetworkMedium_t::WIFI:     return F("WiFi");
    case NetworkMedium_t::Ethernet: return F("Ethernet");
    case NetworkMedium_t::ESPEasyNOW_only:  return F(ESPEASY_NOW_NAME " only");
    case NetworkMedium_t::NotSet:   return F("Not Set");

      // Do not use default: as this allows the compiler to detect any missing cases.
  }
  return F("Unknown");
}
