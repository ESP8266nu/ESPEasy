#ifndef DATASTRUCTS_CONTROLLERSETTINGSSTRUCT_H
#define DATASTRUCTS_CONTROLLERSETTINGSSTRUCT_H

/*********************************************************************************************\
* ControllerSettingsStruct definition
\*********************************************************************************************/
#include <Arduino.h>
#include <memory> // For std::shared_ptr
#include <new> // for std::nothrow

#include "../../ESPEasy_common.h"
#include "../Globals/Plugins.h"

class IPAddress;
class WiFiClient;
class WiFiUDP;

// Minimum delay between messages for a controller to send in msec.
#ifndef CONTROLLER_DELAY_QUEUE_DELAY_MAX
# define CONTROLLER_DELAY_QUEUE_DELAY_MAX   3600000
#endif // ifndef CONTROLLER_DELAY_QUEUE_DELAY_MAX
#ifndef CONTROLLER_DELAY_QUEUE_DELAY_DFLT
# define CONTROLLER_DELAY_QUEUE_DELAY_DFLT  100
#endif // ifndef CONTROLLER_DELAY_QUEUE_DELAY_DFLT

// Queue length for controller messages not yet sent.
#ifndef CONTROLLER_DELAY_QUEUE_DEPTH_MAX
# define CONTROLLER_DELAY_QUEUE_DEPTH_MAX   50
#endif // ifndef CONTROLLER_DELAY_QUEUE_DEPTH_MAX
#ifndef CONTROLLER_DELAY_QUEUE_DEPTH_DFLT
# define CONTROLLER_DELAY_QUEUE_DEPTH_DFLT  10
#endif // ifndef CONTROLLER_DELAY_QUEUE_DEPTH_DFLT

// Number of retries to send a message by a controller.
// N.B. Retries without a connection to wifi do not count as retry.
#ifndef CONTROLLER_DELAY_QUEUE_RETRY_MAX
# define CONTROLLER_DELAY_QUEUE_RETRY_MAX   10
#endif // ifndef CONTROLLER_DELAY_QUEUE_RETRY_MAX
#ifndef CONTROLLER_DELAY_QUEUE_RETRY_DFLT
# define CONTROLLER_DELAY_QUEUE_RETRY_DFLT  10
#endif // ifndef CONTROLLER_DELAY_QUEUE_RETRY_DFLT

// Timeout of the client in msec.
#ifndef CONTROLLER_CLIENTTIMEOUT_MAX
# define CONTROLLER_CLIENTTIMEOUT_MAX     4000 // Not sure if this may trigger SW watchdog.
#endif // ifndef CONTROLLER_CLIENTTIMEOUT_MAX
#ifndef CONTROLLER_CLIENTTIMEOUT_DFLT
# define CONTROLLER_CLIENTTIMEOUT_DFLT     100
#endif // ifndef CONTROLLER_CLIENTTIMEOUT_DFLT

#ifndef CONTROLLER_DEFAULT_CLIENTID
# define CONTROLLER_DEFAULT_CLIENTID  "%sysname%_%unit%"
#endif // ifndef CONTROLLER_DEFAULT_CLIENTID

struct ControllerSettingsStruct
{
  // ********************************************************************************
  //   IDs of controller settings, used to generate web forms
  // ********************************************************************************
  enum VarType {
    CONTROLLER_USE_DNS                  = 0, // PLace this before HOSTNAME/IP
    CONTROLLER_USE_EXTENDED_CREDENTIALS = 1, // Place this before USER/PASS
    CONTROLLER_HOSTNAME,
    CONTROLLER_IP,
    CONTROLLER_PORT,
    CONTROLLER_USER,
    CONTROLLER_PASS,
    CONTROLLER_MIN_SEND_INTERVAL,
    CONTROLLER_MAX_QUEUE_DEPTH,
    CONTROLLER_MAX_RETRIES,
    CONTROLLER_FULL_QUEUE_ACTION,
    CONTROLLER_ALLOW_EXPIRE,
    CONTROLLER_DEDUPLICATE,
    CONTROLLER_CHECK_REPLY,
    CONTROLLER_CLIENT_ID,
    CONTROLLER_UNIQUE_CLIENT_ID_RECONNECT,
    CONTROLLER_RETAINFLAG,
    CONTROLLER_SUBSCRIBE,
    CONTROLLER_PUBLISH,
    CONTROLLER_LWT_TOPIC,
    CONTROLLER_LWT_CONNECT_MESSAGE,
    CONTROLLER_LWT_DISCONNECT_MESSAGE,
    CONTROLLER_SEND_LWT,
    CONTROLLER_WILL_RETAIN,
    CONTROLLER_CLEAN_SESSION,
    CONTROLLER_TIMEOUT,
    CONTROLLER_SAMPLE_SET_INITIATOR,
    CONTROLLER_SEND_BINARY,
    CONTROLLER_ENABLE_ESPEASY_NOW_FALLBACK,

    // Keep this as last, is used to loop over all parameters
    CONTROLLER_ENABLED
  };


  ControllerSettingsStruct();

  void      reset();

  bool      isSet() const;

  void      validate();

  IPAddress getIP() const;

  String    getHost() const;

  void      setHostname(const String& controllerhostname);

  bool      checkHostReachable(bool quick);

  bool      connectToHost(WiFiClient& client);

  bool      beginPacket(WiFiUDP& client);

  String    getHostPortString() const;

  // VariousFlags defaults to 0, keep in mind when adding bit lookups.
  bool      mqtt_cleanSession() const;
  void      mqtt_cleanSession(bool value);

  bool      mqtt_sendLWT() const;
  void      mqtt_sendLWT(bool value);

  bool      mqtt_willRetain() const;
  void      mqtt_willRetain(bool value);

  bool      mqtt_uniqueMQTTclientIdReconnect() const;
  void      mqtt_uniqueMQTTclientIdReconnect(bool value);

  bool      mqtt_retainFlag() const;
  void      mqtt_retainFlag(bool value);

  bool      useExtendedCredentials() const;
  void      useExtendedCredentials(bool value);

  bool      sendBinary() const;
  void      sendBinary(bool value);

  bool      enableESPEasyNowFallback() const;
  void      enableESPEasyNowFallback(bool value);

  bool      allowExpire() const;
  void      allowExpire(bool value);

  bool      deduplicate() const;
  void      deduplicate(bool value);

  bool         UseDNS;
  uint8_t      IP[4];
  unsigned int Port;
  char         HostName[65];
  char         Publish[129];
  char         Subscribe[129];
  char         MQTTLwtTopic[129];
  char         LWTMessageConnect[129];
  char         LWTMessageDisconnect[129];
  unsigned int MinimalTimeBetweenMessages;
  unsigned int MaxQueueDepth;
  unsigned int MaxRetry;
  bool         DeleteOldest;       // Action to perform when buffer full, delete oldest, or ignore newest.
  unsigned int ClientTimeout;
  bool         MustCheckReply;     // When set to false, a sent message is considered always successful.
  taskIndex_t  SampleSetInitiator; // The first task to start a sample set.
  uint32_t     VariousFlags;       // Various flags
  char         ClientID[65];       // Used to define the Client ID used by the controller

private:

  bool ipSet() const;

  bool updateIPcache();
};

typedef std::shared_ptr<ControllerSettingsStruct> ControllerSettingsStruct_ptr_type;
#define MakeControllerSettings(T) ControllerSettingsStruct_ptr_type ControllerSettingsStruct_ptr(new (std::nothrow)  ControllerSettingsStruct()); \
  ControllerSettingsStruct& T = *ControllerSettingsStruct_ptr;

// Check to see if MakeControllerSettings was successful
#define AllocatedControllerSettings() (ControllerSettingsStruct_ptr.get() != nullptr)

#endif // DATASTRUCTS_CONTROLLERSETTINGSSTRUCT_H
