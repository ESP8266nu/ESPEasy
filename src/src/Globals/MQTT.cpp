#include "../Globals/MQTT.h"

#include "../../ESPEasy_common.h"

#ifdef USES_MQTT


// MQTT client
WiFiClient   mqtt;
PubSubClient MQTTclient(mqtt);
bool MQTTclient_should_reconnect        = true;
bool MQTTclient_must_send_LWT_connected = false;
bool MQTTclient_connected               = false;
int  mqtt_reconnect_count               = 0;
#endif // USES_MQTT

#ifdef USES_P037

// mqtt import status
bool P037_MQTTImport_connected = false;
#endif // ifdef USES_P037
