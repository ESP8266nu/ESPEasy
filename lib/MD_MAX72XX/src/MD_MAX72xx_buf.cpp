/*
MD_MAX72xx - Library for using a MAX7219/7221 LED matrix controller

See header file for comments
This file contains methods that act on display buffers.

Copyright (C) 2012-13 Marco Colli. All rights reserved.

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
#ifdef ARDUINO 
#include <Arduino.h>
#endif
#include "MD_MAX72xx.h"
#include "MD_MAX72xx_lib.h"

/**
 * \file
 * \brief Implements buffer related methods
 */

bool MD_MAX72XX::clear(uint8_t buf)
{
  if (buf > LAST_BUFFER)
    return(false);

  memset(_matrix[buf].dig, 0, sizeof(_matrix[buf].dig));
  _matrix[buf].changed = ALL_CHANGED;

  if (_updateEnabled) flushBuffer(buf);

  return(true);
}

uint8_t MD_MAX72XX::bitReverse(uint8_t b)
// Reverse the order of bits within a byte.
// Returns the reversed byte value.
{
  b = ((b & 0xf0) >>  4) | ((b & 0x0f) << 4);
  b = ((b & 0xcc) >>  2) | ((b & 0x33) << 2);
  b = ((b & 0xaa) >>  1) | ((b & 0x55) << 1);

  return(b);
}

bool MD_MAX72XX::copyColumn(uint8_t buf, uint8_t cSrc, uint8_t cDest)
{
  if (_hwDigRows) return(copyC(buf, cSrc, cDest));
  else return(copyR(buf, cSrc, cDest));
}

bool MD_MAX72XX::copyRow(uint8_t buf, uint8_t rSrc, uint8_t rDest)
{
  if (_hwDigRows) return(copyR(buf, rSrc, rDest));
  else return(copyC(buf, rSrc, rDest));
}

bool MD_MAX72XX::copyC(uint8_t buf, uint8_t cSrc, uint8_t cDest)
// Src and Dest are in pixel coordinates.
// if we are just copying rows there is no need to repackage any data
{
  uint8_t maskSrc = 1 << HW_COL(cSrc);  // which column/row of bits is the column data

  if (_hwDigRows) { PRINT("\ncopyCol: (", buf); }
  else { PRINT("\ncopyRow: (", buf); }
  PRINT(", ", cSrc);
  PRINT(", ", cDest);
  PRINTS(") ");

  if ((buf > LAST_BUFFER) || (cSrc >= COL_SIZE) || (cDest >= COL_SIZE))
    return(false);

  for (uint8_t i=0; i<ROW_SIZE; i++)
  {
      if (_matrix[buf].dig[i] & maskSrc)
        bitSet(_matrix[buf].dig[i], HW_COL(cDest));
    else
        bitClear(_matrix[buf].dig[i], HW_COL(cDest));
  }

  _matrix[buf].changed = ALL_CHANGED;

  if (_updateEnabled) flushBuffer(buf);

  return(true);
}

bool MD_MAX72XX::copyR(uint8_t buf, uint8_t rSrc, uint8_t rDest)
// Src and Dest are in pixel coordinates.
// if we are just copying digits there is no need to repackage any data
{
  if (_hwDigRows) { PRINT("\ncopyRow: (", buf); }
  else { PRINT("\ncopyColumn: (", buf); }
  PRINT(", ", rSrc);
  PRINT(", ", rDest);
  PRINTS(") ");

  if ((buf > LAST_BUFFER) || (rSrc >= ROW_SIZE) || (rDest >= ROW_SIZE))
    return(false);

  _matrix[buf].dig[HW_ROW(rDest)] = _matrix[buf].dig[HW_ROW(rSrc)];
  bitSet(_matrix[buf].changed, HW_ROW(rDest));

  if (_updateEnabled) flushBuffer(buf);

  return(true);
}

uint8_t MD_MAX72XX::getColumn(uint8_t buf, uint8_t c)
{
  if (_hwDigRows) return(getC(buf, c));
  else return(getR(buf, c));
}

uint8_t MD_MAX72XX::getRow(uint8_t buf, uint8_t r)
{
  if (_hwDigRows) return(getR(buf, r));
  else return(getC(buf, r));
}

uint8_t MD_MAX72XX::getC(uint8_t buf, uint8_t c)
// c is in pixel coordinates and the return value must be in pixel coordinate order
{
  uint8_t mask = 1 << HW_COL(c);  // which column/row of bits is the column data
  uint8_t value = 0;        // assembles data to be returned to caller

  if (_hwDigRows) { PRINT("\ngetCol: (", buf); }
  else { PRINT("\ngetRow: (", buf); }
  PRINT(", ", c);
  PRINTS(") ");

  if ((buf > LAST_BUFFER) || (c >= COL_SIZE))
    return(0);

  PRINTX("mask 0x", mask);

  // for each digit data, pull out the column/row bit and place
  // it in value. The loop creates the data in pixel coordinate order as it goes.
  for (uint8_t i=0; i<ROW_SIZE; i++)
  {
      if (_matrix[buf].dig[HW_ROW(i)] & mask)
        bitSet(value, i);
  }

  PRINTX(" value 0x", value);

  return(value);
}

uint8_t MD_MAX72XX::getR(uint8_t buf, uint8_t r)
// r is in pixel coordinates for this buffer
// returned value is in pixel coordinates
{
  if (_hwDigRows) { PRINT("\ngetRow: (", buf); }
  else { PRINT("\ngetCol: (", buf); }
  PRINT(", ", r);
  PRINTS(") ");

  if ((buf > LAST_BUFFER) || (r >= ROW_SIZE))
    return(0);

  uint8_t value = _hwRevCols ? bitReverse(_matrix[buf].dig[HW_ROW(r)]) : _matrix[buf].dig[HW_ROW(r)];

  PRINTX("0x", value);

  return(value);
}

bool MD_MAX72XX::setColumn(uint8_t buf, uint8_t c, uint8_t value)
{
  if (_hwDigRows) return(setC(buf, c, value));
  else return(setR(buf, c, value));
}

bool MD_MAX72XX::setRow(uint8_t buf, uint8_t r, uint8_t value)
{
  if (_hwDigRows) return(setR(buf, r, value));
  else return(setC(buf, r, value));
}

bool MD_MAX72XX::setC(uint8_t buf, uint8_t c, uint8_t value)
// c and value are in pixel coordinate order
{
  if (_hwDigRows) { PRINT("\nsetCol: (", buf); }
  else { PRINT("\nsetRow: (", buf); }
  PRINT(", ", c);
  PRINTX(") 0x", value);

  if ((buf > LAST_BUFFER) || (c >= COL_SIZE))
    return(false);

  for (uint8_t i=0; i<ROW_SIZE; i++)
  {
      if (value & (1 << i))   // mask off next column/row value passed in and set it in the dig buffer
        bitSet(_matrix[buf].dig[HW_ROW(i)], HW_COL(c));
      else
        bitClear(_matrix[buf].dig[HW_ROW(i)], HW_COL(c));
  }
  _matrix[buf].changed = ALL_CHANGED;

  if (_updateEnabled) flushBuffer(buf);

  return(true);
}

bool MD_MAX72XX::setR(uint8_t buf, uint8_t r, uint8_t value)
// r and value are in pixel coordinates
{
  if (_hwDigRows) { PRINT("\nsetRow: (", buf); }
  else { PRINT("\nsetCol: (", buf); }
  PRINT(", ", r);
  PRINTX(") 0x", value);

  if ((buf > LAST_BUFFER) || (r >= ROW_SIZE))
    return(false);

  _matrix[buf].dig[HW_ROW(r)] = _hwRevCols ? bitReverse(value) : value;
  bitSet(_matrix[buf].changed, HW_ROW(r));

  if (_updateEnabled) flushBuffer(buf);

  return(true);
}

bool MD_MAX72XX::transform(uint8_t buf, transformType_t ttype)
{
  if (buf > LAST_BUFFER)
    return(false);

  if (!transformBuffer(buf, ttype))
    return(false);

  if (_updateEnabled) flushBuffer(buf);

  return(true);
}

bool MD_MAX72XX::transformBuffer(uint8_t buf, transformType_t ttype)
{
  uint8_t t[ROW_SIZE];

  switch (ttype)
  {
  //--------------
    case TSL: // Transform Shift Left one pixel element
      if (_hwDigRows)
      {
        for (uint8_t i=0; i<ROW_SIZE; i++)
        {
          if (_hwRevCols)
            _matrix[buf].dig[i] >>= 1;
          else
            _matrix[buf].dig[i] <<= 1;
        }
      }
      else
      {
        for (uint8_t i=ROW_SIZE; i>0; --i)
          _matrix[buf].dig[i] = _matrix[buf].dig[i-1];
      }
      break;

  //--------------
  case TSR: // Transform Shift Right one pixel element
      if (_hwDigRows)
      {
        for (uint8_t i=0; i<ROW_SIZE; i++)
        {
          if (_hwRevCols)
            _matrix[buf].dig[i] <<= 1;
          else
            _matrix[buf].dig[i] >>= 1;
        }
      }
      else
      {
        for (uint8_t i=0; i<ROW_SIZE-1; i++)
          _matrix[buf].dig[i] = _matrix[buf].dig[i+1];
      }
    break;

  //--------------
    case TSU: // Transform Shift Up one pixel element
    if (_wrapAround)  // save the first row or a zero row
      t[0] = getRow(buf, 0);
    else
      t[0] = 0;

    if (_hwDigRows)
    {
      for (uint8_t i=0; i<ROW_SIZE-1; i++)
        copyRow(buf, i+1, i);
    }
    else
    {
      for (int8_t i=ROW_SIZE-1; i>=0; i--)
        _matrix[buf].dig[i] <<= 1;
    }
    setRow(buf, ROW_SIZE-1, t[0]);
    break;

  //--------------
    case TSD: // Transform Shift Down one pixel element
    if (_wrapAround)  // save the last row or a zero row
      t[0] = getRow(buf, ROW_SIZE-1);
    else
      t[0] = 0;

    if (_hwDigRows)
    {
      for (uint8_t i=ROW_SIZE; i>0; --i)
        copyRow(buf, i-1, i);
    }
    else
    {
      for (uint8_t i=0; i<ROW_SIZE; i++)
        _matrix[buf].dig[i] >>= 1;
    }
    setRow(buf, 0, t[0]);
    break;

  //--------------
  case TFLR: // Transform Flip Left to Right
    if (_hwDigRows)
    {
      for (uint8_t i=0; i<ROW_SIZE; i++)
        _matrix[buf].dig[i] = bitReverse(_matrix[buf].dig[i]);
    }
    else  // really a TFUD
    {
      for (uint8_t i=0; i<ROW_SIZE/2; i++)
      {
        uint8_t	t = _matrix[buf].dig[i];
        _matrix[buf].dig[i] = _matrix[buf].dig[ROW_SIZE-i-1];
        _matrix[buf].dig[ROW_SIZE-i-1] = t;
      }
    }
    break;

  //--------------
  case TFUD: // Transform Flip Up to Down
    if (_hwDigRows)
    {
      for (uint8_t i=0; i<ROW_SIZE/2; i++)
      {
        uint8_t	t = _matrix[buf].dig[i];
        _matrix[buf].dig[i] = _matrix[buf].dig[ROW_SIZE-i-1];
        _matrix[buf].dig[ROW_SIZE-i-1] = t;
      }
    }
    else    // really a TFLR
    {
      for (uint8_t i=0; i<ROW_SIZE; i++)
        _matrix[buf].dig[i] = bitReverse(_matrix[buf].dig[i]);
    }
    break;

  //--------------
  case TRC: // Transform Rotate Clockwise
    for (uint8_t i=0; i<ROW_SIZE; i++)
      t[i] = getColumn(buf, COL_SIZE-1-i);

    for (uint8_t i=0; i<ROW_SIZE; i++)
      setRow(buf, i, t[i]);
    break;

  //--------------
  case TINV: // Transform INVert
    for (uint8_t i=0; i<ROW_SIZE; i++)
      _matrix[buf].dig[i] = ~_matrix[buf].dig[i];
    break;

    default:
      return(false);
  }

  _matrix[buf].changed = ALL_CHANGED;

  return(true);
}
