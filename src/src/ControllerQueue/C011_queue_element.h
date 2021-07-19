#ifndef CONTROLLERQUEUE_C011_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_C011_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"
#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/DeviceStruct.h"
#include "../DataStructs/UnitMessageCount.h"
#include "../Globals/CPlugins.h"
#include "../Globals/Plugins.h"

struct EventStruct;


#ifdef USES_C011

/*********************************************************************************************\
* C011_queue_element for queueing requests for C011: Generic HTTP Advanced.
\*********************************************************************************************/
class C011_queue_element {
public:

  C011_queue_element() = default;

  C011_queue_element(C011_queue_element&& other) = default;

#ifdef USE_SECOND_HEAP
  C011_queue_element(const C011_queue_element& other) = default;
#else
  C011_queue_element(const C011_queue_element& other) = delete;
#endif

  C011_queue_element(const struct EventStruct *event);

  bool isDuplicate(const C011_queue_element& other) const;

  const UnitMessageCount_t* getUnitMessageCount() const { return nullptr; }

  size_t getSize() const;

  String uri;
  String HttpMethod;
  String header;
  String postStr;
  int idx                          = 0;
  unsigned long _timestamp         = millis();
  taskIndex_t TaskIndex            = INVALID_TASK_INDEX;
  controllerIndex_t controller_idx = INVALID_CONTROLLER_INDEX;
  Sensor_VType sensorType          = Sensor_VType::SENSOR_TYPE_NONE;
};

#endif //USES_C011


#endif // CONTROLLERQUEUE_C011_QUEUE_ELEMENT_H
