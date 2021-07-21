#ifndef CONTROLLERQUEUE_C016_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_C016_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"
#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/DeviceStruct.h"
#include "../DataStructs/MessageRouteInfo.h"
#include "../Globals/Plugins.h"

struct EventStruct;


#ifdef USES_C016

/*********************************************************************************************\
* C016_queue_element for queueing requests for C016: Cached HTTP.
\*********************************************************************************************/

// TD-er: This one has a fixed uint8_t order and is stored.
// This also means the order of members should not be changed!
class C016_queue_element {
public:

  C016_queue_element();

  C016_queue_element(C016_queue_element&& other);

  C016_queue_element(const struct EventStruct *event,
                     uint8_t                      value_count,
                     unsigned long             unixTime);

  C016_queue_element& operator=(C016_queue_element&& other);


  size_t getSize() const;

  bool isDuplicate(const C016_queue_element& other) const;

  const MessageRouteInfo_t* getMessageRouteInfo() const { return nullptr; }

  float values[VARS_PER_TASK] = { 0 };
  unsigned long _timestamp    = 0; // Unix timestamp
  taskIndex_t TaskIndex       = INVALID_TASK_INDEX;
  controllerIndex_t controller_idx = INVALID_CONTROLLER_INDEX;
  Sensor_VType sensorType     = Sensor_VType::SENSOR_TYPE_NONE;
  uint8_t valueCount             = 0;
};

#endif //USES_C016


#endif // CONTROLLERQUEUE_C016_QUEUE_ELEMENT_H
