#include "../DataTypes/ESPEasyFileType.h"

#include "../Globals/ResetFactoryDefaultPref.h"

bool matchFileType(const String& filename, FileType::Enum filetype)
{
  if (filename.startsWith(F("/"))) {
    return matchFileType(filename.substring(1), filetype);
  }
  return filename.equalsIgnoreCase(getFileName(filetype));
}

bool isProtectedFileType(const String& filename)
{
  return matchFileType(filename, FileType::CONFIG_DAT) ||
         matchFileType(filename, FileType::SECURITY_DAT) ||
         matchFileType(filename, FileType::NOTIFICATION_DAT) ||
         matchFileType(filename, FileType::PROVISIONING_DAT);
}

String getFileName(FileType::Enum filetype) {
  String result;

  switch (filetype)
  {
    case FileType::CONFIG_DAT:
      result += F("config.dat");
      break;
    case FileType::NOTIFICATION_DAT:
      result += F("notification.dat");
      break;
    case FileType::SECURITY_DAT:
      result += F("security.dat");
      break;
    case FileType::PROVISIONING_DAT:
      result += F("provisioning.dat");
    case FileType::RULES_TXT:
      // Use getRulesFileName
      break;

    case FileType::MAX_FILETYPE:
      break;
  }
  return result;
}

String getFileName(FileType::Enum filetype, unsigned int filenr) {
  if (filetype == FileType::RULES_TXT) {
    return getRulesFileName(filenr);
  }
  return getFileName(filetype);
}

// filenr = 0...3 for files rules1.txt ... rules4.txt
String getRulesFileName(unsigned int filenr) {
  String result;

  if (filenr < 4) {
    result += F("rules");
    result += filenr + 1;
    result += F(".txt");
  }
  return result;
}

bool getDownloadFiletypeChecked(FileType::Enum filetype, unsigned int filenr) {
  bool isChecked = false;

  switch (filetype) {
    case FileType::CONFIG_DAT: isChecked       = ResetFactoryDefaultPreference.fetchConfigDat(); break;
    case FileType::SECURITY_DAT: isChecked     = ResetFactoryDefaultPreference.fetchSecurityDat(); break;
    case FileType::NOTIFICATION_DAT: isChecked = ResetFactoryDefaultPreference.fetchNotificationDat(); break;
    case FileType::RULES_TXT: isChecked        = ResetFactoryDefaultPreference.fetchRulesTXT(filenr); break;
    case FileType::PROVISIONING_DAT: isChecked = ResetFactoryDefaultPreference.fetchProvisioningDat(); break;

    case FileType::MAX_FILETYPE:
      break;
  }
  return isChecked;
}
