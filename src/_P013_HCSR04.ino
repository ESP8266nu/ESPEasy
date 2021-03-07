#include "_Plugin_Helper.h"

#ifdef USES_P013
//#######################################################################################################
//############################### Plugin 013: HC-SR04, RCW-0001, etc. ###################################
//#######################################################################################################


#define PLUGIN_013
#define PLUGIN_ID_013        13
#define PLUGIN_NAME_013       "Position - HC-SR04, RCW-0001, etc."
#define PLUGIN_VALUENAME1_013 "Distance"
#define PLUGIN_VALUENAME2_013 "Switch"

#include <Arduino.h>
#include <map>
#include <NewPingESP8266.h>

// PlugIn specific defines
// operatingMode
#define OPMODE_VALUE        (0)
#define OPMODE_STATE        (1)
#define OPMODE_COMBINED     (2)

// measuringUnit
#define UNIT_CM             (0)
#define UNIT_INCH           (1)

// filterType
#define FILTER_NONE         (0)
#define FILTER_MEDIAN       (1)

// map of sensors
std::map<unsigned int, std::shared_ptr<NewPingESP8266> > P_013_sensordefs;

boolean Plugin_013(byte function, struct EventStruct *event, String& string)
{
  static byte switchstate[TASKS_MAX];
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_013;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_DUAL;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;

        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_013);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_013));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_013));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_output(F("Trigger"));
        event->String2 = formatGpioName_input(F("Echo, 5V"));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        int16_t operatingMode = PCONFIG(0);
        int16_t threshold = PCONFIG(1);
        int16_t max_distance = PCONFIG(2);
        int16_t measuringUnit = PCONFIG(3);
        int16_t filterType = PCONFIG(4);
        int16_t filterSize = PCONFIG(5);

        // default filtersize = 5
        if (filterSize == 0) {
          filterSize = 5;
          PCONFIG(5) = filterSize;
        }

        String strUnit = (measuringUnit == UNIT_CM) ? F("cm") : F("inch");

        String optionsOpMode[3];
        int optionValuesOpMode[3] = { 0, 1, 2 };
        optionsOpMode[0] = F("Value");
        optionsOpMode[1] = F("State");
        optionsOpMode[2] = F("Combined");
        addFormSelector(F("Mode"), F("p013_mode"), 3, optionsOpMode, optionValuesOpMode, operatingMode);

        if ((operatingMode == OPMODE_STATE) || (operatingMode == OPMODE_COMBINED))
        {
        	addFormNumericBox(F("Threshold"), F("p013_threshold"), threshold);
          addUnit(strUnit);
        }
        addFormNumericBox(F("Max Distance"), F("p013_max_distance"), max_distance, 0, 500);
        addUnit(strUnit);

        String optionsUnit[2];
        int optionValuesUnit[2] = { 0, 1 };
        optionsUnit[0] = F("Metric");
        optionsUnit[1] = F("Imperial");
        addFormSelector(F("Unit"), F("p013_Unit"), 2, optionsUnit, optionValuesUnit, measuringUnit);

        String optionsFilter[2];
        int optionValuesFilter[2] = { 0, 1 };
        optionsFilter[0] = F("None");
        optionsFilter[1] = F("Median");
        addFormSelector(F("Filter"), F("p013_FilterType"), 2, optionsFilter, optionValuesFilter, filterType);

        // enable filtersize option if filter is used,
        if (filterType != FILTER_NONE)
        	addFormNumericBox(F("Number of Pings"), F("p013_FilterSize"), filterSize, 2, 20);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        int16_t operatingMode = PCONFIG(0);
        int16_t filterType = PCONFIG(4);

        PCONFIG(0) = getFormItemInt(F("p013_mode"));
        if ((operatingMode == OPMODE_STATE) || (operatingMode == OPMODE_COMBINED))
          PCONFIG(1) = getFormItemInt(F("p013_threshold"));
        PCONFIG(2) = getFormItemInt(F("p013_max_distance"));

        PCONFIG(3) = getFormItemInt(F("p013_Unit"));
        PCONFIG(4) = getFormItemInt(F("p013_FilterType"));
        if (filterType != FILTER_NONE)
          PCONFIG(5) = getFormItemInt(F("p013_FilterSize"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        int16_t max_distance = PCONFIG(2);
        int16_t measuringUnit = PCONFIG(3);
        int16_t filterType = PCONFIG(4);
        int16_t filterSize = PCONFIG(5);

        int8_t Plugin_013_TRIG_Pin = CONFIG_PIN1;
        int8_t Plugin_013_IRQ_Pin = CONFIG_PIN2;
        int16_t max_distance_cm = (measuringUnit == UNIT_CM) ? max_distance : (float)max_distance * 2.54f;

        // create sensor instance and add to std::map
        P_013_sensordefs.erase(event->TaskIndex);
        P_013_sensordefs[event->TaskIndex] =
          std::shared_ptr<NewPingESP8266> (new NewPingESP8266(Plugin_013_TRIG_Pin, Plugin_013_IRQ_Pin, max_distance_cm));

        String log = F("ULTRASONIC : TaskNr: ");
        log += event->TaskIndex +1;
        log += F(" TrigPin: ");
        log += Plugin_013_TRIG_Pin;
        log += F(" IRQ_Pin: ");
        log += Plugin_013_IRQ_Pin;
        log += F(" max dist ");
        log += (measuringUnit == UNIT_CM) ? F("[cm]: ") : F("[inch]: ");
        log += max_distance;
        log += F(" max echo: ");
        log += P_013_sensordefs[event->TaskIndex]->getMaxEchoTime();
        log += F(" Filter: ");
        if (filterType == FILTER_NONE)
          log += F("none");
        else
          if (filterType == FILTER_MEDIAN) {
            log += F("Median size: ");
            log += filterSize;
          }
          else
            log += F("invalid!");
        log += F(" nr_tasks: ");
        log += P_013_sensordefs.size();
        addLog(LOG_LEVEL_INFO, log);

        unsigned long tmpmillis = millis();
        unsigned long tmpmicros = micros();
        delay(100);
        long millispassed = timePassedSince(tmpmillis);
        long microspassed = usecPassedSince(tmpmicros);

        log = F("ULTRASONIC : micros() test: ");
        log += millispassed;
        log += F(" msec, ");
        log += microspassed;
        log += F(" usec, ");
        addLog(LOG_LEVEL_INFO, log);

        success = true;
        break;
      }

    case PLUGIN_EXIT:
      {
        P_013_sensordefs.erase(event->TaskIndex);
        break;
      }

    case PLUGIN_READ: // If we select value mode, read and send the value based on global timer
      {
        int16_t operatingMode = PCONFIG(0);
        int16_t measuringUnit = PCONFIG(3);
        if (operatingMode == OPMODE_VALUE)
           event->sensorType = Sensor_VType::SENSOR_TYPE_SINGLE;
           
        if ((operatingMode == OPMODE_VALUE) || (operatingMode == OPMODE_COMBINED))
        {
          float value = Plugin_013_read(event->TaskIndex);
          String log = F("ULTRASONIC : TaskNr: ");
          log += event->TaskIndex +1;
          log += F(" Distance: ");
          UserVar[event->BaseVarIndex] = value;
          log += formatUserVarNoCheck(event->TaskIndex, 0);
          log += (measuringUnit == UNIT_CM) ? F(" cm ") : F(" inch ");
          if (value == NO_ECHO)
          {
             log += F(" Error: ");
             log += Plugin_013_getErrorStatusString(event->TaskIndex);
          }

          addLog(LOG_LEVEL_INFO,log);
        }
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND: // If we select state mode, do more frequent checks and send only state changes
      {
        int16_t operatingMode = PCONFIG(0);
        int16_t threshold = PCONFIG(1);

        if ((operatingMode == OPMODE_STATE) || (operatingMode == OPMODE_COMBINED))
        {
          byte state = 0;
          float value = Plugin_013_read(event->TaskIndex);
          if (value != NO_ECHO)
          {
            if (value < threshold)
              state = 1;
            if (state != switchstate[event->TaskIndex])
            {
              String log = F("ULTRASONIC : TaskNr: ");
              log += event->TaskIndex +1;
              log += F(" state: ");
              log += state;
              addLog(LOG_LEVEL_INFO,log);
              switchstate[event->TaskIndex] = state;
              UserVar[event->BaseVarIndex] = state;
              event->sensorType = Sensor_VType::SENSOR_TYPE_SWITCH;
              sendData(event);
            }
          }
          else {
            String log = F("ULTRASONIC : TaskNr: ");
            log += event->TaskIndex +1;
            log += F(" Error: ");
            log += Plugin_013_getErrorStatusString(event->TaskIndex);
            addLog(LOG_LEVEL_INFO,log);
          }

        }
        success = true;
        break;
      }
  }
  return success;
}

/*********************************************************************/
float Plugin_013_read(taskIndex_t taskIndex)
/*********************************************************************/
{
  if (P_013_sensordefs.count(taskIndex) == 0)
    return 0;

  int16_t max_distance = Settings.TaskDevicePluginConfig[taskIndex][2];
  int16_t measuringUnit = Settings.TaskDevicePluginConfig[taskIndex][3];
  int16_t filterType = Settings.TaskDevicePluginConfig[taskIndex][4];
  int16_t filterSize = Settings.TaskDevicePluginConfig[taskIndex][5];
  int16_t max_distance_cm = (measuringUnit == UNIT_CM) ? max_distance : (float)max_distance * 2.54f;

  unsigned int echoTime = 0;

  switch  (filterType) {
    case FILTER_NONE:
      echoTime = (P_013_sensordefs[taskIndex])->ping();
      break;
    case FILTER_MEDIAN:
      echoTime = (P_013_sensordefs[taskIndex])->ping_median(filterSize, max_distance_cm);
      break;
    default:
      addLog(LOG_LEVEL_INFO, F("invalid Filter Type setting!"));
  }

  if (measuringUnit == UNIT_CM)
    return NewPingESP8266::convert_cm_F(echoTime);
  else
    return NewPingESP8266::convert_in_F(echoTime);
}

/*********************************************************************/
String Plugin_013_getErrorStatusString(taskIndex_t taskIndex)
/*********************************************************************/
{
  if (P_013_sensordefs.count(taskIndex) == 0)
    return String(F("invalid taskindex"));

  switch ((P_013_sensordefs[taskIndex])->getErrorState()) {
    case NewPingESP8266::STATUS_SENSOR_READY: {
      return String(F("Sensor ready"));
    }

    case NewPingESP8266::STATUS_MEASUREMENT_VALID: {
      return String(F("no error, measurement valid"));
    }

    case NewPingESP8266::STATUS_ECHO_TRIGGERED: {
      return String(F("Echo triggered, waiting for Echo end"));
    }

    case NewPingESP8266::STATUS_ECHO_STATE_ERROR: {
      return String(F("Echo pulse error, Echopin not low on trigger"));
    }

    case NewPingESP8266::STATUS_ECHO_START_TIMEOUT_50ms: {
      return String(F("Echo timeout error, no echo start whithin 50 ms"));
    }

    case NewPingESP8266::STATUS_ECHO_START_TIMEOUT_DISTANCE: {
      return String(F("Echo timeout error, no echo start whithin time for max. distance"));
    }

    case NewPingESP8266::STATUS_MAX_DISTANCE_EXCEEDED: {
      return String(F("Echo too late, maximum distance exceeded"));
    }

    default: {
      return String(F("unknown error"));
    }

  }
}
#endif // USES_P013
