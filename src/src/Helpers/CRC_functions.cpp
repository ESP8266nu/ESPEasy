#include "CRC_functions.h"


int calc_CRC16(const String& text) {
  return calc_CRC16(text.c_str(), text.length());
}

int calc_CRC16(const char *ptr, int count)
{
  int crc = 0;
  if (ptr != nullptr) {
    while (--count >= 0)
    {
      crc = crc ^ (int)*ptr++ << 8;
      char i = 8;

      do
      {
        if (crc & 0x8000) {
          crc = crc << 1 ^ 0x1021;
        }
        else {
          crc = crc << 1;
        }
      } while(--i);
    }
  }
  return crc;
}

uint32_t calc_CRC32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xffffffff;

  if (data != nullptr) {
    while (length--) {
      uint8_t c = *data++;

      for (uint32_t i = 0x80; i > 0; i >>= 1) {
        bool bit = crc & 0x80000000;

        if (c & i) {
          bit = !bit;
        }
        crc <<= 1;

        if (bit) {
          crc ^= 0x04c11db7;
        }
      }
    }
  }
  return crc;
}
