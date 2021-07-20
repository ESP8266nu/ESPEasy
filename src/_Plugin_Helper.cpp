#include "_Plugin_Helper.h"

#include "ESPEasy_common.h"

#include "src/CustomBuild/ESPEasyLimits.h"
#include "src/DataStructs/SettingsStruct.h"
#include "src/Globals/Plugins.h"
#include "src/Globals/Settings.h"
#include "src/Globals/SecuritySettings.h"
#include "src/Helpers/Misc.h"
#include "src/Helpers/StringParser.h"


PluginTaskData_base *Plugin_task_data[TASKS_MAX] = { nullptr, };

String PCONFIG_LABEL(int n) {
  if (n < PLUGIN_CONFIGVAR_MAX) {
    String result = F("pconf_");
    result += n;
    return result;
  }
  return F("error");
}

void resetPluginTaskData() {
  for (taskIndex_t i = 0; i < TASKS_MAX; ++i) {
    Plugin_task_data[i] = nullptr;
  }
}

void clearPluginTaskData(taskIndex_t taskIndex) {
  if (validTaskIndex(taskIndex)) {
    if (Plugin_task_data[taskIndex] != nullptr) {
      delete Plugin_task_data[taskIndex];
      Plugin_task_data[taskIndex] = nullptr;
    }
  }
}

void initPluginTaskData(taskIndex_t taskIndex, PluginTaskData_base *data) {
  if (!validTaskIndex(taskIndex)) {
    if (data != nullptr) {
      delete data;
    }
    return;
  }

  clearPluginTaskData(taskIndex);

  if (data == nullptr) {
    return;
  }

  if (Settings.TaskDeviceEnabled[taskIndex]) {
    Plugin_task_data[taskIndex]                     = data;
    Plugin_task_data[taskIndex]->_taskdata_pluginID = Settings.TaskDeviceNumber[taskIndex];
  } else if (data != nullptr) {
    delete data;
  }
}

PluginTaskData_base* getPluginTaskData(taskIndex_t taskIndex) {
  if (pluginTaskData_initialized(taskIndex)) {
    return Plugin_task_data[taskIndex];
  }
  return nullptr;
}

bool pluginTaskData_initialized(taskIndex_t taskIndex) {
  if (!validTaskIndex(taskIndex)) {
    return false;
  }
  return Plugin_task_data[taskIndex] != nullptr &&
         (Plugin_task_data[taskIndex]->_taskdata_pluginID == Settings.TaskDeviceNumber[taskIndex]);
}

String getPluginCustomArgName(int varNr) {
  String argName = F("pc_arg");

  argName += varNr + 1;
  return argName;
}

int getFormItemIntCustomArgName(int varNr) {
  return getFormItemInt(getPluginCustomArgName(varNr));
}

// Helper function to create formatted custom values for display in the devices overview page.
// When called from PLUGIN_WEBFORM_SHOW_VALUES, the last item should add a traling div_br class
// if the regular values should also be displayed.
// The call to PLUGIN_WEBFORM_SHOW_VALUES should only return success = true when no regular values should be displayed
// Note that the varNr of the custom values should not conflict with the existing variable numbers (e.g. start at VARS_PER_TASK)
void pluginWebformShowValue(taskIndex_t taskIndex, uint8_t varNr, const __FlashStringHelper * label, const String& value, bool addTrailingBreak) {
  pluginWebformShowValue(taskIndex, varNr, String(label), value, addTrailingBreak);
}

void pluginWebformShowValue(taskIndex_t   taskIndex,
                            uint8_t          varNr,
                            const String& label,
                            const String& value,
                            bool          addTrailingBreak) {
  if (varNr > 0) {
    addHtmlDiv(F("div_br"));
  }

  pluginWebformShowValue(
    label, String(F("valuename_")) + taskIndex + '_' + varNr,
    value, String(F("value_")) + taskIndex + '_' + varNr,
    addTrailingBreak);
}

void pluginWebformShowValue(const String& valName, const String& value, bool addBR) {
  pluginWebformShowValue(valName, EMPTY_STRING, value, EMPTY_STRING, addBR);
}

void pluginWebformShowValue(const String& valName, const String& valName_id, const String& value, const String& value_id, bool addBR) {
  String valName_tmp(valName);

  if (!valName_tmp.endsWith(F(":"))) {
    valName_tmp += ':';
  }
  addHtmlDiv(F("div_l"), valName_tmp, valName_id);
  addHtmlDiv(F("div_r"), value,       value_id);

  if (addBR) {
    addHtmlDiv(F("div_br"));
  }
}

bool pluginOptionalTaskIndexArgumentMatch(taskIndex_t taskIndex, const String& string, uint8_t paramNr) {
  if (!validTaskIndex(taskIndex)) {
    return false;
  }
  const taskIndex_t found_taskIndex = parseCommandArgumentTaskIndex(string, paramNr);

  if (!validTaskIndex(found_taskIndex)) {
    // Optional parameter not present
    return true;
  }
  return found_taskIndex == taskIndex;
}

bool pluginWebformShowGPIOdescription(taskIndex_t taskIndex, const String& newline)
{
  struct EventStruct TempEvent(taskIndex);
  TempEvent.String1 = newline;
  String dummy;
  return PluginCall(PLUGIN_WEBFORM_SHOW_GPIO_DESCR, &TempEvent, dummy);
}

int getValueCountForTask(taskIndex_t taskIndex) {
  struct EventStruct TempEvent(taskIndex);
  String dummy;

  PluginCall(PLUGIN_GET_DEVICEVALUECOUNT, &TempEvent, dummy);
  return TempEvent.Par1;
}

int checkDeviceVTypeForTask(struct EventStruct *event) {
  if (event->sensorType == Sensor_VType::SENSOR_TYPE_NOT_SET) {
    if (validTaskIndex(event->TaskIndex)) {
      String dummy;

      if (PluginCall(PLUGIN_GET_DEVICEVTYPE, event, dummy)) {
        return event->idx; // pconfig_index
      }
    }
  }
  return -1;
}
