#include "_Plugin_Helper.h"
#ifdef USES_P008
//#######################################################################################################
//################################# Plugin 008: Wiegand RFID Tag Reader #################################
//#######################################################################################################

/*
History:
2020-07-04 tonhuisman: Add checkbox for 'Present hex as decimal value' option (with note) so hexadecimal
           value of f.e. a numeric keypad using the Wiegand protocol (hexadecimal data) will be cast to decimal.
           When enabled entering 1234# will result in Tag = 1234 instead of 4660 (= 0x1234), any A-F
           entered will result in a 0 in the output value.
-------------
No initial history available.
*/



#define PLUGIN_008
#define PLUGIN_ID_008         8
#define PLUGIN_NAME_008       "RFID - Wiegand"
#define PLUGIN_VALUENAME1_008 "Tag"

void Plugin_008_interrupt1() ICACHE_RAM_ATTR;
void Plugin_008_interrupt2() ICACHE_RAM_ATTR;

volatile byte Plugin_008_bitCount = 0;     // Count the number of bits received.
uint64_t Plugin_008_keyBuffer = 0;    // A 64-bit-long keyBuffer into which the number is stored.
byte Plugin_008_timeoutCount = 0;
byte Plugin_008_WiegandSize = 26;          // size of a tag via wiegand (26-bits or 36-bits)

boolean Plugin_008_init = false;

/**
 * Convert/cast a hexadecimal input to a decimal representation, so 0x1234 (= 4660) comes out as 1234.
 * 
 * //FIXME Move to a more global place to also be used elsewhere?
 */
uint64_t castHexAsDec(uint64_t hexValue) {
  uint64_t result = 0;
  uint8_t digit;
  for (int i = 0; i < 8; i++) {
    digit = (hexValue & 0x0000000F);
    if (digit > 10) digit = 0; // Cast by dropping any non-decimal input
    if (digit > 0) { // Avoid 'expensive' pow operation if not used
      result += (digit * static_cast<uint64_t>(pow(10, i)));
    }
    hexValue >>= 4;
    if (hexValue == 0) break; // Stop when no more to process
  }
  return result;
}

boolean Plugin_008(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_008;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_LONG;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_008);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_008));
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        event->String1 = formatGpioName_input(F("D0 (Green, 5V)"));
        event->String2 = formatGpioName_input(F("D1 (White, 5V)"));
        break;
      }

    case PLUGIN_INIT:
      {
        Plugin_008_init = true;
        Plugin_008_WiegandSize = PCONFIG(0);
        pinMode(CONFIG_PIN1, INPUT_PULLUP);
        pinMode(CONFIG_PIN2, INPUT_PULLUP);
        attachInterrupt(CONFIG_PIN1, Plugin_008_interrupt1, FALLING);
        attachInterrupt(CONFIG_PIN2, Plugin_008_interrupt2, FALLING);
        success = true;
        break;
      }

    case PLUGIN_TIMER_IN:
      {
        if (Plugin_008_init && PCONFIG(2) == 0) { // PCONFIG(2) check uses inversed logic!
          // Reset card id on timeout
          UserVar.setSensorTypeLong(event->TaskIndex, PCONFIG_LONG(0));
          addLog(LOG_LEVEL_INFO, F("RFID : Removed Tag"));
          if (PCONFIG(3) == 1) {
            sendData(event);
          }
          success = true;
        }
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        if (Plugin_008_init)
        {
          if (Plugin_008_bitCount > 0)
          {
            if (Plugin_008_bitCount % 4 == 0 && ((Plugin_008_keyBuffer & 0xF) == 11))
            {
              // a number of keys were pressed and finished by #
              Plugin_008_keyBuffer = Plugin_008_keyBuffer >> 4;  // Strip #
            }
            else if (Plugin_008_bitCount == Plugin_008_WiegandSize)
            {
              // read a tag
              Plugin_008_keyBuffer = Plugin_008_keyBuffer >> 1;          // Strip leading and trailing parity bits from the keyBuffer
              if (Plugin_008_WiegandSize == 26)
                Plugin_008_keyBuffer &= 0xFFFFFF;
              else
                Plugin_008_keyBuffer &= 0xFFFFFFFF;
            }
            else
            {
              // not enough bits, maybe next time
              Plugin_008_timeoutCount++;
              if (Plugin_008_timeoutCount > 5)
              {
                if (loglevelActiveFor(LOG_LEVEL_INFO)) {
                  String log = F("RFID : reset bits: ");
                  log += Plugin_008_bitCount;
                  addLog(LOG_LEVEL_INFO, log );
                }
                // reset after ~5 sec
                Plugin_008_keyBuffer = 0;
                Plugin_008_bitCount = 0;
                Plugin_008_timeoutCount = 0;
              }
              break;
            }

            unsigned long old_key = UserVar.getSensorTypeLong(event->TaskIndex);
            bool new_key = false;
            if (PCONFIG(1) == 1) {
              Plugin_008_keyBuffer = castHexAsDec(Plugin_008_keyBuffer);
            }
            
            if (old_key != Plugin_008_keyBuffer) {
              UserVar.setSensorTypeLong(event->TaskIndex, Plugin_008_keyBuffer);
              new_key = true;
            }
            
            if (loglevelActiveFor(LOG_LEVEL_INFO)) {
              // write log
              String log = F("RFID : ");
              if (new_key) {
                log += F("New Tag: ");
              } else {
                log += F("Old Tag: "); 
              }
              log += (unsigned long) Plugin_008_keyBuffer;
              log += F(" Bits: ");
              log += Plugin_008_bitCount;
              addLog(LOG_LEVEL_INFO, log);
            }
            // reset everything
            Plugin_008_keyBuffer = 0;
            Plugin_008_bitCount = 0;
            Plugin_008_timeoutCount = 0;

            if (new_key) sendData(event);
            uint32_t resetTimer = PCONFIG_LONG(1);
            if (resetTimer < 250) resetTimer = 250;
            Scheduler.setPluginTaskTimer(resetTimer, event->TaskIndex, event->Par1);

          //   String info = "";
          //   uint64_t invalue = 0x1234;
          //   uint64_t outvalue = castHexAsDec(invalue);
          //   info.reserve(40);
          //   info += F("Test castHexAsDec(");
          //   info += (double)invalue;
          //   info += F(") => ");
          //   info += (double)outvalue;
          //   addLog(LOG_LEVEL_INFO, info);
          }
        }
        break;
      }
      case PLUGIN_WEBFORM_LOAD:
        {
          byte choice = PCONFIG(0);
          {
            const __FlashStringHelper * options[2];
            options[0] = F("26 Bits");
            options[1] = F("34 Bits");
            int optionValues[2];
            optionValues[0] = 26;
            optionValues[1] = 34;
            addFormSelector(F("Wiegand Type"), F("p008_type"), 2, options, optionValues, choice);
          }
          bool presentHexToDec = PCONFIG(1) == 1;
          addFormCheckBox(F("Present hex as decimal value"), F("p008_hexasdec"), presentHexToDec);
          addFormNote(F("Useful only for numeric keypad input!"));

          bool autoTagRemoval = PCONFIG(2) == 0; // Inverted state!
          addFormCheckBox(F("Automatic Tag removal"), F("p008_autotagremoval"), autoTagRemoval);

          if (PCONFIG_LONG(1) == 0) PCONFIG_LONG(1) = 500; // Defaulty 500 mSec (was hardcoded value)
          addFormNumericBox(F("Automatic Tag removal after"),F("p008_removaltimeout"), PCONFIG_LONG(1), 250, 60000); // 0.25 to 60 seconds
          addUnit(F("mSec."));

          addFormNumericBox(F("Value to set on Tag removal"),F("p008_removalvalue"), PCONFIG_LONG(0), 0, 2147483647); // Max allowed is int = 0x7FFFFFFF ...

          bool eventOnRemoval = PCONFIG(3) == 1; // Normal state!
          addFormCheckBox(F("Event on Tag removal"), F("p008_sendreset"), eventOnRemoval);

          success = true;
          break;
        }

      case PLUGIN_WEBFORM_SAVE:
        {
          PCONFIG(0)      = getFormItemInt(F("p008_type"));
          PCONFIG(1)      = isFormItemChecked(F("p008_hexasdec")) ? 1 : 0;
          PCONFIG(2)      = isFormItemChecked(F("p008_autotagremoval")) ? 0 : 1; // Inverted logic!
          PCONFIG(3)      = isFormItemChecked(F("p008_sendreset")) ? 1 : 0;
          PCONFIG_LONG(0) = getFormItemInt(F("p008_removalvalue"));
          PCONFIG_LONG(1) = getFormItemInt(F("p008_removaltimeout"));

          success = true;
          break;
        }
  }
  return success;
}

/*********************************************************************/
void Plugin_008_interrupt2()
/*********************************************************************/
{
  // We've received a 1 bit. (bit 0 = high, bit 1 = low)
  Plugin_008_keyBuffer = Plugin_008_keyBuffer << 1;     // Left shift the number (effectively multiplying by 2)
  Plugin_008_keyBuffer += 1;         // Add the 1 (not necessary for the zeroes)
  Plugin_008_bitCount++;         // Increment the bit count
}

/*********************************************************************/
void Plugin_008_interrupt1()
/*********************************************************************/
{
  // We've received a 0 bit. (bit 0 = low, bit 1 = high)
  Plugin_008_keyBuffer = Plugin_008_keyBuffer << 1;     // Left shift the number (effectively multiplying by 2)
  Plugin_008_bitCount++;           // Increment the bit count
}
#endif // USES_P008
