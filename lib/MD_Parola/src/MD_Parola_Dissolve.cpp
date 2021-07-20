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
 * \brief Implements dissolve effect
 */

void MD_PZone::effectDissolve(bool bIn)
// Dissolve the current message in/out
{
  switch (_fsmState)
  {
  case INITIALISE:  // bIn = true
  case PAUSE:   // bIn = false
  case GET_FIRST_CHAR:  // first stage dissolve
    PRINT_STATE("IO DISS");
    for (int16_t i = ZONE_START_COL(_zoneStart); i <= ZONE_END_COL(_zoneEnd); i++)
    {
      uint8_t col = DATA_BAR(_MX->getColumn(i));

      col |= (i&1 ? 0x55 : 0xaa); // checkerboard pattern
      _MX->setColumn(i, DATA_BAR(col));
    }
    _fsmState = GET_NEXT_CHAR;
    break;

  case GET_NEXT_CHAR:   // second stage dissolve
    PRINT_STATE("IO DISS");
    zoneClear();
    if (bIn) commonPrint();
    for (int16_t i = ZONE_START_COL(_zoneStart); i <= ZONE_END_COL(_zoneEnd); i++)
    {
      uint8_t col = DATA_BAR(_MX->getColumn(i));

      col |= (i&1 ? 0xaa : 0x55); // alternate checkerboard pattern
      _MX->setColumn(i, DATA_BAR(col));
    }
    _fsmState = PUT_CHAR;
    break;

  case PUT_CHAR:
    PRINT_STATE("IO DISS");
    zoneClear();
    if (bIn) commonPrint();
    _fsmState = (bIn ? PAUSE : END);
    break;

  default:
    PRINT_STATE("IO DISS");
    _fsmState = (bIn ? PAUSE : END);
  }
}
