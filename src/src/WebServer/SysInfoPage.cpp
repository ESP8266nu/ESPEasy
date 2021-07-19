#include "../WebServer/SysInfoPage.h"

#include "../WebServer/WebServer.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"


#include "../../ESPEasy-Globals.h"

#include "../Commands/Diagnostic.h"

#include "../DataStructs/RTCStruct.h"

#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"

#include "../Globals/CRCValues.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/NetworkState.h"
#include "../Globals/RTC.h"

#include "../Helpers/CompiletimeDefines.h"
#include "../Helpers/ESPEasyStatistics.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Memory.h"
#include "../Helpers/Misc.h"
#include "../Helpers/OTA.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_GPIO.h"
#include "../Helpers/StringGenerator_System.h"

#include "../Static/WebStaticData.h"

#ifdef USES_MQTT
# include "../Globals/MQTT.h"
# include "../Helpers/PeriodicalActions.h" // For finding enabled MQTT controller
#endif

#ifdef ESP32
# include <esp_partition.h>
#endif // ifdef ESP32


#ifdef WEBSERVER_NEW_UI

// ********************************************************************************
// Web Interface sysinfo page
// ********************************************************************************
void handle_sysinfo_json() {
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_sysinfo"));
  # endif // ifndef BUILD_NO_RAM_TRACKER

  if (!isLoggedIn()) { return; }
  TXBuffer.startJsonStream();
  json_init();
  json_open();
  json_open(false, F("general"));
  json_number(F("unit"), String(Settings.Unit));
  json_prop(F("time"),   node_time.getDateTimeString('-', ':', ' '));
  json_prop(F("uptime"), getExtendedValue(LabelType::UPTIME));
  json_number(F("cpu_load"),   String(getCPUload()));
  json_number(F("loop_count"), String(getLoopCountPerSec()));
  json_close();

  int freeMem = ESP.getFreeHeap();
  json_open(false, F("mem"));
  json_number(F("free"),    String(freeMem));
  json_number(F("low_ram"), String(
  # ifndef BUILD_NO_RAM_TRACKER
                lowestRAM
  # else // ifndef BUILD_NO_RAM_TRACKER
                0
  # endif // ifndef BUILD_NO_RAM_TRACKER
                ));
  json_prop(F("low_ram_fn"),
  # ifndef BUILD_NO_RAM_TRACKER
            lowestRAMfunction
  # else // ifndef BUILD_NO_RAM_TRACKER
            0
  # endif // ifndef BUILD_NO_RAM_TRACKER
            );
  json_number(F("stack"),     String(getCurrentFreeStack()));
  json_number(F("low_stack"), String(
  # ifndef BUILD_NO_RAM_TRACKER
                lowestFreeStack
  # else // ifndef BUILD_NO_RAM_TRACKER
                0
  # endif // ifndef BUILD_NO_RAM_TRACKER
                ));
  json_prop(F("low_stack_fn"),
  # ifndef BUILD_NO_RAM_TRACKER
            lowestFreeStackfunction
  # else // ifndef BUILD_NO_RAM_TRACKER
            0
  # endif // ifndef BUILD_NO_RAM_TRACKER
            );
  json_close();

  json_open(false, F("boot"));
  json_prop(F("last_cause"), getLastBootCauseString());
  json_number(F("counter"), String(RTC.bootCounter));
  json_prop(F("reset_reason"), getResetReasonString());
  json_close();

  json_open(false, F("wifi"));
  json_prop(F("type"), toString(getConnectionProtocol()));
  json_number(F("rssi"), String(WiFi.RSSI()));
  json_prop(F("dhcp"),          useStaticIP() ? getLabel(LabelType::IP_CONFIG_STATIC) : getLabel(LabelType::IP_CONFIG_DYNAMIC));
  json_prop(F("ip"),            getValue(LabelType::IP_ADDRESS));
  json_prop(F("subnet"),        getValue(LabelType::IP_SUBNET));
  json_prop(F("gw"),            getValue(LabelType::GATEWAY));
  json_prop(F("dns1"),          getValue(LabelType::DNS_1));
  json_prop(F("dns2"),          getValue(LabelType::DNS_2));
  json_prop(F("allowed_range"), describeAllowedIPrange());
  json_prop(F("sta_mac"),       getValue(LabelType::STA_MAC));
  json_prop(F("ap_mac"),        getValue(LabelType::AP_MAC));
  json_prop(F("ssid"),          getValue(LabelType::SSID));
  json_prop(F("bssid"),         getValue(LabelType::BSSID));
  json_number(F("channel"),     getValue(LabelType::CHANNEL));
  json_prop(F("encryption"),    getValue(LabelType::ENCRYPTION_TYPE_STA));
  json_prop(F("connected"),     getValue(LabelType::CONNECTED));
  json_prop(F("ldr"),           getValue(LabelType::LAST_DISC_REASON_STR));
  json_number(F("reconnects"),  getValue(LabelType::NUMBER_RECONNECTS));
  json_prop(F("ssid1"),         getValue(LabelType::WIFI_STORED_SSID1));
  json_prop(F("ssid2"),         getValue(LabelType::WIFI_STORED_SSID2));
  json_close();

# ifdef HAS_ETHERNET
  json_open(false, F("ethernet"));
  json_prop(F("ethwifimode"),   getValue(LabelType::ETH_WIFI_MODE));
  json_prop(F("ethconnected"),  getValue(LabelType::ETH_CONNECTED));
  json_prop(F("ethduplex"),     getValue(LabelType::ETH_DUPLEX));
  json_prop(F("ethspeed"),      getValue(LabelType::ETH_SPEED));
  json_prop(F("ethstate"),      getValue(LabelType::ETH_STATE));
  json_prop(F("ethspeedstate"), getValue(LabelType::ETH_SPEED_STATE));
  json_close();
# endif // ifdef HAS_ETHERNET

  json_open(false, F("firmware"));
  json_prop(F("build"),       String(BUILD));
  json_prop(F("notes"),       F(BUILD_NOTES));
  json_prop(F("libraries"),   getSystemLibraryString());
  json_prop(F("git_version"), getValue(LabelType::GIT_BUILD));
  json_prop(F("plugins"),     getPluginDescriptionString());
  json_prop(F("md5"),         String(CRCValues.compileTimeMD5[0], HEX));
  json_number(F("md5_check"), String(CRCValues.checkPassed()));
  json_prop(F("build_time"),     get_build_time());
  json_prop(F("filename"),       getValue(LabelType::BINARY_FILENAME));
  json_prop(F("build_platform"), getValue(LabelType::BUILD_PLATFORM));
  json_prop(F("git_head"),       getValue(LabelType::GIT_HEAD));
  json_close();

  json_open(false, F("esp"));
  json_prop(F("chip_id"), getValue(LabelType::ESP_CHIP_ID));
  json_number(F("cpu"), getValue(LabelType::ESP_CHIP_FREQ));

  # ifdef ARDUINO_BOARD
  json_prop(F("board"), ARDUINO_BOARD);
  # endif // ifdef ARDUINO_BOARD
  json_close();
  json_open(false, F("storage"));

  # if defined(ESP8266)
  uint32_t flashChipId = ESP.getFlashChipId();

  // Set to HEX may be something like 0x1640E0.
  // Where manufacturer is 0xE0 and device is 0x4016.
  json_number(F("chip_id"), String(flashChipId));

  if (flashChipVendorPuya())
  {
    if (puyaSupport()) {
      json_prop(F("vendor"), F("puya, supported"));
    } else {
      json_prop(F("vendor"), F("puya, error"));
    }
  }
  uint32_t flashDevice = (flashChipId & 0xFF00) | ((flashChipId >> 16) & 0xFF);
  json_number(F("device"),    String(flashDevice));
  # endif // if defined(ESP8266)
  json_number(F("real_size"), String(getFlashRealSizeInBytes() / 1024));
  json_number(F("ide_size"),  String(ESP.getFlashChipSize() / 1024));

  // Please check what is supported for the ESP32
  # if defined(ESP8266)
  json_number(F("flash_speed"), String(ESP.getFlashChipSpeed() / 1000000));

  FlashMode_t ideMode = ESP.getFlashChipMode();

  switch (ideMode) {
    case FM_QIO:   json_prop(F("mode"), F("QIO"));  break;
    case FM_QOUT:  json_prop(F("mode"), F("QOUT")); break;
    case FM_DIO:   json_prop(F("mode"), F("DIO"));  break;
    case FM_DOUT:  json_prop(F("mode"), F("DOUT")); break;
    default:
      json_prop(F("mode"), getUnknownString()); break;
  }
  # endif // if defined(ESP8266)

  json_number(F("writes"),        String(RTC.flashDayCounter));
  json_number(F("flash_counter"), String(RTC.flashCounter));
  json_number(F("sketch_size"),   String(ESP.getSketchSize() / 1024));
  json_number(F("sketch_free"),   String(ESP.getFreeSketchSpace() / 1024));

  json_number(F("spiffs_size"),   String(SpiffsTotalBytes() / 1024));
  json_number(F("spiffs_free"),   String(SpiffsFreeSpace() / 1024));
  json_close();
  json_close();

  TXBuffer.endStream();
}

#endif // WEBSERVER_NEW_UI

#ifdef WEBSERVER_SYSINFO

void handle_sysinfo() {
  # ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_sysinfo"));
  # endif // ifndef BUILD_NO_RAM_TRACKER

  if (!isLoggedIn()) { return; }
  navMenuIndex = MENU_INDEX_TOOLS;
  html_reset_copyTextCounter();
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate();

  addHtml(printWebString);
  addHtml(F("<form>"));

  // the table header
  html_table_class_normal();


  # ifdef WEBSERVER_GITHUB_COPY

  // Not using addFormHeader() to get the copy button on the same header line as 2nd column
  html_TR();
  html_table_header(F("System Info"), 225);
  addHtml(F("<TH>")); // Needed to get the copy button on the same header line.
  addCopyButton(F("copyText"), F("\\n"), F("Copy info to clipboard"));

  TXBuffer += githublogo;
  serve_JS(JSfiles_e::GitHubClipboard);

  # else // ifdef WEBSERVER_GITHUB_COPY
  addFormHeader(F("System Info"));

  # endif // ifdef WEBSERVER_GITHUB_COPY

  handle_sysinfo_basicInfo();

  handle_sysinfo_memory();

  handle_sysinfo_Network();

# ifdef HAS_ETHERNET
  handle_sysinfo_Ethernet();
# endif // ifdef HAS_ETHERNET

  handle_sysinfo_WiFiSettings();

  handle_sysinfo_Firmware();

  handle_sysinfo_SystemStatus();

  handle_sysinfo_NetworkServices();

  handle_sysinfo_ESP_Board();

  handle_sysinfo_Storage();


  html_end_table();
  html_end_form();
  sendHeadandTail_stdtemplate(true);
  TXBuffer.endStream();
}

void handle_sysinfo_basicInfo() {
  addRowLabelValue(LabelType::UNIT_NR);

  if (node_time.systemTimePresent())
  {
    addRowLabelValue(LabelType::LOCAL_TIME);
  }

  addRowLabel(LabelType::UPTIME);
  {
    addHtml(getExtendedValue(LabelType::UPTIME));
  }

  addRowLabel(LabelType::LOAD_PCT);

  if (wdcounter > 0)
  {
    String html;
    html.reserve(32);
    html += getCPUload();
    html += F("% (LC=");
    html += getLoopCountPerSec();
    html += ')';
    addHtml(html);
  }
  addRowLabelValue(LabelType::CPU_ECO_MODE);


  addRowLabel(F("Boot"));
  {
    String html;
    html.reserve(64);

    html += getLastBootCauseString();
    html += " (";
    html += RTC.bootCounter;
    html += ')';
    addHtml(html);
  }
  addRowLabelValue(LabelType::RESET_REASON);
  addRowLabelValue(LabelType::LAST_TASK_BEFORE_REBOOT);
  addRowLabelValue(LabelType::SW_WD_COUNT);
}

void handle_sysinfo_memory() {
  addTableSeparator(F("Memory"), 2, 3);

# ifdef ESP32
  addRowLabelValue(LabelType::HEAP_SIZE);
  addRowLabelValue(LabelType::HEAP_MIN_FREE);
# endif // ifdef ESP32

  int freeMem = ESP.getFreeHeap();
  addRowLabel(LabelType::FREE_MEM);
  {
    String html;
    html.reserve(64);

    html += freeMem;
# ifndef BUILD_NO_RAM_TRACKER
    html += " (";
    html += lowestRAM;
    html += F(" - ");
    html += lowestRAMfunction;
    html += ')';
# endif // ifndef BUILD_NO_RAM_TRACKER
    addHtml(html);
  }
# if defined(CORE_POST_2_5_0) || defined(ESP32)
  addRowLabelValue(LabelType::HEAP_MAX_FREE_BLOCK);
# endif // if defined(CORE_POST_2_5_0) || defined(ESP32)
# if defined(CORE_POST_2_5_0)
  addRowLabelValue(LabelType::HEAP_FRAGMENTATION);
  addHtml('%');
# endif // ifdef CORE_POST_2_5_0
  {
    #ifdef USE_SECOND_HEAP
    addRowLabelValue(LabelType::FREE_HEAP_IRAM);
    #endif
  }



  addRowLabel(LabelType::FREE_STACK);
  {
    String html;
    html.reserve(64);
    html += getCurrentFreeStack();
# ifndef BUILD_NO_RAM_TRACKER
    html += " (";
    html += lowestFreeStack;
    html += F(" - ");
    html += lowestFreeStackfunction;
    html += ')';
# endif // ifndef BUILD_NO_RAM_TRACKER
    addHtml(html);
  }

# if defined(ESP32) && defined(ESP32_ENABLE_PSRAM)

  if (ESP.getPsramSize() > 0) {
    addRowLabelValue(LabelType::PSRAM_SIZE);
    addRowLabelValue(LabelType::PSRAM_FREE);
    addRowLabelValue(LabelType::PSRAM_MIN_FREE);
    addRowLabelValue(LabelType::PSRAM_MAX_FREE_BLOCK);
  }
# endif // if defined(ESP32) && defined(ESP32_ENABLE_PSRAM)
}

# ifdef HAS_ETHERNET
void handle_sysinfo_Ethernet() {
  if (active_network_medium == NetworkMedium_t::Ethernet) {
    addTableSeparator(F("Ethernet"), 2, 3);
    addRowLabelValue(LabelType::ETH_STATE);
    addRowLabelValue(LabelType::ETH_SPEED);
    addRowLabelValue(LabelType::ETH_DUPLEX);
    addRowLabelValue(LabelType::ETH_MAC);
//    addRowLabelValue(LabelType::ETH_IP_ADDRESS_SUBNET);
//    addRowLabelValue(LabelType::ETH_IP_GATEWAY);
//    addRowLabelValue(LabelType::ETH_IP_DNS);
  }
}

# endif // ifdef HAS_ETHERNET

void handle_sysinfo_Network() {
  addTableSeparator(F("Network"), 2, 3);

  # ifdef HAS_ETHERNET
  addRowLabelValue(LabelType::ETH_WIFI_MODE);
  # endif // ifdef HAS_ETHERNET

  addRowLabelValue(LabelType::IP_CONFIG);
  addRowLabelValue(LabelType::IP_ADDRESS_SUBNET);
  addRowLabelValue(LabelType::GATEWAY);
  addRowLabelValue(LabelType::CLIENT_IP);
  addRowLabelValue(LabelType::DNS);
  addRowLabelValue(LabelType::ALLOWED_IP_RANGE);
  addRowLabelValue(LabelType::CONNECTED);
  addRowLabelValue(LabelType::NUMBER_RECONNECTS);

  addTableSeparator(F("WiFi"), 2, 3, F("Wifi"));

  const bool showWiFiConnectionInfo = !WiFiEventData.WiFiDisconnected();


  addRowLabel(LabelType::WIFI_CONNECTION);
  if (showWiFiConnectionInfo)
  {
    addHtml(toString(getConnectionProtocol()));
    addHtml(F(" (RSSI "));
    addHtmlInt(WiFi.RSSI());
    addHtml(F(" dBm)"));
  } else addHtml('-');

  addRowLabel(LabelType::SSID);
  if (showWiFiConnectionInfo)
  {
    String html;
    html.reserve(64);

    html += WiFi.SSID();
    html += " (";
    html += WiFi.BSSIDstr();
    html += ')';
    addHtml(html);
  } else addHtml('-');

  addRowLabel(getLabel(LabelType::CHANNEL));
  if (showWiFiConnectionInfo) {
    addHtml(getValue(LabelType::CHANNEL));
  } else addHtml('-');

  addRowLabel(getLabel(LabelType::ENCRYPTION_TYPE_STA));
  if (showWiFiConnectionInfo) {
    addHtml(getValue(LabelType::ENCRYPTION_TYPE_STA));
  } else addHtml('-');

  if (active_network_medium == NetworkMedium_t::WIFI)
  {
    addRowLabel(LabelType::LAST_DISCONNECT_REASON);
    addHtml(getValue(LabelType::LAST_DISC_REASON_STR));
    addRowLabelValue(LabelType::WIFI_STORED_SSID1);
    addRowLabelValue(LabelType::WIFI_STORED_SSID2);
  }

  addRowLabelValue(LabelType::STA_MAC);
  addRowLabelValue(LabelType::AP_MAC);
  html_TR();
}

void handle_sysinfo_WiFiSettings() {
  addTableSeparator(F("WiFi Settings"), 2, 3);
  addRowLabelValue(LabelType::FORCE_WIFI_BG);
  addRowLabelValue(LabelType::RESTART_WIFI_LOST_CONN);
# ifdef ESP8266
  addRowLabelValue(LabelType::FORCE_WIFI_NOSLEEP);
# endif // ifdef ESP8266
# ifdef SUPPORT_ARP
  addRowLabelValue(LabelType::PERIODICAL_GRAT_ARP);
# endif // ifdef SUPPORT_ARP
  addRowLabelValue(LabelType::CONNECTION_FAIL_THRESH);
  addRowLabelValue(LabelType::WIFI_TX_MAX_PWR);
  addRowLabelValue(LabelType::WIFI_CUR_TX_PWR);
  addRowLabelValue(LabelType::WIFI_SENS_MARGIN);
  addRowLabelValue(LabelType::WIFI_SEND_AT_MAX_TX_PWR);
  addRowLabelValue(LabelType::WIFI_NR_EXTRA_SCANS);
  addRowLabelValue(LabelType::WIFI_PERIODICAL_SCAN);
  addRowLabelValue(LabelType::WIFI_USE_LAST_CONN_FROM_RTC);
}

void handle_sysinfo_Firmware() {
  addTableSeparator(F("Firmware"), 2, 3);

  addRowLabelValue_copy(LabelType::BUILD_DESC);
  addHtml(' ');
  addHtml(F(BUILD_NOTES));

  addRowLabelValue_copy(LabelType::SYSTEM_LIBRARIES);
  addRowLabelValue_copy(LabelType::GIT_BUILD);
  addRowLabelValue_copy(LabelType::PLUGIN_COUNT);
  addHtml(' ');
  addHtml(getPluginDescriptionString());

  addRowLabel(F("Build Origin"));
  addHtml(get_build_origin());
  addRowLabelValue_copy(LabelType::BUILD_TIME);
  addRowLabelValue_copy(LabelType::BINARY_FILENAME);
  addRowLabelValue_copy(LabelType::BUILD_PLATFORM);
  addRowLabelValue_copy(LabelType::GIT_HEAD);
}

void handle_sysinfo_SystemStatus() {
  addTableSeparator(F("System Status"), 2, 3);

  // Actual Loglevel
  addRowLabelValue(LabelType::SYSLOG_LOG_LEVEL);
  addRowLabelValue(LabelType::SERIAL_LOG_LEVEL);
  addRowLabelValue(LabelType::WEB_LOG_LEVEL);
    # ifdef FEATURE_SD
  addRowLabelValue(LabelType::SD_LOG_LEVEL);
    # endif // ifdef FEATURE_SD
}

void handle_sysinfo_NetworkServices() {
  addTableSeparator(F("Network Services"), 2, 3);

  addRowLabel(F("Network Connected"));
  addEnabled(NetworkConnected());

  addRowLabel(F("NTP Initialized"));
  addEnabled(statusNTPInitialized);

  #ifdef USES_MQTT
  if (validControllerIndex(firstEnabledMQTT_ControllerIndex())) {
    addRowLabel(F("MQTT Client Connected"));
    addEnabled(MQTTclient_connected);
  }
  #endif
}

void handle_sysinfo_ESP_Board() {
  addTableSeparator(F("ESP Board"), 2, 3);


  addRowLabel(LabelType::ESP_CHIP_ID);
  {
    String html;
    html.reserve(32);
    html += getChipId();
    html += F(" (0x");
    String espChipId(getChipId(), HEX);
    espChipId.toUpperCase();
    html += espChipId;
    html += ')';
    addHtml(html);
  }

  addRowLabel(LabelType::ESP_CHIP_FREQ);
  addHtmlInt(ESP.getCpuFreqMHz());
  addHtml(F(" MHz"));

  addRowLabelValue(LabelType::ESP_CHIP_MODEL);

  # if defined(ESP32)
  addRowLabelValue(LabelType::ESP_CHIP_REVISION);
  # endif // if defined(ESP32)
  addRowLabelValue(LabelType::ESP_CHIP_CORES);

  # ifdef ARDUINO_BOARD
  addRowLabel(LabelType::ESP_BOARD_NAME);
  addHtml(ARDUINO_BOARD);
  # endif // ifdef ARDUINO_BOARD
}

void handle_sysinfo_Storage() {
  addTableSeparator(F("Storage"), 2, 3);

  uint32_t flashChipId = getFlashChipId();

  if (flashChipId != 0) {
    addRowLabel(LabelType::FLASH_CHIP_ID);


    // Set to HEX may be something like 0x1640E0.
    // Where manufacturer is 0xE0 and device is 0x4016.
    addHtml(F("Vendor: "));
    addHtml(formatToHex(flashChipId & 0xFF));

    if (flashChipVendorPuya())
    {
      addHtml(F(" (PUYA"));

      if (puyaSupport()) {
        addHtml(F(", supported"));
      } else {
        addHtml(F(HTML_SYMBOL_WARNING));
      }
      addHtml(')');
    }
    addHtml(F(" Device: "));
    uint32_t flashDevice = (flashChipId & 0xFF00) | ((flashChipId >> 16) & 0xFF);
    addHtml(formatToHex(flashDevice));
  }
  uint32_t realSize = getFlashRealSizeInBytes();
  uint32_t ideSize  = ESP.getFlashChipSize();

  addRowLabel(LabelType::FLASH_CHIP_REAL_SIZE);
  addHtmlInt(realSize / 1024);
  addHtml(F(" kB"));

  addRowLabel(LabelType::FLASH_IDE_SIZE);
  addHtmlInt(ideSize / 1024);
  addHtml(F(" kB"));

  // Please check what is supported for the ESP32
  # if defined(ESP8266)
  addRowLabel(LabelType::FLASH_IDE_SPEED);
  addHtmlInt(ESP.getFlashChipSpeed() / 1000000);
  addHtml(F(" MHz"));

  FlashMode_t ideMode = ESP.getFlashChipMode();
  addRowLabel(LabelType::FLASH_IDE_MODE);
  {
    String html;

    switch (ideMode) {
      case FM_QIO:   html += F("QIO");  break;
      case FM_QOUT:  html += F("QOUT"); break;
      case FM_DIO:   html += F("DIO");  break;
      case FM_DOUT:  html += F("DOUT"); break;
      default:
        html += getUnknownString(); break;
    }
    addHtml(html);
  }
  # endif // if defined(ESP8266)

  addRowLabel(LabelType::FLASH_WRITE_COUNT);
  {
    String html;
    html.reserve(32);
    html += RTC.flashDayCounter;
    html += F(" daily / ");
    html += RTC.flashCounter;
    html += F(" boot");
    addHtml(html);
  }

  {
    // FIXME TD-er: Must also add this for ESP32.
    addRowLabel(LabelType::SKETCH_SIZE);
    {
      String html;
      html.reserve(32);
      html += ESP.getSketchSize() / 1024;
      html += F(" kB (");
      html += ESP.getFreeSketchSpace() / 1024;
      html += F(" kB free)");
      addHtml(html);
    }

    uint32_t maxSketchSize;
    bool     use2step;
    # if defined(ESP8266)
    bool otaEnabled =
    # endif // if defined(ESP8266)
    OTA_possible(maxSketchSize, use2step);
    addRowLabel(LabelType::MAX_OTA_SKETCH_SIZE);
    {
      String html;
      html.reserve(32);

      html += maxSketchSize / 1024;
      html += F(" kB (");
      html += maxSketchSize;
      html += F(" bytes)");
      addHtml(html);
    }

    # if defined(ESP8266)
    addRowLabel(LabelType::OTA_POSSIBLE);
    addHtml(boolToString(otaEnabled));

    addRowLabel(LabelType::OTA_2STEP);
    addHtml(boolToString(use2step));
    # endif // if defined(ESP8266)
  }

  addRowLabel(LabelType::FS_SIZE);
  {
    String html;
    html.reserve(32);

    html += SpiffsTotalBytes() / 1024;
    html += F(" kB (");
    html += SpiffsFreeSpace() / 1024;
    html += F(" kB free)");
    addHtml(html);
  }
  # ifndef LIMIT_BUILD_SIZE
  addRowLabel(F("Page size"));
  addHtmlInt(SpiffsPagesize());

  addRowLabel(F("Block size"));
  addHtmlInt(SpiffsBlocksize());

  addRowLabel(F("Number of blocks"));
  addHtmlInt(SpiffsTotalBytes() / SpiffsBlocksize());

  {
  #  if defined(ESP8266)
    fs::FSInfo fs_info;
    ESPEASY_FS.info(fs_info);
    addRowLabel(F("Maximum open files"));
    addHtmlInt(fs_info.maxOpenFiles);

    addRowLabel(F("Maximum path length"));
    addHtmlInt(fs_info.maxPathLength);

  #  endif // if defined(ESP8266)
  }
  # endif // ifndef LIMIT_BUILD_SIZE

# ifndef BUILD_MINIMAL_OTA

  if (showSettingsFileLayout) {
    addTableSeparator(F("Settings Files"), 2, 3);
    html_TR_TD();
    addHtml(F("Layout Settings File"));
    html_TD();
    getConfig_dat_file_layout();
    html_TR_TD();
    html_TD();
    addHtml(F("(offset / size per item / index)"));

    for (int st = 0; st < static_cast<int>(SettingsType::Enum::SettingsType_MAX); ++st) {
      SettingsType::Enum settingsType = static_cast<SettingsType::Enum>(st);
      html_TR_TD();
      addHtml(SettingsType::getSettingsTypeString(settingsType));
      html_BR();
      addHtml(SettingsType::getSettingsFileName(settingsType));
      html_TD();
      getStorageTableSVG(settingsType);
    }
  }
# endif // ifndef BUILD_MINIMAL_OTA

  # ifdef ESP32
  addTableSeparator(F("Partitions"), 2, 3,
                    F("https://dl.espressif.com/doc/esp-idf/latest/api-guides/partition-tables.html"));

  addRowLabel(F("Data Partition Table"));

  //   TXBuffer += getPartitionTableHeader(F(" - "), F("<BR>"));
  //   TXBuffer += getPartitionTable(ESP_PARTITION_TYPE_DATA, F(" - "), F("<BR>"));
  getPartitionTableSVG(ESP_PARTITION_TYPE_DATA, 0x5856e6);

  addRowLabel(F("App Partition Table"));

  //   TXBuffer += getPartitionTableHeader(F(" - "), F("<BR>"));
  //   TXBuffer += getPartitionTable(ESP_PARTITION_TYPE_APP , F(" - "), F("<BR>"));
  getPartitionTableSVG(ESP_PARTITION_TYPE_APP, 0xab56e6);
  # endif // ifdef ESP32
}

#endif    // ifdef WEBSERVER_SYSINFO
