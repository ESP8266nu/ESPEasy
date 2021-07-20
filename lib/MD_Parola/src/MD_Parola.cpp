/*
MD_Parola - Library for modular scrolling text and Effects

See header file for comments

Copyright (C) 2013 Marco Colli. All rights reserved.

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

#include <MD_Parola.h>
#include <MD_Parola_lib.h>
//#include <MD_MAX72xx.h>
/**
 * \file
 * \brief Implements core MD_Parola class methods
 */

MD_Parola::~MD_Parola(void)
{
#if !STATIC_ZONES
  // release the dynamically allocated zone array
  delete[] _Z;
#endif
}

void MD_Parola::begin(uint8_t numZones)
{
  _D.begin();

  // Check boundaries for the number of zones
  if (numZones == 0) numZones = 1;
#if STATIC_ZONES
  if (numZones > MAX_ZONES) numZones = MAX_ZONES;  // static zones
#endif
  _numZones = numZones;

#if !STATIC_ZONES
  // Create the zone objects array for dynamic zones
  _Z = new MD_PZone[_numZones];
#endif

  for (uint8_t i = 0; i < _numZones; i++) {
    _Z[i].begin(&_D);
    delay(0); // Feed the watchdog
  }

  // for one zone automatically make it all modules, user will override if not intended
  if (_numZones == 1)
    setZone(0, 0, _numModules - 1);

  // initialize zone-independent options
  setSpeedInOut(10, 10);
  setPause(10 * getSpeed());
  setCharSpacing(1);
  setScrollSpacing(0);
  setTextAlignment(PA_LEFT);
  setTextEffect(PA_PRINT, PA_NO_EFFECT);
  setInvert(false);
}

bool MD_Parola::setZone(uint8_t z, uint8_t moduleStart, uint8_t moduleEnd)
{
  if ((moduleStart <= moduleEnd) && (moduleEnd < _numModules) && (z < _numZones))
  {
    _Z[z].setZone(moduleStart, moduleEnd);
    return(true);
  }

  return(false);
}

void MD_Parola::displayZoneText(uint8_t z, const char *pText, textPosition_t align, uint16_t speed, uint16_t pause, textEffect_t effectIn, textEffect_t effectOut)
{
  setTextBuffer(z, pText);
  setTextAlignment(z, align);
  setSpeed(z, speed);
  setPause(z, pause);
  setTextEffect(z, effectIn, effectOut);

  displayReset(z);
  delay(0); // Feed the watchdog
}

bool MD_Parola::displayAnimate(void)
{
  bool b = false;

  // suspend the display while we animate a frame
  _D.update(0);

  for (uint8_t i = 0; i < _numZones; i++) {
    b |= _Z[i].zoneAnimate();
    delay(0); // Feed the watchdog
  }

  // re-enable and update the display
  _D.update(1);
  delay(0); // Feed the watchdog

  return(b);
}


size_t MD_Parola::write(const char *str)
// .print() extension of a string
{
  displayText((char *)str, getTextAlignment(), 0, 0, PA_PRINT, PA_NO_EFFECT);
  while (displayAnimate())
    /* do nothing */;

  return(strlen(str));
}

size_t MD_Parola::write(const uint8_t *buffer, size_t size)
// .print() extension
{
  char *psz = (char *)malloc(sizeof(char) * (size + 1));

  if (psz == nullptr) return(0);

  memcpy(psz, buffer, size);
  psz[size] = '\0';
  write(psz);
  free(psz);

  return(size);
}
