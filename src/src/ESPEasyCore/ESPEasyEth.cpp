#include "../ESPEasyCore/ESPEasyEth.h"

#ifdef HAS_ETHERNET

#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#include "../Globals/NetworkState.h"
#include "../Globals/Settings.h"
#include "../Helpers/StringConverter.h"

#include <ETH.h>
#if ESP_IDF_VERSION_MAJOR > 3
 #include <esp_eth_phy.h>
#else
 #include <eth_phy/phy.h>
#endif

bool ethUseStaticIP() {
  return Settings.ETH_IP[0] != 0 && Settings.ETH_IP[0] != 255;
}

void ethSetupStaticIPconfig() {
  if (!ethUseStaticIP()) { 
    const IPAddress IP_zero(0, 0, 0, 0); 
    ETH.config(IP_zero, IP_zero, IP_zero);
    return; 
  }
  const IPAddress ip     = Settings.ETH_IP;
  const IPAddress gw     = Settings.ETH_Gateway;
  const IPAddress subnet = Settings.ETH_Subnet;
  const IPAddress dns    = Settings.ETH_DNS;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("ETH IP   : Static IP : ");
    log += formatIP(ip);
    log += F(" GW: ");
    log += formatIP(gw);
    log += F(" SN: ");
    log += formatIP(subnet);
    log += F(" DNS: ");
    log += formatIP(dns);
    addLog(LOG_LEVEL_INFO, log);
  }
  ETH.config(ip, gw, subnet, dns);
}

bool ethCheckSettings() {
  return isValid(Settings.ETH_Phy_Type) 
      && isValid(Settings.ETH_Clock_Mode)
      && isValid(Settings.NetworkMedium)
      && (Settings.ETH_Pin_mdc   <= MAX_GPIO)
      && (Settings.ETH_Pin_mdio  <= MAX_GPIO)
      && (Settings.ETH_Pin_power <= MAX_GPIO);
}

bool ethPrepare() {
  {
    char hostname[40];
    safe_strncpy(hostname, NetworkCreateRFCCompliantHostname().c_str(), sizeof(hostname));
    ETH.setHostname(hostname);
  }
  ethSetupStaticIPconfig();
  return true;
}

void ethPrintSettings() {
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(115);
//    log += F("ETH/Wifi mode: ");
//    log += toString(active_network_medium);
    log += F("ETH PHY Type: ");
    log += toString(Settings.ETH_Phy_Type);
    log += F(" PHY Addr: ");
    log += Settings.ETH_Phy_Addr;
    log += F(" Eth Clock mode: ");
    log += toString(Settings.ETH_Clock_Mode);
    log += F(" MDC Pin: ");
    log += String(Settings.ETH_Pin_mdc);
    log += F(" MIO Pin: ");
    log += String(Settings.ETH_Pin_mdio);
    log += F(" Power Pin: ");
    log += String(Settings.ETH_Pin_power);
    addLog(LOG_LEVEL_INFO, log);
  }
}

MAC_address ETHMacAddress() {
  MAC_address mac;
  if(!EthEventData.ethInitSuccess) {
    addLog(LOG_LEVEL_ERROR, F("Call NetworkMacAddress() only on connected Ethernet!"));
  } else {
    #if ESP_IDF_VERSION_MAJOR > 3
    ETH.macAddress(mac.mac);
    #else
    esp_eth_get_mac(mac.mac);
    #endif
  }
  return mac;
}

bool ETHConnectRelaxed() {
  if (EthEventData.ethInitSuccess) {
    return EthLinkUp();
  }
  ethPrintSettings();
  if (!ethCheckSettings())
  {
    addLog(LOG_LEVEL_ERROR, F("ETH: Settings not correct!!!"));
    EthEventData.ethInitSuccess = false;
    return false;
  }
  EthEventData.markEthBegin();
  EthEventData.ethInitSuccess = ETH.begin( 
    Settings.ETH_Phy_Addr,
    Settings.ETH_Pin_power,
    Settings.ETH_Pin_mdc,
    Settings.ETH_Pin_mdio,
    (eth_phy_type_t)Settings.ETH_Phy_Type,
    (eth_clock_mode_t)Settings.ETH_Clock_Mode);
  if (EthEventData.ethInitSuccess) {
    EthEventData.ethConnectAttemptNeeded = false;
  }
  return EthEventData.ethInitSuccess;
}

bool ETHConnected() {
  if (EthEventData.EthServicesInitialized()) {
    if (EthLinkUp()) {
      stop_eth_dhcps();
      return true;
    }
    // Apparently we missed an event
    EthEventData.processedDisconnect = false;
  } else if (EthEventData.ethInitSuccess) {
    if (EthLinkUp()) {
      stop_eth_dhcps();
      EthEventData.setEthConnected();
      if (NetworkLocalIP() != IPAddress(0, 0, 0, 0) && 
          !EthEventData.EthGotIP()) {
        EthEventData.processedGotIP = false;
      }
      if (EthEventData.lastConnectMoment.isSet()) {
        if (!EthEventData.EthServicesInitialized()) {
          if (EthEventData.lastConnectMoment.millisPassedSince() > 10000 &&
              EthEventData.lastGetIPmoment.isSet()) {
            EthEventData.processedGotIP = false;
            EthEventData.markLostIP();
          }
        }
      }
      return false;
    } else {
      setNetworkMedium(NetworkMedium_t::WIFI);
    }
  }
  return false;
}

#endif