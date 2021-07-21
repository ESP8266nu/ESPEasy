#ifndef GLOBALS_SERVICES_H
#define GLOBALS_SERVICES_H

#include "../../ESPEasy_common.h"

#ifdef FEATURE_ARDUINO_OTA
  //enable Arduino OTA updating.
  //Note: This adds around 10kb to the firmware size, and 1kb extra ram.
  #include <ArduinoOTA.h>

  extern bool ArduinoOTAtriggered;
#endif

#ifdef FEATURE_MDNS
  //enable mDNS mode (adds about 6kb ram and some bytes IRAM)
  #ifdef ESP8266
    #include <ESP8266mDNS.h>
  #endif
  #ifdef ESP32
    #include <ESPmDNS.h>
  #endif
#endif

#ifdef ESP8266

  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>

  extern ESP8266WebServer web_server;
  #ifndef NO_HTTP_UPDATER
  #include <ESP8266HTTPUpdateServer.h>
  extern ESP8266HTTPUpdateServer httpUpdater;
  #endif

#endif

// FIXME TD-er: For now declared in ESPEasy.ino

#ifdef ESP32

  #include <WiFi.h>
  #include <WebServer.h>
  
  extern WebServer web_server;
  #ifndef NO_HTTP_UPDATER
  #include <ESP32HTTPUpdateServer.h>
  extern ESP32HTTPUpdateServer httpUpdater;
  #endif

#endif

#ifdef FEATURE_DNS_SERVER
  #include <DNSServer.h>
  extern DNSServer  dnsServer;
  extern bool dnsServerActive;
#endif


#endif // GLOBALS_SERVICES_H
  
