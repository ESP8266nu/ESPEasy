#include "Plugins.h"

#include "../CustomBuild/ESPEasyLimits.h"

#include "../../_Plugin_Helper.h"

#include "../DataStructs/ESPEasy_EventStruct.h"
#include "../DataStructs/TimingStats.h"

#include "../DataTypes/ESPEasy_plugin_functions.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/Serial.h"

#include "../Globals/Cache.h"
#include "../Globals/Device.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/ExtraTaskSettings.h"
#include "../Globals/EventQueue.h"
#include "../Globals/GlobalMapPortStatus.h"
#include "../Globals/Settings.h"

#include "../Helpers/ESPEasyRTC.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Misc.h"
#include "../Helpers/PortStatus.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"



int deviceCount = -1;

boolean (*Plugin_ptr[PLUGIN_MAX])(uint8_t,
                                  struct EventStruct *,
                                  String&);

pluginID_t DeviceIndex_to_Plugin_id[PLUGIN_MAX + 1];
std::map<pluginID_t, deviceIndex_t> Plugin_id_to_DeviceIndex;
std::vector<deviceIndex_t> DeviceIndex_sorted;


bool validDeviceIndex(deviceIndex_t index) {
  if (index < PLUGIN_MAX) {
    const pluginID_t pluginID = DeviceIndex_to_Plugin_id[index];
    return pluginID != INVALID_PLUGIN_ID;
  }
  return false;
}

bool validTaskIndex(taskIndex_t index) {
  return index < TASKS_MAX;
}

bool validPluginID(pluginID_t pluginID) {
  return (pluginID != INVALID_PLUGIN_ID);
}

bool validPluginID_fullcheck(pluginID_t pluginID) {
  if (!validPluginID(pluginID)) {
    return false;
  }
  auto it = Plugin_id_to_DeviceIndex.find(pluginID);
  return (it != Plugin_id_to_DeviceIndex.end());
}

bool validUserVarIndex(userVarIndex_t index) {
  return index < USERVAR_MAX_INDEX;
}

bool validTaskVarIndex(taskVarIndex_t index) {
  return index < VARS_PER_TASK;
}

bool supportedPluginID(pluginID_t pluginID) {
  return validDeviceIndex(getDeviceIndex(pluginID));
}

deviceIndex_t getDeviceIndex_from_TaskIndex(taskIndex_t taskIndex) {
  if (validTaskIndex(taskIndex)) {
    return getDeviceIndex(Settings.TaskDeviceNumber[taskIndex]);
  }
  return INVALID_DEVICE_INDEX;
}

pluginID_t getPluginID_from_TaskIndex(taskIndex_t taskIndex) {
  deviceIndex_t deviceIndex = getDeviceIndex_from_TaskIndex(taskIndex);

  if (deviceIndex == INVALID_DEVICE_INDEX) {
    return INVALID_PLUGIN_ID;
  }
  return DeviceIndex_to_Plugin_id[deviceIndex];
}

deviceIndex_t getDeviceIndex(pluginID_t pluginID)
{
  if (pluginID != INVALID_PLUGIN_ID) {
    auto it = Plugin_id_to_DeviceIndex.find(pluginID);

    if (it != Plugin_id_to_DeviceIndex.end())
    {
      if (!validDeviceIndex(it->second)) { return INVALID_DEVICE_INDEX; }

      if (Device[it->second].Number != pluginID) {
        // FIXME TD-er: Just a check for now, can be removed later when it does not occur.
        addLog(LOG_LEVEL_ERROR, F("getDeviceIndex error in Device Vector"));
      }
      return it->second;
    }
  }
  return INVALID_DEVICE_INDEX;
}

/********************************************************************************************\
   Find name of plugin given the plugin device index..
 \*********************************************************************************************/
String getPluginNameFromDeviceIndex(deviceIndex_t deviceIndex) {
  String deviceName = "";

  if (validDeviceIndex(deviceIndex)) {
    Plugin_ptr[deviceIndex](PLUGIN_GET_DEVICENAME, nullptr, deviceName);
  }
  return deviceName;
}

String getPluginNameFromPluginID(pluginID_t pluginID) {
  deviceIndex_t deviceIndex = getDeviceIndex(pluginID);

  if (!validDeviceIndex(deviceIndex)) {
    String name = F("Plugin ");
    name += String(static_cast<int>(pluginID));
    name += F(" not included in build");
    return name;
  }
  return getPluginNameFromDeviceIndex(deviceIndex);
}

// ********************************************************************************
// Device Sort routine, compare two array entries
// ********************************************************************************
bool arrayLessThan(const String& ptr_1, const String& ptr_2)
{
  unsigned int i = 0;

  while (i < ptr_1.length()) // For each character in string 1, starting with the first:
  {
    if (ptr_2.length() < i)  // If string 2 is shorter, then switch them
    {
      return true;
    }
    const char check1 = static_cast<char>(ptr_1[i]); // get the same char from string 1 and string 2
    const char check2 = static_cast<char>(ptr_2[i]);

    if (check1 == check2) {
      // they're equal so far; check the next char !!
      i++;
    } else {
      return check2 > check1;
    }
  }
  return false;
}

// ********************************************************************************
// Device Sort routine, actual sorting alfabetically by plugin name.
// Sorting does happen case sensitive.
// ********************************************************************************
void sortDeviceIndexArray() {
  // First fill the existing number of the DeviceIndex.
  DeviceIndex_sorted.resize(deviceCount + 1);

  for (deviceIndex_t x = 0; x <= deviceCount; x++) {
    if (validPluginID(DeviceIndex_to_Plugin_id[x])) {
      DeviceIndex_sorted[x] = x;
    } else {
      DeviceIndex_sorted[x] = INVALID_DEVICE_INDEX;
    }
  }

  // Do the sorting.
  int innerLoop;
  int mainLoop;

  for (mainLoop = 1; mainLoop <= deviceCount; mainLoop++)
  {
    innerLoop = mainLoop;

    while (innerLoop  >= 1)
    {
      if (arrayLessThan(
            getPluginNameFromDeviceIndex(DeviceIndex_sorted[innerLoop]),
            getPluginNameFromDeviceIndex(DeviceIndex_sorted[innerLoop - 1])))
      {
        deviceIndex_t temp = DeviceIndex_sorted[innerLoop - 1];
        DeviceIndex_sorted[innerLoop - 1] = DeviceIndex_sorted[innerLoop];
        DeviceIndex_sorted[innerLoop]     = temp;
      }
      innerLoop--;
    }
  }
}

// ********************************************************************************
// Functions to assist changing I2C multiplexer port or clock speed 
// when addressing a task
// ********************************************************************************

void prepare_I2C_by_taskIndex(taskIndex_t taskIndex, deviceIndex_t DeviceIndex) {
  if (!validTaskIndex(taskIndex) || !validDeviceIndex(DeviceIndex)) {
    return;
  }
  if (Device[DeviceIndex].Type != DEVICE_TYPE_I2C) {
    return;
  }
#ifdef FEATURE_I2CMULTIPLEXER
  I2CMultiplexerSelectByTaskIndex(taskIndex);
  // Output is selected after this write, so now we must make sure the
  // frequency is set before anything else is sent.
#endif

  if (bitRead(Settings.I2C_Flags[taskIndex], I2C_FLAGS_SLOW_SPEED)) {
    I2CSelectClockSpeed(true); // Set to slow
  }
}


void post_I2C_by_taskIndex(taskIndex_t taskIndex, deviceIndex_t DeviceIndex) {
  if (!validTaskIndex(taskIndex) || !validDeviceIndex(DeviceIndex)) {
    return;
  }
  if (Device[DeviceIndex].Type != DEVICE_TYPE_I2C) {
    return;
  }
#ifdef FEATURE_I2CMULTIPLEXER
  I2CMultiplexerOff();
#endif

  if (bitRead(Settings.I2C_Flags[taskIndex], I2C_FLAGS_SLOW_SPEED)) {
    I2CSelectClockSpeed(false);  // Reset
  }
}

// Add an event to the event queue.
// event value 1 = taskIndex (first task = 1)
// event value 2 = return value of the plugin function
// Example:  TaskInit#bme=1,0    (taskindex = 0, return value = 0)
void queueTaskEvent(const String& eventName, taskIndex_t taskIndex, int value1) {
  if (Settings.UseRules) {
    String event;
    event.reserve(eventName.length() + 32);
    event  = eventName;
    event += '#';
    event += getTaskDeviceName(taskIndex);
    event += '=';
    event += taskIndex + 1;
    event += ',';
    event += value1;
    eventQueue.addMove(std::move(event));
  }
}


/**
 * Call the plugin of 1 task for 1 function, with standard EventStruct and optional command string
 */
bool PluginCallForTask(taskIndex_t taskIndex, uint8_t Function, EventStruct *TempEvent, String& command, EventStruct *event = nullptr) {
  bool retval = false;
  if (Settings.TaskDeviceEnabled[taskIndex] && validPluginID_fullcheck(Settings.TaskDeviceNumber[taskIndex]))
  {
    if (Settings.TaskDeviceDataFeed[taskIndex] == 0) // these calls only to tasks with local feed
    {
      const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(taskIndex);
      if (validDeviceIndex(DeviceIndex)) {
        TempEvent->setTaskIndex(taskIndex);
        TempEvent->sensorType   = Device[DeviceIndex].VType;
        if (event != nullptr) {
          TempEvent->OriginTaskIndex = event->TaskIndex;
        }

        prepare_I2C_by_taskIndex(taskIndex, DeviceIndex);
        switch (Function) {
          case PLUGIN_WRITE:          // First set
          case PLUGIN_REQUEST:
          case PLUGIN_ONCE_A_SECOND:  // Second set
          case PLUGIN_TEN_PER_SECOND:
          case PLUGIN_FIFTY_PER_SECOND:
          case PLUGIN_INIT:           // Second set, instead of PLUGIN_INIT_ALL
          case PLUGIN_CLOCK_IN:
          case PLUGIN_EVENT_OUT:
          case PLUGIN_TIME_CHANGE:
            {
              #ifndef BUILD_NO_RAM_TRACKER
              checkRAM(F("PluginCall_s"), taskIndex);
              #endif
              break;
            }
        }
        START_TIMER;
        retval = (Plugin_ptr[DeviceIndex](Function, TempEvent, command));
        STOP_TIMER_TASK(DeviceIndex, Function);

        if (Function == PLUGIN_INIT) {
          // Schedule the plugin to be read.
          Scheduler.schedule_task_device_timer_at_init(TempEvent->TaskIndex);
          queueTaskEvent(F("TaskInit"), taskIndex, retval);
        }

        post_I2C_by_taskIndex(taskIndex, DeviceIndex);
        delay(0); // SMY: call delay(0) unconditionally
      }
    }
  }
  return retval;
}

/*********************************************************************************************\
* Function call to all or specific plugins
\*********************************************************************************************/
bool PluginCall(uint8_t Function, struct EventStruct *event, String& str)
{
  struct EventStruct TempEvent;

  if (event == nullptr) {
    event = &TempEvent;
  }
  else {
    TempEvent.deep_copy(*event);
  }

  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("PluginCall"), Function);
  #endif

  switch (Function)
  {
    // Unconditional calls to all plugins
    case PLUGIN_DEVICE_ADD:
    case PLUGIN_UNCONDITIONAL_POLL:    // FIXME TD-er: PLUGIN_UNCONDITIONAL_POLL is not being used at the moment

      for (deviceIndex_t x = 0; x < PLUGIN_MAX; x++) {
        if (validPluginID(DeviceIndex_to_Plugin_id[x])) {
          if (Function == PLUGIN_DEVICE_ADD) {
            if ((deviceCount + 2) > static_cast<int>(Device.size())) {
              // Increase with 16 to get some compromise between number of resizes and wasted space
              unsigned int newSize = Device.size();
              newSize = newSize + 16 - (newSize % 16);
              Device.resize(newSize);

              // FIXME TD-er: Also resize DeviceIndex_to_Plugin_id ?
            }
          }
          // FIXME TD-er: This is not correct as we don't have a taskIndex here when addressing a plugin
          /*
          taskIndex_t  taskIndex = INVALID_TASK_INDEX;
          if (Function != PLUGIN_DEVICE_ADD && Device[x].Type == DEVICE_TYPE_I2C) {
            unsigned int varNr;
            validTaskVars(event, taskIndex, varNr);
            prepare_I2C_by_taskIndex(taskIndex, x);
          }
          */
          START_TIMER;
          Plugin_ptr[x](Function, event, str);
          STOP_TIMER_TASK(x, Function);
          /*
          // FIXME TD-er: This is not correct as we don't have a taskIndex here when addressing a plugin
          if (Function != PLUGIN_DEVICE_ADD) {
            post_I2C_by_taskIndex(taskIndex, x);
          }
          */
          delay(0); // SMY: call delay(0) unconditionally
        }
      }
      return true;

    case PLUGIN_MONITOR:

      for (auto it = globalMapPortStatus.begin(); it != globalMapPortStatus.end(); ++it) {
        // only call monitor function if there the need to
        if (it->second.monitor || it->second.command || it->second.init) {
          TempEvent.Par1 = getPortFromKey(it->first);

          // initialize the "x" variable to synch with the pluginNumber if second.x == -1
          if (!validDeviceIndex(it->second.x)) { it->second.x = getDeviceIndex(getPluginFromKey(it->first)); }

          const deviceIndex_t DeviceIndex = it->second.x;
          if (validDeviceIndex(DeviceIndex))  {
            // FIXME TD-er: This is not correct, as the event is NULL for calls to PLUGIN_MONITOR
            /*
            taskIndex_t  taskIndex = INVALID_TASK_INDEX;
            if (Device[DeviceIndex].Type == DEVICE_TYPE_I2C) {
              unsigned int varNr;  
              validTaskVars(event, taskIndex, varNr);
              prepare_I2C_by_taskIndex(taskIndex, DeviceIndex);
            }
            */
            START_TIMER;
            Plugin_ptr[DeviceIndex](Function, &TempEvent, str);
            STOP_TIMER_TASK(DeviceIndex, Function);
            // post_I2C_by_taskIndex(taskIndex, DeviceIndex);
          }
        }
      }
      return true;


    // Call to all plugins. Return at first match
    case PLUGIN_WRITE:
//    case PLUGIN_REQUEST: @giig1967g: replaced by new function getGPIOPluginValues()
    {
      taskIndex_t firstTask = 0;
      taskIndex_t lastTask = TASKS_MAX;
      String command = String(str);                           // Local copy to avoid warning in ExecuteCommand
      int dotPos = command.indexOf('.');                      // Find first period
      if (Function == PLUGIN_WRITE                            // Only applicable on PLUGIN_WRITE function
        && dotPos > -1) {                                     // First precondition is just a quick check for a period (fail-fast strategy)
        String arg0 = parseString(command, 1);                // Get first argument
        dotPos = arg0.indexOf('.');
        if (dotPos > -1) {
          String thisTaskName = command.substring(0, dotPos); // Extract taskname prefix
          thisTaskName.replace(F("["), EMPTY_STRING);                      // Remove the optional square brackets
          thisTaskName.replace(F("]"), EMPTY_STRING);
          if (thisTaskName.length() > 0) {                    // Second precondition
            taskIndex_t thisTask = findTaskIndexByName(thisTaskName);
            if (!validTaskIndex(thisTask)) {                  // Taskname not found or invalid, check for a task number?
              thisTask = static_cast<taskIndex_t>(atoi(thisTaskName.c_str()));
              if (thisTask == 0 || thisTask > TASKS_MAX) {
                thisTask = INVALID_TASK_INDEX;
              } else {
                thisTask--;                                   // 0-based
              }
            }
            if (validTaskIndex(thisTask)) {                   // Known taskindex?
#ifdef USES_P022                                              // Exclude P022 as it has rather explicit differences in commands when used with the [<TaskName>]. prefix
              if (Settings.TaskDeviceEnabled[thisTask]        // and internally needs to know wether it was called with the taskname prefixed
                && validPluginID_fullcheck(Settings.TaskDeviceNumber[thisTask])
                && Settings.TaskDeviceDataFeed[thisTask] == 0) {
                const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(thisTask);
                if (validDeviceIndex(DeviceIndex) && Device[DeviceIndex].Number == 22 /* PLUGIN_ID_022 define no longer available, 'assume' 22 for now */) {
                  thisTask = INVALID_TASK_INDEX;
                }
              }
              if (validTaskIndex(thisTask)) {
#endif
                firstTask = thisTask;
                lastTask  = thisTask + 1;                     // Add 1 to satisfy the for condition
                command   = command.substring(dotPos + 1);    // Remove [<TaskName>]. prefix
#ifdef USES_P022
              }
#endif
            }
          }
        }
      }
  // String info = F("PLUGIN_WRITE first: "); // To remove
  // info += firstTask;
  // info += F(" last: ");
  // info += lastTask;
  // addLog(LOG_LEVEL_INFO, info);

      for (taskIndex_t task = firstTask; task < lastTask; task++)
      {
        bool retval = PluginCallForTask(task, Function, &TempEvent, command);

        if (retval) {
          CPluginCall(CPlugin::Function::CPLUGIN_ACKNOWLEDGE, &TempEvent, command);
          return true;
        }
      }

      if (Function == PLUGIN_REQUEST) {
        // @FIXME TD-er: work-around as long as gpio command is still performed in P001_switch.
        for (deviceIndex_t deviceIndex = 0; deviceIndex < PLUGIN_MAX; deviceIndex++) {
          if (validPluginID(DeviceIndex_to_Plugin_id[deviceIndex])) {
            if (Plugin_ptr[deviceIndex](Function, event, str)) {
              delay(0); // SMY: call delay(0) unconditionally
              CPluginCall(CPlugin::Function::CPLUGIN_ACKNOWLEDGE, event, str);
              return true;
            }
          }
        }
      }
      break;
    }

    // Call to all plugins used in a task. Return at first match
    case PLUGIN_SERIAL_IN:
    case PLUGIN_UDP_IN:
    {
      for (taskIndex_t taskIndex = 0; taskIndex < TASKS_MAX; taskIndex++)
      {
        bool retval = PluginCallForTask(taskIndex, Function, &TempEvent, str);

        if (retval) {
          #ifndef BUILD_NO_RAM_TRACKER
          checkRAM(F("PluginCallUDP"), taskIndex);
          #endif
          return true;
        }
      }
      return false;
    }

    // Call to all plugins that are used in a task
    case PLUGIN_ONCE_A_SECOND:
    case PLUGIN_TEN_PER_SECOND:
    case PLUGIN_FIFTY_PER_SECOND:
    case PLUGIN_INIT_ALL:
    case PLUGIN_CLOCK_IN:
    case PLUGIN_EVENT_OUT:
    case PLUGIN_TIME_CHANGE:
    {
      if (Function == PLUGIN_INIT_ALL) {
        Function = PLUGIN_INIT;
      }

      for (taskIndex_t taskIndex = 0; taskIndex < TASKS_MAX; taskIndex++)
      {
        #ifndef BUILD_NO_DEBUG
        const int freemem_begin = ESP.getFreeHeap();
        #endif

        PluginCallForTask(taskIndex, Function, &TempEvent, str, event);

        #ifndef BUILD_NO_DEBUG
        if (Function == PLUGIN_INIT) {
          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            // See also logMemUsageAfter()
            const int freemem_end = ESP.getFreeHeap();
            String log;
            log.reserve(128);
            log  = F("After PLUGIN_INIT ");
            log += F(" task: ");
            if (taskIndex < 9) log += ' ';
            log += taskIndex + 1;
            while (log.length() < 30) log += ' ';
            log += F("Free mem after: ");
            log += freemem_end;
            while (log.length() < 53) log += ' ';
            log += F("plugin: ");
            log += freemem_begin - freemem_end;
            while (log.length() < 67) log += ' ';

            log += Settings.TaskDeviceEnabled[taskIndex] ? F("[ena]") : F("[dis]");
            while (log.length() < 73) log += ' ';
            log += getPluginNameFromDeviceIndex(getDeviceIndex_from_TaskIndex(taskIndex));

            addLog(LOG_LEVEL_DEBUG, log);
          }
        }
        #endif
      }
      if (Function == PLUGIN_INIT) {
        updateTaskCaches();
      }

      return true;
    }

    // Call to specific task which may interact with the hardware
    case PLUGIN_INIT:
    case PLUGIN_EXIT:
    case PLUGIN_WEBFORM_LOAD:
    case PLUGIN_READ:
    case PLUGIN_GET_PACKED_RAW_DATA:
    {
      const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);

      if (validDeviceIndex(DeviceIndex)) {
        if (ExtraTaskSettings.TaskIndex != event->TaskIndex) {
          // LoadTaskSettings may call PLUGIN_GET_DEVICEVALUENAMES.
          LoadTaskSettings(event->TaskIndex);
        }
        event->BaseVarIndex = event->TaskIndex * VARS_PER_TASK;
        {
          #ifndef BUILD_NO_RAM_TRACKER
          String descr;
          descr.reserve(20);
          descr  = F("PluginCall_task_");
          descr += event->TaskIndex;
          checkRAM(descr, String(Function));
          #endif
        }
        prepare_I2C_by_taskIndex(event->TaskIndex, DeviceIndex);
        START_TIMER;
        bool retval =  Plugin_ptr[DeviceIndex](Function, event, str);

        if (retval && (Function == PLUGIN_READ)) {
          saveUserVarToRTC();
        }
        if (Function == PLUGIN_INIT) {
          // Schedule the plugin to be read.
          Scheduler.schedule_task_device_timer_at_init(TempEvent.TaskIndex);
          updateTaskCaches();
          queueTaskEvent(F("TaskInit"), event->TaskIndex, retval);
        }
        if (Function == PLUGIN_EXIT) {
          clearPluginTaskData(event->TaskIndex);
          updateTaskCaches();
          initSerial();
          queueTaskEvent(F("TaskExit"), event->TaskIndex, retval);
        }
        STOP_TIMER_TASK(DeviceIndex, Function);
        post_I2C_by_taskIndex(event->TaskIndex, DeviceIndex);
        delay(0); // SMY: call delay(0) unconditionally

        return retval;
      }
      return false;
    }

    // Call to specific task not interacting with hardware
    case PLUGIN_GET_CONFIG:
    case PLUGIN_GET_DEVICEVALUENAMES:
    case PLUGIN_GET_DEVICEVALUECOUNT:
    case PLUGIN_GET_DEVICEVTYPE:
    case PLUGIN_GET_DEVICEGPIONAMES:
    case PLUGIN_WEBFORM_SAVE:
    case PLUGIN_WEBFORM_SHOW_VALUES:
    case PLUGIN_WEBFORM_SHOW_CONFIG:
    case PLUGIN_WEBFORM_SHOW_I2C_PARAMS:
    case PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS:
    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    case PLUGIN_FORMAT_USERVAR:
    case PLUGIN_SET_CONFIG:
    case PLUGIN_SET_DEFAULTS:

    // PLUGIN_MQTT_xxx functions are directly called from the scheduler.
    //case PLUGIN_MQTT_CONNECTION_STATE:
    //case PLUGIN_MQTT_IMPORT:
    {
      const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);

      if (validDeviceIndex(DeviceIndex)) {
        // LoadTaskSettings may call PLUGIN_GET_DEVICEVALUENAMES.
        LoadTaskSettings(event->TaskIndex);
        event->BaseVarIndex = event->TaskIndex * VARS_PER_TASK;
        {
          #ifndef BUILD_NO_RAM_TRACKER
          String descr;
          descr.reserve(20);
          descr  = F("PluginCall_task_");
          descr += event->TaskIndex;
          checkRAM(descr, String(Function));
          #endif
        }
        if (Function == PLUGIN_SET_DEFAULTS) {
          for (int i = 0; i < VARS_PER_TASK; ++i) {
            UserVar[event->BaseVarIndex + i] = 0.0f;
          }
        }
        if (Function == PLUGIN_GET_DEVICEVALUECOUNT) {
          event->Par1 = Device[DeviceIndex].ValueCount;
        }
        if (Function == PLUGIN_GET_DEVICEVTYPE) {
          event->sensorType = Device[DeviceIndex].VType;
        }

        START_TIMER;
        bool retval =  Plugin_ptr[DeviceIndex](Function, event, str);
        if (Function == PLUGIN_SET_DEFAULTS) {
          saveUserVarToRTC();
        }
        if (Function == PLUGIN_GET_DEVICEVALUECOUNT) {
          // Check if we have a valid value count.
          if (Output_Data_type_t::Simple == Device[DeviceIndex].OutputDataType) {
            if (event->Par1 < 1 || event->Par1 > 4) {
              // Output_Data_type_t::Simple only allows for 1 .. 4 output types.
              // Apparently the value is not correct, so use the default.
              event->Par1 = Device[DeviceIndex].ValueCount;
            }
          }
        }

        // Calls may have updated ExtraTaskSettings, so validate them.
        ExtraTaskSettings.validate();
        
        STOP_TIMER_TASK(DeviceIndex, Function);
        delay(0); // SMY: call delay(0) unconditionally
        return retval;
      }
      return false;
    }

  } // case
  return false;
}

bool addPlugin(pluginID_t pluginID, deviceIndex_t x) {
  if (x < PLUGIN_MAX) { 
    DeviceIndex_to_Plugin_id[x] = pluginID; 
    Plugin_id_to_DeviceIndex[pluginID] = x;
    return true;
  }
  String log = F("System: Error - Too many Plugins. PLUGIN_MAX = ");
  log += PLUGIN_MAX;
  addLog(LOG_LEVEL_ERROR, log);
  return false;
}
