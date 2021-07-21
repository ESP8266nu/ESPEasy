#ifndef CONTROLLERQUEUE_C018_QUEUE_ELEMENT_H
#define CONTROLLERQUEUE_C018_QUEUE_ELEMENT_H

#include "../../ESPEasy_common.h"
#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/MessageRouteInfo.h"
#include "../Globals/CPlugins.h"


struct EventStruct;

#ifdef USES_C018

/*********************************************************************************************\
* C018_queue_element for queueing requests for C018: TTN/RN2483
\*********************************************************************************************/


class C018_queue_element {
public:

  C018_queue_element() = default;

  C018_queue_element(const C018_queue_element& other) = delete;

  C018_queue_element(C018_queue_element&& other) = default;

  C018_queue_element(struct EventStruct *event,
                     uint8_t             sampleSetCount);

  size_t getSize() const;

  bool isDuplicate(const C018_queue_element& other) const;

  const MessageRouteInfo_t* getMessageRouteInfo() const { return nullptr; }

  String packed;
  unsigned long _timestamp         = millis();
  taskIndex_t TaskIndex            = INVALID_TASK_INDEX;
  controllerIndex_t controller_idx = INVALID_CONTROLLER_INDEX;
};

#endif //USES_C018


#endif // CONTROLLERQUEUE_C018_QUEUE_ELEMENT_H
