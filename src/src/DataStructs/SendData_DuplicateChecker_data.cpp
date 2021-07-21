#include "SendData_DuplicateChecker_data.h"

#include "../ESPEasyCore/Controller.h"
#include "../Helpers/ESPEasy_time_calc.h"

#define TIMEOUT_ASK_FOR_DUPLICATE 100

SendData_DuplicateChecker_data::SendData_DuplicateChecker_data(EventStruct *event) {
  _event.deep_copy(event);
}

bool SendData_DuplicateChecker_data::doSend()
{
  if (timePassedSince(_timestamp) > TIMEOUT_ASK_FOR_DUPLICATE) {
    sendData(&_event);
    return true;
  }
  return false;
}
