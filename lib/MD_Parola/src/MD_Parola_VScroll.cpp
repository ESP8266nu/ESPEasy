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
 * \brief Implements vertical scroll effect
 */

void MD_PZone::effectVScroll(bool bUp, bool bIn)
// Scroll the display horizontally up or down, depending on the selected effect
{
  if (bIn)  // incoming
  {
    switch (_fsmState)
    {
    case INITIALISE:
      PRINT_STATE("I VSCROLL");
      _nextPos = COL_SIZE-1;
      _MX->control(_zoneStart, _zoneEnd, MD_MAX72XX::WRAPAROUND, 0);
      _fsmState = PUT_CHAR;
      // fall through to next state

    case GET_FIRST_CHAR:
    case GET_NEXT_CHAR:
    case PUT_CHAR:
    case PAUSE:
      PRINT_STATE("I VSCROLL");

      zoneClear();
      commonPrint();

      // scroll each column of the zone so that the message appears to be animated
      // Note: Directions are reversed here because we start with the message in the
      // middle position thru commonPrint() and to see it animated moving DOWN we
      // need to scroll it UP, and vice versa.
      for (int16_t j = ZONE_START_COL(_zoneStart); j <= ZONE_END_COL(_zoneEnd); j++)   // for each column
      {
        uint8_t c = _MX->getColumn(j);

        for (int8_t i = _nextPos; i > 0; i--)
        {
          c = (bUp ? c << 1 : c >> 1);
          if (_inverted) c |= (bUp ? 0x01 : 0x80);
        }

        _MX->setColumn(j, c);
      }

      // check if we have finished
      if (_nextPos == 0) _fsmState = PAUSE;

      _nextPos--;
      break;

    default:
      PRINT_STATE("I VSCROLL");
      _fsmState = PAUSE;
    }
  }
  else  // exiting
  {
    switch (_fsmState)
    {
    case PAUSE:
    case INITIALISE:
      PRINT_STATE("O VSCROLL");
      _nextPos = 0;
      _fsmState = PUT_CHAR;
      // fall through to next state

    case GET_FIRST_CHAR:
    case GET_NEXT_CHAR:
    case PUT_CHAR:
      PRINT_STATE("O VSCROLL");

      for (int16_t j = ZONE_START_COL(_zoneStart); j <= ZONE_END_COL(_zoneEnd); j++)   // for each column
      {
        uint8_t c = _MX->getColumn(j);

        c = (bUp ? c >> 1 : c << 1);
        if (_inverted) c |= (bUp ? 0x80 : 0x01);

        _MX->setColumn(j, c);
      }

      // check if we have finished
      if (_nextPos == COL_SIZE - 1) _fsmState = END;

      _nextPos++;
      break;

    default:
      PRINT_STATE("O VSCROLL");
      _fsmState = END;
      break;
    }
  }
}
