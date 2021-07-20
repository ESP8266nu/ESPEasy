#pragma once

/**
 * \file
 * \brief Main header file for the MD_MAX72xx library
 */

#if defined(__MBED__) && !defined(ARDUINO)
#include "mbed.h"
#define delay   ThisThread::sleep_for
#ifndef PROGMEM
 #define PROGMEM
#endif
#ifndef boolean
 #define boolean bool
#endif
#ifndef bitRead
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#endif
#ifndef bitSet
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#endif
#ifndef bitClear
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#endif
#ifndef bitWrite
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))
#endif
#ifndef pgm_read_byte
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
 #define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
 #define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif
#else
#include <Arduino.h>
#endif
#include <SPI.h>

/**
\mainpage Arduino LED Matrix Library
The Maxim 72xx LED Controller IC
--------------------------------
The MAX7219/MAX7221 are compact, serial input/output display drivers that
interface microprocessors to 7-segment numeric LED displays of up to 8 digits,
bar-graph displays, or 64 individual LEDs. Included on-chip are a BCD code-B
decoder, multiplex scan circuitry, segment and digit drivers, and an 8x8 static
RAM that stores each digit.

A 4-wire serial interface (SPI) allows the devices to be cascaded, with
communications passed through the first device in the chain to all others. Individual
elements may be addressed and updated without rewriting the entire display.

This library implements functions that allow the MAX72xx to be used
for LED matrices (64 individual LEDs), allowing the programmer to use the LED
matrix as a pixel device, displaying graphics elements much like any other
pixel addressable display.

Topics
------
- \subpage pageHardware
- \subpage pageSoftware
- \subpage pageConnect
- \subpage pageFontUtility
- \subpage pageRevisionHistory
- \subpage pageCopyright
- \subpage pageDonation

\page pageDonation Support the Library
If you like and use this library please consider making a small donation using [PayPal](https://paypal.me/MajicDesigns/4USD)

\page pageCopyright Copyright
Copyright (C) 2012-18 Marco Colli. All rights reserved.

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

\page pageRevisionHistory Revision History
Jan 2021 version 3.3.0
- Added SimplePong example
- New form of constructor allows specifying alternative hardware SPI

Dec 2020 version 3.2.6
- Added SimpleSlots example
- Added Examples README.md

Dec 2020 version 3.2.5
- Fixed issue with MBED and ARDUINO compatibility exposed by Nano BLE33 (issue #38, PR #39)

Nov 2020 version 3.2.4
- Fixed TSLR bug reported in transform() function

Aug 2020 version 3.2.3
- Corrected Message_Serial example

Nov 2019 version 3.2.2
- Added structured hardware names for untested hardware configurations (\ref pageNewHardware).
- Mbed (https://github.com/ARMmbed/mbed-os) support (PR #30 from JojoS62, 2020-07-06)

Oct 2019 version 3.2.1
- First large (150 module system) - increase size of SPI counters to int16_t

Sep 2019 version 3.2.0
- Change character codes to 16 bit to allow up to 65535 characters in font table.
- Retested examples for clean compile with new version of compiler.

May 2019 version 3.1.0
- Changed font definition to more modern look.
- Font ASCII code > 127 now conforms to unicode Latin-1 supplement.

Oct 2018 version 3.0.2
- Updated to remove 'left over' serial print debug.

Aug 2018 version 3.0.1
- Fixed problem with calculating font width in info structure.

June 2018 version 3.0.0
- Implemented new font file format (file format version 1).
- Removed 'drawXXX' graphics functions to new MD_MAXPanel library.
- Removed font indexing as this never used.
- Added getFontHeight() method.
- Module type now specified at run time.

Apr 2018 version 2.11.1
- Another attempt to further clarify editing of header file for hardware changes.

Apr 2018 version 2.11.0
- Restructured header file to make hardware flags more obvious
- Added drawHLine(), drawVLine(), drawRectangle() methods

Mar 2018 version 2.10.1
- Reworked HW Mapping utility.
- setColumn() parameter changed from uint8_t to uint16_t to allow for more than 256 column in the matrix [BUG]

Nov 2017 version 2.10.0
- Changed SPI buffer handling and isolation of AVR specific features (eg PROGMEM)
- Added MD_MAX72xx_Message_ESP8266 example
- Minor source file cleanup
- Added Extended ASCII font, vertical rotated font and RobotEyes font in fontbuilder
- Modified font builder output code for consistency with new code
- Added getFont(), getMaxFontWidth() methods
- Changed example - replaced MD_KeySwitch with new MD_UISwitch library

Nov 2016 version 2.9.0
- Added WordClock example
- Deprecated USE_LIBRARY_SPI as no problems reported with new implementation
- Changed MD_ branding to new MajicDesigns diamond
- Small adjustments to initialization code

Mar 2016 version 2.8
- Added example Message_SD and renamed Message to Message_Serial
- Added Pacman example
- Added PushWheel example
- Added USE_LIBRARY_SPI to enable library SPI object
- Modified all examples to conditionally include <SPI.h>
- FontBuilder modified to handle definitions for double height fonts
- New txt2font utility for easier font creation from a text file
- Revised and re-organized documentation; expanded section on fonts

April 2015 version 2.7
- Changed to Daft Punk example to run without switch
- Now supporting IDE Library Manager

February 2015 version 2.6
- Improvements to HW_Mapper utility
- Added HW_USE_FC16 for FC-16 display modules
- Added USE_HW_OTHER for user defined hardware configuration
- Fixed incorrect spelling for HW_REV_COLS in transformBuffer() & corresponding bug
- Added PrintText_ML example

February 2015 version 2.5
- Documented process for adding new hardware module type
- Fixed PROGMEM definitions for IDE version 1.5.7 compile error
- Added Daft Punk example code
- Updated HW_Mapper example/utility with built-in instructions
- Minor problems with Parola font setting interaction fixed

April 2014 version 2.4
- Improved reliability of initialization code to remove artifacts
 + Changed order of hardware initialization for SS, _csPin
 + Changed initialization sequence at begin()
 + Fixed memset bug identified by bperrybap
- Reworked command SPI transmissions for efficiency
- Cleanup up compiler warnings on inline wrapper code functions
- Cleaned up examples begin() - better defined library default values
- Reviewed and tidied up some documentation

March 2014 - version 2.3
- Extensive rework of the font system
 + New Microsoft Excel VBA based font builder tool available
 + Removed USE_FONT_ADJUST and related code - replace by builder tool
 + Fixed width font has been removed from the library. Definition still available in font builder
 + fontype_t definition changed to suit new requirements
- Transform zoning implemented (contiguous subset of services)
 + Transformation functions, control, clear, setRow methods overloaded with range specifier
 + User callback for L/R rotation function syntax added a device parameter
 + New Zones example
- USE_*_HW hardware types now separated out for future flexibility
 + Rework of the library to use new schema for defining hardware characteristics
 + New utility code to map out digits and segments for unknown hardware types
- Rechecked and reworked examples for new library

November 2013 - version 2.2
- Replaced reference to SPI library with inline code to allow for different select lines
- Obsoleted INCLUDE_HARDWARE_SPI conditional compile switch
- Fixed legacy code function name error when USE_FONT_ADJUST switch turned on
- Implemented USE_PAROLA_HW to allow cheaply available matrix modules to be used in ganged mode
- Fixed reversal of bit field for set/get Row/Column functions -> flipped charset data
- Added Eyes example program
- Upgraded and reorganized documentation

June 2013 - version 2.1
- Include the selection of hardware SPI interface (10x speed improvement)
- Tidied up comments

April 2013 - version 2.0
- Major update and rewrite of library code:
 + Improved speed and efficiency of code
 + Increased level of abstraction in the library for pixel methods
 + Increased level of abstraction for character and font methods
 + Increased number of functions and added variable sized font
 + Changed defines to enumerated types within the scope of the class
 + Updated functionality to simplify controlling multiple devices
- Changed text and comments to be aligned to doxygen documentation generation

June 2012 - version 1.0
- Incorporated elements of Arduino LedControl (Eberhard Fahle) and MAX7219 libraries
- Easier functionality for pixel graphics treatment of 8x8 matrices

\page pageSoftware Software Library
The Library
-----------
The library implements functions that allow the MAX72xx to be used
for LED matrices (64 individual LEDs), allowing the programmer to use the LED
matrix as a pixel device, displaying graphics elements much like any other
pixel addressable display.

In this scenario, it is convenient to abstract out the concept of the hardware device
and create a uniform and consistent pixel address space, with the libraries determining
device and device-element address. Similarly, control of the devices should be uniform
and abstracted to a system level.

The library still retains flexibility for device level control, should the developer
require, through the use of overloaded class methods.
___

Conditional Compilation Switches
--------------------------------
The library allows the run time code to be tailored through the use of compilation
switches. The compile options start with USE_ and are documented in the section
related to the main header file MD_MAX72xx.h.

_NOTE_: Compile switches must be edited in the library header file. Arduino header file
'mashing' during compilation makes the setting of these switches from user code
completely unreliable.

\page pageConnect System Connections
Connections to the Arduino Board (SPI interface)
------------------------------------------------
The modules are connected through a 4-wire serial interface (SPI), and devices are cascaded,
with communications passed through the first device in the chain to all others. The Arduino
should be connected to the IN side of the first module in the chain.

The Arduino interface is implemented with either
+ The hardware SPI interface, or
+ 3 arbitrary digital outputs that are passed through to the class constructor.

The AVR hardware SPI interface is fast but fixed to predetermined output pins. The more general
software interface uses the Arduino shiftOut() library function, making it slower but allows the
use of arbitrary digital pins to send the data to the device. Which mode is enabled depends
on the class constructor used.

The Arduino interface is implemented with 3 digital outputs that are passed through to
the class constructor. The digital outputs define the SPI interface as follows:
- DIN (MOSI) - the Data IN signal shifts data into the display module. Data is loaded into
the device's internal 16-bit shift register on CLK's rising edge.
- CLK (SCK) - the CLocK signal that is used to time the data for the device.
- LD (SS) - the interface is active when LoaD signal is LOW. Serial data are loaded into the
device shift register while LOAD is LOW and latched in on the rising edge.

The LD signal is used to select the entire device chain. This allows separate LD
outputs to control multiple displays sharing the same DIN and CLK signals. The
software needs to instantiate a separate object for each display.

The remaining interface pins are for +5V and GND. The power supply must be able to supply
enough current for the number of connected modules.
*/

/**
 \def USE_LOCAL_FONT
 Set to 1 (default) to enable local font in this library and enable
 loadChar() and related methods. If the library is just used for
 graphics some FLASH RAM can be saved by not including the code to process
 font data. The font file is stored in PROGMEM.
 */
#ifndef USE_LOCAL_FONT
#define USE_LOCAL_FONT 1
#endif

// Display parameter constants
// Defined values that are used throughout the library to define physical limits
#define ROW_SIZE  8   ///< The size in pixels of a row in the device LED matrix array
#define COL_SIZE  8   ///< The size in pixels of a column in the device LED matrix array
#define MAX_INTENSITY 0xf ///< The maximum intensity value that can be set for a LED array
#define MAX_SCANLIMIT 7   ///< The maximum scan limit value that can be set for the devices

/**
 * Core object for the MD_MAX72XX library
 */
class MD_MAX72XX
{
public:
  /**
  * Module Type enumerated type.
  *
  * This enumerated type is used to defined the type of
  * modules being used in the application. The types of modules are
  * discussed in detail in the Hardware section of this documentation.
  * For structured name types see \ref pageNewHardware.
  */
  enum moduleType_t
  {
    GENERIC_HW,   ///< Use 'generic' style hardware modules commonly available.
    FC16_HW,      ///< Use FC-16 style hardware module.
    PAROLA_HW,    ///< Use the Parola style hardware modules.
    ICSTATION_HW, ///< Use ICStation style hardware module.

    DR0CR0RR0_HW, ///< Structured name
    DR0CR0RR1_HW, ///< Structured name
    DR0CR1RR0_HW, ///< Structured name equivalent to GENERIC_HW
    DR0CR1RR1_HW, ///< Structured name
    DR1CR0RR0_HW, ///< Structured name equivalent to FC16_HW
    DR1CR0RR1_HW, ///< Structured name
    DR1CR1RR0_HW, ///< Structured name equivalent to PAROLA_HW
    DR1CR1RR1_HW  ///< Structured name equivalent to ICSTATION_HW
  };

#if USE_LOCAL_FONT
  /**
  * Font definition type.
  *
  * This type is used in the setFont() method to set the font to be used
  */
  typedef  const uint8_t fontType_t;
#endif

  /**
  * Control Request enumerated type.
  *
  * This enumerated type is used with the control() method to identify
  * the control action request.
  */
  enum controlRequest_t
  {
    SHUTDOWN = 0,   ///< Shut down the MAX72XX. Requires ON/OFF value. Library default is OFF.
    SCANLIMIT = 1,  ///< Set the scan limit for the MAX72XX. Requires numeric value [0..MAX_SCANLIMIT]. Library default is all on.
    INTENSITY = 2,  ///< Set the LED intensity for the MAX72XX. Requires numeric value [0..MAX_INTENSITY]. LIbrary default is MAX_INTENSITY/2.
    TEST = 3,       ///< Set the MAX72XX in test mode. Requires ON/OFF value. Library default is OFF.
    DECODE = 4,     ///< Set the MAX72XX 7 segment decode mode. Requires ON/OFF value. Library default is OFF.
    UPDATE = 10,    ///< Enable or disable auto updates of the devices from the library. Requires ON/OFF value. Library default is ON.
    WRAPAROUND = 11 ///< Enable or disable wraparound when shifting (circular buffer). Requires ON/OFF value. Library default is OFF.
  };

  /**
  * Control Value enumerated type.
  *
  * This enumerated type is used with the control() method as the
  * ON/OFF value for a control request. Other values may be used
  * if numeric data is required.
  */
  enum controlValue_t
  {
   MD_OFF = 0,  ///< General OFF status request
   MD_ON = 1    ///< General ON status request
  };

  /**
   * Transformation Types enumerated type.
  *
  * This enumerated type is used in the transform() methods to identify a
  * specific transformation of the display data in the device buffers.
  */
  enum transformType_t
  {
    TSL,  ///< Transform Shift Left one pixel element
    TSR,  ///< Transform Shift Right one pixel element
    TSU,  ///< Transform Shift Up one pixel element
    TSD,  ///< Transform Shift Down one pixel element
    TFLR, ///< Transform Flip Left to Right
    TFUD, ///< Transform Flip Up to Down
    TRC,  ///< Transform Rotate Clockwise 90 degrees
    TINV  ///< Transform INVert (pixels inverted)
  };

  /**
   * Class Constructor - arbitrary digital interface.
   *
   * Instantiate a new instance of the class. The parameters passed are used to
   * connect the software to the hardware. Multiple instances may co-exist
   * but they should not share the same hardware CS pin (SPI interface).
   *
   * \param mod       module type used in this application. One of the moduleType_t values.
   * \param dataPin   output on the Arduino where data gets shifted out.
   * \param clkPin    output for the clock signal.
   * \param csPin     output for selecting the device.
   * \param numDevices  number of devices connected. Default is 1 if not supplied.
   *                    Memory for device buffers is dynamically allocated based
   *                    on this parameter.
   */
  MD_MAX72XX(moduleType_t mod, uint8_t dataPin, uint8_t clkPin, uint8_t csPin, uint8_t numDevices=1);

  /**
   * Class Constructor - default SPI hardware interface.
   *
   * Instantiate a new instance of the class. The parameters passed are used to
   * connect the software to the hardware. Multiple instances may co-exist
   * but they should not share the same hardware CS pin (SPI interface).
   * The dataPin and the clockPin are defined by the Arduino hardware definition
   * (SPI MOSI and SCK signals).
   *
   * \param mod     module type used in this application. One of the moduleType_t values.
   * \param csPin   output for selecting the device.
   * \param numDevices  number of devices connected. Default is 1 if not supplied.
   *                    Memory for device buffers is dynamically allocated based
   *                    on this parameter.
   */
  MD_MAX72XX(moduleType_t mod, uint8_t csPin, uint8_t numDevices=1);

  /**
   * Class Constructor - specify SPI hardware interface.
   *
   * Instantiate a new instance of the class with a specified SPI object. This
   * allows a specific SPI interface to be specified for architectures with more
   * than one hardware SPI interface.
   * The parameters passed are used to connect the software to the hardware.
   * Multiple instances may co-exist but they should not share the same hardware
   * CS pin (SPI interface). The dataPin and the clockPin are defined by the Arduino
   * hardware definition for the specified SPI interface (SPI MOSI and SCK signals).
   *
   * \param mod     module type used in this application. One of the moduleType_t values.
   * \param spi     Reference to the SPI object to use for comms to the device
   * \param csPin   output for selecting the device.
   * \param numDevices  number of devices connected. Default is 1 if not supplied.
   *                    Memory for device buffers is dynamically allocated based
   *                    on this parameter.
   */
  MD_MAX72XX(moduleType_t mod, SPIClass &spi, uint8_t csPin, uint8_t numDevices = 1);

  /**
   * Initialize the object.
   *
   * Initialize the object data. This needs to be called during setup() to initialize
   * new data for the class that cannot be done during the object creation.
   *
   * The LED hardware is initialized to the middle intensity value, all rows showing,
   * and all LEDs cleared (off). Test, shutdown and decode modes are off. Display updates
   * are on and wraparound is off.
   */
  void begin(void);

  /**
   * Class Destructor.
   *
   * Released allocated memory and does the necessary to clean up once the object is
   * no longer required.
   */
  ~MD_MAX72XX();

  //--------------------------------------------------------------
  /** \name Methods for object and hardware control.
   * @{
   */
  /**
   * Set the control status of the specified parameter for the specified device.
   *
   * The device has a number of control parameters that can be set through this method.
   * The type of control action required is passed through the mode parameter and
   * should be one of the control actions defined by controlRequest_t. The value that
   * needs to be supplied on the control action required is one of the defined
   * actions in controlValue_t or a numeric parameter suitable for the control action.
   *
   * \param dev     address of the device to control [0..getDeviceCount()-1].
   * \param mode    one of the defined control requests.
   * \param value   parameter value or one of the control status defined.
   * \return false if parameter errors, true otherwise.
   */
  bool control(uint8_t dev, controlRequest_t mode, int value);

  /**
   * Set the control status of the specified parameter for all devices.
   *
   * Invokes the control function for each device in turn. as this is a wrapper for the
   * control(startDev, endDev, ...) methods, see the documentation for that method.
   *
   * \param mode    one of the defined control requests.
   * \param value   parameter value or one of the control status defined.
   * \return No return value.
   */
  inline void control(controlRequest_t mode, int value) { control(0, getDeviceCount()-1, mode, value); };

  /**
   * Set the control status of the specified parameter for contiguous subset of devices.
   *
   * Invokes the control function for each device in turn for the devices in the subset.
   * See documentation for the control() method.
   *
   * \param startDev  the first device for the control action [0..getDeviceCount()-1]
   * \param endDev    the last device for the control action [0..getDeviceCount()-1]
   * \param mode      one of the defined control requests.
   * \param value     parameter value or one of the control status defined.
   * \return false if parameter errors, true otherwise.
   */
  bool control(uint8_t startDev, uint8_t endDev, controlRequest_t mode, int value);

  /**
   * Gets the number of devices attached to this class instance.
   *
   * \return uint8_t representing the number of devices attached to this object.
   */
  uint8_t getDeviceCount(void) { return(_maxDevices); };

  /**
   * Gets the maximum number of columns for devices attached to this class instance.
   *
   * \return uint16_t representing the number of columns.
   */
  uint16_t getColumnCount(void) { return(_maxDevices*COL_SIZE); };

  /**
   * Set the type of hardware module being used.
   *
   * This method changes the type of module being used in the application
   * during at run time.
   *
   * \param mod module type used in this application; one of the moduleType_t values.
   * \return No return data
   */
  void setModuleType(moduleType_t mod) { setModuleParameters(mod); };

  /**
   * Set the Shift Data In callback function.
   *
   * The callback function is called from the library when a transform shift left
   * or shift right operation is executed and the library needs to obtain data for
   * the end element of the shift (ie, conceptually this is the new data that is
   * shifted 'into' the display). The callback function is invoked when
   * - WRAPAROUND is not active, as the data would automatically supplied within the library.
   * - the call to transform() is global (ie, not for an individual buffer).
   *
   * The callback function takes 2 parameters:
   * - the device number requesting the data [0..getDeviceCount()-1]
   * - one of the transformation types in transformType_t) that tells the callback function
   * what shift is being performed
   * The return value is the data for the column to be shifted into the display.
   *
   * \param cb  the address of the user function to be called from the library.
   * \return No return data
   */
  void setShiftDataInCallback(uint8_t (*cb)(uint8_t dev, transformType_t t)) { _cbShiftDataIn = cb; };

  /**
   * Set the Shift Data Out callback function.
   *
   * The callback function is called from the library when a transform shift left
   * or shift right operation is executed and the library is about to discard the data for
   * the first element of the shift (ie, conceptually this is the data that 'falls' off
   * the front end of the scrolling display). The callback function is invoked when
   * - WRAPAROUND is not active, as the data would automatically supplied to the tail end.
   * - the call to transform() is global (ie, not for an individual buffer).
   *
   * The callback function is with supplied 3 parameters, with no return value required:
   * - the device number that is the source of the data [0..getDeviceCount()-1]
   * - one of the transformation types transformType_t that tells the callback
   * function the type of shifting being executed
   * - the data for the column being shifted out
   *
   * \param cb  the address of the user function to be called from the library.
   * \return No return data
   */
  void setShiftDataOutCallback(void (*cb)(uint8_t dev, transformType_t t, uint8_t colData)) { _cbShiftDataOut = cb; };

  /** @} */

  //--------------------------------------------------------------
  /** \name Methods for graphics and bitmap related abstraction.
   * @{
   */
  /**
   * Clear all the display data on all the display devices.
   *
   * \return No return value.
   */
  inline void clear(void) { clear(0, getDeviceCount()-1); };

  /**
   * Clear all the display data on a subset of devices.
   *
   * endDev must be greater than or equal to startDev.
   *
   * \param startDev  the first device to clear [0..getDeviceCount()-1]
   * \param endDev    the last device to clear [0..getDeviceCount()-1]
   * \return No return value.
   */
  void clear(uint8_t startDev, uint8_t endDev);

  /**
   * Load a bitmap from the display buffers to a user buffer.
   *
   * Allows the calling program to read bitmaps (characters or graphic)
   * elements from the library display buffers. The data buffer
   * pointer should be a block of uint8_t data of size elements that will
   * contain the returned data.
   *
   * \param col   address of the display column [0..getColumnCount()-1].
   * \param size  number of columns of data to return.
   * \param *pd   Pointer to a data buffer [0..size-1].
   * \return false if parameter errors, true otherwise. If true, data will be in the buffer at *pd.
   */
  bool getBuffer(uint16_t col, uint8_t size, uint8_t *pd);

  /**
   * Get the LEDS status for the specified column.
   *
   * This method operates on a specific buffer
   *
   * This method operates on one column, getting the bit field value of
   * the LEDs in the column. The column is referenced with the absolute column
   * number (ie, the device number is inferred from the column).
   *
   * \param c   column to be read [0..getColumnCount()-1].
   * \return uint8_t value with each bit set to 1 if the corresponding LED is lit. 0 is returned for parameter error.
   */
  uint8_t getColumn(uint8_t c) { return getColumn((c / COL_SIZE), c % COL_SIZE); };

  /**
   * Get the status of a single LED, addressed as a pixel.
   *
   * The method will get the status of a specific LED element based on its
   * coordinate position. The column number is dereferenced into the device
   * and column within the device, allowing the LEDs to be treated as a
   * continuous pixel field.
   *
   * \param r   row coordinate for the point [0..ROW_SIZE-1].
   * \param c   column coordinate for the point [0..getColumnCount()-1].
   * \return true if LED is on, false if off or parameter errors.
   */
  bool getPoint(uint8_t r, uint16_t c);

 /**
   * Load a bitfield from the user buffer to a display buffer.
   *
   * Allows the calling program to define bitmaps (characters or graphic)
   * elements and pass them to the library for display. The data buffer
   * pointer should be a block of uint8_t data of size elements that define
   * the bitmap.
   *
   * \param col   address of the start display column [0..getColumnCount()-1].
   * \param size  number of columns of data following.
   * \param *pd   Pointer to a data buffer [0..size-1].
   * \return false if parameter errors, true otherwise.
   */
  bool setBuffer(uint16_t col, uint8_t size, uint8_t *pd);

  /**
   * Set all LEDs in a specific column to a new state.
   *
   * This method operates on one column, setting the value of the LEDs in
   * the column to the specified value bitfield. The column is
   * referenced with the absolute column number (ie, the device number is
   * inferred from the column). The method is useful for drawing vertical
   * lines and patterns when the display is being treated as a pixel field.
   * The least significant bit of the value is the lowest row number.
   *
   * \param c     column which is to be set [0..getColumnCount()-1].
   * \param value each bit set to 1 will light up the corresponding LED.
   * \return false if parameter errors, true otherwise.
   */
  bool setColumn(uint16_t c, uint8_t value) { return setColumn((c / COL_SIZE), c % COL_SIZE, value); };

  /**
   * Set the status of a single LED, addressed as a pixel.
   *
   * The method will set the value of a specific LED element based on its
   * coordinate position. The LED will be turned on or off depending on the
   * value supplied. The column number is dereferenced into the device and
   * column within the device, allowing the LEDs to be treated as a
   * continuous pixel field.
   *
   * \param r     row coordinate for the point [0..ROW_SIZE-1].
   * \param c     column coordinate for the point [0..getColumnCount()-1].
   * \param state true - switch on; false - switch off.
   * \return false if parameter errors, true otherwise.
   */
  bool setPoint(uint8_t r, uint16_t c, bool state);

  /**
   * Set all LEDs in a row to a new state on all devices.
   *
   * This method operates on all devices, setting the value of the LEDs in
   * the row to the specified value bit field. The method is useful for
   * drawing patterns and lines horizontally across on the entire display.
   * The least significant bit of the value is the lowest column number.
   *
   * \param r      row which is to be set [0..ROW_SIZE-1].
   * \param value  each bit set to 1 will light up the corresponding LED on each device.
   * \return false if parameter errors, true otherwise.
   */
  inline bool setRow(uint8_t r, uint8_t value) { return setRow(0, getDeviceCount()-1, r, value); };

  /**
   * Set all LEDs in a row to a new state on contiguous subset of devices.
   *
   * This method operates on a contiguous subset of devices, setting the value
   * of the LEDs in the row to the specified value bit field. The method is useful for
   * drawing patterns and lines horizontally across specific devices only.
   * endDev must be greater than or equal to startDev.
   * The least significant bit of the value is the lowest column number.
   *
   * \param startDev  the first device for the transformation [0..getDeviceCount()-1]
   * \param endDev    the last device for the transformation [0..getDeviceCount()-1]
   * \param r         row which is to be set [0..ROW_SIZE-1].
   * \param value     each bit set to 1 will light up the corresponding LED on each device.
   * \return false if parameter errors, true otherwise.
   */
  bool setRow(uint8_t startDev, uint8_t endDev, uint8_t r, uint8_t value);

  /**
   * Apply a transformation to the data in all the devices.
   *
   * The buffers for all devices can be transformed using one of the enumerated
   * transformations in transformType_t. The transformation is carried across
   * device boundaries (ie, there is overflow to an adjacent devices if appropriate).
   *
   * \param ttype  one of the transformation types in transformType_t.
   * \return false if parameter errors, true otherwise.
   */
  inline bool transform(transformType_t ttype) { return transform(0, getDeviceCount()-1, ttype); };

  /**
   * Apply a transformation to the data in contiguous subset of devices.
   *
   * The buffers for all devices in the subset can be transformed using one of the enumerated
   * transformations in transformType_t. The transformation is carried across
   * device boundaries (ie, there is overflow to an adjacent devices if appropriate).
   * endDev must be greater than or equal to startDev.
   *
   * \param startDev  the first device for the transformation [0..getDeviceCount()-1]
   * \param endDev    the last device for the transformation [0..getDeviceCount()-1]
   * \param ttype     one of the transformation types in transformType_t.
   * \return false if parameter errors, true otherwise.
   */
  bool transform(uint8_t startDev, uint8_t endDev, transformType_t ttype);

  /**
   * Turn auto display updates on or off.
   *
   * Turn auto updates on and off, as required. When auto updates are turned OFF the
   * display will not update after each operation. Display updates can be forced at any
   * time using using a call to update() with no parameters.
   *
   * This function is a convenience wrapper for the more general control() function call.
   *
   * \param mode  one of the types in controlValue_t (ON/OFF).
   * \return No return value.
   */
  void update(controlValue_t mode) { control(UPDATE, mode); };

  /**
   * Force an update of all devices
   *
   * Used when auto updates have been turned off through the control
   * method. This will force all buffered changes to be written to
   * all the connected devices.
   *
   * \return no return value.
   */
  void update(void) { flushBufferAll(); };

  /**
   * Turn display wraparound on or off.
   *
   * When shifting left or right, up or down, the outermost edge is normally lost and a blank
   * row or column inserted on the opposite side. If this options is enabled, the edge is wrapped
   * around to the opposite side.
   *
   * This function is a convenience wrapper for the more general control() function call.
   *
   * \param mode  one of the types in controlValue_t (ON/OFF).
   * \return No return value.
   */
  void wraparound(controlValue_t mode) { control(WRAPAROUND, mode); };
  /** @} */

  //--------------------------------------------------------------
  /** \name Methods for managing specific devices or display buffers.
   * @{
   */
  /**
   * Clear all display data in the specified buffer.
   *
   * \param buf   address of the buffer to clear [0..getDeviceCount()-1].
   * \return false if parameter errors, true otherwise.
   */
  bool clear(uint8_t buf);

  /**
   * Get the state of the LEDs in a specific column.
   *
   * This method operates on the specific buffer, returning the bit field value of
   * the LEDs in the column.
   *
   * \param buf   address of the display [0..getDeviceCount()-1].
   * \param c     column which is to be set [0..COL_SIZE-1].
   * \return uint8_t value with each bit set to 1 if the corresponding LED is lit. 0 is returned for parameter error.
   */
  uint8_t getColumn(uint8_t buf, uint8_t c);

  /**
   * Get the state of the LEDs in a specified row.
   *
   * This method operates on the specific buffer, returning the bit field value of
   * the LEDs in the row.
   *
   * \param buf   address of the display [0..getDeviceCount()-1].
   * \param r     row which is to be set [0..ROW_SIZE-1].
   * \return uint8_t value with each bit set to 1 if the corresponding LED is lit. 0 is returned for parameter error.
   */
  uint8_t getRow(uint8_t buf, uint8_t r);

  /**
   * Set all LEDs in a column to a new state.
   *
   * This method operates on a specific buffer, setting the value of the LEDs in
   * the column to the specified value bit field. The method is useful for
   * drawing patterns and lines vertically on the display device.
   * The least significant bit of the value is the lowest column number.
   *
   * \param buf   address of the display [0..getDeviceCount()-1].
   * \param c     column which is to be set [0..COL_SIZE-1].
   * \param value each bit set to 1 will light up the	corresponding LED.
   * \return false if parameter errors, true otherwise.
   */
  bool setColumn(uint8_t buf, uint8_t c, uint8_t value);

  /**
   * Set all LEDs in a row to a new state.
   *
   * This method operates on a specific device, setting the value of the LEDs in
   * the row to the specified value bit field. The method is useful for
   * drawing patterns and lines horizontally across the display device.
   * The least significant bit of the value is the lowest row number.
   *
   * \param buf   address of the display [0..getDeviceCount()-1].
   * \param r     row which is to be set [0..ROW_SIZE-1].
   * \param value each bit set to 1 within this byte will light up the corresponding LED.
   * \return false if parameter errors, true otherwise.
   */
  bool setRow(uint8_t buf, uint8_t r, uint8_t value);

  /**
   * Apply a transformation to the data in the specified device.
   *
   * The buffer for one device can be transformed using one of the enumerated
   * transformations in transformType_t. The transformation is limited to the
   * nominated device buffer only (ie, there is no overflow to an adjacent device).
   *
   * \param buf    address of the display [0..getBufferCount()-1].
   * \param ttype  one of the transformation types in transformType_t.
   * \return false if parameter errors, true otherwise.
   */
  bool transform(uint8_t buf, transformType_t ttype);

  /**
   * Force an update of one buffer.
   *
   * Used when auto updates have been turned off through the control()
   * method. This will force all buffered display changes to be written to
   * the specified device at the same time.
   * Note that control() messages are not buffered but cause immediate action.
   *
   * \param buf address of the display [0..getBufferCount()-1].
   * \return No return value.
   */
  void update(uint8_t buf) { flushBuffer(buf); };
  /** @} */

#if USE_LOCAL_FONT
  //--------------------------------------------------------------
  /** \name Methods for font and characters.
   * @{
   */
  /**
   * Load a character from the font data into a user buffer.
   *
   * Copy the bitmap for a library font character (current font set by setFont()) and
   * return it in the data area passed by the user. If the user buffer is not large
   * enough, only the first size elements are copied to the buffer.
   *
   * NOTE: This function is only available if the library defined value
   * USE_LOCAL_FONT is set to 1.
   *
   * \param c     the character to retrieve.
   * \param size  the size of the user buffer in unit8_t units.
   * \param buf   address of the user buffer supplied.
   * \return width (in columns) of the character, 0 if parameter errors.
   */
  uint8_t getChar(uint16_t c, uint8_t size, uint8_t *buf);

  /**
   * Load a character from the font data starting at a specific column.
   *
   * Load a character from the font table directly into the display at the column
   * specified. The currently selected font table is used as the source.
   *
   * NOTE: This function is only available if the library defined value
   * USE_LOCAL_FONT is set to 1.
   *
   * \param col column of the display in the range accepted [0..getColumnCount()-1].
   * \param c   the character to display.
   * \return width (in columns) of the character, 0 if parameter errors.
   */
  uint8_t setChar(uint16_t col, uint16_t c);

  /**
   * Set the current font table.
   *
   * Font data is stored in PROGMEM, in the format described elsewhere in the
   * documentation. All characters retrieved or used after this call will use
   * the nominated font (default or user defined). To specify a user defined
   * character set, pass the PROGMEM address of the font table. Passing a nullptr
   * resets the font table to the library default table.
   *
   * NOTE: This function is only available if the library defined value
   * USE_LOCAL_FONT is set to 1.
   *
   * \param f fontType_t pointer to the table of font data in PROGMEM or nullptr.
   * \return false if parameter errors, true otherwise.
   */
  bool setFont(fontType_t *f);

  /**
  * Get the maximum width character for the font.
  *
  * Returns the number of columns for the widest character in the currently
  * selected font table. Useful to allocated buffers of the right size before
  * loading characters from the font table.
  *
  * NOTE: This function is only available if the library defined value
  * USE_LOCAL_FONT is set to 1.
  *
  * \return number of columns (width) for the widest character.
  */
  uint8_t getMaxFontWidth(void) { return(_fontInfo.widthMax); };

  /**
  * Get height of a character for the font.
  *
  * Returns the number of rows specified as the height of a character in the
  * currently selected font table.
  *
  * NOTE: This function is only available if the library defined value
  * USE_LOCAL_FONT is set to 1.
  *
  * \return number of rows (height) for the font.
  */
  uint8_t getFontHeight(void) { return(_fontInfo.height); };

  /**
   * Get the pointer to current font table.
   *
   * Returns the pointer to the current font table. Useful if user code needs
   * to replace the current font temporarily and then restore previous font.
   *
   * NOTE: This function is only available if the library defined value
   * USE_LOCAL_FONT is set to 1.
   *
   * \return pointer to the start of the font table in PROGMEM.
   */
  fontType_t *getFont(void) { return(_fontData); };
#endif // USE_LOCAL_FONT
  /** @} */

private:
  typedef struct
  {
  uint8_t dig[ROW_SIZE];  // data for each digit of the MAX72xx (DIG0-DIG7)
  uint8_t changed;        // one bit for each digit changed ('dirty bit')
  } deviceInfo_t;

  // LED module wiring parameters defined by hardware type
  moduleType_t _mod;  // The module type from the available list
  bool _hwDigRows;    // MAX72xx digits are mapped to rows in on the matrix
  bool _hwRevCols;    // Normal orientation is col 0 on the right. Set to true if reversed
  bool _hwRevRows;    // Normal orientation is row 0 at the top. Set to true if reversed

  uint8_t _dataPin;     // DATA is shifted out of this pin ...
  uint8_t _clkPin;      // ... signaled by a CLOCK on this pin ...
  uint8_t _csPin;       // ... and LOADed when the chip select pin is driven HIGH to LOW
  bool    _hardwareSPI; // true if SPI interface is the hardware interface
  SPIClass& _spiRef;    // reference to the SPI object to use for hardware comms

  // Device buffer data
  uint8_t _maxDevices;  // maximum number of devices in use
  deviceInfo_t* _matrix;// the current status of the LED matrix (buffers)
  uint8_t*  _spiData;   // data buffer for writing to SPI interface

  // User callback function for shifting operations
  uint8_t (*_cbShiftDataIn)(uint8_t dev, transformType_t t);
  void    (*_cbShiftDataOut)(uint8_t dev, transformType_t t, uint8_t colData);

  // Control data for the library
  bool    _updateEnabled; // update the display when this is true, suspend otherwise
  bool    _wrapAround;    // when shifting, wrap left to right and vice versa (circular buffer)

  // SPI interface data
#if defined(__MBED__) && !defined(ARDUINO)
  SPI   _spi;           // Mbed SPI object
  DigitalOut _cs;
#endif

#if USE_LOCAL_FONT
  // Font properties info structure
   typedef struct
   {
     uint8_t version;     // (v1) font definition version number (for compliance)
     uint8_t height;      // (v1) font height in pixels
     uint8_t widthMax;    // (v1) font maximum width in pixels (widest character)
     uint16_t firstASCII; // (v1,2) the first character code in the font table
     uint16_t lastASCII;  // (v1,2) the last character code in the font table
     uint16_t dataOffset; // (v1) offset from the start of table to first character definition
   } fontInfo_t;

  // Font related data
  fontType_t  *_fontData;   // pointer to the current font data being used
  fontInfo_t  _fontInfo;    // properties of the current font table

  void    setFontInfoDefault(void);      // set the default parameters for the font info file
  void    loadFontInfo(void);            // load the font info block from the font data
  uint8_t getFontWidth(void);            // get the maximum font width by inspecting the font table
  int32_t getFontCharOffset(uint16_t c); // find the character in the font data. If not there, return -1
#endif

  // Private functions
  void spiSend(void);         // do the actual physical communications task
  inline void spiClearBuffer(void);  // clear the SPI send buffer
  void controlHardware(uint8_t dev, controlRequest_t mode, int value);  // set hardware control commands
  void controlLibrary(controlRequest_t mode, int value);  // set internal control commands

  void flushBuffer(uint8_t buf);  // determine what needs to be sent for one device and transmit
  void flushBufferAll(void);      // determine what needs to be sent for all devices and transmit

  uint8_t bitReverse(uint8_t b);  // reverse the order of bits in the byte
  bool transformBuffer(uint8_t buf, transformType_t ttype); // internal transform function

  bool copyRow(uint8_t buf, uint8_t rSrc, uint8_t rDest);   // copy a row from Src to Dest
  bool copyColumn(uint8_t buf, uint8_t cSrc, uint8_t cDest);// copy a row from Src to Dest

  void setModuleParameters(moduleType_t mod);   // setup parameters based on module type

  // _hwDigRev switched function for internal use
  bool copyC(uint8_t buf, uint8_t cSrc, uint8_t cDest);
  bool copyR(uint8_t buf, uint8_t rSrc, uint8_t rDest);
  uint8_t getC(uint8_t buf, uint8_t c);
  uint8_t getR(uint8_t buf, uint8_t r);
  bool setC(uint8_t buf, uint8_t c, uint8_t value);
  bool setR(uint8_t buf, uint8_t r, uint8_t value);

};
