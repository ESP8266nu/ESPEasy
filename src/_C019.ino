#ifdef USES_C019

// #######################################################################################################
// ################### Controller Plugin 019: ESPEasy-NOW ################################################
// #######################################################################################################

#include "ESPEasy_fdwdecl.h"
#include "src/ControllerQueue/C019_queue_element.h"
#include "src/Globals/ESPEasy_now_handler.h"
#include "src/Helpers/_CPlugin_Helper.h"
#include "src/Helpers/Controller/C019_ESPEasyNow_helper.h"


#define CPLUGIN_019
#define CPLUGIN_ID_019         19
#define CPLUGIN_NAME_019       ESPEASY_NOW_NAME

bool CPlugin_019(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      Protocol[++protocolCount].Number       = CPLUGIN_ID_019;
      Protocol[protocolCount].usesMQTT       = false;
      Protocol[protocolCount].usesTemplate   = false;
      Protocol[protocolCount].usesAccount    = false;
      Protocol[protocolCount].usesPassword   = true;
      Protocol[protocolCount].usesExtCreds   = false;
      Protocol[protocolCount].defaultPort    = 0;
      Protocol[protocolCount].usesID         = true;
      Protocol[protocolCount].Custom         = false;
      Protocol[protocolCount].usesHost       = false;
      Protocol[protocolCount].usesPort       = false;
      Protocol[protocolCount].usesQueue      = false;
      Protocol[protocolCount].usesCheckReply = false;
      Protocol[protocolCount].usesTimeout    = false;
      Protocol[protocolCount].usesSampleSets = false;
      Protocol[protocolCount].needsNetwork   = false;
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_019);
      break;
    }

    case CPlugin::Function::CPLUGIN_INIT:
    {
      success = init_c019_delay_queue(event->ControllerIndex);
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_TEMPLATE:
    {
      event->String1 = F("%sysname%/#");
      event->String2 = F("%sysname%/%tskname%/%valname%");
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      success = C019_DelayHandler->addToQueue(C019_queue_element(event));

      Scheduler.scheduleNextDelayQueue(ESPEasy_Scheduler::IntervalTimer_e::TIMER_C019_DELAY_QUEUE, C019_DelayHandler->getNextScheduleTime());
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_RECV:
    {
      C019_ESPEasyNow_helper::process_receive(event);

      break;
    }

    case CPlugin::Function::CPLUGIN_FIFTY_PER_SECOND:
    {
      //      C019_data.async_loop();

      // FIXME TD-er: Handle reading error state or return values.
      break;
    }

    case CPlugin::Function::CPLUGIN_FLUSH:
    {
      process_c019_delay_queue();
      delay(0);
      break;
    }

    default:
      break;
  }

  return success;
}

// Uncrustify may change this into multi line, which will result in failed builds
// *INDENT-OFF*
bool do_process_c019_delay_queue(int controller_number, const C019_queue_element& element, ControllerSettingsStruct& ControllerSettings);
// *INDENT-ON*

bool do_process_c019_delay_queue(int controller_number, const C019_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
  ESPEasy_Now_p2p_data data;

  const taskIndex_t taskIndex = element.event.TaskIndex;

  data.dataType        = ESPEasy_Now_p2p_data_type::PluginData;
  data.sourceTaskIndex = taskIndex;
  data.plugin_id       = getPluginID_from_TaskIndex(taskIndex);
  data.sourceUnit      = Settings.Unit;
  data.idx             = Settings.TaskDeviceID[element.controller_idx][taskIndex];
  data.sensorType      = element.event.sensorType;
  data.valueCount      = getValueCountForTask(taskIndex);

  for (uint8_t i = 0; i < data.valueCount; ++i)
  {
    switch (data.sensorType) {
      case Sensor_VType::SENSOR_TYPE_LONG:
        data.addString(String((unsigned long)UserVar[element.event.BaseVarIndex] +
                              ((unsigned long)UserVar[element.event.BaseVarIndex + 1] << 16)));
        break;
      case Sensor_VType::SENSOR_TYPE_STRING:
        data.addString(element.event.String2);
        break;

      default:
        data.addFloat(UserVar[element.event.BaseVarIndex + i]);
        break;
    }
  }

  if (element.packed.length() > 0) {
    data.addString(element.packed);
  }

  MAC_address broadcast;

  // FIXME TD-er: Must add a way to reach further than WiFi reach
  for (int i = 0; i < 6; ++i) {
    broadcast.mac[i] = 0xFF;
  }

  return ESPEasy_now_handler.sendESPEasyNow_p2p(controller_number, broadcast, data);
}

#endif // ifdef USES_C019
