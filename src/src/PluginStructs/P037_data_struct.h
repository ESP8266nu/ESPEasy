#ifndef PLUGINSTRUCTS_P037_DATA_STRUCT_H
#define PLUGINSTRUCTS_P037_DATA_STRUCT_H

#include "../../_Plugin_Helper.h"
#ifdef USES_P037

# include "../CustomBuild/StorageLayout.h"
# include "src/Globals/EventQueue.h"
# include "src/Globals/MQTT.h"
# include "src/Globals/CPlugins.h"
# include "src/Globals/Plugins.h"
# include "src/Helpers/ESPEasy_Storage.h"
# include "src/Helpers/Misc.h"
# include "src/Helpers/StringParser.h"

# include <ArduinoJson.h>

// # define PLUGIN_037_DEBUG     // Additional debugging information

# define P037_MAPPING_SUPPORT // Enable Value mapping support
# define P037_FILTER_SUPPORT  // Enable filtering support
# define P037_JSON_SUPPORT    // Enable Json support

// # define P037_OVERRIDE        // When defined, do not limit features because of LIMIT_BUILD_SIZE

# if defined(LIMIT_BUILD_SIZE) && !defined(P037_OVERRIDE) // Leave out the fancy stuff if available flash is tight
  #  ifdef PLUGIN_037_DEBUG
    #   undef PLUGIN_037_DEBUG
  #  endif // ifdef PLUGIN_037_DEBUG
  #  ifdef P037_MAPPING_SUPPORT
    #   undef P037_MAPPING_SUPPORT
  #  endif // ifdef P037_MAPPING_SUPPORT
  #  if defined(FEATURE_ADC_VCC) && defined(P037_FILTER_SUPPORT)
    #   undef P037_FILTER_SUPPORT
  #  endif // if defined(FEATURE_ADC_VCC) && defined(P037_FILTER_SUPPORT)

// #ifdef P037_JSON_SUPPORT
//   #undef P037_JSON_SUPPORT
// #endif
# endif // ifdef LIMIT_BUILD_SIZE

# define P037_MAX_MAPPINGS  25
# define P037_MAX_FILTERS   VARS_PER_TASK                              // When VARS_PER_TASK is used, the filter is 1:1 mapped to a MQTT
                                                                       // topic
# define P037_EXTRA_VALUES  5                                          // The number of extra, empty, values to show when adding mappings
                                                                       // (or filters if not 1:1 with topics is used)

# if defined(P037_FILTER_SUPPORT) && P037_MAX_FILTERS == VARS_PER_TASK // Only 1 filter per topic
  #  ifndef P037_FILTER_PER_TOPIC
    #   define P037_FILTER_PER_TOPIC
  #  endif // ifndef P037_FILTER_PER_TOPIC
# endif // if defined(P037_FILTER_SUPPORT) && P037_MAX_FILTERS == VARS_PER_TASK

# define P037_OPERAND_COUNT 2
# define P037_OPERAND_LIST  F("=%")

# define P037_FILTER_COUNT  3
# define P037_FILTER_LIST   F("=-:") // Length should at least match P037_FILTER_COUNT

// Data structure
struct P037_data_struct : public PluginTaskData_base
{
  P037_data_struct(taskIndex_t taskIndex);
  ~P037_data_struct();

  bool webform_load(
  # ifdef P037_MAPPING_SUPPORT
    bool mappingEnabled
  # endif // ifdef P037_MAPPING_SUPPORT
  # if defined(P037_MAPPING_SUPPORT) && defined(P037_FILTER_SUPPORT)
    ,
  # endif // if defined(P037_MAPPING_SUPPORT) && defined(P037_FILTER_SUPPORT)
  # ifdef P037_FILTER_SUPPORT
    bool filterEnabled
  # endif // ifdef P037_FILTER_SUPPORT
  # if (defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)) && defined(P037_JSON_SUPPORT)
    ,
  # endif // if (defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)) && defined(P037_JSON_SUPPORT)
  # ifdef P037_JSON_SUPPORT
    bool jsonEnabled
  # endif // ifdef P037_JSON_SUPPORT
    );
  bool webform_save(
  # ifdef P037_FILTER_SUPPORT
    bool filterEnabled
  # endif // ifdef P037_FILTER_SUPPORT
  # if defined(P037_FILTER_SUPPORT) && defined(P037_JSON_SUPPORT)
    ,
  # endif // if defined(P037_FILTER_SUPPORT) && defined(P037_JSON_SUPPORT)
  # ifdef P037_JSON_SUPPORT
    bool jsonEnabled
  # endif // ifdef P037_JSON_SUPPORT
    );
  # ifdef P037_MAPPING_SUPPORT
  String mapValue(const String& input,
                  const String& attribute);
  void   logMapValue(const String& input,
                     const String& result);
  # endif // P037_MAPPING_SUPPORT
  # ifdef P037_FILTER_SUPPORT
  bool hasFilters();
  bool checkFilters(const String& key,
                    const String& value,
                    int8_t        topicId);
  #  ifdef P037_FILTER_PER_TOPIC
  String getFilterAsTopic(uint8_t topicId);
  #  endif // P037_FILTER_PER_TOPIC
  #  ifdef PLUGIN_037_DEBUG
  void   logFilterValue(const String& text,
                        const String& key,
                        const String& value,
                        const String& match);
  #  endif // PLUGIN_037_DEBUG
  # endif  // P037_FILTER_SUPPORT

  // The settings structures
  // The stuff we want to save between settings
  struct tP037_StoredSettings_struct {
    char deviceTemplate[VARS_PER_TASK][41];              // variable for saving the subscription topics, leave as first element for backward
                                                         // compatibility
    char jsonAttributes[VARS_PER_TASK][21];              // variable for saving the json attribute to use
  # if defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)
  #  ifdef P037_FILTER_PER_TOPIC
  #   define MAP_FILTER_SIZE (DAT_TASKS_CUSTOM_SIZE / 2) // Use half of available size
  #  else // ifdef P037_FILTER_PER_TOPIC
  #   define MAP_FILTER_SIZE DAT_TASKS_CUSTOM_SIZE       // Use entire space
  #  endif // ifdef P037_FILTER_PER_TOPIC

    // All saved in a single string for most efficient storage
    char valueMappings[MAP_FILTER_SIZE - ((VARS_PER_TASK * 41) + (VARS_PER_TASK * 21))]; // name=num,name2=num,name3%num,... mappings + | +
                                                                                         // name=value,name2=value2,... Json filters
  # endif // if defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)
  };

  // Stored settings data:
  tP037_StoredSettings_struct StoredSettings;

  # ifdef P037_JSON_SUPPORT
  bool parseJSONMessage(const String& message);
  void cleanupJSON();
  JsonObject           doc;
  JsonObject::iterator iter;
  # endif // P037_JSON_SUPPORT

  std::map<uint8_t, String>deviceTemplate;
  std::map<uint8_t, String>jsonAttributes;

private:

  // Private methods and vars
  bool loadSettings();

  # if defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)
  void parseMappings();
  # endif // if defined(P037_MAPPING_SUPPORT) || defined(P037_FILTER_SUPPORT)
  taskIndex_t _taskIndex = TASKS_MAX;
  # ifdef P037_MAPPING_SUPPORT
  std::map<uint8_t, String>_mapping;
  int8_t                   _maxIdx = -1;
  # endif // ifdef P037_MAPPING_SUPPORT
  # ifdef P037_FILTER_SUPPORT
  std::map<uint8_t, String>  _filter;
  std::map<uint8_t, uint16_t>_filterIdx;
  int8_t                     _maxFilter = -1;
  # endif // ifdef P037_FILTER_SUPPORT
  # ifdef P037_JSON_SUPPORT
  DynamicJsonDocument *root                  = nullptr;
  uint16_t             lastJsonMessageLength = 0;
  # endif // P037_JSON_SUPPORT
};

#endif    // ifdef USED_P037
#endif // ifndef PLUGINSTRUCTS_P037_DATA_STRUCT_H
