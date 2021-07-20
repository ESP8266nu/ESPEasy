#include "_Plugin_Helper.h"
#ifdef USES_P104

// #######################################################################################################
// #######################   Plugin 104 - dot matrix display plugin MAX7219       ########################
// #######################################################################################################
//
// Chips/displays supported:
//  MAX7219/21 -- 3 pins (SPI + CS) - Dot matrix display, 8x8 leds per unit/module, units can be connected daisy chained, up to 255
// (theoretically, untested yet)
//
// Plugin Commands:
// Commands have this syntax: dotmatrix,<subcommand>,<zone>[,<argument>]
// <zone> is in the range of configured zones, limited to 1..8 (ESP8266) or 1..16 (ESP32)
// Subcommands:
// clear[,all|<zone>]           : Clears the entire display (all zones if 'all' or no zone specified) or the zone specified
// update[,all|<zone>]          : Updates the entire display (all zones if 'all' or no zone specified) or the zone specified
// size,<zone>,<modules>        : Set the number of modules (Size) for that zone (1..64). A complete reconfiguration will be done if this
//                                setting is changed
// txt,<zone>,<text>            : Put the <text> (use quotes if it contains spaces or commas) in the specified zone, for Bar graph a set of
//                                graphStrings can be set
// settxt,<zone>,<text>         : As the txt subcommand, but also stores the text in the settings for that zone (not automatically saved)
// content,<zone>,<contenttype> : Set the desired content type for that zone (0..)
//                                0 = Text : Any text, including variable expansion
//                                1 = Text reverse : Any text, including variable expansion, reversed when displayed
//                                2 = Clock (4 mod.) : Time in 4 digits (HH:mm) 24h with flashing colon or Clock settings are applied
//                                3 = Clock sec (6 mod) : Time in 6 digits (HH:mm ss) 24h with flashing colon between HH and mm
//                                4 = Date (4 mod.) : Date in 4 digits (dd MM)
//                                5 = Date yy (6/7 mod.) : Date in 6 or 8 digits (dd MM yy / dd mm yyyy)
//                                6 = Date/time (9/13 mod.) : Date + time in 10 digits (dd MM yy HH:mm) 24h, flashing colon between HH and
//                                    mm
//                                7 = Bar graph : See below at bar/setbar commands on how to set the graphString(s).
//                                The (n mod.) suffix indicates the number of modules required to make all digits visible at once
// alignment,<zone>,<alignment> : Set the Alignment of the zone (0 = Left, 1 = Center, 2 = Right). A complete reconfiguration will be done
//                                if this setting is changed
// anim.in,<zone>,<animation>   : Set the Animation In type of the zone (1..). A complete reconfiguration will be done if this setting is
//                                changed
// anim.out,<zone>,<animation>  : Set the Animation Out type of the zone (0..). A complete reconfiguration will be done if this setting is
//                                changed
//                                The supported animation ID's are visible in the Animation In/Out selection comboboxes
//                                Only supported animations are accepted (some can be disabled at compiletime)
// speed,<zone>,<speed>         : Set the Speed of the zone, determining the delay in millis between each animation step
// pause,<zone>,<pause>         : Set the Pause of the zone, determining the delay in millis after Animation In is completed before
//                                Animation Out starts
// font,<zone>,<font ID>        : Set the font for the zone. The font ID is visible in the UI in the Font selection combobox.
// layout,<zone>,<layout ID>    : Set the Layout type if a double-height font is included (upper(1) or lower(2) part) else only Default (0)
//                                is available
// specialeffect,<zone>,<effect>: Set the Special Effects field, 0 = None, 1 = Flip Up/Done, 2 = Flip Left/Right, 3 = Flip u/d & l/r
//                                Any special effect marked with an asterisk * should not be combined with an Animation marked with an
//                                asterisk, as that could result in unexpected effects during display (it may look strange)
// offset,<zone>,<modules>      : Set the Offset of modules for that zone (0..size). A complete reconfiguration will be done if this setting
//                                is changed
// brightness,<zone>,<level>    : Set the Brightness for the zone, range from 0 (off) to 15 (very bright)
// repeat,<zone>,<delay_sec>    : Set the Repeat (sec) for the zone, after this delay the text & animation will be repeated.
//                                -1 = off, range 0..86400 seconds (24h)
// bar,<zone>,<graph-string>    : Set the graph-string for a Bargraph zone, format: value,max,min,direction,barType|...
// setbar,<zone>,<graph-string> : as the bar subcommand, but also stores the graph-string in the settings for that zone (not automatically
//                                saved)
//                                value: numeric value of type double, variables can be used
//                                max: value for a full bar display, default 100 (percent)
//                                min: value for minimal bar display, default 0
//                                direction: 0 (default): right to left, 1: left to right
//                                barType: 0: solid, width: 8/number of graph-strings
//                                         1: solid, width: 1
//                                         2: dotted line, alternating, only if the bar is wider than 1 pixel
//                                Up to 8 graph-strings can be provided and must be separated by a pipe |
//                                The bar width is determined by the number of graph-strings
//
// History:
// 2021-07-18 tonhuisman: Small optimizations and improvements
// 2021-07-14 tonhuisman: Fix some bugs in font selection, add Text reverse content type to improve usability of Vertical font
// 2021-07-12 tonhuisman: Reduce number of reconfiguration during command handling, will be applied the next time content is displayed
//                        update/correct some documentation
// 2021-07-08 tonhuisman: Several bugfixes: settings defaults, fix brightness to enable 0 value, simplify storing the zone settings
// 2021-06-29-2021-07-03: Add Actions column to insert before/after zone or delete a zone, order the zones either in numeric order
//            tonhuisman: or in display order ('upside-down'), fixed many bugs, refactored bar-graph method, other improvements
//                        All optional at compile-time by P104_USE_ZONE_ACTIONS and P104_USE_ZONE_ORDERING
//                        Disabled P104_DEBUG_DEV by default
// 2021-06-28 tonhuisman: Bugfixes during testing, re-enable subcommands for ESP8266 display build
// 2021-06-27 tonhuisman: Implement 'barType' option for Bar-graph, bugfixes, bugfixes, bugfixes
// 2021-06-26 tonhuisman: Implement 'direction' option for Bar-graph, bugfixes
// 2021-06-26 tonhuisman: Add update command for updating one or all zones, restart repeat timer if content is updated by command, bugfixes
// 2021-06-24 tonhuisman: Add min/max range with negative minimal value support and zero-point indication if possible
// 2021-06-23 tonhuisman: Add Bar-graph option and bar command (initial feature, options to implement) guarded with P104_USE_BAR_GRAPH
// 2021-06-22 tonhuisman: Add Bar-graph initial code-infrastructure
// 2021-06-21 tonhuisman: Add options for 'formatting' time & date, will be disabled on memory-tight configs guarded by
//                        P104_USE_DATETIME_OPTIONS
//                        Introduced guard P104_ADD_SETTINGS_NOTES to disable some addFormNotes() to further reduce code size
// 2021-06-19 tonhuisman: Implement repeat delay, add settxt command, add command reference (above), bug fixing, some source reorganization
//                        Webform_Load now works on current settings if the plugin is active, instead of last stored settings
//                        Disabled most commands a some fonts to make the build fit in the ESP8266 display configuration
// 2021-06-18 tonhuisman: Implement PLUGIN_WRITE commands
//                        Implement several fonts extracted from MD_Parola examples (Vertical, Extended ASCII, Full Double Height, Arabic,
//                        Greek, Katakana) (NB: Vertical isn't working as expected yet) Will be disabled when flash memory is tight
// 2021-06-16 tonhuisman: Implement Clock and Date (once per second)
// 2021-06-13 tonhuisman: First working version, many improvemnts to make, like command handling, repeat timer implementaation
// 2021-05 tonhuisman: Store and retrieve settings for max 8 (ESP82xx) or 16 (ESP32) zones
// 2021-04 tonhuisman: Pickup and rework to get it working with ESPEasy
// 2021-03 rixtyan : Initial plugin setup, partially based on P073 MAX7219 LED display

# define PLUGIN_104
# define PLUGIN_ID_104           104
# define PLUGIN_NAME_104         "Display - MAX7219 dot matrix [DEVELOPMENT]"

# define PLUGIN_104_DEBUG        true // activate extra log info in the debug


# include "src/PluginStructs/P104_data_struct.h"


boolean Plugin_104(uint8_t function, struct EventStruct *event, String& string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD: {
      Device[++deviceCount].Number           = PLUGIN_ID_104;
      Device[deviceCount].Type               = DEVICE_TYPE_SPI;
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

    case PLUGIN_SET_DEFAULTS: {
      CONFIG_PORT = -1;
      break;
    }

    case PLUGIN_GET_DEVICENAME: {
      string = F(PLUGIN_NAME_104);
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES: {
      event->String1 = formatGpioName_output("CS");
      break;
    }

    case PLUGIN_WEBFORM_LOAD: {
      int8_t din_pin = -1;
      int8_t clk_pin = -1;
      int    pinnr = -1;
      bool   input, output, warning;
      # ifdef ESP32

      switch (Settings.InitSPI) {
        case 1:
          din_pin = 23;
          clk_pin = 19;
          break;
        case 2:
          din_pin = 13;
          clk_pin = 14;
          break;
      }
      # else // ESP82xx

      if (Settings.InitSPI == 1) {
        din_pin = 13;
        clk_pin = 14;
      }
      # endif // ifdef ESP32
      String note;
      note.reserve(72);
      note = F("SPI->MAX7219: MOSI");

      if (din_pin != -1) {
        note += '(';
        getGpioInfo(din_pin, pinnr, input, output, warning);
        note += createGPIO_label(din_pin, pinnr, true, true, false);
        note += ')';
      }
      note += F("->DIN, CLK");

      if (clk_pin != -1) {
        note += '(';
        getGpioInfo(clk_pin, pinnr, input, output, warning);
        note += createGPIO_label(clk_pin, pinnr, true, true, false);
        note += ')';
      }
      note += F("->CLK");
      addFormNote(note);

      P104_data_struct *P104_data = static_cast<P104_data_struct *>(getPluginTaskData(event->TaskIndex));

      bool createdWhileActive = false;

      if (nullptr == P104_data) { // Create new object if not active atm.
        P104_data = new (std::nothrow) P104_data_struct(static_cast<MD_MAX72XX::moduleType_t>(P104_CONFIG_HARDWARETYPE),
                                                        event->TaskIndex,
                                                        CONFIG_PIN1,
                                                        -1);
        createdWhileActive = true;
      }

      if (nullptr == P104_data) {
        addFormNote(F("Memory allocation error, re-open task to load."));
        return success;
      }

      if (createdWhileActive) {
        P104_data->loadSettings();
      }
      success = P104_data->webform_load(event);

      if (createdWhileActive) {
        delete P104_data; // Clean up
      }
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      P104_data_struct *P104_data = static_cast<P104_data_struct *>(getPluginTaskData(event->TaskIndex));

      bool createdWhileActive = false;

      if (nullptr == P104_data) { // Create new object if not active atm.
        P104_data = new (std::nothrow) P104_data_struct(static_cast<MD_MAX72XX::moduleType_t>(P104_CONFIG_HARDWARETYPE),
                                                        event->TaskIndex,
                                                        CONFIG_PIN1,
                                                        -1);
        createdWhileActive = true;
      }

      if (nullptr != P104_data) {
        P104_data->setZones(P104_CONFIG_ZONE_COUNT);
        success = P104_data->webform_save(event);

        if (!success) {
          addHtmlError(P104_data->getError());
        }

        if (createdWhileActive) {
          delete P104_data; // Cleanup
        }
      }

      break;
    }

    case PLUGIN_INIT: {
      uint8_t numDevices = P104_CONFIG_TOTAL_UNITS;

      if (numDevices == 0) { // fallback value
        numDevices++;
      }
      # ifdef P104_DEBUG

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log;
        log.reserve(38);
        log  = F("dotmatrix: PLUGIN_INIT numDevices: ");
        log += numDevices;
        addLog(LOG_LEVEL_INFO, log);
      }
      # endif // ifdef P104_DEBUG

      initPluginTaskData(event->TaskIndex,
                         new (std::nothrow) P104_data_struct(static_cast<MD_MAX72XX::moduleType_t>(P104_CONFIG_HARDWARETYPE),
                                                             event->TaskIndex,
                                                             CONFIG_PIN1,
                                                             numDevices));

      P104_data_struct *P104_data =
        static_cast<P104_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P104_data) {
        return success;
      }

      P104_data->logAllText = bitRead(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_LOG_ALL_TEXT);

      // initialise the LED display
      if (P104_data->begin()) {
        // Setup the zones from configuration
        P104_data->configureZones();

        // Not sure if we'll need this in the future...
        //     invertUpperZone = (HARDWARE_TYPE == MD_MAX72XX::GENERIC_HW || HARDWARE_TYPE == MD_MAX72XX::PAROLA_HW);

        //      if (invertUpperZone)
        //      {
        //        P.setZoneEffect(ZONE_UPPER, true, PA_FLIP_UD);
        //        P.setZoneEffect(ZONE_UPPER, true, PA_FLIP_LR);


        success = true;
      }
      break;
    }

    case PLUGIN_EXIT: {
      P104_data_struct *P104_data = static_cast<P104_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P104_data) {
        return success;
      }

      if (bitRead(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_CLEAR_DISABLE)) { // Clear on exit?
        P104_data->P->displayClear();
      }
      success = true;

      break;
    }

    case PLUGIN_WRITE: {
      P104_data_struct *P104_data = static_cast<P104_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P104_data) {
        return success;
      }

      success = P104_data->handlePluginWrite(event->TaskIndex, string); // process commands

      break;
    }

    case PLUGIN_TEN_PER_SECOND: {
      P104_data_struct *P104_data = static_cast<P104_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P104_data) {
        return success;
      }

      P104_data->P->displayAnimate(); // Keep the animations moving
      success = true;

      break;
    }

    case PLUGIN_ONCE_A_SECOND: {
      P104_data_struct *P104_data = static_cast<P104_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P104_data) {
        return success;
      }

      if (P104_data->P->displayAnimate()) {     // At least 1 zone is ready
        for (uint8_t z = 0; z < P104_CONFIG_ZONE_COUNT; z++) {
          if (P104_data->P->getZoneStatus(z)) { // If the zone is ready, see if it should be repeated
            P104_data->checkRepeatTimer(z);
          }
        }

        P104_data->handlePluginOncePerSecond(event); // Update date & time contents, if needed
      }
    }
  }

  return success;
}

#endif // USES_P104
