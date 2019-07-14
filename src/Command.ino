
#include "Commands/Common.h"
#include "Commands/Blynk.h"
#include "Commands/Blynk_c015.h"
#include "Commands/Diagnostic.h"
#include "Commands/HTTP.h"
#include "Commands/i2c.h"
#include "Commands/MQTT.h"
#include "Commands/Networks.h"
#include "Commands/Notifications.h"
#include "Commands/RTC.h"
#include "Commands/Rules.h"
#include "Commands/SDCARD.h"
#include "Commands/Settings.h"
#include "Commands/System.h"
#include "Commands/Tasks.h"
#include "Commands/Time.h"
#include "Commands/Timer.h"
#include "Commands/UPD.h"
#include "Commands/wd.h"
#include "Commands/WiFi.h"


/*********************************************************************************************\
* Registers command
\*********************************************************************************************/
String doExecuteCommand(const char * cmd, struct EventStruct *event, const char* line)
{
  String cmd_lc;
	cmd_lc = cmd;
	cmd_lc.toLowerCase();
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
  	String log = F("Command: ");
  	log += cmd_lc;
  	addLog(LOG_LEVEL_INFO, log);
#ifndef BUILD_NO_DEBUG
    addLog(LOG_LEVEL_DEBUG, line); // for debug purposes add the whole line.
#endif
  }
  // Simple macro to match command to function call.
  #define COMMAND_CASE(S, C) if (strcmp_P(cmd_lc.c_str(), PSTR(S)) == 0) { return (C(event, line)); }

  switch (cmd_lc[0]) {
    case 'a': {
	  COMMAND_CASE("accessinfo"             , Command_AccessInfo_Ls);              // Network Command
      break;
    }
    case 'b': {
	  COMMAND_CASE("background"             , Command_Background);                 // Diagnostic.h
    #ifdef USES_C012
	  COMMAND_CASE("blynkget"               , Command_Blynk_Get);
    #endif
    #ifdef USES_C015
	  COMMAND_CASE("blynkset"               , Command_Blynk_Set);
    #endif
	  COMMAND_CASE("build"                  , Command_Settings_Build);             // Settings.h
      break;
    }
    case 'c': {
	  COMMAND_CASE("clearaccessblock"       , Command_AccessInfo_Clear);           // Network Command
	  COMMAND_CASE("clearrtcram"            , Command_RTC_Clear);                  // RTC.h
	  COMMAND_CASE("config"                 , Command_Task_RemoteConfig);          // Tasks.h
      break;
    }
    case 'd': {
	  COMMAND_CASE("debug"                  , Command_Debug);                      // Diagnostic.h
	  COMMAND_CASE("deepsleep"              , Command_System_deepSleep);           // System.h
	  COMMAND_CASE("delay"                  , Command_Delay);                      // Timers.h
	  COMMAND_CASE("dns"                    , Command_DNS);                        // Network Command
	  COMMAND_CASE("dst"                    , Command_DST);                        // Time.h
      break;
    }
    case 'e': {
	  COMMAND_CASE("erase"                  , Command_WiFi_Erase);                 // WiFi.h
	  COMMAND_CASE("event"                  , Command_Rules_Events);               // Rule.h
	  COMMAND_CASE("executerules"           , Command_Rules_Execute);              // Rule.h
      break;
    }
    case 'g': {
	  COMMAND_CASE("gateway"                , Command_Gateway);                    // Network Command
      break;
    }
    case 'i': {
	  COMMAND_CASE("i2cscanner"             , Command_i2c_Scanner);                // i2c.h
	  COMMAND_CASE("ip"                     , Command_IP);                         // Network Command
      break;
    }
    case 'j': {
    COMMAND_CASE("jsonportstatus"         , Command_JSONPortStatus);             // Diagnostic.h
    }
    case 'l': {
    COMMAND_CASE("let"                    , Command_Rules_Let);                  // Rules.h
	  COMMAND_CASE("load"                   , Command_Settings_Load);              // Settings.h
	  COMMAND_CASE("logentry"               , Command_logentry);                   // Diagnostic.h
    COMMAND_CASE("logportstatus"          , Command_logPortStatus);              // Diagnostic.h
	  COMMAND_CASE("lowmem"                 , Command_Lowmem);                     // Diagnostic.h
      break;
    }
    case 'm': {
	  COMMAND_CASE("malloc"                 , Command_Malloc);                     // Diagnostic.h
	  COMMAND_CASE("meminfo"                , Command_MemInfo);                    // Diagnostic.h
    COMMAND_CASE("meminfodetail"          , Command_MemInfo_detail);             // Diagnostic.h
	  COMMAND_CASE("messagedelay"           , Command_MQTT_messageDelay);          // MQTT.h
	  COMMAND_CASE("mqttretainflag"         , Command_MQTT_Retain);                // MQTT.h
      break;
    }
    case 'n': {
	  COMMAND_CASE("name"                   , Command_Settings_Name);              // Settings.h
	  COMMAND_CASE("nosleep"                , Command_System_NoSleep);             // System.h
	  COMMAND_CASE("notify"                 , Command_Notifications_Notify);       // Notifications.h
	  COMMAND_CASE("ntphost"                , Command_NTPHost);                    // Time.h
      break;
    }
    case 'p': {
	  COMMAND_CASE("password"               , Command_Settings_Password);          // Settings.h
    COMMAND_CASE("publish"                , Command_MQTT_Publish);               // MQTT.h
      break;
    }
    case 'r': {
	  COMMAND_CASE("readuservarfromrtc"     , Command_readUserVarFromRTC);         // RTC.h
	  COMMAND_CASE("reboot"                 , Command_System_Reboot);              // System.h
	  COMMAND_CASE("reset"                  , Command_Settings_Reset);             // Settings.h
	  COMMAND_CASE("resetflashwritecounter" , Command_RTC_resetFlashWriteCounter); // RTC.h
	  COMMAND_CASE("restart"                , Command_System_Restart);             // System.h
	  COMMAND_CASE("rules"                  , Command_Rules_UseRules);             // Rule.h
      break;
    }
    case 's': {
	  COMMAND_CASE("save"                   , Command_Settings_Save);              // Settings.h
	  COMMAND_CASE("saveuservartortc"       , Command_saveUserVarToRTC);           // RTC.h
	#ifdef FEATURE_SD
	  COMMAND_CASE("sdcard"                 , Command_SD_LS);                      // SDCARDS.h
	  COMMAND_CASE("sdremove"               , Command_SD_Remove);                  // SDCARDS.h
	#endif
	  COMMAND_CASE("sendto"                 , Command_UPD_SendTo);                 // UDP.h
	  COMMAND_CASE("sendtohttp"             , Command_HTTP_SendToHTTP);            // HTTP.h
	  COMMAND_CASE("sendtoudp"              , Command_UDP_SendToUPD);              // UDP.h
	  COMMAND_CASE("serialfloat"            , Command_SerialFloat);                // Diagnostic.h
	  COMMAND_CASE("settings"               , Command_Settings_Print);             // Settings.h
	  COMMAND_CASE("subnet"                 , Command_Subnet);                     // Network Command
	  COMMAND_CASE("sysload"                , Command_SysLoad);                    // Diagnostic.h
      break;
    }
    case 't': {
	  COMMAND_CASE("taskclear"              , Command_Task_Clear);                 // Tasks.h
	  COMMAND_CASE("taskclearall"           , Command_Task_ClearAll);              // Tasks.h
	  COMMAND_CASE("taskrun"                , Command_Task_Run);                   // Tasks.h
    COMMAND_CASE("taskvalueset"           , Command_Task_ValueSet);              // Tasks.h
    COMMAND_CASE("taskvaluetoggle"        , Command_Task_ValueToggle);           // Tasks.h
	  COMMAND_CASE("taskvaluesetandrun"     , Command_Task_ValueSetAndRun);        // Tasks.h
	  COMMAND_CASE("timerpause"             , Command_Timer_Pause);                // Timers.h
	  COMMAND_CASE("timerresume"            , Command_Timer_Resume);               // Timers.h
	  COMMAND_CASE("timerset"               , Command_Timer_Set);                  // Timers.h
	  COMMAND_CASE("timezone"               , Command_TimeZone);                   // Time.h
      break;
    }
    case 'u': {
	  COMMAND_CASE("udpport"                , Command_UDP_Port);                   // UDP.h
	  COMMAND_CASE("udptest"                , Command_UDP_Test);                   // UDP.h
	  COMMAND_CASE("unit"                   , Command_Settings_Unit);              // Settings.h
	  COMMAND_CASE("usentp"                 , Command_useNTP);                     // Time.h
      break;
    }
    case 'w': {
	  COMMAND_CASE("wdconfig"               , Command_WD_Config);                  // WD.h
	  COMMAND_CASE("wdread"                 , Command_WD_Read);                    // WD.h
	  COMMAND_CASE("wifiapmode"             , Command_Wifi_APMode);                // WiFi.h
	  COMMAND_CASE("wificonnect"            , Command_Wifi_Connect);               // WiFi.h
	  COMMAND_CASE("wifidisconnect"         , Command_Wifi_Disconnect);            // WiFi.h
	  COMMAND_CASE("wifikey"                , Command_Wifi_Key);                   // WiFi.h
	  COMMAND_CASE("wifikey2"               , Command_Wifi_Key2);                  // WiFi.h
	  COMMAND_CASE("wifimode"               , Command_Wifi_Mode);                  // WiFi.h
	  COMMAND_CASE("wifiscan"               , Command_Wifi_Scan);                  // WiFi.h
	  COMMAND_CASE("wifissid"               , Command_Wifi_SSID);                  // WiFi.h
	  COMMAND_CASE("wifissid2"              , Command_Wifi_SSID2);                 // WiFi.h
	  COMMAND_CASE("wifistamode"            , Command_Wifi_STAMode);               // WiFi.h
      break;
    }
    default:
      break;
  }
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String errorUnknown = F("Command unknown: \"");
  	errorUnknown += cmd_lc;
  	errorUnknown += '\"';
  	addLog(LOG_LEVEL_INFO, errorUnknown);
  }
	return F("\nUnknown command!");

  #undef COMMAND_CASE
}

// Simple function to return "Ok", to avoid flash string duplication in the firmware.
String return_command_success()
{
	return F("\nOk");
}
String return_command_failed()
{
	return F("\nFailed");
}
String return_not_connected()
{
	return F("Not connected to WiFi");
}

String return_result(struct EventStruct *event, const String& result)
{
	serialPrintln(result);
	if (event->Source == VALUE_SOURCE_SERIAL) {
		return return_command_success();
	}
	return result;
}

String return_see_serial(struct EventStruct *event)
{
	if (event->Source == VALUE_SOURCE_SERIAL) {
		return return_command_success();
	}
	return F("Output sent to serial");
}

void ExecuteCommand(byte source, const char *Line)
{
	checkRAM(F("ExecuteCommand"));
  String cmd;
  if (!GetArgv(Line, cmd, 1)) {
    return;
  }
	struct EventStruct TempEvent;
	// FIXME TD-er: Not sure what happens now, but TaskIndex cannot be set here
	// since commands can originate from anywhere.
	TempEvent.Source = source;
  {
    // Use extra scope to delete the TmpStr1 before executing command.
    String TmpStr1;
    if (GetArgv(Line, TmpStr1, 2)) TempEvent.Par1 = CalculateParam(TmpStr1.c_str());
  	if (GetArgv(Line, TmpStr1, 3)) TempEvent.Par2 = CalculateParam(TmpStr1.c_str());
  	if (GetArgv(Line, TmpStr1, 4)) TempEvent.Par3 = CalculateParam(TmpStr1.c_str());
  	if (GetArgv(Line, TmpStr1, 5)) TempEvent.Par4 = CalculateParam(TmpStr1.c_str());
  	if (GetArgv(Line, TmpStr1, 6)) TempEvent.Par5 = CalculateParam(TmpStr1.c_str());
  }

  String status = doExecuteCommand(cmd.c_str(), &TempEvent, Line);
  delay(0);
  SendStatus(source, status);
  delay(0);
/*
  } else {
    // Schedule to run async
    schedule_command_timer(cmd.c_str(), &TempEvent, Line);
  }
*/
}

#ifdef FEATURE_SD
void printDirectory(File dir, int numTabs)
{
	while (true) {
		File entry = dir.openNextFile();
		if (!entry) {
			// no more files
			break;
		}
		for (uint8_t i = 0; i < numTabs; i++) {
			serialPrint("\t");
		}
		serialPrint(entry.name());
		if (entry.isDirectory()) {
			serialPrintln("/");
			printDirectory(entry, numTabs + 1);
		} else {
			// files have sizes, directories do not
			serialPrint("\t\t");
			serialPrintln(String(entry.size(), DEC));
		}
		entry.close();
	}
}
#endif
