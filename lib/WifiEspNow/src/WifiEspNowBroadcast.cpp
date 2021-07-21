#include "WifiEspNowBroadcast.h"

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <user_interface.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <esp_wifi.h>
#else
#error "This library supports ESP8266 and ESP32 only."
#endif

WifiEspNowBroadcastClass WifiEspNowBroadcast;

WifiEspNowBroadcastClass::WifiEspNowBroadcastClass()
  : m_isScanning(false)
{
}

bool
WifiEspNowBroadcastClass::begin(const char* ssid, int channel, int scanFreq)
{
  m_ssid = ssid;
  m_nextScan = 0;
  m_scanFreq = scanFreq;

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, nullptr, channel);

  return WifiEspNow.begin();
}

void
WifiEspNowBroadcastClass::loop()
{
  if (millis() >= m_nextScan && !m_isScanning && WiFi.scanComplete() != WIFI_SCAN_RUNNING) {
    this->scan();
  }
#ifdef ESP32
  if (m_isScanning && WiFi.scanComplete() >= 0) {
    this->processScan();
  }
#endif
}

void
WifiEspNowBroadcastClass::end()
{
  WifiEspNow.end();
  WiFi.softAPdisconnect();
  m_ssid = "";
}

bool
WifiEspNowBroadcastClass::send(const uint8_t* buf, size_t count)
{
  return WifiEspNow.send(nullptr, buf, count);
}

void
WifiEspNowBroadcastClass::scan()
{
  m_isScanning = true;
#if defined(ESP8266)
  scan_config sc;
#elif defined(ESP32)
  wifi_scan_config_t sc;
#endif
  memset(&sc, 0, sizeof(sc));
  sc.ssid = reinterpret_cast<uint8_t*>(const_cast<char*>(m_ssid.c_str()));
#if defined(ESP8266)
  wifi_station_scan(&sc, reinterpret_cast<scan_done_cb_t>(WifiEspNowBroadcastClass::processScan));
#elif defined(ESP32)
  esp_wifi_scan_start(&sc, false);
#endif
}

#if defined(ESP8266)
void
WifiEspNowBroadcastClass::processScan(void* result, int status)
{
  WifiEspNowBroadcast.processScan2(result, status);
}

void
WifiEspNowBroadcastClass::processScan2(void* result, int status)

#define FOREACH_AP(f) \
  do { \
    for (bss_info* it = reinterpret_cast<bss_info*>(result); it; it = STAILQ_NEXT(it, next)) { \
      (f)(it->bssid, it->channel); \
    } \
  } while (false)

#define DELETE_APS \
  do {} while(false)

#elif defined(ESP32)
void
WifiEspNowBroadcastClass::processScan()

// ESP32 WiFiScanClass::_scanDone is always invoked after a scan complete event, so we can use
// Arduino's copy of AP records, but we must check SSID, and should not always delete AP records.

#define FOREACH_AP(f) \
  do { \
    int nNetworks = WiFi.scanComplete(); \
    for (uint8_t i = 0; static_cast<int>(i) < nNetworks; ++i) { \
      if (WiFi.SSID(i) != m_ssid) { \
        continue; \
      } \
      (f)(WiFi.BSSID(i), static_cast<uint8_t>(WiFi.channel(i))); \
    } \
  } while (false)

#define DELETE_APS \
  do { \
    bool hasOtherSsid = false; \
    int nNetworks = WiFi.scanComplete(); \
    for (uint8_t i = 0; static_cast<int>(i) < nNetworks; ++i) { \
      if (WiFi.SSID(i) == m_ssid) { \
        continue; \
      } \
      hasOtherSsid = true; \
      break; \
    } \
    if (!hasOtherSsid) { \
      WiFi.scanDelete(); \
    } \
  } while(false)

#endif
{
  m_isScanning = false;
  m_nextScan = millis() + m_scanFreq;
#ifdef ESP8266
  if (status != 0) {
    return;
  }
#endif

  const int MAX_PEERS = 20;
  WifiEspNowPeerInfo oldPeers[MAX_PEERS];
  int nOldPeers = std::min(WifiEspNow.listPeers(oldPeers, MAX_PEERS), MAX_PEERS);
  const uint8_t PEER_FOUND = 0xFF; // assigned to .channel to indicate peer is matched

  FOREACH_AP([&] (const uint8_t* bssid, uint8_t channel) {
    for (int i = 0; i < nOldPeers; ++i) {
      if (memcmp(bssid, oldPeers[i].mac, 6) != 0) {
        continue;
      }
      oldPeers[i].channel = PEER_FOUND;
      break;
    }
  });

  for (int i = 0; i < nOldPeers; ++i) {
    if (oldPeers[i].channel != PEER_FOUND) {
      WifiEspNow.removePeer(oldPeers[i].mac);
    }
  }

  FOREACH_AP([&] (const uint8_t* bssid, uint8_t channel) {
    WifiEspNow.addPeer(bssid, channel);
  });

  DELETE_APS;
}
