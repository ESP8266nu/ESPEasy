#include "../ESPEasyCore/ESPEasyRules.h"

#include "../Commands/InternalCommands.h"
#include "../DataStructs/TimingStats.h"
#include "../DataTypes/EventValueSource.h"
#include "../ESPEasyCore/ESPEasy_backgroundtasks.h"
#include "../ESPEasyCore/Serial.h"
#include "../Globals/Device.h"
#include "../Globals/EventQueue.h"
#include "../Globals/ExtraTaskSettings.h"
#include "../Globals/Plugins.h"
#include "../Globals/Plugins_other.h"
#include "../Globals/Settings.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/FS_Helper.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/Rules_calculate.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"

#include "../../_Plugin_Helper.h"



#include <math.h>

boolean activeRuleSets[RULESETS_MAX];

String EventToFileName(const String& eventName) {
  int size  = eventName.length();
  int index = eventName.indexOf('=');

  if (index > -1) {
    size = index;
  }
#if defined(ESP8266)
  String fileName = F("rules/");
#endif // if defined(ESP8266)
#if defined(ESP32)
  String fileName = F("/rules/");
#endif // if defined(ESP32)
  fileName += eventName.substring(0, size);
  fileName.replace('#', RULE_FILE_SEPARAROR);
  fileName.toLowerCase();
  return fileName;
}

String FileNameToEvent(const String& fileName) {
#if defined(ESP8266)
  String eventName = fileName.substring(6);
#endif // if defined(ESP8266)
#if defined(ESP32)
  String eventName = fileName.substring(7);
#endif // if defined(ESP32)
  eventName.replace(RULE_FILE_SEPARAROR, '#');
  return eventName;
}

void checkRuleSets() {
  for (uint8_t x = 0; x < RULESETS_MAX; x++) {
#if defined(ESP8266)
    String fileName = F("rules");
#endif // if defined(ESP8266)
#if defined(ESP32)
    String fileName = F("/rules");
#endif // if defined(ESP32)
    fileName += x + 1;
    fileName += F(".txt");

    if (fileExists(fileName)) {
      activeRuleSets[x] = true;
    }
    else {
      activeRuleSets[x] = false;
    }

#ifndef BUILD_NO_DEBUG

    if (Settings.SerialLogLevel == LOG_LEVEL_DEBUG_DEV) {
      serialPrint(fileName);
      serialPrint(" ");
      serialPrintln(String(activeRuleSets[x] ? 0 : 1));
    }
#endif // ifndef BUILD_NO_DEBUG
  }
}

/********************************************************************************************\
   Process next event from event queue
 \*********************************************************************************************/
bool processNextEvent() {
  if (Settings.UseRules)
  {
    String nextEvent;

    if (eventQueue.getNext(nextEvent)) {
      rulesProcessing(nextEvent);
      return true;
    }
  }

  // Just make sure any (accidentally) added or remaining events are not kept.
  eventQueue.clear();
  return false;
}

/********************************************************************************************\
   Rules processing
 \*********************************************************************************************/
void rulesProcessing(const String& event) {
  if (!Settings.UseRules) {
    return;
  }
  START_TIMER
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("rulesProcessing"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
#ifndef BUILD_NO_DEBUG
  unsigned long timer = millis();
#endif // ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("EVENT: ");
    log += event;
    addLog(LOG_LEVEL_INFO, log);
  }

  if (Settings.OldRulesEngine()) {
    for (uint8_t x = 0; x < RULESETS_MAX; x++) {
      if (activeRuleSets[x]) {
        rulesProcessingFile(getRulesFileName(x), event);
      }
    }
  } else {
    #ifdef WEBSERVER_NEW_RULES
    String fileName = EventToFileName(event);

    // if exists processed the rule file
    if (fileExists(fileName)) {
      rulesProcessingFile(fileName, event);
    }
# ifndef BUILD_NO_DEBUG
    else {
      addLog(LOG_LEVEL_DEBUG, String(F("EVENT: ")) + event +
             F(" is ingnored. File ") + fileName +
             F(" not found."));
    }
# endif    // ifndef BUILD_NO_DEBUG
    #endif // WEBSERVER_NEW_RULES
  }

#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("EVENT: ");
    log += event;
    log += F(" Processing time:");
    log += timePassedSince(timer);
    log += F(" milliSeconds");
    addLog(LOG_LEVEL_DEBUG, log);
  }
#endif // ifndef BUILD_NO_DEBUG
  STOP_TIMER(RULES_PROCESSING);
  backgroundtasks();
}

/********************************************************************************************\
   Rules processing
 \*********************************************************************************************/
String rulesProcessingFile(const String& fileName, const String& event) {
  if (!Settings.UseRules || !fileExists(fileName)) {
    return EMPTY_STRING;
  }
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("rulesProcessingFile"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
#ifndef BUILD_NO_DEBUG

  if (Settings.SerialLogLevel == LOG_LEVEL_DEBUG_DEV) {
    serialPrint(F("RuleDebug Processing:"));
    serialPrintln(fileName);
    serialPrintln(F("     flags CMI  parse output:"));
  }
#endif // ifndef BUILD_NO_DEBUG

  static uint8_t nestingLevel = 0;

  nestingLevel++;

  if (nestingLevel > RULES_MAX_NESTING_LEVEL) {
    addLog(LOG_LEVEL_ERROR, F("EVENT: Error: Nesting level exceeded!"));
    nestingLevel--;
    return EMPTY_STRING;
  }

  fs::File f = tryOpenFile(fileName, "r+");
  SPIFFS_CHECK(f, fileName.c_str());

  // Try to get the best possible estimate on line length based on earlier parsing of the rules.
  static size_t longestLineSize = RULES_BUFFER_SIZE;
  String line;
  line.reserve(longestLineSize);
  bool match     = false;
  bool codeBlock = false;
  bool isCommand = false;
  bool condition[RULES_IF_MAX_NESTING_LEVEL];
  bool ifBranche[RULES_IF_MAX_NESTING_LEVEL];
  uint8_t ifBlock     = 0;
  uint8_t fakeIfBlock = 0;

  std::vector<uint8_t> buf;
  buf.resize(RULES_BUFFER_SIZE);

  bool firstNonSpaceRead = false;
  bool commentFound      = false;

  while (f.available()) {
    int len = f.read(&buf[0], RULES_BUFFER_SIZE);

    for (int x = 0; x < len; x++) {
      int data = buf[x];

      switch (static_cast<char>(data))
      {
        case '\n':
        {
          // Line end, parse rule
          line.trim();
          check_rules_line_user_errors(line);
          const size_t lineLength = line.length();

          if (lineLength > longestLineSize) {
            longestLineSize = lineLength;
          }

          if ((lineLength > 0) && !line.startsWith(F("//"))) {
            // Parse the line and extract the action (if there is any)
            String action;
            parseCompleteNonCommentLine(line, event, action, match, codeBlock,
                                        isCommand, condition, ifBranche, ifBlock,
                                        fakeIfBlock);

            if (match) // rule matched for one action or a block of actions
            {
              processMatchedRule(action, event, match, codeBlock,
                                 isCommand, condition, ifBranche, ifBlock, fakeIfBlock);
            }

            backgroundtasks();
          }

          // Prepare for new line
          line = EMPTY_STRING;
          line.reserve(longestLineSize);
          firstNonSpaceRead = false;
          commentFound      = false;
          break;
        }
        case '\r': // Just skip this character
          break;
        case '\t': // tab
        case ' ':  // space
        {
          // Strip leading spaces.
          if (firstNonSpaceRead) {
            line += ' ';
          }
          break;
        }
        case '/':
        {
          if (!commentFound) {
            line += '/';

            if (line.endsWith(F("//"))) {
              // consider the rest of the line a comment
              commentFound = true;
            }
          }
          break;
        }
        default: // Any other character
        {
          firstNonSpaceRead = true;

          if (!commentFound) {
            line += char(data);
          }
          break;
        }
      }
    }
  }

  if (f) {
    f.close();
  }

  nestingLevel--;
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("rulesProcessingFile2"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  return EMPTY_STRING;
}

/********************************************************************************************\
   Strip comment from the line.
   Return true when comment was stripped.
 \*********************************************************************************************/
bool rules_strip_trailing_comments(String& line)
{
  // Strip trailing comments
  int comment = line.indexOf(F("//"));

  if (comment >= 0) {
    line = line.substring(0, comment);
    line.trim();
    return true;
  }
  return false;
}

/********************************************************************************************\
   Test for common mistake
   Return true if mistake was found (and corrected)
 \*********************************************************************************************/
bool rules_replace_common_mistakes(const String& from, const String& to, String& line)
{
  if (line.indexOf(from) == -1) {
    return false; // Nothing replaced
  }

  if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
    String log;
    if (log.reserve(32 + from.length() + to.length() + line.length())) {
      log  = F("Rules (Syntax Error, auto-corrected): '");
      log += from;
      log += F("' => '");
      log += to;
      log += F("' in: '");
      log += line;
      log += '\'';
      addLog(LOG_LEVEL_ERROR, log);
    }
  }
  line.replace(from, to);
  return true;
}

/********************************************************************************************\
   Check for common mistakes
   Return true if nothing strange found
 \*********************************************************************************************/
bool check_rules_line_user_errors(String& line)
{
  bool res = true;

  if (rules_replace_common_mistakes(F("if["), F("if ["), line)) {
    res = false;
  }

  if (rules_replace_common_mistakes(F("if%"), F("if %"), line)) {
    res = false;
  }

  return res;
}

/********************************************************************************************\
   Parse string commands
 \*********************************************************************************************/
bool get_next_inner_bracket(const String& line, int& startIndex, int& closingIndex, char closingBracket)
{
  char openingBracket = closingIndex;

  switch (closingBracket) {
    case ']': openingBracket = '['; break;
    case '}': openingBracket = '{'; break;
    case ')': openingBracket = '('; break;
    default:
      // unknown bracket type
      return false;
  }
  closingIndex = line.indexOf(closingBracket);

  if (closingIndex == -1) { return false; }

  for (int i = closingIndex; i >= 0; --i) {
    if (line[i] == openingBracket) {
      startIndex = i;
      return true;
    }
  }
  return false;
}

bool get_next_argument(const String& fullCommand, int& index, String& argument, char separator)
{
  if (index == -1) {
    return false;
  }
  int newIndex = fullCommand.indexOf(separator, index);

  if (newIndex == -1) {
    argument = fullCommand.substring(index);
  } else {
    argument = fullCommand.substring(index, newIndex);
  }

  if (argument.startsWith(String(separator))) {
    argument = argument.substring(1);
  }

  //  addLog(LOG_LEVEL_INFO, String("get_next_argument: ") + String(index) + " " + fullCommand + " " + argument);
  index = newIndex;

  if (index != -1) {
    ++index;
  }
  return argument.length() > 0;
}

bool parse_bitwise_functions(const String& cmd_s_lower, const String& arg1, const String& arg2, const String& arg3, int64_t& result) {
  #ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("Bitwise: {");
    log += wrapIfContains(cmd_s_lower, ':', '\"');
    log += ':';
    log += wrapIfContains(arg1, ':', '\"');

    if (arg2.length() > 0) {
      log += ':';
      log += wrapIfContains(arg2, ':', '\"');

      if (arg3.length() > 0) {
        log += ':';
        log += wrapIfContains(arg3, ':', '\"');
      }
    }
    log += '}';
    addLog(LOG_LEVEL_DEBUG, log);
  }
  #endif

  if (cmd_s_lower.length() < 2) {
    return false;
  }

  if (cmd_s_lower.startsWith(F("bit"))) {
    #define bitSetULL(value, bit) ((value) |= (1ULL << (bit)))
    #define bitClearULL(value, bit) ((value) &= ~(1ULL << (bit)))
    #define bitWriteULL(value, bit, bitvalue) (bitvalue ? bitSetULL(value, bit) : bitClearULL(value, bit))
    uint32_t bitnr = 0;
    uint64_t iarg2 = 0;

    if (!validUIntFromString(arg1, bitnr) || !validUInt64FromString(arg2, iarg2)) {
      return false;
    }

    if (cmd_s_lower.equals(F("bitread"))) {
      // Syntax like {bitread:0:123} to get a single decimal '1'
      result = bitRead(iarg2, bitnr);
    } else if (cmd_s_lower.equals(F("bitset"))) {
      // Syntax like {bitset:0:122} to set least significant bit of the given nr '122' to '1' => '123'
      result = iarg2;
      bitSetULL(result, bitnr);
    } else if (cmd_s_lower.equals(F("bitclear"))) {
      // Syntax like {bitclear:0:123} to set least significant bit of the given nr '123' to '0' => '122'
      result = iarg2;
      bitClearULL(result, bitnr);
    } else if (cmd_s_lower.equals(F("bitwrite"))) {
      uint32_t iarg3 = 0;

      // Syntax like {bitwrite:0:122:1} to set least significant bit of the given nr '122' to '1' => '123'
      if (validUIntFromString(arg3, iarg3)) {
        const int bitvalue = (iarg3 & 1); // Only use the last bit of the given parameter
        result = iarg2;
        bitWriteULL(result, bitnr, bitvalue);
      } else {
        // Need 3 parameters, but 3rd one is not a valid uint
        return false;
      }
    } else {
      // Starts with "bit", but no matching function found
      return false;
    }

    // all functions starting with "bit" are checked
    return true;
  }

  uint64_t iarg1, iarg2 = 0;

  if (!validUInt64FromString(arg1, iarg1) || !validUInt64FromString(arg2, iarg2)) {
    return false;
  }

  if (cmd_s_lower.equals(F("xor"))) {
    // Syntax like {xor:127:15} to XOR the binary values 1111111 and 1111 => 1110000
    result = iarg1 ^ iarg2;
  } else if (cmd_s_lower.equals(F("and"))) {
    // Syntax like {and:254:15} to AND the binary values 11111110 and 1111 => 1110
    result = iarg1 & iarg2;
  } else if (cmd_s_lower.equals(F("or"))) {
    // Syntax like {or:254:15} to OR the binary values 11111110 and 1111 => 11111111
    result = iarg1 | iarg2;
  } else {
    // No matching function found
    return false;
  }
  return true;
}

bool parse_math_functions(const String& cmd_s_lower, const String& arg1, const String& arg2, const String& arg3, double& result) {
  double farg1;
  float  farg2, farg3 = 0.0f;

  if (!validDoubleFromString(arg1, farg1)) {
    return false;
  }

  if (cmd_s_lower.equals(F("constrain"))) {
    // Contrain a value X to be within range of A to B
    // Syntax like {constrain:x:a:b} to constrain x in range a...b
    if (validFloatFromString(arg2, farg2) && validFloatFromString(arg3, farg3)) {
      if (farg2 > farg3) {
        const float tmp = farg2;
        farg2 = farg3;
        farg3 = tmp;
      }
      result = constrain(farg1, farg2, farg3);
    } else {
      return false;
    }
  } else {
    // No matching function found
    return false;
  }
  return true;
}

void parse_string_commands(String& line) {
  int startIndex, closingIndex;

  while (get_next_inner_bracket(line, startIndex, closingIndex, '}')) {
    // Command without opening and closing brackets.
    String fullCommand = line.substring(startIndex + 1, closingIndex);
    String cmd_s_lower = parseString(fullCommand, 1, ':');
    String arg1        = parseStringKeepCase(fullCommand, 2, ':');
    String arg2        = parseStringKeepCase(fullCommand, 3, ':');
    String arg3        = parseStringKeepCase(fullCommand, 4, ':');

    if (cmd_s_lower.length() > 0) {
      String replacement; // maybe just replace with empty to avoid looping?
      //      addLog(LOG_LEVEL_INFO, String(F("parse_string_commands cmd: ")) + cmd_s_lower + " " + arg1 + " " + arg2 + " " + arg3);

      uint64_t iarg1, iarg2 = 0;
      double   fresult = 0.0;
      int64_t  iresult = 0;

      if (parse_math_functions(cmd_s_lower, arg1, arg2, arg3, fresult)) {
        const bool trimTrailingZeros = true;
        replacement = doubleToString(fresult, maxNrDecimals_double(fresult), trimTrailingZeros);
      } else if (parse_bitwise_functions(cmd_s_lower, arg1, arg2, arg3, iresult)) {
        replacement = ull2String(iresult);
      } else if (cmd_s_lower.equals(F("substring"))) {
        // substring arduino style (first char included, last char excluded)
        // Syntax like 12345{substring:8:12:ANOTHER HELLO WORLD}67890
        int startpos, endpos = -1;

        if (validIntFromString(arg1, startpos)
            && validIntFromString(arg2, endpos)) {
          replacement = arg3.substring(startpos, endpos);
        }
      } else if (cmd_s_lower.equals(F("strtol"))) {
        // string to long integer (from cstdlib)
        // Syntax like 1234{strtol:16:38}7890
        if (validUInt64FromString(arg1, iarg1)
            && validUInt64FromString(arg2, iarg2)) {
          replacement = String(strtoul(arg2.c_str(), NULL, iarg1));
        }

        // FIXME TD-er: removed for now as it is too specific.
        // Maybe introduce one using 2 or 3 parameters ({div:100:255:3} for *100/255 3 decimals)

        /*
           } else if (cmd_s_lower.equals(F("div100ths"))) {
           // division and giving the 100ths as integer
           // 5 / 100 would yield 5
           // useful for fractions that use a full uint8_t gaining a
           // precision/granularity of 1/256 instead of only 1/100
           // Syntax like XXX{div100ths:24:256}XXX
           if (validUInt64FromString(arg1, iarg1)
            && validUInt64FromString(arg2, iarg2)) {
           float val = (100.0 * iarg1) / (1.0 * iarg2);
           char sval[10];
           sprintf_P(sval, PSTR("%02d"), (int)val);
           replacement = String(sval);
           }
         */
      } else if (cmd_s_lower.equals(F("tobin"))) {
        // Convert to binary string
        // Syntax like 1234{tobin:15}7890
        if (validUInt64FromString(arg1, iarg1)) {
          replacement = ull2String(iarg1, BIN);
        }
      } else if (cmd_s_lower.equals(F("tohex"))) {
        // Convert to HEX string
        // Syntax like 1234{tohex:15}7890
        if (validUInt64FromString(arg1, iarg1)) {
          replacement = ull2String(iarg1, HEX);
        }
      } else if (cmd_s_lower.equals(F("ord"))) {
        // Give the ordinal/integer value of the first character of a string
        // Syntax like let 1,{ord:B}
        uint8_t uval = arg1.c_str()[0];
        replacement = String(uval);
      }

      if (replacement.isEmpty()) {
        // part in braces is not a supported command.
        // replace the {} with other characters to mask the braces so we can continue parsing.
        // We have to unmask then after we're finished.
        // See: https://github.com/letscontrolit/ESPEasy/issues/2932#issuecomment-596139096
        replacement = line.substring(startIndex, closingIndex + 1);
        replacement.replace('{', static_cast<char>(0x02));
        replacement.replace('}', static_cast<char>(0x03));
      }

      // Replace the full command including opening and closing brackets.
      line.replace(line.substring(startIndex, closingIndex + 1), replacement);

      /*
         if (replacement.length() > 0) {
         addLog(LOG_LEVEL_INFO, String(F("parse_string_commands cmd: ")) + fullCommand + String(F(" -> ")) + replacement);
         }
       */
    }
  }

  // We now have to check if we did mask some parts and unmask them.
  // Let's hope we don't mess up any Unicode here.
  line.replace(static_cast<char>(0x02), '{');
  line.replace(static_cast<char>(0x03), '}');
}

void replace_EventValueN_Argv(String& line, const String& argString, unsigned int argc)
{
  String eventvalue;

  eventvalue.reserve(16);
  eventvalue = F("%eventvalue");

  if (argc == 0) {
    // Used for compatibility reasons
    // it still needs to call the "1st" argument
    argc = 1;
  } else {
    eventvalue += argc;
  }
  eventvalue += '%';
  String tmpParam;

  if (GetArgv(argString.c_str(), tmpParam, argc)) {
    line.replace(eventvalue, tmpParam);
  }
}

void substitute_eventvalue(String& line, const String& event) {
  if (substitute_eventvalue_CallBack_ptr != nullptr) {
    substitute_eventvalue_CallBack_ptr(line, event);
  }

  if (line.indexOf(F("%eventvalue")) != -1) {
    if (event.charAt(0) == '!') {
      line.replace(F("%eventvalue%"), event); // substitute %eventvalue% with
                                              // literal event string if
                                              // starting with '!'
    } else {
      int equalsPos = event.indexOf('=');

      if (equalsPos > 0) {
        // Replace %eventvalueX% with the actual value of the event.
        // For compatibility reasons also replace %eventvalue%  (argc = 0)
        String argString = event.substring(equalsPos + 1);

        for (unsigned int argc = 0; argc <= 4; ++argc) {
          replace_EventValueN_Argv(line, argString, argc);
        }
      }
    }
  }
}

void parseCompleteNonCommentLine(String& line, const String& event,
                                 String& action, bool& match,
                                 bool& codeBlock, bool& isCommand,
                                 bool condition[], bool ifBranche[],
                                 uint8_t& ifBlock, uint8_t& fakeIfBlock) {
  const bool lineStartsWith_on = line.substring(0, 3).equalsIgnoreCase(F("on "));

  if (!codeBlock && !match) {
    // We're looking for a new code block.
    // Continue with next line if none was found on current line.
    if (!lineStartsWith_on) {
      return;
    }
  }

  isCommand = true;

  rules_strip_trailing_comments(line);

  if (match || !codeBlock) {
    // only parse [xxx#yyy] if we have a matching ruleblock or need to eval the
    // "on" (no codeBlock)
    // This to avoid wasting CPU time...
    if (match && !fakeIfBlock) {
      // substitution of %eventvalue% is made here so it can be used on if
      // statement too
      substitute_eventvalue(line, event);
    }

    if (match || lineStartsWith_on) {
      // Only parseTemplate when we are actually doing something with the line.
      // When still looking for the "on ... do" part, do not change it before we found the block.
      line = parseTemplate(line);
    }
  }


  String lineOrg = line; // store original line for future use

  line.toLowerCase();    // convert all to lower case to make checks easier

  String eventTrigger;

  action = EMPTY_STRING;

  if (!codeBlock) // do not check "on" rules if a block of actions is to be
                  // processed
  {
    if (lineStartsWith_on) {
      ifBlock     = 0;
      fakeIfBlock = 0;
      line        = line.substring(3);
      int split = line.indexOf(F(" do"));

      if (split != -1) {
        eventTrigger = line.substring(0, split);
        action       = lineOrg.substring(split + 7);

        // Remove trailing and leadin spaces on the eventTrigger and action.
        eventTrigger.trim();
        action.trim();
      }

      if (eventTrigger == "*") { // wildcard, always process
        match = true;
      }
      else {
        match = ruleMatch(event, eventTrigger);
      }

      if (action.length() > 0) // single on/do/action line, no block
      {
        isCommand = true;
        codeBlock = false;
      } else {
        isCommand = false;
        codeBlock = true;
      }
    }
  } else {
    action = lineOrg;
  }

  if (action.equalsIgnoreCase(F("endon"))) // Check if action block has ended, then we will
                                           // wait for a new "on" rule
  {
    isCommand   = false;
    codeBlock   = false;
    match       = false;
    ifBlock     = 0;
    fakeIfBlock = 0;
  }

#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG_DEV)) {
    String log = F("RuleDebug: ");
    log += codeBlock ? 0 : 1;
    log += match ? 0 : 1;
    log += isCommand ? 0 : 1;
    log += ": ";
    log += line;
    addLog(LOG_LEVEL_DEBUG_DEV, log);
  }
#endif // ifndef BUILD_NO_DEBUG
}

void processMatchedRule(String& action, const String& event,
                        bool& match, bool& codeBlock,
                        bool& isCommand, bool condition[], bool ifBranche[],
                        uint8_t& ifBlock, uint8_t& fakeIfBlock) {
  String lcAction = action;

  lcAction.toLowerCase();

  if (fakeIfBlock) {
    isCommand = false;
  }
  else if (ifBlock) {
    if (condition[ifBlock - 1] != ifBranche[ifBlock - 1]) {
      isCommand = false;
    }
  }
  int split =
    lcAction.indexOf(F("elseif ")); // check for optional "elseif" condition

  if (split != -1) {
    // Found "elseif" condition
    isCommand = false;

    if (ifBlock && !fakeIfBlock) {
      if (ifBranche[ifBlock - 1]) {
        if (condition[ifBlock - 1]) {
          ifBranche[ifBlock - 1] = false;
        }
        else {
          String check = lcAction.substring(split + 7);
          check.trim();
          condition[ifBlock - 1] = conditionMatchExtended(check);
#ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            String log  = F("Lev.");
            log += String(ifBlock);
            log += F(": [elseif ");
            log += check;
            log += F("]=");
            log += boolToString(condition[ifBlock - 1]);
            addLog(LOG_LEVEL_DEBUG, log);
          }
#endif // ifndef BUILD_NO_DEBUG
        }
      }
    }
  } else {
    // check for optional "if" condition
    split = lcAction.indexOf(F("if "));

    if (split != -1) {
      if (ifBlock < RULES_IF_MAX_NESTING_LEVEL) {
        if (isCommand) {
          ifBlock++;
          String check = lcAction.substring(split + 3);
          check.trim();
          condition[ifBlock - 1] = conditionMatchExtended(check);
          ifBranche[ifBlock - 1] = true;
#ifndef BUILD_NO_DEBUG

          if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            String log  = F("Lev.");
            log += String(ifBlock);
            log += F(": [if ");
            log += check;
            log += F("]=");
            log += boolToString(condition[ifBlock - 1]);
            addLog(LOG_LEVEL_DEBUG, log);
          }
#endif // ifndef BUILD_NO_DEBUG
        } else {
          fakeIfBlock++;
        }
      } else {
        fakeIfBlock++;

        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log  = F("Lev.");
          log += String(ifBlock);
          log += F(": Error: IF Nesting level exceeded!");
          addLog(LOG_LEVEL_ERROR, log);
        }
      }
      isCommand = false;
    }
  }

  if ((lcAction == F("else")) && !fakeIfBlock) // in case of an "else" block of
                                               // actions, set ifBranche to
                                               // false
  {
    ifBranche[ifBlock - 1] = false;
    isCommand              = false;
#ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log  = F("Lev.");
      log += String(ifBlock);
      log += F(": [else]=");
      log += boolToString(condition[ifBlock - 1] == ifBranche[ifBlock - 1]);
      addLog(LOG_LEVEL_DEBUG, log);
    }
#endif // ifndef BUILD_NO_DEBUG
  }

  if (lcAction == F("endif")) // conditional block ends here
  {
    if (fakeIfBlock) {
      fakeIfBlock--;
    }
    else if (ifBlock) {
      ifBlock--;
    }
    isCommand = false;
  }

  // process the action if it's a command and unconditional, or conditional and
  // the condition matches the if or else block.
  if (isCommand) {
    substitute_eventvalue(action, event);

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String actionlog = F("ACT  : ");
      actionlog += action;
      addLog(LOG_LEVEL_INFO, actionlog);
    }

    ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_RULES, action.c_str());
    delay(0);
  }
}

/********************************************************************************************\
   Check if an event matches to a given rule
 \*********************************************************************************************/
bool ruleMatch(const String& event, const String& rule) {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("ruleMatch"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  String tmpEvent = event;
  String tmpRule  = rule;
  tmpEvent.trim();
  tmpRule.trim();

  // Ignore escape char
  tmpRule.replace(F("["), EMPTY_STRING);
  tmpRule.replace(F("]"), EMPTY_STRING);

  if (tmpEvent.equalsIgnoreCase(tmpRule)) {
    return true;
  }


  // Special handling of literal string events, they should start with '!'
  if (event.charAt(0) == '!') {
    const int pos = rule.indexOf('*');

    if (pos != -1) // a * sign in rule, so use a'wildcard' match on message
    {
      return event.substring(0, pos).equalsIgnoreCase(rule.substring(0, pos));
    } else {
      const bool pound_char_found = rule.indexOf('#') != -1;

      if (!pound_char_found)
      {
        // no # sign in rule, use 'wildcard' match on event 'source'
        return event.substring(0, rule.length()).equalsIgnoreCase(rule);
      }
    }
    return tmpEvent.equalsIgnoreCase(tmpRule);
  }

  // clock events need different handling...
  if (event.substring(0, 10).equalsIgnoreCase(F("Clock#Time")))
  {
    int pos1 = event.indexOf('=');
    int pos2 = rule.indexOf('=');

    if ((pos1 > 0) && (pos2 > 0)) {
      if (event.substring(0, pos1).equalsIgnoreCase(rule.substring(0, pos2))) // if this is a clock rule
      {
        unsigned long clockEvent = string2TimeLong(event.substring(pos1 + 1));
        unsigned long clockSet   = string2TimeLong(rule.substring(pos2 + 1));

        return matchClockEvent(clockEvent, clockSet);
      }
    } else {
      // Not supported yet, see: https://github.com/letscontrolit/ESPEasy/issues/2640
      return false;
    }
  }

  // parse event into verb and value
  double value = 0;
  int    pos   = event.indexOf('=');

  if (pos >= 0) {
    if (!validDoubleFromString(event.substring(pos + 1), value)) {
      return false;

      // FIXME TD-er: What to do when trying to match NaN values?
    }
    tmpEvent = event.substring(0, pos);
  }

  // parse rule
  int  posStart, posEnd;
  char compare;

  if (!findCompareCondition(rule, compare, posStart, posEnd)) {
    // No compare condition found, so just check if the event- and rule string match.
    return tmpEvent.equalsIgnoreCase(rule);
  }

  const bool stringMatch = tmpEvent.equalsIgnoreCase(rule.substring(0, posStart));
  double     ruleValue   = 0;

  if (!validDoubleFromString(rule.substring(posEnd), ruleValue)) {
    return false;

    // FIXME TD-er: What to do when trying to match NaN values?
  }

  bool match = false;

  if (stringMatch) {
    match = compareDoubleValues(compare, value, ruleValue);
  }
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("ruleMatch2"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  return match;
}

/********************************************************************************************\
   Check expression
 \*********************************************************************************************/
bool conditionMatchExtended(String& check) {
  int  condAnd   = -1;
  int  condOr    = -1;
  bool rightcond = false;
  bool leftcond  = conditionMatch(check); // initial check

  #ifndef BUILD_NO_DEBUG
  String debugstr;

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    debugstr += boolToString(leftcond);
  }
  #endif // ifndef BUILD_NO_DEBUG

  do {
    #ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String log = F("conditionMatchExtended: ");
      log += debugstr;
      log += ' ';
      log += wrap_String(check, '"');
      addLog(LOG_LEVEL_DEBUG, log);
    }
    #endif // ifndef BUILD_NO_DEBUG
    condAnd = check.indexOf(F(" and "));
    condOr  = check.indexOf(F(" or "));

    if ((condAnd > 0) || (condOr > 0)) {                             // we got AND/OR
      if ((condAnd > 0) && (((condOr < 0) /*&& (condOr < condAnd)*/) ||
                            ((condOr > 0) && (condOr > condAnd)))) { // AND is first
        check     = check.substring(condAnd + 5);
        rightcond = conditionMatch(check);
        leftcond  = (leftcond && rightcond);

        #ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          debugstr += F(" && ");
        }
        #endif // ifndef BUILD_NO_DEBUG
      } else { // OR is first
        check     = check.substring(condOr + 4);
        rightcond = conditionMatch(check);
        leftcond  = (leftcond || rightcond);

        #ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          debugstr += F(" || ");
        }
        #endif // ifndef BUILD_NO_DEBUG
      }

      #ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        debugstr += boolToString(rightcond);
      }
      #endif // ifndef BUILD_NO_DEBUG
    }
  } while (condAnd > 0 || condOr > 0);

  #ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    check = debugstr;
  }
  #endif // ifndef BUILD_NO_DEBUG
  return leftcond;
}

// Find the compare condition.
// @param posStart = first position of the compare condition in the string
// @param posEnd   = first position rest of the string, right after the compare condition.
bool findCompareCondition(const String& check, char& compare, int& posStart, int& posEnd)
{
  posStart = check.length();
  posEnd   = posStart;
  int  comparePos = 0;
  bool found      = false;

  if (((comparePos = check.indexOf(F("!="))) > 0) && (comparePos < posStart)) {
    posStart = comparePos;
    posEnd   = posStart + 2;
    compare  = '<' + '>';
    found    = true;
  }

  if (((comparePos = check.indexOf(F("<>"))) > 0) && (comparePos < posStart)) {
    posStart = comparePos;
    posEnd   = posStart + 2;
    compare  = '<' + '>';
    found    = true;
  }

  if (((comparePos = check.indexOf(F(">="))) > 0) && (comparePos < posStart)) {
    posStart = comparePos;
    posEnd   = posStart + 2;
    compare  = '>' + '=';
    found    = true;
  }

  if (((comparePos = check.indexOf(F("<="))) > 0) && (comparePos < posStart)) {
    posStart = comparePos;
    posEnd   = posStart + 2;
    compare  = '<' + '=';
    found    = true;
  }

  if (((comparePos = check.indexOf(F("=="))) > 0) && (comparePos < posStart)) {
    posStart = comparePos;
    posEnd   = posStart + 2;
    compare  = '=';
    found    = true;
  }

  if (((comparePos = check.indexOf('<')) > 0) && (comparePos < posStart)) {
    posStart = comparePos;
    posEnd   = posStart + 1;
    compare  = '<';
    found    = true;
  }

  if (((comparePos = check.indexOf('>')) > 0) && (comparePos < posStart)) {
    posStart = comparePos;
    posEnd   = posStart + 1;
    compare  = '>';
    found    = true;
  }

  if (((comparePos = check.indexOf('=')) > 0) && (comparePos < posStart)) {
    posStart = comparePos;
    posEnd   = posStart + 1;
    compare  = '=';
    found    = true;
  }
  return found;
}

bool compareIntValues(char compare, const int& Value1, const int& Value2)
{
  switch (compare) {
    case '>' + '=': return Value1 >= Value2;
    case '<' + '=': return Value1 <= Value2;
    case '<' + '>': return Value1 != Value2;
    case '>':       return Value1 > Value2;
    case '<':       return Value1 < Value2;
    case '=':       return Value1 == Value2;
  }
  return false;
}

bool compareDoubleValues(char compare, const double& Value1, const double& Value2)
{
  switch (compare) {
    case '>' + '=': return !definitelyLessThan(Value1, Value2);
    case '<' + '=': return !definitelyGreaterThan(Value1, Value2);
    case '<' + '>': return !essentiallyEqual(Value1, Value2);
    case '>':       return definitelyGreaterThan(Value1, Value2);
    case '<':       return definitelyLessThan(Value1, Value2);
    case '=':       return essentiallyEqual(Value1, Value2);
  }
  return false;
}

void logtimeStringToSeconds(const String& tBuf, int hours, int minutes, int seconds, bool valid)
{
  #ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;
    log  = F("timeStringToSeconds: ");
    log += wrap_String(tBuf, '"');
    log += F(" --> ");
    if (valid) {
      if (hours < 10) log += '0';
      log += hours;
      log += ':';
      if (minutes < 10) log += '0';
      log += minutes;
      log += ':';
      if (seconds < 10) log += '0';
      log += seconds;
    } else {
      log += F("invalid");
    }
    addLog(LOG_LEVEL_DEBUG, log);
  }

  #endif // ifndef BUILD_NO_DEBUG
}

// convert old and new time string to nr of seconds
// return whether it should be considered a time string.
bool timeStringToSeconds(const String& tBuf, int& time_seconds, String& timeString) {
  {
    // Make sure we only check for expected characters
    // e.g. if 10:11:12 > 7:07 and 10:11:12 < 20:09:04
    // Should only try to match "7:07", not "7:07 and 10:11:12" 
    // Or else it will find "7:07:11"
    bool done = false;
    for (uint8_t pos = 0; !done && timeString.length() < 8 && pos < tBuf.length(); ++pos) {
      char c = tBuf[pos];
      if (isdigit(c) || c == ':') {
        timeString += c;
      } else {
        done = true;
      }
    }
  }

  time_seconds = -1;
  int hours   = 0;
  int minutes = 0;
  int seconds = 0;

  int tmpIndex = 0;
  String hours_str, minutes_str, seconds_str;
  bool   validTime = false;

  if (get_next_argument(timeString, tmpIndex, hours_str, ':')) {
    if (validIntFromString(hours_str, hours)) {
      validTime = true;

      if ((hours < 0) || (hours > 24)) {
        validTime = false;
      } else {
        time_seconds = hours * 60 * 60;
      }

      if (validTime && get_next_argument(timeString, tmpIndex, minutes_str, ':')) {
        if (validIntFromString(minutes_str, minutes)) {
          if ((minutes < 0) || (minutes > 59)) {
            validTime = false;
          } else {
            time_seconds += minutes * 60;
          }

          if (validTime && get_next_argument(timeString, tmpIndex, seconds_str, ':')) {
            // New format, only HH:MM:SS
            if (validIntFromString(seconds_str, seconds)) {
              if ((seconds < 0) || (seconds > 59)) {
                validTime = false;
              } else {
                time_seconds += seconds;
              }
            }
          } else {
            // Old format, only HH:MM
          }
        }
      } else {
        // It is a valid time string, but could also be just a numerical.
        // We mark it here as invalid, meaning the 'other' time to compare it to must contain more than just the hour.
        validTime = false;
      }
    }
  }
  logtimeStringToSeconds(timeString, hours, minutes, seconds, validTime);
  return validTime;
}

// Balance the count of parentheses (aka round braces) by adding the missing left or right parentheses, if any
// Returns the number of added parentheses, < 0 is left parentheses added, > 0 is right parentheses added
int balanceParentheses(String& string) {
  int left = 0;
  int right = 0;
  for (unsigned int i = 0; i < string.length(); i++) {
    switch (string[i]) {
      case '(':
        left++;
        break;
      case ')':
        right++;
        break;
    }
  }
  if (left != right) {
    string.reserve(string.length() + abs(right - left)); // Re-allocate max. once
  }
  if (left > right) {
    for (int i = 0; i < left - right; i++) {
      string += ')';
    }
  } else if (right > left) {
    for (int i = 0; i < right - left; i++) {
      string = String(F("(")) + string; // This is quite 'expensive'
    }
  }
  return left - right;
}

bool conditionMatch(const String& check) {
  int  posStart, posEnd;
  char compare;

  if (!findCompareCondition(check, compare, posStart, posEnd)) {
    return false;
  }

  String tmpCheck1 = check.substring(0, posStart);
  String tmpCheck2 = check.substring(posEnd);

  tmpCheck1.trim();
  tmpCheck2.trim();
  double Value1 = 0;
  double Value2 = 0;

  int  timeInSec1 = 0;
  int  timeInSec2 = 0;
  String timeString1, timeString2;
  bool validTime1 = timeStringToSeconds(tmpCheck1, timeInSec1, timeString1);
  bool validTime2 = timeStringToSeconds(tmpCheck2, timeInSec2, timeString2);
  bool result     = false;

  bool compareTimes = false;

  if ((validTime1 || validTime2) && (timeInSec1 != -1) && (timeInSec2 != -1))
  {
    // At least one is a time containing ':' separator
    // AND both can be considered a time, so use it as a time and compare seconds.
    compareTimes = true;
    result       = compareIntValues(compare, timeInSec1, timeInSec2);
    tmpCheck1    = timeString1;
    tmpCheck2    = timeString2;
  } else {
    int condAnd = tmpCheck2.indexOf(F(" and "));
    int condOr  = tmpCheck2.indexOf(F(" or "));
    if (condAnd > -1 || condOr > -1) {            // Only parse first condition, rest will be parsed 'later'
      if (condAnd > -1 && (condOr == -1 || condAnd < condOr)) {
        tmpCheck2 = tmpCheck2.substring(0, condAnd);
      } else if (condOr > -1) {
        tmpCheck2 = tmpCheck2.substring(0, condOr);
      }
      tmpCheck2.trim();
    }
    balanceParentheses(tmpCheck1);
    balanceParentheses(tmpCheck2);
    if (isError(Calculate(tmpCheck1, Value1)) ||
        isError(Calculate(tmpCheck2, Value2)))
    {
      return false;
    }
    result = compareDoubleValues(compare, Value1, Value2);
  }

  #ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("conditionMatch: ");
    log += wrap_String(check, '"');
    log += F(" --> ");

    log += wrap_String(tmpCheck1, '"');
    log += wrap_String(check.substring(posStart, posEnd), ' '); // Compare
    log += wrap_String(tmpCheck2, '"');

    log += F(" --> ");
    log += boolToString(result);
    log += ' ';

    log += '(';
    const bool trimTrailingZeros = true;
    log += compareTimes ? String(timeInSec1) : doubleToString(Value1, 6, trimTrailingZeros);
    log += wrap_String(check.substring(posStart, posEnd), ' '); // Compare
    log += compareTimes ? String(timeInSec2) : doubleToString(Value2, 6, trimTrailingZeros);
    log += ')';
    addLog(LOG_LEVEL_DEBUG, log);
  }
  #else // ifndef BUILD_NO_DEBUG
  (void)compareTimes; // To avoid compiler warning
  #endif // ifndef BUILD_NO_DEBUG
  return result;
}

/********************************************************************************************\
   Generate rule events based on task refresh
 \*********************************************************************************************/
void createRuleEvents(struct EventStruct *event) {
  if (!Settings.UseRules) {
    return;
  }
  const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(event->TaskIndex);

  if (!validDeviceIndex(DeviceIndex)) { return; }

  #ifdef USE_SECOND_HEAP
  HeapSelectIram ephemeral;  
  #endif


  LoadTaskSettings(event->TaskIndex);

  const uint8_t valueCount = getValueCountForTask(event->TaskIndex);

  // Small optimization as sensor type string may result in large strings
  // These also only yield a single value, so no need to check for combining task values.
  if (event->sensorType == Sensor_VType::SENSOR_TYPE_STRING) {
    size_t expectedSize = 2 + getTaskDeviceName(event->TaskIndex).length();
    expectedSize += strlen(ExtraTaskSettings.TaskDeviceValueNames[0]);
   
    bool appendCompleteStringvalue = false;
    String eventString;

    if (eventString.reserve(expectedSize + event->String2.length())) {
      appendCompleteStringvalue = true;
    } else if (!eventString.reserve(expectedSize + 24)) {
      // No need to continue as we can't even allocate the event, we probably also cannot process it
      addLog(LOG_LEVEL_ERROR, F("Not enough memory for event"));
      return;
    }
    eventString  = getTaskDeviceName(event->TaskIndex);
    eventString += F("#");
    eventString += ExtraTaskSettings.TaskDeviceValueNames[0];
    eventString += F("=");
    eventString += '`';
    if (appendCompleteStringvalue) {
      eventString += event->String2;
    } else {
      eventString += event->String2.substring(0, 10);
      eventString += F("...");
      eventString += event->String2.substring(event->String2.length() - 10);
    }
    eventString += '`';
    eventQueue.addMove(std::move(eventString));    
  } else if (Settings.CombineTaskValues_SingleEvent(event->TaskIndex)) {
    String eventString;
    eventString.reserve(128); // Enough for most use cases, prevent lots of memory allocations.
    eventString  = getTaskDeviceName(event->TaskIndex);
    eventString += F("#All=");

    for (uint8_t varNr = 0; varNr < valueCount; varNr++) {
      if (varNr != 0) {
        eventString += ',';
      }
      eventString += formatUserVarNoCheck(event, varNr);
    }
    eventQueue.addMove(std::move(eventString));
  } else {
    for (uint8_t varNr = 0; varNr < valueCount; varNr++) {
      String eventString;
      eventString.reserve(64); // Enough for most use cases, prevent lots of memory allocations.
      eventString  = getTaskDeviceName(event->TaskIndex);
      eventString += F("#");
      eventString += ExtraTaskSettings.TaskDeviceValueNames[varNr];
      eventString += F("=");
      eventString += formatUserVarNoCheck(event, varNr);
      eventQueue.addMove(std::move(eventString));
    }
  }
}
