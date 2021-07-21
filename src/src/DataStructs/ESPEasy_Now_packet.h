#ifndef DATASTRUCTS_ESPEASY_NOW_INCOMING_H
#define DATASTRUCTS_ESPEASY_NOW_INCOMING_H

#include <Arduino.h>

#include "../Globals/ESPEasy_now_state.h"
#ifdef USES_ESPEASY_NOW

# include "MAC_address.h"
# include "ESPEasy_now_hdr.h"

# include <list>
# include <map>
class ESPEasy_Now_packet {
public:


  // Constructor for sending a packet
  // Actual allocated size may be lower than requested.
  explicit ESPEasy_Now_packet(const ESPEasy_now_hdr& header,
                              size_t                 payloadSize);

  ESPEasy_Now_packet() = default;

  ESPEasy_Now_packet(const ESPEasy_Now_packet& other) = delete;

  ESPEasy_Now_packet(ESPEasy_Now_packet&& other);

  ESPEasy_Now_packet& operator=(const ESPEasy_Now_packet& other) = delete;
  ESPEasy_Now_packet& operator=(ESPEasy_Now_packet&& other);

  bool ICACHE_FLASH_ATTR setReceivedPacket(const MAC_address& mac,
                         const uint8_t     *buf,
                         size_t             packetSize);

  // A packet may become invalid if it was not possible to allocate enough memory for the buffer
  bool ICACHE_FLASH_ATTR valid() const;

  bool            checksumValid() const;

  size_t          getSize() const;

  size_t          getPayloadSize() const;

  static size_t   getMaxPayloadSize();

  ESPEasy_now_hdr getHeader() const;

  void            setHeader(ESPEasy_now_hdr header);

  void            setMac(const MAC_address& mac);

  void            setBroadcast();

  size_t          addBinaryData(const uint8_t *data,
                                size_t         length,
                                size_t       & payload_pos);

  size_t getBinaryData(uint8_t *data,
                       size_t   length,
                       size_t & payload_pos) const;

  // Add a string to the packet, starting at payload position payload_pos
  // Return the number of bytes added (can be 1 more than the given string)
  size_t addString(const String& string,
                   size_t      & payload_pos);

  // Return a string starting from position pos in the buffer.
  // payload_pos will contain the new position to start for a next string
  String getString(size_t& payload_pos) const;

  // Get a pointer to the start of the string starting from position pos in the buffer.
  // The char pointer will be guaranteed null terminated.
  // payload_pos will contain the new position to start for a next string
  // @param str_length will contain the length of the found string
  const char* get_c_str(size_t& payload_pos,
                        size_t& str_length) const;

  // Get pointer to the begin of the payload
  const uint8_t * begin() const;

  const uint8_t * operator[](size_t idx) const {
    if (_valid) {
      return &_buf[idx];
    }
    return nullptr;
  }

  String getLogString() const;

  uint8_t _mac[6] = { 0 };

private:

  std::vector<uint8_t>_buf;

  bool _valid = false;

  void     setSize(size_t packetSize);

  uint16_t computeChecksum() const;
};

typedef std::list<ESPEasy_Now_packet> ESPEasy_Now_packet_list;

typedef std::map<uint8_t, ESPEasy_Now_packet> ESPEasy_Now_packet_map;


#endif // ifdef USES_ESPEASY_NOW

#endif // DATASTRUCTS_ESPEASY_NOW_INCOMING_H
