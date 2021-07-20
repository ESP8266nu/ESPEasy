/*
MD_Parola - Library for modular scrolling text and Effects

See header file for comments

Copyright (C) 2016 Marco Colli. All rights reserved.

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
 * \brief Implements FADE effect
 */

void MD_PZone::effectFade(bool bIn)
// Fade the display in and out.
// If the overall intensity is changed while the animation is running, the
// intensity at the start of the animation will be restored at the end, overriding
// any user code changes.
{
  if (bIn) // incoming
  {
    switch (_fsmState)
    {
    case INITIALISE:
      PRINT_STATE("I FADE");
      _nextPos = 0;
      _endPos = getIntensity();

      zoneClear();

      _fsmState = GET_FIRST_CHAR;
      break;

    case GET_FIRST_CHAR:
      FSMPRINT(" I:", _nextPos);
      FSMPRINT("/", _endPos);

      setIntensity(_nextPos++);
      commonPrint();
      _fsmState = PUT_CHAR;
      break;

    case GET_NEXT_CHAR:
    case PUT_CHAR:
    case PAUSE:
      PRINT_STATE("I FADE");
      FSMPRINT(" I:", _nextPos);
      FSMPRINT("/", _endPos);

      // check if we have finished
      if (_nextPos > _endPos)
        _fsmState = PAUSE;
      else
        setIntensity(_nextPos++);
      break;

    default:
      PRINT_STATE("I FADE");
      _fsmState = PAUSE;
    }
  }
  else  // exiting
  {
    switch (_fsmState)
    {
    case PAUSE:
    case INITIALISE:
      PRINT_STATE("O FADE");
      _nextPos = _endPos = getIntensity();

      FSMPRINT(" I:", _nextPos);
      FSMPRINT("/", _endPos);

      setIntensity(_nextPos);
      commonPrint();

      _fsmState = PUT_CHAR;
      break;

    case GET_FIRST_CHAR:
    case GET_NEXT_CHAR:
    case PUT_CHAR:
      PRINT_STATE("O FADE");

      FSMPRINT(" I:", _nextPos);
      FSMPRINT("/", _endPos);

      // check if we have finished
      if (_nextPos < 0)
      {
        setIntensity(_endPos);  // set to original conditions
        zoneClear();            // display nothing - we are currently at 0
        _fsmState = END;
      }
      else
        setIntensity(_nextPos--);
      break;

    default:
      PRINT_STATE("O FADE");
      _fsmState = END;
      break;
    }
  }
}
