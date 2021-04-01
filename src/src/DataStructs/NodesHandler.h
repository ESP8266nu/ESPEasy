#ifndef DATASTRUCTS_NODESHANDLER_H
#define DATASTRUCTS_NODESHANDLER_H

#include "../DataStructs/NodeStruct.h"

#include "../DataStructs/MAC_address.h"

#ifdef USES_ESPEASY_NOW
#include "../DataStructs/ESPEasy_now_traceroute.h"
#include "../DataStructs/ESPEasy_now_Node_statistics.h"
#endif

class NodesHandler {
public:

  // Add node to the list of known nodes.
  // @retval true when the node was not yet present in the list.
  bool                     addNode(const NodeStruct& node);

#ifdef USES_ESPEASY_NOW
  bool                     addNode(const NodeStruct& node, const ESPEasy_now_traceroute_struct& traceRoute);
#endif


  bool                     hasNode(uint8_t unit_nr) const;

  bool                     hasNode(const uint8_t *mac) const;

  NodeStruct             * getNode(uint8_t unit_nr);
  const NodeStruct       * getNode(uint8_t unit_nr) const;

  NodeStruct             * getNodeByMac(const MAC_address& mac);
  const NodeStruct       * getNodeByMac(const MAC_address& mac) const;
  const NodeStruct       * getNodeByMac(const MAC_address& mac,
                                        bool             & match_STA) const;

  NodesMap::const_iterator begin() const;
  NodesMap::const_iterator end() const;
  NodesMap::const_iterator find(uint8_t unit_nr) const;

  // Remove nodes in list older than max_age_allowed (msec)
  // Returns oldest age, max_age (msec) not removed from the list.
  // Return true if a node has been removed.
  bool              refreshNodeList(unsigned long  max_age_allowed,
                                    unsigned long& max_age);

  
  const NodeStruct* getPreferredNode() const;
  const NodeStruct* getPreferredNode_notMatching(const MAC_address& not_matching) const;

#ifdef USES_ESPEASY_NOW
  const ESPEasy_now_traceroute_struct* getTraceRoute(uint8_t unit) const;
  const ESPEasy_now_traceroute_struct* getDiscoveryRoute(uint8_t unit) const;

  void setTraceRoute(const MAC_address& mac, const ESPEasy_now_traceroute_struct& traceRoute);
#endif

  // Update the node referring to this unit with the most recent info.
  void updateThisNode();

  const NodeStruct * getThisNode();

  uint8_t getDistance() const {
    return _distance;
  }

  bool lastTimeValidDistanceExpired() const;

  unsigned long get_lastTimeValidDistance() const {
    return _lastTimeValidDistance;
  }

  bool isEndpoint() const;

  uint8_t getESPEasyNOW_channel() const;

  bool recentlyBecameDistanceZero();

  void setRSSI(const MAC_address& mac, int rssi);

  void setRSSI(uint8_t unit, int rssi);

#ifdef USES_ESPEASY_NOW
  void updateSuccessRate(byte unit, bool success);
  void updateSuccessRate(const MAC_address& mac, bool success);

  int getRouteSuccessRate(byte unit, uint8_t& distance) const;

  uint8_t getSuccessRate(byte unit) const;
#endif

private:

  void setRSSI(NodeStruct * node, int rssi);

#ifdef USES_ESPEASY_NOW
  bool hasTraceRoute(uint8_t unit) const;
#endif

  unsigned long _lastTimeValidDistance = 0;

  uint8_t _distance = 255;  // Cached value

  NodesMap _nodes;

#ifdef USES_ESPEASY_NOW
  ESPEasy_now_Node_statisticsMap _nodeStats;
#endif

  bool _recentlyBecameDistanceZero = false;
};


#endif // DATASTRUCTS_NODESHANDLER_H
