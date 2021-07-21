#include "../DataStructs/ControllerSettingsStruct.h"

#include "../../ESPEasy_common.h"

#include "../CustomBuild/ESPEasyLimits.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringConverter.h"


#include <IPAddress.h>
#include <WString.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

ControllerSettingsStruct::ControllerSettingsStruct()
{
  reset();
}

void ControllerSettingsStruct::reset() {
  UseDNS                     = false;
  Port                       = 0;
  MinimalTimeBetweenMessages = CONTROLLER_DELAY_QUEUE_DELAY_DFLT;
  MaxQueueDepth              = CONTROLLER_DELAY_QUEUE_DEPTH_DFLT;
  MaxRetry                   = CONTROLLER_DELAY_QUEUE_RETRY_DFLT;
  DeleteOldest               = false;
  ClientTimeout              = CONTROLLER_CLIENTTIMEOUT_DFLT;
  MustCheckReply             = false;
  SampleSetInitiator         = INVALID_TASK_INDEX;
  VariousFlags               = 0;

  for (uint8_t i = 0; i < 4; ++i) {
    IP[i] = 0;
  }
  ZERO_FILL(HostName);
  ZERO_FILL(ClientID);
  ZERO_FILL(Publish);
  ZERO_FILL(Subscribe);
  ZERO_FILL(MQTTLwtTopic);
  ZERO_FILL(LWTMessageConnect);
  ZERO_FILL(LWTMessageDisconnect);
  safe_strncpy(ClientID, F(CONTROLLER_DEFAULT_CLIENTID), sizeof(ClientID));
}

bool ControllerSettingsStruct::isSet() const {
  if (UseDNS) {
    return HostName[0] != 0;
  }
  return ipSet();
}

void ControllerSettingsStruct::validate() {
  if (Port > 65535) { Port = 0; }

  if ((MinimalTimeBetweenMessages < 1) ||  (MinimalTimeBetweenMessages > CONTROLLER_DELAY_QUEUE_DELAY_MAX)) {
    MinimalTimeBetweenMessages = CONTROLLER_DELAY_QUEUE_DELAY_DFLT;
  }

  if (MaxQueueDepth > CONTROLLER_DELAY_QUEUE_DEPTH_MAX) { MaxQueueDepth = CONTROLLER_DELAY_QUEUE_DEPTH_DFLT; }

  if (MaxRetry > CONTROLLER_DELAY_QUEUE_RETRY_MAX) { MaxRetry = CONTROLLER_DELAY_QUEUE_RETRY_MAX; }

  if (MaxQueueDepth == 0) { MaxQueueDepth = CONTROLLER_DELAY_QUEUE_DEPTH_DFLT; }

  if (MaxRetry == 0) { MaxRetry = CONTROLLER_DELAY_QUEUE_RETRY_DFLT; }

  if ((ClientTimeout < 10) || (ClientTimeout > CONTROLLER_CLIENTTIMEOUT_MAX)) {
    ClientTimeout = CONTROLLER_CLIENTTIMEOUT_DFLT;
  }
  ZERO_TERMINATE(HostName);
  ZERO_TERMINATE(Publish);
  ZERO_TERMINATE(Subscribe);
  ZERO_TERMINATE(MQTTLwtTopic);
  ZERO_TERMINATE(LWTMessageConnect);
  ZERO_TERMINATE(LWTMessageDisconnect);
}

IPAddress ControllerSettingsStruct::getIP() const {
  IPAddress host(IP[0], IP[1], IP[2], IP[3]);

  return host;
}

String ControllerSettingsStruct::getHost() const {
  if (UseDNS) {
    return HostName;
  }
  return getIP().toString();
}

void ControllerSettingsStruct::setHostname(const String& controllerhostname) {
  safe_strncpy(HostName, controllerhostname.c_str(), sizeof(HostName));
  updateIPcache();
}

bool ControllerSettingsStruct::checkHostReachable(bool quick) {
  if (!isSet()) {
    // No IP/hostname set
    return false;
  }
  if (!NetworkConnected(10)) {
    return false; // Not connected, so no use in wasting time to connect to a host.
  }
  delay(0);       // Make sure the Watchdog will not trigger a reset.

  if (quick && ipSet()) { return true; }

  if (UseDNS) {
    if (!updateIPcache()) {
      return false;
    }
  }
  return hostReachable(getIP());
}

bool ControllerSettingsStruct::connectToHost(WiFiClient& client) {
  if (!checkHostReachable(true)) {
    return false; // Host not reachable
  }
  uint8_t retry     = 2;
  bool connected = false;

  while (retry > 0 && !connected) {
    --retry;
    connected = connectClient(client, getIP(), Port, ClientTimeout);

    if (connected) { return true; }

    if (!checkHostReachable(false)) {
      return false;
    }
  }
  return false;
}

bool ControllerSettingsStruct::beginPacket(WiFiUDP& client) {
  if (!checkHostReachable(true)) {
    return false; // Host not reachable
  }
  uint8_t retry     = 2;
  while (retry > 0) {
    --retry;
    FeedSW_watchdog();
    if (client.beginPacket(getIP(), Port) == 1) {
      return true;
    }

    if (!checkHostReachable(false)) {
      return false;
    }
    delay(10);
  }
  return false;
}

String ControllerSettingsStruct::getHostPortString() const {
  String result = getHost();

  result += ":";
  result += Port;
  return result;
}

bool ControllerSettingsStruct::ipSet() const {
  for (uint8_t i = 0; i < 4; ++i) {
    if (IP[i] != 0) { return true; }
  }
  return false;
}

bool ControllerSettingsStruct::updateIPcache() {
  if (!UseDNS) {
    return true;
  }

  if (!NetworkConnected()) { return false; }
  IPAddress tmpIP;

  if (resolveHostByName(HostName, tmpIP, ClientTimeout)) {
    for (uint8_t x = 0; x < 4; x++) {
      IP[x] = tmpIP[x];
    }
    return true;
  }
  return false;
}

bool ControllerSettingsStruct::mqtt_cleanSession() const
{
  return bitRead(VariousFlags, 1);
}

void ControllerSettingsStruct::mqtt_cleanSession(bool value)
{
  bitWrite(VariousFlags, 1, value);
}

bool ControllerSettingsStruct::mqtt_sendLWT() const
{
  return !bitRead(VariousFlags, 2);
}

void ControllerSettingsStruct::mqtt_sendLWT(bool value)
{
  bitWrite(VariousFlags, 2, !value);
}

bool ControllerSettingsStruct::mqtt_willRetain() const
{
  return !bitRead(VariousFlags, 3);
}

void ControllerSettingsStruct::mqtt_willRetain(bool value)
{
  bitWrite(VariousFlags, 3, !value);
}

bool ControllerSettingsStruct::mqtt_uniqueMQTTclientIdReconnect() const
{
  return bitRead(VariousFlags, 4);
}

void ControllerSettingsStruct::mqtt_uniqueMQTTclientIdReconnect(bool value)
{
  bitWrite(VariousFlags, 4, value);
}

bool ControllerSettingsStruct::mqtt_retainFlag() const
{
  return bitRead(VariousFlags, 5);
}

void ControllerSettingsStruct::mqtt_retainFlag(bool value)
{
  bitWrite(VariousFlags, 5, value);
}

bool ControllerSettingsStruct::useExtendedCredentials() const
{
  return bitRead(VariousFlags, 6);
}

void ControllerSettingsStruct::useExtendedCredentials(bool value)
{
  bitWrite(VariousFlags, 6, value);
}

bool ControllerSettingsStruct::sendBinary() const
{
  return bitRead(VariousFlags, 7);
}

void ControllerSettingsStruct::sendBinary(bool value)
{
  bitWrite(VariousFlags, 7, value);
}

bool ControllerSettingsStruct::enableESPEasyNowFallback() const
{
  return bitRead(VariousFlags, 8);
}

void ControllerSettingsStruct::enableESPEasyNowFallback(bool value)
{
  bitWrite(VariousFlags, 8, value);
}

bool ControllerSettingsStruct::allowExpire() const
{
  return bitRead(VariousFlags, 9);
}

void ControllerSettingsStruct::allowExpire(bool value)
{
  bitWrite(VariousFlags, 9, value);
}

bool ControllerSettingsStruct::deduplicate() const
{
  return bitRead(VariousFlags, 10);
}

void ControllerSettingsStruct::deduplicate(bool value)
{
  bitWrite(VariousFlags, 10, value);
}
