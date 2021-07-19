/*

   ESPeasySoftwareSerial.cpp - Implementation of the Arduino software serial for ESP8266.
   Copyright (c) 2015-2016 Peter Lerup. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 */
#ifdef ESP8266 // Needed for precompile issues.
#include <Arduino.h>

// The Arduino standard GPIO routines are not enough,
// must use some from the Espressif SDK as well
extern "C" {
#include "gpio.h"
}


  #ifndef CORE_POST_3_0_0
    #define IRAM_ATTR ICACHE_RAM_ATTR
  #endif

#include <ESPEasySoftwareSerial.h>

#define MAX_PIN 16
#define USABLE_PINS 10
#define NR_CONCURRENT_SOFT_SERIALS 3

// As the Arduino attachInterrupt has no parameter, lists of objects
// and callbacks corresponding to each possible GPIO pins have to be defined
static ESPeasySoftwareSerial *ObjList[NR_CONCURRENT_SOFT_SERIALS];
static uint8_t PinControllerMap[NR_CONCURRENT_SOFT_SERIALS]={}; // Zero all elements

void IRAM_ATTR espeasy_sws_isr_0() { ObjList[0]->rxRead(); };
void IRAM_ATTR espeasy_sws_isr_1() { ObjList[1]->rxRead(); };
void IRAM_ATTR espeasy_sws_isr_2() { ObjList[2]->rxRead(); };
/*void IRAM_ATTR espeasy_sws_isr_3() { ObjList[3]->rxRead(); };
void IRAM_ATTR espeasy_sws_isr_4() { ObjList[4]->rxRead(); };
void IRAM_ATTR espeasy_sws_isr_5() { ObjList[5]->rxRead(); };
   // Pin 6 to 11 can not be used
   void IRAM_ATTR espeasy_sws_isr_12() { ObjList[6]->rxRead(); };
   void IRAM_ATTR espeasy_sws_isr_13() { ObjList[7]->rxRead(); };
   void IRAM_ATTR espeasy_sws_isr_14() { ObjList[8]->rxRead(); };
   void IRAM_ATTR espeasy_sws_isr_15() { ObjList[9]->rxRead(); };
 */

static void (*ISRList[NR_CONCURRENT_SOFT_SERIALS])() = {
  espeasy_sws_isr_0,
  espeasy_sws_isr_1,
  espeasy_sws_isr_2 /*,
                       espeasy_sws_isr_3,
                       espeasy_sws_isr_4,
                       espeasy_sws_isr_5,
                       // Pin 6 to 11 can not be used
                       espeasy_sws_isr_12,
                       espeasy_sws_isr_13,
                       espeasy_sws_isr_14,
                       espeasy_sws_isr_15
                     */
};

ESPeasySoftwareSerial::ESPeasySoftwareSerial(uint8_t receivePin, uint8_t transmitPin, bool inverse_logic, uint16_t buffSize) {
  m_rxValid   = m_txValid = m_txEnableValid = false;
  m_buffer    = NULL;
  m_invert    = inverse_logic;
  m_rxEnabled = false;

  if (isValidGPIOpin(receivePin)) {
    m_rxPin    = receivePin;
    m_buffSize = buffSize;
    m_buffer   = (uint8_t *)malloc(m_buffSize);

    if (m_buffer != NULL) {
      m_rxValid = true;
      m_inPos   = m_outPos = 0;
      pinMode(m_rxPin, INPUT);
      const uint8_t index = pinToIndex(m_rxPin);

      if (index == NR_CONCURRENT_SOFT_SERIALS) {
        return; // Not possible to add software Serial.
      }
      ObjList[index] = this;
      enableRx(true);
    }
  }

  if (isValidGPIOpin(transmitPin)) {
    m_txValid = true;
    m_txPin   = transmitPin;
    pinMode(m_txPin, OUTPUT);
    digitalWrite(m_txPin, !m_invert);
  }

  // Default speed
  begin(9600);
}

ESPeasySoftwareSerial::~ESPeasySoftwareSerial() {
  enableRx(false);

  if (m_rxValid) {
    const uint8_t index = pinToIndex(m_rxPin);

    if (index < NR_CONCURRENT_SOFT_SERIALS) {
      PinControllerMap[index] = 0;
      ObjList[index]          = NULL;
    }
  }

  if (m_buffer) {
    free(m_buffer);
  }
}

bool ESPeasySoftwareSerial::isValidGPIOpin(uint8_t pin) {
  if ((pin >= 0) && (pin <= 5)) {
    return true;
  }

  if ((pin >= 12) && (pin <= MAX_PIN)) {
    return true;
  }
  return false;
}

uint8_t ESPeasySoftwareSerial::pinToIndex(uint8_t pin) {
  // Pin will be stored in the map, only "1" will be added,
  // to allow simple initialize to 0 and still use GPIO-0.
  const uint8_t stored_pin = pin + 1;

  for (unsigned i = 0; i < NR_CONCURRENT_SOFT_SERIALS; ++i) {
    if (PinControllerMap[i] == stored_pin) { return i; }
  }

  // Not found, add as first free option.
  for (unsigned i = 0; i < NR_CONCURRENT_SOFT_SERIALS; ++i) {
    if (PinControllerMap[i] == 0) {
      PinControllerMap[i] = stored_pin;
      return i;
    }
  }

  // No more controllers available.
  return NR_CONCURRENT_SOFT_SERIALS;
}

void ESPeasySoftwareSerial::begin(long speed) {
  // Use getCycleCount() loop to get as exact timing as possible
  m_bitTime = ESP.getCpuFreqMHz() * 1000000 / speed;

  if (!m_rxEnabled) {
    enableRx(true);
  }
}

void ESPeasySoftwareSerial::setTransmitEnablePin(uint8_t transmitEnablePin) {
  if (isValidGPIOpin(transmitEnablePin)) {
    m_txEnableValid = true;
    m_txEnablePin   = transmitEnablePin;
    pinMode(m_txEnablePin, OUTPUT);
    digitalWrite(m_txEnablePin, LOW);
  } else {
    m_txEnableValid = false;
  }
}

void ESPeasySoftwareSerial::enableRx(bool on) {
  if (m_rxValid) {
    if (on) {
      attachInterrupt(m_rxPin, ISRList[pinToIndex(m_rxPin)], m_invert ? RISING : FALLING);
    } else {
      detachInterrupt(m_rxPin);
    }
    m_rxEnabled = on;
  }
}

int ESPeasySoftwareSerial::read() {
  if (!m_rxValid || (m_inPos == m_outPos)) { return -1; }
  uint8_t ch = m_buffer[m_outPos];
  m_outPos = (m_outPos + 1) % m_buffSize;
  return ch;
}

int ESPeasySoftwareSerial::available() {
  if (!m_rxValid) { return 0; }
  int avail = m_inPos - m_outPos;

  if (avail < 0) { avail += m_buffSize; }
  return avail;
}

#define WAIT { while (ESP.getCycleCount() - start < wait); wait += m_bitTime; }

size_t ESPeasySoftwareSerial::write(uint8_t b) {
  if (!m_txValid) { return 0; }

  if (m_invert) { b = ~b; }

  // Disable interrupts in order to get a clean transmit
  cli();

  if (m_txEnableValid) { digitalWrite(m_txEnablePin, HIGH); }
  unsigned long wait = m_bitTime;
  digitalWrite(m_txPin, HIGH);
  unsigned long start = ESP.getCycleCount();

  // Start bit;
  digitalWrite(m_txPin, LOW);
  WAIT;

  for (int i = 0; i < 8; i++) {
    digitalWrite(m_txPin, (b & 1) ? HIGH : LOW);
    WAIT;
    b >>= 1;
  }

  // Stop bit
  digitalWrite(m_txPin, HIGH);
  WAIT;

  if (m_txEnableValid) { digitalWrite(m_txEnablePin, LOW); }
  sei();
  return 1;
}

void ESPeasySoftwareSerial::flush() {
  m_inPos = m_outPos = 0;
}

int ESPeasySoftwareSerial::peek() {
  if (!m_rxValid || (m_inPos == m_outPos)) { return -1; }
  return m_buffer[m_outPos];
}

void IRAM_ATTR ESPeasySoftwareSerial::rxRead() {
  // Advance the starting point for the samples but compensate for the
  // initial delay which occurs before the interrupt is delivered
  unsigned long wait  = m_bitTime + m_bitTime / 3 - 500;
  unsigned long start = ESP.getCycleCount();
  uint8_t rec         = 0;

  for (uint8_t i = 0; i < 8; i++) {
    WAIT;
    rec >>= 1;

    if (digitalRead(m_rxPin)) {
      rec |= 0x80;
    }
  }

  if (m_invert) { rec = ~rec; }

  // Stop bit
  WAIT;

  // Store the received value in the buffer unless we have an overflow
  uint16_t next = (m_inPos + 1) % m_buffSize;

  if (next != m_inPos) {
    m_buffer[m_inPos] = rec;
    m_inPos           = next;
  }

  // Must clear this bit in the interrupt register,
  // it gets set even when interrupts are disabled
  GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << m_rxPin);
}

#endif // ifdef ESP8266
