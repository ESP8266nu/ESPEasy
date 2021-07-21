#ifndef CUSTOMBUILD_ESPEASY_LIMITS_H
#define CUSTOMBUILD_ESPEASY_LIMITS_H

#include "../../ESPEasy_common.h"

// ***********************************************************************
// * These limits have direct impact on the settings files
// * Do not change them!
// * Else settings files will no longer be compatible with official builds
// * Some of these are related to the values defined in StorageLayout.h
// ***********************************************************************


// Performing a 2-stage define assignment using the _TMP defines
// See: https://github.com/letscontrolit/ESPEasy/issues/2621
#ifdef USE_NON_STANDARD_24_TASKS
  #define TASKS_MAX_TMP                      24
#else
  #define TASKS_MAX_TMP                      12
#endif


#if defined(ESP8266)
  #ifndef TASKS_MAX
    #define TASKS_MAX                          TASKS_MAX_TMP
  #endif
  #ifndef MAX_GPIO
    #define MAX_GPIO                           16
  #endif
#endif
#if defined(ESP32)
  #ifndef TASKS_MAX
    #define TASKS_MAX                          32
  #endif
  #ifndef MAX_GPIO
    #define MAX_GPIO                           39
  #endif
#endif

#ifndef CONTROLLER_MAX
  #define CONTROLLER_MAX                      3 // max 4!
#endif
#ifndef NOTIFICATION_MAX
  #define NOTIFICATION_MAX                    3 // max 4!
#endif
#ifndef VARS_PER_TASK
  #define VARS_PER_TASK                       4
#endif
#ifndef PLUGIN_CONFIGVAR_MAX
  #define PLUGIN_CONFIGVAR_MAX                8
#endif
#ifndef PLUGIN_CONFIGFLOATVAR_MAX
  #define PLUGIN_CONFIGFLOATVAR_MAX           4
#endif
#ifndef PLUGIN_CONFIGLONGVAR_MAX
  #define PLUGIN_CONFIGLONGVAR_MAX            4
#endif
#ifndef PLUGIN_EXTRACONFIGVAR_MAX
  #define PLUGIN_EXTRACONFIGVAR_MAX          16
#endif
#ifndef NAME_FORMULA_LENGTH_MAX
  #define NAME_FORMULA_LENGTH_MAX            40
#endif



// ***********************************************************************
// * ESPEasy-NOW related limits
// ***********************************************************************

#ifndef ESPEASY_NOW_PEER_MAX
  #define ESPEASY_NOW_PEER_MAX                10 // 10 when using encryption, 20 without encryption
#endif


// ***********************************************************************
// * The next limits affect memory usage
// ***********************************************************************
#ifndef DEVICES_MAX
  // TODO TD-er: This should be set automatically by counting the number of included plugins.
  #if defined(PLUGIN_BUILD_TESTING) || defined(PLUGIN_BUILD_DEV)
    # define DEVICES_MAX                      95
  #else    // if defined(PLUGIN_BUILD_TESTING) || defined(PLUGIN_BUILD_DEV)
    # ifdef ESP32
      # ifdef PLUGIN_BUILD_MAX_ESP32
        #  define DEVICES_MAX                      125
      # else // ifdef PLUGIN_BUILD_MAX_ESP32
      #  define DEVICES_MAX                      100
      # endif // ifdef PLUGIN_BUILD_MAX_ESP32
    # else // ifdef ESP32
      #  define DEVICES_MAX                      60
    # endif // ifdef ESP32
  #endif // if defined(PLUGIN_BUILD_TESTING) || defined(PLUGIN_BUILD_DEV)
#endif

#ifndef PLUGIN_MAX
  #define PLUGIN_MAX                DEVICES_MAX
#endif
#ifndef CPLUGIN_MAX
  #define CPLUGIN_MAX                        20
#endif
#ifndef NPLUGIN_MAX
  #define NPLUGIN_MAX                         4
#endif

#ifndef UNIT_NUMBER_MAX
  #define UNIT_NUMBER_MAX                   254 // Stored in Settings.Unit  unit 255 = broadcast
#endif


// ***********************************************************************
// * Limits regarding Rules
// ***********************************************************************

#ifndef RULES_TIMER_MAX
  #define RULES_TIMER_MAX                   256
#endif
//#ifndef PINSTATE_TABLE_MAX
//#define PINSTATE_TABLE_MAX                 32
//#endif
#ifndef RULES_MAX_SIZE
  #define RULES_MAX_SIZE                   2048
#endif
#ifndef RULES_MAX_NESTING_LEVEL
  #define RULES_MAX_NESTING_LEVEL             3
#endif
#ifndef RULESETS_MAX
  #define RULESETS_MAX                        4
#endif
#ifndef RULES_BUFFER_SIZE
  #define RULES_BUFFER_SIZE                  64
#endif

#ifndef RULES_IF_MAX_NESTING_LEVEL
  #define RULES_IF_MAX_NESTING_LEVEL          4
#endif


// ***********************************************************************
// * Extended SecuritySettings
// ***********************************************************************
#ifndef EXT_SECURITY_MAX_USER_LENGTH
  #define EXT_SECURITY_MAX_USER_LENGTH        128
#endif
#ifndef EXT_SECURITY_MAX_PASS_LENGTH
  #define EXT_SECURITY_MAX_PASS_LENGTH        128
#endif

// ***********************************************************************
// * Other operational limits
// ***********************************************************************

#ifndef MAX_FLASHWRITES_PER_DAY
  #define MAX_FLASHWRITES_PER_DAY           100 // per 24 hour window
#endif
#ifndef UDP_PACKETSIZE_MAX
  #define UDP_PACKETSIZE_MAX               256 // Currently only needed for C013_Receive
#endif
#ifndef TIMER_GRATUITOUS_ARP_MAX
  #define TIMER_GRATUITOUS_ARP_MAX           5000
#endif

#define DOMOTICZ_MAX_IDX            999999999 // Looks like it is an unsigned int, so could be up to 4 bln.


#endif // CUSTOMBUILD_ESPEASY_LIMITS_H
