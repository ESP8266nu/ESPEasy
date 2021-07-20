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
 * \brief Implements wipe effect
 */

void MD_PZone::effectWipe(bool bLightBar, bool bIn)
// Wipe the message over with a new one
// Print up the whole message and then remove the parts we
// don't need in order to do the animation.
{
  if (bIn)  // incoming
  {
    switch (_fsmState)
    {
    case INITIALISE:
      PRINT_STATE("I WIPE");
      setInitialEffectConditions();
      _fsmState = PUT_CHAR;
      // fall through to next state

    case GET_FIRST_CHAR:
    case GET_NEXT_CHAR:
    case PUT_CHAR:
    case PAUSE:
      PRINT_STATE("I WIPE");
      if (_fsmState == PAUSE)
        _fsmState = PUT_CHAR;

      commonPrint();

      // blank out the part of the display we don't need
      FSMPRINT(" - Clear ", _nextPos);
      FSMPRINT(" to ", _endPos);
      FSMPRINT(" step ", _posOffset);
      for (int16_t i = _nextPos; i != _endPos + _posOffset; i += _posOffset)
        _MX->setColumn(i, EMPTY_BAR);

      if (bLightBar && (_nextPos != _endPos + _posOffset)) _MX->setColumn(_nextPos, LIGHT_BAR);

      // check if we have finished
      if (_nextPos == _endPos + _posOffset) _fsmState = PAUSE;

      _nextPos += _posOffset; // for the next time around
      break;

    default:
      PRINT_STATE("I WIPE");
      _fsmState = PAUSE;
    }
  }
  else  // exiting
  {
    switch (_fsmState)
    {
    case PAUSE:
    case INITIALISE:
      PRINT_STATE("O WIPE");
      setInitialEffectConditions();
      _fsmState = PUT_CHAR;
      // fall through to next state

    case GET_FIRST_CHAR:
    case GET_NEXT_CHAR:
    case PUT_CHAR:
      PRINT_STATE("O WIPE");
      commonPrint();

      // blank out the part of the display we don't need
      FSMPRINT(" - Clear ", _nextPos);
      FSMPRINT(" to ", _endPos);
      FSMPRINT(" step ", _posOffset);
      for (int16_t i = _startPos; i != _nextPos + _posOffset; i += _posOffset)
        _MX->setColumn(i, EMPTY_BAR);

      if (bLightBar && (_nextPos != _endPos + _posOffset)) _MX->setColumn(_nextPos, LIGHT_BAR);

      // check if we have finished
      if (_nextPos == _endPos + _posOffset) _fsmState = END;

      _nextPos += _posOffset; // for the next time around
      break;

    default:
      PRINT_STATE("O WIPE");
      _fsmState = END;
      break;
    }
  }
}
