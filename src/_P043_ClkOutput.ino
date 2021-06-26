#include "_Plugin_Helper.h"

#ifdef USES_P043
//#######################################################################################################
//#################################### Plugin 043: Clock Output #########################################
//#######################################################################################################


#define PLUGIN_043
#define PLUGIN_ID_043         43
#define PLUGIN_NAME_043       "Output - Clock"
# define PLUGIN_VALUENAME_043 "Output"
//#define PLUGIN_VALUENAME1_043 "Output"
//#define PLUGIN_VALUENAME2_043 "Output2"
#define PLUGIN_043_MAX_SETTINGS 8
# define P043_SENSOR_TYPE_INDEX  2
# define P043_NR_OUTPUT_VALUES   getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P043_SENSOR_TYPE_INDEX)))

String Plugin_043_valuename(byte value_nr, bool displayString) {
  String name = F("Output");

  if (value_nr != 0) {
    name += String(value_nr + 1);
  }
    if (!displayString) {
    name.toLowerCase();
  }
  return name;
}

boolean Plugin_043(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_043;
        Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_SWITCH;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].OutputDataType = Output_Data_type_t::Simple;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_043);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      for (byte i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P043_NR_OUTPUT_VALUES) {
          safe_strncpy(
            ExtraTaskSettings.TaskDeviceValueNames[i],
            Plugin_043_valuename(i, false),
            sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
          ExtraTaskSettings.TaskDeviceValueDecimals[i] = 2;
        } else {
          ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
        }
      }
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P043_NR_OUTPUT_VALUES;
      success     = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(P043_SENSOR_TYPE_INDEX));
      event->idx        = P043_SENSOR_TYPE_INDEX;
      success           = true;
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_output(F("Clock Event"));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        const __FlashStringHelper *  options[3];
        options[0] = F("");
        options[1] = F("Off");
        options[2] = F("On");
 
        for (byte x = 0; x < PLUGIN_043_MAX_SETTINGS; x++)
        {
        	addFormTextBox(String(F("Day,Time ")) + (x + 1), String(F("p043_clock")) + (x), timeLong2String(ExtraTaskSettings.TaskDevicePluginConfigLong[x]), 32);
          if (CONFIG_PIN1 >= 0) {
            addHtml(' ');
            const byte choice = ExtraTaskSettings.TaskDevicePluginConfig[x];
            addSelector(String(F("p043_state")) + (x), 3, options, NULL, NULL, choice);
          }
          else addFormNumericBox(String(F("Value")) + (x + 1), String(F("p043_state")) + (x), ExtraTaskSettings.TaskDevicePluginConfig[x]);
        }
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        for (byte x = 0; x < PLUGIN_043_MAX_SETTINGS; x++)
        {
          String argc = F("p043_clock");
          argc += x;
          String plugin1 = webArg(argc);
          ExtraTaskSettings.TaskDevicePluginConfigLong[x] = string2TimeLong(plugin1);

          argc = F("p043_state");
          argc += x;
          String plugin2 = webArg(argc);
          ExtraTaskSettings.TaskDevicePluginConfig[x] = plugin2.toInt();
        }
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        success = true;
        break;
      }

    case PLUGIN_CLOCK_IN:
      {
        LoadTaskSettings(event->TaskIndex);
        for (byte x = 0; x < PLUGIN_043_MAX_SETTINGS; x++)
        {
          unsigned long clockEvent = (unsigned long)node_time.minute() % 10 | (unsigned long)(node_time.minute() / 10) << 4 | (unsigned long)(node_time.hour() % 10) << 8 | (unsigned long)(node_time.hour() / 10) << 12 | (unsigned long)node_time.weekday() << 16;
          unsigned long clockSet = ExtraTaskSettings.TaskDevicePluginConfigLong[x];

          if (matchClockEvent(clockEvent,clockSet))
          {
            byte state = ExtraTaskSettings.TaskDevicePluginConfig[x];
            if (state != 0) 
            {
              if (CONFIG_PIN1 >= 0) { // if GPIO is specified, use the old behavior
                state--;
                pinMode(CONFIG_PIN1, OUTPUT);
                digitalWrite(CONFIG_PIN1, state);
                UserVar[event->BaseVarIndex] = state;
              }
              else {
                UserVar[event->BaseVarIndex] = x+1;
                UserVar[event->BaseVarIndex+1] = state;
              }
              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                String log = F("TCLK : State ");
                log += state;
                addLog(LOG_LEVEL_INFO, log);
              }
              sendData(event);
            }
          }
        }
        break;
      }
  }
  return success;
}
#endif // USES_P043
