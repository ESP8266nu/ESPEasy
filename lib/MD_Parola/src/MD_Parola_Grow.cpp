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
 * \brief Implements grow effects
 */

void MD_PZone::effectGrow(bool bUp, bool bIn)
// Scan the message over with a new one
// Print up the whole message and then remove the parts we
// don't need in order to do the animation.
{
  if (bIn)  // incoming
  {
    switch (_fsmState)
    {
    case INITIALISE:
      PRINT_STATE("I GROW");
      setInitialEffectConditions();
      _nextPos = (bUp ? 0xff : 1); // this is the bit mask
      _fsmState = PUT_CHAR;
      // fall through to next state

    case GET_FIRST_CHAR:
    case GET_NEXT_CHAR:
    case PUT_CHAR:
    case PAUSE:
      PRINT_STATE("I GROW");

      commonPrint();
      // check if we have finished
      if (_nextPos == (bUp ? 0 : 0xff)) // all bits covered
      {
        _fsmState = PAUSE;
        break;
      }

      // blank out the part of the display we don't need
      FSMPRINT("Keep bits ", _nextPos);
      for (int16_t i = _startPos; i != _endPos + _posOffset; i += _posOffset)
      {
        uint8_t c = DATA_BAR(_MX->getColumn(i)) & (bUp ? ~_nextPos : _nextPos);

        _MX->setColumn(i, DATA_BAR(c));
      }

      // for the next time around
      if (bUp)
        _nextPos >>= 1;
      else
        _nextPos = (_nextPos << 1) | 1;
      break;

    default:
      PRINT_STATE("I GROW");
      _fsmState = PAUSE;
    }
  }
  else  // exiting
  {
    switch (_fsmState)
    {
    case PAUSE:
    case INITIALISE:
      PRINT_STATE("O GROW");
      setInitialEffectConditions();
      _nextPos = (bUp ? 1 : 0xff);  // this is the bit mask
      _fsmState = PUT_CHAR;
      // fall through to next state

    case GET_FIRST_CHAR:
    case GET_NEXT_CHAR:
    case PUT_CHAR:
      PRINT_STATE("O GROW");
      commonPrint();

      // blank out the part of the display we don't need
      FSMPRINT(" Keep bits ", _nextPos);
      for (int16_t i =_startPos; i != _endPos + _posOffset; i += _posOffset)
      {
        uint8_t c = DATA_BAR(_MX->getColumn(i)) & (bUp ? ~_nextPos : _nextPos);

        _MX->setColumn(i, DATA_BAR(c));
      }

      // check if we have finished
      if (_nextPos == (bUp ? 0xff : 0x0)) // all bits covered
        _fsmState = END;

      // for the next time around
      if (bUp)
        _nextPos = (_nextPos << 1) | 1;
      else
        _nextPos >>= 1;
      break;

    default:
      PRINT_STATE("O GROW");
      _fsmState = END;
      break;
    }
  }
}
