#include "../Helpers/ESPEasy_now_peermanager.h"

#ifdef USES_ESPEASY_NOW

# include "../ESPEasyCore/ESPEasy_Log.h"
# include "../ESPEasyCore/ESPEasyWifi.h"
# include "../Globals/Nodes.h"
# include "../Globals/SecuritySettings.h"


ESPEasy_now_peermanager_t::ESPEasy_now_peermanager_t() {
  removeAllPeers();
}

MAC_address ESPEasy_now_peermanager_t::getBroadcastMAC()
{
  static MAC_address ESPEasy_now_broadcast_MAC;

  if (!isBroadcastMAC(ESPEasy_now_broadcast_MAC)) {
    for (int i = 0; i < 6; ++i) {
      ESPEasy_now_broadcast_MAC.mac[i] = 0xFF;
    }
  }
  return ESPEasy_now_broadcast_MAC;
}

bool ESPEasy_now_peermanager_t::isBroadcastMAC(const MAC_address& mac)
{
  return mac.all_one();
}

bool ESPEasy_now_peermanager_t::addPeer(const MAC_address& mac, int channel, const uint8_t key[WIFIESPNOW_KEYLEN])
{
  if (!use_EspEasy_now || mac.all_zero()) {
    return false;
  }
  {
    // Don't add yourself as a peer
    MAC_address this_mac;
    WiFi.macAddress(this_mac.mac);

    if (this_mac == mac) { return false; }

    WiFi.softAPmacAddress(this_mac.mac);

    if (this_mac == mac) { return false; }
  }

  bool res = true;

  if (!isBroadcastMAC(mac)) {
    const NodeStruct *nodeInfo = Nodes.getNodeByMac(mac);

    if (nodeInfo != nullptr) {
      if (channel == 0) {
        channel = nodeInfo->channel;
      }
    }

    if (!WifiEspNow.hasPeer(mac.mac)) {
      res = WifiEspNow.addPeer(mac.mac, channel);

      if (!res && (activePeers.size() != 0)) {
        WifiEspNow.removePeer(activePeers.front().mac);
        activePeers.pop_front();
        res = WifiEspNow.addPeer(mac.mac, channel, key);
      }

      if (res) {
        activePeers.push_back(mac);
      } else {
        if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
          String log;
          if (log.reserve(64)) {
            log = F(ESPEASY_NOW_NAME);
            log += F(": Failed to add peer ");
            log += MAC_address(mac).toString();
            addLog(LOG_LEVEL_ERROR, log);
          }
        }
      }
    } else {
      // Move the MAC address to the back of the list as it is actively used
      if (activePeers.back() != mac) {
        auto it    = activePeers.begin();
        bool found = false;

        while (it != activePeers.end() && !found) {
          if (*it == mac) {
            found = true;
            activePeers.erase(it);
          }
          ++it;
        }
        activePeers.push_back(mac);
      }
    }
  }
  return res;
}

void ESPEasy_now_peermanager_t::removeAllPeers() {
  const int MAX_PEERS = 20;
  WifiEspNowPeerInfo oldPeers[MAX_PEERS];
  int nOldPeers = std::min(WifiEspNow.listPeers(oldPeers, MAX_PEERS), MAX_PEERS);

  for (int i = 0; i < nOldPeers; ++i) {
    WifiEspNow.removePeer(oldPeers[i].mac);
  }
  activePeers.clear();
}

void ESPEasy_now_peermanager_t::addKnownPeers()
{
  for (uint8_t peer = 0; peer < ESPEASY_NOW_PEER_MAX; ++peer) {
    if (SecuritySettings.peerMacSet(peer)) {
      addPeer(SecuritySettings.EspEasyNowPeerMAC[peer], 0);
    }
  }

  for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
    if (it->second.ESPEasyNowPeer) {
      addPeer(it->second.ESPEasy_Now_MAC(), it->second.channel);
    }
  }
}

#endif // ifdef USES_ESPEASY_NOW
