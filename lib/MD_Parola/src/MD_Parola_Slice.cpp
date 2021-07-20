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
 * \brief Implements slice effect
 */

void MD_PZone::effectSlice(bool bIn)
{
  if (bIn)
  {
    switch(_fsmState)
    {
    case INITIALISE:
    case GET_FIRST_CHAR:
      PRINT_STATE("I SLICE");

      if (!getFirstChar(_charCols))
      {
        _fsmState = END;
        break;
      }
      zoneClear();
      _countCols = 0;
      _nextPos = ZONE_START_COL(_zoneStart);
      _endPos = _limitLeft;

      FSMPRINT(" - Start ", _nextPos);
      FSMPRINT(", End ", _endPos);

      _fsmState = PUT_CHAR;
      break;

    case GET_NEXT_CHAR: // Load the next character from the font table
      PRINT_STATE("I SLICE");
      // Have we reached the end of the characters string?
      do
      {
        if (!getNextChar(_charCols))
          _fsmState = PAUSE;
      } while (_charCols == 0 && _fsmState != PAUSE);

      if (_fsmState == PAUSE) break;

      _countCols = 0;
      _fsmState = PUT_CHAR;
      // !! fall through to next state to start displaying

    case PUT_CHAR:  // display the next part of the character
      PRINT_STATE("I SLICE");
      FSMPRINT(" - Next ", _endPos);
      FSMPRINT(", anim ", _nextPos);

      // if the text is too long for the zone, stop when we are at the last column of the zone
      if (_nextPos == _endPos)
      {
        _MX->setColumn(_nextPos, DATA_BAR(_cBuf[_countCols]));
        _fsmState = PAUSE;
        break;
      }

      if (_cBuf[_countCols] == 0) // empty column ?
      {
        _nextPos = _endPos; // pretend we just animated it!
      }
      else  // something to animate
      {
        // clear the column and animate the next one
        if (_nextPos != _endPos) _MX->setColumn(_nextPos, EMPTY_BAR);
        _nextPos++;
        _MX->setColumn(_nextPos, DATA_BAR(_cBuf[_countCols]));
      }

      // set up for the next time
      if (_nextPos == _endPos)
      {
        _nextPos = ZONE_START_COL(_zoneStart);
        _countCols++;
        _endPos--;
      }
      if (_countCols == _charCols) _fsmState = GET_NEXT_CHAR;
      break;

    default:
      _fsmState = PAUSE;
    }
  }
  else  // exiting
  {
    switch(_fsmState)
    {
    case PAUSE:
      PRINT_STATE("O SLICE");
      _nextPos = _endPos = _limitLeft;
      _fsmState = PUT_CHAR;
      // fall through

    case GET_FIRST_CHAR:
    case GET_NEXT_CHAR:
    case PUT_CHAR:
      PRINT_STATE("O SLICE");
      FSMPRINT(" - Next ", _endPos);
      FSMPRINT(", anim ", _nextPos);

      while(_MX->getColumn(_nextPos) == EMPTY_BAR && _endPos >= _limitRight)
        _nextPos = _endPos--; // pretend we just animated it!

      if (_endPos + 1 < _limitRight)
        _fsmState = END;  //reached the end
      else
      {
        // Move the column over to the left and blank out previous position
        if (_nextPos < ZONE_END_COL(_zoneEnd))
          _MX->setColumn(_nextPos + 1, _MX->getColumn(_nextPos));
        _MX->setColumn(_nextPos, EMPTY_BAR);
        _nextPos++;

        // set up for the next time
        if (_nextPos == ZONE_END_COL(_zoneEnd) + 1)
          _nextPos = _endPos--;
      }
      break;

    default:
      _fsmState = END;
    }
  }
}
