#include "_Plugin_Helper.h"
#ifdef USES_P098

// #######################################################################################################
// ######################## Plugin 098 PWM Motor I2C Barometric Pressure Sensor  ########################
// #######################################################################################################


# include "src/PluginStructs/P098_data_struct.h"

# define PLUGIN_098
# define PLUGIN_ID_098         98
# define PLUGIN_NAME_098       "Motor - PWM Motor"
# define PLUGIN_VALUENAME1_098 "Position"
# define PLUGIN_VALUENAME2_098 "LimitA"
# define PLUGIN_VALUENAME3_098 "LimitB"
# define PLUGIN_VALUENAME4_098 "LimitApos"


# define P098_PWM_FREQ       PCONFIG_LONG(0)
# define P098_FLAGS          PCONFIG_LONG(1)
# define P098_LIMIT_SWA_GPIO PCONFIG(0)
# define P098_LIMIT_SWB_GPIO PCONFIG(1)
# define P098_MOTOR_CONTROL  PCONFIG(2)
# ifdef ESP32
#  define P098_ANALOG_GPIO    PCONFIG(3)
# endif // ifdef ESP32
# define P098_LIMIT_SWA_DEBOUNCE PCONFIG(4)
# define P098_LIMIT_SWB_DEBOUNCE PCONFIG(5)


# define P098_FLAGBIT_LIM_A_PULLUP         0
# define P098_FLAGBIT_LIM_B_PULLUP         1
# define P098_FLAGBIT_ENC_IN_PULLUP        2
# define P098_FLAGBIT_LIM_A_INVERTED       3
# define P098_FLAGBIT_LIM_B_INVERTED       4
# define P098_FLAGBIT_MOTOR_FWD_INVERTED   5
# define P098_FLAGBIT_MOTOR_REV_INVERTED   6


boolean Plugin_098(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_098;
      Device[deviceCount].Type               = DEVICE_TYPE_CUSTOM0;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_QUAD;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 4;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_098);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_098));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_098));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_098));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_098));
      break;
    }

    case PLUGIN_WEBFORM_SHOW_GPIO_DESCR:
    {
      addHtml(F("M Fwd:"));
      addHtml(F("&nbsp;"));
      addHtml(formatGpioLabel(Settings.TaskDevicePin1[event->TaskIndex], true));
      addHtml(event->String1);
      addHtml(F("M Rev:"));
      addHtml(F("&nbsp;"));
      addHtml(formatGpioLabel(Settings.TaskDevicePin2[event->TaskIndex], true));
      addHtml(event->String1);
      addHtml(F("Enc:"));
      addHtml(F("&nbsp;"));
      addHtml(formatGpioLabel(Settings.TaskDevicePin3[event->TaskIndex], true));
      addHtml(event->String1);
      # ifdef ESP32
      addHtml(F("Analog:"));
      addHtml(F("&nbsp;"));
      addHtml(formatGpioLabel(P098_ANALOG_GPIO, true));
      addHtml(event->String1);
      # endif // ifdef ESP32
      addHtml(F("Lim A:"));
      addHtml(F("&nbsp;"));
      addHtml(formatGpioLabel(P098_LIMIT_SWA_GPIO, true));
      addHtml(event->String1);
      addHtml(F("Lim B:"));
      addHtml(F("&nbsp;"));
      addHtml(formatGpioLabel(P098_LIMIT_SWB_GPIO, true));
      success = true;
      break;
    }

    /*
        case PLUGIN_WEBFORM_SHOW_VALUES:
        {
          P098_data_struct *P098_data =
            static_cast<P098_data_struct *>(getPluginTaskData(event->TaskIndex));

          if (nullptr != P098_data) {
            byte varNr = VARS_PER_TASK;
            int limitApos, limitBpos;
            P098_data->getLimitSwitchPositions(limitApos, limitBpos);

            pluginWebformShowValue(event->TaskIndex, varNr++, F("LimitBpos"), String(limitBpos));
            pluginWebformShowValue(event->TaskIndex, varNr++, F("State"),  String(static_cast<int>(P098_data->state)), true);

            // success = true;
          }
          break;
        }
     */
    case PLUGIN_SET_DEFAULTS:
    {
      P098_LIMIT_SWA_GPIO     = -1;
      P098_LIMIT_SWB_GPIO     = -1;
      P098_LIMIT_SWA_DEBOUNCE = 100;
      P098_LIMIT_SWB_DEBOUNCE = 100;
      P098_MOTOR_CONTROL      = 0; // No PWM
      P098_PWM_FREQ           = 1000;
      # ifdef ESP32
      P098_ANALOG_GPIO = -1;       // Analog feedback
      # endif // ifdef ESP32

      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormSubHeader(F("Motor Control"));

      // We load/save the TaskDevicePin ourselves to allow to combine the pin specific configuration be shown along with the pin selection.
      addFormPinSelect(PinSelectPurpose::Generic_output, 
                       formatGpioName_output(F("Motor Fwd")), 
                       F("taskdevicepin1"),
                       Settings.TaskDevicePin1[event->TaskIndex]);
      addFormCheckBox(F("Motor Fwd Inverted"), F("mot_fwd_inv"), bitRead(P098_FLAGS, P098_FLAGBIT_MOTOR_FWD_INVERTED));

      addFormSeparator(2);

      addFormPinSelect(PinSelectPurpose::Generic_output, 
                       formatGpioName_output(F("Motor Rev")), 
                       F("taskdevicepin2"),
                       Settings.TaskDevicePin2[event->TaskIndex]);
      addFormCheckBox(F("Motor Rev Inverted"), F("mot_rev_inv"), bitRead(P098_FLAGS, P098_FLAGBIT_MOTOR_REV_INVERTED));

      addFormSeparator(2);

      {
        # define P098_PWM_MODE_TYPES  static_cast<int>(P098_config_struct::PWM_mode_type::MAX_TYPE)
        String options[P098_PWM_MODE_TYPES];
        int optionValues[P098_PWM_MODE_TYPES];

        for (int i = 0; i < P098_PWM_MODE_TYPES; ++i) {
          options[i]      = P098_config_struct::toString(static_cast<P098_config_struct::PWM_mode_type>(i));
          optionValues[i] = i;
        }
        addFormSelector(F("Motor Control"), F("p098_motor_contr"), P098_PWM_MODE_TYPES, options, optionValues, P098_MOTOR_CONTROL);
      }
      addFormNumericBox(F("PWM Frequency"), F("p098_pwm_freq"), P098_PWM_FREQ, 1000, 100000);
      addUnit(F("Hz"));

      addFormSubHeader(F("Feedback"));

      addFormPinSelect(PinSelectPurpose::Generic_input,
                       formatGpioName_input_optional(F("Encoder")),
                       F("taskdevicepin3"),
                       Settings.TaskDevicePin3[event->TaskIndex]);
      addFormCheckBox(F("Encoder Pull-Up"), F("enc_pu"), bitRead(P098_FLAGS, P098_FLAGBIT_ENC_IN_PULLUP));

      # ifdef ESP32
      {
        addRowLabel(formatGpioName_input_optional(F("Analog Feedback")));
        addADC_PinSelect(AdcPinSelectPurpose::ADC_Touch_Optional, F("analogpin"), P098_ANALOG_GPIO);
      }
      # endif // ifdef ESP32

      addFormSubHeader(F("Limit Switches"));

      addFormPinSelect(PinSelectPurpose::Generic_input, 
                       formatGpioName_input_optional(F("Limit A")), 
                       F("limit_a"), 
                       P098_LIMIT_SWA_GPIO);
      addFormNumericBox(F("Limit A Debounce"), F("limit_a_debounce"), P098_LIMIT_SWA_DEBOUNCE, 0, 1000);
      addUnit(F("ms"));
      addFormCheckBox(F("Limit A Pull-Up"),  F("limit_a_pu"),  bitRead(P098_FLAGS, P098_FLAGBIT_LIM_A_PULLUP));
      addFormCheckBox(F("Limit A Inverted"), F("limit_a_inv"), bitRead(P098_FLAGS, P098_FLAGBIT_LIM_A_INVERTED));

      addFormSeparator(2);

      addFormPinSelect(PinSelectPurpose::Generic_input, 
                       formatGpioName_input_optional(F("Limit B")), 
                       F("limit_b"), 
                       P098_LIMIT_SWB_GPIO);
      addFormCheckBox(F("Limit B Pull-Up"),  F("limit_b_pu"),  bitRead(P098_FLAGS, P098_FLAGBIT_LIM_B_PULLUP));
      addFormCheckBox(F("Limit B Inverted"), F("limit_b_inv"), bitRead(P098_FLAGS, P098_FLAGBIT_LIM_B_INVERTED));
      addFormNumericBox(F("Limit B Debounce"), F("limit_b_debounce"), P098_LIMIT_SWB_DEBOUNCE, 0, 1000);
      addUnit(F("ms"));


      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      // We load/save the TaskDevicePin ourselves to allow to combine the pin specific configuration be shown along with the pin selection.
      Settings.TaskDevicePin1[event->TaskIndex] = getFormItemInt(F("taskdevicepin1"));
      Settings.TaskDevicePin2[event->TaskIndex] = getFormItemInt(F("taskdevicepin2"));
      Settings.TaskDevicePin3[event->TaskIndex] = getFormItemInt(F("taskdevicepin3"));

      P098_LIMIT_SWA_GPIO     = getFormItemInt(F("limit_a"));
      P098_LIMIT_SWB_GPIO     = getFormItemInt(F("limit_b"));
      P098_LIMIT_SWA_DEBOUNCE = getFormItemInt(F("limit_a_debounce"));
      P098_LIMIT_SWB_DEBOUNCE = getFormItemInt(F("limit_b_debounce"));

      P098_MOTOR_CONTROL = getFormItemInt(F("p098_motor_contr"));
      P098_PWM_FREQ      = getFormItemInt(F("p098_pwm_freq"));
      # ifdef ESP32
      P098_ANALOG_GPIO = getFormItemInt(F("analogpin"));
      # endif // ifdef ESP32

      P098_FLAGS = 0;

      if (isFormItemChecked(F("mot_fwd_inv"))) { bitSet(P098_FLAGS, P098_FLAGBIT_MOTOR_FWD_INVERTED); }

      if (isFormItemChecked(F("mot_rev_inv"))) { bitSet(P098_FLAGS, P098_FLAGBIT_MOTOR_REV_INVERTED); }

      if (isFormItemChecked(F("enc_pu"))) { bitSet(P098_FLAGS, P098_FLAGBIT_ENC_IN_PULLUP); }

      if (isFormItemChecked(F("limit_a_pu"))) { bitSet(P098_FLAGS, P098_FLAGBIT_LIM_A_PULLUP); }

      if (isFormItemChecked(F("limit_b_pu"))) { bitSet(P098_FLAGS, P098_FLAGBIT_LIM_B_PULLUP); }

      if (isFormItemChecked(F("limit_a_inv"))) { bitSet(P098_FLAGS, P098_FLAGBIT_LIM_A_INVERTED); }

      if (isFormItemChecked(F("limit_b_inv"))) { bitSet(P098_FLAGS, P098_FLAGBIT_LIM_B_INVERTED); }

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      P098_config_struct config;
      config.motorFwd.gpio          = CONFIG_PIN1;
      config.motorRev.gpio          = CONFIG_PIN2;
      config.encoder.gpio           = CONFIG_PIN3;
      config.limitA.gpio            = P098_LIMIT_SWA_GPIO;
      config.limitB.gpio            = P098_LIMIT_SWB_GPIO;
      config.limitA.debounceTime_us = P098_LIMIT_SWA_DEBOUNCE * 1000;
      config.limitB.debounceTime_us = P098_LIMIT_SWB_DEBOUNCE * 1000;
      # ifdef ESP32
      config.gpio_analogIn = P098_ANALOG_GPIO;
      # endif // ifdef ESP32
      config.motorFwd.inverted = bitRead(P098_FLAGS, P098_FLAGBIT_MOTOR_FWD_INVERTED);
      config.motorRev.inverted = bitRead(P098_FLAGS, P098_FLAGBIT_MOTOR_REV_INVERTED);
      config.limitA.inverted   = bitRead(P098_FLAGS, P098_FLAGBIT_LIM_A_INVERTED);
      config.limitB.inverted   = bitRead(P098_FLAGS, P098_FLAGBIT_LIM_B_INVERTED);
      config.limitA.pullUp     = bitRead(P098_FLAGS, P098_FLAGBIT_LIM_A_PULLUP);
      config.limitB.pullUp     = bitRead(P098_FLAGS, P098_FLAGBIT_LIM_B_PULLUP);
      config.encoder.pullUp    = bitRead(P098_FLAGS, P098_FLAGBIT_ENC_IN_PULLUP);

      config.PWM_mode = static_cast<P098_config_struct::PWM_mode_type>(P098_MOTOR_CONTROL);


      initPluginTaskData(event->TaskIndex, new (std::nothrow) P098_data_struct(config));
      P098_data_struct *P098_data =
        static_cast<P098_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P098_data) {
        // Restore the last known values on warm boot.
        P098_data->begin(
          UserVar[event->BaseVarIndex + 0], // Position
          UserVar[event->BaseVarIndex + 3], // limitApos
          0);                               // limitBpos is not stored in RTC
        success = true;
      }
      break;
    }

    case PLUGIN_FIFTY_PER_SECOND:
    {
      P098_data_struct *P098_data =
        static_cast<P098_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P098_data) {
        bool limitA_triggered, limitB_triggered = false;
        P098_data->getLimitSwitchStates(limitA_triggered, limitB_triggered);

        if (!P098_data->loop()) {}

        switch (P098_data->state) {
          case P098_data_struct::State::Idle:
          case P098_data_struct::State::RunFwd:
          case P098_data_struct::State::RunRev:
            break;
          case P098_data_struct::State::StopLimitSw:
          case P098_data_struct::State::StopPosReached:
          {
            if (Settings.UseRules) {
              String RuleEvent = getTaskDeviceName(event->TaskIndex);
              RuleEvent += '#';

              if (limitA_triggered) {
                eventQueue.addMove(String(RuleEvent + F("limitA")));
              }

              if (limitB_triggered) {
                eventQueue.addMove(String(RuleEvent + F("limitB")));
              }

              if (P098_data->state == P098_data_struct::State::StopPosReached) {
                eventQueue.addMove(String(RuleEvent + F("positionReached")));
              }
            }
            P098_data->state = P098_data_struct::State::Idle;
            break;
          }
        }

        // }
        int limitApos, limitBpos;
        P098_data->getLimitSwitchPositions(limitApos, limitBpos);
        UserVar[event->BaseVarIndex + 0] = P098_data->getPosition();
        UserVar[event->BaseVarIndex + 1] = limitA_triggered ? 1 : 0;
        UserVar[event->BaseVarIndex + 2] = limitB_triggered ? 1 : 0;
        UserVar[event->BaseVarIndex + 3] = limitApos;
      }
      break;
    }

    case PLUGIN_READ:
    {
      P098_data_struct *P098_data =
        static_cast<P098_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P098_data) {
        // What to do here?
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      P098_data_struct *P098_data =
        static_cast<P098_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P098_data) {
        const String command = parseString(string, 1);

        if (command.startsWith(F("pwmmotor"))) {
          if (command == F("pwmmotorhome")) {
            // Run the motor in reverse till limit A switch is reached
            P098_data->findHome();
            success = true;
          } else if (command == F("pwmmotorend")) {
            // Run the motor forward till limit B switch is reached
            P098_data->moveForward(-1);
            success = true;
          } else if (command == F("pwmmotorforward")) {
            // Run the motor N steps forward
            // N <= 0: Move till limit B switch is reached
            P098_data->moveForward(event->Par1);
            success = true;
          } else if (command == F("pwmmotorreverse")) {
            // Run the motor N steps in reverse
            P098_data->moveReverse(event->Par1);
            success = true;
          } else if (command == F("pwmmotorstop")) {
            // Run the motor N steps in reverse
            P098_data->stop();
            success = true;
          } else if (command == F("pwmmotormovetopos")) {
            // Run the motor in the required direction to position N
            // What to do when position is unknown?
            if (!P098_data->moveToPos(event->Par1)) {
              if (!P098_data->homePosSet()) {
                addLog(LOG_LEVEL_ERROR, F("PWM motor: Home position unknown"));
              } else {
                addLog(LOG_LEVEL_ERROR, F("PWM motor: Cannot move to position"));
              }
            }
            success = true;
          }
        }
      }

      break;
    }
  }
  return success;
}

#endif // USES_P098
