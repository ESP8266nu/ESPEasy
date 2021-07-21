#include "_Plugin_Helper.h"
#ifdef USES_P101

// #######################################################################################################
// ####################################### Plugin 101: Wake On LAN #######################################
// #######################################################################################################

// Wake On LAN (WOL): Send Magic Packet to MAC address; Wake-up sleeping computer if configured for WOL.
// Plugin Written by ThomasB (ThomasTech), Oct-12-2020.
//   CHANGE LOG
//   Oct-12-2020: Creation
//   Oct-16-2020: Beta Test Release to ESPEasy Forum.
//   Oct-18-2020: Re-assigned as plugin number P101 (was P248).
//   Oct-20-2020: Github PR #3328, Submitted as [Testing] plugin.
//
// This ESPEasy plugin requires the WakeOnLan library found here:
//   https://github.com/a7md0/WakeOnLan
//
// Command Keyword:
//   Command Keyword is WAKEONLAN or Device's Task name. Keyword is case insensitive.
//
// Command Syntax:
//   WakeOnLan,<Optional MAC Address>,<Optional IP Address>,<Optional Port Number>
//
// Command Examples (Using Task Name = WakeKitchenPC):
//   WakeOnLan
//   WakeKitchenPC
//   WakeOnLan,01:23:45:67:89:AB
//   WakeOnLan,01:23:45:67:89:AB,192.168.1.255
//   WakeOnLan,01:23:45:67:89:AB,255.255.255.255,9
//
// Multiple Task Instances:
//   Multiple Wake-On-Lan device tasks can be installed. Each task MUST have a unique name for use
//   as the command keyword.
//
// Configuration Tips:
//   The standard Magic Packet IP address is 255.255.255.255. If the router does not accept this
//   then use the router's local subnet IP address with 255 at the end. For example, 192.168.1.255.
//   Some WOL devices will accept magic packets sent to their specific IP too.
//   Common UDP Port values are 0, 7, and 9.
//   PC's with integrated WiFi and Ethernet ports will likely have two MAC addresses, with specific
//   WOL settings for each one.
//   I don't believe USB connected network adapters are capable of WOL; Check the adapter's manual.
//
// Useful WOL tools:
// WakeMeOnLan WOL Scanner/Sender: https://www.nirsoft.net/utils/wake_on_lan.html
// wolsniffer WOL Packet Analyzer: https://apreltech.com/Downloads/wolsniffer.zip
// Online MAC Address Generator  : https://www.browserling.com/tools/random-mac
//
// ************************************************************************************************


#include <WakeOnLan.h>

// Plugin defines
#define PLUGIN_101
#define PLUGIN_ID_101      101
#define PLUGIN_NAME_101    "Communication - Wake On LAN [Testing]"

// Config Setting defines
#define CUSTOMTASK_STR_SIZE_P101 20
#define DEF_TASK_NAME_P101 "WAKE_ON_LAN"
#define UDP_PORT_P101      ExtraTaskSettings.TaskDevicePluginConfigLong[1]
#define FORM_PORT_P101     "P101_port"

// Command keyword defines
#define CMD_NAME_P101      "WAKEONLAN"

// MAC Defines.
#define MAC_ADDR_SIZE_P101 17  // MAC Addr String size (fixed length). e.g. FA:39:09:67:89:AB
#define MAC_BUFF_SIZE_P101 18  // MAC Addr Buffer size, including NULL terminator.
#define MAC_SEP_CHAR_P101  ':' // MAC Addr segments are separated by a colon.
#define MAC_SEP_CNT_P101   5   // MAC Addr colon separator count for valid address.
#define MAC_STR_DEF_P101   "00:00:00:00:00:00"
#define MAC_STR_EXP_P101   "5d:89:22:56:9c:60"

// IP Defines.
#define IP_ADDR_SIZE_P101  15  // IPv4 Addr String size (max length). e.g. 192.168.001.255
#define IP_BUFF_SIZE_P101  16  // IPv4 Addr Buffer size, including NULL terminator.
#define IP_MIN_SIZE_P101   7   // IPv4 Addr Minimum size, allows IP strings as short as 0.0.0.0
#define IP_SEP_CHAR_P101   '.' // IPv4 Addr segments are separated by a dot.
#define IP_SEP_CNT_P101    3   // IPv4 Addr dot separator count for valid address.
#define IP_STR_DEF_P101    "255.255.255.255"

// Port Defines.
#define PORT_DEF_P101      9
#define PORT_MAX_P101      65535

// Misc Defines
#define LOG_NAME_P101      "WAKE ON LAN: "
#define NAME_MISSING       0
#define NAME_SAFE          1
#define NAME_UNSAFE        2

// ************************************************************************************************
// WOL Objects
WiFiUDP   udp;
WakeOnLan WOL(udp);

// ************************************************************************************************
// Local Functions
uint8_t safeName(taskIndex_t index);
bool    validateIp(const String& ipStr);
bool    validateMac(const String& macStr);
bool    validatePort(const String& portStr);

// ************************************************************************************************

boolean Plugin_101(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_101;
      Device[deviceCount].Type               = DEVICE_TYPE_DUMMY;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].ValueCount         = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_101);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES: {
      break;
    }

    case PLUGIN_SET_DEFAULTS:
    {
      UDP_PORT_P101 = PORT_DEF_P101;
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      char   ipString[IP_BUFF_SIZE_P101]   = "";
      char   macString[MAC_BUFF_SIZE_P101] = "";
      String msgStr;

      addFormSubHeader(""); // Blank line, vertical space.
      addFormHeader(F("Default Settings"));

      String strings[2];
      LoadCustomTaskSettings(event->TaskIndex, strings, 2, CUSTOMTASK_STR_SIZE_P101);

      safe_strncpy(macString, strings[1], MAC_BUFF_SIZE_P101);
      addFormTextBox(F("MAC Address"), getPluginCustomArgName(1), macString, MAC_ADDR_SIZE_P101);
      msgStr  = F("Format Example, ");
      msgStr += F(MAC_STR_EXP_P101);
      addFormNote(msgStr);
      addFormSubHeader(""); // Blank line, vertical space.

      safe_strncpy(ipString, strings[0], IP_BUFF_SIZE_P101);
      addFormTextBox(F("IPv4 Address"), getPluginCustomArgName(0), ipString, IP_ADDR_SIZE_P101);
      addFormNumericBox(F("UDP Port"), F(FORM_PORT_P101), UDP_PORT_P101, 0, PORT_MAX_P101);
      msgStr  = F("Typical Installations use IP Address ");
      msgStr += F(IP_STR_DEF_P101);
      msgStr += F(", Port ");
      msgStr += PORT_DEF_P101;
      addFormNote(msgStr);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      char   ipString[IP_BUFF_SIZE_P101]   = "";
      char   macString[MAC_BUFF_SIZE_P101] = "";
      char   deviceTemplate[2][CUSTOMTASK_STR_SIZE_P101];
      String errorStr;
      String msgStr;
      const String wolStr = F(LOG_NAME_P101);

      LoadTaskSettings(event->TaskIndex);

      // Check Task Name.
      uint8_t nameCode = safeName(event->TaskIndex);

      if ((nameCode == NAME_MISSING) || (nameCode == NAME_UNSAFE)) {               // Check to see if user submitted safe device name.
        strcpy(ExtraTaskSettings.TaskDeviceName, (char *)(F(DEF_TASK_NAME_P101))); // Use default name.

        if (nameCode == NAME_UNSAFE) {
          errorStr = F("ALERT, Renamed Unsafe Task Name. ");
        }
      }

      // Check IP Address.
      if (!safe_strncpy(ipString, webArg(getPluginCustomArgName(0)), IP_BUFF_SIZE_P101)) {
        // msgStr = getCustomTaskSettingsError(0); // Report string too long.
        // errorStr += msgStr;
        // msgStr    = wolStr + msgStr;
        // addLog(LOG_LEVEL_INFO, msgStr);
      }

      if (strlen(ipString) == 0) { // IP Address missing, use default value (without webform warning).
        strcpy_P(ipString, (char *)(F(IP_STR_DEF_P101)));

        msgStr  = wolStr;
        msgStr += F("Loaded Default IP = ");
        msgStr += F(IP_STR_DEF_P101);
        addLog(LOG_LEVEL_INFO, msgStr);
      }
      else if (strlen(ipString) < IP_MIN_SIZE_P101) { // IP Address too short, load default value. Warn User.
        strcpy_P(ipString, (char *)(F(IP_STR_DEF_P101)));

        msgStr    = F("Provided IP Invalid (Using Default). ");
        errorStr += msgStr;
        msgStr    = wolStr + msgStr;
        msgStr   += F("[");
        msgStr   += F(IP_STR_DEF_P101);
        msgStr   += F("]");
        addLog(LOG_LEVEL_INFO, msgStr);
      }
      else if (!validateIp(ipString)) { // Unexpected IP Address value. Leave as-is, but Warn User.
        msgStr    = F("WARNING, Please Review IP Address. ");
        errorStr += msgStr;
        msgStr    = wolStr + msgStr;
        msgStr   += F("[");
        msgStr   += ipString;
        msgStr   += F("]");
        addLog(LOG_LEVEL_INFO, msgStr);
      }

      // Check MAC Address.
      if (!safe_strncpy(macString, webArg(getPluginCustomArgName(1)), MAC_BUFF_SIZE_P101)) {
        // msgStr += getCustomTaskSettingsError(1); // Report string too long.
        // errorStr += msgStr;
        // msgStr    = wolStr + msgStr;
        // addLog(LOG_LEVEL_INFO, msgStr);
      }

      if (strlen(macString) == 0) { // MAC Address missing, use default value.
        strcpy_P(macString, (char *)(F(MAC_STR_DEF_P101)));

        msgStr    = F("MAC Address Not Provided, Populated with Zero Values. ");
        errorStr += msgStr;
        addLog(LOG_LEVEL_INFO, wolStr + msgStr);
      }
      else if (!validateMac(macString)) { // Suspicious MAC Address. Leave as-is, but warn User.
        msgStr    = F("ERROR, MAC Address Invalid. ");
        errorStr += msgStr;
        msgStr    = wolStr + msgStr;
        msgStr   += F("[");
        msgStr   += macString;
        msgStr   += F("]");
        addLog(LOG_LEVEL_INFO, msgStr);
      }

      // Save the user's IP and MAC Address parameters into Custom Settings.
      safe_strncpy(deviceTemplate[0], ipString,   IP_BUFF_SIZE_P101);
      safe_strncpy(deviceTemplate[1], macString, MAC_BUFF_SIZE_P101);

      if (errorStr.length() > 0) { // Send error messages (if any) to webform.
        addHtmlError(errorStr);
      }

      // Save all the Task parameters.
      SaveCustomTaskSettings(event->TaskIndex, (uint8_t *)&deviceTemplate, sizeof(deviceTemplate));
      UDP_PORT_P101 = getFormItemInt(F(FORM_PORT_P101));
      success       = true;
      break;
    }

    case PLUGIN_INIT: {
      success = true;
      break;
    }

    case PLUGIN_READ: {
      success = false;
      break;
    }

    case PLUGIN_WRITE: {
      char   ipString[IP_BUFF_SIZE_P101]   = "";
      char   macString[MAC_BUFF_SIZE_P101] = "";
      bool   taskEnable                    = false;
      uint8_t   parse_error                   = false;
      String msgStr;
      String strings[2];
      String tmpString    = string;
      const String wolStr = F(LOG_NAME_P101);

      //  addLog(LOG_LEVEL_INFO, String(F("--> WOL taskIndex= ")) + String(event->TaskIndex)); // Debug

      String cmd = parseString(tmpString, 1);

      //  Warning, event->TaskIndex is invalid in PLUGIN_WRITE during controller ack calls.
      //  So checking the Device Name needs special attention.
      //  See https://github.com/letscontrolit/ESPEasy/issues/3317
      if (validTaskIndex(event->TaskIndex) &&
          (cmd.equalsIgnoreCase(F(CMD_NAME_P101)) ||
           cmd.equalsIgnoreCase(getTaskDeviceName(event->TaskIndex)))) {
        LoadTaskSettings(event->TaskIndex);
        taskEnable = Settings.TaskDeviceEnabled[event->TaskIndex];

        // Do not process WOL command if plugin disabled. This code is for errant situations which may never occur.
        if (!taskEnable) {
          // String ErrorStr = F("Plugin is Disabled, Command Ignored. ");
          // addLog(LOG_LEVEL_INFO, wolStr + ErrorStr);
          // SendStatus(event, ErrorStr); // Reply (echo) to sender. This will print message on browser.
          break;
        }

        success = true;

        LoadCustomTaskSettings(event->TaskIndex, strings, 2, CUSTOMTASK_STR_SIZE_P101);
        safe_strncpy(ipString,  strings[0],  IP_BUFF_SIZE_P101);
        safe_strncpy(macString, strings[1], MAC_BUFF_SIZE_P101);

        String paramMac  = parseString(tmpString, 2); // MAC Address (optional)
        String paramIp   = parseString(tmpString, 3); // IP Address (optional)
        String paramPort = parseString(tmpString, 4); // UDP Port (optional)

        // Populate Parameters with default settings when missing from command line.
        if (paramMac.isEmpty()) {                         // Missing from command line, use default setting.
          paramMac = macString;
        }

        if (paramIp.isEmpty()) { // Missing from command line, use default setting.
          paramIp = ipString;
        }

        if (paramPort.isEmpty()) {
          int portNumber = UDP_PORT_P101; // Get default Port from user settings.
          paramPort = portNumber;
        }

        // Validate the MAC Address.
        if (!validateMac(paramMac)) {
          parse_error = true;
          msgStr      = wolStr;
          msgStr     += F("Error, MAC Addr Invalid [");
          msgStr     += paramMac;
          msgStr     += F("]");
          addLog(LOG_LEVEL_INFO, msgStr);
        }

        // Validate IP Address.
        if (!validateIp(paramIp)) {
          parse_error = true;
          msgStr      = wolStr;
          msgStr     += F("Error, IP Addr Invalid [");
          msgStr     += paramIp;
          msgStr     += F("]");
          addLog(LOG_LEVEL_INFO, msgStr);
        }

        // Validate UDP Port.
        if (!validatePort(paramPort)) {
          parse_error = true;
          msgStr      = wolStr;
          msgStr     += F("Error, Port Invalid [");
          msgStr     += paramPort;
          msgStr     += F("]");
          addLog(LOG_LEVEL_INFO, msgStr);
        }

        // If no errors we can send Magic Packet.
        if (parse_error == true) {
          msgStr = F("CMD Syntax Error");
          addLog(LOG_LEVEL_INFO, wolStr + msgStr);
          msgStr += F(" <br>");
          SendStatus(event, msgStr); // Reply (echo) to sender. This will print message on browser.
        }
        else {                               // No parsing errors, Send Magic Packet (Wake Up the MAC).
          msgStr  = wolStr;
          msgStr += F("MAC= ");
          msgStr += paramMac;
          msgStr += F(", IP= ");
          msgStr += paramIp;
          msgStr += F(", Port= ");
          msgStr += paramPort;
          addLog(LOG_LEVEL_INFO, msgStr);

          // Send Magic Packet.
          if (WiFi.status() == WL_CONNECTED) {
            IPAddress local_IP;
            local_IP.fromString(paramIp);
            WOL.setBroadcastAddress(local_IP);

            // WOL.setRepeat(1, 0); // One Magic Packet, No Repeats. (Library default)

            if (!WOL.sendMagicPacket(paramMac, paramPort.toInt())) {
              msgStr  = wolStr;
              msgStr += F("Error, Magic Packet Failed (check parameters)");
              addLog(LOG_LEVEL_INFO, msgStr);
            }
          }
          else {
            msgStr  = wolStr;
            msgStr += F("Error, WiFi Off-Line");
            addLog(LOG_LEVEL_INFO, msgStr);
          }
        }
      }
      break;
    }

    case PLUGIN_EXIT:
    {
      break;
    }
  }
  return success;
}

// ************************************************************************************************
// safeName(): Check task name and confirm it is present and safe to use.
// Arg: task index
// Returns: NAME_SAFE, NAME_MISSING, or NAME_USAFE.
uint8_t safeName(taskIndex_t index) {
  uint8_t safeCode = NAME_SAFE;
  String  devName  = getTaskDeviceName(index);

  devName.toLowerCase();

  if (devName.isEmpty()) {
    safeCode = NAME_MISSING;
  }

  if (devName == F("reboot")) {
    safeCode = NAME_UNSAFE;
  }
  else if (devName == F("reset")) {
    safeCode = NAME_UNSAFE;
  }

  return safeCode;
}

// ************************************************************************************************
// validateIp(): IPv4 Address validity checker.
// Arg: IP Address String in dot separated format (192.168.1.255).
// Return true if IP string appears legit.
bool validateIp(const String& ipStr) {
  IPAddress ip;
  unsigned int length = ipStr.length();

  if ((length < IP_MIN_SIZE_P101) || (length > IP_ADDR_SIZE_P101)) {
    return false;
  }
  else if (ip.fromString(ipStr) == false) { // ThomasTech's Trick to Check IP for valid formatting.
    return false;
  }

  return true;
}

// ************************************************************************************************
// validateMac(): MAC Address validity checker.
// Arg: MAC Address String in colon separated format (00:00:00:00:00:00).
// Return true if MAC string appears legit.
bool validateMac(const String& macStr) {
  uint8_t pos = 0;
  char    hexChar;

  if (macStr.length() != MAC_ADDR_SIZE_P101) {
    return false;
  }

  for (uint8_t strPos = 0; strPos < MAC_ADDR_SIZE_P101; strPos++) {
    uint8_t mod = strPos % 3;

    if (mod == 2) {
      if (macStr[strPos] == MAC_SEP_CHAR_P101) { // Must be a colon in the third position.
        pos++;
      }
      else {
        return false;
      }
    }
    else {
      hexChar = macStr[strPos];

      if (!isHexadecimalDigit(hexChar)) {
        return false;
      }
    }
  }
  return true;
}

// ************************************************************************************************
// validatePort(): Simple UDP Port Number validity checker.
// Arg: Port Number String (decimal).
// Return true if Port string appears legit.
bool validatePort(const String& portStr) {
  bool pass = true;
  long portNumber;

  portNumber = portStr.toInt();

  if ((portNumber < 0) || (portNumber > PORT_MAX_P101)) {
    pass = false;
  }
  else if (!isDigit(portStr.charAt(0))) {
    pass = false;
  }

  return pass;
}

// ************************************************************************************************

#endif // USES_P101
