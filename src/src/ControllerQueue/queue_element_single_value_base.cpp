#include "../ControllerQueue/queue_element_single_value_base.h"

#include "../DataStructs/ESPEasy_EventStruct.h"

queue_element_single_value_base::queue_element_single_value_base(const struct EventStruct *event, uint8_t value_count) :
  idx(event->idx),
  TaskIndex(event->TaskIndex),
  controller_idx(event->ControllerIndex),
  valuesSent(0),
  valueCount(value_count) {}

queue_element_single_value_base::queue_element_single_value_base(queue_element_single_value_base&& rval)
  : idx(rval.idx), _timestamp(rval._timestamp),  TaskIndex(rval.TaskIndex),
  controller_idx(rval.controller_idx),
  valuesSent(rval.valuesSent), valueCount(rval.valueCount)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    txt[i] = std::move(rval.txt[i]);
  }
}

queue_element_single_value_base& queue_element_single_value_base::operator=(queue_element_single_value_base&& rval) {
  idx            = rval.idx;
  _timestamp     = rval._timestamp;
  TaskIndex      = rval.TaskIndex;
  controller_idx = rval.controller_idx;
  valuesSent     = rval.valuesSent;
  valueCount     = rval.valueCount;
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif // ifdef USE_SECOND_HEAP

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    txt[i] = std::move(rval.txt[i]);
  }
  return *this;
}

bool queue_element_single_value_base::checkDone(bool succesfull) const {
  if (succesfull) { ++valuesSent; }
  return valuesSent >= valueCount || valuesSent >= VARS_PER_TASK;
}

size_t queue_element_single_value_base::getSize() const {
  size_t total = sizeof(*this);

  for (int i = 0; i < VARS_PER_TASK; ++i) {
    total += txt[i].length();
  }
  return total;
}

bool queue_element_single_value_base::isDuplicate(const queue_element_single_value_base& other) const {
  if ((other.controller_idx != controller_idx) ||
      (other.TaskIndex != TaskIndex) ||
      (other.valueCount != valueCount) ||
      (other.idx != idx)) {
    return false;
  }

  for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
    if (other.txt[i] != txt[i]) {
      return false;
    }
  }
  return true;
}
