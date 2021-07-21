#ifndef DATASTRUCT_ESPEASY_NOW_MQTT_QUEUE_CHECK_PACKET_H
#define DATASTRUCT_ESPEASY_NOW_MQTT_QUEUE_CHECK_PACKET_H

#include <Arduino.h>

#include "../Globals/ESPEasy_now_state.h"
#ifdef USES_ESPEASY_NOW

class ESPEasy_Now_MQTT_queue_check_packet {
public:

  enum class QueueState : uint8_t {
    Unset,
    Empty,
    Full
  };

  ESPEasy_Now_MQTT_queue_check_packet() {}

  void setState(bool isFull);

  bool isFull() const;

  bool isSet() const;

  void markSendTime();

  unsigned long _millis_out = millis();
  QueueState state          = QueueState::Unset;
};

#endif // ifdef USES_ESPEASY_NOW

#endif // DATASTRUCT_ESPEASY_NOW_MQTT_QUEUE_CHECK_PACKET_H
