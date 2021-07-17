#ifndef HELPERS_ESPEASY_STORAGE_H
#define HELPERS_ESPEASY_STORAGE_H

#include <FS.h>

#include "../DataStructs/ProvisioningStruct.h"
#include "../DataTypes/ESPEasyFileType.h"
#include "../DataTypes/SettingsType.h"
#include "../Globals/Plugins.h"
#include "../Globals/CPlugins.h"

#include "../../ESPEasy_common.h"

/********************************************************************************************\
   file system error handling
   Look here for error # reference: https://github.com/pellepl/spiffs/blob/master/src/spiffs.h
 \*********************************************************************************************/
String FileError(int line, const char *fname);

/********************************************************************************************\
   Keep track of number of flash writes.
 \*********************************************************************************************/
void flashCount();

String flashGuard();

String appendLineToFile(const String& fname, const String& line);

String appendToFile(const String& fname, const uint8_t *data, unsigned int size);

bool fileExists(const String& fname);

fs::File tryOpenFile(const String& fname, const String& mode);

bool tryRenameFile(const String& fname_old, const String& fname_new);

bool tryDeleteFile(const String& fname);

/********************************************************************************************\
   Fix stuff to clear out differences between releases
 \*********************************************************************************************/
String BuildFixes();

/********************************************************************************************\
   Mount FS and check config.dat
 \*********************************************************************************************/
void fileSystemCheck();

/********************************************************************************************\
   Garbage collection
 \*********************************************************************************************/
bool GarbageCollection();

/********************************************************************************************\
   Save settings to file system
 \*********************************************************************************************/
String SaveSettings();

String SaveSecuritySettings();

void afterloadSettings();

/********************************************************************************************\
   Load settings from file system
 \*********************************************************************************************/
String LoadSettings();

/********************************************************************************************\
   Disable Plugin, based on bootFailedCount
 \*********************************************************************************************/
uint8_t disablePlugin(uint8_t bootFailedCount);

/********************************************************************************************\
   Disable Controller, based on bootFailedCount
 \*********************************************************************************************/
uint8_t disableController(uint8_t bootFailedCount);

/********************************************************************************************\
   Disable Notification, based on bootFailedCount
 \*********************************************************************************************/
uint8_t disableNotification(uint8_t bootFailedCount);

bool getAndLogSettingsParameters(bool read, SettingsType::Enum settingsType, int index, int& offset, int& max_size);

/********************************************************************************************\
   Load array of Strings from Custom settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String LoadStringArray(SettingsType::Enum settingsType, int index, String strings[], uint16_t nrStrings, uint16_t maxStringLength);


/********************************************************************************************\
   Save array of Strings from Custom settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String SaveStringArray(SettingsType::Enum settingsType, int index, const String strings[], uint16_t nrStrings, uint16_t maxStringLength);


/********************************************************************************************\
   Save Task settings to file system
 \*********************************************************************************************/
String SaveTaskSettings(taskIndex_t TaskIndex);

/********************************************************************************************\
   Load Task settings from file system
 \*********************************************************************************************/
String LoadTaskSettings(taskIndex_t TaskIndex);

/********************************************************************************************\
   Save Custom Task settings to file system
 \*********************************************************************************************/
String SaveCustomTaskSettings(taskIndex_t TaskIndex, uint8_t *memAddress, int datasize);

/********************************************************************************************\
   Save array of Strings to Custom Task settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String SaveCustomTaskSettings(taskIndex_t TaskIndex, String strings[], uint16_t nrStrings, uint16_t maxStringLength);

String getCustomTaskSettingsError(uint8_t varNr);

/********************************************************************************************\
   Clear custom task settings
 \*********************************************************************************************/
String ClearCustomTaskSettings(taskIndex_t TaskIndex);

/********************************************************************************************\
   Load Custom Task settings from file system
 \*********************************************************************************************/
String LoadCustomTaskSettings(taskIndex_t TaskIndex, uint8_t *memAddress, int datasize);

/********************************************************************************************\
   Load array of Strings from Custom Task settings
   Use maxStringLength = 0 to optimize for size (strings will be concatenated)
 \*********************************************************************************************/
String LoadCustomTaskSettings(taskIndex_t TaskIndex, String strings[], uint16_t nrStrings, uint16_t maxStringLength);

/********************************************************************************************\
   Save Controller settings to file system
 \*********************************************************************************************/
String SaveControllerSettings(controllerIndex_t ControllerIndex, ControllerSettingsStruct& controller_settings);

/********************************************************************************************\
   Load Controller settings to file system
 \*********************************************************************************************/
String LoadControllerSettings(controllerIndex_t ControllerIndex, ControllerSettingsStruct& controller_settings);

/********************************************************************************************\
   Clear Custom Controller settings
 \*********************************************************************************************/
String ClearCustomControllerSettings(controllerIndex_t ControllerIndex);

/********************************************************************************************\
   Save Custom Controller settings to file system
 \*********************************************************************************************/
String SaveCustomControllerSettings(controllerIndex_t ControllerIndex, uint8_t *memAddress, int datasize);

/********************************************************************************************\
   Load Custom Controller settings to file system
 \*********************************************************************************************/
String LoadCustomControllerSettings(controllerIndex_t ControllerIndex, uint8_t *memAddress, int datasize);


#ifdef USE_CUSTOM_PROVISIONING
/********************************************************************************************\
   Save Provisioning Settings
 \*********************************************************************************************/
String saveProvisioningSettings(ProvisioningStruct& ProvisioningSettings);

/********************************************************************************************\
   Load Provisioning Settings
 \*********************************************************************************************/
String loadProvisioningSettings(ProvisioningStruct& ProvisioningSettings);
#endif




/********************************************************************************************\
   Save Controller settings to file system
 \*********************************************************************************************/
String SaveNotificationSettings(int NotificationIndex, uint8_t *memAddress, int datasize);


/********************************************************************************************\
   Load Controller settings to file system
 \*********************************************************************************************/
String LoadNotificationSettings(int NotificationIndex, uint8_t *memAddress, int datasize);


/********************************************************************************************\
   Init a file with zeros on file system
 \*********************************************************************************************/
String InitFile(const String& fname, int datasize);

String InitFile(SettingsType::Enum settingsType);
String InitFile(SettingsType::SettingsFileEnum file_type);

/********************************************************************************************\
   Save data into config file on file system
 \*********************************************************************************************/
// Save to file in r+ mode
// Open for reading and writing.  
// The stream is positioned at the beginning of the file.
String SaveToFile(const char *fname, int index, const uint8_t *memAddress, int datasize);

// Save to file in w+ mode
// Open for reading and writing.  
// The file is created if it does not exist, otherwise it is truncated.
// The stream is positioned at the beginning of the file.
String SaveToFile_trunc(const char *fname, int index, const byte *memAddress, int datasize);

// See for mode description: https://github.com/esp8266/Arduino/blob/master/doc/filesystem.rst
String doSaveToFile(const char *fname, int index, const uint8_t *memAddress, int datasize, const char *mode);


/********************************************************************************************\
   Clear a certain area in a file (set to 0)
 \*********************************************************************************************/
String ClearInFile(const char *fname, int index, int datasize);

/********************************************************************************************\
   Load data from config file on file system
 \*********************************************************************************************/
String LoadFromFile(const char *fname, int offset, uint8_t *memAddress, int datasize);

/********************************************************************************************\
   Wrapper functions to handle errors in accessing settings
 \*********************************************************************************************/
String getSettingsFileIndexRangeError(bool read, SettingsType::Enum settingsType, int index);

String getSettingsFileDatasizeError(bool read, SettingsType::Enum settingsType, int index, int datasize, int max_size);

String LoadFromFile(SettingsType::Enum settingsType, int index, uint8_t *memAddress, int datasize, int offset_in_block);

String LoadFromFile(SettingsType::Enum settingsType, int index, uint8_t *memAddress, int datasize);

String SaveToFile(SettingsType::Enum settingsType, int index, uint8_t *memAddress, int datasize);

String SaveToFile(SettingsType::Enum settingsType, int index, uint8_t *memAddress, int datasize, int posInBlock);

String ClearInFile(SettingsType::Enum settingsType, int index);

/********************************************************************************************\
   Check file system area settings
 \*********************************************************************************************/
int SpiffsSectors();

size_t SpiffsUsedBytes();

size_t SpiffsTotalBytes();

size_t SpiffsBlocksize();

size_t SpiffsPagesize();

size_t SpiffsFreeSpace();

bool SpiffsFull();

/********************************************************************************************\
   Handling cached data
 \*********************************************************************************************/
String createCacheFilename(unsigned int count);

// Match string with an integer between '_' and ".bin"
int getCacheFileCountFromFilename(const String& fname);

// Look into the filesystem to see if there are any cache files present on the filesystem
// Return true if any found.
bool getCacheFileCounters(uint16_t& lowest, uint16_t& highest, size_t& filesizeHighest);

/********************************************************************************************\
   Get partition table information
 \*********************************************************************************************/
#ifdef ESP32

String getPartitionType(uint8_t pType, uint8_t pSubType);

String getPartitionTableHeader(const String& itemSep, const String& lineEnd);

String getPartitionTable(uint8_t pType, const String& itemSep, const String& lineEnd);

#endif // ifdef ESP32


/********************************************************************************************\
   Download ESPEasy file types from HTTP server
 \*********************************************************************************************/
#ifdef USE_DOWNLOAD
String downloadFileType(const String& url, const String& user, const String& pass, FileType::Enum filetype, unsigned int filenr = 0);

#endif
#ifdef USE_CUSTOM_PROVISIONING
// Download file type based on settings stored in provisioning.dat file.
String downloadFileType(FileType::Enum filetype, unsigned int filenr = 0);

#endif




#endif // HELPERS_ESPEASY_STORAGE_H