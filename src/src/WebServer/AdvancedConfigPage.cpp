#include "../WebServer/AdvancedConfigPage.h"

#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"
#include "../WebServer/WebServer.h"

#include "../ESPEasyCore/ESPEasyWifi.h"

#include "../Globals/ESPEasy_time.h"
#include "../Globals/Settings.h"
#include "../Globals/TimeZone.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/StringConverter.h"


#ifdef WEBSERVER_ADVANCED

void setLogLevelFor(uint8_t destination, LabelType::Enum label) {
  setLogLevelFor(destination, getFormItemInt(getInternalLabel(label)));
}

// ********************************************************************************
// Web Interface config page
// ********************************************************************************
void handle_advanced() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_advanced"));
  #endif

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();

  if (!webArg(F("edit")).isEmpty())
  {
//    Settings.MessageDelay_unused = getFormItemInt(F("messagedelay"));
    Settings.IP_Octet     = webArg(F("ip")).toInt();
    strncpy_webserver_arg(Settings.NTPHost, F("ntphost"));
    Settings.TimeZone = getFormItemInt(F("timezone"));
    TimeChangeRule dst_start(getFormItemInt(F("dststartweek")), getFormItemInt(F("dststartdow")), getFormItemInt(F("dststartmonth")), getFormItemInt(F("dststarthour")), Settings.TimeZone);

    if (dst_start.isValid()) { Settings.DST_Start = dst_start.toFlashStoredValue(); }
    TimeChangeRule dst_end(getFormItemInt(F("dstendweek")), getFormItemInt(F("dstenddow")), getFormItemInt(F("dstendmonth")), getFormItemInt(F("dstendhour")), Settings.TimeZone);

    if (dst_end.isValid()) { Settings.DST_End = dst_end.toFlashStoredValue(); }
    webArg2ip(F("syslogip"), Settings.Syslog_IP);
    Settings.WebserverPort = getFormItemInt(F("webport"));
    Settings.UDPPort = getFormItemInt(F("udpport"));

    Settings.SyslogFacility = getFormItemInt(F("syslogfacility"));
    Settings.SyslogPort     = getFormItemInt(F("syslogport"));
    Settings.UseSerial      = isFormItemChecked(F("useserial"));
    setLogLevelFor(LOG_TO_SYSLOG, LabelType::SYSLOG_LOG_LEVEL);
    setLogLevelFor(LOG_TO_SERIAL, LabelType::SERIAL_LOG_LEVEL);
    setLogLevelFor(LOG_TO_WEBLOG, LabelType::WEB_LOG_LEVEL);
#ifdef FEATURE_SD
    setLogLevelFor(LOG_TO_SDCARD, LabelType::SD_LOG_LEVEL);
#endif // ifdef FEATURE_SD
    Settings.UseValueLogger              = isFormItemChecked(F("valuelogger"));
    Settings.BaudRate                    = getFormItemInt(F("baudrate"));
    Settings.UseNTP                      = isFormItemChecked(F("usentp"));
    Settings.DST                         = isFormItemChecked(F("dst"));
    Settings.WDI2CAddress                = getFormItemInt(F("wdi2caddress"));
    #ifdef USES_SSDP
    Settings.UseSSDP                     = isFormItemChecked(F("usessdp"));
    #endif // USES_SSDP
    Settings.WireClockStretchLimit       = getFormItemInt(F("wireclockstretchlimit"));
    Settings.UseRules                    = isFormItemChecked(F("userules"));
    Settings.ConnectionFailuresThreshold = getFormItemInt(LabelType::CONNECTION_FAIL_THRESH);
    Settings.ArduinoOTAEnable            = isFormItemChecked(F("arduinootaenable"));
    Settings.UseRTOSMultitasking         = isFormItemChecked(F("usertosmultitasking"));

    // MQTT settings now moved to the controller settings.
//    Settings.MQTTRetainFlag_unused              = isFormItemChecked(F("mqttretainflag"));
//    Settings.MQTTUseUnitNameAsClientId   = isFormItemChecked(F("mqttuseunitnameasclientid"));
//    Settings.uniqueMQTTclientIdReconnect(isFormItemChecked(F("uniquemqttclientidreconnect")));
    Settings.Latitude  = getFormItemFloat(F("latitude"));
    Settings.Longitude = getFormItemFloat(F("longitude"));
    #ifdef WEBSERVER_NEW_RULES
    Settings.OldRulesEngine(isFormItemChecked(F("oldrulesengine")));
    #endif // WEBSERVER_NEW_RULES
    Settings.TolerantLastArgParse(isFormItemChecked(F("tolerantargparse")));
    Settings.SendToHttp_ack(isFormItemChecked(F("sendtohttp_ack")));
    Settings.ForceWiFi_bg_mode(isFormItemChecked(LabelType::FORCE_WIFI_BG));
    Settings.WiFiRestart_connection_lost(isFormItemChecked(LabelType::RESTART_WIFI_LOST_CONN));
    Settings.EcoPowerMode(isFormItemChecked(LabelType::CPU_ECO_MODE));
    Settings.WifiNoneSleep(isFormItemChecked(LabelType::FORCE_WIFI_NOSLEEP));
#ifdef SUPPORT_ARP
    Settings.gratuitousARP(isFormItemChecked(LabelType::PERIODICAL_GRAT_ARP));
#endif // ifdef SUPPORT_ARP
    Settings.setWiFi_TX_power(getFormItemFloat(LabelType::WIFI_TX_MAX_PWR));
    Settings.WiFi_sensitivity_margin = getFormItemInt(LabelType::WIFI_SENS_MARGIN);
    Settings.UseMaxTXpowerForSending(isFormItemChecked(LabelType::WIFI_SEND_AT_MAX_TX_PWR));
    Settings.NumberExtraWiFiScans = getFormItemInt(LabelType::WIFI_NR_EXTRA_SCANS);
    Settings.PeriodicalScanWiFi(isFormItemChecked(LabelType::WIFI_PERIODICAL_SCAN));
    Settings.UseLastWiFiFromRTC(isFormItemChecked(LabelType::WIFI_USE_LAST_CONN_FROM_RTC));
    Settings.JSONBoolWithoutQuotes(isFormItemChecked(LabelType::JSON_BOOL_QUOTES));
    Settings.EnableTimingStats(isFormItemChecked(LabelType::ENABLE_TIMING_STATISTICS));
    #ifdef ESP8266
    Settings.UseAlternativeDeepSleep(isFormItemChecked(LabelType::DEEP_SLEEP_ALTERNATIVE_CALL));
    #endif

#ifdef USES_ESPEASY_NOW
    Settings.UseESPEasyNow(isFormItemChecked(getInternalLabel(LabelType::USE_ESPEASY_NOW)));
    Settings.ForceESPEasyNOWchannel = getFormItemInt(getInternalLabel(LabelType::FORCE_ESPEASY_NOW_CHANNEL));
#endif

    addHtmlError(SaveSettings());

    if (node_time.systemTimePresent()) {
      node_time.initTime();
    }
  }

  addHtml(F("<form  method='post'>"));
  html_table_class_normal();

  addFormHeader(F("Advanced Settings"), F("RTDTools/Tools.html#advanced"));

  addFormSubHeader(F("Rules Settings"));

  addFormCheckBox(F("Rules"),      F("userules"),       Settings.UseRules);
  #ifdef WEBSERVER_NEW_RULES
  addFormCheckBox(F("Old Engine"), F("oldrulesengine"), Settings.OldRulesEngine());
  #endif // WEBSERVER_NEW_RULES
  addFormCheckBox(F("Tolerant last parameter"), F("tolerantargparse"), Settings.TolerantLastArgParse());
  addFormNote(F("Perform less strict parsing on last argument of some commands (e.g. publish and sendToHttp)"));
  addFormCheckBox(F("SendToHTTP wait for ack"), F("sendtohttp_ack"), Settings.SendToHttp_ack());

  /*
  // MQTT settings now moved to the controller settings.
  addFormSubHeader(F("Controller Settings"));

  addFormNumericBox(F("Message Interval"), F("messagedelay"), Settings.MessageDelay_unused, 0, INT_MAX);
  addUnit(F("ms"));

  addFormCheckBox(F("MQTT Retain Msg"), F("mqttretainflag"), Settings.MQTTRetainFlag_unused);
  addFormCheckBox(F("MQTT use unit name as ClientId"),    F("mqttuseunitnameasclientid"),   Settings.MQTTUseUnitNameAsClientId);
  addFormCheckBox(F("MQTT change ClientId at reconnect"), F("uniquemqttclientidreconnect"), Settings.uniqueMQTTclientIdReconnect_unused());
*/

  addFormSubHeader(F("NTP Settings"));

  addFormCheckBox(F("Use NTP"), F("usentp"), Settings.UseNTP);
  addFormTextBox(F("NTP Hostname"), F("ntphost"), Settings.NTPHost, 63);

  addFormSubHeader(F("DST Settings"));
  addFormDstSelect(true,  Settings.DST_Start);
  addFormDstSelect(false, Settings.DST_End);
  addFormCheckBox(F("DST"), F("dst"), Settings.DST);

  addFormSubHeader(F("Location Settings"));
  addFormNumericBox(F("Timezone Offset (UTC +)"), F("timezone"), Settings.TimeZone, -720, 840); // UTC-12H ... UTC+14h
  addUnit(F("minutes"));
  addFormFloatNumberBox(F("Latitude"), F("latitude"), Settings.Latitude, -90.0f, 90.0f);
  addUnit(F("&deg;"));
  addFormFloatNumberBox(F("Longitude"), F("longitude"), Settings.Longitude, -180.0f, 180.0f);
  addUnit(F("&deg;"));
  addFormNote(F("Longitude and Latitude are used to compute sunrise and sunset"));

  addFormSubHeader(F("Log Settings"));

  addFormIPBox(F("Syslog IP"), F("syslogip"), Settings.Syslog_IP);
  addFormNumericBox(F("Syslog UDP port"), F("syslogport"), Settings.SyslogPort, 0, 65535);

  addFormLogLevelSelect(LabelType::SYSLOG_LOG_LEVEL, Settings.SyslogLevel);
  addFormLogFacilitySelect(F("Syslog Facility"), F("syslogfacility"), Settings.SyslogFacility);
  addFormLogLevelSelect(LabelType::SERIAL_LOG_LEVEL, Settings.SerialLogLevel);
  addFormLogLevelSelect(LabelType::WEB_LOG_LEVEL,    Settings.WebLogLevel);

#ifdef FEATURE_SD
  addFormLogLevelSelect(LabelType::SD_LOG_LEVEL,     Settings.SDLogLevel);

  addFormCheckBox(F("SD Card Value Logger"), F("valuelogger"), Settings.UseValueLogger);
#endif // ifdef FEATURE_SD


  addFormSubHeader(F("Serial Settings"));

  addFormCheckBox(F("Enable Serial port"), F("useserial"), Settings.UseSerial);
  addFormNumericBox(F("Baud Rate"), F("baudrate"), Settings.BaudRate, 0, 1000000);


  addFormSubHeader(F("Inter-ESPEasy Network"));

  addFormNumericBox(F("UDP port"), F("udpport"), Settings.UDPPort, 0, 65535);

  // TODO sort settings in groups or move to other pages/groups
  addFormSubHeader(F("Special and Experimental Settings"));

  addFormNumericBox(F("Webserver port"), F("webport"), Settings.WebserverPort, 0, 65535);
  addFormNote(F("Requires reboot to activate"));

  addFormNumericBox(F("Fixed IP Octet"), F("ip"),           Settings.IP_Octet,     0, 255);

  addFormNumericBox(F("WD I2C Address"), F("wdi2caddress"), Settings.WDI2CAddress, 0, 127);
  addHtml(F(" (decimal)"));

  addFormNumericBox(F("I2C ClockStretchLimit"), F("wireclockstretchlimit"), Settings.WireClockStretchLimit); // TODO define limits
  #if defined(FEATURE_ARDUINO_OTA)
  addFormCheckBox(F("Enable Arduino OTA"), F("arduinootaenable"), Settings.ArduinoOTAEnable);
  #endif // if defined(FEATURE_ARDUINO_OTA)
  #if defined(ESP32)
  addFormCheckBox_disabled(F("Enable RTOS Multitasking"), F("usertosmultitasking"), Settings.UseRTOSMultitasking);
  #endif // if defined(ESP32)

  addFormCheckBox(LabelType::JSON_BOOL_QUOTES, Settings.JSONBoolWithoutQuotes());
  #ifdef USES_TIMING_STATS
  addFormCheckBox(LabelType::ENABLE_TIMING_STATISTICS, Settings.EnableTimingStats());
  #endif
  #ifdef ESP8266
  addFormCheckBox(LabelType::DEEP_SLEEP_ALTERNATIVE_CALL, Settings.UseAlternativeDeepSleep());
  #endif


  #ifdef USES_SSDP
  addFormCheckBox_disabled(F("Use SSDP"), F("usessdp"), Settings.UseSSDP);
  #endif // ifdef USES_SSDP

  addFormNumericBox(LabelType::CONNECTION_FAIL_THRESH, Settings.ConnectionFailuresThreshold, 0, 100);
#ifdef ESP8266
  addFormCheckBox(LabelType::FORCE_WIFI_BG, Settings.ForceWiFi_bg_mode());
#endif // ifdef ESP8266
#ifdef ESP32

  // Disabled for now, since it is not working properly.
  addFormCheckBox_disabled(LabelType::FORCE_WIFI_BG, Settings.ForceWiFi_bg_mode());
#endif // ifdef ESP32

  addFormCheckBox(LabelType::RESTART_WIFI_LOST_CONN, Settings.WiFiRestart_connection_lost());
#ifdef ESP8266
  addFormCheckBox(LabelType::FORCE_WIFI_NOSLEEP,     Settings.WifiNoneSleep());
#endif // ifdef ESP8266
  addFormNote(F("Change WiFi sleep settings requires reboot to activate"));
#ifdef SUPPORT_ARP
  addFormCheckBox(LabelType::PERIODICAL_GRAT_ARP, Settings.gratuitousARP());
#endif // ifdef SUPPORT_ARP
  addFormCheckBox(LabelType::CPU_ECO_MODE,        Settings.EcoPowerMode());
  addFormNote(F("Node may miss receiving packets with Eco mode enabled"));
  {
    float maxTXpwr;
    float threshold = GetRSSIthreshold(maxTXpwr);
    addFormFloatNumberBox(LabelType::WIFI_TX_MAX_PWR, Settings.getWiFi_TX_power(), 0.0f, 20.5f, 2, 0.25f);
    addUnit(F("dBm"));
    String note;
    note = F("Current max: ");
    note += String(maxTXpwr, 2);
    note += F(" dBm");
    addFormNote(note);

    addFormNumericBox(LabelType::WIFI_SENS_MARGIN, Settings.WiFi_sensitivity_margin, -20, 30);
    addUnit(F("dB")); // Relative, thus the unit is dB, not dBm
    note = F("Adjust TX power to target the AP with (threshold + margin) dBm signal strength. Current threshold: ");
    note += String(threshold, 2);
    note += F(" dBm");
    addFormNote(note);
  }
  addFormCheckBox(LabelType::WIFI_SEND_AT_MAX_TX_PWR, Settings.UseMaxTXpowerForSending());
  {
    addFormNumericBox(LabelType::WIFI_NR_EXTRA_SCANS, Settings.NumberExtraWiFiScans, 0, 5);
    String note = F("Number of extra times to scan all channels to have higher chance of finding the desired AP");
    addFormNote(note);
  }
  addFormCheckBox(LabelType::WIFI_PERIODICAL_SCAN, Settings.PeriodicalScanWiFi());
  addFormCheckBox(LabelType::WIFI_USE_LAST_CONN_FROM_RTC, Settings.UseLastWiFiFromRTC());



#ifdef USES_ESPEASY_NOW
  addFormCheckBox(LabelType::USE_ESPEASY_NOW, Settings.UseESPEasyNow());
  {
    addFormNumericBox(LabelType::FORCE_ESPEASY_NOW_CHANNEL, Settings.ForceESPEasyNOWchannel, 0, 14);
    String note = F("Force channel to use for ");
    note += F(ESPEASY_NOW_NAME);
    note += F("-only mode (0 = use any channel)");
    addFormNote(note);
  }

#endif

  addFormSeparator(2);

  html_TR_TD();
  html_TD();
  addSubmitButton();
  addHtml(F("<input type='hidden' name='edit' value='1'>"));
  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();
}

void addFormDstSelect(bool isStart, uint16_t choice) {
  uint16_t tmpstart(choice);
  uint16_t tmpend(choice);

  if (!TimeChangeRule(choice, 0).isValid()) {
    time_zone.getDefaultDst_flash_values(tmpstart, tmpend);
  }
  TimeChangeRule rule(isStart ? tmpstart : tmpend, 0);
  {
    String weeklabel = isStart ? F("Start")  : F("End");
    weeklabel += F(" (week, dow, month)");
    String weekid  = isStart ? F("dststartweek")  : F("dstendweek");
    const __FlashStringHelper *  week[5] = { F("Last"), F("1st"), F("2nd"), F("3rd"), F("4th") };
    int    weekValues[5] = { 0, 1, 2, 3, 4 };

    addRowLabel(weeklabel);
    addSelector(weekid, 5, week, weekValues, NULL, rule.week);
  }
  html_BR();
  {
    String dowid   = isStart ? F("dststartdow")   : F("dstenddow");
    const __FlashStringHelper *  dow[7] = { F("Sun"), F("Mon"), F("Tue"), F("Wed"), F("Thu"), F("Fri"), F("Sat") };
    int    dowValues[7]  = { 1, 2, 3, 4, 5, 6, 7 };

    addSelector(dowid, 7, dow, dowValues, NULL, rule.dow);
  }
  html_BR();
  {
    String monthid = isStart ? F("dststartmonth") : F("dstendmonth");
    const __FlashStringHelper * month[12] = { F("Jan"), F("Feb"), F("Mar"), F("Apr"), F("May"), F("Jun"), F("Jul"), F("Aug"), F("Sep"), F("Oct"), F("Nov"), F(
                             "Dec") };
    int    monthValues[12] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

    addSelector(monthid, 12, month, monthValues, NULL, rule.month);
  }
  {
    String hourid  = isStart ? F("dststarthour")  : F("dstendhour");
    String hourlabel = isStart ? F("Start (localtime, e.g. 2h&rarr;3h)")  : F("End (localtime, e.g. 3h&rarr;2h)");

    addFormNumericBox(hourlabel, hourid, rule.hour, 0, 23);
    addUnit(isStart ? F("hour &#x21b7;") : F("hour &#x21b6;"));
  }
}

void addFormLogLevelSelect(LabelType::Enum label, int choice)
{
  addRowLabel(getLabel(label));
  const __FlashStringHelper * options[LOG_LEVEL_NRELEMENTS + 1];
  int    optionValues[LOG_LEVEL_NRELEMENTS + 1] = { 0 };

  options[0]      = getLogLevelDisplayString(0);

  for (int i = 0; i < LOG_LEVEL_NRELEMENTS; ++i) {
    options[i + 1] = getLogLevelDisplayStringFromIndex(i, optionValues[i + 1]);
  }
  addSelector(getInternalLabel(label), LOG_LEVEL_NRELEMENTS + 1, options, optionValues, NULL, choice);

}

void addFormLogFacilitySelect(const __FlashStringHelper * label, const __FlashStringHelper * id, int choice)
{
  addRowLabel(label);
  const __FlashStringHelper * options[12] =
  { F("Kernel"), F("User"),   F("Daemon"),   F("Message"), F("Local0"),  F("Local1"),
    F("Local2"), F("Local3"), F("Local4"),   F("Local5"),  F("Local6"),  F("Local7") };
  int optionValues[12] = { 0, 1, 3, 5, 16, 17, 18, 19, 20, 21, 22, 23 };

  addSelector(id, 12, options, optionValues, NULL, choice);
}

#endif // ifdef WEBSERVER_ADVANCED
