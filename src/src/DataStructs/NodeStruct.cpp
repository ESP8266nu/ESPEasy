#include "NodeStruct.h"

#include "../../ESPEasy-Globals.h"
#include "../Globals/SecuritySettings.h"
#include "../Globals/Settings.h"
#include "../Helpers/ESPEasy_time_calc.h"

#define NODE_STRUCT_AGE_TIMEOUT 300000  // 5 minutes

NodeStruct::NodeStruct() : ESPEasyNowPeer(0), useAP_ESPEasyNow(0), scaled_rssi(0)
{}

bool NodeStruct::valid() const {
  // FIXME TD-er: Must make some sanity checks to see if it is a valid message
  return true;
}

bool NodeStruct::validate() {
  if (build < 20107) {
    // webserverPort introduced in 20107
    webgui_portnumber = 80;
    for (uint8_t i = 0; i < 6; ++i) {
      ap_mac[i] = 0;
    }
    load              = 0;
    distance          = 255;
    timeSource        = static_cast<uint8_t>(timeSource_t::No_time_source);
    channel           = 0;
    ESPEasyNowPeer    = 0;
    useAP_ESPEasyNow  = 0;
    setRSSI(0);
    lastUpdated = 0;
  }

  // FIXME TD-er: Must make some sanity checks to see if it is a valid message
  return valid();
}

bool NodeStruct::operator<(const NodeStruct &other) const {
  const bool thisExpired = isExpired();
  if (thisExpired != other.isExpired()) {
    return !thisExpired;
  }

  const bool markedAsPriority = markedAsPriorityPeer();
  if (markedAsPriority != other.markedAsPriorityPeer()) {
    return markedAsPriority;
  }

  if (ESPEasyNowPeer != other.ESPEasyNowPeer) {
    // One is confirmed, so prefer that one.
    return ESPEasyNowPeer;
  }

  const int8_t thisRssi = getRSSI();
  const int8_t otherRssi = other.getRSSI();

  int score_this = getLoad();
  int score_other = other.getLoad();

  if (distance != other.distance) {
    if (!isExpired() && !other.isExpired()) {
      // Distance is not the same, so take distance into account.
      return distance < other.distance;
/*
      int distance_penalty = distance - other.distance;
      distance_penalty = distance_penalty * distance_penalty * 10;
      if (distance > other.distance) {
        score_this += distance_penalty;
      } else {
        score_other += distance_penalty;
      }
*/
    }
  }

  if (thisRssi >= 0 || otherRssi >= 0) {
    // One or both have no RSSI, so cannot use RSSI in computing score
  } else {
    // RSSI value is negative, so subtract the value
    // RSSI range from -38 ... 99
    // Shift RSSI and add a weighing factor to make sure
    // A load of 100% with RSSI of -40 is preferred over a load of 20% with an RSSI of -80.
    score_this -= (thisRssi + 38) * 2;
    score_other -= (otherRssi + 38) * 2;
  }
  return score_this < score_other;
}


const __FlashStringHelper * NodeStruct::getNodeTypeDisplayString(uint8_t nodeType) {
  switch (nodeType)
  {
    case NODE_TYPE_ID_ESP_EASY_STD:     return F("ESP Easy");
    case NODE_TYPE_ID_ESP_EASYM_STD:    return F("ESP Easy Mega");
    case NODE_TYPE_ID_ESP_EASY32_STD:   return F("ESP Easy 32");
    case NODE_TYPE_ID_RPI_EASY_STD:     return F("RPI Easy");
    case NODE_TYPE_ID_ARDUINO_EASY_STD: return F("Arduino Easy");
    case NODE_TYPE_ID_NANO_EASY_STD:    return F("Nano Easy");
  }
  return F("");
}

const __FlashStringHelper * NodeStruct::getNodeTypeDisplayString() const {
  return NodeStruct::getNodeTypeDisplayString(nodeType);
}

String NodeStruct::getNodeName() const {
  String res;
  size_t length = strnlen(reinterpret_cast<const char *>(nodeName), sizeof(nodeName));

  res.reserve(length);

  for (size_t i = 0; i < length; ++i) {
    res += static_cast<char>(nodeName[i]);
  }
  return res;
}

IPAddress NodeStruct::IP() const {
  return IPAddress(ip[0], ip[1], ip[2], ip[3]);
}

MAC_address NodeStruct::STA_MAC() const {
  return MAC_address(sta_mac);
}

MAC_address NodeStruct::ESPEasy_Now_MAC() const {
  if (ESPEasyNowPeer == 0) return MAC_address();
  if (useAP_ESPEasyNow) {
    return MAC_address(ap_mac);
  }
  return MAC_address(sta_mac);
}

unsigned long NodeStruct::getAge() const {
  return timePassedSince(lastUpdated);
}

bool  NodeStruct::isExpired() const {
  return getAge() > NODE_STRUCT_AGE_TIMEOUT;
}

float NodeStruct::getLoad() const {
  return load / 2.55;
}

String NodeStruct::getSummary() const {
  String res;

  res.reserve(48);
  res  = F("Unit: ");
  res += unit;
  res += F(" \"");
  res += getNodeName();
  res += '"';
  res += F(" load: ");
  res += String(getLoad(), 1);
  res += F(" RSSI: ");
  res += getRSSI();
  res += F(" ch: ");
  res += channel;
  res += F(" dst: ");
  res += distance;
  return res;
}

bool NodeStruct::setESPEasyNow_mac(const MAC_address& received_mac)
{
  if (received_mac.all_zero()) return false;
  if (received_mac == sta_mac) {
    ESPEasyNowPeer   = 1;
    useAP_ESPEasyNow = 0;
    return true;
  }

  if (received_mac == ap_mac) {
    ESPEasyNowPeer   = 1;
    useAP_ESPEasyNow = 1;
    return true;
  }
  return false;
}

int8_t NodeStruct::getRSSI() const
{
  if (scaled_rssi == 0) {
    return 0; // Not set
  }

  if (scaled_rssi == 0x3F) {
    return 31; // Error state
  }

  // scaled_rssi = 1 ... 62
  // output = -38 ... -99
  int8_t rssi = scaled_rssi + 37;
  return rssi * -1;
}

void NodeStruct::setRSSI(int8_t rssi)
{
  if (rssi == 0) {
    // Not set
    scaled_rssi = 0;
    return;
  }

  if (rssi > 0) {
    // Error state
    scaled_rssi = 0x3F;
    return;
  }
  rssi *= -1;
  rssi -= 37;

  if (rssi < 1) {
    scaled_rssi = 1;
    return;
  }

  if (rssi >= 0x3F) {
    scaled_rssi = 0x3F - 1;
    return;
  }
  scaled_rssi = rssi;
}

bool NodeStruct::markedAsPriorityPeer() const
{
  for (int i = 0; i < ESPEASY_NOW_PEER_MAX; ++i) {
    if (SecuritySettings.peerMacSet(i)) {
      if (match(SecuritySettings.EspEasyNowPeerMAC[i])) {
        return true;
      }
    }
  }
  return false;
}

bool NodeStruct::match(const MAC_address& mac) const
{
  return (mac == sta_mac || mac == ap_mac);
}

bool NodeStruct::isThisNode() const
{
    // Check to see if we process a node we've sent ourselves.
    MAC_address tmp;
    WiFi.softAPmacAddress(tmp.mac);
    if (tmp == ap_mac) return true;
    WiFi.macAddress(tmp.mac);
    if (tmp == sta_mac) return true;

    return false;
}

void NodeStruct::setAP_MAC(const MAC_address& mac)
{
  mac.get(ap_mac);
}