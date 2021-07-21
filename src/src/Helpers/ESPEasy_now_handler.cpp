#include "ESPEasy_now_handler.h"

#ifdef USES_ESPEASY_NOW

# include "../../ESPEasy_fdwdecl.h"
# include "../ControllerQueue/MQTT_queue_element.h"
# include "../DataStructs/ESPEasy_Now_DuplicateCheck.h"
# include "../DataStructs/ESPEasy_Now_packet.h"
# include "../DataStructs/ESPEasy_now_splitter.h"
# include "../DataStructs/NodeStruct.h"
# include "../DataStructs/TimingStats.h"
# include "../ESPEasyCore/Controller.h"
# include "../ESPEasyCore/ESPEasyWifi.h"
# include "../ESPEasyCore/ESPEasy_Log.h"
# include "../Globals/ESPEasy_now_peermanager.h"
# include "../Globals/ESPEasyWiFiEvent.h"
# include "../Globals/ESPEasy_time.h"
# include "../Globals/MQTT.h"
# include "../Globals/Nodes.h"
# include "../Globals/RTC.h"
# include "../Globals/SecuritySettings.h"
# include "../Globals/SendData_DuplicateChecker.h"
# include "../Globals/Settings.h"
# include "../Globals/WiFi_AP_Candidates.h"
# include "../Helpers/CRC_functions.h"
# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/ESPEasy_time_calc.h"
# include "../Helpers/ESPEasyMutex.h"
# include "../Helpers/PeriodicalActions.h"
# include "../Helpers/_CPlugin_Helper.h"


# include <list>


# define ESPEASY_NOW_ACTIVITY_TIMEOUT      125000 // 2 minutes + 5 sec
# define ESPEASY_NOW_SINCE_LAST_BROADCAST   65000 // 1 minute + 5 sec to start sending a node directly
# define ESPEASY_NOW_CHANNEL_SEARCH_TIMEOUT  5000 // Time to wait for announcement packets on a single channel

# define ESPEASY_NOW_MAX_CHANNEL               13

# define ESPEASY_NOW_TMP_SSID       "ESPEASY_NOW"
# define ESPEASY_NOW_TMP_PASSPHRASE "random_passphrase"

static uint64_t ICACHE_FLASH_ATTR mac_to_key(const uint8_t *mac, ESPEasy_now_hdr::message_t messageType, uint8_t message_count)
{
  uint64_t key = message_count;

  key  = key << 8;
  key += static_cast<uint8_t>(messageType);

  for (uint8_t i = 0; i < 6; ++i) {
    key  = key << 8;
    key += mac[i];
  }
  return key;
}

ESPEasy_Mutex ESPEasy_now_in_queue_mutex;
std::map<uint64_t, ESPEasy_now_merger> ESPEasy_now_in_queue;

std::list<ESPEasy_now_traceroute_struct> ESPEasy_now_traceroute_queue;
std::list<MAC_address> ESPEasy_now_MQTT_check_queue;

void ICACHE_FLASH_ATTR ESPEasy_now_onReceive(const uint8_t mac[6], const uint8_t *buf, size_t count, void *cbarg) {
  START_TIMER;
  const size_t payload_length  = count - sizeof(ESPEasy_now_hdr);
  if (count < sizeof(ESPEasy_now_hdr) || (payload_length > ESPEasy_Now_packet::getMaxPayloadSize())) {
    STOP_TIMER(INVALID_ESPEASY_NOW_LOOP);
    return; // Too small
  }

  ESPEasy_now_hdr header(buf);

  if (header.header_version != ESPEASY_NOW_HEADER_VERSION) {
    STOP_TIMER(INVALID_ESPEASY_NOW_LOOP);
    return;
  }

  const uint8_t *payload = buf + sizeof(ESPEasy_now_hdr);
  const uint16_t checksum = calc_CRC16(reinterpret_cast<const char *>(payload), payload_length);

  if (header.checksum != checksum) {
    STOP_TIMER(INVALID_ESPEASY_NOW_LOOP);
    return;
  }
  const uint64_t key = mac_to_key(mac, header.message_type, header.message_count);

  {
    ESPEasy_now_in_queue_mutex.lock();
    ESPEasy_now_in_queue[key].addPacket(header.packet_nr, mac, buf, count);
    ESPEasy_now_in_queue_mutex.unlock();
  }
  STOP_TIMER(RECEIVE_ESPEASY_NOW_LOOP);
}

bool ESPEasy_now_handler_t::begin()
{
  if (!Settings.UseESPEasyNow()) { return false; }
  if (use_EspEasy_now) { return true; }
  if (WiFi.scanComplete() == WIFI_SCAN_RUNNING || !WiFiEventData.processedScanDone) { return false;}
  if (WiFiEventData.wifiConnectInProgress) {
    /*
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      if (log.reserve(64)) {
        log = F(ESPEASY_NOW_NAME);
        log += F(": wifiConnectInProgress");
        addLog(LOG_LEVEL_INFO, log);
      }
    }
*/
    return false;
  }
  if (temp_disable_EspEasy_now_timer != 0) {
    if (!timeOutReached(temp_disable_EspEasy_now_timer)) {
      return false;
    } else {
      temp_disable_EspEasy_now_timer = 0;
    }
  }

  _usedWiFiChannel = Nodes.getESPEasyNOW_channel();
  if (_usedWiFiChannel == 0) {
    _scanChannelsMode = true;
    ++_lastScannedChannel;
    if (_lastScannedChannel > ESPEASY_NOW_MAX_CHANNEL) {
      _lastScannedChannel = 1;
    }
    _usedWiFiChannel = _lastScannedChannel;
  } else {
    _scanChannelsMode = false;
  }

  _last_traceroute_sent = 0;
  _last_traceroute_received = 0;
  _last_used = millis();
  _last_started = millis();
  _controllerIndex = INVALID_CONTROLLER_INDEX;

  if (isESPEasy_now_only()) {
    setConnectionSpeed();
  }

  const String ssid       = F(ESPEASY_NOW_TMP_SSID);
  const String passphrase = F(ESPEASY_NOW_TMP_PASSPHRASE);

  setAP(true);
  #ifdef ESP32
  // We're running a dummy AP, so don't run DHCP server.
  tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
  #endif

  // Make sure AP will not be turned off.
  WiFiEventData.timerAPoff.clear();
  WiFiEventData.timerAPstart.clear();


  int ssid_hidden    = 1;
  int max_connection = 6;
  WiFi.softAP(ssid.c_str(), passphrase.c_str(), _usedWiFiChannel, ssid_hidden, max_connection);

  //    WiFi.softAPdisconnect(false);

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F(ESPEASY_NOW_NAME ": begin on channel ");
    log += _usedWiFiChannel;
    addLog(LOG_LEVEL_INFO, log);
  }

  if (!WifiEspNow.begin()) {
    addLog(LOG_LEVEL_ERROR, F(ESPEASY_NOW_NAME ": Failed to initialize ESPEasy-NOW"));
    return false;
  }
  ESPEasy_now_peermanager.removeAllPeers();
  ESPEasy_now_peermanager.addKnownPeers();

  WifiEspNow.onReceive(ESPEasy_now_onReceive, nullptr);

  use_EspEasy_now = true;
  addLog(LOG_LEVEL_INFO, F(ESPEASY_NOW_NAME " enabled"));

  if (_scanChannelsMode) {
    WiFiEventData.lastScanMoment.clear();
  }

  if (_scanChannelsMode) {
    WifiScan(false, _usedWiFiChannel);
    // Send discovery announce to all found hidden APs using the current channel
    for (auto it = WiFi_AP_Candidates.scanned_begin(); it != WiFi_AP_Candidates.scanned_end(); ++it) {
      if (it->isHidden && it->channel == _usedWiFiChannel) {
        const MAC_address mac(it->bssid);
        sendDiscoveryAnnounce(mac, _usedWiFiChannel);
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F(ESPEASY_NOW_NAME ": Send discovery to ");
          log += mac.toString();
          addLog(LOG_LEVEL_INFO, log);
        }
      }
    }
  } else {
    sendDiscoveryAnnounce();
  }
  
  return true;
}

void ESPEasy_now_handler_t::end()
{
  _controllerIndex = INVALID_CONTROLLER_INDEX;
  _usedWiFiChannel = 0;
  use_EspEasy_now  = false;
  RTC.clearLastWiFi(); // Force a WiFi scan
  if (_last_used != 0) {
    // Only call WifiEspNow.end() if it was started.
    WifiEspNow.end();
    _last_used = 0;
    _last_started = 0;
  }
  {
    ESPEasy_now_in_queue_mutex.lock();
    ESPEasy_now_in_queue.clear();
    ESPEasy_now_in_queue_mutex.unlock();
  }
  addLog(LOG_LEVEL_INFO, F(ESPEASY_NOW_NAME " disabled"));
}

bool ESPEasy_now_handler_t::loop()
{
  loop_check_ESPEasyNOW_run_state();
  if (!use_EspEasy_now) return false;
  bool somethingProcessed = loop_process_ESPEasyNOW_in_queue();

  loop_process_ESPEasyNOW_send_queue();

  if (_send_failed_count > 30 /*|| !active()*/) {
    _send_failed_count = 0;
    // FIXME TD-er: Must check/mark so this becomes true: isESPEasy_now_only()

    // Start scanning the next channel to see if we may end up with a new found node
    //    WifiScan(false, false);
    //    addPeerFromWiFiScan();
    //    _last_used = millis();
    end();
    begin();
  }
  return somethingProcessed;
}

void ESPEasy_now_handler_t::loop_check_ESPEasyNOW_run_state()
{
  const uint8_t ESPEasyNOW_channel = Nodes.getESPEasyNOW_channel();
  const bool channelChanged =  _usedWiFiChannel != ESPEasyNOW_channel;

  if (!WifiIsAP(WiFi.getMode()) || (channelChanged && !Nodes.isEndpoint())) {
    // AP mode may be turned off externally, or WiFi channel may have changed.
    // If so restart ESPEasy-now handler
    if (use_EspEasy_now) {
      if (ESPEasyNOW_channel == 0) {
        // We need to give it some time to receive announcement messages and maybe even a traceroute
        if (timePassedSince(_last_started) > ESPEASY_NOW_CHANNEL_SEARCH_TIMEOUT) {
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log;
            if (log.reserve(80)) {
              log = F(ESPEASY_NOW_NAME);
              log += F(": No peers with distance set on channel ");
              log += _usedWiFiChannel;
              addLog(LOG_LEVEL_INFO, log);
            }
          }

          end();
        }
      } else {
        if (channelChanged) {
          if (loglevelActiveFor(LOG_LEVEL_INFO)) {
            String log;
            if (log.reserve(64)) {
              log = F(ESPEASY_NOW_NAME);
              log += F(": Move to channel ");
              log += ESPEasyNOW_channel;
              addLog(LOG_LEVEL_INFO, log);
            }
          }
        }

        end();
      }
    }
  }

  if (use_EspEasy_now) {
    if (!Nodes.isEndpoint()) {
      const bool traceroute_received_timeout = _last_traceroute_received != 0 && (timePassedSince(_last_traceroute_received) > ESPEASY_NOW_ACTIVITY_TIMEOUT + 30000);
      const bool traceroute_sent_timeout     = _last_traceroute_sent != 0 && (timePassedSince(_last_traceroute_sent) > ESPEASY_NOW_ACTIVITY_TIMEOUT);
      const bool first_traceroute_receive_timeout = _last_traceroute_received == 0 && (timePassedSince(_last_started) > ESPEASY_NOW_ACTIVITY_TIMEOUT + 30000);
      if (traceroute_received_timeout || traceroute_sent_timeout || first_traceroute_receive_timeout) {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log;
          if (log.reserve(64)) {
            log = F(ESPEASY_NOW_NAME);
            log += F(": Inactive due to not receiving trace routes");
            addLog(LOG_LEVEL_INFO, log);
          }
        }
        end();
        temp_disable_EspEasy_now_timer = millis() + 10000;
        WifiScan(true);
        return;
      }
    }
  }


  if (Settings.UseESPEasyNow() != use_EspEasy_now) {
    if (!use_EspEasy_now) {
      begin();
    } else {
      end();
    }
  }
}

bool ESPEasy_now_handler_t::loop_process_ESPEasyNOW_in_queue()
{
  bool somethingProcessed = false;

  if (!ESPEasy_now_in_queue.empty()) {
    unsigned long timeout = millis() + 20;
    for (auto it = ESPEasy_now_in_queue.begin(); !timeOutReached(timeout) && it != ESPEasy_now_in_queue.end();) {
      const bool expired = it->second.expired();
      bool removeMessage = true;
      START_TIMER;

      bool valid = it->second.valid();
      if (!valid || !it->second.messageComplete()) {
        if (!valid || expired) {
          if (expired) {
            STOP_TIMER(EXPIRED_ESPEASY_NOW_LOOP);
          } else {
            STOP_TIMER(INVALID_ESPEASY_NOW_LOOP);
          }

          if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
            String log;
            if (log.reserve(85)) {
              log = F(ESPEASY_NOW_NAME);
              log += F(": ");
              log += it->second.getLogString();
              if (expired) {
                log += F(" Expired!");
              } else {
                log += F(" Invalid!");
              }
              addLog(LOG_LEVEL_ERROR, log);
            }
          }
        } else {
          if (!expired) {
            removeMessage = false;
          }
        }
      } else if (!expired) {
        // Process it
        bool mustKeep = !removeMessage;
        somethingProcessed = processMessage(it->second, mustKeep);
        removeMessage      = !mustKeep;
        STOP_TIMER(HANDLE_ESPEASY_NOW_LOOP);
      }

      if (removeMessage) {
        ESPEasy_now_in_queue_mutex.lock();
        it = ESPEasy_now_in_queue.erase(it);
        ESPEasy_now_in_queue_mutex.unlock();

/*
        // FIXME TD-er: For now only process one item and then wait for the next loop.
        if (somethingProcessed) {
          return true;
        }
*/
      } else {
        ++it;
      }
    }
  }
  return somethingProcessed;
}

void ESPEasy_now_handler_t::loop_process_ESPEasyNOW_send_queue()
{
  // Try to pace the broadcasts of traceroutes and sending MQTT checks
  // Only process one every 100 msec.
  static unsigned long last_queue_processed = 0;
  if (timePassedSince(last_queue_processed) > 100) {
    if (!ESPEasy_now_traceroute_queue.empty()) {
      ESPEasy_now_traceroute_queue.sort();
      const ESPEasy_now_traceroute_struct route = ESPEasy_now_traceroute_queue.front();
      sendTraceRoute(route);
      _last_traceroute_received = millis();
      last_queue_processed = millis();
      // Remove possible duplicate routes and keep the best 2
      size_t nrRoutes = 0;
      for (auto it = ESPEasy_now_traceroute_queue.begin(); it != ESPEasy_now_traceroute_queue.end();) {
        if (route.sameRoute(*it) || nrRoutes >= 2) {
          it = ESPEasy_now_traceroute_queue.erase(it);
        } else {
          ++nrRoutes;
          ++it;
        }
      }
    } else if (!ESPEasy_now_MQTT_check_queue.empty()) {
      const MAC_address mac = ESPEasy_now_MQTT_check_queue.front();
      const NodeStruct * node = Nodes.getNodeByMac(mac);
      if (node != nullptr) {
        const uint8_t channel = node->channel;
        sendMQTTCheckControllerQueue(mac, channel);
        last_queue_processed = millis();
      }
      // Remove duplicate entries in the list.
      for (auto it = ESPEasy_now_MQTT_check_queue.begin(); it != ESPEasy_now_MQTT_check_queue.end();) {
        if (*it == mac) {
          it = ESPEasy_now_MQTT_check_queue.erase(it);
        } else {
          ++it;
        }
      }
    }
  }
}

bool ESPEasy_now_handler_t::active() const
{
  if (!use_EspEasy_now) {
    return false;
  }

  if (_last_started == 0) {
    return false;
  }
  if (timePassedSince(_last_started) < ESPEASY_NOW_SINCE_LAST_BROADCAST) {
    // Give the unit some time to find other nodes.
    return true;
  }
  /*
  if (Nodes.lastTimeValidDistanceExpired()) {
    return false;
  }
  */
  const bool traceroute_received_timeout = _last_traceroute_received != 0 && (timePassedSince(_last_traceroute_received) > ESPEASY_NOW_ACTIVITY_TIMEOUT + 30000);
  const bool traceroute_sent_timeout     = _last_traceroute_sent != 0 && (timePassedSince(_last_traceroute_sent) > ESPEASY_NOW_ACTIVITY_TIMEOUT);
  const bool first_traceroute_receive_timeout = _last_traceroute_received == 0 && (timePassedSince(_last_started) > ESPEASY_NOW_ACTIVITY_TIMEOUT + 30000);
  if (traceroute_received_timeout || traceroute_sent_timeout || first_traceroute_receive_timeout) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      if (log.reserve(64)) {
        log = F(ESPEASY_NOW_NAME);
        log += F(": Inactive due to not receiving trace routes");
        addLog(LOG_LEVEL_INFO, log);
      }
    }

    return false;
  }

  return timePassedSince(_last_used) < ESPEASY_NOW_ACTIVITY_TIMEOUT;
}

MAC_address ESPEasy_now_handler_t::getActiveESPEasyNOW_MAC() const
{
  MAC_address this_mac;

  if (use_EspEasy_now) {
    if (WifiIsAP(WiFi.getMode())) {
      WiFi.softAPmacAddress(this_mac.mac);
    } else {
      WiFi.macAddress(this_mac.mac);
    }
  }
  return this_mac;
}

void ESPEasy_now_handler_t::addPeerFromWiFiScan()
{
  const int8_t scanCompleteStatus = WiFi.scanComplete();

  for (int8_t i = 0; i < scanCompleteStatus; ++i) {
    addPeerFromWiFiScan(i);
  }
}

void ESPEasy_now_handler_t::addPeerFromWiFiScan(const WiFi_AP_Candidate& peer)
{
  MAC_address peer_mac(peer.bssid);
  auto nodeInfo = Nodes.getNodeByMac(peer_mac);

  if (nodeInfo != nullptr) {
    Nodes.setRSSI(peer_mac, peer.rssi);
    // Sometimes a scan on one channel may see a node on another channel
    // So don't set the channel of known node based on the WiFi scan
    //nodeInfo->channel = WiFi.channel(scanIndex);
  } else {
    NodeStruct tmpNodeInfo;
    tmpNodeInfo.setRSSI(peer.rssi);
    tmpNodeInfo.channel = peer.channel;
    peer_mac.get(tmpNodeInfo.ap_mac);

    if (tmpNodeInfo.markedAsPriorityPeer()) {
      // FIXME TD-er: For now we assume the other node uses AP for ESPEasy-NOW
      tmpNodeInfo.setESPEasyNow_mac(peer_mac);
      if (Nodes.addNode(tmpNodeInfo)) {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F(ESPEASY_NOW_NAME ": Found node via WiFi scan: ");
          log += peer_mac.toString();
          log += F(" ");
          log += tmpNodeInfo.getRSSI();
          log += F(" dBm ch: ");
          log += tmpNodeInfo.channel;
          addLog(LOG_LEVEL_INFO, log);
        }

        // Must trigger a discovery request from the node.
        sendDiscoveryAnnounce(peer_mac, peer.channel);
      }
    }
  }
}

bool ESPEasy_now_handler_t::processMessage(const ESPEasy_now_merger& message, bool& mustKeep)
{
  bool handled = false;
  bool considerActive = false;
  mustKeep = false;

  {
    // Check if message is sent by this node
    MAC_address receivedMAC;
    message.getMac(receivedMAC.mac);
    MAC_address tmp;
    WiFi.softAPmacAddress(tmp.mac);
    if (tmp == receivedMAC) return handled;
    WiFi.macAddress(tmp.mac);
    if (tmp == receivedMAC) return handled;
  }
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    if (log.reserve(75)) {
      log = F(ESPEASY_NOW_NAME);
      log += F(": received ");
      log += message.getLogString();
      addLog(LOG_LEVEL_INFO, log);
    }
  }

  switch (message.getMessageType())
  {
    case ESPEasy_now_hdr::message_t::NotSet:
    case ESPEasy_now_hdr::message_t::ChecksumError:
      break;
    case ESPEasy_now_hdr::message_t::Acknowledgement:
      break;
    case ESPEasy_now_hdr::message_t::Announcement:
      handled = handle_DiscoveryAnnounce(message, mustKeep);
      break;
    case ESPEasy_now_hdr::message_t::NTP_Query:
      handled = handle_NTPquery(message, mustKeep);
      break;
    case ESPEasy_now_hdr::message_t::MQTTControllerMessage:
      handled = handle_MQTTControllerMessage(message, mustKeep);
      considerActive = true;
      break;
    case ESPEasy_now_hdr::message_t::MQTTCheckControllerQueue:
      handled = handle_MQTTCheckControllerQueue(message, mustKeep);
      considerActive = true;
      break;
    case ESPEasy_now_hdr::message_t::SendData_DuplicateCheck:
      handled = handle_SendData_DuplicateCheck(message, mustKeep);
      break;
    case ESPEasy_now_hdr::message_t::P2P_data:
      handled = handle_ESPEasyNow_p2p(message, mustKeep);
      considerActive = true;
      break;
    case ESPEasy_now_hdr::message_t::TraceRoute:
      handled = handle_TraceRoute(message, mustKeep);
      considerActive = true;
      break;
  }

  if (handled && considerActive) {
    _last_used = millis();
  }

  return handled;
}

// *************************************************************
// * Discovery Announcement
// *************************************************************

void ESPEasy_now_handler_t::sendDiscoveryAnnounce(int channel)
{
  MAC_address broadcast;

  for (int i = 0; i < 6; ++i) {
    broadcast.mac[i] = 0xFF;
  }
  sendDiscoveryAnnounce(ESPEasy_now_peermanager.getBroadcastMAC(), channel);
}

void ESPEasy_now_handler_t::sendDiscoveryAnnounce(const MAC_address& mac, int channel)
{
  const NodeStruct *thisNode = Nodes.getThisNode();

  if (thisNode == nullptr) {
    // Should not happen
    return;
  }

  // Append traceroute (if available)
  const ESPEasy_now_traceroute_struct* thisTraceRoute = Nodes.getDiscoveryRoute(thisNode->unit);

  size_t len = sizeof(NodeStruct) + 1; // Append length indicator for traceroute
  uint8_t traceroute_size = 0;
  const uint8_t* traceroute_data = nullptr;
  if (thisTraceRoute != nullptr) {
    traceroute_data = thisTraceRoute->getData(traceroute_size);
    len += traceroute_size;
  }

  ESPEasy_now_splitter msg(ESPEasy_now_hdr::message_t::Announcement, len);
  if (sizeof(NodeStruct) != msg.addBinaryData(reinterpret_cast<const uint8_t *>(thisNode), sizeof(NodeStruct))) {
    return;
  }
  if (sizeof(uint8_t) != msg.addBinaryData(reinterpret_cast<const uint8_t *>(&traceroute_size), sizeof(uint8_t))) {
    return;
  }
  if (traceroute_data != nullptr) {
    if (traceroute_size != msg.addBinaryData(traceroute_data, traceroute_size)) {
      return;
    }
  }
  if (channel < 0) {
    // Send to all channels

    const unsigned long start = millis();

    // FIXME TD-er: Not sure whether we can send to channels > 11 in all countries.
    for (int ch = 1; ch < ESPEASY_NOW_MAX_CHANNEL; ++ch) {
      msg.send(mac, ch);
      delay(0);
    }
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F(ESPEASY_NOW_NAME ": Sent discovery to all channels in ");
      log += String(timePassedSince(start));
      log += F(" ms");
      addLog(LOG_LEVEL_INFO, log);
    }
  } else {
    if (mac.all_one()) {
      for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
        if (it->second.getAge() > ESPEASY_NOW_SINCE_LAST_BROADCAST) {
          msg.send(it->second.ESPEasy_Now_MAC(), channel);
        }
      }
    }

    msg.send(mac, channel);
  }
//  WifiScan(true, channel);
}


bool ESPEasy_now_handler_t::handle_DiscoveryAnnounce(const ESPEasy_now_merger& message, bool& mustKeep)
{
  mustKeep = false;
  NodeStruct received;
  ESPEasy_now_traceroute_struct traceRoute;

  const uint8_t cur_distance = Nodes.getDistance();

  // Discovery messages have a single binary blob, starting at 0
  size_t payload_pos = 0;

  message.getBinaryData(reinterpret_cast<uint8_t *>(&received), sizeof(NodeStruct), payload_pos);

  if (!received.validate()) { return false; }

  MAC_address mac;
  message.getMac(mac);

  // Check to see if we process a node we've sent ourselves.
  if (received.isThisNode()) {
    return false;
  }

  if (!received.setESPEasyNow_mac(mac)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log;
      log  = F(ESPEASY_NOW_NAME ": Received discovery message from MAC not stated in message: ");
      log += mac.toString();
      addLog(LOG_LEVEL_ERROR, log);
    }
    return false;
  }

  uint8_t traceroute_size = 0;
  bool isNewNode = false;
  bool nodeAdded = false;
  if (message.getBinaryData(reinterpret_cast<uint8_t *>(&traceroute_size), sizeof(uint8_t), payload_pos) != 0) {
    if (traceroute_size != 0) {
      ESPEasy_now_traceroute_struct traceroute(traceroute_size);
      if (message.getBinaryData(traceroute.get(), traceroute_size, payload_pos) == traceroute_size) {
        isNewNode = Nodes.addNode(received, traceroute);
        nodeAdded = true;
      }
    }
  }
  if (!nodeAdded) {
    isNewNode = Nodes.addNode(received, ESPEasy_now_traceroute_struct());
    nodeAdded = true;
  }

  // Test to see if the discovery announce could be a good candidate for next NTP query.
  _best_NTP_candidate.find_best_NTP(
    mac,
    static_cast<timeSource_t>(received.timeSource),
    received.lastUpdated);

/*
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(128);
    log  = F(ESPEASY_NOW_NAME ": discovery: ");
    log += message.getLogString();
    log += '\n';
    log += received.getSummary();
    addLog(LOG_LEVEL_INFO, log);
  }
  */

  const NodeStruct * preferred = Nodes.getPreferredNode();
  if (preferred != nullptr) {
    if (preferred->unit == received.unit) {
      Nodes.updateThisNode();
    }
  }

  if (received.distance == 255 && Nodes.getDistance() < 255) {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      if (log.reserve(80)) {
        log  = F(ESPEASY_NOW_NAME);
        log += F(": Send announce back to unit: ");
        log += String(received.unit);
        addLog(LOG_LEVEL_INFO, log);
      }
    }

    sendDiscoveryAnnounce(received.ESPEasy_Now_MAC(), received.channel);
    {
      if (Nodes.isEndpoint()) {
        ESPEasy_now_traceroute_struct thisTraceRoute;
        thisTraceRoute.addUnit(Settings.Unit);
        // Since we're the end node, claim highest success rate
        thisTraceRoute.setSuccessRate_last_node(Settings.Unit, 255);
        sendTraceRoute(received.ESPEasy_Now_MAC(), thisTraceRoute, received.channel);
      }
    }
  }

  const uint8_t new_distance = Nodes.getDistance();
  if (new_distance != cur_distance || isNewNode) {
    if (new_distance == 0) {
      //sendDiscoveryAnnounce(-1);  // Send to all channels
      sendDiscoveryAnnounce(); // For now disable sending the announce to all channels.
    } else {
      sendDiscoveryAnnounce();
    }
  }

  return true;
}


// *************************************************************
// * Trace Route
// *************************************************************
void ESPEasy_now_handler_t::sendTraceRoute()
{
  if (Nodes.isEndpoint()) {
    ESPEasy_now_traceroute_struct thisTraceRoute;
    thisTraceRoute.addUnit(Settings.Unit);
    // Since we're the end node, claim highest success rate
    thisTraceRoute.setSuccessRate_last_node(Settings.Unit, 255);
 
    int channel = WiFiEventData.usedChannel;
    if (channel == 0) {
      channel = WiFi.channel();
    }

    sendTraceRoute(thisTraceRoute, channel);
    _last_traceroute_sent = millis();
  }
}

void ESPEasy_now_handler_t::sendTraceRoute(const ESPEasy_now_traceroute_struct& traceRoute, int channel)
{
  for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
    if (it->second.getAge() > ESPEASY_NOW_SINCE_LAST_BROADCAST) {
      sendTraceRoute(it->second.ESPEasy_Now_MAC(), traceRoute, channel);
    }
  }

  MAC_address broadcast = ESPEasy_now_peermanager.getBroadcastMAC();
  sendTraceRoute(broadcast, traceRoute, channel);
}


void ESPEasy_now_handler_t::sendTraceRoute(const MAC_address& mac, const ESPEasy_now_traceroute_struct& traceRoute, int channel)
{
  size_t len = 1;
  uint8_t traceroute_size = 0;
  const uint8_t* traceroute_data = traceRoute.getData(traceroute_size);
  len += traceroute_size;

  ESPEasy_now_splitter msg(ESPEasy_now_hdr::message_t::TraceRoute, len);
  if (sizeof(uint8_t) != msg.addBinaryData(reinterpret_cast<const uint8_t *>(&traceroute_size), sizeof(uint8_t))) {
    addLog(LOG_LEVEL_ERROR, F(ESPEASY_NOW_NAME ": sendTraceRoute error adding route size"));;
    return;
  }
  if (traceroute_size != msg.addBinaryData(traceroute_data, traceroute_size)) {
    addLog(LOG_LEVEL_ERROR, F(ESPEASY_NOW_NAME ": sendTraceRoute error adding trace route"));;
    return;
  }
  if (channel < 0) {
    // Send to all channels

    const unsigned long start = millis();

    // FIXME TD-er: Not sure whether we can send to channels > 11 in all countries.
    for (int ch = 1; ch < ESPEASY_NOW_MAX_CHANNEL; ++ch) {
      msg.send(mac, ch);
      delay(0);
    }
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log = F(ESPEASY_NOW_NAME ": Sent Traceroute to all channels in ");
      log += timePassedSince(start);
      log += F(" ms");
      addLog(LOG_LEVEL_INFO, log);
    }
  } else {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      log += F(ESPEASY_NOW_NAME ": sendTraceRoute ");
      log += traceRoute.toString();
      log += F(" ch: ");
      log += channel;
      addLog(LOG_LEVEL_INFO, log);
    }

    msg.send(mac, channel);
  }
}

bool ESPEasy_now_handler_t::handle_TraceRoute(const ESPEasy_now_merger& message, bool& mustKeep)
{
  mustKeep = false;
  size_t payload_pos = 0;
  uint8_t traceroute_size = 0;
  if (message.getBinaryData(reinterpret_cast<uint8_t *>(&traceroute_size), sizeof(uint8_t), payload_pos) != 0) {
    if (traceroute_size != 0) {
      ESPEasy_now_traceroute_struct traceroute(traceroute_size);
      if (message.getBinaryData(traceroute.get(), traceroute_size, payload_pos) == traceroute_size) {
        if (traceroute.getDistance() < 255) {
          const uint8_t thisunit = Settings.Unit;
          if (!traceroute.unitInTraceRoute(thisunit)) {
            MAC_address mac;
            message.getMac(mac);
            Nodes.setTraceRoute(mac, traceroute);
            if (thisunit != 0 && thisunit != 255 && !Nodes.isEndpoint()) {
              // Do not forward the trace route if we're an endpoint.
              traceroute.addUnit(thisunit);
              traceroute.setSuccessRate_last_node(thisunit, Nodes.getSuccessRate(thisunit));
              ESPEasy_now_traceroute_queue.push_back(traceroute);
              // Send MQTT queue check to the node we received the traceroute from
              // It may be a viable path to send MQTT to, so stay informed of its MQTT queue state
              ESPEasy_now_MQTT_check_queue.push_back(mac);
            }
          }
        }
      }
    }
  }
  return true;
}

// *************************************************************
// * NTP Query
// *************************************************************

void ESPEasy_now_handler_t::sendNTPquery()
{
  if (!_best_NTP_candidate.hasLowerWander()) { return; }
  MAC_address mac;

  if (!_best_NTP_candidate.getMac(mac)) { return; }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F(ESPEASY_NOW_NAME ": Send NTP query to: ");
    log += mac.toString();
    addLog(LOG_LEVEL_INFO, log);
  }

  _best_NTP_candidate.markSendTime();
  ESPEasy_Now_NTP_query query;
  const size_t len = sizeof(ESPEasy_Now_NTP_query);
  ESPEasy_now_splitter msg(ESPEasy_now_hdr::message_t::NTP_Query, len);

  if (len == msg.addBinaryData(reinterpret_cast<uint8_t *>(&query), len)) {
    msg.send(mac.mac);
  }
}

void ESPEasy_now_handler_t::sendNTPbroadcast()
{
  ESPEasy_Now_NTP_query query;

  query.createBroadcastNTP();
  const size_t len = sizeof(ESPEasy_Now_NTP_query);
  ESPEasy_now_splitter msg(ESPEasy_now_hdr::message_t::NTP_Query, len);
  if (len == msg.addBinaryData(reinterpret_cast<uint8_t *>(&query), len)) {
    msg.send(query._mac);
  }
}

bool ESPEasy_now_handler_t::handle_NTPquery(const ESPEasy_now_merger& message, bool& mustKeep)
{
  mustKeep = false;
  ESPEasy_Now_NTP_query query;

  // NTP query messages have a single binary blob, starting at 0
  size_t payload_pos = 0;

  message.getBinaryData(reinterpret_cast<uint8_t *>(&query), sizeof(ESPEasy_Now_NTP_query), payload_pos);

  if (query._timeSource == timeSource_t::No_time_source) {
    // Received a query, must generate an answer for it.

    // first fetch the MAC address to reply to
    if (!message.getMac(query._mac)) { return false; }

    // Fill the reply
    query.createReply(message.getFirstPacketTimestamp());

    const size_t len = sizeof(ESPEasy_Now_NTP_query);
    ESPEasy_now_splitter msg(ESPEasy_now_hdr::message_t::NTP_Query, len);
    if (len == msg.addBinaryData(reinterpret_cast<uint8_t *>(&query), len)) {
      msg.send(query._mac);
      return true;
    }
    return false;
  }

  // Received a reply on our own query
  return _best_NTP_candidate.processReply(query, message.getFirstPacketTimestamp());
}

// *************************************************************
// * MQTT controller forwarder
// *************************************************************

bool ESPEasy_now_handler_t::sendToMQTT(
  controllerIndex_t controllerIndex, 
  const String& topic, 
  const String& payload, 
  const MessageRouteInfo_t* messageRouteInfo)
{
  if (!use_EspEasy_now) { return false; }

  const uint8_t distance = Nodes.getDistance();

  if (((distance == 0) && MQTTclient_connected) || (distance == 255)) {
    // No need to send via ESPEasy_Now.
    // We're either connected ((distance == 0) && MQTTclient_connected)
    // or have no neighbor that can forward the message (distance == 255)
    return false;
  }

  if (validControllerIndex(controllerIndex)) {
    load_ControllerSettingsCache(controllerIndex);
  }

  bool processed = false;

  if (_enableESPEasyNowFallback /*&& !WiFiConnected(10) */) {
    // Must make sure we don't forward it to the unit we received the message from.
    const uint8_t unit_nr = (messageRouteInfo == nullptr) ? 0 : messageRouteInfo->getLastUnitNotMatching(Settings.Unit);
    const NodeStruct *preferred = Nodes.getPreferredNode_notMatching(unit_nr);

    if (preferred != nullptr /* && Nodes.getDistance() > preferred->distance */) {
      if ((messageRouteInfo != nullptr) && messageRouteInfo->countUnitInTrace(preferred->unit) > 2) {
        // Preferred node appears in trace quite often, reduce success rate of this node.
        Nodes.updateSuccessRate(preferred->unit, false);
      }
      MAC_address mac = preferred->ESPEasy_Now_MAC();
      switch (Nodes.getMQTTQueueState(preferred->unit)) {
        case ESPEasy_Now_MQTT_queue_check_packet::QueueState::Unset:
        case ESPEasy_Now_MQTT_queue_check_packet::QueueState::Full:
          ESPEasy_now_MQTT_check_queue.push_back(mac);
          return false;
        case ESPEasy_Now_MQTT_queue_check_packet::QueueState::Empty:
          break;
      }


      // each string has null termination
      const size_t topic_length   = topic.length() + 1;
      const size_t payload_length = payload.length() + 1;
      size_t routeInfo_length = 0;

      MessageRouteInfo_t::uint8_t_vector routeInfo;
      // Check if the intended recipient supports routeInfo
      if (messageRouteInfo != nullptr && preferred->build > 20113) {
        // FIXME TD-er: Disabled for now as forwarding messages does not work well right now
//        routeInfo = messageRouteInfo->serialize();
//        routeInfo_length = routeInfo.size();
      }

      // Todo: Add   cpluginID_t cpluginID; to the message
      size_t len = topic_length + payload_length + routeInfo_length;

      ESPEasy_now_splitter msg(ESPEasy_now_hdr::message_t::MQTTControllerMessage, len);

      if (topic_length != msg.addString(topic)) {
        return false;
      }
      if (payload_length != msg.addString(payload)) {
        return false;
      }
      if (routeInfo_length != 0 && routeInfo_length != msg.addBinaryData(&(routeInfo[0]), routeInfo_length)) {
//        return false;
      }

      WifiEspNowSendStatus sendStatus = msg.send(mac, _ClientTimeout, preferred->channel);

      switch (sendStatus) {
        case WifiEspNowSendStatus::OK:
        {
          processed = true;
          break;
        }
        case WifiEspNowSendStatus::NONE:
        case WifiEspNowSendStatus::FAIL:
        {
          Nodes.setMQTTQueueState(preferred->unit, ESPEasy_Now_MQTT_queue_check_packet::QueueState::Unset);
          ESPEasy_now_MQTT_check_queue.push_back(mac);
          ++_send_failed_count;
          break;
        }
        default: break;
      }
    }
  }
  return processed;
}

bool ESPEasy_now_handler_t::handle_MQTTControllerMessage(const ESPEasy_now_merger& message, bool& mustKeep)
{
  # ifdef USES_MQTT

  // FIXME TD-er: Quick hack to just echo all data to the first enabled MQTT controller

  controllerIndex_t controllerIndex = firstEnabledMQTT_ControllerIndex();

  if (validControllerIndex(controllerIndex)) {
    load_ControllerSettingsCache(controllerIndex);
    MAC_address mac;

    bool success = false;

    if (message.getMac(mac)) {
      MessageRouteInfo_t MessageRouteInfo;
      const NodeStruct* node = Nodes.getNodeByMac(mac);
      if (node != nullptr) {
        if (message.getMessageCount(MessageRouteInfo.count)) {
          MessageRouteInfo.unit = node->unit;
        }
      }
      
      success = MQTTpublish(controllerIndex, message, MessageRouteInfo, _mqtt_retainFlag);
      if (!success) {
        mustKeep = false;
        return success;
      }

      ESPEasy_Now_MQTT_queue_check_packet query;
      const bool queue_full = MQTT_queueFull(controllerIndex);
      query.setState(queue_full);

      if (loglevelActiveFor(LOG_LEVEL_INFO) && queue_full) {
        addLog(LOG_LEVEL_INFO, F(ESPEASY_NOW_NAME ": After MQTT message received: Full"));
      }
      sendMQTTCheckControllerQueue(mac, 0, query.state);
    }

    mustKeep = !success;
    return success;
  }

  # endif // ifdef USES_MQTT
  mustKeep = false;
  return false;
}

// *************************************************************
// * Check MQTT queue state of preferred node
// *************************************************************
bool ESPEasy_now_handler_t::sendMQTTCheckControllerQueue(controllerIndex_t controllerIndex)
{
  if (!use_EspEasy_now) { return false; }

  const uint8_t distance = Nodes.getDistance();

  if ((distance == 0) || (distance == 255)) {
    // No need to send via ESPEasy_Now.
    // We're either connected (distance == 0)
    // or have no neighbor that can forward the message (distance == 255)
    return false;
  }
  const NodeStruct *preferred = Nodes.getPreferredNode();

  if (preferred != nullptr && Nodes.getDistance() > preferred->distance) {
    return sendMQTTCheckControllerQueue(preferred->ESPEasy_Now_MAC(), preferred->channel);
  }
  return false;
}

bool ESPEasy_now_handler_t::sendMQTTCheckControllerQueue(const MAC_address                             & mac,
                                                         int                                             channel,
                                                         ESPEasy_Now_MQTT_queue_check_packet::QueueState state) {
  ESPEasy_Now_MQTT_queue_check_packet query;

  query.state = state;
  const size_t len = sizeof(ESPEasy_Now_MQTT_queue_check_packet);
  ESPEasy_now_splitter msg(ESPEasy_now_hdr::message_t::MQTTCheckControllerQueue, len);
  if (len != msg.addBinaryData(reinterpret_cast<uint8_t *>(&query), len)) {
    return false;
  }
  size_t timeout                  = 10;
  WifiEspNowSendStatus sendStatus = msg.send(mac, timeout, channel);

  switch (sendStatus) {
    case WifiEspNowSendStatus::OK:
    {
      return true;
    }
    case WifiEspNowSendStatus::NONE:
    case WifiEspNowSendStatus::FAIL:
    {
      break;
    }
    default: break;
  }
  return false;
}

bool ESPEasy_now_handler_t::handle_MQTTCheckControllerQueue(const ESPEasy_now_merger& message, bool& mustKeep)
{
  mustKeep = false;
  # ifdef USES_MQTT

  ESPEasy_Now_MQTT_queue_check_packet query;
  size_t payload_pos = 0;
  message.getBinaryData(reinterpret_cast<uint8_t *>(&query), sizeof(ESPEasy_Now_MQTT_queue_check_packet), payload_pos);

  controllerIndex_t controllerIndex = firstEnabledMQTT_ControllerIndex();

  if (validControllerIndex(controllerIndex)) {
    if (query.isSet()) {
      // Got an answer from our query
      MAC_address mac;
      if (message.getMac(mac)) {

        Nodes.setMQTTQueueState(mac, query.state);
        #  ifndef BUILD_NO_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_DEBUG_MORE)) {
          String log;
          log  = F(ESPEASY_NOW_NAME ": Received Queue state: ");
          log += query.isFull() ? F("Full") : F("not Full");
          addLog(LOG_LEVEL_DEBUG_MORE, log);
        }
        #  endif // ifndef BUILD_NO_DEBUG
      }
      return true;
    } else {
      MAC_address mac;

      if (message.getMac(mac)) {
        // We have to give our own queue state and reply
        query.setState(MQTT_queueFull(controllerIndex));

        //        addLog(LOG_LEVEL_INFO, F(ESPEASY_NOW_NAME ": reply to queue state query"));
        const size_t len = sizeof(ESPEasy_Now_MQTT_queue_check_packet);
        ESPEasy_now_splitter msg(ESPEasy_now_hdr::message_t::MQTTCheckControllerQueue, len);
        if (len == msg.addBinaryData(reinterpret_cast<uint8_t *>(&query), len)) {
          msg.send(mac);
          return true;
        }
      }
    }
  }

  # endif // ifdef USES_MQTT
  return false;
}

// *************************************************************
// * Controller Message Duplicate Check
// *************************************************************

void ESPEasy_now_handler_t::sendSendData_DuplicateCheck(uint32_t                              key,
                                                        ESPEasy_Now_DuplicateCheck::message_t message_type,
                                                        const MAC_address                   & mac)
{
  ESPEasy_Now_DuplicateCheck check(key, message_type);
  const size_t len = sizeof(ESPEasy_Now_DuplicateCheck);
  ESPEasy_now_splitter msg(ESPEasy_now_hdr::message_t::SendData_DuplicateCheck, len);

  if (len != msg.addBinaryData(reinterpret_cast<uint8_t *>(&check), len)) {
    return;
  }

  switch (message_type) {
    case ESPEasy_Now_DuplicateCheck::message_t::KeyToCheck:
      msg.sendToBroadcast();
      break;
    case ESPEasy_Now_DuplicateCheck::message_t::AlreadyProcessed:
      msg.send(mac);
      break;
  }

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;
    log = F("ESPEasy_now dup check: ");

    switch (message_type) {
      case ESPEasy_Now_DuplicateCheck::message_t::KeyToCheck:
        log += F("Broadcast check key ");
        log += key;
        break;
      case ESPEasy_Now_DuplicateCheck::message_t::AlreadyProcessed:
        log += F("Processed key ");
        log += key;
        log += ' ';
        log += mac.toString();
        break;
    }
    addLog(LOG_LEVEL_DEBUG, log);
  }
}

bool ESPEasy_now_handler_t::handle_SendData_DuplicateCheck(const ESPEasy_now_merger& message, bool& mustKeep)
{
  mustKeep = false;
  ESPEasy_Now_DuplicateCheck check;
  size_t payload_pos = 0;

  message.getBinaryData(reinterpret_cast<uint8_t *>(&check), sizeof(ESPEasy_Now_DuplicateCheck), payload_pos);

  switch (check._type) {
    case ESPEasy_Now_DuplicateCheck::message_t::KeyToCheck:

      // This is a query from another node.
      // Check if it has already been processed by some node.
      if (SendData_DuplicateChecker.historicKey(check._key)) {
        // Must reply back to that node we already have seen it
        MAC_address mac;

        if (message.getMac(mac)) {
          sendSendData_DuplicateCheck(check._key,
                                      ESPEasy_Now_DuplicateCheck::message_t::AlreadyProcessed,
                                      mac);
        }
      }
      return true;

    case ESPEasy_Now_DuplicateCheck::message_t::AlreadyProcessed:

      // This is a rejection indicating some other node already has the data processed
      SendData_DuplicateChecker.remove(check._key);

      return true;
  }
  return false;
}

void ESPEasy_now_handler_t::load_ControllerSettingsCache(controllerIndex_t controllerIndex)
{
  if (validControllerIndex(controllerIndex) && controllerIndex != _controllerIndex)
  {
    // Place the ControllerSettings in a scope to free the memory as soon as we got all relevant information.
    MakeControllerSettings(ControllerSettings);
    LoadControllerSettings(controllerIndex, ControllerSettings);
    _enableESPEasyNowFallback = ControllerSettings.enableESPEasyNowFallback();
    _ClientTimeout            = ControllerSettings.ClientTimeout;
    _mqtt_retainFlag          = ControllerSettings.mqtt_retainFlag();
    _controllerIndex          = controllerIndex;
  }
}



// *************************************************************
// * ESPEasyNow p2p 
// *************************************************************
bool ESPEasy_now_handler_t::sendESPEasyNow_p2p(controllerIndex_t controllerIndex, const MAC_address& mac, const ESPEasy_Now_p2p_data& data) {
  if (!use_EspEasy_now) { return false; }

  ESPEasy_now_splitter msg(ESPEasy_now_hdr::message_t::P2P_data, data.getTotalSize());
  // Add the first part of the data object, without the data array.
  if (data.dataOffset != msg.addBinaryData(reinterpret_cast<const uint8_t *>(&data), data.dataOffset)) {
    return false;
  }
  
  // Fetch the data array information, will also update size.
  size_t size = 0;
  const uint8_t* data_ptr = data.getBinaryData(0, size);
  if (size != msg.addBinaryData(data_ptr, size)) {
    return false;
  }

  return msg.send(mac);
}

bool ESPEasy_now_handler_t::handle_ESPEasyNow_p2p(const ESPEasy_now_merger& message, bool& mustKeep) {
  mustKeep = false;
  controllerIndex_t controller_index = findFirstEnabledControllerWithId(19); // CPLUGIN_ID_019
  if (!validControllerIndex(controller_index)) {
    addLog(LOG_LEVEL_ERROR, F("Controller C019 not enabled"));
    return false;
  }

  ESPEasy_Now_p2p_data data;
  size_t payload_pos = 0;
  size_t payload_size = message.getPayloadSize();
  size_t headerSize = data.dataOffset;
  if (headerSize > payload_size) {
    // This can only happen when the receiving end has a larger ESPEasy_Now_p2p_data struct
    headerSize = payload_size;
  }
  message.getBinaryData(reinterpret_cast<uint8_t *>(&data), headerSize, payload_pos);
  // dataOffset may have changed to match the offset used by the sender.
  payload_pos = data.dataOffset;

  size_t binaryData_size = payload_size - headerSize;
  uint8_t* binaryData_ptr = data.prepareBinaryData(binaryData_size);
  if (binaryData_ptr == nullptr) {
    addLog(LOG_LEVEL_ERROR, F("handle_ESPEasyNow_p2p: Cannot allocate data"));
    // Cannot allocate memory to process message, so return true to make sure it gets deleted.
    return true;
  }

  size_t received_size = message.getBinaryData(binaryData_ptr, binaryData_size, payload_pos);
  if (received_size != binaryData_size) {
    // Did not receive all data
    String log = F("handle_ESPEasyNow_p2p: Did not receive all data ");
    log += received_size;
    log += '/';
    log += binaryData_size;
    log += F(" dataSize: ");
    log += data.dataSize;
    log += F(" payload_pos: ");
    log += data.dataOffset;
    addLog(LOG_LEVEL_ERROR, log);
//    return false;
  }

  // Call C019 controller with event containing this data object as a pointer.
  EventStruct event;
  event.ControllerIndex = controller_index;
  event.Par1 = sizeof(ESPEasy_Now_p2p_data);
  event.Data = reinterpret_cast<uint8_t *>(&data);
  CPluginCall(CPlugin::Function::CPLUGIN_PROTOCOL_RECV, &event);

  return true;
}

#endif // ifdef USES_ESPEASY_NOW
