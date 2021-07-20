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
 * \brief Implements static print effect
 */

void MD_PZone::commonPrint(void)
{
  int16_t nextPos;

  PRINTS("\ncommonPrint");
  zoneClear();
  nextPos = _limitLeft;
  getFirstChar(_charCols);
  _countCols = 0;

  while (nextPos >= _limitRight)
  {
    PRINT("\nCountCol=", _countCols);
    PRINT(" CharCol=", _charCols);
    if (_countCols == _charCols || _charCols == 0)
    {
      getNextChar(_charCols);
      _countCols = 0;
    }

    // now put something on the display
    if (_charCols != 0)
      _MX->setColumn(nextPos--, DATA_BAR(_cBuf[_countCols++]));
  }
}

void MD_PZone::effectPrint(bool bIn)
// Just print the message in the justification selected
{
  if (bIn)  // incoming
  {
    commonPrint();
    _fsmState = PAUSE;
  }
  else  //exiting
  {
    zoneClear();
    _fsmState = END;
  }
}
