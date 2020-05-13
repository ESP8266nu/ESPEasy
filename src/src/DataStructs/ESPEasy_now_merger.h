#ifndef DATASTRUCTS_ESPEASY_NOW_MERGER_H
#define DATASTRUCTS_ESPEASY_NOW_MERGER_H

#include "ESPEasy_Now_packet.h"

#ifdef USES_ESPEASY_NOW


// Class to process all incoming messages from a single sender.
// One or more packets form a complete message.
// This class only contains packets which have the same:
// - Sender mac
// - Message Count
// - Message Type
class ESPEasy_now_merger {
public:

  ESPEasy_now_merger();

  void addPacket(
    uint8_t        packet_nr,
    const uint8_t  mac[6],
    const uint8_t *buf,
    size_t         packetSize);

  // Check if all parts of the packet have been received
  bool messageComplete() const;

  uint8_t receivedCount(uint8_t& nr_packets) const;

  // Check of set has expired (not all packets received within timeout)
  bool expired() const;

  ESPEasy_Now_packet_map::const_iterator find(uint8_t packet_nr) const;

  // Get combined size of all packets.
  size_t getPayloadSize() const;

  // Return a string starting from position pos in the buffer.
  // payload_pos will contain the new position to start for a next string
  String getString(size_t& payload_pos) const;

  size_t getBinaryData(uint8_t* data, size_t length) const;

  ESPEasy_now_hdr getFirstHeader() const;

  bool getMac(uint8_t* mac) const;

  String getLogString() const;

private:

  uint8_t findPacketWithPayloadPos(size_t  payload_pos,
                                   size_t& packet_start_payload_pos) const;

  unsigned long _firstPacketTimestamp = 0;
  std::map<uint8_t, ESPEasy_Now_packet> _queue;
  uint8_t _nr_packets = 255;
};

#endif // ifdef USES_ESPEASY_NOW

#endif // DATASTRUCTS_ESPEASY_NOW_MERGER_H