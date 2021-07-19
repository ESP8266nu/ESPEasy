#include "_Plugin_Helper.h"
#ifdef USES_P026
//#######################################################################################################
//#################################### Plugin 026: System Info ##########################################
//#######################################################################################################


#include "src/DataStructs/ESPEasy_packed_raw_data.h"
#include "src/ESPEasyCore/ESPEasyNetwork.h"
#include "src/Globals/ESPEasyWiFiEvent.h"
#include "src/Helpers/Memory.h"
#include "ESPEasy-Globals.h"

#define PLUGIN_026
#define PLUGIN_ID_026         26
#define PLUGIN_NAME_026       "Generic - System Info"

// place sensor type selector right after the output value settings
#define P026_QUERY1_CONFIG_POS  0
#define P026_SENSOR_TYPE_INDEX  (P026_QUERY1_CONFIG_POS + VARS_PER_TASK)
#define P026_NR_OUTPUT_VALUES   getValueCountFromSensorType(static_cast<Sensor_VType>(PCONFIG(P026_SENSOR_TYPE_INDEX)))

#define P026_NR_OUTPUT_OPTIONS  14

const __FlashStringHelper * Plugin_026_valuename(uint8_t value_nr, bool displayString) {
  switch (value_nr) {
    case 0:  return displayString ? F("Uptime") : F("uptime");
    case 1:  return displayString ? F("Free RAM") : F("freeheap");
    case 2:  return displayString ? F("Wifi RSSI") : F("rssi");
    case 3:  return displayString ? F("Input VCC") : F("vcc");
    case 4:  return displayString ? F("System load") : F("load");
    case 5:  return displayString ? F("IP 1.Octet") : F("ip1");
    case 6:  return displayString ? F("IP 2.Octet") : F("ip2");
    case 7:  return displayString ? F("IP 3.Octet") : F("ip3");
    case 8:  return displayString ? F("IP 4.Octet") : F("ip4");
    case 9:  return displayString ? F("Web activity") : F("web");
    case 10: return displayString ? F("Free Stack") : F("freestack");
    case 11: return displayString ? F("None") : F("");
    case 12: return displayString ? F("WiFi TX pwr") : F("txpwr");
    case 13: return displayString ? F("Free 2nd Heap") : F("free2ndheap");
    default:
      break;
  }
  return F("");
}

boolean Plugin_026(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number       = PLUGIN_ID_026;
      Device[deviceCount].VType          = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].ValueCount     = 4;
      Device[deviceCount].SendDataOption = true;
      Device[deviceCount].TimerOption    = true;
      Device[deviceCount].FormulaOption  = true;
      Device[deviceCount].OutputDataType = Output_Data_type_t::Simple;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_026);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
        if (i < P026_NR_OUTPUT_VALUES) {
          const uint8_t pconfigIndex = i + P026_QUERY1_CONFIG_POS;
          uint8_t choice             = PCONFIG(pconfigIndex);
          safe_strncpy(
            ExtraTaskSettings.TaskDeviceValueNames[i],
            Plugin_026_valuename(choice, false),
            sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
        } else {
          ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
        }
      }
      break;
    }

    case PLUGIN_GET_DEVICEVALUECOUNT:
    {
      event->Par1 = P026_NR_OUTPUT_VALUES;
      success = true;
      break;
    }

    case PLUGIN_GET_DEVICEVTYPE:
    {
      event->sensorType = static_cast<Sensor_VType>(PCONFIG(P026_SENSOR_TYPE_INDEX));
      event->idx = P026_SENSOR_TYPE_INDEX;
      success = true;
      break;
    }


    case PLUGIN_SET_DEFAULTS:
    {
      PCONFIG(0) = 0;    // "Uptime"

      for (uint8_t i = 1; i < VARS_PER_TASK; ++i) {
        PCONFIG(i) = 11; // "None"
      }
      PCONFIG(P026_SENSOR_TYPE_INDEX) = static_cast<uint8_t>(Sensor_VType::SENSOR_TYPE_QUAD);
      success                         = true;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      const __FlashStringHelper * options[P026_NR_OUTPUT_OPTIONS];
      int indices[P026_NR_OUTPUT_OPTIONS];

      int index = 0;
      for (uint8_t option = 0; option < P026_NR_OUTPUT_OPTIONS; ++option) {
        if (option != 11) {
          options[index] = Plugin_026_valuename(option, true);
          indices[index] = option;
          ++index;
        }
      }
      // Work around to get the "none" at the end.
      options[index] = Plugin_026_valuename(11, true);
      indices[index] = 11;

      for (uint8_t i = 0; i < P026_NR_OUTPUT_VALUES; ++i) {
        const uint8_t pconfigIndex = i + P026_QUERY1_CONFIG_POS;
        sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, P026_NR_OUTPUT_OPTIONS, options, indices);
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      // Save output selector parameters.
      for (uint8_t i = 0; i < P026_NR_OUTPUT_VALUES; ++i) {
        const uint8_t pconfigIndex = i + P026_QUERY1_CONFIG_POS;
        const uint8_t choice       = PCONFIG(pconfigIndex);
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, Plugin_026_valuename(choice, false));
      }
      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      for (int i = 0; i < P026_NR_OUTPUT_VALUES; ++i) {
        UserVar[event->BaseVarIndex + i] = P026_get_value(PCONFIG(i));
      }

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log;
        log.reserve(7 * (P026_NR_OUTPUT_VALUES + 1));
        log = F("SYS  : ");

        for (int i = 0; i < P026_NR_OUTPUT_VALUES; ++i) {
          if (i != 0) {
            log += ',';
          }
          log += formatUserVarNoCheck(event->TaskIndex, i);
        }
        addLog(LOG_LEVEL_INFO, log);
      }
      success = true;
      break;
    }
#ifdef USES_PACKED_RAW_DATA
   case PLUGIN_GET_PACKED_RAW_DATA:
    {
      // Matching JS code:
      // return decode(bytes, 
      //  [header, uint24, uint24, int8, vcc, pct_8, uint8, uint8, uint8, uint8, uint24, uint16],
      //  ['header', 'uptime', 'freeheap', 'rssi', 'vcc', 'load', 'ip1', 'ip2', 'ip3', 'ip4', 'web', 'freestack']);
      int index = 0;
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint24);  // uptime
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint24);  // freeheap
      string += LoRa_addFloat(P026_get_value(index++), PackedData_int8);  // rssi
      string += LoRa_addFloat(P026_get_value(index++), PackedData_vcc);   // vcc
      string += LoRa_addFloat(P026_get_value(index++), PackedData_pct_8); // load
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint8);   // ip1
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint8);   // ip2
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint8);   // ip3
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint8);   // ip4
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint24);  // web
      string += LoRa_addInt(P026_get_value(index++), PackedData_uint16);  // freestack
      event->Par1 = index; // valuecount
      success = true;
      break;
    }
#endif // USES_PACKED_RAW_DATA
  }
  return success;
}

float P026_get_value(int type)
{
  float value = 0;

  switch (type)
  {
    case 0:
    {
      value = getUptimeMinutes();
      break;
    }
    case 1:
    {
      value = FreeMem();
      break;
    }
    case 2:
    {
      value = WiFi.RSSI();
      break;
    }
    case 3:
    {
# if FEATURE_ADC_VCC
      value = vcc;
# else // if FEATURE_ADC_VCC
      value = -1.0f;
# endif // if FEATURE_ADC_VCC
      break;
    }
    case 4:
    {
      value = getCPUload();
      break;
    }
    case 5:
    case 6:
    case 7:
    case 8:
    {
      value = NetworkLocalIP()[type - 5];
      break;
    }
    case 9:
    {
      value = timePassedSince(lastWeb) / 1000; // respond in seconds
      break;
    }
    case 10:
    {
      value = getCurrentFreeStack();
      break;
    }
    case 12:
    {
      value = WiFiEventData.wifi_TX_pwr;
      break;
    }
    case 13:
    {
      #ifdef USE_SECOND_HEAP
      value = FreeMem2ndHeap();
      #endif
      break;
    }


  }
  return value;
}

#endif // USES_P026
