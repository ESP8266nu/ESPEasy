#include "../DataStructs/ProvisioningStruct.h"

#ifdef USE_CUSTOM_PROVISIONING

# include "../Helpers/StringConverter.h"
# include "../Helpers/Hardware.h"

ProvisioningStruct::ProvisioningStruct() {
  ZERO_FILL(user);
  ZERO_FILL(pass);
  ZERO_FILL(url);
}

void ProvisioningStruct::validate() {
  ZERO_TERMINATE(user);
  ZERO_TERMINATE(pass);
  ZERO_TERMINATE(url);
}

bool ProvisioningStruct::matchingFlashSize() const
{
  return modelMatchingFlashSize(ResetFactoryDefaultPreference.getDeviceModel());
}

bool ProvisioningStruct::setUser(const String& username)
{
  return safe_strncpy(user, username, sizeof(user));
}

bool ProvisioningStruct::setPass(const String& password)
{
  return safe_strncpy(pass, password, sizeof(pass));
}

bool ProvisioningStruct::setUrl(const String& url_str)
{
  return safe_strncpy(url, url_str, sizeof(url));
}

#endif // ifdef USE_CUSTOM_PROVISIONING
