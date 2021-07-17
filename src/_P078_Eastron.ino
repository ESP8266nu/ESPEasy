#include "_Plugin_Helper.h"

#ifdef USES_P078

// #######################################################################################################
// ############## Plugin 078: SDM120/SDM120CT/220/230/630/72D/DDM18SD Eastron Energy Meter ###############
// #######################################################################################################

/*
   Plugin written by: Sergio Faustino sjfaustino__AT__gmail.com

   This plugin reads available values of an Eastron SDM120C SDM120/SDM120CT/220/230/630/72D & also DDM18SD.
 */

# define PLUGIN_078
# define PLUGIN_ID_078         78
# define PLUGIN_NAME_078       "Energy (AC) - Eastron SDM120/SDM120CT/220/230/630/72D/DDM18SD [TESTING]"

# define P078_DEV_ID          PCONFIG(0)
# define P078_DEV_ID_LABEL    PCONFIG_LABEL(0)
# define P078_MODEL           PCONFIG(1)
# define P078_MODEL_LABEL     PCONFIG_LABEL(1)
# define P078_BAUDRATE        PCONFIG(2)
# define P078_BAUDRATE_LABEL  PCONFIG_LABEL(2)
# define P078_QUERY1          PCONFIG(3)
# define P078_QUERY2          PCONFIG(4)
# define P078_QUERY3          PCONFIG(5)
# define P078_QUERY4          PCONFIG(6)
# define P078_DEPIN           CONFIG_PIN3

# define P078_DEV_ID_DFLT     1
# define P078_MODEL_DFLT      0 // SDM120C
# define P078_BAUDRATE_DFLT   1 // 9600 baud
# define P078_QUERY1_DFLT     0 // Voltage (V)
# define P078_QUERY2_DFLT     1 // Current (A)
# define P078_QUERY3_DFLT     2 // Power (W)
# define P078_QUERY4_DFLT     5 // Power Factor (cos-phi)


# define P078_NR_OUTPUT_VALUES                            4
# define P078_NR_OUTPUT_OPTIONS_SDM220_SDM120CT_SDM120    14
# define P078_NR_OUTPUT_OPTIONS_SDM230                    24
# define P078_NR_OUTPUT_OPTIONS_SDM630                    86
# define P078_NR_OUTPUT_OPTIONS_SDM72D                    9
# define P078_NR_OUTPUT_OPTIONS_DDM18SD                   7

# define P078_QUERY1_CONFIG_POS  3

# include <ESPeasySerial.h>
# include <SDM.h> // Requires SDM library from Reaper7 - https://github.com/reaper7/SDM_Energy_Meter/

// These pointers may be used among multiple instances of the same plugin,
// as long as the same serial settings are used.
ESPeasySerial *Plugin_078_SoftSerial = NULL;
SDM *Plugin_078_SDM                  = NULL;
boolean Plugin_078_init              = false;


// Forward declaration helper functions
const __FlashStringHelper* p078_getQueryString(uint8_t query,
                                               uint8_t model);
const __FlashStringHelper* p078_getQueryValueString(uint8_t query,
                                                    uint8_t model);
unsigned int               p078_getRegister(uint8_t query,
                                            uint8_t model);
float                      p078_readVal(uint8_t      query,
                                        uint8_t      node,
                                        unsigned int model);


boolean Plugin_078(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_078;
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL_PLUS1; // connected through 3 datapins
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = P078_NR_OUTPUT_VALUES;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_078);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
          if ( i < P078_NR_OUTPUT_VALUES) {
            uint8_t choice = PCONFIG(i + P078_QUERY1_CONFIG_POS);
            safe_strncpy(
              ExtraTaskSettings.TaskDeviceValueNames[i],
              p078_getQueryValueString(choice, P078_MODEL),
              sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
          } else {
            ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
          }
        }
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      serialHelper_getGpioNames(event);
      event->String3 = formatGpioName_output_optional(F("DE"));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P078_DEV_ID   = P078_DEV_ID_DFLT;
      P078_MODEL    = P078_MODEL_DFLT;
      P078_BAUDRATE = P078_BAUDRATE_DFLT;
      P078_QUERY1   = P078_QUERY1_DFLT;
      P078_QUERY2   = P078_QUERY2_DFLT;
      P078_QUERY3   = P078_QUERY3_DFLT;
      P078_QUERY4   = P078_QUERY4_DFLT;

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS:
    {
      if ((P078_DEV_ID == 0) || (P078_DEV_ID > 247) || (P078_BAUDRATE >= 6)) {
        // Load some defaults
        P078_DEV_ID   = P078_DEV_ID_DFLT;
        P078_MODEL    = P078_MODEL_DFLT;
        P078_BAUDRATE = P078_BAUDRATE_DFLT;
        P078_QUERY1   = P078_QUERY1_DFLT;
        P078_QUERY2   = P078_QUERY2_DFLT;
        P078_QUERY3   = P078_QUERY3_DFLT;
        P078_QUERY4   = P078_QUERY4_DFLT;
      }
      {
        String options_baudrate[6];

        for (int i = 0; i < 6; ++i) {
          options_baudrate[i] = String(p078_storageValueToBaudrate(i));
        }
        addFormSelector(F("Baud Rate"), P078_BAUDRATE_LABEL, 6, options_baudrate, NULL, P078_BAUDRATE);
        addUnit(F("baud"));
      }

      if ((P078_MODEL == 0) && (P078_BAUDRATE > 3)) {
        addFormNote(F("<span style=\"color:red\"> SDM120 only allows up to 9600 baud with default 2400!</span>"));
      }

      if ((P078_MODEL == 3) && (P078_BAUDRATE == 0)) {
        addFormNote(F("<span style=\"color:red\"> SDM630 only allows 2400 to 38400 baud with default 9600!</span>"));
      }

      addFormNumericBox(F("Modbus Address"), P078_DEV_ID_LABEL, P078_DEV_ID, 1, 247);

      if (Plugin_078_SDM != nullptr) {
        addRowLabel(F("Checksum (pass/fail)"));
        String chksumStats;
        chksumStats  = Plugin_078_SDM->getSuccCount();
        chksumStats += '/';
        chksumStats += Plugin_078_SDM->getErrCount();
        addHtml(chksumStats);
      }

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      {
        const __FlashStringHelper *options_model[5] =
        { F("SDM220 & SDM120CT & SDM120"), F("SDM230"), F("SDM72D"), F("DDM18SD"), F("SDM630") };
        addFormSelector(F("Model Type"), P078_MODEL_LABEL, 5, options_model, NULL, P078_MODEL);
        addFormNote(F("Submit after changing the modell to update Output Configuration."));
      }

      {
        // In a separate scope to free memory of String array as soon as possible
        sensorTypeHelper_webformLoad_header();
        int nrOptions = 0;

        switch (P078_MODEL) {
          case 0: nrOptions = P078_NR_OUTPUT_OPTIONS_SDM220_SDM120CT_SDM120; break;
          case 1: nrOptions = P078_NR_OUTPUT_OPTIONS_SDM230; break;
          case 2: nrOptions = P078_NR_OUTPUT_OPTIONS_SDM72D; break;
          case 3: nrOptions = P078_NR_OUTPUT_OPTIONS_DDM18SD; break;
          case 4: nrOptions = P078_NR_OUTPUT_OPTIONS_SDM630; break;
        }
        const __FlashStringHelper *options[nrOptions];

        for (int i = 0; i < nrOptions; ++i) {
          options[i] = p078_getQueryString(i, P078_MODEL);
        }

        for (uint8_t i = 0; i < P078_NR_OUTPUT_VALUES; ++i) {
          const uint8_t pconfigIndex = i + P078_QUERY1_CONFIG_POS;
          sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, nrOptions, options);
        }
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      // Save output selector parameters.
      int model = P078_MODEL;

      for (uint8_t i = 0; i < P078_NR_OUTPUT_VALUES; ++i) {
        const uint8_t pconfigIndex = i + P078_QUERY1_CONFIG_POS;
        const uint8_t choice       = PCONFIG(pconfigIndex);
        sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, p078_getQueryValueString(choice, model));
      }

      P078_DEV_ID   = getFormItemInt(P078_DEV_ID_LABEL);
      P078_MODEL    = getFormItemInt(P078_MODEL_LABEL);
      P078_BAUDRATE = getFormItemInt(P078_BAUDRATE_LABEL);

      Plugin_078_init = false; // Force device setup next time
      success         = true;
      break;
    }

    case PLUGIN_INIT:
    {
      Plugin_078_init = true;

      if (Plugin_078_SoftSerial != NULL) {
        delete Plugin_078_SoftSerial;
        Plugin_078_SoftSerial = NULL;
      }
      Plugin_078_SoftSerial = new (std::nothrow) ESPeasySerial(static_cast<ESPEasySerialPort>(CONFIG_PORT), CONFIG_PIN1, CONFIG_PIN2);

      if (Plugin_078_SoftSerial == nullptr) {
        break;
      }
      unsigned int baudrate = p078_storageValueToBaudrate(P078_BAUDRATE);
      Plugin_078_SoftSerial->begin(baudrate);

      if (Plugin_078_SDM != NULL) {
        delete Plugin_078_SDM;
        Plugin_078_SDM = NULL;
      }
      Plugin_078_SDM = new SDM(*Plugin_078_SoftSerial, baudrate, P078_DEPIN);

      if (Plugin_078_SDM != nullptr) {
        Plugin_078_SDM->begin();
        success = true;
      }
      break;
    }

    case PLUGIN_EXIT:
    {
      Plugin_078_init = false;

      if (Plugin_078_SoftSerial != NULL) {
        delete Plugin_078_SoftSerial;
        Plugin_078_SoftSerial = NULL;
      }

      if (Plugin_078_SDM != NULL) {
        delete Plugin_078_SDM;
        Plugin_078_SDM = NULL;
      }
      break;
    }

    case PLUGIN_READ:
    {
      if (Plugin_078_init)
      {
        int     model  = P078_MODEL;
        uint8_t dev_id = P078_DEV_ID;
        UserVar[event->BaseVarIndex]     = p078_readVal(P078_QUERY1, dev_id, model);
        UserVar[event->BaseVarIndex + 1] = p078_readVal(P078_QUERY2, dev_id, model);
        UserVar[event->BaseVarIndex + 2] = p078_readVal(P078_QUERY3, dev_id, model);
        UserVar[event->BaseVarIndex + 3] = p078_readVal(P078_QUERY4, dev_id, model);

        success = true;
        break;
      }
      break;
    }
  }
  return success;
}

float p078_readVal(uint8_t query, uint8_t node, unsigned int model) {
  if (Plugin_078_SDM == NULL) { return 0.0f; }

  uint8_t  retry_count = 3;
  bool  success     = false;
  float _tempvar    = NAN;

  while (retry_count > 0 && !success) {
    Plugin_078_SDM->clearErrCode();
    _tempvar = Plugin_078_SDM->readVal(p078_getRegister(query, model), node);
    --retry_count;

    if (Plugin_078_SDM->getErrCode() == SDM_ERR_NO_ERROR) {
      success = true;
    }
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("EASTRON: (");
    log += node;
    log += ',';
    log += model;
    log += F(") ");
    log += p078_getQueryString(query, model);
    log += F(": ");
    log += _tempvar;
    addLog(LOG_LEVEL_INFO, log);
  }
  delay(1);
  return _tempvar;
}

unsigned int p078_getRegister(uint8_t query, uint8_t model) {
  if (model == 0) { // SDM220 & SDM120CT & SDM120
    switch (query) {
      case 0:  return SDM_PHASE_1_VOLTAGE;
      case 1:  return SDM_PHASE_1_CURRENT;
      case 2:  return SDM_PHASE_1_POWER;
      case 3:  return SDM_PHASE_1_APPARENT_POWER;
      case 4:  return SDM_PHASE_1_REACTIVE_POWER;
      case 5:  return SDM_PHASE_1_POWER_FACTOR;
      case 6:  return SDM_PHASE_1_ANGLE;
      case 7:  return SDM_FREQUENCY;
      case 8:  return SDM_IMPORT_ACTIVE_ENERGY;
      case 9:  return SDM_EXPORT_ACTIVE_ENERGY;
      case 10: return SDM_IMPORT_REACTIVE_ENERGY;
      case 11: return SDM_EXPORT_REACTIVE_ENERGY;
      case 12: return SDM_TOTAL_ACTIVE_ENERGY;
      case 13: return SDM_TOTAL_REACTIVE_ENERGY;
    }
  } else if (model == 1) { // SDM230
    switch (query) {
      case 0:  return SDM_PHASE_1_VOLTAGE;
      case 1:  return SDM_PHASE_1_CURRENT;
      case 2:  return SDM_PHASE_1_POWER;
      case 3:  return SDM_PHASE_1_APPARENT_POWER;
      case 4:  return SDM_PHASE_1_REACTIVE_POWER;
      case 5:  return SDM_PHASE_1_POWER_FACTOR;
      case 6:  return SDM_PHASE_1_ANGLE;
      case 7:  return SDM_FREQUENCY;
      case 8:  return SDM_IMPORT_ACTIVE_ENERGY;
      case 9:  return SDM_EXPORT_ACTIVE_ENERGY;
      case 10: return SDM_IMPORT_REACTIVE_ENERGY;
      case 11: return SDM_EXPORT_REACTIVE_ENERGY;
      case 12: return SDM_TOTAL_SYSTEM_POWER_DEMAND;
      case 13: return SDM_MAXIMUM_TOTAL_SYSTEM_POWER_DEMAND;
      case 14: return SDM_CURRENT_SYSTEM_POSITIVE_POWER_DEMAND;
      case 15: return SDM_MAXIMUM_SYSTEM_POSITIVE_POWER_DEMAND;
      case 16: return SDM_CURRENT_SYSTEM_REVERSE_POWER_DEMAND;
      case 17: return SDM_MAXIMUM_SYSTEM_REVERSE_POWER_DEMAND;
      case 18: return SDM_PHASE_1_CURRENT_DEMAND;
      case 19: return SDM_MAXIMUM_PHASE_1_CURRENT_DEMAND;
      case 20: return SDM_TOTAL_ACTIVE_ENERGY;
      case 21: return SDM_TOTAL_REACTIVE_ENERGY;
      case 22: return SDM_CURRENT_RESETTABLE_TOTAL_ACTIVE_ENERGY;
      case 23: return SDM_CURRENT_RESETTABLE_TOTAL_REACTIVE_ENERGY;
    }
  } else if (model == 2) { // SDM72D
    switch (query) {
      case 0: return SDM_TOTAL_SYSTEM_POWER;
      case 1: return SDM_IMPORT_ACTIVE_ENERGY;
      case 2: return SDM_EXPORT_ACTIVE_ENERGY;
      case 3: return SDM_TOTAL_ACTIVE_ENERGY;
      case 4: return SDM_CURRENT_RESETTABLE_TOTAL_ACTIVE_ENERGY;
      case 5: return SDM_CURRENT_RESETTABLE_IMPORT_ENERGY;
      case 6: return SDM_CURRENT_RESETTABLE_EXPORT_ENERGY;
      case 7: return SDM_IMPORT_POWER;
      case 8: return SDM_EXPORT_POWER;
    }
  }  else if (model == 3) { // DDM18SD
    switch (query) {
      case 0: return DDM_PHASE_1_VOLTAGE;
      case 1: return DDM_PHASE_1_CURRENT;
      case 2: return DDM_PHASE_1_POWER;
      case 3: return DDM_PHASE_1_REACTIVE_POWER;
      case 4: return DDM_PHASE_1_POWER_FACTOR;
      case 5: return DDM_FREQUENCY;
      case 6: return DDM_IMPORT_ACTIVE_ENERGY;
      case 7: return DDM_IMPORT_REACTIVE_ENERGY;
    }
  } else if (model == 4) { // SDM630
    switch (query) {
      case 0:  return SDM_PHASE_1_VOLTAGE;
      case 1:  return SDM_PHASE_2_VOLTAGE;
      case 2:  return SDM_PHASE_3_VOLTAGE;
      case 3:  return SDM_PHASE_1_CURRENT;
      case 4:  return SDM_PHASE_2_CURRENT;
      case 5:  return SDM_PHASE_3_CURRENT;
      case 6:  return SDM_PHASE_1_POWER;
      case 7:  return SDM_PHASE_2_POWER;
      case 8:  return SDM_PHASE_3_POWER;
      case 9:  return SDM_PHASE_1_APPARENT_POWER;
      case 10: return SDM_PHASE_2_APPARENT_POWER;
      case 11: return SDM_PHASE_3_APPARENT_POWER;
      case 12: return SDM_PHASE_1_REACTIVE_POWER;
      case 13: return SDM_PHASE_2_REACTIVE_POWER;
      case 14: return SDM_PHASE_3_REACTIVE_POWER;
      case 15: return SDM_PHASE_1_POWER_FACTOR;
      case 16: return SDM_PHASE_2_POWER_FACTOR;
      case 17: return SDM_PHASE_3_POWER_FACTOR;
      case 18: return SDM_PHASE_1_ANGLE;
      case 19: return SDM_PHASE_2_ANGLE;
      case 20: return SDM_PHASE_3_ANGLE;
      case 21: return SDM_AVERAGE_L_TO_N_VOLTS;
      case 22: return SDM_AVERAGE_LINE_CURRENT;
      case 23: return SDM_SUM_LINE_CURRENT;
      case 24: return SDM_TOTAL_SYSTEM_POWER;
      case 25: return SDM_TOTAL_SYSTEM_APPARENT_POWER;
      case 26: return SDM_TOTAL_SYSTEM_REACTIVE_POWER;
      case 27: return SDM_TOTAL_SYSTEM_POWER_FACTOR;
      case 28: return SDM_TOTAL_SYSTEM_PHASE_ANGLE;
      case 29: return SDM_FREQUENCY;
      case 30: return SDM_IMPORT_ACTIVE_ENERGY;
      case 31: return SDM_EXPORT_ACTIVE_ENERGY;
      case 32: return SDM_IMPORT_REACTIVE_ENERGY;
      case 33: return SDM_EXPORT_REACTIVE_ENERGY;
      case 34: return SDM_VAH_SINCE_LAST_RESET;
      case 35: return SDM_AH_SINCE_LAST_RESET;
      case 36: return SDM_TOTAL_SYSTEM_POWER_DEMAND;
      case 37: return SDM_MAXIMUM_TOTAL_SYSTEM_POWER_DEMAND;
      case 38: return SDM_TOTAL_SYSTEM_VA_DEMAND;
      case 39: return SDM_MAXIMUM_TOTAL_SYSTEM_VA_DEMAND;
      case 40: return SDM_NEUTRAL_CURRENT_DEMAND;
      case 41: return SDM_MAXIMUM_NEUTRAL_CURRENT;
      case 42: return SDM_LINE_1_TO_LINE_2_VOLTS;
      case 43: return SDM_LINE_2_TO_LINE_3_VOLTS;
      case 44: return SDM_LINE_3_TO_LINE_1_VOLTS;
      case 45: return SDM_AVERAGE_LINE_TO_LINE_VOLTS;
      case 46: return SDM_NEUTRAL_CURRENT;
      case 47: return SDM_PHASE_1_LN_VOLTS_THD;
      case 48: return SDM_PHASE_2_LN_VOLTS_THD;
      case 49: return SDM_PHASE_3_LN_VOLTS_THD;
      case 50: return SDM_PHASE_1_CURRENT_THD;
      case 51: return SDM_PHASE_2_CURRENT_THD;
      case 52: return SDM_PHASE_3_CURRENT_THD;
      case 53: return SDM_AVERAGE_LINE_TO_NEUTRAL_VOLTS_THD;
      case 54: return SDM_AVERAGE_LINE_CURRENT_THD;
      case 55: return SDM_TOTAL_SYSTEM_POWER_FACTOR_INV;
      case 56: return SDM_PHASE_1_CURRENT_DEMAND;
      case 57: return SDM_PHASE_2_CURRENT_DEMAND;
      case 58: return SDM_PHASE_3_CURRENT_DEMAND;
      case 59: return SDM_MAXIMUM_PHASE_1_CURRENT_DEMAND;
      case 60: return SDM_MAXIMUM_PHASE_2_CURRENT_DEMAND;
      case 61: return SDM_MAXIMUM_PHASE_3_CURRENT_DEMAND;
      case 62: return SDM_LINE_1_TO_LINE_2_VOLTS_THD;
      case 63: return SDM_LINE_2_TO_LINE_3_VOLTS_THD;
      case 64: return SDM_LINE_3_TO_LINE_1_VOLTS_THD;
      case 65: return SDM_AVERAGE_LINE_TO_LINE_VOLTS_THD;
      case 66: return SDM_TOTAL_ACTIVE_ENERGY;
      case 67: return SDM_TOTAL_REACTIVE_ENERGY;
      case 68: return SDM_L1_IMPORT_ACTIVE_ENERGY;
      case 69: return SDM_L2_IMPORT_ACTIVE_ENERGY;
      case 70: return SDM_L3_IMPORT_ACTIVE_ENERGY;
      case 71: return SDM_L1_EXPORT_ACTIVE_ENERGY;
      case 72: return SDM_L2_EXPORT_ACTIVE_ENERGY;
      case 73: return SDM_L3_EXPORT_ACTIVE_ENERGY;
      case 74: return SDM_L1_TOTAL_ACTIVE_ENERGY;
      case 75: return SDM_L2_TOTAL_ACTIVE_ENERGY;
      case 76: return SDM_L3_TOTAL_ACTIVE_ENERGY;
      case 77: return SDM_L1_IMPORT_REACTIVE_ENERGY;
      case 78: return SDM_L2_IMPORT_REACTIVE_ENERGY;
      case 79: return SDM_L3_IMPORT_REACTIVE_ENERGY;
      case 80: return SDM_L1_EXPORT_REACTIVE_ENERGY;
      case 81: return SDM_L2_EXPORT_REACTIVE_ENERGY;
      case 82: return SDM_L3_EXPORT_REACTIVE_ENERGY;
      case 83: return SDM_L1_TOTAL_REACTIVE_ENERGY;
      case 84: return SDM_L2_TOTAL_REACTIVE_ENERGY;
      case 85: return SDM_L3_TOTAL_REACTIVE_ENERGY;
    }
  }
  return 0;
}

const __FlashStringHelper* p078_getQueryString(uint8_t query, uint8_t model) {
  if (model == 0) { // SDM220 & SDM120CT & SDM120
    switch (query) {
      case 0:  return F("Voltage (V)");
      case 1:  return F("Current (A)");
      case 2:  return F("Power (W)");
      case 3:  return F("ActiveApparentPower (VA)");
      case 4:  return F("ReactiveApparentPower (VAr)");
      case 5:  return F("PowerFactor (cos-phi)");
      case 6:  return F("PhaseAngle(Degrees) (not SDM120)");
      case 7:  return F("Frequency (Hz)");
      case 8:  return F("ImportActiveEnergy (kWh/MWh)");
      case 9:  return F("ExportActiveEnergy (kWh/MWh)");
      case 10: return F("ImportReactiveEnergy (kVArh/MVArh)");
      case 11: return F("ExportReactiveEnergy (kVArh/MVArh)");
      case 12: return F("TotalActiveEnergy (kWh)");
      case 13: return F("TotalReactiveEnergy (kVArh)");
    }
  } else if (model == 1) { // SDM230
    switch (query) {
      case 0:  return F("Voltage (V)");
      case 1:  return F("Current (A)");
      case 2:  return F("Power (W)");
      case 3:  return F("Active Apparent Power (VA)");
      case 4:  return F("Reactive Apparent Power (VAr)");
      case 5:  return F("Power Factor (cos-phi)");
      case 6:  return F("Phase Angle (Degrees)");
      case 7:  return F("Frequency (Hz)");
      case 8:  return F("Import Active Energy (kWh/MWh)");
      case 9:  return F("Export Active Energy (kWh/MWh)");
      case 10: return F("Import Reactive Energy ( kVArh/MVArh)");
      case 11: return F("Export Reactive Energy ( kVArh/MVArh)");
      case 12: return F("Total system power demand (W)");
      case 13: return F("Total Reactive Energy  (kVArh)");
      case 14: return F("Maximum total system power demand (W)");
      case 15: return F("Maximum system positive power demand  (W)");
      case 16: return F("Currentsystem reverse power demand (W)");
      case 17: return F("Maximum system reverse power demand (W)");
      case 18: return F("Current demand (A)");
      case 19: return F("Maximum current demand (A)");
      case 20: return F("Total active energy (kWh)");
      case 21: return F("Total reactive energy (kVArh)");
      case 22: return F("Current resettable total active energy (kWh)");
      case 23: return F("Current resettable total reactive energy (kVArh)");
    }
  } else if (model == 2) { // SDM72D
    switch (query) {
      case 0: return F("Total System Power (W)");
      case 1: return F("Import Active Energy since last reset (kWh/MWh)");
      case 2: return F("Export Active Energy since last reset (kWh/MWh)");
      case 3: return F("Total Active Energy (kwh)");
      case 4: return F("Resettable total Active Energy (kWh)");
      case 5: return F("Resettable import (kWh)");
      case 6: return F("Resettable export (kWh)");
      case 7: return F("Import power (W)");
      case 8: return F("Export power (W)");
    }
  }  else if (model == 3) { // DDM18SD
    switch (query) {
      case 0: return F("Voltage (V)");
      case 1: return F("Current (A)");
      case 2: return F("Power (W)");
      case 3: return F("Reactive Power (VAr)");
      case 4: return F("Power Factor (cos-phi)");
      case 5: return F("Frequency (Hz)");
      case 6: return F("Import Active Energy (kWh)");
      case 7: return F("Import Reactive Energy (kVArh)");
    }
  } else if (model == 4) { // SDM630
    switch (query) {
      case 0:  return F("Voltage (V) Phase 1");
      case 1:  return F("Voltage (V) Phase 2");
      case 2:  return F("Voltage (V) Phase 3");
      case 3:  return F("Current (A) Phase 1");
      case 4:  return F("Current (A) Phase 2");
      case 5:  return F("Current (A) Phase 3");
      case 6:  return F("Power (W) Phase 1");
      case 7:  return F("Power (W) Phase 2");
      case 8:  return F("Power (W) Phase 3");
      case 9:  return F("Active Apparent Power (VA) Phase 1");
      case 10: return F("Active Apparent Power (VA) Phase 2");
      case 11: return F("Active Apparent Power (VA) Phase 3");
      case 12: return F("Reactive Apparent Power (VAr) Phase 1");
      case 13: return F("Reactive Apparent Power (VAr) Phase 2");
      case 14: return F("Reactive Apparent Power (VAr) Phase 3");
      case 15: return F("Power Factor (cos-phi) Phase 1");
      case 16: return F("Power Factor (cos-phi) Phase 2");
      case 17: return F("Power Factor (cos-phi) Phase 3");
      case 18: return F("Phase Angle (Degrees) Phase 1");
      case 19: return F("Phase Angle (Degrees) Phase 2");
      case 20: return F("Phase Angle (Degrees) Phase 3");
      case 21: return F("Average line to neutral volts (V)");
      case 22: return F("Average line Current (A)");
      case 23: return F("Sum of line Currents (A)");
      case 24: return F("Total System Power (W)");
      case 25: return F("Total System Active Apparent Power (VA)");
      case 26: return F("Total System Reactive Apparent Power (VAr)");
      case 27: return F("Total System Power Factor (cos-phi)");
      case 28: return F("Total System Phase Angle (Degrees)");
      case 29: return F("Frequency of Supply Voltages (Hz)");
      case 30: return F("Import Active Energy since last reset (kWh/MWh)");
      case 31: return F("Export Active Energy since last reset (kWh/MWh)");
      case 32: return F("Import Reactive Energy since last reset (kVArh/MVArh)");
      case 33: return F("Export Reactive Energy since last reset (kVArh/MVArh)");
      case 34: return F("VAh since last Reset (kVAh/MVAh)");
      case 35: return F("Ah since last Reset (Ah/kAh)");
      case 36: return F("Total System Power Demand (W)");
      case 37: return F("Maximum Total System Power Demand (W)");
      case 38: return F("Total System VA demand (VA)");
      case 39: return F("Maximum Total System VA Demand (VA)");
      case 40: return F("Neutral Current Demand (A)");
      case 41: return F("Maximum Neutral Current Demand (A)");
      case 42: return F("Line 1 to Line 2 Volts (V)");
      case 43: return F("Line 2 to Line 3 Volts (V)");
      case 44: return F("Line 3 to Line 1 Volts (V)");
      case 45: return F("Average Line to Line Volts (V)");
      case 46: return F("Neutral Current (A)");
      case 47: return F("L/N Volts THD (%) Phase 1");
      case 48: return F("L/N Volts THD (%) Phase 2");
      case 49: return F("L/N Volts THD (%) Phase 3");
      case 50: return F("Current THD  (%) Phase 1 ");
      case 51: return F("Current THD  (%) Phase 2");
      case 52: return F("Current THD  (%) Phase 3");
      case 53: return F("Average Line to Neutral Volts THD (cos-phi)");
      case 54: return F("Average Line Current THD  (%)");
      case 55: return F("Total System Power Factor (%)");
      case 56: return F("Current Demand (A) Phase 1 ");
      case 57: return F("Current Demand (A) Phase 2");
      case 58: return F("Current Demand (A) Phase 3");
      case 59: return F("Maximum Current Demand (A) Phase 1 ");
      case 60: return F("Maximum Current Demand (A) Phase 2");
      case 61: return F("Maximum Current Demand (A) Phase 3");
      case 62: return F("Line 1 to Line 2 Volts THD (%)");
      case 63: return F("Line 2 to Line 3 Volts THD (%)");
      case 64: return F("Line 3 to Line 1 Volts THD (%)");
      case 65: return F("Average Line to Line Volts THD (%)");
      case 66: return F("Total Active Energy (kwh)");
      case 67: return F("Total Reactive Energy (kVArh)");
      case 68: return F("Import Active Energy (kWh) L1");
      case 69: return F("Import Active Energy (kWh) L2");
      case 70: return F("Import Active Energy (kWh) L3");
      case 71: return F("Export Active Energy (kWh) L1");
      case 72: return F("Export Active Energy (kWh) L2");
      case 73: return F("Export Active Energy (kWh) L3");
      case 74: return F("Total Active Energy (kWh) L1");
      case 75: return F("Total Active Energy (kWh) L2");
      case 76: return F("Total Active Energy (kWh) L3");
      case 77: return F("Import Reactive Energy (kVArh) L1");
      case 78: return F("Import Reactive Energy (kVArh) L2");
      case 79: return F("Import Reactive Energy (kVArh) L3");
      case 80: return F("Export Reactive Energy (kVArh) L1");
      case 81: return F("Export Reactive Energy (kVArh) L2");
      case 82: return F("Export Reactive Energy (kVArh) L3");
      case 83: return F("Total Reactive Energy (kVArh) L1");
      case 84: return F("Total Reactive Energy (kVArh) L2");
      case 85: return F("Total Reactive Energy (kVArh) L3");
    }
  }
  return F("");
}

const __FlashStringHelper* p078_getQueryValueString(uint8_t query, uint8_t model) {
  if (model == 0) { // SDM220 & SDM120CT & SDM120
    switch (query) {
      case 0:  return F("V");
      case 1:  return F("A");
      case 2:  return F("W");
      case 3:  return F("VA");
      case 4:  return F("VAr");
      case 5:  return F("cos-phi");
      case 6:  return F("Degrees");
      case 7:  return F("Hz");
      case 8:  
      case 9:  return F("kWh/MWh");
      case 10: 
      case 11: return F("kVArh/MVArh");
      case 12: return F("kWh");
      case 13: return F("kVArh");
    }
  } else if (model == 1) { // SDM230
    switch (query) {
      case 0:  return F("V");
      case 1:  return F("A");
      case 2:  return F("W");
      case 3:  return F("VA");
      case 4:  return F("VAr");
      case 5:  return F("cos-phi");
      case 6:  return F("Degrees");
      case 7:  return F("Hz");
      case 8:  
      case 9:  return F("kWh/MWh");
      case 10: 
      case 11: return F("kVArh/MVArh");
      case 12: 
      case 13: 
      case 14: 
      case 15: 
      case 16: 
      case 17: return F("W");
      case 18: 
      case 19: return F("A");
      case 20: return F("kWh");
      case 21: return F("kVArh");
      case 22: return F("kWh");
      case 23: return F("kVArh");
    }
  } else if (model == 2) { // SDM72D
    switch (query) {
      case 0: return F("W");
      case 1: 
      case 2: return F("kWh/MWh");
      case 3: 
      case 4: 
      case 5: 
      case 6: return F("kWh");
      case 7: 
      case 8: return F("W");
    }
  } else if (model == 3) { // DDM18SD
    switch (query) {
      case 0: return F("V");
      case 1: return F("A");
      case 2: return F("W");
      case 3: return F("Var");
      case 4: return F("cos-phi");
      case 5: return F("Hz");
      case 6: return F("kWh");
      case 7: return F("kVArh");
    }
  } else if (model == 4) { // SDM630
    switch (query) {
      case 0:
      case 1:
      case 2:  return F("V");
      case 3:
      case 4:
      case 5:  return F("A");
      case 6:
      case 7:
      case 8:  return F("W");
      case 9:  
      case 10: 
      case 11: return F("VA");
      case 12: 
      case 13: 
      case 14: return F("VAr");
      case 15: 
      case 16: 
      case 17: return F("cos-phi");
      case 18: 
      case 19: 
      case 20: return F("Degrees");
      case 21: return F("V");
      case 22: 
      case 23: return F("A");
      case 24: return F("W");
      case 25: return F("VA");
      case 26: return F("VAr");
      case 27: return F("cos-phi");
      case 28: return F("Degrees");
      case 29: return F("Hz");
      case 30: 
      case 31: return F("kWh/MWh");
      case 32: 
      case 33: return F("kVArh/MVArh");
      case 34: return F("kVAh/MVAh");
      case 35: return F("Ah/kAh");
      case 36: 
      case 37: return F("W");
      case 38: 
      case 39: return F("VA");
      case 40: 
      case 41: return F("A");
      case 42: 
      case 43: 
      case 44: 
      case 45: return F("V");
      case 46: return F("A");
      case 47: 
      case 48: 
      case 49: 
      case 50: 
      case 51: 
      case 52: 
      case 53: 
      case 54: return F("%");
      case 55: return F("cos-phi");
      case 56: 
      case 57: 
      case 58: 
      case 59: 
      case 60: 
      case 61: return F("A");
      case 62: 
      case 63: 
      case 64: 
      case 65: return F("%");
      case 66: return F("kWh");
      case 67: return F("kVArh");
      case 68: 
      case 69: 
      case 70: 
      case 71: 
      case 72: 
      case 73: 
      case 74: 
      case 75: 
      case 76: return F("kWh");
      case 77: 
      case 78: 
      case 79: 
      case 80: 
      case 81: 
      case 82: 
      case 83: 
      case 84: 
      case 85: return F("kVArh");
    }
  }
  return F("");
}

int p078_storageValueToBaudrate(uint8_t baudrate_setting) {
  unsigned int baudrate = 9600;

  switch (baudrate_setting) {
    case 0:  baudrate = 1200; break;
    case 1:  baudrate = 2400; break;
    case 2:  baudrate = 4800; break;
    case 3:  baudrate = 9600; break;
    case 4:  baudrate = 19200; break;
    case 5:  baudrate = 38400; break;
  }
  return baudrate;
}

#endif // USES_P078
