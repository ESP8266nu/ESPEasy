#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C014

#include "src/Commands/InternalCommands.h"
#include "src/Globals/Device.h"
#include "src/Globals/MQTT.h"
#include "src/Globals/Plugins.h"
#include "src/Globals/Statistics.h"
#include "src/Helpers/PeriodicalActions.h"
#include "_Plugin_Helper.h"

//#######################################################################################################
//################################# Controller Plugin 0014: Homie 3/4 ###################################
//#######################################################################################################

#define CPLUGIN_014
#define CPLUGIN_ID_014              14

// Define which Homie version to use
//#define CPLUGIN_014_V3
#define CPLUGIN_014_V4

#ifdef CPLUGIN_014_V3
  #define CPLUGIN_014_HOMIE_VERSION   "3.0.0"
  #define CPLUGIN_NAME_014            "Homie MQTT (Version 3.0.1)"
#endif
#ifdef CPLUGIN_014_V4
  #define CPLUGIN_014_HOMIE_VERSION   "4.0.0"
  #define CPLUGIN_NAME_014            "Homie MQTT (Version 4.0.0 dev)"
#endif

// subscribe and publish schemes should not be changed by the user. This will probably break the homie convention. Do @ your own risk;)
#define CPLUGIN_014_SUBSCRIBE       "homie/%sysname%/+/+/set" // only subscribe to /set topics to reduce load by receiving all retained messages
#define CPLUGIN_014_PUBLISH         "homie/%sysname%/%tskname%/%valname%"

#define CPLUGIN_014_BASE_TOPIC      "homie/%sysname%/#"
#define CPLUGIN_014_BASE_VALUE      "homie/%sysname%/%device%/%node%/%property%"
#define CPLUGIN_014_INTERVAL        "90" // to prevent timeout !ToDo set by lowest plugin interval
#define CPLUGIN_014_SYSTEM_DEVICE   "SYSTEM" // name for system device Plugin for cmd and GIO values
#define CPLUGIN_014_CMD_VALUE       "cmd" // name for command value
#define CPLUGIN_014_GPIO_VALUE      "gpio" // name for gpio value i.e. "gpio1"
#define CPLUGIN_014_CMD_VALUE_NAME  "Command" // human readabele name for command value

uint8_t msgCounter=0; // counter for send Messages (currently for information / log only!

String CPlugin_014_pubname;
bool CPlugin_014_mqtt_retainFlag = false;

/*
// send MQTT Message with complete Topic / Payload
bool CPlugin_014_sendMQTTmsg(String& topic, const char* payload, int& errorCounter) {
        bool mqttReturn = MQTTpublish(CPLUGIN_ID_014, INVALID_TASK_INDEX, topic.c_str(), payload, true);
        if (mqttReturn) msgCounter++;
          else errorCounter++;
        if (loglevelActiveFor(LOG_LEVEL_INFO) && mqttReturn) {
          String log = F("C014 : msg T:");
          log += topic;
          log += F(" P: ");
          log += payload;
          addLog(LOG_LEVEL_DEBUG_MORE, log+" success!");
        }
        if (loglevelActiveFor(LOG_LEVEL_INFO) && !mqttReturn) {
          String log = F("C014 : msg T:");
          log += topic;
          log += F(" P: ");
          log += payload;
          addLog(LOG_LEVEL_ERROR, log+" ERROR!");
        }
        return mqttReturn;
}
*/

// send MQTT Message with CPLUGIN_014_BASE_TOPIC Topic scheme / Payload
bool CPlugin_014_sendMQTTdevice(String tmppubname, taskIndex_t taskIndex, const char* topic, const char* payload, int& errorCounter) {
        tmppubname.replace(F("#"), topic);
        bool mqttReturn = MQTTpublish(CPLUGIN_ID_014, taskIndex, tmppubname.c_str(), payload, true);
        if (mqttReturn) msgCounter++;
          else errorCounter++;
        if (loglevelActiveFor(LOG_LEVEL_INFO) && mqttReturn) {
          String log = F("C014 : T:");
          log += topic;
          log += F(" P: ");
          log += payload;
          addLog(LOG_LEVEL_DEBUG_MORE, log+" success!");
        }
        if (loglevelActiveFor(LOG_LEVEL_INFO) && !mqttReturn) {
          String log = F("C014 : T:");
          log += topic;
          log += F(" P: ");
          log += payload;
          addLog(LOG_LEVEL_ERROR, log+" ERROR!");
        }
        processMQTTdelayQueue();
        return mqttReturn;

}

// send MQTT Message with CPLUGIN_014_BASE_VALUE Topic scheme / Payload
bool CPlugin_014_sendMQTTnode(String tmppubname, const char* node, const char* value, const char* topic, const char* payload, int& errorCounter) {
        tmppubname.replace(F("%device%"), node);
        tmppubname.replace(F("%node%"), value);
        tmppubname.replace(F("/%property%"), topic); // leading forward slash required to send "homie/device/value" topics
        bool mqttReturn = MQTTpublish(CPLUGIN_ID_014, INVALID_TASK_INDEX, tmppubname.c_str(), payload, true);
        if (mqttReturn) msgCounter++;
          else errorCounter++;
        if (loglevelActiveFor(LOG_LEVEL_INFO) && mqttReturn) {
          String log = F("C014 : V:");
          log += value;
          log += F(" T: ");
          log += topic;
          log += F(" P: ");
          log += payload;
          addLog(LOG_LEVEL_DEBUG_MORE, log+" success!");
        }
        if (loglevelActiveFor(LOG_LEVEL_INFO) && !mqttReturn) {
          String log = F("C014 : V:");
          log += value;
          log += F(" T: ");
          log += topic;
          log += F(" P: ");
          log += payload;
          addLog(LOG_LEVEL_ERROR, log+" ERROR!");
        }
        processMQTTdelayQueue();
        return mqttReturn;

}

// and String a comma seperated list
void CPLUGIN_014_addToList(String& valuesList, const char* node)
{
  if (valuesList.length()>0) valuesList += F(",");
  valuesList += node;
}

bool CPlugin_014(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;
  int errorCounter = 0;
  String log;
  String pubname;
  String tmppubname;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
      {
        Protocol[++protocolCount].Number = CPLUGIN_ID_014;
        Protocol[protocolCount].usesMQTT = true;
        Protocol[protocolCount].usesTemplate = true;
        Protocol[protocolCount].usesAccount = true;
        Protocol[protocolCount].usesPassword = true;
        Protocol[protocolCount].usesExtCreds = true;
        Protocol[protocolCount].defaultPort = 1883;
        Protocol[protocolCount].usesID = false;
        break;
      }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
      {
        string = F(CPLUGIN_NAME_014);
        break;
      }

    case CPlugin::Function::CPLUGIN_INIT:
      {
        success = init_mqtt_delay_queue(event->ControllerIndex, CPlugin_014_pubname, CPlugin_014_mqtt_retainFlag);
        break;
      }

    case CPlugin::Function::CPLUGIN_EXIT:
      {
        exit_mqtt_delay_queue();
        break;
      }

    case CPlugin::Function::CPLUGIN_INTERVAL:
      {
        if (MQTTclient.connected())
        {
          errorCounter = 0;

          pubname = CPLUGIN_014_BASE_TOPIC; // Scheme to form device messages
          pubname.replace(F("%sysname%"), Settings.Name);

#ifdef CPLUGIN_014_V3
          // $stats/uptime	Device → Controller	Time elapsed in seconds since the boot of the device	Yes	Yes
          CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex,"$stats/uptime",toString(getUptimeMinutes()*60,0).c_str(),errorCounter);

          // $stats/signal	Device → Controller	Signal strength in %	Yes	No
          float RssI = WiFi.RSSI();
          RssI = isnan(RssI) ? -100.0f : RssI;
          RssI = min(max(2 * (RssI + 100.0f), 0.0f), 100.0f);

          CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex,"$stats/signal",toString(RssI,1).c_str(),errorCounter);
#endif

          if (errorCounter>0)
          {
            // alert: this is the state the device is when connected to the MQTT broker, but something wrong is happening. E.g. a sensor is not providing data and needs human intervention. You have to send this message when something is wrong.
            CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex,"$state","alert",errorCounter);
            success = false;
          } else {
            // ready: this is the state the device is in when it is connected to the MQTT broker, has sent all Homie messages and is ready to operate. You have to send this message after all other announcements message have been sent.
            CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex,"$state","ready",errorCounter);
            success = true;
          }
          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            String log = F("C014 : $stats information sent with ");
            if (errorCounter>0) log+=errorCounter;
              else log+=F("no");
            log+=F(" errors! (");
            log+=msgCounter;
            log+=F(" messages)");
            msgCounter=0;
            addLog(LOG_LEVEL_DEBUG, log);
          }
        }
        break;
      }

    case CPlugin::Function::CPLUGIN_GOT_CONNECTED: //// call after connected to mqtt server to publich device autodicover features
      {
        statusLED(true);

        // send autodiscover header
        pubname = CPLUGIN_014_BASE_TOPIC; // Scheme to form device messages
        pubname.replace(F("%sysname%"), Settings.Name);
        int deviceCount = 1; // minimum the SYSTEM device exists
        int nodeCount = 1; // minimum the cmd node exists
        errorCounter = 0;

        if (lastBootCause!=BOOT_CAUSE_DEEP_SLEEP) // skip sending autodiscover data when returning from deep sleep
        {
          String nodename = CPLUGIN_014_BASE_VALUE; // Scheme to form node messages
          nodename.replace(F("%sysname%"), Settings.Name);
          String nodesList; // build comma separated List for nodes
          String valuesList; // build comma separated List for values
          String deviceName; // current Device Name nr:name
          String valueName; // current Value Name
          String unitName; // estaimate Units

          // init: this is the state the device is in when it is connected to the MQTT broker, but has not yet sent all Homie messages and is not yet ready to operate. This is the first message that must that must be sent.
          CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex,"$state","init",errorCounter);

          // $homie	Device → Controller	Version of the Homie convention the device conforms to	Yes	Yes
          CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex,"$homie",CPLUGIN_014_HOMIE_VERSION,errorCounter);

          // $name	Device → Controller	Friendly name of the device	Yes	Yes
          CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex,"$name",Settings.Name,errorCounter);

          // $localip	Device → Controller	IP of the device on the local network	Yes	Yes
#ifdef CPLUGIN_014_V3
          CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex,"$localip",formatIP(NetworkLocalIP()).c_str(),errorCounter);

          // $mac	Device → Controller	Mac address of the device network interface. The format MUST be of the type A1:B2:C3:D4:E5:F6	Yes	Yes
          CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex,"$mac",NetworkMacAddress().c_str(),errorCounter);

          // $implementation	Device → Controller	An identifier for the Homie implementation (example esp8266)	Yes	Yes
          #if defined(ESP8266)
            CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex,"$implementation","ESP8266",errorCounter);
          #endif
          #if defined(ESP32)
            CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex,"$implementation","ESP32",errorCounter);
          #endif

          // $fw/version	Device → Controller	Version of the firmware running on the device	Yes	Yes
          CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex,"$fw/version",toString(Settings.Build,0).c_str(),errorCounter);

          // $fw/name	Device → Controller	Name of the firmware running on the device. Allowed characters are the same as the device ID	Yes	Yes
          CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex,"$fw/name",String(NodeStruct::getNodeTypeDisplayString(NODE_TYPE_ID)).c_str(),errorCounter);

          // $stats/interval	Device → Controller	Interval in seconds at which the device refreshes its $stats/+: See next section for details about statistical attributes	Yes	Yes
          CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex,"$stats/interval",CPLUGIN_014_INTERVAL,errorCounter);
#endif

          //always send the SYSTEM device with the cmd node
          CPLUGIN_014_addToList(nodesList,CPLUGIN_014_SYSTEM_DEVICE);
          CPLUGIN_014_addToList(valuesList,CPLUGIN_014_CMD_VALUE);

          // $name	Device → Controller	Friendly name of the Node	Yes	Yes
          CPlugin_014_sendMQTTnode(nodename, CPLUGIN_014_SYSTEM_DEVICE, "$name", "", CPLUGIN_014_SYSTEM_DEVICE, errorCounter);

          //$name	Device → Controller	Friendly name of the property.	Any String	Yes	No ("")
          CPlugin_014_sendMQTTnode(nodename, CPLUGIN_014_SYSTEM_DEVICE, CPLUGIN_014_CMD_VALUE, "/$name", CPLUGIN_014_CMD_VALUE_NAME, errorCounter);
          //$datatype	The data type. See Payloads.	Enum: [integer, float, boolean, string, enum, color]
          CPlugin_014_sendMQTTnode(nodename, CPLUGIN_014_SYSTEM_DEVICE, CPLUGIN_014_CMD_VALUE, "/$datatype", "string", errorCounter);

          //$settable	Device → Controller	Specifies whether the property is settable (true) or readonly (false)	true or false	Yes	No (false)
          CPlugin_014_sendMQTTnode(nodename, CPLUGIN_014_SYSTEM_DEVICE, CPLUGIN_014_CMD_VALUE, "/$settable", "true", errorCounter);

          // enum all devices

          // FIRST Standard GPIO tasks
          int gpio = 0;
          while (gpio <= MAX_GPIO) {
            if (Settings.getPinBootState(gpio) != PinBootState::Default_state) // anything but default
            {
              nodeCount++;
              valueName = CPLUGIN_014_GPIO_VALUE;
              valueName += toString(gpio,0);
              CPLUGIN_014_addToList(valuesList,valueName.c_str());

              //$name	Device → Controller	Friendly name of the property.	Any String	Yes	No ("")
              CPlugin_014_sendMQTTnode(nodename, CPLUGIN_014_SYSTEM_DEVICE, valueName.c_str(), "/$name", valueName.c_str(), errorCounter);
              //$datatype	The data type. See Payloads.	Enum: [integer, float, boolean,string, enum, color]
              CPlugin_014_sendMQTTnode(nodename, CPLUGIN_014_SYSTEM_DEVICE, valueName.c_str(), "/$datatype", "boolean", errorCounter);
              if (Settings.getPinBootState(gpio) != PinBootState::Input) // defined as output
              {
                //$settable	Device → Controller	Specifies whether the property is settable (true) or readonly (false)	true or false	Yes	No (false)
                CPlugin_014_sendMQTTnode(nodename, CPLUGIN_014_SYSTEM_DEVICE, valueName.c_str(), "/$settable", "true", errorCounter);
              }
            }
            ++gpio;
          }

          // $properties	Device → Controller	Properties the node exposes, with format id separated by a , if there are multiple nodes.	Yes	Yes
          CPlugin_014_sendMQTTnode(nodename, CPLUGIN_014_SYSTEM_DEVICE, "$properties", "", valuesList.c_str(), errorCounter);
          valuesList = "";
          deviceCount++;

          // SECOND Plugins
          for (taskIndex_t x = 0; x < TASKS_MAX; x++)
          {
            if (validPluginID_fullcheck((Settings.TaskDeviceNumber[x])))
            {
              LoadTaskSettings(x);
              deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(x);

              deviceName = ExtraTaskSettings.TaskDeviceName;

              if (validDeviceIndex(DeviceIndex) && Settings.TaskDeviceEnabled[x])  // Device is enabled so send information
              { // device enabled
                valuesList="";

                const uint8_t valueCount = getValueCountForTask(x);
                if (!Device[DeviceIndex].SendDataOption) // check if device is not sending data = assume that it can receive.
                {
                  if (Device[DeviceIndex].Number==86) // Homie receiver
                  {
                    for (uint8_t varNr = 0; varNr < valueCount; varNr++) {
                      if (validPluginID_fullcheck(Settings.TaskDeviceNumber[x])) {
                        if (ExtraTaskSettings.TaskDeviceValueNames[varNr][0]!=0) { // do not send if Value Name is empty!
                          CPLUGIN_014_addToList(valuesList,ExtraTaskSettings.TaskDeviceValueNames[varNr]);
                          //$settable	Device → Controller	Specifies whether the property is settable (true) or readonly (false)	true or false	Yes	No (false)
                          CPlugin_014_sendMQTTnode(nodename, deviceName.c_str(), ExtraTaskSettings.TaskDeviceValueNames[varNr], "/$settable", "true", errorCounter);
                          //$name	Device → Controller	Friendly name of the property.	Any String	Yes	No ("")
                          valueName = F("Homie Receiver: ");
                          valueName += ExtraTaskSettings.TaskDeviceValueNames[varNr];
                          CPlugin_014_sendMQTTnode(nodename, deviceName.c_str(), ExtraTaskSettings.TaskDeviceValueNames[varNr], "/$name",valueName.c_str(), errorCounter);
                          //$datatype	The data type. See Payloads.	Enum: [integer, float, boolean,string, enum, color]
                          unitName = "";
                          switch(Settings.TaskDevicePluginConfig[x][varNr]) {
                            case 0: valueName = F("integer");
                                    if (ExtraTaskSettings.TaskDevicePluginConfig[varNr]!=0 || ExtraTaskSettings.TaskDevicePluginConfig[varNr+5]!=0) {
                                      unitName = ExtraTaskSettings.TaskDevicePluginConfig[varNr];
                                      unitName += ":";
                                      unitName += ExtraTaskSettings.TaskDevicePluginConfig[varNr + valueCount];
                                    }
                                    break;
                            case 1: valueName = F("float");
                                    if (ExtraTaskSettings.TaskDevicePluginConfig[varNr]!=0 || ExtraTaskSettings.TaskDevicePluginConfig[varNr+5]!=0) {
                                      unitName = ExtraTaskSettings.TaskDevicePluginConfig[varNr];
                                      unitName += ":";
                                      unitName += ExtraTaskSettings.TaskDevicePluginConfig[varNr + valueCount];
                                    }
                                    break;
                            case 2: valueName = F("boolean"); break;
                            case 3: valueName = F("string"); break;
                            case 4: valueName = F("enum");
                                    unitName = ExtraTaskSettings.TaskDeviceFormula[varNr];
                                    break;
                            case 5: valueName = F("color");
                                    unitName = F("rgb");
                                    break;
                            case 6: valueName = F("color");
                                    unitName = F("hsv");
                                    break;
                          }
                          CPlugin_014_sendMQTTnode(nodename, deviceName.c_str(), ExtraTaskSettings.TaskDeviceValueNames[varNr], "/$datatype", valueName.c_str(), errorCounter);
                          if (!unitName.isEmpty()) CPlugin_014_sendMQTTnode(nodename, deviceName.c_str(), ExtraTaskSettings.TaskDeviceValueNames[varNr], "/$format", unitName.c_str(), errorCounter);
                          nodeCount++;
                        }
                      }
                    }
                  }
                } else {
                  // ignore cutom values for now! Assume all Values are standard float.
                  // String customValuesStr;
                  // customValues = PluginCall(PLUGIN_WEBFORM_SHOW_VALUES, &TempEvent, customValuesStr);
                  uint8_t customValues = false;
                  if (!customValues)
                  { // standard Values
                    for (uint8_t varNr = 0; varNr < valueCount; varNr++)
                    {
                      if (validPluginID_fullcheck(Settings.TaskDeviceNumber[x]))
                      {
                        if (ExtraTaskSettings.TaskDeviceValueNames[varNr][0]!=0) // do not send if Value Name is empty!
                        {
                          CPLUGIN_014_addToList(valuesList,ExtraTaskSettings.TaskDeviceValueNames[varNr]);

                          //$name	Device → Controller	Friendly name of the property.	Any String	Yes	No ("")
                          CPlugin_014_sendMQTTnode(nodename, deviceName.c_str(), ExtraTaskSettings.TaskDeviceValueNames[varNr], "/$name", ExtraTaskSettings.TaskDeviceValueNames[varNr], errorCounter);
                          //$datatype	The data type. See Payloads.	Enum: [integer, float, boolean,string, enum, color]
                          CPlugin_014_sendMQTTnode(nodename, deviceName.c_str(), ExtraTaskSettings.TaskDeviceValueNames[varNr], "/$datatype", "float", errorCounter);

                          if (Device[DeviceIndex].Number==33) // Dummy Device can send AND receive Data
                            CPlugin_014_sendMQTTnode(nodename, deviceName.c_str(), ExtraTaskSettings.TaskDeviceValueNames[varNr], "/$settable", "true", errorCounter);

                          nodeCount++;
/*                          // because values in ESPEasy are unitless lets assueme some units by the value name (still case sensitive)
                          if (strstr(ExtraTaskSettings.TaskDeviceValueNames[varNr], "temp") != NULL )
                          {
                            unitName = "°C";
                          } else if (strstr(ExtraTaskSettings.TaskDeviceValueNames[varNr], "humi") != NULL )
                          {
                            unitName = "%";
                          } else if (strstr(ExtraTaskSettings.TaskDeviceValueNames[varNr], "press") != NULL )
                          {
                            unitName = "Pa";
                          } // ToDo: .... and more

                          if (unitName != "")  // found a unit match
                          {
                            // $unit	Device → Controller	A string containing the unit of this property. You are not limited to the recommended values, although they are the only well known ones that will have to be recognized by any Homie consumer.	Recommended: Yes	No ("")
                            CPlugin_014_sendMQTTnode(nodename, deviceName.c_str(), ExtraTaskSettings.TaskDeviceValueNames[varNr], "/$unit", unitName.c_str(), errorCounter);
                          }
                          unitName = "";
*/                      }
                      }
                    } // end loop throug values
                  } else { // Device has custom Values
                    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
                      String log = F("C014 : Device has custom values: ");
                      log += getPluginNameFromDeviceIndex(getDeviceIndex_from_TaskIndex(x));
                      addLog(LOG_LEVEL_DEBUG, log+" not implemented!");
                    }
                  }
                }
                if (!valuesList.isEmpty())
                {
                  // only add device to list if it has nodes!
                  // $name	Device → Controller	Friendly name of the Node	Yes	Yes
                  CPlugin_014_sendMQTTnode(nodename, deviceName.c_str(), "$name", "",ExtraTaskSettings.TaskDeviceName, errorCounter);

                  // $type	Device → Controller	Type of the node	Yes	Yes
                  CPlugin_014_sendMQTTnode(nodename, deviceName.c_str(), "$type", "", getPluginNameFromDeviceIndex(DeviceIndex).c_str(), errorCounter);

                  // add device to device list
                  CPLUGIN_014_addToList(nodesList,deviceName.c_str());
                  deviceCount++;
                  // $properties	Device → Controller	Properties the node exposes, with format id separated by a , if there are multiple nodes.	Yes	Yes
                  CPlugin_014_sendMQTTnode(nodename, deviceName.c_str(), "$properties", "", valuesList.c_str(), errorCounter);
                  valuesList="";
                }
              } else { // device not enabeled
                if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
                  String log = F("C014 : Device Disabled: ");
                  log += getPluginNameFromDeviceIndex(getDeviceIndex_from_TaskIndex(x));
                  addLog(LOG_LEVEL_DEBUG, log+" not propagated!");
                }
              }
            } // device configured
          } // loop through devices

          // and finally ...
          // $nodes	Device → Controller	Nodes the device exposes, with format id separated by a , if there are multiple nodes. To make a node an array, append [] to the ID.	Yes	Yes
          CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex, "$nodes", nodesList.c_str(), errorCounter);
        }

        if (errorCounter>0)
        {
          // alert: this is the state the device is when connected to the MQTT broker, but something wrong is happening. E.g. a sensor is not providing data and needs human intervention. You have to send this message when something is wrong.
          CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex,"$state","alert",errorCounter);
          success = false;
        } else {
          // ready: this is the state the device is in when it is connected to the MQTT broker, has sent all Homie messages and is ready to operate. You have to send this message after all other announcements message have been sent.
          CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex,"$state","ready",errorCounter);
          success = true;
        }
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("C014 : autodiscover information of ");
          log += deviceCount;
          log += F(" Devices and ");
          log += nodeCount;
          log += F(" Nodes sent with ");
          if (errorCounter>0) log+=errorCounter;
            else log+=F("no");
          log+=F(" errors! (");
          log+=msgCounter;
          log+=F(" messages)");
          msgCounter = 0;
          errorCounter = 0;
          addLog(LOG_LEVEL_INFO, log);
        }
        break;
      }

    case CPlugin::Function::CPLUGIN_PROTOCOL_TEMPLATE:
      {
        event->String1 = F(CPLUGIN_014_SUBSCRIBE);
        event->String2 = F(CPLUGIN_014_PUBLISH);
        break;
      }

    case CPlugin::Function::CPLUGIN_GOT_INVALID:
      {
        pubname = CPLUGIN_014_BASE_TOPIC; // Scheme to form device messages
        pubname.replace(F("%sysname%"), Settings.Name);
        // disconnected: this is the state the device is in when it is cleanly disconnected from the MQTT broker. You must send this message before cleanly disconnecting
        success = CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex,"$state","disconnected",errorCounter);
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("C014 : Device: ");
          log += Settings.Name;
          if (success) log += F(" got invalid (disconnected).");
            else log += F(" got invaild (disconnect) failed!");
          addLog(LOG_LEVEL_INFO, log);
        }
        break;
      }

    case CPlugin::Function::CPLUGIN_FLUSH:
      {
        pubname = CPLUGIN_014_BASE_TOPIC; // Scheme to form device messages
        pubname.replace(F("%sysname%"), Settings.Name);
        // sleeping: this is the state the device is in when the device is sleeping. You have to send this message before sleeping.
        success = CPlugin_014_sendMQTTdevice(pubname, event->TaskIndex,"$state","sleeping",errorCounter);
        break;
      }

    case CPlugin::Function::CPLUGIN_PROTOCOL_RECV:
      {
        controllerIndex_t ControllerID = findFirstEnabledControllerWithId(CPLUGIN_ID_014);
        bool validTopic = false;
        if (!validControllerIndex(ControllerID)) {
          // Controller is not enabled.
          break;
        } else {
          String cmd;
          int valueNr=0;
          taskIndex_t taskIndex = INVALID_TASK_INDEX;
          struct EventStruct TempEvent(event->TaskIndex);
          TempEvent.Source = EventValueSource::Enum::VALUE_SOURCE_MQTT; // to trigger the correct acknowledgment
          int lastindex = event->String1.lastIndexOf('/');
          errorCounter = 0;
          if (event->String1.substring(lastindex + 1) == F("set"))
          {
            pubname = event->String1.substring(0,lastindex);
            lastindex = pubname.lastIndexOf('/');
            String nodeName = pubname.substring(0,lastindex);
            String valueName = pubname. substring(lastindex+1);
            lastindex = nodeName.lastIndexOf('/');
            nodeName = nodeName. substring(lastindex+1);
            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              log=F("C014 : MQTT received: ");
              log+=F("/set: N: ");
              log+=nodeName;
              log+=F(" V: ");
              log+=valueName;
            }
            if (nodeName == F(CPLUGIN_014_SYSTEM_DEVICE)) // msg to a system device
            {
              if (valueName.substring(0,strlen(CPLUGIN_014_GPIO_VALUE)) == F(CPLUGIN_014_GPIO_VALUE)) // msg to to set gpio values
              {
                cmd = ("GPIO,");
                cmd += valueName.substring(strlen(CPLUGIN_014_GPIO_VALUE)).toInt(); // get the GPIO
                if (event->String2=="true" || event->String2=="1") cmd += F(",1");
                  else cmd += F(",0");
                validTopic = true;
              } else if (valueName==CPLUGIN_014_CMD_VALUE) // msg to send a command
              {
                cmd = event->String2;
                validTopic = true;
              } else
              {
                cmd = F("SYSTEM/");
                cmd += valueName;
                cmd += F(" unknown!");
              }
            } else // msg to a receiving plugin
            {
              taskIndex=findTaskIndexByName(nodeName);
              deviceIndex_t deviceIndex = getDeviceIndex_from_TaskIndex(taskIndex);
              taskVarIndex_t taskVarIndex = event->Par2 - 1;
              if (validDeviceIndex(deviceIndex) && validTaskVarIndex(taskVarIndex)) {
                int pluginID=Device[deviceIndex].Number;

                if (pluginID==33) // Plugin 33 Dummy Device
                { // DummyValueSet,<task/device nr>,<value nr>,<value/formula (!ToDo) >, works only with new version of P033!
                  valueNr = findDeviceValueIndexByName(valueName, taskIndex);
                  if (valueNr != VARS_PER_TASK) // value Name identified
                  {
                    cmd = F("DummyValueSet,"); // Set a Dummy Device Value
                    cmd += (taskIndex+1); // set the device Number
                    cmd += F(",");
                    cmd += (valueNr+1); // set the value Number
                    cmd += F(",");
                    cmd += event->String2; // expect float as payload!
                    validTopic = true;
                  }
                } else if (pluginID==86) { // Plugin Homie receiver. Schedules the event defined in the plugin. Does NOT store the value. Use HomieValueSet to save the value. This will acknolage back to the controller too.
                  valueNr = findDeviceValueIndexByName(valueName, taskIndex);
                  if (valueNr != VARS_PER_TASK) {
                    cmd = F("event,");
                    cmd += valueName;
                    cmd += "=";
                    if (Settings.TaskDevicePluginConfig[taskIndex][valueNr]==3) { // Quote Sting parameters. PLUGIN_086_VALUE_STRING
                      cmd += '"';
                      cmd += event->String2;
                      cmd += '"';
                    } else {
                      if (Settings.TaskDevicePluginConfig[taskIndex][valueNr]==4) { // Enumeration parameter, find Number of item. PLUGIN_086_VALUE_ENUM
                        String enumList = ExtraTaskSettings.TaskDeviceFormula[taskVarIndex];
                        int i = 1;
                        while (!parseString(enumList,i).isEmpty()) { // lookup result in enum List
                          if (parseString(enumList,i)==event->String2) break;
                          i++;
                        }
                        cmd += i;
                        cmd += ",";
                      }
                      cmd += event->String2;
                    }
                    validTopic = true;
                  }
                }
              }
            }

            if (validTopic) {
              parseCommandString(&TempEvent, cmd);
              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                log+=F(" cmd: ");
                log+=cmd;
                addLog(LOG_LEVEL_INFO, log+ F(" OK"));
              }
            } else {
              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                addLog(LOG_LEVEL_INFO, log+ F(" INVALID MSG"));
              }
            }
          }

          if (validTopic) {
            // in case of event, store to buffer and return...
            String command = parseString(cmd, 1);
            if (command == F("event") || command == F("asyncevent"))
            {
              if (Settings.UseRules) {
                String newEvent = parseStringToEnd(cmd, 2);
                eventQueue.add(newEvent);
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  log=F("C014 : taskIndex:");
                  if (!validTaskIndex(taskIndex)) {
                    log += F("Invalid");
                  } else {
                    log+=taskIndex;
                    log+=F(" valueNr:");
                    log+=valueNr;
                    log+=F(" valueType:");
                    log+=Settings.TaskDevicePluginConfig[taskIndex][valueNr];
                  }
                  log+=F(" Event: ");
                  log+=newEvent;
                  addLog(LOG_LEVEL_INFO, log);
                }
              }
            } else { // not an event
              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                log=F("C014 :");
              }
              // FIXME TD-er: Command is not parsed, should we call ExecuteCommand here?
              if (ExecuteCommand_internal(EventValueSource::Enum::VALUE_SOURCE_MQTT, cmd.c_str())) {
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  log +=F(" Internal Command: OK!");
                }
              } else if (PluginCall(PLUGIN_WRITE, &TempEvent, cmd)) {
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  log +=F(" PluginCall: OK!");
                }
              } else {
                remoteConfig(&TempEvent, cmd);
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  log +=F(" Plugin/Internal command failed! remoteConfig?");
                }
              }
              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                addLog(LOG_LEVEL_INFO, log);
              }
            }

          }
        }
        success = validTopic;
        break;
      }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
      {
        String pubname = CPlugin_014_pubname;
        bool mqtt_retainFlag = CPlugin_014_mqtt_retainFlag;

        statusLED(true);

        parseControllerVariables(pubname, event, false);
        LoadTaskSettings(event->TaskIndex);

        uint8_t valueCount = getValueCountForTask(event->TaskIndex);
        for (uint8_t x = 0; x < valueCount; x++)
        {
          String tmppubname = pubname;
          String value;
          parseSingleControllerVariable(tmppubname, event, x, false);

          // Small optimization so we don't try to copy potentially large strings
          if (event->getSensorType() == Sensor_VType::SENSOR_TYPE_STRING) {
            MQTTpublish(event->ControllerIndex, event->TaskIndex, tmppubname.c_str(), event->String2.c_str(), mqtt_retainFlag);
            value = event->String2.substring(0, 20); // For the log
          } else {
            value = formatUserVarNoCheck(event, x);
            MQTTpublish(event->ControllerIndex, event->TaskIndex, tmppubname.c_str(), value.c_str(), mqtt_retainFlag);
          }
          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            String log = F("C014 : Sent to ");
            log += tmppubname;
            log += ' ';
            log += value;
            addLog(LOG_LEVEL_DEBUG, log);
          }
        }
        break;
      }

      case CPlugin::Function::CPLUGIN_ACKNOWLEDGE:
      {
        LoadTaskSettings(event->Par1-1);
/*        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log = F("CPLUGIN_ACKNOWLEDGE: ");
          log += string;
          log += F(" / ");
          log += ExtraTaskSettings.TaskDeviceName;
          log += F(" / ");
          log += ExtraTaskSettings.TaskDeviceValueNames[event->Par2-1];
          log += F(" sensorType:");
          log += event->sensorType;
          log += F(" Source:");
          log += event->Source;
          log += F(" idx:");
          log += event->idx;
          log += F(" S1:");
          log += event->String1;
          log += F(" S2:");
          log += event->String2;
          log += F(" S3:");
          log += event->String3;
          log += F(" S4:");
          log += event->String4;
          log += F(" S5:");
          log += event->String5;
          log += F(" P1:");
          log += event->Par1;
          log += F(" P2:");
          log += event->Par2;
          log += F(" P3:");
          log += event->Par3;
          log += F(" P4:");
          log += event->Par4;
          log += F(" P5:");
          log += event->Par5;
          addLog(LOG_LEVEL_DEBUG, log);
        } */
        success = false;
        if (!string.isEmpty()) {
          String commandName = parseString(string, 1); // could not find a way to get the command out of the event structure.
          if (commandName == F("gpio")) //!ToDo : As gpio is like any other plugin commands should be integrated below!
          {
            int port = event->Par1; // parseString(string, 2).toInt();
            int valueInt = event->Par2; //parseString(string, 3).toInt();
            String valueBool = "false";
            if (valueInt==1) valueBool = "true";

            String topic = CPLUGIN_014_PUBLISH; // ControllerSettings.Publish not used because it can be modified by the user!
            topic.replace(F("%sysname%"), Settings.Name);
            topic.replace(F("%tskname%"), CPLUGIN_014_SYSTEM_DEVICE);
            topic.replace(F("%valname%"), CPLUGIN_014_GPIO_VALUE + toString(port,0));

            success = MQTTpublish(CPLUGIN_ID_014, INVALID_TASK_INDEX, topic.c_str(), valueBool.c_str(), false);
            if (loglevelActiveFor(LOG_LEVEL_INFO) && success) {
              String log = F("C014 : Acknowledged GPIO");
              log += port;
              log += F(" value:");
              log += valueBool;
              log += F(" (");
              log += valueInt;
              log += F(")");
              addLog(LOG_LEVEL_INFO, log+" success!");
            }
            if (loglevelActiveFor(LOG_LEVEL_ERROR) && !success) {
              String log = F("C014 : Acknowledged GPIO");
              log += port;
              log += F(" value:");
              log += valueBool;
              log += F(" (");
              log += valueInt;
              log += F(")");
              addLog(LOG_LEVEL_ERROR, log+" ERROR!");
            }
          } else // not gpio
          {
            taskVarIndex_t taskVarIndex = event->Par2 - 1;
            if (validTaskVarIndex(taskVarIndex)) {
              userVarIndex_t userVarIndex = event->BaseVarIndex + taskVarIndex;
              String topic = CPLUGIN_014_PUBLISH;
              topic.replace(F("%sysname%"), Settings.Name);
              int deviceIndex = event->Par1; //parseString(string, 2).toInt();
              LoadTaskSettings(deviceIndex-1);
              String deviceName = ExtraTaskSettings.TaskDeviceName;
              topic.replace(F("%tskname%"), deviceName);
              String valueName = ExtraTaskSettings.TaskDeviceValueNames[event->Par2-1]; //parseString(string, 3).toInt()-1];
              topic.replace(F("%valname%"), valueName);
              String valueStr;
              int valueInt = 0;

              if ((commandName == F("taskvalueset")) || (commandName == F("dummyvalueset"))) // should work for both
              {
                valueStr = formatUserVarNoCheck(event, taskVarIndex); //parseString(string, 4);
                success = MQTTpublish(CPLUGIN_ID_014, INVALID_TASK_INDEX, topic.c_str(), valueStr.c_str(), false);
                if (loglevelActiveFor(LOG_LEVEL_INFO) && success) {
                  String log = F("C014 : Acknowledged: ");
                  log += deviceName;
                  log += F(" var: ");
                  log += valueName;
                  log += F(" topic: ");
                  log += topic;
                  log += F(" value: ");
                  log += valueStr;
                  addLog(LOG_LEVEL_INFO, log+" success!");
                }
                if (loglevelActiveFor(LOG_LEVEL_ERROR) && !success) {
                  String log = F("C014 : Aacknowledged: ");
                  log += deviceName;
                  log += F(" var: ");
                  log += valueName;
                  log += F(" topic: ");
                  log += topic;
                  log += F(" value: ");
                  log += valueStr;
                  addLog(LOG_LEVEL_ERROR, log+" ERROR!");
                }
              } else if (parseString(commandName, 1) == F("homievalueset")) { // acknolages value form P086 Homie Receiver
                switch (Settings.TaskDevicePluginConfig[deviceIndex-1][taskVarIndex]) {
                  case 0: // PLUGIN_085_VALUE_INTEGER
                    valueInt = static_cast<int>(UserVar[userVarIndex]);
                    valueStr = toString(UserVar[userVarIndex],0);
                    break;
                  case 1: // PLUGIN_085_VALUE_FLOAT
                    valueStr = formatUserVarNoCheck(event, taskVarIndex);
                    break;
                  case 2: // PLUGIN_085_VALUE_BOOLEAN
                    if ( UserVar[userVarIndex] == 1) valueStr="true";
                      else valueStr = "false";
                    break;
                  case 3: // PLUGIN_085_VALUE_STRING
                    //valueStr = ExtraTaskSettings.TaskDeviceFormula[taskVarIndex];
                    valueStr = parseStringToEndKeepCase(string,4);
                    break;
                  case 4: // PLUGIN_085_VALUE_ENUM
                    valueInt = static_cast<int>(UserVar[userVarIndex]);
                    valueStr = parseStringKeepCase(ExtraTaskSettings.TaskDeviceFormula[taskVarIndex],valueInt);
                    break;
                  case 5: // PLUGIN_085_VALUE_RGB
                    //valueStr = ExtraTaskSettings.TaskDeviceFormula[taskVarIndex];
                    valueStr = parseStringToEnd(string,4);
                    break;
                  case 6: // PLUGIN_085_VALUE_HSV
                    //valueStr = ExtraTaskSettings.TaskDeviceFormula[taskVarIndex];
                    valueStr = parseStringToEnd(string,4);
                    break;
                }
                success = MQTTpublish(CPLUGIN_ID_014, INVALID_TASK_INDEX, topic.c_str(), valueStr.c_str(), false);
                if (loglevelActiveFor(LOG_LEVEL_INFO) && success) {
                  String log = F("C014 : homie acknowledge: ");
                  log += deviceName;
                  log += F(" taskIndex:");
                  log += deviceIndex;
                  log += F(" valueNr:");
                  log += event->Par2;
                  log += F(" valueName:");
                  log += valueName;
                  log += F(" valueType:");
                  log += Settings.TaskDevicePluginConfig[deviceIndex-1][taskVarIndex];
                  log += F(" topic:");
                  log += topic;
                  log += F(" valueInt:");
                  log += valueInt;
                  log += F(" valueStr:");
                  log += valueStr;
                  addLog(LOG_LEVEL_INFO, log+" success!");
                }
                if (loglevelActiveFor(LOG_LEVEL_ERROR) && !success) {
                  String log = F("C014 : homie acknowledge: ");
                  log += deviceName;
                  log += F(" var: ");
                  log += valueName;
                  log += F(" topic: ");
                  log += topic;
                  log += F(" value: ");
                  log += valueStr;
                  addLog(LOG_LEVEL_ERROR, log+" failed!");
                }
              } else // Acknowledge not implemented yet
              {
  /*              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  String log = F("C014 : Plugin acknowledged: ");
                  log+=function;
                  log+=F(" / ");
                  log+=commandName;
                  log+=F(" cmd: ");
                  log+=string;
                  log+=F(" not implemented!");
                  addLog(LOG_LEVEL_ERROR, log);
                } */
                success = false;
              }
            }
          }
        }
        break;
      }

    default:
      break;

  }

  return success;
}
#endif
