#include "EventQueue.h"

void EventQueueStruct::add(const String& event)
{
  _eventQueue.push_back(event);
}

void EventQueueStruct::add(const __FlashStringHelper * event)
{
  _eventQueue.push_back(event);
}

void EventQueueStruct::addMove(String&& event)
{
  _eventQueue.emplace_back(std::move(event));
}

bool EventQueueStruct::getNext(String& event)
{
  if (_eventQueue.empty()) {
    return false;
  }
  event = std::move(_eventQueue.front());
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