#ifndef DATASTRUCTS_UNITMESSAGECOUNT_H
#define DATASTRUCTS_UNITMESSAGECOUNT_H

#include "../../ESPEasy_common.h"

#include <map>

// For deduplication, some controllers may add a unit ID and current counter.
// This count will wrap around, so it is just to detect if a message is received multiple times.
// The unit ID is the unit where the message originates from and thus should be kept along when forwarding.
struct MessageRouteInfo_t {
  typedef std::vector<uint8_t> uint8_t_vector;


  MessageRouteInfo_t() {}

  MessageRouteInfo_t(uint8_t unitnr, uint8_t messageCount) : unit(unitnr), count(messageCount) {}

  MessageRouteInfo_t(const uint8_t* serializedData, size_t size);

  MessageRouteInfo_t(const uint8_t_vector& serializedData);

  MessageRouteInfo_t(const MessageRouteInfo_t& other) = default;
  MessageRouteInfo_t(MessageRouteInfo_t&& other) = default;
  MessageRouteInfo_t& operator=(const MessageRouteInfo_t& other) = default;
  MessageRouteInfo_t& operator=(MessageRouteInfo_t&& other) = default;

  bool deserialize(const uint8_t_vector& serializedData);
  bool deserialize(const uint8_t* serializedData, size_t size);

  uint8_t_vector serialize() const;

  size_t getSerializedSize() const;

  bool appendUnit(uint8_t unitnr);

  bool traceHasUnit(uint8_t unitnr) const;

  size_t countUnitInTrace(uint8_t unitnr) const;

  uint8_t getLastUnitNotMatching(uint8_t unitnr) const;

  String toString() const;

  // Source unit
  uint8_t unit  = 0; // Initialize to "not set"

  // Message counter of the source unit, used to check for duplicates.
  uint8_t count = 0;

  // Destination unit (not yet used)
  // Can be useful for sending messages to other units in the mesh
  uint8_t dest_unit  = 0; // Initialize to "not set"

  // List of all unit numbers that may have forwarded the message.
  // Used to detect loops
  uint8_t_vector trace;
};

struct UnitMessageRouteInfo_map {
  bool isNew(const MessageRouteInfo_t *count) const;

  void add(const MessageRouteInfo_t *count);

private:

  std::map<uint8_t, MessageRouteInfo_t> _map;
};

#endif // ifndef DATASTRUCTS_UNITMESSAGECOUNT_H
