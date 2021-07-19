#ifndef I2C_TYPES_H
#define I2C_TYPES_H I2C_TYPES_H

#include <Arduino.h>
#include <vector>

// **************************************************************************/
// Object to store data to and from I2C devices
// **************************************************************************/
template<class T>
struct I2Cdata {
  I2Cdata(uint8_t length) : start_reg(0) {
    data.resize(length);
  }

  I2Cdata(uint8_t length, uint8_t start_register) : start_reg(start_register) {
    data.resize(length);
  }

  T operator[](uint8_t n) const {
    if (n < start_reg) { return 0; }

    if (n >= (data.size() + start_reg)) { return 0; }
    return data[(n - start_reg)];
  }

  bool addAtIndex(uint8_t index, T value) {
    if (index >= data.size()) { return false; }
    data[index] = value;
    return true;
  }

  bool addRegister(uint8_t reg, T value) {
    if (reg < start_reg) { return false; }
    return addAtIndex(reg - start_reg, value);
  }

  uint8_t getSize() const {
    return data.size();
  }

  T* get() {
    return &(data[0]);
  }

  uint8_t getRegister() const {
    return start_reg;
  }

private:

  uint8_t       start_reg;
  std::vector<T>data;
};

typedef I2Cdata<uint8_t> I2Cdata_bytes;
typedef I2Cdata<uint16_t>I2Cdata_words;

enum class I2C_bus_state {
    NotConfigured,
    OK,
    BusCleared,
    ClearingProcessActive,
    SCL_Low,             // I2C bus error. Could not clear. SCL clock line held low
    SDA_Low_over_2_sec,  // I2C bus error. Could not clear. SCL clock line held low by slave clock stretch for >2sec
    SDA_Low_20_clocks   // I2C bus error. Could not clear. SDA data line held low
};

const __FlashStringHelper * toString(I2C_bus_state state);



#endif // I2C_TYPES_H
