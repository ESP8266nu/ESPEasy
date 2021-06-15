#include "_Plugin_Helper.h"
#ifdef USES_P065

// #######################################################################################################
// ############################# Plugin 065: P065_DFR0299_MP3 ############################################
// #######################################################################################################

// ESPEasy Plugin to controls a MP3-player-module DFPlayer-Mini SKU:DFR0299
// written by Jochen Krapf (jk@nerd2nerd.org)

// Change history:
// 2021-05-01, tonhuisman: Add mode and repeat commands, optimize string and log usage to reduce size

// Important! The module WTV020-SD look similar to the module DFPlayer-Mini but is NOT pin nor command compatible!

// Commands:
// play,<track>        Plays the n-th track 1...3000 on SD-card in root folder. The track number is the physical order - not the order
// displayed in file explorer!
// stop                Stops actual playing sound
// vol,<volume>        Set volume level 1...30
// eq,<type>           Set the equalizer type 0=Normal, 1=Pop, 2=Rock, 3=Jazz, 4=classic, 5=Base
// mode,<mode>         Set the playback mode 0=Repeat, 1=Folder repeat, 2=Single repeat, 3=Random
// repeat,<0/1>        Set repeat mode 0=Off, 1=On

// Circuit wiring
// 1st-GPIO -> ESP TX to module RX [Pin2]
// 5 V to module VCC [Pin1] (can be more than 100 mA) Note: Use a blocking capacitor to stabilise VCC
// GND to module GND [Pin7+Pin10]
// Speaker to module SPK_1 and SPK_2 [Pin6,Pin8] (not to GND!) Note: If speaker has to low impedance, use a resistor (like 33 Ohm) in line
// to speaker
// (optional) module BUSY [Pin16] to LED driver (3.3 V on idle, 0 V on playing)
// All other pins unconnected

// Note: Notification sounds with Creative Commons Attribution license: https://notificationsounds.com/

// Datasheet: https://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299


# define PLUGIN_065
# define PLUGIN_ID_065         65
# define PLUGIN_NAME_065       "Notify - DFPlayer-Mini MP3 [TESTING]"
# define PLUGIN_VALUENAME1_065 ""

# include <ESPeasySerial.h>


ESPeasySerial *P065_easySerial = NULL;


boolean Plugin_065(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_065;
      Device[deviceCount].Type               = DEVICE_TYPE_SINGLE;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_NONE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = false;
      Device[deviceCount].ValueCount         = 0;
      Device[deviceCount].SendDataOption     = false;
      Device[deviceCount].TimerOption        = false;
      Device[deviceCount].GlobalSyncOption   = false;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_065);
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      event->String1 = formatGpioName_TX(false);
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormNumericBox(F("Volume"), F("volume"), PCONFIG(0), 1, 30);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      PCONFIG(0) = getFormItemInt(F("volume"));

      success = true;
      break;
    }

    case PLUGIN_INIT:
    {
      # pragma GCC diagnostic push

      // note: we cant fix this, its a upstream bug.
      # pragma GCC diagnostic warning "-Wdelete-non-virtual-dtor"

      if (P065_easySerial) {
        delete P065_easySerial;
      }
      # pragma GCC diagnostic pop


      P065_easySerial = new (std::nothrow) ESPeasySerial(static_cast<ESPEasySerialPort>(CONFIG_PORT), -1, CONFIG_PIN1); // no RX, only TX

      if (P065_easySerial != nullptr) {
        P065_easySerial->begin(9600);
        Plugin_065_SetVol(PCONFIG(0)); // set default volume

        success = true;
      }
      break;
    }

    case PLUGIN_WRITE:
    {
      if (!P065_easySerial) {
        break;
      }

      String command = parseString(string, 1);
      String param   = parseString(string, 2);
      int    value;
      bool   valueValid = validIntFromString(param, value);

      if (valueValid && (command == F("play")))
      {
        Plugin_065_Play(value);
        success = true;
      }

      if (command == F("stop"))
      {
        Plugin_065_SendCmd(0x0E, 0);
        success = true;
      }

      if (valueValid && (command == F("vol")))
      {
        if (value == 0) { value = 30; }
        PCONFIG(0) = value;
        Plugin_065_SetVol(value);
        success = true;
      }

      if (valueValid && (command == F("eq")))
      {
        Plugin_065_SetEQ(value);
        success = true;
      }

      if (valueValid && (command == F("mode")))
      {
        Plugin_065_SetMode(value);
        success = true;
      }

      if (valueValid && (command == F("repeat")))
      {
        Plugin_065_SetRepeat(value);
        success = true;
      }

      if (success && loglevelActiveFor(LOG_LEVEL_INFO)) {
        String log;
        log.reserve(48);
        log  = F("MP3  : ");
        log += command;

        if (command != F("stop")) {
          log += '=';
          log += value;
        }
        addLog(LOG_LEVEL_INFO, log);
      }
      break;
    }
  }
  return success;
}

void Plugin_065_Play(uint16_t track)
{
  if (track < 0) { track = 0; }

  if (track > 2999) { track = 2999; }

  Plugin_065_SendCmd(0x03, track);
}

void Plugin_065_SetVol(int8_t vol)
{
  if (vol < 1) { vol = 1; }

  if (vol > 30) { vol = 30; }

  Plugin_065_SendCmd(0x06, vol);
}

void Plugin_065_SetEQ(int8_t eq)
{
  if (eq < 0) { eq = 0; }

  if (eq > 5) { eq = 5; }

  Plugin_065_SendCmd(0x07, eq);
}

void Plugin_065_SetMode(int8_t mode)
{
  if (mode < 0) { mode = 0; }

  if (mode > 3) { mode = 3; }

  Plugin_065_SendCmd(0x08, mode);
}

void Plugin_065_SetRepeat(int8_t repeat)
{
  Plugin_065_SendCmd(0x11, (repeat <= 0) ? 0 : 1);
}

void Plugin_065_SendCmd(byte cmd, int16_t data)
{
  if (!P065_easySerial) {
    return;
  }

  byte buffer[10] = { 0x7E, 0xFF, 0x06, 0, 0x00, 0, 0, 0, 0, 0xEF };

  buffer[3] = cmd;
  buffer[5] = data >> 8;   // high byte
  buffer[6] = data & 0xFF; // low byte

  int16_t checksum = -(buffer[1] + buffer[2] + buffer[3] + buffer[4] + buffer[5] + buffer[6]);

  buffer[7] = checksum >> 8;          // high byte
  buffer[8] = checksum & 0xFF;        // low byte

  P065_easySerial->write(buffer, 10); // Send the byte array

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("MP3  : Send Cmd ");

    for (byte i = 0; i < 10; i++) {
      log += String(buffer[i], 16);
      log += ' ';
    }
    addLog(LOG_LEVEL_DEBUG, log);
  }
}

#endif // USES_P065
