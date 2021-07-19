#include "../ControllerQueue/queue_element_formatted_uservar.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../Helpers/StringConverter.h"
#include "../../_Plugin_Helper.h"


queue_element_formatted_uservar::queue_element_formatted_uservar(queue_element_formatted_uservar&& other)
  :
  idx(other.idx),
  _timestamp(other._timestamp),
  TaskIndex(other.TaskIndex),
  controller_idx(other.controller_idx),
  sensorType(other.sensorType),
  valueCount(other.valueCount)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif

  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    txt[i] = std::move(other.txt[i]);
  }
}

queue_element_formatted_uservar::queue_element_formatted_uservar(EventStruct *event) :
  idx(event->idx),
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex),
  sensorType(event->sensorType)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif

  valueCount = getValueCountForTask(TaskIndex);

  for (uint8_t i = 0; i < valueCount; ++i) {
    txt[i] = formatUserVarNoCheck(event, i);
  }
}

queue_element_formatted_uservar& queue_element_formatted_uservar::operator=(queue_element_formatted_uservar&& other) {
  idx            = other.idx;
  _timestamp     = other._timestamp;
  TaskIndex      = other.TaskIndex;
  controller_idx = other.controller_idx;
  sensorType     = other.sensorType;
  valueCount     = other.valueCount;

  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

  for (size_t i = 0; i < VARS_PER_TASK; ++i) {
    txt[i] = std::move(other.txt[i]);
  }
  return *this;
}

size_t queue_element_formatted_uservar::getSize() const {
  size_t total = sizeof(*this);

  for (int i = 0; i < VARS_PER_TASK; ++i) {
    total += txt[i].length();
  }
  return total;
}

bool queue_element_formatted_uservar::isDuplicate(const queue_element_formatted_uservar& other) const {
  if ((other.controller_idx != controller_idx) ||
      (other.TaskIndex != TaskIndex) ||
      (other.sensorType != sensorType) ||
      (other.valueCount != valueCount)) {
    return false;
  }

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (other.txt[i] != txt[i]) {
      return false;
    }
  }
  return true;
}
