#ifndef CONTROLLERQUEUE_SIMPLE_QUEUE_ELEMENT_STRING_ONLY_H
#define CONTROLLERQUEUE_SIMPLE_QUEUE_ELEMENT_STRING_ONLY_H

#include "../../ESPEasy_common.h"
#include "../DataStructs/MessageRouteInfo.h"
#include "../Globals/CPlugins.h"


/*********************************************************************************************\
* Simple queue element, only storing controller index and some String
\*********************************************************************************************/
class simple_queue_element_string_only {
public:

  simple_queue_element_string_only() = default;

  simple_queue_element_string_only(const simple_queue_element_string_only& other) = delete;
  
  simple_queue_element_string_only(simple_queue_element_string_only&& other) = default;

  explicit simple_queue_element_string_only(int           ctrl_idx,
                                            taskIndex_t   TaskIndex,
                                            String&&      req);

  size_t getSize() const;

  bool isDuplicate(const simple_queue_element_string_only& other) const;

  const MessageRouteInfo_t* getMessageRouteInfo() const { return nullptr; }

  String txt;
  unsigned long _timestamp         = millis();
  taskIndex_t TaskIndex            = INVALID_TASK_INDEX;
  controllerIndex_t controller_idx = INVALID_CONTROLLER_INDEX;
};


#endif // CONTROLLERQUEUE_SIMPLE_QUEUE_ELEMENT_STRING_ONLY_H
