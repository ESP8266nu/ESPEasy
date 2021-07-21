#ifndef DATASTRUCTS_ESPEASY_NOW_DUPLICATECHECK_H
#define DATASTRUCTS_ESPEASY_NOW_DUPLICATECHECK_H

#include <Arduino.h>

class ESPEasy_Now_DuplicateCheck {
public:

  enum class message_t : uint8_t {
    KeyToCheck       = 0,
    AlreadyProcessed = 1
  };

  ESPEasy_Now_DuplicateCheck();

  ESPEasy_Now_DuplicateCheck(uint32_t  key,
                             message_t message_type);

  const uint32_t _key;
  const message_t _type;
};


#endif // DATASTRUCTS_ESPEASY_NOW_DUPLICATECHECK_H
