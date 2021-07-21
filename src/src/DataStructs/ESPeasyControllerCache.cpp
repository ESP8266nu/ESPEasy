#include "ESPEasyControllerCache.h"


ControllerCache_struct::~ControllerCache_struct() {
  if (_RTC_cache_handler != nullptr) {
    delete _RTC_cache_handler;
    _RTC_cache_handler = nullptr;
  }
}

// Write a single sample set to the buffer
bool ControllerCache_struct::write(uint8_t *data, unsigned int size) {
  if (_RTC_cache_handler == nullptr) {
    return false;
  }
  return _RTC_cache_handler->write(data, size);
}

// Read a single sample set, either from file or buffer.
// May delete a file if it is all read and not written to.
bool ControllerCache_struct::read(uint8_t *data, unsigned int size) {
  return true;
}

// Dump whatever is in the buffer to the filesystem
bool ControllerCache_struct::flush() {
  if (_RTC_cache_handler == nullptr) {
    return false;
  }
  return _RTC_cache_handler->flush();
}

void ControllerCache_struct::init() {
  if (_RTC_cache_handler == nullptr) {
    _RTC_cache_handler = new (std::nothrow) RTC_cache_handler_struct;
  }
}

bool ControllerCache_struct::isInitialized() {
  return _RTC_cache_handler != nullptr;
}

// Clear all caches
void ControllerCache_struct::clearCache() {}

bool ControllerCache_struct::deleteOldestCacheBlock() {
  if (_RTC_cache_handler != nullptr) {
    return _RTC_cache_handler->deleteOldestCacheBlock();
  }
  return false;
}

void ControllerCache_struct::resetpeek() {
  if (_RTC_cache_handler != nullptr) {
    _RTC_cache_handler->resetpeek();
  }
}

// Read data without marking it as being read.
bool ControllerCache_struct::peek(uint8_t *data, unsigned int size) {
  if (_RTC_cache_handler == nullptr) {
    return false;
  }
  return _RTC_cache_handler->peek(data, size);
}

String ControllerCache_struct::getPeekCacheFileName(bool& islast) {
  if (_RTC_cache_handler == nullptr) {
    return "";
  }
  return _RTC_cache_handler->getPeekCacheFileName(islast);
}
