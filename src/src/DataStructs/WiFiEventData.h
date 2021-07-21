#ifndef DATASTRUCTS_WIFIEVENTDATA_H
#define DATASTRUCTS_WIFIEVENTDATA_H

#include "../../ESPEasy-Globals.h"

#include "../DataStructs/MAC_address.h"
#include "../DataTypes/WiFiDisconnectReason.h"
#include "../Helpers/LongTermTimer.h"


#ifdef ESP32
# include <esp_event.h>
# include <WiFiGeneric.h>
# include <WiFiType.h>

#endif // ifdef ESP32

#include <IPAddress.h>

#ifdef ESP8266
# include <ESP8266WiFiGeneric.h>
# include <ESP8266WiFiType.h>
#endif // ifdef ESP8266

// WifiStatus
#define ESPEASY_WIFI_DISCONNECTED            0


struct WiFiEventData_t {
  bool WiFiConnectAllowed() const;

  bool unprocessedWifiEvents() const;

  void clearAll();
  void markWiFiTurnOn();
  void markWiFiBegin();

  bool WiFiDisconnected() const;
  bool WiFiGotIP() const;
  bool WiFiConnected() const;
  bool WiFiServicesInitialized() const;

  void setWiFiDisconnected();
  void setWiFiGotIP();
  void setWiFiConnected();
  void setWiFiServicesInitialized();


  void markGotIP();
  void markLostIP();
  void markDisconnect(WiFiDisconnectReason reason);
  void markConnected(const String& ssid,
                     const uint8_t bssid[6],
                     uint8_t          channel);
  void markConnectedAPmode(const uint8_t mac[6]);
  void markDisconnectedAPmode(const uint8_t mac[6]);

  void setAuthMode(uint8_t newMode);



  // WiFi related data
  bool          wifiSetup        = false;
  bool          wifiSetupConnect = false;
  uint8_t       wifiStatus       = ESPEASY_WIFI_DISCONNECTED;
  LongTermTimer last_wifi_connect_attempt_moment;
  unsigned int  wifi_connect_attempt   = 0;
  bool          wifi_considered_stable = false;
  int           wifi_reconnects        = -1; // First connection attempt is not a reconnect.
  String        last_ssid;
  float         wifi_TX_pwr            = 0;
  bool          bssid_changed          = false;
  bool          channel_changed        = false;

  uint8_t       auth_mode = 0;
  uint8_t       lastScanChannel = 0;
  uint8_t       usedChannel = 0;

  bool          eventError = false;


  WiFiDisconnectReason    lastDisconnectReason = WIFI_DISCONNECT_REASON_UNSPECIFIED;
  LongTermTimer           lastScanMoment;
  LongTermTimer           lastConnectMoment;
  LongTermTimer           lastDisconnectMoment;
  LongTermTimer           lastWiFiResetMoment;
  LongTermTimer           lastGetIPmoment;
  LongTermTimer           lastGetScanMoment;
  LongTermTimer::Duration lastConnectedDuration_us = 0ll;
  LongTermTimer           timerAPoff;   // Timer to check whether the AP mode should be disabled (0 = disabled)
  LongTermTimer           timerAPstart; // Timer to start AP mode, started when no valid network is detected.
  LongTermTimer           lastAPmodeStationConnectMoment;
  bool                    intent_to_reboot = false;
  MAC_address             lastMacConnectedAPmode;
  MAC_address             lastMacDisconnectedAPmode;

  // processDisconnect() may clear all WiFi settings, resulting in clearing processedDisconnect
  // This can cause recursion, so a semaphore is needed here.
  LongTermTimer           processingDisconnect;


  // Semaphore like bools for processing data gathered from WiFi events.
  bool processedConnect          = true;
  bool processedDisconnect       = true;
  bool processedGotIP            = true;
  bool processedDHCPTimeout      = true;
  bool processedConnectAPmode    = true;
  bool processedDisconnectAPmode = true;
  bool processedScanDone         = true;
  bool processedProbeRequestAPmode = true;
  bool wifiConnectAttemptNeeded  = true;
  bool wifiConnectInProgress     = false;
  bool warnedNoValidWiFiSettings = false;

  bool performedClearWiFiCredentials = false;

  unsigned long connectionFailures = 0;


};

#endif   // ifndef DATASTRUCTS_WIFIEVENTDATA_H
