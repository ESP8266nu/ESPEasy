#include "../DataStructs/FactoryDefaultPref.h"

#include "../../ESPEasy_common.h"

ResetFactoryDefaultPreference_struct::ResetFactoryDefaultPreference_struct(uint32_t preference) : _preference(preference) {}

DeviceModel ResetFactoryDefaultPreference_struct::getDeviceModel() const {
  return static_cast<DeviceModel>(_preference & 0xFF);
}

void ResetFactoryDefaultPreference_struct::setDeviceModel(DeviceModel model) {
  _preference &= ~(0xFF); // set DeviceModel bits to 0
  _preference |= static_cast<uint32_t>(model);
}

bool ResetFactoryDefaultPreference_struct::keepWiFi() const {
  return bitRead(_preference, 9);
}

void ResetFactoryDefaultPreference_struct::keepWiFi(bool keep) {
  bitWrite(_preference, 9, keep);
}

bool ResetFactoryDefaultPreference_struct::keepNTP() const {
  return bitRead(_preference, 10);
}

void ResetFactoryDefaultPreference_struct::keepNTP(bool keep) {
  bitWrite(_preference, 10, keep);
}

bool ResetFactoryDefaultPreference_struct::keepNetwork() const {
  return bitRead(_preference, 11);
}

void ResetFactoryDefaultPreference_struct::keepNetwork(bool keep) {
  bitWrite(_preference, 11, keep);
}

bool ResetFactoryDefaultPreference_struct::keepLogSettings() const {
  return bitRead(_preference, 12);
}

void ResetFactoryDefaultPreference_struct::keepLogSettings(bool keep) {
  bitWrite(_preference, 12, keep);
}

bool ResetFactoryDefaultPreference_struct::keepUnitName() const {
  return bitRead(_preference, 13);
}

void ResetFactoryDefaultPreference_struct::keepUnitName(bool keep) {
  bitWrite(_preference, 13, keep);
}

// filenr = 0...3 for files rules1.txt ... rules4.txt
bool ResetFactoryDefaultPreference_struct::fetchRulesTXT(int filenr) const {
  return bitRead(_preference, 14 + filenr);
}

void ResetFactoryDefaultPreference_struct::fetchRulesTXT(int filenr, bool fetch) {
  bitWrite(_preference, 14 + filenr, fetch);
}

bool ResetFactoryDefaultPreference_struct::fetchNotificationDat() const {
  return bitRead(_preference, 18);
}

void ResetFactoryDefaultPreference_struct::fetchNotificationDat(bool fetch) {
  bitWrite(_preference, 18, fetch);
}

bool ResetFactoryDefaultPreference_struct::fetchSecurityDat() const {
  return bitRead(_preference, 19);
}

void ResetFactoryDefaultPreference_struct::fetchSecurityDat(bool fetch) {
  bitWrite(_preference, 19, fetch);
}

bool ResetFactoryDefaultPreference_struct::fetchConfigDat() const {
  return bitRead(_preference, 20);
}

void ResetFactoryDefaultPreference_struct::fetchConfigDat(bool fetch) {
  bitWrite(_preference, 20, fetch);
}

bool ResetFactoryDefaultPreference_struct::deleteFirst() const {
  return bitRead(_preference, 21);
}

void ResetFactoryDefaultPreference_struct::deleteFirst(bool checked) {
  bitWrite(_preference, 21, checked);
}

bool ResetFactoryDefaultPreference_struct::saveURL() const {
  return bitRead(_preference, 22);
}

void ResetFactoryDefaultPreference_struct::saveURL(bool checked) {
  bitWrite(_preference, 22, checked);
}

bool ResetFactoryDefaultPreference_struct::allowFetchByCommand() const {
  return bitRead(_preference, 23);
}

void ResetFactoryDefaultPreference_struct::allowFetchByCommand(bool checked) {
  bitWrite(_preference, 23, checked);
}

bool ResetFactoryDefaultPreference_struct::storeCredentials() const {
  return bitRead(_preference, 24);
}

void ResetFactoryDefaultPreference_struct::storeCredentials(bool checked) {
  bitWrite(_preference, 24, checked);
}

bool ResetFactoryDefaultPreference_struct::fetchProvisioningDat() const {
  return bitRead(_preference, 25);
}

void ResetFactoryDefaultPreference_struct::fetchProvisioningDat(bool checked) {
  bitWrite(_preference, 25, checked);
}


uint32_t ResetFactoryDefaultPreference_struct::getPreference() {
  return _preference;
}
