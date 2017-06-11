//********************************************************************************
// Web Interface init
//********************************************************************************
void WebServerInit()
{
  // Prepare webserver pages
  WebServer.on("/", handle_root);
  WebServer.on("/config", handle_config);
  WebServer.on("/controllers", handle_controllers);
  WebServer.on("/hardware", handle_hardware);
  WebServer.on("/devices", handle_devices);
  WebServer.on("/notifications", handle_notifications);
  WebServer.on("/log", handle_log);
  WebServer.on("/tools", handle_tools);
  WebServer.on("/i2cscanner", handle_i2cscanner);
  WebServer.on("/wifiscanner", handle_wifiscanner);
  WebServer.on("/login", handle_login);
  WebServer.on("/control", handle_control);
  WebServer.on("/download", handle_download);
  WebServer.on("/upload", HTTP_GET, handle_upload);
  WebServer.on("/upload", HTTP_POST, handle_upload_post, handleFileUpload);
  WebServer.onNotFound(handleNotFound);
  WebServer.on("/filelist", handle_filelist);
  WebServer.on("/SDfilelist", handle_SDfilelist);
  WebServer.on("/advanced", handle_advanced);
  WebServer.on("/setup", handle_setup);
  WebServer.on("/json", handle_json);
  WebServer.on("/rules", handle_rules);
  WebServer.on("/sysinfo", handle_sysinfo);

  if (ESP.getFlashChipRealSize() > 524288)
    httpUpdater.setup(&WebServer);

  if (Settings.UseSSDP)
  {
    WebServer.on("/ssdp.xml", HTTP_GET, []() {
      SSDP_schema(WebServer.client());
    });
    SSDP_begin();
  }

  WebServer.begin();
}

void sendWebPage(const String& tmplName, String& pageContent)
{
  String pageResult;
  String pageTemplate;

  String fileName = tmplName;
  fileName += F(".htm");

  fs::File f = SPIFFS.open(fileName, "r+");
  if (f)
  {
    while (f.available())
      pageTemplate += (char)f.read();
    f.close();
  }
  else
  {
    getWebPageTemplateDefault(tmplName, pageTemplate);
  }

  processWebPageTemplate(pageTemplate, pageResult, pageContent);

  //TEST addLog(LOG_LEVEL_INFO, pageResult);

  pageTemplate = F("");
  pageContent = F("");

  WebServer.send(200, "text/html", pageResult);
}


void getWebPageTemplateDefault(const String& tmplName, String& tmpl)
{
  if (tmplName == F("TmplAP"))
  {
    tmpl += F(
      "<!DOCTYPE html>"
      "<head>"
        "<title>{{name}}</title>"
        "{{css}}"
      "</head>"
      "<body>"
        "<h1>Welcome to ESP Easy Mega AP</h1>"
        "{{error}}"
        "{{content}}"
        "<BR><h6>Powered by www.letscontrolit.com</h6>"
      "</body>"
    );
  }
  else if (tmplName == F("TmplMsg"))
  {
    tmpl += F(
      "<!DOCTYPE html>"
      "<head>"
        "<title>{{name}}</title>"
        "{{css}}"
      "</head>"
      "<body>"
        "<h1>ESP Easy Mega: {{name}}</h1>"
        "{{error}}"
        "{{content}}"
        "<BR><h6>Powered by www.letscontrolit.com</h6>"
      "</body>"
    );
  }
  else   //all other template names e.g. TmplStd
  {
    tmpl += F(
      "<!DOCTYPE html>"
      "<head>"
        "<title>{{name}}</title>"
        "{{js}}"
        "{{css}}"
      "</head>"
      "<body>"
        "<h1>ESP Easy Mega: {{name}} {{logo}}</h1>"
        "<br/>{{menu}}<br/>"
        "{{error}}"
        "{{content}}"
        "<BR><h6>Powered by www.letscontrolit.com</h6>"
      "</body>"
    );
  }
}


void processWebPageTemplate(String& pageTemplate, String& pageResult, String& pageContent)
{
  int indexStart, indexEnd;
  String varName, varValue;

  String log = F("HTML : [processWebPageTemplate] Template-Size=");
  log += pageTemplate.length();
  log += F(" Content-Size=");
  log += pageContent.length();

  while ((indexStart = pageTemplate.indexOf("{{")) >= 0)
  {
    pageResult += pageTemplate.substring(0, indexStart);
    pageTemplate = pageTemplate.substring(indexStart);

    if ((indexEnd = pageTemplate.indexOf("}}")) > 0)
    {
      varName = pageTemplate.substring(2, indexEnd);
      pageTemplate = pageTemplate.substring(indexEnd+2);

      varName.toLowerCase();

      if (varName == F("content"))
      {
        pageResult += pageContent;
        pageContent = F("");   //free mem - content can only added once
      }
      else
      {
        getWebPageTemplateVar(varName, varValue);
/*TEST
        String log = F("> VarName: ");
        log += varName;
        log += F(" ");
        log += indexStart;
        log += F("+");
        log += indexEnd;
        log += F(" : ");
        log += varValue;
        addLog(LOG_LEVEL_DEBUG, log);
*/
        pageResult += varValue;
      }
    }
    else   //no closing "}}"
      pageTemplate = pageTemplate.substring(2);   //eat "{{"
  }
  pageResult += pageTemplate;
  pageTemplate = F("");   //free mem

  log += F(" Result-Size=");
  log += pageResult.length();
  addLog(LOG_LEVEL_DEBUG, log);
}


static byte navMenuIndex = 0;

void getWebPageTemplateVar(const String& varName, String& varValue)
{
  varValue = F("");

  if (varName == F("name"))
  {
    varValue = Settings.Name;
  }

  else if (varName == F("unit"))
  {
    varValue = Settings.Unit;
  }

  else if (varName == F("menu"))
  {
    static const __FlashStringHelper* gpMenu[8][2] = {
      F("Main"), F("."),                      //0
      F("Config"), F("config"),               //1
      F("Controllers"), F("controllers"),     //2
      F("Hardware"), F("hardware"),           //3
      F("Devices"), F("devices"),             //4
      F("Rules"), F("rules"),                 //5
      F("Notifications"), F("notifications"), //6
      F("Tools"), F("tools"),                 //7
    };

    varValue += F("<div class='menubar'>");

    for (byte i=0; i<8; i++)
    {
      if (i == 5 && !Settings.UseRules)   //hide rules menu item
        continue;

      varValue += F("<a class='menu");
      if (i == navMenuIndex)
        varValue += F(" active");
      varValue += F("' href='");
      varValue += gpMenu[i][1];
      varValue += F("'>");
      varValue += gpMenu[i][0];
      varValue += F("</a>");
    }

    varValue += F("</div>");
  }

  else if (varName == F("logo"))
  {
    if (SPIFFS.exists("esp.png"))
    {
      varValue = F("<img src=\"esp.png\" width=48 height=48 align=right>");
    }
  }

  else if (varName == F("css"))
  {
    if (SPIFFS.exists("esp.css"))
    {
      varValue = F("<link rel=\"stylesheet\" type=\"text/css\" href=\"esp.css\">");
    }
    else
    {
      varValue = F(
        "<style>"
          "* {font-family:sans-serif; font-size:12pt;}"
          "h1 {font-size:16pt; color:black; margin:8px 0 0 0; font-weight:bold;}"
          "h2 {font-size:12pt; margin:8px -4px 0 -4px; padding:6px; background-color:black; color:#FFF; font-weight:bold;}"
          "h3 {font-size:12pt; margin:16px -4px 0 -4px; padding:4px; background-color:#EEE; color:#444; font-weight:bold;}"
          "h6 {font-size:10pt; color:black; text-align:center;}"
          ".menu {background-color:#FFF; color:blue; margin:8px; text-decoration:none}"
          ".button {padding:4px 16px; background-color:#07D; color:#FFF; border:solid 1px #FFF; text-decoration:none}"
          ".button.link {}"
          ".button.help {padding:2px 4px; border-radius:50%}"
          ".menu:hover {background:#DDF;}"
          ".button:hover {background:#369;}"
          "th {padding:6px; background-color:black; color:#FFF; font-weight:bold;}"
          "td {padding:4px;}"
          "tr {padding:4px;}"
          "table {color:black;}"
          ".div_l {float:left;}"
          ".div_r {float:right; margin:2px; padding:1px 10px; border-radius:7px; background-color:#080; color:#FFF;}"
          ".div_br {clear:both;}"
          ".note {color:#444; font-style:italic}"
          ".active {text-decoration:underline;}"
          ".on {color:green;}"
          ".off {color:red;}"
        "</style>"
        );
    }
  }

  else if (varName == F("js"))
  {
    varValue += F(
      "<script><!--\n"
      "function dept_onchange(frmselect) {frmselect.submit();}"
      "\n//--></script>");
  }

  else if (varName == F("error"))
  {
    //print last error - not implemented yet
  }

  else if (varName == F("debug"))
  {
    //print debug messages - not implemented yet
  }

  else
  {
    String log = F("Templ: Unknown Var : ");
    log += varName;
    addLog(LOG_LEVEL_ERROR, log);
    //no return string - eat var name
  }
}

//********************************************************************************
// Add top menu
//********************************************************************************
void addHeader(boolean showMenu, String& str)
{
  //not longer used - now part of template
}


//********************************************************************************
// Add footer to web page
//********************************************************************************
void addFooter(String& str)
{
  //not longer used - now part of template
}


//********************************************************************************
// Web Interface root page
//********************************************************************************
void handle_root() {

  // if Wifi setup, launch setup wizard
  if (wifiSetup)
  {
    WebServer.send(200, "text/html", F("<meta HTTP-EQUIV='REFRESH' content='0; url=/setup'>"));
    return;
  }

  if (!isLoggedIn()) return;

  int freeMem = ESP.getFreeHeap();
  String sCommand = WebServer.arg(F("cmd"));

  if ((strcasecmp_P(sCommand.c_str(), PSTR("wifidisconnect")) != 0) && (strcasecmp_P(sCommand.c_str(), PSTR("reboot")) != 0))
  {
    if (timerAPoff)
      timerAPoff = millis() + 2000L;  //user has reached the main page - AP can be switched off in 2..3 sec

    String reply = "";
    navMenuIndex = 0;
    addHeader(true, reply);

    printToWeb = true;
    printWebString = "";
    if (sCommand.length() > 0)
      ExecuteCommand(VALUE_SOURCE_HTTP, sCommand.c_str());

    IPAddress ip = WiFi.localIP();
    IPAddress gw = WiFi.gatewayIP();

    reply += printWebString;
    reply += F("<form>");
    reply += F("<table><TR><TH>System Info<TH>Value<TH><TH>System Info<TH>Value<TH>");

    reply += F("<TR><TD>Unit:<TD>");
    reply += Settings.Unit;

    reply += F("<TD><TD>GIT version:<TD>");
    reply += BUILD_GIT;

    reply += F("<TR><TD>Local Time:<TD>");
    if (Settings.UseNTP)
    {
    	reply += getDateTimeString('-', ':', ' ');
    }
    else
      reply += F("NTP disabled");

    reply += F("<TD><TD>Uptime:<TD>");
    char strUpTime[40];
    int minutes = wdcounter / 2;
    int days = minutes / 1440;
    minutes = minutes % 1440;
    int hrs = minutes / 60;
    minutes = minutes % 60;
    sprintf_P(strUpTime, PSTR("%d days %d hours %d minutes"), days, hrs, minutes);
    reply += strUpTime;

    reply += F("<TR><TD>Load:<TD>");
    if (wdcounter > 0)
    {
      reply += 100 - (100 * loopCounterLast / loopCounterMax);
      reply += F("% (LC=");
      reply += int(loopCounterLast / 30);
      reply += F(")");
    }

    reply += F("<TD><TD>Free Mem:<TD>");
    reply += freeMem;
    reply += F(" (");
    reply += lowestRAM;
    reply += F(")");

    char str[20];
    sprintf_P(str, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
    reply += F("<TR><TD>IP:<TD>");
    reply += str;

    reply += F("<TD><TD>Wifi RSSI:<TD>");
    if (WiFi.status() == WL_CONNECTED)
    {
      reply += WiFi.RSSI();
      reply += F(" dB");
    }

    reply += F("<TR><TH>Node List:<TH>Name<TH>Build<TH>Type<TH>IP<TH>Age<TR><TD><TD>");
    for (byte x = 0; x < UNIT_MAX; x++)
    {
      if (Nodes[x].ip[0] != 0)
      {
        char url[80];
        sprintf_P(url, PSTR("<a class='button link' href='http://%u.%u.%u.%u'>%u.%u.%u.%u</a>"), Nodes[x].ip[0], Nodes[x].ip[1], Nodes[x].ip[2], Nodes[x].ip[3], Nodes[x].ip[0], Nodes[x].ip[1], Nodes[x].ip[2], Nodes[x].ip[3]);
        reply += F("<TR><TD>Unit ");
        reply += x;
        reply += F("<TD>");
        if (x != Settings.Unit)
          reply += Nodes[x].nodeName;
        else
          reply += Settings.Name;
        reply += F("<TD>");
        if (Nodes[x].build)
          reply += Nodes[x].build;
        reply += F("<TD>");
        if (Nodes[x].nodeType)
          switch (Nodes[x].nodeType)
          {
            case NODE_TYPE_ID_ESP_EASY_STD:
              reply += F("ESP Easy");
              break;
            case NODE_TYPE_ID_ESP_EASYM_STD:
              reply += F("ESP Easy Mega");
              break;
            case NODE_TYPE_ID_ESP_EASY32_STD:
              reply += F("ESP Easy 32");
              break;
            case NODE_TYPE_ID_ARDUINO_EASY_STD:
              reply += F("Arduino Easy");
              break;
            case NODE_TYPE_ID_NANO_EASY_STD:
              reply += F("Nano Easy");
              break;
          }
        reply += F("<TD>");
        reply += url;
        reply += F("<TD>");
        reply += Nodes[x].age;
      }
    }

    reply += F("</table></form>");
    addFooter(reply);
    sendWebPage(F("TmplStd"), reply);
    printWebString = "";
    printToWeb = false;
  }
  else
  {
    //TODO: move this to handle_tools, from where it is actually called?

    // have to disconnect or reboot from within the main loop
    // because the webconnection is still active at this point
    // disconnect here could result into a crash/reboot...
    if (strcasecmp_P(sCommand.c_str(), PSTR("wifidisconnect")) == 0)
    {
      String log = F("WIFI : Disconnecting...");
      addLog(LOG_LEVEL_INFO, log);
      cmd_within_mainloop = CMD_WIFI_DISCONNECT;
    }

    if (strcasecmp_P(sCommand.c_str(), PSTR("reboot")) == 0)
    {
      String log = F("     : Rebooting...");
      addLog(LOG_LEVEL_INFO, log);
      cmd_within_mainloop = CMD_REBOOT;
    }

    WebServer.send(200, "text/html", "OK");
  }
}


//********************************************************************************
// Web Interface config page
//********************************************************************************
void handle_config() {
  if (!isLoggedIn()) return;

  if (timerAPoff)
    timerAPoff = millis() + 2000L;  //user has reached the main page - AP can be switched off in 2..3 sec

  char tmpString[64];

  navMenuIndex = 1;
  String name = WebServer.arg(F("name"));
  //String password = WebServer.arg(F("password"));
  String ssid = WebServer.arg(F("ssid"));
  //String key = WebServer.arg(F("key"));
  String ssid2 = WebServer.arg(F("ssid2"));
  //String key2 = WebServer.arg(F("key2"));
  String sensordelay = WebServer.arg(F("delay"));
  String deepsleep = WebServer.arg(F("deepsleep"));
  String deepsleeponfail = WebServer.arg(F("deepsleeponfail"));
  String espip = WebServer.arg(F("espip"));
  String espgateway = WebServer.arg(F("espgateway"));
  String espsubnet = WebServer.arg(F("espsubnet"));
  String espdns = WebServer.arg(F("espdns"));
  String unit = WebServer.arg(F("unit"));
  //String apkey = WebServer.arg(F("apkey"));

  String reply = "";
  addHeader(true, reply);

  if (ssid[0] != 0)
  {
    strncpy(Settings.Name, name.c_str(), sizeof(Settings.Name));
    //strncpy(SecuritySettings.Password, password.c_str(), sizeof(SecuritySettings.Password));
    copyFormPassword(F("password"), SecuritySettings.Password, sizeof(SecuritySettings.Password));
    strncpy(SecuritySettings.WifiSSID, ssid.c_str(), sizeof(SecuritySettings.WifiSSID));
    //strncpy(SecuritySettings.WifiKey, key.c_str(), sizeof(SecuritySettings.WifiKey));
    copyFormPassword(F("key"), SecuritySettings.WifiKey, sizeof(SecuritySettings.WifiKey));
    strncpy(SecuritySettings.WifiSSID2, ssid2.c_str(), sizeof(SecuritySettings.WifiSSID2));
    //strncpy(SecuritySettings.WifiKey2, key2.c_str(), sizeof(SecuritySettings.WifiKey2));
    copyFormPassword(F("key2"), SecuritySettings.WifiKey2, sizeof(SecuritySettings.WifiKey2));
    //strncpy(SecuritySettings.WifiAPKey, apkey.c_str(), sizeof(SecuritySettings.WifiAPKey));
    copyFormPassword(F("apkey"), SecuritySettings.WifiAPKey, sizeof(SecuritySettings.WifiAPKey));

    Settings.Delay = sensordelay.toInt();
    Settings.deepSleep = (deepsleep == "on");
    Settings.deepSleepOnFail = (deepsleeponfail == "on");
    espip.toCharArray(tmpString, 26);
    str2ip(tmpString, Settings.IP);
    espgateway.toCharArray(tmpString, 26);
    str2ip(tmpString, Settings.Gateway);
    espsubnet.toCharArray(tmpString, 26);
    str2ip(tmpString, Settings.Subnet);
    espdns.toCharArray(tmpString, 26);
    str2ip(tmpString, Settings.DNS);
    Settings.Unit = unit.toInt();
    if (!SaveSettings())
      reply += F("<span style=\"color:red\">Error saving to flash!</span>");
  }

  reply += F("<form name='frmselect' method='post'><table>");

  addFormHeader(reply, F("Main Settings"));

  Settings.Name[25] = 0;
  SecuritySettings.Password[25] = 0;
  addFormTextBox(reply, F("Unit Name"), F("name"), Settings.Name, 25);
  addFormNumericBox(reply, F("Unit Number"), F("unit"), Settings.Unit, 0, 9999);
  addFormPasswordBox(reply, F("Admin Password"), F("password"), SecuritySettings.Password, 25);

  addFormSubHeader(reply, F("Wifi Settings"));

  addFormTextBox(reply, F("SSID"), F("ssid"), SecuritySettings.WifiSSID, 31);
  addFormPasswordBox(reply, F("WPA Key"), F("key"), SecuritySettings.WifiKey, 63);
  addFormTextBox(reply, F("Fallback SSID"), F("ssid2"), SecuritySettings.WifiSSID2, 31);
  addFormPasswordBox(reply, F("Fallback WPA Key"), F("key2"), SecuritySettings.WifiKey2, 63);
  addFormSeparator(reply);
  addFormPasswordBox(reply, F("WPA AP Mode Key"), F("apkey"), SecuritySettings.WifiAPKey, 63);


  addFormSubHeader(reply, F("IP Settings"));

  addFormIPBox(reply, F("ESP IP"), F("espip"), Settings.IP);
  addFormIPBox(reply, F("ESP GW"), F("espgateway"), Settings.Gateway);
  addFormIPBox(reply, F("ESP Subnet"), F("espsubnet"), Settings.Subnet);
  addFormIPBox(reply, F("ESP DNS"), F("espdns"), Settings.DNS);
  addFormNote(reply, F("Leave empty for DHCP"));


  addFormSubHeader(reply, F("Sleep Mode"));

  addFormCheckBox(reply, F("Sleep enabled"), F("deepsleep"), Settings.deepSleep);

  addHelpButton(reply, F("SleepMode"));
  addFormNumericBox(reply, F("Sleep Delay"), F("delay"), Settings.Delay, 0, 4294);   //limited by hardware to ~1.2h
  addUnit(reply, F("sec"));

  addFormCheckBox(reply, F("Sleep on connection failure"), F("deepsleeponfail"), Settings.deepSleepOnFail);

  addFormSeparator(reply);

  reply += F("<TR><TD><TD>");
  addSubmitButton(reply);
  reply += F("</table></form>");
  addFooter(reply);
  sendWebPage(F("TmplStd"), reply);
}


//********************************************************************************
// Web Interface controller page
//********************************************************************************
void handle_controllers() {
  if (!isLoggedIn()) return;

  struct EventStruct TempEvent;
  char tmpString[64];

  navMenuIndex = 2;
  String controllerindex = WebServer.arg(F("index"));
  String usedns = WebServer.arg(F("usedns"));
  String controllerip = WebServer.arg(F("controllerip"));
  String controllerhostname = WebServer.arg(F("controllerhostname"));
  String controllerport = WebServer.arg(F("controllerport"));
  String protocol = WebServer.arg(F("protocol"));
  String controlleruser = WebServer.arg(F("controlleruser"));
  String controllerpassword = WebServer.arg(F("controllerpassword"));
  String controllersubscribe = WebServer.arg(F("controllersubscribe"));
  String controllerpublish = WebServer.arg(F("controllerpublish"));
  String controllerenabled = WebServer.arg(F("controllerenabled"));

  String reply = "";
  addHeader(true, reply);

  byte index = controllerindex.toInt();

  if (protocol.length() != 0)
  {
    ControllerSettingsStruct ControllerSettings;
    if (Settings.Protocol[index - 1] != protocol.toInt())
    {
      Settings.Protocol[index - 1] = protocol.toInt();
      byte ProtocolIndex = getProtocolIndex(Settings.Protocol[index - 1]);
      ControllerSettings.Port = Protocol[ProtocolIndex].defaultPort;
      if (Protocol[ProtocolIndex].usesTemplate)
        CPlugin_ptr[ProtocolIndex](CPLUGIN_PROTOCOL_TEMPLATE, &TempEvent, dummyString);
      strncpy(ControllerSettings.Subscribe, TempEvent.String1.c_str(), sizeof(ControllerSettings.Subscribe));
      strncpy(ControllerSettings.Publish, TempEvent.String2.c_str(), sizeof(ControllerSettings.Publish));
      TempEvent.String1 = "";
      TempEvent.String2 = "";
    }
    else
    {
      if (Settings.Protocol != 0)
      {
        byte ProtocolIndex = getProtocolIndex(Settings.Protocol[index - 1]);
        TempEvent.ControllerIndex = index - 1;
        TempEvent.ProtocolIndex = ProtocolIndex;
        CPlugin_ptr[ProtocolIndex](CPLUGIN_WEBFORM_SAVE, &TempEvent, dummyString);
        ControllerSettings.UseDNS = usedns.toInt();
        if (ControllerSettings.UseDNS)
        {
          strncpy(ControllerSettings.HostName, controllerhostname.c_str(), sizeof(ControllerSettings.HostName));
          IPAddress IP;
          WiFi.hostByName(ControllerSettings.HostName, IP);
          for (byte x = 0; x < 4; x++)
            ControllerSettings.IP[x] = IP[x];
        }
        else
        {
          if (controllerip.length() != 0)
          {
            controllerip.toCharArray(tmpString, 26);
            str2ip(tmpString, ControllerSettings.IP);
          }
        }

        Settings.ControllerEnabled[index - 1] = (controllerenabled == "on");
        ControllerSettings.Port = controllerport.toInt();
        strncpy(SecuritySettings.ControllerUser[index - 1], controlleruser.c_str(), sizeof(SecuritySettings.ControllerUser[0]));
        //strncpy(SecuritySettings.ControllerPassword[index - 1], controllerpassword.c_str(), sizeof(SecuritySettings.ControllerPassword[0]));
        copyFormPassword(F("controllerpassword"), SecuritySettings.ControllerPassword[index - 1], sizeof(SecuritySettings.ControllerPassword[0]));
        strncpy(ControllerSettings.Subscribe, controllersubscribe.c_str(), sizeof(ControllerSettings.Subscribe));
        strncpy(ControllerSettings.Publish, controllerpublish.c_str(), sizeof(ControllerSettings.Publish));
      }
    }
    SaveControllerSettings(index - 1, (byte*)&ControllerSettings, sizeof(ControllerSettings));
    if (!SaveSettings())
      reply += F("<span style=\"color:red\">Error saving to flash!</span>");
  }

  reply += F("<form name='frmselect' method='post'>");

  if (index == 0)
  {
    reply += F("<table border=1px frame='box' rules='all'><TR><TH>");
    reply += F("<TH>Nr<TH>Enabled<TH>Protocol<TH>IP<TH>Port");

    ControllerSettingsStruct ControllerSettings;
    for (byte x = 0; x < CONTROLLER_MAX; x++)
    {
      LoadControllerSettings(x, (byte*)&ControllerSettings, sizeof(ControllerSettings));
      reply += F("<TR><TD>");
      reply += F("<a class='button link' href=\"controllers?index=");
      reply += x + 1;
      reply += F("\">Edit</a>");
      reply += F("<TD>");
      reply += getControllerSymbol(x);
      reply += F("<TD>");
      if (Settings.Protocol[x] != 0)
      {
        addEnabled(reply, Settings.ControllerEnabled[x]);

        reply += F("<TD>");
        byte ProtocolIndex = getProtocolIndex(Settings.Protocol[x]);
        String ProtocolName = "";
        CPlugin_ptr[ProtocolIndex](CPLUGIN_GET_DEVICENAME, 0, ProtocolName);
        reply += ProtocolName;

        char str[20];
        reply += F("<TD>");
        sprintf_P(str, PSTR("%u.%u.%u.%u"), ControllerSettings.IP[0], ControllerSettings.IP[1], ControllerSettings.IP[2], ControllerSettings.IP[3]);
        reply += str;
        reply += F("<TD>");
        reply += ControllerSettings.Port;
      }
      else
        reply += F("<TD><TD><TD>");
    }
    reply += F("</table></form>");
  }
  else
  {
    reply += F("<table><TR><TH>Controller Settings<TH>");
    reply += F("<TR><TD>Protocol:");
    byte choice = Settings.Protocol[index - 1];
    reply += F("<TD>");
    addSelector_Head(reply, F("protocol"), true);
    addSelector_Item(reply, F("- Standalone -"), 0, false, false, F(""));
    for (byte x = 0; x <= protocolCount; x++)
    {
      String ProtocolName = "";
      CPlugin_ptr[x](CPLUGIN_GET_DEVICENAME, 0, ProtocolName);

      addSelector_Item(reply,
        ProtocolName,
        Protocol[x].Number,
        choice == Protocol[x].Number,
        !((index == 1) || !Protocol[x].usesMQTT),
        F(""));
    }
    addSelector_Foot(reply);

    addHelpButton(reply, F("EasyProtocols"));


    char str[20];

    if (Settings.Protocol[index - 1])
    {
      ControllerSettingsStruct ControllerSettings;
      LoadControllerSettings(index - 1, (byte*)&ControllerSettings, sizeof(ControllerSettings));
      byte choice = ControllerSettings.UseDNS;
      String options[2];
      options[0] = F("Use IP address");
      options[1] = F("Use Hostname");

      addFormSelector(reply, F("Locate Controller"), F("usedns"), 2, options, NULL, NULL, choice, true);

      if (ControllerSettings.UseDNS)
      {
      	addFormTextBox(reply, F("Controller Hostname"), F("controllerhostname"), ControllerSettings.HostName, 64);
      }
      else
      {
      	addFormIPBox(reply, F("Controller IP"), F("controllerip"), ControllerSettings.IP);
      }

      addFormNumericBox(reply, F("Controller Port"), F("controllerport"), ControllerSettings.Port, 1, 65535);

      byte ProtocolIndex = getProtocolIndex(Settings.Protocol[index - 1]);
      if (Protocol[ProtocolIndex].usesAccount)
      {
      	addFormTextBox(reply, F("Controller User"), F("controlleruser"), SecuritySettings.ControllerUser[index - 1], 26);
      }

      if (Protocol[ProtocolIndex].usesPassword)
      {
      	addFormPasswordBox(reply, F("Controller Password"), F("controllerpassword"), SecuritySettings.ControllerPassword[index - 1], 64);
      }

      if (Protocol[ProtocolIndex].usesTemplate || Protocol[ProtocolIndex].usesMQTT)
      {
      	addFormTextBox(reply, F("Controller Subscribe"), F("controllersubscribe"), ControllerSettings.Subscribe, 64);
      }

      if (Protocol[ProtocolIndex].usesTemplate || Protocol[ProtocolIndex].usesMQTT)
      {
      	addFormTextBox(reply, F("Controller Publish"), F("controllerpublish"), ControllerSettings.Publish, 64);
      }

      addFormCheckBox(reply, F("Enabled"), F("controllerenabled"), Settings.ControllerEnabled[index - 1]);

      TempEvent.ControllerIndex = index - 1;
      TempEvent.ProtocolIndex = ProtocolIndex;
      CPlugin_ptr[ProtocolIndex](CPLUGIN_WEBFORM_LOAD, &TempEvent, reply);

    }

    addFormSeparator(reply);

    reply += F("<TR><TD><TD><a class='button link' href=\"controllers\">Close</a>");
    addSubmitButton(reply);
    reply += F("</table></form>");
  }
  addFooter(reply);
  sendWebPage(F("TmplStd"), reply);
}


//********************************************************************************
// Web Interface notifcations page
//********************************************************************************
void handle_notifications() {
  if (!isLoggedIn()) return;

  struct EventStruct TempEvent;
  char tmpString[64];

  navMenuIndex = 6;
  String notificationindex = WebServer.arg(F("index"));
  String notification = WebServer.arg(F("notification"));
  String domain = WebServer.arg(F("domain"));
  String server = WebServer.arg(F("server"));
  String port = WebServer.arg(F("port"));
  String sender = WebServer.arg(F("sender"));
  String receiver = WebServer.arg(F("receiver"));
  String subject = WebServer.arg(F("subject"));
  String body = WebServer.arg(F("body"));
  String pin1 = WebServer.arg(F("pin1"));
  String pin2 = WebServer.arg(F("pin2"));
  String notificationenabled = WebServer.arg(F("notificationenabled"));

  String reply = "";
  addHeader(true, reply);

  byte index = notificationindex.toInt();

  if (notification.length() != 0)
  {
    NotificationSettingsStruct NotificationSettings;
    if (Settings.Notification[index - 1] != notification.toInt())
    {
      Settings.Notification[index - 1] = notification.toInt();
      NotificationSettings.Domain[0] = 0;
      NotificationSettings.Server[0] = 0;
      NotificationSettings.Port = 0;
      NotificationSettings.Sender[0] = 0;
      NotificationSettings.Receiver[0] = 0;
      NotificationSettings.Subject[0] = 0;
      NotificationSettings.Body[0] = 0;
    }
    else
    {
      if (Settings.Notification != 0)
      {
        byte NotificationProtocolIndex = getNotificationIndex(Settings.Notification[index - 1]);
        NPlugin_ptr[NotificationProtocolIndex](NPLUGIN_WEBFORM_SAVE, 0, dummyString);
        NotificationSettings.Port = port.toInt();
        NotificationSettings.Pin1 = pin1.toInt();
        NotificationSettings.Pin2 = pin2.toInt();
        Settings.NotificationEnabled[index - 1] = (notificationenabled == "on");
        strncpy(NotificationSettings.Domain, domain.c_str(), sizeof(NotificationSettings.Domain));
        strncpy(NotificationSettings.Server, server.c_str(), sizeof(NotificationSettings.Server));
        strncpy(NotificationSettings.Sender, sender.c_str(), sizeof(NotificationSettings.Sender));
        strncpy(NotificationSettings.Receiver, receiver.c_str(), sizeof(NotificationSettings.Receiver));
        strncpy(NotificationSettings.Subject, subject.c_str(), sizeof(NotificationSettings.Subject));
        strncpy(NotificationSettings.Body, body.c_str(), sizeof(NotificationSettings.Body));
      }
    }
    SaveNotificationSettings(index - 1, (byte*)&NotificationSettings, sizeof(NotificationSettings));
    if (!SaveSettings())
      reply += F("<span style=\"color:red\">Error saving to flash!</span>");
  }

  reply += F("<form name='frmselect' method='post'>");

  if (index == 0)
  {
    reply += F("<table border=1px frame='box' rules='all'><TR><TH>");
    reply += F("<TH>Nr<TH>Enabled<TH>Service<TH>Server<TH>Port");

    NotificationSettingsStruct NotificationSettings;
    for (byte x = 0; x < NOTIFICATION_MAX; x++)
    {
      LoadNotificationSettings(x, (byte*)&NotificationSettings, sizeof(NotificationSettings));
      reply += F("<TR><TD>");
      reply += F("<a class='button link' href=\"notifications?index=");
      reply += x + 1;
      reply += F("\">Edit</a>");
      reply += F("<TD>");
      reply += x + 1;
      reply += F("<TD>");
      if (Settings.Notification[x] != 0)
      {
        addEnabled(reply, Settings.NotificationEnabled[x]);

        reply += F("<TD>");
        byte NotificationProtocolIndex = getNotificationIndex(Settings.Notification[x]);
        String NotificationName = "";
        NPlugin_ptr[NotificationProtocolIndex](NPLUGIN_GET_DEVICENAME, 0, NotificationName);
        reply += NotificationName;
        reply += F("<TD>");
        reply += NotificationSettings.Server;
        reply += F("<TD>");
        reply += NotificationSettings.Port;
      }
      else
        reply += F("<TD><TD><TD>");
    }
    reply += F("</table></form>");
  }
  else
  {
    reply += F("<table><TR><TH>Notification Settings<TH>");
    reply += F("<TR><TD>Notification:");
    byte choice = Settings.Notification[index - 1];
    reply += F("<TD>");
    addSelector_Head(reply, F("notification"), true);
    addSelector_Item(reply, F("- None -"), 0, false, false, F(""));
    for (byte x = 0; x <= notificationCount; x++)
    {
      String NotificationName = "";
      NPlugin_ptr[x](NPLUGIN_GET_DEVICENAME, 0, NotificationName);
      addSelector_Item(reply,
        NotificationName,
        Notification[x].Number,
        choice == Notification[x].Number,
        false,
        F(""));
    }
    addSelector_Foot(reply);

    addHelpButton(reply, F("EasyNotifications"));


    char str[20];

    if (Settings.Notification[index - 1])
    {
      NotificationSettingsStruct NotificationSettings;
      LoadNotificationSettings(index - 1, (byte*)&NotificationSettings, sizeof(NotificationSettings));

      byte NotificationProtocolIndex = getNotificationIndex(Settings.Notification[index - 1]);

      if (Notification[NotificationProtocolIndex].usesMessaging)
      {
        reply += F("<TR><TD>Domain:<TD><input type='text' name='domain' size=64 value='");
        reply += NotificationSettings.Domain;
        reply += F("'>");

        reply += F("<TR><TD>Server:<TD><input type='text' name='server' size=64 value='");
        reply += NotificationSettings.Server;
        reply += F("'>");

        reply += F("<TR><TD>Port:<TD><input type='text' name='port' value='");
        reply += NotificationSettings.Port;
        reply += F("'>");

        reply += F("<TR><TD>Sender:<TD><input type='text' name='sender' size=64 value='");
        reply += NotificationSettings.Sender;
        reply += F("'>");

        reply += F("<TR><TD>Receiver:<TD><input type='text' name='receiver' size=64 value='");
        reply += NotificationSettings.Receiver;
        reply += F("'>");

        reply += F("<TR><TD>Subject:<TD><input type='text' name='subject' size=64 value='");
        reply += NotificationSettings.Subject;
        reply += F("'>");

        reply += F("<TR><TD>Body:<TD><textarea name='body' rows='5' cols='80' size=512 wrap='off'>");
        reply += NotificationSettings.Body;
        reply += F("</textarea>");
      }

      if (Notification[NotificationProtocolIndex].usesGPIO > 0)
      {
        reply += F("<TR><TD>1st GPIO:<TD>");
        addPinSelect(false, reply, "pin1", NotificationSettings.Pin1);
      }

      reply += F("<TR><TD>Enabled:<TD>");
      addCheckBox(reply, F("notificationenabled"), Settings.NotificationEnabled[index - 1]);

      TempEvent.NotificationIndex = index - 1;
      NPlugin_ptr[NotificationProtocolIndex](NPLUGIN_WEBFORM_LOAD, &TempEvent, reply);

    }

    addFormSeparator(reply);

    reply += F("<TR><TD><TD><a class='button link' href=\"notifications\">Close</a>");
    addSubmitButton(reply);
    reply += F("</table></form>");
  }
  addFooter(reply);
  sendWebPage(F("TmplStd"), reply);
}


//********************************************************************************
// Web Interface hardware page
//********************************************************************************
void handle_hardware() {
  if (!isLoggedIn()) return;

  navMenuIndex = 3;
  String reply = "";
  addHeader(true, reply);

  if (isFormItem(F("psda")))
  {
    Settings.Pin_status_led  = getFormItemInt(F("pled"));
    Settings.Pin_status_led_Inversed  = isFormItemChecked(F("pledi"));

    Settings.Pin_i2c_sda     = getFormItemInt(F("psda"));
    Settings.Pin_i2c_scl     = getFormItemInt(F("pscl"));

    Settings.InitSPI = isFormItemChecked(F("initspi"));      // SPI Init
    Settings.Pin_sd_cs  = getFormItemInt(F("sd"));

    Settings.PinBootStates[0]  =  getFormItemInt(F("p0"));
    Settings.PinBootStates[2]  =  getFormItemInt(F("p2"));
    Settings.PinBootStates[4]  =  getFormItemInt(F("p4"));
    Settings.PinBootStates[5]  =  getFormItemInt(F("p5"));
    Settings.PinBootStates[9]  =  getFormItemInt(F("p9"));
    Settings.PinBootStates[10] =  getFormItemInt(F("p10"));
    Settings.PinBootStates[12] =  getFormItemInt(F("p12"));
    Settings.PinBootStates[13] =  getFormItemInt(F("p13"));
    Settings.PinBootStates[14] =  getFormItemInt(F("p14"));
    Settings.PinBootStates[15] =  getFormItemInt(F("p15"));
    Settings.PinBootStates[16] =  getFormItemInt(F("p16"));

    if (!SaveSettings())
      reply += F("<span style=\"color:red\">Error saving to flash!</span>");
  }

  reply += F("<form  method='post'><table><TR><TH>Hardware Settings<TH><TR><TD>");
  addFormSubHeader(reply, F("Wifi Status LED"));

  addFormPinSelect(reply, F("Pin LED"), "pled", Settings.Pin_status_led);
  addFormCheckBox(reply, F("Inversed LED"), F("pledi"), Settings.Pin_status_led_Inversed);
  addFormNote(reply, F("Use &rsquo;GPIO-2 (D4)&rsquo; with &rsquo;Inversed&rsquo; checked for onboard LED"));

  addFormSubHeader(reply, F("I2C Interface"));

  addFormPinSelectI2C(reply, F("SDA"), F("psda"), Settings.Pin_i2c_sda);
  addFormPinSelectI2C(reply, F("SCL"), F("pscl"), Settings.Pin_i2c_scl);

  // SPI Init
  addFormSubHeader(reply, F("SPI Interface"));

  addFormCheckBox(reply, F("Init SPI"), F("initspi"), Settings.InitSPI);
  addFormNote(reply, F("Chip Select (CS) config must be done in the plugin"));
  addFormPinSelect(reply, F("SD Card CS Pin"), "sd", Settings.Pin_sd_cs);

  addFormSubHeader(reply, F("GPIO boot states"));

  addFormPinStateSelect(reply, F("Pin mode 0 (D3)"), F("p0"), Settings.PinBootStates[0]);
  addFormPinStateSelect(reply, F("Pin mode 2 (D4)"), F("p2"), Settings.PinBootStates[2]);
  addFormPinStateSelect(reply, F("Pin mode 4 (D2)"), F("p4"), Settings.PinBootStates[4]);
  addFormPinStateSelect(reply, F("Pin mode 5 (D1)"), F("p5"), Settings.PinBootStates[5]);
  addFormPinStateSelect(reply, F("Pin mode 9 (D11)"), F("p9"), Settings.PinBootStates[9]);
  addFormPinStateSelect(reply, F("Pin mode 10 (D12)"), F("p10"), Settings.PinBootStates[10]);
  addFormPinStateSelect(reply, F("Pin mode 12 (D6)"), F("p12"), Settings.PinBootStates[12]);
  addFormPinStateSelect(reply, F("Pin mode 13 (D7)"), F("p13"), Settings.PinBootStates[13]);
  addFormPinStateSelect(reply, F("Pin mode 14 (D5)"), F("p14"), Settings.PinBootStates[14]);
  addFormPinStateSelect(reply, F("Pin mode 15 (D8)"), F("p15"), Settings.PinBootStates[15]);
  addFormPinStateSelect(reply, F("Pin mode 16 (D0)"), F("p16"), Settings.PinBootStates[16]);

  addFormSeparator(reply);

  reply += F("<TR><TD><TD>");
  addSubmitButton(reply);
  addHelpButton(reply, F("ESPEasy#Hardware_page"));

  reply += F("<TR><TD></table></form>");
  addFooter(reply);
  sendWebPage(F("TmplStd"), reply);
}


//********************************************************************************
// Add a GPIO pin select dropdown list
//********************************************************************************
void addFormPinStateSelect(String& str, const String& label, const String& id, int choice)
{
  addRowLabel(str, label);
  addPinStateSelect(str, id, choice);
}

void addPinStateSelect(String& str, String name, int choice)
{
  String options[4] = { F("Default"), F("Output Low"), F("Output High"), F("Input") };
  addSelector(str, name, 4, options, NULL, NULL, choice, false);
}


//********************************************************************************
// Web Interface device page
//********************************************************************************
void handle_devices() {
  if (!isLoggedIn()) return;

  navMenuIndex = 4;
  char tmpString[41];
  struct EventStruct TempEvent;

  String taskindex = WebServer.arg(F("index"));
  String taskdevicenumber = WebServer.arg(F("TDNUM"));
  String taskdevicetimer = WebServer.arg(F("TDT"));
  String taskdeviceid[CONTROLLER_MAX];
  String taskdevicepin1 = WebServer.arg(F("taskdevicepin1"));   // "taskdevicepin*" should not be changed because it is uses by plugins and expected to be saved by this code
  String taskdevicepin2 = WebServer.arg(F("taskdevicepin2"));
  String taskdevicepin3 = WebServer.arg(F("taskdevicepin3"));
  String taskdevicepin1pullup = WebServer.arg(F("TDPPU"));
  String taskdevicepin1inversed = WebServer.arg(F("TDPI"));
  String taskdevicename = WebServer.arg(F("TDN"));
  String taskdeviceport = WebServer.arg(F("TDP"));
  String taskdeviceformula[VARS_PER_TASK];
  String taskdevicevaluename[VARS_PER_TASK];
  String taskdevicevaluedecimals[VARS_PER_TASK];
  String taskdevicesenddata[CONTROLLER_MAX];
  String taskdeviceglobalsync = WebServer.arg(F("TDGS"));
  String taskdeviceenabled = WebServer.arg(F("TDE"));

  for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
  {
    char argc[25];
    String arg = F("TDF");
    arg += varNr + 1;
    arg.toCharArray(argc, 25);
    taskdeviceformula[varNr] = WebServer.arg(argc);

    arg = F("TDVN");
    arg += varNr + 1;
    arg.toCharArray(argc, 25);
    taskdevicevaluename[varNr] = WebServer.arg(argc);

    arg = F("TDVD");
    arg += varNr + 1;
    arg.toCharArray(argc, 25);
    taskdevicevaluedecimals[varNr] = WebServer.arg(argc);
  }

  for (byte controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
  {
    char argc[25];
    String arg = F("TDID");
    arg += controllerNr + 1;
    arg.toCharArray(argc, 25);
    taskdeviceid[controllerNr] = WebServer.arg(argc);

    arg = F("TDSD");
    arg += controllerNr + 1;
    arg.toCharArray(argc, 25);
    taskdevicesenddata[controllerNr] = WebServer.arg(argc);
  }

  String edit = WebServer.arg(F("edit"));
  byte page = WebServer.arg(F("page")).toInt();
  if (page == 0)
    page = 1;
  byte setpage = WebServer.arg(F("setpage")).toInt();
  if (setpage > 0)
  {
    if (setpage <= (TASKS_MAX / 4))
      page = setpage;
    else
      page = TASKS_MAX / 4;
  }

  String reply = "";
  //reply.reserve(8192);
  addHeader(true, reply);

  byte index = taskindex.toInt();

  byte DeviceIndex = 0;

  if (edit.toInt() != 0) // when form submitted
  {
    if (Settings.TaskDeviceNumber[index - 1] != taskdevicenumber.toInt()) // change of device, clear all other values
    {
      taskClear(index - 1, false); // clear settings, but do not save
      Settings.TaskDeviceNumber[index - 1] = taskdevicenumber.toInt();
      if (taskdevicenumber.toInt() != 0) // preload valuenames
      {
        TempEvent.TaskIndex = index - 1;
        if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0) // if field set empty, reload defaults
          PluginCall(PLUGIN_GET_DEVICEVALUENAMES, &TempEvent, dummyString);
      }
    }
    else if (taskdevicenumber.toInt() != 0)
    {
      Settings.TaskDeviceNumber[index - 1] = taskdevicenumber.toInt();
      DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[index - 1]);

      if (taskdevicetimer.toInt() > 0)
        Settings.TaskDeviceTimer[index - 1] = taskdevicetimer.toInt();
      else
      {
        if (!Device[DeviceIndex].TimerOptional) // Set default delay, unless it's optional...
          Settings.TaskDeviceTimer[index - 1] = Settings.Delay;
        else
          Settings.TaskDeviceTimer[index - 1] = 0;
      }

      taskdevicename.toCharArray(tmpString, 41);
      Settings.TaskDeviceEnabled[index - 1] = (taskdeviceenabled == "on");
      strcpy(ExtraTaskSettings.TaskDeviceName, tmpString);
      Settings.TaskDevicePort[index - 1] = taskdeviceport.toInt();

      for (byte controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
      {
        Settings.TaskDeviceID[controllerNr][index - 1] = taskdeviceid[controllerNr].toInt();
        Settings.TaskDeviceSendData[controllerNr][index - 1] = (taskdevicesenddata[controllerNr] == "on");
      }

      if (taskdevicepin1.length() != 0)
        Settings.TaskDevicePin1[index - 1] = taskdevicepin1.toInt();

      if (taskdevicepin2.length() != 0)
        Settings.TaskDevicePin2[index - 1] = taskdevicepin2.toInt();

      if (taskdevicepin3.length() != 0)
        Settings.TaskDevicePin3[index - 1] = taskdevicepin3.toInt();

      if (Device[DeviceIndex].PullUpOption)
        Settings.TaskDevicePin1PullUp[index - 1] = (taskdevicepin1pullup == "on");

      if (Device[DeviceIndex].InverseLogicOption)
        Settings.TaskDevicePin1Inversed[index - 1] = (taskdevicepin1inversed == "on");

      if (Settings.GlobalSync)
      {
        if (Device[DeviceIndex].GlobalSyncOption)
          Settings.TaskDeviceGlobalSync[index - 1] = (taskdeviceglobalsync == "on");

        // Send task info if set global
        if (Settings.TaskDeviceGlobalSync[index - 1])
        {
          SendUDPTaskInfo(0, index - 1, index - 1);
        }
      }

      for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
      {
        taskdeviceformula[varNr].toCharArray(tmpString, 41);
        strcpy(ExtraTaskSettings.TaskDeviceFormula[varNr], tmpString);
        ExtraTaskSettings.TaskDeviceValueDecimals[varNr] = taskdevicevaluedecimals[varNr].toInt();
      }

      // task value names handling.
      for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
      {
        taskdevicevaluename[varNr].toCharArray(tmpString, 41);
        strcpy(ExtraTaskSettings.TaskDeviceValueNames[varNr], tmpString);
      }

      TempEvent.TaskIndex = index - 1;
      if (ExtraTaskSettings.TaskDeviceValueNames[0][0] == 0) // if field set empty, reload defaults
        PluginCall(PLUGIN_GET_DEVICEVALUENAMES, &TempEvent, dummyString);

      PluginCall(PLUGIN_WEBFORM_SAVE, &TempEvent, dummyString);
    }
    SaveTaskSettings(index - 1);
    if (!SaveSettings())
      reply += F("<span style=\"color:red\">Error saving to flash!</span>");
    if (taskdevicenumber.toInt() != 0 && Settings.TaskDeviceEnabled[index - 1])
      PluginCall(PLUGIN_INIT, &TempEvent, dummyString);
  }

  // show all tasks as table
  if (index == 0)
  {
    reply += F("<table border=1px frame='box' rules='all'><TR><TH>");
    reply += F("<a class='button link' href=\"devices?setpage=");
    if (page > 1)
      reply += page - 1;
    else
      reply += page;
    reply += F("\">&lt;</a>");
    reply += F("<a class='button link' href=\"devices?setpage=");
    if (page < (TASKS_MAX / 4))
      reply += page + 1;
    else
      reply += page;
    reply += F("\">&gt;</a>");

    reply += F("<TH>Task<TH>Enabled<TH>Device<TH>Name<TH>Port<TH>Ctr (IDX)<TH>GPIO<TH>Values");

    String deviceName;

    for (byte x = (page - 1) * 4; x < ((page) * 4); x++)
    {
      reply += F("<TR><TD>");
      reply += F("<a class='button link' href=\"devices?index=");
      reply += x + 1;
      reply += F("&page=");
      reply += page;
      reply += F("\">Edit</a>");
      reply += F("<TD>");
      reply += x + 1;
      reply += F("<TD>");

      if (Settings.TaskDeviceNumber[x] != 0)
      {
        LoadTaskSettings(x);
        DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[x]);
        TempEvent.TaskIndex = x;

        deviceName = "";
        Plugin_ptr[DeviceIndex](PLUGIN_GET_DEVICENAME, &TempEvent, deviceName);

        addEnabled(reply, Settings.TaskDeviceEnabled[x]);

        reply += F("<TD>");
        reply += deviceName;
        reply += F("<TD>");
        reply += ExtraTaskSettings.TaskDeviceName;
        reply += F("<TD>");

        byte customConfig = false;
        customConfig = PluginCall(PLUGIN_WEBFORM_SHOW_CONFIG, &TempEvent, reply);
        if (!customConfig)
          if (Device[DeviceIndex].Ports != 0)
            reply += Settings.TaskDevicePort[x];

        reply += F("<TD>");

        if (Device[DeviceIndex].SendDataOption)
        {
          boolean doBR = false;
          for (byte controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
          {
            byte ProtocolIndex = getProtocolIndex(Settings.Protocol[controllerNr]);
            if (Settings.TaskDeviceSendData[controllerNr][x])
            {
              if (doBR)
                reply += F("<BR>");
              reply += getControllerSymbol(controllerNr);
              if (Protocol[ProtocolIndex].usesID && Settings.Protocol[controllerNr] != 0)
              {
                reply += F(" (");
                reply += Settings.TaskDeviceID[controllerNr][x];
                reply += F(")");
                if (Settings.TaskDeviceID[controllerNr][x] == 0)
                  reply += F(" &#9888;");
              }
              doBR = true;
            }
          }
        }

        reply += F("<TD>");

        if (Settings.TaskDeviceDataFeed[x] == 0)
        {
          if (Device[DeviceIndex].Type == DEVICE_TYPE_I2C)
          {
            reply += F("GPIO-");
            reply += Settings.Pin_i2c_sda;
            reply += F("<BR>GPIO-");
            reply += Settings.Pin_i2c_scl;
          }
          if (Device[DeviceIndex].Type == DEVICE_TYPE_ANALOG)
            reply += F("ADC (TOUT)");

          if (Settings.TaskDevicePin1[x] != -1)
          {
            reply += F("GPIO-");
            reply += Settings.TaskDevicePin1[x];
          }

          if (Settings.TaskDevicePin2[x] != -1)
          {
            reply += F("<BR>GPIO-");
            reply += Settings.TaskDevicePin2[x];
          }

          if (Settings.TaskDevicePin3[x] != -1)
          {
            reply += F("<BR>GPIO-");
            reply += Settings.TaskDevicePin3[x];
          }
        }

        reply += F("<TD>");
        byte customValues = false;
        customValues = PluginCall(PLUGIN_WEBFORM_SHOW_VALUES, &TempEvent, reply);
        if (!customValues)
        {
          if (Device[DeviceIndex].VType == SENSOR_TYPE_LONG)
          {
            reply  += F("<div class=\"div_l\">");
            reply  += ExtraTaskSettings.TaskDeviceValueNames[0];
            reply  += F(":</div><div class=\"div_r\">");
            reply  += (unsigned long)UserVar[x * VARS_PER_TASK] + ((unsigned long)UserVar[x * VARS_PER_TASK + 1] << 16);
            reply  += F("</div>");
          }
          else
          {
            for (byte varNr = 0; varNr < VARS_PER_TASK; varNr++)
            {
              if ((Settings.TaskDeviceNumber[x] != 0) and (varNr < Device[DeviceIndex].ValueCount))
              {
                if (varNr > 0)
                  reply += F("<div class=\"div_br\"></div>");
                reply += F("<div class=\"div_l\">");
                reply += ExtraTaskSettings.TaskDeviceValueNames[varNr];
                reply += F(":</div><div class=\"div_r\">");
                reply += String(UserVar[x * VARS_PER_TASK + varNr], ExtraTaskSettings.TaskDeviceValueDecimals[varNr]);
                reply += "</div>";
              }
            }
          }
        }
      }
      else
        reply += F("<TD><TD><TD><TD><TD><TD>");

    } // next
    reply += F("</table></form>");
  }
  // Show edit form if a specific entry is chosen with the edit button
  else
  {
    LoadTaskSettings(index - 1);
    DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[index - 1]);
    TempEvent.TaskIndex = index - 1;

    reply += F("<form name='frmselect' method='post'><table>");
    addFormHeader(reply, F("Task Settings"));

    reply += F("<TR><TD>Device:<TD>");
    addDeviceSelect(reply, "TDNUM", Settings.TaskDeviceNumber[index - 1]);   //="taskdevicenumber"

    if (Settings.TaskDeviceNumber[index - 1] != 0 )   //any device selected?
    {
      addHelpButton(reply, String(F("Plugin")) + Settings.TaskDeviceNumber[index - 1]);

      addFormTextBox(reply, F("Name"), F("TDN"), ExtraTaskSettings.TaskDeviceName, 40);   //="taskdevicename"

      addFormCheckBox(reply, F("Enabled"), F("TDE"), Settings.TaskDeviceEnabled[index - 1]);   //="taskdeviceenabled"

      if (Settings.GlobalSync && Device[DeviceIndex].GlobalSyncOption && Settings.TaskDeviceDataFeed[index - 1] == 0 && Settings.UDPPort != 0)
      {
        addFormCheckBox(reply, F("Global Sync"), F("TDGS"), Settings.TaskDeviceGlobalSync[index - 1]);   //="taskdeviceglobalsync"
      }

      // section: Sensor / Actuator
      if (!Device[DeviceIndex].Custom && Settings.TaskDeviceDataFeed[index - 1] == 0 &&
        ((Device[DeviceIndex].Ports != 0) || (Device[DeviceIndex].PullUpOption) || (Device[DeviceIndex].InverseLogicOption) || (Device[DeviceIndex].Type >= DEVICE_TYPE_SINGLE && Device[DeviceIndex].Type <= DEVICE_TYPE_TRIPLE)) )
      {
        addFormSubHeader(reply, (Device[DeviceIndex].SendDataOption) ? F("Sensor") : F("Actuator"));

        if (Device[DeviceIndex].Ports != 0)
          addFormNumericBox(reply, F("Port"), F("TDP"), Settings.TaskDevicePort[index - 1]);   //="taskdeviceport"

        if (Device[DeviceIndex].PullUpOption)
        {
          addFormCheckBox(reply, F("Internal PullUp"), F("TDPPU"), Settings.TaskDevicePin1PullUp[index - 1]);   //="taskdevicepin1pullup"
          if ((Settings.TaskDevicePin1[index - 1] == 16) || (Settings.TaskDevicePin2[index - 1] == 16) || (Settings.TaskDevicePin3[index - 1] == 16))
            addFormNote(reply, F("GPIO-16 (D0) does not support PullUp"));
        }

        if (Device[DeviceIndex].InverseLogicOption)
          addFormCheckBox(reply, F("Inversed Logic"), F("TDPI"), Settings.TaskDevicePin1Inversed[index - 1]);   //="taskdevicepin1inversed"

        if (Device[DeviceIndex].Type >= DEVICE_TYPE_SINGLE && Device[DeviceIndex].Type <= DEVICE_TYPE_TRIPLE)
          addFormPinSelect(reply, F("1st GPIO"), F("taskdevicepin1"), Settings.TaskDevicePin1[index - 1]);
        if (Device[DeviceIndex].Type >= DEVICE_TYPE_DUAL && Device[DeviceIndex].Type <= DEVICE_TYPE_TRIPLE)
          addFormPinSelect(reply, F("2nd GPIO"), F("taskdevicepin2"), Settings.TaskDevicePin2[index - 1]);
        if (Device[DeviceIndex].Type == DEVICE_TYPE_TRIPLE)
          addFormPinSelect(reply, F("3rd GPIO"), F("taskdevicepin3"), Settings.TaskDevicePin3[index - 1]);
      }

      //add plugins content
      if (Settings.TaskDeviceDataFeed[index - 1] == 0) // only show additional config for local connected sensors
        PluginCall(PLUGIN_WEBFORM_LOAD, &TempEvent, reply);

      //section: Data Acquisition
      if (Device[DeviceIndex].SendDataOption)
      {
        addFormSubHeader(reply, F("Data Acquisition"));

        for (byte controllerNr = 0; controllerNr < CONTROLLER_MAX; controllerNr++)
        {
          if (Settings.Protocol[controllerNr] != 0)
          {
            String id = F("TDSD");   //="taskdevicesenddata"
            id += controllerNr + 1;

            reply += F("<TR><TD>Send to Controller ");
            reply += getControllerSymbol(controllerNr);
            reply += F("<TD>");
            addCheckBox(reply, id, Settings.TaskDeviceSendData[controllerNr][index - 1]);

            byte ProtocolIndex = getProtocolIndex(Settings.Protocol[controllerNr]);
            if (Protocol[ProtocolIndex].usesID && Settings.Protocol[controllerNr] != 0)
            {
              reply += F(" &nbsp; IDX: ");
              id = F("TDID");   //="taskdeviceid"
              id += controllerNr + 1;
              addNumericBox(reply, id, Settings.TaskDeviceID[controllerNr][index - 1], 0, 9999);
            }
          }
        }
      }

      addFormSeparator(reply);

      if (Device[DeviceIndex].TimerOption)
      {
        addFormNumericBox(reply, F("Delay"), F("TDT"), Settings.TaskDeviceTimer[index - 1], 0, 65535);   //="taskdevicetimer"
        addUnit(reply, F("sec"));
        if (Device[DeviceIndex].TimerOptional)
          reply += F(" (Optional for this Device)");
      }

      //section: Values
      if (!Device[DeviceIndex].Custom && Device[DeviceIndex].ValueCount > 0)
      {
        addFormSubHeader(reply, F("Values"));
        reply += F("</table><table>");

        //table header
        reply += F("<TR><TH>Value");
        reply += F("<TH>Name");

        if (Device[DeviceIndex].FormulaOption)
        {
          reply += F("<TH>Formula");
          addHelpButton(reply, F("EasyFormula"));
        }

        if (Device[DeviceIndex].FormulaOption || Device[DeviceIndex].DecimalsOnly)
        {
          reply += F("<TH>Decimals");
        }

        //table body
        for (byte varNr = 0; varNr < Device[DeviceIndex].ValueCount; varNr++)
        {
          reply += F("<TR><TD>");
          reply += varNr + 1;
          reply += F("<TD>");
          String id = F("TDVN");   //="taskdevicevaluename"
          id += (varNr + 1);
          addTextBox(reply, id, ExtraTaskSettings.TaskDeviceValueNames[varNr], 40);

          if (Device[DeviceIndex].FormulaOption)
          {
            reply += F("<TD>");
            String id = F("TDF");   //="taskdeviceformula"
            id += (varNr + 1);
            addTextBox(reply, id, ExtraTaskSettings.TaskDeviceFormula[varNr], 40);
          }

          if (Device[DeviceIndex].FormulaOption || Device[DeviceIndex].DecimalsOnly)
          {
            reply += F("<TD>");
            String id = F("TDVD");   //="taskdevicevaluedecimals"
            id += (varNr + 1);
            addNumericBox(reply, id, ExtraTaskSettings.TaskDeviceValueDecimals[varNr], 0, 6);
          }
        }
      }
    }

    addFormSeparator(reply);

    reply += F("<TR><TD><TD><a class='button link' href=\"devices?setpage=");
    reply += page;
    reply += F("\">Close</a>");
    addSubmitButton(reply);
    reply += F("<input type='hidden' name='edit' value='1'>");
    reply += F("<input type='hidden' name='page' value='1'>");
    reply += F("</table></form>");
  }

  addFooter(reply);
  checkRAM(9);
  String log = F("DEBUG: String size:");
  log += reply.length();
  addLog(LOG_LEVEL_DEBUG_MORE, log);
  sendWebPage(F("TmplStd"), reply);
}


byte sortedIndex[DEVICES_MAX + 1];
//********************************************************************************
// Add a device select dropdown list
//********************************************************************************
void addDeviceSelect(String& str, String name,  int choice)
{
  // first get the list in alphabetic order
  for (byte x = 0; x <= deviceCount; x++)
    sortedIndex[x] = x;
  sortDeviceArray();

  String deviceName;

  addSelector_Head(str, name, true);
  addSelector_Item(str, F("- None -"), 0, false, false, F(""));
  for (byte x = 0; x <= deviceCount; x++)
  {
    byte index = sortedIndex[x];
    if (Plugin_id[index] != 0)
      Plugin_ptr[index](PLUGIN_GET_DEVICENAME, 0, deviceName);

#ifdef PLUGIN_BUILD_DEV
    int num = index+1;
    String plugin = F("P");
    if (num<10) plugin += F("0");
    if (num<100) plugin += F("0");
    plugin += num;
    plugin += F(" - ");
    deviceName = plugin + deviceName;
#endif

    addSelector_Item(str,
      deviceName,
      Device[index].Number,
      choice == Device[index].Number,
      false,
      F(""));
  }
  addSelector_Foot(str);
}


//********************************************************************************
// Device Sort routine, switch array entries
//********************************************************************************
void switchArray(byte value)
{
  byte temp;
  temp = sortedIndex[value - 1];
  sortedIndex[value - 1] = sortedIndex[value];
  sortedIndex[value] = temp;
}


//********************************************************************************
// Device Sort routine, compare two array entries
//********************************************************************************
byte arrayLessThan(char *ptr_1, char *ptr_2)
{
  char check1;
  char check2;

  int i = 0;
  while (i < strlen(ptr_1))    // For each character in string 1, starting with the first:
  {
    check1 = (char)ptr_1[i];  // get the same char from string 1 and string 2

    if (strlen(ptr_2) < i)    // If string 2 is shorter, then switch them
    {
      return 1;
    }
    else
    {
      check2 = (char)ptr_2[i];

      if (check2 > check1)
      {
        return 1;       // String 2 is greater; so switch them
      }
      if (check2 < check1)
      {
        return 0;       // String 2 is LESS; so DONT switch them
      }
      // OTHERWISE they're equal so far; check the next char !!
      i++;
    }
  }

  return 0;
}


//********************************************************************************
// Device Sort routine, actual sorting
//********************************************************************************
void sortDeviceArray()
{
  String deviceName;
  char deviceName1[41];
  char deviceName2[41];
  int innerLoop ;
  int mainLoop ;

  for ( mainLoop = 1; mainLoop <= deviceCount; mainLoop++)
  {
    innerLoop = mainLoop;
    while (innerLoop  >= 1)
    {
      Plugin_ptr[sortedIndex[innerLoop]](PLUGIN_GET_DEVICENAME, 0, deviceName);
      deviceName.toCharArray(deviceName1, 26);
      Plugin_ptr[sortedIndex[innerLoop - 1]](PLUGIN_GET_DEVICENAME, 0, deviceName);
      deviceName.toCharArray(deviceName2, 26);

      if (arrayLessThan(deviceName1, deviceName2) == 1)
      {
        switchArray(innerLoop);
      }
      innerLoop--;
    }
  }
}


void addFormPinSelect(String& str, const String& label, const String& id, int choice)
{
  addRowLabel(str, label);
  addPinSelect(false, str, id, choice);
}


void addFormPinSelectI2C(String& str, const String& label, const String& id, int choice)
{
  addRowLabel(str, label);
  addPinSelect(true, str, id, choice);
}


//********************************************************************************
// Add a GPIO pin select dropdown list for both 8266 and 8285
//********************************************************************************
#ifdef ESP8285
// Code for the ESP8285

//********************************************************************************
// Add a GPIO pin select dropdown list
//********************************************************************************
void addPinSelect(boolean forI2C, String& str, String name,  int choice)
{
  String options[18];
  options[0] = F(" ");
  options[1] = F("GPIO-0 (D3)");
  options[2] = F("GPIO-1 (D10)");
  options[3] = F("GPIO-2 (D4)");
  options[4] = F("GPIO-3 (D9)");
  options[5] = F("GPIO-4 (D2)");
  options[6] = F("GPIO-5 (D1)");
  options[7] = F("GPIO-6");
  options[8] = F("GPIO-7");
  options[9] = F("GPIO-8");
  options[10] = F("GPIO-9 (D11)");
  options[11] = F("GPIO-10 (D12)");
  options[12] = F("GPIO-11");
  options[13] = F("GPIO-12 (D6)");
  options[14] = F("GPIO-13 (D7)");
  options[15] = F("GPIO-14 (D5)");
  options[16] = F("GPIO-15 (D8)");
  options[17] = F("GPIO-16 (D0)");
  int optionValues[18];
  optionValues[0] = -1;
  optionValues[1] = 0;
  optionValues[2] = 1;
  optionValues[3] = 2;
  optionValues[4] = 3;
  optionValues[5] = 4;
  optionValues[6] = 5;
  optionValues[7] = 7;
  optionValues[8] = 7;
  optionValues[9] = 8;
  optionValues[10] = 9;
  optionValues[11] = 10;
  optionValues[12] = 11;
  optionValues[13] = 12;
  optionValues[14] = 13;
  optionValues[15] = 14;
  optionValues[16] = 15;
  optionValues[17] = 16;
  renderHTMLForPinSelect(options, optionValues, forI2C, str, name, choice, 18);
}


#else
// Code for the ESP8266

//********************************************************************************
// Add a GPIO pin select dropdown list
//********************************************************************************
void addPinSelect(boolean forI2C, String& str, String name,  int choice)
{
  String options[14];
  options[0] = F(" ");
  options[1] = F("GPIO-0 (D3)");
  options[2] = F("GPIO-1 (D10)");
  options[3] = F("GPIO-2 (D4)");
  options[4] = F("GPIO-3 (D9)");
  options[5] = F("GPIO-4 (D2)");
  options[6] = F("GPIO-5 (D1)");
  options[7] = F("GPIO-9 (D11)");
  options[8] = F("GPIO-10 (D12)");
  options[9] = F("GPIO-12 (D6)");
  options[10] = F("GPIO-13 (D7)");
  options[11] = F("GPIO-14 (D5)");
  options[12] = F("GPIO-15 (D8)");
  options[13] = F("GPIO-16 (D0)");
  int optionValues[14];
  optionValues[0] = -1;
  optionValues[1] = 0;
  optionValues[2] = 1;
  optionValues[3] = 2;
  optionValues[4] = 3;
  optionValues[5] = 4;
  optionValues[6] = 5;
  optionValues[7] = 9;
  optionValues[8] = 10;
  optionValues[9] = 12;
  optionValues[10] = 13;
  optionValues[11] = 14;
  optionValues[12] = 15;
  optionValues[13] = 16;
  renderHTMLForPinSelect(options, optionValues, forI2C, str, name, choice, 14);
}

#endif

//********************************************************************************
// Helper function actually rendering dropdown list for addPinSelect()
//********************************************************************************
void renderHTMLForPinSelect(String options[], int optionValues[], boolean forI2C, String& str, String name,  int choice, int count) {
    addSelector_Head(str, name, false);
    for (byte x = 0; x < count; x++)
    {
      boolean disabled = false;

      if (optionValues[x] != -1) // empty selection can never be disabled...
      {
        if (!forI2C && ((optionValues[x] == Settings.Pin_i2c_sda) || (optionValues[x] == Settings.Pin_i2c_scl)))
          disabled = true;
        if (Settings.UseSerial && ((optionValues[x] == 1) || (optionValues[x] == 3)))
        disabled = true;
      }
      addSelector_Item(str,
        options[x],
        optionValues[x],
        choice == optionValues[x],
        disabled,
        F(""));
    }
    addSelector_Foot(str);
}


void addFormSelectorI2C(String& str, const String& id, int addressCount, const int addresses[], int selectedIndex)
{
  String options[addressCount];
  for (byte x = 0; x < addressCount; x++)
  {
    options[x] = F("0x");
    options[x] += String(addresses[x], HEX);
    if (x == 0)
      options[x] += F(" - (default)");
  }
  addFormSelector(str, F("I2C Address"), id, addressCount, options, addresses, NULL, selectedIndex, false);
}

void addFormSelector(String& str, const String& label, const String& id, int optionCount, const String options[], const int indices[], int selectedIndex)
{
  addFormSelector(str, label, id, optionCount, options, indices, NULL, selectedIndex, false);
}

void addFormSelector(String& str, const String& label, const String& id, int optionCount, const String options[], const int indices[], const String attr[], int selectedIndex, boolean reloadonchange)
{
  addRowLabel(str, label);
  addSelector(str, id, optionCount, options, indices, attr, selectedIndex, reloadonchange);
}

void addSelector(String& str, const String& id, int optionCount, const String options[], const int indices[], const String attr[], int selectedIndex, boolean reloadonchange)
{
  int index;

  str += F("<select name='");
  str += id;
  str += F("'");
  if (reloadonchange)
    str += F(" onchange=\"return dept_onchange(frmselect)\"");
  str += F(">");
  for (byte x = 0; x < optionCount; x++)
  {
    if (indices)
      index = indices[x];
    else
      index = x;
    str += F("<option value=");
    str += index;
    if (selectedIndex == index)
      str += F(" selected");
    if (attr)
    {
      str += F(" ");
      str += attr[x];
    }
    str += ">";
    str += options[x];
    str += F("</option>");
  }
  str += F("</select>");
}


void addSelector_Head(String& str, const String& id, boolean reloadonchange)
{
  str += F("<select name='");
  str += id;
  str += F("'");
  if (reloadonchange)
    str += F(" onchange=\"return dept_onchange(frmselect)\"");
  str += F(">");
}

void addSelector_Item(String& str, const String& option, int index, boolean selected, boolean disabled, const String& attr)
{
  str += F("<option value=");
  str += index;
  if (selected)
    str += F(" selected");
  if (disabled)
    str += F(" disabled");
  if (attr && attr.length() > 0)
  {
    str += F(" ");
    str += attr;
  }
  str += ">";
  str += option;
  str += F("</option>");
}


void addSelector_Foot(String& str)
{
  str += F("</select>");
}


void addUnit(String& str, const String& unit)
{
  str += F(" [");
  str += unit;
  str += F("]");
}


void addRowLabel(String& str, const String& label)
{
  str += F("<TR><TD>");
  str += label;
  str += F(":<TD>");
}

void addButton(String& str, const String &url, const String &label)
{
  str += F("<a class='button link' href='");
  str += url;
  str += F("'>");
  str += label;
  str += F("</a>");
}

void addSubmitButton(String& str)
{
  str += F("<input class='button link' type='submit' value='Submit'>");
}

//********************************************************************************
// Add a header
//********************************************************************************
void addFormHeader(String& str, const String& header1, const String& header2)
{
  str += F("<TR><TH>");
  str += header1;
  str += F("<TH>");
  str += header2;
  str += F("");
}

void addFormHeader(String& str, const String& header)
{
  str += F("<TR><TD colspan='2'><h2>");
  str += header;
  str += F("</h2>");
}


//********************************************************************************
// Add a sub header
//********************************************************************************
void addFormSubHeader(String& str, const String& header)
{
  str += F("<TR><TD colspan='2'><h3>");
  str += header;
  str += F("</h3>");
}


//********************************************************************************
// Add a note as row start
//********************************************************************************
void addFormNote(String& str, const String& text)
{
  str += F("<TR><TD><TD><div class='note'>Note: ");
  str += text;
  str += F("</div>");
}


//********************************************************************************
// Add a separator as row start
//********************************************************************************
void addFormSeparator(String& str)
{
  str += F("<TR><TD colspan='2'><hr>");
}


//********************************************************************************
// Add a checkbox
//********************************************************************************
void addCheckBox(String& str, const String& id, boolean checked)
{
  str += F("<input type=checkbox id='");
  str += id;
  str += F("' name='");
  str += id;
  str += F("'");
  if (checked)
    str += F(" checked");
  str += F(">");
}

void addFormCheckBox(String& str, const String& label, const String& id, boolean checked)
{
  addRowLabel(str, label);
  addCheckBox(str, id, checked);
}


//********************************************************************************
// Add a numeric box
//********************************************************************************
void addNumericBox(String& str, const String& id, int value, int min, int max)
{
  str += F("<input type='number' name='");
  str += id;
  str += F("'");
  if (min != INT_MIN)
  {
		str += F(" min=");
		str += min;
  }
  if (max != INT_MAX)
  {
    str += F(" max=");
    str += max;
  }
  str += F(" style='width:5em;' value=");
  str += value;
  str += F(">");
}

void addNumericBox(String& str, const String& id, int value)
{
	addNumericBox(str, id, value, INT_MIN, INT_MAX);
}

void addFormNumericBox(String& str, const String& label, const String& id, int value, int min, int max)
{
  addRowLabel(str,  label);
  addNumericBox(str, id, value, min, max);
}

void addFormNumericBox(String& str, const String& label, const String& id, int value)
{
	addFormNumericBox(str, label, id, value, INT_MIN, INT_MAX);
}



void addTextBox(String& str, const String& id, const String&  value, int maxlength)
{
  str += F("<input type='text' name='");
  str += id;
  str += F("' maxlength=");
  str += maxlength;
  str += F(" value='");
  str += value;
  str += F("'>");
}

void addFormTextBox(String& str, const String& label, const String& id, const String&  value, int maxlength)
{
  addRowLabel(str, label);
  addTextBox(str, id, value, maxlength);
}


void addFormPasswordBox(String& str, const String& label, const String& id, const String& password, int maxlength)
{
  addRowLabel(str, label);
  str += F("<input type='password' name='");
  str += id;
  str += F("' maxlength=");
  str += maxlength;
  str += F(" value='");
  if (password != F(""))   //no password?
    str += F("*****");
  //str += password;   //password will not published over HTTP
  str += F("'>");
}

void copyFormPassword(const String& id, char* pPassword, int maxlength)
{
  String password = WebServer.arg(id);
  if (password == F("*****"))   //no change?
    return;
  strncpy(pPassword, password.c_str(), maxlength);
}

void addFormIPBox(String& str, const String& label, const String& id, const byte ip[4])
{
  char strip[20];
  if (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0)
    strip[0] = 0;
  else
    sprintf_P(strip, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);

  addRowLabel(str, label);
  str += F("<input type='text' name='");
  str += id;
  str += F("' value='");
  str += strip;
  str += F("'>");
}

// adds a Help Button with points to the the given Wiki Subpage
void addHelpButton(String& str, const String& url)
{
  str += F(" <a class=\"button help\" href=\"http://www.letscontrolit.com/wiki/index.php/");
  str += url;
  str += F("\" target=\"_blank\">&#10068;</a>");
}


void addEnabled(String& str, boolean enabled)
{
  if (enabled)
    str += F("<span class='enabled on'>&#10004;</span>");
  else
    str += F("<span class='enabled off'>&#10008;</span>");
}


//********************************************************************************
// Add a task select dropdown list
//********************************************************************************
void addTaskSelect(String& str, String name,  int choice)
{
  struct EventStruct TempEvent;
  String deviceName;

  str += F("<select name='");
  str += name;
  str += F("' onchange=\"return dept_onchange(frmselect)\">");

  for (byte x = 0; x < TASKS_MAX; x++)
  {
    deviceName = "";
    if (Settings.TaskDeviceNumber[x] != 0 )
    {
      byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[x]);

      if (Plugin_id[DeviceIndex] != 0)
        Plugin_ptr[DeviceIndex](PLUGIN_GET_DEVICENAME, &TempEvent, deviceName);
    }
    LoadTaskSettings(x);
    str += F("<option value='");
    str += x;
    str += "'";
    if (choice == x)
      str += " selected";
    if (Settings.TaskDeviceNumber[x] == 0)
      str += " disabled";
    str += ">";
    str += x + 1;
    str += " - ";
    str += deviceName;
    str += " - ";
    str += ExtraTaskSettings.TaskDeviceName;
    str += "</option>";
  }
}


bool isFormItemChecked(const String& id)
{
  return WebServer.arg(id) == "on";
}

int getFormItemInt(const String& id)
{
  String val = WebServer.arg(id);
  return val.toInt();
}

float getFormItemFloat(const String& id)
{
  String val = WebServer.arg(id);
  return val.toFloat();
}

bool isFormItem(const String& id)
{
  return (WebServer.arg(id).length() != 0);
}


//********************************************************************************
// Add a Value select dropdown list, based on TaskIndex
//********************************************************************************
void addTaskValueSelect(String& str, String name,  int choice, byte TaskIndex)
{
  str += F("<select name='");
  str += name;
  str += "'>";

  byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[TaskIndex]);

  for (byte x = 0; x < Device[DeviceIndex].ValueCount; x++)
  {
    str += F("<option value='");
    str += x;
    str += "'";
    if (choice == x)
      str += " selected";
    str += ">";
    str += ExtraTaskSettings.TaskDeviceValueNames[x];
    str += "</option>";
  }
}


//********************************************************************************
// Web Interface log page
//********************************************************************************
void handle_log() {
  if (!isLoggedIn()) return;

  navMenuIndex = 7;
  char *TempString = (char*)malloc(80);

  String reply = "";
  addHeader(true, reply);
  reply += F("<script>function RefreshMe(){window.location = window.location}setTimeout('RefreshMe()', 3000);</script>");
  reply += F("<table><TR><TH>Log<TR><TD>");

  if (logcount != -1)
  {
    byte counter = logcount;
    do
    {
      counter++;
      if (counter > 9)
        counter = 0;
      if (Logging[counter].timeStamp > 0)
      {
        reply += Logging[counter].timeStamp;
        reply += " : ";
        reply += Logging[counter].Message;
        reply += F("<BR>");
      }
    }  while (counter != logcount);
  }
  reply += F("</table>");
  addFooter(reply);
  sendWebPage(F("TmplStd"), reply);
  free(TempString);
}


//********************************************************************************
// Web Interface debug page
//********************************************************************************
void handle_tools() {
  if (!isLoggedIn()) return;

  navMenuIndex = 7;
  String webrequest = WebServer.arg(F("cmd"));

  String reply = "";
  addHeader(true, reply);


  reply += F("<form>");
  reply += F("<table>");

  addFormHeader(reply, F("Tools"));

  addFormSubHeader(reply, F("System"));

  reply += F("<TR><TD HEIGHT=\"30\">");
  addButton(reply, F("/?cmd=reboot"), F("Reboot"));
  reply += F("<TD>");
  reply += F("Reboots ESP");

  reply += F("<TR><TD HEIGHT=\"30\">");
  addButton(reply, F("log"), F("Log"));
  reply += F("<TD>");
  reply += F("Open log output");

  reply += F("<TR><TD HEIGHT=\"30\">");
  addButton(reply, F("sysinfo"), F("Info"));
  reply += F("<TD>");
  reply += F("Open system info page");

  reply += F("<TR><TD HEIGHT=\"30\">");
  addButton(reply, F("advanced"), F("Advanced"));
  reply += F("<TD>");
  reply += F("Open advanced settings");

  reply += F("<TR><TD HEIGHT=\"30\">");
  addButton(reply, F("json"), F("Show JSON"));
  reply += F("<TD>");
  reply += F("Open JSON output");

  addFormSubHeader(reply, F("Wifi"));

  reply += F("<TR><TD HEIGHT=\"30\">");
  addButton(reply, F("/?cmd=wificonnect"), F("Connect"));
  reply += F("<TD>");
  reply += F("Connects to known Wifi network");

  reply += F("<TR><TD HEIGHT=\"30\">");
  addButton(reply, F("/?cmd=wifidisconnect"), F("Disconnect"));
  reply += F("<TD>");
  reply += F("Disconnect from wifi network");

  reply += F("<TR><TD HEIGHT=\"30\">");
  addButton(reply, F("wifiscanner"), F("Scan"));
  reply += F("<TD>");
  reply += F("Scan for wifi networks");

  addFormSubHeader(reply, F("Interfaces"));

  reply += F("<TR><TD HEIGHT=\"30\">");
  addButton(reply, F("i2cscanner"), F("I2C Scan"));
  reply += F("<TD>");
  reply += F("Scan for I2C devices");

  addFormSubHeader(reply, F("Settings"));

  reply += F("<TR><TD HEIGHT=\"30\">");
  addButton(reply, F("upload"), F("Load"));
  reply += F("<TD>");
  reply += F("Loads a settings file");
  addFormNote(reply, F("(File MUST be renamed to \"config.dat\" before upload!)"));

  reply += F("<TR><TD HEIGHT=\"30\">");
  addButton(reply, F("download"), F("Save"));
  reply += F("<TD>");
  reply += F("Saves a settings file");

  if (ESP.getFlashChipRealSize() > 524288)
  {
    addFormSubHeader(reply, F("Firmware"));
    reply += F("<TR><TD HEIGHT=\"30\">");
    addButton(reply, F("update"), F("Load"));
    addHelpButton(reply, F("EasyOTA"));
    reply += F("<TD>");
    reply += F("Load a new firmware");
  }

  addFormSubHeader(reply, F("Filesystem"));

  reply += F("<TR><TD HEIGHT=\"30\">");
  addButton(reply, F("filelist"), F("Flash"));
  reply += F("<TD>");
  reply += F("Show files on internal flash");

  reply += F("<TR><TD HEIGHT=\"30\">");
  addButton(reply, F("SDfilelist"), F("SD Card"));
  reply += F("<TD>");
  reply += F("Show files on SD-Card");

  addFormSubHeader(reply, F("Command"));
  reply += F("<TR><TD HEIGHT=\"30\">");
  reply += F("<input type='text' name='cmd' value='");
  reply += webrequest;
  reply += F("'><TD>");
  addSubmitButton(reply);
  reply += F("<TR><TD>");

  printToWeb = true;
  printWebString = "";

  if (webrequest.length() > 0)
  {
    struct EventStruct TempEvent;
    parseCommandString(&TempEvent, webrequest);
    TempEvent.Source = VALUE_SOURCE_HTTP;
    if (!PluginCall(PLUGIN_WRITE, &TempEvent, webrequest))
      ExecuteCommand(VALUE_SOURCE_HTTP, webrequest.c_str());
  }

  if (printWebString.length() > 0)
  {
    reply += F("<TR><TD>Command Output<TD><textarea readonly rows='10' cols='60' wrap='on'>");
    reply += printWebString;
    reply += F("</textarea>");
  }
  reply += F("</table></form>");
  addFooter(reply);
  sendWebPage(F("TmplStd"), reply);
  printWebString = "";
  printToWeb = false;
}


//********************************************************************************
// Web Interface I2C scanner
//********************************************************************************
void handle_i2cscanner() {
  if (!isLoggedIn()) return;

  navMenuIndex = 7;
  char *TempString = (char*)malloc(80);

  String reply = "";
  addHeader(true, reply);
  reply += F("<table border=1px frame='box' rules='all'><TH>I2C Addresses in use<TH>Supported devices");

  byte error, address;
  int nDevices;
  nDevices = 0;
  for (address = 1; address <= 127; address++ )
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {
      reply += "<TR><TD>0x";
      reply += String(address, HEX);
      reply += "<TD>";
      switch (address)
      {
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x25:
        case 0x26:
        case 0x27:
          reply += F("PCF8574<BR>MCP23017<BR>LCD");
          break;
        case 0x23:
          reply += F("PCF8574<BR>MCP23017<BR>LCD<BR>BH1750");
          break;
        case 0x24:
          reply += F("PCF8574<BR>MCP23017<BR>LCD<BR>PN532");
          break;
        case 0x29:
          reply += F("TSL2561<BR>TCS34725");
          break;
        case 0x38:
        case 0x3A:
        case 0x3B:
        case 0x3E:
        case 0x3F:
          reply += F("PCF8574A");
          break;
        case 0x39:
          reply += F("PCF8574A<BR>TSL2561");
          break;
        case 0x3C:
        case 0x3D:
          reply += F("PCF8574A<BR>OLED");
          break;
        case 0x40:
          reply += F("SI7021<BR>HTU21D<BR>INA219<BR>PCA9685");
          break;
        case 0x41:
        case 0x42:
        case 0x43:
          reply += F("INA219");
          break;
        case 0x48:
        case 0x4A:
        case 0x4B:
          reply += F("PCF8591<BR>ADS1115");
          break;
        case 0x49:
          reply += F("PCF8591<BR>ADS1115<BR>TSL2561");
          break;
        case 0x4C:
        case 0x4E:
        case 0x4F:
          reply += F("PCF8591");
          break;
        case 0x4D:
          reply += F("PCF8591<BR>MCP3221");
          break;
        case 0x5A:
          reply += F("MLX90614");
          break;
        case 0x5C:
          reply += F("DHT12<BR>AM2320<BR>BH1750");
          break;
        case 0x60:
          reply += F("Adafruit Motorshield v2<BR>SI1145");
          break;
        case 0x70:
          reply += F("TCA9548A<BR>Adafruit Motorshield v2 (Catchall)");
          break;
        case 0x76:
          reply += F("BME280<BR>BMP280<BR>MS5607<BR>MS5611");
          break;
        case 0x77:
          reply += F("BMP085<BR>BMP180<BR>BME280<BR>BMP280<BR>MS5607<BR>MS5611");
          break;
        case 0x7f:
          reply += F("Arduino PME");
          break;
      }
      nDevices++;
    }
    else if (error == 4)
    {
      reply += F("<TR><TD>Unknown error at address 0x");
      reply += String(address, HEX);
    }
  }

  if (nDevices == 0)
    reply += F("<TR>No I2C devices found");

  reply += F("</table>");
  addFooter(reply);
  sendWebPage(F("TmplStd"), reply);
  free(TempString);
}


//********************************************************************************
// Web Interface Wifi scanner
//********************************************************************************
void handle_wifiscanner() {
  if (!isLoggedIn()) return;

  navMenuIndex = 7;
  char *TempString = (char*)malloc(80);

  String reply = "";
  addHeader(true, reply);
  reply += F("<table><TR><TH>Access Points:<TH>RSSI");

  int n = WiFi.scanNetworks();
  if (n == 0)
    reply += F("No Access Points found");
  else
  {
    for (int i = 0; i < n; ++i)
    {
      reply += F("<TR><TD>");
      reply += WiFi.SSID(i);
      reply += "<TD>";
      reply += WiFi.RSSI(i);
    }
  }


  reply += F("</table>");
  addFooter(reply);
  sendWebPage(F("TmplStd"), reply);
  free(TempString);
}


//********************************************************************************
// Web Interface login page
//********************************************************************************
void handle_login() {

  String webrequest = WebServer.arg(F("password"));
  char command[80];
  command[0] = 0;
  webrequest.toCharArray(command, 80);

  String reply = "";
  reply += F("<form method='post'>");
  reply += F("<table><TR><TD>Password<TD>");
  reply += F("<input type='password' name='password' value='");
  reply += webrequest;
  reply += F("'><TR><TD><TD>");
  addSubmitButton(reply);
  reply += F("<TR><TD>");
  reply += F("</table></form>");

  if (webrequest.length() != 0)
  {
    // compare with stored password and set timer if there's a match
    if ((strcasecmp(command, SecuritySettings.Password) == 0) || (SecuritySettings.Password[0] == 0))
    {
      WebLoggedIn = true;
      WebLoggedInTimer = 0;
      reply = F("<script>window.location = '.'</script>");
    }
    else
    {
      reply += F("Invalid password!");
    }
  }

  sendWebPage(F("TmplMsg"), reply);
  printWebString = "";
  printToWeb = false;
}


//********************************************************************************
// Web Interface control page (no password!)
//********************************************************************************
void handle_control() {

  String webrequest = WebServer.arg(F("cmd"));

  // in case of event, store to buffer and return...
  String command = parseString(webrequest, 1);
  if (command == F("event"))
  {
    eventBuffer = webrequest.substring(6);
    WebServer.send(200, "text/html", "OK");
  }

  struct EventStruct TempEvent;
  parseCommandString(&TempEvent, webrequest);
  TempEvent.Source = VALUE_SOURCE_HTTP;

  printToWeb = true;
  printWebString = "";
  String reply = "";

  if (PluginCall(PLUGIN_WRITE, &TempEvent, webrequest));
  else if (remoteConfig(&TempEvent, webrequest));
  else
    reply += F("Unknown or restricted command!");

  reply += printWebString;

  if (printToWebJSON)
    WebServer.send(200, "application/json", reply);
  else
    WebServer.send(200, "text/html", reply);

  printWebString = "";
  printToWeb = false;
  printToWebJSON = false;
}


//********************************************************************************
// Web Interface JSON page (no password!)
//********************************************************************************

boolean handle_json()
{
  String tasknr = WebServer.arg(F("tasknr"));
  String reply = "";

  if (tasknr.length() == 0)
  {
    reply += F("{\"System\":{\n");
    reply += F("\"Build\": ");
    reply += BUILD;
    reply += F(",\n\"Unit\": ");
    reply += Settings.Unit;
    reply += F(",\n\"Uptime\": ");
    reply += wdcounter / 2;
    reply += F(",\n\"Free RAM\": ");
    reply += ESP.getFreeHeap();
    reply += F("\n},\n");
  }

  byte taskNr = tasknr.toInt();
  byte firstTaskIndex = 0;
  byte lastTaskIndex = TASKS_MAX - 1;
  if (taskNr != 0 )
  {
    firstTaskIndex = taskNr - 1;
    lastTaskIndex = taskNr - 1;
  }

  byte lastActiveTaskIndex = 0;
  for (byte TaskIndex = firstTaskIndex; TaskIndex <= lastTaskIndex; TaskIndex++)
    if (Settings.TaskDeviceNumber[TaskIndex])
      lastActiveTaskIndex = TaskIndex;

  if (taskNr == 0 )
    reply += F("\"Sensors\":[\n");
  for (byte TaskIndex = firstTaskIndex; TaskIndex <= lastTaskIndex; TaskIndex++)
  {
    if (Settings.TaskDeviceNumber[TaskIndex])
    {
      byte BaseVarIndex = TaskIndex * VARS_PER_TASK;
      byte DeviceIndex = getDeviceIndex(Settings.TaskDeviceNumber[TaskIndex]);
      LoadTaskSettings(TaskIndex);
      reply += F("{\n");

      reply += F("\"TaskName\": \"");
      reply += ExtraTaskSettings.TaskDeviceName;
      reply += F("\"");
      if (Device[DeviceIndex].ValueCount != 0)
        reply += F(",");
      reply += F("\n");

      for (byte x = 0; x < Device[DeviceIndex].ValueCount; x++)
      {
        reply += F("\"");
        reply += ExtraTaskSettings.TaskDeviceValueNames[x];
        reply += F("\": ");
        reply += UserVar[BaseVarIndex + x];
        if (x < (Device[DeviceIndex].ValueCount - 1))
          reply += F(",");
        reply += F("\n");
      }
      reply += F("}");
      if (TaskIndex != lastActiveTaskIndex)
        reply += F(",");
      reply += F("\n");
    }
  }
  if (taskNr == 0 )
    reply += F("]}\n");

  WebServer.send(200, "application/json", reply);
}


//********************************************************************************
// Web Interface config page
//********************************************************************************
void handle_advanced() {
  if (!isLoggedIn()) return;

  navMenuIndex = 7;
  char tmpString[81];

  String messagedelay = WebServer.arg(F("messagedelay"));
  String ip = WebServer.arg(F("ip"));
  String syslogip = WebServer.arg(F("syslogip"));
  String ntphost = WebServer.arg(F("ntphost"));
  String timezone = WebServer.arg(F("timezone"));
  String dst = WebServer.arg(F("dst"));
  String sysloglevel = WebServer.arg(F("sysloglevel"));
  String udpport = WebServer.arg(F("udpport"));
  String useserial = WebServer.arg(F("useserial"));
  String serialloglevel = WebServer.arg(F("serialloglevel"));
  String webloglevel = WebServer.arg(F("webloglevel"));
  String sdloglevel = WebServer.arg(F("sdloglevel"));
  String baudrate = WebServer.arg(F("baudrate"));
  String usentp = WebServer.arg(F("usentp"));
  String wdi2caddress = WebServer.arg(F("wdi2caddress"));
  String usessdp = WebServer.arg(F("usessdp"));
  String edit = WebServer.arg(F("edit"));
  String wireclockstretchlimit = WebServer.arg(F("wireclockstretchlimit"));
  String globalsync = WebServer.arg(F("globalsync"));
  String userules = WebServer.arg(F("userules"));
  String cft = WebServer.arg(F("cft"));
  String MQTTRetainFlag = WebServer.arg(F("mqttretainflag"));

  String reply = "";
  addHeader(true, reply);

  if (edit.length() != 0)
  {
    Settings.MessageDelay = messagedelay.toInt();
    Settings.IP_Octet = ip.toInt();
    ntphost.toCharArray(tmpString, 64);
    strcpy(Settings.NTPHost, tmpString);
    Settings.TimeZone = timezone.toInt();
    syslogip.toCharArray(tmpString, 26);
    str2ip(tmpString, Settings.Syslog_IP);
    Settings.UDPPort = udpport.toInt();
    Settings.SyslogLevel = sysloglevel.toInt();
    Settings.UseSerial = (useserial == "on");
    Settings.SerialLogLevel = serialloglevel.toInt();
    Settings.WebLogLevel = webloglevel.toInt();
    Settings.SDLogLevel = sdloglevel.toInt();
    Settings.BaudRate = baudrate.toInt();
    Settings.UseNTP = (usentp == "on");
    Settings.DST = (dst == "on");
    Settings.WDI2CAddress = wdi2caddress.toInt();
    Settings.UseSSDP = (usessdp == "on");
    Settings.WireClockStretchLimit = wireclockstretchlimit.toInt();
    Settings.UseRules = (userules == "on");
    Settings.GlobalSync = (globalsync == "on");
    Settings.ConnectionFailuresThreshold = cft.toInt();
    Settings.MQTTRetainFlag = (MQTTRetainFlag == "on");
    if (!SaveSettings())
      reply += F("<span style=\"color:red\">Error saving to flash!</span>");
    if (Settings.UseNTP)
      initTime();
  }

  char str[20];

  reply += F("<form  method='post'><table>");

  addFormHeader(reply, F("Advanced Settings"));

  addFormCheckBox(reply, F("Rules"), F("userules"), Settings.UseRules);

  addFormSubHeader(reply, F("Controller Settings"));

  addFormCheckBox(reply, F("MQTT Retain Msg"), F("mqttretainflag"), Settings.MQTTRetainFlag);
  addFormNumericBox(reply, F("Message Delay"), F("messagedelay"), Settings.MessageDelay, 0, 10000);
  addUnit(reply, F("ms"));

  addFormSubHeader(reply, F("NTP Settings"));

  addFormCheckBox(reply, F("Use NTP"), F("usentp"), Settings.UseNTP);
  addFormTextBox(reply, F("NTP Hostname"), F("ntphost"), Settings.NTPHost, 63);
  addFormNumericBox(reply, F("Timezone Offset"), F("timezone"), Settings.TimeZone, -43200, 43200);   // +/-12h
  addUnit(reply, F("minutes"));
  addFormCheckBox(reply, F("DST"), F("dst"), Settings.DST);


  addFormSubHeader(reply, F("Log Settings"));

  addFormIPBox(reply, F("Syslog IP"), F("syslogip"), Settings.Syslog_IP);
  addFormNumericBox(reply, F("Syslog Level"), F("sysloglevel"), Settings.SyslogLevel, 0, 4);

  addFormNumericBox(reply, F("Serial log Level"), F("serialloglevel"), Settings.SerialLogLevel, 0, 4);
  addFormNumericBox(reply, F("Web log Level"), F("webloglevel"), Settings.WebLogLevel, 0, 4);
  addFormNumericBox(reply, F("SD Card log Level"), F("sdloglevel"), Settings.SDLogLevel, 0, 4);


  addFormSubHeader(reply, F("Serial Settings"));

  addFormCheckBox(reply, F("Enable Serial port"), F("useserial"), Settings.UseSerial);
  addFormNumericBox(reply, F("Baud Rate"), F("baudrate"), Settings.BaudRate, 0, 1000000);


  addFormSubHeader(reply, F("Inter-ESPEasy Network (experimental)"));

  addFormCheckBox(reply, F("Global Sync"), F("globalsync"), Settings.GlobalSync);
  addFormNumericBox(reply, F("UDP port"), F("udpport"), Settings.UDPPort, 0, 65535);


  //TODO sort settings in groups or move to other pages/groups
  addFormSubHeader(reply, F("Special and Experimental Settings"));

  addFormNumericBox(reply, F("Fixed IP Octet"), F("ip"), Settings.IP_Octet, 0, 255);

  addFormNumericBox(reply, F("WD I2C Address"), F("wdi2caddress"), Settings.WDI2CAddress, 0, 127);
  reply += F(" (decimal)");

  addFormCheckBox(reply, F("Use SSDP"), F("usessdp"), Settings.UseSSDP);

  addFormNumericBox(reply, F("Connection Failure Threshold"), F("cft"), Settings.ConnectionFailuresThreshold, 0, 100);

  addFormNumericBox(reply, F("I2C ClockStretchLimit"), F("wireclockstretchlimit"), Settings.WireClockStretchLimit);   //TODO define limits

  addFormSeparator(reply);

  reply += F("<TR><TD><TD>");
  addSubmitButton(reply);
  reply += F("<input type='hidden' name='edit' value='1'>");
  reply += F("</table></form>");
  addFooter(reply);
  sendWebPage(F("TmplStd"), reply);
}


//********************************************************************************
// Login state check
//********************************************************************************
boolean isLoggedIn()
{
  if (SecuritySettings.Password[0] == 0)
    WebLoggedIn = true;

  if (!WebLoggedIn)
  {
    WebServer.sendContent(F("HTTP/1.1 302 \r\nLocation: /login\r\n"));
  }
  else
  {
    WebLoggedInTimer = 0;
  }

  return WebLoggedIn;
}


//********************************************************************************
// Web Interface download page
//********************************************************************************
void handle_download()
{
  if (!isLoggedIn()) return;

  navMenuIndex = 7;
  fs::File dataFile = SPIFFS.open("config.dat", "r");
  if (!dataFile)
    return;

  String str = F("attachment; filename=config_");
  str += Settings.Name;
  str += "_U";
  str += Settings.Unit;
  str += F("_Build");
  str += BUILD;
  str += F("_");
  if (Settings.UseNTP)
  {
  	str += getDateTimeString('\0', '\0', '\0');
  }
  str += (".dat");

  WebServer.sendHeader("Content-Disposition", str);
  WebServer.streamFile(dataFile, "application/octet-stream");
}


//********************************************************************************
// Web Interface upload page
//********************************************************************************
byte uploadResult = 0;
void handle_upload() {
  if (!isLoggedIn()) return;

  String reply = "";
  addHeader(true, reply);

  navMenuIndex = 7;
  reply += F("<form enctype=\"multipart/form-data\" method=\"post\"><p>Upload settings file:<br><input type=\"file\" name=\"datafile\" size=\"40\"></p><div><input class='button link' type='submit' value='Upload'></div><input type='hidden' name='edit' value='1'></form>");
  addFooter(reply);
  sendWebPage(F("TmplStd"), reply);
  printWebString = "";
  printToWeb = false;
}


//********************************************************************************
// Web Interface upload page
//********************************************************************************
void handle_upload_post() {
  if (!isLoggedIn()) return;

  navMenuIndex = 7;
  String reply = "";

  if (uploadResult == 1)
  {
    reply += F("Upload OK!<BR>You may need to reboot to apply all settings...");
    LoadSettings();
  }

  if (uploadResult == 2)
    reply += F("<font color=\"red\">Upload file invalid!</font>");

  if (uploadResult == 3)
    reply += F("<font color=\"red\">No filename!</font>");

  addHeader(true, reply);
  reply += F("Upload finished");
  addFooter(reply);
  sendWebPage(F("TmplStd"), reply);
  printWebString = "";
  printToWeb = false;
}


//********************************************************************************
// Web Interface upload handler
//********************************************************************************
fs::File uploadFile;
void handleFileUpload() {
  if (!isLoggedIn()) return;

  static boolean valid = false;
  String log = "";

  HTTPUpload& upload = WebServer.upload();

  if (upload.filename.c_str()[0] == 0)
  {
    uploadResult = 3;
    return;
  }

  if (upload.status == UPLOAD_FILE_START)
  {
    log = F("Upload: START, filename: ");
    log += upload.filename;
    addLog(LOG_LEVEL_INFO, log);
    valid = false;
    uploadResult = 0;
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    // first data block, if this is the config file, check PID/Version
    if (upload.totalSize == 0)
    {
      if (strcasecmp(upload.filename.c_str(), "config.dat") == 0)
      {
        struct TempStruct {
          unsigned long PID;
          int Version;
        } Temp;
        for (int x = 0; x < sizeof(struct TempStruct); x++)
        {
          byte b = upload.buf[x];
          memcpy((byte*)&Temp + x, &b, 1);
        }
        if (Temp.Version == VERSION && Temp.PID == ESP_PROJECT_PID)
          valid = true;
      }
      else
      {
        // other files are always valid...
        valid = true;
      }
      if (valid)
      {
        // once we're safe, remove file and create empty one...
        SPIFFS.remove((char *)upload.filename.c_str());
        uploadFile = SPIFFS.open(upload.filename.c_str(), "w");
        flashCount();
      }
    }
    if (uploadFile) uploadFile.write(upload.buf, upload.currentSize);
    log = F("Upload: WRITE, Bytes: ");
    log += upload.currentSize;
    addLog(LOG_LEVEL_INFO, log);
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (uploadFile) uploadFile.close();
    log = F("Upload: END, Size: ");
    log += upload.totalSize;
    addLog(LOG_LEVEL_INFO, log);
  }

  if (valid)
    uploadResult = 1;
  else
    uploadResult = 2;

}


//********************************************************************************
// Web Interface server web file from SPIFFS
//********************************************************************************
bool loadFromFS(boolean spiffs, String path) {
  if (!isLoggedIn()) return false;

  String dataType = F("text/plain");
  if (path.endsWith("/")) path += F("index.htm");

  if (path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".htm")) dataType = F("text/html");
  else if (path.endsWith(".css")) dataType = F("text/css");
  else if (path.endsWith(".js")) dataType = F("application/javascript");
  else if (path.endsWith(".png")) dataType = F("image/png");
  else if (path.endsWith(".gif")) dataType = F("image/gif");
  else if (path.endsWith(".jpg")) dataType = F("image/jpeg");
  else if (path.endsWith(".ico")) dataType = F("image/x-icon");
  else if (path.endsWith(".txt")) dataType = F("application/octet-stream");
  else if (path.endsWith(".dat")) dataType = F("application/octet-stream");

  path = path.substring(1);
  if (spiffs)
  {
    fs::File dataFile = SPIFFS.open(path.c_str(), "r");
    if (!dataFile)
      return false;
    if (path.endsWith(".dat"))
      WebServer.sendHeader("Content-Disposition", "attachment;");
    WebServer.streamFile(dataFile, dataType);
    dataFile.close();
  }
  else
  {
    File dataFile = SD.open(path.c_str());
    if (!dataFile)
      return false;
    if (path.endsWith(".DAT"))
      WebServer.sendHeader("Content-Disposition", "attachment;");
    WebServer.streamFile(dataFile, dataType);
    dataFile.close();
  }
  return true;
}


//********************************************************************************
// Web Interface file list
//********************************************************************************
void handle_filelist() {

  navMenuIndex = 7;
  String fdelete = WebServer.arg(F("delete"));

  if (fdelete.length() > 0)
  {
    SPIFFS.remove(fdelete);
    flashCount();
  }

  String reply = "";
  addHeader(true, reply);
  reply += F("<table border=1px frame='box' rules='all'><TH><TH>Filename:<TH>Size");

  fs::Dir dir = SPIFFS.openDir("");
  while (dir.next())
  {
    reply += F("<TR><TD>");
    if (dir.fileName() != "config.dat" && dir.fileName() != "security.dat" && dir.fileName() != "notification.dat")
    {
      reply += F("<a class='button link' href=\"filelist?delete=");
      reply += dir.fileName();
      reply += F("\">Del</a>");
    }

    reply += F("<TD><a href=\"");
    reply += dir.fileName();
    reply += F("\">");
    reply += dir.fileName();
    reply += F("</a>");
    fs::File f = dir.openFile("r");
    reply += F("<TD>");
    reply += f.size();
  }
  reply += F("</table></form>");
  reply += F("<BR><a class='button link' href=\"/upload\">Upload</a>");
  addFooter(reply);
  sendWebPage(F("TmplStd"), reply);
}


//********************************************************************************
// Web Interface SD card file list
//********************************************************************************
void handle_SDfilelist() {

  navMenuIndex = 7;
  String fdelete = WebServer.arg(F("delete"));

  if (fdelete.length() > 0)
  {
    SD.remove((char*)fdelete.c_str());
  }

  String reply = "";
  addHeader(true, reply);
  reply += F("<table border=1px frame='box' rules='all'><TH><TH>Filename:<TH>Size");

  File root = SD.open("/");
  root.rewindDirectory();
  File entry = root.openNextFile();
  while (entry)
  {
    if (!entry.isDirectory())
    {
      reply += F("<TR><TD>");
      if (entry.name() != "config.dat" && entry.name() != "security.dat")
      {
        reply += F("<a class='button link' href=\"SDfilelist?delete=");
        reply += entry.name();
        reply += F("\">Del</a>");
      }
      reply += F("<TD><a href=\"");
      reply += entry.name();
      reply += F("\">");
      reply += entry.name();
      reply += F("</a>");
      reply += F("<TD>");
      reply += entry.size();
    }
    entry.close();
    entry = root.openNextFile();
  }
  //entry.close();
  root.close();
  reply += F("</table></form>");
  //reply += F("<BR><a class='button link' href=\"/upload\">Upload</a>");
  addFooter(reply);
  sendWebPage(F("TmplStd"), reply);
}


//********************************************************************************
// Web Interface handle other requests
//********************************************************************************
void handleNotFound() {

  if (wifiSetup)
  {
    WebServer.send(200, "text/html", "<meta HTTP-EQUIV='REFRESH' content='0; url=/setup'>");
    return;
  }

  if (!isLoggedIn()) return;
  if (loadFromFS(true, WebServer.uri())) return;
  if (loadFromFS(false, WebServer.uri())) return;
  String message = "URI: ";
  message += WebServer.uri();
  message += "\nMethod: ";
  message += (WebServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += WebServer.args();
  message += "\n";
  for (uint8_t i = 0; i < WebServer.args(); i++) {
    message += " NAME:" + WebServer.argName(i) + "\n VALUE:" + WebServer.arg(i) + "\n";
  }
  WebServer.send(404, "text/plain", message);
}


//********************************************************************************
// Web Interface Setup Wizard
//********************************************************************************
void handle_setup() {

  String reply = "";
  addHeader(false, reply);

  if (WiFi.status() == WL_CONNECTED)
  {
    SaveSettings();
    IPAddress ip = WiFi.localIP();
    char host[20];
    sprintf_P(host, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
    reply += F("<BR>ESP is connected and using IP Address: <BR><h1>");
    reply += host;
    reply += F("</h1><BR><BR>Connect your laptop / tablet / phone<BR>back to your main Wifi network and<BR><BR>");
    reply += F("<a class='button' href='http://");
    reply += host;
    reply += F("/config'>Proceed to main config</a>");
    addFooter(reply);
    sendWebPage(F("TmplAP"), reply);

    wifiSetup = false;
    //WifiAPMode(false);  //this forces the iPhone to exit safari and this page was never displayed
    timerAPoff = millis() + 60000L;  //switch the AP off in 1 minute
    return;
  }

  static byte status = 0;
  static int n = 0;
  static byte refreshCount = 0;
  String ssid = WebServer.arg(F("ssid"));
  String other = WebServer.arg(F("other"));
  String password = WebServer.arg(F("pass"));

  if (other.length() != 0)
  {
    ssid = other;
  }

  // if ssid config not set and params are both provided
  if (status == 0 && ssid.length() != 0 && strcasecmp(SecuritySettings.WifiSSID, "ssid") == 0)
  {
    strncpy(SecuritySettings.WifiKey, password.c_str(), sizeof(SecuritySettings.WifiKey));
    strncpy(SecuritySettings.WifiSSID, ssid.c_str(), sizeof(SecuritySettings.WifiSSID));
    wifiSetupConnect = true;
    status = 1;
    refreshCount = 0;
  }

  reply += F("<h1>Wifi Setup wizard</h1><BR>");
  reply += F("<form name='frmselect' method='post'>");

  if (status == 0)  // first step, scan and show access points within reach...
  {
    if (n == 0)
      n = WiFi.scanNetworks();

    if (n == 0)
      reply += F("No Access Points found");
    else
    {
      for (int i = 0; i < n; ++i)
      {
        reply += F("<input type='radio' name='ssid' value='");
        reply += WiFi.SSID(i);
        reply += F("'");
        if (WiFi.SSID(i) == ssid)
          reply += F(" checked ");
        reply += F(">");
        reply += WiFi.SSID(i);
        reply += F("</input><br>");
      }
    }

    reply += F("<input type='radio' name='ssid' id='other_ssid' value='other' >other SSID:</input>");
    reply += F("<input type ='text' name='other' value='");
    reply += other;
    reply += F("'><br><br>");
    reply += F("Password: <input type ='text' name='pass' value='");
    reply += password;
    reply += F("'><br>");

    reply += F("<input type='submit' value='Connect'>");
  }

  if (status == 1)  // connecting stage...
  {
    if (refreshCount > 0)
    {
      status = 0;
      strncpy(SecuritySettings.WifiSSID, "ssid", sizeof(SecuritySettings.WifiSSID));
      SecuritySettings.WifiKey[0] = 0;
      reply += F("<a class=\"button\" href=\"setup\">Back to Setup</a>");
    }
    else
    {
      int wait = 20;
      if (refreshCount != 0)
        wait = 3;
      reply += F("Please wait for <h1 id=\"countdown\">20..</h1>");
      reply += F("<script type=\"text/JavaScript\">");
      reply += F("function timedRefresh(timeoutPeriod) {");
      reply += F("   var timer = setInterval(function() {");
      reply += F("   if (timeoutPeriod > 0) {");
      reply += F("       timeoutPeriod -= 1;");
      reply += F("       document.getElementById(\"countdown\").innerHTML = timeoutPeriod + \"..\" + \"<br />\";");
      reply += F("   } else {");
      reply += F("       clearInterval(timer);");
      reply += F("            window.location.href = window.location.href;");
      reply += F("       };");
      reply += F("   }, 1000);");
      reply += F("};");
      reply += F("timedRefresh(");
      reply += wait;
      reply += F(");");
      reply += F("</script>");
      reply += F("seconds while trying to connect");
    }
    refreshCount++;
  }

  reply += F("</form>");
  addFooter(reply);
  sendWebPage(F("TmplAP"), reply);
  delay(10);
}


//********************************************************************************
// Web Interface rules page
//********************************************************************************
void handle_rules() {
  if (!isLoggedIn()) return;
  static byte currentSet = 1;

  navMenuIndex = 5;
  String set = WebServer.arg(F("set"));
  byte rulesSet = 1;
  if (set.length() > 0)
  {
    rulesSet = set.toInt();
  }

  String fileName = F("rules");
  fileName += rulesSet;
  fileName += F(".txt");

  String reply = "";
  checkRAM(8);
  addHeader(true, reply);

  if (WebServer.args() > 0)
  {
    if (currentSet == rulesSet) // only save when the dropbox was not used to change set
    {
      String rules = WebServer.arg(F("rules"));
      if (rules.length() > RULES_MAX_SIZE)
        reply += F("<span style=\"color:red\">Data was not saved, exceeds web editor limit!</span>");
      else
      {

        if (RTC.flashDayCounter > MAX_FLASHWRITES_PER_DAY)
        {
          String log = F("FS   : Daily flash write rate exceeded! (powercyle to reset this)");
          addLog(LOG_LEVEL_ERROR, log);
          reply += F("<span style=\"color:red\">Error saving to flash!</span>");
        }
        else
        {
          fs::File f = SPIFFS.open(fileName, "w");
          if (f)
          {
            f.print(rules);
            f.close();
            flashCount();
          }
        }
      }
    }
    else // changed set, check if file exists and create new
    {
      if (!SPIFFS.exists(fileName))
      {
        fs::File f = SPIFFS.open(fileName, "w");
        f.close();
      }
    }
  }

  if (rulesSet != currentSet)
    currentSet = rulesSet;

  reply += F("<form name = 'frmselect' method = 'post'><table><TR><TH>Rules");

  byte choice = rulesSet;
  String options[RULESETS_MAX];
  int optionValues[RULESETS_MAX];
  for (byte x = 0; x < RULESETS_MAX; x++)
  {
    options[x] = F("Rules Set ");
    options[x] += x + 1;
    optionValues[x] = x + 1;
  }

  reply += F("<TR><TD>Edit: ");
  addSelector(reply, F("set"), RULESETS_MAX, options, optionValues, NULL, choice, true);
  addHelpButton(reply, F("Tutorial_Rules"));

  // load form data from flash

  int size = 0;
  fs::File f = SPIFFS.open(fileName, "r+");
  if (f)
  {
    size = f.size();
    if (size > RULES_MAX_SIZE)
      reply += F("<span style=\"color:red\">Filesize exceeds web editor limit!</span>");
    else
    {
      reply += F("<TR><TD><textarea name='rules' rows='15' cols='80' wrap='off'>");
      while (f.available())
      {
        String c((char)f.read());
        htmlEscape(c);
        reply += c;
      }
      reply += F("</textarea>");
    }
    f.close();
  }

  reply += F("<TR><TD>Current size: ");
  reply += size;
  reply += F(" characters (Max ");
  reply += RULES_MAX_SIZE;
  reply += F(")");

  addFormSeparator(reply);

  reply += F("<TR><TD>");
  addSubmitButton(reply);
  reply += F("</table></form>");
  addFooter(reply);
  sendWebPage(F("TmplStd"), reply);
}


//********************************************************************************
// Web Interface root page
//********************************************************************************
void handle_sysinfo() {
  if (!isLoggedIn()) return;

  int freeMem = ESP.getFreeHeap();
  String reply = "";
  addHeader(true, reply);

  IPAddress ip = WiFi.localIP();
  IPAddress gw = WiFi.gatewayIP();

  reply += printWebString;
  reply += F("<form>");
  reply += F("<table><TR><TH>System Info<TH>");

  reply += F("<TR><TD>Unit:<TD>");
  reply += Settings.Unit;

  if (Settings.UseNTP)
  {

    reply += F("<TR><TD>Local Time:<TD>");
  	reply += getDateTimeString('-', ':', ' ');
  }

  reply += F("<TR><TD>Uptime:<TD>");
  char strUpTime[40];
  int minutes = wdcounter / 2;
  int days = minutes / 1440;
  minutes = minutes % 1440;
  int hrs = minutes / 60;
  minutes = minutes % 60;
  sprintf_P(strUpTime, PSTR("%d days %d hours %d minutes"), days, hrs, minutes);
  reply += strUpTime;

  reply += F("<TR><TD>Load:<TD>");
  if (wdcounter > 0)
  {
    reply += 100 - (100 * loopCounterLast / loopCounterMax);
    reply += F("% (LC=");
    reply += int(loopCounterLast / 30);
    reply += F(")");
  }

  reply += F("<TR><TD>Free Mem:<TD>");
  reply += freeMem;
  reply += F(" (");
  reply += lowestRAM;
  reply += F(")");

  if (WiFi.status() == WL_CONNECTED)
  {
    reply += F("<TR><TD>Wifi RSSI:<TD>");
    reply += WiFi.RSSI();
    reply += F(" dB");
    reply += F("<TR><TD>Wifi Type:<TD>");
    byte PHYmode = wifi_get_phy_mode();
    switch(PHYmode)
    {
      case 1:
        reply += F("802.11B");
        break;
      case 2:
        reply += F("802.11G");
        break;
      case 3:
        reply += F("802.11N");
        break;
    }
  }

  char str[20];
  sprintf_P(str, PSTR("%u.%u.%u.%u"), ip[0], ip[1], ip[2], ip[3]);
  reply += F("<TR><TD>IP:<TD>");
  reply += str;

  sprintf_P(str, PSTR("%u.%u.%u.%u"), gw[0], gw[1], gw[2], gw[3]);
  reply += F("<TR><TD>GW:<TD>");
  reply += str;

  reply += F("<TR><TD>Build:<TD>");
  reply += BUILD;
  reply += F(" ");
  reply += F(BUILD_NOTES);

  reply += F("<TR><TD>GIT version:<TD>");
  reply += BUILD_GIT;

  reply += F("<TR><TD>Plugin sets:<TD>");

  #ifdef PLUGIN_BUILD_NORMAL
    reply += F("[Normal] ");
  #endif

  #ifdef PLUGIN_BUILD_TESTING
    reply += F("[Testing] ");
  #endif

  #ifdef PLUGIN_BUILD_DEV
    reply += F("[Development] ");
  #endif

  reply += F("<TR><TD>Core Version:<TD>");
  reply += ESP.getCoreVersion();

  reply += F("<TR><TD>Flash Size:<TD>");
  reply += ESP.getFlashChipRealSize() / 1024; //ESP.getFlashChipSize();
  reply += F(" kB");

  reply += F("<TR><TD>Flash Writes (daily/boot):<TD>");
  reply += RTC.flashDayCounter;
  reply += F(" / ");
  reply += RTC.flashCounter;

  reply += F("<TR><TD>Sketch Size/Free:<TD>");
  reply += ESP.getSketchSize() / 1024;
  reply += F(" kB / ");
  reply += ESP.getFreeSketchSpace() / 1024;
  reply += F(" kB");

  reply += F("<TR><TD>Devices:<TD>");
  reply += deviceCount + 1;

  reply += F("<TR><TD>Boot cause:<TD>");
  switch (lastBootCause)
  {
    case BOOT_CAUSE_MANUAL_REBOOT:
      reply += F("Manual reboot");
      break;
    case BOOT_CAUSE_DEEP_SLEEP: //nobody should ever see this, since it should sleep again right away.
      reply += F("Deep sleep");
      break;
    case BOOT_CAUSE_COLD_BOOT:
      reply += F("Cold boot");
      break;
    case BOOT_CAUSE_EXT_WD:
      reply += F("External Watchdog");
      break;
  }

  reply += F("<TR><TD>Warm boot count:<TD>");
  reply += RTC.bootCounter;

  reply += F("<TR><TD>STA MAC:<TD>");
  uint8_t mac[] = {0, 0, 0, 0, 0, 0};
  uint8_t* macread = WiFi.macAddress(mac);
  char macaddress[20];
  sprintf_P(macaddress, PSTR("%02x:%02x:%02x:%02x:%02x:%02x"), macread[0], macread[1], macread[2], macread[3], macread[4], macread[5]);
  reply += macaddress;

  reply += F("<TR><TD>AP MAC:<TD>");
  macread = WiFi.softAPmacAddress(mac);
  sprintf_P(macaddress, PSTR("%02x:%02x:%02x:%02x:%02x:%02x"), macread[0], macread[1], macread[2], macread[3], macread[4], macread[5]);
  reply += macaddress;

  reply += F("<TR><TD>ESP Chip ID:<TD>");
  reply += ESP.getChipId();

  reply += F("<TR><TD>Flash Chip ID:<TD>");
  reply += ESP.getFlashChipId();

  reply += F("</table></form>");
  addFooter(reply);
  sendWebPage(F("TmplStd"), reply);
}


//********************************************************************************
// URNEncode char string to string object
//********************************************************************************
String URLEncode(const char* msg)
{
  const char *hex = "0123456789abcdef";
  String encodedMsg = "";

  while (*msg != '\0') {
    if ( ('a' <= *msg && *msg <= 'z')
         || ('A' <= *msg && *msg <= 'Z')
         || ('0' <= *msg && *msg <= '9') ) {
      encodedMsg += *msg;
    } else {
      encodedMsg += '%';
      encodedMsg += hex[*msg >> 4];
      encodedMsg += hex[*msg & 15];
    }
    msg++;
  }
  return encodedMsg;
}


String getControllerSymbol(byte index)
{
  String ret = F("&#");
  ret += 10102 + index;
  ret += F(";");
  return ret;
}

String getValueSymbol(byte index)
{
  String ret = F("&#");
  ret += 10112 + index;
  ret += F(";");
  return ret;
}
