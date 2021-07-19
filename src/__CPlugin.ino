#include "ESPEasy_common.h"

#include "src/DataStructs/ESPEasy_EventStruct.h"
#include "src/DataStructs/TimingStats.h"

#include "src/DataTypes/ESPEasy_plugin_functions.h"

#include "src/Globals/CPlugins.h"
#include "src/Globals/Protocol.h"
#include "src/Globals/Settings.h"

#include "src/Helpers/Misc.h"

// ********************************************************************************
// Initialize all Controller CPlugins that where defined earlier
// and initialize the function call pointer into the CCPlugin array
// ********************************************************************************

// Uncrustify must not be used on macros, so turn it off.
// *INDENT-OFF*
#define ADDCPLUGIN(NNN) if (addCPlugin(CPLUGIN_ID_##NNN, x)) { CPlugin_ptr[x++] = &CPlugin_##NNN; }
// Uncrustify must not be used on macros, but we're now done, so turn Uncrustify on again.
// *INDENT-ON*

void CPluginInit(void)
{
  #ifdef USE_SECOND_HEAP
  HeapSelectDram ephemeral;
  #endif
  ProtocolIndex_to_CPlugin_id[CPLUGIN_MAX] = INVALID_C_PLUGIN_ID;
  uint8_t x;

  // Clear pointer table for all plugins
  for (x = 0; x < CPLUGIN_MAX; x++)
  {
    CPlugin_ptr[x]                 = nullptr;
    ProtocolIndex_to_CPlugin_id[x] = INVALID_C_PLUGIN_ID;
    // Do not initialize CPlugin_id_to_ProtocolIndex[x] to an invalid value. (it is map)
  }

  x = 0;

#ifdef CPLUGIN_001
  ADDCPLUGIN(001)
#endif

#ifdef CPLUGIN_002
  ADDCPLUGIN(002)
#endif

#ifdef CPLUGIN_003
  ADDCPLUGIN(003)
#endif

#ifdef CPLUGIN_004
  ADDCPLUGIN(004)
#endif

#ifdef CPLUGIN_005
  ADDCPLUGIN(005)
#endif

#ifdef CPLUGIN_006
  ADDCPLUGIN(006)
#endif

#ifdef CPLUGIN_007
  ADDCPLUGIN(007)
#endif

#ifdef CPLUGIN_008
  ADDCPLUGIN(008)
#endif

#ifdef CPLUGIN_009
  ADDCPLUGIN(009)
#endif

#ifdef CPLUGIN_010
  ADDCPLUGIN(010)
#endif

#ifdef CPLUGIN_011
  ADDCPLUGIN(011)
#endif

#ifdef CPLUGIN_012
  ADDCPLUGIN(012)
#endif

#ifdef CPLUGIN_013
  ADDCPLUGIN(013)
#endif

#ifdef CPLUGIN_014
  ADDCPLUGIN(014)
#endif

#ifdef CPLUGIN_015
  ADDCPLUGIN(015)
#endif

#ifdef CPLUGIN_016
  ADDCPLUGIN(016)
#endif

#ifdef CPLUGIN_017
  ADDCPLUGIN(017)
#endif

#ifdef CPLUGIN_018
  ADDCPLUGIN(018)
#endif

#ifdef CPLUGIN_019
  ADDCPLUGIN(019)
#endif

#ifdef CPLUGIN_020
  ADDCPLUGIN(020)
#endif

#ifdef CPLUGIN_021
  ADDCPLUGIN(021)
#endif

#ifdef CPLUGIN_022
  ADDCPLUGIN(022)
#endif

#ifdef CPLUGIN_023
  ADDCPLUGIN(023)
#endif

#ifdef CPLUGIN_024
  ADDCPLUGIN(024)
#endif

#ifdef CPLUGIN_025
  ADDCPLUGIN(025)
#endif

// When extending this, search for EXTEND_CONTROLLER_IDS 
// in the code to find all places that need to be updated too.

  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("ADDCPLUGIN(...)"));
  #endif

  CPluginCall(CPlugin::Function::CPLUGIN_PROTOCOL_ADD, 0);
  #ifndef BUILD_NO_RAM_TRACKER
  logMemUsageAfter(F("CPLUGIN_PROTOCOL_ADD"));
  #endif


  // Set all not supported cplugins to disabled.
  for (controllerIndex_t controller = 0; controller < CONTROLLER_MAX; ++controller) {
    if (!supportedCPluginID(Settings.Protocol[controller])) {
      Settings.ControllerEnabled[controller] = false;
    }
  }
  CPluginCall(CPlugin::Function::CPLUGIN_INIT_ALL, 0);
}
