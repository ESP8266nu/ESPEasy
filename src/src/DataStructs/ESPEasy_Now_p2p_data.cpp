#include "ESPEasy_Now_p2p_data.h"

#include <cstddef>


ESPEasy_Now_p2p_data::~ESPEasy_Now_p2p_data() {
  if (data != nullptr) {
    delete data;
    data = nullptr;
  }
}

bool ESPEasy_Now_p2p_data::validate() const {
  if (data != nullptr) {
    if (dataSize == 0) { return false; }
  }

  if (dataOffset == 0) { return false; }

  if (!validTaskIndex(sourceTaskIndex)) { return false; }

  if (!validPluginID(plugin_id)) { return false; }

  // TODO TD-er: Must add more sanity checks here.
  return true;
}

bool ESPEasy_Now_p2p_data::addFloat(float value) {
  return addBinaryData((uint8_t *)(&value), sizeof(float));
}

bool ESPEasy_Now_p2p_data::getFloat(float& value, size_t& offset) const {
  if ((offset + sizeof(float)) >= dataSize) {
    return false;
  }
  memcpy((uint8_t *)(&value), &data[offset], sizeof(float));
  offset += sizeof(float);
  return true;
}

bool ESPEasy_Now_p2p_data::addString(const String& value) {
  return addBinaryData((uint8_t *)(value.c_str()), value.length() + 1); // Include null termination
}

bool ESPEasy_Now_p2p_data::getString(String& value, size_t& offset) const {
  int maxStrLen = dataSize - offset;

  if (maxStrLen < 2) {
    return false;
  }
  const size_t str_len = strnlen(reinterpret_cast<const char*>(&data[offset]), maxStrLen);

  value.reserve(str_len);

  for (size_t i = 0; i < str_len; ++i) {
    value += data[offset];
    ++offset;
  }
  return true;
}

size_t ESPEasy_Now_p2p_data::getTotalSize() const {
  return dataOffset + dataSize;
}

bool ESPEasy_Now_p2p_data::addBinaryData(uint8_t *binaryData, size_t size) {
  size_t oldSize;

  if (allocate(size, oldSize)) {
    memcpy(&data[oldSize], binaryData, size);
    return true;
  }
  return false;
}

const uint8_t * ESPEasy_Now_p2p_data::getBinaryData(size_t offset, size_t& size) const {
  if (offset >= dataSize) {
    size = 0;
    return nullptr;
  }

  const size_t available = dataSize - offset;

  if (size > available) {
    size = available;
  }
  return &data[offset];
}

uint8_t * ESPEasy_Now_p2p_data::prepareBinaryData(size_t& size) {
  size_t oldSize;
  dataSize = 0;
  data     = nullptr;

  if (!allocate(size, oldSize)) {
    size = 0;
    return nullptr;
  }
  size = dataSize;
  return data;
}

bool ESPEasy_Now_p2p_data::allocate(size_t size, size_t& oldSize) {
  oldSize = dataSize;
  const size_t newSize = oldSize + size;

  uint8_t *tmp_ptr = new (std::nothrow) uint8_t[newSize];

  if (tmp_ptr == nullptr) {
    return false;
  }
  memset(tmp_ptr, 0, newSize);

  if (data != nullptr) {
    memcpy(tmp_ptr, data, oldSize);
    delete[] data;
  }
  data     = tmp_ptr;
  dataSize = newSize;
  return true;
}
