#include "ESPEasy_now_splitter.h"

#ifdef USES_ESPEASY_NOW

# include "../ESPEasyCore/ESPEasy_Log.h"
# include "../ESPEasyCore/ESPEasyWifi.h"
# include "../DataStructs/TimingStats.h"
# include "../Globals/Nodes.h"
# include "../Globals/Settings.h"
# include "../Globals/ESPEasy_now_peermanager.h"
# include "../Helpers/ESPEasy_time_calc.h"

static uint8_t ESPEasy_now_message_count = 1;

ESPEasy_now_splitter::ESPEasy_now_splitter(ESPEasy_now_hdr::message_t message_type, size_t totalSize)
  : _header(message_type), _totalSize(totalSize)
{
  _header.message_count = ++ESPEasy_now_message_count;
}

size_t ESPEasy_now_splitter::addBinaryData(const uint8_t *data, size_t length)
{
  if (data == nullptr || length == 0) {
    return 0;
  }
  size_t data_left = length;

  while ((data_left > 0) && (_totalSize > _bytesStored)) {
    if (!createNextPacket()) {
      return length - data_left;
    }
    const size_t bytesAdded = _queue.back().addBinaryData(data, data_left, _payload_pos);
    if (bytesAdded == 0) {
      return length - data_left;
    }
    data_left    -= bytesAdded;
    data         += bytesAdded;
    _bytesStored += bytesAdded;
  }
  return length;
}

size_t ESPEasy_now_splitter::addString(const String& string)
{
  size_t length = string.length() + 1; // Store the extra null-termination

  return addBinaryData(reinterpret_cast<const uint8_t *>(string.c_str()), length);
}

bool ESPEasy_now_splitter::createNextPacket()
{
  size_t current_PayloadSize = ESPEasy_Now_packet::getMaxPayloadSize();

  if (_queue.size() > 0) {
    current_PayloadSize = _queue.back().getPayloadSize();
  }

  if (current_PayloadSize > _payload_pos) {
    // No need to create a new file yet

    /*
       String log;
       log = F("createNextPacket ");
       log += data_left;
       addLog(LOG_LEVEL_INFO, log);
     */
    return true;
  }

  // Determine size of next packet
  size_t message_bytes_left = _totalSize - _bytesStored;
  _header.payload_size = message_bytes_left - sizeof(ESPEasy_now_hdr);

  ESPEasy_Now_packet newPacket(_header, message_bytes_left);
  if (!newPacket.valid()) {
    return false;
  }
  _queue.push_back(std::move(newPacket));
  _payload_pos = 0;

  // Set the packet number for the next packet.
  // Total packet count will be set right before sending them.
  _header.packet_nr++;
  return true;
}

bool ESPEasy_now_splitter::sendToBroadcast()
{
  return send(ESPEasy_now_peermanager.getBroadcastMAC(), 0);
}

bool ESPEasy_now_splitter::send(const MAC_address& mac,
                                int                channel)
{
  if (!prepareForSend(mac)) {
    return false;
  }

  const size_t nr_packets = _queue.size();

  for (uint8_t i = 0; i < nr_packets; ++i) {
    if (!send(_queue[i], channel)) { return false; }
  }
  return true;
}

WifiEspNowSendStatus ESPEasy_now_splitter::send(const MAC_address& mac, size_t timeout, int channel)
{
  START_TIMER;
  if (!prepareForSend(mac)) {
    return WifiEspNowSendStatus::FAIL;
  }

  WifiEspNowSendStatus sendStatus = WifiEspNowSendStatus::NONE;
  const size_t nr_packets = _queue.size();

  for (uint8_t i = 0; i < nr_packets; ++i) {
    uint8_t retry = 2;

    while (retry > 0) {
      --retry;
      send(_queue[i], channel);
      sendStatus = waitForSendStatus(timeout);

      if (sendStatus == WifiEspNowSendStatus::OK) {
        retry = 0;
      }
      # ifndef BUILD_NO_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
        String log;
        log.reserve(85);

        switch (sendStatus) {
          case WifiEspNowSendStatus::NONE:
          {
            log += F(ESPEASY_NOW_NAME);
            log += F(": TIMEOUT to: ");
            break;
          }
          case WifiEspNowSendStatus::FAIL:
          {
            log += F(ESPEASY_NOW_NAME);
            log += F(": Sent FAILED to: ");
            break;
          }
          case WifiEspNowSendStatus::OK:
          {
            log += F(ESPEASY_NOW_NAME);
            log += F(": Sent to: ");
            break;
          }
        }
        log += _queue[i].getLogString();
        addLog(LOG_LEVEL_DEBUG, log);
      }
      # endif // ifndef BUILD_NO_DEBUG

      if (retry != 0) {
        delay(10);
      }
    }

    switch (sendStatus) {
      case WifiEspNowSendStatus::NONE:
      case WifiEspNowSendStatus::FAIL:
      {
        Nodes.updateSuccessRate(mac, false);
        STOP_TIMER(ESPEASY_NOW_SEND_MSG_FAIL);
        return sendStatus;
      }
      case WifiEspNowSendStatus::OK:
      {
        Nodes.updateSuccessRate(mac, true);
        ESPEasy_now_peermanager.addPeer(mac, channel);
        break;
      }
    }
  }
  STOP_TIMER(ESPEASY_NOW_SEND_MSG_SUC);
  return sendStatus;
}

bool ESPEasy_now_splitter::send(const ESPEasy_Now_packet& packet, int channel)
{
  bool res = false;
  START_TIMER;
  if (ESPEasy_now_peermanager.addPeer(packet._mac, channel)) {
    // Set TX power based on RSSI of other ESPEasy-NOW node.
    // For broadcast messages power must be max.
    float tx_pwr = 30; // Will be set higher based on RSSI when needed.
    int8_t rssi = -99; // Assume worst RSSI for broadcast
    SetWiFiTXpower(tx_pwr, rssi);
    if (packet.valid()) {
      res = WifiEspNow.send(packet._mac, packet[0], packet.getSize());
    }
  }

  STOP_TIMER(ESPEASY_NOW_SEND_PCKT);

  delay(0);
  return res;
}

WifiEspNowSendStatus ESPEasy_now_splitter::waitForSendStatus(size_t timeout) const
{
  WifiEspNowSendStatus sendStatus = WifiEspNowSendStatus::NONE;

  if (timeout < 20) {
    // ESP-now keeps sending for 20 msec using lower transfer rated every 2nd attempt.
    timeout = 20;
  }

  size_t timer = millis() + timeout;

  while (!timeOutReached(timer) && sendStatus == WifiEspNowSendStatus::NONE) {
    sendStatus = WifiEspNow.getSendStatus();
    delay(1);
  }
  return sendStatus;
}

bool ESPEasy_now_splitter::prepareForSend(const MAC_address& mac)
{
  if (!use_EspEasy_now) return false;
  size_t nr_packets = _queue.size();

  for (uint8_t i = 0; i < nr_packets; ++i) {
    ESPEasy_now_hdr header = _queue[i].getHeader();
    header.nr_packets = nr_packets;
    header.payload_size = _queue[i].getPayloadSize();
    _queue[i].setHeader(header);
    _queue[i].setMac(mac);
    if (!_queue[i].valid()) {
      if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
        addLog(LOG_LEVEL_ERROR, F(ESPEASY_NOW_NAME ": Could not prepare for send"));
      }
      return false;
    }
  }
  return true;
}


#endif // ifdef USES_ESPEASY_NOW
