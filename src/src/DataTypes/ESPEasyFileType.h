#ifndef DATATYPES_ESPEASYFILETYPE_H
#define DATATYPES_ESPEASYFILETYPE_H

#include <Arduino.h>

struct FileType {
  enum Enum : short {
    CONFIG_DAT,
    SECURITY_DAT,
    RULES_TXT,
    NOTIFICATION_DAT,
    PROVISIONING_DAT,


    MAX_FILETYPE
  };
};

bool matchFileType(const String& filename, FileType::Enum filetype);

bool isProtectedFileType(const String& filename);

String getFileName(FileType::Enum filetype);
String getFileName(FileType::Enum filetype,
                   unsigned int   filenr);


// filenr = 0...3 for files rules1.txt ... rules4.txt
String getRulesFileName(unsigned int filenr);

bool   getDownloadFiletypeChecked(FileType::Enum filetype,
                                  unsigned int   filenr);


#endif // ifndef DATATYPES_ESPEASYFILETYPE_H
