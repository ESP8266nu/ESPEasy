/**
   \mainpage Main Page
   The Parola Library
   ------------------
   The Parola library is implemented to work with the MD_MAX2XX library. It
   depends on the MD_MAX72xx library for hardware control and will run on all
   hardware supported by that library. The MD_MAX72XX library can be found
   [here] (http://github.com/MajicDesigns/MAX72xx).

   This software library implements functions to simplify the implementation
   of text special effects on the Parola display.
   - Left, right or center text justification in the display field.
   - Text scrolling, text entering and exit effects.
   - Control display parameters and animation speed.
   - Multiple virtual displays (zones) in each string of LED modules.
   - User defined fonts and/or individual characters substitutions.
   - Double height and vertical displays.
   - Support for mixing text and graphics on the same display.

   The latest copy of the Parola Software and hardware files can be found
   at the [Parola distribution site] (http://github.com/MajicDesigns/Parola).

   ![Parola Display with 8 modules connected] (Working_Display.jpg "Working System")

   System Components
   -----------------
- Hardware - documentation for supported hardware found in the [MD_MAX72xx library] (http://github.com/MajicDesigns/MD_MAX72XX) documentation.
   - \subpage pageSoftware
   - \subpage pageRevHistory
   - \subpage pageCopyright
   - \subpage pageDonation

   Parola A-to-Z Blog Articles
   ---------------------------
   - [RAM Requirements] (https://arduinoplusplus.wordpress.com/2017/08/27/parola-a-to-z-ram-requirements/)
   - [Adapting for Different Hardware] (https://arduinoplusplus.wordpress.com/2017/04/14/parola-a-to-z-adapting-for-different-hardware/)
   - [Defining Fonts] (https://arduinoplusplus.wordpress.com/2016/11/08/parola-fonts-a-to-z-defining-fonts/)
   - [Managing Fonts] (https://arduinoplusplus.wordpress.com/2016/11/13/parola-fonts-a-to-z-managing-fonts/)
   - [UTF-8 Characters] (https://arduinoplusplus.wordpress.com/2020/03/21/parola-a-to-z-handling-non-ascii-characters-utf-8/})
   - [Text Animation] (https://arduinoplusplus.wordpress.com/2017/02/10/parola-a-to-z-text-animation/)
   - [Managing Animation] (https://arduinoplusplus.wordpress.com/2017/03/02/parola-a-to-z-managing-animation/)
   - [Double Height Displays] (https://arduinoplusplus.wordpress.com/2017/03/15/parola-a-to-z-double-height-displays/)
   - [Multi Zone Displays] (https://arduinoplusplus.wordpress.com/2017/04/18/parola-a-to-z-multi-zone-displays/)
   - [Vertical Displays] (https://arduinoplusplus.wordpress.com/2017/07/22/parola-a-to-z-vertical-displays/)
   - [Mixing Text and Graphics] (https://arduinoplusplus.wordpress.com/2018/03/29/parola-a-to-z-mixing-text-and-graphics/)
   - [Sprite Text Effects] (https://arduinoplusplus.wordpress.com/2018/04/19/parola-a-to-z-sprite-text-effects/)
   - [Optimizing Flash Memory] (https://arduinoplusplus.wordpress.com/2018/09/23/parola-a-to-z-optimizing-flash-memory/)

   \page pageDonation Support the Library
   If you like and use this library please consider making a small donation using [PayPal](https://paypal.me/MajicDesigns/4USD)

   \page pageRevHistory Revision History
  // 2021-04-27, tonhuisman: added constant values textEffect_t enum to retain settings when enabling/disabling compile-time options
May 2010 - version 3.5.7
- Fixed issues with text occasionally remaining on edge of display for diagonal scroll effects

Mar 2021 - version 3.5.6
- Added TG_Combo, TG_Coord and TG_Zones examples
- Deleted Test_TG example
- Fixed ENA_* no longer working (compiler changes)
- Deleted revision history prior to v2.0.0

   Dec 2020 - version 3.5.5
   - Fixed erratic zone scrolling behavior in introduced with changes to 3.5.3
   - Added README.md to examples folder

   Oct 2020 - version 3.5.4
   - Fixed typo in HelloWorld example

   Oct 2020 - version 3.5.3
   - Fixed behavior of SCROLL_UP, DOWN, UP_LEFT, UP_RIGHT, DOWN_LEFT, DOWN_RIGHT for inverted display

   Aug 2020 - version 3.5.2
   - Fixed ambiguous overloading setSpeed(uint8_t zone, uint16_t spd) and setSpeed(uint16_t spdIn, uint16_t spdOut)

   Aug 2020 - version 3.5.1
   - Fixed non-functional setIntensity()

   Aug 2020 - version 3.5.0
   - setSpeed() now allows setting independent IN and OUT speed
   - Added getZone() method
   - Added Animation_2Speed example
   - Workaround for issue #56 (https://github.com/MajicDesigns/MD_Parola/issues/56) built into library
   - Maintenance release for new (more picky) IDE compiler settings

   Aug 2020 - version 3.4.0
   - Updated parts of documentation
   - Exposed getTextColumns() as public

   Oct 2019 - version 3.3.0
   - Reverted back to dynamic zone allocation removed in v2.6.6. Tested 22 zones seems ok.
   STATIC_ZONES defined value will immediately switch back to static zones.

   Aug 2019 - version 3.2.0
   - Changed to use 16 bit character code
   - Checked all examples for clean compile with current version

   Jun 2019 - version 3.1.1
   - Use const char* parameter instead of char* parameter.
   - Some compiler warnings fixed.
   - Uninitialized variables fixed.

   Dec 2018 - version 3.1.0
   - Fixed issues caused by inter char spacing fix in v3.0.2.

   Dec 2018 - version 3.0.2
   - Fixed another compile error with ESP8266.
   - Added Double_Scoreboard example.
   - Stopped inter char spacing with zero length character

   Jul 2018 - version 3.0.1
   - Added getFont() method.
   - Cleaned up double height examples relying on old USE_*_HW defines.

   Jun 2018 - version 3.0.0
   - Minor corrections to previous version examples and keyword.txt.
   - Added help text on sprite animations and minor doc updates.
   - Added Sprites_Simple example.
   - Adapted for changes to the MD_MAX72XX library.

   Apr 2018 - version 2.7.4
   - Fixed bug with ESP8266 compilation.

   Apr 2018 - version 2.7.3
   - Reworked sprite effects to enable user sprites as more sustainable in long run.
   - Removed all built in sprites from previous version.
   - Added Sprites example.

   Apr 2018 - version 2.7.2
   - Fixed bug with last text column persisting for PA_SCAN_HORIZ and PA_SCAN_HORIZX effect.
   - Added sprite based text effects

   Mar 2018 - version 2.7.1
   - Adjusted Scrolling_Menu example for changes to MD_Menu library.
   - Added graphics methods to support text + graphics displays
   - Added text & graphics example Test_TG

   Dec 2017 - version 2.6.6
   - Created MAX_ZONES constant to allow static zones array. Interim measure until resolution
   of the errors (?) caused when dynamically allocating the _Z array.
   - Cleaned up most compiler warnings.
   - Reworked Parola_Test example

   Nov 2017 - version 2.6.5
   - Fixed RANDOM effect locking issue
   - Added Parola_Bluetooth example
   - Added Double_Height_Russian example
   - Added Scrolling_Menu example
   - Added Scrolling_Vertical example
   - Added SCAN_HORIZX/SCAN_VERTX text effects
   - Fixed bug with WIPE, SCAN and GROW when using 1 module only
   - Removed hard coded internal font buffer size
   - Changed examples - replaced MD_KeySwitch with new MD_UISwitch library

   Apr 2017 - version 2.6.4
   - Added Parola_UFT-8_Display example for double multi-byte character handling
   - Fixed bug: single blank column in SCROLL_LEFT with PA_LEFT
   - Changed Scrolling example to parametrize more options
   - Added RANDOM text effect

   Feb 2017 - version 2.6.3
   - Full review of double height functionality, mods as required
   - Added Double_Height_Test example
   - Cleaned up some example files

   Jan 2017 - version 2.6.2
   - Added shutdown() method to enable low power mode
   - Corrected alignment offset problems in low level functions

   Jan 2017 - version 2.6.1
   - Added ESP8266 example
   - Corrected keywords.txt file and main header file
   - Cleaned up looping logic in Double_Height examples
   - Corrected bug when double height PA_SCROLL_RIGHT

   Dec 2016 - version 2.6
   - Prefaced all Parola enumerated types with PA_ as some clash with other libraries
   - Edited main library core routines to allow scrolling in double height mode
   - Eliminated trailing char separator from displayed string
   - Adjusted some animations to allow for changes to core utility functions
   - Changed Double_Height_v2 example to allow for Generic and Parola modules
   - Fixed font definitions for font based examples
   - Added Arduino Library Print Class extension for easy printing

   Nov 2016 - version 2.5
   - Added ambulance example
   - Updated branding to MD_ diamond
   - Added README.md file

   Jan 2016 - version 2.4
   - Added dynamic zone example
   - Added synchZoneStart() method to allow zones start times to be synchronized
   - Added double height character example by Arek00
   - Modified all examples to conditionally include <SPI.h>
   - Added double height character example (v2) using Font file created by MD_MAX72xx font builder
   - Added double height clock example
   - Added HelloWorld example - simplest working code
   - Added FADE animation
   - Adjusted documentation structure
   - Added preprocessor defines ENA_* for granular selection of animations (memory saving)

   Aug 2015 - version 2.3
   - Added set/getScrollSpacing() methods and associated Scrolling_Spacing example
   - Added set/getZoneEffect with FLIP_LR and FLIP_UD, with associated Zone_Mirror example
   - Added MESH animation
   - Fixed minor bugs and documentation

   April 2015 - version 2.2
   - Added Scrolling_ML example
   - Added Zone_Mesg example
   - Overloaded displayClear() for single zone clear
   - Fixed bug in SLICE effect when text was too long for display

   February 2015 - version 2.1
   - Fixed small animation problems with SLICE and SCAN_VERT
   - Fixed PROGMEM error compiling with IDE version 1.5.7

   March 2014 - version 2.0
   - Mods to accommodate revised font handling in MD_MAX72xx library
 + Users can now provide a user defined font PROGMEM data table
 + User code can provide individual character override for equivalent font character
   - Additional animations
 + SCAN_HORIZ, SCAN_VERT
 + GROW_UP, GROW_DOWN
 + SCROLL_UP_LEFT, SCROLL_UP_RIGHT, SCROLL_DOWN_LEFT, SCROLL_DOWN_RIGHT
   - Implemented Zoned scrolling
 + Multiple independent zoned scrolling areas in one display.
 + Each zone has all character attributes (font, alignment, speed, pause, etc).
   - textEffect_t and textAlign_t taken out of the class definition. Requires legacy code mods.
   - Backward compatible with library version 1.x code.

   September 2013 - version 1.1
   - Mods to accommodate changes to hardware SPI implementation in MD_MAX72xx library

   June 2013 - version 1.0
   - New library

   \page pageCopyright Copyright
   Copyright (C) 2013-2018 Marco Colli. All rights reserved.

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

   \page pageSoftware Parola Library
   The Library
   -----------
   The Parola library is implemented using the MD_MAX72xx library for hardware
   control. The library implements functions to simplify the implementation
   of text special effects on the LED matrix.
   - Text left, right or center justification in the display
   - Text scrolling, appearance and disappearance effects
   - Control display parameters and animation speed
   - Support for hardware SPI interface
   - Multiple virtual displays (zones) in each string of LED modules
   - User defined fonts and/or individual characters substitutions

 ### External Dependencies
   - Parola uses the MD_MAX72xx library for hardware level control primitives.
   The latest copy of this library can be found
   [here] (http://github.com/MajicDesigns/MAX72xx).

   ___

   Display Zones
   -------------
   A matrix display can be treated as a single contiguous set of modules or it can be
   split into multiple 'virtual' displays (zones). Prior to version 2.0 of the library,
   each display was effectively a single zone.

   A zone is a contiguous subset of one or more display modules (LED matrices) that has all
   the attributes of a display - animation, speed, font, spacing, etc. This allows complex
   displays to be created. For example, one part can show relatively static text while a
   different one has animation and movement.

   For backward compatibility, all the methods from version 1 remain. If the new library
   is compiled with older user source code, the library defaults to using a single zone
   for the whole display. Zone-aware functions have an added parameter to specify the zone
   to which the method invocation applies. Methods invoked without specifying a zone (such
   as set*()) usually have their effect applied to all zones.

 ### More Information
   - [Parola A to Z - Multi Zone Displays] (https://arduinoplusplus.wordpress.com/2017/04/18/parola-a-to-z-multi-zone-displays/)
   - [Parola A to Z - Double Height Displays] (https://arduinoplusplus.wordpress.com/2017/03/15/parola-a-to-z-double-height-displays/)

   ___

   Fonts
   -----
   The standard MD_MAX72xx library font can be substituted with a user font definition conforming
   to the font encoding rules in the MD_MAX72XX documentation. New fonts can be designed with the
   the MD_MAX72xx font builder.

   Each zone can have its own substituted font. The default font can be reselected for the zone by
   specifying a nullptr font table pointer.

 ### More Information
   - [Parola A to Z - Defining Fonts] (https://arduinoplusplus.wordpress.com/2016/11/08/parola-fonts-a-to-z-defining-fonts/)
   - [Parola A to Z - Managing Fonts] (https://arduinoplusplus.wordpress.com/2016/11/13/parola-fonts-a-to-z-managing-fonts/)
   - [Parola A to Z - Optimizing Flash Memory] (https://arduinoplusplus.wordpress.com/2018/09/23/parola-a-to-z-optimizing-flash-memory/)

   ___

   User Characters
   ---------------
   Individual characters can be substituted for user character definitions. These can be added and
   deleted to individual zones as required.

   The character data is the same format as a single character from the font definition file,
   and is held in a local lookup table that is parsed before loading the defined font character.
   If a character is specified with a code the same as an existing character, the existing data
   will be substituted for the new data.

   ASCII 0 character ('\0') cannot be substituted as this denotes the end of string character
   for C++ and cannot be used in an actual string.

   The library only retains a pointer to the user data definition, so the data must remain in scope.
   Also, any changes to the data storage in the calling program will be reflected by the library the
   next time the character is used.

 ### More Information
   - [Parola A to Z - Defining Fonts] (https://arduinoplusplus.wordpress.com/2016/11/08/parola-fonts-a-to-z-defining-fonts/)
   - [Parola A to Z - Managing Fonts] (https://arduinoplusplus.wordpress.com/2016/11/13/parola-fonts-a-to-z-managing-fonts/)
   - [PArola A to Z - Handling non-ASCII (UTF-8) Characters]
    ###(https://arduinoplusplus.wordpress.com/2020/03/21/parola-a-to-z-handling-non-ascii-characters-utf-8/)

   ___

   Sprite Text Effect
   ------------------
   The PA_SPRITE text effect requires additional information, as it extends the functionality
   of the library to include fully customizable, user defined, animated bitmaps to wipe text
   on and off the LED matrix display.

   Each frame is defined by a sequence of numbers that encode the columns of the bitmap. The
   least significant bit is at the top of the bitmap. If the sprite has a front and rear, the
   bitmap should be defined for the sprite moving to the right. The library will mirror reverse
   the image when it moves left. The sprites are essentially defined in the same way as the
   character font and the same tools can be used to define the data for the sprite bitmap.

   A sprite has at least one frame. If more than one frame is required, a similar definition is
   created for each frame of the animation, and a data table constructed defining the animated
   sprite. To ensure smooth animations, remember that once the last frame is reached, it will loop
   back to the first, so avoid discontinuities between the two ends of the data table.

   The library is given the sprite definition setSpriteData() method and the text effect is
   specified using the effect id PA_SPRITE.

 ### More Information
   - [Parola A to Z - Sprite Text Effects] (https://arduinoplusplus.wordpress.com/2018/04/19/parola-a-to-z-sprite-text-effects/)

   ___

   Conditional Compilation Switches
   --------------------------------
   The library allows the run time code to be tailored through the use of compilation
   switches. The compile options start with ENA_ and are documented in the section
   related to the main header file MD_Parola.h.

   _NOTE_: Compile switches must be edited in the library header file. Arduino header file
   'mashing' during compilation makes the setting of these switches from user code
   completely unreliable.

 ### More Information
   - [Parola A to Z - Optimizing Flash Memory] (https://arduinoplusplus.wordpress.com/2018/09/23/parola-a-to-z-optimizing-flash-memory/)

   ___

   Implementing New Text Effects
   -----------------------------
   Each of the selected text effects is implemented as a function. This makes it easy to add new
   effects:
   - Choose a name for the effect and add it to the textEffect_t enumerated type.
   - Clone an existing method and modify it according to the guidelines below.
   - Add the function prototype for the new effect to the MD_PZone class definition in the MD_Parola.h file.
   - Modify the zoneAnimate() method in MD_PZone.cpp to invoke the new method.

 ###New Text Effects
   The effects functions are implemented as finite state machines that are called with the
   frequency set by the setSpeed() method. The class variable _fsmState holds the state from
   the last invocation of an effect method.

   An effect method can work in one of 2 ways:
   - *Additive*: where the animation frames are incrementally built up to the initial display.
   With this method, the function will need to use the getFirstChar() and getNextChar() methods
   to build up the displayed text, column by column.
   - *Subtractive*: where the final displayed text is placed in the buffer using the commonPrint()
   method and the elements that are not visible at that stage of the animation are removed.

   Which algorithm is used depends on the type animation and what is convenient for the coder.
   Examples of both are found in the supplied library text effects.

   Each effect method is implemented in 2 parts. One part implements the text move IN to the display
   (method parameter bIn is true) and the other when the text is moving OUT of the display (bIn false).
   Because the IN and OUT effects can be different for a display cycle, the method must not assume
   that the first part was ever called. The first phase should always end with the text in its
   display position (depending on the alignment specified) and the second phase should assume the text
   is in that position when called. Text position parameters are held in the class variables _limitLeft and
   _limitRight found in the library header file.

   The first phase starts with _fsmState set to INITIALISE and ends when the state is set to PAUSE
   within the effect method. The second phase starts with a PAUSE state and ends when the state is
   set to END by the method.  Aside from the INITIALISE state (set by the displayReset() method),
   all other state changes are under the control of the effect functions. Delays between frames and
   the pause between IN and OUT are handled outside of the effect method.

 ### More Information
   - [Parola A to Z - Text Animation] (https://arduinoplusplus.wordpress.com/2017/02/10/parola-a-to-z-text-animation/)
   - [Parola A to Z - Managing Animation] (https://arduinoplusplus.wordpress.com/2017/03/02/parola-a-to-z-managing-animation/)

   ___

   Coding Tips
   -----------
 + The MD_MAX72XX library sets the origin for the LED matrix at the top right of the display. This
   makes the leftmost text column a higher column number that the far right column. Sometimes this
   is not intuitive when coding and is worth remembering. Rows are numbered from top to bottom, 0-7.

 + Ensure that a new effect is tested in combination with other effects to make sure that transitions
   are smooth and the IN and OUT effects combine well. Common errors are misaligned entry compared to
   exit, with causes a small jump in the text position when the effects are combined.

 + Display update times grow proportionally with the number of modules in a display, so some timing
   parameters may need to adapt. Hardware SPI runs approximately 10 times faster and the delay
   increase is not appreciable with up to 12 modules. For the arbitrary pin outs, using
   shiftout(), a 6 module chain updates in approximately 14ms on an Uno, while a 12 module display
   takes around 25ms. Most of the time taken is to physically update the display, as animating frames
   takes about 1-2ms to update in the MD_MAX72XX display buffers.
 */
#pragma once

#include <Arduino.h>
#include <MD_MAX72xx.h>

/**
 * \file
 * \brief Main header file for the MD_Parola library
 */

// Granular selection of animations/functions to include in the library
// If an animation class is not used at all some memory savings can be made
// by excluding the animation code.

#ifndef ENA_MISC
# define ENA_MISC    1 ///< Enable miscellaneous animations
#endif // ifndef ENA_MISC
#ifndef ENA_WIPE
# define ENA_WIPE    1 ///< Enable wipe type animations
#endif // ifndef ENA_WIPE
#ifndef ENA_SCAN
# define ENA_SCAN    1 ///< Enable scanning animations
#endif // ifndef ENA_SCAN
#ifndef ENA_SCR_DIA
# define ENA_SCR_DIA 1 ///< Enable diagonal scrolling animation
#endif // ifndef ENA_SCR_DIA
#ifndef ENA_OPNCLS
# define ENA_OPNCLS  1 ///< Enable open and close scan effects
#endif // ifndef ENA_OPNCLS
#ifndef ENA_GROW
# define ENA_GROW    1 ///< Enable grow effects
#endif // ifndef ENA_GROW
#ifndef ENA_SPRITE
# define ENA_SPRITE  1 ///< Enable sprite effects
#endif // ifndef ENA_SPRITE

// If function is not used at all, then some memory savings can be made
// by excluding associated code.
#ifndef ENA_GRAPHICS
# define ENA_GRAPHICS  1 ///< Enable graphics functionality
#endif // ifndef ENA_GRAPHICS

// Miscellaneous defines
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0])) ///< Generic macro for obtaining number of elements of an array
#define STATIC_ZONES 0                             ///< Developer testing flag for quickly flipping between static/dynamic zones

#if STATIC_ZONES
# ifndef MAX_ZONES
#  define MAX_ZONES 4 ///< Maximum number of zones allowed. Change to allow more or less zones but uses RAM even if not used.
# endif // ifndef MAX_ZONES
#endif // if STATIC_ZONES

// Zone column calculations
#define ZONE_START_COL(m) ((m) * COL_SIZE)           ///< The first column of the first zone module
#define ZONE_END_COL(m) ((((m) + 1) * COL_SIZE) - 1) ///< The last column of the last zone module

class MD_Parola;

/**
 * Text alignment enumerated type specification.
 *
 * Used to define the display text alignment and to specify direction for
 * scrolling and animations. In the situation where LEFT AND RIGHT are the only sensible
 * options (eg, text scrolling direction), CENTER will behave the same as LEFT.
 */
enum textPosition_t
{
  PA_LEFT,   ///< The leftmost column for the first character will be on the left side of the display
  PA_CENTER, ///< The text will be placed with equal number of blank display columns either side
  PA_RIGHT   ///< The rightmost column of the last character will be on the right side of the display
};

/**
 * Text effects enumerated type specification.
 *
 * Used to define the effects to be used for the entry and exit of text in the display area.
 */

// 2021-04-27, tonhuisman added constant values to retain settings when enabling/disabling compile-time options
enum textEffect_t
{
  PA_NO_EFFECT    = 0,       ///< Used as a place filler, executes no operation
  PA_PRINT        = 1,       ///< Text just appears (printed)
  PA_SCROLL_UP    = 2,       ///< Text scrolls up through the display
  PA_SCROLL_DOWN  = 3,       ///< Text scrolls down through the display
  PA_SCROLL_LEFT  = 4,       ///< Text scrolls right to left on the display
  PA_SCROLL_RIGHT = 5,       ///< Text scrolls left to right on the display
#if ENA_SPRITE
  PA_SPRITE = 6,             ///< Text enters and exits using user defined sprite
#endif // if ENA_SPRITE
#if ENA_MISC
  PA_SLICE    = 7,           ///< Text enters and exits a slice (column) at a time from the right
  PA_MESH     = 8,           ///< Text enters and exits in columns moving in alternate direction (U/D)
  PA_FADE     = 9,           ///< Text enters and exits by fading from/to 0 and intensity setting
  PA_DISSOLVE = 10,          ///< Text dissolves from one display to another
  PA_BLINDS   = 11,          ///< Text is replaced behind vertical blinds
  PA_RANDOM   = 12,          ///< Text enters and exits as random dots
#endif // ENA_MISC
#if ENA_WIPE
  PA_WIPE        = 13,       ///< Text appears/disappears one column at a time, looks like it is wiped on and off
  PA_WIPE_CURSOR = 14,       ///< WIPE with a light bar ahead of the change
#endif  // ENA_WIPES
#if ENA_SCAN
  PA_SCAN_HORIZ  = 15,       ///< Scan the LED column one at a time then appears/disappear at end
  PA_SCAN_HORIZX = 16,       ///< Scan a blank column through the text one column at a time then appears/disappear at end
  PA_SCAN_VERT   = 17,       ///< Scan the LED row one at a time then appears/disappear at end
  PA_SCAN_VERTX  = 18,       ///< Scan a blank row through the text one row at a time then appears/disappear at end
#endif // ENA_SCAN
#if ENA_OPNCLS
  PA_OPENING        = 19,    ///< Appear and disappear from the center of the display, towards the ends
  PA_OPENING_CURSOR = 20,    ///< OPENING with light bars ahead of the change
  PA_CLOSING        = 21,    ///< Appear and disappear from the ends of the display, towards the middle
  PA_CLOSING_CURSOR = 22,    ///< CLOSING with light bars ahead of the change
#endif // ENA_OPNCLS
#if ENA_SCR_DIA
  PA_SCROLL_UP_LEFT    = 23, ///< Text moves in/out in a diagonal path up and left (North East)
  PA_SCROLL_UP_RIGHT   = 24, ///< Text moves in/out in a diagonal path up and right (North West)
  PA_SCROLL_DOWN_LEFT  = 25, ///< Text moves in/out in a diagonal path down and left (South East)
  PA_SCROLL_DOWN_RIGHT = 26, ///< Text moves in/out in a diagonal path down and right (North West)
#endif // ENA_SCR_DIA
#if ENA_GROW
  PA_GROW_UP   = 27,         ///< Text grows from the bottom up and shrinks from the top down
  PA_GROW_DOWN = 28,         ///< Text grows from the top down and and shrinks from the bottom up
#endif // ENA_GROW
};

/**
 * Zone effect enumerated type specification.
 *
 * Used to define the effects to be used for text in the zone.
 *
 * The FLIP_UD and FLIP_LR effects are specifically designed to allow rectangular shaped display
 * modules (like Parola or Generic types) to be placed in an inverted position to allow all matrices
 * to be tightly packed into a 2 line display. One of the lines must be flipped horizontally and
 * vertically to remain legible in this configuration.
 */
enum zoneEffect_t
{
  PA_FLIP_UD, ///< Flip the zone Up to Down (effectively upside down). Works with all textEffect_t values
  PA_FLIP_LR, ///< Flip the zone Left to Right (effectively mirrored). Does not work with textEffect_t types SLICE, SCROLL_LEFT,
              // SCROLL_RIGHT
};

/**
 * Zone object for the Parola library.
 * This class contains the text to be displayed and all the attributes for the zone.
 */
class MD_PZone {
public:

  /**
   * Class constructor.
   *
   * Instantiate a new instance of the class.
   */
  MD_PZone(void);

  /**
   * Initialize the object.
   *
   * Initialize the object data. This will be called to initialize
   * new data for the class that cannot be done during the object creation.
   *
   * \param p pointer to the parent's MD_MAX72xx object.
   */
  void begin(MD_MAX72XX *p);

  /**
   * Class Destructor.
   *
   * Release allocated memory and does the necessary to clean up once the object is
   * no longer required.
   */
  ~MD_PZone(void);

  // --------------------------------------------------------------

  /** \name Methods for core object control.
   * @{
   */
  /**
   * Animate the zone.
   *
   * Animate using the currently specified text and animation parameters.
   * This method is invoked from the main Parola object.
   *
   * \return bool true if the zone animation has completed, false otherwise.
   */
  bool zoneAnimate(void);

  /**
   * Get the completion status.
   *
   * Return the current completion status for the zone animation.
   *
   * See comments for the MD_Parola getZoneStatus() method.
   *
   * \return bool true if the zone animation is completed
   */
  bool getStatus(void) {
    return _fsmState == END;
  }

  /**
   * Get the start and end parameters for a zone.
   *
   * See comments for the MD_Parola namesake method.
   *
   * \param zStart  value for the start module number placed here [0..numZones-1].
   * \param zEnd  value for the end module number placed here [0..numZones-1].
   */
  inline void getZone(uint8_t& zStart, uint8_t& zEnd) {
    zStart = _zoneStart; zEnd = _zoneEnd;
  }

  /**
   * Check if animation frame has advanced.
   *
   * Check if the last call to zoneAnimate() resulted in the animation frame advancing by
   * one or more frames in one or more zones.
   *
   * \return True if the animation frame advanced in any of the display zones.
   */
  bool isAnimationAdvanced(void) {
    return _animationAdvanced;
  }

  /**
   * Clear the zone.
   *
   * See comments for the MD_Parola namesake method.
   *
   * \return No return value.
   */
  void zoneClear(void) {
    _MX->clear(_zoneStart, _zoneEnd);

    if (_inverted) { _MX->transform(_zoneStart, _zoneEnd, MD_MAX72XX::TINV); }
  }

  /**
   * Reset the current zone animation to restart.
   *
   * See comments for the MD_Parola namesake method.
   *
   * \return No return value.
   */
  inline void zoneReset(void) {
    _fsmState = INITIALISE;
  }

  /**
   * Shutdown or resume zone hardware.
   *
   * See comments for the MD_Parola namesake method.
   *
   * \param b  boolean value to shutdown (true) or resume (false).
   * \return No return value.
   */
  void zoneShutdown(bool b) {
    _MX->control(_zoneStart, _zoneEnd, MD_MAX72XX::SHUTDOWN, b ? 1 : 0);
  }

  /**
   * Suspend or resume zone updates.
   *
   * See comments for the MD_Parola namesake method.
   *
   * \param b boolean value to suspend (true) or resume (false).
   * \return No return value.
   */
  inline void zoneSuspend(bool b) {
    _suspend = b;
  }

  /**
   * Set the start and end parameters for a zone.
   *
   * See comments for the MD_Parola namesake method.
   *
   * \param zStart  the first module number for the zone [0..numZones-1].
   * \param zEnd  the last module number for the zone [0..numZones-1].
   */
  inline void setZone(uint8_t zStart, uint8_t zEnd) {
    _zoneStart = zStart; _zoneEnd = zEnd;
  }

  /** @} */

  // --------------------------------------------------------------

  /** \name Support methods for visually adjusting the display.
   * @{
   */

  /**
   * Get the zone inter-character spacing in columns.
   *
   * \return the current setting for the space between characters in columns.
   */
  inline uint8_t getCharSpacing(void) {
    return _charSpacing;
  }

  /**
   * Get the zone brightness.
   *
   * Get the intensity (brightness) of the display.
   *
   * \return The intensity setting.
   */
  inline uint8_t getIntensity() {
    return _intensity;
  }

  /**
   * Get the zone current invert state.
   *
   * See the setInvert() method.
   *
   * \return the inverted boolean value.
   */
  inline bool getInvert(void) {
    return _inverted;
  }

  /**
   * Get the zone pause time.
   *
   * See the setPause() method.
   *
   * \return the pause value in milliseconds.
   */
  inline uint16_t getPause(void) {
    return _pauseTime;
  }

  /**
   * Get the horizontal Scroll spacing.
   *
   * See the setScrollSpacing() method
   *
   * \return the space between message in columns.
   */
  inline uint16_t getScrollSpacing(void) {
    return _scrollDistance;
  }

  /**
   * Get the zone animation speed.
   *
   * See the setSpeed() method.
   * This should be replaced with either getSpeedIn() or getSpeedOut()
   * unless it is known that both directions are running at the same speed.
   *
   * \return the IN speed value.
   */
  inline uint16_t getSpeed(void) {
    return getSpeedIn();
  }

  /**
   * Get the zone animation IN speed.
   *
   * See the setSpeed() method.
   *
   * \return the speed value.
   */
  inline uint16_t getSpeedIn(void) {
    return _tickTimeIn;
  }

  /**
   * Get the zone animation OUT speed.
   *
   * See the setSpeed() method.
   *
   * \return the speed value.
   */
  inline uint16_t getSpeedOut(void) {
    return _tickTimeOut;
  }

  /**
   * Get the zone animation start time.
   *
   * See the setSynchTime() method
   *
   * \return the internal time reference.
   */
  inline uint32_t getSynchTime(void) {
    return _lastRunTime;
  }

  /**
   * Get the current text alignment specification.
   *
   * \return the current text alignment setting
   */
  inline textPosition_t getTextAlignment(void) {
    return _textAlignment;
  }

  /**
   *  Get the width of text in columns
   *
   * Calculate the width of the characters and the space between them
   * using the current font and text settings.
   *
   * \param p   pointer to a text string
   * \return the width of the string in display columns
   */
  uint16_t    getTextWidth(const uint8_t *p);

  /**
   * Get the value of specified display effect.
   *
   * The display effect is one of the zoneEffect_t types. The returned value will be
   * true if the attribute is set, false if the attribute is not set.
   *
   * \param ze  the required text alignment.
   * \return true if the value is set, false otherwise.
   */
  boolean     getZoneEffect(zoneEffect_t ze);

  /**
   * Set the zone inter-character spacing in columns.
   *
   * Set the number of blank columns between characters when they are displayed.
   *
   * \param cs  space between characters in columns.
   * \return No return value.
   */
  inline void setCharSpacing(uint8_t cs) {
    _charSpacing = cs; allocateFontBuffer();
  }

  /**
   * Set the zone brightness.
   *
   * Set the intensity (brightness) of the display.
   *
   * \param intensity the intensity to set the display (0-15).
   * \return No return value.
   */
  inline void setIntensity(uint8_t intensity) {
    _intensity = intensity; _MX->control(_zoneStart, _zoneEnd, MD_MAX72XX::INTENSITY, _intensity);
  }

  /**
   * Invert the zone display.
   *
   * Set the display to inverted (ON LED turns OFF and vice versa).
   *
   * \param invert  true for inverted display, false for normal display
   * \return No return value.
   */
  inline void setInvert(uint8_t invert) {
    _inverted = invert;
  }

  /**
   * Set the pause between ENTER and EXIT animations for this zone.
   *
   * Between each entry and exit, the library will pause by the number of milliseconds
   * specified to allow the viewer to read the message. For continuous scrolling displays
   * this should be set to the same value as the display speed.
   *
   * \param pause the time, in milliseconds, between animations.
   * \return No return value.
   */
  inline void setPause(uint16_t pause) {
    _pauseTime = pause;
  }

  /**
   * Set the horizontal scrolling distance between messages.
   *
   * When scrolling horizontally, the distance between the end of one message and the
   * start of the next can be set using this method. Normal operation is for the message
   * to be fully off the display before the new message starts.
   * Set to zero for default behavior.
   *
   * \param space the spacing, in columns, between messages; zero for default behaviour..
   * \return No return value.
   */
  inline void setScrollSpacing(uint16_t space) {
    _scrollDistance = space;
  }

  /**
   * Set the zone animation frame speed.
   *
   * The speed of the display is the 'tick' time between animation frames. The lower this time
   * the faster the animation; set it to zero to run as fast as possible.
   *
   * This method will set the same value for both IN and OUT animations speed.
   *
   * \param speed the time, in milliseconds, between animation frames.
   * \return No return value.
   */
  inline void setSpeed(uint16_t speed) {
    setSpeedInOut(speed, speed);
  }

  /**
   * Set separate IN and OUT zone animation frame speed.
   *
   * The speed of the display is the 'tick' time between animation frames. The lower this time
   * the faster the animation; set it to zero to run as fast as possible.
   *
   * This method will set both the IN and OUT animations separately to the specified speed.
   *
   * \param speedIn the time, in milliseconds, between IN animation frames.
   * \param speedOut the time, in milliseconds, between OUT animation frames.
   * \return No return value.
   */
  inline void setSpeedInOut(uint16_t speedIn, uint16_t speedOut) {
    _tickTimeIn = speedIn; _tickTimeOut = speedOut;
  }

#if ENA_SPRITE

  /**
   * Set data for user sprite effects.
   *
   * Set up the data parameters for user sprite text entry/exit effects.
   * See the comments for the namesake method in MD_Parola.
   *
   * \param inData pointer to the data table defining the entry sprite.
   * \param inWidth the width (in bytes) of each frame of the sprite.
   * \param inFrames the number of frames for the sprite.
   * \param outData pointer to the data table that is inWidth*InFrames in size.
   * \param outWidth the width (in bytes) of each frame of the sprite.
   * \param outFrames the number of frames for the sprite.
   * \return No return value.
   */
  void setSpriteData(const uint8_t *inData,
                     uint8_t        inWidth,
                     uint8_t        inFrames,
                     const uint8_t *outData,
                     uint8_t        outWidth,
                     uint8_t        outFrames);
#endif // if ENA_SPRITE

  /**
   * Set the zone animation start time.
   *
   * Each zone animation has an associated start time. The start time
   * defaults to the time when the zone is initialized. This method allows
   * synchronization between zones by setting the same start time. Should be
   * used in conjunction with the getSynchTime() method as the return value
   * should only be treated as an internal reference and arbitrary values
   * will result in irregular behavior.
   *
   * \param zt the required start time.
   * \return No return value.
   */
  inline void setSynchTime(uint32_t zt) {
    _lastRunTime = zt;
  }

  /**
   * Set the text alignment within the zone.
   *
   * Text alignment is specified as one of the values in textPosition_t.
   *
   * \param ta  the required text alignment.
   * \return No return value.
   */
  inline void setTextAlignment(textPosition_t ta) {
    _textAlignment = ta;
  }

  /**
   * Set the pointer to the text buffer for this zone.
   *
   * Sets the text buffer to be a pointer to user data.
   * See the comments for the namesake method in MD_Parola.
   *
   * \param pb  pointer to the text buffer to be used.
   * \return No return value.
   */
  inline void setTextBuffer(const char *pb) {
    _pText = (const uint8_t *)pb;
  }

  /**
   * Set the entry and exit text effects for the zone.
   *
   * See the comments for the namesake method in MD_Parola.
   *
   * \param effectIn  the entry effect, one of the textEffect_t enumerated values.
   * \param effectOut the exit effect, one of the textEffect_t enumerated values.
   * \return No return value.
   */
  inline void setTextEffect(textEffect_t effectIn, textEffect_t effectOut) {
    _effectIn = (effectIn == PA_NO_EFFECT ? PA_PRINT : effectIn), _effectOut = effectOut;
  }

  /**
   * Set the zone display effect.
   *
   * The display effect is one of the zoneEffect_t types, and this will be set (true) or
   * reset (false) depending on the boolean value. The resulting zone display will be
   * modified as per the required effect.
   *
   * \param b set the value if true, reset the value if false
   * \param ze  the required text alignment.
   * \return No return value.
   */
  void setZoneEffect(boolean      b,
                     zoneEffect_t ze);

  /** @} */

  // --------------------------------------------------------------

  /** \name Support methods for fonts and characters.
   * @{
   */

  /**
   * Add a user defined character to the replacement list.
   *
   * Add a replacement characters to the user defined list. The character data must be
   * the same as for a single character in the font definition file. If a character is
   * specified with a code the same as an existing character the existing data will be
   * substituted for the new data. A character code of 0 is illegal as this denotes the
   * end of string character for C++ and cannot be used in an actual string.
   * The library does not copy the in the data in the data definition but only retains
   * a pointer to the data, so any changes to the data storage in the calling program will
   * be reflected in the library.
   *
   * \param code  code for the character data.
   * \param data  pointer to the character data.
   * \return true of the character was inserted in the substitution list.
   */
  bool addChar(uint16_t code,
               uint8_t *data);

  /**
   * Delete a user defined character to the replacement list.
   *
   * Delete a replacement character to the user defined list. A character code of 0 is
   * illegal as this denotes the end of string character for C++ and cannot be used in
   * an actual string.
   *
   * \param code  ASCII code for the character data.
   * \return true of the character was found in the substitution list.
   */
  bool                           delChar(uint16_t code);

  /**
   * Get the display font.
   *
   * Return the current font table pointer for this zone.
   *
   * \return Pointer to the font definition used.
   */
  inline MD_MAX72XX::fontType_t* getZoneFont(void) {
    return _fontDef;
  }

  /**
   * Set the display font.
   *
   * See comments for the namesake Parola method.
   *
   * \param fontDef Pointer to the font definition to be used.
   * \return No return value.
   */
  void setZoneFont(MD_MAX72XX::fontType_t *fontDef) {
    _fontDef = fontDef; _MX->setFont(_fontDef); allocateFontBuffer();
  }

  /** @} */

#if ENA_GRAPHICS

  // --------------------------------------------------------------

  /** \name Support methods for graphics.
   * @{
   */
  /**
   * Get the start and end column numbers for the zone.
   *
   * Returns the start and end column numbers for the zone display.
   * This retains consistency between user code and library.
   *
   * \param startColumn the by-reference parameter that will hold the return value for the start column.
   * \param endColumn the by-reference parameter that will hold the return value for the end column.
   * \return The start and end columns in the by-reference parameters.
   */
  void getZoneExtent(uint16_t& startColumn, uint16_t& endColumn) {
    startColumn = ZONE_START_COL(_zoneStart); endColumn = ZONE_END_COL(_zoneEnd);
  }

  /**
   * Get the start and end column numbers for the text displayed.
   *
   * Returns the start and end column numbers for the text displayed in the zone.
   * This retains consistency between user code and library.
   *
   * \param startColumn the by-reference parameter that will hold the return value for the start column.
   * \param endColumn the by-reference parameter that will hold the return value for the end column.
   * \return The start and end columns in the by-reference parameters.
   */
  void getTextExtent(uint16_t& startColumn, uint16_t& endColumn) {
    startColumn = _limitLeft; endColumn = _limitRight;
  }

  /** @} */
#endif // if ENA_GRAPHICS

private:

  /***
   *  Finite State machine states enumerated type.
   */
  enum fsmState_t
  {
    INITIALISE,     ///< Initialize all variables
    GET_FIRST_CHAR, ///< Get the first character
    GET_NEXT_CHAR,  ///< Get the next character
    PUT_CHAR,       ///< Placing a character
    PUT_FILLER,     ///< Placing filler (blank) columns
    PAUSE,          ///< Pausing between animations
    END             ///< Display cycle has completed
  };

  /***
   *  Structure for list of user defined characters substitutions.
   */
  struct charDef_t
  {
    uint16_t   code;                     ///< the ASCII code for the user defined character
    uint8_t   *data;                     ///< user supplied data
    charDef_t *next;                     ///< next in the list
  };

  MD_MAX72XX *_MX;                       ///< Pointer to parent's MD_MAX72xx object passed in at begin()

  // Time and speed controlling data and methods
  bool _suspend;                         // don't do anything
  uint32_t _lastRunTime;                 // the millis() value for when the animation was last run
  uint16_t _tickTimeIn;                  // the time between IN animations in milliseconds
  uint16_t _tickTimeOut;                 // the time between OUT animations in milliseconds
  uint16_t _pauseTime;                   // time to pause the animation between 'in' and 'out'

  // Display control data and methods
  fsmState_t _fsmState;                  // fsm state for all FSMs used to display text
  uint16_t _textLen;                     // length of current text in columns
  int16_t _limitLeft;                    // leftmost limit for the current display effect
  int16_t _limitRight;                   // rightmost limit for the current display effect
  bool _limitOverflow;                   // true if the text will overflow the display
  textPosition_t _textAlignment;         // current text alignment
  textEffect_t _effectIn;                // the effect for text entering the display
  textEffect_t _effectOut;               // the effect for text exiting the display
  bool _moveIn;                          // animation is moving IN when true, OUT when false
  bool _inverted;                        // true if the display needs to be inverted
  uint16_t _scrollDistance;              // the space in columns between the end of one message and the start of the next
  uint8_t _zoneEffect;                   // bit mapped zone effects
  uint8_t _intensity;                    // display intensity
  bool _animationAdvanced;               // true is animation advanced inthe last animation call

  void setInitialConditions(void);       // set up initial conditions for an effect
  bool calcTextLimits(const uint8_t *p); // calculate the right and left limits for the text

  // Variables used in the effects routines. These can be used by the functions as needed.
  uint8_t _zoneStart;                    // First zone module number
  uint8_t _zoneEnd;                      // Last zone module number
  int16_t _nextPos;                      // Next position for animation. Can be used in several different ways depending on the function.
  int8_t _posOffset;                     // Looping increment depends on the direction of the scan for animation
  int16_t _startPos;                     // Start position for the text LED
  int16_t _endPos;                       // End limit for the text LED.

  void setInitialEffectConditions(void); // set the initial conditions for loops in the FSM

  // Character buffer handling data and methods
  const uint8_t *_pText;                 // pointer to text buffer from user call
  const uint8_t *_pCurChar;              // the current character being processed in the text
  bool _endOfText;                       // true when the end of the text string has been reached.
  void moveTextPointer(void);            // move the text pointer depending on direction of buffer scan

  bool getFirstChar(uint8_t& len);       // put the first Text char into the char buffer
  bool getNextChar(uint8_t& len);        // put the next Text char into the char buffer

  // Font character handling data and methods
  charDef_t *_userChars;                 // the root of the list of user defined characters
  uint8_t _cBufSize;                     // allocated size of the array for loading character font (cBuf)
  uint8_t *_cBuf;                        // buffer for loading character font - allocated when font is set
  uint8_t _charSpacing;                  // spacing in columns between characters
  uint8_t _charCols;                     // number of columns for this character
  int16_t _countCols;                    // count of number of columns already shown
  MD_MAX72XX::fontType_t *_fontDef;      // font for this zone

  void    allocateFontBuffer(void);      // allocate _cBuf based on the size of the largest font characters
  uint8_t findChar(uint16_t code,
                   uint8_t  size,
                   uint8_t *cBuf);       // look for user defined character
  uint8_t makeChar(uint16_t c,
                   bool     addBlank);   // load a character bitmap and add in trailing _charSpacing blanks if req'd
  void    reverseBuf(uint8_t *p,
                     uint8_t  size);     // reverse the elements of the buffer
  void    invertBuf(uint8_t *p,
                    uint8_t  size);      // invert the elements of the buffer

  /// Sprite management
#if ENA_SPRITE
  uint8_t *_spriteInData, *_spriteOutData;
  uint8_t _spriteInWidth, _spriteOutWidth;
  uint8_t _spriteInFrames, _spriteOutFrames;
#endif // if ENA_SPRITE

  // Debugging aid
  const char* state2string(fsmState_t s);

  // Effect functions
  void        commonPrint(void);
  void        effectPrint(bool bIn);
  void        effectVScroll(bool bUp,
                            bool bIn);
  void        effectHScroll(bool bLeft,
                            bool bIn);
#if ENA_MISC
  void        effectSlice(bool bIn);
  void        effectMesh(bool bIn);
  void        effectFade(bool bIn);
  void        effectBlinds(bool bIn);
  void        effectDissolve(bool bIn);
  void        effectRandom(bool bIn);
#endif // ENA_MISC
#if ENA_SPRITE
  void        effectSprite(bool    bIn,
                           uint8_t id);
#endif // ENA_SPRITE
#if ENA_WIPE
  void effectWipe(bool bLightBar,
                  bool bIn);
#endif // ENA_WIPE
#if ENA_OPNCLS
  void effectOpen(bool bLightBar,
                  bool bIn);
  void effectClose(bool bLightBar,
                   bool bIn);
#endif // ENA_OPNCLS
#if ENA_SCR_DIA
  void effectDiag(bool bUp,
                  bool bLeft,
                  bool bIn);
#endif // ENA_SCR_DIA
#if ENA_SCAN
  void effectHScan(bool bIn,
                   bool bBlank);
  void effectVScan(bool bIn,
                   bool bBlank);
#endif // ENA_SCAN
#if ENA_GROW
  void effectGrow(bool bUp,
                  bool bIn);
#endif // ENA_GROW
};

/**
 * Core object for the Parola library.
 * This class contains one or more zones for display.
 */
class MD_Parola : public Print {
public:

  /**
   * Class constructor - arbitrary output pins.
   *
   * Instantiate a new instance of the class. The parameters passed are used to
   * connect the software to the hardware using the MD_MAX72XX class.
   *
   * See documentation for the MD_MAX72XX library for detailed explanation of parameters.
   *
   * \param mod       the hardware module type used in the application. One of the MD_MAX72XX::moduleType_t values.
   * \param dataPin   output on the Arduino where data gets shifted out.
   * \param clkPin    output for the clock signal.
   * \param csPin     output for selecting the device.
   * \param numDevices  number of devices connected. Default is 1 if not supplied.
   */
  MD_Parola(MD_MAX72XX::moduleType_t mod, uint8_t dataPin, uint8_t clkPin, uint8_t csPin, uint8_t numDevices = 1) :
    _D(mod, dataPin, clkPin, csPin, numDevices), _numModules(numDevices)
  {}

  /**
   * Class constructor - SPI hardware interface.
   *
   * Instantiate a new instance of the class. The parameters passed are used to
   * connect the software to the hardware using the MD_MAX72XX class.
   *
   * See documentation for the MD_MAX72XX library for detailed explanation of parameters.
   *
   * \param mod       the hardware module type used in the application. One of the MD_MAX72XX::moduleType_t values.
   * \param csPin   output for selecting the device.
   * \param numDevices  number of devices connected. Default is 1 if not supplied.
   */
  MD_Parola(MD_MAX72XX::moduleType_t mod, uint8_t csPin, uint8_t numDevices = 1) :
    _D(mod, csPin, numDevices), _numModules(numDevices)
  {}

  /**
   * Initialize the object.
   *
   * Initialize the object data. This needs to be called during setup() to initialize new
   * data for the class that cannot be done during the object creation. This form of the
   * method is for backward compatibility and creates one zone for the entire display.
   */
  void begin(void) {
    begin(1);
  }

  /**
   * Initialize the object.
   *
   * Initialize the object data. This needs to be called during setup() to initialize new
   * data for the class that cannot be done during the object creation. This form of the
   * method allows specifying the number of zones used. The module limits for the individual
   * zones are initialized separately using setZone(), which should be done immediately after
   * the invoking begin().
   *
   * \sa setZone()
   *
   * \param numZones  maximum number of zones
   */
  void begin(uint8_t numZones);

  /**
   * Class Destructor.
   *
   * Release allocated memory and does the necessary to clean up once the object is
   * no longer required.
   */
  ~MD_Parola(void);

  // --------------------------------------------------------------

  /** \name Methods for core object control.
   * @{
   */
  /**
   * Animate the display.
   *
   * Animate all the zones in the display using the currently specified text and
   * animation parameters. This method needs to be invoked as often as possible
   * to ensure smooth animation. The animation is governed by a time tick that
   * is set by the setSpeed() or setSpeedInOut() methods and it will pause between
   * entry and exit using the time set by the setPause() method.
   *
   * The calling program should monitor the return value for 'true' in order to either
   * reset the zone animation or supply another string for display. A 'true' return
   * value means that one or more zones have completed their animation.
   *
   * Not all calls to this method result in an animation, as this is governed by
   * the timing parameters set for the animation. To determine when an animation has
   * advanced during the call, the user code can call the isAnimationAdvanced() method.
   *
   * \return bool true if at least one zone animation has completed, false otherwise.
   */
  bool displayAnimate(void);

  /**
   * Get the completion status for a zone.
   *
   * This method is to determine which zone has completed when displayAnimate()
   * has returned a completion status.
   *
   * The calling program should monitor the return value for 'true' in order to either
   * reset the zone animation or supply another string for display. A 'true' return
   * value means that the zone has completed its animation cycle.
   *
   * \param z     specified zone
   * \return bool true if the zone animation has completed, false otherwise.
   */
  bool getZoneStatus(uint8_t z) {
    if (z < _numZones) { return _Z[z].getStatus(); } else { return true; }
  }

  /**
   * Clear the display.
   *
   * Clear all the zones in the current display.
   *
   * \return No return value.
   */
  void displayClear(void) {
    for (uint8_t i = 0; i < _numZones; i++) { _Z[i].zoneClear(); }
  }

  /**
   * Clear one zone in the display.
   *
   * Clear the specified zone in the current display.
   *
   * \param z   specified zone
   * \return No return value.
   */
  void displayClear(uint8_t z) {
    if (z < _numZones) { _Z[z].zoneClear(); }
  }

  /**
   * Reset the current animation to restart for all zones.
   *
   * This method is used to reset all the zone animations an animation back to the start
   * of their cycle current cycle.
   * It is normally invoked after all the parameters for a display are set and the
   * animation needs to be started (or restarted).
   *
   * \return No return value.
   */
  void displayReset(void) {
    for (uint8_t i = 0; i < _numZones; i++) { _Z[i].zoneReset(); }
  }

  /**
   * Reset the current animation to restart for the specified zone.
   *
   * See the comments for the 'all zones' variant of this method.
   *
   * \param z specified zone
   * \return No return value.
   */
  void displayReset(uint8_t z) {
    if (z < _numZones) { _Z[z].zoneReset(); }
  }

  /**
   * Shutdown or restart display hardware.
   *
   * Shutdown the display hardware to a low power state. The display will
   * be blank during the shutdown. Calling animate() will continue to
   * animate the display in the memory buffers but this will not be visible
   * on the display (ie, the libraries still function but the display does not).
   *
   * \param b  boolean value to shutdown (true) or resume (false).
   * \return No return value.
   */
  void displayShutdown(bool b) {
    for (uint8_t i = 0; i < _numZones; i++) { _Z[i].zoneShutdown(b); }
  }

  /**
   * Suspend or resume display updates.
   *
   * Stop the current display animation. When pausing it leaves the
   * display showing the current text. Resuming will restart the animation where
   * it left off. To reset the animation back to the beginning, use the
   * displayReset() method.
   *
   * \param b  boolean value to suspend (true) or resume (false).
   * \return No return value.
   */
  void displaySuspend(bool b) {
    for (uint8_t i = 0; i < _numZones; i++) { _Z[i].zoneSuspend(b); }
  }

  /**
   * Check if animation frame has advanced.
   *
   * Check if the last call to animate() resulted in the animation frame advancing by
   * one or more frames in one or more zones. Useful to integrate graphics into the
   * display as they may be affected by the text animation.
   *
   * For displays with more than one zone, the user code will need to interrogate each zone
   * to determine whether the animation advanced in that zone.
   *
   * \return True if the animation frame advanced in any of the display zones.
   */
  bool isAnimationAdvanced(void) {
    bool b = false;

    for (uint8_t i = 0; i < _numZones; i++) { b |= _Z[i].isAnimationAdvanced(); } return b;
  }

  /**
   * Get the module limits for a zone.
   *
   * Once a zone has been defined, this method will return the
   * start and end module that were defined for the specified zone.
   *
   * \sa setZone()
   *
   * \param z   zone number.
   * \param moduleStart returns the first module number for the zone [0..numZones-1].
   * \param moduleEnd   returns last module number for the zone [0..numZones-1].
   */
  inline void getZone(uint8_t z, uint8_t& moduleStart, uint8_t& moduleEnd) {
    if (z < _numZones) { _Z[z].getZone(moduleStart, moduleEnd); }
  }

  /**
   * Define the module limits for a zone.
   *
   * When multiple zones are defined, the library needs to know the contiguous module
   * ranges that make up the different zones. If the library has been started with only
   * one zone then it will automatically initialize the zone to be the entire range for
   * the display modules, so calling this function is not required. However, when multiple
   * zones are defined, setZone() for each zone should be should be invoked immediately
   * after the call to begin().
   *
   * A module is a unit of 8x8 LEDs, as defined in the MD_MAX72xx library.
   * Zones should not overlap or unexpected results will occur.
   *
   * \sa begin()
   *
   * \param z   zone number.
   * \param moduleStart the first module number for the zone [0..numZones-1].
   * \param moduleEnd   the last module number for the zone [0..numZones-1].
   * \return true if set, false otherwise.
   */
  bool setZone(uint8_t z,
               uint8_t moduleStart,
               uint8_t moduleEnd);

  /** @} */

  // --------------------------------------------------------------

  /** \name Methods for quick start displays.
   * @{
   */
  /**
   * Easy start for a scrolling text display.
   *
   * This method is a convenient way to set up a scrolling display. All the data
   * necessary for setup is passed through as parameters and the display animation
   * is started. Assumes one zone only (zone 0).
   *
   * \param pText   parameter suitable for the setTextBuffer() method.
   * \param align   parameter suitable for the the setTextAlignment() method.
   * \param effect  parameter suitable for the the setTextEffect() method.
   * \param speed   parameter suitable for the setSpeed() method.
   * \return No return value.
   */
  inline void displayScroll(const char *pText, textPosition_t align, textEffect_t effect, uint16_t speed)
  {
    displayZoneText(0, pText, align, speed, 0, effect, effect);
  }

  /**
   * Easy start for a non-scrolling text display.
   *
   * This method is a convenient way to set up a static text display. All the data
   * necessary for setup is passed through as parameters and the display animation
   * is started. Assumes one zone only (zone 0).
   *
   * \param pText parameter suitable for the setTextBuffer() method.
   * \param align parameter suitable for the the setTextAlignment() method.
   * \param speed parameter suitable for the setSpeed() method.
   * \param pause parameter suitable for the setPause() method.
   * \param effectIn  parameter suitable for the setTextEffect() method.
   * \param effectOut parameter suitable for the setTextEffect() method.
   * \return No return value.
   */
  inline void displayText(const char    *pText,
                          textPosition_t align,
                          uint16_t       speed,
                          uint16_t       pause,
                          textEffect_t   effectIn,
                          textEffect_t   effectOut = PA_NO_EFFECT)
  {
    displayZoneText(0, pText, align, speed, pause, effectIn, effectOut);
  }

  /**
   * Easy start for a non-scrolling zone text display.
   *
   * This method is a convenient way to set up a static text display within the
   * specified zone. All the data necessary for setup is passed through as
   * parameters and the display animation is started.
   *
   * \param z   zone specified.
   * \param pText parameter suitable for the setTextBuffer() method.
   * \param align parameter suitable for the the setTextAlignment() method.
   * \param speed parameter suitable for the setSpeed() method.
   * \param pause parameter suitable for the setPause() method.
   * \param effectIn  parameter suitable for the setTextEffect() method.
   * \param effectOut parameter suitable for the setTextEffect() method.
   * \return No return value.
   */
  void displayZoneText(uint8_t        z,
                       const char    *pText,
                       textPosition_t align,
                       uint16_t       speed,
                       uint16_t       pause,
                       textEffect_t   effectIn,
                       textEffect_t   effectOut = PA_NO_EFFECT);

  /** @} */

  // --------------------------------------------------------------

  /** \name Support methods for visually adjusting the display.
   * @{
   */

  /**
   * Get the inter-character spacing in columns.
   *
   * \return the current setting for the space between characters in columns. Assumes one zone only.
   */
  inline uint8_t getCharSpacing(void) {
    return getCharSpacing(0);
  }

  /**
   * Get the inter-character spacing in columns for a specific zone.
   *
   * \param z   zone number.
   * \return the current setting for the space between characters in columns.
   */
  inline uint8_t getCharSpacing(uint8_t z) {
    return z < _numZones ? _Z[z].getCharSpacing() : 0;
  }

  /**
   * Get the current display invert state.
   *
   * See the setInvert() method.
   *
   * \return true if the display is inverted. Assumes one zone only.
   */
  inline bool getInvert(void) {
    return getInvert(0);
  }

  /**
   * Get the current display invert state for a specific zone.
   *
   * See the setInvert() method.
   *
   * \param z   zone number.
   * \return the inverted boolean value for the specified zone.
   */
  inline bool getInvert(uint8_t z) {
    return z < _numZones ? _Z[z].getInvert() : false;
  }

  /**
   * Get the current pause time.
   *
   * See the setPause() method. Assumes one zone only.
   *
   * \return the pause value in milliseconds.
   */
  inline uint16_t getPause(void) {
    return getPause(0);
  }

  /**
   * Get the current pause time for a specific zone.
   *
   * See the setPause() method.
   *
   * \param z   zone number.
   * \return the pause value in milliseconds for the specified zone.
   */
  inline uint16_t getPause(uint8_t z) {
    return z < _numZones ? _Z[z].getPause() : 0;
  }

  /**
   * Get the horizontal scrolling spacing.
   *
   * See the setScrollSpacing() method. Assumes one zone only
   *
   * \return the speed value.
   */
  inline uint16_t getScrollSpacing(void) {
    return _Z[0].getScrollSpacing();
  }

  /**
   * Get the current IN animation speed.
   *
   * See the setSpeed() method. Assumes one zone only
   *
   * \return the speed value.
   */
  inline uint16_t getSpeed(void) {
    return getSpeed(0);
  }

  /**
   * Get the current IN animation speed for the specified zone.
   *
   * See the setSpeed() method.
   *
   * \param z   zone number.
   * \return the speed value for the specified zone.
   */
  inline uint16_t getSpeed(uint8_t z) {
    return z < _numZones ? _Z[z].getSpeed() : 0;
  }

  /**
   * Get the current IN animation speed for the specified zone.
   *
   * See the setSpeed() method.
   *
   * \param z   zone number.
   * \return the IN speed value for the specified zone.
   */
  inline uint16_t getSpeedIn(uint8_t z) {
    return z < _numZones ? _Z[z].getSpeedIn() : 0;
  }

  /**
   * Get the current OUT animation speed for the specified zone.
   *
   * See the setSpeed() method.
   *
   * \param z   zone number.
   * \return the OUT speed value for the specified zone.
   */
  inline uint16_t getSpeedOut(uint8_t z) {
    return z < _numZones ? _Z[z].getSpeedOut() : 0;
  }

  /**
   * Get the current text alignment specification.
   *
   * Assumes one zone only.
   *
   * \return the current text alignment setting.
   */
  inline textPosition_t getTextAlignment(void) {
    return getTextAlignment(0);
  }

  /**
   * Get the current text alignment specification for the specified zone.
   *
   * \param z   zone number.
   * \return the current text alignment setting for the specified zone.
   */
  inline textPosition_t getTextAlignment(uint8_t z) {
    return z < _numZones ? _Z[z].getTextAlignment() : PA_CENTER;
  }

  /**
   * Get the text width in columns

   * Evaluate the width in column for the text string *p as the sum of all
   * the characters and the space between them, using the currently assigned font.
   * Assumes one zone display.
   *
   * \param p   nul terminate character string to evaluate.
   * \return the number of columns used to display the text.
   */
  inline uint16_t getTextColumns(const char *p) {
    return getTextColumns(0, p);
  }

  /**
   * Get the text width in columns

   * Evaluate the width in column for the text string *p in the zone specified, as
   * the sum of all the characters and the space between them. As each zone can
   * display using a different font table, the result can vary between zones.
   *
   * \param z   zone number.
   * \param p   nul terminate character string to evaluate.
   * \return the number of columns used to display the text.
   */
  inline uint16_t getTextColumns(uint8_t z, const char *p) {
    return z < _numZones && p != nullptr ? _Z[z].getTextWidth((uint8_t *)p) : 0;
  }

  /**
   * Get the value of specified display effect.
   *
   * The display effect is one of the zoneEffect_t types. The returned value will be
   * true if the attribute is set, false if the attribute is not set.
   *
   * \param z   zone number.
   * \param ze  the required text alignment.
   * \return true if the value is set, false otherwise.
   */
  inline boolean getZoneEffect(uint8_t z, zoneEffect_t ze) {
    return z < _numZones ? _Z[z].getZoneEffect(ze) : false;
  }

  /**
   * Set the inter-character spacing in columns for all zones.
   *
   * Set the number of blank columns between characters when they are displayed.
   *
   * \param cs  space between characters in columns.
   * \return No return value.
   */
  void setCharSpacing(uint8_t cs) {
    for (uint8_t i = 0; i < _numZones; i++) { _Z[i].setCharSpacing(cs); }
  }

  /**
   * Set the inter-character spacing in columns for the specified zone.
   *
   * See comments for the 'all zones' variant of this method.
   *
   * \param z   zone number.
   * \param cs  space between characters in columns.
   * \return No return value.
   */
  inline void setCharSpacing(uint8_t z, uint8_t cs) {
    if (z < _numZones) { _Z[z].setCharSpacing(cs); }
  }

  /**
   * Set the display brightness for all the zones.
   *
   * Set the intensity (brightness) of the display.
   *
   * \param intensity the intensity to set the display (0-15).
   * \return No return value.
   */
  inline void setIntensity(uint8_t intensity) {
    for (uint8_t i = 0; i < _numZones; i++) { _Z[i].setIntensity(intensity); }
  }

  /**
   * Set the display brightness for the specified zone.
   *
   * See comments for the 'all zones' variant of this method.
   *
   * \param z   zone number.
   * \param intensity the intensity to set the display (0-15).
   * \return No return value.
   */
  inline void setIntensity(uint8_t z, uint8_t intensity) {
    if (z < _numZones) { _Z[z].setIntensity(intensity); }
  }

  /**
   * Invert the display in all the zones.
   *
   * Set the display to inverted (ON LED turns OFF and vice versa).
   *
   * \param invert  true for inverted display, false for normal display
   * \return No return value.
   */
  inline void setInvert(uint8_t invert) {
    for (uint8_t i = 0; i < _numZones; i++) { _Z[i].setInvert(invert); }
  }

  /**
   * Invert the display in the specified zone.
   *
   * See comments for the 'all zones' variant of this method.
   *
   * \param z   zone number.
   * \param invert  true for inverted display, false for normal display
   * \return No return value.
   */
  inline void setInvert(uint8_t z, uint8_t invert) {
    if (z < _numZones) { _Z[z].setInvert(invert); }
  }

  /**
   * Set the pause between ENTER and EXIT animations for all zones.
   *
   * Between each entry and exit, the library will pause by the number of milliseconds
   * specified to allow the viewer to read the message. For continuous scrolling displays
   * this should be set to the same value as the display speed.
   *
   * \param pause the time, in milliseconds, between animations.
   * \return No return value.
   */
  inline void setPause(uint16_t pause) {
    for (uint8_t i = 0; i < _numZones; i++) { _Z[i].setPause(pause); }
  }

  /**
   * Set the pause between ENTER and EXIT animations for the specified zone.
   *
   * See comments for the 'all zones' variant of this method.
   *
   * \param z   zone number.
   * \param pause the time, in milliseconds, between animations.
   * \return No return value.
   */
  inline void setPause(uint8_t z, uint16_t pause) {
    if (z < _numZones) { _Z[z].setPause(pause); }
  }

  /**
   * Set the horizontal scrolling distance between messages for all the zones.
   *
   * When scrolling horizontally, the distance between the end of one message and the
   * start of the next can be set using this method. Default behavior is for the message
   * to be fully off the display before the new message starts.
   * Set to zero for default behavior.
   *
   * \param space the spacing, in columns, between messages; zero for default behaviour..
   * \return No return value.
   */
  inline void setScrollSpacing(uint16_t space) {
    for (uint8_t i = 0; i < _numZones; i++) { _Z[i].setScrollSpacing(space); }
  }

  /**
   * Set identical IN and OUT animation frame speed for all zones.
   *
   * The speed of the display is the 'tick' time between animation frames. The lower this time
   * the faster the animation; set it to zero to run as fast as possible.
   *
   * This method sets the IN and OUT animation speeds to be the same.
   *
   * \param speed the time, in milliseconds, between animation frames.
   * \return No return value.
   */
  inline void setSpeed(uint16_t speed) {
    for (uint8_t i = 0; i < _numZones; i++) { _Z[i].setSpeed(speed); }
  }

  /**
   * Set separate IN and OUT animation frame speed for all zones.
   *
   * The speed of the display is the 'tick' time between animation frames. The lower this time
   * the faster the animation; set it to zero to run as fast as possible.
   *
   * This method allows the IN and OUT animation speeds to be different.
   *
   * \param speedIn the time, in milliseconds, between IN animation frames.
   * \param speedOut the time, in milliseconds, between OUT animation frames.
   * \return No return value.
   */
  inline void setSpeedInOut(uint16_t speedIn, uint16_t speedOut) {
    for (uint8_t i = 0; i < _numZones; i++) { _Z[i].setSpeedInOut(speedIn, speedOut); }
  }

  /**
   * Set the identical IN and OUT animation frame speed for the specified zone.
   *
   * See comments for the 'all zones' variant of this method.
   *
   * \param z   zone number.
   * \param speed the time, in milliseconds, between animation frames.
   * \return No return value.
   */
  inline void setSpeed(uint8_t z, uint16_t speed) {
    if (z < _numZones) { _Z[z].setSpeed(speed); }
  }

  /**
   * Set the separate IN and OUT animation frame speed for the specified zone.
   *
   * See comments for the 'all zones' variant of this method.
   *
   * \param z   zone number.
   * \param speedIn the time, in milliseconds, between IN animation frames.
   * \param speedOut the time, in milliseconds, between OUT animation frames.
   * \return No return value.
   */
  inline void setSpeedInOut(uint8_t z, uint16_t speedIn, uint16_t speedOut) {
    if (z < _numZones) { _Z[z].setSpeedInOut(speedIn, speedOut); }
  }

#if ENA_SPRITE

  /**
   * Set data for user sprite effects (single zone).
   *
   * This method is used to set up user data needed so that the library can
   * display the sprite ahead of the entry/exit of text when the PA_SPRITE
   * animation type is selected.
   *
   * A sprite is made up of a number of frames that run sequentially to make
   * make the animation on the display. Once the animation reaches the last frame
   * it restarts from the first frame.
   *
   * A sprite is defined similarly to a character in the font table. Each byte
   * is a bit pattern defining a column in the sprite. A frame is xWidth bytes
   * in size and there are xFrames in the animation.
   *
   * \param z zone number.
   * \param inData pointer to the data table defining the entry sprite.
   * \param inWidth the width (in bytes) of each frame of the sprite.
   * \param inFrames the number of frames for the sprite.
   * \param outData pointer to the data table that is inWidth*InFrames in size.
   * \param outWidth the width (in bytes) of each frame of the sprite.
   * \param outFrames the number of frames for the sprite.
   * \return No return value.
   */
  void setSpriteData(uint8_t z, const uint8_t *inData, uint8_t inWidth, uint8_t inFrames,
                     const uint8_t *outData, uint8_t outWidth, uint8_t outFrames)
  {
    if (z < _numZones) { _Z[z].setSpriteData(inData, inWidth, inFrames, outData, outWidth, outFrames); }
  }

  /**
   * Set data for user sprite effect (whole display).
   *
   * See the comments for single zone variant of this method.
   *
   * \param inData pointer to the data table defining the entry sprite.
   * \param inWidth the width (in bytes) of each frame of the sprite.
   * \param inFrames the number of frames for the sprite.
   * \param outData pointer to the data table that is inWidth*InFrames in size.
   * \param outWidth the width (in bytes) of each frame of the sprite.
   * \param outFrames the number of frames for the sprite.
   * \return No return value.
   */
  void setSpriteData(const uint8_t *inData, uint8_t inWidth, uint8_t inFrames,
                     const uint8_t *outData, uint8_t outWidth, uint8_t outFrames)
  {
    for (uint8_t i = 0; i < _numZones; i++) { _Z[i].setSpriteData(inData, inWidth, inFrames, outData, outWidth, outFrames); }
  }

#endif // if ENA_SPRITE

  /**
   * Set the text alignment for all zones.
   *
   * Text alignment is specified as one of the values in textPosition_t.
   *
   * \param ta  the required text alignment.
   * \return No return value.
   */
  inline void setTextAlignment(textPosition_t ta) {
    for (uint8_t i = 0; i < _numZones; i++) { _Z[i].setTextAlignment(ta); }
  }

  /**
   * Set the text alignment for the specified zone.
   *
   * See comments for the 'all zones' variant of this method.
   *
   * \param z zone number.
   * \param ta  the required text alignment.
   * \return No return value.
   */
  inline void setTextAlignment(uint8_t z, textPosition_t ta) {
    if (z < _numZones) { _Z[z].setTextAlignment(ta); }
  }

  /**
   * Set the pointer to the text buffer.
   *
   * Sets the text buffer to be a pointer to user data. The library does not allocate
   * any memory for the text message, rather it is the calling program that supplies
   * a pointer to a buffer. This reduces memory requirements and offers the flexibility
   * to keep a single buffer or rotate buffers with different messages, all under calling
   * program control, with no library limit to the size or numbers of buffers. The text
   * placed in the buffer must be properly terminated by the NUL ('\0') character or
   * processing will overrun the end of the message.
   *
   * This form of the method assumes one zone only.
   *
   * \param pb  pointer to the text buffer to be used.
   * \return No return value.
   */
  inline void setTextBuffer(const char *pb) {
    setTextBuffer(0, pb);
  }

  /**
   * Set the pointer to the text buffer for the specified zone.
   *
   * See comments for the single zone version of this method.
   *
   * \param z zone number.
   * \param pb  pointer to the text buffer to be used.
   * \return No return value.
   */
  inline void setTextBuffer(uint8_t z, const char *pb) {
    if (z < _numZones) { _Z[z].setTextBuffer(pb); }
  }

  /**
   * Set the entry and exit text effects for all zones.
   *
   * The 'in' and 'out' text effects are specified using the textEffect_t enumerated
   * type. If no effect is required, NO_EFFECT should be specified. NO_EFFECT
   * is most useful when no exit effect is required (e.g., when DISSOLVE is used) and
   * the entry effect is sufficient.
   *
   * \param effectIn  the entry effect, one of the textEffect_t enumerated values.
   * \param effectOut the exit effect, one of the textEffect_t enumerated values.
   * \return No return value.
   */
  inline void setTextEffect(textEffect_t effectIn, textEffect_t effectOut) {
    for (uint8_t i = 0; i < _numZones; i++) { _Z[i].setTextEffect(effectIn, effectOut); }
  }

  /**
   * Set the entry and exit text effects for a specific zone.
   *
   * See comments for the 'all zones' variant of this method.
   *
   * \param z     zone number.
   * \param effectIn  the entry effect, one of the textEffect_t enumerated values.
   * \param effectOut the exit effect, one of the textEffect_t enumerated values.
   * \return No return value.
   */
  inline void setTextEffect(uint8_t z, textEffect_t effectIn, textEffect_t effectOut) {
    if (z < _numZones) { _Z[z].setTextEffect(effectIn, effectOut); }
  }

  /**
   * Set the display effect for the specified zone.
   *
   * The display effect is one of the zoneEffect_t types, and this will be set (true) or
   * reset (false) depending on the boolean value. The resulting zone display will be
   * modified as per the required effect.
   *
   * \param z   zone number.
   * \param b   set the value if true, reset the value if false
   * \param ze  the required text alignment.
   * \return No return value.
   */
  inline void setZoneEffect(uint8_t z, boolean b, zoneEffect_t ze) {
    if (z < _numZones) { _Z[z].setZoneEffect(b, ze); }
  }

  /**
   * Synchronize the start of zone animations.
   *
   * When zones are set up, the animation start time will default
   * to the set-up time. If several zones need to be animated
   * in synchronized fashion (eg, they are visually stacked vertically),
   * this method will ensure that all the zones start at the same instant.
   * The method should be invoked before the call to displayAnimate().
   *
   * \return No return value.
   */
  inline void synchZoneStart(void) {
    for (uint8_t i = 1; i < _numZones; i++) { _Z[i].setSynchTime(_Z[0].getSynchTime()); }
  }

  /** @} */

  // --------------------------------------------------------------

  /** \name Support methods for fonts and characters.
   * @{
   */

  /**
   * Add a user defined character to the replacement list for all zones.
   *
   * Add a replacement characters to the user defined list. The character data must be
   * the same as for a single character in the font definition file. If a character is
   * specified with a code the same as an existing character the existing data will be
   * substituted for the new data. A character code of 0 ('\0') is illegal as this
   * denotes the end of string character for C++ and cannot be used in an actual string.
   *
   * The library does not copy the data definition but only retains a pointer to the data,
   * so any changes to the data storage in the calling program will be reflected into the
   * library. The data must also remain in scope while it is being used.
   *
   * \param code  code for the character data.
   * \param data  pointer to the character data.
   * \return No return value.
   */
  inline void addChar(uint16_t code, uint8_t *data) {
    for (uint8_t i = 0; i < _numZones; i++) { _Z[i].addChar(code, data); }
  }

  /**
   * Add a user defined character to the replacement specified zone.
   *
   * See the comments for the 'all zones' variant of this method
   *
   * \param z   zone specified
   * \param code  ASCII code for the character data.
   * \param data  pointer to the character data.
   * \return true of the character was inserted in the substitution list.
   */
  inline bool addChar(uint8_t z, uint16_t code, uint8_t *data) {
    return z < _numZones ? _Z[z].addChar(code, data) : false;
  }

  /**
   * Delete a user defined character to the replacement list for all zones.
   *
   * Delete a reference to a replacement character in the user defined list.
   *
   * \param code  ASCII code for the character data.
   * \return No return value.
   */
  inline void delChar(uint16_t code) {
    for (uint8_t i = 0; i < _numZones; i++) { _Z[i].delChar(code); }
  }

  /**
   * Delete a user defined character to the replacement list for the specified zone.
   *
   * See the comments for the 'all zones' variant of this method.
   *
   * \param z   zone specified
   * \param code  ASCII code for the character data.
   * \return true of the character was found in the substitution list.
   */
  inline bool delChar(uint8_t z, uint16_t code) {
    return z < _numZones ? _Z[z].delChar(code) : false;
  }

  /**
   * Get the display font for specified zone.
   *
   * Get the current font table pointer for the specified zone.
   *
   * \param z specified zone.
   * \return Pointer to the font definition used.
   */
  inline MD_MAX72XX::fontType_t* getFont(uint8_t z) {
    if (z < _numZones) { return _Z[z].getZoneFont(); } else { return nullptr; }
  }

  /**
   * Get the display font for single zone display.
   *
   * Return the current font table pointer for single zone displays.
   *
   * \return Pointer to the font definition used.
   */
  inline MD_MAX72XX::fontType_t* getFont(void) {
    return getFont(0);
  }

  /**
   * Set the display font for all zones.
   *
   * Set the display font to a user defined font table. This can be created using the
   * MD_MAX72xx font builder (refer to documentation for the tool and the MD_MAX72xx library).
   * Passing nullptr resets to the library default font.
   *
   * \param fontDef Pointer to the font definition to be used.
   * \return No return value.
   */
  inline void setFont(MD_MAX72XX::fontType_t *fontDef) {
    for (uint8_t i = 0; i < _numZones; i++) { _Z[i].setZoneFont(fontDef); }
  }

  /**
   * Set the display font for a specific zone.
   *
   * Set the display font to a user defined font table. This can be created using the
   * MD_MAX72xx font builder (refer to documentation for the tool and the MD_MAX72xx library).
   * Passing nullptr resets to the library default font.
   *
   * \param z   specified zone.
   * \param fontDef Pointer to the font definition to be used.
   * \return No return value.
   */
  inline void setFont(uint8_t z, MD_MAX72XX::fontType_t *fontDef) {
    if (z < _numZones) { _Z[z].setZoneFont(fontDef); }
  }

  /** @} */

#if ENA_GRAPHICS

  // --------------------------------------------------------------

  /** \name Support methods for graphics.
   * @{
   */
  /**
   * Get a pointer to the instantiated graphics object.
   *
   * Provides a pointer to the MD_MAX72XX object to allow access to
   * the display graphics functions.
   *
   * \return Pointer to the MD_MAX72xx object used by the library.
   */
  inline MD_MAX72XX* getGraphicObject(void) {
    return &_D;
  }

  /**
   * Get the start and end column numbers for the whole display.
   *
   * Returns the start and end column numbers for the matrix display.
   * This retains consistency between user code and library.
   *
   * \param startColumn the by-reference parameter that will hold the return value for the start column.
   * \param endColumn the by-reference parameter that will hold the return value for the end column.
   * \return The start and end columns in the by-reference parameters.
   */
  inline void getDisplayExtent(uint16_t& startColumn, uint16_t& endColumn) {
    startColumn = ZONE_START_COL(0); endColumn = ZONE_END_COL(_numModules - 1);
  }

  /**
   * Get the start and end column numbers for the required zone.
   *
   * Returns the start and end column numbers for the zone display.
   * This retains consistency between user code and library.
   * The by-reference parameters remain unchanged if an invalid zone
   * number is requested.
   *
   * \param z zone specified
   * \param startColumn the by-reference parameter that will hold the return value for the start column.
   * \param endColumn the by-reference parameter that will hold the return value for the end column.
   * \return The start and end columns in the by-reference parameters.
   */
  inline void getDisplayExtent(uint8_t z, uint16_t& startColumn, uint16_t& endColumn) {
    if (z < _numZones) { _Z[z].getZoneExtent(startColumn, endColumn); }
  }

  /**
   * Get the start and end column numbers for the text displayed.
   *
   * Returns the start and end column numbers for the text displayed in zone 0.
   * This can be used for simplicity when the display is a single zone.
   *
   * \param startColumn the by-reference parameter that will hold the return value for the start column.
   * \param endColumn the by-reference parameter that will hold the return value for the end column.
   * \return The start and end columns in the by-reference parameters.
   */
  inline void getTextExtent(uint16_t& startColumn, uint16_t& endColumn) {
    getTextExtent(0, startColumn, endColumn);
  }

  /**
   * Get the start and end column numbers for the text displayed.
   *
   * Returns the start and end column numbers for the text displayed in the zone.
   * This retains consistency between user code and library.
   *
   * \param z zone specified
   * \param startColumn the by-reference parameter that will hold the return value for the start column.
   * \param endColumn the by-reference parameter that will hold the return value for the end column.
   * \return The start and end columns in the by-reference parameters.
   */
  inline void getTextExtent(uint8_t z, uint16_t& startColumn, uint16_t& endColumn) {
    if (z < _numZones) { _Z[z].getTextExtent(startColumn, endColumn); }
  }

  /** @} */
#endif // if ENA_GRAPHICS

  // --------------------------------------------------------------

  /** \name Support methods for Print class extension.
   * @{
   */

  /**
   * Write a single character to the output display
   *
   * Display a character when given the ASCII code for it. The character is
   * converted to a string and the string printing function invoked.
   * The LED display is designed for string based output, so it does not make much
   * sense to do this. Creating the short string is a consistent way to way to handle
   * single the character.
   *
   * \param c  ASCII code for the character to write.
   * \return the number of characters written.
   */
  virtual size_t write(uint8_t c) {
    char sz[2]; sz[0] = c; sz[1] = '\0'; write(sz); return 1;
  }

  /**
   * Write a nul terminated string to the output display.
   *
   * Display a nul terminated string when given a pointer to the char array.
   * Invokes an animation using PA_PRINT with all other settings (alignment,
   * speed, etc) taken from current defaults.
   * This method also invokes the animation for the print and returns when that has
   * finished, so it blocks while the printing is happening, which should be at least
   * one iteration of the wait loop.
   *
   * \param str  Pointer to the nul terminated char array.
   * \return the number of characters written.
   */
  virtual size_t write(const char *str);

  /**
   * Write a character buffer to the output display.
   *
   * Display a non-nul terminated string given a pointer to the buffer and
   * the size of the buffer. The buffer is turned into a nul terminated string
   * and the simple write() method is invoked. Memory is allocated and freed
   * in this method to copy the string.
   *
   * \param buffer Pointer to the data buffer.
   * \param size The number of bytes to write.
   * \return the number of bytes written.
   */
  virtual size_t write(const uint8_t *buffer,
                       size_t         size);

  /** @} */

private:

  // The display hardware controlled by this library
  MD_MAX72XX _D;          ///< Hardware library object
#if STATIC_ZONES
  MD_PZone _Z[MAX_ZONES]; ///< Fixed number of zones - static zone allocation
#else // if STATIC_ZONES
  MD_PZone *_Z;           ///< Zones buffers - dynamic zone allocation
#endif // if STATIC_ZONES
  uint8_t _numModules;    ///< Number of display modules [0..numModules-1]
  uint8_t _numZones;      ///< Max number of zones in the display [0..numZones-1]
};
