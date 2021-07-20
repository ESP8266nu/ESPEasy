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
 * \brief Implements scan effect
 */

void MD_PZone::effectHScan(bool bIn, bool bBlank)
// Scan the message end to end.
// if bBlank is true, a blank column scans the text. If false, a non-blank scans the text.
// Print up the whole message and then remove the parts we
// don't need in order to do the animation.
{
  if (bIn)  // incoming
  {
    switch (_fsmState)
    {
    case INITIALISE:
      PRINT_STATE("I SCANH");
      setInitialEffectConditions();
      _fsmState = PUT_CHAR;
      // fall through to next state

    case GET_FIRST_CHAR:
    case GET_NEXT_CHAR:
    case PUT_CHAR:
    case PAUSE:
      PRINT_STATE("I SCANH");

      commonPrint();
      // check if we have finished
      if (_nextPos == _endPos)
      {
        _fsmState = PAUSE;
        break;
      }

      // blank out the part of the display we don't need
      FSMPRINT("Scan col ", _nextPos);
      for (int16_t i = _startPos; i != _endPos + _posOffset; i += _posOffset)
      {
        if ((!bBlank && (i != _nextPos)) || (bBlank && (i == _nextPos)))
          _MX->setColumn(i, EMPTY_BAR);
      }

      _nextPos += _posOffset; // for the next time around
      break;

    default:
      PRINT_STATE("I SCANH");
      _fsmState = PAUSE;
    }
  }
  else  // exiting
  {
    switch (_fsmState)
    {
    case PAUSE:
    case INITIALISE:
      PRINT_STATE("O SCANH");
      setInitialEffectConditions();
      _fsmState = PUT_CHAR;
      // fall through to next state

    case GET_FIRST_CHAR:
    case GET_NEXT_CHAR:
    case PUT_CHAR:
      PRINT_STATE("O SCANH");
      commonPrint();

      // blank out the part of the display we don't need
      FSMPRINT(" Scan col ", _nextPos);
      for (int16_t i = _startPos; i != _endPos + _posOffset; i += _posOffset)
      {
        if ((!bBlank && (i != _nextPos)) || (bBlank && (i == _nextPos)))
          _MX->setColumn(i, EMPTY_BAR);
      }

      // check if we have finished
      if (_nextPos < _endPos) _fsmState = END;

      _nextPos += _posOffset; // for the next time around
      break;

    default:
      PRINT_STATE("O SCANH");
      _fsmState = END;
      break;
    }
  }
}

void MD_PZone::effectVScan(bool bIn, bool bBlank)
// Scan the message over with a new one
// if bBlank is true, a blank column scans the text. If false, a non-blank scans the text.
// Print up the whole message and then remove the parts we
// don't need in order to do the animation.
{
  uint8_t maskCol = 0;

  if (bIn)  // incoming
  {
    switch (_fsmState)
    {
    case INITIALISE:
      PRINT_STATE("I SCANV");
      setInitialEffectConditions();
      _nextPos = 0; // this is the bit number
      _fsmState = PUT_CHAR;
      // fall through to next state

    case GET_FIRST_CHAR:
    case GET_NEXT_CHAR:
    case PUT_CHAR:
    case PAUSE:
      PRINT_STATE("I SCANV");
      commonPrint();

      // check if we have finished
      if (_nextPos == 8) // bits numbered 0 to 7
      {
        _fsmState = PAUSE;
        break;
      }

      // blank out the part of the display we don't need
      FSMPRINT("Keep bit ", _nextPos);
      maskCol = (1 << _nextPos);
      for (int16_t i = _startPos; i != _endPos + _posOffset; i += _posOffset)
      {
        uint8_t c = DATA_BAR(_MX->getColumn(i) & (bBlank ? ~maskCol : maskCol));

        _MX->setColumn(i, DATA_BAR(c));
      }

      _nextPos++; // for the next time around
      break;

    default:
      PRINT_STATE("I SCANV");
      _fsmState = PAUSE;
    }
  }
  else  // exiting
  {
    switch (_fsmState)
    {
    case PAUSE:
    case INITIALISE:
      PRINT_STATE("O SCANV");
      setInitialEffectConditions();
      _nextPos = 7; // the bit number
      _fsmState = PUT_CHAR;
      // fall through to next state

    case GET_FIRST_CHAR:
    case GET_NEXT_CHAR:
    case PUT_CHAR:
      PRINT_STATE("O SCANV");

      commonPrint();

      // blank out the part of the display we don't need
      FSMPRINT(" Keep bit ", _nextPos);
      if (_nextPos >= 0)
        maskCol = 1 << _nextPos;
      for (int16_t i = _startPos; i != _endPos + _posOffset; i += _posOffset)
      {
        uint8_t c = DATA_BAR(_MX->getColumn(i) & (bBlank ? ~maskCol : maskCol));

        _MX->setColumn(i, DATA_BAR(c));
      }

      // check if we have finished
      if (_nextPos < 0)
        _fsmState = END;

      _nextPos--; // for the next time around
      break;

    default:
      PRINT_STATE("O SCANV");
      _fsmState = END;
      break;
    }
  }
}
