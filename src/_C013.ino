#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C013

# include "src/Globals/Nodes.h"
# include "src/DataStructs/C013_p2p_dataStructs.h"
# include "src/ESPEasyCore/ESPEasyRules.h"
# include "src/Helpers/Misc.h"
# include "src/Helpers/Network.h"

// #######################################################################################################
// ########################### Controller Plugin 013: ESPEasy P2P network ################################
// #######################################################################################################

# define CPLUGIN_013
# define CPLUGIN_ID_013         13
# define CPLUGIN_NAME_013       "ESPEasy P2P Networking"

WiFiUDP C013_portUDP;


bool CPlugin_013(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      Protocol[++protocolCount].Number     = CPLUGIN_ID_013;
      Protocol[protocolCount].usesMQTT     = false;
      Protocol[protocolCount].usesTemplate = false;
      Protocol[protocolCount].usesAccount  = false;
      Protocol[protocolCount].usesPassword = false;
      Protocol[protocolCount].defaultPort  = 65501;
      Protocol[protocolCount].usesID       = false;
      Protocol[protocolCount].Custom       = true;
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_013);
      break;
    }

    case CPlugin::Function::CPLUGIN_TASK_CHANGE_NOTIFICATION:
    {
      C013_SendUDPTaskInfo(0, event->TaskIndex, event->TaskIndex);
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      C013_SendUDPTaskData(0, event->TaskIndex, event->TaskIndex);
      break;
    }

    case CPlugin::Function::CPLUGIN_UDP_IN:
    {
      C013_Receive(event);
      break;
    }

    /*
        case CPlugin::Function::CPLUGIN_FLUSH:
          {
            process_c013_delay_queue(event->ControllerIndex);
            delay(0);
            break;
          }
     */

    default:
      break;
  }
  return success;
}

// ********************************************************************************
// Generic UDP message
// ********************************************************************************
void C013_SendUDPTaskInfo(uint8_t destUnit, uint8_t sourceTaskIndex, uint8_t destTaskIndex)
{
  if (!NetworkConnected(NETWORK_CONNECTED_TIMEOUT)) {
    return;
  }

  if (!validTaskIndex(sourceTaskIndex) || !validTaskIndex(destTaskIndex)) {
    return;
  }
  pluginID_t pluginID = Settings.TaskDeviceNumber[sourceTaskIndex];

  if (!validPluginID_fullcheck(pluginID)) {
    return;
  }

  struct C013_SensorInfoStruct infoReply;

  infoReply.sourceUnit      = Settings.Unit;
  infoReply.sourceTaskIndex = sourceTaskIndex;
  infoReply.destTaskIndex   = destTaskIndex;
  infoReply.deviceNumber    = pluginID;
  LoadTaskSettings(infoReply.sourceTaskIndex);
  safe_strncpy(infoReply.taskName, getTaskDeviceName(infoReply.sourceTaskIndex), sizeof(infoReply.taskName));

  for (uint8_t x = 0; x < VARS_PER_TASK; x++) {
    safe_strncpy(infoReply.ValueNames[x], ExtraTaskSettings.TaskDeviceValueNames[x], sizeof(infoReply.ValueNames[x]));
  }

  if (destUnit != 0)
  {
    infoReply.destUnit = destUnit;
    C013_sendUDP(destUnit, (uint8_t *)&infoReply, sizeof(C013_SensorInfoStruct));
    delay(10);
  } else {
    for (NodesMap::iterator it = Nodes.begin(); it != Nodes.end(); ++it) {
      if (it->first != Settings.Unit) {
        infoReply.destUnit = it->first;
        C013_sendUDP(it->first, (uint8_t *)&infoReply, sizeof(C013_SensorInfoStruct));
        delay(10);
      }
    }
  }
  delay(50);
}

void C013_SendUDPTaskData(uint8_t destUnit, uint8_t sourceTaskIndex, uint8_t destTaskIndex)
{
  if (!NetworkConnected(NETWORK_CONNECTED_TIMEOUT)) {
    return;
  }
  struct C013_SensorDataStruct dataReply;

  dataReply.sourceUnit      = Settings.Unit;
  dataReply.sourceTaskIndex = sourceTaskIndex;
  dataReply.destTaskIndex   = destTaskIndex;

  for (uint8_t x = 0; x < VARS_PER_TASK; x++) {
    const userVarIndex_t userVarIndex = dataReply.sourceTaskIndex * VARS_PER_TASK + x;

    if (validUserVarIndex(userVarIndex)) {
      dataReply.Values[x] = UserVar[userVarIndex];
    }
  }

  if (destUnit != 0)
  {
    dataReply.destUnit = destUnit;
    C013_sendUDP(destUnit, (uint8_t *)&dataReply, sizeof(C013_SensorDataStruct));
    delay(10);
  } else {
    for (NodesMap::iterator it = Nodes.begin(); it != Nodes.end(); ++it) {
      if (it->first != Settings.Unit) {
        dataReply.destUnit = it->first;
        C013_sendUDP(it->first, (uint8_t *)&dataReply, sizeof(C013_SensorDataStruct));
        delay(10);
      }
    }
  }
  delay(50);
}

/*********************************************************************************************\
   Send UDP message (unit 255=broadcast)
\*********************************************************************************************/
void C013_sendUDP(uint8_t unit, uint8_t *data, uint8_t size)
{
  if (!NetworkConnected(NETWORK_CONNECTED_TIMEOUT)) {
    return;
  }
  NodesMap::iterator it;

  if (unit != 255) {
    it = Nodes.find(unit);

    if (it == Nodes.end()) {
      return;
    }

    if (it->second.ip[0] == 0) {
      return;
    }
  }
# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
    String log = F("C013 : Send UDP message to ");
    log += unit;
    addLog(LOG_LEVEL_DEBUG_MORE, log);
  }
# endif // ifndef BUILD_NO_DEBUG

  statusLED(true);

  IPAddress remoteNodeIP;

  if (unit == 255) {
    remoteNodeIP = { 255, 255, 255, 255 };
  }
  else {
    remoteNodeIP = it->second.ip;
  }

  if (!beginWiFiUDP_randomPort(C013_portUDP)) { return; }

  FeedSW_watchdog();
  if (C013_portUDP.beginPacket(remoteNodeIP, Settings.UDPPort) == 0) { return; }
  C013_portUDP.write(data, size);
  C013_portUDP.endPacket();
  C013_portUDP.stop();
  FeedSW_watchdog();
  delay(0);
}

void C013_Receive(struct EventStruct *event) {
  if (event->Par2 < 6) { return; }
# ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
    if ((event->Data[1] > 1) && (event->Data[1] < 6))
    {
      String log = (F("C013 : msg "));

      for (uint8_t x = 1; x < 6; x++)
      {
        log += ' ';
        log += (int)event->Data[x];
      }
      addLog(LOG_LEVEL_DEBUG_MORE, log);
    }
  }
# endif // ifndef BUILD_NO_DEBUG

  switch (event->Data[1]) {
    case 2: // sensor info pull request
    {
      // SendUDPTaskInfo(packetBuffer[2], packetBuffer[5], packetBuffer[4]);
      break;
    }

    case 3: // sensor info
    {
      struct C013_SensorInfoStruct infoReply;
      int count = sizeof(C013_SensorInfoStruct);

      if (event->Par2 < count) { count = event->Par2; }

      memcpy((uint8_t *)&infoReply, (uint8_t *)event->Data, count);

      if (infoReply.isValid()) {
        // to prevent flash wear out (bugs in communication?) we can only write to an empty task
        // so it will write only once and has to be cleared manually through webgui
        // Also check the receiving end does support the plugin ID.
        if (!validPluginID_fullcheck(Settings.TaskDeviceNumber[infoReply.destTaskIndex]) &&
            supportedPluginID(infoReply.deviceNumber))
        {
          taskClear(infoReply.destTaskIndex, false);
          Settings.TaskDeviceNumber[infoReply.destTaskIndex]   = infoReply.deviceNumber;
          Settings.TaskDeviceDataFeed[infoReply.destTaskIndex] = infoReply.sourceUnit; // remote feed store unit nr sending the data

          for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++) {
            Settings.TaskDeviceSendData[x][infoReply.destTaskIndex] = false;
          }
          safe_strncpy(ExtraTaskSettings.TaskDeviceName, infoReply.taskName, sizeof(infoReply.taskName));

          for (uint8_t x = 0; x < VARS_PER_TASK; x++) {
            safe_strncpy(ExtraTaskSettings.TaskDeviceValueNames[x], infoReply.ValueNames[x], sizeof(infoReply.ValueNames[x]));
          }
          ExtraTaskSettings.TaskIndex = infoReply.destTaskIndex;
          SaveTaskSettings(infoReply.destTaskIndex);
          SaveSettings();
        }
      }
      break;
    }

    case 4: // sensor data pull request
    {
      // SendUDPTaskData(packetBuffer[2], packetBuffer[5], packetBuffer[4]);
      break;
    }

    case 5: // sensor data
    {
      struct C013_SensorDataStruct dataReply;
      int count = sizeof(C013_SensorDataStruct);

      if (event->Par2 < count) { count = event->Par2; }
      memcpy((uint8_t *)&dataReply, (uint8_t *)event->Data, count);

      if (dataReply.isValid()) {
        // only if this task has a remote feed, update values
        const uint8_t remoteFeed = Settings.TaskDeviceDataFeed[dataReply.destTaskIndex];

        if ((remoteFeed != 0) && (remoteFeed == dataReply.sourceUnit))
        {
          for (uint8_t x = 0; x < VARS_PER_TASK; x++)
          {
            UserVar[dataReply.destTaskIndex * VARS_PER_TASK + x] = dataReply.Values[x];
          }

          if (Settings.UseRules) {
            struct EventStruct TempEvent(dataReply.destTaskIndex);
            createRuleEvents(&TempEvent);
          }
        }
      }
      break;
    }
  }
}

#endif // ifdef USES_C013
