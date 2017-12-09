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

#include <Arduino.h>

// The Arduino standard GPIO routines are not enough,
// must use some from the Espressif SDK as well
extern "C" {
#include "gpio.h"
}

#include <ESPeasySoftwareSerial.h>

#define MAX_PIN 15
#define USABLE_PINS 10

// As the Arduino attachInterrupt has no parameter, lists of objects
// and callbacks corresponding to each possible GPIO pins have to be defined
static ESPeasySoftwareSerial *ObjList[USABLE_PINS];

void ICACHE_RAM_ATTR sws_isr_0() { ObjList[0]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_1() { ObjList[1]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_2() { ObjList[2]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_3() { ObjList[3]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_4() { ObjList[4]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_5() { ObjList[5]->rxRead(); };
// Pin 6 to 11 can not be used
void ICACHE_RAM_ATTR sws_isr_12() { ObjList[6]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_13() { ObjList[7]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_14() { ObjList[8]->rxRead(); };
void ICACHE_RAM_ATTR sws_isr_15() { ObjList[9]->rxRead(); };

static void (*ISRList[USABLE_PINS])() = {
      sws_isr_0,
      sws_isr_1,
      sws_isr_2,
      sws_isr_3,
      sws_isr_4,
      sws_isr_5,
      // Pin 6 to 11 can not be used
      sws_isr_12,
      sws_isr_13,
      sws_isr_14,
      sws_isr_15
};

ESPeasySoftwareSerial::ESPeasySoftwareSerial(int receivePin, int transmitPin, bool inverse_logic, unsigned int buffSize) {
   m_rxValid = m_txValid = m_txEnableValid = false;
   m_buffer = NULL;
   m_invert = inverse_logic;
   m_overflow = false;
   m_rxEnabled = false;
   if (isValidGPIOpin(receivePin)) {
      m_rxPin = receivePin;
      m_buffSize = buffSize;
      m_buffer = (uint8_t*)malloc(m_buffSize);
      if (m_buffer != NULL) {
         m_rxValid = true;
         m_inPos = m_outPos = 0;
         pinMode(m_rxPin, INPUT);
         ObjList[pinToIndex(m_rxPin)] = this;
         enableRx(true);
      }
   }
   if (isValidGPIOpin(transmitPin) || transmitPin == 16) {
      m_txValid = true;
      m_txPin = transmitPin;
      pinMode(m_txPin, OUTPUT);
      digitalWrite(m_txPin, !m_invert);
   }
   // Default speed
   begin(9600);
}

ESPeasySoftwareSerial::~ESPeasySoftwareSerial() {
   enableRx(false);
   if (m_rxValid)
      ObjList[pinToIndex(m_rxPin)] = NULL;
   if (m_buffer)
      free(m_buffer);
}

bool ESPeasySoftwareSerial::isValidGPIOpin(int pin) {
   return pinToIndex(pin) >= 0;
}

int ESPeasySoftwareSerial::pinToIndex(int pin) {
  if (pin >= 0 && pin <= 5) {
    return pin;
  }
  if (pin >= 12 && pin <= MAX_PIN) {
    return (pin - 6);
  }
  return -1;
}

void ESPeasySoftwareSerial::begin(long speed) {
   // Use getCycleCount() loop to get as exact timing as possible
   m_bitTime = ESP.getCpuFreqMHz()*1000000/speed;
   m_highSpeed = speed >= 9600;

   if (!m_rxEnabled)
     enableRx(true);
}

long ESPeasySoftwareSerial::baudRate() {
   return ESP.getCpuFreqMHz()*1000000/m_bitTime;
}

void ESPeasySoftwareSerial::setTransmitEnablePin(int transmitEnablePin) {
  if (isValidGPIOpin(transmitEnablePin)) {
     m_txEnableValid = true;
     m_txEnablePin = transmitEnablePin;
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
   if (!m_rxValid || (m_inPos == m_outPos)) return -1;
   uint8_t ch = m_buffer[m_outPos];
   m_outPos = (m_outPos+1) % m_buffSize;
   return ch;
}

int ESPeasySoftwareSerial::available() {
   if (!m_rxValid) return 0;
   int avail = m_inPos - m_outPos;
   if (avail < 0) avail += m_buffSize;
   return avail;
}

#define WAIT { while (ESP.getCycleCount()-start < wait) if (!m_highSpeed) optimistic_yield(1); wait += m_bitTime; }

size_t ESPeasySoftwareSerial::write(uint8_t b) {
   if (!m_txValid) return 0;

   if (m_invert) b = ~b;
   if (m_highSpeed)
     // Disable interrupts in order to get a clean transmit
     cli();
   if (m_txEnableValid) digitalWrite(m_txEnablePin, HIGH);
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
   if (m_txEnableValid) digitalWrite(m_txEnablePin, LOW);
   if (m_highSpeed)
    sei();
   return 1;
}

void ESPeasySoftwareSerial::flush() {
   m_inPos = m_outPos = 0;
}

bool ESPeasySoftwareSerial::overflow() {
   bool res = m_overflow;
   m_overflow = false;
   return res;
}

int ESPeasySoftwareSerial::peek() {
   if (!m_rxValid || (m_inPos == m_outPos)) return -1;
   return m_buffer[m_outPos];
}

void ICACHE_RAM_ATTR ESPeasySoftwareSerial::rxRead() {
   // Advance the starting point for the samples but compensate for the
   // initial delay which occurs before the interrupt is delivered
   unsigned long wait = m_bitTime + m_bitTime/3 - 500;
   unsigned long start = ESP.getCycleCount();
   uint8_t rec = 0;
   for (int i = 0; i < 8; i++) {
     WAIT;
     rec >>= 1;
     if (digitalRead(m_rxPin))
       rec |= 0x80;
   }
   if (m_invert) rec = ~rec;
   // Stop bit
   WAIT;
   // Store the received value in the buffer unless we have an overflow
   int next = (m_inPos+1) % m_buffSize;
   if (next != m_outPos) {
      m_buffer[m_inPos] = rec;
      m_inPos = next;
   } else {
      m_overflow = true;
   }
   // Must clear this bit in the interrupt register,
   // it gets set even when interrupts are disabled
   GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, 1 << m_rxPin);
}
