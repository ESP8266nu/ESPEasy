#ifndef DATASTRUCTS_FACTORY_DEFAULT_PREF_H
#define DATASTRUCTS_FACTORY_DEFAULT_PREF_H

#include "../DataTypes/DeviceModel.h"

#include <stdint.h>

struct ResetFactoryDefaultPreference_struct {
  ResetFactoryDefaultPreference_struct(uint32_t preference = 0);

  DeviceModel getDeviceModel() const;

  void setDeviceModel(DeviceModel model);

  bool keepWiFi() const;
  void keepWiFi(bool keep);

  bool keepNTP() const;
  void keepNTP(bool keep);

  bool keepNetwork() const;
  void keepNetwork(bool keep);

  bool keepLogSettings() const;
  void keepLogSettings(bool keep);

  bool keepUnitName() const;
  void keepUnitName(bool keep);

  // filenr = 0...3 for files rules1.txt ... rules4.txt
  bool fetchRulesTXT(int filenr) const;
  void fetchRulesTXT(int filenr, bool fetch);

  bool fetchNotificationDat() const;
  void fetchNotificationDat(bool fetch);

  bool fetchSecurityDat() const;
  void fetchSecurityDat(bool fetch);

  bool fetchConfigDat() const;
  void fetchConfigDat(bool fetch);

  bool fetchProvisioningDat() const;
  void fetchProvisioningDat(bool fetch);  

  bool deleteFirst() const;
  void deleteFirst(bool checked);

  bool saveURL() const;
  void saveURL(bool checked);

  bool allowFetchByCommand() const;
  void allowFetchByCommand(bool checked);

  bool storeCredentials() const;
  void storeCredentials(bool checked);
  
  uint32_t getPreference();

  // TODO TD-er: Add extra flags for settings to keep/set when reset to default.

private:
  uint32_t _preference = 0;
};


#endif // DATASTRUCTS_FACTORY_DEFAULT_PREF_H
