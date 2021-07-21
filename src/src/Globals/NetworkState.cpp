#include "../Globals/NetworkState.h"

#include "../../ESPEasy_common.h"


// Ethernet Connection status
NetworkMedium_t active_network_medium = NetworkMedium_t::NotSet;
LongTermTimer last_network_medium_set_moment; 

bool webserverRunning(false);
bool webserver_init(false);
bool mDNS_init(false);


// NTP status
bool statusNTPInitialized = false;


// Setup DNS, only used if the ESP has no valid WiFi config
const uint8_t DNS_PORT = 53;
IPAddress  apIP(DEFAULT_AP_IP);



// udp protocol stuff (syslog, global sync, node info list, ntp time)
WiFiUDP portUDP;
