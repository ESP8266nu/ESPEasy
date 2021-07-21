#include "_Plugin_Helper.h"
#ifdef USES_P094

// #######################################################################################################
// #################### Plugin 094 Brick4U CUL Reader ####################################################
// #######################################################################################################
//
// Interact with Brick4U CUL receiver
// Allows to control the mode of the CUL receiver
//


#include "src/Helpers/ESPEasy_Storage.h"
#include "src/Helpers/StringConverter.h"
#include "src/PluginStructs/P094_data_struct.h"

#include <Regexp.h>

#define PLUGIN_094
#define PLUGIN_ID_094           94
#define PLUGIN_NAME_094         "Communication - CUL Reader"


#define P094_BAUDRATE           PCONFIG_LONG(0)
#define P094_BAUDRATE_LABEL     PCONFIG_LABEL(0)

#define P094_DEBUG_SENTENCE_LENGTH  PCONFIG_LONG(1)
#define P094_DEBUG_SENTENCE_LABEL   PCONFIG_LABEL(1)

#define P094_APPEND_RECEIVE_SYSTIME PCONFIG(0)

#define P094_QUERY_VALUE        0 // Temp placement holder until we know what selectors are needed.
#define P094_NR_OUTPUT_OPTIONS  1

#define P094_NR_OUTPUT_VALUES   1
#define P094_QUERY1_CONFIG_POS  3

#define P094_DEFAULT_BAUDRATE   38400


// Plugin settings:
// Validate:
// - [0..9]
// - "+", "-", "."
// - [A..Z]
// - [a..z]
// - ASCII 32 - 217
// Sentence start:  char
// Sentence end:  CR/CRLF/LF/char
// Max length sentence: 1k max
// Interpret as:
// - Float
// - int
// - String
// Init string (incl parsing CRLF like characters)
// Timeout between sentences.


boolean Plugin_094(uint8_t function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_094;
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_STRING;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = false;
      Device[deviceCount].DuplicateDetection = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_094);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P094_NR_OUTPUT_VALUES) {
          const uint8_t pconfigIndex = i + P094_QUERY1_CONFIG_POS;
          uint8_t choice             = PCONFIG(pconfigIndex);
          safe_strncpy(
            ExtraTaskSettings.TaskDeviceValueNames[i],
            Plugin_094_valuename(choice, false),
            sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
        } else {
          ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
        }
      }
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES: {
      serialHelper_getGpioNames(event, false, true); // TX optional
      break;
    }

    case PLUGIN_WEBFORM_SHOW_VALUES:
    {
      P094_data_struct *P094_data =
        static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));

      if ((nullptr != P094_data) && P094_data->isInitialized()) {
        uint32_t success, error, length_last;
        P094_data->getSentencesReceived(success, error, length_last);
        uint8_t varNr = VARS_PER_TASK;
        pluginWebformShowValue(event->TaskIndex, varNr++, F("Success"),     String(success));
        pluginWebformShowValue(event->TaskIndex, varNr++, F("Error"),       String(error));
        pluginWebformShowValue(event->TaskIndex, varNr++, F("Length Last"), String(length_last), true);

        // success = true;
      }
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P094_BAUDRATE = P094_DEFAULT_BAUDRATE;
      P094_DEBUG_SENTENCE_LENGTH = 0;

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS:
    {
      addFormNumericBox(F("Baudrate"), P094_BAUDRATE_LABEL, P094_BAUDRATE, 2400, 115200);
      addUnit(F("baud"));
      break;
    }

    case PLUGIN_WEBFORM_LOAD: 
    {
      addFormSubHeader(F("Filtering"));
      P094_html_show_matchForms(event);

      addFormSubHeader(F("Statistics"));
      P094_html_show_stats(event);

      addFormNumericBox(F("(debug) Generated length"), P094_DEBUG_SENTENCE_LABEL, P094_DEBUG_SENTENCE_LENGTH, 0, 1024);

      addFormCheckBox(F("Append system time"), F("systime"), P094_APPEND_RECEIVE_SYSTIME);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      P094_BAUDRATE = getFormItemInt(P094_BAUDRATE_LABEL);
      P094_DEBUG_SENTENCE_LENGTH = getFormItemInt(P094_DEBUG_SENTENCE_LABEL);

      P094_data_struct *P094_data =
        static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P094_data) {
        for (uint8_t varNr = 0; varNr < P94_Nlines; varNr++)
        {
          P094_data->setLine(varNr, webArg(getPluginCustomArgName(varNr)));
        }

        addHtmlError(SaveCustomTaskSettings(event->TaskIndex, P094_data->_lines, P94_Nlines, 0));
        success = true;
      }

      P094_APPEND_RECEIVE_SYSTIME = isFormItemChecked(F("systime"));

      break;
    }

    case PLUGIN_INIT: {
      const int16_t serial_rx = CONFIG_PIN1;
      const int16_t serial_tx = CONFIG_PIN2;
      const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P094_data_struct());
      P094_data_struct *P094_data =
        static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P094_data) {
        return success;
      }

      if (P094_data->init(port, serial_rx, serial_tx, P094_BAUDRATE)) {
        LoadCustomTaskSettings(event->TaskIndex, P094_data->_lines, P94_Nlines, 0);
        P094_data->post_init();
        success = true;

        serialHelper_log_GpioDescription(port, serial_rx, serial_tx);
      } else {
        clearPluginTaskData(event->TaskIndex);
      }
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND: {
      if (Settings.TaskDeviceEnabled[event->TaskIndex]) {
        P094_data_struct *P094_data =
          static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));

        if ((nullptr != P094_data) && P094_data->loop()) {
          // Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
          delay(0); // Processing a full sentence may take a while, run some
                    // background tasks.
          P094_data->getSentence(event->String2, P094_APPEND_RECEIVE_SYSTIME);

          if (event->String2.length() > 0) {
            if (Plugin_094_match_all(event->TaskIndex, event->String2)) {
              if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                String log;
                if (log.reserve(128)) {
                  log = F("CUL Reader: Sending: ");
                  const size_t messageLength = event->String2.length();
                  if (messageLength < 100) {
                    log += event->String2;
                  } else {
                    // Split string so we get start and end
                    log += event->String2.substring(0, 40);
                    log += F("...");
                    log += event->String2.substring(messageLength - 40);
                  }
                  addLog(LOG_LEVEL_INFO, log);
                }
              }
              // Filter length options:
              // - 22 char, for hash-value then we filter the exact meter including serial and meter type, (that will also prevent very quit sending meters, which normaly is a fault)
              // - 38 char, The exact message, because we have 2 uint8_t from the value payload
              //sendData_checkDuplicates(event, event->String2.substring(0, 22));
              sendData(event);
            }
          }
        }
        success = true;
      }
      break;
    }

    case PLUGIN_READ: {
      if (P094_DEBUG_SENTENCE_LENGTH > 0) {
        P094_data_struct *P094_data =
          static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));

        if ((nullptr != P094_data)) {
          const uint32_t debug_count = P094_data->getDebugCounter();
          event->String2.reserve(P094_DEBUG_SENTENCE_LENGTH);
          event->String2 = String(debug_count);
          event->String2 += '_';
          const char c = '0' + debug_count % 10;
          for (long i = event->String2.length(); i < P094_DEBUG_SENTENCE_LENGTH; ++i) {
            event->String2 += c;
          }
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("CUL Reader: Sending: ");
            log += event->String2.substring(0, 20);
            log += F("...");
            addLog(LOG_LEVEL_INFO, log);
          }
//          sendData_checkDuplicates(event, event->String2.substring(0, 22));
          sendData(event);
        }
      }
      break;
    }

    case PLUGIN_WRITE: {
      String cmd = parseString(string, 1);

      if (cmd.startsWith(F("culreader"))) {
        if (cmd.equals(F("culreader_write"))) {
          P094_data_struct *P094_data =
            static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));

          if ((nullptr != P094_data)) {
            String param1 = parseStringKeepCase(string, 2);
            parseSystemVariables(param1, false);
            P094_data->sendString(param1);
            addLog(LOG_LEVEL_INFO, param1);
            success = true;
          }
        }
      }


      break;
    }
  }
  return success;
}

bool Plugin_094_match_all(taskIndex_t taskIndex, const String& received)
{
  P094_data_struct *P094_data =
    static_cast<P094_data_struct *>(getPluginTaskData(taskIndex));

  if ((nullptr == P094_data)) {
    return false;
  }


  if (P094_data->disableFilterWindowActive()) {
    addLog(LOG_LEVEL_INFO, F("CUL Reader: Disable Filter Window active"));
    return true;
  }

  bool res = P094_data->parsePacket(received);

  if (P094_data->invertMatch()) {
    addLog(LOG_LEVEL_INFO, F("CUL Reader: invert filter"));
    return !res;
  }
  return res;
}

String Plugin_094_valuename(uint8_t value_nr, bool displayString) {
  switch (value_nr) {
    case P094_QUERY_VALUE: return displayString ? F("Value")          : F("v");
  }
  return "";
}

void P094_html_show_matchForms(struct EventStruct *event) {
  P094_data_struct *P094_data =
    static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr != P094_data)) {
    addFormNumericBox(F("Filter Off Window after send"),
                      getPluginCustomArgName(P094_FILTER_OFF_WINDOW_POS),
                      P094_data->getFilterOffWindowTime(),
                      0,
                      60000);
    addUnit(F("msec"));
    addFormNote(F("0 = Do not turn off filter after sending to the connected device."));

    {
      const __FlashStringHelper * options[P094_Match_Type_NR_ELEMENTS];
      int    optionValues[P094_Match_Type_NR_ELEMENTS];

      for (int i = 0; i < P094_Match_Type_NR_ELEMENTS; ++i) {
        P094_Match_Type matchType = static_cast<P094_Match_Type>(i);
        options[i]      = P094_data_struct::MatchType_toString(matchType);
        optionValues[i] = matchType;
      }
      P094_Match_Type choice = P094_data->getMatchType();
      addFormSelector(F("Filter Mode"),
                      getPluginCustomArgName(P094_MATCH_TYPE_POS),
                      P094_Match_Type_NR_ELEMENTS,
                      options,
                      optionValues,
                      choice,
                      false);
    }


    uint8_t filterSet                  = 0;
    uint32_t optional              = 0;
    P094_Filter_Value_Type capture = P094_Filter_Value_Type::P094_packet_length;
    P094_Filter_Comp comparator    = P094_Filter_Comp::P094_Equal_OR;
    String filter;

    for (uint8_t filterLine = 0; filterLine < P094_NR_FILTERS; ++filterLine)
    {
      // Filter parameter number on a filter line.
      bool newLine = (filterLine % P094_AND_FILTER_BLOCK) == 0;

      for (uint8_t filterLinePar = 0; filterLinePar < P094_ITEMS_PER_FILTER; ++filterLinePar)
      {
        String id = getPluginCustomArgName(P094_data_struct::P094_Get_filter_base_index(filterLine) + filterLinePar);

        switch (filterLinePar) {
          case 0:
          {
            filter = P094_data->getFilter(filterLine, capture, optional, comparator);

            if (newLine) {
              // Label + first parameter
              ++filterSet;
              String label;
              label  = F("Filter ");
              label += String(filterSet);
              addRowLabel_tr_id(label, id);
            } else {
              html_B(F("AND"));
              html_BR();
            }

            // Combo box with filter types
            {
              const __FlashStringHelper * options[P094_FILTER_VALUE_Type_NR_ELEMENTS];
              int    optionValues[P094_FILTER_VALUE_Type_NR_ELEMENTS];

              for (int i = 0; i < P094_FILTER_VALUE_Type_NR_ELEMENTS; ++i) {
                P094_Filter_Value_Type filterValueType = static_cast<P094_Filter_Value_Type>(i);
                options[i]      = P094_data_struct::P094_FilterValueType_toString(filterValueType);
                optionValues[i] = filterValueType;
              }
              addSelector(id, P094_FILTER_VALUE_Type_NR_ELEMENTS, options, optionValues, NULL, capture, false, true, EMPTY_STRING);
            }

            break;
          }
          case 1:
          {
            // Optional numerical value
            addNumericBox(id, optional, 0, 1024);
            break;
          }
          case 2:
          {
            // Comparator
            const __FlashStringHelper * options[P094_FILTER_COMP_NR_ELEMENTS];
            int    optionValues[P094_FILTER_COMP_NR_ELEMENTS];

            for (int i = 0; i < P094_FILTER_COMP_NR_ELEMENTS; ++i) {
              P094_Filter_Comp enumValue = static_cast<P094_Filter_Comp>(i);
              options[i]      = P094_data_struct::P094_FilterComp_toString(enumValue);
              optionValues[i] = enumValue;
            }
            addSelector(id, P094_FILTER_COMP_NR_ELEMENTS, options, optionValues, NULL, comparator, false, true, EMPTY_STRING);
            break;
          }
          case 3:
          {
            // Compare with
            addTextBox(id, filter, 8, false, false, EMPTY_STRING, EMPTY_STRING);
            break;
          }
        }
      }
    }
  }
}

void P094_html_show_stats(struct EventStruct *event) {
  P094_data_struct *P094_data =
    static_cast<P094_data_struct *>(getPluginTaskData(event->TaskIndex));

  if ((nullptr == P094_data) || !P094_data->isInitialized()) {
    return;
  }
  {
    addRowLabel(F("Current Sentence"));
    addHtml(P094_data->peekSentence());
  }

  {
    addRowLabel(F("Sentences (pass/fail)"));
    uint32_t success, error, length_last;
    P094_data->getSentencesReceived(success, error, length_last);
    addHtmlInt(success);
    addHtml('/');
    addHtmlInt(error);
    addRowLabel(F("Length Last Sentence"));
    addHtmlInt(length_last);
  }
}

#endif // USES_P094