#include "../PluginStructs/P104_data_struct.h"

#ifdef USES_P104

# include "../Helpers/ESPEasy_Storage.h"
# include "../Helpers/Numerical.h"
# include "../WebServer/Markup_Forms.h"
# include "../WebServer/WebServer.h"
# include "../WebServer/Markup.h"
# include "../WebServer/HTML_wrappers.h"
# include "../ESPEasyCore/ESPEasyRules.h"
# include "../Globals/ESPEasy_time.h"

# include <vector>
# include <MD_Parola.h>
# include <MD_MAX72xx.h>

// Needed also here for PlatformIO's library finder as the .h file
// is in a directory which is excluded in the src_filter

# if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
void createHString(String& string); // Forward definition
# endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
void reverseStr(String& str);       // Forward definition

/****************************************************************
 * Constructor
 ***************************************************************/
P104_data_struct::P104_data_struct(MD_MAX72XX::moduleType_t _mod,
                                   taskIndex_t              _taskIndex,
                                   int8_t                   _cs_pin,
                                   uint8_t                  _modulesize)
  : mod(_mod), taskIndex(_taskIndex), cs_pin(_cs_pin), modules(_modulesize) {
  if (Settings.InitSPI < 1) {
    addLog(LOG_LEVEL_ERROR, F("DOTMATRIX: Required SPI not enabled. Initialization aborted!"));
  } else {
    P = new MD_Parola(mod, cs_pin, modules);
  }
}

/*******************************
 * Initializer/starter
 ******************************/
bool P104_data_struct::begin() {
  if (!initialized) {
    loadSettings();
    initialized = true;
  }

  if ((P != nullptr) && (cs_pin > -1)) {
    addLog(LOG_LEVEL_INFO, F("dotmatrix: P->begin() called"));
    P->begin(expectedZones);
    # ifdef P104_USE_BAR_GRAPH
    pM = P->getGraphicObject();
    # endif // ifdef P104_USE_BAR_GRAPH
    return true;
  }
  return false;
}

# define P104_ZONE_SEP   '\x02'
# define P104_FIELD_SEP  '\x01'
# define P104_ZONE_DISP  ';'
# define P104_FIELD_DISP ','

/**************************************
 * loadSettings
 *************************************/
void P104_data_struct::loadSettings() {
  if (taskIndex < TASKS_MAX) {
    LoadCustomTaskSettings(taskIndex, (uint8_t *)&StoredSettings, sizeof(StoredSettings.bufferSize));
    uint16_t structDataSize = StoredSettings.bufferSize + sizeof(StoredSettings.bufferSize);
    # ifdef P104_DEBUG_DEV
    String log;

    if (loglevelActiveFor(LOG_LEVEL_INFO) &&
        log.reserve(54)) {
      log  = F("P104: loadSettings stored Size: ");
      log += structDataSize;
      log += F(" taskindex: ");
      log += taskIndex;
      addLog(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef P104_DEBUG_DEV
    LoadCustomTaskSettings(taskIndex, (uint8_t *)&StoredSettings, structDataSize); // Read only actual data
    StoredSettings.buffer[StoredSettings.bufferSize + 1] = '\0';                // Terminate string

    uint8_t zoneIndex = 0;

    {
      String buffer;
      buffer.reserve(StoredSettings.bufferSize + 1);
      buffer = String(StoredSettings.buffer);
      # ifdef P104_DEBUG_DEV

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log  = F("P104: loadSettings bufferSize: ");
        log += StoredSettings.bufferSize;
        log += F(" untrimmed: ");
        log += buffer.length();
      }
      # endif // ifdef P104_DEBUG_DEV
      buffer.trim();
      # ifdef P104_DEBUG_DEV

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F(" trimmed: ");
        log += buffer.length();
        addLog(LOG_LEVEL_INFO, log);
      }
      # endif // ifdef P104_DEBUG_DEV

      zones.clear();
      zonesInitialized = false;
      numDevices       = 0;

      int16_t  offset2;
      uint16_t prev2 = 0;
      String   tmp, fld;
      int tmp_int;
      offset2 = buffer.indexOf(P104_ZONE_SEP);

      while (offset2 > -1) {
        tmp.reserve(offset2 - prev2);
        tmp = buffer.substring(prev2, offset2);

        zones.push_back(P104_zone_struct(zoneIndex + 1));

        tmp_int = 0;

        // WARNING: Order of values should match the numeric order of P104_OFFSET_* values

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_SIZE, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].size = tmp_int;
        }

        zones[zoneIndex].text = parseStringKeepCase(tmp, 1 + P104_OFFSET_TEXT, P104_FIELD_SEP);

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_ALIGNMENT, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].alignment = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_ANIM_IN, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].animationIn = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_SPEED, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].speed = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_ANIM_OUT, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].animationOut = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_PAUSE, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].pause = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_FONT, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].font = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_CONTENT, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].content = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_LAYOUT, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].layout = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_SPEC_EFFECT, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].specialEffect = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_OFFSET, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].offset = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_BRIGHTNESS, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].brightness = tmp_int;
        }

        if (validIntFromString(parseString(tmp, 1 + P104_OFFSET_REPEATDELAY, P104_FIELD_SEP), tmp_int)) {
          zones[zoneIndex].repeatDelay = tmp_int;
        }

        delay(0);
        prev2       = offset2 + 1;
        offset2     = buffer.indexOf(P104_ZONE_SEP, prev2);
        numDevices += zones[zoneIndex].size + zones[zoneIndex].offset;
        zoneIndex++;
      }
      buffer.reserve(0); // Free some memory
    }
    # ifdef P104_DEBUG_DEV

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log  = F("P104: read zones from config: ");
      log += zoneIndex;

      // log += F(" struct size: ");
      // log += sizeof(P104_zone_struct);
      addLog(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef P104_DEBUG_DEV

    if (expectedZones == -1) { expectedZones = zoneIndex; }

    if (expectedZones == 0) { expectedZones++; } // Guarantee at least 1 zone to be displayed

    while (zoneIndex < expectedZones) {
      zones.push_back(P104_zone_struct(zoneIndex + 1));

      zoneIndex++;
      delay(0);
    }
    zonesInitialized = true;
    # ifdef P104_DEBUG_DEV

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log  = F("P104: total zones initialized: ");
      log += zoneIndex;
      addLog(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef P104_DEBUG_DEV
  }
}

/****************************************************
 * configureZones: initialize Zones setup
 ***************************************************/
void P104_data_struct::configureZones() {
  if (!zonesInitialized) {
    loadSettings();
  }

  uint8_t currentZone = 0;
  uint8_t zoneOffset  = 0;

  # ifdef P104_DEBUG_DEV
  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO) &&
      log.reserve(45)) {
    log  = F("P104: configureZones to do: ");
    log += zones.size();
    addLog(LOG_LEVEL_INFO, log);
  }
  # endif // ifdef P104_DEBUG_DEV

  P->displayClear();

  for (auto it = zones.begin(); it != zones.end(); ++it) {
    if (it->zone <= expectedZones) {
      zoneOffset += it->offset;
      P->setZone(currentZone, zoneOffset, zoneOffset + it->size - 1);
      # ifdef P104_USE_BAR_GRAPH
      it->_startModule = zoneOffset;
      P->getDisplayExtent(currentZone, it->_lower, it->_upper);
      # endif // ifdef P104_USE_BAR_GRAPH
      zoneOffset += it->size;

      switch (it->font) {
        case P104_DEFAULT_FONT_ID: {
          P->setFont(currentZone, nullptr); // default font
          break;
        }
        # ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
        case P104_DOUBLE_HEIGHT_FONT_ID: {
          P->setFont(currentZone, numeric7SegDouble);
          P->setCharSpacing(currentZone, P->getCharSpacing() * 2); // double spacing as well
          break;
        }
        # endif // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
        # ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
        case P104_FULL_DOUBLEHEIGHT_FONT_ID: {
          P->setFont(currentZone, BigFont);
          P->setCharSpacing(currentZone, P->getCharSpacing() * 2); // double spacing as well
          break;
        }
        # endif // ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
        # ifdef P104_USE_VERTICAL_FONT
        case P104_VERTICAL_FONT_ID: {
          P->setFont(currentZone, _fontVertical);
          break;
        }
        # endif // ifdef P104_USE_VERTICAL_FONT
        # ifdef P104_USE_EXT_ASCII_FONT
        case P104_EXT_ASCII_FONT_ID: {
          P->setFont(currentZone, ExtASCII);
          break;
        }
        # endif // ifdef P104_USE_EXT_ASCII_FONT
        # ifdef P104_USE_ARABIC_FONT
        case P104_ARABIC_FONT_ID: {
          P->setFont(currentZone, fontArabic);
          break;
        }
        # endif // ifdef P104_USE_ARABIC_FONT
        # ifdef P104_USE_GREEK_FONT
        case P104_GREEK_FONT_ID: {
          P->setFont(currentZone, fontGreek);
          break;
        }
        # endif // ifdef P104_USE_GREEK_FONT
        # ifdef P104_USE_KATAKANA_FONT
        case P104_KATAKANA_FONT_ID: {
          P->setFont(currentZone, fontKatakana);
          break;
        }
        # endif // ifdef P104_USE_KATAKANA_FONT

          // Extend here with more fonts if/when available
      }

      // Special Effects
      P->setZoneEffect(currentZone, (it->specialEffect & P104_SPECIAL_EFFECT_UP_DOWN) == P104_SPECIAL_EFFECT_UP_DOWN,       PA_FLIP_UD);
      P->setZoneEffect(currentZone, (it->specialEffect & P104_SPECIAL_EFFECT_LEFT_RIGHT) == P104_SPECIAL_EFFECT_LEFT_RIGHT, PA_FLIP_LR);

      // Brightness
      P->setIntensity(currentZone, it->brightness);

      # ifdef P104_DEBUG_DEV
      log  = F("P104: configureZones #");
      log += (currentZone + 1);
      log += '/';
      log += expectedZones;
      log += F(" offset: ");
      log += zoneOffset;
      addLog(LOG_LEVEL_INFO, log);
      # endif // ifdef P104_DEBUG_DEV

      // Content == text && text != ""
      if (((it->content == P104_CONTENT_TEXT) || (it->content == P104_CONTENT_TEXT_REV)) && (!it->text.isEmpty())) {
        displayOneZoneText(currentZone, *it, it->text);
      }

      # ifdef P104_USE_BAR_GRAPH

      // Content == Bar-graph && text != ""
      if ((it->content == P104_CONTENT_BAR_GRAPH) && (!it->text.isEmpty())) {
        displayBarGraph(currentZone, *it, it->text);
      }
      # endif // ifdef P104_USE_BAR_GRAPH

      if (it->repeatDelay > -1) {
        it->_repeatTimer = millis();
      }
      currentZone++;
    }
  }

  // synchronise the start
  P->synchZoneStart();
}

/**********************************************************
 * Display the text with attributes for a specific zone
 *********************************************************/
void P104_data_struct::displayOneZoneText(uint8_t                 zone,
                                          const P104_zone_struct& zstruct,
                                          const String          & text) {
  if ((zone < 0) || (zone > P104_MAX_ZONES)) { return; } // double check
  sZoneInitial[zone] = String(text); // Keep the original string for future use
  sZoneBuffers[zone] = String(text); // We explicitly want a copy here so it can be modified by parseTemplate()

  parseTemplate(sZoneBuffers[zone]);

  # if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)

  if (zstruct.layout == P104_LAYOUT_DOUBLE_UPPER) {
    createHString(sZoneBuffers[zone]);
  }
  # endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)

  if (zstruct.content == P104_CONTENT_TEXT_REV) {
    reverseStr(sZoneBuffers[zone]);
  }

  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO) &&
      logAllText &&
      log.reserve(28 + text.length() + sZoneBuffers[zone].length())) {
    log  = F("dotmatrix: ZoneText: ");
    log += zone + 1; // UI-number
    log += F(", '");
    log += text;
    log += F("' -> '");
    log += sZoneBuffers[zone];
    log += '\'';
    addLog(LOG_LEVEL_INFO, log);
  }

  P->displayZoneText(zone,
                     sZoneBuffers[zone].c_str(),
                     static_cast<textPosition_t>(zstruct.alignment),
                     zstruct.speed,
                     zstruct.pause,
                     static_cast<textEffect_t>(zstruct.animationIn),
                     static_cast<textEffect_t>(zstruct.animationOut));
}

/*********************************************
 * Update all or the specified zone
 ********************************************/
void P104_data_struct::updateZone(uint8_t                 zone,
                                  const P104_zone_struct& zstruct) {
  if (zone == 0) {
    for (auto it = zones.begin(); it != zones.end(); ++it) {
      if ((it->content == P104_CONTENT_TEXT) || (it->content == P104_CONTENT_TEXT_REV)) {
        displayOneZoneText(it->zone - 1, *it, sZoneInitial[it->zone - 1]); // Re-send last displayed text
        P->displayReset(it->zone - 1);
      }
      # ifdef P104_USE_BAR_GRAPH

      if (it->content == P104_CONTENT_BAR_GRAPH) {
        displayBarGraph(it->zone - 1, *it, sZoneInitial[it->zone - 1]); // Re-send last displayed bar graph
      }
      # endif // ifdef P104_USE_BAR_GRAPH

      if ((zstruct.content == P104_CONTENT_TEXT)
          || zstruct.content == P104_CONTENT_TEXT_REV
          # ifdef P104_USE_BAR_GRAPH
          || zstruct.content == P104_CONTENT_BAR_GRAPH
          # endif // ifdef P104_USE_BAR_GRAPH
          ) {
        if (it->repeatDelay > -1) { // Restart repeat timer
          it->_repeatTimer = millis();
        }
      }
    }
  } else {
    if ((zstruct.content == P104_CONTENT_TEXT) || (zstruct.content == P104_CONTENT_TEXT_REV)) {
      displayOneZoneText(zstruct.zone - 1, zstruct, sZoneInitial[zstruct.zone - 1]); // Re-send last displayed text
      P->displayReset(zstruct.zone - 1);
    }
    # ifdef P104_USE_BAR_GRAPH

    if (zstruct.content == P104_CONTENT_BAR_GRAPH) {
      displayBarGraph(zstruct.zone - 1, zstruct, sZoneInitial[zstruct.zone - 1]); // Re-send last displayed bar graph
    }
    # endif // ifdef P104_USE_BAR_GRAPH

    // Repeat timer is/should be started elsewhere
  }
}

# ifdef P104_USE_BAR_GRAPH

/***********************************************
 * Enable/Disable updating a range of modules
 **********************************************/
void P104_data_struct::modulesOnOff(uint8_t start, uint8_t end, MD_MAX72XX::controlValue_t on_off) {
  for (uint8_t m = start; m <= end; m++) {
    pM->control(m, MD_MAX72XX::UPDATE, on_off);
  }
}

/********************************************************
 * draw a single bar-graph, arguments already adjusted for direction
 *******************************************************/
void P104_data_struct::drawOneBarGraph(uint16_t lower,
                                       uint16_t upper,
                                       int16_t  pixBottom,
                                       int16_t  pixTop,
                                       uint16_t zeroPoint,
                                       uint8_t  barWidth,
                                       uint8_t  barType,
                                       uint8_t  row) {
  bool on_off;

  for (uint8_t r = 0; r < barWidth; r++) {
    for (uint8_t col = lower; col <= upper; col++) {
      on_off = (col >= pixBottom && col <= pixTop); // valid area

      if ((zeroPoint != 0) &&
          (barType == P104_BARTYPE_STANDARD) &&
          (barWidth > 2) &&
          ((r == 0) || (r == barWidth - 1)) &&
          (col == lower + zeroPoint)) {
        on_off = false; // when bar wider than 2, turn off zeropoint top and bottom led
      }

      if ((barType == P104_BARTYPE_SINGLE) && (r > 0)) {
        on_off = false; // barType 1 = only a single line is drawn, independent of the width
      }

      if ((barType == P104_BARTYPE_ALT_DOT) && (barWidth > 1) && on_off) {
        on_off = ((r % 2) == (col % 2)); // barType 2 = dotted line when bar is wider than 1 pixel
      }
      pM->setPoint(row + r, col, on_off);
    }
  }
}

/********************************************************************
 * Process a graph-string to display in a zone, format:
 * value,max-value,min-value,direction,bartype|...
 *******************************************************************/
void P104_data_struct::displayBarGraph(uint8_t                 zone,
                                       const P104_zone_struct& zstruct,
                                       const String          & graph) {
  if (graph.isEmpty()) { return; }
  sZoneInitial[zone] = String(graph); // Keep the original string for future use

  #  define NOT_A_COMMA 0x02          // Something else than a comma, or the parseString function will get confused
  String parsedGraph = String(graph);
  parseTemplate(parsedGraph);
  parsedGraph.replace(',', NOT_A_COMMA);

  std::vector<P104_bargraph_struct> barGraphs;
  uint8_t currentBar = 0;
  bool    loop       = true;

  // Parse the graph-string
  while (loop && currentBar < 8) { // Maximum 8 valuesets possible
    String graphpart = parseString(parsedGraph, currentBar + 1, '|');
    graphpart.trim();
    graphpart.replace(NOT_A_COMMA, ',');

    if (graphpart.isEmpty()) {
      loop = false;
    } else {
      barGraphs.push_back(P104_bargraph_struct(currentBar));
    }

    if (loop && validDoubleFromString(parseString(graphpart, 1), barGraphs[currentBar].value)) { // value
      String datapart = parseString(graphpart, 2);                                               // max (default: 100.0)

      if (datapart.isEmpty()) {
        barGraphs[currentBar].max = 100.0;
      } else {
        validDoubleFromString(datapart, barGraphs[currentBar].max);
      }
      datapart = parseString(graphpart, 3); // min (default: 0.0)

      if (datapart.isEmpty()) {
        barGraphs[currentBar].min = 0.0;
      } else {
        validDoubleFromString(datapart, barGraphs[currentBar].min);
      }
      datapart = parseString(graphpart, 4); // direction

      if (datapart.isEmpty()) {
        barGraphs[currentBar].direction = 0;
      } else {
        int value = 0;
        validIntFromString(datapart, value);
        barGraphs[currentBar].direction = value;
      }
      datapart = parseString(graphpart, 5); // barType

      if (datapart.isEmpty()) {
        barGraphs[currentBar].barType = 0;
      } else {
        int value = 0;
        validIntFromString(datapart, value);
        barGraphs[currentBar].barType = value;
      }

      if (definitelyGreaterThan(barGraphs[currentBar].min, barGraphs[currentBar].max)) {
        std::swap(barGraphs[currentBar].min, barGraphs[currentBar].max);
      }
    }
    #  ifdef P104_DEBUG

    if (logAllText && loglevelActiveFor(LOG_LEVEL_INFO)) {
      String log;
      log.reserve(70);
      log = F("dotmatrix: Bar-graph: ");

      if (loop) {
        log += currentBar;
        log += F(" in: ");
        log += graphpart;
        log += F(" value: ");
        log += barGraphs[currentBar].value;
        log += F(" max: ");
        log += barGraphs[currentBar].max;
        log += F(" min: ");
        log += barGraphs[currentBar].min;
        log += F(" dir: ");
        log += barGraphs[currentBar].direction;
        log += F(" typ: ");
        log += barGraphs[currentBar].barType;
      } else {
        log += F(" bsize: ");
        log += barGraphs.size();
      }
      addLog(LOG_LEVEL_INFO, log);
    }
    #  endif // ifdef P104_DEBUG
    currentBar++; // next
  }
  #  undef NOT_A_COMMA

  if (barGraphs.size() > 0) {
    uint8_t  barWidth = 8 / barGraphs.size(); // Divide the 8 pixel width per number of bars to show
    int16_t  pixTop, pixBottom;
    uint16_t zeroPoint;
    #  ifdef P104_DEBUG
    String log;

    if (logAllText &&
        loglevelActiveFor(LOG_LEVEL_INFO) &&
        log.reserve(64)) {
      log  = F("dotmatrix: bar Width: ");
      log += barWidth;
      log += F(" low: ");
      log += zstruct._lower;
      log += F(" high: ");
      log += zstruct._upper;
    }
    #  endif // ifdef P104_DEBUG
    modulesOnOff(zstruct._startModule, zstruct._startModule + zstruct.size - 1, MD_MAX72XX::MD_OFF); // Stop updates on modules
    P->setIntensity(zstruct.zone - 1, zstruct.brightness); // don't forget to set the brightness
    uint8_t row = 0;

    if ((barGraphs.size() == 3) || (barGraphs.size() == 5) || (barGraphs.size() == 6)) { // Center within the rows a bit
      for (; row < (barGraphs.size() == 5 ? 2 : 1); row++) {
        for (uint8_t col = zstruct._lower; col <= zstruct._upper; col++) {
          pM->setPoint(row, col, false);                                                 // all off
        }
      }
    }

    for (auto it = barGraphs.begin(); it != barGraphs.end(); ++it) {
      if (essentiallyEqual(it->min, 0.0)) {
        pixTop    = zstruct._lower - 1 + (((zstruct._upper + 1) - zstruct._lower) / it->max) * it->value;
        pixBottom = zstruct._lower - 1;
        zeroPoint = 0;
      } else {
        if (definitelyLessThan(it->min, 0.0) && definitelyGreaterThan(it->max, 0.0)) { // Zero-point is used
          zeroPoint = (it->min * -1.0) / ((it->max - it->min) / (1.0 * ((zstruct._upper + 1) - zstruct._lower)));
        } else {
          zeroPoint = 0;
        }
        pixTop    = zstruct._lower + zeroPoint + (((zstruct._upper + 1) - zstruct._lower) / (it->max - it->min)) * it->value;
        pixBottom = zstruct._lower + zeroPoint;

        if (definitelyLessThan(it->value, 0.0)) {
          std::swap(pixTop, pixBottom);
        }
      }

      if (it->direction == 1) { // Left to right display: Flip values within the lower/upper range
        pixBottom = zstruct._upper - (pixBottom - zstruct._lower);
        pixTop    = zstruct._lower + (zstruct._upper - pixTop);
        std::swap(pixBottom, pixTop);
        zeroPoint = zstruct._upper - zstruct._lower - zeroPoint + (zeroPoint == 0 ? 1 : 0);
      }
      #  ifdef P104_DEBUG_DEV

      if (logAllText && loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F(" B: ");
        log += pixBottom;
        log += F(" T: ");
        log += pixTop;
        log += F(" Z: ");
        log += zeroPoint;
      }
      #  endif // ifdef P104_DEBUG_DEV
      drawOneBarGraph(zstruct._lower, zstruct._upper, pixBottom, pixTop, zeroPoint, barWidth, it->barType, row);
      row += barWidth; // Next set of rows
    }

    for (; row < 8; row++) {           // Clear unused rows
      for (uint8_t col = zstruct._lower; col <= zstruct._upper; col++) {
        pM->setPoint(row, col, false); // all off
      }
    }
    #  ifdef P104_DEBUG

    if (logAllText && loglevelActiveFor(LOG_LEVEL_INFO)) {
      addLog(LOG_LEVEL_INFO, log);
    }
    #  endif // ifdef P104_DEBUG
    modulesOnOff(zstruct._startModule, zstruct._startModule + zstruct.size - 1, MD_MAX72XX::MD_ON);  // Continue updates on modules
  }
}

# endif // ifdef P104_USE_BAR_GRAPH

/**************************************************
 * Check if an animation is available in the current build
 *************************************************/
bool isAnimationAvailable(uint8_t animation, bool noneIsAllowed = false) {
  switch (animation) {
    case static_cast<int>(textEffect_t::PA_NO_EFFECT):
    {
      return noneIsAllowed;
    }
    case static_cast<int>(textEffect_t::PA_PRINT):
    case static_cast<int>(textEffect_t::PA_SCROLL_UP):
    case static_cast<int>(textEffect_t::PA_SCROLL_DOWN):
    case static_cast<int>(textEffect_t::PA_SCROLL_LEFT):
    case static_cast<int>(textEffect_t::PA_SCROLL_RIGHT):
    {
      return true;
    }
    # if ENA_SPRITE
    case static_cast<int>(textEffect_t::PA_SPRITE):
    {
      return true;
    }
    # endif // ENA_SPRITE
    # if ENA_MISC
    case static_cast<int>(textEffect_t::PA_SLICE):
    case static_cast<int>(textEffect_t::PA_MESH):
    case static_cast<int>(textEffect_t::PA_FADE):
    case static_cast<int>(textEffect_t::PA_DISSOLVE):
    case static_cast<int>(textEffect_t::PA_BLINDS):
    case static_cast<int>(textEffect_t::PA_RANDOM):
    {
      return true;
    }
    # endif // ENA_MISC
    # if ENA_WIPE
    case static_cast<int>(textEffect_t::PA_WIPE):
    case static_cast<int>(textEffect_t::PA_WIPE_CURSOR):
    {
      return true;
    }
    # endif // ENA_WIPE
    # if ENA_SCAN
    case static_cast<int>(textEffect_t::PA_SCAN_HORIZ):
    case static_cast<int>(textEffect_t::PA_SCAN_HORIZX):
    case static_cast<int>(textEffect_t::PA_SCAN_VERT):
    case static_cast<int>(textEffect_t::PA_SCAN_VERTX):
    {
      return true;
    }
    # endif // ENA_SCAN
    # if ENA_OPNCLS
    case static_cast<int>(textEffect_t::PA_OPENING):
    case static_cast<int>(textEffect_t::PA_OPENING_CURSOR):
    case static_cast<int>(textEffect_t::PA_CLOSING):
    case static_cast<int>(textEffect_t::PA_CLOSING_CURSOR):
    {
      return true;
    }
    # endif // ENA_OPNCLS
    # if ENA_SCR_DIA
    case static_cast<int>(textEffect_t::PA_SCROLL_UP_LEFT):
    case static_cast<int>(textEffect_t::PA_SCROLL_UP_RIGHT):
    case static_cast<int>(textEffect_t::PA_SCROLL_DOWN_LEFT):
    case static_cast<int>(textEffect_t::PA_SCROLL_DOWN_RIGHT):
    {
      return true;
    }
    # endif // ENA_SCR_DIA
    # if ENA_GROW
    case static_cast<int>(textEffect_t::PA_GROW_UP):
    case static_cast<int>(textEffect_t::PA_GROW_DOWN):
    {
      return true;
    }
    # endif // ENA_GROW
    default:
      return false;
  }
}

/*******************************************************
 * handlePluginWrite : process commands
 ******************************************************/
bool P104_data_struct::handlePluginWrite(taskIndex_t   taskIndex,
                                         const String& string) {
  # ifdef P104_USE_COMMANDS
  bool reconfigure = false;
  # endif // ifdef P104_USE_COMMANDS
  bool   success = false;
  String command = parseString(string, 1);

  if (command.equals(F("dotmatrix"))) { // main command: dotmatrix
    String sub = parseString(string, 2);

    int zoneIndex;
    String string4 = parseStringKeepCase(string, 4);
    # ifdef P104_USE_COMMANDS
    int value4;
    validIntFromString(string4, value4);
    # endif // ifdef P104_USE_COMMANDS

    // Global subcommands
    if (sub.equals(F("clear")) && // subcommand: clear[,all]
        (string4.isEmpty() ||
         string4.equalsIgnoreCase(F("all")))) {
      P->displayClear();
      success = true;
    }

    if (sub.equals(F("update")) && // subcommand: update[,all]
        (string4.isEmpty() ||
         string4.equalsIgnoreCase(F("all")))) {
      updateZone(0, P104_zone_struct(0));
      success = true;
    }

    // Zone-specific subcommands
    if (validIntFromString(parseString(string, 3), zoneIndex) &&
        (zoneIndex > 0) &&
        (static_cast<unsigned int>(zoneIndex) <= zones.size())) {
      // subcommands are processed in the same order as they are presented in the UI
      for (auto it = zones.begin(); it != zones.end() && !success; ++it) {
        if ((it->zone == zoneIndex)) {  // This zone
          if (sub.equals(F("clear"))) { // subcommand: clear,<zone>
            P->displayClear(zoneIndex - 1);
            success = true;
            break;
          }

          if (sub.equals(F("update"))) { // subcommand: update,<zone>
            updateZone(zoneIndex, *it);
            success = true;
            break;
          }

          # ifdef P104_USE_COMMANDS

          if (sub.equals(F("size")) && // subcommand: size,<zone>,<size> (1..)
              (value4 > 0) &&
              (value4 <= P104_MAX_MODULES_PER_ZONE)) {
            reconfigure = (it->size != value4);
            it->size    = value4;
            success     = true;
            break;
          }
          # endif // ifdef P104_USE_COMMANDS

          if ((sub.equals(F("txt")) ||                                                          // subcommand: [set]txt,<zone>,<text> (only
                                                                                                // allowed for zones with Text content)
               sub.equals(F("settxt"))) &&
              ((it->content == P104_CONTENT_TEXT) || (it->content == P104_CONTENT_TEXT_REV))) { // no length check, so longer than the UI
                                                                                                // allows is made possible
            if (sub.equals(F("settxt")) &&                                                      // subcommand: settxt,<zone>,<text> (stores
                                                                                                // the text in the settings, is not saved)
                (string4.length() <= P104_MAX_TEXT_LENGTH_PER_ZONE)) {
              it->text = string4;                                                               // Only if not too long, could 'blow up' the
                                                                                                // settings when saved
            }
            displayOneZoneText(zoneIndex - 1, *it, string4);
            success = true;
            break;
          }

          # ifdef P104_USE_COMMANDS

          if (sub.equals(F("content")) && // subcommand: content,<zone>,<contenttype> (0..<P104_CONTENT_count>-1)
              (value4 >= 0) &&
              (value4 < P104_CONTENT_count)) {
            reconfigure = (it->content != value4);
            it->content = value4;
            success     = true;
            break;
          }

          if (sub.equals(F("alignment")) &&                             // subcommand: alignment,<zone>,<alignment> (0..3)
              (value4 >= 0) &&
              (value4 <= static_cast<int>(textPosition_t::PA_RIGHT))) { // last item in the enum
            it->alignment = value4;
            success       = true;
            break;
          }

          if (sub.equals(F("anim.in")) && // subcommand: anim.in,<zone>,<animation> (1..)
              isAnimationAvailable(value4)) {
            it->animationIn = value4;
            success         = true;
            break;
          }

          if (sub.equals(F("speed")) && // subcommand: speed,<zone>,<speed_ms> (0..P104_MAX_SPEED_PAUSE_VALUE)
              (value4 >= 0) &&
              (value4 <= P104_MAX_SPEED_PAUSE_VALUE)) {
            it->speed = value4;
            success   = true;
            break;
          }

          if (sub.equals(F("anim.out")) && // subcommand: anim.out,<zone>,<animation> (0..)
              isAnimationAvailable(value4, true)) {
            it->animationOut = value4;
            success          = true;
            break;
          }

          if (sub.equals(F("pause")) && // subcommand: pause,<zone>,<pause_ms> (0..P104_MAX_SPEED_PAUSE_VALUE)
              (value4 >= 0) &&
              (value4 <= P104_MAX_SPEED_PAUSE_VALUE)) {
            it->pause = value4;
            success   = true;
            break;
          }

          if (sub.equals(F("font")) && // subcommand: font,<zone>,<font id> (only for incuded font id's)
              (
                (value4 == 0)
                #  ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
                || (value4 == P104_DOUBLE_HEIGHT_FONT_ID)
                #  endif // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
                #  ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
                || (value4 == P104_FULL_DOUBLEHEIGHT_FONT_ID)
                #  endif // ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
                #  ifdef P104_USE_VERTICAL_FONT
                || (value4 == P104_VERTICAL_FONT_ID)
                #  endif // ifdef P104_USE_VERTICAL_FONT
                #  ifdef P104_USE_EXT_ASCII_FONT
                || (value4 == P104_EXT_ASCII_FONT_ID)
                #  endif // ifdef P104_USE_EXT_ASCII_FONT
                #  ifdef P104_USE_ARABIC_FONT
                || (value4 == P104_ARABIC_FONT_ID)
                #  endif // ifdef P104_USE_ARABIC_FONT
                #  ifdef P104_USE_GREEK_FONT
                || (value4 == P104_GREEK_FONT_ID)
                #  endif // ifdef P104_USE_GREEK_FONT
                #  ifdef P104_USE_KATAKANA_FONT
                || (value4 == P104_KATAKANA_FONT_ID)
                #  endif // ifdef P104_USE_KATAKANA_FONT
              )
              ) {
            reconfigure = (it->font != value4);
            it->font    = value4;
            success     = true;
            break;
          }

          #  if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)

          if (sub.equals(F("layout")) && // subcommand: layout,<zone>,<layout> (0..2), only when double-height font is available
              (value4 >= 0) &&
              (value4 <= P104_LAYOUT_DOUBLE_LOWER)) {
            reconfigure = (it->layout != value4);
            it->layout  = value4;
            success     = true;
            break;
          }
          #  endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)

          if (sub.equals(F("specialeffect")) && // subcommand: specialeffect,<zone>,<effect> (0..3)
              (value4 >= 0) &&
              (value4 <= P104_SPECIAL_EFFECT_BOTH)) {
            reconfigure       = (it->specialEffect != value4);
            it->specialEffect = value4;
            success           = true;
            break;
          }

          if (sub.equals(F("offset")) && // subcommand: offset,<zone>,<size> (0..<size>-1)
              (value4 >= 0) &&
              (value4 < P104_MAX_MODULES_PER_ZONE) &&
              (value4 < it->size)) {
            reconfigure = (it->offset != value4);
            it->offset  = value4;
            success     = true;
            break;
          }

          if (sub.equals(F("brightness")) && // subcommand: brightness,<zone>,<brightness> (0..15)
              (value4 >= 0) &&
              (value4 <= P104_BRIGHTNESS_MAX)) {
            it->brightness = value4;
            P->setIntensity(zoneIndex - 1, it->brightness); // Change brightness directly
            success = true;
            break;
          }

          if (sub.equals(F("repeat")) && // subcommand: repeaat,<zone>,<repeat_sec> (-1..86400 = 24h)
              (value4 >= -1) &&
              (value4 <= P104_MAX_REPEATDELAY_VALUE)) {
            it->repeatDelay = value4;
            success         = true;

            if (it->repeatDelay > -1) {
              it->_repeatTimer = millis();
            }
            break;
          }
          # else // ifdef P104_USE_COMMANDS
          {
            String validCommands = F(
              "|size|content|alignment|anim.in|speed|anim.out|pause|font|layout|specialeffect|offset|brightness|repeat|");
            String testSub = F("|");
            testSub += sub;
            testSub += '|';

            if (validCommands.indexOf(testSub) > -1) {
              addLog(LOG_LEVEL_ERROR, F("dotmatrix: subcommand not included in build."));
            }
          }
          # endif // ifdef P104_USE_COMMANDS

          # ifdef P104_USE_BAR_GRAPH

          if ((sub.equals(F("bar")) ||                   // subcommand: [set]bar,<zone>,<graph-string> (only allowed for zones with Bargraph
                                                         // content)
               sub.equals(F("setbar"))) &&
              (it->content == P104_CONTENT_BAR_GRAPH)) { // no length check, so longer than the UI allows is made possible
            if (sub.equals(F("setbar")) &&               // subcommand: setbar,<zone>,<graph-string> (stores the graph-string in the
                                                         // settings, is not saved)
                (string4.length() <= P104_MAX_TEXT_LENGTH_PER_ZONE)) {
              it->text = string4;                        // Only if not too long, could 'blow up' the settings when saved
            }
            displayBarGraph(zoneIndex - 1, *it, string4);
            success = true;
            break;
          }
          # endif // ifdef P104_USE_BAR_GRAPH

          if (success) { // Reset the repeat timer
            if (it->repeatDelay > -1) {
              it->_repeatTimer = millis();
            }
          }
        }
      }
    }
  }

  # ifdef P104_USE_COMMANDS

  if (reconfigure) {
    configureZones(); // Re-initialize
    success = true;   // Successful
  }
  # endif // ifdef P104_USE_COMMANDS

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;

    if (log.reserve(32 + string.length())) {
      log = F("dotmatrix: command ");

      if (!success) { log += F("UN"); }
      log += F("succesful: ");
      log += string;
      addLog(LOG_LEVEL_INFO, log);
    }
  }

  return success; // Default: unknown command
}

int8_t getTime(char *psz,
               bool  seconds  = false,
               bool  colon    = true,
               bool  time12h  = false,
               bool  timeAmpm = false) {
  uint16_t h, M, s;
  String   ampm;

  # ifdef P104_USE_DATETIME_OPTIONS

  if (time12h) {
    if (timeAmpm) {
      ampm = (node_time.hour() >= 12 ? F("p") : F("a"));
    }
    h = node_time.hour() % 12;

    if (h == 0) { h = 12; }
  } else
  # endif // ifdef P104_USE_DATETIME_OPTIONS
  {
    h = node_time.hour();
  }
  M = node_time.minute();

  if (!seconds) {
    sprintf_P(psz, PSTR("%02d%c%02d%s"), h, (colon ? ':' : ' '), M, ampm.c_str());
  } else {
    s = node_time.second();
    sprintf_P(psz, PSTR("%02d%c%02d %02d%s"), h, (colon ? ':' : ' '), M, s, ampm.c_str());
  }
  return M;
}

void getDate(char     *psz,
             bool      showYear = true,
             bool      fourDgt  = false
             # ifdef   P104_USE_DATETIME_OPTIONS
             , uint8_t dateFmt = 0
             , uint8_t dateSep = 0
             # endif // ifdef P104_USE_DATETIME_OPTIONS
             ) {
  uint16_t d, m, y;
  uint16_t year = (fourDgt ? node_time.year() : node_time.year() - 2000);

  # ifdef P104_USE_DATETIME_OPTIONS
  String separators = F(" /-.");
  char   sep        = separators[dateSep];
  # else // ifdef P104_USE_DATETIME_OPTIONS
  char sep = ' ';
  # endif // ifdef P104_USE_DATETIME_OPTIONS

  d = node_time.day();
  m = node_time.month();
  y = year;
  # ifdef P104_USE_DATETIME_OPTIONS

  if (showYear) {
    switch (dateFmt) {
      case P104_DATE_FORMAT_US:
        d = node_time.month();
        m = node_time.day();
        y = year;
        break;
      case P104_DATE_FORMAT_JP:
        d = year;
        m = node_time.month();
        y = node_time.day();
        break;
    }
  } else {
    if ((dateFmt == P104_DATE_FORMAT_US) ||
        (dateFmt == P104_DATE_FORMAT_JP)) {
      std::swap(d, m);
    }
  }
  # endif // ifdef P104_USE_DATETIME_OPTIONS

  if (showYear) {
    sprintf_P(psz, PSTR("%02d%c%02d%c%02d"), d, sep, m, sep, y);     // %02d will expand to 04 when needed
  } else {
    sprintf_P(psz, PSTR("%02d%c%02d"), d, sep, m);
  }
}

uint8_t getDateTime(char     *psz,
                    bool      colon    = true,
                    bool      time12h  = false,
                    bool      timeAmpm = false,
                    bool      fourDgt  = false
                    # ifdef   P104_USE_DATETIME_OPTIONS
                    , uint8_t dateFmt = 0
                    , uint8_t dateSep = 0
                    # endif // ifdef P104_USE_DATETIME_OPTIONS
                    ) {
  uint16_t d, M, y;
  uint8_t  h, m;
  uint16_t year = (fourDgt ? node_time.year() : node_time.year() - 2000);
  String   ampm;

  # ifdef P104_USE_DATETIME_OPTIONS
  String separators = F(" /-.");
  char   sep        = separators[dateSep];
  # else // ifdef P104_USE_DATETIME_OPTIONS
  char sep = ' ';
  # endif // ifdef P104_USE_DATETIME_OPTIONS

  # ifdef P104_USE_DATETIME_OPTIONS

  if (time12h) {
    if (timeAmpm) {
      ampm = (node_time.hour() >= 12 ? F("p") : F("a"));
    }
    h = node_time.hour() % 12;

    if (h == 0) { h = 12; }
  } else
  # endif // ifdef P104_USE_DATETIME_OPTIONS
  {
    h = node_time.hour();
  }
  M = node_time.minute();

  # ifdef P104_USE_DATETIME_OPTIONS

  switch (dateFmt) {
    case P104_DATE_FORMAT_US:
      d = node_time.month();
      m = node_time.day();
      y = year;
      break;
    case P104_DATE_FORMAT_JP:
      d = year;
      m = node_time.month();
      y = node_time.day();
      break;
    default:
  # endif // ifdef P104_USE_DATETIME_OPTIONS
  d = node_time.day();
  m = node_time.month();
  y = year;
  # ifdef P104_USE_DATETIME_OPTIONS
}

  # endif // ifdef P104_USE_DATETIME_OPTIONS
  sprintf_P(psz, PSTR("%02d%c%02d%c%02d %02d%c%02d%s"), d, sep, m, sep, y, h, (colon ? ':' : ' '), M, ampm.c_str()); // %02d will expand to
                                                                                                                     // 04 when needed
  return M;
}

# if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
void createHString(String& string) {
  for (uint32_t i = 0; i < string.length(); i++) {
    string[i] |= 0x80;
  }
}

# endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)

void reverseStr(String& str) {
  const uint16_t n = str.length();

  // Swap character starting from two corners
  for (uint16_t i = 0; i < n / 2; i++) {
    std::swap(str[i], str[n - i - 1]);
  }
}

/************************************************************************
 * execute all PLUGIN_ONE_PER_SECOND tasks
 ***********************************************************************/
bool P104_data_struct::handlePluginOncePerSecond(struct EventStruct *event) {
  bool redisplay = false;
  bool success   = false;
  bool useSeconds;

  # ifdef P104_USE_DATETIME_OPTIONS
  bool useFlasher = !bitRead(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_FLASH);
  bool time12h    = bitRead(P104_CONFIG_DATETIME,  P104_CONFIG_DATETIME_12H);
  bool timeAmpm   = bitRead(P104_CONFIG_DATETIME,  P104_CONFIG_DATETIME_AMPM);
  bool year4dgt   = bitRead(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_YEAR4DGT);
  # else // ifdef P104_USE_DATETIME_OPTIONS
  bool useFlasher = true;
  bool time12h    = false;
  bool timeAmpm   = false;
  bool year4dgt   = false;
  # endif // ifdef P104_USE_DATETIME_OPTIONS
  bool newFlasher = !flasher && useFlasher;

  for (auto it = zones.begin(); it != zones.end(); ++it) {
    redisplay = false;

    if (P->getZoneStatus(it->zone - 1)) { // Animations done?
      switch (it->content) {
        case P104_CONTENT_TIME:           // time
        case P104_CONTENT_TIME_SEC:       // time sec
        {
          useSeconds = it->content == P104_CONTENT_TIME_SEC;
          int8_t m = getTime(szTimeL, useSeconds, flasher || !useFlasher, time12h, timeAmpm);
          flasher          = newFlasher;
          redisplay        = useFlasher || useSeconds || (it->_lastChecked != m);
          it->_lastChecked = m;
          break;
        }
        case P104_CONTENT_DATE4: // date/4
        case P104_CONTENT_DATE6: // date/6
        {
          if (it->_lastChecked != node_time.day()) {
            getDate(szTimeL,
                    it->content != P104_CONTENT_DATE4,
                    year4dgt
                    # ifdef P104_USE_DATETIME_OPTIONS
                    , get4BitFromUL(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_FORMAT)
                    , get4BitFromUL(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_SEP_CHAR)
                    # endif // ifdef P104_USE_DATETIME_OPTIONS
                    );
            redisplay        = true;
            it->_lastChecked = node_time.day();
          }
          break;
        }
        case P104_CONTENT_DATE_TIME: // date-time/9
        {
          int8_t m = getDateTime(szTimeL,
                                 flasher || !useFlasher,
                                 time12h,
                                 timeAmpm,
                                 year4dgt
                                 # ifdef P104_USE_DATETIME_OPTIONS
                                 , get4BitFromUL(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_FORMAT)
                                 , get4BitFromUL(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_SEP_CHAR)
                                 # endif // ifdef P104_USE_DATETIME_OPTIONS
                                 );
          flasher          = newFlasher;
          redisplay        = useFlasher || (it->_lastChecked != m);
          it->_lastChecked = m;
          break;
        }
        default:
          break;
      }

      if (redisplay) {
        displayOneZoneText(it->zone - 1, *it, String(szTimeL));
        P->displayReset(it->zone - 1);

        if (it->repeatDelay > -1) {
          it->_repeatTimer = millis();
        }
      }
    }
  }

  if (redisplay) {
    // synchronise the start
    P->synchZoneStart();
  }
  return redisplay || success;
}

/***************************************************
 * restart a zone if the repeat delay (if any) has passed
 **************************************************/
void P104_data_struct::checkRepeatTimer(uint8_t z) {
  bool handled = false;

  for (auto it = zones.begin(); it != zones.end() && !handled; ++it) {
    if (it->zone == z + 1) {
      handled = true;

      if ((it->repeatDelay > -1) && (timePassedSince(it->_repeatTimer) >= (it->repeatDelay - 1) * 1000)) { // Compensated for the '1' in
                                                                                                           // PLUGIN_ONE_PER_SECOND
        # ifdef P104_DEBUG

        if (logAllText && loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log;
          log.reserve(51);
          log  = F("dotmatrix: Repeat zone: ");
          log += it->zone;
          log += F(" delay: ");
          log += it->repeatDelay;
          log += F(" (");
          log += String(timePassedSince(it->_repeatTimer) / 1000.0f); // Decimals can be useful here
          log += ')';
          addLog(LOG_LEVEL_INFO, log);
        }
        # endif // ifdef P104_DEBUG

        if ((it->content == P104_CONTENT_TEXT) ||
            (it->content == P104_CONTENT_TEXT_REV)) {
          displayOneZoneText(it->zone - 1, *it, sZoneInitial[it->zone - 1]); // Re-send last displayed text
          P->displayReset(it->zone - 1);
        }

        if ((it->content == P104_CONTENT_TIME) ||
            (it->content == P104_CONTENT_TIME_SEC) ||
            (it->content == P104_CONTENT_DATE4) ||
            (it->content == P104_CONTENT_DATE6) ||
            (it->content == P104_CONTENT_DATE_TIME)) {
          it->_lastChecked = -1; // Invalidate so next run will re-display the date/time
        }
        # ifdef P104_USE_BAR_GRAPH

        if (it->content == P104_CONTENT_BAR_GRAPH) {
          displayBarGraph(it->zone - 1, *it, sZoneInitial[it->zone - 1]); // Re-send last displayed bar graph
        }
        # endif // ifdef P104_USE_BAR_GRAPH
        it->_repeatTimer = millis();
      }
    }
  }
}

/******************************************
 * enquoteString wrap in ", ' or ` unless all 3 quote types are used
 *****************************************/
String P104_data_struct::enquoteString(const String& input) {
  char quoteChar = '"';

  if (input.indexOf(quoteChar) > -1) {
    quoteChar = '\'';

    if (input.indexOf(quoteChar) > -1) {
      quoteChar = '`';

      if (input.indexOf(quoteChar) > -1) {
        return input; // All types of supported quotes used, return original string
      }
    }
  }
  String result;

  result.reserve(input.length() + 2);
  result  = quoteChar;
  result += input;
  result += quoteChar;

  return result;
}

/***************************************
 * saveSettings gather the zones data from the UI and store in customsettings
 **************************************/
bool P104_data_struct::saveSettings() {
  error = EMPTY_STRING; // Clear
  String zbuffer;

  # ifdef P104_DEBUG_DEV
  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO) &&
      log.reserve(64)) {
    log  = F("P104: saving zones, count: ");
    log += expectedZones;
    addLog(LOG_LEVEL_INFO, log);
  }
  # endif // ifdef P104_DEBUG_DEV

  uint8_t index      = 0;
  uint8_t action     = 0;
  uint8_t zoneIndex  = 0;
  int8_t  zoneOffset = 0;

  zones.clear(); // Start afresh

  for (uint8_t zCounter = 0; zCounter < expectedZones; zCounter++) {
    # ifdef P104_USE_ZONE_ACTIONS
    action = getFormItemInt(getPluginCustomArgName(index + P104_OFFSET_ACTION));

    if (((action == P104_ACTION_ADD_ABOVE) && (zoneOrder == 0)) ||
        ((action == P104_ACTION_ADD_BELOW) && (zoneOrder == 1))) {
      zones.push_back(P104_zone_struct(0));
      zoneOffset++;
      #  ifdef P104_DEBUG_DEV

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log  = F("P104: insert before zone: ");
        log += (zoneIndex + 1);
        addLog(LOG_LEVEL_INFO, log);
      }
      #  endif // ifdef P104_DEBUG_DEV
    }
    # endif    // ifdef P104_USE_ZONE_ACTIONS
    zoneIndex = zCounter + zoneOffset;

    if (action != P104_ACTION_DELETE) {
      zones.push_back(P104_zone_struct(zoneIndex + 1));

      zones[zoneIndex].size          = getFormItemIntCustomArgName(index + P104_OFFSET_SIZE);
      zones[zoneIndex].text          = enquoteString(webArg(getPluginCustomArgName(index + P104_OFFSET_TEXT)));
      zones[zoneIndex].content       = getFormItemIntCustomArgName(index + P104_OFFSET_CONTENT);
      zones[zoneIndex].alignment     = getFormItemIntCustomArgName(index + P104_OFFSET_ALIGNMENT);
      zones[zoneIndex].animationIn   = getFormItemIntCustomArgName(index + P104_OFFSET_ANIM_IN);
      zones[zoneIndex].speed         = getFormItemIntCustomArgName(index + P104_OFFSET_SPEED);
      zones[zoneIndex].animationOut  = getFormItemIntCustomArgName(index + P104_OFFSET_ANIM_OUT);
      zones[zoneIndex].pause         = getFormItemIntCustomArgName(index + P104_OFFSET_PAUSE);
      zones[zoneIndex].font          = getFormItemIntCustomArgName(index + P104_OFFSET_FONT);
      zones[zoneIndex].layout        = getFormItemIntCustomArgName(index + P104_OFFSET_LAYOUT);
      zones[zoneIndex].specialEffect = getFormItemIntCustomArgName(index + P104_OFFSET_SPEC_EFFECT);
      zones[zoneIndex].offset        = getFormItemIntCustomArgName(index + P104_OFFSET_OFFSET);

      if (zones[zoneIndex].size != 0) { // for newly added zone, use defaults
        zones[zoneIndex].brightness  = getFormItemIntCustomArgName(index + P104_OFFSET_BRIGHTNESS);
        zones[zoneIndex].repeatDelay = getFormItemIntCustomArgName(index + P104_OFFSET_REPEATDELAY);
      }
    } else {
      zoneOffset--;
    }
    # ifdef P104_DEBUG_DEV

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log  = F("P104: add zone: ");
      log += (zoneIndex + 1);
      addLog(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef P104_DEBUG_DEV

    # ifdef P104_USE_ZONE_ACTIONS

    if (((action == P104_ACTION_ADD_BELOW) && (zoneOrder == 0)) ||
        ((action == P104_ACTION_ADD_ABOVE) && (zoneOrder == 1))) {
      zones.push_back(P104_zone_struct(0));
      zoneOffset++;
      #  ifdef P104_DEBUG_DEV

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log  = F("P104: insert after zone: ");
        log += (zoneIndex + 2);
        addLog(LOG_LEVEL_INFO, log);
      }
      #  endif // ifdef P104_DEBUG_DEV
    }
    # endif    // ifdef P104_USE_ZONE_ACTIONS

    index += P104_OFFSET_COUNT;
    delay(0);
  }

  uint16_t bufSize = (zones.size() * 58) + 50; // Use actual size and add a little spare
  numDevices = 0;                              // Count the number of connected display units

  if (zbuffer.reserve(bufSize)) {
    for (auto it = zones.begin(); it != zones.end(); ++it) {
      // WARNING: Order of values should match the numeric order of P104_OFFSET_* values
      zbuffer += it->size;                                       // 2
      zbuffer += P104_FIELD_SEP;                                 // 1
      zbuffer += it->text;                                       // ~15
      zbuffer += P104_FIELD_SEP;                                 // 1
      zbuffer += it->content;                                    // 1
      zbuffer += P104_FIELD_SEP;                                 // 1
      zbuffer += it->alignment;                                  // 1
      zbuffer += P104_FIELD_SEP;                                 // 1
      zbuffer += it->animationIn;                                // 2
      zbuffer += P104_FIELD_SEP;                                 // 1
      zbuffer += it->speed;                                      // 5
      zbuffer += P104_FIELD_SEP;                                 // 1
      zbuffer += it->animationOut;                               // 2
      zbuffer += P104_FIELD_SEP;                                 // 1
      zbuffer += it->pause;                                      // 5
      zbuffer += P104_FIELD_SEP;                                 // 1
      zbuffer += it->font;                                       // 1
      zbuffer += P104_FIELD_SEP;                                 // 1
      zbuffer += it->layout;                                     // 1
      zbuffer += P104_FIELD_SEP;                                 // 1
      zbuffer += it->specialEffect;                              // 1
      zbuffer += P104_FIELD_SEP;                                 // 1
      zbuffer += it->offset;                                     // 2
      zbuffer += P104_FIELD_SEP;                                 // 1
      zbuffer += it->brightness;                                 // 2
      zbuffer += P104_FIELD_SEP;                                 // 1
      zbuffer += it->repeatDelay;                                // 4
      zbuffer += P104_FIELD_SEP;                                 // 1

      zbuffer += P104_ZONE_SEP;                                  // 1
                                                                 // 58 total
      numDevices += (it->size != 0 ? it->size : 1) + it->offset; // Count corrected for newly added zones
      # ifdef P104_DEBUG_DEV

      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log  = F("P104: append zone: ");
        log += it->zone;
        log += F(" buffer len: ");
        log += zbuffer.length();
        log += F(" size: ");
        log += it->size;
        addLog(LOG_LEVEL_INFO, log);
      }
      # endif // ifdef P104_DEBUG_DEV
      delay(0);
    }

    if (numDevices > 255) {
      error += F("More than 255 modules configured (");
      error += numDevices;
      error += ')';
      error += '\n';
    }
    zbuffer.trim();
    StoredSettings.bufferSize = zbuffer.length();
    # ifdef P104_DEBUG_DEV

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log  = F("P104: saveSettings zones: ");
      log += expectedZones;
      log += F(" buffer length: ");
      log += StoredSettings.bufferSize;
      log += F(" numDevices: ");
      log += numDevices;
      addLog(LOG_LEVEL_INFO, log);
    }
    # endif // ifdef P104_DEBUG_DEV
  } else {
    addLog(LOG_LEVEL_ERROR, F("DOTMATRIX: Can't allocate string for saving settings, insufficient memory!"));
    return false; // Don't continue
  }

  if (StoredSettings.bufferSize < sizeof(StoredSettings.buffer)) {
    if (!safe_strncpy(StoredSettings.buffer, zbuffer.c_str(), StoredSettings.bufferSize + 1)) {
      error.reserve(42);
      error += F("Moving settings to config buffer failed.\n");
      addLog(LOG_LEVEL_ERROR, error);
      addLog(LOG_LEVEL_ERROR, zbuffer);
      return false;
    }
    uint16_t structDataSize = StoredSettings.bufferSize + sizeof(StoredSettings.bufferSize);
    # ifdef P104_DEBUG_DEV

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log  = F("P104: saveSettings structSize: ");
      log += structDataSize;
      log += F(" taskindex: ");
      log += taskIndex;
      addLog(LOG_LEVEL_INFO, log);
      zbuffer.replace(P104_FIELD_SEP, P104_FIELD_DISP);
      zbuffer.replace(P104_ZONE_SEP,  P104_ZONE_DISP);
      addLog(LOG_LEVEL_INFO, zbuffer);
    }
    # endif // ifdef P104_DEBUG_DEV
    zbuffer.clear(); // Clear string after reporting any errors
    error += SaveCustomTaskSettings(taskIndex, (uint8_t *)&StoredSettings, structDataSize);
    return error.isEmpty();
  }
  error.reserve(55);
  error += F("Total combination of Zones & text too long to store.\n");
  addLog(LOG_LEVEL_ERROR, error);
  return false;
}

/**************************************************************
* webform_load
**************************************************************/
bool P104_data_struct::webform_load(struct EventStruct *event) {
  addFormSubHeader(F("Device settings"));

  {                                       // Hardware types
    # define P104_hardwareTypeCount 8
    const __FlashStringHelper *hardwareTypes[P104_hardwareTypeCount] = {
      F("Generic (DR:0, CR:1, RR:0)"),    // 010
      F("Parola (DR:1, CR:1, RR:0)"),     // 110
      F("FC16 (DR:1, CR:0, RR:0)"),       // 100
      F("IC Station (DR:1, CR:1, RR:1)"), // 111
      F("Other 1 (DR:0, CR:0, RR:0)"),    // 000
      F("Other 2 (DR:0, CR:0, RR:1)"),    // 001
      F("Other 3 (DR:0, CR:1, RR:1)"),    // 011
      F("Other 4 (DR:1, CR:0, RR:1)")     // 101
    };
    int hardwareOptions[P104_hardwareTypeCount] = {
      static_cast<int>(MD_MAX72XX::moduleType_t::GENERIC_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::PAROLA_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::FC16_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::ICSTATION_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::DR0CR0RR0_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::DR0CR0RR1_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::DR0CR1RR1_HW),
      static_cast<int>(MD_MAX72XX::moduleType_t::DR1CR0RR1_HW)
    };
    addFormSelector(F("Hardware type"),
                    F("plugin_104_hardware"),
                    P104_hardwareTypeCount,
                    hardwareTypes,
                    hardwareOptions,
                    P104_CONFIG_HARDWARETYPE);
    # ifdef P104_ADD_SETTINGS_NOTES
    addFormNote(F("DR = Digits as Rows, CR = Column Reversed, RR = Row Reversed; 0 = no, 1 = yes."));
    # endif // ifdef P104_ADD_SETTINGS_NOTES
  }

  {
    addFormCheckBox(F("Clear display on disable"), F("plugin_104_cleardisable"),
                    bitRead(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_CLEAR_DISABLE));

    addFormCheckBox(F("Log all displayed text (info)"),
                    F("plugin_104_logalltext"),
                    bitRead(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_LOG_ALL_TEXT));
  }

  # ifdef P104_USE_DATETIME_OPTIONS
  {
    addFormSubHeader(F("Content options"));

    addFormCheckBox(F("Clock with flashing colon"), F("plugin_104_clockflash"), !bitRead(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_FLASH));
    addFormCheckBox(F("Clock 12h display"),         F("plugin_104_clock12h"),   bitRead(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_12H));
    addFormCheckBox(F("Clock 12h AM/PM indicator"), F("plugin_104_clockampm"),  bitRead(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_AMPM));
  }
  { // Date format
    const __FlashStringHelper *dateFormats[] = {
      F("Day Month [Year]"),
      F("Month Day [Year] (US-style)"),
      F("[Year] Month Day (Japanese-style)")
    };
    int dateFormatOptions[] = {
      P104_DATE_FORMAT_EU,
      P104_DATE_FORMAT_US,
      P104_DATE_FORMAT_JP
    };
    addFormSelector(F("Date format"), F("plugin_104_dateformat"),
                    3,
                    dateFormats, dateFormatOptions,
                    get4BitFromUL(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_FORMAT));
  }
  { // Date separator
    const __FlashStringHelper *dateSeparators[] = {
      F("Space"),
      F("Slash /"),
      F("Dash -"),
      F("Dot <b>.</b>")
    };
    int dateSeparatorOptions[] = {
      P104_DATE_SEPARATOR_SPACE,
      P104_DATE_SEPARATOR_SLASH,
      P104_DATE_SEPARATOR_DASH,
      P104_DATE_SEPARATOR_DOT
    };
    addFormSelector(F("Date separator"), F("plugin_104_dateseparator"),
                    4,
                    dateSeparators, dateSeparatorOptions,
                    get4BitFromUL(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_SEP_CHAR));

    addFormCheckBox(F("Year uses 4 digits"), F("plugin_104_year4dgt"), bitRead(P104_CONFIG_DATETIME, P104_CONFIG_DATETIME_YEAR4DGT));
  }
  # endif // ifdef P104_USE_DATETIME_OPTIONS

  addFormSubHeader(F("Zones"));

  { // Zones
    String zonesList[P104_MAX_ZONES];
    int    zonesOptions[P104_MAX_ZONES];

    for (uint8_t i = 0; i < P104_MAX_ZONES; i++) {
      zonesList[i]    = String(i + 1);
      zonesOptions[i] = i + 1; // No 0 needed or wanted
    }
    # if defined(P104_USE_TOOLTIPS) || defined(P104_ADD_SETTINGS_NOTES)
    String zonetip;

    if (zonetip.reserve(90)) {
      zonetip  = F("Select between 1 and ");
      zonetip += P104_MAX_ZONES;
      zonetip += F(" zones, changing");
      #  ifdef P104_USE_ZONE_ORDERING
      zonetip += F(" Zones or Zone order");
      #  endif // ifdef P104_USE_ZONE_ORDERING
      zonetip += F(" will save and reload the page.");
    }
    # endif    // if defined(P104_USE_TOOLTIPS) || defined(P104_ADD_SETTINGS_NOTES)
    addFormSelector(F("Zones"), F("plugin_104_zone"), P104_MAX_ZONES, zonesList, zonesOptions, NULL, P104_CONFIG_ZONE_COUNT, true
                    # ifdef P104_USE_TOOLTIPS
                    , zonetip
                    # endif // ifdef P104_USE_TOOLTIPS
                    );

    # ifdef P104_USE_ZONE_ORDERING
    String orderTypes[] = {
      F("Numeric order (1..n)"),
      F("Display order (n..1)")
    };
    int orderOptions[] = { 0, 1 };
    addFormSelector(F("Zone order"), F("plugin_104_zoneorder"), 2, orderTypes, orderOptions, NULL,
                    bitRead(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_ZONE_ORDER) ? 1 : 0, true
                    #  ifdef P104_USE_TOOLTIPS
                    , zonetip
                    #  endif // ifdef P104_USE_TOOLTIPS
                    );
    # endif                  // ifdef P104_USE_ZONE_ORDERING
    # ifdef P104_ADD_SETTINGS_NOTES
    addFormNote(zonetip);
    # endif // ifdef P104_ADD_SETTINGS_NOTES
  }
  expectedZones = P104_CONFIG_ZONE_COUNT;

  if (expectedZones == 0) { expectedZones++; } // Minimum of 1 zone

  { // Optionlists and zones table
    const __FlashStringHelper *alignmentTypes[3] = {
      F("Left"),
      F("Center"),
      F("Right")
    };
    int alignmentOptions[3] = {
      static_cast<int>(textPosition_t::PA_LEFT),
      static_cast<int>(textPosition_t::PA_CENTER),
      static_cast<int>(textPosition_t::PA_RIGHT)
    };
    int animationCount = 6;
    # if ENA_SPRITE
    animationCount += 1;
    # endif // ENA_SPRITE
    # if ENA_MISC
    animationCount += 6;
    # endif // ENA_MISC
    # if ENA_WIPE
    animationCount += 2;
    # endif // ENA_WIPE
    # if ENA_SCAN
    animationCount += 4;
    # endif // ENA_SCAN
    # if ENA_OPNCLS
    animationCount += 4;
    # endif // ENA_OPNCLS
    # if ENA_SCR_DIA
    animationCount += 4;
    # endif // ENA_SCR_DIA
    # if ENA_GROW
    animationCount += 2;
    # endif // ENA_GROW
    String animationTypes[] {
      F("None")
      , F("Print")
      , F("Scroll up")
      , F("Scroll down")
      , F("Scroll left *")
      , F("Scroll right *")
    # if ENA_SPRITE
      , F("Sprite")
    # endif // ENA_SPRITE
    # if ENA_MISC
      , F("Slice *")
      , F("Mesh")
      , F("Fade")
      , F("Dissolve")
      , F("Blinds")
      , F("Random")
    # endif // ENA_MISC
    # if ENA_WIPE
      , F("Wipe")
      , F("Wipe w. cursor")
    # endif // ENA_WIPE
    # if ENA_SCAN
      , F("Scan horiz.")
      , F("Scan horiz. cursor")
      , F("Scan vert.")
      , F("Scan vert. cursor")
    # endif // ENA_SCAN
    # if ENA_OPNCLS
      , F("Opening")
      , F("Opening w. cursor")
      , F("Closing")
      , F("Closing w. cursor")
    # endif // ENA_OPNCLS
    # if ENA_SCR_DIA
      , F("Scroll up left *")
      , F("Scroll up right *")
      , F("Scroll down left *")
      , F("Scroll down right *")
    # endif // ENA_SCR_DIA
    # if ENA_GROW
      , F("Grow up")
      , F("Grow down")
    # endif // ENA_GROW
    };
    int animationOptions[] = {
      static_cast<int>(textEffect_t::PA_NO_EFFECT)
      , static_cast<int>(textEffect_t::PA_PRINT)
      , static_cast<int>(textEffect_t::PA_SCROLL_UP)
      , static_cast<int>(textEffect_t::PA_SCROLL_DOWN)
      , static_cast<int>(textEffect_t::PA_SCROLL_LEFT)
      , static_cast<int>(textEffect_t::PA_SCROLL_RIGHT)
    # if ENA_SPRITE
      , static_cast<int>(textEffect_t::PA_SPRITE)
    # endif // ENA_SPRITE
    # if ENA_MISC
      , static_cast<int>(textEffect_t::PA_SLICE)
      , static_cast<int>(textEffect_t::PA_MESH)
      , static_cast<int>(textEffect_t::PA_FADE)
      , static_cast<int>(textEffect_t::PA_DISSOLVE)
      , static_cast<int>(textEffect_t::PA_BLINDS)
      , static_cast<int>(textEffect_t::PA_RANDOM)
    # endif // ENA_MISC
    # if ENA_WIPE
      , static_cast<int>(textEffect_t::PA_WIPE)
      , static_cast<int>(textEffect_t::PA_WIPE_CURSOR)
    # endif // ENA_WIPE
    # if ENA_SCAN
      , static_cast<int>(textEffect_t::PA_SCAN_HORIZ)
      , static_cast<int>(textEffect_t::PA_SCAN_HORIZX)
      , static_cast<int>(textEffect_t::PA_SCAN_VERT)
      , static_cast<int>(textEffect_t::PA_SCAN_VERTX)
    # endif // ENA_SCAN
    # if ENA_OPNCLS
      , static_cast<int>(textEffect_t::PA_OPENING)
      , static_cast<int>(textEffect_t::PA_OPENING_CURSOR)
      , static_cast<int>(textEffect_t::PA_CLOSING)
      , static_cast<int>(textEffect_t::PA_CLOSING_CURSOR)
    # endif // ENA_OPNCLS
    # if ENA_SCR_DIA
      , static_cast<int>(textEffect_t::PA_SCROLL_UP_LEFT)
      , static_cast<int>(textEffect_t::PA_SCROLL_UP_RIGHT)
      , static_cast<int>(textEffect_t::PA_SCROLL_DOWN_LEFT)
      , static_cast<int>(textEffect_t::PA_SCROLL_DOWN_RIGHT)
    # endif // ENA_SCR_DIA
    # if ENA_GROW
      , static_cast<int>(textEffect_t::PA_GROW_UP)
      , static_cast<int>(textEffect_t::PA_GROW_DOWN)
    # endif // ENA_GROW
    };

    // Append the numeric value as a reference for the 'anim.in' and 'anim.out' subcommands
    for (uint8_t a = 0; a < animationCount; a++) {
      animationTypes[a] += F(" (");
      animationTypes[a] += animationOptions[a];
      animationTypes[a] += ')';
    }
    delay(0);

    int fontCount = 1;
    # ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
    fontCount++;
    # endif // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
    # ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
    fontCount++;
    # endif // ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
    # ifdef P104_USE_VERTICAL_FONT
    fontCount++;
    # endif // ifdef P104_USE_VERTICAL_FONT
    # ifdef P104_USE_EXT_ASCII_FONT
    fontCount++;
    # endif // ifdef P104_USE_EXT_ASCII_FONT
    # ifdef P104_USE_ARABIC_FONT
    fontCount++;
    # endif // ifdef P104_USE_ARABIC_FONT
    # ifdef P104_USE_GREEK_FONT
    fontCount++;
    # endif // ifdef P104_USE_GREEK_FONT
    # ifdef P104_USE_KATAKANA_FONT
    fontCount++;
    # endif // ifdef P104_USE_KATAKANA_FONT
    const __FlashStringHelper *fontTypes[] = {
      F("Default (0)")
    # ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
      , F("Numeric, double height (1)")
    # endif   // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
    # ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
      , F("Full, double height (2)")
    # endif   // ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
    # ifdef P104_USE_VERTICAL_FONT
      , F("Vertical (3)")
    # endif   // ifdef P104_USE_VERTICAL_FONT
    # ifdef P104_USE_EXT_ASCII_FONT
      , F("Extended ASCII (4)")
      # endif // ifdef P104_USE_EXT_ASCII_FONT
    # ifdef P104_USE_ARABIC_FONT
      , F("Arabic (5)")
    # endif   // ifdef P104_USE_ARABIC_FONT
    # ifdef P104_USE_GREEK_FONT
      , F("Greek (6)")
    # endif   // ifdef P104_USE_GREEK_FONT
    # ifdef P104_USE_KATAKANA_FONT
      , F("Katakana (7)")
    # endif   // ifdef P104_USE_KATAKANA_FONT
    };
    int fontOptions[] = {
      P104_DEFAULT_FONT_ID
    # ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
      , P104_DOUBLE_HEIGHT_FONT_ID
    # endif   // ifdef P104_USE_NUMERIC_DOUBLEHEIGHT_FONT
    # ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
      , P104_FULL_DOUBLEHEIGHT_FONT_ID
    # endif   // ifdef P104_USE_FULL_DOUBLEHEIGHT_FONT
    # ifdef P104_USE_VERTICAL_FONT
      , P104_VERTICAL_FONT_ID
    # endif   // ifdef P104_USE_VERTICAL_FONT
    # ifdef P104_USE_EXT_ASCII_FONT
      , P104_EXT_ASCII_FONT_ID
      # endif // ifdef P104_USE_EXT_ASCII_FONT
    # ifdef P104_USE_ARABIC_FONT
      , P104_ARABIC_FONT_ID
    # endif   // ifdef P104_USE_ARABIC_FONT
    # ifdef P104_USE_GREEK_FONT
      , P104_GREEK_FONT_ID
    # endif   // ifdef P104_USE_GREEK_FONT
    # ifdef P104_USE_KATAKANA_FONT
      , P104_KATAKANA_FONT_ID
    # endif   // ifdef P104_USE_KATAKANA_FONT
    };

    int layoutCount = 1;
    # if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
    layoutCount += 2;
    # endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
    const __FlashStringHelper *layoutTypes[] = {
      F("Standard")
    # if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
      , F("Double, upper")
      , F("Double, lower")
    # endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
    };
    int layoutOptions[] = {
      P104_LAYOUT_STANDARD
    # if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
      , P104_LAYOUT_DOUBLE_UPPER
      , P104_LAYOUT_DOUBLE_LOWER
    # endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) || defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
    };

    int specialEffectCount                          = 4;
    const __FlashStringHelper *specialEffectTypes[] = {
      F("None"),
      F("Flip up/down"),
      F("Flip left/right *"),
      F("Flip u/d &amp; l/r *")
    };
    int specialEffectOptions[] = {
      P104_SPECIAL_EFFECT_NONE,
      P104_SPECIAL_EFFECT_UP_DOWN,
      P104_SPECIAL_EFFECT_LEFT_RIGHT,
      P104_SPECIAL_EFFECT_BOTH
    };

    const __FlashStringHelper *contentTypes[] = {
      F("Text"),
      F("Text reverse"),
      F("Clock (4 mod.)"),
      F("Clock sec (6 mod.)"),
      F("Date (4 mod.)"),
      F("Date yr (6/7 mod.)"),
      F("Date/time (9/13 mod.)")
      # ifdef P104_USE_BAR_GRAPH
      , F("Bar graph")
      # endif // ifdef P104_USE_BAR_GRAPH
    };
    int contentOptions[] {
      P104_CONTENT_TEXT,
      P104_CONTENT_TEXT_REV,
      P104_CONTENT_TIME,
      P104_CONTENT_TIME_SEC,
      P104_CONTENT_DATE4,
      P104_CONTENT_DATE6,
      P104_CONTENT_DATE_TIME
      # ifdef P104_USE_BAR_GRAPH
      , P104_CONTENT_BAR_GRAPH
      # endif // ifdef P104_USE_BAR_GRAPH
    };
    # ifdef P104_USE_ZONE_ACTIONS
    uint8_t actionCount = 0;
    const __FlashStringHelper *actionTypes[4];
    int actionOptions[4];
    actionTypes[actionCount]   = F("None");
    actionOptions[actionCount] = P104_ACTION_NONE;
    actionCount++;

    if (zones.size() < P104_MAX_ZONES) {
      actionTypes[actionCount]   = F("New above");
      actionOptions[actionCount] = P104_ACTION_ADD_ABOVE;
      actionCount++;
      actionTypes[actionCount]   = F("New below");
      actionOptions[actionCount] = P104_ACTION_ADD_BELOW;
      actionCount++;
    }
    actionTypes[actionCount]   = F("Delete");
    actionOptions[actionCount] = P104_ACTION_DELETE;
    actionCount++;
    # endif // ifdef P104_USE_ZONE_ACTIONS

    delay(0);

    addFormSubHeader(F("Zone configuration"));

    {
      html_table(EMPTY_STRING); // Sub-table
      html_table_header(F("Zone #&nbsp;"));
      html_table_header(F("Size"));
      html_table_header(F("Text"), 180);
      html_table_header(F("Content"));
      html_table_header(F("Alignment"));
      html_table_header(F("Animation In/Out")); // 1st and 2nd row title
      html_table_header(F("Speed/Pause"));      // 1st and 2nd row title
      html_table_header(F("Font/Layout"));      // 1st and 2nd row title
      html_table_header(F("Special Effects"));
      html_table_header(F("Offset"));
      html_table_header(F("Brightness"));
      html_table_header(F("Repeat (sec)"));
      # ifdef P104_USE_ZONE_ACTIONS
      html_table_header(F(""),       15); // Spacer
      html_table_header(F("Action"), 45);
      # endif // ifdef P104_USE_ZONE_ACTIONS
    }

    uint16_t index;
    int16_t  startZone, endZone;
    int8_t   incrZone = 1;
    # ifdef P104_USE_ZONE_ACTIONS
    uint8_t currentRow = 0;
    # endif // ifdef P104_USE_ZONE_ACTIONS

    # ifdef P104_USE_ZONE_ORDERING

    if (bitRead(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_ZONE_ORDER)) {
      startZone = zones.size() - 1;
      endZone   = -1;
      incrZone  = -1;
    } else
    # endif // ifdef P104_USE_ZONE_ORDERING
    {
      startZone = 0;
      endZone   = zones.size();
    }

    for (int8_t zone = startZone; zone != endZone; zone += incrZone) {
      if (zones[zone].zone <= expectedZones) {
        index = (zones[zone].zone - 1) * P104_OFFSET_COUNT;

        html_TR_TD(); // All columns use max. width available
        addHtml(F("&nbsp;"));
        addHtmlInt(zones[zone].zone);

        html_TD(); // Size
        addNumericBox(getPluginCustomArgName(index + P104_OFFSET_SIZE), zones[zone].size, 1, P104_MAX_MODULES_PER_ZONE);

        html_TD(); // text
        addTextBox(getPluginCustomArgName(index + P104_OFFSET_TEXT),
                   zones[zone].text,
                   P104_MAX_TEXT_LENGTH_PER_ZONE,
                   false,
                   false,
                   EMPTY_STRING,
                   EMPTY_STRING);

        html_TD(); // Content
        addSelector(getPluginCustomArgName(index + P104_OFFSET_CONTENT),
                    P104_CONTENT_count,
                    contentTypes,
                    contentOptions,
                    NULL,
                    zones[zone].content,
                    false,
                    true,
                    EMPTY_STRING);

        html_TD(); // Alignment
        addSelector(getPluginCustomArgName(index + P104_OFFSET_ALIGNMENT),
                    3,
                    alignmentTypes,
                    alignmentOptions,
                    NULL,
                    zones[zone].alignment,
                    false,
                    true,
                    EMPTY_STRING);

        html_TD(); // Animation In (without None by passing the second element index)
        addSelector(getPluginCustomArgName(index + P104_OFFSET_ANIM_IN),
                    animationCount - 1,
                    &animationTypes[1],
                    &animationOptions[1],
                    NULL,
                    zones[zone].animationIn,
                    false,
                    true,
                    F("")
                    # ifdef P104_USE_TOOLTIPS
                    , F("Animation In")
                    # endif // ifdef P104_USE_TOOLTIPS
                    );

        html_TD(); // Speed In
        addNumericBox(getPluginCustomArgName(index + P104_OFFSET_SPEED), zones[zone].speed, 0, P104_MAX_SPEED_PAUSE_VALUE
                      # ifdef P104_USE_TOOLTIPS
                      , EMPTY_STRING // classname
                      , F("Speed")   // title
                      # endif // ifdef P104_USE_TOOLTIPS
                      );

        html_TD(); // Font
        addSelector(getPluginCustomArgName(index + P104_OFFSET_FONT),
                    fontCount,
                    fontTypes,
                    fontOptions,
                    NULL,
                    zones[zone].font,
                    false,
                    true,
                    EMPTY_STRING
                    # ifdef P104_USE_TOOLTIPS
                    , F("Font") // title
                    # endif // ifdef P104_USE_TOOLTIPS
                    );

        html_TD(4); // Fill columns
        # ifdef P104_USE_ZONE_ACTIONS

        html_TD();  // Spacer
        addHtml(F("|"));

        if (currentRow < 2) {
          addHtml(F("<TD style=\"text-align:center;font-size:90%\">")); // Action column, text centered and font-size 90%
        } else {
          html_TD();
        }

        if (currentRow == 0) {
          addHtml(F("(applied immediately!)"));
          currentRow++;
        } else if (currentRow == 1) {
          addHtml(F("(Delete can't be undone!)"));
          currentRow++;
        }
        # endif // ifdef P104_USE_ZONE_ACTIONS

        // Split here
        html_TR_TD(); // Start new row
        html_TD(4); // Start with some blank columns

        html_TD();  // Animation Out
        addSelector(getPluginCustomArgName(index + P104_OFFSET_ANIM_OUT),
                    animationCount,
                    animationTypes,
                    animationOptions,
                    NULL,
                    zones[zone].animationOut,
                    false,
                    true,
                    EMPTY_STRING
                    # ifdef P104_USE_TOOLTIPS
                    , F("Animation Out")
                    # endif // ifdef P104_USE_TOOLTIPS
                    );

        html_TD(); // Pause after Animation In
        addNumericBox(getPluginCustomArgName(index + P104_OFFSET_PAUSE), zones[zone].pause, 0, P104_MAX_SPEED_PAUSE_VALUE
                      # ifdef P104_USE_TOOLTIPS
                      , EMPTY_STRING // classname
                      , F("Pause")   // title
                      # endif // ifdef P104_USE_TOOLTIPS
                      );

        html_TD(); // Layout
        addSelector(getPluginCustomArgName(index + P104_OFFSET_LAYOUT),
                    layoutCount,
                    layoutTypes,
                    layoutOptions,
                    NULL,
                    zones[zone].layout,
                    false,
                    true,
                    EMPTY_STRING
                    # ifdef P104_USE_TOOLTIPS
                    , F("Layout") // title
                    # endif // ifdef P104_USE_TOOLTIPS
                    );

        html_TD(); // Special effects
        addSelector(getPluginCustomArgName(index + P104_OFFSET_SPEC_EFFECT),
                    specialEffectCount,
                    specialEffectTypes,
                    specialEffectOptions,
                    NULL,
                    zones[zone].specialEffect,
                    false,
                    true,
                    EMPTY_STRING);

        html_TD(); // Offset
        addNumericBox(getPluginCustomArgName(index + P104_OFFSET_OFFSET), zones[zone].offset, 0, 254);

        html_TD(); // Brightness

        if (zones[zone].brightness == -1) { zones[zone].brightness = 7; }
        addNumericBox(getPluginCustomArgName(index + P104_OFFSET_BRIGHTNESS), zones[zone].brightness, 0, P104_BRIGHTNESS_MAX);

        html_TD(); // Repeat (sec)
        addNumericBox(getPluginCustomArgName(index + P104_OFFSET_REPEATDELAY),
                      zones[zone].repeatDelay,
                      -1,
                      P104_MAX_REPEATDELAY_VALUE                     // max delay 86400 sec. = 24 hours
                      # ifdef P104_USE_TOOLTIPS
                      , EMPTY_STRING                                 // classname
                      , F("Repeat after this delay (sec), -1 = off") // tooltip
                      # endif // ifdef P104_USE_TOOLTIPS
                      );

        # ifdef P104_USE_ZONE_ACTIONS
        html_TD(); // Spacer
        addHtml(F("|"));

        html_TD(); // Action
        addSelector(getPluginCustomArgName(index + P104_OFFSET_ACTION),
                    actionCount,
                    actionTypes,
                    actionOptions,
                    NULL,
                    0, // Always start with 0
                    true,
                    true,
                    EMPTY_STRING);
        # endif // ifdef P104_USE_ZONE_ACTIONS

        delay(0);
      }
    }
    html_end_table();
  }

  # ifdef P104_ADD_SETTINGS_NOTES
  String devicesMsg;

  if (devicesMsg.reserve(80)) {
    devicesMsg  = F("- Maximum nr. of modules possible (Zones * Size + Offset) = 255. Last saved: ");
    devicesMsg += numDevices;
    addFormNote(devicesMsg);
  }
  addFormNote(F("- 'Animation In' or 'Animation Out' and 'Special Effects' marked with <b>*</b> should <b>not</b> be combined in a Zone."));
  #  if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) && !defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
  addFormNote(F("- 'Layout' 'Double upper' and 'Double lower' are only supported for 'Content' types 'Clock' and 'Date'."));
  #  endif // if defined(P104_USE_NUMERIC_DOUBLEHEIGHT_FONT) && !defined(P104_USE_FULL_DOUBLEHEIGHT_FONT)
  # endif  // ifdef P104_ADD_SETTINGS_NOTES

  return true;
}

/**************************************************************
* webform_save
**************************************************************/
bool P104_data_struct::webform_save(struct EventStruct *event) {
  P104_CONFIG_ZONE_COUNT   = getFormItemInt(F("plugin_104_zone"));
  P104_CONFIG_HARDWARETYPE = getFormItemInt(F("plugin_104_hardware"));

  bitWrite(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_CLEAR_DISABLE, isFormItemChecked(F("plugin_104_cleardisable")));
  bitWrite(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_LOG_ALL_TEXT,  isFormItemChecked(F("plugin_104_logalltext")));

  # ifdef P104_USE_ZONE_ORDERING
  zoneOrder = getFormItemInt(F("plugin_104_zoneorder")); // Is used in saveSettings()
  bitWrite(P104_CONFIG_FLAGS, P104_CONFIG_FLAG_ZONE_ORDER, zoneOrder == 1);
  # endif // ifdef P104_USE_ZONE_ORDERING

  # ifdef P104_USE_DATETIME_OPTIONS
  uint32_t ulDateTime = 0;
  bitWrite(ulDateTime, P104_CONFIG_DATETIME_FLASH,    !isFormItemChecked(F("plugin_104_clockflash"))); // Inverted flag
  bitWrite(ulDateTime, P104_CONFIG_DATETIME_12H,      isFormItemChecked(F("plugin_104_clock12h")));
  bitWrite(ulDateTime, P104_CONFIG_DATETIME_AMPM,     isFormItemChecked(F("plugin_104_clockampm")));
  bitWrite(ulDateTime, P104_CONFIG_DATETIME_YEAR4DGT, isFormItemChecked(F("plugin_104_year4dgt")));
  set4BitToUL(ulDateTime, P104_CONFIG_DATETIME_FORMAT,   getFormItemInt(F("plugin_104_dateformat")));
  set4BitToUL(ulDateTime, P104_CONFIG_DATETIME_SEP_CHAR, getFormItemInt(F("plugin_104_dateseparator")));
  P104_CONFIG_DATETIME = ulDateTime;
  # endif // ifdef P104_USE_DATETIME_OPTIONS

  previousZones = expectedZones;
  expectedZones = P104_CONFIG_ZONE_COUNT;

  bool result = saveSettings();         // Determines numDevices and re-fills zones list

  P104_CONFIG_ZONE_COUNT  = zones.size();
  P104_CONFIG_TOTAL_UNITS = numDevices; // Store counted number of devices

  zones.clear();                        // Free some memory (temporarily)

  return result;
}

#endif // ifdef USES_P104
