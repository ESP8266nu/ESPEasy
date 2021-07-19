#include "EventQueue.h"

#include "../../ESPEasy_common.h"

EventQueueStruct::EventQueueStruct() {}

void EventQueueStruct::add(const String& event)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif

  _eventQueue.push_back(event);
}

void EventQueueStruct::add(const __FlashStringHelper * event)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  #endif
  // Wrap in String() constructor to make sure it is using the 2nd heap allocator if present.
  _eventQueue.push_back(String(event));
}

void EventQueueStruct::addMove(String&& event)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;
  _eventQueue.push_back(event);
  #else
  _eventQueue.emplace_back(std::move(event));
  #endif
}

bool EventQueueStruct::getNext(String& event)
{
  if (_eventQueue.empty()) {
    return false;
  }
  #ifdef USE_SECOND_HEAP
  {
    // Fetch the event and make sure it is allocated on the DRAM heap, not the 2nd heap
    // Otherwise checks like strnlen_P may crash on it.
    HeapSelectDram ephemeral;
    event = std::move(String(_eventQueue.front()));
  }
  #else
  event = std::move(_eventQueue.front());
  #endif
  _eventQueue.pop_front();
  return true;
}

void EventQueueStruct::clear()
{
  _eventQueue.clear();
}

bool EventQueueStruct::isEmpty() const
{
  return _eventQueue.empty();
}