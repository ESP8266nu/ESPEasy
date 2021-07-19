#include "_Plugin_Helper.h"
#ifdef USES_P076
//#######################################################################################################
//#################### Plugin 076 HLW8012 AC Current and Voltage measurement sensor #####################
//#######################################################################################################
//
// This plugin is interfacing with HLW8012 and BL0937 IC which is use with some
// commercial devices like:
// -- Sonoff POW
// -- ElectroDragon HLW8012 Breakout board
// --- and more
//
// The Sonoff POW uses the following PINs: SEL=GPIO05(D1), CF1=GPIO13(D8),
// CF=GPIO14(D5)
// The ED Module has pinheaders so any available PIN on the ESP8266 can be used.
//
// HLW8012 IC works with 5VDC (it seems at 3.3V is not stable in reading)
//

#include <HLW8012.h>

HLW8012 *Plugin_076_hlw = NULL;

#define PLUGIN_076
#define PLUGIN_ID_076 76
#define PLUGIN_076_DEBUG true // activate extra log info in the debug
#define PLUGIN_NAME_076 "Energy (AC) - HLW8012/BL0937  [TESTING]"
#define PLUGIN_VALUENAME1_076 "Voltage"
#define PLUGIN_VALUENAME2_076 "Current"
#define PLUGIN_VALUENAME3_076 "Power"
#define PLUGIN_VALUENAME4_076 "PowerFactor"

#define HLW_DELAYREADING 500

// These are the nominal values for the resistors in the circuit
#define HLW_CURRENT_RESISTOR 0.001
#define HLW_VOLTAGE_RESISTOR_UP (5 * 470000) // Real: 2280k
#define HLW_VOLTAGE_RESISTOR_DOWN (1000)     // Real 1.009k
//-----------------------------------------------------------------------------------------------
int StoredTaskIndex = -1;
uint8_t p076_read_stage = 0;
unsigned long p076_timer = 0;

double p076_hcurrent = 0.0f;
unsigned int p076_hvoltage = 0;
unsigned int p076_hpower = 0;
unsigned int p076_hpowfact = 0;

#define P076_Custom       0

//HLW8012 Devices
#define P076_Sonoff       1
#define P076_Huafan       2
#define P076_KMC          3
#define P076_Aplic        4
#define P076_SK03         5

//BL093 Devices
#define P076_BlitzWolf    6
#define P076_Teckin       7
#define P076_TeckinUS     8
#define P076_Gosund       9
#define P076_Shelly_PLUG_S 10

// Keep values as they are stored and increase this when adding new ones.
#define MAX_P076_DEVICE   11

void IRAM_ATTR p076_hlw8012_cf1_interrupt();
void IRAM_ATTR p076_hlw8012_cf_interrupt();


bool p076_getDeviceString(int device, String& name) {
  switch(device) {
    case P076_Custom   : name = F("Custom");          break;
    case P076_Sonoff   : name = F("Sonoff Pow (r1)"); break;
    case P076_Huafan   : name = F("Huafan SS");       break;
    case P076_KMC      : name = F("KMC 70011");       break;
    case P076_Aplic    : name = F("Aplic WDP303075"); break;
    case P076_SK03     : name = F("SK03 Outdoor");    break;
    case P076_BlitzWolf: name = F("BlitzWolf SHP");   break;
    case P076_Teckin   : name = F("Teckin");          break;
    case P076_TeckinUS : name = F("Teckin US");       break;
    case P076_Gosund   : name = F("Gosund SP1 v23");  break;
    case P076_Shelly_PLUG_S : name = F("Shelly PLUG-S");  break;
    default:
      return false;
  }
  return true;
}

bool p076_getDeviceParameters(int device, uint8_t &SEL_Pin, uint8_t &CF_Pin, uint8_t &CF1_Pin, uint8_t &Cur_read, uint8_t &CF_Trigger, uint8_t &CF1_Trigger) {
  switch(device) {
    case P076_Custom        : SEL_Pin =  0; CF_Pin =  0; CF1_Pin =  0; Cur_read =  LOW; CF_Trigger =     LOW; CF1_Trigger =    LOW; break;
    case P076_Sonoff        : SEL_Pin =  5; CF_Pin = 14; CF1_Pin = 13; Cur_read = HIGH; CF_Trigger =  CHANGE; CF1_Trigger = CHANGE; break;
    case P076_Huafan        : SEL_Pin = 13; CF_Pin = 14; CF1_Pin = 12; Cur_read = HIGH; CF_Trigger =  CHANGE; CF1_Trigger = CHANGE; break;
    case P076_KMC           : SEL_Pin = 12; CF_Pin =  4; CF1_Pin =  5; Cur_read = HIGH; CF_Trigger =  CHANGE; CF1_Trigger = CHANGE; break;
    case P076_Aplic         : SEL_Pin = 12; CF_Pin =  4; CF1_Pin =  5; Cur_read =  LOW; CF_Trigger =  CHANGE; CF1_Trigger = CHANGE; break;
    case P076_SK03          : SEL_Pin = 12; CF_Pin =  4; CF1_Pin =  5; Cur_read =  LOW; CF_Trigger =  CHANGE; CF1_Trigger = CHANGE; break;
    case P076_BlitzWolf     : SEL_Pin = 12; CF_Pin =  5; CF1_Pin = 14; Cur_read =  LOW; CF_Trigger = FALLING; CF1_Trigger = CHANGE; break;
    case P076_Teckin        : SEL_Pin = 12; CF_Pin =  4; CF1_Pin =  5; Cur_read =  LOW; CF_Trigger = FALLING; CF1_Trigger = CHANGE; break;
    case P076_TeckinUS      : SEL_Pin = 12; CF_Pin =  5; CF1_Pin = 14; Cur_read =  LOW; CF_Trigger = FALLING; CF1_Trigger = CHANGE; break;
    case P076_Gosund        : SEL_Pin = 12; CF_Pin =  4; CF1_Pin =  5; Cur_read =  LOW; CF_Trigger = FALLING; CF1_Trigger = CHANGE; break;
    case P076_Shelly_PLUG_S : SEL_Pin = 12; CF_Pin =  5; CF1_Pin = 14; Cur_read =  LOW; CF_Trigger = FALLING; CF1_Trigger = CHANGE; break;
    default:
      return false;
  }
  return true;
}



boolean Plugin_076(uint8_t function, struct EventStruct *event, String &string) {
  boolean success = false;

  switch (function) {
  case PLUGIN_DEVICE_ADD: {
    Device[++deviceCount].Number = PLUGIN_ID_076;
    Device[deviceCount].Type = DEVICE_TYPE_TRIPLE;
    Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_QUAD;
    Device[deviceCount].Ports = 0;
    Device[deviceCount].PullUpOption = false;
    Device[deviceCount].InverseLogicOption = false;
    Device[deviceCount].FormulaOption = true;
    Device[deviceCount].ValueCount = 4;
    Device[deviceCount].SendDataOption = true;
    Device[deviceCount].TimerOption = true;
    Device[deviceCount].GlobalSyncOption = false;
    break;
  }

  case PLUGIN_GET_DEVICENAME: {
    string = F(PLUGIN_NAME_076);
    break;
  }

  case PLUGIN_GET_DEVICEVALUENAMES: {
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0],
             PSTR(PLUGIN_VALUENAME1_076));
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1],
             PSTR(PLUGIN_VALUENAME2_076));
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2],
             PSTR(PLUGIN_VALUENAME3_076));
    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3],
             PSTR(PLUGIN_VALUENAME4_076));
    break;
  }

  case PLUGIN_GET_DEVICEGPIONAMES: {
    event->String1 = formatGpioName_output("SEL");
    event->String2 = formatGpioName_input("CF1");
    event->String3 = formatGpioName_input("CF");
    break;
  }

  case PLUGIN_WEBFORM_LOAD: {
    uint8_t devicePinSettings = PCONFIG(7);

    addFormSubHeader(F("Predefined Pin settings"));
    {
      // Place this in a scope, to keep memory usage low.
      String predefinedNames[MAX_P076_DEVICE];
      int predefinedId[MAX_P076_DEVICE];

      int index = 0;
      for (int i = 0; i < MAX_P076_DEVICE; i++)
      {
        if (p076_getDeviceString(i, predefinedNames[index])) {
          predefinedId[index] = i;
          ++index;
        }
      }
      addFormSelector(F("Device"),
                      F("p076_preDefDevSel"), index,
                      predefinedNames, predefinedId, devicePinSettings );
      addFormNote(F("Enable device and select device type first"));
    }

    {
      // Place this in a scope, to keep memory usage low.
      const __FlashStringHelper * modeRaise[4];
      modeRaise[0] = F("LOW");
      modeRaise[1] = F("CHANGE");
      modeRaise[2] = F("RISING");
      modeRaise[3] = F("FALLING");

      int modeValues[4];
      modeValues[0] = LOW;
      modeValues[1] = CHANGE;
      modeValues[2] = RISING;
      modeValues[3] = FALLING;

      const __FlashStringHelper * modeCurr[2];
      modeCurr[0] = F("LOW");
      modeCurr[1] = F("HIGH");

      int modeCurrValues[2];
      modeCurrValues[0] = LOW;
      modeCurrValues[1] = HIGH;

      uint8_t currentRead = PCONFIG(4);
      if (currentRead != LOW && currentRead != HIGH) {
        currentRead = LOW;
      }
      uint8_t cf_trigger  = PCONFIG(5);
      uint8_t cf1_trigger = PCONFIG(6);
      addFormSubHeader(F("Custom Pin settings (choose Custom above)"));
      addFormSelector(F("SEL Current (A) Reading"), F("p076_curr_read"), 2,
                      modeCurr, modeCurrValues, currentRead );
      addFormSelector(F("CF1  Interrupt Edge"), F("p076_cf1_edge"), 4,
                      modeRaise, modeValues, cf1_trigger);
      addFormSelector(F("CF Interrupt Edge"), F("p076_cf_edge"), 4,
                      modeRaise, modeValues, cf_trigger );
    }


    double current, voltage, power;
    if (Plugin076_LoadMultipliers(event->TaskIndex, current, voltage, power)) {
      addFormSubHeader(F("Calibration Values"));
      addFormTextBox(F("Current Multiplier"), F("p076_currmult"),
                     String(current, 2), 25);
      addFormTextBox(F("Voltage Multiplier"), F("p076_voltmult"),
                     String(voltage, 2), 25);
      addFormTextBox(F("Power Multiplier"), F("p076_powmult"),
                     String(power, 2), 25);
    }

    success = true;
    break;
  }

  case PLUGIN_WEBFORM_SAVE: {

    //Set Pin settings
    uint8_t selectedDevice = getFormItemInt(F("p076_preDefDevSel"));

    PCONFIG(7) = selectedDevice;
    {
      uint8_t SEL_Pin, CF_Pin, CF1_Pin, Cur_read, CF_Trigger, CF1_Trigger;
      if (selectedDevice != 0 && p076_getDeviceParameters(selectedDevice, SEL_Pin, CF_Pin, CF1_Pin, Cur_read, CF_Trigger, CF1_Trigger)) {
        PCONFIG(4) = Cur_read;
        PCONFIG(5) = CF_Trigger;
        PCONFIG(6) = CF1_Trigger;

        CONFIG_PIN1 = SEL_Pin;
        CONFIG_PIN2 = CF1_Pin;
        CONFIG_PIN3 = CF_Pin;
      } else {
        PCONFIG(4) = getFormItemInt(F("p076_curr_read"));
        PCONFIG(5) = getFormItemInt(F("p076_cf_edge"));
        PCONFIG(6) = getFormItemInt(F("p076_cf1_edge"));
      }
    }

    //Set Multipliers
    double hlwMultipliers[3];
    hlwMultipliers[0] = getFormItemFloat(F("p076_currmult"));
    hlwMultipliers[1] = getFormItemFloat(F("p076_voltmult"));
    hlwMultipliers[2] = getFormItemFloat(F("p076_powmult"));
    if (hlwMultipliers[0] > 1.0f && hlwMultipliers[1] > 1.0f && hlwMultipliers[2] > 1.0f) {
      SaveCustomTaskSettings(event->TaskIndex, (uint8_t *)&hlwMultipliers,
                             sizeof(hlwMultipliers));
      if (PLUGIN_076_DEBUG) {
        addLog(LOG_LEVEL_INFO, F("P076: Saved Calibration from Config Page"));
      }

      if (Plugin_076_hlw) {
        Plugin_076_hlw->setCurrentMultiplier(hlwMultipliers[0]);
        Plugin_076_hlw->setVoltageMultiplier(hlwMultipliers[1]);
        Plugin_076_hlw->setPowerMultiplier(hlwMultipliers[2]);
      }

      if (PLUGIN_076_DEBUG) {
        addLog(LOG_LEVEL_INFO, F("P076: Multipliers Reassigned"));
      }
    }

    if (PLUGIN_076_DEBUG) {
      String log = F("P076: PIN Settings ");

      log +=  F(" curr_read: ");
      log +=  PCONFIG(4);
      log +=  F(" cf_edge: ");
      log +=  PCONFIG(5);
      log +=  F(" cf1_edge: ");
      log +=  PCONFIG(6);
      addLog(LOG_LEVEL_INFO, log);
    }

    success = true;
    break;
  }

  case PLUGIN_TEN_PER_SECOND:
    if (Plugin_076_hlw) {
      switch (p076_read_stage) {
      case 0:
        // The stage where we have to wait for a measurement to be started.
        break;
      case 1: // Set mode to read current
        Plugin_076_hlw->setMode(MODE_CURRENT);
        p076_timer = millis() + HLW_DELAYREADING;
        ++p076_read_stage;
        break;
      case 2: // Read current + set mode to read voltage
        if (timeOutReached(p076_timer)) {
          p076_hcurrent = Plugin_076_hlw->getCurrent();
          Plugin_076_hlw->setMode(MODE_VOLTAGE);
          p076_timer = millis() + HLW_DELAYREADING;
          ++p076_read_stage;
        }
        break;
      case 3: // Read voltage + active power + power factor
        if (timeOutReached(p076_timer)) {
          p076_hvoltage = Plugin_076_hlw->getVoltage();
          p076_hpower = Plugin_076_hlw->getActivePower();
          p076_hpowfact = static_cast<int>(100 * Plugin_076_hlw->getPowerFactor());
          ++p076_read_stage;
          // Measurement is done, schedule a new PLUGIN_READ call
          Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
        }
        break;
      default:
        // The PLUGIN_READ should set the new timer and reset the read stage.
        break;
      }
    }
    success = true;
    break;

  case PLUGIN_READ:
    if (Plugin_076_hlw) {
      if (p076_read_stage == 0) {
        // Force a measurement start.
//        ++p076_read_stage;
//      } else if (p076_read_stage > 3) {
          p076_hpower = Plugin_076_hlw->getActivePower();
          p076_hvoltage = Plugin_076_hlw->getVoltage();
          p076_hcurrent = Plugin_076_hlw->getCurrent();
          p076_hpowfact = static_cast<int>(100 * Plugin_076_hlw->getPowerFactor());
        
        // Measurement is complete.
        p076_read_stage = 0;
        if (PLUGIN_076_DEBUG) {
          String log = F("P076: Read values");
          log += F(" - V=");
          log += p076_hvoltage;
          log += F(" - A=");
          log += p076_hcurrent;
          log += F(" - W=");
          log += p076_hpower;
          log += F(" - Pf%=");
          log += p076_hpowfact;
          addLog(LOG_LEVEL_INFO, log);
        }
        UserVar[event->BaseVarIndex] = p076_hvoltage;
        UserVar[event->BaseVarIndex + 1] = p076_hcurrent;
        UserVar[event->BaseVarIndex + 2] = p076_hpower;
        UserVar[event->BaseVarIndex + 3] = p076_hpowfact;
        // Plugin_076_hlw->toggleMode();
        success = true;
      }
    }
    break;

  case PLUGIN_EXIT: {
    Plugin076_Reset(event->TaskIndex);
    success = true;
    break;
  }

  case PLUGIN_INIT: {
    Plugin076_Reset(event->TaskIndex);
    // This initializes the HWL8012 library.
    const uint8_t CF_PIN = CONFIG_PIN3;
    const uint8_t CF1_PIN = CONFIG_PIN2;
    const uint8_t SEL_PIN = CONFIG_PIN1;

    if (CF_PIN != -1 && CF1_PIN != -1 && SEL_PIN != -1) {
      Plugin_076_hlw = new (std::nothrow) HLW8012;
      if (Plugin_076_hlw) {
        uint8_t currentRead = PCONFIG(4);
        uint8_t cf_trigger  = PCONFIG(5);
        uint8_t cf1_trigger = PCONFIG(6);

        Plugin_076_hlw->begin(CF_PIN, CF1_PIN, SEL_PIN, currentRead,
                              true); // set use_interrupts to true to use
                                     // interrupts to monitor pulse widths
        if (PLUGIN_076_DEBUG){
          addLog(LOG_LEVEL_INFO, F("P076: Init object done"));
        }
        Plugin_076_hlw->setResistors(HLW_CURRENT_RESISTOR,
                                     HLW_VOLTAGE_RESISTOR_UP,
                                     HLW_VOLTAGE_RESISTOR_DOWN);
        if (PLUGIN_076_DEBUG){
          addLog(LOG_LEVEL_INFO, F("P076: Init Basic Resistor Values done"));
        }

        double current, voltage, power;
        if (Plugin076_LoadMultipliers(event->TaskIndex, current, voltage, power)) {
          if (PLUGIN_076_DEBUG){
            addLog(LOG_LEVEL_INFO, F("P076: Saved Calibration after INIT"));
          }

          Plugin_076_hlw->setCurrentMultiplier(current);
          Plugin_076_hlw->setVoltageMultiplier(voltage);
          Plugin_076_hlw->setPowerMultiplier(power);
        } else {
          Plugin076_ResetMultipliers();
        }

        if (PLUGIN_076_DEBUG){
          addLog(LOG_LEVEL_INFO, F("P076: Applied Calibration after INIT"));
        }
        StoredTaskIndex = event->TaskIndex; // store task index value in order to
                                            // use it in the PLUGIN_WRITE routine

        // Library expects an interrupt on both edges
        attachInterrupt(CF1_PIN, p076_hlw8012_cf1_interrupt, cf1_trigger);
        attachInterrupt(CF_PIN, p076_hlw8012_cf_interrupt, cf_trigger);
        success = true;
      }
    }
    break;
  }

  case PLUGIN_WRITE:
    if (Plugin_076_hlw) {
      String command = parseString(string, 1);
      if (command.equalsIgnoreCase(F("hlwreset"))) {
        Plugin076_ResetMultipliers();
        success = true;
      }

      if (command.equalsIgnoreCase(F("hlwcalibrate"))) {
        unsigned int CalibVolt = 0;
        double CalibCurr = 0;
        unsigned int CalibAcPwr = 0;
        if (validUIntFromString(parseString(string, 2), CalibVolt)) {
          if (validDoubleFromString(parseString(string, 3), CalibCurr)) {
            validUIntFromString(parseString(string, 4), CalibAcPwr);
          }
        }
        if (PLUGIN_076_DEBUG) {
          String log = F("P076: Calibration to values");
          log += F(" - Expected-V=");
          log += CalibVolt;
          log += F(" - Expected-A=");
          log += CalibCurr;
          log += F(" - Expected-W=");
          log += CalibAcPwr;
          addLog(LOG_LEVEL_INFO, log);
        }
        bool changed = false;
        if (CalibVolt != 0) {
          Plugin_076_hlw->expectedVoltage(CalibVolt);
          changed = true;
        }
        if (CalibCurr > 0.0f) {
          Plugin_076_hlw->expectedCurrent(CalibCurr);
          changed = true;
        }
        if (CalibAcPwr != 0) {
          Plugin_076_hlw->expectedActivePower(CalibAcPwr);
          changed = true;
        }
        // if at least one calibration value has been provided then save the new
        // multipliers //
        if (changed) {
          Plugin076_SaveMultipliers();
        }
        success = true;
      }
    }
    break;

  }
  return success;
}

void Plugin076_ResetMultipliers() {
  if (Plugin_076_hlw) {
    Plugin_076_hlw->resetMultipliers();
    Plugin076_SaveMultipliers();
    if (PLUGIN_076_DEBUG){
      addLog(LOG_LEVEL_INFO, F("P076: Reset Multipliers to DEFAULT"));
    }
  }
}

void Plugin076_SaveMultipliers() {
  if (StoredTaskIndex < 0) return; // Not yet initialized.
  double hlwMultipliers[3];
  if (Plugin076_ReadMultipliers(hlwMultipliers[0], hlwMultipliers[1], hlwMultipliers[2])) {
    SaveCustomTaskSettings(StoredTaskIndex, (uint8_t *)&hlwMultipliers,
                           sizeof(hlwMultipliers));
  }
}

bool Plugin076_ReadMultipliers(double& current, double& voltage, double& power) {
  current = 0.0f;
  voltage = 0.0f;
  power   = 0.0f;
  if (Plugin_076_hlw) {
    current = Plugin_076_hlw->getCurrentMultiplier();
    voltage = Plugin_076_hlw->getVoltageMultiplier();
    power   = Plugin_076_hlw->getPowerMultiplier();
    return true;
  }
  return false;
}


bool Plugin076_LoadMultipliers(taskIndex_t TaskIndex, double& current, double& voltage, double& power) {
  // If multipliers are empty load default ones and save all of them as
  // "CustomTaskSettings"
  if (!Plugin076_ReadMultipliers(current, voltage, power)) {
    return false;
  }
  double hlwMultipliers[3];
  LoadCustomTaskSettings(TaskIndex, (uint8_t *)&hlwMultipliers,
                         sizeof(hlwMultipliers));
  if (hlwMultipliers[0] > 1.0f) {
    current = hlwMultipliers[0];
  }
  if (hlwMultipliers[1] > 1.0f) {
    voltage = hlwMultipliers[1];
  }
  if (hlwMultipliers[2] > 1.0f) {
    power = hlwMultipliers[2];
  }
  return (current > 1.0f) && (voltage > 1.0f) && (power > 1.0f);
}

void Plugin076_Reset(taskIndex_t TaskIndex) {
  if (Plugin_076_hlw) {
    const uint8_t CF_PIN = Settings.TaskDevicePin3[TaskIndex];
    const uint8_t CF1_PIN = Settings.TaskDevicePin2[TaskIndex];
    detachInterrupt(CF_PIN);
    detachInterrupt(CF1_PIN);
    delete Plugin_076_hlw;
    Plugin_076_hlw = nullptr;
  }
  StoredTaskIndex = -1;
  p076_read_stage = 0;
  p076_timer = 0;

  p076_hcurrent = 0.0f;
  p076_hvoltage = 0;
  p076_hpower = 0;
  p076_hpowfact = 0;
}

// When using interrupts we have to call the library entry point
// whenever an interrupt is triggered
void IRAM_ATTR p076_hlw8012_cf1_interrupt() {
  if (Plugin_076_hlw) {
    Plugin_076_hlw->cf1_interrupt();
  }
}

void IRAM_ATTR p076_hlw8012_cf_interrupt() {
  if (Plugin_076_hlw) {
    Plugin_076_hlw->cf_interrupt();
  }
}

#endif // USES_P076
