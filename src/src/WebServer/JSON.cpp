#include "../WebServer/JSON.h"

#include "../WebServer/WebServer.h"
#include "../WebServer/JSON.h"
#include "../WebServer/Markup_Forms.h"

#include "../Globals/Nodes.h"
#include "../Globals/Device.h"
#include "../Globals/Plugins.h"

#include "../Helpers/ESPEasyStatistics.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringProvider.h"

#include "../../_Plugin_Helper.h"
#include "../../ESPEasy-Globals.h"



// ********************************************************************************
// Web Interface get CSV value from task
// ********************************************************************************
void handle_csvval()
{
  String htmlData;
  htmlData.reserve(25); // Reserve for error message
  const int printHeader = getFormItemInt(F("header"), 1);
  bool printHeaderValid = true;
  if (printHeader != 1 && printHeader != 0)
  {
    htmlData = F("ERROR: Header not valid!\n");
    printHeaderValid = false;
  }

  const taskIndex_t taskNr    = getFormItemInt(F("tasknr"), INVALID_TASK_INDEX);
  const bool taskValid = validTaskIndex(taskNr);
  if (!taskValid)
  {
    htmlData = F("ERROR: TaskNr not valid!\n");
  }

  const int INVALID_VALUE_NUM = INVALID_TASKVAR_INDEX + 1;
  const taskVarIndex_t valNr    = getFormItemInt(F("valnr"), INVALID_VALUE_NUM);
  bool valueNumberValid = true;
  if (valNr != INVALID_VALUE_NUM && !validTaskVarIndex(valNr))
  {
    htmlData = F("ERROR: ValueId not valid!\n");
    valueNumberValid = false;
  }

  TXBuffer.startJsonStream();
  if (taskValid && valueNumberValid && printHeaderValid)
  {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(taskNr);

    if (validDeviceIndex(DeviceIndex))
    {
      LoadTaskSettings(taskNr);
      const uint8_t taskValCount = getValueCountForTask(taskNr);
      uint16_t stringReserveSize = (valNr == INVALID_VALUE_NUM ? 1 : taskValCount) * 24;
      htmlData.reserve(stringReserveSize);

      if (printHeader)
      {
        for (uint8_t x = 0; x < taskValCount; x++)
        {
          if (valNr == INVALID_VALUE_NUM || valNr == x)
          {
            htmlData += String(ExtraTaskSettings.TaskDeviceValueNames[x]);
            if (x != taskValCount - 1)
            {
              htmlData += ';';
            }
          }
        }
        htmlData += '\n';
        addHtml(htmlData);
        htmlData = "";
      }

      for (uint8_t x = 0; x < taskValCount; x++)
      {
        if ((valNr == INVALID_VALUE_NUM) || (valNr == x))
        {
          htmlData += formatUserVarNoCheck(taskNr, x);

          if (x != taskValCount - 1)
          {
            htmlData += ';';
          }
        }
      }
      htmlData += '\n';
    }
  }
  addHtml(htmlData);
  TXBuffer.endStream();
}

// ********************************************************************************
// Web Interface JSON page (no password!)
// ********************************************************************************
void handle_json()
{
  const taskIndex_t taskNr    = getFormItemInt(F("tasknr"), INVALID_TASK_INDEX);
  const bool showSpecificTask = validTaskIndex(taskNr);
  bool showSystem             = true;
  bool showWifi               = true;

  #ifdef HAS_ETHERNET
  bool showEthernet = true;
  #endif // ifdef HAS_ETHERNET
  bool showDataAcquisition = true;
  bool showTaskDetails     = true;
  bool showNodes           = true;
  {
    const String view = webArg("view");

    if (!view.isEmpty()) {
      if (view == F("sensorupdate")) {
        showSystem = false;
        showWifi   = false;
        #ifdef HAS_ETHERNET
        showEthernet = false;
        #endif // ifdef HAS_ETHERNET
        showDataAcquisition = false;
        showTaskDetails     = false;
        showNodes           = false;
      }
    }
  }

  TXBuffer.startJsonStream();

  if (!showSpecificTask)
  {
    addHtml('{');

    if (showSystem) {
      addHtml(F("\"System\":{\n"));

      if (wdcounter > 0)
      {
        stream_next_json_object_value(LabelType::LOAD_PCT);
        stream_next_json_object_value(LabelType::LOOP_COUNT);
      }

      static const LabelType::Enum labels[] PROGMEM =
      {
        LabelType::BUILD_DESC,
        LabelType::GIT_BUILD,
        LabelType::SYSTEM_LIBRARIES,
        LabelType::PLUGIN_COUNT,
        LabelType::PLUGIN_DESCRIPTION,
        LabelType::LOCAL_TIME,
        LabelType::ISNTP,
        LabelType::UNIT_NR,
        LabelType::UNIT_NAME,
        LabelType::UPTIME,
        LabelType::UPTIME_MS,
        LabelType::BOOT_TYPE,
        LabelType::RESET_REASON,
        LabelType::CPU_ECO_MODE,

        #ifdef CORE_POST_2_5_0
        LabelType::HEAP_MAX_FREE_BLOCK,
        LabelType::HEAP_FRAGMENTATION,
        #endif // ifdef CORE_POST_2_5_0
        LabelType::FREE_MEM,
      #ifdef USE_SECOND_HEAP
        LabelType::FREE_HEAP_IRAM,
      #endif
        LabelType::SUNRISE,
        LabelType::SUNSET,
        LabelType::TIMEZONE_OFFSET,
        LabelType::LATITUDE,
        LabelType::LONGITUDE,


        LabelType::MAX_LABEL
      };

      stream_json_object_values(labels, true);
      addHtml(F(",\n"));
    }

    if (showWifi) {
      addHtml(F("\"WiFi\":{\n"));
      static const LabelType::Enum labels[] PROGMEM =
      {
        LabelType::HOST_NAME,
      #ifdef FEATURE_MDNS
        LabelType::M_DNS,
      #endif // ifdef FEATURE_MDNS
        LabelType::IP_CONFIG,
        LabelType::IP_ADDRESS,
        LabelType::IP_SUBNET,
        LabelType::GATEWAY,
        LabelType::STA_MAC,
        LabelType::DNS_1,
        LabelType::DNS_2,
        LabelType::SSID,
        LabelType::BSSID,
        LabelType::CHANNEL,
        LabelType::ENCRYPTION_TYPE_STA,
        LabelType::CONNECTED_MSEC,
        LabelType::LAST_DISCONNECT_REASON,
        LabelType::LAST_DISC_REASON_STR,
        LabelType::NUMBER_RECONNECTS,
        LabelType::WIFI_STORED_SSID1,
        LabelType::WIFI_STORED_SSID2,
        LabelType::FORCE_WIFI_BG,
        LabelType::RESTART_WIFI_LOST_CONN,
#ifdef ESP8266
        LabelType::FORCE_WIFI_NOSLEEP,
#endif // ifdef ESP8266
#ifdef SUPPORT_ARP
        LabelType::PERIODICAL_GRAT_ARP,
#endif // ifdef SUPPORT_ARP
        LabelType::CONNECTION_FAIL_THRESH,
        LabelType::WIFI_TX_MAX_PWR,
        LabelType::WIFI_CUR_TX_PWR,
        LabelType::WIFI_SENS_MARGIN,
        LabelType::WIFI_SEND_AT_MAX_TX_PWR,
        LabelType::WIFI_NR_EXTRA_SCANS,
        LabelType::WIFI_PERIODICAL_SCAN,
        LabelType::WIFI_USE_LAST_CONN_FROM_RTC,
        LabelType::WIFI_RSSI,


        LabelType::MAX_LABEL
      };

      stream_json_object_values(labels, true);

      // TODO: PKR: Add ETH Objects
      addHtml(F(",\n"));
    }

    #ifdef HAS_ETHERNET

    if (showEthernet) {
      addHtml(F("\"Ethernet\":{\n"));
      static const LabelType::Enum labels[] PROGMEM =
      {
        LabelType::ETH_WIFI_MODE,
        LabelType::ETH_CONNECTED,
        LabelType::ETH_DUPLEX,
        LabelType::ETH_SPEED,
        LabelType::ETH_STATE,
        LabelType::ETH_SPEED_STATE,


        LabelType::MAX_LABEL
      };

      stream_json_object_values(labels, true);
      addHtml(F(",\n"));
    }
    #endif // ifdef HAS_ETHERNET

    if (showNodes) {
      bool comma_between = false;

      for (NodesMap::iterator it = Nodes.begin(); it != Nodes.end(); ++it)
      {
        if (it->second.ip[0] != 0)
        {
          if (comma_between) {
            addHtml(',');
          } else {
            comma_between = true;
            addHtml(F("\"nodes\":[\n")); // open json array if >0 nodes
          }

          addHtml('{');
          stream_next_json_object_value(F("nr"), String(it->first));
          stream_next_json_object_value(F("name"),
                                        (it->first != Settings.Unit) ? it->second.nodeName : Settings.Name);

          if (it->second.build) {
            stream_next_json_object_value(F("build"), String(it->second.build));
          }

          if (it->second.nodeType) {
            String platform = getNodeTypeDisplayString(it->second.nodeType);

            if (platform.length() > 0) {
              stream_next_json_object_value(F("platform"), platform);
            }
          }
          stream_next_json_object_value(F("ip"), it->second.ip.toString());
          stream_last_json_object_value(F("age"), String(it->second.age));
        } // if node info exists
      }   // for loop

      if (comma_between) {
        addHtml(F("],\n")); // close array if >0 nodes
      }
    }
  }

  taskIndex_t firstTaskIndex = 0;
  taskIndex_t lastTaskIndex  = TASKS_MAX - 1;

  if (showSpecificTask)
  {
    firstTaskIndex = taskNr - 1;
    lastTaskIndex  = taskNr - 1;
  }
  taskIndex_t lastActiveTaskIndex = 0;

  for (taskIndex_t TaskIndex = firstTaskIndex; TaskIndex <= lastTaskIndex; TaskIndex++) {
    if (validPluginID_fullcheck(Settings.TaskDeviceNumber[TaskIndex])) {
      lastActiveTaskIndex = TaskIndex;
    }
  }

  if (!showSpecificTask) {
    addHtml(F("\"Sensors\":[\n"));
  }

  // Keep track of the lowest reported TTL and use that as refresh interval.
  unsigned long lowest_ttl_json = 60;

  for (taskIndex_t TaskIndex = firstTaskIndex; TaskIndex <= lastActiveTaskIndex && validTaskIndex(TaskIndex); TaskIndex++)
  {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(TaskIndex);

    if (validDeviceIndex(DeviceIndex))
    {
      const unsigned long taskInterval = Settings.TaskDeviceTimer[TaskIndex];
      LoadTaskSettings(TaskIndex);
      addHtml(F("{\n"));

      unsigned long ttl_json = 60; // Default value

      // For simplicity, do the optional values first.
      const uint8_t valueCount = getValueCountForTask(TaskIndex);

      if (valueCount != 0) {
        if ((taskInterval > 0) && Settings.TaskDeviceEnabled[TaskIndex]) {
          ttl_json = taskInterval;

          if (ttl_json < lowest_ttl_json) {
            lowest_ttl_json = ttl_json;
          }
        }
        addHtml(F("\"TaskValues\": [\n"));

        for (uint8_t x = 0; x < valueCount; x++)
        {
          addHtml('{');
          const String value = formatUserVarNoCheck(TaskIndex, x);
          uint8_t nrDecimals    = ExtraTaskSettings.TaskDeviceValueDecimals[x];

          if (mustConsiderAsString(value)) {
            // Flag as not to treat as a float
            nrDecimals = 255;
          }
          stream_next_json_object_value(F("ValueNumber"), String(x + 1));
          stream_next_json_object_value(F("Name"),        String(ExtraTaskSettings.TaskDeviceValueNames[x]));
          stream_next_json_object_value(F("NrDecimals"),  String(nrDecimals));
          stream_last_json_object_value(F("Value"), value);

          if (x < (valueCount - 1)) {
            addHtml(F(",\n"));
          }
        }
        addHtml(F("],\n"));
      }

      if (showSpecificTask) {
        stream_next_json_object_value(F("TTL"), String(ttl_json * 1000));
      }

      if (showDataAcquisition) {
        addHtml(F("\"DataAcquisition\": [\n"));

        for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++)
        {
          addHtml('{');
          stream_next_json_object_value(F("Controller"), String(x + 1));
          stream_next_json_object_value(F("IDX"),        String(Settings.TaskDeviceID[x][TaskIndex]));
          stream_last_json_object_value(F("Enabled"), jsonBool(Settings.TaskDeviceSendData[x][TaskIndex]));

          if (x < (CONTROLLER_MAX - 1)) {
            addHtml(F(",\n"));
          }
        }
        addHtml(F("],\n"));
      }

      if (showTaskDetails) {
        stream_next_json_object_value(F("TaskInterval"),     String(taskInterval));
        stream_next_json_object_value(F("Type"),             getPluginNameFromDeviceIndex(DeviceIndex));
        stream_next_json_object_value(F("TaskName"),         String(ExtraTaskSettings.TaskDeviceName));
        stream_next_json_object_value(F("TaskDeviceNumber"), String(Settings.TaskDeviceNumber[TaskIndex]));
#ifdef FEATURE_I2CMULTIPLEXER
        if (Device[DeviceIndex].Type == DEVICE_TYPE_I2C && isI2CMultiplexerEnabled()) {
          int8_t channel = Settings.I2C_Multiplexer_Channel[TaskIndex];
          if (bitRead(Settings.I2C_Flags[TaskIndex], I2C_FLAGS_MUX_MULTICHANNEL)) {
            addHtml(F("\"I2CBus\" : ["));
            uint8_t b = 0;
            for (uint8_t c = 0; c < I2CMultiplexerMaxChannels(); c++) {
              if (bitRead(channel, c)) {
                if (b > 0) { addHtml(F(",\n")); }
                b++;
                String i2cChannel = F("\"Multiplexer channel ");
                i2cChannel += String(c);
                i2cChannel += F("\"");
                addHtml(i2cChannel);
              }
            }
            addHtml(F("],\n"));
          } else {
            if (channel == -1){
              stream_next_json_object_value(F("I2Cbus"),       F("Standard I2C bus"));
            } else {
              String i2cChannel = F("Multiplexer channel ");
              i2cChannel += String(channel);
              stream_next_json_object_value(F("I2Cbus"),       i2cChannel);
            }
          }
        }
#endif
      }
      stream_next_json_object_value(F("TaskEnabled"), jsonBool(Settings.TaskDeviceEnabled[TaskIndex]));
      stream_last_json_object_value(F("TaskNumber"), String(TaskIndex + 1));

      if (TaskIndex != lastActiveTaskIndex) {
        addHtml(',');
      }
      addHtml('\n');
    }
  }

  if (!showSpecificTask) {
    addHtml(F("],\n"));
    stream_last_json_object_value(F("TTL"), String(lowest_ttl_json * 1000));
  }

  TXBuffer.endStream();
}

// ********************************************************************************
// JSON formatted timing statistics
// ********************************************************************************

#ifdef WEBSERVER_NEW_UI
void handle_timingstats_json() {
  TXBuffer.startJsonStream();
  json_init();
  json_open();
  # ifdef USES_TIMING_STATS
  jsonStatistics(false);
  # endif // ifdef USES_TIMING_STATS
  json_close();
  TXBuffer.endStream();
}

#endif // WEBSERVER_NEW_UI

#ifdef WEBSERVER_NEW_UI
void handle_nodes_list_json() {
  if (!isLoggedIn()) { return; }
  TXBuffer.startJsonStream();
  json_init();
  json_open(true);

  for (NodesMap::iterator it = Nodes.begin(); it != Nodes.end(); ++it)
  {
    if (it->second.ip[0] != 0)
    {
      json_open();
      bool isThisUnit = it->first == Settings.Unit;

      if (isThisUnit) {
        json_number(F("thisunit"), String(1));
      }

      json_number(F("first"), String(it->first));
      json_prop(F("name"), isThisUnit ? Settings.Name : it->second.nodeName);

      if (it->second.build) { json_prop(F("build"), String(it->second.build)); }
      json_prop(F("type"), getNodeTypeDisplayString(it->second.nodeType));
      json_prop(F("ip"),   it->second.ip.toString());
      json_number(F("age"), String(it->second.age));
      json_close();
    }
  }
  json_close(true);
  TXBuffer.endStream();
}

void handle_buildinfo() {
  if (!isLoggedIn()) { return; }
  TXBuffer.startJsonStream();
  json_init();
  json_open();
  {
    json_open(true, F("plugins"));

    for (deviceIndex_t x = 0; x <= deviceCount; x++) {
      if (validPluginID(DeviceIndex_to_Plugin_id[x])) {
        json_open();
        json_number(F("id"), String(DeviceIndex_to_Plugin_id[x]));
        json_prop(F("name"), getPluginNameFromDeviceIndex(x));
        json_close();
      }
    }
    json_close(true);
  }
  {
    json_open(true, F("controllers"));

    for (protocolIndex_t x = 0; x < CPLUGIN_MAX; x++) {
      if (getCPluginID_from_ProtocolIndex(x) != INVALID_C_PLUGIN_ID) {
        json_open();
        json_number(F("id"), String(x + 1));
        json_prop(F("name"), getCPluginNameFromProtocolIndex(x));
        json_close();
      }
    }
    json_close(true);
  }
  {
    json_open(true, F("notifications"));

    for (uint8_t x = 0; x < NPLUGIN_MAX; x++) {
      if (validNPluginID(NPlugin_id[x])) {
        json_open();
        json_number(F("id"), String(x + 1));
        json_prop(F("name"), getNPluginNameFromNotifierIndex(x));
        json_close();
      }
    }
    json_close(true);
  }
  json_prop(LabelType::BUILD_DESC);
  json_prop(LabelType::GIT_BUILD);
  json_prop(LabelType::SYSTEM_LIBRARIES);
  json_prop(LabelType::PLUGIN_COUNT);
  json_prop(LabelType::PLUGIN_DESCRIPTION);
  json_close();
  TXBuffer.endStream();
}

#endif // WEBSERVER_NEW_UI


/*********************************************************************************************\
   Streaming versions directly to TXBuffer
\*********************************************************************************************/
void stream_to_json_value(const String& value) {
  NumericalType detectedType;
  bool isNum  = isNumerical(value, detectedType);
  bool isBool = (Settings.JSONBoolWithoutQuotes() && ((value.equalsIgnoreCase(F("true")) || value.equalsIgnoreCase(F("false")))));

  if (!isBool && ((value.isEmpty()) || !isNum || mustConsiderAsString(detectedType))) {
    // Either empty, not a numerical or a BIN/HEX notation.
    addHtml('\"');
    if ((value.indexOf('\n') != -1) || (value.indexOf('\r') != -1) || (value.indexOf('"') != -1)) {
      // Must replace characters, so make a deepcopy
      String tmpValue(value);
      tmpValue.replace('\n', '^');
      tmpValue.replace('\r', '^');
      tmpValue.replace('"',  '\'');
      addHtml(tmpValue);
    } else {
      addHtml(value);
    }
    addHtml('\"');
  } else {
    addHtml(value);
  }
}

void stream_to_json_object_value(const __FlashStringHelper *  object, const String& value) {
  addHtml('\"');
  addHtml(object);
  addHtml(F("\":"));
  stream_to_json_value(value);
}

void stream_to_json_object_value(const String& object, const String& value) {
  addHtml('\"');
  addHtml(object);
  addHtml(F("\":"));
  stream_to_json_value(value);
}

String jsonBool(bool value) {
  return boolToString(value);
}

// Add JSON formatted data directly to the TXbuffer, including a trailing comma.
void stream_next_json_object_value(const __FlashStringHelper * object, const String& value) {
  stream_to_json_object_value(object, value);
  addHtml(F(",\n"));
}

void stream_next_json_object_value(const String& object, const String& value) {
  stream_to_json_object_value(object, value);
  addHtml(F(",\n"));
}

// Add JSON formatted data directly to the TXbuffer, including a closing '}'
void stream_last_json_object_value(const __FlashStringHelper * object, const String& value) {
  stream_to_json_object_value(object, value);
  addHtml(F("\n}"));
}

void stream_last_json_object_value(const String& object, const String& value) {
  stream_to_json_object_value(object, value);
  addHtml(F("\n}"));
}

void stream_json_object_values(const LabelType::Enum labels[], bool markLast)
{
  size_t i = 0;

  while (true) {
    const LabelType::Enum cur  = static_cast<const LabelType::Enum>(pgm_read_byte(labels + i));
    const LabelType::Enum next = static_cast<const LabelType::Enum>(pgm_read_byte(labels + i + 1));
    const bool nextIsLast      = next == LabelType::MAX_LABEL;

    if (markLast && nextIsLast) {
      stream_last_json_object_value(cur);
    } else {
      stream_next_json_object_value(cur);
    }

    if (nextIsLast) {
      return;
    }
    ++i;
  }
}

void stream_next_json_object_value(LabelType::Enum label) {
  stream_next_json_object_value(getLabel(label), getValue(label));
}

void stream_last_json_object_value(LabelType::Enum label) {
  stream_last_json_object_value(getLabel(label), getValue(label));
}
