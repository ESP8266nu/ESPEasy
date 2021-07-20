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
 * \brief Implements horizontal scrolling effect
 */

#define START_POSITION (bLeft) ? ZONE_START_COL(_zoneStart) : ZONE_END_COL(_zoneEnd) ///< Start position depends on the scrolling direction

void MD_PZone::effectHScroll(bool bLeft, bool bIn)
{
  if (bIn)
  {
    switch(_fsmState)
    {
    case INITIALISE:
    case GET_FIRST_CHAR: // Load the first character from the font table
      PRINT_STATE("I HSCROLL");

      if (!getFirstChar(_charCols))
      {
        _fsmState = END;
        break;
      }
      _countCols = 0;
      _fsmState = PUT_CHAR;
      break;

    case GET_NEXT_CHAR: // Load the next character from the font table
      PRINT_STATE("I HSCROLL");

      // Have we reached the end of the characters string?
      do
      {
        if (!getNextChar(_charCols))
          _fsmState = PAUSE;
      } while (_charCols == 0 && _fsmState != PAUSE);

      if (_fsmState == PAUSE) break;

      _countCols = 0;
      _fsmState = PUT_CHAR;
      FSMPRINTS(", fall through");
      // !! fall through to next state to start displaying

    case PUT_CHAR:  // display the next part of the character
      PRINT_STATE("I HSCROLL");

      if (_charCols != 0)
      {
        _MX->transform(_zoneStart, _zoneEnd, bLeft ? MD_MAX72XX::TSL : MD_MAX72XX::TSR);
        _MX->setColumn(START_POSITION, DATA_BAR(_cBuf[_countCols++]));
        FSMPRINTS(", scroll");
      }

      // end of this buffer - we may need to get another one
      if (_countCols == _charCols)
      {
        if (!_endOfText)
          _fsmState = GET_NEXT_CHAR;
        else
        {
          // work out the number of filler columns
          _countCols = (bLeft ? _limitLeft - _textLen - ZONE_START_COL(_zoneStart) + 1 : ZONE_END_COL(_zoneEnd) - _limitLeft);
          FSMPRINT(", filler count ", _countCols);
          _fsmState = (_countCols <= 0) ? PAUSE : PUT_FILLER;
        }
      }
    break;

    case PUT_FILLER:    // keep sending out blank columns until aligned
      PRINT_STATE("I HSCROLL");

      _MX->transform(_zoneStart, _zoneEnd, bLeft ? MD_MAX72XX::TSL : MD_MAX72XX::TSR);
      _MX->setColumn(START_POSITION, EMPTY_BAR);
      FSMPRINTS(", fill");

      if (--_countCols == 0)
        _fsmState = PAUSE;
    break;

    default:
      _fsmState = PAUSE;
      break;
    }
  }
  else  // exiting
  {
    switch(_fsmState)
    {
    case PAUSE:
      PRINT_STATE("O HSCROLL");
      _fsmState = PUT_FILLER;
      FSMPRINTS(" falling thru");
      // fall through

    case PUT_FILLER:
      PRINT_STATE("O HSCROLL");
      _MX->transform(_zoneStart, _zoneEnd, bLeft ? MD_MAX72XX::TSL : MD_MAX72XX::TSR);
      _MX->setColumn(START_POSITION, EMPTY_BAR);

      // check if enough scrolled off to say that new message should start
      // how we count depends on the direction for scrolling
      {
        uint16_t  spaceCount = 0;
        uint16_t  maxCount = (_zoneEnd - _zoneStart + 1) * COL_SIZE;

        if ((_scrollDistance != 0) && (maxCount > _scrollDistance)) maxCount = _scrollDistance;

        if (bLeft)
        {
          for (int16_t i = ZONE_START_COL(_zoneStart);
              (i <= ZONE_END_COL(_zoneEnd)) && (_MX->getColumn(i) == EMPTY_BAR);
               i++, spaceCount++);
        }
        else
        {
          for (int16_t i = ZONE_END_COL(_zoneEnd);
              (i >= ZONE_START_COL(_zoneStart)) && (_MX->getColumn(i) == EMPTY_BAR);
              i--, spaceCount++);
        }

        if (maxCount <= spaceCount) _fsmState = END;  // enough of a space between messages, end this FSM
      }
      break;

    default:
      _fsmState = END;
      break;
    }
  }
}
