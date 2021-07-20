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
#include <MD_MAX72xx.h>
/**
 * \file
 * \brief Implements MD_PZone class methods
 */

MD_PZone::MD_PZone(void) :
_MX(nullptr), _suspend(false), _lastRunTime(0),
_fsmState(END), _scrollDistance(0), _zoneEffect(0), _pText(nullptr), 
_userChars(nullptr), _cBufSize(0), _cBuf(nullptr), _charSpacing(1), 
_fontDef(nullptr)
#if ENA_SPRITE
, _spriteInData(nullptr), _spriteOutData(nullptr)
#endif
{
};

MD_PZone::~MD_PZone(void)
{
  // release the memory for user defined characters
  charDef_t *p = _userChars;

  while (p != nullptr)
  {
    charDef_t *pt = p;
    p = pt->next;
    delete pt;
  };

  // release memory for the character buffer
  delete[] _cBuf;
}

void MD_PZone::begin(MD_MAX72XX *p)
{
  _MX = p;
  allocateFontBuffer();
  delay(0); // Feed the watchdog
}

void MD_PZone::allocateFontBuffer(void)
{
  uint8_t size = _MX->getMaxFontWidth() + getCharSpacing();
  PRINTS("\nallocateFontBuffer");
  if (size > _cBufSize)
  {
    if (_cBuf != nullptr) delete[] _cBuf;
    _cBufSize = size;
    _cBuf = new uint8_t[_cBufSize];
  }
}

void MD_PZone::setZoneEffect(boolean b, zoneEffect_t ze)
{
  switch (ze)
  {
  case PA_FLIP_LR: _zoneEffect = (b ? ZE_SET(_zoneEffect, ZE_FLIP_LR_MASK) : ZE_RESET(_zoneEffect, ZE_FLIP_LR_MASK));  break;
  case PA_FLIP_UD: _zoneEffect = (b ? ZE_SET(_zoneEffect, ZE_FLIP_UD_MASK) : ZE_RESET(_zoneEffect, ZE_FLIP_UD_MASK));  break;
  }

  return;
}

boolean MD_PZone::getZoneEffect(zoneEffect_t ze)
{
  switch (ze)
  {
  case PA_FLIP_LR: return(ZE_TEST(_zoneEffect, ZE_FLIP_LR_MASK)); break;
  case PA_FLIP_UD: return(ZE_TEST(_zoneEffect, ZE_FLIP_UD_MASK)); break;
  }

  return(false);
}

#if ENA_SPRITE
void MD_PZone::setSpriteData(const uint8_t *inData, uint8_t inWidth, uint8_t inFrames, const uint8_t* outData, uint8_t outWidth, uint8_t outFrames)
{
  _spriteInData = (uint8_t *)inData;
  _spriteInWidth = inWidth;
  _spriteInFrames = inFrames;
  _spriteOutData = (uint8_t *)outData;
  _spriteOutWidth = outWidth;
  _spriteOutFrames = outFrames;
}
#endif

void MD_PZone::setInitialConditions(void)
// set the global variables initial conditions for all display effects
{
  PRINTS("\nsetInitialConditions");

  if (_pText == nullptr)
    return;

  _pCurChar = _pText;
  _limitOverflow = !calcTextLimits(_pText);
}

void MD_PZone::setInitialEffectConditions(void)
// set the initial conditions for loops in the FSM
{
  PRINTS("\nsetInitialFSMConditions");

  _startPos = _nextPos = (_textAlignment == PA_RIGHT ? _limitRight : _limitLeft);
  _endPos = (_textAlignment == PA_RIGHT ? _limitLeft : _limitRight);
  _posOffset = (_textAlignment == PA_RIGHT ? 1 : -1);
}

uint16_t MD_PZone::getTextWidth(const uint8_t *p)
// Get the width in columns for the text string passed to the function
// This is the sum of all the characters and the space between them.
{
  uint16_t  sum = 0;
  uint16_t  width;

  PRINT("\ngetTextWidth: ", (const char *)p);

  while (*p != '\0')
  {
    width = findChar(*p++, _cBufSize, _cBuf);
    sum += width;
    if (width != 0 && *p) sum += _charSpacing;  // this char had width, so add inter-character spacing
  }

  PRINT("\ngetTextWidth: W=", sum);

  return(sum);
}

bool MD_PZone::calcTextLimits(const uint8_t *p)
// Work out left and right sides for the text to be displayed,
// depending on the text alignment. If the message will not fit
// in the current display the return false, otherwise true.
{
  bool b = true;
  uint16_t displayWidth = ZONE_END_COL(_zoneEnd) - ZONE_START_COL(_zoneStart) + 1;

  _textLen = getTextWidth(p);

  PRINT("\ncalcTextLimits: disp=", displayWidth);
  PRINT(" text=", _textLen);

  switch (_textAlignment)
  {
  case PA_LEFT:
    _limitLeft = ZONE_END_COL(_zoneEnd);
    if (_textLen > displayWidth)
    {
      _limitRight = ZONE_START_COL(_zoneStart);
      b = false;
     }
    else
    {
      _limitRight = _limitLeft - _textLen + 1;
    }
    break;

  case PA_RIGHT:
    _limitRight = ZONE_START_COL(_zoneStart);
    if (_textLen > displayWidth)
    {
      _limitLeft = ZONE_END_COL(_zoneEnd);
      b = false;
    }
    else
    {
      _limitLeft = _limitRight + _textLen - 1;
    }
    break;

  case PA_CENTER:
    if (_textLen > displayWidth)
    {
      _limitLeft = ZONE_END_COL(_zoneEnd);
      _limitRight = ZONE_START_COL(_zoneStart);
      b = false;
    }
    else
    {
      _limitRight = ZONE_START_COL(_zoneStart) + ((displayWidth - _textLen) / 2);
      _limitLeft = _limitRight + _textLen - 1;
    }
    break;
  }

  PRINT(" -> L:", _limitLeft);
  PRINT(" R:", _limitRight);
  PRINT(" Oveflow:", !b);

  return (b);
}

bool MD_PZone::addChar(uint16_t code, uint8_t *data)
// Add a user defined character to the replacement list
{
  charDef_t *pcd;

  if (code == 0)
    return(false);

  PRINTX("\naddChar 0x", code);

  // first see if we have the code in our list
  pcd = _userChars;
  while (pcd != nullptr)
  {
    if (pcd->code == code)
    {
      pcd->data = data;
      PRINTS(" found existing in list");
      return(true);
    }
    pcd = pcd->next;
  }

  // Now see if we have an empty slot in our list
  pcd = _userChars;
  while (pcd != nullptr)
  {
    if (pcd->code == 0)
    {
      pcd->code = code;
      pcd->data = data;
      PRINTS(" found empty slot");
      return(true);
    }
    pcd = pcd->next;
  }

  // default is to add a new node to the front of the list
  if ((pcd = new charDef_t) != nullptr)
  {
    pcd->code = code;
    pcd->data = data;
    pcd->next = _userChars;
    _userChars = pcd;
    PRINTS(" added new node");
  }
  else
  {
    PRINTS(" failed allocating new node");
  }

  return(pcd != nullptr);
}

bool MD_PZone::delChar(uint16_t code)
// Delete a user defined character from the replacement list
{
  charDef_t *pcd = _userChars;

  if (code == 0)
    return(false);

  // Scan down the linked list
  while (pcd != nullptr)
  {
    if (pcd->code == code)
    {
      pcd->code = 0;
      pcd->data = nullptr;
      break;
    }
    pcd = pcd->next;
  }

  return(pcd != nullptr);
}

uint8_t MD_PZone::findChar(uint16_t code, uint8_t size, uint8_t *cBuf)
// Find a character either in user defined list or from font table
{
  charDef_t *pcd = _userChars;
  uint8_t len;

  PRINTX("\nfindUserChar 0x", code);
  // check local list first
  while (pcd != nullptr)
  {
    PRINTX(" ", pcd->code);
    if (pcd->code == code)  // found it
    {
      PRINTS(" found character");
      len = min(size, pcd->data[0]);
      memcpy(cBuf, &pcd->data[1], len);
      return(len);
    }
    pcd = pcd->next;
  }

  // get it from the standard font
  PRINTS(" no user char");
  _MX->setFont(_fontDef);   // change to the font for this zone
  len = _MX->getChar(code, size, cBuf);

  return(len);
}

uint8_t MD_PZone::makeChar(uint16_t c, bool addBlank)
// Load a character bitmap and add in trailing char spacing blanks
{
  uint8_t len;

  // look for the character
  len = findChar(c, _cBufSize, _cBuf);

  PRINTX("\nmakeChar 0x", c);
  PRINT(", len=", len);

  // Add in the inter char spacing
  if (addBlank && len != 0)
  {
    for (uint8_t i = 0; i < _charSpacing; i++)
    {
      if (len < _cBufSize)
        _cBuf[len++] = 0;
    }
  }

  return(len);
}

void MD_PZone::reverseBuf(uint8_t *p, uint8_t size)
// reverse the elements of the specified buffer
// useful when we are scrolling right and want to insert the columns in reverse order
{
  for (uint8_t i = 0; i < size / 2; i++)
  {
    uint8_t t;

    t = p[i];
    p[i] = p[size - 1 - i];
    p[size - 1 - i] = t;
  }
}

void MD_PZone::invertBuf(uint8_t *p, uint8_t size)
// invert the elements of the specified buffer
// used when the character needs to be inverted when ZE_FLIP_UD
{
  for (uint8_t i = 0; i < size; i++)
  {
    uint8_t v = p[i];

    v = ((v >> 1) & 0x55) | ((v & 0x55) << 1);  // swap odd and even bits
    v = ((v >> 2) & 0x33) | ((v & 0x33) << 2);  // swap consecutive pairs
    v = ((v >> 4) & 0x0F) | ((v & 0x0F) << 4);  // swap nibbles ...

    p[i] = v;
  }
}

void MD_PZone::moveTextPointer(void)
// This method works when increment is done AFTER processing the character
// The _endOfText flag is set as a look ahead (ie, when the last character
// is still valid)
// We need to move a pointer forward or back, depending on the way we are
// travelling through the text buffer.
{
  PRINTS("\nMovePtr");

  if ((!ZE_TEST(_zoneEffect, ZE_FLIP_LR_MASK) && SFX(PA_SCROLL_RIGHT)) ||
    (ZE_TEST(_zoneEffect, ZE_FLIP_LR_MASK) && !SFX(PA_SCROLL_RIGHT)))
  {
    PRINTS(" --");
    _endOfText = (_pCurChar == _pText);
    _pCurChar--;
  }
  else
  {
    PRINTS(" ++");
    _pCurChar++;
    _endOfText = (*_pCurChar == '\0');
  }

  PRINT(": endOfText ", _endOfText);
}

bool MD_PZone::getFirstChar(uint8_t &len)
// load the first char into the char buffer, set len to the number of columns
// return false if there are no characters
{
  len = 0;

  PRINT("\ngetFirst SFX(RIGHT):", SFX(PA_SCROLL_RIGHT));
  PRINT(" ZETEST(UD):", ZE_TEST(_zoneEffect, ZE_FLIP_UD_MASK));
  PRINT(" ZETEST(LR):", ZE_TEST(_zoneEffect, ZE_FLIP_LR_MASK));

  // initialise pointers and make sure we have a good string to process
  _pCurChar = _pText;
  if ((_pCurChar == nullptr) || (*_pCurChar == '\0'))
  {
    _endOfText = true;
    return(false);
  }
  _endOfText = false;
  if ((!ZE_TEST(_zoneEffect, ZE_FLIP_LR_MASK) && (SFX(PA_SCROLL_RIGHT))) ||
    (ZE_TEST(_zoneEffect, ZE_FLIP_LR_MASK) && !SFX(PA_SCROLL_RIGHT)))
  {
    PRINTS("\nReversed String");
    _pCurChar += strlen((const char *)_pText) - 1;
  }

  // good string, get the first char into the current buffer
  len = makeChar(*_pCurChar, *(_pCurChar + 1) != '\0');

  if ((!ZE_TEST(_zoneEffect, ZE_FLIP_LR_MASK) && (SFX(PA_SCROLL_RIGHT))) ||
    (ZE_TEST(_zoneEffect, ZE_FLIP_LR_MASK) && !SFX(PA_SCROLL_RIGHT)))
  {
    PRINTS("\nReverse Buffer");
    reverseBuf(_cBuf, len);
  }

  if ZE_TEST(_zoneEffect, ZE_FLIP_UD_MASK)
  {
    PRINTS("\nInvert buffer");
    invertBuf(_cBuf, len);
  }

  moveTextPointer();

  return(true);
}

bool MD_PZone::getNextChar(uint8_t &len)
// load the next char into the char buffer, set len to the number of columns
// return false if there are no characters
{
  len = 0;

  PRINT("\ngetNexChar SFX(RIGHT):", SFX(PA_SCROLL_RIGHT));
  PRINT(" ZETEST(UD):", ZE_TEST(_zoneEffect, ZE_FLIP_UD_MASK));
  PRINT(" ZETEST(LR):", ZE_TEST(_zoneEffect, ZE_FLIP_LR_MASK));

  if (_endOfText)
    return(false);

  len = makeChar(*_pCurChar, *(_pCurChar + 1) != '\0');

  if ((!ZE_TEST(_zoneEffect, ZE_FLIP_LR_MASK) && (SFX(PA_SCROLL_RIGHT))) ||
    (ZE_TEST(_zoneEffect, ZE_FLIP_LR_MASK) && !SFX(PA_SCROLL_RIGHT)))
  {
    PRINTS("\nReversed Buffer");
    reverseBuf(_cBuf, len);
  }

  if ZE_TEST(_zoneEffect, ZE_FLIP_UD_MASK)
  {
    PRINTS("\nInvert Buffer");
    invertBuf(_cBuf, len);
  }

  moveTextPointer();

  return(true);
}

bool MD_PZone::zoneAnimate(void)
{
#if TIME_PROFILING
  static uint32_t  cycleStartTime;
#endif
  _animationAdvanced = false;   // assume this will not happen this time around

  if (_fsmState == END)
    return(true);

  // work through things that stop us running this at all
  uint32_t tickTime = (_moveIn ? _tickTimeIn : _tickTimeOut);
  if (((_fsmState == PAUSE) && (millis() - _lastRunTime < _pauseTime)) ||
    (millis() - _lastRunTime < tickTime) ||
    (_suspend))
    return(false);

  // save the time now, before we run the animation, so that the animation is part of the
  // delay between animations giving more accurate frame timing.
  _lastRunTime = millis();
  _animationAdvanced = true;    // we now know it will happen!

  // any text to display?
  if (_pText != nullptr)
  {
    switch (_fsmState)
    {
      case END:   // do nothing in this state
        PRINT_STATE("ANIMATE");
        break;

      case INITIALISE:
        PRINT_STATE("ANIMATE");
#if TIME_PROFILING
        cycleStartTime = millis();
#endif
        setInitialConditions();
        _moveIn = true;
        // fall through to process the effect, first call will be with INITIALISE

      default: // All state except END are handled by the special effect functions
        PRINT_STATE("ANIMATE");
        switch (_moveIn ? _effectIn : _effectOut)
        {
        case PA_PRINT:        effectPrint(_moveIn);           break;
        case PA_SCROLL_UP:    effectVScroll(true, _moveIn);   break;
        case PA_SCROLL_DOWN:  effectVScroll(false, _moveIn);  break;
        case PA_SCROLL_LEFT:  effectHScroll(true, _moveIn);   break;
        case PA_SCROLL_RIGHT: effectHScroll(false, _moveIn);  break;
#if ENA_MISC
        case PA_SLICE:    effectSlice(_moveIn);     break;
        case PA_MESH:     effectMesh(_moveIn);      break;
        case PA_FADE:     effectFade(_moveIn);      break;
        case PA_BLINDS:   effectBlinds(_moveIn);    break;
        case PA_DISSOLVE: effectDissolve(_moveIn);  break;
        case PA_RANDOM:   effectRandom(_moveIn);    break;
#endif // ENA_MISC
#if ENA_SPRITE
        case PA_SPRITE:
          effectSprite(_moveIn, _moveIn ? _effectIn : _effectOut);  break;
#endif // ENA_SPRITE
#if ENA_WIPE
        case PA_WIPE:         effectWipe(false, _moveIn); break;
        case PA_WIPE_CURSOR:  effectWipe(true, _moveIn);  break;
#endif // ENA_WIPE
#if ENA_SCAN
        case PA_SCAN_HORIZX: effectHScan(_moveIn, true); break;
        case PA_SCAN_HORIZ:  effectHScan(_moveIn, false); break;
        case PA_SCAN_VERTX:  effectVScan(_moveIn, true); break;
        case PA_SCAN_VERT:   effectVScan(_moveIn, false); break;
#endif // ENA_SCAN
#if ENA_OPNCLS
        case PA_OPENING:        effectOpen(false, _moveIn); break;
        case PA_OPENING_CURSOR: effectOpen(true, _moveIn);  break;
        case PA_CLOSING:        effectClose(false, _moveIn); break;
        case PA_CLOSING_CURSOR: effectClose(true, _moveIn); break;
#endif // ENA_OPNCLS
#if ENA_SCR_DIA
        case PA_SCROLL_UP_LEFT:     effectDiag(true, true, _moveIn);  break;
        case PA_SCROLL_UP_RIGHT:    effectDiag(true, false, _moveIn); break;
        case PA_SCROLL_DOWN_LEFT:   effectDiag(false, true, _moveIn); break;
        case PA_SCROLL_DOWN_RIGHT:  effectDiag(false, false, _moveIn); break;
#endif // ENA_SCR_DIA
#if ENA_GROW
        case PA_GROW_UP:     effectGrow(true, _moveIn);  break;
        case PA_GROW_DOWN:   effectGrow(false, _moveIn); break;
#endif // ENA_GROW
        default:
          _fsmState = END;
      }

      // one way toggle for input to output, reset on initialize
      _moveIn = _moveIn && !(_fsmState == PAUSE);
      break;
    }
  }

#if  TIME_PROFILING
  Serial.print("\nAnim time: ");
  Serial.print(millis() - _lastRunTime);
  if (_fsmState == END)
  {
    Serial.print("\nCycle time: ");
    Serial.print(millis() - cycleStartTime);
  }
#endif

  return(_fsmState == END);
}

#if DEBUG_PAROLA_FSM
const char *MD_PZone::state2string(fsmState_t s)
{
  switch (s)
  {
  case INITIALISE:     return("INIT");
  case GET_FIRST_CHAR: return("FIRST");
  case GET_NEXT_CHAR:  return("NEXT");
  case PUT_CHAR:       return("PUT");
  case PUT_FILLER:     return("FILLER");
  case PAUSE:          return("PAUSE");
  case END:            return("END");
  }
  return("???");
};
#endif
