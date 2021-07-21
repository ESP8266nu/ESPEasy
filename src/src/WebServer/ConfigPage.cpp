#include "../WebServer/ConfigPage.h"

#ifdef WEBSERVER_CONFIG

#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"
#include "../WebServer/WebServer.h"

#include "../ESPEasyCore/Controller.h"

#include "../Globals/MQTT.h"
#include "../Globals/Nodes.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Settings.h"

#include "../Helpers/DeepSleep.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringConverter.h"



#include "src/DataStructs/MAC_address.h"

// ********************************************************************************
// Web Interface config page
// ********************************************************************************
void handle_config() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_config"));
  #endif

  if (!isLoggedIn()) { return; }

  navMenuIndex = MENU_INDEX_CONFIG;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  if (web_server.args() != 0)
  {
    String name = webArg(F("name"));
    name.trim();

    Settings.Delay              = getFormItemInt(F("delay"), Settings.Delay);
    Settings.deepSleep_wakeTime = getFormItemInt(F("awaketime"), Settings.deepSleep_wakeTime);
    Settings.Unit = getFormItemInt(F("unit"), Settings.Unit);

    if (strcmp(Settings.Name, name.c_str()) != 0) {
      addLog(LOG_LEVEL_INFO, F("Unit Name changed."));

      if (CPluginCall(CPlugin::Function::CPLUGIN_GOT_INVALID, 0)) { // inform controllers that the old name will be invalid from now on.
#ifdef USES_MQTT
        MQTTDisconnect();                                           // disconnect form MQTT Server if invalid message was sent succesfull.
#endif // USES_MQTT
      }
#ifdef USES_MQTT
      MQTTclient_should_reconnect = true;
#endif // USES_MQTT
    }

    // Unit name
    safe_strncpy(Settings.Name, name.c_str(), sizeof(Settings.Name));
    Settings.appendUnitToHostname(isFormItemChecked(F("appendunittohostname")));

    // Password
    copyFormPassword(F("password"), SecuritySettings.Password, sizeof(SecuritySettings.Password));

    // SSID 1
    safe_strncpy(SecuritySettings.WifiSSID, webArg(F("ssid")).c_str(), sizeof(SecuritySettings.WifiSSID));
    copyFormPassword(F("key"), SecuritySettings.WifiKey, sizeof(SecuritySettings.WifiKey));

    // SSID 2
    strncpy_webserver_arg(SecuritySettings.WifiSSID2, F("ssid2"));
    copyFormPassword(F("key2"),  SecuritySettings.WifiKey2,  sizeof(SecuritySettings.WifiKey2));

    // Hidden SSID
    Settings.IncludeHiddenSSID(isFormItemChecked(F("hiddenssid")));

    // Access point password.
    copyFormPassword(F("apkey"), SecuritySettings.WifiAPKey, sizeof(SecuritySettings.WifiAPKey));

    // When set you can use the Sensor in AP-Mode without being forced to /setup
    Settings.ApDontForceSetup(isFormItemChecked(F("ApDontForceSetup")));

    // Usually the AP will be started when no WiFi is defined, or the defined one cannot be found. This flag may prevent it.
    Settings.DoNotStartAP(isFormItemChecked(F("DoNotStartAP")));


    // TD-er Read access control from form.
    SecuritySettings.IPblockLevel = getFormItemInt(F("ipblocklevel"));

    switch (SecuritySettings.IPblockLevel) {
      case LOCAL_SUBNET_ALLOWED:
      {
        IPAddress low, high;
        getSubnetRange(low, high);

        for (uint8_t i = 0; i < 4; ++i) {
          SecuritySettings.AllowedIPrangeLow[i]  = low[i];
          SecuritySettings.AllowedIPrangeHigh[i] = high[i];
        }
        break;
      }
      case ONLY_IP_RANGE_ALLOWED:
      case ALL_ALLOWED:

        webArg2ip(F("iprangelow"),  SecuritySettings.AllowedIPrangeLow);
        webArg2ip(F("iprangehigh"), SecuritySettings.AllowedIPrangeHigh);
        break;
    }

    #ifdef USES_ESPEASY_NOW
    for (int peer = 0; peer < ESPEASY_NOW_PEER_MAX; ++peer) {
      String id = F("peer");
      id += String(peer);
      String peer_mac  = web_server.arg(id);
      if (peer_mac.length() == 0) {
        peer_mac = F("00:00:00:00:00:00");
      }
      MAC_address mac;
      if (mac.set(peer_mac.c_str())) {
        mac.get(SecuritySettings.EspEasyNowPeerMAC[peer]);
      }
      /*
      String log = F("MAC decoding ");
      log += peer_mac;
      log += F(" => ");
      log += mac.toString();
      addLog(LOG_LEVEL_INFO, log);
      */
    }
    #endif

    Settings.deepSleepOnFail = isFormItemChecked(F("deepsleeponfail"));
    webArg2ip(F("espip"),      Settings.IP);
    webArg2ip(F("espgateway"), Settings.Gateway);
    webArg2ip(F("espsubnet"),  Settings.Subnet);
    webArg2ip(F("espdns"),     Settings.DNS);
#ifdef HAS_ETHERNET
    webArg2ip(F("espethip"),      Settings.ETH_IP);
    webArg2ip(F("espethgateway"), Settings.ETH_Gateway);
    webArg2ip(F("espethsubnet"),  Settings.ETH_Subnet);
    webArg2ip(F("espethdns"),     Settings.ETH_DNS);
#endif
    addHtmlError(SaveSettings());
  }

  html_add_form();
  html_table_class_normal();

  addFormHeader(F("Main Settings"));

  Settings.Name[25]             = 0;
  SecuritySettings.Password[25] = 0;
  addFormTextBox(F("Unit Name"), F("name"), Settings.Name, 25);
  addFormNumericBox(F("Unit Number"), F("unit"), Settings.Unit, 0, UNIT_NUMBER_MAX);
  addFormCheckBox(F("Append Unit Number to hostname"), F("appendunittohostname"), Settings.appendUnitToHostname());
  addFormPasswordBox(F("Admin Password"), F("password"), SecuritySettings.Password, 25);

  addFormSubHeader(F("Wifi Settings"));

  addFormTextBox(getLabel(LabelType::SSID), F("ssid"), SecuritySettings.WifiSSID, 31);
  addFormPasswordBox(F("WPA Key"), F("key"), SecuritySettings.WifiKey, 63);
  addFormTextBox(F("Fallback SSID"), F("ssid2"), SecuritySettings.WifiSSID2, 31);
  addFormPasswordBox(F("Fallback WPA Key"), F("key2"), SecuritySettings.WifiKey2, 63);

  addFormCheckBox(F("Include Hidden SSID"), F("hiddenssid"), Settings.IncludeHiddenSSID());
  addFormNote(F("Must be checked to connect to a hidden SSID"));

  addFormSeparator(2);
  addFormPasswordBox(F("WPA AP Mode Key"), F("apkey"), SecuritySettings.WifiAPKey, 63);

  addFormCheckBox(F("Don't force /setup in AP-Mode"), F("ApDontForceSetup"), Settings.ApDontForceSetup());
  addFormNote(F("When set you can use the Sensor in AP-Mode without being forced to /setup. /setup can still be called."));

  addFormCheckBox(F("Do Not Start AP"), F("DoNotStartAP"), Settings.DoNotStartAP());
  #ifdef HAS_ETHERNET
  addFormNote(F("Do not allow to start an AP when unable to connect to configured LAN/WiFi"));
  #else
  addFormNote(F("Do not allow to start an AP when configured WiFi cannot be found"));
  #endif


  // TD-er add IP access box F("ipblocklevel")
  addFormSubHeader(F("Client IP filtering"));
  {
    IPAddress low, high;
    getIPallowedRange(low, high);
    uint8_t iplow[4];
    uint8_t iphigh[4];

    for (uint8_t i = 0; i < 4; ++i) {
      iplow[i]  = low[i];
      iphigh[i] = high[i];
    }
    addFormIPaccessControlSelect(F("Client IP block level"), F("ipblocklevel"), SecuritySettings.IPblockLevel);
    addFormIPBox(F("Access IP lower range"), F("iprangelow"),  iplow);
    addFormIPBox(F("Access IP upper range"), F("iprangehigh"), iphigh);
  }

  addFormSubHeader(F("WiFi IP Settings"));

  addFormIPBox(F("ESP WiFi IP"),         F("espip"),      Settings.IP);
  addFormIPBox(F("ESP WiFi Gateway"),    F("espgateway"), Settings.Gateway);
  addFormIPBox(F("ESP WiFi Subnetmask"), F("espsubnet"),  Settings.Subnet);
  addFormIPBox(F("ESP WiFi DNS"),        F("espdns"),     Settings.DNS);
  addFormNote(F("Leave empty for DHCP"));

#ifdef HAS_ETHERNET
  addFormSubHeader(F("Ethernet IP Settings"));

  addFormIPBox(F("ESP Ethernet IP"),         F("espethip"),      Settings.ETH_IP);
  addFormIPBox(F("ESP Ethernet Gateway"),    F("espethgateway"), Settings.ETH_Gateway);
  addFormIPBox(F("ESP Ethernet Subnetmask"), F("espethsubnet"),  Settings.ETH_Subnet);
  addFormIPBox(F("ESP Ethernet DNS"),        F("espethdns"),     Settings.ETH_DNS);
  addFormNote(F("Leave empty for DHCP"));
#endif

#ifdef USES_ESPEASY_NOW
  addFormSubHeader(F("ESPEasy-NOW"));
  for (int peer = 0; peer < ESPEASY_NOW_PEER_MAX; ++peer) {
    String label = F("Peer ");
    label += String(peer + 1);
    String id = F("peer");
    id += String(peer);
    addFormMACBox(label, id, SecuritySettings.EspEasyNowPeerMAC[peer]);

    bool match_STA;
    const NodeStruct* nodeInfo = Nodes.getNodeByMac(SecuritySettings.EspEasyNowPeerMAC[peer], match_STA);
    if (nodeInfo != nullptr)
    {
      String summary = nodeInfo->getSummary();
      summary += match_STA ? F(" (STA)") : F(" (AP)");
      addFormNote(summary);
    }
    
  }
#endif

  addFormSubHeader(F("Sleep Mode"));

  addFormNumericBox(F("Sleep awake time"), F("awaketime"), Settings.deepSleep_wakeTime, 0, 255);
  addUnit(F("sec"));
  addHelpButton(F("SleepMode"));
  addFormNote(F("0 = Sleep Disabled, else time awake from sleep"));

  int dsmax = getDeepSleepMax();
  addFormNumericBox(F("Sleep time"), F("delay"), Settings.Delay, 0, dsmax); // limited by hardware
  {
    String maxSleeptimeUnit = F("sec (max: ");
    maxSleeptimeUnit += String(dsmax);
    maxSleeptimeUnit += ')';
    addUnit(maxSleeptimeUnit);
  }

  addFormCheckBox(F("Sleep on connection failure"), F("deepsleeponfail"), Settings.deepSleepOnFail);

  addFormSeparator(2);

  html_TR_TD();
  html_TD();
  addSubmitButton();
  html_end_table();
  html_end_form();

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

#endif // ifdef WEBSERVER_CONFIG
