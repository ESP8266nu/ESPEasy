/*
MD_Parola - Library for modular scrolling text and Effects

See header file for comments

Copyright (C) 2018 Marco Colli. All rights reserved.

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
 * \brief Implements various sprite effects
 */

#if ENA_SPRITE

void MD_PZone::effectSprite(bool bIn, uint8_t id)
// Animated Pacman sprite leads or eats up the message.
// Print up the whole message and then remove the parts we
// don't need in order to do the animation.
{
  if (bIn)  // incoming - sprite moves left to right in the zone
  {
    switch (_fsmState)
    {
    case INITIALISE:
      PRINT_STATE("I SPRITE");
      setInitialEffectConditions();
      if (_startPos < _endPos)
      {
        int16_t t = _startPos;
        _startPos = _endPos;
        _endPos = t;
      }
      if (_spriteInData == nullptr)
      {
        _fsmState = END;
        break;
      }
      _posOffset = 0;   // current animation frame for the sprite
      _nextPos = ZONE_END_COL(_zoneEnd) + 1;
      _fsmState = PUT_CHAR;
      // fall through to next state

    case GET_FIRST_CHAR:
    case GET_NEXT_CHAR:
    case PUT_CHAR:
    case PAUSE:
      PRINT_STATE("I SPRITE");

      commonPrint();

      // move reference column and draw new graphic
      _nextPos--;
      for (uint8_t i = 0; i < _spriteInWidth; i++)
      {
        if ((_nextPos + i) <= ZONE_END_COL(_zoneEnd) && (_nextPos + i) >= ZONE_START_COL(_zoneStart))
          _MX->setColumn(_nextPos + i, DATA_BAR(pgm_read_byte(_spriteInData + (_posOffset * _spriteInWidth) + i)));
      }

      // blank out the part of the display we don't need
      // this is the part to the right of the sprite
      for (int16_t i = _nextPos - 1; i >= _endPos; i--)
        _MX->setColumn(i, EMPTY_BAR);

      // advance the animation frame
      _posOffset++;
      if (_posOffset >= _spriteInFrames)
        _posOffset = 0;

      // check if we have finished
      if (_nextPos == ZONE_START_COL(_zoneStart) - _spriteInWidth - 1)
        _fsmState = PAUSE;
      break;

    default:
      PRINT_STATE("I SPRITE");
      _fsmState = PAUSE;
    }
  }
  else  // exiting - sprite moves left to right in the zone
  {
    switch (_fsmState)
    {
    case PAUSE:
    case INITIALISE:
      PRINT_STATE("O SPRITE");
      setInitialEffectConditions();
      if (_startPos < _endPos)
      {
        int16_t t = _startPos;
        _startPos = _endPos;
        _endPos = t;
      }
      if (_spriteOutData == nullptr)
      {
        _fsmState = END;
        break;
      }
      _nextPos = ZONE_START_COL(_zoneStart) - 1;
      _posOffset = 0;
      _fsmState = PUT_CHAR;
      // fall through to next state

    case GET_FIRST_CHAR:
    case GET_NEXT_CHAR:
    case PUT_CHAR:
      PRINT_STATE("O SPRITE");
      commonPrint();

      // move reference column and draw new graphic
      _nextPos++;
      for (uint8_t i = 0; i < _spriteOutWidth; i++)
      {
        if ((_nextPos - i) <= ZONE_END_COL(_zoneEnd) && (_nextPos - i) >= ZONE_START_COL(_zoneStart))
          _MX->setColumn(_nextPos - i, DATA_BAR(pgm_read_byte(_spriteOutData + (_posOffset * _spriteOutWidth) + i)));
      }

      // blank out the part of the display we don't need
      // this is the part to the right of the sprite
      for (int16_t i = _nextPos - _spriteOutWidth; i >= _endPos; i--)
        _MX->setColumn(i, EMPTY_BAR);

      // advance the animation frame
      _posOffset++;
      if (_posOffset >= _spriteOutFrames)
        _posOffset = 0;

      // check if we have finished
      if (_nextPos == ZONE_END_COL(_zoneEnd) + _spriteOutWidth + 1)
        _fsmState = END;
      break;

    default:
      PRINT_STATE("O SPRITE");
      _fsmState = END;
      break;
    }
  }
}

#endif