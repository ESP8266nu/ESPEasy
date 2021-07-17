#ifndef DATASTRUCTS_PROVISIONINGSTRUCT_H
#define DATASTRUCTS_PROVISIONINGSTRUCT_H

#include "../../ESPEasy_common.h"

#ifdef USE_CUSTOM_PROVISIONING
# include "../CustomBuild/ESPEasyLimits.h"
# include "../DataStructs/FactoryDefaultPref.h"

/*********************************************************************************************\
* ProvisioningStruct
\*********************************************************************************************/
struct ProvisioningStruct
{
  ProvisioningStruct();

  void validate();

  bool matchingFlashSize() const;

  bool setUser(const String& username);
  bool setPass(const String& password);
  bool setUrl(const String& url_str);

  // its safe to extend this struct, up to 4096 bytes, default values in config are 0.
  uint8_t md5[16]        = { 0 };
  uint8_t ProgmemMd5[16] = { 0 }; // crc of the binary that last saved the struct to file.

  ResetFactoryDefaultPreference_struct ResetFactoryDefaultPreference;

  // Credentials + URL for fetching configuration from HTTP server
  char user[26] = { 0 };
  char pass[64] = { 0 };
  char url[128] = { 0 };
};

typedef std::shared_ptr<ProvisioningStruct> ProvisioningStruct_ptr_type;
# define MakeProvisioningSettings(T) ProvisioningStruct_ptr_type ProvisioningStruct_ptr(new (std::nothrow)  ProvisioningStruct()); \
  ProvisioningStruct& T = *ProvisioningStruct_ptr;

// Check to see if MakeProvisioningSettings was successful
# define AllocatedProvisioningSettings() (ProvisioningStruct_ptr.get() != nullptr)


#endif // ifdef USE_CUSTOM_PROVISIONING

#endif // ifndef DATASTRUCTS_PROVISIONINGSTRUCT_H
