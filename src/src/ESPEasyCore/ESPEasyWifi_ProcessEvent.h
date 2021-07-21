#ifndef ESPEASYWIFI_PROCESSEVENT_H
#define ESPEASYWIFI_PROCESSEVENT_H

#include "../../ESPEasy_common.h"

void handle_unprocessedNetworkEvents();
void processDisconnect();
void processConnect();
void processGotIP();
void processDisconnectAPmode();
void processProbeRequestAPmode();
void processConnectAPmode();
void processDisableAPmode();
void processScanDone();

#ifdef HAS_ETHERNET
void processEthernetConnected();
void processEthernetDisconnected();
void processEthernetGotIP();
#endif

#endif //ESPEASYWIFI_PROCESSEVENT_H