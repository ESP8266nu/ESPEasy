#ifndef ESPEASY_WIFI_EVENT_H
#define ESPEASY_WIFI_EVENT_H

#include "../../ESPEasy_common.h"

#include <IPAddress.h>

// ********************************************************************************

// Work-around for setting _useStaticIP
// See reported issue: https://github.com/esp8266/Arduino/issues/4114
// ********************************************************************************
#ifdef ESP32
#include <IPv6Address.h>
#include <WiFiSTA.h>
class WiFi_Access_Static_IP : public WiFiSTAClass {
public:

  void set_use_static_ip(bool enabled);
};
#endif

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WiFiSTA.h>
class WiFi_Access_Static_IP : public ESP8266WiFiSTAClass {
public:

  void set_use_static_ip(bool enabled);
};
#endif


void setUseStaticIP(bool enabled);


// ********************************************************************************
// Functions called on events.
// Make sure not to call anything in these functions that result in delay() or yield()
// ********************************************************************************
#ifdef ESP32
 #if ESP_IDF_VERSION_MAJOR > 3
  void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);
 #else
  void WiFiEvent(system_event_id_t event, system_event_info_t info);
 #endif
#endif

#ifdef ESP8266

void onConnected(const WiFiEventStationModeConnected& event);

void onDisconnect(const WiFiEventStationModeDisconnected& event);

void onGotIP(const WiFiEventStationModeGotIP& event);

void onDHCPTimeout();

void onConnectedAPmode(const WiFiEventSoftAPModeStationConnected& event);

void onDisconnectedAPmode(const WiFiEventSoftAPModeStationDisconnected& event);

void onProbeRequestAPmode(const WiFiEventSoftAPModeProbeRequestReceived& event);

void onStationModeAuthModeChanged(const WiFiEventStationModeAuthModeChanged& event);

void onWiFiScanDone(void *arg, STATUS status);

#endif


#endif // ESPEASY_WIFI_EVENT_H