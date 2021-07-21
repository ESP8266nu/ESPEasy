#include "ESPEasy_now_hdr.h"

#ifdef USES_ESPEASY_NOW

const __FlashStringHelper *  ESPEasy_now_hdr::toString(ESPEasy_now_hdr::message_t messageType)
{
  switch (messageType) {
    case ESPEasy_now_hdr::message_t::NotSet: return F("-");
    case ESPEasy_now_hdr::message_t::Acknowledgement: return F("Ack");
    case ESPEasy_now_hdr::message_t::Announcement: return F("Ann");
    case ESPEasy_now_hdr::message_t::MQTTControllerMessage: return F("MQTTctr");
    case ESPEasy_now_hdr::message_t::NTP_Query: return F("NTP");
    case ESPEasy_now_hdr::message_t::SendData_DuplicateCheck: return F("DupChk");
    case ESPEasy_now_hdr::message_t::MQTTCheckControllerQueue: return F("MQTTchk");
    case ESPEasy_now_hdr::message_t::P2P_data: return F("p2p");
    case ESPEasy_now_hdr::message_t::TraceRoute: return F("tracert");
    case ESPEasy_now_hdr::message_t::ChecksumError: return F("ChkErr");
  }
  return F("?");
}

ESPEasy_now_hdr::ESPEasy_now_hdr() {}

ESPEasy_now_hdr::ESPEasy_now_hdr(ESPEasy_now_hdr::message_t messageType)
  : message_type(messageType)  {}

ESPEasy_now_hdr::ESPEasy_now_hdr(const uint8_t *buf)
{
  memcpy(this, buf, sizeof(ESPEasy_now_hdr));
}

ESPEasy_now_hdr& ESPEasy_now_hdr::operator=(const ESPEasy_now_hdr& other)
{
  if (&other == this) {
    return *this;
  }
  header_version = other.header_version;
  message_type   = other.message_type;
  packet_nr      = other.packet_nr;
  nr_packets     = other.nr_packets;
  message_count  = other.message_count;
  payload_size   = other.payload_size;
  checksum       = other.checksum;
  return *this;
}

#endif // ifdef USES_ESPEASY_NOW
