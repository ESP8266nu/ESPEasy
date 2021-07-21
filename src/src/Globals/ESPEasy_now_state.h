#ifndef GLOBALS_ESPEASY_NOW_STATE_H
#define GLOBALS_ESPEASY_NOW_STATE_H

#include "../../ESPEasy_common.h"

#ifdef USES_ESPEASY_NOW

# include <WifiEspNow.h>

extern bool use_EspEasy_now;
extern bool plugin_EspEasy_now_active;
extern unsigned long temp_disable_EspEasy_now_timer;


#endif // ifdef USES_ESPEASY_NOW

#endif // GLOBALS_ESPEASY_NOW_STATE_H
