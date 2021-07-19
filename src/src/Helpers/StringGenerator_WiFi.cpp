#include "../Helpers/StringGenerator_WiFi.h"

#include "../Globals/ESPEasyWiFiEvent.h"

const __FlashStringHelper * WiFi_encryptionType(uint8_t encryptionType) {
switch (encryptionType) {
  #ifdef ESP32
    case WIFI_AUTH_OPEN:             return F("open"); 
    case WIFI_AUTH_WEP:              return F("WEP"); 
    case WIFI_AUTH_WPA_PSK:          return F("WPA/PSK"); 
    case WIFI_AUTH_WPA2_PSK:         return F("WPA2/PSK"); 
    case WIFI_AUTH_WPA_WPA2_PSK:     return F("WPA/WPA2/PSK"); 
    case WIFI_AUTH_WPA2_ENTERPRISE:  return F("WPA2 Enterprise"); 
  #else // ifdef ESP32
    case ENC_TYPE_WEP:   return F("WEP"); 
    case ENC_TYPE_TKIP:  return F("WPA/PSK"); 
    case ENC_TYPE_CCMP:  return F("WPA2/PSK"); 
    case ENC_TYPE_NONE:  return F("open"); 
    case ENC_TYPE_AUTO:  return F("WPA/WPA2/PSK"); 
  #endif // ifdef ESP32
    default:
	break;
      
  }
  return F("-");
}


#ifdef ESP8266
#ifdef LIMIT_BUILD_SIZE
String SDKwifiStatusToString(uint8_t sdk_wifistatus)
{
  return String(sdk_wifistatus);
}
#else
const __FlashStringHelper * SDKwifiStatusToString(uint8_t sdk_wifistatus)
{
  switch (sdk_wifistatus) {
    case STATION_IDLE:           return F("STATION_IDLE");
    case STATION_CONNECTING:     return F("STATION_CONNECTING");
    case STATION_WRONG_PASSWORD: return F("STATION_WRONG_PASSWORD");
    case STATION_NO_AP_FOUND:    return F("STATION_NO_AP_FOUND");
    case STATION_CONNECT_FAIL:   return F("STATION_CONNECT_FAIL");
    case STATION_GOT_IP:         return F("STATION_GOT_IP");
  }
  return F("Unknown");
}
#endif
#endif

const __FlashStringHelper * ArduinoWifiStatusToFlashString(uint8_t arduino_corelib_wifistatus) {
  switch (arduino_corelib_wifistatus) {
    case WL_NO_SHIELD:       return F("WL_NO_SHIELD");
    case WL_IDLE_STATUS:     return F("WL_IDLE_STATUS");
    case WL_NO_SSID_AVAIL:   return F("WL_NO_SSID_AVAIL");
    case WL_SCAN_COMPLETED:  return F("WL_SCAN_COMPLETED");
    case WL_CONNECTED:       return F("WL_CONNECTED");
    case WL_CONNECT_FAILED:  return F("WL_CONNECT_FAILED");
    case WL_CONNECTION_LOST: return F("WL_CONNECTION_LOST");
    case WL_DISCONNECTED:    return F("WL_DISCONNECTED");
  }
  return F("-");
}

String ArduinoWifiStatusToString(uint8_t arduino_corelib_wifistatus) {
  #ifdef LIMIT_BUILD_SIZE
  return String(arduino_corelib_wifistatus);
  #else
  String log = ArduinoWifiStatusToFlashString(arduino_corelib_wifistatus);
  log += ' ';
  log += arduino_corelib_wifistatus;
  return log;
  #endif
}


const __FlashStringHelper * getLastDisconnectReason(WiFiDisconnectReason reason) {
  switch (reason) {
    case WIFI_DISCONNECT_REASON_UNSPECIFIED:                return F("Unspecified");              
    case WIFI_DISCONNECT_REASON_AUTH_EXPIRE:                return F("Auth expire");              
    case WIFI_DISCONNECT_REASON_AUTH_LEAVE:                 return F("Auth leave");               
    case WIFI_DISCONNECT_REASON_ASSOC_EXPIRE:               return F("Assoc expire");             
    case WIFI_DISCONNECT_REASON_ASSOC_TOOMANY:              return F("Assoc toomany");            
    case WIFI_DISCONNECT_REASON_NOT_AUTHED:                 return F("Not authed");               
    case WIFI_DISCONNECT_REASON_NOT_ASSOCED:                return F("Not assoced");              
    case WIFI_DISCONNECT_REASON_ASSOC_LEAVE:                return F("Assoc leave");              
    case WIFI_DISCONNECT_REASON_ASSOC_NOT_AUTHED:           return F("Assoc not authed");         
    case WIFI_DISCONNECT_REASON_DISASSOC_PWRCAP_BAD:        return F("Disassoc pwrcap bad");      
    case WIFI_DISCONNECT_REASON_DISASSOC_SUPCHAN_BAD:       return F("Disassoc supchan bad");     
    case WIFI_DISCONNECT_REASON_IE_INVALID:                 return F("IE invalid");               
    case WIFI_DISCONNECT_REASON_MIC_FAILURE:                return F("Mic failure");              
    case WIFI_DISCONNECT_REASON_4WAY_HANDSHAKE_TIMEOUT:     return F("4way handshake timeout");   
    case WIFI_DISCONNECT_REASON_GROUP_KEY_UPDATE_TIMEOUT:   return F("Group key update timeout"); 
    case WIFI_DISCONNECT_REASON_IE_IN_4WAY_DIFFERS:         return F("IE in 4way differs");       
    case WIFI_DISCONNECT_REASON_GROUP_CIPHER_INVALID:       return F("Group cipher invalid");     
    case WIFI_DISCONNECT_REASON_PAIRWISE_CIPHER_INVALID:    return F("Pairwise cipher invalid");  
    case WIFI_DISCONNECT_REASON_AKMP_INVALID:               return F("AKMP invalid");             
    case WIFI_DISCONNECT_REASON_UNSUPP_RSN_IE_VERSION:      return F("Unsupp RSN IE version");    
    case WIFI_DISCONNECT_REASON_INVALID_RSN_IE_CAP:         return F("Invalid RSN IE cap");       
    case WIFI_DISCONNECT_REASON_802_1X_AUTH_FAILED:         return F("802 1X auth failed");       
    case WIFI_DISCONNECT_REASON_CIPHER_SUITE_REJECTED:      return F("Cipher suite rejected");    
    case WIFI_DISCONNECT_REASON_BEACON_TIMEOUT:             return F("Beacon timeout");           
    case WIFI_DISCONNECT_REASON_NO_AP_FOUND:                return F("No AP found");              
    case WIFI_DISCONNECT_REASON_AUTH_FAIL:                  return F("Auth fail");                
    case WIFI_DISCONNECT_REASON_ASSOC_FAIL:                 return F("Assoc fail");               
    case WIFI_DISCONNECT_REASON_HANDSHAKE_TIMEOUT:          return F("Handshake timeout");        
    default:  return F("Unknown");
  }
}

String getLastDisconnectReason() {
  String reason;
  reason += '(';

  reason += WiFiEventData.lastDisconnectReason;
  reason += ')';

  #ifndef LIMIT_BUILD_SIZE
  reason += ' ';
  reason += getLastDisconnectReason(WiFiEventData.lastDisconnectReason);
  #endif
  return reason;
}
