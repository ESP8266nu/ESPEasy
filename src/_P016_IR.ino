#include "_Plugin_Helper.h"
#ifdef USES_P016

// #######################################################################################################
// #################################### Plugin 016: Input IR #############################################
// #######################################################################################################

// Usage: Connect a TSOP module, preferably a 38Khz one, preferably to GPIO14 (D5)
// On the device tab add a new device and select "Communication - TSOP4838"
// Enable the device and select the GPIO pin
// Power on the ESP and connect to it
// By monitoring the serial or the web log (Tools -> Log) and then pressing a button on a remote
// you will get as output the replay solutions that where found.
// Typicaly solutions are given by the IRremoteESP8266 library (example IRSEND,NEC,ASDFZCV,32)
// but if no replay solutions are found by this library then a RAW2 solution in computed (example IRSEND,RAW2,ASDFASDFASDFASDF,38,50,40)
// If RAW2 also fails, then in the serial monitor there are dumped the raw IR data timings
// that can be used to calculate a RAW solution with use of GDocs for this purpose.
//
// IF the IR code is an Air Condition protocol that the  IR library can decode, then there will be a human-readable description of that IR
// message.
// If the IR library can encode those kind of messages then a JSON formated command will be given, that can be replayed by P035 as well.
// That commands format is:
// IRSENDAC,'{"protocol":"COOLIX","power":"on","mode":"dry","fanspeed":"auto","temp":22,"swingv":"max","swingh":"off"}'
# include <ArduinoJson.h>
# include <IRremoteESP8266.h>
# include <IRutils.h>
# include <IRrecv.h>
# include "src/Helpers/Memory.h"

# include "src/PluginStructs/P016_data_struct.h"

# include "src/ESPEasyCore/Serial.h"

# ifdef P016_P035_Extended_AC
#  include <IRac.h>
# endif // ifdef P016_P035_Extended_AC

# define PLUGIN_016
# define PLUGIN_ID_016 16
# define PLUGIN_NAME_016 "Communication - TSOP4838"
# define PLUGIN_VALUENAME1_016 "IR"
# define P016_CMDINHIBIT       PCONFIG(1)
# define P016_SETTINGS_VERSION PCONFIG(7) // 0 = V1, 2 = V2

# ifndef P016_SEND_IR_TO_CONTROLLER
#  define P016_SEND_IR_TO_CONTROLLER false
# endif // ifndef P016_SEND_IR_TO_CONTROLLER

# define P016_MAX_DECODETYPES 97 // The number of decodeTypes supported by IRrecv, actual value is logged during PLUGIN_WEBFORM_LOAD when
                                 // PLUGIN_016_DEBUG is on

// History
// @tonhuisman: 2021-07-20
// CHG: Merge in branch 'mega', minor changes, formatted source using Uncrustify
// @tonhuisman: 2021-06-05
// CHG: Move internal settings from fixed array to std::vector, and change saving to file to separate chunks (to try avoiding stack
// overflows)
// @tonhuisman: 2021-05-24
// CHG: Added support for 64 bit IR codes, with DecodeType and Repeat separated from the Code/AlternativeCode in settings (V2)
// CHG: includes conversion of the old V1 to the new V2 settings format, after first save of V2 settings further conversion is skipped
//      ! NB: If current stored codes are longer than previously supported 23 bits, conversion will not be correct!
// CHG: new layout for settings
// @tonhuisman: 2021-05-23
// CHG: use hexToUL() instead of strtol() for hex to (unsigned) long conversions
// CHG: some String optimizations
// @uwekaditz: 2020-10-19
// CHG: reduce memory usage when plugin not used
// NEW: Inhibit time between executing the same command
// CHG: ressouce-saving string calculation
// CHG: automatic adding of new IR codes is disabled after boot up
// @uwekaditz: 2020-10-17
// NEW: received valid IR code can be saved and a command can be assigned to it, the command is submitted if the code is received again

// A lot of the following code has been taken directly (with permission) from the IRrecvDumpV2.ino example code
// of the IRremoteESP8266 library. (https://github.com/markszabo/IRremoteESP8266)

// ==================== start of TUNEABLE PARAMETERS ====================
// As this program is a special purpose capture/decoder, let us use a larger
// than normal buffer so we can handle Air Conditioner remote codes.
const uint16_t kCaptureBufferSize = 1024;

// kTimeout is the Nr. of milli-Seconds of no-more-data before we consider a
// message ended.
// This parameter is an interesting trade-off. The longer the timeout, the more
// complex a message it can capture. e.g. Some device protocols will send
// multiple message packets in quick succession, like Air Conditioner remotes.
// Air Coniditioner protocols often have a considerable gap (20-40+ms) between
// packets.
// The downside of a large timeout value is a lot of less complex protocols
// send multiple messages when the remote's button is held down. The gap between
// them is often also around 20+ms. This can result in the raw data be 2-3+
// times larger than needed as it has captured 2-3+ messages in a single
// capture. Setting a low timeout value can resolve this.
// So, choosing the best kTimeout value for your use particular case is
// quite nuanced. Good luck and happy hunting.
// NOTE: Don't exceed kMaxTimeoutMs. Typically 130ms.
// #if DECODE_AC
// Some A/C units have gaps in their protocols of ~40ms. e.g. Kelvinator
// A value this large may swallow repeats of some protocols
const uint8_t P016_TIMEOUT = 50;

// #else   // DECODE_AC
// Suits most messages, while not swallowing many repeats.
// const uint8_t P016_TIMEOUT = 15;
// #endif  // DECODE_AC
// Alternatives:
// const uint8_t kTimeout = 90;
// Suits messages with big gaps like XMP-1 & some aircon units, but can
// accidentally swallow repeated messages in the rawData[] output.
//
// const uint8_t kTimeout = kMaxTimeoutMs;
// This will set it to our currently allowed maximum.
// Values this high are problematic because it is roughly the typical boundary
// where most messages repeat.
// e.g. It will stop decoding a message and start sending it to serial at
//      precisely the time when the next message is likely to be transmitted,
//      and may miss it.

// Set the smallest sized "UNKNOWN" message packets we actually care about.
// This value helps reduce the false-positive detection rate of IR background
// noise as real messages. The chances of background IR noise getting detected
// as a message increases with the length of the kTimeout value. (See above)
// The downside of setting this message too large is you can miss some valid
// short messages for protocols that this library doesn't yet decode.
//
// Set higher if you get lots of random short UNKNOWN messages when nothing
// should be sending a message.
// Set lower if you are sure your setup is working, but it doesn't see messages
// from your device. (e.g. Other IR remotes work.)
// NOTE: Set this value very high to effectively turn off UNKNOWN detection.
const uint16_t kMinUnknownSize = 12;

// ==================== end of TUNEABLE PARAMETERS ====================

IRrecv *irReceiver          = nullptr;
bool    bEnableIRcodeAdding = false;
# ifdef P016_P035_USE_RAW_RAW2

/* *INDENT-OFF* */
boolean displayRawToReadableB32Hex(String& outputStr, decode_results results);
/* *INDENT-ON* */
# endif // ifdef P016_P035_USE_RAW_RAW2

# ifdef PLUGIN_016_DEBUG
void P016_infoLogMemory(const String& text) {
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(37 + text.length());
    log  = F("P016: Free memory ");
    log += text;
    log += F(": ");
    log += FreeMem();
    log += F(" stack: ");
    log += getCurrentFreeStack();
    addLog(LOG_LEVEL_INFO, log);
  }
}

# endif // ifdef PLUGIN_016_DEBUG

boolean Plugin_016(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number = PLUGIN_ID_016;
      Device[deviceCount].Type     = DEVICE_TYPE_SINGLE;

      if (P016_SEND_IR_TO_CONTROLLER) {
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_STRING;
      } else {
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_LONG;
      }
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = true;
      Device[deviceCount].InverseLogicOption = true;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 1;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_016);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_016));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_input(F("IR"));
      break;
    }

    case PLUGIN_INIT:
    {
      # ifdef PLUGIN_016_DEBUG
      addLog(LOG_LEVEL_INFO, F("P016_PLUGIN_INIT ..."));
      # endif // PLUGIN_016_DEBUG

      initPluginTaskData(event->TaskIndex, new (std::nothrow) P016_data_struct());
      P016_data_struct *P016_data =
        static_cast<P016_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P016_data) {
        return success;
      }
      P016_data->init(event, P016_CMDINHIBIT);

      int irPin = CONFIG_PIN1;

      if ((irReceiver == nullptr) && (irPin != -1))
      {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          addLog(LOG_LEVEL_INFO, F("INIT: IR RX"));
          addLog(LOG_LEVEL_INFO, F("IR lib Version: " _IRREMOTEESP8266_VERSION_));
        }
        irReceiver = new IRrecv(irPin, kCaptureBufferSize, P016_TIMEOUT, true);
        irReceiver->setUnknownThreshold(kMinUnknownSize); // Ignore messages with less than minimum on or off pulses.
        irReceiver->enableIRIn();                         // Start the receiver
      }

      if ((nullptr != irReceiver) && (irPin == -1))
      {
        irReceiver->disableIRIn();
        delete irReceiver;
        irReceiver = nullptr;
      }

      # ifdef PLUGIN_016_DEBUG
      addLog(LOG_LEVEL_INFO, F("P016_PLUGIN_INIT done"));
      # endif // PLUGIN_016_DEBUG

      success = true;
      break;
    }
    case PLUGIN_EXIT:
    {
      # ifdef PLUGIN_016_DEBUG
      addLog(LOG_LEVEL_INFO, F("P016_PLUGIN_EXIT ..."));
      # endif // PLUGIN_016_DEBUG

      if (nullptr != irReceiver)
      {
        irReceiver->disableIRIn(); // Stop the receiver
        delete irReceiver;
        irReceiver = nullptr;
      }

      # ifdef PLUGIN_016_DEBUG
      addLog(LOG_LEVEL_INFO, F("P016_PLUGIN_EXIT done"));
      # endif // PLUGIN_016_DEBUG
      success = true;
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      # ifdef PLUGIN_016_DEBUG
      addLog(LOG_LEVEL_INFO, F("P016_PLUGIN_SET_DEFAULTS ..."));
      # endif // PLUGIN_016_DEBUG

      P016_SETTINGS_VERSION = P16_SETTINGS_LATEST; // New installs don't need conversion
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      # ifdef PLUGIN_016_DEBUG
      addLog(LOG_LEVEL_INFO, F("P016_PLUGIN_WEBFORM_LOAD ..."));
      P016_infoLogMemory(F("before load"));
      # endif // PLUGIN_016_DEBUG

      addRowLabel(F("Info"));
      addHtml(F("Check serial or web log for replay solutions via Communication - IR Transmit plugin"));

      addFormSubHeader(F("Content"));

      bool bAddNewCode = bitRead(PCONFIG_LONG(0), P016_BitAddNewCode);
      addFormCheckBox(F("Add new received code to command lines"), F("p016_AddNewCode"), bAddNewCode);
      bool bExecuteCmd = bitRead(PCONFIG_LONG(0), P016_BitExecuteCmd);
      addFormCheckBox(F("Execute commands"),                       F("p016_ExecuteCmd"), bExecuteCmd);
      addFormNumericBox(F("Inhibit time for the same command [ms]"),
                        F("p016_cmdinhibit"),
                        P016_CMDINHIBIT,
                        1,
                        2000);

      {
        // For load and save of the display lines, we must not rely on the data in memory.
        // This data in memory can be altered through write commands.
        // Therefore we must read the lines from flash in a temporary object.
        P016_data_struct *P016_data = new (std::nothrow) P016_data_struct();

        if (nullptr != P016_data) {
          P016_data->loadCommandLines(event); // load saved codes and commands

          addFormSubHeader(F("Code - command map"));

          int size = static_cast<int>(decode_type_t::kLastDecodeType) + 1;
          # ifdef PLUGIN_016_DEBUG
          String log;
          log.reserve(35);
          log  = F("P016: available decodetypes: ");
          log += size;

          if (size > P016_MAX_DECODETYPES) {
            log += F(" #define P016_MAX_DECODETYPES should be updated, currently:");
            log += P016_MAX_DECODETYPES;
          }
          addLog(LOG_LEVEL_INFO, log);
          # endif // ifdef PLUGIN_016_DEBUG

          // Fill an array with all supported decode_type_t
          String decodeTypes[P016_MAX_DECODETYPES];
          int    decodeTypeOptions[P016_MAX_DECODETYPES];

          for (int i = 0; i < size && i < P016_MAX_DECODETYPES; i++) {
            decodeTypeOptions[i] = i;
            decodeTypes[i]       = typeToString(static_cast<decode_type_t>(i), false);

            // addLog(LOG_LEVEL_INFO,typeToString(static_cast<decode_type_t>(i), false)); // For debugging purposes
            delay(0);
          }
          const String P016_HEX_INPUT_PATTERN = F("(0x)?[0-9a-fA-F]{0,16}"); // 16 nibbles = 64 bit, 0x prefix is allowed but not added by
                                                                             // default

          String strCode;
          strCode.reserve(20);

          addRowLabel(F("Code - command map"));

          html_table(EMPTY_STRING);
          html_table_header(F("&nbsp;#&nbsp;"));
          html_table_header(F("Decode type"));
          html_table_header(F("Repeat"));
          html_table_header(F("Code [Hex]"));
          html_table_header(F("Alt. Decode type"));
          html_table_header(F("Repeat"));
          html_table_header(F("Alt. Code [Hex]"));

          int rowCnt = 0;

          for (uint8_t varNr = 0; varNr < P16_Nlines; varNr++) {
            html_TR_TD();

            if (varNr < 9) {
              addHtml(F("&nbsp;"));
            }
            addHtmlInt(varNr + 1); // #
            html_TD();
            {                      // Decode type
              addSelector(getPluginCustomArgName(rowCnt + 0), size, decodeTypes, decodeTypeOptions, NULL,
                          static_cast<int>(P016_data->CommandLines[varNr].CodeDecodeType), false, true, EMPTY_STRING);
            }
            html_TD();
            addCheckBox(getPluginCustomArgName(rowCnt + 1), bitRead(P016_data->CommandLines[varNr].CodeFlags, P16_FLAGS_REPEAT));
            html_TD();
            strCode = EMPTY_STRING;

            if (P016_data->CommandLines[varNr].Code > 0) {
              strCode = uint64ToString(P016_data->CommandLines[varNr].Code, 16); // convert code to hex for display
            }
            addTextBox(getPluginCustomArgName(rowCnt + 2), strCode, P16_Cchars - 1, false, false, P016_HEX_INPUT_PATTERN, EMPTY_STRING);

            html_TD();
            {
              addSelector(getPluginCustomArgName(rowCnt + 3), size, decodeTypes, decodeTypeOptions, NULL,
                          static_cast<int>(P016_data->CommandLines[varNr].AlternativeCodeDecodeType), false, true, EMPTY_STRING);
            }
            html_TD();
            addCheckBox(getPluginCustomArgName(rowCnt + 4), bitRead(P016_data->CommandLines[varNr].AlternativeCodeFlags, P16_FLAGS_REPEAT));
            html_TD();
            strCode = EMPTY_STRING;

            if (P016_data->CommandLines[varNr].AlternativeCode > 0) {
              strCode = uint64ToString(P016_data->CommandLines[varNr].AlternativeCode, 16); // convert code to hex for display
            }
            addTextBox(getPluginCustomArgName(rowCnt + 5), strCode, P16_Cchars - 1, false, false, P016_HEX_INPUT_PATTERN, EMPTY_STRING);

            html_TR();                                                   // Separate row for the command input

            addHtml(F("<TD colspan=\"2\" style=\"text-align:right\">")); // Align label to right with the input field
            addHtml(F("Command "));
            addHtmlInt(varNr + 1);
            addHtml(':');
            addHtml(F("<TD colspan=\"5\">")); // Use as much of available width (though limited to 500px by css)
            addTextBox(getPluginCustomArgName(rowCnt + 6), String(P016_data->CommandLines[varNr].Command), P16_Nchars - 1);

            rowCnt += 7;
            delay(0);
          }
          html_end_table();

          // Need to delete the allocated object here
          delete P016_data;
        }

        if (P016_SETTINGS_VERSION != P16_SETTINGS_LATEST) {
          addFormNote(F("These settings are converted from a previous version and will be stored in updated format when submitted."));
        }
      }

      # ifdef PLUGIN_016_DEBUG
      P016_infoLogMemory(F("after load"));
      addLog(LOG_LEVEL_INFO, F("P016_PLUGIN_WEBFORM_LOAD done"));
      # endif // PLUGIN_016_DEBUG

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      # ifdef PLUGIN_016_DEBUG
      addLog(LOG_LEVEL_INFO, F("P016_PLUGIN_WEBFORM_SAVE ..."));
      # endif // PLUGIN_016_DEBUG

      P016_SETTINGS_VERSION = P16_SETTINGS_LATEST; // Set to use the current settings version.

      // update now
      Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);

      uint32_t lSettings = 0;
      bitWrite(lSettings, P016_BitAddNewCode, isFormItemChecked(F("p016_AddNewCode")));
      bitWrite(lSettings, P016_BitExecuteCmd, isFormItemChecked(F("p016_ExecuteCmd")));

      bEnableIRcodeAdding = true;
      PCONFIG_LONG(0)     = lSettings;
      P016_CMDINHIBIT     = getFormItemInt(F("p016_cmdinhibit"));

      {
        // For load and save of the display lines, we must not rely on the data in memory.
        // This data in memory can be altered through write commands.
        // Therefore we must use a temporary version to store the settings.
        P016_data_struct *P016_data = new (std::nothrow) P016_data_struct();

        if (nullptr != P016_data) {
          char   strCode[P16_Cchars];
          String strError;
          strError.reserve(30); // Length of expected string, needed for strings > 11 chars
          String   strID;
          uint64_t iCode;

          int rowCnt = 0;

          P016_data->CommandLines.clear(); // Start fresh

          for (uint8_t varNr = 0; varNr < P16_Nlines; varNr++) {
            P016_data->CommandLines.push_back(tCommandLinesV2());
            strError = EMPTY_STRING;

            // Normal Code & flags
            P016_data->CommandLines[varNr].CodeDecodeType = static_cast<decode_type_t>(getFormItemInt(getPluginCustomArgName(rowCnt + 0)));
            bitWrite(P016_data->CommandLines[varNr].CodeFlags, P16_FLAGS_REPEAT, isFormItemChecked(getPluginCustomArgName(rowCnt + 1)));
            iCode = 0;

            if (!safe_strncpy(strCode, webArg(getPluginCustomArgName(rowCnt + 2)), P16_Cchars)) {
              strError += F("Code ");
              strError += (varNr + 1);
              strError += ' ';
            } else {
              iCode = hexToULL(strCode); // convert string with hexnumbers to uint64_t
            }
            P016_data->CommandLines[varNr].Code = iCode;

            delay(0);

            // Alternate Code & flags
            P016_data->CommandLines[varNr].AlternativeCodeDecodeType =
              static_cast<decode_type_t>(getFormItemInt(getPluginCustomArgName(rowCnt + 3)));
            bitWrite(P016_data->CommandLines[varNr].AlternativeCodeFlags, P16_FLAGS_REPEAT,
                     isFormItemChecked(getPluginCustomArgName(rowCnt + 4)));
            iCode = 0;

            if (!safe_strncpy(strCode, webArg(getPluginCustomArgName(rowCnt + 5)), P16_Cchars)) {
              strError += F("Alt.Code ");
              strError += (varNr + 1);
              strError += ' ';
            } else {
              iCode = hexToULL(strCode); // convert string with hexnumbers to uint64_t
            }
            P016_data->CommandLines[varNr].AlternativeCode = iCode;

            // Command
            if (!safe_strncpy(P016_data->CommandLines[varNr].Command, webArg(getPluginCustomArgName(rowCnt + 6)), P16_Nchars)) {
              strError += F("Command ");
              strError += (varNr + 1);
            }
            P016_data->CommandLines[varNr].Command[P16_Nchars - 1] = 0; // Terminate string

            if (!strError.isEmpty()) {
              addHtmlError(strError);
            }

            rowCnt += 7;
            delay(0);
          }

          # ifdef PLUGIN_016_DEBUG
          P016_infoLogMemory(F("before save"));
          # endif // ifdef PLUGIN_016_DEBUG

          P016_data->saveCommandLines(event);

          # ifdef PLUGIN_016_DEBUG
          P016_infoLogMemory(F("after save"));
          # endif // ifdef PLUGIN_016_DEBUG

          // Need to delete the allocated object here
          delete P016_data;
        }
      }

      # ifdef PLUGIN_016_DEBUG
      addLog(LOG_LEVEL_INFO, F("P016_PLUGIN_WEBFORM_SAVE Done"));
      # endif // PLUGIN_016_DEBUG
      success = true;
      break;
    }

    case PLUGIN_ONCE_A_SECOND:
    {
      P016_data_struct *P016_data =
        static_cast<P016_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P016_data) {
        if (P016_data->bCodeChanged) { // code has been added -> SaveCustomTaskSettings
          P016_data->saveCommandLines(event);
          P016_data->bCodeChanged = false;
          # ifdef PLUGIN_016_DEBUG
          addLog(LOG_LEVEL_INFO, F("P016_PLUGIN_ONCE_A_SECOND CustomTaskSettings Saved"));
          # endif // PLUGIN_016_DEBUG
        }
      }
      success = true;
      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      decode_results results;

      if (irReceiver->decode(&results))
      {
        yield(); // Feed the WDT after a time expensive decoding procedure

        if (results.overflow)
        {
          addLog(LOG_LEVEL_INFO, F("IR: WARNING, IR code is too big for buffer. Try pressing the transmiter button only momenteraly"));
          success = false;
          break;             // Do not continue and risk hanging the ESP
        }
        String output;
        output.reserve(100); // Length of expected string, needed for strings > 11 chars

        // Display the basic output of what we found.
        if (results.decode_type != decode_type_t::UNKNOWN)
        {
          // String output = String(F("IRSEND,")) + typeToString(results.decode_type, results.repeat) + ',' + resultToHexidecimal(&results)
          // + ',' + uint64ToString(results.bits);
          // addLog(LOG_LEVEL_INFO, output); //Show the appropriate command to the user, so he can replay the message via P035 // Old style
          // command
          output  = F("{\"protocol\":\"");
          output += typeToString(results.decode_type, results.repeat);
          output += F("\",\"data\":\"");
          output += resultToHexidecimal(&results);
          output += F("\",\"bits\":");
          output += uint64ToString(results.bits);
          output += '}';

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String Log;
            Log.reserve(output.length() + 22);
            Log  = F("IRSEND,\'");
            Log += output;
            Log += F("\' type: 0x");
            Log += uint64ToString(results.decode_type);
            addLog(LOG_LEVEL_INFO, Log); // JSON representation of the command
          }
          event->String2 = output;

          // Check if this is a code we have a command for or we have to add
          P016_data_struct *P016_data =
            static_cast<P016_data_struct *>(getPluginTaskData(event->TaskIndex));

          if (nullptr != P016_data) {
            // convert result to uint64_t and 2x uint16_t
            uint64_t iCode                = 0;
            decode_type_t iCodeDecodeType = results.decode_type;    //
            uint16_t iCodeFlags           = 0;
            bitWrite(iCodeFlags, P16_FLAGS_REPEAT, results.repeat); //
            String strCode = resultToHexidecimal(&results);

            if (strCode.length() <= P16_Cchars) {
              iCode += hexToULL(strCode);

              if (bitRead(PCONFIG_LONG(0), P016_BitAddNewCode) && bEnableIRcodeAdding) {
                P016_data->AddCode(iCode, iCodeDecodeType, iCodeFlags); // add code if not saved so far
              }

              if (bitRead(PCONFIG_LONG(0), P016_BitExecuteCmd)) {
                P016_data->ExecuteCode(iCode, iCodeDecodeType, iCodeFlags); // execute command for code if available
              }
            }
          }
        }

        // Check if a solution for RAW2 is found and if not give the user the option to access the timings info.
        # ifdef P016_P035_USE_RAW_RAW2

        if ((results.decode_type == decode_type_t::UNKNOWN) && !displayRawToReadableB32Hex(event->String2, results))
        # else // ifdef P016_P035_USE_RAW_RAW2

        if (results.decode_type == decode_type_t::UNKNOWN)
        # endif // ifdef P016_P035_USE_RAW_RAW2
        {
          addLog(LOG_LEVEL_INFO,
                 F("IR: No replay solutions found! Press button again or try RAW encoding (timings are in the serial output)"));
          serialPrint(F("IR: RAW TIMINGS: "));
          serialPrint(resultToSourceCode(&results));
          event->String2 = F("NaN");
          yield(); // Feed the WDT as it can take a while to print.
                   // addLog(LOG_LEVEL_DEBUG,(String(F("IR: RAW TIMINGS: ")) + resultToSourceCode(&results))); // Output the results as RAW
                   // source code //not showing up nicely in the web log
        }

        # ifdef P016_P035_Extended_AC

        // Display any extra A/C info if we have it.
        // Display the human readable state of an A/C message if we can.
        stdAc::state_t state;

        // Initialize state settings
        state.protocol = decode_type_t::UNKNOWN;
        state.model    = -1; // Unknown.
        state.power    = false;
        state.mode     = stdAc::opmode_t::kAuto;
        state.celsius  = true;
        state.degrees  = 22;
        state.fanspeed = stdAc::fanspeed_t::kAuto;
        state.swingv   = stdAc::swingv_t::kAuto;
        state.swingh   = stdAc::swingh_t::kAuto;
        state.quiet    = false;
        state.turbo    = false;
        state.econo    = false;
        state.light    = false;
        state.filter   = false;
        state.clean    = false;
        state.beep     = false;
        state.sleep    = -1;
        state.clock    = -1;

        String description = IRAcUtils::resultAcToString(&results);

        if (!description.isEmpty()) {
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            // If we got a human-readable description of the message, display it.
            String log;
            log.reserve(10 + description.length());
            log  = F("AC State: ");
            log += description;
            addLog(LOG_LEVEL_INFO, log);
          }
        }

        if (IRac::isProtocolSupported(results.decode_type)) // Check If there is a replayable AC state and show the JSON command that can be
                                                            // send
        {
          IRAcUtils::decodeToState(&results, &state);
          StaticJsonDocument<300> doc;

          // Checks if a particular state is something else than the default and only then it adds it to the JSON document
          doc[F("protocol")] = typeToString(state.protocol);

          if (state.model >= 0) {
            doc[F("model")] = irutils::modelToStr(state.protocol, state.model); // The specific model of A/C if applicable.
          }
          doc[F("power")] = IRac::boolToString(state.power);                    // POWER ON or OFF
          doc[F("mode")]  = IRac::opmodeToString(state.mode);                   // What operating mode should the unit perform? e.g. Cool =
                                                                                // doc[""]; Heat etc.
          doc[F("temp")] = state.degrees;                                       // What temperature should the unit be set to?

          if (!state.celsius) {
            doc[F("use_celsius")] = IRac::boolToString(state.celsius);          // Use degreees Celsius, otherwise Fahrenheit.
          }

          if (state.fanspeed != stdAc::fanspeed_t::kAuto) {
            doc[F("fanspeed")] = IRac::fanspeedToString(state.fanspeed); // Fan Speed setting
          }

          if (state.swingv != stdAc::swingv_t::kAuto) {
            doc[F("swingv")] = IRac::swingvToString(state.swingv); // Vertical swing setting
          }

          if (state.swingh != stdAc::swingh_t::kAuto) {
            doc[F("swingh")] = IRac::swinghToString(state.swingh); // Horizontal swing setting
          }

          if (state.quiet) {
            doc[F("quiet")] = IRac::boolToString(state.quiet); // Quiet setting ON or OFF
          }

          if (state.turbo) {
            doc[F("turbo")] = IRac::boolToString(state.turbo); // Turbo setting ON or OFF
          }

          if (state.econo) {
            doc[F("econo")] = IRac::boolToString(state.econo); // Economy setting ON or OFF
          }

          if (!state.light) {
            doc[F("light")] = IRac::boolToString(state.light); // Light setting ON or OFF
          }

          if (state.filter) {
            doc[F("filter")] = IRac::boolToString(state.filter); // Filter setting ON or OFF
          }

          if (state.clean) {
            doc[F("clean")] = IRac::boolToString(state.clean); // Clean setting ON or OFF
          }

          if (state.beep) {
            doc[F("beep")] = IRac::boolToString(state.beep); // Beep setting ON or OFF
          }

          if (state.sleep > 0) {
            doc[F("sleep")] = state.sleep; // Nr. of mins of sleep mode, or use sleep mode. (<= 0 means off.)
          }

          if (state.clock >= 0) {
            doc[F("clock")] = state.clock; // Nr. of mins past midnight to set the clock to. (< 0 means off.)
          }
          output = EMPTY_STRING;
          serializeJson(doc, output);
          event->String2 = output;

          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            // Show the command that the user can put to replay the AC state with P035
            String log;
            log.reserve(12 + output.length());
            log  = F("IRSENDAC,'");
            log += output;
            log += '\'';
            addLog(LOG_LEVEL_INFO, log);
          }
        }
        # endif // P016_P035_Extended_AC

        if (!P016_SEND_IR_TO_CONTROLLER) {
          unsigned long IRcode = results.value;
          UserVar.setSensorTypeLong(event->TaskIndex, IRcode);
        }
        sendData(event);
      }
      success = true;
      break;
    }
  }
  return success;
}

# ifdef P016_P035_USE_RAW_RAW2
#  define PCT_TOLERANCE 8u                                // Percent tolerance
#  define pct_tolerance(v) ((v) / (100u / PCT_TOLERANCE)) // Tolerance % is calculated as the delta between any original timing, and the
                                                          // result after encoding and decoding
// #define MIN_TOLERANCE       10u
// #define get_tolerance(v)    (pct_tolerance(v) > MIN_TOLERANCE? pct_tolerance(v) : MIN_TOLERANCE)
#  define get_tolerance(v) (pct_tolerance(v))
#  define MIN_VIABLE_DIV 40u // Minimum viable timing denominator
#  define to_32hex(c) ((c) < 10 ? (c) + '0' : (c) + 'A' - 10)

// This function attempts to convert the raw IR timings buffer to a short string that can be sent over as
// an IRSEND HTTP/MQTT command. It analyzes the timings, and searches for a common denominator which can be
// used to compress the values. If found, it then produces a string consisting of B32 Hex digit for each
// timing value, appended by the denominators for Pulse and Blank. This string can then be used in an
// IRSEND command. An important advantage of this string over the current IRSEND RAW B32 format implemented
// by GusPS is that it allows easy inspections and modifications after the code is constructed.
//
// Author: Gilad Raz (jazzgil)  23sep2018

boolean displayRawToReadableB32Hex(String& outputStr, decode_results results)
{
  uint16_t div[2];

  // print the values: either pulses or blanks
  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String line;

    for (uint16_t i = 1; i < results.rawlen; i++) {
      line += uint64ToString(results.rawbuf[i] * RAWTICK, 10) + ",";
    }
    addLog(LOG_LEVEL_DEBUG, line); // Display the RAW timings
  }

  // Find a common denominator divisor for odd indexes (pulses) and then even indexes (blanks).
  for (uint16_t p = 0; p < 2; p++)
  {
    uint16_t cd = 0xFFFFU; // current divisor

    // find the lowest value to start the divisor with.
    for (uint16_t i = 1 + p; i < results.rawlen; i += 2)
    {
      uint16_t val = results.rawbuf[i] * RAWTICK;

      if (cd > val) {
        cd = val;
      }
    }

    uint16_t bstDiv = -1, bstAvg = 0xFFFFU;
    float    bstMul = 5000;
    cd += get_tolerance(cd) + 1;

    // serialPrintln(String("p="+ uint64ToString(p, 10) + " start cd=" + uint64ToString(cd, 10)).c_str());
    // find the best divisor based on lowest avg err, within allowed tolerance.
    while (--cd >= MIN_VIABLE_DIV)
    {
      uint32_t avg    = 0;
      uint16_t totTms = 0;

      // calculate average error for current divisor, and verify it's within tolerance for all timings.
      for (uint16_t i = 1 + p; i < results.rawlen; i += 2)
      {
        uint16_t val  = results.rawbuf[i] * RAWTICK;
        uint16_t rmdr = val >= cd ? val % cd : cd - val;

        if (rmdr > get_tolerance(val))
        {
          avg = 0xFFFFU;
          break;
        }
        avg    += rmdr;
        totTms += val / cd + (cd > val ? 1 : 0);
      }

      if (avg == 0xFFFFU) {
        continue;
      }
      avg /= results.rawlen / 2;
      float avgTms = (float)totTms / (results.rawlen / 2);

      if ((avgTms <= bstMul) && (avg < bstAvg))
      {
        bstMul = avgTms;
        bstAvg = avg;
        bstDiv = cd;

        // serialPrintln(String("p="+ uint64ToString(p, 10) + " cd=" + uint64ToString(cd, 10) +"  avgErr=" + uint64ToString(avg, 10) + "
        // totTms="+ uint64ToString(totTms, 10) + " avgTms="+ uint64ToString((uint16_t)(avgTms*10), 10) ).c_str());
      }
    }

    if (bstDiv == 0xFFFFU)
    {
      // addLog(LOG_LEVEL_INFO, F("IR2: No proper divisor found. Try again..."));
      return false;
    }
    div[p] = bstDiv;

    if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
      String line;
      line  = p ? F("Blank: ") : F("Pulse: ");
      line += F(" divisor=");
      line += uint64ToString(bstDiv, 10);
      line += F("  avgErr=");
      line += uint64ToString(bstAvg, 10);
      line += F(" avgMul=");
      line += uint64ToString((uint16_t)bstMul, 10);
      line += '.';
      line += ((char)((bstMul - (uint16_t)bstMul) * 10) + '0');
      addLog(LOG_LEVEL_DEBUG, line);
    }
  }

  // Generate the B32 Hex string, per the divisors found.
  uint16_t total = results.rawlen - 1, tmOut[total];

  // line = "Timing muls ("+ uint64ToString(total, 10) + "): ";

  for (unsigned int i = 0; i < total; i++)
  {
    uint16_t val    = results.rawbuf[i + 1] * RAWTICK;
    unsigned int dv = div[(i) & 1];
    unsigned int tm = val / dv + (val % dv > dv / 2 ? 1 : 0);
    tmOut[i] = tm;

    // line += uint64ToString(tm, 10) + ",";
  }

  // serialPrintln(line);

  char out[total];
  unsigned int iOut = 0, s = 2, d = 0;

  for (; s + 1 < total; d = s, s += 2)
  {
    unsigned int vals = 2;

    while (s + 1 < total && tmOut[s] == tmOut[d] && tmOut[s + 1] == tmOut[d + 1])
    {
      vals += 2;
      s    += 2;
    }

    if ((iOut + 5 > sizeof(out)) || (tmOut[d] >= 32 * 32) || (tmOut[d + 1] >= 32 * 32) || (vals >= 64))
    {
      // addLog(LOG_LEVEL_INFO, F("IR2: Raw code too long. Try again..."));
      return false;
    }

    if ((vals > 4) || ((vals == 4) && ((tmOut[d] >= 32) || (tmOut[d + 1] >= 32))))
    {
      out[iOut++] = '*';
      out[iOut++] = to_32hex(vals / 2);
      vals        = 2;
    }

    while (vals--) {
      iOut = storeB32Hex(out, iOut, tmOut[d++]);
    }
  }

  while (d < total) {
    iOut = storeB32Hex(out, iOut, tmOut[d++]);
  }

  out[iOut] = 0;

  outputStr.reserve(32 + iOut);
  outputStr  = F("IRSEND,RAW2,");
  outputStr += out;
  outputStr += F(",38,");
  outputStr += uint64ToString(div[0], 10);
  outputStr += ',';
  outputStr += uint64ToString(div[1], 10);
  addLog(LOG_LEVEL_INFO, outputStr);
  return true;
}

unsigned int storeB32Hex(char out[], unsigned int iOut, unsigned int val)
{
  if (val >= 32)
  {
    out[iOut++] = '^';
    out[iOut++] = to_32hex(val / 32);
    val        %= 32;
  }
  out[iOut++] = to_32hex(val);
  return iOut;
}

# endif // P016_P035_RAW_RAW2

#endif  // USES_P016

void enableIR_RX(boolean enable)
{
#ifdef PLUGIN_016

  if (irReceiver == 0) { return; }

  if (enable) {
    irReceiver->enableIRIn();  // Start the receiver
  } else {
    irReceiver->disableIRIn(); // Stop the receiver
  }
#endif // PLUGIN_016
}
