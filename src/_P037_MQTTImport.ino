#include "_Plugin_Helper.h"
#ifdef USES_P037
//#######################################################################################################
//#################################### Plugin 037: MQTT Import ##########################################
//#######################################################################################################


// Original plugin created by Namirda

// This task reads data from the MQTT Import input stream and saves the value

/**
 * 2021-02-13, tonhuisman: Refactoring to reduce memory use and String re-allocations
 * 2020-12-10, tonhuisman: Add name-value mapping, filtering and json parsing
 * 2020-12-17, tonhuisman: Bugfixes, filter per MQTT Topic, reorganized Device page
 */

#include "src/PluginStructs/P037_data_struct.h"

#define PLUGIN_037
#define PLUGIN_ID_037         37
#define PLUGIN_NAME_037       "Generic - MQTT Import"

#define PLUGIN_VALUENAME1_037 "Value1"
#define PLUGIN_VALUENAME2_037 "Value2"
#define PLUGIN_VALUENAME3_037 "Value3"
#define PLUGIN_VALUENAME4_037 "Value4"


# define P037_PARSE_JSON      PCONFIG(1)  // Parse/process json messages
# define P037_APPLY_MAPPINGS  PCONFIG(2)  // Apply mapping strings to numbers
# define P037_APPLY_FILTERS   PCONFIG(3)  // Apply filtering on data values
# define P037_SEND_EVENTS     PCONFIG(4)  // Send event for each received topic

#if defined(P037_MAPPING_SUPPORT) || defined(P037_JSON_SUPPORT)
String P037_getMQTTLastTopicPart(String topic) {
  String result = topic;
  result.trim();
  int16_t lastSlash = result.lastIndexOf('/');
  result = result.substring(lastSlash + 1); // Take last part of the topic
  return result;
}
#endif // P037_MAPPING_SUPPORT || P037_JSON_SUPPORT

boolean Plugin_037(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_037;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_SINGLE;     // This means it has a single pin
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true; // Need this in order to get the decimals option
        Device[deviceCount].ValueCount = VARS_PER_TASK;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_037);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_037));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_037));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_037));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_037));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormSubHeader(F("Options"));
        #if defined(P037_MAPPING_SUPPORT) || defined(P037_JSON_SUPPORT) || defined(P037_FILTER_SUPPORT)
        const __FlashStringHelper * optionsNoYes[2] = { F("No"), F("Yes") };
        int optionValuesNoYes[2] = { 0, 1 };
        #endif

        #ifdef P037_JSON_SUPPORT
        addFormSelector(F("Parse JSON messages"), F("p037_parse_json"),     2, optionsNoYes, optionValuesNoYes, P037_PARSE_JSON, true);
        #endif
        #ifdef P037_FILTER_SUPPORT
        addFormSelector(F("Apply filters"),       F("p037_apply_filters"),  2, optionsNoYes, optionValuesNoYes, P037_APPLY_FILTERS, true);
        #endif
        #ifdef P037_MAPPING_SUPPORT
        addFormSelector(F("Apply mappings"),      F("p037_apply_mappings"), 2, optionsNoYes, optionValuesNoYes, P037_APPLY_MAPPINGS, true);
        #endif
        #if defined(P037_MAPPING_SUPPORT) || defined(P037_JSON_SUPPORT) || defined(P037_FILTER_SUPPORT)
        addFormNote(F("Changing a Yes/No option will reload the page. Changing to No will clear corresponding settings!"));
        #endif
        addFormCheckBox(F("Generate events for accepted topics"),
                                                F("p037_send_events"), P037_SEND_EVENTS);
        addFormNote(F("Event: &lt;TaskName&gt;#&lt;topic&gt;=&lt;payload&gt;"));

        P037_data_struct *P037_data = new (std::nothrow) P037_data_struct(event->TaskIndex);

        if (nullptr == P037_data) {
          return success;
        }
        success = P037_data->webform_load(
        #ifdef P037_MAPPING_SUPPORT
                                          P037_APPLY_MAPPINGS
        #endif
        #if defined(P037_MAPPING_SUPPORT) && defined(P037_FILTER_SUPPORT)
                                          ,
        #endif
        #ifdef P037_FILTER_SUPPORT
                                          P037_APPLY_FILTERS
        #endif
        #if (defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)) && defined(P037_JSON_SUPPORT)
                                          ,
        #endif
        #ifdef P037_JSON_SUPPORT
                                          P037_PARSE_JSON
        #endif
                                         );
        delete P037_data;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        #ifdef P037_JSON_SUPPORT
        P037_PARSE_JSON     = getFormItemInt(F("p037_parse_json"));
        #endif
        #ifdef P037_MAPPING_SUPPORT
        P037_APPLY_MAPPINGS = getFormItemInt(F("p037_apply_mappings"));
        #endif
        #ifdef P037_FILTER_SUPPORT
        P037_APPLY_FILTERS  = getFormItemInt(F("p037_apply_filters"));
        #endif
        P037_SEND_EVENTS    = isFormItemChecked(F("p037_send_events"));

        P037_data_struct *P037_data = new (std::nothrow) P037_data_struct(event->TaskIndex);

        if (nullptr == P037_data) {
          return success;
        }
        success = P037_data->webform_save(
        #ifdef P037_FILTER_SUPPORT
                                          P037_APPLY_FILTERS
        #endif
        #if defined(P037_FILTER_SUPPORT) && defined(P037_JSON_SUPPORT)
                                          ,
        #endif
        #ifdef P037_JSON_SUPPORT
                                          P037_PARSE_JSON
        #endif
                                         );
        delete P037_data;
        break;
      }

    case PLUGIN_INIT:
      {
        success = false;
        initPluginTaskData(event->TaskIndex, new (std::nothrow) P037_data_struct(event->TaskIndex));

        //    When we edit the subscription data from the webserver, the plugin is called again with init.
        //    In order to resubscribe we have to disconnect and reconnect in order to get rid of any obsolete subscriptions
        if (MQTTclient_connected) {
          //		Subscribe to ALL the topics from ALL instance of this import module
          MQTTSubscribe_037(event);
          success = true;
        }
        break;
      }

    case PLUGIN_EXIT:
      {
        P037_data_struct *P037_data = static_cast<P037_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr == P037_data) {
          return success;
        }

        delete P037_data;

        break;
      }



    case PLUGIN_READ:
      {
        // This routine does not output any data and so we do not need to respond to regular read requests

        success = false;
        break;
      }

    case PLUGIN_MQTT_CONNECTION_STATE:
      {
        const bool currentConnectedState = event->Par1 == 1;
        if (P037_MQTTImport_connected != currentConnectedState) {
          P037_MQTTImport_connected = currentConnectedState;
          if (Settings.UseRules) {
            eventQueue.add(currentConnectedState ? F("MQTTimport#Connected") : F("MQTTimport#Disconnected"));
          }
        }

        if (currentConnectedState) {
          success = MQTTSubscribe_037(event);
        }
        break;
      }

    case PLUGIN_MQTT_IMPORT:
      {
        LoadTaskSettings(event->TaskIndex);

        // Resolved tonhuisman: TD-er: It may be useful to generate events with string values.
        // Get the payload and check it out
        String Payload         = event->String2;
        String unparsedPayload; // To keep an unprocessed copy

        // #ifdef PLUGIN_037_DEBUG
        // String info = F("P037 : topic: ");
        // info += event->String1;
        // info += F(" value: ");
        // info += Payload;
        // addLog(LOG_LEVEL_INFO, info);
        // #endif

        P037_data_struct *P037_data = static_cast<P037_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr == P037_data) {
          return success;
        }

        bool checkJson = false;

        #if defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT) || defined(P037_JSON_SUPPORT)
        bool processData = false; // Don't do the for loop again if we're not going to match
        // As we can receive quite a lot of topics not intended for this plugin,
        // first do a quick check if the topic matches here, to try and avoid a bunch of unneeded mapping, filtering and logging
        bool matchedTopic = false; // Ignore by default
        for (uint8_t x = 0; x < VARS_PER_TASK; x++)
        {
          if (P037_data->deviceTemplate[x].length() == 0) continue;							// skip blank subscriptions

          // Now check if the incoming topic matches one of our subscriptions
          String subscriptionTopicParsed = P037_data->deviceTemplate[x];
          parseSystemVariables(subscriptionTopicParsed, false);
          if (MQTTCheckSubscription_037(event->String1, subscriptionTopicParsed)) {
            matchedTopic = true; // Yes we should process it here
            processData = true; // Allow going into second for loop
          }
        }
        #else
        bool processData = true;
        #endif // P037_MAPPING_SUPPORT or P037_FILTER_SUPPORT or P037_JSON_SUPPORT
        #ifdef P037_JSON_SUPPORT
        if (matchedTopic && P037_PARSE_JSON && Payload.substring(0, 1) == F("{")) { // With JSON enabled and rudimentary check for JSon content
          #ifdef PLUGIN_037_DEBUG
          addLog(LOG_LEVEL_INFO, F("IMPT : MQTT 037 JSON data detected."));
          #endif
          checkJson = true;
        }
        #endif // P037_JSON_SUPPORT
        if (!checkJson) { // Avoid storing any json in an extra copy in memory
          unparsedPayload = event->String2;
        }

        String log;
        log.reserve(64);

        bool continueProcessing = false;
        String key;

        #ifdef P037_MAPPING_SUPPORT
        if (matchedTopic && !checkJson && P037_APPLY_MAPPINGS) { // Apply mappings?
          key = P037_getMQTTLastTopicPart(event->String1);
          Payload = P037_data->mapValue(Payload, key);
        }
        #endif // P037_MAPPING_SUPPORT

        #ifdef P037_JSON_SUPPORT
        if (checkJson) {
          continueProcessing = P037_data->parseJSONMessage(event->String2);
        }
        #endif // P037_JSON_SUPPORT

        #ifdef P037_FILTER_SUPPORT
        #ifdef P037_FILTER_PER_TOPIC
        for (uint8_t x = 0; x < VARS_PER_TASK && matchedTopic; x++) {
          if (P037_data->deviceTemplate[x].length() == 0) continue;							// skip blank subscriptions
        #else
        int8_t x = -1;
        if (matchedTopic) {
        #endif // P037_FILTER_PER_TOPIC
          // non-json filter check
          if (!checkJson && P037_data->hasFilters()) { // See if we pass the filters
            key = P037_getMQTTLastTopicPart(event->String1);
            #ifdef P037_MAPPING_SUPPORT
            if (P037_APPLY_MAPPINGS) {
              Payload = P037_data->mapValue(Payload, key);
            }
            #endif // P037_MAPPING_SUPPORT
            processData = P037_data->checkFilters(key, Payload, x + 1); // Will return true unless key matches *and* Payload doesn't
          }
          #ifdef P037_JSON_SUPPORT

          #ifndef P037_FILTER_PER_TOPIC
          // json filter check
          if (checkJson && P037_data->hasFilters()) { // See if we pass the filters for all json attributes
            do {
              key = P037_data->iter->key().c_str();
              Payload = P037_data->iter->value().as<String>();
              #ifdef P037_MAPPING_SUPPORT
              if (P037_APPLY_MAPPINGS) {
                Payload = P037_data->mapValue(Payload, key);
              }
              #endif // P037_MAPPING_SUPPORT
              processData = P037_data->checkFilters(key, Payload, x + 1); // Will return true unless key matches *and* Payload doesn't
              ++P037_data->iter;
            } while (processData && P037_data->iter != P037_data->doc.end());
            P037_data->iter = P037_data->doc.begin();
          }
          #endif // P037_FILTER_PER_TOPIC
          #endif // P037_JSON_SUPPORT
        }
        if (matchedTopic && P037_data->hasFilters()) { // Single log statement
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            log = F("IMPT : MQTT 037 filter result: ");
            log += processData ? F("true") : F("false");
            addLog(LOG_LEVEL_INFO, log);
          }
        }
        #endif // P037_FILTER_SUPPORT

        // Get the Topic and see if it matches any of the subscriptions
        for (uint8_t x = 0; x < VARS_PER_TASK && processData; x++)
        {
          if (P037_data->deviceTemplate[x].length() == 0) continue;							// skip blank subscriptions

          // Now check if the incoming topic matches one of our subscriptions
          String subscriptionTopicParsed = P037_data->deviceTemplate[x];
          parseSystemVariables(subscriptionTopicParsed, false);
          if (MQTTCheckSubscription_037(event->String1, subscriptionTopicParsed)) {
            #ifdef P037_JSON_SUPPORT
            #ifdef P037_FILTER_PER_TOPIC
            // json filter check
            bool passFilter = true;
            if (checkJson && P037_data->hasFilters()) { // See if we pass the filters for all json attributes
              do {
                key = P037_data->iter->key().c_str();
                Payload = P037_data->iter->value().as<String>();
                #ifdef P037_MAPPING_SUPPORT
                if (P037_APPLY_MAPPINGS) {
                  Payload = P037_data->mapValue(Payload, key);
                }
                #endif // P037_MAPPING_SUPPORT
                passFilter = P037_data->checkFilters(key, Payload, x + 1); // Will return true unless key matches *and* Payload doesn't
                ++P037_data->iter;
              } while (passFilter && P037_data->iter != P037_data->doc.end());
              P037_data->iter = P037_data->doc.begin();
            }
            if (passFilter) // Watch it, no curly braces, so only 1 statement processed: (do {...) !
            #endif // P037_FILTER_PER_TOPIC
            #endif // P037_JSON_SUPPORT
            do {
              #ifdef P037_JSON_SUPPORT
              if (checkJson && P037_data->iter != P037_data->doc.end()) {
                String jsonAttribute = P037_data->StoredSettings.jsonAttributes[x];
                jsonAttribute.replace(';', ',');
                String jsonIndex = parseString(jsonAttribute, 2);
                jsonAttribute = parseString(jsonAttribute, 1);
                jsonAttribute.trim();
                if (jsonAttribute.length() > 0) {
                  key = jsonAttribute;
                  Payload = P037_data->doc[key].as<String>();
                  unparsedPayload = Payload;
                  int8_t jIndex = jsonIndex.toInt();
                  if (jIndex > 1) {
                    Payload.replace(';', ',');
                    Payload = parseString(Payload, jIndex);
                  }
                  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                    log = F("IMPT : MQTT 037 fetched json attribute: ");
                    log += key;
                    log += F(" payload: ");
                    log += Payload;
                    if (jsonIndex.length() > 0) {
                      log += F(" index: ");
                      log += jsonIndex;
                    }
                    addLog(LOG_LEVEL_INFO, log);
                  }
                  continueProcessing = false; // no need to loop over all attributes, the configured one is found
                } else {
                  key = P037_data->iter->key().c_str();
                  Payload = P037_data->iter->value().as<String>();
                  unparsedPayload = Payload;
                }
                #ifdef PLUGIN_037_DEBUG
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  log = F("P037 json key: ");
                  log += key;
                  log += F(" payload: ");
                  #ifdef P037_MAPPING_SUPPORT
                  log += (P037_APPLY_MAPPINGS ? P037_data->mapValue(Payload, key) : Payload);
                  #else
                  log += Payload;
                  #endif // P037_MAPPING_SUPPORT
                  addLog(LOG_LEVEL_INFO, log);
                }
                #endif
                ++P037_data->iter;
              }
              #ifdef P037_MAPPING_SUPPORT
              if (P037_APPLY_MAPPINGS) {
                Payload = P037_data->mapValue(Payload, key);
              }
              #endif // P037_MAPPING_SUPPORT
              #endif // P037_JSON_SUPPORT
              bool numericPayload = true; // Unless it's not
              if (!checkJson || (checkJson && key.length() > 0)) {
                float floatPayload;
                if (!string2float(Payload, floatPayload)) {
                  if (!checkJson && P037_SEND_EVENTS == 0) { // If we want all values as events, then no error logged and don't stop here
                    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                      log = F("IMPT : Bad Import MQTT Command ");
                      log += event->String1;
                      addLog(LOG_LEVEL_ERROR, log);
                      log = F("ERR  : Illegal Payload ");
                      log += Payload;
                      log += ' ';
                      log += getTaskDeviceName(event->TaskIndex);
                      addLog(LOG_LEVEL_INFO, log);
                    }
                    success = false;
                    break;
                  }
                  numericPayload = false; // No, it isn't numeric
                  floatPayload = NAN;     // Invalid value
                }
                UserVar[event->BaseVarIndex + x] = floatPayload;							// Save the new value

                if (!checkJson && P037_SEND_EVENTS && Settings.UseRules) { // Generate event of all non-json topic/payloads
                  String RuleEvent;
                  RuleEvent.reserve(64);
                  RuleEvent  = getTaskDeviceName(event->TaskIndex);
                  RuleEvent += '#';
                  RuleEvent += event->String1;
                  RuleEvent += '=';
                  RuleEvent += unparsedPayload;
                  eventQueue.addMove(std::move(RuleEvent));
                }

                // Log the event
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  String log = F("IMPT : [");
                  log += getTaskDeviceName(event->TaskIndex);
                  log += '#';
                  if (checkJson) {
                    log += key;
                  } else {
                    log += ExtraTaskSettings.TaskDeviceValueNames[x];
                  }
                  log += F("] : ");
                  log += floatPayload;
                  addLog(LOG_LEVEL_INFO, log);
                }

                // Generate event for rules processing - proposed by TridentTD

                if (Settings.UseRules) {
                  String RuleEvent;
                  RuleEvent.reserve(64);
                  if (checkJson) {
                    // For JSON payloads generate <Topic>#<Attribute>=<Payload> event
                    RuleEvent  = event->String1;
                    #if defined(P037_FILTER_SUPPORT) && defined(P037_FILTER_PER_TOPIC)
                    RuleEvent += P037_data->getFilterAsTopic(x + 1);
                    #endif
                    RuleEvent += '#';
                    RuleEvent += key;
                    RuleEvent += '=';
                    bool hasSemicolon = unparsedPayload.indexOf(';') > -1;
                    if (numericPayload && !hasSemicolon) {
                      RuleEvent += floatPayload;
                    } else if (numericPayload && hasSemicolon) { // semicolon separated list, pass unparsed
                      RuleEvent += Payload;
                      RuleEvent += ',';
                      RuleEvent += unparsedPayload;
                    } else {
                      RuleEvent += Payload; // Pass mapped result
                    }
                    eventQueue.addMove(std::move(RuleEvent));
                  } 
                  // (Always) Generate <Taskname>#<Valuename>=<Payload> event
                  RuleEvent  = getTaskDeviceName(event->TaskIndex);
                  RuleEvent += '#';
                  RuleEvent += ExtraTaskSettings.TaskDeviceValueNames[x];
                  RuleEvent += '=';
                  if (numericPayload) {
                    RuleEvent += floatPayload;
                  } else {
                    RuleEvent += Payload;
                  }
                  eventQueue.addMove(std::move(RuleEvent));
                }
                #ifdef P037_JSON_SUPPORT
                if (checkJson && P037_data->iter == P037_data->doc.end()) {
                  continueProcessing = false;
                }
                #endif // P037_JSON_SUPPORT
              }
            } while (continueProcessing);

            success = true;
          }
        }
        #ifdef P037_JSON_SUPPORT
        if (checkJson) {
          P037_data->cleanupJSON(); // Free/cleanup memory
        }
        #endif // P037_JSON_SUPPORT

        break;

      }
    }

  return success;
}

bool MQTTSubscribe_037(struct EventStruct *event)
{
  // We must subscribe to the topics.
  P037_data_struct *P037_data = static_cast<P037_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P037_data) {
    return false;
  }

  //char deviceTemplate[VARS_PER_TASK][41];		// variable for saving the subscription topics
  LoadCustomTaskSettings(event->TaskIndex, (uint8_t*)&P037_data->StoredSettings, sizeof(P037_data->StoredSettings));

  // Now loop over all import variables and subscribe to those that are not blank
  for (uint8_t x = 0; x < VARS_PER_TASK; x++)
  {
    String subscribeTo = P037_data->StoredSettings.deviceTemplate[x];
    subscribeTo.trim();

    if (subscribeTo.length() > 0)
    {
      parseSystemVariables(subscribeTo, false);
      if (MQTTclient.subscribe(subscribeTo.c_str()))
      {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("IMPT : [");
          LoadTaskSettings(event->TaskIndex);
          log += getTaskDeviceName(event->TaskIndex);
          log += F("#");
          log += ExtraTaskSettings.TaskDeviceValueNames[x];
          log += F("] subscribed to ");
          log += subscribeTo;
          addLog(LOG_LEVEL_INFO, log);
        }
      }
      else
      {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log = F("IMPT : Error subscribing to ");
          log += subscribeTo;
          addLog(LOG_LEVEL_ERROR, log);
        }
        return false;
      }
    }
  }
  return true;
}

//
// Check to see if Topic matches the MQTT subscription
//
bool MQTTCheckSubscription_037(const String& Topic, const String& Subscription) {
  if (Topic.isEmpty() || Subscription.isEmpty())  {
    return false;
  }

  String tmpTopic = Topic;
  String tmpSub = Subscription;

  tmpTopic.trim();
  tmpSub.trim();

  // Get rid of leading '/'
  if (tmpTopic[0] == '/') { tmpTopic = tmpTopic.substring(1); }
  if (tmpSub[0] == '/') { tmpSub = tmpSub.substring(1); }

  // Test for multi-level wildcard (#) see: http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718107 (for MQTT 3 and MQTT 5)

  if (tmpSub == F("#")) return true; // If the subscription is for '#' then all topics are accepted
  if (tmpSub.endsWith(F("/#"))) { // A valid MQTT multi-level wildcard is a # at the end of the topic that's preceded by a /
    bool multiLevelWildcard = tmpTopic.startsWith(tmpSub.substring(0, tmpSub.length() - 1));
    if (tmpSub.indexOf('+') == -1) {
      return multiLevelWildcard; // It matched, or not
    }
  } else {
    if (tmpSub.indexOf('#') != -1) return false; // Invalid topic
  }

  // Add trailing / if required

  int lenTopic = tmpTopic.length();
  if (tmpTopic.substring(lenTopic - 1, lenTopic) != "/")tmpTopic += '/';

  int lenSub = tmpSub.length();
  if (tmpSub.substring(lenSub - 1, lenSub) != "/")tmpSub += '/';

  // Now get first part

  int SlashTopic;
  int SlashSub;
  int count = 0;

  String pTopic;
  String pSub;

  while (count < 10) {

    //  Get locations of the first /

    SlashTopic = tmpTopic.indexOf('/');
    SlashSub = tmpSub.indexOf('/');

    //  If no slashes found then match is OK
    //  If only one slash found then not OK

    if ((SlashTopic == -1) && (SlashSub == -1)) return true;
    if ((SlashTopic == -1) && (SlashSub != -1)) return false;
    if ((SlashTopic != -1) && (SlashSub == -1)) return false;

    //  Get the values for the current subtopic

    pTopic = tmpTopic.substring(0, SlashTopic);
    pSub = tmpSub.substring(0, SlashSub);

    //  And strip the subtopic from the topic

    tmpTopic = tmpTopic.substring(SlashTopic + 1);
    tmpSub = tmpSub.substring(SlashSub + 1);

    //  If the subtopics match then OK - otherwise fail
    if (pSub == "#")  return true;
    if ((pTopic != pSub) && (pSub != F("+"))) return false;

    count = count + 1;
  }
  return false;
}
#endif // USES_P037
