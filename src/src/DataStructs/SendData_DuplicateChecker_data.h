#ifndef DATASTRUCTS_SENDDATA_DUPLICATECHECKER_DATA_H
#define DATASTRUCTS_SENDDATA_DUPLICATECHECKER_DATA_H

#include "ESPEasy_EventStruct.h"

class SendData_DuplicateChecker_data {
public:

  SendData_DuplicateChecker_data(struct EventStruct *event);

  bool doSend();

private:

  EventStruct _event;
  unsigned long _timestamp = millis();
};


#endif // DATASTRUCTS_SENDDATA_DUPLICATECHECKER_DATA_H
