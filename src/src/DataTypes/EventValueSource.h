#ifndef DATATYPES_EVENT_VALUE_SOURCE_H
#define DATATYPES_EVENT_VALUE_SOURCE_H

#include <Arduino.h>

struct EventValueSourceGroup {
  enum class Enum : uint8_t {
    RESTRICTED,
    ALL
  };
};


struct EventValueSource {
  // Keep the values as they can be used by other/older builds to communicate with ESPEasy
  enum class Enum : uint8_t {
    VALUE_SOURCE_NOT_SET      = 0,
    VALUE_SOURCE_SYSTEM       = 1,
    VALUE_SOURCE_SERIAL       = 2,
    VALUE_SOURCE_HTTP         = 3,
    VALUE_SOURCE_MQTT         = 4,
    VALUE_SOURCE_UDP          = 5,
    VALUE_SOURCE_WEB_FRONTEND = 6,
    VALUE_SOURCE_RULES        = 7,
    VALUE_SOURCE_ESPEASY_NOW  = 8,

    VALUE_SOURCE_NR_VALUES
  };

  static bool partOfGroup(EventValueSource::Enum source, EventValueSourceGroup::Enum group)
  {
    switch (source) {
      case EventValueSource::Enum::VALUE_SOURCE_NOT_SET:
      case EventValueSource::Enum::VALUE_SOURCE_NR_VALUES:
        return false;
      case EventValueSource::Enum::VALUE_SOURCE_SYSTEM:
      case EventValueSource::Enum::VALUE_SOURCE_SERIAL:
      case EventValueSource::Enum::VALUE_SOURCE_UDP:
      case EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND:
      case EventValueSource::Enum::VALUE_SOURCE_RULES:
        return true;
      case EventValueSource::Enum::VALUE_SOURCE_HTTP:
      case EventValueSource::Enum::VALUE_SOURCE_MQTT:
      case EventValueSource::Enum::VALUE_SOURCE_ESPEASY_NOW:
        return group == EventValueSourceGroup::Enum::ALL;
    }
    return false;
  }
};


#endif // DATATYPES_EVENT_VALUE_SOURCE_H
