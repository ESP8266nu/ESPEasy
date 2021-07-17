#ifndef WEBSERVER_WEBSERVER_SETTINGSARCHIVE_H
#define WEBSERVER_WEBSERVER_SETTINGSARCHIVE_H

#include "../WebServer/common.h"

#ifdef USE_SETTINGS_ARCHIVE

#include "../DataTypes/ESPEasyFileType.h"

// ********************************************************************************
// Web Interface to manage archived settings
// ********************************************************************************
void handle_settingsarchive();

// ********************************************************************************
// download filetype selectors
// ********************************************************************************
void addDownloadFiletypeCheckbox(FileType::Enum filetype, unsigned int filenr = 0);

void storeDownloadFiletypeCheckbox(FileType::Enum filetype, unsigned int filenr = 0);


bool tryDownloadFileType(const String& url, const String& user, const String& pass, FileType::Enum filetype, unsigned int filenr = 0);

#endif // ifdef USE_SETTINGS_ARCHIVE

#endif