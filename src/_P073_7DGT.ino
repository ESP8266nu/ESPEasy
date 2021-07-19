#include "_Plugin_Helper.h"
#ifdef USES_P073

// #######################################################################################################
// ###################   Plugin 073 - 7-segment display plugin TM1637/MAX7219       ######################
// #######################################################################################################
//
// Chips/displays supported:
//  0 - TM1637     -- 2 pins - 4 digits and colon in the middle (XX:XX)
//  1 - TM1637     -- 2 pins - 4 digits and dot on each digit (X.X.X.X.)
//  2 - TM1637     -- 2 pins - 6 digits and dot on each digit (X.X.X.X.X.X.)
//  3 - MAX7219/21 -- 3 pins - 8 digits and dot on each digit (X.X.X.X.X.X.X.X.)
//
// Plugin can be setup as:
//  - Manual        -- display is manually updated sending commands
//                     "7dn,<number>"        (number can be negative or positive, even with decimal)
//                     "7dt,<temperature>"   (temperature can be negative or positive and containing decimals)
//                     "7ddt,<temperature>,<temperature>"   (Dual temperatures on Max7219 (8 digits) only, temperature can be negative or positive and containing decimals)
//                     "7dst,<hh>,<mm>,<ss>" (show manual time -not current-, no checks done on numbers validity!)
//                     "7dsd,<dd>,<mm>,<yy>" (show manual date -not current-, no checks done on numbers validity!)
//                     "7dtext,<text>"       (show free text - supported chars 0-9,a-z,A-Z," ","-","=","_","/","^") Depending on Font used
//                     "7dfont,<font>"       (select the used font: 0/7DGT/Default = default, 1/Siekoo = Siekoo, 2/Siekoo_Upper = Siekoo with uppercase CHNORUX, 3/dSEG7 = dSEG7)
//                                           Siekoo: https://www.fakoo.de/siekoo (uppercase CHNORUX is a local extension)
//                                           dSEG7 : https://www.keshikan.net/fonts-e.html
//                     "7dbin,[uint8_t],..."    (show data binary formatted, bits clock-wise from left to right, dot, top, right 2x, bottom, left 2x, center), scroll-enabled
//  - Clock-Blink     -- display is automatically updated with current time and blinking dot/lines
//  - Clock-NoBlink   -- display is automatically updated with current time and steady dot/lines
//  - Clock12-Blink   -- display is automatically updated with current time (12h clock) and blinking dot/lines
//  - Clock12-NoBlink -- display is automatically updated with current time (12h clock) and steady dot/lines
//  - Date            -- display is automatically updated with current date
//
// Generic commands:
//  - "7don"      -- turn ON the display
//  - "7doff"     -- turn OFF the display
//  - "7db,<0-15> -- set brightness to specific value between 0 and 15
//
// History
// 2021-02-13, tonhuisman: Fixed self-introduced bug of conversion from MAX7219 to TM1637 bit mapping, removed now unused TM1637 character maps, moved some logging to DEBUG level
// 2021-01-30, tonhuisman: Added font support for 7Dgt (default), Siekoo, Siekoo with uppercase CHNORUX, dSEG7 fonts. Default/7Dgt comes with these special characters: " -^=/_
//                         Siekoo comes _without_ AOU with umlauts and Eszett characters, has many extra special characters "%@.,;:+*#!?'\"<>\\()|", and optional uppercase "CHNORUX",
//                         "^" displays as degree symbol and "|" displays overscsore (top-line only).
//                         'Merged' fontdata for TM1637 by converting the data for MAX7219 (bits 0-6 are swapped around), to save a little space and maintanance burden.
//                         Added 7dfont,<font> command for changing the font dynamically runtime. NB: The numbers digits are equal for all fonts!
//                         Added Scroll Text option for scrolling texts longer then the display is wide
//                         Added 7dbin,<uint8_t>[,...] for displaying binary formatted data bits clock-wise from left to right, dot, top, right 2x, bottom, left 2x, center), scroll-enabled
// 2021-01-10, tonhuisman: Added optional . as dot display (7dtext)
//                         Added 7ddt,<temp1>,<temp2> dual temperature display
//                         Added optional removal of degree symbol on temperature display
//                         Added optional right-shift of 7dt,<temp> on MAX7219 display (is normally shifted to left by 1 digit, so last digit is blank)

#define PLUGIN_073
#define PLUGIN_ID_073           73
#define PLUGIN_NAME_073         "Display - 7-segment display"

#define P073_TM1637_4DGTCOLON   0
#define P073_TM1637_4DGTDOTS    1
#define P073_TM1637_6DGT        2
#define P073_MAX7219_8DGT       3

#define P073_DISP_MANUAL        0
#define P073_DISP_CLOCK24BLNK   1
#define P073_DISP_CLOCK24       2
#define P073_DISP_CLOCK12BLNK   3
#define P073_DISP_CLOCK12       4
#define P073_DISP_DATE          5

#define P073_OPTION_PERIOD      0 // Period as dot
#define P073_OPTION_HIDEDEGREE  1 // Hide degree symbol for temperatures
#define P073_OPTION_RIGHTALIGN  2 // Align 7dt output right on MAX7219 display
#define P073_OPTION_SCROLLTEXT  3 // Scroll text > 8 characters
#define P073_OPTION_SCROLLFULL  4 // Scroll text from the right in, starting with a blank display

#define P073_7DDT_COMMAND         // Enable 7ddt by default
#define P073_EXTRA_FONTS          // Enable extra fonts
#define P073_SCROLL_TEXT          // Enable scrolling of 7dtext by default
#define P073_7DBIN_COMMAND        // Enable input of binary data via 7dbin,uint8_t,... command

#ifndef PLUGIN_SET_TESTING
// #define P073_DEBUG                // Leave out some debugging on demnand, activates extra log info in the debug
#else
#undef P073_7DDT_COMMAND          // Optionally activate if .bin file space is really problematic, to remove the 7ddt command
#undef P073_EXTRA_FONTS           // Optionally activate if .bin file space is really problematic, to remove the font selection and 7dfont command
#undef P073_SCROLL_TEXT           // Optionally activate if .bin file space is really problematic, to remove the scrolling text feature
#undef P073_7DBIN_COMMAND         // Optionally activate if .bin file space is really problematic, to remove the 7dbin command
#endif

struct P073_data_struct : public PluginTaskData_base {
  P073_data_struct()
    : dotpos(-1), pin1(-1), pin2(-1), pin3(-1), displayModel(0), output(0),
    brightness(0), timesep(false), shift(false), periods(false), hideDegree(false),
    rightAlignTempMAX7219(false), fontset(0)
    #ifdef P073_7DBIN_COMMAND
    , binaryData(false)
    #endif // P073_7DBIN_COMMAND
    #ifdef P073_SCROLL_TEXT
    , txtScrolling(false), scrollCount(0), scrollPos(0), scrollFull(false)
    , _scrollSpeed(0)
    #endif // P073_SCROLL_TEXT
     {
    ClearBuffer();
  }

  void FillBufferWithTime(boolean sevendgt_now, uint8_t sevendgt_hours,
                          uint8_t sevendgt_minutes, uint8_t sevendgt_seconds,
                          boolean flag12h) {
    ClearBuffer();

    if (sevendgt_now) {
      sevendgt_hours   = node_time.hour();
      sevendgt_minutes = node_time.minute();
      sevendgt_seconds = node_time.second();
    }

    if (flag12h && (sevendgt_hours > 12)) {
      sevendgt_hours -= 12; // if flag 12h is TRUE and h>12 adjust subtracting 12
    }

    if (flag12h && (sevendgt_hours == 0)) {
      sevendgt_hours = 12; // if flag 12h is TRUE and h=0  adjust to h=12
    }
    showbuffer[0] = static_cast<uint8_t>(sevendgt_hours / 10);
    showbuffer[1] = sevendgt_hours % 10;
    showbuffer[2] = static_cast<uint8_t>(sevendgt_minutes / 10);
    showbuffer[3] = sevendgt_minutes % 10;
    showbuffer[4] = static_cast<uint8_t>(sevendgt_seconds / 10);
    showbuffer[5] = sevendgt_seconds % 10;
  }

  void FillBufferWithDate(boolean sevendgt_now, uint8_t sevendgt_day,
                          uint8_t sevendgt_month, int sevendgt_year) {
    ClearBuffer();
    int sevendgt_year0 = sevendgt_year;

    if (sevendgt_now) {
      sevendgt_day   = node_time.day();
      sevendgt_month = node_time.month();
      sevendgt_year0 = node_time.year();
    } else {
      if (sevendgt_year0 < 100) {
        sevendgt_year0 += 2000;
      }
    }
    uint8_t sevendgt_year1 = static_cast<uint8_t>(sevendgt_year0 / 100);
    uint8_t sevendgt_year2 = static_cast<uint8_t>(sevendgt_year0 % 100);

    showbuffer[0] = static_cast<uint8_t>(sevendgt_day / 10);
    showbuffer[1] = sevendgt_day % 10;
    showbuffer[2] = static_cast<uint8_t>(sevendgt_month / 10);
    showbuffer[3] = sevendgt_month % 10;
    showbuffer[4] = static_cast<uint8_t>(sevendgt_year1 / 10);
    showbuffer[5] = sevendgt_year1 % 10;
    showbuffer[6] = static_cast<uint8_t>(sevendgt_year2 / 10);
    showbuffer[7] = sevendgt_year2 % 10;
  }

  void FillBufferWithNumber(const String& number) {
    ClearBuffer();
    uint8_t p073_numlenght = number.length();
    uint8_t p073_index     = 7;
    dotpos = -1; // -1 means no dot to display

    for (int i = p073_numlenght - 1; i >= 0 && p073_index >= 0; --i) {
      char p073_tmpchar = number.charAt(i);

      if (p073_tmpchar == '.') { // dot
        dotpos = p073_index;
      } else {
        showbuffer[p073_index] = P073_mapCharToFontPosition(p073_tmpchar, fontset);
        p073_index--;
      }
    }
  }

  void FillBufferWithTemp(long temperature) {
    ClearBuffer();
    char   p073_digit[8];
    bool   between10and0 = (temperature < 10 && temperature >= 0);      // To have a zero prefix (0.x and -0.x) display between 0.9 and -0.9 degrees,
    bool   between0andMinus10 = (temperature < 0 && temperature > -10); // as all display types use 1 digit for temperatures between 10.0 and -10.0
    String format;
    if (hideDegree) {
      format = (between10and0 ? F("      %02d") : (between0andMinus10 ? F("     %03d") : F("%8d")));
    } else {
      format = (between10and0 ? F("     %02d") : (between0andMinus10 ? F("    %03d") : F("%7d")));
    }
    sprintf_P(p073_digit, format.c_str(), static_cast<int>(temperature));
    int p073_numlenght = strlen(p073_digit);

    for (int i = 0; i < p073_numlenght; i++) {
      showbuffer[i] = P073_mapCharToFontPosition(p073_digit[i], fontset);
    }
    if (!hideDegree) {
      showbuffer[7] = 12; // degree "°"
    }
  }

#ifdef P073_7DDT_COMMAND
/**
 * FillBufferWithDualTemp()
 * leftTemperature or rightTempareature < -100.0 then shows dashes
 */
  void FillBufferWithDualTemp(long leftTemperature, bool leftWithDecimal, long rightTemperature, bool rightWithDecimal) {
    ClearBuffer();
    char   p073_digit[8];
    String format;
    bool   leftBetween10and0       = (leftWithDecimal && leftTemperature < 10 && leftTemperature >= 0);    // To have a zero prefix (0.x and -0.x) display between 0.9 and -0.9 degrees,
    bool   leftBetween0andMinus10  = (leftWithDecimal && leftTemperature < 0 && leftTemperature > -10);    // as all display types use 1 digit for temperatures between 10.0 and -10.0
    if (hideDegree) {
           format                  = (leftBetween10and0 ? F("  %02d") : (leftBetween0andMinus10 ? F(" %03d") : leftTemperature < -1000 ? F("----") : F("%4d"))); // Include a space for compensation of the degree sym,bol
    } else {
           format                  = (leftBetween10and0 ? F(" %02d ") : (leftBetween0andMinus10 ? F("%03d ") : leftTemperature < -100 ? F("----") : F("%3d "))); // Include a space for compensation of the degree sym,bol
    }
    bool   rightBetween10and0      = (rightWithDecimal && rightTemperature < 10 && rightTemperature >= 0); // To have a zero prefix (0.x and -0.x) display between 0.9 and -0.9 degrees,
    bool   rightBetween0andMinus10 = (rightWithDecimal && rightTemperature < 0 && rightTemperature > -10); // as all display types use 1 digit for temperatures between 10.0 and -10.0
    if (hideDegree) {
           format                 += (rightBetween10and0 ? F("  %02d") : (rightBetween0andMinus10 ? F(" %03d") : rightTemperature < -1000 ? F("----") : F("%4d")));
    } else {
           format                 += (rightBetween10and0 ? F(" %02d") : (rightBetween0andMinus10 ? F("%03d") : rightTemperature < -100 ? F("----") : F("%3d")));
    }
    sprintf_P(p073_digit, format.c_str(), static_cast<int>(leftTemperature), static_cast<int>(rightTemperature));
    int p073_numlenght = strlen(p073_digit);

    for (int i = 0; i < p073_numlenght; i++) {
      showbuffer[i] = P073_mapCharToFontPosition(p073_digit[i], fontset);
    }
    if (!hideDegree) {
      if (leftTemperature  > -100.0) showbuffer[3] = 12; // degree "°"
      if (rightTemperature > -100.0) showbuffer[7] = 12; // degree "°"
    }
// addLog(LOG_LEVEL_INFO, String(F("7dgt format")) + format);
  }
#endif

  void FillBufferWithString(const String& textToShow, bool useBinaryData = false) {
    #ifdef P073_7DBIN_COMMAND
    binaryData = useBinaryData;
    #endif // P073_7DBIN_COMMAND
    ClearBuffer();
    int p073_txtlength = textToShow.length();

    int p = 0;
    for (int i = 0; i < p073_txtlength && p <= 8; i++) { // p <= 8 to allow a period after last digit
      if (periods
          && textToShow.charAt(i) == '.'
          #ifdef P073_7DBIN_COMMAND
          && !binaryData
          #endif // P073_7DBIN_COMMAND
         ) { // If setting periods true
        if (p == 0) { // Text starts with a period, becomes a space with a dot
          showperiods[p] = true;
          p++;
        } else {
        // if (p > 0) {
          showperiods[p - 1] = true; // The period displays as a dot on the previous digit!
        }
        if (i > 0 && textToShow.charAt(i - 1) == '.') { // Handle consecutive periods
          p++;
          showperiods[p - 1] = true; // The period displays as a dot on the previous digit!
        }
      } else if (p < 8) {
        #ifdef P073_7DBIN_COMMAND
        showbuffer[p] = useBinaryData ? textToShow.charAt(i) : P073_mapCharToFontPosition(textToShow.charAt(i), fontset);
        #else // P073_7DBIN_COMMAND
        showbuffer[p] = P073_mapCharToFontPosition(textToShow.charAt(i), fontset);
        #endif // P073_7DBIN_COMMAND
        p++;
      }
    }
#ifdef P073_DEBUG
    LogBufferContent(F("7dtext"));
#endif
  }

#ifdef P073_SCROLL_TEXT
  uint8_t getBufferLength(uint8_t displayModel) {
    uint8_t bufLen = 0;
    switch(displayModel) {
      case P073_TM1637_4DGTCOLON:
      case P073_TM1637_4DGTDOTS:
        bufLen = 4;
        break;
      case P073_TM1637_6DGT:
        bufLen = 6;
        break;
      case P073_MAX7219_8DGT:
        bufLen = 8;
        break;
    }
    return bufLen;
  }

  int getEffectiveTextLength(const String& text) {
    int textLength = text.length();
    int p = 0;
    for (int i = 0; i < textLength; i++) {
      if (periods && text.charAt(i) == '.') { // If setting periods true
        if (p == 0) { // Text starts with a period, becomes a space with a dot
          p++;
        }
        if (i > 0 && text.charAt(i - 1) == '.') { // Handle consecutive periods
          p++;
        }
      } else {
        p++;
      }
    }
    return p;
  }

  void NextScroll() {
    if (txtScrolling && _textToScroll.length() > 0) {
      if (scrollCount > 0 && scrollCount < 0xFFFF) scrollCount--;
      if (scrollCount == 0) {
        scrollCount = 0xFFFF; // Max value to avoid interference when scrolling long texts
        int bufToFill = getBufferLength(displayModel);
        ClearBuffer();
        int p073_txtlength = _textToScroll.length();

        int p = 0;
        for (int i = scrollPos; i < p073_txtlength && p <= bufToFill; i++) { // p <= bufToFill to allow a period after last digit
          if (periods
              && _textToScroll.charAt(i) == '.'
              #ifdef P073_7DBIN_COMMAND
              && !binaryData
              #endif // P073_7DBIN_COMMAND
             ) { // If setting periods true
            if (p == 0) { // Text starts with a period, becomes a space with a dot
              showperiods[p] = true;
              p++;
            } else {
              showperiods[p - 1] = true; // The period displays as a dot on the previous digit!
            }
            if (i > scrollPos && _textToScroll.charAt(i - 1) == '.') { // Handle consecutive periods
              showperiods[p - 1] = true; // The period displays as a dot on the previous digit!
              p++;
            }
          } else if (p < bufToFill) {
            #ifdef P073_7DBIN_COMMAND
            showbuffer[p] = binaryData ? _textToScroll.charAt(i) : P073_mapCharToFontPosition(_textToScroll.charAt(i), fontset);
            #else // P073_7DBIN_COMMAND
            showbuffer[p] = P073_mapCharToFontPosition(_textToScroll.charAt(i), fontset);
            #endif // P073_7DBIN_COMMAND
            p++;
          }
        }
        scrollPos++;
        if (scrollPos > _textToScroll.length() - bufToFill) scrollPos = 0; // Restart when all text displayed
        scrollCount = _scrollSpeed; // Restart countdown
        #ifdef P073_DEBUG
        LogBufferContent(F("nextScroll"));
        #endif // P073_DEBUG
      }
    }
  }

  void setTextToScroll(const String& text) {
    _textToScroll = EMPTY_STRING;
    if (text.length() > 0) {
      int bufToFill = getBufferLength(displayModel);
      _textToScroll.reserve(text.length() + bufToFill + (scrollFull ? bufToFill : 0));
      for (int i = 0; scrollFull && i < bufToFill; i++) { // Scroll text in from the right, so start with all spaces
        _textToScroll += 
        #ifdef P073_7DBIN_COMMAND
        binaryData ? (char)0x00 :
        #endif // P073_7DBIN_COMMAND
        ' ';
      }
      _textToScroll += text;
      for (int i = 0; i < bufToFill; i++) { // Scroll text off completely before restarting
        _textToScroll += 
        #ifdef P073_7DBIN_COMMAND
        binaryData ? (char)0x00 :
        #endif // P073_7DBIN_COMMAND
        ' ';
      }
    }
    scrollCount = _scrollSpeed;
    scrollPos   = 0;
    #ifdef P073_7DBIN_COMMAND
    binaryData  = false;
    #endif // P073_7DBIN_COMMAND
  }

  void setScrollSpeed(uint8_t speed) {
    _scrollSpeed = speed;
    scrollCount  = _scrollSpeed;
    scrollPos    = 0;
  }
#endif // P073_SCROLL_TEXT

#ifdef P073_7DBIN_COMMAND
  void setBinaryData(const String& data) {
    binaryData    = true;
    #ifdef P073_SCROLL_TEXT
    setTextToScroll(data);
    binaryData    = true; // is reset in setTextToScroll
    scrollCount   = _scrollSpeed;
    scrollPos     = 0;
    #else // P073_SCROLL_TEXT
    _textToScroll = data;
    #endif // P073_SCROLL_TEXT
  }
#endif // P073_7DBIN_COMMAND

#ifdef P073_DEBUG
void LogBufferContent(String prefix) {
  String log;
  log.reserve(48);
  log = prefix;
  log += F(" buffer: periods: ");
  log += periods ? 't' : 'f';
  log += ' ';
  for (int i = 0; i < 8; i++) {
    if (i > 0) log += ',';
    log += F("0x");
    log += String(showbuffer[i], HEX);
    log += ',';
    log += showperiods[i] ? F(".") : F("");
  }
  addLog(LOG_LEVEL_INFO, log);
}
#endif // P073_DEBUG

  // in case of error show all dashes
  void FillBufferWithDash() {
    memset(showbuffer, 11, sizeof(showbuffer));
  }

  void ClearBuffer() {
    memset(showbuffer, 
    #ifdef P073_7DBIN_COMMAND
    binaryData ? 0 :
    #endif // P073_7DBIN_COMMAND
    10, sizeof(showbuffer));
    for (int i = 0; i < 8; i++) {
      showperiods[i] = false;
    }
  }

  int      dotpos = 0;
  uint8_t  showbuffer[8] = {0};
  bool     showperiods[8];
  uint8_t  spidata[2] = {0};
  uint8_t  pin1, pin2, pin3;
  uint8_t  displayModel;
  uint8_t  output;
  uint8_t  brightness;
  bool     timesep;
  bool     shift;
  bool     periods;
  bool     hideDegree;
  bool     rightAlignTempMAX7219;
  uint8_t  fontset;
#ifdef P073_7DBIN_COMMAND
  bool     binaryData;
#endif // P073_7DBIN_COMMAND
#ifdef P073_SCROLL_TEXT
  bool     txtScrolling;
  uint16_t scrollCount;
  uint16_t scrollPos;
  bool     scrollFull;
private:
  uint16_t _scrollSpeed;
#endif // P073_SCROLL_TEXT
#if defined(P073_SCROLL_TEXT) || defined(P073_7DBIN_COMMAND)
  String   _textToScroll;
#endif // P073_SCROLL_TEXT
};

#define TM1637_POWER_ON B10001000
#define TM1637_POWER_OFF B10000000
#define TM1637_CLOCKDELAY 40
#define TM1637_4DIGIT 4
#define TM1637_6DIGIT 2

// each char table is specific for each display and maps all numbers/symbols
// needed:
//   - pos 0-9   - Numbers from 0 to 9
//   - pos 10    - Space " "
//   - pos 11    - minus symbol "-"
//   - pos 12    - degree symbol "°"
//   - pos 13    - equal "="
//   - pos 14    - triple lines "/"
//   - pos 15    - underscore "_"
//   - pos 16-41 - Letters from A to Z
static const uint8_t DefaultCharTable[42] PROGMEM = {
  B01111110, B00110000, B01101101, B01111001, B00110011, B01011011,
  B01011111, B01110000, B01111111, B01111011, B00000000, B00000001,
  B01100011, B00001001, B01001001, B00001000, B01110111, B00011111,
  B01001110, B00111101, B01001111, B01000111, B01011110, B00110111,
  B00000110, B00111100, B01010111, B00001110, B01010100, B01110110,
  B01111110, B01100111, B01101011, B01100110, B01011011, B00001111,
  B00111110, B00111110, B00101010, B00110111, B00111011, B01101101 };

#ifdef P073_EXTRA_FONTS
// Siekoo alphabet https://www.fakoo.de/siekoo
// as the 'over score' character isn't normally available, the pipe "|" is used for that, and for degree the "^"" is used
// specials:
//   - pos 0-9   - Numbers from 0 to 9
//   - pos 10    - Space " "
//   - pos 11    - minus symbol "-"
//   - pos 12    - degree symbol "°" (specially handled "^" into a degree)
//   - pos 13    - equal "="
//   - pos 14    - slash "/"                            
//   - pos 15    - underscore "_"
//   - pos 16-40 - Special characters not handled yet -- MAX7219 -- -- TM1637 --
//   - pos 16    - percent "%"                          B00010010
//   - pos 17    - at "@"                               B01110100
//   - pos 18    - period "."                           B00000100
//   - pos 10    - comma ","                            B00011000
//   - pos 20    - semicolon ";"                        B00101000
//   - pos 21    - colon ":"                            B01001000
//   - pos 22    - plus "+"                             B00110001
//   - pos 23    - asterisk "*"                         B01001001
//   - pos 24    - hash "#"                             B00110110
//   - pos 25    - exclamation mark "!"                 B01101011
//   - pos 26    - question mark "?"                    B01101001
//   - pos 27    - single quote "'"                     B00000010
//   - pos 28    - double quote '"'                     B00100010
//   - pos 29    - left sharp bracket "<"               B01000010
//   - pos 30    - right sharp bracket ">"              B01100000
//   - pos 31    - backslash "\"                        B00010011
//   - pos 32    - left round bracket "("               B01001110
//   - pos 33    - right round bracket ")"              B01111000
//   - pos 34    - overscore "|" (the top-most line)    B01000000
//   - pos 35    - uppercase C "C" (optionally enabled) B01001110
//   - pos 36    - uppercase H "H"                      B00110111
//   - pos 37    - uppercase N "N"                      B01110110
//   - pos 38    - uppercase O "O"                      B01111110
//   - pos 39    - uppercase R "R"                      B01100110
//   - pos 40    - uppercase U "U"                      B00111110
//   - pos 41    - uppercase X "X"                      B00110111
//   - pos 42-67 - Letters from A to Z Siekoo style
static const uint8_t SiekooCharTable[68] PROGMEM = {
  B01111110, B00110000, B01101101, B01111001, B00110011, B01011011,
  B01011111, B01110000, B01111111, B01111011, B00000000, B00000001,
  B01100011, B00001001, B00100101, B00001000, B00010010, B01110100,
  B00000100, B00011000, B00101000, B01001000, B00110001, B01001001,
  B00110110, B01101011, B01101001, B00000010, B00100010, B01000010,
  B01100000, B00010011, B01001110, B01111000, B01000000, B01001110,
  B00110111, B01110110, B01111110, B01100110, B00111110, B00110111,
  B01111101, B00011111, B00001101, B00111101, B01001111, B01000111, /* ABCDEF */
  B01011110, B00010111, B01000100, B01011000, B01010111, B00001110,
  B01010101, B00010101, B00011101, B01100111, B01110011, B00000101,
  B01011010, B00001111, B00011100, B00101010, B00101011, B00010100,
  B00111011, B01101100 };

// dSEG7 https://www.keshikan.net/fonts-e.html
// specials:
//   - pos 0-9   - Numbers from 0 to 9
//   - pos 10    - Space " "
//   - pos 11    - minus symbol "-"
//   - pos 12    - degree symbol "°" (specially handled "^" into a degree)
//   - pos 13    - equal "="
//   - pos 14    - slash "/"                            
//   - pos 15    - underscore "_"
//   - pos 16-41 - Letters from A to Z dSEG7 style
static const uint8_t Dseg7CharTable[42] PROGMEM = {
  B01111110, B00110000, B01101101, B01111001, B00110011, B01011011,
  B01011111, B01110000, B01111111, B01111011, B00000000, B00000001,
  B01100011, B00001001, B01001001, B00001000, B01110111, B00011111, /* AB */
  B00001101, B00111101, B01001111, B01000111, B01011110, B00010111,
  B00010000, B00111100, B01010111, B00001110, B01110110, B00010101,
  B00011101, B01100111, B01110011, B00000101, B00011011, B00001111,
  B00011100, B00111110, B00111111, B00110111, B00111011, B01101100 };

#endif // P073_EXTRA_FONTS

uint8_t P073_mapCharToFontPosition(char character, uint8_t fontset) {
  uint8_t position = 10;
#ifdef P073_EXTRA_FONTS
  String specialChars = F(" -^=/_%@.,;:+*#!?'\"<>\\()|");
  String chnorux      = F("CHNORUX");

  switch(fontset) {
    case 1: // Siekoo
    case 2: // Siekoo with uppercase 'CHNORUX'
      if (fontset == 2 && chnorux.indexOf(character) > -1) {
        position = chnorux.indexOf(character) + 35;
      } else if ((character >= '0') && (character <= '9')) {
        position = character - '0';
      } else if ((character >= 'A') && (character <= 'Z')) {
        position = character - 'A' + 42;
      } else if ((character >= 'a') && (character <= 'z')) {
        position = character - 'a' + 42;
      } else {
        uint8_t idx = specialChars.indexOf(character);
        if (idx > -1) {
          position = idx + 10;
        }
      }
      break;
    case 3: // dSEG7 (same table size as 7Dgt)
    default: // Original fontset (7Dgt)
#endif // P073_EXTRA_FONTS
      if ((character >= '0') && (character <= '9')) {
        position = character - '0';
      } else if ((character >= 'A') && (character <= 'Z')) {
        position = character - 'A' + 16;
      } else if ((character >= 'a') && (character <= 'z')) {
        position = character - 'a' + 16;
      } else {
        switch (character) {
          case ' ': position = 10; break;
          case '-': position = 11; break;
          case '^': position = 12; break; // degree
          case '=': position = 13; break;
          case '/': position = 14; break;
          case '_': position = 15; break;
        }
      }
#ifdef P073_EXTRA_FONTS
  }
#endif // P073_EXTRA_FONTS
  return position;
}

uint8_t P073_mapMAX7219FontToTM1673Font(uint8_t character) {
  uint8_t newCharacter = character & 0x80; // Keep dot-bit if passed in
  for (int b = 0; b < 7; b++) {
    if (character & (0x01 << b)) {
      newCharacter |= (0x40 >> b);
    }
  }
  return newCharacter;
}

boolean Plugin_073(uint8_t function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_073;
      Device[deviceCount].Type               = DEVICE_TYPE_TRIPLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 0;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].TimerOptional      = false;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_073);
      break;
    }

    #ifdef P073_SCROLL_TEXT
    case PLUGIN_SET_DEFAULTS: {
      PCONFIG(3) = 10;  // Default 10 * 0.1 sec scroll speed
      break;
    }
    #endif // P073_SCROLL_TEXT

    case PLUGIN_WEBFORM_LOAD: {
      addFormNote(F("TM1637:  1st=CLK-Pin, 2nd=DIO-Pin"));
      addFormNote(F("MAX7219: 1st=DIN-Pin, 2nd=CLK-Pin, 3rd=CS-Pin"));
      {
        const __FlashStringHelper * displtype[4] = { F("TM1637 - 4 digit (colon)"),
                                F("TM1637 - 4 digit (dots)"),
                                F("TM1637 - 6 digit"),
                                F("MAX7219 - 8 digit") };
        addFormSelector(F("Display Type"), F("plugin_073_displtype"), 4, displtype, NULL, PCONFIG(0));
      }
      {
        const __FlashStringHelper * displout[6] = { F("Manual"),
                              F("Clock 24h - Blink"),
                              F("Clock 24h - No Blink"),
                              F("Clock 12h - Blink"),
                              F("Clock 12h - No Blink"),
                              F("Date") };
        addFormSelector(F("Display Output"), F("plugin_073_displout"), 6, displout, NULL, PCONFIG(1));
      }

      addFormNumericBox(F("Brightness"), F("plugin_073_brightness"), PCONFIG(2), 0, 15);

      #ifdef P073_EXTRA_FONTS
      {
        const __FlashStringHelper * fontset[4] = { F("Default"),
                              F("Siekoo"),
                              F("Siekoo with uppercase 'CHNORUX'"),
                              F("dSEG7") };
        addFormSelector(F("Font set"), F("plugin_073_fontset"), 4, fontset, NULL, PCONFIG(4));
        addFormNote(F("Check documentation for examples of the font sets."));
      }
      #endif // P073_EXTRA_FONTS

      addFormSubHeader(F("Options"));

      bool bPeriodsAsDots = bitRead(PCONFIG_LONG(0), P073_OPTION_PERIOD);
      addFormCheckBox(F("Text show periods as dot"), F("plugin_073_periods"), bPeriodsAsDots);

      bool bHideDegree = bitRead(PCONFIG_LONG(0), P073_OPTION_HIDEDEGREE);
      addFormCheckBox(F("Hide &deg; for Temperatures"), F("plugin_073_hide_degree"), bHideDegree);
      #ifdef P073_7DDT_COMMAND
      addFormNote(F("Commands 7dt,&lt;temp&gt; and 7ddt,&lt;temp1&gt;,&lt;temp2&gt;"));
      #else
      addFormNote(F("Command 7dt,&lt;temp&gt;"));
      #endif // P073_7DDT_COMMAND

      #ifdef P073_SCROLL_TEXT
      bool bScrollText = bitRead(PCONFIG_LONG(0), P073_OPTION_SCROLLTEXT);
      bool bScrollFull = bitRead(PCONFIG_LONG(0), P073_OPTION_SCROLLFULL);
      addFormCheckBox(F("Scroll text &gt; display width"), F("plugin_073_scroll_text"), bScrollText);
      addFormCheckBox(F("Scroll text in from right"), F("plugin_073_scroll_full"), bScrollFull);
      if (PCONFIG(3) == 0) PCONFIG(3) = 10;
      addFormNumericBox(F("Scroll speed (0.1 sec/step)"), F("plugin_073_scrollspeed"), PCONFIG(3), 1, 600);
      addUnit(F("1..600 = 0.1..60 sec/step"));
      #endif // P073_SCROLL_TEXT

      addFormSubHeader(F("Options for MAX7219 - 8 digit"));

      bool bRightAlign = bitRead(PCONFIG_LONG(0), P073_OPTION_RIGHTALIGN);
      addFormCheckBox(F("Right-align Temperature (7dt)"), F("plugin_073_temp_rightalign"), bRightAlign);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      PCONFIG(0) = getFormItemInt(F("plugin_073_displtype"));
      PCONFIG(1) = getFormItemInt(F("plugin_073_displout"));
      PCONFIG(2) = getFormItemInt(F("plugin_073_brightness"));
      uint32_t lSettings = 0;
      bitWrite(lSettings, P073_OPTION_PERIOD,     isFormItemChecked(F("plugin_073_periods")));
      bitWrite(lSettings, P073_OPTION_HIDEDEGREE, isFormItemChecked(F("plugin_073_hide_degree")));
      bitWrite(lSettings, P073_OPTION_RIGHTALIGN, isFormItemChecked(F("plugin_073_temp_rightalign")));
      #ifdef P073_SCROLL_TEXT
      bitWrite(lSettings, P073_OPTION_SCROLLTEXT, isFormItemChecked(F("plugin_073_scroll_text")));
      bitWrite(lSettings, P073_OPTION_SCROLLFULL, isFormItemChecked(F("plugin_073_scroll_full")));
      PCONFIG(3) = getFormItemInt(F("plugin_073_scrollspeed"));
      #endif // P073_SCROLL_TEXT
      #ifdef P073_EXTRA_FONTS
      PCONFIG(4) = getFormItemInt(F("plugin_073_fontset"));
      #endif // P073_EXTRA_FONTS
      PCONFIG_LONG(0) = lSettings;

      P073_data_struct *P073_data =
        static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P073_data) {
        P073_data->pin1         = CONFIG_PIN1;
        P073_data->pin2         = CONFIG_PIN2;
        P073_data->pin3         = CONFIG_PIN3;
        P073_data->displayModel = PCONFIG(0);
        P073_data->output       = PCONFIG(1);
        P073_data->brightness   = PCONFIG(2);
        P073_data->periods      = bitRead(PCONFIG_LONG(0), P073_OPTION_PERIOD);
        P073_data->hideDegree   = bitRead(PCONFIG_LONG(0), P073_OPTION_HIDEDEGREE);
        #ifdef P073_SCROLL_TEXT
        P073_data->txtScrolling = bitRead(PCONFIG_LONG(0), P073_OPTION_SCROLLTEXT);
        P073_data->scrollFull   = bitRead(PCONFIG_LONG(0), P073_OPTION_SCROLLFULL);
        P073_data->setScrollSpeed(PCONFIG(3));
        #endif // P073_SCROLL_TEXT
        P073_data->rightAlignTempMAX7219 = bitRead(PCONFIG_LONG(0), P073_OPTION_RIGHTALIGN);
        P073_data->timesep      = true;
        #ifdef P073_EXTRA_FONTS
        P073_data->fontset      = PCONFIG(4);
        #endif // P073_EXTRA_FONTS

        switch (PCONFIG(0)) {
          case P073_TM1637_4DGTCOLON: // set brightness of TM1637
          case P073_TM1637_4DGTDOTS:
          case P073_TM1637_6DGT: {
            int tm1637_bright = PCONFIG(2) / 2;
            tm1637_SetPowerBrightness(CONFIG_PIN1, CONFIG_PIN2, tm1637_bright,
                                      true);

            if (PCONFIG(1) == P073_DISP_MANUAL) {
              tm1637_ClearDisplay(CONFIG_PIN1, CONFIG_PIN2);
            }
            break;
          }
          case P073_MAX7219_8DGT: // set brightness of MAX7219
          {
            max7219_SetPowerBrightness(event, CONFIG_PIN1, CONFIG_PIN2, CONFIG_PIN3,
                                       PCONFIG(2), true);

            if (PCONFIG(1) == P073_DISP_MANUAL) {
              max7219_ClearDisplay(event, CONFIG_PIN1, CONFIG_PIN2, CONFIG_PIN3);
            }
            break;
          }
        }
      }
      success = true;
      break;
    }

    case PLUGIN_EXIT: {
      success = true;
      break;
    }

    case PLUGIN_INIT: {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P073_data_struct());
      P073_data_struct *P073_data =
        static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P073_data) {
        return success;
      }
      P073_data->pin1         = CONFIG_PIN1;
      P073_data->pin2         = CONFIG_PIN2;
      P073_data->pin3         = CONFIG_PIN3;
      P073_data->displayModel = PCONFIG(0);
      P073_data->output       = PCONFIG(1);
      P073_data->brightness   = PCONFIG(2);
      P073_data->periods      = bitRead(PCONFIG_LONG(0), P073_OPTION_PERIOD);
      P073_data->hideDegree   = bitRead(PCONFIG_LONG(0), P073_OPTION_HIDEDEGREE);
      #ifdef P073_SCROLL_TEXT
      P073_data->txtScrolling = bitRead(PCONFIG_LONG(0), P073_OPTION_SCROLLTEXT);
      P073_data->scrollFull   = bitRead(PCONFIG_LONG(0), P073_OPTION_SCROLLFULL);
      P073_data->setScrollSpeed(PCONFIG(3));
      #endif // P073_SCROLL_TEXT
      P073_data->rightAlignTempMAX7219 = bitRead(PCONFIG_LONG(0), P073_OPTION_RIGHTALIGN);
      P073_data->timesep      = true;
      #ifdef P073_EXTRA_FONTS
      P073_data->fontset      = PCONFIG(4);
      #endif // P073_EXTRA_FONTS

      switch (PCONFIG(0)) {
        case P073_TM1637_4DGTCOLON:
        case P073_TM1637_4DGTDOTS:
        case P073_TM1637_6DGT: {
          tm1637_InitDisplay(CONFIG_PIN1, CONFIG_PIN2);
          int tm1637_bright = PCONFIG(2) / 2;
          tm1637_SetPowerBrightness(CONFIG_PIN1, CONFIG_PIN2, tm1637_bright, true);
          if (PCONFIG(1) == P073_DISP_MANUAL) {
            tm1637_ClearDisplay(CONFIG_PIN1, CONFIG_PIN2);
          }
          break;
        }
        case P073_MAX7219_8DGT: {
          max7219_InitDisplay(event, CONFIG_PIN1, CONFIG_PIN2, CONFIG_PIN3);
          delay(10); // small poweroff/poweron delay
          max7219_SetPowerBrightness(event, CONFIG_PIN1, CONFIG_PIN2, CONFIG_PIN3,
                                     PCONFIG(2), true);
          if (PCONFIG(1) == P073_DISP_MANUAL) {
            max7219_ClearDisplay(event, CONFIG_PIN1, CONFIG_PIN2, CONFIG_PIN3);
          }
          break;
        }
      }
      success = true;
      break;
    }

    case PLUGIN_WRITE: {
      success = p073_plugin_write(event, string);
      break;
    }

    case PLUGIN_ONCE_A_SECOND: {
      P073_data_struct *P073_data =
        static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P073_data) {
        break;
      }

      if (P073_data->output == P073_DISP_MANUAL) {
        break;
      }

      if ((P073_data->output == P073_DISP_CLOCK24BLNK) ||
          (P073_data->output == P073_DISP_CLOCK12BLNK)) {
        P073_data->timesep = !P073_data->timesep;
      } else {
        P073_data->timesep = true;
      }

      if (P073_data->output == P073_DISP_DATE) {
        P073_data->FillBufferWithDate(true, 0, 0, 0);
      }
      else if ((P073_data->output == P073_DISP_CLOCK24BLNK) ||
               (P073_data->output == P073_DISP_CLOCK24)) {
        P073_data->FillBufferWithTime(true, 0, 0, 0, false);
      }
      else {
        P073_data->FillBufferWithTime(true, 0, 0, 0, true);
      }

      switch (P073_data->displayModel) {
        case P073_TM1637_4DGTCOLON:
        case P073_TM1637_4DGTDOTS: {
          tm1637_ShowTimeTemp4(event, P073_data->timesep, 0);
          break;
        }
        case P073_TM1637_6DGT: {
          if (PCONFIG(1) == P073_DISP_DATE) {
            tm1637_ShowDate6(event);
          }
          else {
            tm1637_ShowTime6(event);
          }
          break;
        }
        case P073_MAX7219_8DGT: {
          if (PCONFIG(1) == P073_DISP_DATE) {
            max7219_ShowDate(event, P073_data->pin1, P073_data->pin2,
                             P073_data->pin3);
          }
          else {
            max7219_ShowTime(event, P073_data->pin1, P073_data->pin2,
                             P073_data->pin3, P073_data->timesep);
          }
          break;
        }
      }
      break;
    }

    #ifdef P073_SCROLL_TEXT
    case PLUGIN_TEN_PER_SECOND: {
      P073_data_struct *P073_data =
        static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P073_data) {
        break;
      }

      if (P073_data->output != P073_DISP_MANUAL || !P073_data->txtScrolling) {
        break;
      }

      P073_data->NextScroll();

      switch (P073_data->displayModel) {
        case P073_TM1637_4DGTCOLON:
        case P073_TM1637_4DGTDOTS: {
          tm1637_ShowBuffer(event, 0, 4);
          break;
        }
        case P073_TM1637_6DGT: {
          tm1637_SwapDigitInBuffer(event, 0); // only needed for 6-digits displays
          tm1637_ShowBuffer(event, 0, 6);
          break;
        }
        case P073_MAX7219_8DGT: {
          P073_data->dotpos = -1; // avoid to display the dot
          max7219_ShowBuffer(event, P073_data->pin1, P073_data->pin2,
                            P073_data->pin3);
          break;
        }
      }
      break;
    }
    #endif // P073_SCROLL_TEXT
  }
  return success;
}

bool p073_plugin_write(struct EventStruct *event, const String& string) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return false;
  }

  String cmd = parseString(string, 1);
  cmd.toLowerCase();
  String text = parseStringToEndKeepCase(string, 2);
  if (cmd.equals("7dn")) {
    return p073_plugin_write_7dn(event, text);
  } else if (cmd.equals("7dt")) {
    return p073_plugin_write_7dt(event, text);
  #ifdef P073_7DDT_COMMAND
  } else if (cmd.equals("7ddt")) {
    return p073_plugin_write_7ddt(event, text);
  #endif
  } else if (cmd.equals("7dst")) {
    return p073_plugin_write_7dst(event);
  } else if (cmd.equals("7dsd")) {
    return p073_plugin_write_7dsd(event);
  } else if (cmd.equals("7dtext")) {
    return p073_plugin_write_7dtext(event, text);
  #ifdef P073_EXTRA_FONTS
  } else if (cmd.equals("7dfont")) {
    return p073_plugin_write_7dfont(event, text);
  #endif // P073_EXTRA_FONTS
  #ifdef P073_7DBIN_COMMAND
  } else if (cmd.equals("7dbin")) {
    return p073_plugin_write_7dbin(event, text);
  #endif // P073_7DBIN_COMMAND
  } else {
    bool p073_validcmd = false;
    bool p073_displayon = false;

    if (cmd.equals("7don")) {
      addLog(LOG_LEVEL_INFO, F("7DGT : Display ON"));
      p073_displayon = true;
      p073_validcmd  = true;
    } else if (cmd.equals("7doff")) {
      addLog(LOG_LEVEL_INFO, F("7DGT : Display OFF"));
      p073_displayon = false;
      p073_validcmd  = true;
    } else if (cmd.equals("7db")) {
      if ((event->Par1 >= 0) && (event->Par1 < 16)) {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("7DGT : Brightness=");
          log += event->Par1;
          addLog(LOG_LEVEL_INFO, log);
        }
        P073_data->brightness = event->Par1;
        p073_displayon        = true;
        p073_validcmd         = true;
      }
    }

    if (p073_validcmd) {
      switch (P073_data->displayModel) {
        case P073_TM1637_4DGTCOLON:
        case P073_TM1637_4DGTDOTS:
        case P073_TM1637_6DGT: {
          int tm1637_bright = P073_data->brightness / 2;
          tm1637_SetPowerBrightness(P073_data->pin1, P073_data->pin2,
                                    tm1637_bright, p073_displayon);
          break;
        }
        case P073_MAX7219_8DGT: {
          max7219_SetPowerBrightness(event, P073_data->pin1, P073_data->pin2,
                                     P073_data->pin3, P073_data->brightness,
                                     p073_displayon);
          break;
        }
      }
    }
    return p073_validcmd;
  }
  return false;
}

bool p073_plugin_write_7dn(struct EventStruct *event, const String& text) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return false;
  }

  if (P073_data->output != P073_DISP_MANUAL) {
    return false;
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("7DGT : Show Number=");
    log += event->Par1;
    addLog(LOG_LEVEL_INFO, log);
  }

  switch (P073_data->displayModel) {
    case P073_TM1637_4DGTCOLON: {
      if ((event->Par1 > -1000) && (event->Par1 < 10000)) {
        P073_data->FillBufferWithNumber(String(int(event->Par1)));
      }
      else {
        P073_data->FillBufferWithDash();
      }
      tm1637_ShowBuffer(event, TM1637_4DIGIT, 8);
      break;
    }
    case P073_TM1637_4DGTDOTS: {
      if ((event->Par1 > -1000) && (event->Par1 < 10000)) {
        P073_data->FillBufferWithNumber(text.c_str());
      }
      else {
        P073_data->FillBufferWithDash();
      }
      tm1637_ShowBuffer(event, TM1637_4DIGIT, 8);
      break;
    }
    case P073_TM1637_6DGT: {
      if ((event->Par1 > -100000) && (event->Par1 < 1000000)) {
        P073_data->FillBufferWithNumber(text.c_str());
      }
      else {
        P073_data->FillBufferWithDash();
      }
      tm1637_SwapDigitInBuffer(event, 2); // only needed for 6-digits displays
      tm1637_ShowBuffer(event, TM1637_6DIGIT, 8);
      break;
    }
    case P073_MAX7219_8DGT: {
      if (text.length() > 0) {
        if ((event->Par1 > -10000000) && (event->Par1 < 100000000)) {
          P073_data->FillBufferWithNumber(text.c_str());
        } else {
          P073_data->FillBufferWithDash();
        }
        max7219_ShowBuffer(event, P073_data->pin1, P073_data->pin2,
                           P073_data->pin3);
      }
      break;
    }
  }
  return true;
}

bool p073_plugin_write_7dt(struct EventStruct *event, const String& text) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return false;
  }

  if (P073_data->output != P073_DISP_MANUAL) {
    return false;
  }
  double p073_temptemp    = 0;
  bool   p073_tempflagdot = false;
  if (text.length() > 0) {
    validDoubleFromString(text, p073_temptemp);
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("7DGT : Show Temperature=");
    log += p073_temptemp;
    addLog(LOG_LEVEL_INFO, log);
  }

  switch (P073_data->displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS: {
      if ((p073_temptemp > 999) || (p073_temptemp < -99.9)) {
        P073_data->FillBufferWithDash();
      }
      else {
        if ((p073_temptemp < 100) && (p073_temptemp > -10)) {
          p073_temptemp    = int(p073_temptemp * 10);
          p073_tempflagdot = true;
        }
        P073_data->FillBufferWithTemp(p073_temptemp);

        if ((p073_temptemp == 0) && p073_tempflagdot) {
          P073_data->showbuffer[5] = 0;
        }
      }
      tm1637_ShowTimeTemp4(event, p073_tempflagdot, 4);
      break;
    }
    case P073_TM1637_6DGT: {
      if ((p073_temptemp > 999) || (p073_temptemp < -99.9)) {
        P073_data->FillBufferWithDash();
      }
      else {
        if ((p073_temptemp < 100) && (p073_temptemp > -10)) {
          p073_temptemp    = int(p073_temptemp * 10);
          p073_tempflagdot = true;
        }
        P073_data->FillBufferWithTemp(p073_temptemp);

        if ((p073_temptemp == 0) && p073_tempflagdot) {
          P073_data->showbuffer[5] = 0;
        }
      }
      tm1637_ShowTemp6(event, p073_tempflagdot);
      break;
    }
    case P073_MAX7219_8DGT: {
      p073_temptemp = int(p073_temptemp * 10);
      P073_data->FillBufferWithTemp(p073_temptemp);

      max7219_ShowTemp(event, P073_data->pin1, P073_data->pin2, P073_data->pin3, P073_data->hideDegree ? 6 : 5, -1);
      break;
    }
  }
  #ifdef P073_DEBUG
  P073_data->LogBufferContent(F("7dt"));
  #endif
  return true;
}

#ifdef P073_7DDT_COMMAND
bool p073_plugin_write_7ddt(struct EventStruct *event, const String& text) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return false;
  }

  if (P073_data->output != P073_DISP_MANUAL) {
    return false;
  }
  double p073_lefttemp    = 0.0;
  double p073_righttemp   = 0.0;
  bool   p073_tempflagdot = false;
  if (text.length() > 0) {
    validDoubleFromString(parseString(text, 1), p073_lefttemp);
    if (text.indexOf(',') > -1) {
      validDoubleFromString(parseString(text, 2), p073_righttemp);
    }
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("7DGT : Show Temperature 1st=");
    log += p073_lefttemp;
    log += F(" 2nd=");
    log += p073_righttemp;
    addLog(LOG_LEVEL_INFO, log);
  }

  switch (P073_data->displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS: {
      P073_data->FillBufferWithDash();
      tm1637_ShowTimeTemp4(event, p073_tempflagdot, 4);
      break;
    }
    case P073_TM1637_6DGT: {
      P073_data->FillBufferWithDash();
      tm1637_ShowTemp6(event, p073_tempflagdot);
      break;
    }
    case P073_MAX7219_8DGT: {
      uint8_t firstDot       = -1; // No decimals is no dots
      uint8_t secondDot      = -1;
      double  hideFactor     = P073_data->hideDegree ? 10.0 : 1.0;
      bool    firstDecimals  = false;
      bool    secondDecimals = false;

      if ((p073_lefttemp > 999.99 * hideFactor) || (p073_lefttemp < -99.99 * hideFactor)) {
        p073_lefttemp = -101.0 * hideFactor; // Triggers on -100
      }
      else {
        if ((p073_lefttemp < 100.0 * hideFactor) && (p073_lefttemp > -10.0 * hideFactor)) {
          p073_lefttemp = int(p073_lefttemp * 10.0);
          firstDot      = P073_data->hideDegree ? 2 : 1;
          firstDecimals = true;
        }
      }

      if ((p073_righttemp > 999.99 * hideFactor) || (p073_righttemp < -99.99 * hideFactor)) {
        p073_righttemp = -101.0 * hideFactor;
      }
      else {
        if ((p073_righttemp < 100.0 * hideFactor) && (p073_righttemp > -10.0 * hideFactor)) {
          p073_righttemp = int(p073_righttemp * 10.0);
          secondDot      = P073_data->hideDegree ? 6 : 5;
          secondDecimals = true;
        }
      }
      // #ifdef P073_DEBUG
      // String log = F("7DGT : preprocessed 1st=");
      // log += p073_lefttemp;
      // log += F(" 2nd=");
      // log += p073_righttemp;
      // addLog(LOG_LEVEL_INFO, log);
      // #endif

      P073_data->FillBufferWithDualTemp(p073_lefttemp, firstDecimals, p073_righttemp, secondDecimals);

      bool alignSave = P073_data->rightAlignTempMAX7219; // Save setting
      P073_data->rightAlignTempMAX7219 = true;

      max7219_ShowTemp(event, P073_data->pin1, P073_data->pin2, P073_data->pin3, firstDot, secondDot);

      P073_data->rightAlignTempMAX7219 = alignSave; // Restore

      break;
    }
  }
  #ifdef P073_DEBUG
  P073_data->LogBufferContent(F("7ddt"));
  #endif
  return true;
}
#endif

bool p073_plugin_write_7dst(struct EventStruct *event) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return false;
  }

  if (P073_data->output != P073_DISP_MANUAL) {
    return false;
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("7DGT : Show Time=");
    log += event->Par1;
    log += ":";
    log += event->Par2;
    log += ":";
    log += event->Par3;
    addLog(LOG_LEVEL_INFO, log);
  }
  P073_data->timesep = true;
  P073_data->FillBufferWithTime(false, event->Par1, event->Par2, event->Par3, false);

  switch (P073_data->displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS: {
      tm1637_ShowTimeTemp4(event, P073_data->timesep, 0);
      break;
    }
    case P073_TM1637_6DGT: {
      tm1637_ShowTime6(event);
      break;
    }
    case P073_MAX7219_8DGT: {
      max7219_ShowTime(event, P073_data->pin1, P073_data->pin2, P073_data->pin3,
                       P073_data->timesep);
      break;
    }
  }
  return true;
}

bool p073_plugin_write_7dsd(struct EventStruct *event) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return false;
  }

  if (P073_data->output != P073_DISP_MANUAL) {
    return false;
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("7DGT : Show Date=");
    log += event->Par1;
    log += "-";
    log += event->Par2;
    log += "-";
    log += event->Par3;
    addLog(LOG_LEVEL_INFO, log);
  }
  P073_data->FillBufferWithDate(false, event->Par1, event->Par2, event->Par3);

  switch (P073_data->displayModel) {
    case P073_TM1637_4DGTCOLON:
    case P073_TM1637_4DGTDOTS: {
      tm1637_ShowTimeTemp4(event, P073_data->timesep, 0);
      break;
    }
    case P073_TM1637_6DGT: {
      tm1637_ShowDate6(event);
      break;
    }
    case P073_MAX7219_8DGT: {
      max7219_ShowDate(event, P073_data->pin1, P073_data->pin2, P073_data->pin3);
      break;
    }
  }
  return true;
}

bool p073_plugin_write_7dtext(struct EventStruct *event, const String& text) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return false;
  }

  if (P073_data->output != P073_DISP_MANUAL) {
    return false;
  }
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("7DGT : Show Text=");
    log += text;
    addLog(LOG_LEVEL_INFO, log);
  }
  #ifdef P073_SCROLL_TEXT
  P073_data->setTextToScroll("");
  uint8_t bufLen = P073_data->getBufferLength(P073_data->displayModel);
  if (P073_data->txtScrolling && P073_data->getEffectiveTextLength(text) > bufLen) {
    P073_data->setTextToScroll(text);
  } else 
  #endif // P073_SCROLL_TEXT
  {
    P073_data->FillBufferWithString(text);

    switch (P073_data->displayModel) {
      case P073_TM1637_4DGTCOLON:
      case P073_TM1637_4DGTDOTS: {
        tm1637_ShowBuffer(event, 0, 4);
        break;
      }
      case P073_TM1637_6DGT: {
        tm1637_SwapDigitInBuffer(event, 0); // only needed for 6-digits displays
        tm1637_ShowBuffer(event, 0, 6);
        break;
      }
      case P073_MAX7219_8DGT: {
        P073_data->dotpos = -1; // avoid to display the dot
        max7219_ShowBuffer(event, P073_data->pin1, P073_data->pin2,
                          P073_data->pin3);
        break;
      }
    }
  }
  return true;
}

#ifdef P073_EXTRA_FONTS
bool p073_plugin_write_7dfont(struct EventStruct *event, const String& text) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return false;
  }

  if (text.length() > 0) {
    String fontArg = parseString(text, 1);
    int fontNr = -1;
    if (fontArg == F("default") || fontArg == F("7dgt")) {
      fontNr = 0;
    } else if (fontArg == F("siekoo")) {
      fontNr = 1;
    } else if (fontArg == F("siekoo_upper")) {
      fontNr = 2;
    } else if (fontArg == F("dseg7")) {
      fontNr = 3;
    } else if (!validIntFromString(text, fontNr)) {
      fontNr = -1; // reset if invalid
    }
    #ifdef P073_DEBUG
    String info = F("P037 7dfont,");
    info += fontArg;
    info += F(" -> ");
    info += fontNr;
    addLog(LOG_LEVEL_INFO, info);
    #endif // P073_DEBUG

    if (fontNr >= 0 && fontNr <= 3) {
      P073_data->fontset = fontNr;
      PCONFIG(4) = fontNr;
      return true;
    }
  }
  return false;
}
#endif // P073_EXTRA_FONTS

#ifdef P073_7DBIN_COMMAND
bool p073_plugin_write_7dbin(struct EventStruct *event, const String& text) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return false;
  }

  if (text.length() > 0) {
    String data;
    int byteValue;
    int arg = 1;
    String argValue = parseString(text, arg);
    while (argValue.length() > 0) {
      if (validIntFromString(argValue, byteValue) && byteValue < 256 && byteValue > -1) {
        data += static_cast<char>(P073_data->displayModel == P073_MAX7219_8DGT ? byteValue : P073_mapMAX7219FontToTM1673Font(byteValue));
      }
      arg++;
      argValue = parseString(text, arg);
    }
    #ifdef P073_SCROLL_TEXT
    uint8_t bufLen = P073_data->getBufferLength(P073_data->displayModel);
    #endif // P073_SCROLL_TEXT
    if (data.length() > 0) {
      #ifdef P073_SCROLL_TEXT
      P073_data->setTextToScroll(EMPTY_STRING); // Clear any scrolling text
      if (P073_data->txtScrolling && data.length() > bufLen) {
        P073_data->setBinaryData(data);
      } else
      #endif // P073_SCROLL_TEXT
      {
        P073_data->FillBufferWithString(data, true);

        switch (P073_data->displayModel) {
          case P073_TM1637_4DGTCOLON:
          case P073_TM1637_4DGTDOTS: {
            tm1637_ShowBuffer(event, 0, 4);
            break;
          }
          case P073_TM1637_6DGT: {
            tm1637_SwapDigitInBuffer(event, 0); // only needed for 6-digits displays
            tm1637_ShowBuffer(event, 0, 6);
            break;
          }
          case P073_MAX7219_8DGT: {
            P073_data->dotpos = -1; // avoid to display the dot
            max7219_ShowBuffer(event, P073_data->pin1, P073_data->pin2,
                              P073_data->pin3);
            break;
          }
        }
      }
      return true;
    }
  }
  return false;
}
#endif // P073_7DBIN_COMMAND

// ===================================
// ---- TM1637 specific functions ----
// ===================================

#define CLK_HIGH() digitalWrite(clk_pin, HIGH)
#define CLK_LOW() digitalWrite(clk_pin, LOW)
#define DIO_HIGH() pinMode(dio_pin, INPUT)
#define DIO_LOW() pinMode(dio_pin, OUTPUT)

void tm1637_i2cStart(uint8_t clk_pin, uint8_t dio_pin) {
  #ifdef P073_DEBUG
  addLog(LOG_LEVEL_DEBUG, F("7DGT : Comm Start"));
  #endif
  DIO_HIGH();
  CLK_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  DIO_LOW();
}

void tm1637_i2cStop(uint8_t clk_pin, uint8_t dio_pin) {
#ifdef P073_DEBUG
  addLog(LOG_LEVEL_DEBUG, F("7DGT : Comm Stop"));
#endif
  CLK_LOW();
  delayMicroseconds(TM1637_CLOCKDELAY);
  DIO_LOW();
  delayMicroseconds(TM1637_CLOCKDELAY);
  CLK_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  DIO_HIGH();
}

void tm1637_i2cAck(uint8_t clk_pin, uint8_t dio_pin) {
  #ifdef P073_DEBUG
  bool dummyAck = false;
  #endif

  CLK_LOW();
  pinMode(dio_pin, INPUT_PULLUP);

  // DIO_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);

  // while(digitalRead(dio_pin));
  #ifdef P073_DEBUG
  dummyAck = 
  #endif
  digitalRead(dio_pin);

  #ifdef P073_DEBUG
  String log = F("7DGT : Comm ACK=");

  if (dummyAck == 0) {
    log += "TRUE";
  } else {
    log += "FALSE";
  }
  addLog(LOG_LEVEL_DEBUG, log);
  #endif
  CLK_HIGH();
  delayMicroseconds(TM1637_CLOCKDELAY);
  CLK_LOW();
  pinMode(dio_pin, OUTPUT);
}

void tm1637_i2cWrite_ack(uint8_t clk_pin, uint8_t dio_pin,
                         uint8_t bytesToPrint[], uint8_t length) {
  tm1637_i2cStart(clk_pin, dio_pin);

  for (uint8_t i = 0; i < length; ++i) {
    tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint[i]);
  }
  tm1637_i2cStop(clk_pin, dio_pin);
}

void tm1637_i2cWrite_ack(uint8_t clk_pin, uint8_t dio_pin,
                         uint8_t bytetoprint) {
  tm1637_i2cWrite(clk_pin, dio_pin, bytetoprint);
  tm1637_i2cAck(clk_pin, dio_pin);
}

void tm1637_i2cWrite(uint8_t clk_pin, uint8_t dio_pin, uint8_t bytetoprint) {
  #ifdef P073_DEBUG
  addLog(LOG_LEVEL_DEBUG, F("7DGT : WriteByte"));
  #endif
  uint8_t i;

  for (i = 0; i < 8; i++) {
    CLK_LOW();

    if (bytetoprint & B00000001) {
      DIO_HIGH();
    } else {
      DIO_LOW();
    }
    delayMicroseconds(TM1637_CLOCKDELAY);
    bytetoprint = bytetoprint >> 1;
    CLK_HIGH();
    delayMicroseconds(TM1637_CLOCKDELAY);
  }
}

void tm1637_ClearDisplay(uint8_t clk_pin, uint8_t dio_pin) {
  uint8_t bytesToPrint[7] = { 0 };

  bytesToPrint[0] = 0xC0;
  tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint, 7);
}

void tm1637_SetPowerBrightness(uint8_t clk_pin, uint8_t dio_pin,
                               uint8_t brightlvl, bool poweron) {
  #ifdef P073_DEBUG
  addLog(LOG_LEVEL_INFO, F("7DGT : Set BRIGHT"));
  #endif
  uint8_t brightvalue = (brightlvl & 0b111);

  if (poweron) {
    brightvalue = TM1637_POWER_ON | brightvalue;
  } else {
    brightvalue = TM1637_POWER_OFF | brightvalue;
  }

  uint8_t bytesToPrint[1] = { 0 };
  bytesToPrint[0] = brightvalue;
  tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint, 1);
}

void tm1637_InitDisplay(uint8_t clk_pin, uint8_t dio_pin) {
  pinMode(clk_pin, OUTPUT);
  pinMode(dio_pin, OUTPUT);
  CLK_HIGH();
  DIO_HIGH();

  //  pinMode(dio_pin, INPUT_PULLUP);
  //  pinMode(clk_pin, OUTPUT);
  uint8_t bytesToPrint[1] = { 0 };
  bytesToPrint[0] = 0x40;
  tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint, 1);
  tm1637_ClearDisplay(clk_pin, dio_pin);
}

uint8_t tm1637_separator(uint8_t value, bool sep) {
  if (sep) {
    value |= 0b10000000;
  }
  return value;
}

uint8_t tm1637_getFontChar(uint8_t index, uint8_t fontset) {
  #ifdef P073_EXTRA_FONTS
  switch (fontset) {
    case 1: // Siekoo
    case 2: // Siekoo uppercase CHNORUX
      return P073_mapMAX7219FontToTM1673Font(pgm_read_byte(&(SiekooCharTable[index]))); // SiekooTableTM1637[index];
    case 3: // dSEG7
      return P073_mapMAX7219FontToTM1673Font(pgm_read_byte(&(Dseg7CharTable[index]))); // Dseg7TableTM1637[index];
    default: // Standard fontset
  #endif // P073_EXTRA_FONTS
      return P073_mapMAX7219FontToTM1673Font(pgm_read_byte(&(DefaultCharTable[index]))); // CharTableTM1637[index];
  #ifdef P073_EXTRA_FONTS
  }
  #endif // P073_EXTRA_FONTS
}

void tm1637_ShowTime6(struct EventStruct *event) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }

  uint8_t clk_pin         = P073_data->pin1;
  uint8_t dio_pin         = P073_data->pin2;
  bool    sep             = P073_data->timesep;
  uint8_t bytesToPrint[7] = { 0 };
  bytesToPrint[0] = 0xC0;
  bytesToPrint[1] = tm1637_getFontChar(P073_data->showbuffer[2], P073_data->fontset);
  bytesToPrint[2] =
    tm1637_separator(tm1637_getFontChar(P073_data->showbuffer[1], P073_data->fontset), sep);
  bytesToPrint[3] = tm1637_getFontChar(P073_data->showbuffer[0], P073_data->fontset);
  bytesToPrint[4] = tm1637_getFontChar(P073_data->showbuffer[5], P073_data->fontset);
  bytesToPrint[5] = tm1637_getFontChar(P073_data->showbuffer[4], P073_data->fontset);
  bytesToPrint[6] =
    tm1637_separator(tm1637_getFontChar(P073_data->showbuffer[3], P073_data->fontset), sep);
  tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint, 7);
}

void tm1637_ShowDate6(struct EventStruct *event) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }
  uint8_t clk_pin = P073_data->pin1;
  uint8_t dio_pin = P073_data->pin2;
  bool    sep     = P073_data->timesep;

  uint8_t bytesToPrint[7] = { 0 };
  bytesToPrint[0] = 0xC0;
  bytesToPrint[1] = tm1637_getFontChar(P073_data->showbuffer[2], P073_data->fontset);
  bytesToPrint[2] =
    tm1637_separator(tm1637_getFontChar(P073_data->showbuffer[1], P073_data->fontset), sep);
  bytesToPrint[3] = tm1637_getFontChar(P073_data->showbuffer[0], P073_data->fontset);
  bytesToPrint[4] = tm1637_getFontChar(P073_data->showbuffer[7], P073_data->fontset);
  bytesToPrint[5] = tm1637_getFontChar(P073_data->showbuffer[6], P073_data->fontset);
  bytesToPrint[6] =
    tm1637_separator(tm1637_getFontChar(P073_data->showbuffer[3], P073_data->fontset), sep);
  tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint, 7);
}

void tm1637_ShowTemp6(struct EventStruct *event, bool sep) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }
  uint8_t clk_pin = P073_data->pin1;
  uint8_t dio_pin = P073_data->pin2;

  uint8_t bytesToPrint[7] = { 0 };
  bytesToPrint[0] = 0xC0;
  bytesToPrint[1] =
    tm1637_separator(tm1637_getFontChar(P073_data->showbuffer[5], P073_data->fontset), sep);
  bytesToPrint[2] = tm1637_getFontChar(P073_data->showbuffer[4], P073_data->fontset);
  bytesToPrint[3] = tm1637_getFontChar(10, P073_data->fontset);
  bytesToPrint[4] = tm1637_getFontChar(10, P073_data->fontset);
  bytesToPrint[5] = tm1637_getFontChar(P073_data->showbuffer[7], P073_data->fontset);
  bytesToPrint[6] = tm1637_getFontChar(P073_data->showbuffer[6], P073_data->fontset);
  tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint, 7);
}

void tm1637_ShowTimeTemp4(struct EventStruct *event, bool sep, uint8_t bufoffset) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }
  uint8_t clk_pin = P073_data->pin1;
  uint8_t dio_pin = P073_data->pin2;

  uint8_t bytesToPrint[7] = { 0 };
  bytesToPrint[0] = 0xC0;
  bytesToPrint[1] = tm1637_getFontChar(P073_data->showbuffer[0 + bufoffset], P073_data->fontset);
  bytesToPrint[2] = tm1637_separator(
    tm1637_getFontChar(P073_data->showbuffer[1 + bufoffset], P073_data->fontset), sep);
  bytesToPrint[3] = tm1637_getFontChar(P073_data->showbuffer[2 + bufoffset], P073_data->fontset);
  bytesToPrint[4] = tm1637_getFontChar(P073_data->showbuffer[3 + bufoffset], P073_data->fontset);
  tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint, 5);
}

void tm1637_SwapDigitInBuffer(struct EventStruct *event, uint8_t startPos) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }
  uint8_t p073_temp;
  p073_temp                           = P073_data->showbuffer[2 + startPos];
  P073_data->showbuffer[2 + startPos] = P073_data->showbuffer[0 + startPos];
  P073_data->showbuffer[0 + startPos] = p073_temp;
  p073_temp                           = P073_data->showbuffer[3 + startPos];
  P073_data->showbuffer[3 + startPos] = P073_data->showbuffer[5 + startPos];
  P073_data->showbuffer[5 + startPos] = p073_temp;

  switch (P073_data->dotpos) {
    case 2: {
      P073_data->dotpos = 4;
      break;
    }
    case 4: {
      P073_data->dotpos = 2;
      break;
    }
    case 5: {
      P073_data->dotpos = 7;
      break;
    }
    case 7: {
      P073_data->dotpos = 5;
      break;
    }
  }
}

void tm1637_ShowBuffer(struct EventStruct *event, uint8_t firstPos, uint8_t lastPos) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }
  uint8_t clk_pin = P073_data->pin1;
  uint8_t dio_pin = P073_data->pin2;

  uint8_t bytesToPrint[8] = { 0 };
  bytesToPrint[0] = 0xC0;
  uint8_t length = 1;

  if (P073_data->dotpos > -1) {
    P073_data->showperiods[P073_data->dotpos] = true;
  }

  for (int i = firstPos; i < lastPos; i++) {
    uint8_t p073_datashowpos1 = tm1637_separator(
      tm1637_getFontChar(P073_data->showbuffer[i], P073_data->fontset), P073_data->showperiods[i]);
    bytesToPrint[length] = p073_datashowpos1;
    ++length;
  }
  tm1637_i2cWrite_ack(clk_pin, dio_pin, bytesToPrint, length);
}

// ====================================
// ---- MAX7219 specific functions ----
// ====================================

#define OP_DECODEMODE   9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15

void max7219_spiTransfer(struct EventStruct *event, uint8_t din_pin,
                         uint8_t clk_pin, uint8_t cs_pin, volatile uint8_t opcode,
                         volatile uint8_t data) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }

  P073_data->spidata[1] = opcode;
  P073_data->spidata[0] = data;
  digitalWrite(cs_pin, LOW);
  shiftOut(din_pin, clk_pin, MSBFIRST, P073_data->spidata[1]);
  shiftOut(din_pin, clk_pin, MSBFIRST, P073_data->spidata[0]);
  digitalWrite(cs_pin, HIGH);
}

void max7219_ClearDisplay(struct EventStruct *event, uint8_t din_pin,
                          uint8_t clk_pin, uint8_t cs_pin) {
  for (int i = 0; i < 8; i++) {
    max7219_spiTransfer(event, din_pin, clk_pin, cs_pin, i + 1, 0);
  }
}

void max7219_SetPowerBrightness(struct EventStruct *event, uint8_t din_pin,
                                uint8_t clk_pin, uint8_t cs_pin,
                                uint8_t brightlvl, bool poweron) {
  max7219_spiTransfer(event, din_pin, clk_pin, cs_pin, OP_INTENSITY, brightlvl);
  max7219_spiTransfer(event, din_pin, clk_pin, cs_pin, OP_SHUTDOWN,
                      poweron ? 1 : 0);
}

void max7219_SetDigit(struct EventStruct *event, uint8_t din_pin,
                      uint8_t clk_pin, uint8_t cs_pin, int dgtpos,
                      uint8_t dgtvalue, boolean showdot, bool binaryData = false) {
  uint8_t p073_tempvalue;

  #ifdef P073_EXTRA_FONTS
  switch (PCONFIG(4)) {
    case 1: // Siekoo
    case 2: // Siekoo with uppercase CHNORUX
      p073_tempvalue = pgm_read_byte(&(SiekooCharTable[dgtvalue]));
      break;
    case 3: // dSEG7
      p073_tempvalue = pgm_read_byte(&(Dseg7CharTable[dgtvalue]));
      break;
    default: // Default fontset
  #endif // P073_EXTRA_FONTS
      p073_tempvalue = pgm_read_byte(&(DefaultCharTable[dgtvalue]));
  #ifdef P073_EXTRA_FONTS
  }
  #endif // P073_EXTRA_FONTS

  if (showdot) {
    p073_tempvalue |= 0b10000000;
  }
  if (binaryData) p073_tempvalue = dgtvalue; // Overwrite if binary data
  max7219_spiTransfer(event, din_pin, clk_pin, cs_pin, dgtpos + 1,
                      p073_tempvalue);
}

void max7219_InitDisplay(struct EventStruct *event, uint8_t din_pin,
                         uint8_t clk_pin, uint8_t cs_pin) {
  pinMode(din_pin, OUTPUT);
  pinMode(clk_pin, OUTPUT);
  pinMode(cs_pin,  OUTPUT);
  digitalWrite(cs_pin, HIGH);
  max7219_spiTransfer(event, din_pin, clk_pin, cs_pin, OP_DISPLAYTEST, 0);
  max7219_spiTransfer(event, din_pin, clk_pin, cs_pin, OP_SCANLIMIT,   7); // scanlimit setup to max at Init
  max7219_spiTransfer(event, din_pin, clk_pin, cs_pin, OP_DECODEMODE,  0);
  max7219_ClearDisplay(event, din_pin, clk_pin, cs_pin);
  max7219_SetPowerBrightness(event, din_pin, clk_pin, cs_pin, 0, false);
}

void max7219_ShowTime(struct EventStruct *event, uint8_t din_pin,
                      uint8_t clk_pin, uint8_t cs_pin, bool sep) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }

  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 0, P073_data->showbuffer[5], false);
  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 1, P073_data->showbuffer[4], false);
  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 3, P073_data->showbuffer[3], false);
  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 4, P073_data->showbuffer[2], false);
  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 6, P073_data->showbuffer[1], false);
  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 7, P073_data->showbuffer[0], false);
  uint8_t sepChar = P073_mapCharToFontPosition(sep ? '-' : ' ', P073_data->fontset);
  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 2,   sepChar,                false);
  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 5,   sepChar,                false);
}

void max7219_ShowTemp(struct EventStruct *event, uint8_t din_pin,
                      uint8_t clk_pin, uint8_t cs_pin, int8_t firstDot, int8_t secondDot) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }

  max7219_SetDigit(event, din_pin, clk_pin, cs_pin, 0, 10, false);
  if (firstDot  > -1) P073_data->showperiods[firstDot] = true;
  if (secondDot > -1) P073_data->showperiods[secondDot] = true;

  int alignRight = P073_data->rightAlignTempMAX7219 ? 0 : 1;
  for (int i = alignRight; i < 8; i++) {
    max7219_SetDigit(event, din_pin, clk_pin, cs_pin, i,
                     P073_data->showbuffer[(7 + alignRight) - i], P073_data->showperiods[(7 + alignRight) - i]);
  }
}

void max7219_ShowDate(struct EventStruct *event, uint8_t din_pin,
                      uint8_t clk_pin, uint8_t cs_pin) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }

  uint8_t dotflags[8] = { false, true, false, true, false, false, false, false };

  for (int i = 0; i < 8; i++) {
    max7219_SetDigit(event, din_pin, clk_pin, cs_pin, i,
                     P073_data->showbuffer[7 - i], dotflags[7 - i]);
  }
}

void max7219_ShowBuffer(struct EventStruct *event, uint8_t din_pin,
                        uint8_t clk_pin, uint8_t cs_pin) {
  P073_data_struct *P073_data =
    static_cast<P073_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P073_data) {
    return;
  }

  if (P073_data->dotpos > -1) {
    P073_data->showperiods[P073_data->dotpos] = true;
  }

  for (int i = 0; i < 8; i++) {
    max7219_SetDigit(event, din_pin, clk_pin, cs_pin, i,
                     P073_data->showbuffer[7 - i], P073_data->showperiods[7 - i]
                     #ifdef P073_7DBIN_COMMAND
                     , P073_data->binaryData
                     #endif // P073_7DBIN_COMMAND
                     );
  }
}

#endif // USES_P073
