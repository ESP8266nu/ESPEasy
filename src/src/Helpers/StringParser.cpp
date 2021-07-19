#include "StringParser.h"


#include "../../_Plugin_Helper.h"

#include "../DataStructs/TimingStats.h"

#include "../ESPEasyCore/ESPEasyRules.h"
#include "../Commands/GPIO.h"

#include "../Globals/Cache.h"
#include "../Globals/Plugins_other.h"
#include "../Globals/RuntimeData.h"

#include "../Helpers/ESPEasy_math.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/Rules_calculate.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_GPIO.h"
#include "../Helpers/StringParser.h"

#include <Arduino.h>

/********************************************************************************************\
   Parse string template
 \*********************************************************************************************/
String parseTemplate(String& tmpString)
{
  return parseTemplate(tmpString, false);
}

String parseTemplate(String& tmpString, bool useURLencode)
{
  return parseTemplate_padded(tmpString, 0, useURLencode);
}

String parseTemplate_padded(String& tmpString, uint8_t minimal_lineSize)
{
  return parseTemplate_padded(tmpString, minimal_lineSize, false);
}

String parseTemplate_padded(String& tmpString, uint8_t minimal_lineSize, bool useURLencode)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("parseTemplate_padded"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  START_TIMER;

  // Keep current loaded taskSettings to restore at the end.
  uint8_t   currentTaskIndex = ExtraTaskSettings.TaskIndex;
  String newString;

  newString.reserve(minimal_lineSize); // Our best guess of the new size.


  if (parseTemplate_CallBack_ptr != nullptr) {
    parseTemplate_CallBack_ptr(tmpString, useURLencode);
  }
  parseSystemVariables(tmpString, useURLencode);


  int startpos = 0;
  int lastStartpos = 0;
  int endpos = 0;
  String deviceName, valueName, format;

  while (findNextDevValNameInString(tmpString, startpos, endpos, deviceName, valueName, format)) {
    // First copy all upto the start of the [...#...] part to be replaced.
    newString += tmpString.substring(lastStartpos, startpos);

    // deviceName is lower case, so we can compare literal string (no need for equalsIgnoreCase)
    if (deviceName.equals(F("plugin")))
    {
      // Handle a plugin request.
      // For example: "[Plugin#GPIO#Pinstate#N]"
      // The command is stored in valueName & format
      String command;
      command.reserve(valueName.length() + format.length() + 1);
      command  = valueName;
      command += '#';
      command += format;
      command.replace('#', ',');

      if (getGPIOPinStateValues(command)) {
        newString += command;
      }
/* @giig1967g
      if (PluginCall(PLUGIN_REQUEST, 0, command))
      {
        // Do not call transformValue here.
        // The "format" is not empty so must not call the formatter function.
        newString += command;
      }
*/
    }
    else if (deviceName.equals(F("var")) || deviceName.equals(F("int")))
    {
      // Address an internal variable either as float or as int
      // For example: Let,10,[VAR#9]
      unsigned int varNum;

      if (validUIntFromString(valueName, varNum)) {
        unsigned char nr_decimals = maxNrDecimals_double(getCustomFloatVar(varNum));
        bool trimTrailingZeros    = true;

        if (deviceName.equals(F("int"))) {
          nr_decimals = 0;
        } else if (!format.isEmpty())
        {
          // There is some formatting here, so do not throw away decimals
          trimTrailingZeros = false;
        }
        String value = doubleToString(getCustomFloatVar(varNum), nr_decimals, trimTrailingZeros);
        value.trim();
        transformValue(newString, minimal_lineSize, value, format, tmpString);
      }
    }
    else
    {
      // Address a value from a plugin.
      // For example: "[bme#temp]"
      // If value name is unknown, run a PLUGIN_GET_CONFIG command.
      // For example: "[<taskname>#getLevel]"
      taskIndex_t taskIndex = findTaskIndexByName(deviceName);

      if (validTaskIndex(taskIndex) && Settings.TaskDeviceEnabled[taskIndex]) {
        uint8_t valueNr = findDeviceValueIndexByName(valueName, taskIndex);

        if (valueNr != VARS_PER_TASK) {
          // here we know the task and value, so find the uservar
          // Try to format and transform the values
          bool   isvalid;
          String value = formatUserVar(taskIndex, valueNr, isvalid);

          if (isvalid) {
            transformValue(newString, minimal_lineSize, value, format, tmpString);
          }
        } else {
          // try if this is a get config request
          struct EventStruct TempEvent(taskIndex);
          String tmpName = valueName;

          if (PluginCall(PLUGIN_GET_CONFIG, &TempEvent, tmpName))
          {
            transformValue(newString, minimal_lineSize, tmpName, format, tmpString);
          }
        }
      }
    }


    // Conversion is done (or impossible) for the found "[...#...]"
    // Continue with the next one.
    lastStartpos = endpos + 1;
    startpos     = endpos + 1;

    // This may have taken some time, so call delay()
    delay(0);
  }

  // Copy the rest of the string (or all if no replacements were done)
  newString += tmpString.substring(lastStartpos);
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("parseTemplate2"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  // Restore previous loaded taskSettings
  if (currentTaskIndex != 255)
  {
    LoadTaskSettings(currentTaskIndex);
  }

  parseStandardConversions(newString, useURLencode);

  // process other markups as well
  parse_string_commands(newString);

  // padding spaces
  while (newString.length() < minimal_lineSize) {
    newString += ' ';
  }

  STOP_TIMER(PARSE_TEMPLATE_PADDED);
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("parseTemplate3"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  return newString;
}

/********************************************************************************************\
   Transform values
 \*********************************************************************************************/

// Syntax: [task#value#transformation#justification]
// valueFormat="transformation#justification"
void transformValue(
  String      & newString,
  uint8_t          lineSize,
  String        value,
  String      & valueFormat,
  const String& tmpString)
{
  // FIXME TD-er: This function does append to newString and uses its length to perform right aling.
  // Is this the way it is intended to use?
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("transformValue"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  // start changes by giig1967g - 2018-04-20
  // Syntax: [task#value#transformation#justification]
  // valueFormat="transformation#justification"
  if (valueFormat.length() > 0) // do the checks only if a Format is defined to optimize loop
  {
    String valueJust;

    int hashtagIndex = valueFormat.indexOf('#');

    if (hashtagIndex >= 0)
    {
      valueJust   = valueFormat.substring(hashtagIndex + 1); // Justification part
      valueFormat = valueFormat.substring(0, hashtagIndex);  // Transformation part
    }

    // valueFormat="transformation"
    // valueJust="justification"
    if (valueFormat.length() > 0) // do the checks only if a Format is defined to optimize loop
    {
      int logicVal    = 0;
      double valFloat = 0.0;

      if (validDoubleFromString(value, valFloat))
      {
        // to be used for binary values (0 or 1)
        logicVal = static_cast<int>(roundf(valFloat)) == 0 ? 0 : 1;
      } else {
        if (value.length() > 0) {
          logicVal = 1;
        }
      }
      String tempValueFormat = valueFormat;
      {
        const int invertedIndex = tempValueFormat.indexOf('!');

        if (invertedIndex != -1) {
          // We must invert the value.
          logicVal = (logicVal == 0) ? 1 : 0;

          // Remove the '!' from the string.
          tempValueFormat.remove(invertedIndex, 1);
        }
      }

      const int  rightJustifyIndex = tempValueFormat.indexOf('R');
      const bool rightJustify      = rightJustifyIndex >= 0 ? 1 : 0;

      if (rightJustify) {
        tempValueFormat.remove(rightJustifyIndex, 1);
      }

      const int tempValueFormatLength = tempValueFormat.length();

      // Check Transformation syntax
      if (tempValueFormatLength > 0)
      {
        switch (tempValueFormat[0])
        {
          case 'V': // value = value without transformations
            break;
          case 'p': // Password hide using asterisks or custom character: pc
          {
            char maskChar = '*';

            if (tempValueFormatLength > 1)
            {
              maskChar = tempValueFormat[1];
            }

            if (value == F("0")) {
              value = "";
            } else {
              const int valueLength = value.length();

              for (int i = 0; i < valueLength; i++) {
                value[i] = maskChar;
              }
            }
            break;
          }
          case 'O':
            value = logicVal == 0 ? F("OFF") : F(" ON"); // (equivalent to XOR operator)
            break;
          case 'C':
            value = logicVal == 0 ? F("CLOSE") : F(" OPEN");
            break;
          case 'c':
            value = logicVal == 0 ? F("CLOSED") : F("  OPEN");
            break;
          case 'M':
            value = logicVal == 0 ? F("AUTO") : F(" MAN");
            break;
          case 'm':
            value = logicVal == 0 ? F("A") : F("M");
            break;
          case 'H':
            value = logicVal == 0 ? F("COLD") : F(" HOT");
            break;
          case 'U':
            value = logicVal == 0 ? F("DOWN") : F("  UP");
            break;
          case 'u':
            value = logicVal == 0 ? F("D") : F("U");
            break;
          case 'Y':
            value = logicVal == 0 ? F(" NO") : F("YES");
            break;
          case 'y':
            value = logicVal == 0 ? F("N") : F("Y");
            break;
          case 'X':
            value = logicVal == 0 ? F("O") : F("X");
            break;
          case 'I':
            value = logicVal == 0 ? F("OUT") : F(" IN");
            break;
          case 'L':
            value = logicVal == 0 ? F(" LEFT") : F("RIGHT");
            break;
          case 'l':
            value = logicVal == 0 ? F("L") : F("R");
            break;
          case 'Z': // return "0" or "1"
            value = logicVal == 0 ? "0" : "1";
            break;
          case 'D': // Dx.y min 'x' digits zero filled & 'y' decimal fixed digits
          case 'd': // like above but with spaces padding
          {
            int x;
            int y;
            x = 0;
            y = 0;

            switch (tempValueFormatLength)
            {
              case 2: // Dx

                if (isDigit(tempValueFormat[1]))
                {
                  x = (int)tempValueFormat[1] - '0';
                }
                break;
              case 3: // D.y

                if ((tempValueFormat[1] == '.') && isDigit(tempValueFormat[2]))
                {
                  y = (int)tempValueFormat[2] - '0';
                }
                break;
              case 4: // Dx.y

                if (isDigit(tempValueFormat[1]) && (tempValueFormat[2] == '.') && isDigit(tempValueFormat[3]))
                {
                  x = (int)tempValueFormat[1] - '0';
                  y = (int)tempValueFormat[3] - '0';
                }
                break;
              case 1:  // D
              default: // any other combination x=0; y=0;
                break;
            }
            bool trimTrailingZeros = false;
            value = doubleToString(valFloat, y, trimTrailingZeros);
            int indexDot = value.indexOf('.');

            if (indexDot == -1) {
              indexDot = value.length();
            }

            for (uint8_t f = 0; f < (x - indexDot); f++) {
              value = (tempValueFormat[0] == 'd' ? ' ' : '0') + value;
            }
            break;
          }
          case 'F': // FLOOR (round down)
            value = (int)floorf(valFloat);
            break;
          case 'E': // CEILING (round up)
            value = (int)ceilf(valFloat);
            break;
          default:
            value = F("ERR");
            break;
        }

        // Check Justification syntax
        const int valueJustLength = valueJust.length();

        if (valueJustLength > 0) // do the checks only if a Justification is defined to optimize loop
        {
          value.trim();          // remove right justification spaces for backward compatibility

          switch (valueJust[0])
          {
            case 'P': // Prefix Fill with n spaces: Pn

              if (valueJustLength > 1)
              {
                if (isDigit(valueJust[1]))                          // Check Pn where n is between 0 and 9
                {
                  int filler = valueJust[1] - value.length() - '0'; // char '0' = 48; char '9' = 58

                  for (uint8_t f = 0; f < filler; f++) {
                    newString += ' ';
                  }
                }
              }
              break;
            case 'S': // Suffix Fill with n spaces: Sn

              if (valueJustLength > 1)
              {
                if (isDigit(valueJust[1]))                          // Check Sn where n is between 0 and 9
                {
                  int filler = valueJust[1] - value.length() - '0'; // 48

                  for (uint8_t f = 0; f < filler; f++) {
                    value += ' ';
                  }
                }
              }
              break;
            case 'L': // left part of the string

              if (valueJustLength > 1)
              {
                if (isDigit(valueJust[1])) // Check n where n is between 0 and 9
                {
                  value = value.substring(0, (int)valueJust[1] - '0');
                }
              }
              break;
            case 'R': // Right part of the string

              if (valueJustLength > 1)
              {
                if (isDigit(valueJust[1])) // Check n where n is between 0 and 9
                {
                  value = value.substring(std::max(0, (int)value.length() - ((int)valueJust[1] - '0')));
                }
              }
              break;
            case 'U': // Substring Ux.y where x=firstChar and y=number of characters

              if (valueJustLength > 1)
              {
                if (isDigit(valueJust[1]) && (valueJust[2] == '.') && isDigit(valueJust[3]) && (valueJust[1] > '0') && (valueJust[3] > '0'))
                {
                  value = value.substring(std::min((int)value.length(), (int)valueJust[1] - '0' - 1),
                                          (int)valueJust[1] - '0' - 1 + (int)valueJust[3] - '0');
                }
                else
                {
                  newString += F("ERR");
                }
              }
              break;
            case 'C': // Capitalize First Word-Character value (space/period are checked)

              if (value.length() > 0) {
                value.toLowerCase();
                bool nextCapital = true;

                for (uint8_t i = 0; i < value.length(); i++) {
                  if (nextCapital) {
                    value[i] = toupper(value[i]);
                  }
                  nextCapital = (value[i] == ' ' || value[i] == '.'); // Very simple, capitalize-first-after-space/period
                }
              }
              break;
            case 'u': // Uppercase
              value.toUpperCase();
              break;
            case 'l': // Lowercase
              value.toLowerCase();
              break;
            default:
              newString += F("ERR");
              break;
          }
        }
      }

      if (rightJustify)
      {
        int filler = lineSize - newString.length() - value.length() - tmpString.length();

        for (uint8_t f = 0; f < filler; f++) {
          newString += ' ';
        }
      }
      {
#ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String logFormatted = F("DEBUG: Formatted String='");
          logFormatted += newString;
          logFormatted += value;
          logFormatted += '\'';
          addLog(LOG_LEVEL_DEBUG, logFormatted);
        }
#endif // ifndef BUILD_NO_DEBUG
      }
    }
  }

  // end of changes by giig1967g - 2018-04-18

  newString += value;
  {
#ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
      String logParsed = F("DEBUG DEV: Parsed String='");
      logParsed += newString;
      logParsed += '\'';
      addLog(LOG_LEVEL_DEBUG_DEV, logParsed);
    }
#endif // ifndef BUILD_NO_DEBUG
  }
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("transformValue2"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
}

// Find the first (enabled) task with given name
// Return INVALID_TASK_INDEX when not found, else return taskIndex
taskIndex_t findTaskIndexByName(const String& deviceName)
{
  // cache this, since LoadTaskSettings does take some time.
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  auto result = Cache.taskIndexName.find(String(deviceName));
  #else
  auto result = Cache.taskIndexName.find(deviceName);
  #endif


  if (result != Cache.taskIndexName.end()) {
    return result->second;
  }

  for (taskIndex_t taskIndex = 0; taskIndex < TASKS_MAX; taskIndex++)
  {
    if (Settings.TaskDeviceEnabled[taskIndex]) {
      String taskDeviceName = getTaskDeviceName(taskIndex);

      if (!taskDeviceName.isEmpty())
      {
        // Use entered taskDeviceName can have any case, so compare case insensitive.
        if (deviceName.equalsIgnoreCase(taskDeviceName))
        {
          #ifdef USE_SECOND_HEAP
          Cache.taskIndexName[String(deviceName)] = taskIndex;
          #else
          Cache.taskIndexName[deviceName] = taskIndex;
          #endif
          return taskIndex;
        }
      }
    }
  }
  return INVALID_TASK_INDEX;
}

// Find the first device value index of a taskIndex.
// Return VARS_PER_TASK if none found.
uint8_t findDeviceValueIndexByName(const String& valueName, taskIndex_t taskIndex)
{
  const deviceIndex_t deviceIndex = getDeviceIndex_from_TaskIndex(taskIndex);

  if (!validDeviceIndex(deviceIndex)) { return VARS_PER_TASK; }

  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif


  // cache this, since LoadTaskSettings does take some time.
  // We need to use a cache search key including the taskIndex,
  // to allow several tasks to have the same value names.
  String cache_valueName;

  cache_valueName.reserve(valueName.length() + 4);
  cache_valueName  = valueName;
  cache_valueName += '#';        // The '#' cannot exist in a value name, use it in the cache key.
  cache_valueName += taskIndex;
  cache_valueName.toLowerCase(); // No need to store multiple versions of the same entry with only different case.

  auto result = Cache.taskIndexValueName.find(cache_valueName);

  if (result != Cache.taskIndexValueName.end()) {
    return result->second;
  }
  LoadTaskSettings(taskIndex); // Probably already loaded, but just to be sure

  const uint8_t valCount = getValueCountForTask(taskIndex);

  for (uint8_t valueNr = 0; valueNr < valCount; valueNr++)
  {
    // Check case insensitive, since the user entered value name can have any case.
    if (valueName.equalsIgnoreCase(ExtraTaskSettings.TaskDeviceValueNames[valueNr]))
    {
      Cache.taskIndexValueName[cache_valueName] = valueNr;
      return valueNr;
    }
  }
  return VARS_PER_TASK;
}

// Find positions of [...#...] in the given string.
// Only update pos values on success.
// Return true when found.
bool findNextValMarkInString(const String& input, int& startpos, int& hashpos, int& endpos) {
  int tmpStartpos = input.indexOf('[', startpos);

  if (tmpStartpos == -1) { return false; }
  int tmpHashpos = input.indexOf('#', tmpStartpos);

  if (tmpHashpos == -1) { return false; }

  // We found a hash position, check if there is another '[' inbetween.
  for (int i = tmpStartpos; i < tmpHashpos; ++i) {
    if (input[i] == '[') {
      tmpStartpos = i;
    }
  }

  int tmpEndpos = input.indexOf(']', tmpStartpos);

  if (tmpEndpos == -1) { return false; }

  if (tmpHashpos < tmpEndpos) {
    hashpos  = tmpHashpos;
    startpos = tmpStartpos;
    endpos   = tmpEndpos;
    return true;
  }
  return false;
}

// Find [deviceName#valueName] or [deviceName#valueName#format]
// DeviceName and valueName will be returned in lower case.
// Format may contain case sensitive formatting syntax.
bool findNextDevValNameInString(const String& input, int& startpos, int& endpos, String& deviceName, String& valueName, String& format) {
  int hashpos;

  if (!findNextValMarkInString(input, startpos, hashpos, endpos)) { return false; }
  deviceName = input.substring(startpos + 1, hashpos);
  valueName  = input.substring(hashpos + 1, endpos);
  hashpos    = valueName.indexOf('#');

  if (hashpos != -1) {
    // Found an extra '#' in the valueName, will split valueName and format.
    format    = valueName.substring(hashpos + 1);
    valueName = valueName.substring(0, hashpos);
  } else {
    format = "";
  }
  deviceName.toLowerCase();
  valueName.toLowerCase();
  return true;
}

/********************************************************************************************\
   Check to see if a given argument is a valid taskIndex (argc = 0 => command)
 \*********************************************************************************************/
taskIndex_t parseCommandArgumentTaskIndex(const String& string, unsigned int argc)
{
  taskIndex_t taskIndex = INVALID_TASK_INDEX;
  const int   ti        = parseCommandArgumentInt(string, argc);

  if (ti > 0) {
    // Task Index used as argument in commands start at 1.
    taskIndex = static_cast<taskIndex_t>(ti - 1);
  }
  return taskIndex;
}

/********************************************************************************************\
   Get int from command argument (argc = 0 => command)
 \*********************************************************************************************/
int parseCommandArgumentInt(const String& string, unsigned int argc)
{
  int value = 0;

  if (argc > 0) {
    // No need to check for the command (argc == 0)
    String TmpStr;

    if (GetArgv(string.c_str(), TmpStr, argc + 1)) {
      value = CalculateParam(TmpStr);
    }
  }
  return value;
}

/********************************************************************************************\
   Parse a command string to event struct
 \*********************************************************************************************/
void parseCommandString(struct EventStruct *event, const String& string)
{
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("parseCommandString"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  event->Par1 = parseCommandArgumentInt(string, 1);
  event->Par2 = parseCommandArgumentInt(string, 2);
  event->Par3 = parseCommandArgumentInt(string, 3);
  event->Par4 = parseCommandArgumentInt(string, 4);
  event->Par5 = parseCommandArgumentInt(string, 5);
}
