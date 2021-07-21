#include "C019_ESPEasyNow_helper.h"

#include "../../DataStructs/ESPEasy_EventStruct.h"
#include "../../ESPEasyCore/ESPEasy_Log.h"
#include "../../Globals/CPlugins.h"


bool C019_ESPEasyNow_helper::process_receive(struct EventStruct *event) {
  controllerIndex_t ControllerID = findFirstEnabledControllerWithId(19 /* CPLUGIN_ID_019 */ );

  if (!validControllerIndex(ControllerID)) {
    // Controller is not enabled.
    return false;
  }

  if ((event->Data == nullptr) || (event->Par1 != sizeof(ESPEasy_Now_p2p_data))) {
    return false;
  }
  ESPEasy_Now_p2p_data *data = reinterpret_cast<ESPEasy_Now_p2p_data *>(event->Data);

  if (!data->validate()) {
    // return false;
  }

  String log = F("ESPEasy_Now PluginData: ");

  log += data->plugin_id;
  log += F(" sourceUnit: ");
  log += data->sourceUnit;
  addLog(LOG_LEVEL_INFO, log);

  switch (data->dataType) {
    case ESPEasy_Now_p2p_data_type::PluginData:
    {
      C019_ESPEasyNow_helper::process_received_PluginData(*data);
      break;
    }
    default:
      break;
  }

  return true;
}

void C019_ESPEasyNow_helper::process_received_PluginData(const ESPEasy_Now_p2p_data& data) {
    
}
