#include "../DataStructs/ESPEasy_Now_MQTT_queue_check_packet.h"


#ifdef USES_ESPEASY_NOW

void ESPEasy_Now_MQTT_queue_check_packet::markSendTime() {
  _millis_out = millis();
  state       = QueueState::Unset;
}

void ESPEasy_Now_MQTT_queue_check_packet::setState(bool isFull)
{
  state = isFull ? QueueState::Full : QueueState::Empty;
}

bool ESPEasy_Now_MQTT_queue_check_packet::isFull() const
{
  return state == QueueState::Full;
}

bool ESPEasy_Now_MQTT_queue_check_packet::isSet() const
{
  return state != QueueState::Unset;
}

#endif // ifdef USES_ESPEASY_NOW
