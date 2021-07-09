#include "_Plugin_Helper.h"
#ifdef USES_P002


#include "src/Helpers/Hardware.h"

// #######################################################################################################
// #################################### Plugin 002: Analog ###############################################
// #######################################################################################################

#define PLUGIN_002
#define PLUGIN_ID_002         2
#define PLUGIN_NAME_002       "Analog input - internal"
#define PLUGIN_VALUENAME1_002 "Analog"


#ifdef ESP32
  # define P002_MAX_ADC_VALUE    4095
#endif // ifdef ESP32
#ifdef ESP8266
  # define P002_MAX_ADC_VALUE    1023
#endif // ifdef ESP8266


#define P002_IIR_FILTER_FRACTION PCONFIG(0)
#define P002_CALIBRATION_ENABLED PCONFIG(3)
#define P002_CALIBRATION_POINT1  PCONFIG_LONG(0)
#define P002_CALIBRATION_POINT2  PCONFIG_LONG(1)
#define P002_CALIBRATION_VALUE1  PCONFIG_FLOAT(0)
#define P002_CALIBRATION_VALUE2  PCONFIG_FLOAT(1)

#define P002_IIR_FILTER_FACTOR   32

struct P002_data_struct : public PluginTaskData_base {
  P002_data_struct() {}

  void reset() {
    valueRead = false;
  }

  void addSample(int currentValue, int iir_factor) {
    if (currentValue < 0) {
      return;
    }
    int sample = currentValue;
    sample *= 256; // pad for extra resolution.

    if (!valueRead || iir_factor < 2) {
      iir_valA = sample;
      iir_valB = sample;
      valueRead = true;
    } else {
      // Use cascaded IIR filter.
      iir_valA += (sample - iir_valA) / iir_factor;
      iir_valB += (iir_valA - iir_valB) / iir_factor;
    }
  }

  bool getOversamplingValue(float& float_value, int& raw_value) {
    if (valueRead) {
      float_value = static_cast<float>(iir_valB) / 256.0;
      raw_value = iir_valB / 256;
    }
    return valueRead;
  }

private:

  int32_t iir_valA = 0;
  int32_t iir_valB = 0;
  bool valueRead = false;
};

boolean Plugin_002(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_002;
      Device[deviceCount].Type               = DEVICE_TYPE_ANALOG;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_002);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_002));
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      #if defined(ESP32)
      addHtml(F("<TR><TD>Analog Pin:<TD>"));
      addADC_PinSelect(AdcPinSelectPurpose::ADC_Touch_HallEffect, F("taskdevicepin1"), CONFIG_PIN1);

      #endif // if defined(ESP32)

      addFormNumericBox(F("IIR Filter Fraction"), F("p002_iir_ff"), P002_IIR_FILTER_FRACTION, 0, 256);

      addFormSubHeader(F("Two Point Calibration"));

      addFormCheckBox(F("Calibration Enabled"), F("p002_cal"), P002_CALIBRATION_ENABLED);

      addFormNumericBox(F("Point 1"), F("p002_adc1"), P002_CALIBRATION_POINT1, 0, P002_MAX_ADC_VALUE);
      html_add_estimate_symbol();
      addTextBox(F("p002_out1"), String(P002_CALIBRATION_VALUE1, 3), 10);

      addFormNumericBox(F("Point 2"), F("p002_adc2"), P002_CALIBRATION_POINT2, 0, P002_MAX_ADC_VALUE);
      html_add_estimate_symbol();
      addTextBox(F("p002_out2"), String(P002_CALIBRATION_VALUE2, 3), 10);

      {
        // Output the statistics for the current settings.
        int   raw_value = 0;
        float value;

        if (P002_getOutputValue(event, raw_value, value)) {
          P002_formatStatistics(F("Current"), raw_value, value);
        }

        if (P002_CALIBRATION_ENABLED) {
          P002_formatStatistics(F("Minimum"),   0,                  P002_applyCalibration(event, 0));
          P002_formatStatistics(F("Maximum"),   P002_MAX_ADC_VALUE, P002_applyCalibration(event, P002_MAX_ADC_VALUE));

          float stepsize = P002_applyCalibration(event, 1.0f) - P002_applyCalibration(event, 0.0f);
          P002_formatStatistics(F("Step size"), 1,                  stepsize);
        }
      }

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P002_IIR_FILTER_FRACTION = getFormItemInt(F("p002_iir_ff"));

      P002_CALIBRATION_ENABLED = isFormItemChecked(F("p002_cal"));

      P002_CALIBRATION_POINT1 = getFormItemInt(F("p002_adc1"));
      P002_CALIBRATION_VALUE1 = getFormItemFloat(F("p002_out1"));

      P002_CALIBRATION_POINT2 = getFormItemInt(F("p002_adc2"));
      P002_CALIBRATION_VALUE2 = getFormItemFloat(F("p002_out2"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P002_data_struct());
      P002_data_struct *P002_data =
        static_cast<P002_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P002_data);
      break;
    }
    case PLUGIN_TEN_PER_SECOND:
    {
      if (P002_IIR_FILTER_FRACTION > 0) // Oversampling
      {
        P002_data_struct *P002_data =
          static_cast<P002_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P002_data) {
          int currentValue;

          P002_performRead(event, currentValue);
          // TODO TD-er: Must make clipping detection optional.
          if (currentValue >0 && currentValue < P002_MAX_ADC_VALUE) {
            P002_data->addSample(currentValue, P002_IIR_FILTER_FRACTION);
          }
        }
      }
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      int   raw_value = 0;
      float res_value = 0.0f;

      if (P002_getOutputValue(event, raw_value, res_value)) {
        UserVar[event->BaseVarIndex] = res_value;

        P002_data_struct *P002_data =
          static_cast<P002_data_struct *>(getPluginTaskData(event->TaskIndex));

        if (nullptr != P002_data) {
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log = F("ADC  : Analog value: ");
            log += String(raw_value);
            log += F(" = ");
            log += formatUserVarNoCheck(event->TaskIndex, 0);

            if (P002_IIR_FILTER_FRACTION) {
              log += F(" (filtered)");
            }
            addLog(LOG_LEVEL_INFO, log);
          }
          // TODO TD-er: Must perform reset?
          //P002_data->reset();
          success = true;
        } else {
          addLog(LOG_LEVEL_ERROR, F("ADC  : No value received "));
          success = false;
        }
      }

      break;
    }
  }
  return success;
}

bool P002_getOutputValue(struct EventStruct *event, int& raw_value, float& res_value) {
  P002_data_struct *P002_data =
    static_cast<P002_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P002_data) {
    return false;
  }
  float float_value = 0.0f;

  bool valueRead = P002_IIR_FILTER_FRACTION && P002_data->getOversamplingValue(float_value, raw_value);

  if (!valueRead) {
    P002_performRead(event, raw_value);
    float_value = static_cast<float>(raw_value);
  }

  res_value = P002_applyCalibration(event, float_value);
  return true;
}

float P002_applyCalibration(struct EventStruct *event, float float_value) {
  if (P002_CALIBRATION_ENABLED) // Calibration?
  {
    int   adc1 = P002_CALIBRATION_POINT1;
    int   adc2 = P002_CALIBRATION_POINT2;
    float out1 = P002_CALIBRATION_VALUE1;
    float out2 = P002_CALIBRATION_VALUE2;

    if (adc1 != adc2)
    {
      const float normalized = static_cast<float>(float_value - adc1) / static_cast<float>(adc2 - adc1);
      float_value = normalized * (out2 - out1) + out1;
    }
  }
  return float_value;
}

void P002_performRead(struct EventStruct *event, int& value) {
  #ifdef ESP8266
  value = espeasy_analogRead(A0);
  #endif // if defined(ESP8266)
  #if defined(ESP32)
  value = espeasy_analogRead(CONFIG_PIN1);
  #endif // if defined(ESP32)
}

void P002_formatStatistics(const __FlashStringHelper * label, int raw, float float_value) {
  addRowLabel(label);
  addHtmlInt(raw);
  html_add_estimate_symbol();
  addHtml(String(float_value, 3));
}

#endif // USES_P002
