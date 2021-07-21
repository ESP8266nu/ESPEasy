#ifndef HELPERS_ESPEASY_NOW_HANDLER_H
#define HELPERS_ESPEASY_NOW_HANDLER_H

#include "../Globals/ESPEasy_now_state.h"

#ifdef USES_ESPEASY_NOW

# include "../DataStructs/ESPEasy_now_hdr.h"
# include "../DataStructs/ESPEasy_Now_DuplicateCheck.h"
# include "../DataStructs/ESPEasy_Now_packet.h"
# include "../DataStructs/ESPEasy_now_merger.h"
# include "../DataStructs/ESPEasy_Now_NTP_query.h"
# include "../DataStructs/ESPEasy_Now_MQTT_queue_check_packet.h"
# include "../DataStructs/ESPEasy_Now_p2p_data.h"
# include "../DataStructs/ESPEasy_now_traceroute.h"
# include "../DataStructs/MAC_address.h"
# include "../DataStructs/MessageRouteInfo.h"
# include "../DataStructs/WiFi_AP_Candidate.h"
# include "../Globals/CPlugins.h"


class ESPEasy_now_handler_t {
public:

  ESPEasy_now_handler_t() {}

  bool begin();

  void end();

  bool loop();

private:

  void loop_check_ESPEasyNOW_run_state();

  bool loop_process_ESPEasyNOW_in_queue();

  void loop_process_ESPEasyNOW_send_queue();


public:

  bool active() const;

  MAC_address getActiveESPEasyNOW_MAC() const;

  void addPeerFromWiFiScan();
  void addPeerFromWiFiScan(const WiFi_AP_Candidate& peer);

private:

  bool processMessage(const ESPEasy_now_merger& message, bool& mustKeep);

public:

  // Send out the discovery announcement via broadcast.
  // This may be picked up by others
  void sendDiscoveryAnnounce(int channel = 0);
  void sendDiscoveryAnnounce(const MAC_address& mac, int channel = 0);

  // Send trace route as 'connected' node
  void sendTraceRoute();
  
  void sendTraceRoute(const ESPEasy_now_traceroute_struct& traceRoute, int channel = 0);
  void sendTraceRoute(const MAC_address& mac, const ESPEasy_now_traceroute_struct& traceRoute, int channel = 0);

  void sendNTPquery();

  void sendNTPbroadcast();

  bool sendToMQTT(controllerIndex_t controllerIndex,
                  const String    & topic,
                  const String    & payload,
                  const MessageRouteInfo_t* messageRouteInfo);

  bool sendMQTTCheckControllerQueue(controllerIndex_t controllerIndex);
  bool sendMQTTCheckControllerQueue(const MAC_address& mac, 
                                    int channel, 
                                    ESPEasy_Now_MQTT_queue_check_packet::QueueState state = ESPEasy_Now_MQTT_queue_check_packet::QueueState::Unset);

  void sendSendData_DuplicateCheck(uint32_t                              key,
                                   ESPEasy_Now_DuplicateCheck::message_t message_type,
                                   const MAC_address& mac);

  bool sendESPEasyNow_p2p(controllerIndex_t controllerIndex, const MAC_address& mac, const ESPEasy_Now_p2p_data& data);

private:

  bool handle_DiscoveryAnnounce(const ESPEasy_now_merger& message, bool& mustKeep);

  bool handle_TraceRoute(const ESPEasy_now_merger& message, bool& mustKeep);

  bool handle_NTPquery(const ESPEasy_now_merger& message, bool& mustKeep);

  bool handle_MQTTControllerMessage(const ESPEasy_now_merger& message, bool& mustKeep);

  bool handle_MQTTCheckControllerQueue(const ESPEasy_now_merger& message, bool& mustKeep);

  bool handle_SendData_DuplicateCheck(const ESPEasy_now_merger& message, bool& mustKeep);

  bool handle_ESPEasyNow_p2p(const ESPEasy_now_merger& message, bool& mustKeep);

  void load_ControllerSettingsCache(controllerIndex_t controllerIndex);

  ESPEasy_Now_NTP_query _best_NTP_candidate;

  unsigned long _last_used = 0;
  unsigned long _last_started = 0;

  // Trace route is only sent by 'connected' nodes (distance = 0)
  unsigned long _last_traceroute_sent = 0;

  // Trace route will be received only when at least one node in the connected mesh has distance = 0
  unsigned long _last_traceroute_received = 0;

  uint8_t _send_failed_count = 0;

  unsigned int _ClientTimeout = 0;
  uint8_t      _usedWiFiChannel = 0;
  uint8_t      _lastScannedChannel = 0;
  controllerIndex_t _controllerIndex = INVALID_CONTROLLER_INDEX;
  bool _scanChannelsMode = true;
  bool _enableESPEasyNowFallback = false;
  bool _mqtt_retainFlag = false;

};


#endif // ifdef USES_ESPEASY_NOW

#endif // HELPERS_ESPEASY_NOW_HANDLER_H
