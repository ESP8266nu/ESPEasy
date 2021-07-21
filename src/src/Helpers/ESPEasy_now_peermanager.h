#ifndef HELPERS_ESPEASY_NOW_PEERMANAGER_H
#define HELPERS_ESPEASY_NOW_PEERMANAGER_H

#include "../../ESPEasy_common.h"
#include "../Globals/ESPEasy_now_state.h"
#ifdef USES_ESPEASY_NOW
# include "../DataStructs/MAC_address.h"

# include <list>

struct ESPEasy_now_peermanager_t {
  ESPEasy_now_peermanager_t();

  // Return FF:FF:FF:FF:FF:FF
  static MAC_address getBroadcastMAC();

  static bool isBroadcastMAC(const MAC_address& mac);

  // ESP-now does send broadcast packets to known peers (and those that still may receive it)
  // This makes broadcast packets much more reliable when the intended recipient is a known peer
  // However we only have a limited number of peer slots (depending on whether or not an encryption key is used), 
  // so we must keep track of 'popular' nodes to keep those active as peer
  bool addPeer(const MAC_address& mac,
               int                channel,
               const uint8_t      key[WIFIESPNOW_KEYLEN] = nullptr);

  void removeAllPeers();

  // Add preferred peers from the settings and known nodes.
  // Nodes will be added as last, as those are likely to be more up-to-date.
  void addKnownPeers();

private:

  // Keep track of active peers so we can remove the oldest when the max. nr. of peers is reached.
  std::list<MAC_address> activePeers;
};
#endif // ifdef USES_ESPEASY_NOW
#endif // ifndef HELPERS_ESPEASY_NOW_PEERMANAGER_H
