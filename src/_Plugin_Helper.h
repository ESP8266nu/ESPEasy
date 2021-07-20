#ifndef PLUGIN_HELPER_H
#define PLUGIN_HELPER_H

#include <Arduino.h>

#include "ESPEasy_common.h"

#include "src/CustomBuild/ESPEasyLimits.h"

#include "src/DataStructs/DeviceStruct.h"
#include "src/DataStructs/ESPEasy_EventStruct.h"
#include "src/DataStructs/PinMode.h"

#include "src/DataTypes/ESPEasy_plugin_functions.h"

#include "src/ESPEasyCore/Controller.h"
#include "src/ESPEasyCore/ESPEasy_Log.h"

#include "src/Globals/Device.h"
#include "src/Globals/ESPEasy_Scheduler.h"
#include "src/Globals/ESPEasy_time.h"
#include "src/Globals/EventQueue.h"
#include "src/Globals/ExtraTaskSettings.h"
#include "src/Globals/GlobalMapPortStatus.h"
#include "src/Globals/I2Cdev.h"
#include "src/Globals/Plugins.h"
#include "src/Globals/RuntimeData.h"
#include "src/Globals/Settings.h"

#include "src/Helpers/ESPEasy_math.h"
#include "src/Helpers/ESPEasy_Storage.h"
#include "src/Helpers/ESPEasy_time_calc.h"
#include "src/Helpers/I2C_access.h"
#include "src/Helpers/Misc.h"
#include "src/Helpers/Numerical.h"
#include "src/Helpers/PortStatus.h"
#include "src/Helpers/StringConverter.h"
#include "src/Helpers/StringGenerator_GPIO.h"
#include "src/Helpers/StringParser.h"
#include "src/Helpers/_Plugin_SensorTypeHelper.h"
#include "src/Helpers/_Plugin_Helper_serial.h"

#include "src/WebServer/HTML_wrappers.h"
#include "src/WebServer/Markup.h"
#include "src/WebServer/Markup_Forms.h"
#include "src/WebServer/WebServer.h"


// Defines to make plugins more readable.

#ifndef PCONFIG
  # define PCONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][(n)])
#endif // ifndef PCONFIG
#ifndef PCONFIG_FLOAT
  # define PCONFIG_FLOAT(n) (Settings.TaskDevicePluginConfigFloat[event->TaskIndex][(n)])
#endif // ifndef PCONFIG_FLOAT
#ifndef PCONFIG_LONG
  # define PCONFIG_LONG(n) (Settings.TaskDevicePluginConfigLong[event->TaskIndex][(n)])
#endif // ifndef PCONFIG_LONG
#ifndef PIN

// Please note the 'offset' of N compared to normal pin numbering.
  # define PIN(n) (Settings.TaskDevicePin[n][event->TaskIndex])
#endif // ifndef PIN
#ifndef CONFIG_PIN1
  # define CONFIG_PIN1 (Settings.TaskDevicePin1[event->TaskIndex])
#endif // ifndef CONFIG_PIN1
#ifndef CONFIG_PIN2
  # define CONFIG_PIN2 (Settings.TaskDevicePin2[event->TaskIndex])
#endif // ifndef CONFIG_PIN2
#ifndef CONFIG_PIN3
  # define CONFIG_PIN3 (Settings.TaskDevicePin3[event->TaskIndex])
#endif // ifndef CONFIG_PIN3
#ifndef CONFIG_PORT
  # define CONFIG_PORT (Settings.TaskDevicePort[event->TaskIndex])
#endif // ifndef CONFIG_PORT

String PCONFIG_LABEL(int n);

// ==============================================
// Data used by instances of plugins.
// =============================================

// base class to be able to delete a data object from the array.
// N.B. in order to use this, a data object must inherit from this base class.
//      This is a compile time check.
struct PluginTaskData_base {
  virtual ~PluginTaskData_base() {}

  // We cannot use dynamic_cast, so we must keep track of the plugin ID to
  // perform checks on the casting.
  // This is also a check to only use these functions and not to insert pointers
  // at random in the Plugin_task_data array.
  pluginID_t _taskdata_pluginID = INVALID_PLUGIN_ID;
};


void                 resetPluginTaskData();

void                 clearPluginTaskData(taskIndex_t taskIndex);

void                 initPluginTaskData(taskIndex_t          taskIndex,
                                        PluginTaskData_base *data);

PluginTaskData_base* getPluginTaskData(taskIndex_t taskIndex);

bool                 pluginTaskData_initialized(taskIndex_t taskIndex);

String               getPluginCustomArgName(int varNr);

int                  getFormItemIntCustomArgName(int varNr);

// Helper function to create formatted custom values for display in the devices overview page.
// When called from PLUGIN_WEBFORM_SHOW_VALUES, the last item should add a traling div_br class
// if the regular values should also be displayed.
// The call to PLUGIN_WEBFORM_SHOW_VALUES should only return success = true when no regular values should be displayed
// Note that the varNr of the custom values should not conflict with the existing variable numbers (e.g. start at VARS_PER_TASK)
void pluginWebformShowValue(taskIndex_t   taskIndex,
                            uint8_t          varNr,
                            const __FlashStringHelper * label,
                            const String& value,
                            bool          addTrailingBreak = false);

void pluginWebformShowValue(taskIndex_t   taskIndex,
                            uint8_t          varNr,
                            const String& label,
                            const String& value,
                            bool          addTrailingBreak = false);

void pluginWebformShowValue(const String& valName,
                            const String& value,
                            bool          addBR = true);
void pluginWebformShowValue(const String& valName,
                            const String& valName_id,
                            const String& value,
                            const String& value_id,
                            bool          addBR = true);

// Check if given parameter nr matches with given taskIndex.
// paramNr == 0 -> command, paramNr == 1 -> 1st parameter
// When there is no parameter at given parameter position, this function will return true. (as it is an optional parameter)
// When given taskIndex is invalid, return value is false.
// Return if parameter at given paramNr matches given taskIndex.
bool pluginOptionalTaskIndexArgumentMatch(taskIndex_t   taskIndex,
                                          const String& string,
                                          uint8_t          paramNr);

bool pluginWebformShowGPIOdescription(taskIndex_t taskIndex, const String& newline);

int getValueCountForTask(taskIndex_t taskIndex);

// Check if the DeviceVType is set and update if it isn't.
// Return pconfig_index
int checkDeviceVTypeForTask(struct EventStruct *event);

#endif // PLUGIN_HELPER_H
