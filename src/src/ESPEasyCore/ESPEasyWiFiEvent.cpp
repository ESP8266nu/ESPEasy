#include "ESPEasyWiFiEvent.h"

#ifdef HAS_ETHERNET
#include "ETH.h"
#endif

#include "../DataStructs/RTCStruct.h"

#include "../DataTypes/ESPEasyTimeSource.h"

#include "../ESPEasyCore/ESPEasyEth.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasyWifi.h"
#include "../ESPEasyCore/ESPEasyWifi_ProcessEvent.h"

#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/NetworkState.h"
#include "../Globals/RTC.h"
#include "../Globals/WiFi_AP_Candidates.h"

#include "../Helpers/ESPEasy_time_calc.h"



#ifdef ESP32
void WiFi_Access_Static_IP::set_use_static_ip(bool enabled) {
  _useStaticIp = enabled;
}

#endif // ifdef ESP32
#ifdef ESP8266
void WiFi_Access_Static_IP::set_use_static_ip(bool enabled) {
  _useStaticIp = enabled;
}

#endif // ifdef ESP8266


void setUseStaticIP(bool enabled) {
  WiFi_Access_Static_IP tmp_wifi;

  tmp_wifi.set_use_static_ip(enabled);
}


// ********************************************************************************
// Functions called on events.
// Make sure not to call anything in these functions that result in delay() or yield()
// ********************************************************************************
#ifdef ESP32
#include <WiFi.h>

static bool ignoreDisconnectEvent = false;

#if ESP_IDF_VERSION_MAJOR > 3
void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
#else
void WiFiEvent(system_event_id_t event, system_event_info_t info) {
#endif
  switch (event) {
    case SYSTEM_EVENT_WIFI_READY:
      // ESP32 WiFi ready
      break;
    case SYSTEM_EVENT_STA_START:
      addLog(LOG_LEVEL_INFO, F("WiFi : STA Started"));
      break;
    case SYSTEM_EVENT_STA_STOP:
      addLog(LOG_LEVEL_INFO, F("WiFi : STA Stopped"));
      break;
    case SYSTEM_EVENT_AP_START:
      addLog(LOG_LEVEL_INFO, F("WiFi : AP Started"));
      break;
    case SYSTEM_EVENT_AP_STOP:
      addLog(LOG_LEVEL_INFO, F("WiFi : AP Stopped"));
      break;
    case SYSTEM_EVENT_STA_LOST_IP:
      // ESP32 station lost IP and the IP is reset to 0
      #ifdef HAS_ETHERNET
      if (active_network_medium == NetworkMedium_t::Ethernet) {
        EthEventData.markLostIP();
      }
      else
      #endif
      WiFiEventData.markLostIP();
      addLog(LOG_LEVEL_INFO, 
        active_network_medium == NetworkMedium_t::Ethernet ?
        F("ETH : Lost IP") : F("WiFi : Lost IP"));
      break;

    case SYSTEM_EVENT_AP_PROBEREQRECVED:
      // Receive probe request packet in soft-AP interface
      // TODO TD-er: Must implement like onProbeRequestAPmode for ESP8266
      addLog(LOG_LEVEL_INFO, F("WiFi : AP got probed"));
      break;

    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
      #if ESP_IDF_VERSION_MAJOR > 3
      WiFiEventData.setAuthMode(info.wifi_sta_authmode_change.new_mode);
      #else
      WiFiEventData.setAuthMode(info.auth_change.new_mode);
      #endif
      break;

    case SYSTEM_EVENT_STA_CONNECTED:
    {
      char ssid_copy[33] = { 0 }; // Ensure space for maximum len SSID (32) plus trailing 0
      #if ESP_IDF_VERSION_MAJOR > 3
      memcpy(ssid_copy, info.wifi_sta_connected.ssid, info.wifi_sta_connected.ssid_len);
      ssid_copy[32] = 0; // Potentially add 0-termination if none present earlier
      WiFiEventData.markConnected((const char*) ssid_copy, info.wifi_sta_connected.bssid, info.wifi_sta_connected.channel);
      #else
      memcpy(ssid_copy, info.connected.ssid, info.connected.ssid_len);
      ssid_copy[32] = 0; // Potentially add 0-termination if none present earlier
      WiFiEventData.markConnected((const char*) ssid_copy, info.connected.bssid, info.connected.channel);
      #endif
      break;
    }
    case SYSTEM_EVENT_STA_DISCONNECTED:
      if (!ignoreDisconnectEvent) {
        ignoreDisconnectEvent = true;
        #if ESP_IDF_VERSION_MAJOR > 3
        WiFiEventData.markDisconnect(static_cast<WiFiDisconnectReason>(info.wifi_sta_disconnected.reason));
        #else
        WiFiEventData.markDisconnect(static_cast<WiFiDisconnectReason>(info.disconnected.reason));
        #endif
        WiFi.persistent(false);
        WiFi.disconnect(true);
      }
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      ignoreDisconnectEvent = false;
      WiFiEventData.markGotIP();
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      #if ESP_IDF_VERSION_MAJOR > 3
      WiFiEventData.markConnectedAPmode(info.wifi_ap_staconnected.mac);
      #else
      WiFiEventData.markConnectedAPmode(info.sta_connected.mac);
      #endif
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      #if ESP_IDF_VERSION_MAJOR > 3
      WiFiEventData.markDisconnectedAPmode(info.wifi_ap_stadisconnected.mac);
      #else
      WiFiEventData.markDisconnectedAPmode(info.sta_disconnected.mac);
      #endif
      break;
    case SYSTEM_EVENT_SCAN_DONE:
      WiFiEventData.processedScanDone = false;
      break;
#ifdef HAS_ETHERNET
    case SYSTEM_EVENT_ETH_START:
      if (ethPrepare()) {
        addLog(LOG_LEVEL_INFO, F("ETH  : Started"));
      } else {
        addLog(LOG_LEVEL_ERROR, F("ETH  : Could not prepare ETH!"));
      }
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      addLog(LOG_LEVEL_INFO, F("ETH  : Connected"));
      EthEventData.markConnected();
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      EthEventData.markGotIP();
      addLog(LOG_LEVEL_INFO, F("ETH  : Got IP"));
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      addLog(LOG_LEVEL_ERROR, F("ETH Disconnected"));
      EthEventData.markDisconnect();
      break;
    case SYSTEM_EVENT_ETH_STOP:
      addLog(LOG_LEVEL_INFO, F("ETH Stopped"));
      break;
    case SYSTEM_EVENT_GOT_IP6:
      addLog(LOG_LEVEL_INFO, F("ETH Got IP6"));
      break;
#endif //HAS_ETHERNET
    default:
      {
        String log = F("UNKNOWN WIFI/ETH EVENT: ");
        log += event;
        addLog(LOG_LEVEL_ERROR, log);
      }
      break;
  }
}

#endif // ifdef ESP32

#ifdef ESP8266

void onConnected(const WiFiEventStationModeConnected& event) {
  WiFiEventData.markConnected(event.ssid, event.bssid, event.channel);
}

void onDisconnect(const WiFiEventStationModeDisconnected& event) {
  WiFiEventData.markDisconnect(event.reason);
  if (WiFi.status() == WL_CONNECTED) {
    // See https://github.com/esp8266/Arduino/issues/5912
    WiFi.persistent(false);
    WiFi.disconnect(true);
  }
}

void onGotIP(const WiFiEventStationModeGotIP& event) {
  WiFiEventData.markGotIP();
}

void onDHCPTimeout() {
  WiFiEventData.processedDHCPTimeout = false;
}

void onConnectedAPmode(const WiFiEventSoftAPModeStationConnected& event) {
  WiFiEventData.markConnectedAPmode(event.mac);
}

void onDisconnectedAPmode(const WiFiEventSoftAPModeStationDisconnected& event) {
  WiFiEventData.markDisconnectedAPmode(event.mac);
}

void onProbeRequestAPmode(const WiFiEventSoftAPModeProbeRequestReceived& event) {
  APModeProbeRequestReceived_list.push_back(event);
  WiFiEventData.processedProbeRequestAPmode = false;
}

void onStationModeAuthModeChanged(const WiFiEventStationModeAuthModeChanged& event) {
  WiFiEventData.setAuthMode(event.newMode);
}

void onWiFiScanDone(void *arg, STATUS status) {
  if (status == OK) {
    auto *head = reinterpret_cast<bss_info *>(arg);
    int scanCount = 0;
    for (bss_info *it = head; it != nullptr; it = STAILQ_NEXT(it, next)) {
      WiFi_AP_Candidates.process_WiFiscan(*it);
      ++scanCount;
    }
    WiFi_AP_Candidates.after_process_WiFiscan();
    WiFiEventData.lastGetScanMoment.setNow();
    WiFiEventData.processedScanDone = true;
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F("WiFi : Scan finished, found: ");
      log += scanCount;
      addLog(LOG_LEVEL_INFO, log);
    }
  }

  WiFiMode_t mode = WiFi.getMode();
  WiFi.mode(WIFI_OFF);
  WiFi.mode(mode);
}

#endif // ifdef ESP8266
