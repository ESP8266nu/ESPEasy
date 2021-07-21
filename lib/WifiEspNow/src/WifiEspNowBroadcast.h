#ifndef WIFIESPNOW_BROADCAST_H
#define WIFIESPNOW_BROADCAST_H

#include "WifiEspNow.h"

#include <WString.h>

class WifiEspNowBroadcastClass
{
public:
  WifiEspNowBroadcastClass();

  /** \brief Initialize ESP-NOW with pseudo broadcast.
   *  \param ssid AP SSID to announce and find peers
   *  \param channel AP channel, used if there is no STA connection
   *  \param scanFreq how often to scan for peers, in millis
   *  \return whether success
   *
   *  In pseudo broadcast mode, every node announces itself as a group member by advertising a
   *  certain AP SSID. A node periodically scans other BSSIDs announcing the same SSID, and adds
   *  them as ESP-NOW peers. Messages are sent to all knows peers.
   *
   *  Pseudo broadcast does not depend on ESP-NOW API to support broadcast.
   */
  bool
  begin(const char* ssid, int channel = 1, int scanFreq = 15000);

  /** \brief Refresh peers if scanning is due.
   */
  void
  loop();

  /** \brief Stop ESP-NOW.
   */
  void
  end();

  /** \brief Set receive callback.
   *  \param cb the callback
   *  \param cbarg an arbitrary argument passed to the callback
   *  \note Only one callback is allowed; this replaces any previous callback.
   */
  void
  onReceive(WifiEspNowClass::RxCallback cb, void* cbarg)
  {
    WifiEspNow.onReceive(cb, cbarg);
  }

  /** \brief Broadcast a message.
   *  \param buf payload
   *  \param count payload size, must not exceed \p WIFIESPNOW_MAXMSGLEN
   *  \return whether success (message queued for transmission)
   */
  bool
  send(const uint8_t* buf, size_t count);

private:
  void
  scan();

#if defined(ESP8266)
  static void
  processScan(void* result, int status);

  void
  processScan2(void* result, int status);
#elif defined(ESP32)
  void
  processScan();
#endif

private:
  String m_ssid;
  int m_scanFreq;
  unsigned long m_nextScan;
  bool m_isScanning;
};

extern WifiEspNowBroadcastClass WifiEspNowBroadcast;

#endif // WIFIESPNOW_BROADCAST_H
