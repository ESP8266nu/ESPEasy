#include "../WebServer/WebServer.h"

#include "../WebServer/404.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/AdvancedConfigPage.h"
#include "../WebServer/CacheControllerPages.h"
#include "../WebServer/ConfigPage.h"
#include "../WebServer/ControlPage.h"
#include "../WebServer/ControllerPage.h"
#include "../WebServer/CustomPage.h"
#include "../WebServer/DevicesPage.h"
#include "../WebServer/DownloadPage.h"
#include "../WebServer/FactoryResetPage.h"
#include "../WebServer/Favicon.h"
#include "../WebServer/FileList.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/HardwarePage.h"
#include "../WebServer/I2C_Scanner.h"
#include "../WebServer/JSON.h"
#include "../WebServer/LoadFromFS.h"
#include "../WebServer/Log.h"
#include "../WebServer/Login.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"
#include "../WebServer/NotificationPage.h"
#include "../WebServer/PinStates.h"
#include "../WebServer/RootPage.h"
#include "../WebServer/Rules.h"
#include "../WebServer/SettingsArchive.h"
#include "../WebServer/SetupPage.h"
#include "../WebServer/SysInfoPage.h"
#include "../WebServer/SysVarPage.h"
#include "../WebServer/TimingStats.h"
#include "../WebServer/ToolsPage.h"
#include "../WebServer/UploadPage.h"
#include "../WebServer/WiFiScanner.h"


#include "../../ESPEasy-Globals.h"
#include "../../_Plugin_Helper.h"
#include "../../ESPEasy_common.h"

#include "../DataStructs/TimingStats.h"

#include "../DataTypes/SettingsType.h"

#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"

#include "../Globals/CPlugins.h"
#include "../Globals/Device.h"
#include "../Globals/ExtraTaskSettings.h"
#include "../Globals/NetworkState.h"
#include "../Globals/Protocol.h"
#include "../Globals/SecuritySettings.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Networking.h"
#include "../Helpers/OTA.h"
#include "../Helpers/StringConverter.h"

#include "../Static/WebStaticData.h"

// Determine what pages should be visible
#ifndef MENU_INDEX_MAIN_VISIBLE
  # define MENU_INDEX_MAIN_VISIBLE true
#endif // ifndef MENU_INDEX_MAIN_VISIBLE

#ifndef MENU_INDEX_CONFIG_VISIBLE
  # define MENU_INDEX_CONFIG_VISIBLE true
#endif // ifndef MENU_INDEX_CONFIG_VISIBLE

#ifndef MENU_INDEX_CONTROLLERS_VISIBLE
  # define MENU_INDEX_CONTROLLERS_VISIBLE true
#endif // ifndef MENU_INDEX_CONTROLLERS_VISIBLE

#ifndef MENU_INDEX_HARDWARE_VISIBLE
  # define MENU_INDEX_HARDWARE_VISIBLE true
#endif // ifndef MENU_INDEX_HARDWARE_VISIBLE

#ifndef MENU_INDEX_DEVICES_VISIBLE
  # define MENU_INDEX_DEVICES_VISIBLE true
#endif // ifndef MENU_INDEX_DEVICES_VISIBLE

#ifndef MENU_INDEX_RULES_VISIBLE
  # define MENU_INDEX_RULES_VISIBLE true
#endif // ifndef MENU_INDEX_RULES_VISIBLE

#ifndef MENU_INDEX_NOTIFICATIONS_VISIBLE
  # define MENU_INDEX_NOTIFICATIONS_VISIBLE true
#endif // ifndef MENU_INDEX_NOTIFICATIONS_VISIBLE

#ifndef MENU_INDEX_TOOLS_VISIBLE
  # define MENU_INDEX_TOOLS_VISIBLE true
#endif // ifndef MENU_INDEX_TOOLS_VISIBLE


#if defined(NOTIFIER_SET_NONE) && defined(MENU_INDEX_NOTIFICATIONS_VISIBLE)
  #undef MENU_INDEX_NOTIFICATIONS_VISIBLE
  #define MENU_INDEX_NOTIFICATIONS_VISIBLE false
#endif



void safe_strncpy_webserver_arg(char *dest, const String& arg, size_t max_size) {
  if (web_server.hasArg(arg)) { 
    safe_strncpy(dest, webArg(arg).c_str(), max_size); 
  }
}

void sendHeadandTail(const String& tmplName, boolean Tail, boolean rebooting) {
  // This function is called twice per serving a web page.
  // So it must keep track of the timer longer than the scope of this function.
  // Therefore use a local static variable.
  #ifdef USES_TIMING_STATS
  static unsigned statisticsTimerStart = 0;

  if (!Tail) {
    statisticsTimerStart = micros();
  }
  #endif // ifdef USES_TIMING_STATS
  {
    String pageTemplate;
    String fileName = tmplName;

    fileName += F(".htm");
    fs::File f = tryOpenFile(fileName, "r");

    if (f) {
      pageTemplate.reserve(f.size());

      while (f.available()) { pageTemplate += (char)f.read(); }
      f.close();
    } else {
      // TODO TD-er: Should send data directly to TXBuffer instead of using large strings.
      getWebPageTemplateDefault(tmplName, pageTemplate);
    }
    #ifndef BUILD_NO_RAM_TRACKER
    checkRAM(F("sendWebPage"));
    #endif // ifndef BUILD_NO_RAM_TRACKER

    // web activity timer
    lastWeb = millis();

    if (Tail) {
      int pos = pageTemplate.indexOf(F("{{content}}"));
      if (pos >= 0) {
        pos += 11; // Size of "{{content}}"
        const int length = pageTemplate.length();
        // Prevent copy'ing the string, just stream directly to the buffer.
        for (int i = pos; i < length; ++i) {
          addHtml(pageTemplate[i]);
        }
      }
    } else {
      int indexStart = 0;
      int indexEnd   = 0;
      int readPos    = 0; // Position of data sent to TXBuffer
      String varName;     // , varValue;

      while ((indexStart = pageTemplate.indexOf(F("{{"), indexStart)) >= 0) {
        for (int i = readPos; i < indexStart; ++i) {
          addHtml(pageTemplate[i]);
        }
        readPos = indexStart;

        if ((indexEnd = pageTemplate.indexOf(F("}}"), indexStart)) > 0) {
          varName    = pageTemplate.substring(indexStart + 2, indexEnd);
          indexStart = indexEnd + 2;
          readPos    = indexEnd + 2;
          varName.toLowerCase();

          if (varName == F("content")) { // is var == page content?
            break;                       // send first part of result only
          } else if (varName == F("error")) {
            getErrorNotifications();
          }
          else if (varName == F("meta")) {
            if (rebooting) {
              addHtml(F("<meta http-equiv='refresh' content='10 url=/'>"));
            }
          }
          else {
            getWebPageTemplateVar(varName);
          }
        } else { // no closing "}}"
          // eat "{{"
          readPos    += 2;
          indexStart += 2;
        }
      }
    }
  }

  if (shouldReboot) {
    // we only add this here as a seperate chunk to prevent using too much memory at once
    serve_JS(JSfiles_e::Reboot);
  }
  STOP_TIMER(HANDLE_SERVING_WEBPAGE);
}

void sendHeadandTail_stdtemplate(boolean Tail, boolean rebooting) {
  sendHeadandTail(F("TmplStd"), Tail, rebooting);

  if (!Tail) {
    if (!clientIPinSubnet() && WifiIsAP(WiFi.getMode()) && (WiFi.softAPgetStationNum() > 0)) {
      addHtmlError(F("Warning: Connected via AP"));
    }

    #ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      const int nrArgs = web_server.args();

      if (nrArgs > 0) {
        String log = F(" Webserver args:");

        for (int i = 0; i < nrArgs; ++i) {
          log += ' ';
          log += i;
          log += F(": '");
          log += web_server.argName(i);
          log += F("' length: ");
          log += webArg(i).length();
        }
        addLog(LOG_LEVEL_INFO, log);
      }
    }
    #endif // ifndef BUILD_NO_DEBUG
  }
}

size_t streamFile_htmlEscape(const String& fileName)
{
  fs::File f    = tryOpenFile(fileName, "r");
  size_t   size = 0;

  if (f)
  {
    while (f.available())
    {
      char c = (char)f.read();

      String escaped;

      if (htmlEscapeChar(c, escaped)) {
        addHtml(escaped);
      } else {
        addHtml(c);
      }
      ++size;
    }
    f.close();
  }
  return size;
}


bool captivePortal() {
  const bool fromAP = web_server.client().localIP() == apIP;
  const bool hasWiFiCredentials = SecuritySettings.hasWiFiCredentials();
  if (hasWiFiCredentials || !fromAP) {
    return false;
  }
  if (!isIP(web_server.hostHeader()) && web_server.hostHeader() != (NetworkGetHostname() + F(".local"))) {
    String redirectURL = F("http://");
    redirectURL += web_server.client().localIP().toString();
    #ifdef WEBSERVER_SETUP
    if (fromAP && !hasWiFiCredentials) {
      redirectURL += F("/setup");
    }
    #endif
    web_server.sendHeader(F("Location"), redirectURL, true);
    web_server.send(302, F("text/plain"), EMPTY_STRING);   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    web_server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}


// ********************************************************************************
// Web Interface init
// ********************************************************************************
// #include "core_version.h"


void WebServerInit()
{
  if (webserver_init) { return; }
  webserver_init = true;

  // Prepare webserver pages

  // FIXME TD-er: The added String() wrapper is needed for the latest ESP32 core lib.
  // See: https://github.com/espressif/arduino-esp32/issues/4374
  #ifdef WEBSERVER_ROOT
  web_server.on(F("/"),             handle_root);
  // Entries for several captive portal URLs.
  // Maybe not needed. Might be handled by notFound handler.
  web_server.on(F("/generate_204"), handle_root);  //Android captive portal.
  web_server.on(F("/fwlink"),       handle_root);  //Microsoft captive portal.
  #endif // ifdef WEBSERVER_ROOT
  #ifdef WEBSERVER_ADVANCED
  web_server.on(F("/advanced"),    handle_advanced);
  #endif // ifdef WEBSERVER_ADVANCED
  #ifdef WEBSERVER_CONFIG
  web_server.on(F("/config"),      handle_config);
  #endif // ifdef WEBSERVER_CONFIG
  #ifdef WEBSERVER_CONTROL
  web_server.on(F("/control"),     handle_control);
  #endif // ifdef WEBSERVER_CONTROL
  #ifdef WEBSERVER_CONTROLLERS
  web_server.on(F("/controllers"), handle_controllers);
  #endif // ifdef WEBSERVER_CONTROLLERS
  #ifdef WEBSERVER_DEVICES
  web_server.on(F("/devices"),     handle_devices);
  #endif // ifdef WEBSERVER_DEVICES
  #ifdef WEBSERVER_DOWNLOAD
  web_server.on(F("/download"),    handle_download);
  #endif // ifdef WEBSERVER_DOWNLOAD

#ifdef USES_C016

  // web_server.on(F("/dumpcache"),     handle_dumpcache);  // C016 specific entrie
  web_server.on(F("/cache_json"), handle_cache_json); // C016 specific entrie
  web_server.on(F("/cache_csv"),  handle_cache_csv);  // C016 specific entrie
#endif // USES_C016

  #ifdef WEBSERVER_FACTORY_RESET
  web_server.on(F("/factoryreset"),    handle_factoryreset);
  #endif // ifdef WEBSERVER_FACTORY_RESET
  #ifdef USE_SETTINGS_ARCHIVE
  web_server.on(F("/settingsarchive"), handle_settingsarchive);
  #endif // ifdef USE_SETTINGS_ARCHIVE
  web_server.on(F("/favicon.ico"),     handle_favicon);
  #ifdef WEBSERVER_FILELIST
  web_server.on(F("/filelist"),        handle_filelist);
  #endif // ifdef WEBSERVER_FILELIST
  #ifdef WEBSERVER_HARDWARE
  web_server.on(F("/hardware"),        handle_hardware);
  #endif // ifdef WEBSERVER_HARDWARE
  #ifdef WEBSERVER_I2C_SCANNER
  web_server.on(F("/i2cscanner"),      handle_i2cscanner);
  #endif // ifdef WEBSERVER_I2C_SCANNER
  web_server.on(F("/json"),            handle_json); // Also part of WEBSERVER_NEW_UI
  web_server.on(F("/csv"),             handle_csvval);
  web_server.on(F("/log"),             handle_log);
  web_server.on(F("/login"),           handle_login);
  web_server.on(F("/logjson"),         handle_log_JSON); // Also part of WEBSERVER_NEW_UI
#ifdef USES_NOTIFIER
  web_server.on(F("/notifications"),   handle_notifications);
#endif // ifdef USES_NOTIFIER
  #ifdef WEBSERVER_PINSTATES
  web_server.on(F("/pinstates"),       handle_pinstates);
  #endif // ifdef WEBSERVER_PINSTATES
  #ifdef WEBSERVER_RULES
  web_server.on(F("/rules"),           handle_rules_new);
  web_server.on(F("/rules/"),          Goto_Rules_Root);
  # ifdef WEBSERVER_NEW_RULES
  web_server.on(F("/rules/add"),       []()
  {
    handle_rules_edit(web_server.uri(), true);
  });
  web_server.on(F("/rules/backup"), handle_rules_backup);
  web_server.on(F("/rules/delete"), handle_rules_delete);
  # endif // WEBSERVER_NEW_RULES
  #endif  // WEBSERVER_RULES
#ifdef FEATURE_SD
  web_server.on(F("/SDfilelist"),  handle_SDfilelist);
#endif   // ifdef FEATURE_SD
#ifdef WEBSERVER_SETUP
  web_server.on(F("/setup"),       handle_setup);
#endif // ifdef WEBSERVER_SETUP
#ifdef WEBSERVER_SYSINFO
  web_server.on(F("/sysinfo"),     handle_sysinfo);
#endif // ifdef WEBSERVER_SYSINFO
#ifdef WEBSERVER_SYSVARS
  web_server.on(F("/sysvars"),     handle_sysvars);
#endif // WEBSERVER_SYSVARS
#ifdef WEBSERVER_TIMINGSTATS
  web_server.on(F("/timingstats"), handle_timingstats);
#endif // WEBSERVER_TIMINGSTATS
#ifdef WEBSERVER_TOOLS
  web_server.on(F("/tools"),       handle_tools);
#endif // ifdef WEBSERVER_TOOLS
#ifdef WEBSERVER_UPLOAD
  web_server.on(F("/upload"),      HTTP_GET,  handle_upload);
  web_server.on(F("/upload"),      HTTP_POST, handle_upload_post, handleFileUpload);
#endif // ifdef WEBSERVER_UPLOAD
#ifdef WEBSERVER_WIFI_SCANNER
  web_server.on(F("/wifiscanner"), handle_wifiscanner);
#endif // ifdef WEBSERVER_WIFI_SCANNER

#ifdef WEBSERVER_NEW_UI
  web_server.on(F("/buildinfo"),         handle_buildinfo); // Also part of WEBSERVER_NEW_UI
  web_server.on(F("/factoryreset_json"), handle_factoryreset_json);
  web_server.on(F("/filelist_json"),     handle_filelist_json);
  web_server.on(F("/i2cscanner_json"),   handle_i2cscanner_json);
  web_server.on(F("/node_list_json"),    handle_nodes_list_json);
  web_server.on(F("/pinstates_json"),    handle_pinstates_json);
  web_server.on(F("/sysinfo_json"),      handle_sysinfo_json);
  web_server.on(F("/timingstats_json"),  handle_timingstats_json);
  web_server.on(F("/upload_json"),       HTTP_POST, handle_upload_json, handleFileUpload);
  web_server.on(F("/wifiscanner_json"),  handle_wifiscanner_json);
#endif // WEBSERVER_NEW_UI

  web_server.onNotFound(handleNotFound);

  #if defined(ESP8266) || defined(ESP32)
  {
    # ifndef NO_HTTP_UPDATER
    uint32_t maxSketchSize;
    bool     use2step;

    if (OTA_possible(maxSketchSize, use2step)) {
      httpUpdater.setup(&web_server);
    }
    # endif // ifndef NO_HTTP_UPDATER
  }
  #endif    // if defined(ESP8266)

  #if defined(ESP8266)

  # ifdef USES_SSDP

  if (Settings.UseSSDP)
  {
    web_server.on(F("/ssdp.xml"), HTTP_GET, []() {
      WiFiClient client(web_server.client());
      client.setTimeout(CONTROLLER_CLIENTTIMEOUT_DFLT);
      SSDP_schema(client);
    });
    SSDP_begin();
  }
  # endif // USES_SSDP
  #endif  // if defined(ESP8266)
}

void setWebserverRunning(bool state) {
  if (webserverRunning == state) {
    return;
  }

  if (state) {
    WebServerInit();
    web_server.begin(Settings.WebserverPort);
    addLog(LOG_LEVEL_INFO, F("Webserver: start"));
  } else {
    web_server.stop();
    addLog(LOG_LEVEL_INFO, F("Webserver: stop"));
  }
  webserverRunning = state;
  CheckRunningServices(); // Uses webserverRunning state.
}

void getWebPageTemplateDefault(const String& tmplName, String& tmpl)
{
  const bool addJS   = true;
  const bool addMeta = true;

  if (tmplName == F("TmplAP"))
  {
    static size_t expectedSize = 200;

    tmpl.reserve(expectedSize);
    getWebPageTemplateDefaultHead(tmpl, !addMeta, !addJS);

    #ifndef WEBPAGE_TEMPLATE_AP_HEADER
    tmpl += F("<body><header class='apheader'>"
              "<h1>Welcome to ESP Easy Mega AP</h1>");
    #else
    tmpl += F(WEBPAGE_TEMPLATE_AP_HEADER);
    #endif

    tmpl += F("</header>");
    getWebPageTemplateDefaultContentSection(tmpl);
    getWebPageTemplateDefaultFooter(tmpl);
    if (tmpl.length() > expectedSize) {
      expectedSize = tmpl.length();
    }
  }
  else if (tmplName == F("TmplMsg"))
  {
    static size_t expectedSize = 200;

    tmpl.reserve(expectedSize);
    getWebPageTemplateDefaultHead(tmpl, !addMeta, !addJS);
    tmpl += F("<body>");
    getWebPageTemplateDefaultHeader(tmpl, F("{{name}}"), false);
    getWebPageTemplateDefaultContentSection(tmpl);
    getWebPageTemplateDefaultFooter(tmpl);
    if (tmpl.length() > expectedSize) {
      expectedSize = tmpl.length();
    }
  }
  else if (tmplName == F("TmplDsh"))
  {
    static size_t expectedSize = 200;

    tmpl.reserve(expectedSize);
    getWebPageTemplateDefaultHead(tmpl, !addMeta, addJS);
    tmpl += F(
      "<body>"
      "{{content}}"
      "</body></html>"
      );
    if (tmpl.length() > expectedSize) {
      expectedSize = tmpl.length();
    }
  }
  else // all other template names e.g. TmplStd
  {
    static size_t expectedSize = 200;

    tmpl.reserve(expectedSize);
    getWebPageTemplateDefaultHead(tmpl, addMeta, addJS);
    tmpl += F("<body class='bodymenu'>"
              "<span class='message' id='rbtmsg'></span>");
    getWebPageTemplateDefaultHeader(tmpl, F("{{name}} {{logo}}"), true);
    getWebPageTemplateDefaultContentSection(tmpl);
    getWebPageTemplateDefaultFooter(tmpl);
    if (tmpl.length() > expectedSize) {
      expectedSize = tmpl.length();
    }
  }
//  addLog(LOG_LEVEL_INFO, String(F("tmpl.length(): ")) + String(tmpl.length()));
}

void getWebPageTemplateDefaultHead(String& tmpl, bool addMeta, bool addJS) {
  tmpl += F("<!DOCTYPE html><html lang='en'>"
            "<head>"
            "<meta charset='utf-8'/>"
            "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
            "<title>{{name}}</title>");

  if (addMeta) { tmpl += F("{{meta}}"); }

  if (addJS) { tmpl += F("{{js}}"); }

  tmpl += F("{{css}}"
            "</head>");
}

void getWebPageTemplateDefaultHeader(String& tmpl, const String& title, bool addMenu) {
  {
    String tmp;
  #ifndef WEBPAGE_TEMPLATE_DEFAULT_HEADER
    tmp = F("<header class='headermenu'><h1>ESP Easy Mega: {{title}}</h1><BR>");
  #else // ifndef WEBPAGE_TEMPLATE_DEFAULT_HEADER
    tmp = F(WEBPAGE_TEMPLATE_DEFAULT_HEADER);
  #endif // ifndef WEBPAGE_TEMPLATE_DEFAULT_HEADER

    tmp.replace(F("{{title}}"), title);
    tmpl += tmp;
  }

  if (addMenu) { tmpl += F("{{menu}}"); }
  tmpl += F("</header>");
}

void getWebPageTemplateDefaultContentSection(String& tmpl) {
  tmpl += F("<section>"
            "<span class='message error'>"
            "{{error}}"
            "</span>"
            "{{content}}"
            "</section>"
            );
}

void getWebPageTemplateDefaultFooter(String& tmpl) {
  #ifndef WEBPAGE_TEMPLATE_DEFAULT_FOOTER
  tmpl += F("<footer>"
            "<br>"
            "<h6>Powered by <a href='http://www.letscontrolit.com' style='font-size: 15px; text-decoration: none'>Let's Control It</a> community</h6>"
            "</footer>"
            "</body></html>"
            );
#else // ifndef WEBPAGE_TEMPLATE_DEFAULT_FOOTER
  tmpl += F(WEBPAGE_TEMPLATE_DEFAULT_FOOTER);
#endif // ifndef WEBPAGE_TEMPLATE_DEFAULT_FOOTER
}

void getErrorNotifications() {
  // Check number of MQTT controllers active.
  int nrMQTTenabled = 0;

  for (controllerIndex_t x = 0; x < CONTROLLER_MAX; x++) {
    if (Settings.Protocol[x] != 0) {
      protocolIndex_t ProtocolIndex = getProtocolIndex_from_ControllerIndex(x);

      if (validProtocolIndex(ProtocolIndex) && Settings.ControllerEnabled[x] && Protocol[ProtocolIndex].usesMQTT) {
        ++nrMQTTenabled;
      }
    }
  }

  if (nrMQTTenabled > 1) {
    // Add warning, only one MQTT protocol should be used.
    addHtmlError(F("Only one MQTT controller should be active."));
  }

  // Check checksum of stored settings.
}

uint8_t navMenuIndex = MENU_INDEX_MAIN;

// See https://github.com/letscontrolit/ESPEasy/issues/1650
const __FlashStringHelper * getGpMenuIcon(uint8_t index) {
  switch (index) {
    case MENU_INDEX_MAIN: return F("&#8962;");
    case MENU_INDEX_CONFIG: return F("&#9881;");
    case MENU_INDEX_CONTROLLERS: return F("&#128172;");
    case MENU_INDEX_HARDWARE: return F("&#128204;");
    case MENU_INDEX_DEVICES: return F("&#128268;");
    case MENU_INDEX_RULES: return F("&#10740;");
    case MENU_INDEX_NOTIFICATIONS: return F("&#9993;");
    case MENU_INDEX_TOOLS: return F("&#128295;");
  }
  return F("");
}

const __FlashStringHelper * getGpMenuLabel(uint8_t index) {
  switch (index) {
    case MENU_INDEX_MAIN: return F("Main");
    case MENU_INDEX_CONFIG: return F("Config");
    case MENU_INDEX_CONTROLLERS: return F("Controllers");
    case MENU_INDEX_HARDWARE: return F("Hardware");
    case MENU_INDEX_DEVICES: return F("Devices");
    case MENU_INDEX_RULES: return F("Rules");
    case MENU_INDEX_NOTIFICATIONS: return F("Notifications");
    case MENU_INDEX_TOOLS: return F("Tools");
  }
  return F("");
}

const __FlashStringHelper * getGpMenuURL(uint8_t index) {
  switch (index) {
    case MENU_INDEX_MAIN: return F("/");
    case MENU_INDEX_CONFIG: return F("/config");
    case MENU_INDEX_CONTROLLERS: return F("/controllers");
    case MENU_INDEX_HARDWARE: return F("/hardware");
    case MENU_INDEX_DEVICES: return F("/devices");
    case MENU_INDEX_RULES: return F("/rules");
    case MENU_INDEX_NOTIFICATIONS: return F("/notifications");
    case MENU_INDEX_TOOLS: return F("/tools");
  }
  return F("");
}


bool GpMenuVisible(uint8_t index) {
  switch (index) {
    case MENU_INDEX_MAIN: return MENU_INDEX_MAIN_VISIBLE;
    case MENU_INDEX_CONFIG: return MENU_INDEX_CONFIG_VISIBLE;
    case MENU_INDEX_CONTROLLERS: return MENU_INDEX_CONTROLLERS_VISIBLE;
    case MENU_INDEX_HARDWARE: return MENU_INDEX_HARDWARE_VISIBLE;
    case MENU_INDEX_DEVICES: return MENU_INDEX_DEVICES_VISIBLE;
    case MENU_INDEX_RULES: return MENU_INDEX_RULES_VISIBLE;
    case MENU_INDEX_NOTIFICATIONS: return MENU_INDEX_NOTIFICATIONS_VISIBLE;
    case MENU_INDEX_TOOLS: return MENU_INDEX_TOOLS_VISIBLE;
  }
  return false;
}

void getWebPageTemplateVar(const String& varName)
{
  // serialPrint(varName); serialPrint(" : free: "); serialPrint(ESP.getFreeHeap());   serialPrint("var len before:  "); serialPrint
  // (varValue.length()) ;serialPrint("after:  ");
  // varValue = "";

  if (varName == F("name"))
  {
    addHtml(Settings.Name);
  }

  else if (varName == F("unit"))
  {
    addHtmlInt(Settings.Unit);
  }

  else if (varName == F("menu"))
  {
    addHtml(F("<div class='menubar'>"));

    for (uint8_t i = 0; i < 8; i++)
    {
      if (!GpMenuVisible(i)) {
        // hide menu item
        continue;
      }
      if ((i == MENU_INDEX_RULES) && !Settings.UseRules) { // hide rules menu item
        continue;
      }
#ifndef USES_NOTIFIER

      if (i == MENU_INDEX_NOTIFICATIONS) { // hide notifications menu item
        continue;
      }
#endif // ifndef USES_NOTIFIER

      addHtml(F("<a "));

      addHtmlAttribute(F("class"), (i == navMenuIndex) ? F("menu active") : F("menu"));
      addHtmlAttribute(F("href"),  getGpMenuURL(i));
      addHtml('>');
      addHtml(getGpMenuIcon(i));
      addHtml(F("<span class='showmenulabel'>"));
      addHtml(getGpMenuLabel(i));
      addHtml(F("</span></a>"));
    }

    addHtml(F("</div>"));
  }

  else if (varName == F("logo"))
  {
    if (fileExists(F("esp.png")))
    {
      addHtml(F("<img src=\"esp.png\" width=48 height=48 align=right>"));
    }
  }

  else if (varName == F("css"))
  {
    serve_favicon();
    serve_CSS();
  }


  else if (varName == F("js"))
  {
    html_add_autosubmit_form();
    serve_JS(JSfiles_e::Toasting);
  }

  else if (varName == F("error"))
  {
    // print last error - not implemented yet
  }

  else if (varName == F("debug"))
  {
    // print debug messages - not implemented yet
  }

  else
  {
    #ifndef BUILD_NO_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("Templ: Unknown Var : ");
      log += varName;
      addLog(LOG_LEVEL_ERROR, log);
    }
    #endif // ifndef BUILD_NO_DEBUG

    // no return string - eat var name
  }
}

void writeDefaultCSS(void)
{
  return; // TODO

/*
#ifndef WEBSERVER_USE_CDN_JS_CSS

  if (!fileExists(F("esp.css")))
  {
    fs::File f = tryOpenFile(F("esp.css"), "w");

    if (f)
    {
      String defaultCSS;
      defaultCSS = PGMT(DATA_ESPEASY_DEFAULT_MIN_CSS);

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log = F("CSS  : Writing default CSS file to FS (");
        log += defaultCSS.length();
        log += F(" bytes)");
        addLog(LOG_LEVEL_INFO, log);
      }
      f.write((const unsigned char *)defaultCSS.c_str(), defaultCSS.length()); // note: content must be in RAM - a write of F("XXX") does
                                                                               // not work
      f.close();
    }
  }
#endif
*/
}

// ********************************************************************************
// Functions to stream JSON directly to TXBuffer
// FIXME TD-er: replace stream_xxx_json_object* into this code.
// N.B. handling of numerical values differs (string vs. no string)
// ********************************************************************************

int8_t level     = 0;
int8_t lastLevel = -1;

void json_quote_name(const __FlashStringHelper * val) {
  json_quote_name(String(val));
}

void json_quote_name(const String& val) {
  if (lastLevel == level) {
    addHtml(',');
  }

  if (val.length() > 0) {
    json_quote_val(val);
    addHtml(':');
  }
}

void json_quote_val(const String& val) {
  addHtml('\"');
  addHtml(val);
  addHtml('\"');
}

void json_open(bool arr) {
  json_open(arr, EMPTY_STRING);
}

void json_open(bool arr, const __FlashStringHelper * name) {
  json_quote_name(name);
  addHtml(arr ? '[' : '{');
  lastLevel = level;
  level++;
}

void json_open(bool arr, const String& name) {
  json_quote_name(name);
  addHtml(arr ? '[' : '{');
  lastLevel = level;
  level++;
}

void json_init() {
  level     = 0;
  lastLevel = -1;
}

void json_close() {
  json_close(false);
}

void json_close(bool arr) {
  addHtml(arr ? ']' : '}');
  level--;
  lastLevel = level;
}

void json_number(const String& name, const String& value) {
  json_quote_name(name);
  json_quote_val(value);
  lastLevel = level;
}

void json_prop(const String& name, const String& value) {
  json_quote_name(name);
  json_quote_val(value);
  lastLevel = level;
}

void json_prop(LabelType::Enum label) {
  json_prop(getInternalLabel(label, '-'), getValue(label));
}

// ********************************************************************************
// Add a task select dropdown list
// This allows to select a task index based on the existing tasks.
// ********************************************************************************
void addTaskSelect(const String& name,  taskIndex_t choice)
{
  String deviceName;

  addHtml(F("<select "));
  addHtmlAttribute(F("id"),       F("selectwidth"));
  addHtmlAttribute(F("name"),     name);
  addHtmlAttribute(F("onchange"), F("return dept_onchange(frmselect)"));
  addHtml('>');

  for (taskIndex_t x = 0; x < TASKS_MAX; x++)
  {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(x);
    deviceName = getPluginNameFromDeviceIndex(DeviceIndex);
    LoadTaskSettings(x);

    {
      addHtml(F("<option value='"));
      addHtmlInt(x);
      addHtml('\'');

      if (choice == x) {
        addHtml(F(" selected"));
      }
    }

    if (!validPluginID_fullcheck(Settings.TaskDeviceNumber[x])) {
      addDisabled();
    }
    {
      addHtml('>');
      addHtmlInt(x + 1);
      addHtml(F(" - "));
      addHtml(deviceName);
      addHtml(F(" - "));
      addHtml(ExtraTaskSettings.TaskDeviceName);
      addHtml(F("</option>"));
    }
  }
}

// ********************************************************************************
// Add a Value select dropdown list, based on TaskIndex
// This allows to select a task value, based on the existing tasks.
// ********************************************************************************
void addTaskValueSelect(const String& name, int choice, taskIndex_t TaskIndex)
{
  if (!validTaskIndex(TaskIndex)) { return; }
  const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(TaskIndex);

  if (!validDeviceIndex(DeviceIndex)) { return; }

  addHtml(F("<select "));
  addHtmlAttribute(F("id"),   F("selectwidth"));
  addHtmlAttribute(F("name"), name);
  addHtml('>');

  LoadTaskSettings(TaskIndex);
  const uint8_t valueCount = getValueCountForTask(TaskIndex);

  for (uint8_t x = 0; x < valueCount; x++)
  {
    addHtml(F("<option value='"));
    addHtmlInt(x);
    addHtml('\'');

    if (choice == x) {
      addHtml(F(" selected"));
    }
    addHtml('>');
    addHtml(ExtraTaskSettings.TaskDeviceValueNames[x]);
    addHtml(F("</option>"));
  }
}

// ********************************************************************************
// Login state check
// ********************************************************************************
bool isLoggedIn(bool mustProvideLogin)
{
  String www_username = F(DEFAULT_ADMIN_USERNAME);

  if (!clientIPallowed()) { return false; }

  if (SecuritySettings.Password[0] == 0) { return true; }

  if (!mustProvideLogin) {
    return false;
  }
  if (!web_server.authenticate(www_username.c_str(), SecuritySettings.Password))

  // Basic Auth Method with Custom realm and Failure Response
  // return server.requestAuthentication(BASIC_AUTH, www_realm, authFailResponse);
  // Digest Auth Method with realm="Login Required" and empty Failure Response
  // return server.requestAuthentication(DIGEST_AUTH);
  // Digest Auth Method with Custom realm and empty Failure Response
  // return server.requestAuthentication(DIGEST_AUTH, www_realm);
  // Digest Auth Method with Custom realm and Failure Response
  {
#ifdef CORE_PRE_2_5_0

    // See https://github.com/esp8266/Arduino/issues/4717
    HTTPAuthMethod mode = BASIC_AUTH;
#else // ifdef CORE_PRE_2_5_0
    HTTPAuthMethod mode = DIGEST_AUTH;
#endif // ifdef CORE_PRE_2_5_0
    String message = F("Login Required (default user: ");
    message += www_username;
    message += ')';
    web_server.requestAuthentication(mode, message.c_str());
    return false;
  }
  return true;
}

String getControllerSymbol(uint8_t index)
{
  String ret = F("<p style='font-size:20px; background: #00000000;'>&#");

  ret += 10102 + index;
  ret += F(";</p>");
  return ret;
}

/*
   String getValueSymbol(uint8_t index)
   {
   String ret = F("&#");
   ret += 10112 + index;
   ret += ';';
   return ret;
   }
 */
void addSVG_param(const String& key, float value) {
  addSVG_param(key, String(value, 2));
}

void addSVG_param(const String& key, const String& value) {
  addHtml(' ');
  addHtml(key);
  addHtml('=');
  addHtml('\"');
  addHtml(value);
  addHtml('\"');
}

void createSvgRect_noStroke(const __FlashStringHelper * classname, unsigned int fillColor, float xoffset, float yoffset, float width, float height, float rx, float ry) {
  createSvgRect(classname, fillColor, fillColor, xoffset, yoffset, width, height, 0, rx, ry);
}

void createSvgRect(const String& classname,
                   unsigned int fillColor,
                   unsigned int strokeColor,
                   float        xoffset,
                   float        yoffset,
                   float        width,
                   float        height,
                   float        strokeWidth,
                   float        rx,
                   float        ry) {
  addHtml(F("<rect"));
  if (!classname.isEmpty()) {
    addSVG_param(F("class"), classname);
  }
  addSVG_param(F("fill"), formatToHex(fillColor, F("#")));

  if (!approximatelyEqual(strokeWidth, 0)) {
    addSVG_param(F("stroke"),       formatToHex(strokeColor, F("#")));
    addSVG_param(F("stroke-width"), strokeWidth);
  }
  addSVG_param("x",         xoffset);
  addSVG_param("y",         yoffset);
  addSVG_param(F("width"),  width);
  addSVG_param(F("height"), height);
  addSVG_param(F("rx"),     rx);
  addSVG_param(F("ry"),     ry);
  addHtml(F("/>"));
}

void createSvgHorRectPath(unsigned int color, int xoffset, int yoffset, int size, int height, int range, float SVG_BAR_WIDTH) {
  float width = SVG_BAR_WIDTH * size / range;

  if (width < 2) { width = 2; }
  addHtml(formatToHex(color, F("<path fill=\"#")));
  addHtml(F("\" d=\"M"));
  addHtml(toString(SVG_BAR_WIDTH * xoffset / range, 2));
  addHtml(' ');
  addHtmlInt(yoffset);
  addHtml('h');
  addHtml(toString(width, 2));
  addHtml('v');
  addHtmlInt(height);
  addHtml('H');
  addHtml(toString(SVG_BAR_WIDTH * xoffset / range, 2));
  addHtml(F("z\"/>\n"));
}

void createSvgTextElement(const String& text, float textXoffset, float textYoffset) {
  addHtml(F("<text style=\"line-height:1.25\" x=\""));
  addHtml(toString(textXoffset, 2));
  addHtml(F("\" y=\""));
  addHtml(toString(textYoffset, 2));
  addHtml(F("\" stroke-width=\".3\" font-family=\"sans-serif\" font-size=\"8\" letter-spacing=\"0\" word-spacing=\"0\">\n"));
  addHtml(F("<tspan x=\""));
  addHtml(toString(textXoffset, 2));
  addHtml(F("\" y=\""));
  addHtml(toString(textYoffset, 2));
  addHtml(F("\">"));
  addHtml(text);
  addHtml(F("</tspan>\n</text>"));
}

#define SVG_BAR_HEIGHT 16
#define SVG_BAR_WIDTH 400

void write_SVG_image_header(int width, int height) {
  write_SVG_image_header(width, height, false);
}

void write_SVG_image_header(int width, int height, bool useViewbox) {
  addHtml(F("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\""));
  addHtmlInt(width);
  addHtml(F("\" height=\""));
  addHtmlInt(height);
  addHtml(F("\" version=\"1.1\""));

  if (useViewbox) {
    addHtml(F(" viewBox=\"0 0 100 100\""));
  }
  addHtml('>');
}

/*
   void getESPeasyLogo(int width_pixels) {
   write_SVG_image_header(width_pixels, width_pixels, true);
   addHtml(F("<g transform=\"translate(-33.686 -7.8142)\"><rect x=\"49\" y=\"23.1\" width=\"69.3\" height=\"69.3\" fill=\"#2c72da\"
      stroke=\"#2c72da\"
      stroke-linecap=\"round\"stroke-linejoin=\"round\" stroke-width=\"30.7\"/><g transform=\"matrix(3.3092 0 0 3.3092 -77.788
         -248.96)\"><path d=\"m37.4 89 7.5-7.5M37.4 96.5l15-15M37.4 96.5l15-15M37.4 104l22.5-22.5M44.9 104l15-15\"
      fill=\"none\"stroke=\"#fff\" stroke-linecap=\"round\" stroke-width=\"2.6\"/><circle cx=\"58\" cy=\"102.1\" r=\"3\"
         fill=\"#fff\"/></g></g></svg>");
   }
 */
void getWiFi_RSSI_icon(int rssi, int width_pixels)
{
  const int nbars_filled = (rssi + 100) / 8;
  int nbars              = 5;
  int white_between_bar  = (static_cast<float>(width_pixels) / nbars) * 0.2;

  if (white_between_bar < 1) { white_between_bar = 1; }
  const int barWidth   = (width_pixels - (nbars - 1) * white_between_bar) / nbars;
  int svg_width_pixels = nbars * barWidth + (nbars - 1) * white_between_bar;

  write_SVG_image_header(svg_width_pixels, svg_width_pixels, true);
  float scale               = 100.0f / svg_width_pixels;
  const int bar_height_step = 100 / nbars;

  for (int i = 0; i < nbars; ++i) {
    unsigned int color = i < nbars_filled ? 0x0 : 0xa1a1a1; // Black/Grey
    int barHeight      = (i + 1) * bar_height_step;
    createSvgRect_noStroke(i < nbars_filled ? F("bar_highlight") : F("bar_dimmed"), color, i * (barWidth + white_between_bar) * scale, 100 - barHeight, barWidth, barHeight, 0, 0);
  }
  addHtml(F("</svg>\n"));
}

#ifndef BUILD_MINIMAL_OTA
void getConfig_dat_file_layout() {
  const int shiftY  = 2;
  float     yOffset = shiftY;

  write_SVG_image_header(SVG_BAR_WIDTH + 250, SVG_BAR_HEIGHT + shiftY);

  int max_index, offset, max_size;
  int struct_size = 0;

  // background
  const uint32_t realSize = SettingsType::getFileSize(SettingsType::Enum::TaskSettings_Type);

  createSvgHorRectPath(0xcdcdcd, 0, yOffset, realSize, SVG_BAR_HEIGHT - 2, realSize, SVG_BAR_WIDTH);

  for (int st = 0; st < static_cast<int>(SettingsType::Enum::SettingsType_MAX); ++st) {
    SettingsType::Enum settingsType = static_cast<SettingsType::Enum>(st);

    if (SettingsType::getSettingsFile(settingsType) == SettingsType::SettingsFileEnum::FILE_CONFIG_type) {
      unsigned int color = SettingsType::getSVGcolor(settingsType);
      SettingsType::getSettingsParameters(settingsType, 0, max_index, offset, max_size, struct_size);

      for (int i = 0; i < max_index; ++i) {
        SettingsType::getSettingsParameters(settingsType, i, offset, max_size);

        // Struct position
        createSvgHorRectPath(color, offset, yOffset, max_size, SVG_BAR_HEIGHT - 2, realSize, SVG_BAR_WIDTH);
      }
    }
  }

  // Text labels
  float textXoffset = SVG_BAR_WIDTH + 2;
  float textYoffset = yOffset + 0.9 * SVG_BAR_HEIGHT;

  createSvgTextElement(SettingsType::getSettingsFileName(SettingsType::Enum::TaskSettings_Type), textXoffset, textYoffset);
  addHtml(F("</svg>\n"));
}

void getStorageTableSVG(SettingsType::Enum settingsType) {
  uint32_t realSize   = SettingsType::getFileSize(settingsType);
  unsigned int color  = SettingsType::getSVGcolor(settingsType);
  const int    shiftY = 2;

  int max_index, offset, max_size;
  int struct_size = 0;

  SettingsType::getSettingsParameters(settingsType, 0, max_index, offset, max_size, struct_size);

  if (max_index == 0) { return; }

  // One more to add bar indicating struct size vs. reserved space.
  write_SVG_image_header(SVG_BAR_WIDTH + 250, (max_index + 1) * SVG_BAR_HEIGHT + shiftY);
  float yOffset = shiftY;

  for (int i = 0; i < max_index; ++i) {
    SettingsType::getSettingsParameters(settingsType, i, offset, max_size);

    // background
    createSvgHorRectPath(0xcdcdcd, 0,      yOffset, realSize, SVG_BAR_HEIGHT - 2, realSize, SVG_BAR_WIDTH);

    // Struct position
    createSvgHorRectPath(color,    offset, yOffset, max_size, SVG_BAR_HEIGHT - 2, realSize, SVG_BAR_WIDTH);

    // Text labels
    float textXoffset = SVG_BAR_WIDTH + 2;
    float textYoffset = yOffset + 0.9 * SVG_BAR_HEIGHT;
    createSvgTextElement(formatHumanReadable(offset, 1024),   textXoffset, textYoffset);
    textXoffset = SVG_BAR_WIDTH + 60;
    createSvgTextElement(formatHumanReadable(max_size, 1024), textXoffset, textYoffset);
    textXoffset = SVG_BAR_WIDTH + 130;
    createSvgTextElement(String(i),                           textXoffset, textYoffset);
    yOffset += SVG_BAR_HEIGHT;
  }

  // usage
  createSvgHorRectPath(0xcdcdcd, 0, yOffset, max_size, SVG_BAR_HEIGHT - 2, max_size, SVG_BAR_WIDTH);

  // Struct size (used part of the reserved space)
  if (struct_size != 0) {
    createSvgHorRectPath(color, 0, yOffset, struct_size, SVG_BAR_HEIGHT - 2, max_size, SVG_BAR_WIDTH);
  }

  // Text labels
  float textXoffset = SVG_BAR_WIDTH + 2;
  float textYoffset = yOffset + 0.9 * SVG_BAR_HEIGHT;

  if (struct_size != 0) {
    String text = formatHumanReadable(struct_size, 1024);
    text += '/';
    text += formatHumanReadable(max_size, 1024);
    text += F(" per item");
    createSvgTextElement(text, textXoffset, textYoffset);
  } else {
    createSvgTextElement(F("Variable size"), textXoffset, textYoffset);
  }
  addHtml(F("</svg>\n"));
}

#endif // ifndef BUILD_MINIMAL_OTA

#ifdef ESP32

# include <esp_partition.h>

int getPartionCount(uint8_t pType) {
  esp_partition_type_t partitionType       = static_cast<esp_partition_type_t>(pType);
  esp_partition_iterator_t _mypartiterator = esp_partition_find(partitionType, ESP_PARTITION_SUBTYPE_ANY, NULL);
  int nrPartitions                         = 0;

  if (_mypartiterator) {
    do {
      ++nrPartitions;
    } while ((_mypartiterator = esp_partition_next(_mypartiterator)) != NULL);
  }
  esp_partition_iterator_release(_mypartiterator);
  return nrPartitions;
}

void getPartitionTableSVG(uint8_t pType, unsigned int partitionColor) {
  int nrPartitions = getPartionCount(pType);

  if (nrPartitions == 0) { return; }
  const int shiftY = 2;

  uint32_t realSize                      = getFlashRealSizeInBytes();
  esp_partition_type_t     partitionType = static_cast<esp_partition_type_t>(pType);
  const esp_partition_t   *_mypart;
  esp_partition_iterator_t _mypartiterator = esp_partition_find(partitionType, ESP_PARTITION_SUBTYPE_ANY, NULL);

  write_SVG_image_header(SVG_BAR_WIDTH + 250, nrPartitions * SVG_BAR_HEIGHT + shiftY);
  float yOffset = shiftY;

  if (_mypartiterator) {
    do {
      _mypart = esp_partition_get(_mypartiterator);
      createSvgHorRectPath(0xcdcdcd,       0,                yOffset, realSize,      SVG_BAR_HEIGHT - 2, realSize, SVG_BAR_WIDTH);
      createSvgHorRectPath(partitionColor, _mypart->address, yOffset, _mypart->size, SVG_BAR_HEIGHT - 2, realSize, SVG_BAR_WIDTH);
      float textXoffset = SVG_BAR_WIDTH + 2;
      float textYoffset = yOffset + 0.9 * SVG_BAR_HEIGHT;
      createSvgTextElement(formatHumanReadable(_mypart->size, 1024),          textXoffset, textYoffset);
      textXoffset = SVG_BAR_WIDTH + 60;
      createSvgTextElement(_mypart->label,                                    textXoffset, textYoffset);
      textXoffset = SVG_BAR_WIDTH + 130;
      createSvgTextElement(getPartitionType(_mypart->type, _mypart->subtype), textXoffset, textYoffset);
      yOffset += SVG_BAR_HEIGHT;
    } while ((_mypartiterator = esp_partition_next(_mypartiterator)) != NULL);
  }
  addHtml(F("</svg>\n"));
  esp_partition_iterator_release(_mypartiterator);
}

#endif // ifdef ESP32

bool webArg2ip(const String& arg, uint8_t *IP) {
  return str2ip(webArg(arg), IP);
}

#ifdef ESP8266
const String& webArg(const __FlashStringHelper * arg)
{
  return web_server.arg(String(arg));
}

const String& webArg(const String& arg)
{
  return web_server.arg(arg);
}

const String& webArg(int i)
{
  return web_server.arg(i);
}
#endif

#ifdef ESP32
String webArg(const __FlashStringHelper * arg)
{
  return web_server.arg(String(arg));
}

String webArg(const String& arg)
{
  return web_server.arg(arg);
}

String webArg(int i)
{
  return web_server.arg(i);
}
#endif