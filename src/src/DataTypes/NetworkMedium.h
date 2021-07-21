#ifndef ESPEASY_WTH_WIFI_H_
#define ESPEASY_WTH_WIFI_H_

#include <Arduino.h>

// Is stored in settings
enum class NetworkMedium_t : uint8_t {
  WIFI            = 0,
  Ethernet        = 1,
  ESPEasyNOW_only = 2,
  NotSet          = 255
};

bool   isValid(NetworkMedium_t medium);

const __FlashStringHelper * toString(NetworkMedium_t medium);


#endif // ESPEASY_WTH_WIFI_H_
