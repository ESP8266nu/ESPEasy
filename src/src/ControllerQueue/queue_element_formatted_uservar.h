#ifndef CONTROLLERQUEUE_QUEUE_ELEMENT_FORMATTED_USERVAR_H
#define CONTROLLERQUEUE_QUEUE_ELEMENT_FORMATTED_USERVAR_H

#include "../../ESPEasy_common.h"
#include "../DataStructs/DeviceStruct.h"
#include "../DataStructs/UnitMessageCount.h"
#include "../Globals/CPlugins.h"
#include "../Globals/Plugins.h"


struct EventStruct;

/*********************************************************************************************\
* For queueing task values already formatted according to the task settings
\*********************************************************************************************/
class queue_element_formatted_uservar {
public:

  queue_element_formatted_uservar() = default;

#ifdef USE_SECOND_HEAP
  queue_element_formatted_uservar(const queue_element_formatted_uservar& other) = default;
#else
  queue_element_formatted_uservar(const queue_element_formatted_uservar& other) = delete;
#endif

  queue_element_formatted_uservar(queue_element_formatted_uservar&& other);

  queue_element_formatted_uservar(struct EventStruct *event);

  queue_element_formatted_uservar& operator=(queue_element_formatted_uservar&& other);

  size_t                           getSize() const;

  bool                             isDuplicate(const queue_element_formatted_uservar& other) const;

  const UnitMessageCount_t       * getUnitMessageCount() const {
    return nullptr;
  }

  String txt[VARS_PER_TASK];
  int idx                          = 0;
  unsigned long _timestamp         = millis();
  taskIndex_t TaskIndex            = INVALID_TASK_INDEX;
  controllerIndex_t controller_idx = INVALID_CONTROLLER_INDEX;
  Sensor_VType sensorType          = Sensor_VType::SENSOR_TYPE_NONE;
  uint8_t valueCount                  = 0;
};

#endif // CONTROLLERQUEUE_QUEUE_ELEMENT_FORMATTED_USERVAR_H
