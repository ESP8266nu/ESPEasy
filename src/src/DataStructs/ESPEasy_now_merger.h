#ifndef DATASTRUCTS_ESPEASY_NOW_MERGER_H
#define DATASTRUCTS_ESPEASY_NOW_MERGER_H

#include "ESPEasy_Now_packet.h"

#ifdef USES_ESPEASY_NOW

# include "MAC_address.h"

// Class to process all incoming messages from a single sender.
// One or more packets form a complete message.
// This class only contains packets which have the same:
// - Sender mac
// - Message Count
// - Message Type
class ESPEasy_now_merger {
public:

  ESPEasy_now_merger();
  ESPEasy_now_merger(const ESPEasy_now_merger& other) = delete;
  ESPEasy_now_merger(ESPEasy_now_merger&& other) = default;

  ESPEasy_now_merger& operator=(ESPEasy_now_merger&& other) = default;

  void ICACHE_FLASH_ATTR addPacket(
    uint8_t            packet_nr,
    const MAC_address& mac,
    const uint8_t     *buf,
    size_t             packetSize);

  // Check if all parts of the packet have been received
  bool                                   messageComplete() const;

  // Check of set has expired (not all packets received within timeout)
  bool                                   expired() const;

  bool                                   valid() const;

  ESPEasy_Now_packet_map::const_iterator find(uint8_t packet_nr) const;

  // Get combined size of all packets.
  size_t                                 getPayloadSize() const;

  // Return a string starting from position pos in the buffer.
  // payload_pos will contain the new position to start for a next string
  String getString(size_t& payload_pos) const;
  bool   getString(String& string,
                   size_t& payload_pos) const;

  // Compute the length of a string starting from position pos in the buffer.
  // payload_pos will contain the new position to start for a next string
  size_t str_len(size_t& payload_pos) const;

  size_t getBinaryData(uint8_t *data,
                       size_t   length,
                       size_t & payload_pos) const;

  uint8_t         getExpectedNrPackets() const;
  ESPEasy_now_hdr::message_t getMessageType() const;

  bool            getMac(uint8_t *mac) const;
  bool            getMac(MAC_address& mac) const;

  String          getLogString() const;

  unsigned long   getFirstPacketTimestamp() const;

  bool getMessageCount(uint8_t& count) const;


private:

  // Find packet + payload position in packet for payload_pos of entire message
  uint8_t findPacketWithPayloadPos(size_t  payload_pos,
                                   size_t& payload_pos_in_packet) const;

  unsigned long _firstPacketTimestamp = 0;
  std::map<uint8_t, ESPEasy_Now_packet>_queue;
  uint8_t _nr_packets = 255;
};

#endif // ifdef USES_ESPEASY_NOW

#endif // DATASTRUCTS_ESPEASY_NOW_MERGER_H
