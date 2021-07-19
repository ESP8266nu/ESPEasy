#include "../ControllerQueue/SimpleQueueElement_string_only.h"


simple_queue_element_string_only::simple_queue_element_string_only(int ctrl_idx, taskIndex_t TaskIndex,  String&& req) :
  TaskIndex(TaskIndex), controller_idx(ctrl_idx)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif

  // Copy in the scope of the constructor, so we might store it in the 2nd heap
  txt = std::move(req);
}

size_t simple_queue_element_string_only::getSize() const {
  return sizeof(*this) + txt.length();
}

bool simple_queue_element_string_only::isDuplicate(const simple_queue_element_string_only& other) const {
  if ((other.controller_idx != controller_idx) ||
      (other.TaskIndex != TaskIndex) ||
      (other.txt != txt)) {
    return false;
  }
  return true;
}
