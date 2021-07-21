#include "ESPEasy_Now_DuplicateCheck.h"

ESPEasy_Now_DuplicateCheck::ESPEasy_Now_DuplicateCheck()
  : _key(0), _type(ESPEasy_Now_DuplicateCheck::message_t::KeyToCheck) {}

ESPEasy_Now_DuplicateCheck::ESPEasy_Now_DuplicateCheck(uint32_t  key,
                                                       message_t message_type)
  : _key(key), _type(message_type) {}
