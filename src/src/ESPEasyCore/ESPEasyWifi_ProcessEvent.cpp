#include "../ESPEasyCore/ESPEasyWifi_ProcessEvent.h"

// FIXME TD-er: Rename this to ESPEasyNetwork_ProcessEvent

#include "../../ESPEasy-Globals.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/ESPEasyWiFiEvent.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/ESPEasy_Scheduler.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/EventQueue.h"
#include "../Globals/MQTT.h"
#include "../Globals/NetworkState.h"
#include "../Globals/RTC.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Settings.h"
#include "../Globals/Services.h"
#include "../Globals/WiFi_AP_Candidates.h"
#include "../Helpers/ESPEasyRTC.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Network.h"
#include "../Helpers/Networking.h"
#include "../Helpers/PeriodicalActions.h"
#include "../Helpers/Scheduler.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_WiFi.h"


// ********************************************************************************
// Called from the loop() to make sure events are processed as soon as possible.
// These functions are called from Setup() or Loop() and thus may call delay() or yield()
// ********************************************************************************
void handle_unprocessedNetworkEvents()
{
#ifdef HAS_ETHERNET
  if (EthEventData.unprocessedEthEvents()) {
    // Process disconnect events before connect events.
    if (!EthEventData.processedDisconnect) {
      #ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("Eth  : Entering processDisconnect()"));
      #endif // ifndef BUILD_NO_DEBUG
      processEthernetDisconnected();
    }

    // Must process the Ethernet Connected event regardless the active network medium.
    // It may happen by plugging in the cable while WiFi was active.
    if (!EthEventData.processedConnect) {
      #ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("Eth  : Entering processConnect()"));
      #endif // ifndef BUILD_NO_DEBUG
      processEthernetConnected();
    }
  }

  if (active_network_medium == NetworkMedium_t::Ethernet) {
    if (!EthEventData.EthServicesInitialized() || EthEventData.unprocessedEthEvents())
    {
      if (!EthEventData.unprocessedEthEvents() && EthEventData.EthConnectAllowed()) {
        NetworkConnectRelaxed();
      }
 
      if (!EthEventData.processedGotIP) {
        #ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("Eth  : Entering processGotIP()"));
        #endif // ifndef BUILD_NO_DEBUG
        processEthernetGotIP();
      }

      if (!EthEventData.processedDHCPTimeout) {
        #ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("Eth  : DHCP timeout, Calling disconnect()"));
        #endif // ifndef BUILD_NO_DEBUG
        EthEventData.processedDHCPTimeout = true;
        //WifiDisconnect();
      }
    }
    EthEventData.setEthServicesInitialized();
  }

#endif
  if (WiFiEventData.unprocessedWifiEvents()) {
    // Process disconnect events before connect events.
    if (!WiFiEventData.processedDisconnect) {
      #ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, F("WIFI : Entering processDisconnect()"));
      #endif // ifndef BUILD_NO_DEBUG
      processDisconnect();
    }
  }

  if (active_network_medium == NetworkMedium_t::WIFI) {
    if ((!WiFiEventData.WiFiServicesInitialized()) || WiFiEventData.unprocessedWifiEvents()) {
      if (WiFi.status() == WL_DISCONNECTED && WiFiEventData.wifiConnectInProgress) {
        if (WiFiEventData.wifiConnectInProgress) {
          if (WiFiEventData.last_wifi_connect_attempt_moment.isSet() && WiFiEventData.last_wifi_connect_attempt_moment.millisPassedSince() > 20000) {
            WiFiEventData.last_wifi_connect_attempt_moment.clear();
          }
          if (!WiFiEventData.last_wifi_connect_attempt_moment.isSet()) {
            WiFiEventData.wifiConnectInProgress = false;
          }
        }
        delay(10);
      }

      // WiFi connection is not yet available, so introduce some extra delays to
      // help the background tasks managing wifi connections
      delay(0);

      NetworkConnectRelaxed();

      if (!WiFiEventData.processedConnect) {
        #ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("WIFI : Entering processConnect()"));
        #endif // ifndef BUILD_NO_DEBUG
        processConnect();
      }

      if (!WiFiEventData.processedGotIP) {
        #ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("WIFI : Entering processGotIP()"));
        #endif // ifndef BUILD_NO_DEBUG
        processGotIP();
      }

      if (!WiFiEventData.processedDHCPTimeout) {
        #ifndef BUILD_NO_DEBUG
        addLog(LOG_LEVEL_DEBUG, F("WIFI : DHCP timeout, Calling disconnect()"));
        #endif // ifndef BUILD_NO_DEBUG
        WiFiEventData.processedDHCPTimeout = true;
        WifiDisconnect();
      }
    }
    const bool should_be_initialized = (WiFiEventData.WiFiGotIP() && WiFiEventData.WiFiConnected()) || NetworkConnected();
    if (WiFiEventData.WiFiServicesInitialized() != should_be_initialized)
    {
      if (!WiFiEventData.WiFiServicesInitialized()) {
        WiFiEventData.processedDHCPTimeout  = true;  // FIXME TD-er:  Find out when this happens  (happens on ESP32 sometimes)
        WiFiEventData.setWiFiServicesInitialized();
        CheckRunningServices();
      }
    }

    if (WiFiEventData.WiFiDisconnected()) {
      #ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        static LongTermTimer lastDisconnectMoment_log;
        static uint8_t lastWiFiStatus_log = 0;
        uint8_t cur_wifi_status = WiFi.status();
        if (WiFiEventData.lastDisconnectMoment.get() != lastDisconnectMoment_log.get() || 
            lastWiFiStatus_log != cur_wifi_status) {
          lastDisconnectMoment_log.set(WiFiEventData.lastDisconnectMoment.get());
          lastWiFiStatus_log = cur_wifi_status;
          String wifilog = F("WIFI : Disconnected: WiFi.status() = ");
          wifilog += ESPeasyWifiStatusToString();
          wifilog += F(" RSSI: ");
          wifilog += String(WiFi.RSSI());
          wifilog += F(" status: ");
          #ifdef ESP8266
          station_status_t status = wifi_station_get_connect_status();
          wifilog += SDKwifiStatusToString(status);
          #endif
          #ifdef ESP32
          wifilog += ArduinoWifiStatusToString(WiFi.status());
          #endif
          addLog(LOG_LEVEL_DEBUG, wifilog);
        }
      }
      #endif // ifndef BUILD_NO_DEBUG

      // While connecting to WiFi make sure the device has ample time to do so
      delay(10);
    }

    if (!WiFiEventData.processedDisconnectAPmode) { processDisconnectAPmode(); }

    if (!WiFiEventData.processedConnectAPmode) { processConnectAPmode(); }

    if (WiFiEventData.timerAPoff.isSet()) { processDisableAPmode(); }

    if (!WiFiEventData.processedScanDone) { processScanDone(); }

    if (WiFiEventData.wifi_connect_attempt > 0) {
      // We only want to clear this counter if the connection is currently stable.
      if (WiFiEventData.WiFiServicesInitialized()) {
        if (WiFiEventData.lastConnectMoment.isSet() && WiFiEventData.lastConnectMoment.timeoutReached(WIFI_CONNECTION_CONSIDERED_STABLE)) {
          // Connection considered stable
          WiFiEventData.wifi_connect_attempt = 0;
          WiFiEventData.wifi_considered_stable = true;
          WiFi_AP_Candidates.markCurrentConnectionStable();

          if (!WiFi.getAutoConnect()) {
            WiFi.setAutoConnect(true);
          }
        } else {
          if (WiFi.getAutoConnect()) {
            WiFi.setAutoConnect(false);
          }
        }
      }
    }
  }
  updateUDPport();
}

// ********************************************************************************
// Functions to process the data gathered from the events.
// These functions are called from Setup() or Loop() and thus may call delay() or yield()
// ********************************************************************************
void processDisconnect() {
  if (WiFiEventData.processingDisconnect.isSet()) {
    if (WiFiEventData.processingDisconnect.millisPassedSince() > 5000 || WiFiEventData.processedDisconnect) {
      WiFiEventData.processingDisconnect.clear();
    }
  }


  if (WiFiEventData.processedDisconnect || 
      WiFiEventData.processingDisconnect.isSet()) { return; }
  WiFiEventData.processingDisconnect.setNow();
  WiFiEventData.setWiFiDisconnected();
  WiFiEventData.wifiConnectAttemptNeeded = true;
  delay(100); // FIXME TD-er: See https://github.com/letscontrolit/ESPEasy/issues/1987#issuecomment-451644424

  if (Settings.UseRules) {
    eventQueue.add(F("WiFi#Disconnected"));
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("WIFI : Disconnected! Reason: '");
    log += getLastDisconnectReason();
    log += '\'';

    if (WiFiEventData.lastConnectedDuration_us > 0) {
      log += F(" Connected for ");
      log += format_msec_duration(WiFiEventData.lastConnectedDuration_us / 1000ll);
    } else {
      log += F(" Connected for a long time...");
    }
    addLog(LOG_LEVEL_INFO, log);
  }


  bool mustRestartWiFi = Settings.WiFiRestart_connection_lost();
  if (WiFiEventData.lastConnectedDuration_us > 0 && (WiFiEventData.lastConnectedDuration_us / 1000) < 5000) {
    mustRestartWiFi = true;
  }

  if (mustRestartWiFi) {
    WifiDisconnect(); // Needed or else node may not reconnect reliably.
    delay(100);
    setWifiMode(WIFI_OFF);
    initWiFi();
    delay(100);
    if (WiFiEventData.unprocessedWifiEvents()) {
      handle_unprocessedNetworkEvents();
    }

    WifiScan(false);
  }
  logConnectionStatus();
  WiFiEventData.processedDisconnect = true;
  WiFiEventData.processingDisconnect.clear();
}

void processConnect() {
  if (WiFiEventData.processedConnect) { return; }
  //delay(100); // FIXME TD-er: See https://github.com/letscontrolit/ESPEasy/issues/1987#issuecomment-451644424
  if (checkAndResetWiFi()) {
    return;
  }
  WiFiEventData.processedConnect = true;
  WiFiEventData.setWiFiConnected();
  ++WiFiEventData.wifi_reconnects;

  if (WiFi_AP_Candidates.getCurrent().isEmergencyFallback) {
    #ifdef CUSTOM_EMERGENCY_FALLBACK_RESET_CREDENTIALS
    const bool mustResetCredentials = CUSTOM_EMERGENCY_FALLBACK_RESET_CREDENTIALS;
    #else
    const bool mustResetCredentials = false;
    #endif
    #ifdef CUSTOM_EMERGENCY_FALLBACK_START_AP
    const bool mustStartAP = CUSTOM_EMERGENCY_FALLBACK_START_AP;
    #else
    const bool mustStartAP = false;
    #endif
    if (mustStartAP) {
      int allowedUptimeMinutes = 10;
      #ifdef CUSTOM_EMERGENCY_FALLBACK_ALLOW_MINUTES_UPTIME
      allowedUptimeMinutes = CUSTOM_EMERGENCY_FALLBACK_ALLOW_MINUTES_UPTIME;
      #endif
      if (getUptimeMinutes() < allowedUptimeMinutes) {
        WiFiEventData.timerAPstart.setNow();
      }
    }
    if (mustResetCredentials && !WiFiEventData.performedClearWiFiCredentials) {
      WiFiEventData.performedClearWiFiCredentials = true;
      SecuritySettings.clearWiFiCredentials();
      SaveSecuritySettings();
      WiFiEventData.markDisconnect(WIFI_DISCONNECT_REASON_AUTH_EXPIRE);
      WiFi_AP_Candidates.force_reload();
    }
  }


  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    const LongTermTimer::Duration connect_duration = WiFiEventData.last_wifi_connect_attempt_moment.timeDiff(WiFiEventData.lastConnectMoment);
    String log = F("WIFI : Connected! AP: ");
    log += WiFi.SSID();
    log += " (";
    log += WiFi.BSSIDstr();
    log += F(") Ch: ");
    log += RTC.lastWiFiChannel;

    if ((connect_duration > 0ll) && (connect_duration < 30000000ll)) {
      // Just log times when they make sense.
      log += F(" Duration: ");
      log += String(static_cast<int32_t>(connect_duration / 1000));
      log += F(" ms");
    }
    addLog(LOG_LEVEL_INFO, log);
  }

  if (Settings.UseRules) {
    if (WiFiEventData.bssid_changed) {
      eventQueue.add(F("WiFi#ChangedAccesspoint"));
    }

    if (WiFiEventData.channel_changed) {
      eventQueue.add(F("WiFi#ChangedWiFichannel"));
    }
  } 

  if (useStaticIP()) {
    WiFiEventData.markGotIP(); // in static IP config the got IP event is never fired.
  }
  saveToRTC();

  logConnectionStatus();
}

void processGotIP() {
  if (WiFiEventData.processedGotIP) {
    return;
  }
  if (checkAndResetWiFi()) {
    return;
  }

  IPAddress ip = NetworkLocalIP();

  if (!useStaticIP()) {
    #ifdef ESP8266
    if (!ip.isSet()) {
    #else
    if (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0) {
    #endif
      return;
    }
  }
  const IPAddress gw       = NetworkGatewayIP();
  const IPAddress subnet   = NetworkSubnetMask();
  const LongTermTimer::Duration dhcp_duration = WiFiEventData.lastConnectMoment.timeDiff(WiFiEventData.lastGetIPmoment);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("WIFI : ");

    if (useStaticIP()) {
      log += F("Static IP: ");
    } else {
      log += F("DHCP IP: ");
    }
    log += formatIP(ip);
    log += " (";
    log += NetworkGetHostname();
    log += F(") GW: ");
    log += formatIP(gw);
    log += F(" SN: ");
    log += formatIP(subnet);

    if ((dhcp_duration > 0ll) && (dhcp_duration < 30000000ll)) {
      // Just log times when they make sense.
      log += F("   duration: ");
      log += static_cast<int32_t>(dhcp_duration / 1000);
      log += F(" ms");
    }
    addLog(LOG_LEVEL_INFO, log);
  }

  // Might not work in core 2.5.0
  // See https://github.com/esp8266/Arduino/issues/5839
  if ((Settings.IP_Octet != 0) && (Settings.IP_Octet != 255))
  {
    ip[3] = Settings.IP_Octet;

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("IP   : Fixed IP octet:");
      log += formatIP(ip);
      addLog(LOG_LEVEL_INFO, log);
    }
    WiFi.config(ip, gw, subnet, NetworkDnsIP(0), NetworkDnsIP(1));
  }

  // First try to get the time, since that may be used in logs
  if (node_time.systemTimePresent()) {
    node_time.initTime();
  }
#ifdef USES_MQTT
  mqtt_reconnect_count        = 0;
  MQTTclient_should_reconnect = true;
  timermqtt_interval          = 100;
  Scheduler.setIntervalTimer(ESPEasy_Scheduler::IntervalTimer_e::TIMER_MQTT);
  scheduleNextMQTTdelayQueue();
#endif // USES_MQTT
  Scheduler.sendGratuitousARP_now();

  if (Settings.UseRules)
  {
    eventQueue.add(F("WiFi#Connected"));
  }
  statusLED(true);

  //  WiFi.scanDelete();

  if (WiFiEventData.wifiSetup) {
    // Wifi setup was active, Apparently these settings work.
    WiFiEventData.wifiSetup = false;
    SaveSecuritySettings();
  }

  if ((WiFiEventData.WiFiConnected() || WiFi.isConnected()) && hasIPaddr()) {
    WiFiEventData.processedGotIP = true;
    WiFiEventData.setWiFiGotIP();
  }
  refreshNodeList();
  logConnectionStatus();
}

// A client disconnected from the AP on this node.
void processDisconnectAPmode() {
  if (WiFiEventData.processedDisconnectAPmode) { return; }
  WiFiEventData.processedDisconnectAPmode = true;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    const int nrStationsConnected = WiFi.softAPgetStationNum();
    String    log                 = F("AP Mode: Client disconnected: ");
    log += WiFiEventData.lastMacDisconnectedAPmode.toString();
    log += F(" Connected devices: ");
    log += nrStationsConnected;
    addLog(LOG_LEVEL_INFO, log);
  }
}

// Client connects to AP on this node
void processConnectAPmode() {
  if (WiFiEventData.processedConnectAPmode) { return; }
  WiFiEventData.processedConnectAPmode = true;
  // Extend timer to switch off AP.
  WiFiEventData.timerAPoff.setMillisFromNow(WIFI_AP_OFF_TIMER_DURATION);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("AP Mode: Client connected: ");
    log += WiFiEventData.lastMacConnectedAPmode.toString();
    log += F(" Connected devices: ");
    log += WiFi.softAPgetStationNum();
    addLog(LOG_LEVEL_INFO, log);
  }

  #ifdef FEATURE_DNS_SERVER
  // Start DNS, only used if the ESP has no valid WiFi config
  // It will reply with it's own address on all DNS requests
  // (captive portal concept)
  if (!dnsServerActive) {
    dnsServerActive = true;
    dnsServer.start(DNS_PORT, "*", apIP);
  }
  #endif
}

// Switch of AP mode when timeout reached and no client connected anymore.
void processDisableAPmode() {
  if (!WiFiEventData.timerAPoff.isSet()) { return; }

  if (WifiIsAP(WiFi.getMode())) {
    // disable AP after timeout and no clients connected.
    if (WiFiEventData.timerAPoff.timeReached() && (WiFi.softAPgetStationNum() == 0)) {
      setAP(false);
    }
  }

  if (!WifiIsAP(WiFi.getMode())) {
    WiFiEventData.timerAPoff.clear();
  }
}

void processScanDone() {
  WiFi_AP_Candidates.load_knownCredentials();
  if (WiFiEventData.processedScanDone) { return; }



  // Better act on the scan done event, as it may get triggered for normal wifi begin calls.
  int8_t scanCompleteStatus = WiFi.scanComplete();
  switch (scanCompleteStatus) {
    case 0: // Nothing (yet) found
      if (WiFiEventData.lastGetScanMoment.timeoutReached(5000)) {
        WiFiEventData.processedScanDone = true;
      }
      return;
    case -1: // WIFI_SCAN_RUNNING
      // FIXME TD-er: Set timeout...
      if (WiFiEventData.lastGetScanMoment.timeoutReached(5000)) {
        addLog(LOG_LEVEL_ERROR, F("WiFi : Scan Running Timeout"));
        WiFiEventData.processedScanDone = true;
      }
      return;
    case -2: // WIFI_SCAN_FAILED
      addLog(LOG_LEVEL_ERROR, F("WiFi : Scan failed"));
      WiFiEventData.processedScanDone = true;
      return;
  }

  WiFiEventData.lastGetScanMoment.setNow();
  WiFiEventData.processedScanDone = true;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("WiFi : Scan finished, found: ");
    log += scanCompleteStatus;
    addLog(LOG_LEVEL_INFO, log);
  }

  WiFi_AP_Candidates.process_WiFiscan(scanCompleteStatus);

  NetworkConnectRelaxed();
}




#ifdef HAS_ETHERNET

void processEthernetConnected() {
  if (EthEventData.processedConnect) return;
  // FIXME TD-er: Must differentiate among reconnects for WiFi and Ethernet.
  ++WiFiEventData.wifi_reconnects;
  EthEventData.setEthConnected();
  EthEventData.processedConnect = true;
  if (Settings.UseRules)
  {
    eventQueue.add(F("Ethernet#LinkUp"));
  }  
  setNetworkMedium(Settings.NetworkMedium);
}

void processEthernetDisconnected() {
  EthEventData.setEthDisconnected();
  EthEventData.processedDisconnect = true;
  EthEventData.ethConnectAttemptNeeded = true;
  if (Settings.UseRules)
  {
    eventQueue.add(F("ETHERNET#Disconnected"));
  }
}

void processEthernetGotIP() {
  if (EthEventData.processedGotIP) {
    return;
  }
  IPAddress ip = NetworkLocalIP();
  const IPAddress gw       = NetworkGatewayIP();
  const IPAddress subnet   = NetworkSubnetMask();
  const LongTermTimer::Duration dhcp_duration = EthEventData.lastConnectMoment.timeDiff(EthEventData.lastGetIPmoment);

  if (loglevelActiveFor(LOG_LEVEL_INFO))
  {
    String log;
    log.reserve(160);
    log = F("ETH MAC: ");
    log += NetworkMacAddress().toString();
    log += ' ';
    if (useStaticIP()) {
      log += F("Static");
    } else {
      log += F("DHCP");
    }
    log += F(" IP: ");
    log += NetworkLocalIP().toString();
    log += F(" (");
    log += NetworkGetHostname();
    log += F(") GW: ");
    log += NetworkGatewayIP().toString();
    log += F(" SN: ");
    log += NetworkSubnetMask().toString();
    if (EthLinkUp()) {
      if (EthFullDuplex()) {
        log += F(" FULL_DUPLEX");
      }
      log += ' ';
      log += EthLinkSpeed();
      log += F("Mbps");
    } else {
      log += F(" Link Down");
    }
    
    if ((dhcp_duration > 0ll) && (dhcp_duration < 30000000ll)) {
      // Just log times when they make sense.
      log += F("   duration: ");
      log += static_cast<int32_t>(dhcp_duration / 1000);
      log += F(" ms");
    }

    addLog(LOG_LEVEL_INFO, log);
  }

  // First try to get the time, since that may be used in logs
  if (node_time.systemTimePresent()) {
    node_time.initTime();
  }
#ifdef USES_MQTT
  mqtt_reconnect_count        = 0;
  MQTTclient_should_reconnect = true;
  timermqtt_interval          = 100;
  Scheduler.setIntervalTimer(ESPEasy_Scheduler::IntervalTimer_e::TIMER_MQTT);
  scheduleNextMQTTdelayQueue();
#endif // USES_MQTT
  Scheduler.sendGratuitousARP_now();

  if (Settings.UseRules)
  {
    eventQueue.add(F("Ethernet#Connected"));
  }
  statusLED(true);
  logConnectionStatus();

  EthEventData.processedGotIP = true;
  EthEventData.setEthGotIP();
  CheckRunningServices();
}

#endif