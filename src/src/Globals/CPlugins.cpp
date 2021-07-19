#include "../Globals/CPlugins.h"

#include "../../_Plugin_Helper.h"
#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../DataStructs/TimingStats.h"
#include "../DataTypes/ESPEasy_plugin_functions.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/Protocol.h"
#include "../Globals/Settings.h"



// FIXME TD-er: Make these private and add functions to access its content.
std::map<cpluginID_t, protocolIndex_t> CPlugin_id_to_ProtocolIndex;
cpluginID_t ProtocolIndex_to_CPlugin_id[CPLUGIN_MAX + 1];

bool (*CPlugin_ptr[CPLUGIN_MAX])(CPlugin::Function,
                                 struct EventStruct *,
                                 String&);


/********************************************************************************************\
   Call CPlugin functions
 \*********************************************************************************************/
bool CPluginCall(CPlugin::Function Function, struct EventStruct *event) {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  String dummy;

  return CPluginCall(Function, event, dummy);
}

bool CPluginCall(CPlugin::Function Function, struct EventStruct *event, String& str)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif

  struct EventStruct TempEvent;

  if (event == 0) {
    event = &TempEvent;
  }

  switch (Function)
  {
    // Unconditional calls to all included controller in the build
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:

      for (protocolIndex_t x = 0; x < CPLUGIN_MAX; x++) {
        const cpluginID_t cpluginID = ProtocolIndex_to_CPlugin_id[x];
        if (validCPluginID(cpluginID)) {
          #ifndef BUILD_NO_RAM_TRACKER
          checkRAM(F("CPluginCallADD"), x);
          #endif

          /*
          #ifndef BUILD_NO_DEBUG
          const int freemem_begin = ESP.getFreeHeap();
          #endif
          */

          String dummy;
          CPluginCall(x, Function, event, dummy);
          /*
          #ifndef BUILD_NO_DEBUG
          if (Function == CPlugin::Function::CPLUGIN_PROTOCOL_ADD) {
            // See also logMemUsageAfter()
            const int freemem_end = ESP.getFreeHeap();
            String log = F("After CPLUGIN_ADD ");
            log += cpluginID;
            while (log.length() < 30) log += ' ';
            log += F("Free mem after: ");
            log += freemem_end;
            while (log.length() < 54) log += ' ';
            log += F("ctrlr: ");
            log += freemem_begin - freemem_end;

            addLog(LOG_LEVEL_INFO, log);
          }
          #endif
          */
        }
      }
      return true;


    // calls to all active controllers
    case CPlugin::Function::CPLUGIN_INIT_ALL:
    case CPlugin::Function::CPLUGIN_UDP_IN:
    case CPlugin::Function::CPLUGIN_INTERVAL:      // calls to send stats information
    case CPlugin::Function::CPLUGIN_GOT_CONNECTED: // calls to send autodetect information
    case CPlugin::Function::CPLUGIN_GOT_INVALID:   // calls to mark unit as invalid
    case CPlugin::Function::CPLUGIN_FLUSH:
    case CPlugin::Function::CPLUGIN_TEN_PER_SECOND:
    case CPlugin::Function::CPLUGIN_FIFTY_PER_SECOND:

      if (Function == CPlugin::Function::CPLUGIN_INIT_ALL) {
        Function = CPlugin::Function::CPLUGIN_INIT;
      }

      for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++) {
        if ((Settings.Protocol[x] != 0) && Settings.ControllerEnabled[x]) {
          protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(x);
          event->ControllerIndex = x;
          String dummy;
          CPluginCall(ProtocolIndex, Function, event, dummy);
        }
      }
      return true;

    // calls to specific controller
    case CPlugin::Function::CPLUGIN_INIT:
    case CPlugin::Function::CPLUGIN_EXIT:
    case CPlugin::Function::CPLUGIN_PROTOCOL_TEMPLATE:
    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    case CPlugin::Function::CPLUGIN_PROTOCOL_RECV:
    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    case CPlugin::Function::CPLUGIN_WEBFORM_LOAD:
    case CPlugin::Function::CPLUGIN_WEBFORM_SAVE:
    case CPlugin::Function::CPLUGIN_GET_PROTOCOL_DISPLAY_NAME:
    case CPlugin::Function::CPLUGIN_TASK_CHANGE_NOTIFICATION:
    case CPlugin::Function::CPLUGIN_WEBFORM_SHOW_HOST_CONFIG:
    {
      controllerIndex_t controllerindex = event->ControllerIndex;

      if (Settings.ControllerEnabled[controllerindex] && supportedCPluginID(Settings.Protocol[controllerindex]))
      {
        if (Function == CPlugin::Function::CPLUGIN_PROTOCOL_SEND) {
          checkDeviceVTypeForTask(event);
        }
        protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(controllerindex);
        CPluginCall(ProtocolIndex, Function, event, str);
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_ACKNOWLEDGE: // calls to send acknowledge back to controller

      for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++) {
        if (Settings.ControllerEnabled[x] && supportedCPluginID(Settings.Protocol[x])) {
          protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(x);
          CPluginCall(ProtocolIndex, Function, event, str);
        }
      }
      return true;
  }

  return false;
}

bool CPluginCall(protocolIndex_t protocolIndex, CPlugin::Function Function, struct EventStruct *event, String& str) {
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif
  if (validProtocolIndex(protocolIndex)) {
    #ifndef BUILD_NO_DEBUG
    const int freemem_begin = ESP.getFreeHeap();
    #endif

    START_TIMER;
    bool ret = CPlugin_ptr[protocolIndex](Function, event, str);
    STOP_TIMER_CONTROLLER(protocolIndex, Function);
    #ifndef BUILD_NO_DEBUG
    if (Function == CPlugin::Function::CPLUGIN_INIT) {
      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        // See also logMemUsageAfter()
        const int freemem_end = ESP.getFreeHeap();
        String log = F("After CPLUGIN_INIT ");
        while (log.length() < 30) log += ' ';
        log += F("Free mem after: ");
        log += freemem_end;
        while (log.length() < 54) log += ' ';
        log += F("ctrlr: ");
        log += freemem_begin - freemem_end;
        while (log.length() < 73) log += ' ';
        log += getCPluginNameFromProtocolIndex(protocolIndex);

        addLog(LOG_LEVEL_DEBUG, log);
      }
    }
    #endif
    return ret;
  }
  return false;
}

// Check if there is any controller enabled.
bool anyControllerEnabled() {
  for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++) {
    if (Settings.ControllerEnabled[x] && supportedCPluginID(Settings.Protocol[x])) {
      return true;
    }
  }
  return false;
}

// Find first enabled controller index with this protocol
controllerIndex_t findFirstEnabledControllerWithId(cpluginID_t cpluginid) {
  if (!supportedCPluginID(cpluginid)) {
    return INVALID_CONTROLLER_INDEX;
  }

  for (controllerIndex_t i = 0; i < CONTROLLER_MAX; i++) {
    if ((Settings.Protocol[i] == cpluginid) && Settings.ControllerEnabled[i]) {
      return i;
    }
  }
  return INVALID_CONTROLLER_INDEX;
}

bool validProtocolIndex(protocolIndex_t index)
{
  return getCPluginID_from_ProtocolIndex(index) != INVALID_C_PLUGIN_ID;
}

bool validControllerIndex(controllerIndex_t index)
{
  return index < CONTROLLER_MAX;
}

bool validCPluginID(cpluginID_t cpluginID)
{
  if (cpluginID == INVALID_C_PLUGIN_ID) {
    return false;
  }
  auto it = CPlugin_id_to_ProtocolIndex.find(cpluginID);
  return it != CPlugin_id_to_ProtocolIndex.end();
}

bool supportedCPluginID(cpluginID_t cpluginID)
{
  return validProtocolIndex(getProtocolIndex(cpluginID));
}

protocolIndex_t getProtocolIndex_from_ControllerIndex(controllerIndex_t index) {
  if (validControllerIndex(index)) {
    return getProtocolIndex(Settings.Protocol[index]);
  }
  return INVALID_PROTOCOL_INDEX;
}

cpluginID_t getCPluginID_from_ProtocolIndex(protocolIndex_t index) {
  if (index < CPLUGIN_MAX) {
    const cpluginID_t cpluginID = ProtocolIndex_to_CPlugin_id[index];
    return cpluginID;
  }
  return INVALID_C_PLUGIN_ID;
}

cpluginID_t getCPluginID_from_ControllerIndex(controllerIndex_t index) {
  const protocolIndex_t protocolIndex = getProtocolIndex_from_ControllerIndex(index);

  return getCPluginID_from_ProtocolIndex(protocolIndex);
}

protocolIndex_t getProtocolIndex(cpluginID_t cpluginID)
{
  if (cpluginID != INVALID_C_PLUGIN_ID) {
    auto it = CPlugin_id_to_ProtocolIndex.find(cpluginID);

    if (it != CPlugin_id_to_ProtocolIndex.end())
    {
      if (!validProtocolIndex(it->second)) { return INVALID_PROTOCOL_INDEX; }
      #ifndef BUILD_NO_DEBUG
      if (Protocol[it->second].Number != cpluginID) {
        // FIXME TD-er: Just a check for now, can be removed later when it does not occur.
        String log = F("getProtocolIndex error in Protocol Vector. CPluginID: ");
        log += String(cpluginID);
        log += F(" p_index: ");
        log += String(it->second);
        addLog(LOG_LEVEL_ERROR, log);
      }
      #endif
      return it->second;
    }
  }
  return INVALID_PROTOCOL_INDEX;
}

String getCPluginNameFromProtocolIndex(protocolIndex_t ProtocolIndex) {
  String controllerName;

  if (validProtocolIndex(ProtocolIndex)) {
    CPlugin_ptr[ProtocolIndex](CPlugin::Function::CPLUGIN_GET_DEVICENAME, nullptr, controllerName);
  }
  return controllerName;
}

String getCPluginNameFromCPluginID(cpluginID_t cpluginID) {
  protocolIndex_t protocolIndex = getProtocolIndex(cpluginID);

  if (!validProtocolIndex(protocolIndex)) {
    String name = F("CPlugin ");
    name += String(static_cast<int>(cpluginID));
    name += F(" not included in build");
    return name;
  }
  return getCPluginNameFromProtocolIndex(protocolIndex);
}


bool addCPlugin(cpluginID_t cpluginID, protocolIndex_t x) {
  if (x < CPLUGIN_MAX) { 
    ProtocolIndex_to_CPlugin_id[x] = cpluginID; 
    CPlugin_id_to_ProtocolIndex[cpluginID] = x;
    return true;
  }
  String log = F("System: Error - Too many C-Plugins. CPLUGIN_MAX = ");
  log += CPLUGIN_MAX;
  addLog(LOG_LEVEL_ERROR, log);
  return false;
}
