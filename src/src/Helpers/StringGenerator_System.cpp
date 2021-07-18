#include "../Helpers/StringGenerator_System.h"


#include <Arduino.h>


/*********************************************************************************************\
   ESPEasy specific strings
\*********************************************************************************************/


#ifdef USES_MQTT

#include <PubSubClient.h>
#include "../Globals/MQTT.h"

const __FlashStringHelper * getMQTT_state() {
  switch (MQTTclient.state()) {
    case MQTT_CONNECTION_TIMEOUT: return F("Connection timeout");
    case MQTT_CONNECTION_LOST: return F("Connection lost");
    case MQTT_CONNECT_FAILED: return F("Connect failed");
    case MQTT_DISCONNECTED: return F("Disconnected");
    case MQTT_CONNECTED: return F("Connected");
    case MQTT_CONNECT_BAD_PROTOCOL: return F("Connect bad protocol");
    case MQTT_CONNECT_BAD_CLIENT_ID: return F("Connect bad client_id");
    case MQTT_CONNECT_UNAVAILABLE: return F("Connect unavailable");
    case MQTT_CONNECT_BAD_CREDENTIALS: return F("Connect bad credentials");
    case MQTT_CONNECT_UNAUTHORIZED: return F("Connect unauthorized");
    default: break;
  }
  return F("");
}

#endif // USES_MQTT

/********************************************************************************************\
   Get system information
 \*********************************************************************************************/
const __FlashStringHelper * getLastBootCauseString() {
  switch (lastBootCause)
  {
    case BOOT_CAUSE_MANUAL_REBOOT: return F("Manual Reboot");
    case BOOT_CAUSE_DEEP_SLEEP:    return F("Deep Sleep");
    case BOOT_CAUSE_COLD_BOOT:     return F("Cold Boot");
    case BOOT_CAUSE_EXT_WD:        return F("External Watchdog");
    case BOOT_CAUSE_SOFT_RESTART:  return F("Soft Reboot");
    case BOOT_CAUSE_SW_WATCHDOG:   return F("SW Watchdog");
    case BOOT_CAUSE_EXCEPTION:     return F("Exception");
    case BOOT_CAUSE_POWER_UNSTABLE: return F("PWR Unstable"); // ESP32 only
  }
  return F("Unknown");
}

#ifdef ESP32
 #ifdef ESP32S2
  #include <esp32s2/rom/rtc.h>
 #else
  #include <rom/rtc.h>
 #endif


// See https://github.com/espressif/esp-idf/blob/master/components/esp32/include/rom/rtc.h
String getResetReasonString(uint8_t icore) {
  bool isDEEPSLEEP_RESET(false);

  #ifdef ESP32S2

	// See tools\sdk\esp32\include\esp_rom\include\esp32s2\rom\rtc.h
  switch (rtc_get_reset_reason(icore)) {
    case POWERON_RESET:          return F("Vbat power on reset");                              // 1
    case RTC_SW_SYS_RESET:       return F("Software reset digital core");                      // 3
    case DEEPSLEEP_RESET:        isDEEPSLEEP_RESET = true; break;                              // 5
    case TG0WDT_SYS_RESET:       return F("Timer Group0 Watch dog reset digital core");        // 7
    case TG1WDT_SYS_RESET:       return F("Timer Group1 Watch dog reset digital core");        // 8
    case RTCWDT_SYS_RESET:       return F("RTC Watch dog Reset digital core");                 // 9
    case INTRUSION_RESET:        return F("Instrusion tested to reset CPU");                   // 10
    case TG0WDT_CPU_RESET:       return F("Time Group0 reset CPU");                            // 11
    case RTC_SW_CPU_RESET:       return F("Software reset CPU");                               // 12
    case RTCWDT_CPU_RESET:       return F("RTC Watch dog Reset CPU");                          // 13
    case RTCWDT_BROWN_OUT_RESET: return F("Reset when the vdd voltage is not stable");         // 15
    case RTCWDT_RTC_RESET:       return F("RTC Watch dog reset digital core and rtc module");  // 16
    case TG1WDT_CPU_RESET:       return F("Time Group1 reset CPU");                            // 17
    case SUPER_WDT_RESET:        return F("Super watchdog reset digital core and rtc module"); // 18
    case GLITCH_RTC_RESET:       return F("Glitch reset digital core and rtc module");         // 19
  }

  #else

  // See https://github.com/espressif/esp-idf/blob/master/components/esp32/include/rom/rtc.h
  switch (rtc_get_reset_reason((RESET_REASON)icore)) {
    case NO_MEAN:                return F("NO_MEAN");
    case POWERON_RESET:          return F("Vbat power on reset");
    case SW_RESET:               return F("Software reset digital core");
    case OWDT_RESET:             return F("Legacy watch dog reset digital core");
    case DEEPSLEEP_RESET:        isDEEPSLEEP_RESET = true; break;
    case SDIO_RESET:             return F("Reset by SLC module, reset digital core");
    case TG0WDT_SYS_RESET:       return F("Timer Group0 Watch dog reset digital core");
    case TG1WDT_SYS_RESET:       return F("Timer Group1 Watch dog reset digital core");
    case RTCWDT_SYS_RESET:       return F("RTC Watch dog Reset digital core");
    case INTRUSION_RESET:        return F("Instrusion tested to reset CPU");
    case TGWDT_CPU_RESET:        return F("Time Group reset CPU");
    case SW_CPU_RESET:           return F("Software reset CPU");
    case RTCWDT_CPU_RESET:       return F("RTC Watch dog Reset CPU");
    case EXT_CPU_RESET:          return F("for APP CPU, reseted by PRO CPU");
    case RTCWDT_BROWN_OUT_RESET: return F("Reset when the vdd voltage is not stable");
    case RTCWDT_RTC_RESET:       return F("RTC Watch dog reset digital core and rtc module");
    default: break;
  }
  #endif

  if (isDEEPSLEEP_RESET) {
    String reason = F("Deep Sleep, Wakeup reason (");
    reason += rtc_get_wakeup_cause();
    reason += ')';

/*
  switch (reason) {
  #if CONFIG_IDF_TARGET_ESP32
    case POWERON_RESET:
    case SW_CPU_RESET:
    case DEEPSLEEP_RESET:
    case SW_RESET:
  #elif CONFIG_IDF_TARGET_ESP32S2
    case POWERON_RESET:
    case RTC_SW_CPU_RESET:
    case DEEPSLEEP_RESET:
    case RTC_SW_SYS_RESET:
  #endif
  }
*/
    return reason;
  }

  return getUnknownString();
}

#endif // ifdef ESP32

String getResetReasonString() {
  #ifdef ESP32
  String reason = F("CPU0: ");
  reason += getResetReasonString(0);
  reason += F(" CPU1: ");
  reason += getResetReasonString(1);
  return reason;
  #else // ifdef ESP32
  return ESP.getResetReason();
  #endif // ifdef ESP32
}

String getSystemBuildString() {
  String result;

  result += BUILD;
  result += ' ';
  result += F(BUILD_NOTES);
  return result;
}

String getPluginDescriptionString() {
  String result;

  #ifdef PLUGIN_BUILD_NORMAL
  result += F(" [Normal]");
  #endif // ifdef PLUGIN_BUILD_NORMAL
  #ifdef PLUGIN_BUILD_TESTING
  result += F(" [Testing]");
  #endif // ifdef PLUGIN_BUILD_TESTING
  #ifdef PLUGIN_BUILD_DEV
  result += F(" [Development]");
  #endif // ifdef PLUGIN_BUILD_DEV
  #ifdef PLUGIN_DESCR
  result += " [";
  result += F(PLUGIN_DESCR);
  result += ']';
  #endif // ifdef PLUGIN_DESCR
  #ifdef USE_NON_STANDARD_24_TASKS
  result += F(" 24tasks");
  #endif // ifdef USE_NON_STANDARD_24_TASKS
  result.trim();
  return result;
}

String getSystemLibraryString() {
  String result;

  #if defined(ESP32)
  result += F("ESP32 SDK ");
  result += ESP.getSdkVersion();
  #else // if defined(ESP32)
  result += F("ESP82xx Core ");
  result += ESP.getCoreVersion();
  result += F(", NONOS SDK ");
  result += system_get_sdk_version();
  result += F(", LWIP: ");
  result += getLWIPversion();
  #endif // if defined(ESP32)

  if (puyaSupport()) {
    result += F(" PUYA support");
  }
  return result;
}

#ifdef ESP8266
String getLWIPversion() {
  String result;

  result += LWIP_VERSION_MAJOR;
  result += '.';
  result += LWIP_VERSION_MINOR;
  result += '.';
  result += LWIP_VERSION_REVISION;

  if (LWIP_VERSION_IS_RC) {
    result += F("-RC");
    result += LWIP_VERSION_RC;
  } else if (LWIP_VERSION_IS_DEVELOPMENT) {
    result += F("-dev");
  }
  return result;
}

#endif // ifdef ESP8266
