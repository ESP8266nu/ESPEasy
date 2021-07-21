#ifndef DATASTRUCTS_ESPEASYCONTROLLERCACHE_H
#define DATASTRUCTS_ESPEASYCONTROLLERCACHE_H

#include <Arduino.h>

#include "../DataStructs/RTC_cache_handler_struct.h"

struct ControllerCache_struct {
  ControllerCache_struct() = default;

  ~ControllerCache_struct();

  // Write a single sample set to the buffer
  bool write(uint8_t     *data,
             unsigned int size);

  // Read a single sample set, either from file or buffer.
  // May delete a file if it is all read and not written to.
  bool read(uint8_t     *data,
            unsigned int size);

  // Dump whatever is in the buffer to the filesystem
  bool   flush();

  void   init();

  bool   isInitialized();

  // Clear all caches
  void   clearCache();

  bool   deleteOldestCacheBlock();

  void   resetpeek();

  // Read data without marking it as being read.
  bool   peek(uint8_t     *data,
              unsigned int size);

  String getPeekCacheFileName(bool& islast);

  int readFileNr = 0;
  int readPos    = 0;

private:

  RTC_cache_handler_struct *_RTC_cache_handler = nullptr;
};


#endif // ifndef DATASTRUCTS_ESPEASYCONTROLLERCACHE_H
