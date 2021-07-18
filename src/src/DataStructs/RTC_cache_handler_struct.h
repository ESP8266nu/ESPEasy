#ifndef DATASTRUCTS_RTC_CACHE_HANDLER_STRUCT_H
#define DATASTRUCTS_RTC_CACHE_HANDLER_STRUCT_H


#include "RTCCacheStruct.h"

#include "../../ESPEasy_common.h"

#include <FS.h>
#include <vector>


// Locations where to store the cached data
// As a file on the filesystem
#define CACHE_STORAGE_SPIFFS        0

// Between the sketch and FS, including OTA area (will overwrite this area when performing OTA)
#define CACHE_STORAGE_OTA_FREE      1

// Only use the free space between sketch and FS, thus avoid OTA area
#define CACHE_STORAGE_NO_OTA_FREE   2

// Use space after FS. (e.g. on 16M flash partitioned as 4M, or 4M flash partitioned as 2M)
#define CACHE_STORAGE_BEHIND_SPIFFS 3


//#define RTC_STRUCT_DEBUG

/********************************************************************************************\
   RTC located cache
 \*********************************************************************************************/
struct RTC_cache_handler_struct
{
  RTC_cache_handler_struct();

  unsigned int getFreeSpace();

  void         resetpeek();

  bool         peek(uint8_t     *data,
                    unsigned int size);

  // Write a single sample set to the buffer
  bool write(uint8_t     *data,
             unsigned int size);

  // Mark all content as being processed and empty buffer.
  bool flush();

  // Return usable filename for reading.
  // Will be empty if there is no file to process.
  String getReadCacheFileName(int& readPos);

  String getPeekCacheFileName(bool& islast);

  bool   deleteOldestCacheBlock();

private:

  bool     loadMetaData();

  bool     loadData();

  bool     saveRTCcache();

  bool     saveRTCcache(unsigned int startOffset,
                        size_t       nrBytes);

  uint32_t getDataChecksum();

  void     initRTCcache_data();

  void     clearRTCcacheData();

  // Return true if any cache file found
  bool     updateRTC_filenameCounters();

  bool     prepareFileForWrite();

#ifdef RTC_STRUCT_DEBUG
  void     rtc_debug_log(const String& description,
                         size_t        nrBytes);
#endif // ifdef RTC_STRUCT_DEBUG

#ifdef ESP8266
  RTC_cache_struct    RTC_cache;
  std::vector<uint8_t>RTC_cache_data;
#endif
  File                fw;
  File                fr;
  File                fp;
  size_t              peekfilenr  = 0;
  size_t              peekreadpos = 0;

  uint8_t storageLocation = CACHE_STORAGE_SPIFFS;
  bool writeerror      = false;
};

#endif // ifndef DATASTRUCTS_RTC_CACHE_HANDLER_STRUCT_H
