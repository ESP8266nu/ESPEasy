#include "_Plugin_Helper.h"
#ifdef USES_P053
//#######################################################################################################
//#################################### Plugin 053: Plantower PMSx003 ####################################
//#######################################################################################################
//
// http://www.aqmd.gov/docs/default-source/aq-spec/resources-page/plantower-pms5003-manual_v2-3.pdf?sfvrsn=2
//
// The PMSx003 are particle sensors. Particles are measured by blowing air through the enclosure and,
// together with a laser, count the amount of particles. These sensors have an integrated microcontroller
// that counts particles and transmits measurement data over the serial connection.


#include <ESPeasySerial.h>

#define PLUGIN_053_ENABLE_EXTRA_SENSORS // Can be unset for memory-tight builds to remove support for the PMSx003ST and PMS2003/PMS3003 sensor models
// #define PLUGIN_053_ENABLE_S_AND_T // Enable setting to support S and T types, in addition to bas PMSx003 and PMSx003ST

#if defined(SIZE_1M) && defined(PLUGIN_053_ENABLE_EXTRA_SENSORS) // Turn off for 1M builds
#undef PLUGIN_053_ENABLE_EXTRA_SENSORS
#endif

#define PLUGIN_053
#define PLUGIN_ID_053 53
#define PLUGIN_NAME_053 "Dust - PMSx003"
#ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
#ifdef PLUGIN_NAME_053
#undef PLUGIN_NAME_053
#endif
#define PLUGIN_NAME_053 "Dust - PMSx003 / PMSx003ST" // 'upgrade' plugin-name
#endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
#define PLUGIN_VALUENAME1_053 "pm1.0"
#define PLUGIN_VALUENAME2_053 "pm2.5"
#define PLUGIN_VALUENAME3_053 "pm10"
#define PLUGIN_VALUENAME4_053 "HCHO"  // Is not set into the Values on purpose.
#define PMSx003_SIG1 0x42
#define PMSx003_SIG2 0x4d
#define PMSx003_SIZE   32
#define PMSx003S_SIZE  32
#define PMSx003T_SIZE  32
#define PMSx003ST_SIZE 40
#define PMS3003_SIZE   24

#define PMSx003_TYPE    0   // PMSx003 = PMS1003 / PMS5003 / PMS7003
#define PMS3003_TYPE    1   // PMS2003/PMS3003
#define PMSx003_TYPE_S  2   // PMS5003S // Not supported yet
#define PMSx003_TYPE_T  3   // PMS5003T // Not supported yet
#define PMSx003_TYPE_ST 4   // PMS5003ST

#define PLUGIN_053_OUTPUT_PART 0 // Particles pm1.0/pm2.5/pm10
#define PLUGIN_053_OUTPUT_THC  1 // pm2.5/Temp/Hum/HCHO

ESPeasySerial *P053_easySerial = nullptr;
boolean Plugin_053_init = false;
boolean values_received = false;
uint8_t Plugin_053_sensortype = PMSx003_TYPE;

// Read 2 bytes from serial and make an uint16 of it. Additionally calculate
// checksum for PMSx003. Assumption is that there is data available, otherwise
// this function is blocking.
void SerialRead16(uint16_t* value, uint16_t* checksum)
{
  uint8_t data_high, data_low;

  // If P053_easySerial is initialized, we are using soft serial
  if (P053_easySerial == nullptr) return;
  data_high = P053_easySerial->read();
  data_low = P053_easySerial->read();

  *value = data_low;
  *value |= (data_high << 8);

  if (checksum != nullptr)
  {
    *checksum += data_high;
    *checksum += data_low;
  }

#if 0
  // Low-level logging to see data from sensor
  String log = F("PMSx003 : uint8_t high=0x");
  log += String(data_high,HEX);
  log += F(" uint8_t low=0x");
  log += String(data_low,HEX);
  log += F(" result=0x");
  log += String(*value,HEX);
  addLog(LOG_LEVEL_INFO, log);
#endif
}

void SerialFlush() {
  if (P053_easySerial != nullptr) {
    P053_easySerial->flush();
  }
}

uint8_t P053_packetSize(uint8_t sensorType) {
  switch(sensorType) {
    case PMSx003_TYPE:    return PMSx003_SIZE;
    #ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
    case PMSx003_TYPE_S:  return PMSx003S_SIZE;
    case PMSx003_TYPE_T:  return PMSx003T_SIZE;
    case PMSx003_TYPE_ST: return PMSx003ST_SIZE;
    case PMS3003_TYPE:    return PMS3003_SIZE;
    #endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  }
  return 0u;
}

boolean PacketAvailable(void)
{
  if (P053_easySerial != nullptr) // Software serial
  {
    // When there is enough data in the buffer, search through the buffer to
    // find header (buffer may be out of sync)
    if (!P053_easySerial->available()) return false;
    while ((P053_easySerial->peek() != PMSx003_SIG1) && P053_easySerial->available()) {
      P053_easySerial->read(); // Read until the buffer starts with the first uint8_t of a message, or buffer empty.
    }
    if (P053_easySerial->available() < P053_packetSize(Plugin_053_sensortype)) return false; // Not enough yet for a complete packet
  }
  return true;
}

boolean Plugin_053_process_data(struct EventStruct *event) {
  uint16_t checksum = 0, checksum2 = 0;
  uint16_t framelength = 0;
  uint16_t packet_header = 0;
  SerialRead16(&packet_header, &checksum); // read PMSx003_SIG1 + PMSx003_SIG2
  if (packet_header != ((PMSx003_SIG1 << 8) | PMSx003_SIG2)) {
    // Not the start of the packet, stop reading.
    return false;
  }

  SerialRead16(&framelength, &checksum);
  if (framelength != (P053_packetSize(Plugin_053_sensortype) - 4)) {
    if (loglevelActiveFor(LOG_LEVEL_ERROR)) {
      String log;
      log.reserve(34);
      log = F("PMSx003 : invalid framelength - ");
      log += framelength;
      addLog(LOG_LEVEL_ERROR, log);
    }
    return false;
  }

  int loopSize = Plugin_053_sensortype == PMSx003_TYPE_ST ? 17 : 13;
  uint16_t data[17]; // uint8_t data_low, data_high;
  for (int i = 0; i < loopSize; i++)
    SerialRead16(&data[i], &checksum);

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;
    log.reserve(87);
    log = F("PMSx003 : pm1.0=");
    log += data[0];
    log += F(", pm2.5=");
    log += data[1];
    log += F(", pm10=");
    log += data[2];
    log += F(", pm1.0a=");
    log += data[3];
    log += F(", pm2.5a=");
    log += data[4];
    log += F(", pm10a=");
    log += data[5];
    addLog(LOG_LEVEL_DEBUG, log);
  }

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;
    log.reserve(96);
    log = F("PMSx003 : count/0.1L : 0.3um=");
    log += data[6];
    log += F(", 0.5um=");
    log += data[7];
    log += F(", 1.0um=");
    log += data[8];
    log += F(", 2.5um=");
    log += data[9];
    log += F(", 5.0um=");
    log += data[10];
    log += F(", 10um=");
    log += data[11];
    addLog(LOG_LEVEL_DEBUG, log);
  }

  #ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
  if (loglevelActiveFor(LOG_LEVEL_DEBUG) && PCONFIG(0) == PMSx003_TYPE_ST) {
    String log;
    log.reserve(45);
    log = F("PMSx003 : temp=");
    log += static_cast<float>(data[13]) / 10.0;
    log += F(", humi=");
    log += static_cast<float>(data[14]) / 10.0;
    log += F(", hcho=");
    log += static_cast<float>(data[12]) / 1000.0;
    addLog(LOG_LEVEL_DEBUG, log);
  }
  #endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

  // Compare checksums
  SerialRead16(&checksum2, nullptr);
  SerialFlush(); // Make sure no data is lost due to full buffer.
  if (checksum == checksum2)
  {
    // Data is checked and good, fill in output
    #ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
    switch(PCONFIG(1)) {
      case PLUGIN_053_OUTPUT_PART:
      {
    #endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
        UserVar[event->BaseVarIndex]     = data[3];
        UserVar[event->BaseVarIndex + 1] = data[4];
        UserVar[event->BaseVarIndex + 2] = data[5];
    #ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
        UserVar[event->BaseVarIndex + 3] = 0.0;
        break;
      }
      case PLUGIN_053_OUTPUT_THC:
      {
        UserVar[event->BaseVarIndex]     = data[4];
        UserVar[event->BaseVarIndex + 1] = static_cast<float>(data[13]) / 10.0;   // TEMP
        UserVar[event->BaseVarIndex + 2] = static_cast<float>(data[14]) / 10.0;   // HUMI
        UserVar[event->BaseVarIndex + 3] = static_cast<float>(data[12]) / 1000.0; // HCHO
        break;
      }
      default:
        break; // Ignore invalid options
    }
    if (PCONFIG(2) == 1 && PCONFIG(0) == PMSx003_TYPE_ST) { // Events only applicable to ST model
      switch(PCONFIG(1)) {
        case PLUGIN_053_OUTPUT_PART:
        {
          String baseEvent;
          baseEvent.reserve(41);
          baseEvent  = getTaskDeviceName(event->TaskIndex);
          baseEvent += '#';
          String valueEvent;
          valueEvent.reserve(52);
          // Temperature
          valueEvent  = baseEvent;
          valueEvent += F("Temp");
          valueEvent += '=';
          valueEvent += static_cast<float>(data[13]) / 10.0;
          eventQueue.addMove(std::move(valueEvent));
          // Humidity
          valueEvent  = baseEvent;
          valueEvent += F("Humi");
          valueEvent += '=';
          valueEvent += static_cast<float>(data[14]) / 10.0;
          eventQueue.addMove(std::move(valueEvent));
          // Formaldebyde (HCHO)
          valueEvent  = baseEvent;
          valueEvent += F("HCHO");
          valueEvent += '=';
          valueEvent += static_cast<float>(data[12]) / 1000.0;
          eventQueue.addMove(std::move(valueEvent));
          break;
        }
        case PLUGIN_053_OUTPUT_THC:
        {
          String baseEvent;
          baseEvent.reserve(41);
          baseEvent  = getTaskDeviceName(event->TaskIndex);
          baseEvent += '#';
          String valueEvent;
          valueEvent.reserve(52);
          // Particles 1.0 ug/m3
          valueEvent  = baseEvent;
          valueEvent += F("pm1.0");
          valueEvent += '=';
          valueEvent += data[3];
          eventQueue.addMove(std::move(valueEvent));
          // Particles 10 ug/m3
          valueEvent  = baseEvent;
          valueEvent += F("pm10");
          valueEvent += '=';
          valueEvent += data[5];
          eventQueue.addMove(std::move(valueEvent));
          break;
        }
        default:
          break;
      }
    }
    #endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
    values_received = true;
    return true;
  }
  return false;
}

boolean Plugin_053(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_053;
        Device[deviceCount].Type = DEVICE_TYPE_SERIAL_PLUS1;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_TRIPLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        #ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 4;
        #else
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 3;
        #endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        success = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_053);
        success = true;
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_053));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_053));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_053));
        #ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
        for (uint8_t i = 0; i < 3; i++) {
          ExtraTaskSettings.TaskDeviceValueDecimals[i] = 0; // Set to former default
        }
        // 4th ValueName and decimals not (re)set on purpose
        #endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
        success = true;
        break;
      }

    #ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
    case PLUGIN_GET_DEVICEVALUECOUNT:
      {
        event->Par1 = PCONFIG(1) == PLUGIN_053_OUTPUT_PART ? 3 : 4;
        success = true;
        break;
      }
    #endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        serialHelper_getGpioNames(event);
        event->String3 = formatGpioName_output(F("Reset"));
        break;
      }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
      {
        string += serialHelper_getSerialTypeLabel(event);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_LOAD: {
      #ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
      {
        addFormSubHeader(F("Device"));
        #ifdef PLUGIN_053_ENABLE_S_AND_T
        int unitModelCount = 5;
        #else
        int unitModelCount = 3;
        #endif // ifdef PLUGIN_053_ENABLE_S_AND_T
        const __FlashStringHelper * unitModels[] = {
          F("PMS1003 / PMS5003 / PMS7003"),
          F("PMS2003 / PMS3003"),
          #ifdef PLUGIN_053_ENABLE_S_AND_T
          F("PMS5003S"),
          F("PMS5003T"),
          #endif // ifdef PLUGIN_053_ENABLE_S_AND_T
          F("PMS5003ST")
        };
        int unitModelOptions[] = {
          PMSx003_TYPE,
          PMS3003_TYPE,
          #ifdef PLUGIN_053_ENABLE_S_AND_T
          PMSx003_TYPE_S,
          PMSx003_TYPE_T,
          #endif // ifdef PLUGIN_053_ENABLE_S_AND_T
          PMSx003_TYPE_ST
        };
        addFormSelector(F("Sensor model"), F("p053_model"), unitModelCount, unitModels, unitModelOptions, PCONFIG(0));
      }
      {
        addFormSubHeader(F("Output"));
        const __FlashStringHelper * outputOptions[] = {
          F("Particles: pm1.0, pm2.5, pm10"),
          F("Particles: pm2.5; Other: Temp, Humi, HCHO (PMS5003ST)")
        };
        int outputOptionValues[] = { PLUGIN_053_OUTPUT_PART, PLUGIN_053_OUTPUT_THC };
        addFormSelector(F("Output values"), F("p053_output"), 2, outputOptions, outputOptionValues, PCONFIG(1), true);
        addFormNote(F("Manually change 'Values' names and decimals accordingly! Changing this reloads the page."));
        addFormCheckBox(F("Events for non-output values"), F("p053_events"), PCONFIG(2) == 1);
        addFormNote(F("Only generates the 'missing' events, (taskname#temp/humi/hcho or taskname#pm1.0/pm10)."));
      }
      #endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE: {
      #ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
      PCONFIG(0) = getFormItemInt(F("p053_model"));
      PCONFIG(1) = getFormItemInt(F("p053_output"));
      if (PCONFIG(0) == PMSx003_TYPE || PCONFIG(0) == PMS3003_TYPE) { // Base models only support particle values, no use in setting other output values
        PCONFIG(1) = PLUGIN_053_OUTPUT_PART;
      }
      PCONFIG(2) = isFormItemChecked(F("P053_events")) ? 1 : 0;
      #endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
      success = true;
      break;
    }

    case PLUGIN_INIT:
      {
        int rxPin = CONFIG_PIN1;
        int txPin = CONFIG_PIN2;
        const ESPEasySerialPort port = static_cast<ESPEasySerialPort>(CONFIG_PORT);
        int resetPin = CONFIG_PIN3;

        #ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS
        Plugin_053_sensortype = PCONFIG(0);
        #endif // ifdef PLUGIN_053_ENABLE_EXTRA_SENSORS

        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
          String log;
          log.reserve(25);
          log = F("PMSx003 : config ");
          log += rxPin;
          log += ' ';
          log += txPin;
          log += ' ';
          log += resetPin;
          addLog(LOG_LEVEL_DEBUG, log);
        }

        if (P053_easySerial != nullptr) {
          // Regardless the set pins, the software serial must be deleted.
          delete P053_easySerial;
          P053_easySerial = nullptr;
        }

        // Hardware serial is RX on 3 and TX on 1
        if (rxPin == 3 && txPin == 1) {
          addLog(LOG_LEVEL_INFO, F("PMSx003 : using hardware serial"));
        } else {
          addLog(LOG_LEVEL_INFO, F("PMSx003: using software serial"));
        }
        P053_easySerial = new (std::nothrow) ESPeasySerial(port, rxPin, txPin, false, 96); // 96 Bytes buffer, enough for up to 3 packets.
        if (P053_easySerial == nullptr) {
          break;
        }
        P053_easySerial->begin(9600);
        P053_easySerial->flush();

        if (resetPin >= 0) { // Reset if pin is configured
          // Toggle 'reset' to assure we start reading header
          addLog(LOG_LEVEL_INFO, F("PMSx003: resetting module"));
          pinMode(resetPin, OUTPUT);
          digitalWrite(resetPin, LOW);
          delay(250);
          digitalWrite(resetPin, HIGH);
          pinMode(resetPin, INPUT_PULLUP);
        }

        Plugin_053_init = true;
        success = true;
        break;
      }

    case PLUGIN_EXIT:
      {
        if (P053_easySerial) {
          delete P053_easySerial;
          P053_easySerial=nullptr;
        }
        break;
      }

    // The update rate from the module is 200ms .. multiple seconds. Practise
    // shows that we need to read the buffer many times per seconds to stay in
    // sync.
    case PLUGIN_TEN_PER_SECOND:
      {
        if (Plugin_053_init && PacketAvailable()) {
          // Check if a complete packet is available in the UART FIFO.
            addLog(LOG_LEVEL_DEBUG_MORE, F("PMSx003 : Packet available"));
            success = Plugin_053_process_data(event);
        }
        break;
      }
    case PLUGIN_READ:
      {
        // When new data is available, return true
        success = values_received;
        values_received = false;
        break;
      }
  }
  return success;
}
#endif // USES_P053
