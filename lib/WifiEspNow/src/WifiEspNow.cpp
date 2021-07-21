#include "WifiEspNow.h"

#include <string.h>

#if defined(ESP8266)
#include <c_types.h>
#include <espnow.h>
#elif defined(ESP32)
#include <esp_now.h>
#else
#error "This library supports ESP8266 and ESP32 only."
#endif

WifiEspNowClass WifiEspNow;

WifiEspNowClass::WifiEspNowClass()
  : m_rxCb(nullptr)
  , m_rxCbArg(nullptr)
  , m_begin(false)
{
}

bool
WifiEspNowClass::begin()
{
  m_begin = esp_now_init() == 0 &&
#ifdef ESP8266
         esp_now_set_self_role(ESP_NOW_ROLE_COMBO) == 0 &&
#endif
         esp_now_register_recv_cb(reinterpret_cast<esp_now_recv_cb_t>(WifiEspNowClass::rx)) == 0 &&
         esp_now_register_send_cb(reinterpret_cast<esp_now_send_cb_t>(WifiEspNowClass::tx)) == 0;
  return m_begin;
}

void
WifiEspNowClass::end()
{
  if (m_begin) {
    esp_now_deinit();
    m_begin = false;
  }
}

int
WifiEspNowClass::listPeers(WifiEspNowPeerInfo* peers, int maxPeers) const
{
  int n = 0;
  if (m_begin) {
#if defined(ESP8266)
    for (u8* mac = esp_now_fetch_peer(true);
        mac != nullptr;
        mac = esp_now_fetch_peer(false)) {
      uint8_t channel = static_cast<uint8_t>(esp_now_get_peer_channel(mac));
#elif defined(ESP32)
    esp_now_peer_info_t peer;
    for (esp_err_t e = esp_now_fetch_peer(true, &peer);
        e == ESP_OK;
        e = esp_now_fetch_peer(false, &peer)) {
      uint8_t* mac = peer.peer_addr;
      uint8_t channel = peer.channel;
#endif
      if (n < maxPeers) {
        memcpy(peers[n].mac, mac, 6);
        peers[n].channel = channel;
      }
      ++n;
    }
  }
  return n;
}

bool
WifiEspNowClass::hasPeer(const uint8_t mac[6]) const
{
  if (m_begin) {
    return esp_now_is_peer_exist(const_cast<uint8_t*>(mac));
  }
  return false;
}

#if defined(ESP8266)
bool
WifiEspNowClass::addPeer(const uint8_t mac[6], int channel, const uint8_t key[WIFIESPNOW_KEYLEN])
{
  if (!m_begin) return false;
  if (this->hasPeer(mac)) {
    if (esp_now_get_peer_channel(const_cast<u8*>(mac)) == channel) {
      return true;
    }
    this->removePeer(mac);
  }
  return esp_now_add_peer(const_cast<u8*>(mac), ESP_NOW_ROLE_SLAVE, static_cast<u8>(channel),
                          const_cast<u8*>(key), key == nullptr ? 0 : WIFIESPNOW_KEYLEN) == 0;
}
#elif defined(ESP32)
bool
WifiEspNowClass::addPeer(const uint8_t mac[6], int channel, const uint8_t key[WIFIESPNOW_KEYLEN], int netif)
{
  if (!m_begin) return false;
  esp_now_peer_info_t pi;
  if (esp_now_get_peer(mac, &pi) == ESP_OK) {
    if (pi.channel == static_cast<uint8_t>(channel)) {
      return true;
    }
    this->removePeer(mac);
  }
  memset(&pi, 0, sizeof(pi));
  memcpy(pi.peer_addr, mac, ESP_NOW_ETH_ALEN);
  pi.channel = static_cast<uint8_t>(channel);
  pi.ifidx = static_cast<wifi_interface_t>(netif);
  if (key != nullptr) {
    memcpy(pi.lmk, key, ESP_NOW_KEY_LEN);
    pi.encrypt = true;
  }
  return esp_now_add_peer(&pi) == ESP_OK;
}
#endif

bool
WifiEspNowClass::removePeer(const uint8_t mac[6])
{
  if (!m_begin) return false;
  return esp_now_del_peer(const_cast<uint8_t*>(mac)) == 0;
}

void
WifiEspNowClass::onReceive(RxCallback cb, void* cbarg)
{
  m_rxCb = cb;
  m_rxCbArg = cbarg;
}

bool
WifiEspNowClass::send(const uint8_t mac[6], const uint8_t* buf, size_t count)
{
  if (!m_begin) return false;
  if (count > WIFIESPNOW_MAXMSGLEN || count == 0) {
    return false;
  }
  WifiEspNow.m_txRes = WifiEspNowSendStatus::NONE;
  return esp_now_send(const_cast<uint8_t*>(mac), const_cast<uint8_t*>(buf), static_cast<int>(count)) == 0;
}

void
WifiEspNowClass::rx(const uint8_t* mac, const uint8_t* data, uint8_t len)
{
  if (WifiEspNow.m_rxCb != nullptr) {
    (*WifiEspNow.m_rxCb)(mac, data, len, WifiEspNow.m_rxCbArg);
  }
}

void
WifiEspNowClass::tx(const uint8_t* mac, uint8_t status)
{
  WifiEspNow.m_txRes = status == 0 ? WifiEspNowSendStatus::OK : WifiEspNowSendStatus::FAIL;
}
