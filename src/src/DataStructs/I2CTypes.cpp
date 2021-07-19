#include "../DataStructs/I2CTypes.h"

const __FlashStringHelper* toString(I2C_bus_state state) {
  switch (state) {
    case I2C_bus_state::BusCleared:            return F("Bus Cleared");
    case I2C_bus_state::OK:                    return F("OK");
    case I2C_bus_state::NotConfigured:         return F("Not Configured");
    case I2C_bus_state::ClearingProcessActive: return F("Clearing Process Active");
    case I2C_bus_state::SCL_Low:               return F("SCL Low");
    case I2C_bus_state::SDA_Low_over_2_sec:    return F("SCL Low by I2C device clock stretch > 2 sec");
    case I2C_bus_state::SDA_Low_20_clocks:     return F("SDA Low");
  }
  return F("");
}
