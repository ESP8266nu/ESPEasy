#include "../ControllerQueue/C019_queue_element.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../ESPEasyCore/ESPEasy_Log.h"


#ifdef USES_PACKED_RAW_DATA
String getPackedFromPlugin(struct EventStruct *event,
                           uint8_t             sampleSetCount);
#endif // USES_PACKED_RAW_DATA

C019_queue_element::C019_queue_element() {}

C019_queue_element::C019_queue_element(struct EventStruct *event_p) :
  controller_idx(event_p->ControllerIndex),
  event(*event_p)
{
  #ifdef USES_PACKED_RAW_DATA
  packed = getPackedFromPlugin(event_p, 0);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("C019 queue element: ");
    log += packed;
    addLog(LOG_LEVEL_INFO, log);
  }
  #endif // USES_PACKED_RAW_DATA

  // Extra check to make sure sensorType is correct.
  event.sensorType = event.getSensorType();
}

size_t C019_queue_element::getSize() const {
  return sizeof(*this) + packed.length();
}