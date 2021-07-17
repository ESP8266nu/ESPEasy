#include "../PluginStructs/P037_data_struct.h"

#ifdef USES_P037

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Numerical.h"
#include "../WebServer/Markup_Forms.h"
#include "../WebServer/WebServer.h"
#include "../WebServer/Markup.h"
#include "../WebServer/HTML_wrappers.h"
#include "../ESPEasyCore/ESPEasyRules.h"

P037_data_struct::P037_data_struct(taskIndex_t taskIndex) : _taskIndex(taskIndex) {
  loadSettings();
}

P037_data_struct::~P037_data_struct() {}

/**
 * Load the settings from file
 */
bool P037_data_struct::loadSettings() {
  if (_taskIndex < TASKS_MAX) {
    String tmp;
    tmp.reserve(45);
    LoadCustomTaskSettings(_taskIndex, (uint8_t*)&StoredSettings, sizeof(StoredSettings));
    for (uint8_t i = 0; i < VARS_PER_TASK; i++) {
      tmp = StoredSettings.deviceTemplate[i];
      tmp.trim();
      deviceTemplate[i] = tmp;
      tmp = StoredSettings.jsonAttributes[i];
      tmp.trim();
      jsonAttributes[i] = tmp;
    }
    return true;
  }
  return false;
}

#if defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)
/**
 * Parse the mappings and filters from the settings-string into arrays
 */
void P037_data_struct::parseMappings() {

  if (
    #ifdef P037_MAPPING_SUPPORT
    _maxIdx == -1
    #endif
    #if defined(P037_MAPPING_SUPPORT) && defined(P037_FILTER_SUPPORT)
    ||
    #endif
    #ifdef P037_FILTER_SUPPORT
    _maxFilter == -1
    #endif
    ) {
    #ifdef P037_MAPPING_SUPPORT
    _maxIdx = 0; // Initialize to empty
    #endif
    #ifdef P037_FILTER_SUPPORT
    _maxFilter = 0; // Initialize to empty
    #endif
    #ifdef P037_MAPPING_SUPPORT
    _mapping.clear();
    #endif
    #ifdef P037_FILTER_SUPPORT
    _filter.clear();
    _filterIdx.clear();
    #endif

    String filterMap;
    String valueMap = String(StoredSettings.valueMappings);
    int16_t pipe = valueMap.indexOf(F("|"));
    if (pipe > -1) {
      filterMap = valueMap.substring(pipe + 1);
      valueMap = valueMap.substring(0, pipe);
    }
    #ifdef PLUGIN_037_DEBUG
    String debug;
    debug.reserve(64);
    #endif

    int16_t parse;
    int8_t operandIndex;
    #ifdef P037_MAPPING_SUPPORT
    String operands = P037_OPERAND_LIST; // Anticipate more operations

    while (valueMap.length() > 0 && _maxIdx < P037_MAX_MAPPINGS * 3) {
      int16_t comma   = valueMap.indexOf(F(","));
      int16_t equals  = valueMap.indexOf(operands.substring(0, 1));
      int16_t percent = valueMap.indexOf(operands.substring(1, 2));
      if (comma == -1) comma = valueMap.length(); // last value
      if ((equals == -1 && percent > -1) || (equals > -1 && percent > -1 && percent < equals)) {
        operandIndex = 1;
        parse = percent;
      } else {
        operandIndex = 0;
        parse = equals;
      }
      _mapping[_maxIdx + 0] = valueMap.substring(0, parse);
      _mapping[_maxIdx + 1] = operands.substring(operandIndex, operandIndex + 1);
      _mapping[_maxIdx + 2] = valueMap.substring(parse + 1, comma);
      #ifdef PLUGIN_037_DEBUG
      if (debug.length() > 50) {
        addLog(LOG_LEVEL_DEBUG, debug);
        debug = F("");
      }
      if (debug.length() == 0) {
        debug = F("P037 mapping:");
      }
      debug += ' ';
      debug += _mapping[_maxIdx + 0];
      debug += ' ';
      debug += _mapping[_maxIdx + 1];
      debug += ' ';
      debug += _mapping[_maxIdx + 2];
      debug += ';';
      #endif
      valueMap = valueMap.substring(comma + 1);
      _maxIdx += 3;
      delay(0);
    }
    #ifdef PLUGIN_037_DEBUG
    if (debug.length() > 0) {
      addLog(LOG_LEVEL_DEBUG, debug);
      debug = F("");
    }
    #endif
    #endif // P037_MAPPING_SUPPORT

    #ifdef P037_FILTER_SUPPORT
    #ifdef P037_FILTER_PER_TOPIC
    uint8_t countFilters = 0;
    #endif
    String filters = P037_FILTER_LIST; // Anticipate more filters
    while (filterMap.length() > 0 && _maxFilter < P037_MAX_FILTERS * 3) {
      int16_t comma  = filterMap.indexOf(F(","));
      int16_t equals = filterMap.indexOf(filters.substring(0, 1));
      int16_t dash   = filterMap.indexOf(filters.substring(1, 2));
      if (comma == -1) comma = filterMap.length(); // last value
      if ((equals == -1 && dash > -1) || (equals > -1 && dash > -1 && dash < equals)) {
        operandIndex = 1;
        parse = dash;
      } else {
        operandIndex = 0;
        parse = equals;
      }
      _filter[_maxFilter + 0] = filterMap.substring(0, parse);
      _filter[_maxFilter + 0].replace(';', ',');
      String _idx = parseString(_filter[_maxFilter + 0], 2);
      if (_idx.length() > 0) {
        _filterIdx[_maxFilter + 0] = _idx.toInt();
        _filter[_maxFilter + 0] = parseString(_filter[_maxFilter + 0], 1);
      }
      _filter[_maxFilter + 1] = filters.substring(operandIndex, operandIndex + 1);
      _filter[_maxFilter + 2] = filterMap.substring(parse + 1, comma);
      _filter[_maxFilter + 2].replace(';', ',');
      #ifdef P037_FILTER_PER_TOPIC
      countFilters += (_filter[_maxFilter + 0].length() > 0 && _filter[_maxFilter + 2].length() > 0 ? 1 : 0);
      #endif // P037_FILTER_PER_TOPIC
      #ifdef PLUGIN_037_DEBUG
      if (debug.length() > 50) {
        addLog(LOG_LEVEL_DEBUG, debug);
        debug = F("");
      }
      if (debug.length() == 0) {
        debug = F("P037 filter:");
      }
      debug += ' ';
      debug += _filter[_maxFilter + 0];
      debug += ' ';
      debug += _filter[_maxFilter + 1];
      debug += ' ';
      debug += _filter[_maxFilter + 2];
      debug += ';';
      #endif
      filterMap = filterMap.substring(comma + 1);
      _maxFilter += 3;
      delay(0);
    }
    #ifdef PLUGIN_037_DEBUG
    if (debug.length() > 0) {
      addLog(LOG_LEVEL_DEBUG, debug);
    }
    #endif
    #ifdef P037_FILTER_PER_TOPIC
    if (countFilters > 0) {
      _maxFilter = (P037_MAX_FILTERS * 3) - 1;
    } else {
      _maxFilter = 0;
    }
    #endif //P037_FILTER_PER_TOPIC
    #endif // P037_FILTER_SUPPORT
  }
} // parseMappings
#endif // P037_MAPPING_SUPPORT || P037_FILTER_SUPPORT

bool P037_data_struct::webform_load(
  #ifdef P037_MAPPING_SUPPORT
                                    bool mappingEnabled
  #endif
  #if defined(P037_MAPPING_SUPPORT) && defined(P037_FILTER_SUPPORT)
                                    ,
  #endif
  #ifdef P037_FILTER_SUPPORT
                                    bool filterEnabled
  #endif
  #if (defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)) && defined(P037_JSON_SUPPORT)
                                    ,
  #endif
  #ifdef P037_JSON_SUPPORT
                                    bool jsonEnabled
  #endif
                                   ) {

  bool success = false;

  addFormSubHeader(F("Topic subscriptions"));

  #ifdef P037_JSON_SUPPORT
  if (jsonEnabled) {
      addRowLabel(F("MQTT Topic"));
      html_table(F(""), false);  // Sub-table
      html_table_header(F("&nbsp;#&nbsp;"));
      html_table_header(F("Topic"), 500);
      html_table_header(F("JSON Attribute"), 200);
  }
  #endif
  for (uint8_t varNr = 0; varNr < VARS_PER_TASK; varNr++)
  {
    #ifdef P037_JSON_SUPPORT
    if (jsonEnabled) { // Add a column with the json attribute to use for value
      html_TR_TD();
      addHtml(F("&nbsp;"));
      addHtmlInt(varNr + 1);
      html_TD();
      addTextBox(String(F("p037_template")) + (varNr + 1),
                StoredSettings.deviceTemplate[varNr],
                40,
                false, false, F(""), F("wide"));
      html_TD();
      addTextBox(String(F("p037_attribute")) + (varNr + 1),
                StoredSettings.jsonAttributes[varNr],
                20,
                false, false, F(""), F(""));
      html_TD();
    } else {
    #endif
      addFormTextBox(String(F("MQTT Topic ")) + (varNr + 1), String(F("p037_template")) +
          (varNr + 1), StoredSettings.deviceTemplate[varNr], 40);
    #ifdef P037_JSON_SUPPORT
    }
    #endif
  }
  #ifdef P037_JSON_SUPPORT
  if (jsonEnabled) {
    html_end_table();
  }
  #endif

  #if defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)
  parseMappings();
  #endif

  #ifdef P037_FILTER_SUPPORT
  if (filterEnabled) {
    addFormSubHeader(F("Name - value filters"));
    addFormNote(F("Name - value filters are case-sensitive. Do not use ',' or '|'."));

    #ifdef P037_FILTER_PER_TOPIC
    addRowLabel(F("Filter for MQTT Topic"));
    #else
    addRowLabel(F("Filter"));
    #endif
    html_table(F(""), false);  // Sub-table
    html_table_header(F("&nbsp;#&nbsp;"));
    html_table_header(F("Name[;Index]"));
    html_table_header(F("Operand"), 180);
    html_table_header(F("Value"));

    const __FlashStringHelper * filterOptions[P037_FILTER_COUNT];
    filterOptions[0] = F("equals");     // map name to value
    filterOptions[1] = F("range");      // between 2 values
    #if P037_FILTER_COUNT >= 3
    filterOptions[2] = F("list");       // list of multiple values
    #endif
    int filterIndices[] = { 0, 1
    #if P037_FILTER_COUNT >= 3
    , 2
    #endif
    };

    String filters = P037_FILTER_LIST; // Anticipate more filters
    int8_t filterIndex;

    String info;
    info.reserve(25);

    int8_t idx;
    int8_t filterNr = 1;
    #ifdef P037_FILTER_PER_TOPIC
    if (_maxFilter <= 0) _maxFilter = (VARS_PER_TASK * 3);
    #endif
    for (idx = 0; idx < _maxFilter; idx += 3) {

      html_TR_TD();
      addHtml(F("&nbsp;"));
      addHtmlInt(filterNr);
      html_TD();
      addTextBox(getPluginCustomArgName(idx + 100 + 0),
                _filter[idx + 0],
                32,
                false, false, F("[^,|]{0,32}"), F(""));
      html_TD();
      filterIndex = filters.indexOf(_filter[idx + 1]);
      addSelector(getPluginCustomArgName(idx + 100 + 1), P037_FILTER_COUNT, filterOptions, filterIndices, NULL, filterIndex, false, true);
      html_TD();
      addTextBox(getPluginCustomArgName(idx + 100 + 2),
                _filter[idx + 2],
                32,
                false, false, F("[^,|]{0,32}"), F(""));
      addUnit(F("Range/List: separate values with ; "));
      html_TD();
      filterNr++;
    }
    #ifdef PLUGIN_037_DEBUG
    info = F("P037 maxFilter: ");
    info += _maxFilter;
    info += F(" idx: ");
    info += idx;
    addLog(LOG_LEVEL_INFO, info);
    #endif
    #ifndef P037_FILTER_PER_TOPIC
    filterIndex = 0;
    uint8_t extraFilters = 0;
    while (extraFilters < P037_EXTRA_VALUES && idx < P037_MAX_FILTERS * 3) {
      html_TR_TD();
      addHtml(F("&nbsp;"));
      addHtmlInt(filterNr);
      html_TD();
      addTextBox(getPluginCustomArgName(idx + 100 + 0),
                F(""),
                32,
                false, false, F("[^,|]{0,32}"), F(""));
      html_TD();
      addSelector(getPluginCustomArgName(idx + 100 + 1), P037_FILTER_COUNT, filterOptions, filterIndices, NULL, filterIndex, false, true);
      html_TD();
      addTextBox(getPluginCustomArgName(idx + 100 + 2),
                F(""),
                32,
                false, false, F("[^,|]{0,32}"), F(""));
      addUnit(F("Range/List: separate values with ; "));
      html_TD();
      idx += 3;
      extraFilters++;
      filterNr++;
    }
    #endif // ifndef P037_FILTER_PER_TOPIC
    html_end_table();
    #ifndef P037_FILTER_PER_TOPIC
    #ifdef PLUGIN_037_DEBUG
    info = F("P037 extraFilters: ");
    info += extraFilters;
    info += F(" idx: ");
    info += idx;
    addLog(LOG_LEVEL_INFO, info);
    #endif
    #endif
    addFormNote(F("Both Name and Value must be filled for a valid filter."));
    #ifndef P037_FILTER_PER_TOPIC
    if (extraFilters == P037_EXTRA_VALUES) {
      String moreMessage = F("After filling all filters, submitting this page will make extra filters available (up to ");
      moreMessage += P037_MAX_FILTERS;
      moreMessage += F(").");
      addFormNote(moreMessage);
    }
    #endif // ifndef P037_FILTER_PER_TOPIC
  }
  #endif  // P037_FILTER_SUPPORT

  #ifdef P037_MAPPING_SUPPORT
  if (mappingEnabled) {
    addFormSubHeader(F("Name - value mappings"));
    addFormNote(F("Name - value mappings are case-sensitive. Do not use ',' or '|'."));

    addRowLabel(F("Mapping"));
    html_table(F(""), false);  // Sub-table
    html_table_header(F("&nbsp;#&nbsp;"));
    html_table_header(F("Name"));
    html_table_header(F("Operand"), 180);
    html_table_header(F("Value"));

    const __FlashStringHelper * operandOptions[] = {
          F("map"),          // map name to int
          F("percentage") }; // map attribute value to percentage of provided value
    int operandIndices[] = { 0, 1 };

    String operands = P037_OPERAND_LIST; // Anticipate more operations
    int8_t operandIndex;

    String info;
    info.reserve(25);

    int8_t idx;
    int8_t mapNr = 1;
    for (idx = 0; idx < _maxIdx; idx += 3) {

      html_TR_TD();
      addHtml(F("&nbsp;"));
      addHtmlInt(mapNr);
      html_TD();
      addTextBox(getPluginCustomArgName(idx + 0),
                _mapping[idx + 0],
                32,
                false, false, F("[^,|]{0,32}"), F(""));
      html_TD();
      operandIndex = operands.indexOf(_mapping[idx + 1]);
      addSelector(getPluginCustomArgName(idx + 1), P037_OPERAND_COUNT, operandOptions, operandIndices, NULL, operandIndex, false, true);
      html_TD();
      addTextBox(getPluginCustomArgName(idx + 2),
                _mapping[idx + 2],
                32,
                false, false, F("[^,|]{0,32}"), F(""));
      html_TD();
      mapNr++;
    }
    #ifdef PLUGIN_037_DEBUG
    info = F("P037 maxIdx: ");
    info += _maxIdx;
    info += F(" idx: ");
    info += idx;
    addLog(LOG_LEVEL_INFO, info);
    #endif
    operandIndex = 0;
    uint8_t extraMappings = 0;
    while (extraMappings < P037_EXTRA_VALUES && idx < P037_MAX_MAPPINGS * 3) {
      html_TR_TD();
      addHtml(F("&nbsp;"));
      addHtmlInt(mapNr);
      html_TD();
      addTextBox(getPluginCustomArgName(idx + 0),
                F(""),
                32,
                false, false, F("[^,|]{0,32}"), F(""));
      html_TD();
      addSelector(getPluginCustomArgName(idx + 1), P037_OPERAND_COUNT, operandOptions, operandIndices, NULL, operandIndex, false, true);
      html_TD();
      addTextBox(getPluginCustomArgName(idx + 2),
                F(""),
                32,
                false, false, F("[^,|]{0,32}"), F(""));
      html_TD();
      idx += 3;
      extraMappings++;
      mapNr++;
    }
    html_end_table();
    #ifdef PLUGIN_037_DEBUG
    info = F("P037 extraMappings: ");
    info += extraMappings;
    info += F(" idx: ");
    info += idx;
    addLog(LOG_LEVEL_INFO, info);
    #endif
    addFormNote(F("Both Name and Value must be filled for a valid mapping."));
    if (extraMappings == P037_EXTRA_VALUES) {
      String moreMessage = F("After filling all mappings, submitting this page will make extra mappings available (up to ");
      moreMessage += P037_MAX_MAPPINGS;
      moreMessage += F(").");
      addFormNote(moreMessage);
    }
  }
  #endif // P037_MAPPING_SUPPORT

  success = true;
  return success;
} // webform_load

bool P037_data_struct::webform_save(
  #ifdef P037_FILTER_SUPPORT
                                    bool filterEnabled
  #endif
  #if defined(P037_FILTER_SUPPORT) && defined(P037_JSON_SUPPORT)
                                    ,
  #endif
  #ifdef P037_JSON_SUPPORT
                                    bool jsonEnabled
  #endif
                                   ) {
  bool success = false;

  String error;
  error.reserve(80); // Estimated
  for (uint8_t varNr = 0; varNr < VARS_PER_TASK; varNr++)
  {
    String argName = F("p037_template");
    argName += varNr + 1;
    if (!safe_strncpy(StoredSettings.deviceTemplate[varNr], web_server.arg(argName).c_str(), sizeof(StoredSettings.deviceTemplate[varNr]))) {
      error += getCustomTaskSettingsError(varNr);
    }
    #ifdef P037_JSON_SUPPORT
    if (jsonEnabled) {
      argName = F("p037_attribute");
      argName += varNr + 1;
      if (!safe_strncpy(StoredSettings.jsonAttributes[varNr], web_server.arg(argName).c_str(), sizeof(StoredSettings.jsonAttributes[varNr]))) {
        error += getCustomTaskSettingsError(varNr);
      }
    }
    #endif // P037_JSON_SUPPORT
  }

  #if defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)
  String valueMap;
  valueMap.reserve(sizeof(StoredSettings.valueMappings) / 2);

  String left, right;
  bool firstError;
  #endif

// Mapping must be processed first, then Filtering, because they are parsed in that order
  #ifdef P037_MAPPING_SUPPORT
  firstError = true;
  String operands = P037_OPERAND_LIST;
  uint8_t mapNr = 1;
  left.reserve(32);
  right.reserve(32);
  for (int8_t idx = 0; idx < P037_MAX_MAPPINGS * 3; idx += 3) {
    left =  web_server.arg(getPluginCustomArgName(idx + 0));
    left.trim();
    right = web_server.arg(getPluginCustomArgName(idx + 2));
    right.trim();
    if (left.length() > 0 || right.length() > 0) {
      if (valueMap.length() > 0) {
        valueMap += ',';
      }
      valueMap += left;
      uint8_t oper = getFormItemInt(getPluginCustomArgName(idx + 1));
      valueMap += operands.substring(oper, oper + 1);
      valueMap += right;
    }
    if ((left.length() == 0 && right.length() > 0) || (left.length() > 0 && right.length() == 0)) {
      if (firstError) {
        error += F("Name and value should both be filled for mapping ");
        firstError = false;
      } else {
        error += ',';
      }
      error += mapNr;
    }
    mapNr++;
    delay(0); // leave some yield
  }
  if (!firstError) {
    error += '\n';
  }
  #endif

// Filtering must be processed second, after Mapping, because they are parsed in that order
  #ifdef P037_FILTER_SUPPORT
  String filters = P037_FILTER_LIST;
  firstError = true;
  String filterMap;
  uint8_t filterNr = 1;
  filterMap.reserve(sizeof(StoredSettings.valueMappings) / 2);
  for (int8_t idx = 0; idx < P037_MAX_FILTERS * 3; idx += 3) {
    left =  web_server.arg(getPluginCustomArgName(idx + 100 + 0));
    left.trim();
    right = web_server.arg(getPluginCustomArgName(idx + 100 + 2));
    right.trim();
    if (left.length() > 0 || right.length() > 0
    #ifdef P037_FILTER_PER_TOPIC
     || true  // Store all filters and in the same order, including empty filters
    #endif
       ) {
      if (filterMap.length() > 0) {
        filterMap += ',';
      }
      filterMap += left;
      uint8_t oper = getFormItemInt(getPluginCustomArgName(idx + 100 + 1));
      filterMap += filters.substring(oper, oper + 1);
      filterMap += right;
    }
    #ifndef P037_FILTER_PER_TOPIC
    if ((left.length() == 0 && right.length() > 0) || (left.length() > 0 && right.length() == 0)) {
      if (firstError) {
        error += F("Name and value should both be filled for filter ");
        firstError = false;
      } else {
        error += ',';
      }
      error += filterNr;
    }
    #endif // ifndef P037_FILTER_PER_TOPIC
    filterNr++;
    delay(0); // leave some yield
  }
  #ifndef P037_FILTER_PER_TOPIC
  if (!firstError) {
    error += '\n';
  }
  #endif // ifndef P037_FILTER_PER_TOPIC
  if (filterMap.length() > 0) { // Append filters to mappings if used
    valueMap += '|';
    valueMap += filterMap;
  }
  #endif // P037_FILTER_SUPPORT

  #if defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)
  if (!safe_strncpy(StoredSettings.valueMappings, valueMap.c_str(), sizeof(StoredSettings.valueMappings))) {
    error += F("Total combination of mappings/filters too long to store.\n");
  }
  #endif

  if (error.length() > 0) {
    addHtmlError(error);
  }
  #if defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)
  #ifdef PLUGIN_037_DEBUG
  String info = F("P037 Saved mappings/filters, length: ");
  info += valueMap.length();
  info += F(" free: ");
  info += sizeof(StoredSettings.valueMappings) - valueMap.length();
  addLog(LOG_LEVEL_INFO, info);
  addLog(LOG_LEVEL_INFO, valueMap);
  #endif
  #endif
  SaveCustomTaskSettings(_taskIndex, (uint8_t*)&StoredSettings, sizeof(StoredSettings));

  #ifdef P037_MAPPING_SUPPORT
  _maxIdx = -1; // Invalidate current mappings and filters
  #endif
  #ifdef P037_FILTER_SUPPORT
  _maxFilter = -1;
  #endif

  success = true;

  return success;
} // webform_save

#ifdef P037_MAPPING_SUPPORT
void P037_data_struct::logMapValue(const String& input, const String& result) {
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String info;
    info.reserve(45);
    info  = F("IMPT : MQTT 037 mapped value '");
    info += input;
    info += F("' to '");
    info += result;
    info += '\'';
    addLog(LOG_LEVEL_INFO, info);
  }
} // logMapValue

/**
 * Map a string to a (numeric) value, unchanged if no mapping found
 */
String P037_data_struct::mapValue(const String& input, const String& attribute) {
  String result = String(input); // clone
  if (input.length() > 0) {
    parseMappings();
    String operands = P037_OPERAND_LIST;
    for (int8_t idx = 0; idx < _maxIdx; idx += 3) {
      if (_mapping[idx + 0] == input || (attribute.length() > 0 && _mapping[idx + 0] == attribute)) {
        int8_t operandIndex = operands.indexOf(_mapping[idx + 1]);
        switch(operandIndex) {
          case 0: // = => 1:1 mapping
          {
            if (_mapping[idx + 2].length() > 0) {
              result = _mapping[idx + 2];
              logMapValue(input, result);
            }
            break;
          }
          case 1: // % => percentage of mapping
          {
            double inputDouble;
            double mappingDouble;
            if (validDoubleFromString(input, inputDouble)
             && validDoubleFromString(_mapping[idx + 2], mappingDouble)) {
              if (compareDoubleValues('>', mappingDouble, 0.0)) {
                double resultDouble = (100.0 / mappingDouble) * inputDouble; // Simple calculation to percentage
                int8_t decimals = 0;
                int8_t dotPos = input.indexOf('.');
                if (dotPos > -1) {
                  String decPart = input.substring(dotPos + 1);
                  decimals = decPart.length(); // Take the number of decimals to the output value
                }
                result = toString(resultDouble, decimals); // Percentage with same decimals as input
                logMapValue(input, result);
              }
            }
            break;
          }
          default:
            break;
        }
      }
    }
  }

  return result;
} // mapValue
#endif // P037_MAPPING_SUPPORT

#ifdef P037_FILTER_SUPPORT
/**
 * do we have filter values?
 */
bool P037_data_struct::hasFilters() {
  parseMappings(); // When not parsed yet
  return _maxFilter > 0;
} // hasFilters

#ifdef P037_FILTER_PER_TOPIC
String P037_data_struct::getFilterAsTopic(uint8_t topicId) {
  String result;
  result.reserve(32);
  if (hasFilters() && topicId > 0 && topicId <= VARS_PER_TASK) {
    uint8_t fltBase = (topicId - 1) * 3;
    if (_filter[fltBase + 0].length() > 0 && _filter[fltBase + 2].length() > 0) {
      result  = '/';
      result += _filter[fltBase + 0];
      result += '/';
      result += _filter[fltBase + 2];
    }
  }
  return result;
}
#endif // P037_FILTER_PER_TOPIC

#ifdef PLUGIN_037_DEBUG
void P037_data_struct::logFilterValue(const String& text, const String& key, const String& value, const String& match) {
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(50);
    log  = text;
    log += key;
    log += F(" value: ");
    log += value;
    log += F(" match: ");
    log += match;
    addLog(LOG_LEVEL_INFO, log);
  }
} // logFilterValue
#endif // PLUGIN_037_DEBUG

/**
 * checkFilters
 * Algorithm: (all comparisons are case-sensitive)
 * - If key is not found in the list, return true
 * - If key is found and value matches, return true
 * - if key is found but value doesn't match, return false
 * key can be in the list multiple times
 */
bool P037_data_struct::checkFilters(const String& key, const String& value, int8_t topicId) {
  bool result = true;

  if (key.length() > 0 && value.length() > 0) { // Ignore empty input(s)
    String filters = P037_FILTER_LIST;
    String valueData = value;
    String fltKey, fltIndex, filterData;
    double from, to, doubleValue;
    int8_t rangeSeparator;
    bool accept, matchTopicId = true;
    uint8_t fltFrom = 0, fltMax = _maxFilter;
    #ifdef P037_FILTER_PER_TOPIC
    if (topicId > 0) {
      fltFrom = (topicId - 1) * 3;
      fltMax  = topicId * 3;
    }
    #endif

    for (uint8_t flt = fltFrom; flt < fltMax; flt += 3) {
      fltKey         = _filter[flt + 0];
      rangeSeparator = _filterIdx[flt + 0];
      if (rangeSeparator > 0) {
        valueData.replace(';', ',');
        valueData = parseString(valueData, rangeSeparator);
      }
      fltKey = parseString(fltKey, 1);
      fltKey.trim();
      if (fltKey == key) {
        result = false; // Matched key, so now we are looking for matching value
        int8_t filterIndex = filters.indexOf(_filter[flt + 1]);
        filterData = _filter[flt + 2];
        parseSystemVariables(filterData, false); // Replace system variables
        switch(filterIndex) {
          case 0: // = => equals
          {
            if (filterData == valueData) {
              #ifdef PLUGIN_037_DEBUG
              logFilterValue(F("P037 filter equals key: "), key, valueData, _filter[flt + 2] + (topicId > 0 ? (String(F(" topic match: ")) + String(matchTopicId ? F("yes"): F("no"))): String(F(""))));
              #endif
              return matchTopicId; // Match, don't look any further
            }
            break;
          }
          case 1: // - => range x-y (inside) or y-x (outside)
          {
            rangeSeparator = filterData.indexOf(','); // Semicolons are replace with comma during init
            if (rangeSeparator == -1) {
              rangeSeparator = filterData.indexOf('-'); // Fall-back test for dash
            }
            if (rangeSeparator > -1) {
              accept = false;;
              if (validDoubleFromString(filterData.substring(0, rangeSeparator),  from) 
               && validDoubleFromString(filterData.substring(rangeSeparator + 1), to)
               && validDoubleFromString(valueData,                                doubleValue)) {
                if (compareDoubleValues('>' + '=', to, from)) { // Normal low - high range: between low and high
                  if (compareDoubleValues('>' + '=', doubleValue, from)
                   && compareDoubleValues('<' + '=', doubleValue, to)) {
                    accept = true;
                  }
                } else { // Alternative high - low range: outside low and high values
                  if (compareDoubleValues('>' + '=', doubleValue, from)
                   || compareDoubleValues('<' + '=', doubleValue, to)) {
                    accept = true;
                  }
                }
                if (accept) {
                  #ifdef PLUGIN_037_DEBUG
                  logFilterValue(F("P037 filter in range key: "), key, valueData, _filter[flt + 2]);
                  #endif
                  return matchTopicId; // bail out, we're done
                #ifdef PLUGIN_037_DEBUG
                } else {
                  logFilterValue(F("P037 filter NOT in range key: "), key, valueData, _filter[flt + 2]);
                #endif
                }
              }
            }
            break;
          }
          #if P037_FILTER_COUNT >= 3
          case 2: // : => Match against a semicolon-separated list (semicolon is replaced by comma during initialization)
          {
            String item;
            rangeSeparator = filterData.indexOf(',');
            if (rangeSeparator > -1 && validDoubleFromString(valueData, doubleValue)) {
              accept = false;
              do {
                item = filterData.substring(0, rangeSeparator);
                item.trim();
                filterData = filterData.substring(rangeSeparator + 1);
                filterData.trim();
                rangeSeparator = filterData.indexOf(',');
                if (rangeSeparator == -1) rangeSeparator = filterData.length(); // Last value
                if (validDoubleFromString(item, from) && compareDoubleValues('=', doubleValue, from)) {
                  accept = true;
                }
              } while (filterData.length() > 0 && !accept);
              if (accept) {
                #ifdef PLUGIN_037_DEBUG
                logFilterValue(F("P037 filter in list key: "), key, valueData, _filter[flt + 2]);
                #endif
                return matchTopicId; // bail out, we're done
              #ifdef PLUGIN_037_DEBUG
              } else {
                logFilterValue(F("P037 filter NOT in list key: "), key, valueData, _filter[flt + 2]);
              #endif
              }
            }            
            break;
          }
          #endif
          default:
          break;
        }
      }
      delay(0); // Allow some yield
    }
  }
  return result;
}
#endif  // P037_FILTER_SUPPORT

#ifdef P037_JSON_SUPPORT
/**
 * Allocate a DynamicJsonDocument and parse the message.
 * Returns true if the operation succeeded, and doc and iter can be used, when n ot successful the state of those variables is undefined.
 */
bool P037_data_struct::parseJSONMessage(const String& message) {
  bool result = false;

  if (nullptr != root && message.length() > lastJsonMessageLength) {
    cleanupJSON();
  }
  if (nullptr == root) {
    lastJsonMessageLength = message.length();
    root = new (std::nothrow) DynamicJsonDocument(lastJsonMessageLength); // Dynamic allocation
  }

  if (nullptr != root) {
    deserializeJson(*root, message.c_str());

    if (!root->isNull()) {
      result = true;
      doc    = root->as<JsonObject>();
      iter   = doc.begin();
    }
  }
  return result;
}

/**
 * Release the created DynamicJsonDocument (if it was allocated)
 */
void P037_data_struct::cleanupJSON() {
  if (nullptr != root) {
    root->clear();
    delete root;
    root = nullptr;
  }
}
#endif // P037_JSON_SUPPORT

#endif  // ifdef USES_P037
