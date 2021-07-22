#include "../PluginStructs/P016_data_struct.h"

#ifdef USES_P016

# include "../Commands/InternalCommands.h"
# include "../Helpers/ESPEasy_Storage.h"
# include <IRutils.h>

# ifdef P16_SETTINGS_V1
tCommandLinesV2::tCommandLinesV2() {} // Default constructor

// Conversion constructor
tCommandLinesV2::tCommandLinesV2(const String& command,
                                 uint32_t      oldCode,
                                 uint32_t      oldAlternativeCode,
                                 uint8_t       i) {
  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    #  ifndef PLUGIN_016_DEBUG
    log.reserve(20); // less space needed
    #  else // ifndef PLUGIN_016_DEBUG
    log.reserve(80);
    #  endif // ifndef PLUGIN_016_DEBUG
  }

  log  = F("P016: converting "); // Still a little logging
  log += i;
  #  ifdef PLUGIN_016_DEBUG
  log += ':';
  #  endif // ifdef PLUGIN_016_DEBUG

  if (command.length() > 0) {
    safe_strncpy(Command, command, P16_Nchars);
  }

  if (oldCode > 0) {
    CodeDecodeType = static_cast<decode_type_t>((oldCode >> 24));                // decode_type
    bitWrite(CodeFlags, P16_FLAGS_REPEAT, oldCode & (0x1 << P16_CMDBIT_REPEAT)); // Repeat flag
    Code = oldCode & 0x7FFFFF;                                                   // Only keep lowest 23 bits
    #  ifdef PLUGIN_016_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += F(" type ");
      log += typeToString(CodeDecodeType, (oldCode & (0x1 << P16_CMDBIT_REPEAT)) != 0);
      log += F(" code 0x");
      log += uint64ToString(oldCode, 16);
    }
    #  endif // ifdef PLUGIN_016_DEBUG
  }

  if (oldAlternativeCode > 0) {
    AlternativeCodeDecodeType = static_cast<decode_type_t>((oldAlternativeCode >> 24));                // decode_type
    bitWrite(AlternativeCodeFlags, P16_FLAGS_REPEAT, oldAlternativeCode & (0x1 << P16_CMDBIT_REPEAT)); // Repeat flag
    AlternativeCode = oldAlternativeCode & 0x7FFFFF;                                                   // Only keep lowest 23 bits
    #  ifdef PLUGIN_016_DEBUG

    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += F(" alt.type ");
      log += typeToString(AlternativeCodeDecodeType, (oldAlternativeCode & (0x1 << P16_CMDBIT_REPEAT)) != 0);
      log += F(" alt.code 0x");
      log += uint64ToString(oldAlternativeCode, 16);
    }
    #  endif // ifdef PLUGIN_016_DEBUG
  }
  addLog(LOG_LEVEL_INFO, log);
}

# endif // ifdef P16_SETTINGS_V1

P016_data_struct::P016_data_struct() {}

void P016_data_struct::init(struct EventStruct *event, uint16_t CmdInhibitTime) {
  loadCommandLines(event);
  iCmdInhibitTime = CmdInhibitTime;
  iLastCmd        = 0;
  iLastCmdTime    = 0;
}

# ifdef P16_SETTINGS_V1
void P016_data_struct::convertCommandLines(struct EventStruct *event) {
  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    #  ifndef PLUGIN_016_DEBUG
    log.reserve(20); // less space needed
    #  else // ifndef PLUGIN_016_DEBUG
    log.reserve(80);

    log  = F("P016: struct size: ");
    log += sizeof(tCommandLinesV2);
    log += '/';
    log += sizeof(tCommandLines);
    log += F(" enum: ");
    log += sizeof(decode_type_t);
    log += F(" p016: ");
    log += sizeof(P016_data_struct);
    addLog(LOG_LEVEL_INFO, log);
    #  endif // ifdef PLUGIN_016_DEBUG
  }

  // read V1 data && convert
  CommandLinesV1.clear(); // Start fresh
  int loadOffset = 0;

  for (uint8_t i = 0; i < P16_Nlines; i++) {
    CommandLinesV1.push_back(tCommandLines());
    LoadFromFile(SettingsType::Enum::CustomTaskSettings_Type,
                 event->TaskIndex,
                 (uint8_t *)&(CommandLinesV1[i]),
                 sizeof(tCommandLines),
                 loadOffset);
    loadOffset += sizeof(tCommandLines);
  }

  CommandLines.clear(); // Start fresh

  for (int i = 0; i < P16_Nlines; ++i) {
    CommandLines.push_back(tCommandLinesV2(String(CommandLinesV1[i].Command), CommandLinesV1[i].Code, CommandLinesV1[i].AlternativeCode, i));

    delay(0);
  }
  CommandLinesV1.clear(); // clean up after conversion
}

# endif // ifdef P16_SETTINGS_V1

void P016_data_struct::loadCommandLines(struct EventStruct *event) {
  # ifdef P16_SETTINGS_V1

  // Convert the settings if both versions are defined and PCONFIG(7) != latest version
  if (PCONFIG(7) != P16_SETTINGS_LATEST) {
    addLog(LOG_LEVEL_ERROR, F("P016 IR: Settings conversion, save task settings to store in new format."));

    convertCommandLines(event);
  }
  else
  # endif // ifdef P16_SETTINGS_V1
  {
    // read V2 settings data

    CommandLines.clear(); // Start fresh
    int loadOffset = 0;

    for (uint8_t i = 0; i < P16_Nlines; i++) {
      CommandLines.push_back(tCommandLinesV2());
      LoadFromFile(SettingsType::Enum::CustomTaskSettings_Type,
                   event->TaskIndex,
                   (uint8_t *)&(CommandLines[i]),
                   sizeof(tCommandLinesV2),
                   loadOffset);
      loadOffset += sizeof(tCommandLinesV2);
    }
  }

  for (int i = 0; i < P16_Nlines; ++i) {
    CommandLines[i].Command[P16_Nchars - 1] = 0; // Terminate in case of uninitalized data
  }
}

void P016_data_struct::saveCommandLines(struct EventStruct *event) {
  int saveOffset = 0;

  for (uint8_t i = 0; i < P16_Nlines; i++) {
    SaveToFile(SettingsType::Enum::CustomTaskSettings_Type,
               event->TaskIndex,
               (uint8_t *)&(CommandLines[i]),
               sizeof(tCommandLinesV2),
               saveOffset);
    saveOffset += sizeof(tCommandLinesV2);
  }
}

void P016_data_struct::AddCode(uint64_t Code, decode_type_t DecodeType, uint16_t CodeFlags) {
  // add received code
  int _index = P16_Nlines;

  if (Code == 0) {
    return;
  }

  for (int i = 0; i < P16_Nlines; ++i) {
    if (validateCode(i, Code, DecodeType, CodeFlags)) {
      // code already saved
      return;
    }

    if (CommandLines[i].Code == 0) {
      // get first index to add the code
      _index = std::min(i, _index);
    }
  }

  if (_index == P16_Nlines) {
    // no free index
    return;
  }
  CommandLines[_index].Code           = Code;
  CommandLines[_index].CodeDecodeType = DecodeType;
  CommandLines[_index].CodeFlags      = CodeFlags;
  bCodeChanged                        = true;
  # ifdef PLUGIN_016_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log;
    log.reserve(80); // estimated
    log  = F("[P36] AddCode: ");
    log += typeToString(DecodeType, bitRead(CodeFlags, P16_FLAGS_REPEAT));
    log += F(" code: 0x");
    log += uint64ToString(Code, 16);
    log += F(" to index ");
    log += _index;
    addLog(LOG_LEVEL_INFO, log);
  }
  # endif // PLUGIN_016_DEBUG
}

void P016_data_struct::ExecuteCode(uint64_t Code, decode_type_t DecodeType, uint16_t CodeFlags) {
  if (Code == 0) {
    return;
  }

  if ((iLastCmd == Code) && (iLastDecodeType == DecodeType)) {
    // same code as before
    if (iCmdInhibitTime > timePassedSince(iLastCmdTime)) {
      // inhibit time not ellapsed
      return;
    }
  }

  for (int i = 0; i < P16_Nlines; ++i) {
    if (validateCode(i, Code, DecodeType, CodeFlags)) {
      // code already saved
      iLastCmd        = Code;
      iLastDecodeType = DecodeType;
      iLastCodeFlags  = CodeFlags;
      iLastCmdTime    = millis();

      if (CommandLines[i].Command[0] != 0) {
        # ifdef PLUGIN_016_DEBUG
        bool _success =
        # endif // ifdef PLUGIN_016_DEBUG
        ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_SYSTEM, CommandLines[i].Command);
        # ifdef PLUGIN_016_DEBUG

        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log;
          log.reserve(128); // estimated
          log  = F("[P36] Execute: ");
          log += typeToString(DecodeType, bitRead(CodeFlags, P16_FLAGS_REPEAT));
          log += F(" Code: 0x");
          log += uint64ToString(Code, 16);
          log += F(" with command ");
          log += (i + 1);
          log += F(": {");
          log += String(CommandLines[i].Command);
          log += '}';

          if (!_success) {
            log += F(" FAILED!");
          }
          addLog(LOG_LEVEL_INFO, log);
        }
        # endif // PLUGIN_016_DEBUG
      }
      return;
    }
  }
}

bool P016_data_struct::validateCode(int i, uint64_t Code, decode_type_t DecodeType, uint16_t CodeFlags) {
  return ((CommandLines[i].Code == Code)
          && (CommandLines[i].CodeDecodeType == DecodeType)
          && (CommandLines[i].CodeFlags == CodeFlags))
         || ((CommandLines[i].AlternativeCode == Code)
             && (CommandLines[i].AlternativeCodeDecodeType == DecodeType)
             && (CommandLines[i].AlternativeCodeFlags == CodeFlags));
}

#endif // ifdef USES_P016
