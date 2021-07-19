#include "_Plugin_Helper.h"
#ifdef USES_P097

// #######################################################################################################
// #################################### Plugin 097: ESP32 Touch ##########################################
// #######################################################################################################


#ifdef ESP32


# define PLUGIN_097
# define PLUGIN_ID_097         97
# define PLUGIN_NAME_097       "Touch (ESP32) - internal"
# define PLUGIN_VALUENAME1_097 "Touch"


# ifdef ESP32
  #  define P097_MAX_ADC_VALUE    4095
# endif // ifdef ESP32
# ifdef ESP8266
  #  define P097_MAX_ADC_VALUE    1023
# endif // ifdef ESP8266


# define P097_SEND_TOUCH_EVENT    PCONFIG(0)
# define P097_SEND_RELEASE_EVENT  PCONFIG(1)
# define P097_SEND_DURATION_EVENT PCONFIG(2)
# define P097_TOUCH_THRESHOLD     PCONFIG(3)

// Share this bitmap among all instances of this plugin
DRAM_ATTR uint32_t p097_pinTouched     = 0;
DRAM_ATTR uint32_t p097_pinTouchedPrev = 0;
DRAM_ATTR uint32_t p097_timestamp[10]  = { 0 };


boolean Plugin_097(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_097;
      Device[deviceCount].Type               = DEVICE_TYPE_ANALOG;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_SINGLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].DecimalsOnly       = false;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_097);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_097));
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      P097_SEND_TOUCH_EVENT = 1;
      P097_SEND_DURATION_EVENT = 1;
      P097_TOUCH_THRESHOLD = 20;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addHtml(F("<TR><TD>Analog Pin:<TD>"));
      addADC_PinSelect(AdcPinSelectPurpose::TouchOnly, F("taskdevicepin1"), CONFIG_PIN1);

      addFormSubHeader(F("Touch Settings"));

      addFormCheckBox(F("Send Touch Event"),    F("p097_sendtouch"),    P097_SEND_TOUCH_EVENT);
      addFormCheckBox(F("Send Release Event"),  F("p097_sendrelease"),  P097_SEND_RELEASE_EVENT);
      addFormCheckBox(F("Send Duration Event"), F("p097_sendduration"), P097_SEND_DURATION_EVENT);
      addFormNumericBox(F("Touch Threshold"), F("p097_threshold"), P097_TOUCH_THRESHOLD, 0, P097_MAX_ADC_VALUE);

      // Show current value
      addRowLabel(F("Current Pressure"));
      addHtml(String(touchRead(CONFIG_PIN1)));

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P097_SEND_TOUCH_EVENT    = isFormItemChecked(F("p097_sendtouch"));
      P097_SEND_RELEASE_EVENT  = isFormItemChecked(F("p097_sendrelease"));
      P097_SEND_DURATION_EVENT = isFormItemChecked(F("p097_sendduration"));
      P097_TOUCH_THRESHOLD     = getFormItemInt(F("p097_threshold"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      P097_setEventParams(CONFIG_PIN1, P097_TOUCH_THRESHOLD);
      success = true;
      break;
    }
    case PLUGIN_TEN_PER_SECOND:
    {
      if ((p097_pinTouched != 0) || (p097_pinTouchedPrev != 0)) {
        // Some pin has been touched or released.
        // Check if it is 'our' pin
        int adc, ch, t;

        if (getADC_gpio_info(CONFIG_PIN1, adc, ch, t)) {
          const bool touched      = bitRead(p097_pinTouched, t);
          const bool touched_prev = bitRead(p097_pinTouchedPrev, t);

          if (touched) {
            bitClear(p097_pinTouched, t);
          }

          if (touched != touched_prev) {
            // state changed
            UserVar[event->BaseVarIndex] = touchRead(CONFIG_PIN1);

            if (touched) {
              if (P097_SEND_TOUCH_EVENT) {
                // schedule a read to update output values and send to controllers
                Scheduler.schedule_task_device_timer(event->TaskIndex, millis());
              }
              bitSet(p097_pinTouchedPrev, t);
            } else {
              if (P097_SEND_RELEASE_EVENT) {
                // schedule a read to update output values and send to controllers
                Scheduler.schedule_task_device_timer(event->TaskIndex, millis());
              }

              if (P097_SEND_DURATION_EVENT) {
                if (Settings.UseRules) {
                  String eventString;
                  eventString.reserve(32);
                  eventString  = getTaskDeviceName(event->TaskIndex);
                  eventString += F("#Duration=");
                  eventString += timePassedSince(p097_timestamp[t]);
                  eventQueue.add(eventString);
                }
              }
              bitClear(p097_pinTouchedPrev, t);
              p097_timestamp[t] = 0;
            }
          }
        }
      }
      success = true;
      break;
    }

    case PLUGIN_READ:
    {
      int raw_value = touchRead(CONFIG_PIN1);
      UserVar[event->BaseVarIndex] = raw_value;

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("Touch : ");
        log += formatGpioName_ADC(CONFIG_PIN1);
        log += F(": ");
        log += raw_value;
        addLog(LOG_LEVEL_INFO, log);
      }
      success = true;
      break;
    }
  }
  return success;
}

/**********************************************************************************
* Touch pin callback functions
**********************************************************************************/
void P097_setEventParams(int pin, uint16_t threshold) {
  int adc, ch, t;

  if (getADC_gpio_info(pin, adc, ch, t)) {
    switch (t) {
      case 0: touchAttachInterrupt(T0, P097_got_T0, threshold); break;
      case 1: touchAttachInterrupt(T1, P097_got_T1, threshold); break;
      case 2: touchAttachInterrupt(T2, P097_got_T2, threshold); break;
      case 3: touchAttachInterrupt(T3, P097_got_T3, threshold); break;
      case 4: touchAttachInterrupt(T4, P097_got_T4, threshold); break;
      case 5: touchAttachInterrupt(T5, P097_got_T5, threshold); break;
      case 6: touchAttachInterrupt(T6, P097_got_T6, threshold); break;
      case 7: touchAttachInterrupt(T7, P097_got_T7, threshold); break;
      case 8: touchAttachInterrupt(T8, P097_got_T8, threshold); break;
      case 9: touchAttachInterrupt(T9, P097_got_T9, threshold); break;
    }
  }
}

void P097_got_T0() IRAM_ATTR;
void P097_got_T1() IRAM_ATTR;
void P097_got_T2() IRAM_ATTR;
void P097_got_T3() IRAM_ATTR;
void P097_got_T4() IRAM_ATTR;
void P097_got_T5() IRAM_ATTR;
void P097_got_T6() IRAM_ATTR;
void P097_got_T7() IRAM_ATTR;
void P097_got_T8() IRAM_ATTR;
void P097_got_T9() IRAM_ATTR;

void P097_got_T0() {
  bitSet(p097_pinTouched, 0);

  if (p097_timestamp[0] == 0) { p097_timestamp[0] = millis(); }
}

void P097_got_T1() {
  bitSet(p097_pinTouched, 1);

  if (p097_timestamp[1] == 0) { p097_timestamp[1] = millis(); }
}

void P097_got_T2() {
  bitSet(p097_pinTouched, 2);

  if (p097_timestamp[2] == 0) { p097_timestamp[2] = millis(); }
}

void P097_got_T3() {
  bitSet(p097_pinTouched, 3);

  if (p097_timestamp[3] == 0) { p097_timestamp[3] = millis(); }
}

void P097_got_T4() {
  bitSet(p097_pinTouched, 4);

  if (p097_timestamp[4] == 0) { p097_timestamp[4] = millis(); }
}

void P097_got_T5() {
  bitSet(p097_pinTouched, 5);

  if (p097_timestamp[5] == 0) { p097_timestamp[5] = millis(); }
}

void P097_got_T6() {
  bitSet(p097_pinTouched, 6);

  if (p097_timestamp[6] == 0) { p097_timestamp[6] = millis(); }
}

void P097_got_T7() {
  bitSet(p097_pinTouched, 7);

  if (p097_timestamp[7] == 0) { p097_timestamp[7] = millis(); }
}

void P097_got_T8() {
  bitSet(p097_pinTouched, 8);

  if (p097_timestamp[8] == 0) { p097_timestamp[8] = millis(); }
}

void P097_got_T9() {
  bitSet(p097_pinTouched, 9);

  if (p097_timestamp[9] == 0) { p097_timestamp[9] = millis(); }
}

#endif // ifdef ESP32


#endif // USES_P097
