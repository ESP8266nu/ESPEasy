#ifndef HELPERS_CRC_FUNCTIONS_H
#define HELPERS_CRC_FUNCTIONS_H

#include <Arduino.h>

int      calc_CRC16(const String& text);

int ICACHE_FLASH_ATTR calc_CRC16(const char *ptr,
                                 int         count);

uint32_t calc_CRC32(const uint8_t *data,
                    size_t         length);


#endif // ifndef HELPERS_CRC_FUNCTIONS_H
