#include "_Plugin_Helper.h"
#ifdef USES_P049

/*

   This plug in is written by Dmitry (rel22 ___ inbox.ru)
   Plugin is based upon SenseAir plugin by Daniel Tedenljung info__AT__tedenljungconsulting.com
   Additional features based on https://geektimes.ru/post/285572/ by Gerben (infernix__AT__gmail.com)

   This plugin reads the CO2 value from MH-Z19 NDIR Sensor

   Pin-out:
   Hd o
   SR o   o PWM
   Tx o   o AOT
   Rx o   o GND
   Vo o   o Vin
   (bottom view)
   Skipping pin numbers due to inconsistancies in individual data sheet revisions.
   MHZ19:  Connection:
   VCC     5 V
   GND     GND
   Tx      ESP8266 1st GPIO specified in Device-settings
   Rx      ESP8266 2nd GPIO specified in Device-settings
 */

// Uncomment the following define to enable the detection range commands:
// #define ENABLE_DETECTION_RANGE_COMMANDS

#define PLUGIN_049
#define PLUGIN_ID_049         49
#define PLUGIN_NAME_049       "Gases - CO2 MH-Z19"
#define PLUGIN_VALUENAME1_049 "PPM"
#define PLUGIN_VALUENAME2_049 "Temperature" // Temperature in C
#define PLUGIN_VALUENAME3_049 "U"           // Undocumented, minimum measurement per time period?
#define PLUGIN_READ_TIMEOUT   300

#define PLUGIN_049_FILTER_OFF        1
#define PLUGIN_049_FILTER_OFF_ALLSAMPLES 2
#define PLUGIN_049_FILTER_FAST       3
#define PLUGIN_049_FILTER_MEDIUM     4
#define PLUGIN_049_FILTER_SLOW       5

#include <ESPeasySerial.h>


enum MHZ19Types {
  MHZ19_notDetected,
  MHZ19_A,
  MHZ19_B
};


enum mhzCommands : uint8_t { mhzCmdReadPPM,
                          mhzCmdCalibrateZero,
                          mhzCmdABCEnable,
                          mhzCmdABCDisable,
                          mhzCmdReset,
#ifdef ENABLE_DETECTION_RANGE_COMMANDS
                          mhzCmdMeasurementRange1000,
                          mhzCmdMeasurementRange2000,
                          mhzCmdMeasurementRange3000,
                          mhzCmdMeasurementRange5000
#endif // ifdef ENABLE_DETECTION_RANGE_COMMANDS
};

// 9 uint8_t commands:
// mhzCmdReadPPM[]              = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
// mhzCmdCalibrateZero[]        = {0xFF,0x01,0x87,0x00,0x00,0x00,0x00,0x00,0x78};
// mhzCmdABCEnable[]            = {0xFF,0x01,0x79,0xA0,0x00,0x00,0x00,0x00,0xE6};
// mhzCmdABCDisable[]           = {0xFF,0x01,0x79,0x00,0x00,0x00,0x00,0x00,0x86};
// mhzCmdReset[]                = {0xFF,0x01,0x8d,0x00,0x00,0x00,0x00,0x00,0x72};

/* It seems the offsets [3]..[4] for the detection range setting (command uint8_t 0x99) are wrong in the latest
 * online data sheet: http://www.winsen-sensor.com/d/files/infrared-gas-sensor/mh-z19b-co2-ver1_0.pdf
 * According to the MH-Z19B datasheet version 1.2, valid from: 2017.03.22 (received 2018-03-07)
 * the offset should be [6]..[7] instead.
 * 0x99 - Detection range setting, send command:
 * /---------+---------+---------+---------+---------+---------+---------+---------+---------\
 * | Byte 0  | Byte 1  | Byte 2  | Byte 3  | Byte 4  | Byte 5  | Byte 6  | Byte 7  | Byte 8  |
 * |---------+---------+---------+---------+---------+---------+---------+---------+---------|
 * | Start   | Reserved| Command | Reserved|Detection|Detection|Detection|Detection| Checksum|
 * | Byte    |         |         |         |range    |range    |range    |range    |         |
 * |         |         |         |         |24~32 bit|16~23 bit|8~15 bit |0~7 bit  |         |
 * |---------+---------+---------+---------+---------+---------+---------+---------+---------|
 * | 0xFF    | 0x01    | 0x99    | 0x00    | Data 1  | Data 2  | Data 3  | Data 4  | Checksum|
 * \---------+---------+---------+---------+---------+---------+---------+---------+---------/
 * Note: Detection range should be 0~2000, 0~5000, 0~10000 ppm.
 * For example: set 0~2000 ppm  detection range, send command: FF 01 99 00 00 00 07 D0 8F
 *              set 0~10000 ppm detection range, send command: FF 01 99 00 00 00 27 10 8F
 * The latter, updated version above is implemented here.
 */

// mhzCmdMeasurementRange1000[] = {0xFF,0x01,0x99,0x00,0x00,0x00,0x03,0xE8,0x7B};
// mhzCmdMeasurementRange2000[] = {0xFF,0x01,0x99,0x00,0x00,0x00,0x07,0xD0,0x8F};
// mhzCmdMeasurementRange3000[] = {0xFF,0x01,0x99,0x00,0x00,0x00,0x0B,0xB8,0xA3};
// mhzCmdMeasurementRange5000[] = {0xFF,0x01,0x99,0x00,0x00,0x00,0x13,0x88,0xCB};
// Removing redundant data, just keeping offsets [2], [6]..[7]:
const PROGMEM uint8_t mhzCmdData[][3] = {
  { 0x86, 0x00, 0x00 },
  { 0x87, 0x00, 0x00 },
  { 0x79, 0xA0, 0x00 },
  { 0x79, 0x00, 0x00 },
  { 0x8d, 0x00, 0x00 },
#ifdef ENABLE_DETECTION_RANGE_COMMANDS
  { 0x99, 0x03, 0xE8 },
  { 0x99, 0x07, 0xD0 },
  { 0x99, 0x0B, 0xB8 },
  { 0x99, 0x13, 0x88 }
#endif // ifdef ENABLE_DETECTION_RANGE_COMMANDS
};

enum
{
  ABC_enabled  = 0x01,
  ABC_disabled = 0x02
};


struct P049_data_struct : public PluginTaskData_base {
  P049_data_struct() {
    reset();
    sensorResets = 0;
  }

  ~P049_data_struct() {
    reset();
  }

  void reset() {
    if (easySerial != nullptr) {
      delete easySerial;
      easySerial = nullptr;
    }
    linesHandled       = 0;
    checksumFailed     = 0;
    nrUnknownResponses = 0;
    ++sensorResets;

    // Default of the sensor is to run ABC
    ABC_Disable     = false;
    ABC_MustApply   = false;
    modelA_detected = false;
  }

  bool init(ESPEasySerialPort port, const int16_t serial_rx, const int16_t serial_tx, bool setABCdisabled) {
    if ((serial_rx < 0) || (serial_tx < 0)) {
      return false;
    }
    reset();
    easySerial = new (std::nothrow) ESPeasySerial(port, serial_rx, serial_tx);

    if (easySerial == nullptr) {
      return false;
    }
    easySerial->begin(9600);
    ABC_Disable = setABCdisabled;

    if (ABC_Disable) {
      // No guarantee the correct state is active on the sensor after reboot.
      ABC_MustApply = true;
    }
    lastInitTimestamp = millis();
    initTimePassed    = false;
    return isInitialized();
  }

  bool isInitialized() const {
    return easySerial != nullptr;
  }

  void setABCmode(int abcDisableSetting) {
    boolean new_ABC_disable = (abcDisableSetting == ABC_disabled);

    if (ABC_Disable != new_ABC_disable) {
      // Setting changed in the webform.
      ABC_MustApply = true;
      ABC_Disable   = new_ABC_disable;
    }
  }

  uint8_t calculateChecksum() const {
    uint8_t checksum = 0;

    for (uint8_t i = 1; i < 8; i++) {
      checksum += mhzResp[i];
    }
    checksum = 0xFF - checksum;
    return checksum + 1;
  }

  size_t send_mhzCmd(uint8_t CommandId)
  {
    if (!isInitialized()) { return 0; }

    // The receive buffer "mhzResp" is re-used to send a command here:
    mhzResp[0] = 0xFF; // Start uint8_t, fixed
    mhzResp[1] = 0x01; // Sensor number, 0x01 by default
    memcpy_P(&mhzResp[2], mhzCmdData[CommandId], sizeof(mhzCmdData[0]));
    mhzResp[6] = mhzResp[3]; mhzResp[7] = mhzResp[4];
    mhzResp[3] = mhzResp[4] = mhzResp[5] = 0x00;
    mhzResp[8] = calculateChecksum();

    if (!initTimePassed) {
      // Allow for 3 minutes of init time.
      initTimePassed = timePassedSince(lastInitTimestamp) > 180000;
    }

    return easySerial->write(mhzResp, sizeof(mhzResp));
  }

  bool read_ppm(unsigned int& ppm, signed int& temp, unsigned int& s, float& u) {
    if (!isInitialized()) { return false; }

    // send read PPM command
    uint8_t nbBytesSent = send_mhzCmd(mhzCmdReadPPM);

    if (nbBytesSent != 9) {
      return false;
    }

    // get response
    memset(mhzResp, 0, sizeof(mhzResp));

    long timer   = millis() + PLUGIN_READ_TIMEOUT;
    int  counter = 0;

    while (!timeOutReached(timer) && (counter < 9)) {
      if (easySerial->available() > 0) {
        uint8_t value = easySerial->read();

        if (((counter == 0) && (value == 0xFF)) || (counter > 0)) {
          mhzResp[counter++] = value;
        }
      } else {
        delay(10);
      }
    }

    if (counter < 9) {
      // Timeout
      return false;
    }
    ++linesHandled;

    if (!(mhzResp[8] == calculateChecksum())) {
      ++checksumFailed;
      return false;
    }

    if ((mhzResp[0] == 0xFF) && (mhzResp[1] == 0x86)) {
      // calculate CO2 PPM
      ppm = (static_cast<unsigned int>(mhzResp[2]) << 8) + mhzResp[3];

      // set temperature (offset 40)
      unsigned int mhzRespTemp = (unsigned int)mhzResp[4];
      temp = mhzRespTemp - 40;

      // set 's' (stability) value
      s = mhzResp[5];

      if (s != 0) {
        modelA_detected = true;
      }

      // calculate 'u' value
      u = (static_cast<unsigned int>(mhzResp[6]) << 8) + mhzResp[7];
      return true;
    }
    return false;
  }

  bool receivedCommandAcknowledgement(bool& expectReset) {
    expectReset = false;

    if (mhzResp[0] == 0xFF)  {
      switch (mhzResp[1]) {
        case 0x86: // Read CO2 concentration
        case 0x79: // ON/OFF Auto Calibration
          break;
        case 0x87: // Calibrate Zero Point (ZERO)
        case 0x88: // Calibrate Span Point (SPAN)
        case 0x99: // Detection range setting
          expectReset = true;
          break;
        default:
          ++nrUnknownResponses;
          return false;
      }
      uint8_t checksum = calculateChecksum();
      return mhzResp[8] == checksum;
    }
    ++nrUnknownResponses;
    return false;
  }

  String getBufferHexDump() {
    String result;

    result.reserve(27);

    for (int i = 0; i < 9; ++i) {
      result += ' ';
      result += String(mhzResp[i], HEX);
    }
    return result;
  }

  MHZ19Types getDetectedDevice() {
    if (linesHandled > checksumFailed) {
      return modelA_detected ? MHZ19_A : MHZ19_B;
    }
    return MHZ19_notDetected;
  }

  uint32_t      linesHandled       = 0;
  uint32_t      checksumFailed     = 0;
  uint32_t      sensorResets       = 0;
  uint32_t      nrUnknownResponses = 0;
  unsigned long lastInitTimestamp  = 0;

  ESPeasySerial *easySerial = nullptr;
  uint8_t        mhzResp[9] = {0}; // 9 uint8_t response buffer

  // Default of the sensor is to run ABC
  bool ABC_Disable     = false;
  bool ABC_MustApply   = false;
  bool modelA_detected = false;
  bool initTimePassed  = false;
};


boolean Plugin_049_Check_and_ApplyFilter(unsigned int prevVal, unsigned int& newVal, uint32_t s, const int filterValue, String& log) {
  if (s == 1) {
    // S==1 => "A" version sensor bootup, do not use values.
    return false;
  }

  if ((prevVal < 400) || (prevVal > 5000)) {
    // Prevent unrealistic values during start-up with filtering enabled.
    // Just assume the entered value is correct.
    return true;
  }
  boolean filterApplied = filterValue > PLUGIN_049_FILTER_OFF_ALLSAMPLES;
  int32_t difference    = newVal - prevVal;

  if ((s > 0) && (s < 64) && (filterValue != PLUGIN_049_FILTER_OFF)) {
    // Not the "B" version of the sensor, S value is used.
    // S==0 => "B" version, else "A" version
    // The S value is an indication of the stability of the reading.
    // S == 64 represents a stable reading and any lower value indicates (unusual) fast change.
    // Now we increase the delay filter for low values of S and increase response time when the
    // value is more stable.
    // This will make the reading useful in more turbulent environments,
    // where the sensor would report more rapid change of measured values.
    difference    = difference * s;
    difference   /= 64;
    log          += F("Compensate Unstable ");
    filterApplied = true;
  }

  switch (filterValue) {
    case PLUGIN_049_FILTER_OFF: {
      if ((s != 0) && (s != 64)) {
        log += F("Skip Unstable ");
        return false;
      }
      filterApplied = false;
      break;
    }

    // #Samples to reach >= 75% of step response
    case PLUGIN_049_FILTER_OFF_ALLSAMPLES: filterApplied = false; break; // No Delay
    case PLUGIN_049_FILTER_FAST:    difference          /= 2; break;     // Delay: 2 samples
    case PLUGIN_049_FILTER_MEDIUM:  difference          /= 4; break;     // Delay: 5 samples
    case PLUGIN_049_FILTER_SLOW:    difference          /= 8; break;     // Delay: 11 samples
  }

  if (filterApplied) {
    log += F("Raw PPM: ");
    log += newVal;
    log += F(" Filtered ");
  }
  newVal = static_cast<unsigned int>(prevVal + difference);
  return true;
}

boolean Plugin_049(uint8_t function, struct EventStruct *event, String& string)
{
  bool success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      Device[++deviceCount].Number           = PLUGIN_ID_049;
      Device[deviceCount].Type               = DEVICE_TYPE_SERIAL;
      Device[deviceCount].VType              = Sensor_VType::SENSOR_TYPE_TRIPLE;
      Device[deviceCount].Ports              = 0;
      Device[deviceCount].PullUpOption       = false;
      Device[deviceCount].InverseLogicOption = false;
      Device[deviceCount].FormulaOption      = true;
      Device[deviceCount].ValueCount         = 3;
      Device[deviceCount].SendDataOption     = true;
      Device[deviceCount].TimerOption        = true;
      Device[deviceCount].GlobalSyncOption   = true;
      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_049);
      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_049));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_049));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_049));
      break;
    }

    case PLUGIN_GET_DEVICEGPIONAMES:
    {
      serialHelper_getGpioNames(event);
      break;
    }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
    {
      string += serialHelper_getSerialTypeLabel(event);
      success = true;
      break;
    }


    case PLUGIN_WEBFORM_LOAD:
    {
      {
        uint8_t choice         = PCONFIG(0);
        const __FlashStringHelper * options[2]   = { F("Normal"), F("ABC disabled") };
        int optionValues[2] = { ABC_enabled, ABC_disabled };
        addFormSelector(F("Auto Base Calibration"), F("p049_abcdisable"), 2, options, optionValues, choice);
      }
      {
        uint8_t   choiceFilter     = PCONFIG(1);
        const __FlashStringHelper * filteroptions[5] =
        { F("Skip Unstable"), F("Use Unstable"), F("Fast Response"), F("Medium Response"), F("Slow Response") };
        int filteroptionValues[5] = {
          PLUGIN_049_FILTER_OFF,
          PLUGIN_049_FILTER_OFF_ALLSAMPLES,
          PLUGIN_049_FILTER_FAST,
          PLUGIN_049_FILTER_MEDIUM,
          PLUGIN_049_FILTER_SLOW };
        addFormSelector(F("Filter"), F("p049_filter"), 5, filteroptions, filteroptionValues, choiceFilter);
      }
      P049_html_show_stats(event);

      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      const int formValue = getFormItemInt(F("p049_abcdisable"));

      P049_data_struct *P049_data =
        static_cast<P049_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P049_data) {
        P049_data->setABCmode(formValue);
      }
      PCONFIG(0) = formValue;
      PCONFIG(1) = getFormItemInt(F("p049_filter"));
      success    = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P049_data_struct());
      success = P049_performInit(event);
      break;
    }

    case PLUGIN_EXIT: {
      success = true;
      break;
    }

    case PLUGIN_WRITE:
    {
      P049_data_struct *P049_data =
        static_cast<P049_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P049_data) {
        return success;
      }

      String command = parseString(string, 1);

      if (command == F("mhzcalibratezero"))
      {
        P049_data->send_mhzCmd(mhzCmdCalibrateZero);
        addLog(LOG_LEVEL_INFO, F("MHZ19: Calibrated zero point!"));
        success = true;
      }
      else if (command == F("mhzreset"))
      {
        P049_data->send_mhzCmd(mhzCmdReset);
        addLog(LOG_LEVEL_INFO, F("MHZ19: Sent sensor reset!"));
        success = true;
      }
      else if (command == F("mhzabcenable"))
      {
        P049_data->send_mhzCmd(mhzCmdABCEnable);
        addLog(LOG_LEVEL_INFO, F("MHZ19: Sent sensor ABC Enable!"));
        success = true;
      }
      else if (command == F("mhzabcdisable"))
      {
        P049_data->send_mhzCmd(mhzCmdABCDisable);
        addLog(LOG_LEVEL_INFO, F("MHZ19: Sent sensor ABC Disable!"));
        success = true;
      }

#ifdef ENABLE_DETECTION_RANGE_COMMANDS
      else if (command.startsWith(F("mhzmeasurementrange"))) {
        if (command == F("mhzmeasurementrange1000"))
        {
          P049_data->send_mhzCmd(mhzCmdMeasurementRange1000);
          addLog(LOG_LEVEL_INFO, F("MHZ19: Sent measurement range 0-1000PPM!"));
          success = true;
        }
        else if (command == F("mhzmeasurementrange2000"))
        {
          P049_data->send_mhzCmd(mhzCmdMeasurementRange2000);
          addLog(LOG_LEVEL_INFO, F("MHZ19: Sent measurement range 0-2000PPM!"));
          success = true;
        }
        else if (command == F("mhzmeasurementrange3000"))
        {
          P049_data->send_mhzCmd(mhzCmdMeasurementRange3000);
          addLog(LOG_LEVEL_INFO, F("MHZ19: Sent measurement range 0-3000PPM!"));
          success = true;
        }
        else if (command == F("mhzmeasurementrange5000"))
        {
          P049_data->send_mhzCmd(mhzCmdMeasurementRange5000);
          addLog(LOG_LEVEL_INFO, F("MHZ19: Sent measurement range 0-5000PPM!"));
          success = true;
        }
      }
#endif // ENABLE_DETECTION_RANGE_COMMANDS
      break;
    }

    case PLUGIN_READ:
    {
      P049_data_struct *P049_data =
        static_cast<P049_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr == P049_data) {
        return success;
      }
      bool expectReset  = false;
      unsigned int ppm  = 0;
      signed int   temp = 0;
      unsigned int s    = 0;
      float u           = 0;

      if (P049_data->read_ppm(ppm, temp, s, u)) {
        const bool mustLog = loglevelActiveFor(LOG_LEVEL_INFO);
        String log;
        if (mustLog)
          log = F("MHZ19: ");

        // During (and only ever at) sensor boot, 'u' is reported as 15000
        // We log but don't process readings during that time
        if (approximatelyEqual(u, 15000)) {
          if (mustLog)
            log += F("Bootup detected! ");

          if (P049_data->ABC_Disable) {
            // After bootup of the sensor the ABC will be enabled.
            // Thus only actively disable after bootup.
            P049_data->ABC_MustApply = true;
            if (mustLog)
              log                     += F("Will disable ABC when bootup complete. ");
          }
          success = false;

          // Finally, stable readings are used for variables
        } else {
          const int filterValue = PCONFIG(1);

          if (Plugin_049_Check_and_ApplyFilter(UserVar[event->BaseVarIndex], ppm, s, filterValue, log)) {
            UserVar[event->BaseVarIndex]     = (float)ppm;
            UserVar[event->BaseVarIndex + 1] = (float)temp;
            UserVar[event->BaseVarIndex + 2] = (float)u;
            success                          = true;
          } else {
            success = false;
          }
        }

        if ((s == 0) || (s == 64)) {
          // Reading is stable.
          if (P049_data->ABC_MustApply) {
            // Send ABC enable/disable command based on the desired state.
            if (P049_data->ABC_Disable) {
              P049_data->send_mhzCmd(mhzCmdABCDisable);
              addLog(LOG_LEVEL_INFO, F("MHZ19: Sent sensor ABC Disable!"));
            } else {
              P049_data->send_mhzCmd(mhzCmdABCEnable);
              addLog(LOG_LEVEL_INFO, F("MHZ19: Sent sensor ABC Enable!"));
            }
            P049_data->ABC_MustApply = false;
          }
        }

        if (mustLog) {
          // Log values in all cases
          log += F("PPM value: ");
          log += ppm;
          log += F(" Temp/S/U values: ");
          log += temp;
          log += '/';
          log += s;
          log += '/';
          log += u;
          addLog(LOG_LEVEL_INFO, log);
        }
        break;

        // #ifdef ENABLE_DETECTION_RANGE_COMMANDS
        // Sensor responds with 0x99 whenever we send it a measurement range adjustment
      } else if (P049_data->receivedCommandAcknowledgement(expectReset))  {
        addLog(LOG_LEVEL_INFO, F("MHZ19: Received command acknowledgment! "));

        if (expectReset) {
          addLog(LOG_LEVEL_INFO, F("Expecting sensor reset..."));
        }
        success = false;
        break;

        // #endif

        // log verbosely anything else that the sensor reports
      } else {
        if (loglevelActiveFor(LOG_LEVEL_INFO)) {
          String log = F("MHZ19: Unknown response:");
          log += P049_data->getBufferHexDump();
          addLog(LOG_LEVEL_INFO, log);
        }

        // Check for stable reads and allow unstable reads the first 3 minutes after reset.
        if ((P049_data->nrUnknownResponses > 10) && P049_data->initTimePassed) {
          P049_performInit(event);
        }
        success = false;
        break;
      }
      break;
    }
  }
  return success;
}

bool P049_performInit(struct EventStruct *event) {
  bool success                 = false;
  const int16_t serial_rx      = CONFIG_PIN1;
  const int16_t serial_tx      = CONFIG_PIN2;
  const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);
  P049_data_struct *P049_data  =
    static_cast<P049_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P049_data) {
    return success;
  }

  if (P049_data->init(port, serial_rx, serial_tx, (PCONFIG(0) == ABC_disabled))) {
    success = true;
    addLog(LOG_LEVEL_INFO, F("MHZ19: Init OK "));

    // delay first read, because hardware needs to initialize on cold boot
    // otherwise we get a weird value or read error
    Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 15000);
  }
  return success;
}

void P049_html_show_stats(struct EventStruct *event) {
  P049_data_struct *P049_data =
    static_cast<P049_data_struct *>(getPluginTaskData(event->TaskIndex));

  if (nullptr == P049_data) {
    return;
  }

  addRowLabel(F("Checksum (pass/fail/reset)"));
  String chksumStats;

  chksumStats  = P049_data->linesHandled;
  chksumStats += '/';
  chksumStats += P049_data->checksumFailed;
  chksumStats += '/';
  chksumStats += P049_data->sensorResets;
  addHtml(chksumStats);
  addRowLabel(F("Detected"));

  switch (P049_data->getDetectedDevice()) {
    case MHZ19_A: addHtml(F("MH-Z19A")); break;
    case MHZ19_B: addHtml(F("MH-Z19B")); break;
    default: addHtml(F("---")); break;
  }
}

#endif // USES_P049
