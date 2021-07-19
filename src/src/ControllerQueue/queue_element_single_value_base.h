#ifndef CONTROLQUEUE_QUEUE_ELEMENT_SINGLE_VALUE_BASE_H
#define CONTROLQUEUE_QUEUE_ELEMENT_SINGLE_VALUE_BASE_H


#include "../../ESPEasy_common.h"
#include "../CustomBuild/ESPEasyLimits.h"
#include "../DataStructs/UnitMessageCount.h"
#include "../Globals/CPlugins.h"
#include "../Globals/Plugins.h"

struct EventStruct;

/*********************************************************************************************\
* Base class for controllers that only send a single value per request and thus needs to
* keep track of the number of values already sent.
\*********************************************************************************************/
class queue_element_single_value_base {
public:

  queue_element_single_value_base() = default;

  queue_element_single_value_base(const struct EventStruct *event,
                                  uint8_t                      value_count);

#ifdef USE_SECOND_HEAP
  queue_element_single_value_base(const queue_element_single_value_base& rval) = default;
#else
  queue_element_single_value_base(const queue_element_single_value_base& rval) = delete;
#endif
  
  queue_element_single_value_base(queue_element_single_value_base&& rval);

  queue_element_single_value_base& operator=(queue_element_single_value_base&& other);


  bool                             checkDone(bool succesfull) const;

  size_t                           getSize() const;

  bool                             isDuplicate(const queue_element_single_value_base& other) const;

  const UnitMessageCount_t       * getUnitMessageCount() const {
    return nullptr;
  }

  String txt[VARS_PER_TASK];
  int idx                          = 0;
  unsigned long _timestamp         = millis();
  taskIndex_t TaskIndex            = INVALID_TASK_INDEX;
  controllerIndex_t controller_idx = INVALID_CONTROLLER_INDEX;
  mutable uint8_t valuesSent          = 0; // Value must be set by const function checkDone()
  uint8_t valueCount                  = 0;
};


#endif // CONTROLQUEUE_QUEUE_ELEMENT_SINGLE_VALUE_BASE_H
