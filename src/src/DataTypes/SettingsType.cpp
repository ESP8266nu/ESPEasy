#include "SettingsType.h"

#include "../CustomBuild/StorageLayout.h"
#include "../DataStructs/NotificationSettingsStruct.h"
#include "../DataStructs/SecurityStruct.h"
#include "../DataStructs/ControllerSettingsStruct.h"
#include "../DataTypes/ESPEasyFileType.h"

#include "../Globals/ExtraTaskSettings.h"
#include "../Globals/Settings.h"


const __FlashStringHelper * SettingsType::getSettingsTypeString(Enum settingsType) {
  switch (settingsType) {
    case Enum::BasicSettings_Type:             return F("Settings");
    case Enum::TaskSettings_Type:              return F("TaskSettings");
    case Enum::CustomTaskSettings_Type:        return F("CustomTaskSettings");
    case Enum::ControllerSettings_Type:        return F("ControllerSettings");
    case Enum::CustomControllerSettings_Type:  return F("CustomControllerSettings");
    case Enum::NotificationSettings_Type:      return F("NotificationSettings");
    case Enum::SecuritySettings_Type:          return F("SecuritySettings");
    case Enum::ExtdControllerCredentials_Type: return F("ExtendedControllerCredentials");

    case Enum::SettingsType_MAX: break;
  }
  return F("");
}

/********************************************************************************************\
   Offsets in settings files
 \*********************************************************************************************/
bool SettingsType::getSettingsParameters(Enum settingsType, int index, int& max_index, int& offset, int& max_size, int& struct_size) {
  // The defined offsets should be used with () just in case they are the result of a formula in the defines.
  struct_size = 0;

  switch (settingsType) {
    case Enum::BasicSettings_Type:
    {
      max_index   = 1;
      offset      = 0;
      max_size    = (DAT_BASIC_SETTINGS_SIZE);
      struct_size = sizeof(SettingsStruct);
      break;
    }
    case Enum::TaskSettings_Type:
    {
      max_index   = TASKS_MAX;
      offset      = (DAT_OFFSET_TASKS) + (index * (DAT_TASKS_DISTANCE));
      max_size    = DAT_TASKS_SIZE;
      struct_size = sizeof(ExtraTaskSettingsStruct);
      break;
    }
    case Enum::CustomTaskSettings_Type:
    {
      getSettingsParameters(Enum::TaskSettings_Type, index, max_index, offset, max_size, struct_size);
      offset  += (DAT_TASKS_CUSTOM_OFFSET);
      max_size = DAT_TASKS_CUSTOM_SIZE;

      // struct_size may differ.
      struct_size = 0;
      break;
    }
    case Enum::ControllerSettings_Type:
    {
      max_index   = CONTROLLER_MAX;
      offset      = (DAT_OFFSET_CONTROLLER) + (index * (DAT_CONTROLLER_SIZE));
      max_size    = DAT_CONTROLLER_SIZE;
      struct_size = sizeof(ControllerSettingsStruct);
      break;
    }
    case Enum::CustomControllerSettings_Type:
    {
      max_index = CONTROLLER_MAX;
      offset    = (DAT_OFFSET_CUSTOM_CONTROLLER) + (index * (DAT_CUSTOM_CONTROLLER_SIZE));
      max_size  = DAT_CUSTOM_CONTROLLER_SIZE;

      // struct_size may differ.
      struct_size = 0;
      break;
    }
    case Enum::NotificationSettings_Type:
    {
      max_index   = NOTIFICATION_MAX;
      offset      = index * (DAT_NOTIFICATION_SIZE);
      max_size    = DAT_NOTIFICATION_SIZE;
      struct_size = sizeof(NotificationSettingsStruct);
      break;
    }
    case Enum::SecuritySettings_Type:
    {
      max_index   = 1;
      offset      = 0;
      max_size    = DAT_SECURITYSETTINGS_SIZE;
      struct_size = sizeof(SecurityStruct);
      break;
    }
    case Enum::ExtdControllerCredentials_Type:
    {
      max_index = 1;
      offset    = DAT_EXTDCONTR_CRED_OFFSET;
      max_size  = DAT_EXTDCONTR_CRED_SIZE;

      // struct_size may differ.
      struct_size = 0;
      break;
    }
    case Enum::SettingsType_MAX:
    {
      max_index = -1;
      offset    = -1;
      return false;
    }
  }
  return index >= 0 && index < max_index;
}

bool SettingsType::getSettingsParameters(Enum settingsType, int index, int& offset, int& max_size) {
  int max_index = -1;
  int struct_size;

  if (!getSettingsParameters(settingsType, index, max_index, offset, max_size, struct_size)) {
    return false;
  }

  if ((index >= 0) && (index < max_index)) { return true; }
  offset = -1;
  return false;
}

int SettingsType::getMaxFilePos(Enum settingsType) {
  int max_index, offset, max_size;
  int struct_size = 0;

  getSettingsParameters(settingsType, 0,             max_index, offset, max_size, struct_size);
  getSettingsParameters(settingsType, max_index - 1, offset,    max_size);
  return offset + max_size - 1;
}

int SettingsType::getFileSize(Enum settingsType) {
  SettingsType::SettingsFileEnum file_type = SettingsType::getSettingsFile(settingsType);
  int max_file_pos                         = 0;

  for (int st = 0; st < static_cast<int>(Enum::SettingsType_MAX); ++st) {
    if (SettingsType::getSettingsFile(static_cast<Enum>(st)) == file_type) {
      int filePos = SettingsType::getMaxFilePos(static_cast<Enum>(st));

      if (filePos > max_file_pos) {
        max_file_pos = filePos;
      }
    }
  }
  return max_file_pos;
}

#ifndef BUILD_MINIMAL_OTA
unsigned int SettingsType::getSVGcolor(Enum settingsType) {
  switch (settingsType) {
    case Enum::BasicSettings_Type:
      return 0x5F0A87;
    case Enum::TaskSettings_Type:
      return 0xEE6352;
    case Enum::CustomTaskSettings_Type:
      return 0x59CD90;
    case Enum::ControllerSettings_Type:
      return 0x3FA7D6;
    case Enum::CustomControllerSettings_Type:
      return 0xFAC05E;
    case Enum::NotificationSettings_Type:
      return 0xF79D84;

    case Enum::SecuritySettings_Type:
      return 0xff00a2;
    case Enum::ExtdControllerCredentials_Type:
      return 0xc300ff;
    case Enum::SettingsType_MAX:
      break;
  }
  return 0;
}

#endif // ifndef BUILD_MINIMAL_OTA

SettingsType::SettingsFileEnum SettingsType::getSettingsFile(Enum settingsType)
{
  switch (settingsType) {
    case Enum::BasicSettings_Type:
    case Enum::TaskSettings_Type:
    case Enum::CustomTaskSettings_Type:
    case Enum::ControllerSettings_Type:
    case Enum::CustomControllerSettings_Type:
      return SettingsFileEnum::FILE_CONFIG_type;
    case Enum::NotificationSettings_Type:
      return SettingsFileEnum::FILE_NOTIFICATION_type;
    case Enum::SecuritySettings_Type:
    case Enum::ExtdControllerCredentials_Type:
      return SettingsFileEnum::FILE_SECURITY_type;

    case Enum::SettingsType_MAX:
      break;
  }
  return SettingsFileEnum::FILE_UNKNOWN_type;
}

String SettingsType::getSettingsFileName(Enum settingsType) {
  return getSettingsFileName(getSettingsFile(settingsType));
}

String SettingsType::getSettingsFileName(SettingsType::SettingsFileEnum file_type) {
  switch (file_type) {
    case SettingsFileEnum::FILE_CONFIG_type:        return getFileName(FileType::CONFIG_DAT);
    case SettingsFileEnum::FILE_NOTIFICATION_type:  return getFileName(FileType::NOTIFICATION_DAT);
    case SettingsFileEnum::FILE_SECURITY_type:      return getFileName(FileType::SECURITY_DAT);
    case SettingsFileEnum::FILE_UNKNOWN_type:       break;
  }
  return "";
}

size_t SettingsType::getInitFileSize(SettingsType::SettingsFileEnum file_type) {
  switch (file_type) {
    case SettingsFileEnum::FILE_CONFIG_type:        return CONFIG_FILE_SIZE;
    case SettingsFileEnum::FILE_NOTIFICATION_type:
      // fall through
    case SettingsFileEnum::FILE_SECURITY_type:      return 4096;
    case SettingsFileEnum::FILE_UNKNOWN_type:       break;
  }
  return 0;
}