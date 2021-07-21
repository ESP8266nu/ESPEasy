#ifndef GLOBALS_NETWORKSTATE_H
#define GLOBALS_NETWORKSTATE_H

#include <Arduino.h>

#include <IPAddress.h>
#include <WiFiUdp.h>

#include "../DataTypes/ESPEasy_plugin_functions.h"
#include "../DataTypes/NetworkMedium.h"

#include "../Helpers/LongTermTimer.h"

// Ethernet Connectiopn status
extern NetworkMedium_t active_network_medium;
extern LongTermTimer last_network_medium_set_moment;;

extern bool webserverRunning;
extern bool webserver_init;
extern bool mDNS_init;


// NTP status
extern bool statusNTPInitialized;


// Setup DNS, only used if the ESP has no valid WiFi config
extern const uint8_t DNS_PORT;
extern IPAddress  apIP;

// udp protocol stuff (syslog, global sync, node info list, ntp time)
extern WiFiUDP portUDP;


#endif // GLOBALS_NETWORKSTATE_H
