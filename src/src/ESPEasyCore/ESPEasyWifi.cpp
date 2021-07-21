#include "../ESPEasyCore/ESPEasyWifi.h"

#include "../../ESPEasy-Globals.h"
#include "../DataStructs/TimingStats.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWiFiEvent.h"
#include "../ESPEasyCore/ESPEasyWifi_ProcessEvent.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/Serial.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/EventQueue.h"
#include "../Globals/NetworkState.h"
#include "../Globals/RTC.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Services.h"
#include "../Globals/Settings.h"
#include "../Globals/WiFi_AP_Candidates.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_WiFi.h"

#ifdef USES_ESPEASY_NOW
# include "../Globals/ESPEasy_now_handler.h"
#endif



// FIXME TD-er: Cleanup of WiFi code
#ifdef ESPEASY_WIFI_CLEANUP_WORK_IN_PROGRESS
bool ESPEasyWiFi_t::begin() {

  return true;
}

void ESPEasyWiFi_t::end() {


}


void ESPEasyWiFi_t::loop() {
  switch (_state) {
    case WiFiState_e::OFF:
    break;
    case WiFiState_e::AP_only:
    break;
    case WiFiState_e::ErrorRecovery:
    // Wait for timeout to expire
    // Start again from scratch
    break;
    case WiFiState_e::STA_Scanning:
    case WiFiState_e::STA_AP_Scanning:
    // Check if scanning is finished
    // When scanning per channel, call for scanning next channel
    break;
    case WiFiState_e::STA_Connecting:
    case WiFiState_e::STA_Reconnecting:
    // Check if (re)connecting has finished
    break;
    case WiFiState_e::STA_Connected:
    // Check if still connected
    // Reconnect if not.
    // Else mark last timestamp seen as connected
    break;
  }


  {
    // Check if we need to start AP
    // Flag captive portal in webserver and/or whether we might be in setup mode
  }

#ifdef USE_IMPROV
  {
    // Check for Improv mode.
  }
#endif


}


IPAddress  ESPEasyWiFi_t::getIP() const {

  IPAddress res;


  return res;
}

void  ESPEasyWiFi_t::disconnect() {

}


void ESPEasyWiFi_t::checkConnectProgress() {

}

void ESPEasyWiFi_t::startScanning() {
  _state = WiFiState_e::STA_Scanning;
  WifiScan(true);
  _last_state_change.setNow();
}


bool ESPEasyWiFi_t::connectSTA() {
  if (!WiFi_AP_Candidates.hasKnownCredentials()) {
    if (!WiFiEventData.warnedNoValidWiFiSettings) {
      addLog(LOG_LEVEL_ERROR, F("WIFI : No valid wifi settings"));
      WiFiEventData.warnedNoValidWiFiSettings = true;
    }
    WiFiEventData.last_wifi_connect_attempt_moment.clear();
    WiFiEventData.wifi_connect_attempt     = 1;
    WiFiEventData.wifiConnectAttemptNeeded = false;

    // No need to wait longer to start AP mode.
    if (!Settings.DoNotStartAP()) {
      setAP(true);
    }
    return false;
  }
  WiFiEventData.warnedNoValidWiFiSettings = false;
  setSTA(true);
  char hostname[40];
  safe_strncpy(hostname, NetworkCreateRFCCompliantHostname().c_str(), sizeof(hostname));
  #if defined(ESP8266)
  wifi_station_set_hostname(hostname);

  #endif // if defined(ESP8266)
  #if defined(ESP32)
  WiFi.setHostname(hostname);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  #endif // if defined(ESP32)
  setConnectionSpeed();
  setupStaticIPconfig();



    // Start the process of connecting or starting AP
    if (WiFi_AP_Candidates.getNext(true)) {
      // Try to connect to AP

    } else {
      // No (known) AP, start scanning
      startScanning();
    }


  return true;
}

#endif // ESPEASY_WIFI_CLEANUP_WORK_IN_PROGRESS


// ********************************************************************************
// WiFi state
// ********************************************************************************

/*
   WiFi STA states:
   1 STA off                 => ESPEASY_WIFI_DISCONNECTED
   2 STA connecting
   3 STA connected           => ESPEASY_WIFI_CONNECTED
   4 STA got IP              => ESPEASY_WIFI_GOT_IP
   5 STA connected && got IP => ESPEASY_WIFI_SERVICES_INITIALIZED

   N.B. the states are flags, meaning both "connected" and "got IP" must be set
        to be considered ESPEASY_WIFI_SERVICES_INITIALIZED

   The flag wifiConnectAttemptNeeded indicates whether a new connect attempt is needed.
   This is set to true when:
   - Security settings have been saved with AP mode enabled. FIXME TD-er, this may not be the best check.
   - WiFi connect timeout reached  &  No client is connected to the AP mode of the node.
   - Wifi is reset
   - WiFi setup page has been loaded with SSID/pass values.


   WiFi AP mode states:
   1 AP on                        => reset AP disable timer
   2 AP client connect/disconnect => reset AP disable timer
   3 AP off                       => AP disable timer = 0;

   AP mode will be disabled when both apply:
   - AP disable timer (timerAPoff) expired
   - No client is connected to the AP.

   AP mode will be enabled when at least one applies:
   - No valid WiFi settings
   - Start AP timer (timerAPstart) expired

   Start AP timer is set or cleared at:
   - Set timerAPstart when "valid WiFi connection" state is observed.
   - Disable timerAPstart when ESPEASY_WIFI_SERVICES_INITIALIZED wifi state is reached.

   For the first attempt to connect after a cold boot (RTC values are 0), a WiFi scan will be 
   performed to find the strongest known SSID.
   This will set RTC.lastBSSID and RTC.lastWiFiChannel
   
   Quick reconnect (using BSSID/channel of last connection) when both apply:
   - If wifi_connect_attempt < 3
   - RTC.lastBSSID is known
   - RTC.lastWiFiChannel != 0

   Change of wifi settings when both apply:
   - "other" settings valid
   - (wifi_connect_attempt % 2) == 0

   Reset of wifi_connect_attempt to 0 when both apply:
   - connection successful
   - Connection stable (connected for > 5 minutes)

 */


// ********************************************************************************
// Check WiFi connected status
// This is basically the state machine to switch between states:
// - Initiate WiFi reconnect
// - Start/stop of AP mode
// ********************************************************************************
bool WiFiConnected() {
  START_TIMER;

  static bool recursiveCall = false;


  if (WiFiEventData.unprocessedWifiEvents()) { return false; }

  bool wifi_isconnected = WiFi.isConnected();
  #ifdef ESP8266
  // Perform check on SDK function, see: https://github.com/esp8266/Arduino/issues/7432
  station_status_t status = wifi_station_get_connect_status();
  switch(status) {
    case STATION_GOT_IP:
      wifi_isconnected = true;
      break;
    case STATION_NO_AP_FOUND:
    case STATION_CONNECT_FAIL:
    case STATION_WRONG_PASSWORD:
      wifi_isconnected = false;
      break;
    case STATION_IDLE:
    case STATION_CONNECTING:
      break;

    default:
      wifi_isconnected = false;
      break;
  }
  #endif

  if (recursiveCall) return wifi_isconnected;
  recursiveCall = true;


  // For ESP82xx, do not rely on WiFi.status() with event based wifi.
  const int32_t wifi_rssi = WiFi.RSSI();
  bool validWiFi = (wifi_rssi < 0) && wifi_isconnected && hasIPaddr();
  /*
  if (validWiFi && WiFi.channel() != WiFiEventData.usedChannel) {
    validWiFi = false;
  }
  */
  if (validWiFi != WiFiEventData.WiFiServicesInitialized()) {
    // else wifiStatus is no longer in sync.
    if (checkAndResetWiFi()) {
      // Wifi has been reset, so no longer valid WiFi
      validWiFi = false;
    }
  }

  if (validWiFi) {
    // Connected, thus disable any timer to start AP mode. (except when in WiFi setup mode)
    if (!WiFiEventData.wifiSetupConnect) {
      WiFiEventData.timerAPstart.clear();
    }
    STOP_TIMER(WIFI_ISCONNECTED_STATS);
    recursiveCall = false;
    // Only return true after some time since it got connected.
    SetWiFiTXpower();
    return WiFiEventData.wifi_considered_stable || WiFiEventData.lastConnectMoment.timeoutReached(100);
  }

  if ((WiFiEventData.timerAPstart.isSet()) && WiFiEventData.timerAPstart.timeReached()) {
    // Timer reached, so enable AP mode.
    if (!WifiIsAP(WiFi.getMode())) {
      if (!Settings.DoNotStartAP()) {
        setAP(true);
      }
    }
    WiFiEventData.timerAPstart.clear();
  }


  // When made this far in the code, we apparently do not have valid WiFi connection.
  if (!WiFiEventData.timerAPstart.isSet() && !WifiIsAP(WiFi.getMode())) {
    // First run we do not have WiFi connection any more, set timer to start AP mode
    // Only allow the automatic AP mode in the first N minutes after boot.
    if (getUptimeMinutes() < WIFI_ALLOW_AP_AFTERBOOT_PERIOD) {
      WiFiEventData.timerAPstart.setMillisFromNow(WIFI_RECONNECT_WAIT);
      // Fixme TD-er: Make this more elegant as it now needs to know about the extra time needed for the AP start timer.
      WiFiEventData.timerAPoff.setMillisFromNow(WIFI_RECONNECT_WAIT + WIFI_AP_OFF_TIMER_DURATION);
    }
  }

  if (wifiConnectTimeoutReached() && !WiFiEventData.wifiSetup) {
    // It took too long to make a connection, set flag we need to try again
    //if (!wifiAPmodeActivelyUsed()) {
      WiFiEventData.wifiConnectAttemptNeeded = true;
    //}
    WiFiEventData.wifiConnectInProgress = false;
  }
  delay(0);
  STOP_TIMER(WIFI_NOTCONNECTED_STATS);
  recursiveCall = false;
  return false;
}

void WiFiConnectRelaxed() {
  if (!WiFiEventData.WiFiConnectAllowed() || WiFiEventData.wifiConnectInProgress) {
    return; // already connected or connect attempt in progress need to disconnect first
  }
  if (!WiFiEventData.processedScanDone) {
    // Scan is still active, so do not yet connect.
    return;
  }
  if (WiFiEventData.unprocessedWifiEvents()) {
    handle_unprocessedNetworkEvents();
  }
  if (WiFiEventData.unprocessedWifiEvents()) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("WiFi : Connecting not possible, unprocessed WiFi events: ");
      if (!WiFiEventData.processedConnect) {
        log += F(" conn");
      }
      if (!WiFiEventData.processedDisconnect) {
        log += F(" disconn");
      }
      if (!WiFiEventData.processedGotIP) {
        log += F(" gotIP");
      }
      if (!WiFiEventData.processedDHCPTimeout) {
        log += F(" DHCP_t/o");
      }
      
      addLog(LOG_LEVEL_ERROR, log);
    }
    return;
  }


  if (WiFiEventData.unprocessedWifiEvents()) {
    // Still need to process WiFi events
    return;
  }
  if (!WiFiEventData.wifiSetupConnect && wifiAPmodeActivelyUsed()) {
    return;
  }


  // FIXME TD-er: Should not try to prepare when a scan is still busy.
  // This is a logic error which may lead to strange issues if some kind of timeout happens and/or RF calibration was not OK.
  // Split this function into separate parts, with the last part being the actual connect attempt either after a scan is complete or quick connect is possible.

  AttemptWiFiConnect();
}

void AttemptWiFiConnect() {
  if (!WiFiEventData.wifiConnectAttemptNeeded) {
    return;
  }

  if (WiFiEventData.wifiSetupConnect) {
    // wifiSetupConnect is when run from the setup page.
    RTC.clearLastWiFi(); // Force slow connect
    WiFiEventData.wifi_connect_attempt = 0;
    WiFiEventData.wifiSetupConnect     = false;
    if (WiFiEventData.timerAPoff.isSet()) {
      WiFiEventData.timerAPoff.setMillisFromNow(WIFI_RECONNECT_WAIT + WIFI_AP_OFF_TIMER_DURATION);
    }
  }

  WiFiEventData.markWiFiTurnOn();

  if (WiFi_AP_Candidates.getNext(WiFiScanAllowed())) {
    const WiFi_AP_Candidate& candidate = WiFi_AP_Candidates.getCurrent();

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("WIFI : Connecting ");
      log += candidate.toString();
      log += F(" attempt #");
      log += WiFiEventData.wifi_connect_attempt;
      addLog(LOG_LEVEL_INFO, log);
    }
    WiFiEventData.markWiFiBegin();
    if (prepareWiFi()) {
      RTC.clearLastWiFi();
      float tx_pwr = 0; // Will be set higher based on RSSI when needed.
      // FIXME TD-er: Must check WiFiEventData.wifi_connect_attempt to increase TX power
      if (Settings.UseMaxTXpowerForSending()) {
        tx_pwr = Settings.getWiFi_TX_power();
      }
      SetWiFiTXpower(tx_pwr, candidate.rssi);
      // Start connect attempt now, so no longer needed to attempt new connection.
      WiFiEventData.wifiConnectAttemptNeeded = false;
      if (candidate.allowQuickConnect()) {
        WiFi.begin(candidate.ssid.c_str(), candidate.key.c_str(), candidate.channel, candidate.bssid.mac);
      } else {
        WiFi.begin(candidate.ssid.c_str(), candidate.key.c_str());
      }
    } else {
      WiFiEventData.wifiConnectInProgress = false;
    }
  } else {
    if (!wifiAPmodeActivelyUsed() || WiFiEventData.wifiSetupConnect) {
      if (!prepareWiFi()) {
        return;
      }
      // Maybe not scan async to give the ESP some slack in power consumption?
      const bool async = false;
      WifiScan(async);
    } else {
      if (!WiFi_AP_Candidates.addedKnownCandidate()) {
        setNetworkMedium(NetworkMedium_t::ESPEasyNOW_only);
      }
    }
  }

  logConnectionStatus();
}

// ********************************************************************************
// Set Wifi config
// ********************************************************************************
bool prepareWiFi() {
  if (!WiFi_AP_Candidates.hasKnownCredentials()) {
    if (!WiFiEventData.warnedNoValidWiFiSettings) {
      addLog(LOG_LEVEL_ERROR, F("WIFI : No valid wifi settings"));
      WiFiEventData.warnedNoValidWiFiSettings = true;
    }
    WiFiEventData.last_wifi_connect_attempt_moment.clear();
    WiFiEventData.wifi_connect_attempt     = 1;
    WiFiEventData.wifiConnectAttemptNeeded = false;

    // No need to wait longer to start AP mode.
    if (!Settings.DoNotStartAP()) {
      setAP(true);
    }
    return false;
  }
  WiFiEventData.warnedNoValidWiFiSettings = false;
  setSTA(true);

  char hostname[40];
  safe_strncpy(hostname, NetworkCreateRFCCompliantHostname().c_str(), sizeof(hostname));
  #if defined(ESP8266)
  wifi_station_set_hostname(hostname);

  #endif // if defined(ESP8266)
  #if defined(ESP32)
  WiFi.setHostname(hostname);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  #endif // if defined(ESP32)
  setConnectionSpeed();
  setupStaticIPconfig();
  WiFiEventData.wifiConnectAttemptNeeded = true;

  return true;
}

bool checkAndResetWiFi() {
  #ifdef ESP8266
  station_status_t status = wifi_station_get_connect_status();

  switch(status) {
    case STATION_GOT_IP:
      if (WiFi.RSSI() < 0 && NetworkLocalIP().isSet()) {
        //if (WiFi.channel() == WiFiEventData.usedChannel || WiFiEventData.usedChannel == 0) {
          // This is a valid status, no need to reset
          if (!WiFiEventData.WiFiServicesInitialized()) {
            WiFiEventData.setWiFiServicesInitialized();
            setNetworkMedium(NetworkMedium_t::WIFI);
          }

          return false;
        //}
      }
      break;
    case STATION_NO_AP_FOUND:
    case STATION_CONNECT_FAIL:
    case STATION_WRONG_PASSWORD:
      // Reason to reset WiFi
      break;
    case STATION_IDLE:
    case STATION_CONNECTING:
      if (!WiFiEventData.last_wifi_connect_attempt_moment.timeoutReached(15000)) {
        return false;
      }
      break;
  }
  #endif
  #ifdef ESP32
  if (WiFi.isConnected()) {
    //if (WiFi.channel() == WiFiEventData.usedChannel || WiFiEventData.usedChannel == 0) {
      return false;
    //}
  }
  if (!WiFiEventData.last_wifi_connect_attempt_moment.timeoutReached(15000)) {
    return false;
  }
  #endif
  String log = F("WiFi : WiFiConnected() out of sync: ");
  log += ESPeasyWifiStatusToString();
  log += F(" RSSI: ");
  log += String(WiFi.RSSI());
  #ifdef ESP8266
  log += F(" status: ");
  log += SDKwifiStatusToString(status);
  #endif

  // Call for reset first, to make sure a syslog call will not try to send.
  resetWiFi();
  addLog(LOG_LEVEL_INFO, log);
  return true;
}


void resetWiFi() {
  //if (wifiAPmodeActivelyUsed()) return;
  if (WiFiEventData.lastWiFiResetMoment.isSet() && !WiFiEventData.lastWiFiResetMoment.timeoutReached(1000)) {
    // Don't reset WiFi too often
    return;
  }
  FeedSW_watchdog();
  WiFiEventData.clearAll();
  WifiDisconnect();

  // Send this log only after WifiDisconnect() or else sending to syslog may cause issues
  addLog(LOG_LEVEL_INFO, F("Reset WiFi."));

  //  setWifiMode(WIFI_OFF);

  initWiFi();
}

void initWiFi()
{
#ifdef USES_ESPEASY_NOW
  ESPEasy_now_handler.end();
#endif
#ifdef ESP8266

  // See https://github.com/esp8266/Arduino/issues/5527#issuecomment-460537616
  // FIXME TD-er: Do not destruct WiFi object, it may cause crashes with queued UDP traffic.
  WiFi.~ESP8266WiFiClass();
  WiFi = ESP8266WiFiClass();
#endif // ifdef ESP8266

  WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
  // The WiFi.disconnect() ensures that the WiFi is working correctly. If this is not done before receiving WiFi connections,
  // those WiFi connections will take a long time to make or sometimes will not work at all.
  WiFi.disconnect(true);
  setWifiMode(WIFI_OFF);

#if defined(ESP32)
  wm_event_id = WiFi.onEvent(WiFiEvent);
#endif
#ifdef ESP8266
  // WiFi event handlers
  stationConnectedHandler = WiFi.onStationModeConnected(onConnected);
	stationDisconnectedHandler = WiFi.onStationModeDisconnected(onDisconnect);
	stationGotIpHandler = WiFi.onStationModeGotIP(onGotIP);
  stationModeDHCPTimeoutHandler = WiFi.onStationModeDHCPTimeout(onDHCPTimeout);
  stationModeAuthModeChangeHandler = WiFi.onStationModeAuthModeChanged(onStationModeAuthModeChanged);
  APModeStationConnectedHandler = WiFi.onSoftAPModeStationConnected(onConnectedAPmode);
  APModeStationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected(onDisconnectedAPmode);
  APModeProbeRequestReceivedHandler = WiFi.onSoftAPModeProbeRequestReceived(onProbeRequestAPmode);

#endif
}

// ********************************************************************************
// Configure WiFi TX power
// ********************************************************************************
void SetWiFiTXpower() {
  if (Settings.UseESPEasyNow()) {
    // Set at max power for use with ESPEasy-NOW.
    SetWiFiTXpower(30, -99);
  } else {
    SetWiFiTXpower(0.0f); // Just some minimal value, will be adjusted in SetWiFiTXpower
  }
}

void SetWiFiTXpower(float dBm) { 
  SetWiFiTXpower(dBm, WiFi.RSSI());
}

void SetWiFiTXpower(float dBm, float rssi) {
  const WiFiMode_t cur_mode = WiFi.getMode();
  if (cur_mode == WIFI_OFF) {
    return;
  }

  if (Settings.UseESPEasyNow()) {
    // Do not mess with WiFi TX power when ESPEasy-now is used.
    return;
  }

  // Range ESP32  : 2dBm - 20dBm
  // Range ESP8266: 0dBm - 20.5dBm
  float maxTXpwr;
  float threshold = GetRSSIthreshold(maxTXpwr);
  float minTXpwr = 0;

  threshold += Settings.WiFi_sensitivity_margin; // Margin in dBm on top of threshold

  // Assume AP sends with max set by ETSI standard.
  // 2.4 GHz: 100 mWatt (20 dBm)
  // US and some other countries allow 1000 mW (30 dBm)
  // We cannot send with over 20 dBm, thus it makes no sense to force higher TX power all the time.
  const float newrssi = rssi - 20;
  if (newrssi < threshold) {
    minTXpwr = threshold - newrssi;
  }
  if (minTXpwr > maxTXpwr) {
    minTXpwr = maxTXpwr;
  }
  if (dBm > maxTXpwr) {
    dBm = maxTXpwr;
  } else if (dBm < minTXpwr) {
    dBm = minTXpwr;
  }

  #ifdef ESP32
  wifi_power_t val = WIFI_POWER_MINUS_1dBm;
  if (dBm < 0) { 
    val = WIFI_POWER_MINUS_1dBm;
    dBm = -1;
  } else if (dBm < 3.5) {
    val = WIFI_POWER_2dBm;
    dBm = 2;
  } else if (dBm < 6) {
    val = WIFI_POWER_5dBm;
    dBm = 5;
  } else if (dBm < 8) {
    val = WIFI_POWER_7dBm;
    dBm = 7;
  } else if (dBm < 10) {
    val = WIFI_POWER_8_5dBm;
    dBm = 8.5;
  } else if (dBm < 12) {
    val = WIFI_POWER_11dBm;
    dBm = 11;
  } else if (dBm < 14) {
    val = WIFI_POWER_13dBm;
    dBm = 13;
  } else if (dBm < 16) {
    val = WIFI_POWER_15dBm;
    dBm = 15;
  } else if (dBm < 17.75) {
    val = WIFI_POWER_17dBm;
    dBm = 17;
  } else if (dBm < 18.75) {
    val = WIFI_POWER_18_5dBm;
    dBm = 18.5;
  } else if (dBm < 19.25) {
    val = WIFI_POWER_19dBm;
    dBm = 19;
  } else {
    val = WIFI_POWER_19_5dBm;
    dBm = 19.5;
  }
  esp_wifi_set_max_tx_power(val);
  //esp_wifi_get_max_tx_power(&val);
//  dBm = static_cast<float>(val);
//  dBm /= 4.0f;
  #endif

  #ifdef ESP8266
  WiFi.setOutputPower(dBm);
  #endif

  if (WiFiEventData.wifi_TX_pwr < dBm) {
    // Will increase the TX power, give power supply of the unit some rest
    delay(1);
  }

  WiFiEventData.wifi_TX_pwr = dBm;

  delay(0);
  #ifndef BUILD_NO_DEBUG
  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    const int TX_pwr_int = WiFiEventData.wifi_TX_pwr * 4;
    const int maxTXpwr_int = maxTXpwr * 4;
    if (TX_pwr_int != maxTXpwr_int) {
      static int last_log = -1;
      if (TX_pwr_int != last_log) {
        last_log = TX_pwr_int;
        String log = F("WiFi : Set TX power to ");
        log += String(dBm, 0);
        log += F("dBm");
        log += F(" sensitivity: ");
        log += String(threshold, 0);
        log += F("dBm");
        if (rssi < 0) {
          log += F(" RSSI: ");
          log += String(rssi, 0);
          log += F("dBm");
        }
        addLog(LOG_LEVEL_DEBUG, log);
      }
    }
  }
  #endif
}

float GetRSSIthreshold(float& maxTXpwr) {
    if (Settings.UseMaxTXpowerForSending()
#ifdef USES_ESPEASY_NOW
      || isESPEasy_now_only()
#endif
  ) {
    maxTXpwr = 30.0;
  } else {
    maxTXpwr = Settings.getWiFi_TX_power();
  }
  float threshold = -72;
  switch (getConnectionProtocol()) {
    case WiFiConnectionProtocol::WiFi_Protocol_11b:
      threshold = -91;
      if (maxTXpwr > 20.5) maxTXpwr = 20.5;
      break;
    case WiFiConnectionProtocol::WiFi_Protocol_11g:
      threshold = -75;
      if (maxTXpwr > 17) maxTXpwr = 17;
      break;
    case WiFiConnectionProtocol::WiFi_Protocol_11n:
      threshold = -72;
      if (maxTXpwr > 14) maxTXpwr = 14;
      break;
    case WiFiConnectionProtocol::Unknown:
      break;
  }
  return threshold;
}

WiFiConnectionProtocol getConnectionProtocol() {
  if (WiFi.RSSI() < 0) {
    #ifdef ESP8266
    switch (wifi_get_phy_mode()) {
      case PHY_MODE_11B:
        return WiFiConnectionProtocol::WiFi_Protocol_11b;
      case PHY_MODE_11G:
        return WiFiConnectionProtocol::WiFi_Protocol_11g;
      case PHY_MODE_11N:
        return WiFiConnectionProtocol::WiFi_Protocol_11n;
    }
    #endif
    #ifdef ESP32
    uint8_t protocol;
    esp_wifi_get_protocol(WIFI_IF_STA, &protocol);
    if (protocol & WIFI_PROTOCOL_11N) {
      return WiFiConnectionProtocol::WiFi_Protocol_11n;
    }
    if (protocol & WIFI_PROTOCOL_11G) {
      return WiFiConnectionProtocol::WiFi_Protocol_11g;
    }
    if (protocol & WIFI_PROTOCOL_11B) {
      return WiFiConnectionProtocol::WiFi_Protocol_11b;
    }
    #endif
  }
  return WiFiConnectionProtocol::Unknown;
}

// ********************************************************************************
// Disconnect from Wifi AP
// ********************************************************************************
void WifiDisconnect()
{
  #ifdef ESP32
  WiFi.disconnect();
  WiFi.removeEvent(wm_event_id);
  #endif
  #ifdef ESP8266
  // Only call disconnect when STA is active
  if (WifiIsSTA(WiFiMode())) {
    wifi_station_disconnect();
  }
  station_config conf{};
  memset(&conf, 0, sizeof(conf));
  ETS_UART_INTR_DISABLE();
  wifi_station_set_config_current(&conf);
  ETS_UART_INTR_ENABLE();
  #endif // if defined(ESP32)
  WiFiEventData.setWiFiDisconnected();
  WiFiEventData.markDisconnect(WIFI_DISCONNECT_REASON_ASSOC_LEAVE);
  delay(1);
}

// ********************************************************************************
// Scan WiFi network
// ********************************************************************************
void WiFiScanPeriodical() {
  WiFi_AP_Candidates.purge_expired();
  if (!Settings.PeriodicalScanWiFi()) {
    return;
  }
  if (active_network_medium == NetworkMedium_t::Ethernet) {
    return;
  }

  const bool async = true;
  WifiScan(async);
}

bool WiFiScanAllowed() {
  if (WiFi_AP_Candidates.scanComplete() == WIFI_SCAN_RUNNING) {
    return false;
  }
  if (!WiFiEventData.processedScanDone) { 
    processScanDone(); 
  }
  if (WiFiEventData.unprocessedWifiEvents()) {
    handle_unprocessedNetworkEvents();
  }
  if (WiFiEventData.unprocessedWifiEvents()) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("WiFi : Scan not allowed, unprocessed WiFi events: ");
      if (!WiFiEventData.processedConnect) {
        log += F(" conn");
      }
      if (!WiFiEventData.processedDisconnect) {
        log += F(" disconn");
      }
      if (!WiFiEventData.processedGotIP) {
        log += F(" gotIP");
      }
      if (!WiFiEventData.processedDHCPTimeout) {
        log += F(" DHCP_t/o");
      }
      
      addLog(LOG_LEVEL_ERROR, log);
    }
    return false;
  }
  /*
  if (!wifiAPmodeActivelyUsed() && !NetworkConnected()) {
    return true;
  }
  */
  WiFi_AP_Candidates.purge_expired();
  if (WiFiEventData.wifiConnectInProgress) {
    return false;
  }
  if (NetworkConnected() && WiFi_AP_Candidates.getBestCandidate().usable()) {
    addLog(LOG_LEVEL_ERROR, F("WiFi : Scan not needed, good candidate present"));
    return false;
  }

  if (WiFiEventData.lastDisconnectMoment.isSet() && WiFiEventData.lastDisconnectMoment.millisPassedSince() < WIFI_RECONNECT_WAIT) {
    if (!NetworkConnected()) {
      return true;
    }
  }
  if (WiFiEventData.lastScanMoment.isSet()) {
    const LongTermTimer::Duration scanInterval = wifiAPmodeActivelyUsed() ? WIFI_SCAN_INTERVAL_AP_USED : WIFI_SCAN_INTERVAL_MINIMAL;
    if (WiFiEventData.lastScanMoment.millisPassedSince() < scanInterval) {
      return false;
    }
  }
  return true;
}


void WifiScan(bool async, uint8_t channel) {
  if (!WiFiScanAllowed()) {
    return;
  }
  START_TIMER;
  WiFiEventData.lastScanMoment.setNow();
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    if (channel == 0) {
      addLog(LOG_LEVEL_INFO, F("WiFi : Start network scan all channels"));
    } else {
      String log;
      log = F("WiFi : Start network scan channel ");
      log += channel;
      addLog(LOG_LEVEL_INFO, log);
    }
  }
  bool show_hidden         = true;
  WiFiEventData.processedScanDone = false;
  WiFiEventData.lastGetScanMoment.setNow();
  WiFiEventData.lastScanChannel = channel;

  unsigned int nrScans = 1 + (async ? 0 : Settings.NumberExtraWiFiScans);
  while (nrScans > 0) {
    if (!async) {
      WiFi_AP_Candidates.begin_sync_scan();
      FeedSW_watchdog();
    }
    --nrScans;
    #ifdef ESP8266
    /*
    {
      static bool FIRST_SCAN = true;

      struct scan_config config;
      memset(&config, 0, sizeof(config));
      config.ssid = nullptr;
      config.bssid = nullptr;
      config.channel = channel;
      config.show_hidden = 1;
      config.scan_type = WIFI_SCAN_TYPE_ACTIVE;
      if (FIRST_SCAN) {
        config.scan_time.active.min = 100;
        config.scan_time.active.max = 200;
      } else {
        config.scan_time.active.min = 400;
        config.scan_time.active.max = 500;
      }
      FIRST_SCAN = false;
      wifi_station_scan(&config, &onWiFiScanDone);
 
    }
    */
    WiFi.scanNetworks(async, show_hidden, channel);
    #endif
    #ifdef ESP32
    const bool passive = false;
    const uint32_t max_ms_per_chan = 300;
    WiFi.scanNetworks(async, show_hidden, passive, max_ms_per_chan /*, channel */);
    #endif
    if (!async) {
      FeedSW_watchdog();
      processScanDone();
    }
  }
  STOP_TIMER(async ? WIFI_SCAN_ASYNC : WIFI_SCAN_SYNC);
}

// ********************************************************************************
// Scan all Wifi Access Points
// ********************************************************************************
void WifiScan()
{
  // Direct Serial is allowed here, since this function will only be called from serial input.
  serialPrintln(F("WIFI : SSID Scan start"));
  if (WiFi_AP_Candidates.scanComplete() <= 0) {
    WiFiMode_t cur_wifimode = WiFi.getMode();
    WifiScan(false);
    setWifiMode(cur_wifimode);
  }

  const int8_t scanCompleteStatus = WiFi_AP_Candidates.scanComplete();
  if (scanCompleteStatus <= 0) {
    serialPrintln(F("WIFI : No networks found"));
  }
  else
  {
    serialPrint(F("WIFI : "));
    serialPrint(String(scanCompleteStatus));
    serialPrintln(F(" networks found"));

    int i = 0;

    for (auto it = WiFi_AP_Candidates.scanned_begin(); it != WiFi_AP_Candidates.scanned_end(); ++it)
    {
      ++i;
      // Print SSID and RSSI for each network found
      serialPrint(F("WIFI : "));
      serialPrint(String(i));
      serialPrint(": ");
      serialPrintln(it->toString());
      delay(10);
    }
  }
  serialPrintln("");
}

// ********************************************************************************
// Manage Wifi Modes
// ********************************************************************************
void setSTA(bool enable) {
  switch (WiFi.getMode()) {
    case WIFI_OFF:

      if (enable) { setWifiMode(WIFI_STA); }
      break;
    case WIFI_STA:

      if (!enable) { setWifiMode(WIFI_OFF); }
      break;
    case WIFI_AP:

      if (enable) { setWifiMode(WIFI_AP_STA); }
      break;
    case WIFI_AP_STA:

      if (!enable) { setWifiMode(WIFI_AP); }
      break;
    default:
      break;
  }
}

void setAP(bool enable) {
  WiFiMode_t wifimode = WiFi.getMode();

  #ifdef USES_ESPEASY_NOW
  if (!enable && use_EspEasy_now) {
    ESPEasy_now_handler.end();
  }
  #endif

  switch (wifimode) {
    case WIFI_OFF:

      if (enable) { 
        WifiScan(false);
        setWifiMode(WIFI_AP); 
      }
      break;
    case WIFI_STA:

      if (enable) { setWifiMode(WIFI_AP_STA); }
      break;
    case WIFI_AP:

      if (!enable) { setWifiMode(WIFI_OFF); }
      break;
    case WIFI_AP_STA:

      if (!enable) { setWifiMode(WIFI_STA); }
      break;
    default:
      break;
  }
}

// Only internal scope
void setAPinternal(bool enable)
{
  if (enable) {
    // create and store unique AP SSID/PW to prevent ESP from starting AP mode with default SSID and No password!
    // setup ssid for AP Mode when needed
    String softAPSSID = NetworkCreateRFCCompliantHostname();
    String pwd        = SecuritySettings.WifiAPKey;
    IPAddress subnet(DEFAULT_AP_SUBNET);

    if (!WiFi.softAPConfig(apIP, apIP, subnet)) {
      addLog(LOG_LEVEL_ERROR, F("WIFI : [AP] softAPConfig failed!"));
    }

    if (WiFi.softAP(softAPSSID.c_str(), pwd.c_str())) {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        eventQueue.add(F("WiFi#APmodeEnabled"));
        String log(F("WIFI : AP Mode ssid will be "));
        log += softAPSSID;
        log += F(" with address ");
        log += WiFi.softAPIP().toString();
        addLog(LOG_LEVEL_INFO, log);
      }
    } else {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        String log(F("WIFI : Error while starting AP Mode with SSID: "));
        log += softAPSSID;
        log += F(" IP: ");
        log += apIP.toString();
        addLog(LOG_LEVEL_ERROR, log);
      }
    }
    #ifdef ESP32

    #else // ifdef ESP32

    if (wifi_softap_dhcps_status() != DHCP_STARTED) {
      if (!wifi_softap_dhcps_start()) {
        addLog(LOG_LEVEL_ERROR, F("WIFI : [AP] wifi_softap_dhcps_start failed!"));
      }
    }
    #endif // ifdef ESP32
    WiFiEventData.timerAPoff.setMillisFromNow(WIFI_AP_OFF_TIMER_DURATION);
  } else {
    #ifdef FEATURE_DNS_SERVER
    if (dnsServerActive) {
      dnsServerActive = false;
      dnsServer.stop();
    }
    #endif
  }
}

const __FlashStringHelper * getWifiModeString(WiFiMode_t wifimode)
{
  switch (wifimode) {
    case WIFI_OFF:   return F("OFF");
    case WIFI_STA:   return F("STA");
    case WIFI_AP:    return F("AP");
    case WIFI_AP_STA: return F("AP+STA");
    default:
      break;
  }
  return F("Unknown");
}

void setWifiMode(WiFiMode_t wifimode) {
  const WiFiMode_t cur_mode = WiFi.getMode();

  if (cur_mode == wifimode) {
    return;
  }

  if (cur_mode == WIFI_OFF) {
    WiFiEventData.markWiFiTurnOn();

    #if defined(ESP32)
    esp_wifi_set_ps(WIFI_PS_NONE);
    #endif
    #ifdef ESP8266

    // See: https://github.com/esp8266/Arduino/issues/6172#issuecomment-500457407
    WiFi.forceSleepWake(); // Make sure WiFi is really active.
    #endif // ifdef ESP8266
    delay(100);
  }

  addLog(LOG_LEVEL_INFO, String(F("WIFI : Set WiFi to ")) + getWifiModeString(wifimode));

  int retry = 2;
  while (!WiFi.mode(wifimode) && retry > 0) {
    addLog(LOG_LEVEL_INFO, F("WIFI : Cannot set mode!!!!!"));
    delay(100);
    --retry;
  }
  retry = 2;
  while (WiFi.getMode() != wifimode && retry > 0) {
    addLog(LOG_LEVEL_INFO, F("WIFI : mode not yet set"));
    delay(100);
    --retry;
  }


  if (wifimode == WIFI_OFF) {
    WiFiEventData.markWiFiTurnOn();
    delay(100);
    #if defined(ESP32)
    esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
    #endif
    #ifdef ESP8266
    WiFi.forceSleepBegin();
    #endif // ifdef ESP8266
    delay(1);
  } else {
    // Only set power mode when AP is not enabled
    // When AP is enabled, the sleep mode is already set to WIFI_NONE_SLEEP
    if (!WifiIsAP(wifimode)) {
      if (Settings.WifiNoneSleep()) {
        #ifdef ESP8266
        WiFi.setSleepMode(WIFI_NONE_SLEEP);
        #endif
        #ifdef ESP32
        WiFi.setSleep(WIFI_PS_NONE);
        #endif
      } else if (Settings.EcoPowerMode()) {
        // Allow light sleep during idle times
        #ifdef ESP8266
        WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
        #endif
        #ifdef ESP32
        // Maximum modem power saving. 
        // In this mode, interval to receive beacons is determined by the listen_interval parameter in wifi_sta_config_t
        // FIXME TD-er: Must test if this is desired behavior in ESP32.
        WiFi.setSleep(WIFI_PS_MAX_MODEM);
        #endif
      } else {
        // Default
        #ifdef ESP8266
        WiFi.setSleepMode(WIFI_MODEM_SLEEP);
        #endif
        #ifdef ESP32
        // Minimum modem power saving. 
        // In this mode, station wakes up to receive beacon every DTIM period
        WiFi.setSleep(WIFI_PS_MIN_MODEM);
        #endif
      }
    }

    SetWiFiTXpower();
    if (WifiIsSTA(wifimode)) {
      if (WiFi.getAutoConnect()) {
        WiFi.setAutoConnect(false); 
      }
      if (WiFi.getAutoReconnect()) {
        WiFi.setAutoReconnect(false);
      }
    }
    delay(100); // Must allow for some time to init.
  }
  const bool new_mode_AP_enabled = WifiIsAP(wifimode);

  if (WifiIsAP(cur_mode) && !new_mode_AP_enabled) {
    eventQueue.add(F("WiFi#APmodeDisabled"));
  }

  if (WifiIsAP(cur_mode) != new_mode_AP_enabled) {
    // Mode has changed
    setAPinternal(new_mode_AP_enabled);
  }
  #ifdef FEATURE_MDNS
  #ifdef ESP8266
  // notifyAPChange() is not present in the ESP32 MDNSResponder
  MDNS.notifyAPChange();
  #endif
  #endif
}

bool WifiIsAP(WiFiMode_t wifimode)
{
  #if defined(ESP32)
  return (wifimode == WIFI_MODE_AP) || (wifimode == WIFI_MODE_APSTA);
  #else // if defined(ESP32)
  return (wifimode == WIFI_AP) || (wifimode == WIFI_AP_STA);
  #endif // if defined(ESP32)
}

bool WifiIsSTA(WiFiMode_t wifimode)
{
  #if defined(ESP32)
  return (wifimode & WIFI_MODE_STA) != 0;
  #else // if defined(ESP32)
  return (wifimode & WIFI_STA) != 0;
  #endif // if defined(ESP32)
}

bool useStaticIP() {
  return Settings.IP[0] != 0 && Settings.IP[0] != 255;
}

bool wifiConnectTimeoutReached() {
  // For the first attempt, do not wait to start connecting.
  if (WiFiEventData.wifi_connect_attempt == 0) { return true; }
  if (!WiFiEventData.wifiConnectInProgress) { return true; }

  if (!WiFiEventData.last_wifi_connect_attempt_moment.isSet()) {
    // No attempt made
    return true;
  }

  if (WiFiEventData.lastDisconnectMoment.isSet()) {
    // Connection attempt was already ended.
    return true;
  }

  if (WifiIsAP(WiFi.getMode())) {
    // FIXME TD-er: What to do here when using ESPEasy-NOW mode?
    // Initial setup of WiFi, may take much longer since accesspoint is still active.
    return WiFiEventData.last_wifi_connect_attempt_moment.timeoutReached(20000);
  }

  // wait until it connects + add some device specific random offset to prevent
  // all nodes overloading the access point when turning on at the same time.
  #if defined(ESP8266)
  const unsigned int randomOffset_in_msec = (WiFiEventData.wifi_connect_attempt == 1) ? 0 : 1000 * ((ESP.getChipId() & 0xF));
  #endif // if defined(ESP8266)
  #if defined(ESP32)
  const unsigned int randomOffset_in_msec = (WiFiEventData.wifi_connect_attempt == 1) ? 0 : 1000 * ((ESP.getEfuseMac() & 0xF));
  #endif // if defined(ESP32)
  return WiFiEventData.last_wifi_connect_attempt_moment.timeoutReached(DEFAULT_WIFI_CONNECTION_TIMEOUT + randomOffset_in_msec);
}

bool wifiAPmodeActivelyUsed()
{
  #ifdef USES_ESPEASY_NOW
  if (isESPEasy_now_only() &&
        last_network_medium_set_moment.timeoutReached(600 * 1000)) 
  {
    // Only allow the ESPEasy_NOW_only mode for 10 minutes
    setNetworkMedium(Settings.NetworkMedium);
    return false;
  } else if (ESPEasy_now_handler.active()) {
    return true;
  }
  #endif

  if (!WifiIsAP(WiFi.getMode()) || (!WiFiEventData.timerAPoff.isSet())) {
    // AP not active or soon to be disabled in processDisableAPmode()
    return false;
  }

  return WiFi.softAPgetStationNum() != 0;

  // FIXME TD-er: is effectively checking for AP active enough or must really check for connected clients to prevent automatic wifi
  // reconnect?
}

void setConnectionSpeed() {
  #ifdef ESP8266
  if (!Settings.ForceWiFi_bg_mode() || (WiFiEventData.wifi_connect_attempt > 10)) {
    WiFi.setPhyMode(WIFI_PHY_MODE_11N);
  } else {
    WiFiPhyMode_t phyMode = WIFI_PHY_MODE_11G;
    #ifdef USE_ESPEASY_NOW
    if (active_network_medium == NetworkMedium_t::ESPEasyNOW_only) {
      phyMode = WIFI_PHY_MODE_11B;
    }
    #endif
    WiFi.setPhyMode(phyMode);
  }
  #endif // ifdef ESP8266

  // Does not (yet) work, so commented out.
  #ifdef ESP32
  /*
  uint8_t protocol = WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G; // Default to BG

  if (!Settings.ForceWiFi_bg_mode() || (wifi_connect_attempt > 10)) {
    // Set to use BGN
    protocol |= WIFI_PROTOCOL_11N;
  }

  if (WifiIsSTA(WiFi.getMode())) {
    esp_wifi_set_protocol(WIFI_IF_STA, protocol);
  }

  if (WifiIsAP(WiFi.getMode())) {
    esp_wifi_set_protocol(WIFI_IF_AP, protocol);
  }
  */
  #endif // ifdef ESP32
  SetWiFiTXpower();
}

void setupStaticIPconfig() {
  setUseStaticIP(useStaticIP());

  if (!useStaticIP()) { return; }
  const IPAddress ip     = Settings.IP;
  const IPAddress gw     = Settings.Gateway;
  const IPAddress subnet = Settings.Subnet;
  const IPAddress dns    = Settings.DNS;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("IP   : Static IP : ");
    log += formatIP(ip);
    log += F(" GW: ");
    log += formatIP(gw);
    log += F(" SN: ");
    log += formatIP(subnet);
    log += F(" DNS: ");
    log += formatIP(dns);
    addLog(LOG_LEVEL_INFO, log);
  }
  WiFi.config(ip, gw, subnet, dns);
}

// ********************************************************************************
// Formatting WiFi related strings
// ********************************************************************************
String formatScanResult(int i, const String& separator) {
  int32_t rssi = 0;

  return formatScanResult(i, separator, rssi);
}

String formatScanResult(int i, const String& separator, int32_t& rssi) {
  WiFi_AP_Candidate tmp(i);
  rssi = tmp.rssi;
  return tmp.toString(separator);
}


String ESPeasyWifiStatusToString() {
  String log;
  if (WiFiEventData.WiFiDisconnected()) {
    log = F("DISCONNECTED");
  } else {
    if (WiFiEventData.WiFiConnected()) {
      log += F("Conn. ");
    }
    if (WiFiEventData.WiFiGotIP()) {
      log += F("IP ");
    }
    if (WiFiEventData.WiFiServicesInitialized()) {
      log += F("Init");
    }
  }
  return log;
}

void logConnectionStatus() {
  static unsigned long lastLog = 0;
  if (lastLog != 0 && timePassedSince(lastLog) < 1000) {
    return;
  }
  lastLog = millis();
#ifndef BUILD_NO_DEBUG
  #ifdef ESP8266
  const uint8_t arduino_corelib_wifistatus = WiFi.status();
  const uint8_t sdk_wifistatus             = wifi_station_get_connect_status();

  if ((arduino_corelib_wifistatus == WL_CONNECTED) != (sdk_wifistatus == STATION_GOT_IP)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log = F("WiFi : SDK station status differs from Arduino status. SDK-status: ");
      log += SDKwifiStatusToString(sdk_wifistatus);
      log += F(" Arduino status: ");
      log += ArduinoWifiStatusToString(arduino_corelib_wifistatus);
      addLog(LOG_LEVEL_ERROR, log);
    }
  }
  #endif

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("WIFI : Arduino wifi status: ");
    log += ArduinoWifiStatusToString(WiFi.status());
    log += F(" ESPeasy internal wifi status: ");
    log += ESPeasyWifiStatusToString();
    addLog(LOG_LEVEL_INFO, log);
  }
/*
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;

    switch (WiFi.status()) {
      case WL_NO_SSID_AVAIL: {
        log = F("WIFI : No SSID found matching: ");
        break;
      }
      case WL_CONNECT_FAILED: {
        log = F("WIFI : Connection failed to: ");
        break;
      }
      case WL_DISCONNECTED: {
        log = F("WIFI : WiFi.status() = WL_DISCONNECTED  SSID: ");
        break;
      }
      case WL_IDLE_STATUS: {
        log = F("WIFI : Connection in IDLE state: ");
        break;
      }
      case WL_CONNECTED: {
        break;
      }
      default:
        break;
    }

    if (log.length() > 0) {
      const char *ssid = getLastWiFiSettingsSSID();
      log += ssid;
      addLog(LOG_LEVEL_INFO, log);
    }
  }
  */
#endif // ifndef BUILD_NO_DEBUG
}
