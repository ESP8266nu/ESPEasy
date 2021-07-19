#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C017

// #######################################################################################################
// ###########################   Controller Plugin 017: ZABBIX  ##########################################
// #######################################################################################################
// Based on https://www.zabbix.com/documentation/current/manual/appendix/items/trapper
// and https://www.zabbix.com/documentation/4.2/manual/appendix/protocols/header_datalen

// USAGE: at Zabbix server you go at Configuration -> Hosts -> Create host
// The "Host name" should match exactly the EspEasy name (Config -> Unit Name)
// Add a group (mandatory) and hit add. No need to set up IP address or agent.
// Go to the newly created host ->Items ->Create Item
// Name the item something descriptive
// For Key add the EspEasy task Value name (case sensitive)
// Type of information select "Numeric (float)" and press add.
// Aslo make sure that you enable send to controller (under Data Acquisition in tasks)
// and set an interval because you need to actively send the data to Zabbix

# define CPLUGIN_017
# define CPLUGIN_ID_017 17
# define CPLUGIN_NAME_017 "Zabbix"
# include <ArduinoJson.h>

bool CPlugin_017(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      Protocol[++protocolCount].Number     = CPLUGIN_ID_017;
      Protocol[protocolCount].usesMQTT     = false;
      Protocol[protocolCount].usesTemplate = false;
      Protocol[protocolCount].usesAccount  = false;
      Protocol[protocolCount].usesPassword = false;
      Protocol[protocolCount].usesID       = false;
      Protocol[protocolCount].defaultPort  = 10051;
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_017);
      break;
    }

    case CPlugin::Function::CPLUGIN_INIT:
    {
      success = init_c017_delay_queue(event->ControllerIndex);
      break;
    }

    case CPlugin::Function::CPLUGIN_EXIT:
    {
      exit_c017_delay_queue();
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      if (C017_DelayHandler == nullptr) {
        break;
      }

      success = C017_DelayHandler->addToQueue(C017_queue_element(event));
      Scheduler.scheduleNextDelayQueue(ESPEasy_Scheduler::IntervalTimer_e::TIMER_C017_DELAY_QUEUE, C017_DelayHandler->getNextScheduleTime());
      break;
    }

    case CPlugin::Function::CPLUGIN_FLUSH:
    {
      process_c017_delay_queue();
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
bool do_process_c017_delay_queue(int controller_number, const C017_queue_element& element, ControllerSettingsStruct& ControllerSettings);

bool do_process_c017_delay_queue(int controller_number, const C017_queue_element& element, ControllerSettingsStruct& ControllerSettings)
// *INDENT-ON*
{
  if (element.valueCount == 0) {
    return true; // exit if we don't have anything to send.
  }

  if (!NetworkConnected(NETWORK_CONNECTED_TIMEOUT))
  {
    return false;
  }

  WiFiClient client;

  if (!try_connect_host(controller_number, client, ControllerSettings, F("ZBX  : ")))
  {
    return false;
  }

  LoadTaskSettings(element.TaskIndex);

  const size_t capacity = JSON_ARRAY_SIZE(VARS_PER_TASK) + JSON_OBJECT_SIZE(2) + VARS_PER_TASK * JSON_OBJECT_SIZE(3) + VARS_PER_TASK * 50; //Size for esp8266 with 4 variables per task: 288+200
  String JSON_packet_content;
  {
    // Place the JSON document in a separate scope to have it destructed as soon as it is no longer needed.
    DynamicJsonDocument root(capacity);

    // Create the schafolding
    root[F("request")] = F("sender data");
    JsonArray data = root.createNestedArray(F("data"));

    // Populate JSON with the data
    for (uint8_t i = 0; i < element.valueCount; i++)
    {
      if (ExtraTaskSettings.TaskDeviceValueNames[i][0] == 0) {
        continue;                                                   // Zabbix will ignore an empty key anyway
      }
      JsonObject block = data.createNestedObject();
      block[F("host")] = Settings.Name;                             // Zabbix hostname, Unit Name for the ESP easy
      block[F("key")]  = ExtraTaskSettings.TaskDeviceValueNames[i]; // Zabbix item key // Value Name for the ESP easy
      float value = 0.0f;
      validFloatFromString(element.txt[i], value);
      block[F("value")] = value;                                    // ESPeasy supports only floats
    }
    serializeJson(root, JSON_packet_content);
  }

  // Assemble packet
  char packet_header[] = "ZBXD\1";

  uint64_t payload_len = JSON_packet_content.length();

  // addLog(LOG_LEVEL_INFO, String(F("ZBX: ")) + JSON_packet_content);
  // Send the packet
  client.write(packet_header,               sizeof(packet_header) - 1);
  client.write((char *)&payload_len,        sizeof(payload_len));
  client.write(JSON_packet_content.c_str(), payload_len);

  client.stop();
  return true;
}

#endif // ifdef USES_C017
