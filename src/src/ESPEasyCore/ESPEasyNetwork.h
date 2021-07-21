#ifndef ESPEASY_NETWORK_H
#define ESPEASY_NETWORK_H

#include "../../ESPEasy_common.h"

#ifndef NETWORK_H
#define NETWORK_H

#include "../DataStructs/MAC_address.h"

void setNetworkMedium(NetworkMedium_t medium);

bool isESPEasy_now_only();

void NetworkConnectRelaxed();
bool NetworkConnected();
IPAddress NetworkLocalIP();
IPAddress NetworkSubnetMask();
IPAddress NetworkGatewayIP();
IPAddress NetworkDnsIP (uint8_t dns_no);
MAC_address NetworkMacAddress();
String NetworkGetHostNameFromSettings(bool force_add_unitnr = false);
String NetworkGetHostname();
String NetworkCreateRFCCompliantHostname(bool force_add_unitnr = false);
String createRFCCompliantHostname(const String& oldString);
String WifiSoftAPmacAddress();
String WifiSTAmacAddress();

void CheckRunningServices();

#ifdef HAS_ETHERNET
bool EthFullDuplex();
bool EthLinkUp();
uint8_t EthLinkSpeed();
void stop_eth_dhcps();
#endif


#endif 
#endif