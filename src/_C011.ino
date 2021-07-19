#include "src/Helpers/_CPlugin_Helper.h"
#ifdef USES_C011

// #######################################################################################################
// ########################### Controller Plugin 011: Generic HTTP Advanced ##############################
// #######################################################################################################

# define CPLUGIN_011
# define CPLUGIN_ID_011         11
# define CPLUGIN_NAME_011       "Generic HTTP Advanced [TESTING]"

# define C011_HTTP_METHOD_MAX_LEN          16
# define C011_HTTP_URI_MAX_LEN             240
# define C011_HTTP_HEADER_MAX_LEN          256
# define C011_HTTP_BODY_MAX_LEN            512


bool C011_sendBinary = false;

struct C011_ConfigStruct
{
  void zero_last() {
    HttpMethod[C011_HTTP_METHOD_MAX_LEN - 1] = 0;
    HttpUri[C011_HTTP_URI_MAX_LEN - 1]       = 0;
    HttpHeader[C011_HTTP_HEADER_MAX_LEN - 1] = 0;
    HttpBody[C011_HTTP_BODY_MAX_LEN - 1]     = 0;
  }

  char HttpMethod[C011_HTTP_METHOD_MAX_LEN] = { 0 };
  char HttpUri[C011_HTTP_URI_MAX_LEN]       = { 0 };
  char HttpHeader[C011_HTTP_HEADER_MAX_LEN] = { 0 };
  char HttpBody[C011_HTTP_BODY_MAX_LEN]     = { 0 };
};

bool CPlugin_011(CPlugin::Function function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case CPlugin::Function::CPLUGIN_PROTOCOL_ADD:
    {
      Protocol[++protocolCount].Number     = CPLUGIN_ID_011;
      Protocol[protocolCount].usesMQTT     = false;
      Protocol[protocolCount].usesAccount  = true;
      Protocol[protocolCount].usesPassword = true;
      Protocol[protocolCount].usesExtCreds = true;
      Protocol[protocolCount].defaultPort  = 80;
      Protocol[protocolCount].usesID       = false;
      break;
    }

    case CPlugin::Function::CPLUGIN_GET_DEVICENAME:
    {
      string = F(CPLUGIN_NAME_011);
      break;
    }

    case CPlugin::Function::CPLUGIN_INIT:
    {
      {
        MakeControllerSettings(ControllerSettings);

        if (AllocatedControllerSettings()) {
          LoadControllerSettings(event->ControllerIndex, ControllerSettings);
          C011_sendBinary = ControllerSettings.sendBinary();
        }
      }
      success = init_c011_delay_queue(event->ControllerIndex);
      break;
    }

    case CPlugin::Function::CPLUGIN_EXIT:
    {
      exit_c011_delay_queue();
      break;
    }

    case CPlugin::Function::CPLUGIN_WEBFORM_LOAD:
    {
      {
        String HttpMethod;
        String HttpUri;
        String HttpHeader;
        String HttpBody;

        if (!load_C011_ConfigStruct(event->ControllerIndex, HttpMethod, HttpUri, HttpHeader, HttpBody))
        {
          return false;
        }
        addTableSeparator(F("HTTP Config"), 2, 3);
        {
          uint8_t   choice    = 0;
          const __FlashStringHelper * methods[] = { F("GET"), F("POST"), F("PUT"), F("HEAD"), F("PATCH") };

          for (uint8_t i = 0; i < 5; i++)
          {
            if (HttpMethod.equals(methods[i])) {
              choice = i;
            }
          }
          addFormSelector(F("Method"), F("P011httpmethod"), 5, methods, NULL, choice);
        }

        addFormTextBox(F("URI"), F("P011httpuri"), HttpUri, C011_HTTP_URI_MAX_LEN - 1);
        {
          htmlEscape(HttpHeader);
          addFormTextArea(F("Header"), F("P011httpheader"), HttpHeader, C011_HTTP_HEADER_MAX_LEN - 1, 4, 50);
        }
        {
          htmlEscape(HttpBody);
          addFormTextArea(F("Body"), F("P011httpbody"), HttpBody, C011_HTTP_BODY_MAX_LEN - 1, 8, 50);
        }
      }
      {
        // Place in scope to delete ControllerSettings as soon as it is no longer needed
        MakeControllerSettings(ControllerSettings);

        if (!AllocatedControllerSettings()) {
          addHtmlError(F("Out of memory, cannot load page"));
        } else {
          LoadControllerSettings(event->ControllerIndex, ControllerSettings);
          addControllerParameterForm(ControllerSettings, event->ControllerIndex, ControllerSettingsStruct::CONTROLLER_SEND_BINARY);
          addFormNote(F("Do not 'percent escape' body when send binary checked"));
        }
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_WEBFORM_SAVE:
    {
      std::shared_ptr<C011_ConfigStruct> customConfig;
      {
        // Try to allocate on 2nd heap
        #ifdef USE_SECOND_HEAP
//        HeapSelectIram ephemeral;
        #endif
        std::shared_ptr<C011_ConfigStruct> tmp_shared(new (std::nothrow) C011_ConfigStruct);
        customConfig = std::move(tmp_shared);
      }

      if (customConfig) {
        uint8_t   choice    = 0;
        String methods[] = { F("GET"), F("POST"), F("PUT"), F("HEAD"), F("PATCH") };

        for (uint8_t i = 0; i < 5; i++)
        {
          if (methods[i].equals(customConfig->HttpMethod)) {
            choice = i;
          }
        }

        int httpmethod    = getFormItemInt(F("P011httpmethod"), choice);
        String httpuri    = webArg(F("P011httpuri"));
        String httpheader = webArg(F("P011httpheader"));
        String httpbody   = webArg(F("P011httpbody"));

        strlcpy(customConfig->HttpMethod, methods[httpmethod].c_str(), sizeof(customConfig->HttpMethod));
        strlcpy(customConfig->HttpUri,    httpuri.c_str(),             sizeof(customConfig->HttpUri));
        strlcpy(customConfig->HttpHeader, httpheader.c_str(),          sizeof(customConfig->HttpHeader));
        strlcpy(customConfig->HttpBody,   httpbody.c_str(),            sizeof(customConfig->HttpBody));
        customConfig->zero_last();
        SaveCustomControllerSettings(event->ControllerIndex, (uint8_t *)customConfig.get(), sizeof(C011_ConfigStruct));
      }
      break;
    }

    case CPlugin::Function::CPLUGIN_PROTOCOL_SEND:
    {
      success = Create_schedule_HTTP_C011(event);
      break;
    }

    case CPlugin::Function::CPLUGIN_FLUSH:
    {
      process_c011_delay_queue();
      delay(0);
      break;
    }

    default:
      break;
  }
  return success;
}

// ********************************************************************************
// Generic HTTP request
// ********************************************************************************

// Uncrustify may change this into multi line, which will result in failed builds
// *INDENT-OFF*
bool do_process_c011_delay_queue(int controller_number, const C011_queue_element& element, ControllerSettingsStruct& ControllerSettings);

bool do_process_c011_delay_queue(int controller_number, const C011_queue_element& element, ControllerSettingsStruct& ControllerSettings) {
// *INDENT-ON*
  WiFiClient client;

  if (!NetworkConnected()) { return false; }

  int httpCode = -1;

  send_via_http(
    controller_number,
    ControllerSettings,
    element.controller_idx,
    client,
    element.uri,
    element.HttpMethod,
    element.header,
    element.postStr,
    httpCode);

  // HTTP codes:
  // 1xx Informational response
  // 2xx Success
  return httpCode >= 100 && httpCode < 300;
}

bool load_C011_ConfigStruct(controllerIndex_t ControllerIndex, String& HttpMethod, String& HttpUri, String& HttpHeader, String& HttpBody) {
  // Just copy the needed strings and destruct the C011_ConfigStruct as soon as possible
  std::shared_ptr<C011_ConfigStruct> customConfig;
  {
    // Try to allocate on 2nd heap
    #ifdef USE_SECOND_HEAP
//    HeapSelectIram ephemeral;
    #endif
    std::shared_ptr<C011_ConfigStruct> tmp_shared(new (std::nothrow) C011_ConfigStruct);
    customConfig = std::move(tmp_shared);
  }

  if (!customConfig) {
    return false;
  }
  LoadCustomControllerSettings(ControllerIndex, (uint8_t *)customConfig.get(), sizeof(C011_ConfigStruct));
  customConfig->zero_last();
  HttpMethod = customConfig->HttpMethod;
  HttpUri    = customConfig->HttpUri;
  HttpHeader = customConfig->HttpHeader;
  HttpBody   =  customConfig->HttpBody;
  return true;
}

// ********************************************************************************
// Create request
// ********************************************************************************
boolean Create_schedule_HTTP_C011(struct EventStruct *event)
{
  if (C011_DelayHandler == nullptr) {
    addLog(LOG_LEVEL_ERROR, F("No C011_DelayHandler"));
    return false;
  }
  LoadTaskSettings(event->TaskIndex);

  // Add a new element to the queue with the minimal payload
  bool success = C011_DelayHandler->addToQueue(C011_queue_element(event));

  if (success) {
    // Element was added.
    // Now we try to append to the existing element
    // and thus preventing the need to create a long string only to copy it to a queue element.
    C011_queue_element& element = C011_DelayHandler->sendQueue.back();

    if (!load_C011_ConfigStruct(event->ControllerIndex, element.HttpMethod, element.uri, element.header, element.postStr))
    {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        String log = F("C011   : ");
        log += element.HttpMethod;
        log += element.uri;
        log += element.header;
        log += element.postStr;
        addLog(LOG_LEVEL_ERROR, log);
      }
      C011_DelayHandler->sendQueue.pop_back();
      return false;
    }

    ReplaceTokenByValue(element.uri,    event, false);
    ReplaceTokenByValue(element.header, event, false);

    if (element.postStr.length() > 0)
    {
      ReplaceTokenByValue(element.postStr, event, C011_sendBinary);
    }
  } else {
    addLog(LOG_LEVEL_ERROR, F("C011  : Could not add to delay handler"));
  }

  Scheduler.scheduleNextDelayQueue(ESPEasy_Scheduler::IntervalTimer_e::TIMER_C011_DELAY_QUEUE, C011_DelayHandler->getNextScheduleTime());
  return success;
}

// parses the string and returns only the the number of name/values we want
// according to the parameter numberOfValuesWanted
void DeleteNotNeededValues(String& s, uint8_t numberOfValuesWanted)
{
  numberOfValuesWanted++;

  for (uint8_t i = 1; i < 5; i++)
  {
    String startToken;
    startToken += '%';
    startToken += i;
    startToken += '%';
    String endToken = F("%/");
    endToken += i;
    endToken += '%';

    // do we want to keep this one?
    if (i < numberOfValuesWanted)
    {
      // yes, so just remove the tokens
      s.replace(startToken, EMPTY_STRING);
      s.replace(endToken,  EMPTY_STRING);
    }
    else
    {
      // remove all the whole strings including tokes
      int startIndex = s.indexOf(startToken);
      int endIndex   = s.indexOf(endToken);

      while (startIndex != -1 && endIndex != -1  && endIndex > startIndex)
      {
        String p = s.substring(startIndex, endIndex + 4);

        // remove the whole string including tokens
        s.replace(p, EMPTY_STRING);

        // find next ones
        startIndex = s.indexOf(startToken);
        endIndex   = s.indexOf(endToken);
      }
    }
  }
}

// ********************************************************************************
// Replace the token in a string by real value.
//
// Example:
// %1%%vname1%____%tskname%____%val1%%/1%%2%%__%%vname2%____%tskname%____%val2%%/2%
// will become in case of a sensor with 1 value:
// SENSORVALUENAME1____TASKNAME1____VALUE1  <- everything not between %1% and %/1% will be discarded
// in case of a sensor with 2 values:
// SENSORVALUENAME1____TASKNAME1____VALUE1__SENSORVALUENAME2____TASKNAME2____VALUE2
// ********************************************************************************
void ReplaceTokenByValue(String& s, struct EventStruct *event, bool sendBinary)
{
  // example string:
  // write?db=testdb&type=%1%%vname1%%/1%%2%;%vname2%%/2%%3%;%vname3%%/3%%4%;%vname4%%/4%&value=%1%%val1%%/1%%2%;%val2%%/2%%3%;%val3%%/3%%4%;%val4%%/4%
  //	%1%%vname1%,Standort=%tskname% Wert=%val1%%/1%%2%%LF%%vname2%,Standort=%tskname% Wert=%val2%%/2%%3%%LF%%vname3%,Standort=%tskname%
  //  Wert=%val3%%/3%%4%%LF%%vname4%,Standort=%tskname% Wert=%val4%%/4%
  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
    addLog(LOG_LEVEL_DEBUG_MORE, F("HTTP before parsing: "));
    addLog(LOG_LEVEL_DEBUG_MORE, s);
  }
  const uint8_t valueCount = getValueCountForTask(event->TaskIndex);

  DeleteNotNeededValues(s, valueCount);

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
    addLog(LOG_LEVEL_DEBUG_MORE, F("HTTP after parsing: "));
    addLog(LOG_LEVEL_DEBUG_MORE, s);
  }

  parseControllerVariables(s, event, !sendBinary);

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
    addLog(LOG_LEVEL_DEBUG_MORE, F("HTTP after replacements: "));
    addLog(LOG_LEVEL_DEBUG_MORE, s);
  }
}

#endif // ifdef USES_C011
