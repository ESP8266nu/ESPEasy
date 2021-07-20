/*
MD_MAX72xx - Library for using a MAX7219/7221 LED matrix controller

See header file for comments

This file contains methods that work with the fonts and characters defined in the library

Copyright (C) 2012-14 Marco Colli. All rights reserved.

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
 * \brief Implements font definition and methods
 */
#ifndef USE_NEW_FONT
#define USE_NEW_FONT 1
#endif

#if USE_LOCAL_FONT
// Local font handling functions if the option is enabled

void MD_MAX72XX::setFontInfoDefault(void)
// Set the defaults for the info block compatible with version 0 of the file
{
  _fontInfo.version = 0;
  _fontInfo.height = 8;
  _fontInfo.widthMax = 0;
  _fontInfo.firstASCII = 0;
  _fontInfo.lastASCII = 255;
  _fontInfo.dataOffset = 0;
}

void MD_MAX72XX::loadFontInfo(void)
{
  uint8_t c;
  uint16_t offset = 0;
  
  setFontInfoDefault();

  if (_fontData != nullptr)
  {
    PRINTS("\nLoading font info");
    // Read the first character. If this is not the file type indicator
    // then we have a version 0 file and the defaults are ok, otherwise 
    // read the font info from the data table. 
    c = pgm_read_byte(_fontData + offset++);
    if (c == FONT_FILE_INDICATOR)
    {
      c = pgm_read_byte(_fontData + offset++);  // read the version number
      switch (c)
      {
        case 2:
          _fontInfo.firstASCII = (pgm_read_byte(_fontData + offset++) << 8);
          _fontInfo.firstASCII += pgm_read_byte(_fontData + offset++);
          _fontInfo.lastASCII = (pgm_read_byte(_fontData + offset++) << 8);
          _fontInfo.lastASCII += pgm_read_byte(_fontData + offset++);
          _fontInfo.height = pgm_read_byte(_fontData + offset++);
          break;

        case 1:
          _fontInfo.firstASCII = pgm_read_byte(_fontData + offset++);
          _fontInfo.lastASCII  = pgm_read_byte(_fontData + offset++);
          _fontInfo.height     = pgm_read_byte(_fontData + offset++);
          break;
        
        case 0:
        default:
          // nothing to do, use the library defaults
          break;
      }
      _fontInfo.dataOffset = offset;
    }
    PRINT(" F: ", _fontInfo.firstASCII);
    PRINT(" L: ", _fontInfo.lastASCII);
    PRINT(" H: ", _fontInfo.height);

    // these always set
    _fontInfo.widthMax = getFontWidth();
  }
}

uint8_t MD_MAX72XX::getFontWidth(void)
{
  uint8_t   max = 0;
  uint8_t   charWidth;
  uint32_t  offset = _fontInfo.dataOffset;

  PRINTS("\nFinding max font width");
  if (_fontData != nullptr)
  {
    for (uint16_t i = _fontInfo.firstASCII; i <= _fontInfo.lastASCII; i++)
    {
      charWidth = pgm_read_byte(_fontData + offset);
      /*
      PRINT("\nASCII '", i);
      PRINT("' offset ", offset);
      PRINT("' width ", charWidth);
      */
      if (charWidth > max)
      {
        max = charWidth;
        PRINT(":", max);
      }
      offset += charWidth;  // skip character data
      offset++; // skip to size byte
    }
  }
  PRINT(" max ", max);

  return(max);
}

int32_t MD_MAX72XX::getFontCharOffset(uint16_t c)
{
  int32_t  offset = _fontInfo.dataOffset;

  PRINT("\nfontOffset ASCII ", c);

  if (c < _fontInfo.firstASCII || c > _fontInfo.lastASCII)
    offset = -1;
  else
  {
    for (uint16_t i=_fontInfo.firstASCII; i<c; i++)
    {
      PRINTS(".");
      offset += pgm_read_byte(_fontData+offset);
      offset++; // skip size byte we used above
    }

    PRINT(" searched offset ", offset);
  }
  return(offset);
}

bool MD_MAX72XX::setFont(fontType_t *f)
{
  if (f != _fontData) // we actually have a change to process
  {
    _fontData = (f == nullptr ? _sysfont : f);
    loadFontInfo();
  }

  return(true);
}

uint8_t MD_MAX72XX::getChar(uint16_t c, uint8_t size, uint8_t *buf)
{
  PRINT("\ngetChar: '", (char)c);
  PRINT("' ASC ", c);
  PRINT(" - bufsize ", size);

  if (buf == nullptr)
    return(0);

  int32_t offset = getFontCharOffset(c);
  if (offset == -1)
  {
    memset(buf, 0, size);
    size = 0;
  }
  else
  {
    size = min(size, pgm_read_byte(_fontData+offset));

    offset++; // skip the size byte

    for (uint8_t i=0; i<size; i++)
      *buf++ = pgm_read_byte(_fontData+offset+i);
  }
  
  return(size);
}

uint8_t MD_MAX72XX::setChar(uint16_t col, uint16_t c)
{
  PRINT("\nsetChar: '", c);
  PRINT("' column ", col);
  boolean b = _updateEnabled;
  uint8_t size;

  int32_t offset = getFontCharOffset(c);
  if (offset == -1)
    return(0);

  size = pgm_read_byte(_fontData+offset);

  offset++; // skip the size byte

  _updateEnabled = false;
  for (int8_t i=0; i<size; i++)
  {
    uint8_t colData = pgm_read_byte(_fontData+offset+i);
    setColumn(col--, colData);
  }
  _updateEnabled = b;

  if (_updateEnabled) flushBufferAll();

  return(size);
}
// Standard font - variable spacing
MD_MAX72XX::fontType_t PROGMEM _sysfont[] =
{
#if USE_NEW_FONT
  'F', 2, 0, 0, 0, 255, 8,
  0,		// 0 - 'Empty Cell'
  5, 62, 91, 79, 91, 62,		// 1 - 'Sad Smiley'
  5, 62, 107, 79, 107, 62,		// 2 - 'Happy Smiley'
  5, 28, 62, 124, 62, 28,		// 3 - 'Heart'
  5, 24, 60, 126, 60, 24,		// 4 - 'Diamond'
  5, 28, 87, 125, 87, 28,		// 5 - 'Clubs'
  5, 28, 94, 127, 94, 28,		// 6 - 'Spades'
  4, 0, 24, 60, 24,		// 7 - 'Bullet Point'
  5, 255, 231, 195, 231, 255,		// 8 - 'Rev Bullet Point'
  4, 0, 24, 36, 24,		// 9 - 'Hollow Bullet Point'
  5, 255, 231, 219, 231, 255,		// 10 - 'Rev Hollow BP'
  5, 48, 72, 58, 6, 14,		// 11 - 'Male'
  5, 38, 41, 121, 41, 38,		// 12 - 'Female'
  5, 64, 127, 5, 5, 7,		// 13 - 'Music Note 1'
  5, 64, 127, 5, 37, 63,		// 14 - 'Music Note 2'
  5, 90, 60, 231, 60, 90,		// 15 - 'Snowflake'
  5, 127, 62, 28, 28, 8,		// 16 - 'Right Pointer'
  5, 8, 28, 28, 62, 127,		// 17 - 'Left Pointer'
  5, 20, 34, 127, 34, 20,		// 18 - 'UpDown Arrows'
  5, 255, 255, 255, 255, 255,		// 19 - 'Full Block'
  5, 240, 240, 240, 240, 240,		// 20 - 'Half Block Bottom'
  3, 255, 255, 255,		// 21 - 'Half Block LHS'
  5, 0, 0, 0, 255, 255,		// 22 - 'Half Block RHS'
  5, 15, 15, 15, 15, 15,		// 23 - 'Half Block Top'
  5, 8, 4, 126, 4, 8,		// 24 - 'Up Arrow'
  5, 16, 32, 126, 32, 16,		// 25 - 'Down Arrow'
  5, 8, 8, 42, 28, 8,		// 26 - 'Right Arrow'
  5, 8, 28, 42, 8, 8,		// 27 - 'Left Arrow'
  5, 170, 0, 85, 0, 170,		// 28 - '30% shading'
  5, 170, 85, 170, 85, 170,		// 29 - '50% shading'
  5, 48, 56, 62, 56, 48,		// 30 - 'Up Pointer'
  5, 6, 14, 62, 14, 6,		// 31 - 'Down Pointer'
  2, 0, 0, 		// 32 - 'Space'
  1, 95,		// 33 - '!'
  3, 7, 0, 7,		// 34 - '""'
  5, 20, 127, 20, 127, 20,		// 35 - '#'
  5, 68, 74, 255, 74, 50,		// 36 - '$'
  5, 99, 19, 8, 100, 99,		// 37 - '%'
  5, 54, 73, 73, 54, 72,		// 38 - '&'
  1, 7,		// 39 - '''
  3, 62, 65, 65,		// 40 - '('
  3, 65, 65, 62,		// 41 - ')'
  5, 8, 42, 28, 42, 8,		// 42 - '*'
  5, 8, 8, 62, 8, 8,		// 43 - '+'
  2, 96, 224,		// 44 - ','
  4, 8, 8, 8, 8,		// 45 - '-'
  2, 96, 96,		// 46 - '.'
  5, 96, 16, 8, 4, 3,		// 47 - '/'
  5, 62, 81, 73, 69, 62,		// 48 - '0'
  3, 4, 2, 127,		// 49 - '1'
//  3, 66, 127, 64,		// 49 - serifed '1'
  5, 113, 73, 73, 73, 70,		// 50 - '2'
  5, 65, 73, 73, 73, 54,		// 51 - '3'
  5, 15, 8, 8, 8, 127,		// 52 - '4'
  5, 79, 73, 73, 73, 49,		// 53 - '5'
  5, 62, 73, 73, 73, 48,		// 54 - '6'
  5, 3, 1, 1, 1, 127,		// 55 - '7'
//  5, 1, 1, 121, 5, 3,		// 55 - european style '7'
  5, 54, 73, 73, 73, 54,		// 56 - '8'
  5, 6, 73, 73, 73, 62,		// 57 - '9'
  2, 108, 108,		// 58 - ':'
  2, 108, 236,		// 59 - ';'
  3, 8, 20, 34,		// 60 - '<'
  4, 20, 20, 20, 20,		// 61 - '='
  3, 34, 20, 8,		// 62 - '>'
  5, 1, 89, 9, 9, 6,		// 63 - '?'
  5, 62, 65, 93, 89, 78,		// 64 - '@'
  5, 126, 9, 9, 9, 126,		// 65 - 'A'
  5, 127, 73, 73, 73, 54,		// 66 - 'B'
  5, 62, 65, 65, 65, 65,		// 67 - 'C'
  5, 127, 65, 65, 65, 62,		// 68 - 'D'
  5, 127, 73, 73, 73, 65,		// 69 - 'E'
  5, 127, 9, 9, 9, 1,		// 70 - 'F'
  5, 62, 65, 65, 73, 121,		// 71 - 'G'
  5, 127, 8, 8, 8, 127,		// 72 - 'H'
  3, 65, 127, 65,		// 73 - 'I'
  5, 48, 65, 65, 65, 63,		// 74 - 'J'
  5, 127, 8, 20, 34, 65,		// 75 - 'K'
  5, 127, 64, 64, 64, 64,		// 76 - 'L'
  5, 127, 2, 12, 2, 127,		// 77 - 'M'
  5, 127, 4, 8, 16, 127,		// 78 - 'N'
  5, 62, 65, 65, 65, 62,		// 79 - 'O'
  5, 127, 9, 9, 9, 6,		// 80 - 'P'
  5, 62, 65, 65, 97, 126,		// 81 - 'Q'
  5, 127, 9, 25, 41, 70,		// 82 - 'R'
  5, 70, 73, 73, 73, 49,		// 83 - 'S'
  5, 1, 1, 127, 1, 1,		// 84 - 'T'
  5, 63, 64, 64, 64, 63,		// 85 - 'U'
  5, 31, 32, 64, 32, 31,		// 86 - 'V'
  5, 63, 64, 56, 64, 63,		// 87 - 'W'
  5, 99, 20, 8, 20, 99,		// 88 - 'X'
  5, 3, 4, 120, 4, 3,		// 89 - 'Y'
  5, 97, 81, 73, 69, 67,		// 90 - 'Z'
  3, 127, 65, 65,		// 91 - '['
  5, 3, 4, 8, 16, 96,		// 92 - '\'
  3, 65, 65, 127,		// 93 - ']'
  5, 4, 2, 1, 2, 4,		// 94 - '^'
  4, 128, 128, 128, 128,		// 95 - '_'
  3, 1, 2, 4,		// 96 - '`'
  4, 56, 68, 68, 124,		// 97 - 'a'
  4, 127, 68, 68, 56,		// 98 - 'b'
  4, 56, 68, 68, 68,		// 99 - 'c'
  4, 56, 68, 68, 127,		// 100 - 'd'
  4, 56, 84, 84, 88,		// 101 - 'e'
  4, 4, 126, 5, 1,		// 102 - 'f'
  4, 24, 164, 164, 124,		// 103 - 'g'
  4, 127, 4, 4, 120,		// 104 - 'h'
  1, 125,		// 105 - 'i'
  3, 132, 133, 124,		// 106 - 'j'
  4, 127, 16, 40, 68,		// 107 - 'k'
  1, 127,		// 108 - 'l'
  5, 124, 4, 120, 4, 120,		// 109 - 'm'
  4, 124, 4, 4, 120,		// 110 - 'n'
  4, 56, 68, 68, 56,		// 111 - 'o'
  4, 252, 36, 36, 24,		// 112 - 'p'
  4, 24, 36, 36, 252,		// 113 - 'q'
  4, 124, 4, 4, 8,		// 114 - 'r'
  4, 88, 84, 84, 52,		// 115 - 's'
  3, 4, 127, 4,		// 116 - 't'
  4, 60, 64, 64, 124,		// 117 - 'u'
  4, 28, 32, 64, 124,		// 118 - 'v'
  5, 60, 64, 48, 64, 60,		// 119 - 'w'
  4, 108, 16, 16, 108,		// 120 - 'x'
  4, 28, 160, 160, 124,		// 121 - 'y'
  4, 100, 84, 84, 76,		// 122 - 'z'
  4, 8, 54, 65, 65,		// 123 - '{'
  1, 127,		// 124 - '|'
  4, 65, 65, 54, 8,		// 125 - '}'
  4, 2, 1, 2, 1,		// 126 - '~'
  5, 127, 65, 65, 65, 127,		// 127 - 'Hollow Block'
  5, 62, 85, 85, 85, 65,		// 128 - 'Euro symbol'
  5, 56, 68, 68, 56, 68,		// 129 - 'Alpha'
  5, 124, 42, 42, 62, 20,		// 130 - 'Beta'
  5, 126, 2, 2, 6, 6,		// 131 - 'Gamma'
  5, 2, 126, 2, 126, 2,		// 132 - 'Pi'
  5, 99, 85, 73, 65, 99,		// 133 - 'Sigma'
  5, 56, 68, 68, 60, 4,		// 134 - 'Theta'
  5, 64, 126, 32, 30, 32,		// 135 - 'mu'
  5, 6, 2, 126, 2, 2,		// 136 - 'Tau'
  8, 99, 19, 8, 100, 99, 0, 96, 96,		// 137 - 'Percent 00'
  5, 42, 42, 42, 42, 42,		// 138 - '3 Bar Equals'
  3, 81, 74, 68,		// 139 - '>='
  3, 68, 74, 81,		// 140 - '<='
  5, 0, 0, 255, 1, 3,		// 141 - 'Top of Integral'
  3, 224, 128, 255,		// 142 - 'Bot of Integral'
  5, 54, 18, 54, 36, 54,		// 143 - 'Wavy ='
  3, 2, 5, 2,		// 144 - 'Degree'
  2, 24, 24,		// 145 - 'Math Product'
  2, 16, 16,		// 146 - 'Short Dash'
  5, 48, 64, 255, 1, 1,		// 147 - 'Square Root'
  4, 31, 1, 1, 30,		// 148 - 'Superscript n'
  0,		// 149
  0,		// 150  
  0,		// 151  
  0,		// 152
  9, 1, 1, 127, 1, 127, 2, 12, 2, 127,		// 153 - 'Trademark'
  0,		// 154
  0,		// 155
  0,		// 156
  0,		// 157
  0,		// 158
  0,		// 159
  1, 0,		// 160 - ' Non-breaking space'
  1, 125,		// 161 - '¡ Inverted Exclamation Mark'
  4, 24, 36, 126, 36,		// 162 - '¢ Cent sign'
  4, 68, 126, 69, 65,		// 163 - '£ Pound sign'
  5, 34, 28, 20, 28, 34,		// 164 - '¤ Currency sign'
  5, 1, 42, 124, 42, 1,		// 165 - '¥ Yen sign'
  1, 119,		// 166 - '¦ Broken bar'
  4, 78, 85, 85, 57,		// 167 - '§ Section sign'
  3, 2, 0, 2,		// 168 - '¨ Diaeresis (Umlaut)'
  7, 126, 129, 189, 165, 165, 129, 126,		// 169 - '© Copyright sign'
  3, 38, 41, 47,		// 170 - 'ª Feminine Ordinal Indicator'
  5, 8, 20, 42, 20, 34,		// 171 - '« Left-pointing double angle quotation mark'
  4, 8, 8, 8, 24,		// 172 - '¬ Not sign'
  3, 8, 8, 8,		// 173 - ' Soft hyphen'
  7, 126, 129, 189, 149, 169, 129, 126,		// 174 - '® Registered sign'
  5, 1, 1, 1, 1, 1,		// 175 - '¯ macron'
  3, 2, 5, 2,		// 176 - '° Degree symbol'
  3, 36, 46, 36,		// 177 - '± Plus-minus sign'
  3, 25, 21, 18,		// 178 - '² Superscript two'
  3, 21, 21, 10,		// 179 - '³ Superscript three'
  2, 2, 1,		// 180 - '´ Acute accent'
  4, 248, 32, 64, 120,		// 181 - 'µ Micro sign'
  5, 6, 9, 127, 1, 127,		// 182 - 'Pilcrow sign'
  2, 24, 24,		// 183 - '· Middle dot'
  4, 0, 128, 160, 192,		// 184 - '¸ Cedilla'
  3, 18, 31, 16,		// 185 - '¹ Superscript one'
  3, 38, 41, 38,		// 186 - 'º Masculine ordinal indicator'
  5, 34, 20, 42, 20, 8,		// 187 - '» Right-pointing double angle quotation mark'
  8, 64, 47, 16, 8, 4, 30, 17, 124,		// 188 - '¼ Vulgar fraction one quarter'
  8, 64, 47, 16, 8, 4, 98, 85, 76,		// 189 - '½ Vulgar fraction one half'
  9, 21, 85, 63, 16, 8, 4, 30, 17, 124,		// 190 - '¾ Vulgar fraction three quarters'
  5, 48, 72, 72, 77, 64,		// 191 - '¿ Inverted Question Mark'
  5, 120, 21, 22, 20, 120,		// 192 - 'À Latin Capital Letter A with grave'
  5, 120, 20, 22, 21, 120,		// 193 - 'Á Latin Capital letter A with acute'
  5, 122, 21, 20, 21, 122,		// 194 - 'Â Latin Capital letter A with circumflex'
  5, 120, 22, 21, 22, 121,		// 195 - 'Ã Latin Capital letter A with tilde'
  5, 120, 21, 20, 21, 120,		// 196 - 'Ä Latin Capital letter A with diaeresis'
  5, 120, 20, 21, 20, 120,		// 197 - 'Å Latin Capital letter A with ring above'
  7, 126, 9, 9, 127, 73, 73, 65,		// 198 - 'Æ Latin Capital letter Æ'
  5, 158, 161, 97, 33, 33,		// 199 - 'Ç Latin Capital letter C with cedilla'
  5, 124, 84, 85, 70, 68,		// 200 - 'È Latin Capital letter E with grave'
  5, 124, 84, 86, 69, 68,		// 201 - 'É Latin Capital letter E with acute'
  5, 126, 85, 84, 69, 70,		// 202 - 'Ê Latin Capital letter E with circumflex'
  5, 124, 85, 84, 69, 68,		// 203 - 'Ë Latin Capital letter E with diaeresis'
  3, 69, 126, 68,		// 204 - 'Ì Latin Capital letter I with grave'
  3, 68, 126, 69,		// 205 - 'Í Latin Capital letter I with acute'
  3, 70, 125, 70,		// 206 - 'Î Latin Capital letter I with circumflex'
  3, 69, 124, 69,		// 207 - 'Ï Latin Capital letter I with diaeresis'
  5, 8, 127, 73, 65, 62,		// 208 - 'Ð Latin Capital letter Eth'
  5, 124, 10, 17, 34, 125,		// 209 - 'Ñ Latin Capital letter N with tilde'
  5, 56, 68, 69, 70, 56,		// 210 - 'Ò Latin Capital letter O with grave'
  5, 56, 68, 70, 69, 56,		// 211 - 'Ó Latin Capital letter O with acute'
  5, 58, 69, 68, 69, 58,		// 212 - 'Ô Latin Capital letter O with circumflex'
  5, 56, 70, 69, 70, 57,		// 213 - 'Õ Latin Capital letter O with tilde'
  5, 56, 69, 68, 69, 56,		// 214 - 'Ö Latin Capital letter O with diaeresis'
  5, 34, 20, 8, 20, 34,		// 215 - '× Multiplication sign'
  5, 124, 98, 90, 70, 62,		// 216 - 'Ø Latin Capital letter O with stroke'
  5, 60, 64, 65, 66, 60,		// 217 - 'Ù Latin Capital letter U with grave'
  5, 60, 64, 66, 65, 60,		// 218 - 'Ú Latin Capital letter U with acute'
  5, 60, 66, 65, 66, 60,		// 219 - 'Û Latin Capital Letter U with circumflex'
  5, 60, 65, 64, 65, 60,		// 220 - 'Ü Latin Capital Letter U with diaeresis'
  5, 2, 4, 122, 5, 2,		// 221 - 'Ý Latin Capital Letter Y with acute'
  4, 63, 18, 18, 12,		// 222 - 'Þ Latin Capital Letter Thorn'
  5, 126, 73, 73, 78, 48,		// 223 - 'ß Latin Small Letter sharp S'
  4, 56, 69, 70, 124,		// 224 - 'à Latin Small Letter A with grave'
  4, 56, 68, 70, 125,		// 225 - 'á Latin Small Letter A with acute'
  4, 56, 70, 69, 126,		// 226 - 'â Latin Small Letter A with circumflex'
  4, 58, 69, 70, 125,		// 227 - 'ã Latin Small Letter A with tilde'
  4, 56, 69, 68, 125,		// 228 - 'ä Latin Small Letter A with diaeresis'
  4, 48, 74, 77, 122,		// 229 - 'å Latin Small Letter A with ring above'
  7, 32, 84, 84, 56, 84, 84, 88,		// 230 - 'æ Latin Small Letter Æ'
  4, 156, 162, 98, 34,		// 231 - 'ç Latin Small Letter C with cedilla'
  4, 56, 85, 86, 88,		// 232 - 'è Latin Small Letter E with grave'
  4, 56, 84, 86, 89,		// 233 - 'é Latin Small Letter E with acute'
  4, 56, 86, 85, 90,		// 234 - 'ê Latin Small Letter E with circumflex'
  4, 56, 85, 84, 89,		// 235 - 'ë Latin Small Letter E with diaeresis'
  2, 1, 122,		// 236 - 'ì Latin Small Letter I with grave'
  2, 122, 1,		// 237 - 'í Latin Small Letter I with acute'
  3, 2, 121, 2,		// 238 - 'î Latin Small Letter I with circumflex'
  3, 2, 120, 2,		// 239 - 'ï Latin Small Letter I with diaeresis'
  4, 48, 75, 75, 60,		// 240 - 'ð Latin Small Letter Eth'
  4, 122, 9, 10, 113,		// 241 - 'ñ Latin Small Letter N with tilde'
  4, 48, 73, 74, 48,		// 242 - 'ò Latin Small Letter O with grave'
  4, 48, 72, 74, 49,		// 243 - 'ó Latin Small Letter O with acute'
  4, 48, 74, 73, 50,		// 244 - 'ô Latin Small Letter O with circumflex'
  4, 50, 73, 74, 49,		// 245 - 'õ Latin Small Letter O with tilde'
  4, 57, 68, 68, 57,		// 246 - 'ö Latin Small Letter O with diaeresis'
  5, 8, 8, 42, 8, 8,		// 247 - '÷ Division sign'
  4, 56, 84, 76, 56,		// 248 - 'ø Latin Small Letter O with stroke'
  4, 56, 65, 66, 120,		// 249 - 'ù Latin Small Letter U with grave'
  4, 56, 64, 66, 121,		// 250 - 'ú Latin Small Letter U with acute'
  4, 56, 66, 65, 122,		// 251 - 'û Latin Small Letter U with circumflex'
  4, 58, 64, 64, 122,		// 252 - 'ü Latin Small Letter U with diaeresis'
  4, 24, 160, 162, 121,		// 253 - 'ý Latin Small Letter Y with acute'
  4, 252, 40, 40, 16,		// 254 - 'þ Latin Small Letter Thorn'
  4, 26, 160, 160, 122,		// 255 - 'ÿ Latin Small Letter Y with diaeresis'
#else
  'F', 1, 0, 255, 8,
  0,  // 0 - 'Empty Cell'
  5, 0x3e, 0x5b, 0x4f, 0x5b, 0x3e,  // 1 - 'Sad Smiley'
  5, 0x3e, 0x6b, 0x4f, 0x6b, 0x3e,  // 2 - 'Happy Smiley'
  5, 0x1c, 0x3e, 0x7c, 0x3e, 0x1c,  // 3 - 'Heart'
  5, 0x18, 0x3c, 0x7e, 0x3c, 0x18,  // 4 - 'Diamond'
  5, 0x1c, 0x57, 0x7d, 0x57, 0x1c,  // 5 - 'Clubs'
  5, 0x1c, 0x5e, 0x7f, 0x5e, 0x1c,  // 6 - 'Spades'
  4, 0x00, 0x18, 0x3c, 0x18,  // 7 - 'Bullet Point'
  5, 0xff, 0xe7, 0xc3, 0xe7, 0xff,  // 8 - 'Rev Bullet Point'
  4, 0x00, 0x18, 0x24, 0x18,  // 9 - 'Hollow Bullet Point'
  5, 0xff, 0xe7, 0xdb, 0xe7, 0xff,  // 10 - 'Rev Hollow BP'
  5, 0x30, 0x48, 0x3a, 0x06, 0x0e,  // 11 - 'Male'
  5, 0x26, 0x29, 0x79, 0x29, 0x26,  // 12 - 'Female'
  5, 0x40, 0x7f, 0x05, 0x05, 0x07,  // 13 - 'Music Note 1'
  5, 0x40, 0x7f, 0x05, 0x25, 0x3f,  // 14 - 'Music Note 2'
  5, 0x5a, 0x3c, 0xe7, 0x3c, 0x5a,  // 15 - 'Snowflake'
  5, 0x7f, 0x3e, 0x1c, 0x1c, 0x08,  // 16 - 'Right Pointer'
  5, 0x08, 0x1c, 0x1c, 0x3e, 0x7f,  // 17 - 'Left Pointer'
  5, 0x14, 0x22, 0x7f, 0x22, 0x14,  // 18 - 'UpDown Arrows'
  5, 0x5f, 0x5f, 0x00, 0x5f, 0x5f,  // 19 - 'Double Exclamation'
  5, 0x06, 0x09, 0x7f, 0x01, 0x7f,  // 20 - 'Paragraph Mark'
  4, 0x66, 0x89, 0x95, 0x6a,  // 21 - 'Section Mark'
  5, 0x60, 0x60, 0x60, 0x60, 0x60,  // 22 - 'Double Underline'
  5, 0x94, 0xa2, 0xff, 0xa2, 0x94,  // 23 - 'UpDown Underlined'
  5, 0x08, 0x04, 0x7e, 0x04, 0x08,  // 24 - 'Up Arrow'
  5, 0x10, 0x20, 0x7e, 0x20, 0x10,  // 25 - 'Down Arrow'
  5, 0x08, 0x08, 0x2a, 0x1c, 0x08,  // 26 - 'Right Arrow'
  5, 0x08, 0x1c, 0x2a, 0x08, 0x08,  // 27 - 'Left Arrow'
  5, 0x1e, 0x10, 0x10, 0x10, 0x10,  // 28 - 'Angled'
  5, 0x0c, 0x1e, 0x0c, 0x1e, 0x0c,  // 29 - 'Squashed #'
  5, 0x30, 0x38, 0x3e, 0x38, 0x30,  // 30 - 'Up Pointer'
  5, 0x06, 0x0e, 0x3e, 0x0e, 0x06,  // 31 - 'Down Pointer'
  2, 0x00, 0x00,  // 32 - 'Space'
  1, 0x5f,  // 33 - '!'
  3, 0x07, 0x00, 0x07,  // 34 - '"'
  5, 0x14, 0x7f, 0x14, 0x7f, 0x14,  // 35 - '#'
  5, 0x24, 0x2a, 0x7f, 0x2a, 0x12,  // 36 - '$'
  5, 0x23, 0x13, 0x08, 0x64, 0x62,  // 37 - '%'
  5, 0x36, 0x49, 0x56, 0x20, 0x50,  // 38 - '&'
  3, 0x08, 0x07, 0x03,  // 39 - '''
  3, 0x1c, 0x22, 0x41,  // 40 - '('
  3, 0x41, 0x22, 0x1c,  // 41 - ')'
  5, 0x2a, 0x1c, 0x7f, 0x1c, 0x2a,  // 42 - '*'
  5, 0x08, 0x08, 0x3e, 0x08, 0x08,  // 43 - '+'
  3, 0x80, 0x70, 0x30,  // 44 - ','
  5, 0x08, 0x08, 0x08, 0x08, 0x08,  // 45 - '-'
  2, 0x60, 0x60,  // 46 - '.'
  5, 0x20, 0x10, 0x08, 0x04, 0x02,  // 47 - '/'
  5, 0x3e, 0x51, 0x49, 0x45, 0x3e,  // 48 - '0'
  3, 0x42, 0x7f, 0x40,  // 49 - '1'
  5, 0x72, 0x49, 0x49, 0x49, 0x46,  // 50 - '2'
  5, 0x21, 0x41, 0x49, 0x4d, 0x33,  // 51 - '3'
  5, 0x18, 0x14, 0x12, 0x7f, 0x10,  // 52 - '4'
  5, 0x27, 0x45, 0x45, 0x45, 0x39,  // 53 - '5'
  5, 0x3c, 0x4a, 0x49, 0x49, 0x31,  // 54 - '6'
  5, 0x41, 0x21, 0x11, 0x09, 0x07,  // 55 - '7'
  5, 0x36, 0x49, 0x49, 0x49, 0x36,  // 56 - '8'
  5, 0x46, 0x49, 0x49, 0x29, 0x1e,  // 57 - '9'
  1, 0x14,  // 58 - ':'
  2, 0x80, 0x68,  // 59 - ';'
  4, 0x08, 0x14, 0x22, 0x41,  // 60 - '<'
  5, 0x14, 0x14, 0x14, 0x14, 0x14,  // 61 - '='
  4, 0x41, 0x22, 0x14, 0x08,  // 62 - '>'
  5, 0x02, 0x01, 0x59, 0x09, 0x06,  // 63 - '?'
  5, 0x3e, 0x41, 0x5d, 0x59, 0x4e,  // 64 - '@'
  5, 0x7c, 0x12, 0x11, 0x12, 0x7c,  // 65 - 'A'
  5, 0x7f, 0x49, 0x49, 0x49, 0x36,  // 66 - 'B'
  5, 0x3e, 0x41, 0x41, 0x41, 0x22,  // 67 - 'C'
  5, 0x7f, 0x41, 0x41, 0x41, 0x3e,  // 68 - 'D'
  5, 0x7f, 0x49, 0x49, 0x49, 0x41,  // 69 - 'E'
  5, 0x7f, 0x09, 0x09, 0x09, 0x01,  // 70 - 'F'
  5, 0x3e, 0x41, 0x41, 0x51, 0x73,  // 71 - 'G'
  5, 0x7f, 0x08, 0x08, 0x08, 0x7f,  // 72 - 'H'
  3, 0x41, 0x7f, 0x41,  // 73 - 'I'
  5, 0x20, 0x40, 0x41, 0x3f, 0x01,  // 74 - 'J'
  5, 0x7f, 0x08, 0x14, 0x22, 0x41,  // 75 - 'K'
  5, 0x7f, 0x40, 0x40, 0x40, 0x40,  // 76 - 'L'
  5, 0x7f, 0x02, 0x1c, 0x02, 0x7f,  // 77 - 'M'
  5, 0x7f, 0x04, 0x08, 0x10, 0x7f,  // 78 - 'N'
  5, 0x3e, 0x41, 0x41, 0x41, 0x3e,  // 79 - 'O'
  5, 0x7f, 0x09, 0x09, 0x09, 0x06,  // 80 - 'P'
  5, 0x3e, 0x41, 0x51, 0x21, 0x5e,  // 81 - 'Q'
  5, 0x7f, 0x09, 0x19, 0x29, 0x46,  // 82 - 'R'
  5, 0x26, 0x49, 0x49, 0x49, 0x32,  // 83 - 'S'
  5, 0x03, 0x01, 0x7f, 0x01, 0x03,  // 84 - 'T'
  5, 0x3f, 0x40, 0x40, 0x40, 0x3f,  // 85 - 'U'
  5, 0x1f, 0x20, 0x40, 0x20, 0x1f,  // 86 - 'V'
  5, 0x3f, 0x40, 0x38, 0x40, 0x3f,  // 87 - 'W'
  5, 0x63, 0x14, 0x08, 0x14, 0x63,  // 88 - 'X'
  5, 0x03, 0x04, 0x78, 0x04, 0x03,  // 89 - 'Y'
  5, 0x61, 0x59, 0x49, 0x4d, 0x43,  // 90 - 'Z'
  3, 0x7f, 0x41, 0x41,  // 91 - '['
  5, 0x02, 0x04, 0x08, 0x10, 0x20,  // 92 - '\'
  3, 0x41, 0x41, 0x7f,  // 93 - ']'
  5, 0x04, 0x02, 0x01, 0x02, 0x04,  // 94 - '^'
  5, 0x40, 0x40, 0x40, 0x40, 0x40,  // 95 - '_'
  3, 0x03, 0x07, 0x08,  // 96 - '`'
  5, 0x20, 0x54, 0x54, 0x78, 0x40,  // 97 - 'a'
  5, 0x7f, 0x28, 0x44, 0x44, 0x38,  // 98 - 'b'
  5, 0x38, 0x44, 0x44, 0x44, 0x28,  // 99 - 'c'
  5, 0x38, 0x44, 0x44, 0x28, 0x7f,  // 100 - 'd'
  5, 0x38, 0x54, 0x54, 0x54, 0x18,  // 101 - 'e'
  4, 0x08, 0x7e, 0x09, 0x02,  // 102 - 'f'
  5, 0x18, 0xa4, 0xa4, 0x9c, 0x78,  // 103 - 'g'
  5, 0x7f, 0x08, 0x04, 0x04, 0x78,  // 104 - 'h'
  3, 0x44, 0x7d, 0x40,  // 105 - 'i'
  4, 0x40, 0x80, 0x80, 0x7a,  // 106 - 'j'
  4, 0x7f, 0x10, 0x28, 0x44,  // 107 - 'k'
  3, 0x41, 0x7f, 0x40,  // 108 - 'l'
  5, 0x7c, 0x04, 0x78, 0x04, 0x78,  // 109 - 'm'
  5, 0x7c, 0x08, 0x04, 0x04, 0x78,  // 110 - 'n'
  5, 0x38, 0x44, 0x44, 0x44, 0x38,  // 111 - 'o'
  5, 0xfc, 0x18, 0x24, 0x24, 0x18,  // 112 - 'p'
  5, 0x18, 0x24, 0x24, 0x18, 0xfc,  // 113 - 'q'
  5, 0x7c, 0x08, 0x04, 0x04, 0x08,  // 114 - 'r'
  5, 0x48, 0x54, 0x54, 0x54, 0x24,  // 115 - 's'
  4, 0x04, 0x3f, 0x44, 0x24,  // 116 - 't'
  5, 0x3c, 0x40, 0x40, 0x20, 0x7c,  // 117 - 'u'
  5, 0x1c, 0x20, 0x40, 0x20, 0x1c,  // 118 - 'v'
  5, 0x3c, 0x40, 0x30, 0x40, 0x3c,  // 119 - 'w'
  5, 0x44, 0x28, 0x10, 0x28, 0x44,  // 120 - 'x'
  5, 0x4c, 0x90, 0x90, 0x90, 0x7c,  // 121 - 'y'
  5, 0x44, 0x64, 0x54, 0x4c, 0x44,  // 122 - 'z'
  3, 0x08, 0x36, 0x41,  // 123 - '{'
  1, 0x77,  // 124 - '|'
  3, 0x41, 0x36, 0x08,  // 125 - '}'
  5, 0x02, 0x01, 0x02, 0x04, 0x02,  // 126 - '~'
  5, 0x3c, 0x26, 0x23, 0x26, 0x3c,  // 127 - 'Hollow Up Arrow'
  5, 0x1e, 0xa1, 0xa1, 0x61, 0x12,  // 128 - 'C sedilla'
  5, 0x38, 0x42, 0x40, 0x22, 0x78,  // 129 - 'u umlaut'
  5, 0x38, 0x54, 0x54, 0x55, 0x59,  // 130 - 'e acute'
  5, 0x21, 0x55, 0x55, 0x79, 0x41,  // 131 - 'a accent'
  5, 0x21, 0x54, 0x54, 0x78, 0x41,  // 132 - 'a umlaut'
  5, 0x21, 0x55, 0x54, 0x78, 0x40,  // 133 - 'a grave'
  5, 0x20, 0x54, 0x55, 0x79, 0x40,  // 134 - 'a acute'
  5, 0x18, 0x3c, 0xa4, 0xe4, 0x24,  // 135 - 'c sedilla'
  5, 0x39, 0x55, 0x55, 0x55, 0x59,  // 136 - 'e accent'
  5, 0x38, 0x55, 0x54, 0x55, 0x58,  // 137 - 'e umlaut'
  5, 0x39, 0x55, 0x54, 0x54, 0x58,  // 138 - 'e grave'
  3, 0x45, 0x7c, 0x41,  // 139 - 'i umlaut'
  4, 0x02, 0x45, 0x7d, 0x42,  // 140 - 'i hat'
  4, 0x01, 0x45, 0x7c, 0x40,  // 141 - 'i grave'
  5, 0xf0, 0x29, 0x24, 0x29, 0xf0,  // 142 - 'A umlaut'
  5, 0xf0, 0x28, 0x25, 0x28, 0xf0,  // 143 - 'A dot'
  4, 0x7c, 0x54, 0x55, 0x45,  // 144 - 'E grave'
  7, 0x20, 0x54, 0x54, 0x7c, 0x54, 0x54, 0x08,  // 145 - 'ae'
  6, 0x7c, 0x0a, 0x09, 0x7f, 0x49, 0x49,  // 146 - 'AE'
  5, 0x32, 0x49, 0x49, 0x49, 0x32,  // 147 - 'o hat'
  5, 0x30, 0x4a, 0x48, 0x4a, 0x30,  // 148 - 'o umlaut'
  5, 0x32, 0x4a, 0x48, 0x48, 0x30,  // 149 - 'o grave'
  5, 0x3a, 0x41, 0x41, 0x21, 0x7a,  // 150 - 'u hat'
  5, 0x3a, 0x42, 0x40, 0x20, 0x78,  // 151 - 'u grave'
  4, 0x9d, 0xa0, 0xa0, 0x7d,  // 152 - 'y umlaut'
  5, 0x38, 0x45, 0x44, 0x45, 0x38,  // 153 - 'O umlaut'
  5, 0x3c, 0x41, 0x40, 0x41, 0x3c,  // 154 - 'U umlaut'
  5, 0x3c, 0x24, 0xff, 0x24, 0x24,  // 155 - 'Cents'
  5, 0x48, 0x7e, 0x49, 0x43, 0x66,  // 156 - 'Pounds'
  5, 0x2b, 0x2f, 0xfc, 0x2f, 0x2b,  // 157 - 'Yen'
  5, 0xff, 0x09, 0x29, 0xf6, 0x20,  // 158 - 'R +'
  5, 0xc0, 0x88, 0x7e, 0x09, 0x03,  // 159 - 'f notation'
  5, 0x20, 0x54, 0x54, 0x79, 0x41,  // 160 - 'a acute'
  3, 0x44, 0x7d, 0x41,  // 161 - 'i acute'
  5, 0x30, 0x48, 0x48, 0x4a, 0x32,  // 162 - 'o acute'
  5, 0x38, 0x40, 0x40, 0x22, 0x7a,  // 163 - 'u acute'
  4, 0x7a, 0x0a, 0x0a, 0x72,  // 164 - 'n accent'
  5, 0x7d, 0x0d, 0x19, 0x31, 0x7d,  // 165 - 'N accent'
  5, 0x26, 0x29, 0x29, 0x2f, 0x28,  // 166
  5, 0x26, 0x29, 0x29, 0x29, 0x26,  // 167
  5, 0x30, 0x48, 0x4d, 0x40, 0x20,  // 168 - 'Inverted ?'
  5, 0x38, 0x08, 0x08, 0x08, 0x08,  // 169 - 'LH top corner'
  5, 0x08, 0x08, 0x08, 0x08, 0x38,  // 170 - 'RH top corner'
  5, 0x2f, 0x10, 0xc8, 0xac, 0xba,  // 171 - '1/2'
  5, 0x2f, 0x10, 0x28, 0x34, 0xfa,  // 172 - '1/4'
  1, 0x7b,  // 173 - '| split'
  5, 0x08, 0x14, 0x2a, 0x14, 0x22,  // 174 - '<<'
  5, 0x22, 0x14, 0x2a, 0x14, 0x08,  // 175 - '>>'
  5, 0xaa, 0x00, 0x55, 0x00, 0xaa,  // 176 - '30% shading'
  5, 0xaa, 0x55, 0xaa, 0x55, 0xaa,  // 177 - '50% shading'
  5, 0x00, 0x00, 0x00, 0x00, 0xff,  // 178 - 'Right side'
  5, 0x10, 0x10, 0x10, 0x10, 0xff,  // 179 - 'Right T'
  5, 0x14, 0x14, 0x14, 0x14, 0xff,  // 180 - 'Right T double H'
  5, 0x10, 0x10, 0xff, 0x00, 0xff,  // 181 - 'Right T double V'
  5, 0x10, 0x10, 0xf0, 0x10, 0xf0,  // 182 - 'Top Right double V'
  5, 0x14, 0x14, 0x14, 0x14, 0xfc,  // 183 - 'Top Right double H'
  5, 0x14, 0x14, 0xf7, 0x00, 0xff,  // 184 - 'Right T double all'
  5, 0x00, 0x00, 0xff, 0x00, 0xff,  // 185 - 'Right side double'
  5, 0x14, 0x14, 0xf4, 0x04, 0xfc,  // 186 - 'Top Right double'
  5, 0x14, 0x14, 0x17, 0x10, 0x1f,  // 187 - 'Bot Right double'
  5, 0x10, 0x10, 0x1f, 0x10, 0x1f,  // 188 - 'Bot Right double V'
  5, 0x14, 0x14, 0x14, 0x14, 0x1f,  // 189 - 'Bot Right double H'
  5, 0x10, 0x10, 0x10, 0x10, 0xf0,  // 190 - 'Top Right'
  5, 0x00, 0x00, 0x00, 0x1f, 0x10,  // 191 - 'Bot Left'
  5, 0x10, 0x10, 0x10, 0x1f, 0x10,  // 192 - 'Bot T'
  5, 0x10, 0x10, 0x10, 0xf0, 0x10,  // 193 - 'Top T'
  5, 0x00, 0x00, 0x00, 0xff, 0x10,  // 194 - 'Left T'
  5, 0x10, 0x10, 0x10, 0x10, 0x10,  // 195 - 'Top side'
  5, 0x10, 0x10, 0x10, 0xff, 0x10,  // 196 - 'Center +'
  5, 0x00, 0x00, 0x00, 0xff, 0x14,  // 197 - 'Left side double H'
  5, 0x00, 0x00, 0xff, 0x00, 0xff,  // 198 - 'Left side double'
  5, 0x00, 0x00, 0x1f, 0x10, 0x17,  // 199 - 'Bot Left double V'
  5, 0x00, 0x00, 0xfc, 0x04, 0xf4,  // 200 - 'Top Left double V'
  5, 0x14, 0x14, 0x17, 0x10, 0x17,  // 201 - 'Bot T double'
  5, 0x14, 0x14, 0xf4, 0x04, 0xf4,  // 202 - 'Top T double'
  5, 0x00, 0x00, 0xff, 0x00, 0xf7,  // 203 - 'Left Side double spl'
  5, 0x14, 0x14, 0x14, 0x14, 0x14,  // 204 - 'Center double'
  5, 0x14, 0x14, 0xf7, 0x00, 0xf7,  // 205 - 'Center + double'
  5, 0x14, 0x14, 0x14, 0x17, 0x14,  // 206 - 'Bot T double H'
  5, 0x10, 0x10, 0x1f, 0x10, 0x1f,  // 207 - 'Bot Right double V'
  5, 0x14, 0x14, 0x14, 0xf4, 0x14,  // 208 - 'Top T double H'
  5, 0x10, 0x10, 0xf0, 0x10, 0xf0,  // 209 - 'Top Right double V'
  5, 0x00, 0x00, 0x1f, 0x10, 0x1f,  // 210 - 'Bot Left double V'
  5, 0x00, 0x00, 0x00, 0x1f, 0x14,  // 211 - 'Bot Right double H'
  5, 0x00, 0x00, 0x00, 0xfc, 0x14,  // 212 - 'Top Right double H'
  5, 0x00, 0x00, 0xf0, 0x10, 0xf0,  // 213 - 'Top Right double V'
  5, 0x10, 0x10, 0xff, 0x10, 0xff,  // 214 - 'Center + double V'
  5, 0x14, 0x14, 0x14, 0xff, 0x14,  // 215 - 'Center + double H'
  5, 0x10, 0x10, 0x10, 0x10, 0x1f,  // 216 - 'Bot Right'
  5, 0x00, 0x00, 0x00, 0xf0, 0x10,  // 217 - 'Top Left'
  5, 0xff, 0xff, 0xff, 0xff, 0xff,  // 218 - 'Full Block'
  5, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,  // 219 - 'Half Block Bottom'
  3, 0xff, 0xff, 0xff,  // 220 - 'Half Block LHS'
  5, 0x00, 0x00, 0x00, 0xff, 0xff,  // 221 - 'Half Block RHS'
  5, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,  // 222 - 'Half Block Top'
  5, 0x38, 0x44, 0x44, 0x38, 0x44,  // 223 - 'Alpha'
  5, 0x7c, 0x2a, 0x2a, 0x3e, 0x14,  // 224 - 'Beta'
  5, 0x7e, 0x02, 0x02, 0x06, 0x06,  // 225 - 'Gamma'
  5, 0x02, 0x7e, 0x02, 0x7e, 0x02,  // 226 - 'Pi'
  5, 0x63, 0x55, 0x49, 0x41, 0x63,  // 227 - 'Sigma'
  5, 0x38, 0x44, 0x44, 0x3c, 0x04,  // 228 - 'Theta'
  5, 0x40, 0x7e, 0x20, 0x1e, 0x20,  // 229 - 'mu'
  5, 0x06, 0x02, 0x7e, 0x02, 0x02,  // 230 - 'Tau'
  5, 0x99, 0xa5, 0xe7, 0xa5, 0x99,  // 231
  5, 0x1c, 0x2a, 0x49, 0x2a, 0x1c,  // 232
  5, 0x4c, 0x72, 0x01, 0x72, 0x4c,  // 233
  5, 0x30, 0x4a, 0x4d, 0x4d, 0x30,  // 234
  5, 0x30, 0x48, 0x78, 0x48, 0x30,  // 235
  5, 0xbc, 0x62, 0x5a, 0x46, 0x3d,  // 236 - 'Zero Slashed'
  4, 0x3e, 0x49, 0x49, 0x49,  // 237
  5, 0x7e, 0x01, 0x01, 0x01, 0x7e,  // 238
  5, 0x2a, 0x2a, 0x2a, 0x2a, 0x2a,  // 239 - '3 Bar Equals'
  5, 0x44, 0x44, 0x5f, 0x44, 0x44,  // 240 - '+/-'
  5, 0x40, 0x51, 0x4a, 0x44, 0x40,  // 241 - '>='
  5, 0x40, 0x44, 0x4a, 0x51, 0x40,  // 242 - '<='
  5, 0x00, 0x00, 0xff, 0x01, 0x03,  // 243 - 'Top of Integral'
  3, 0xe0, 0x80, 0xff,  // 244 - 'Bot of Integral'
  5, 0x08, 0x08, 0x6b, 0x6b, 0x08,  // 245 - 'Divide'
  5, 0x36, 0x12, 0x36, 0x24, 0x36,  // 246 - 'Wavy ='
  5, 0x06, 0x0f, 0x09, 0x0f, 0x06,  // 247 - 'Degree'
  4, 0x00, 0x00, 0x18, 0x18,  // 248 - 'Math Product'
  4, 0x00, 0x00, 0x10, 0x10,  // 249 - 'Short Dash'
  5, 0x30, 0x40, 0xff, 0x01, 0x01,  // 250 - 'Square Root'
  5, 0x00, 0x1f, 0x01, 0x01, 0x1e,  // 251 - 'Superscript n'
  5, 0x00, 0x19, 0x1d, 0x17, 0x12,  // 252 - 'Superscript 2'
  5, 0x00, 0x3c, 0x3c, 0x3c, 0x3c,  // 253 - 'Centered Square'
  5, 0xff, 0x81, 0x81, 0x81, 0xff,  // 254 - 'Full Frame'
  5, 0xff, 0xff, 0xff, 0xff, 0xff,  // 255 - 'Full Block'
#endif
};

#endif //USE_LOCAL_FONT

