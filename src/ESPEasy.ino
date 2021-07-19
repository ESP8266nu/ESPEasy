
#include <Arduino.h>

#ifdef CONTINUOUS_INTEGRATION
# pragma GCC diagnostic error "-Wall"
#else // ifdef CONTINUOUS_INTEGRATION
# pragma GCC diagnostic warning "-Wall"
#endif // ifdef CONTINUOUS_INTEGRATION

// Include this as first, to make sure all defines are active during the entire compile.
// See: https://www.letscontrolit.com/forum/viewtopic.php?f=4&t=7980
// If Custom.h build from Arduino IDE is needed, uncomment #define USE_CUSTOM_H in ESPEasy_common.h
#include "ESPEasy_common.h"

#ifdef USE_CUSTOM_H

// make the compiler show a warning to confirm that this file is inlcuded
  # warning "**** Using Settings from Custom.h File ***"
#endif // ifdef USE_CUSTOM_H


// Needed due to preprocessor issues.
#ifdef PLUGIN_SET_GENERIC_ESP32
  # ifndef ESP32
    #  define ESP32
  # endif // ifndef ESP32
#endif // ifdef PLUGIN_SET_GENERIC_ESP32


/****************************************************************************************************************************\
 * Arduino project "ESP Easy" © Copyright www.letscontrolit.com
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You received a copy of the GNU General Public License along with this program in file 'License.txt'.
 *
 * IDE download    : https://www.arduino.cc/en/Main/Software
 * ESP8266 Package : https://github.com/esp8266/Arduino
 *
 * Source Code     : https://github.com/ESP8266nu/ESPEasy
 * Support         : http://www.letscontrolit.com
 * Discussion      : http://www.letscontrolit.com/forum/
 *
 * Additional information about licensing can be found at : http://www.gnu.org/licenses
 \*************************************************************************************************************************/

// This file incorporates work covered by the following copyright and permission notice:

/****************************************************************************************************************************\
 * Arduino project "Nodo" © Copyright 2010..2015 Paul Tonkes
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You received a copy of the GNU General Public License along with this program in file 'License.txt'.
 *
 * Voor toelichting op de licentievoorwaarden zie    : http://www.gnu.org/licenses
 * Uitgebreide documentatie is te vinden op          : http://www.nodo-domotica.nl
 * Compiler voor deze programmacode te downloaden op : http://arduino.cc
 \*************************************************************************************************************************/

//   Simple Arduino sketch for ESP module, supporting:
//   =================================================================================
//   Simple switch inputs and direct GPIO output control to drive relays, mosfets, etc
//   Analog input (ESP-7/12 only)
//   Pulse counters
//   Dallas OneWire DS18b20 temperature sensors
//   DHT11/22/12 humidity sensors
//   BMP085 I2C Barometric Pressure sensor
//   PCF8591 4 port Analog to Digital converter (I2C)
//   RFID Wiegand-26 reader
//   MCP23017 I2C IO Expanders
//   BH1750 I2C Luminosity sensor
//   Arduino Pro Mini with IO extender sketch, connected through I2C
//   LCD I2C display 4x20 chars
//   HC-SR04 Ultrasonic distance sensor
//   SI7021 I2C temperature/humidity sensors
//   TSL2561 I2C Luminosity sensor
//   TSOP4838 IR receiver
//   PN532 RFID reader
//   Sharp GP2Y10 dust sensor
//   PCF8574 I2C IO Expanders
//   PCA9685 I2C 16 channel PWM driver
//   OLED I2C display with SSD1306 driver
//   MLX90614 I2C IR temperature sensor
//   ADS1115 I2C ADC
//   INA219 I2C voltage/current sensor
//   BME280 I2C temp/hum/baro sensor
//   MSP5611 I2C temp/baro sensor
//   BMP280 I2C Barometric Pressure sensor
//   SHT1X temperature/humidity sensors
//   Ser2Net server
//   DL-Bus (Technische Alternative)

// Define globals before plugin sets to allow a personal override of the selected plugins
#include "ESPEasy-Globals.h"

// Must be included after all the defines, since it is using TASKS_MAX
#include "_Plugin_Helper.h"

// Plugin helper needs the defined controller sets, thus include after 'define_plugin_sets.h'
#include "src/Helpers/_CPlugin_Helper.h"


#include "src/ESPEasyCore/ESPEasy_setup.h"
#include "src/ESPEasyCore/ESPEasy_loop.h"


#ifdef PHASE_LOCKED_WAVEFORM
# include <core_esp8266_waveform.h>
#endif // ifdef PHASE_LOCKED_WAVEFORM

#if FEATURE_ADC_VCC
ADC_MODE(ADC_VCC);
#endif // if FEATURE_ADC_VCC



#ifdef CORE_POST_2_5_0

/*********************************************************************************************\
* Pre-init
\*********************************************************************************************/
void preinit();
void preinit() {
  system_phy_set_powerup_option(3);
  // Global WiFi constructors are not called yet
  // (global class instances like WiFi, Serial... are not yet initialized)..
  // No global object methods or C++ exceptions can be called in here!
  // The below is a static class method, which is similar to a function, so it's ok.
  #ifndef CORE_POST_3_0_0
  //ESP8266WiFiClass::preinitWiFiOff();
  #endif

  // Prevent RF calibration on power up.
  // TD-er: disabled on 2021-06-07 as it may cause several issues with some boards.
  // It cannot be made a setting as we can't read anything of our own settings.
  //system_phy_set_powerup_option(RF_NO_CAL);
}

#endif // ifdef CORE_POST_2_5_0


void setup() {
  ESPEasy_setup();
}

void loop() {
  ESPEasy_loop();
}
