/*
MD_MAX72xx - Library for using a MAX7219/7221 LED matrix controller

See header file for comments

This file contains class and hardware related methods.

Copyright (C) 2012-14 Marco Colli. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */
#if defined(__MBED__) && !defined(ARDUINO)
#include "mbed.h"
#else
#include <Arduino.h>
#include <SPI.h>
#endif

#include "MD_MAX72xx.h"
#include "MD_MAX72xx_lib.h"

/**
 * \file
 * \brief Implements class definition and general methods
 */

MD_MAX72XX::MD_MAX72XX(moduleType_t mod, uint8_t dataPin, uint8_t clkPin, uint8_t csPin, uint8_t numDevices):
_dataPin(dataPin), _clkPin(clkPin), _csPin(csPin),
_hardwareSPI(false), _spiRef(SPI), _maxDevices(numDevices), _updateEnabled(true)
#if defined(__MBED__) && !defined(ARDUINO)
, _spi((PinName)dataPin, NC, (PinName)clkPin), _cs((PinName)csPin)
#endif
{
  setModuleParameters(mod);
}

MD_MAX72XX::MD_MAX72XX(moduleType_t mod, uint8_t csPin, uint8_t numDevices):
_dataPin(0), _clkPin(0), _csPin(csPin),
_hardwareSPI(true), _spiRef(SPI), _maxDevices(numDevices), _updateEnabled(true)
#if defined(__MBED__) && !defined(ARDUINO)
, _spi(SPI_MOSI, NC, SPI_SCK), _cs((PinName)csPin)
#endif
{
  setModuleParameters(mod);
}

MD_MAX72XX::MD_MAX72XX(moduleType_t mod, SPIClass& spi, uint8_t csPin, uint8_t numDevices):
  _dataPin(0), _clkPin(0), _csPin(csPin),
  _hardwareSPI(true), _spiRef(spi), _maxDevices(numDevices), _updateEnabled(true)
#if defined(__MBED__) && !defined(ARDUINO)
  , _spi(SPI_MOSI, NC, SPI_SCK), _cs((PinName)csPin)
#endif
{
  setModuleParameters(mod);
}

void MD_MAX72XX::setModuleParameters(moduleType_t mod)
// Combinations not listed as tested have *probably* not
// been tested and may not operate correctly.
{
  _mod = mod;
  switch (_mod)
  {
    case DR0CR0RR0_HW: _hwDigRows = false; _hwRevCols = false;  _hwRevRows = false; break;
    case DR0CR0RR1_HW: _hwDigRows = false; _hwRevCols = false;  _hwRevRows = true;  break;
    case DR0CR1RR0_HW: // same as GENERIC_HW, tested MC 9 March 2014
    case GENERIC_HW:   _hwDigRows = false; _hwRevCols = true;  _hwRevRows = false;  break;
    case DR0CR1RR1_HW: _hwDigRows = false; _hwRevCols = true;  _hwRevRows = true;   break;
    case DR1CR0RR0_HW: // same as FC16_HW, tested MC 23 Feb 2015
    case FC16_HW:      _hwDigRows = true;  _hwRevCols = false;  _hwRevRows = false; break;
    case DR1CR0RR1_HW: _hwDigRows = true;  _hwRevCols = false;  _hwRevRows = true;  break;
    case DR1CR1RR0_HW: // same as PAROLA_HW, tested MC 8 March 2014
    case PAROLA_HW:    _hwDigRows = true;  _hwRevCols = true;  _hwRevRows = false;  break;
    case DR1CR1RR1_HW: // same as ICSTATION_HW, tested MC 9 March 2014
    case ICSTATION_HW: _hwDigRows = true;  _hwRevCols = true;  _hwRevRows = true;   break;
  }
}

void MD_MAX72XX::begin(void)
{
  // initialize the SPI interface
#ifdef ARDUINO
  if (_hardwareSPI)
  {
    PRINTS("\nHardware SPI");
    _spiRef.begin();
  }
  else
  {
    PRINTS("\nBitBang SPI")
    pinMode(_dataPin, OUTPUT);
    pinMode(_clkPin, OUTPUT);
  }

  // initialize our preferred CS pin (could be same as SS)
  pinMode(_csPin, OUTPUT);
  digitalWrite(_csPin, HIGH);
#else
  _cs = 1;
#endif

  // object memory and internals
  setShiftDataInCallback(nullptr);
  setShiftDataOutCallback(nullptr);

  _matrix = (deviceInfo_t *)malloc(sizeof(deviceInfo_t) * _maxDevices);
  _spiData = (uint8_t *)malloc(SPI_DATA_SIZE);

#if USE_LOCAL_FONT
  setFont(_sysfont);
#endif // INCLUDE_LOCAL_FONT

  // Initialize the display devices. On initial power-up
  // - all control registers are reset,
  // - scan limit is set to one digit (row/col or LED),
  // - Decoding mode is off,
  // - intensity is set to the minimum,
  // - the display is blanked, and
  // - the MAX7219/MAX7221 is shut down.
  // The devices need to be set to our library defaults prior using the
  // display modules.
  control(TEST, MD_OFF);                   // no test
  control(SCANLIMIT, ROW_SIZE-1);       // scan limit is set to max on startup
  control(INTENSITY, MAX_INTENSITY/2);  // set intensity to a reasonable value
  control(DECODE, MD_OFF);                 // ensure no decoding (warm boot potential issue)
  clear();
  control(SHUTDOWN, MD_OFF);               // take the modules out of shutdown mode
}

MD_MAX72XX::~MD_MAX72XX(void)
{
#ifdef ARDUINO
  if (_hardwareSPI) _spiRef.end();  // reset SPI mode
#endif

  free(_matrix);
  free(_spiData);
}

void MD_MAX72XX::controlHardware(uint8_t dev, controlRequest_t mode, int value)
// control command is for the devices, translate internal request to device bytes
// into the transmission buffer
{
  uint8_t opcode = OP_NOOP;
  uint8_t param = 0;

  // work out data to write
  switch (mode)
  {
    case SHUTDOWN:
      opcode = OP_SHUTDOWN;
      param = (value == MD_OFF ? 1 : 0);
      break;

    case SCANLIMIT:
      opcode = OP_SCANLIMIT;
      param = (value > MAX_SCANLIMIT ? MAX_SCANLIMIT : value);
      break;

    case INTENSITY:
      opcode = OP_INTENSITY;
      param = (value > MAX_INTENSITY ? MAX_INTENSITY : value);
      break;

    case DECODE:
      opcode = OP_DECODEMODE;
      param = (value == MD_OFF ? 0 : 0xff);
      break;

    case TEST:
      opcode = OP_DISPLAYTEST;
      param = (value == MD_OFF ? 0 : 1);
      break;

    default:
      return;
  }

  // put our device data into the buffer
  _spiData[SPI_OFFSET(dev, 0)] = opcode;
  _spiData[SPI_OFFSET(dev, 1)] = param;
}

void MD_MAX72XX::controlLibrary(controlRequest_t mode, int value)
// control command was internal, set required parameters
{
  switch (mode)
  {
    case UPDATE:
      _updateEnabled = (value == MD_ON);
    if (_updateEnabled) flushBufferAll();
      break;

    case WRAPAROUND:
      _wrapAround = (value == MD_ON);
      break;

    default:
      break;
  }
}

bool MD_MAX72XX::control(uint8_t startDev, uint8_t endDev, controlRequest_t mode, int value)
{
  if (endDev < startDev) return(false);

  if (mode < UPDATE)  // device based control
  {
    spiClearBuffer();
    for (uint8_t i = startDev; i <= endDev; i++)
      controlHardware(i, mode, value);
    spiSend();
  }
  else                // internal control function, doesn't relate to specific device
  {
    controlLibrary(mode, value);
  }

  return(true);
}

bool MD_MAX72XX::control(uint8_t buf, controlRequest_t mode, int value)
// dev is zero based and needs adjustment if used
{
  if (buf > LAST_BUFFER) return(false);

  if (mode < UPDATE)  // device based control
  {
    spiClearBuffer();
    controlHardware(buf, mode, value);
    spiSend();
  }
  else                // internal control function, doesn't relate to specific device
  {
    controlLibrary(mode, value);
  }

  return(true);
}

void MD_MAX72XX::flushBufferAll()
// Only one data byte is sent to a device, so if there are many changes, it is more
// efficient to send a data byte all devices at the same time, substantially cutting
// the number of communication messages required.
{
  for (uint8_t i=0; i<ROW_SIZE; i++)  // all data rows
  {
    bool bChange = false; // set to true if we detected a change

    spiClearBuffer();

    for (uint8_t dev = FIRST_BUFFER; dev <= LAST_BUFFER; dev++)	// all devices
    {
      if (bitRead(_matrix[dev].changed, i))
      {
        // put our device data into the buffer
        _spiData[SPI_OFFSET(dev, 0)] = OP_DIGIT0+i;
        _spiData[SPI_OFFSET(dev, 1)] = _matrix[dev].dig[i];
        bChange = true;
      }
    }

  if (bChange) spiSend();
  }

  // mark everything as cleared
  for (uint8_t dev = FIRST_BUFFER; dev <= LAST_BUFFER; dev++)
    _matrix[dev].changed = ALL_CLEAR;
}

void MD_MAX72XX::flushBuffer(uint8_t buf)
// Use this function when the changes are limited to one device only.
// Address passed is a buffer address
{
  PRINT("\nflushBuf: ", buf);
  PRINTS(" r");

  if (buf > LAST_BUFFER)
    return;

  for (uint8_t i = 0; i < ROW_SIZE; i++)
  {
    if (bitRead(_matrix[buf].changed, i))
    {
      PRINT("", i);
      spiClearBuffer();

      // put our device data into the buffer
      _spiData[SPI_OFFSET(buf, 0)] = OP_DIGIT0+i;
      _spiData[SPI_OFFSET(buf, 1)] = _matrix[buf].dig[i];

      spiSend();
    }
  }
  _matrix[buf].changed = ALL_CLEAR;
}

inline void MD_MAX72XX::spiClearBuffer(void)
// Clear out the spi data array
{
  memset(_spiData, OP_NOOP, SPI_DATA_SIZE);
}

void MD_MAX72XX::spiSend(void)
{
#ifdef ARDUINO
  // initialize the SPI transaction
  if (_hardwareSPI)
    _spiRef.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  digitalWrite(_csPin, LOW);

  // shift out the data
  if (_hardwareSPI)
  {
    for (uint16_t i = 0; i < SPI_DATA_SIZE; i++)
      _spiRef.transfer(_spiData[i]);
  }
  else  // not hardware SPI - bit bash it out
  {
    for (uint16_t i = 0; i < SPI_DATA_SIZE; i++)
      shiftOut(_dataPin, _clkPin, MSBFIRST, _spiData[i]);
  }

  // end the SPI transaction
  digitalWrite(_csPin, HIGH);
  if (_hardwareSPI)
    _spiRef.endTransaction();
#else
  _cs = 0;
  _spi.write((const char*)_spiData, SPI_DATA_SIZE, nullptr, 0);
  _cs = 1;
#endif
}
