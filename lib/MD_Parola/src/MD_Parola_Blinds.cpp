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
/**
 * \file
 * \brief Implements blinds effect
 */

const uint8_t BLINDS_SIZE = 4; ///< The width of the blinds in pixels

void MD_PZone::effectBlinds(bool bIn)
// Transfer between messages with blinds effects
{
  switch (_fsmState)
  {
  case INITIALISE:  // bIn = true
  case PAUSE:       // bIn = false
    PRINT_STATE("IO BLIND");
    _nextPos = 0;
    _fsmState = GET_FIRST_CHAR;
    // fall through

  case GET_FIRST_CHAR:  // blinds closing
    PRINT_STATE("IO BLIND");

    _nextPos++;
    for (int16_t i = ZONE_START_COL(_zoneStart); i <= ZONE_END_COL(_zoneEnd); i++)
    {
      if (i % BLINDS_SIZE < _nextPos)
        _MX->setColumn(i, LIGHT_BAR);
    }

    if (_nextPos == BLINDS_SIZE)
    {
      _nextPos = BLINDS_SIZE;
      _fsmState = GET_NEXT_CHAR;
    }
    break;

  case GET_NEXT_CHAR:   // blinds opening
    PRINT_STATE("IO BLIND");
    zoneClear();
    if (bIn) commonPrint(); // only do this when putting the message up

    _nextPos--;
    for (int16_t i = ZONE_START_COL(_zoneStart); i <= ZONE_END_COL(_zoneEnd); i++)
    {
      if (i % BLINDS_SIZE < _nextPos)
        _MX->setColumn(i, LIGHT_BAR);
    }

    if (_nextPos == 0)
      _fsmState = PUT_CHAR;
    break;

  case PUT_CHAR:
    PRINT_STATE("IO BLIND");
    zoneClear();
    if (bIn) commonPrint();
    _fsmState = (bIn ? PAUSE : END);
    break;

  default:
    PRINT_STATE("IO BLIND");
    _fsmState = (bIn ? PAUSE : END);
  }
}
