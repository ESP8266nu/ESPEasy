#ifndef DATASTRUCTS_ESPEASY_NOW_SPLITTER_H
#define DATASTRUCTS_ESPEASY_NOW_SPLITTER_H

#include "ESPEasy_Now_packet.h"

#ifdef USES_ESPEASY_NOW

class ESPEasy_now_splitter {
public:

  ESPEasy_now_splitter(ESPEasy_now_hdr::message_t message_type,
                       size_t                     totalSize);

  size_t               addBinaryData(const uint8_t *data,
                                     size_t         length);

  size_t               addString(const String& string);

  bool                 sendToBroadcast();
  bool                 send(const MAC_address& mac,
                            int                channel = 0);

  WifiEspNowSendStatus send(const MAC_address& mac,
                            size_t             timeout,
                            int                channel);

private:

  // Create next packet when needed.
  // return false when it was needed, but failed to do so.
  bool                 createNextPacket();

  size_t               getPayloadPos() const;

  bool                 send(const ESPEasy_Now_packet& packet,
                            int                       channel);

  bool                 prepareForSend(const MAC_address& mac);


  WifiEspNowSendStatus waitForSendStatus(size_t timeout) const;

  std::vector<ESPEasy_Now_packet>_queue;
  ESPEasy_now_hdr _header;
  size_t _payload_pos     = 255; // Position in the last packet where we left of.
  const size_t _totalSize = 0;   // Total size as we intend to send.
  size_t _bytesStored     = 0;   // Total number of bytes already stored as payload
};

#endif // ifdef USES_ESPEASY_NOW

#endif // DATASTRUCTS_ESPEASY_NOW_SPLITTER_H
