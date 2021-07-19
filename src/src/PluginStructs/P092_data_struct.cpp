#include "../PluginStructs/P092_data_struct.h"

#ifdef USES_P092

//
// DLBus reads and decodes the DL-Bus.
// The DL-Bus is used in heating control units e.g. sold by Technische Alternative (www.ta.co.at).
// Author uwekaditz

// #define DLbus_DEBUG

// Flags for pulse width (bit 0 is the content!)
#define DLbus_FlagSingleWidth                 0x02
#define DLbus_FlagDoubleWidth                 0x04
#define DLbus_FlagShorterThanSingleWidth      0x10
#define DLbus_FlagBetweenDoubleSingleWidth    0x20
#define DLbus_FlagLongerThanDoubleWidth       0x40
#define DLbus_FlagLongerThanTwiceDoubleWidth  0x80
#define DLbus_FlagsWrongTiming                (DLbus_FlagLongerThanTwiceDoubleWidth | DLbus_FlagLongerThanDoubleWidth | \
                                               DLbus_FlagBetweenDoubleSingleWidth | DLbus_FlagShorterThanSingleWidth)

// Helper for ISR call
DLBus *DLBus::__instance                         = nullptr;
volatile  static uint8_t *ISR_PtrChangeBitStream = nullptr; // pointer to received bit change stream

DLBus::DLBus()
{
  if (__instance == nullptr)
  {
    __instance             = this;
    ISR_PtrChangeBitStream = DLbus_ChangeBitStream;
    addToLog(LOG_LEVEL_INFO, F("Class DLBus created"));
  }
}

DLBus::~DLBus()
{
  if (__instance == this)
  {
    __instance             = nullptr;
    ISR_PtrChangeBitStream = nullptr;
    addToLog(LOG_LEVEL_INFO, F("Class DLBus destroyed"));
  }
}

void DLBus::AddToInfoLog(const String& string)
{
  if ((IsLogLevelInfo) && (LogLevelInfo != 0xff)) {
    addToLog(LogLevelInfo, string);
  }
}

void DLBus::AddToErrorLog(const String& string)
{
  if (LogLevelError != 0xff) {
    addToLog(LogLevelError, string);
  }
}

void DLBus::attachDLBusInterrupt(void)
{
  ISR_Receiving = false;
  IsISRset = true;
  IsNoData = false;
  attachInterrupt(digitalPinToInterrupt(ISR_DLB_Pin), ISR, CHANGE);
}

void DLBus::StartReceiving(void)
{
  noInterrupts(); // make sure we don't get interrupted before we are ready
  ISR_PulseCount      = 0;
  ISR_Receiving       = (ISR_PtrChangeBitStream != nullptr);
  ISR_AllBitsReceived = false;
  interrupts(); // interrupts allowed now, next instruction WILL be executed
}

void IRAM_ATTR DLBus::ISR(void)
{
  if (__instance)
  {
    __instance->ISR_PinChanged();
  }
}

void IRAM_ATTR DLBus::ISR_PinChanged(void)
{
//  long TimeDiff = usecPassedSince(ISR_TimeLastBitChange); // time difference to previous pulse in Âµs
  uint32_t _now = micros();
  int32_t TimeDiff = (int32_t)(_now - ISR_TimeLastBitChange);

//  ISR_TimeLastBitChange = micros();                           // save last pin change time
  ISR_TimeLastBitChange = _now;                           // save last pin change time

  if (ISR_Receiving) {
    uint8_t val = digitalRead(ISR_DLB_Pin);               // read state

    // check pulse width
    if (TimeDiff >= 2 * ISR_MinDoublePulseWidth) {
      val |= DLbus_FlagLongerThanTwiceDoubleWidth; // longer then 2x double pulse width
    }
    else if (TimeDiff > ISR_MaxDoublePulseWidth) {
      val |= DLbus_FlagLongerThanDoubleWidth;      // longer then double pulse width
    }
    else if (TimeDiff >= ISR_MinDoublePulseWidth) {
      val |= DLbus_FlagDoubleWidth;                // double pulse width
    }
    else if (TimeDiff > ISR_MaxPulseWidth) {
      val |= DLbus_FlagBetweenDoubleSingleWidth;   // between double and single pulse width
    }
    else if (TimeDiff < ISR_MinPulseWidth) {
      val |= DLbus_FlagShorterThanSingleWidth;     // shorter then single pulse width
    }
    else {
      val |= DLbus_FlagSingleWidth;                // single pulse width
    }

    if (ISR_PulseCount < 2) {
      // check if sync is received
      if (val & DLbus_FlagLongerThanTwiceDoubleWidth) {
        // sync received
        *ISR_PtrChangeBitStream       = !(val & 0x01);
        *(ISR_PtrChangeBitStream + 1) = val;
        ISR_PulseCount                = 2;
      }
      else {
        ISR_PulseCount = 1; // flag that interrupt is receiving
      }
    }
    else {
      *(ISR_PtrChangeBitStream + ISR_PulseCount) = val;         // set bit
      ISR_PulseCount++;
      ISR_Receiving       = (ISR_PulseCount < ISR_PulseNumber); // stop P092_receiving when data frame is complete
      ISR_AllBitsReceived = !ISR_Receiving;
    }
  }
}

boolean DLBus::CheckTimings(void) {
  uint8_t rawval, val;
  uint8_t WrongTimeCnt = 0;
  int     i;

#ifdef DLbus_DEBUG
  uint16_t WrongTimingArray[5][6];
#endif // DLbus_DEBUG

  //  AddToInfoLog(F("Receive stopped."));

  ISR_PulseCount = 0;

  for (i = 0; i <= ISR_PulseNumber; i++) {
    // store DLbus_ChangeBitStream into ByteStream
    rawval = *(ISR_PtrChangeBitStream + i);

    if (rawval & DLbus_FlagsWrongTiming) {
      // wrong DLbus_time_diff
      if (ISR_PulseCount > 0) {
#ifdef DLbus_DEBUG
        WrongTimingArray[WrongTimeCnt][0] = i;
        WrongTimingArray[WrongTimeCnt][1] = ISR_PulseCount;
        WrongTimingArray[WrongTimeCnt][2] = BitNumber;
        WrongTimingArray[WrongTimeCnt][3] = rawval;
#endif // DLbus_DEBUG

        if ((rawval == DLbus_FlagLongerThanTwiceDoubleWidth) && (*(ISR_PtrChangeBitStream + i - 1) == (DLbus_FlagDoubleWidth | 0x01))) {
          // Add two additional short pulses (low and high), previous bit is High and contains DLbus_FlagDoubleWidth
          ProcessBit(0);
          ProcessBit(1);
#ifdef DLbus_DEBUG
          WrongTimingArray[WrongTimeCnt][4] = DLbus_FlagSingleWidth;
          WrongTimingArray[WrongTimeCnt][5] = DLbus_FlagSingleWidth + 1;
#endif // DLbus_DEBUG
        }
#ifdef DLbus_DEBUG
        else {
          WrongTimingArray[WrongTimeCnt][4] = 0xff;
          WrongTimingArray[WrongTimeCnt][5] = 0xff;
        }
#endif // DLbus_DEBUG
        WrongTimeCnt++;

        if (WrongTimeCnt >= 5) {
          return false;
        }
      }
    }
    else {
      val = rawval & 0x01;

      if ((rawval & DLbus_FlagDoubleWidth) == DLbus_FlagDoubleWidth) {
        // double pulse width
        ProcessBit(!val);
        ProcessBit(val);
      }
      else {
        // single pulse width
        ProcessBit(val);
      }
    }
  }

  //  AddToInfoLog(F("DLbus_ChangeBitStream copied."));

#ifdef DLbus_DEBUG

  if (WrongTimeCnt > 0) {
    if (IsLogLevelInfo) {
      String log = F("Wrong Timings: ");
      AddToInfoLog(log);

      for (i = 0; i < WrongTimeCnt; i++) {
        log  = i + 1;
        log += F(": PulseCount:");
        log += WrongTimingArray[i][1];
        log += F(": BitCount:");
        log += WrongTimingArray[i][2];
        log += F(" Value:0x");
        log += String(WrongTimingArray[i][3], HEX);
        log += F(" ValueBefore:0x");
        log += String(*(ISR_PtrChangeBitStream + WrongTimingArray[i][0] - 1), HEX);
        log += F(" ValueAfter:0x");
        log += String(*(ISR_PtrChangeBitStream + WrongTimingArray[i][0] + 1), HEX);

        if (WrongTimingArray[i][4] != 0xff) {
          log += F(" Added:0x");
          log += String(WrongTimingArray[i][4], HEX);
        }

        if (WrongTimingArray[i][5] != 0xff) {
          log += F(" Added:0x");
          log += String(WrongTimingArray[i][5], HEX);
        }
        AddToInfoLog(log);
      }
    }
  }
#endif // DLbus_DEBUG
  return true;
}

void DLBus::ProcessBit(uint8_t b) {
  // ignore first pulse
  ISR_PulseCount++;

  if (ISR_PulseCount % 2) {
    return;
  }
  BitNumber = (ISR_PulseCount / 2);

  if (b) {
    ByteStream[BitNumber / 8] |= (1 << (BitNumber % 8));  // set bit
  }
  else {
    ByteStream[BitNumber / 8] &= ~(1 << (BitNumber % 8)); // clear bit
  }
}

boolean DLBus::Processing(void) {
  boolean inverted = false;
  int16_t StartBit; // first bit of data frame (-1 not recognized)
  String  log;

  AddToInfoLog(F("Processing..."));
  StartBit = Analyze(); // find the data frame's beginning

  // inverted signal?
  while (StartBit == -1) {
    if (inverted) {
      AddToErrorLog(F("Error: Already inverted!"));
      return false;
    }
    Invert(); // invert again
    inverted = true;
    StartBit = Analyze();

    if (StartBit == -1) {
      AddToErrorLog(F("Error: No data frame available!"));
      return false;
    }
    uint16_t RequiredBitStreamLength = (ISR_PulseNumber - DLBus_ReserveBytes) / DLBus_BitChangeFactor;

    if ((BitNumber - StartBit) < RequiredBitStreamLength) {
      // no complete data frame available (difference between start_bit and received bits is < RequiredBitStreamLength)
      AddToErrorLog(F("Start bit too close to end of stream!"));

      if (IsLogLevelInfo) {
        log  = F("# Required bits: ");
        log += RequiredBitStreamLength;
        log += F(" StartBit: ");
        log += StartBit;
        log += F(" / EndBit: ");
        log += BitNumber;
        AddToInfoLog(log);
      }
      return false;
    }
  }

  if (IsLogLevelInfo) {
    log  = F("StartBit: ");
    log += StartBit;
    log += F(" / EndBit: ");
    log += BitNumber;
    AddToInfoLog(log);
  }
  Trim(StartBit);      // remove start and stop bits

  if (CheckDevice()) { // check connected device
    return true;
  }
  else {
    AddToErrorLog(F("Error: Device not found!"));
    return false;
  }
}

int DLBus::Analyze(void) {
  uint8_t sync = 0;

  // find SYNC (16 * sequential 1)
  for (int i = 0; i < BitNumber; i++) {
    if (ReadBit(i)) {
      sync++;
    }
    else {
      sync = 0;
    }

    if (sync == DLBus_SyncBits) {
      // finde erste 0 // find first 0
      while (ReadBit(i) == 1) {
        i++;
      }
      return i; // beginning of data frame
    }
  }

  // no data frame available. check signal?
  return -1;
}

void DLBus::Invert(void) {
  AddToInfoLog(F("Invert bit stream..."));

  for (int i = 0; i < BitNumber; i++) {
    WriteBit(i, ReadBit(i) ? 0 : 1); // invert every bit
  }
}

uint8_t DLBus::ReadBit(int pos) {
  int row = pos / 8;                          // detect position in bitmap
  int col = pos % 8;

  return ((ByteStream[row]) >> (col)) & 0x01; // return bit
}

void DLBus::WriteBit(int pos, uint8_t set) {
  int row = pos / 8; // detect position in bitmap
  int col = pos % 8;

  if (set) {
    ByteStream[row] |= 1 << col;    // set bit
  }
  else {
    ByteStream[row] &= ~(1 << col); // clear bit
  }
}

void DLBus::Trim(int start_bit) {
  for (int i = start_bit, bit = 0; i < BitNumber; i++) {
    int offset = i - start_bit;

    // ignore start and stop bits:
    // start bits: 0 10 20 30, also  x    % 10 == 0
    // stop bits:  9 19 29 39, also (x+1) % 10 == 0
    if (offset % 10 && (offset + 1) % 10) {
      WriteBit(bit, ReadBit(i));
      bit++;
    }
  }
}

boolean DLBus::CheckDevice(void) {
  // Data frame of a device?
  if (ByteStream[0] == DeviceBytes[0]) {
    if ((DeviceBytes[1] == 0) || (ByteStream[1] == DeviceBytes[1])) {
      return true;
    }
  }

  if (IsLogLevelInfo) {
    String log = F("# Received DeviceByte(s): 0x");
    log += String(ByteStream[0], HEX);

    if (DeviceBytes[1] != 0) {
      log += String(ByteStream[1], HEX);
    }
    log += F(" Requested: 0x");
    log += String(DeviceBytes[0], HEX);

    if (DeviceBytes[1] != 0) {
      log += String(DeviceBytes[1], HEX);
    }
    AddToInfoLog(log);
  }
  return false;
}

boolean DLBus::CheckCRC(uint8_t IdxCRC) {
  // CRC check sum
  if (IdxCRC == 0) {
    return true;
  }
  AddToInfoLog(F("Check CRC..."));
  uint16_t dataSum = 0;

  for (int i = 0; i < IdxCRC; i++) {
    dataSum = dataSum + ByteStream[i];
  }
  dataSum = dataSum & 0xff;

  if (dataSum == ByteStream[IdxCRC]) {
    return true;
  }
  AddToErrorLog(F("Check CRC failed!"));

  if (IsLogLevelInfo) {
    String log = F("# Calculated CRC: 0x");
    log += String(dataSum, HEX);
    log += F(" Received: 0x");
    log += String(ByteStream[IdxCRC], HEX);
    AddToInfoLog(log);
  }
  return false;
}

// sensor types
# define DLbus_UNUSED              0b000
# define DLbus_Sensor_DIGITAL      0b001
# define DLbus_Sensor_TEMP         0b010
# define DLbus_Sensor_VOLUME_FLOW  0b011
# define DLbus_Sensor_RAYS         0b110
# define DLbus_Sensor_ROOM         0b111

// room sensor modes
# define DLbus_RSM_AUTO            0b00
# define DLbus_RSM_NORMAL          0b01
# define DLbus_RSM_LOWER           0b10
# define DLbus_RSM_STANDBY         0b11


P092_data_struct::P092_data_struct() {}

P092_data_struct::~P092_data_struct() {
  if (DLbus_Data != nullptr) {
    if (DLbus_Data->ISR_DLB_Pin != 0xFF) {
      detachInterrupt(digitalPinToInterrupt(DLbus_Data->ISR_DLB_Pin));
    }
    delete DLbus_Data;
    DLbus_Data = nullptr;
  }

}

bool P092_data_struct::init(int8_t pin1, int P092DeviceIndex, eP092pinmode P092pinmode) {
  DLbus_Data = new (std::nothrow) DLBus;

  if (DLbus_Data == nullptr) {
    return false;
  }
  DLbus_Data->LogLevelInfo   = LOG_LEVEL_INFO;
  DLbus_Data->LogLevelError  = LOG_LEVEL_ERROR;
  DLbus_Data->IsLogLevelInfo = loglevelActiveFor(LOG_LEVEL_INFO);
  DLbus_Data->ISR_DLB_Pin    = pin1;

  //interrupt is detached in PLUGIN_WEBFORM_SAVE and attached in PLUGIN_ONCE_A_SECOND
  //to ensure that new interrupt is attached after new pin is configured, setting
  //IsISRset to false is done here.
  DLbus_Data->IsISRset       = false;

  switch (P092pinmode) {
    case eP092pinmode::ePPM_InputPullUp:
      addLog(LOG_LEVEL_INFO, F("P092_init: Set input pin with pullup"));
      pinMode(pin1, INPUT_PULLUP);
    break;
#ifdef INPUT_PULLDOWN
    case eP092pinmode::ePPM_InputPullDown:
      addLog(LOG_LEVEL_INFO, F("P092_init: Set input pin with pulldown"));
      pinMode(pin1, INPUT_PULLDOWN);
    break;
#endif
    default:
      addLog(LOG_LEVEL_INFO, F("P092_init: Set input pin"));
      pinMode(pin1, INPUT);
  }

// on a CHANGE on the data pin P092_Pin_changed is called
//DLbus_Data->attachDLBusInterrupt();
  return true;
}

void P092_data_struct::Plugin_092_SetIndices(int P092DeviceIndex) {
  // Set the indices for the DL bus packet
  int iDeviceBytes, iDontCareBytes, iTimeStampBytes;

  // default settings for ESR21
  P092_DataSettings.DataBytes                 = 31;
  P092_DataSettings.DLbus_MinPulseWidth       = P092_min_width_488;
  P092_DataSettings.DLbus_MaxPulseWidth       = P092_max_width_488;
  P092_DataSettings.DLbus_MinDoublePulseWidth = P092_double_min_width_488;
  P092_DataSettings.DLbus_MaxDoublePulseWidth = P092_double_max_width_488;

  P092_DataSettings.DeviceByte0    = 0x70;
  P092_DataSettings.DeviceByte1    = 0x8F;
  iDeviceBytes                     = 2;
  iDontCareBytes                   = 0;
  iTimeStampBytes                  = 0;
  P092_DataSettings.MaxSensors     = 3;
  P092_DataSettings.MaxExtSensors  = 6;
  P092_DataSettings.OutputBytes    = 1;
  P092_DataSettings.SpeedBytes     = 1;
  P092_DataSettings.MaxAnalogOuts  = 1;
  P092_DataSettings.AnalogBytes    = 1;
  P092_DataSettings.VolumeBytes    = 0;
  P092_DataSettings.MaxHeatMeters  = 1;
  P092_DataSettings.CurrentHmBytes = 2;
  P092_DataSettings.MWhBytes       = 2;
  P092_DataSettings.IdxCRC         = 30;

  switch (P092DeviceIndex) {
    case 31: // UVR31
      P092_DataSettings.DataBytes                 = 8;
      P092_DataSettings.DLbus_MinPulseWidth       = P092_min_width_50;
      P092_DataSettings.DLbus_MaxPulseWidth       = P092_max_width_50;
      P092_DataSettings.DLbus_MinDoublePulseWidth = P092_double_min_width_50;
      P092_DataSettings.DLbus_MaxDoublePulseWidth = P092_double_max_width_50;

      P092_DataSettings.DeviceByte0    = 0x30;
      P092_DataSettings.DeviceByte1    = 0;
      iDeviceBytes                     = 1;
      P092_DataSettings.MaxExtSensors  = 0;
      P092_DataSettings.SpeedBytes     = 0;
      P092_DataSettings.AnalogBytes    = 0;
      P092_DataSettings.MaxAnalogOuts  = 0;
      P092_DataSettings.MaxHeatMeters  = 0;
      P092_DataSettings.CurrentHmBytes = 0;
      P092_DataSettings.MWhBytes       = 0;
      P092_DataSettings.IdxCRC         = 0;
      break;
    case 1611: // UVR1611
      P092_DataSettings.DataBytes = 64;

      P092_DataSettings.DeviceByte0    = 0x80;
      P092_DataSettings.DeviceByte1    = 0x7F;
      iDontCareBytes                   = 1;
      iTimeStampBytes                  = 5;
      P092_DataSettings.MaxSensors     = 16;
      P092_DataSettings.MaxExtSensors  = 0;
      P092_DataSettings.OutputBytes    = 2;
      P092_DataSettings.SpeedBytes     = 4;
      P092_DataSettings.AnalogBytes    = 0;
      P092_DataSettings.MaxAnalogOuts  = 0;
      P092_DataSettings.MaxHeatMeters  = 2;
      P092_DataSettings.CurrentHmBytes = 4;
      P092_DataSettings.IdxCRC         = P092_DataSettings.DataBytes - 1;

      break;
    case 6132: // UVR 61-3 (up to V8.2)
      P092_DataSettings.DataBytes = 35;

      P092_DataSettings.DeviceByte0   = 0x90;
      P092_DataSettings.DeviceByte1   = 0x6F;
      iDontCareBytes                  = 1;
      iTimeStampBytes                 = 5;
      P092_DataSettings.MaxSensors    = 6;
      P092_DataSettings.MaxExtSensors = 0;
      P092_DataSettings.MaxAnalogOuts = 1;
      P092_DataSettings.VolumeBytes   = 2;
      P092_DataSettings.MWhBytes      = 4;
      P092_DataSettings.IdxCRC        = P092_DataSettings.DataBytes - 1;

      break;
    case 6133: // UVR 61-3 (from V8.3)
      P092_DataSettings.DataBytes = 62;

      P092_DataSettings.DeviceByte0   = 0x90;
      P092_DataSettings.DeviceByte1   = 0x9F;
      iDontCareBytes                  = 1;
      iTimeStampBytes                 = 5;
      P092_DataSettings.MaxSensors    = 6;
      P092_DataSettings.MaxExtSensors = 9;
      P092_DataSettings.MaxAnalogOuts = 2;
      P092_DataSettings.MaxHeatMeters = 3;
      P092_DataSettings.IdxCRC        = P092_DataSettings.DataBytes - 1;

      break;
  }
  P092_DataSettings.IdxSensor     = iDeviceBytes + iDontCareBytes + iTimeStampBytes;
  P092_DataSettings.IdxExtSensor  = P092_DataSettings.IdxSensor + 2 * P092_DataSettings.MaxSensors;
  P092_DataSettings.IdxOutput     = P092_DataSettings.IdxExtSensor + 2 * P092_DataSettings.MaxExtSensors;
  P092_DataSettings.IdxDrehzahl   = P092_DataSettings.IdxOutput + P092_DataSettings.OutputBytes;
  P092_DataSettings.IdxAnalog     = P092_DataSettings.IdxDrehzahl + P092_DataSettings.SpeedBytes;
  P092_DataSettings.IdxHmRegister = P092_DataSettings.IdxAnalog + (P092_DataSettings.AnalogBytes * P092_DataSettings.MaxAnalogOuts);
  P092_DataSettings.IdxVolume     = P092_DataSettings.IdxHmRegister + 1;
  P092_DataSettings.IdxHeatMeter1 = P092_DataSettings.IdxVolume + P092_DataSettings.VolumeBytes;
  P092_DataSettings.IdxkWh1       = P092_DataSettings.IdxHeatMeter1 + P092_DataSettings.CurrentHmBytes;
  P092_DataSettings.IdxMWh1       = P092_DataSettings.IdxkWh1 + 2;
  P092_DataSettings.IdxHeatMeter2 = P092_DataSettings.IdxMWh1 + P092_DataSettings.MWhBytes;
  P092_DataSettings.IdxkWh2       = P092_DataSettings.IdxHeatMeter2 + P092_DataSettings.CurrentHmBytes;
  P092_DataSettings.IdxMWh2       = P092_DataSettings.IdxkWh2 + 2;
  P092_DataSettings.IdxHeatMeter3 = P092_DataSettings.IdxMWh2 + P092_DataSettings.MWhBytes;
  P092_DataSettings.IdxkWh3       = P092_DataSettings.IdxHeatMeter3 + P092_DataSettings.CurrentHmBytes;
  P092_DataSettings.IdxMWh3       = P092_DataSettings.IdxkWh3 + 2;
}

/****************\
   DLBus P092_receiving
\****************/
void P092_data_struct::Plugin_092_StartReceiving(taskIndex_t taskindex) {
  DLbus_Data->ISR_Receiving   = false;
  DLbus_Data->DeviceBytes[0]  = P092_DataSettings.DeviceByte0;
  DLbus_Data->DeviceBytes[1]  = P092_DataSettings.DeviceByte1;
  DLbus_Data->ISR_PulseNumber =
    (((P092_DataSettings.DataBytes + DLbus_AdditionalRecBytes) * (DLbus_StartBits + 8 +  DLbus_StopBits) + DLBus_SyncBits) *
     DLBus_BitChangeFactor) + DLBus_ReserveBytes;
  DLbus_Data->ISR_MinPulseWidth       = P092_DataSettings.DLbus_MinPulseWidth;
  DLbus_Data->ISR_MaxPulseWidth       = P092_DataSettings.DLbus_MaxPulseWidth;
  DLbus_Data->ISR_MinDoublePulseWidth = P092_DataSettings.DLbus_MinDoublePulseWidth;
  DLbus_Data->ISR_MaxDoublePulseWidth = P092_DataSettings.DLbus_MaxDoublePulseWidth;
  DLbus_Data->StartReceiving();
  uint32_t start = millis();

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("P092_receiving ... TaskIndex:");
    log += taskindex;
    addLog(LOG_LEVEL_INFO, log);
  }

  while ((timePassedSince(start) < 100) && (DLbus_Data->ISR_PulseCount == 0)) {
    // wait for first pulse received (timeout 100ms)
    delay(0);
  }

  if (DLbus_Data->ISR_PulseCount == 0) {
    // nothing received
    DLbus_Data->ISR_Receiving = false;
    DLbus_Data->IsNoData = true;  // stop receiving until next PLUGIN_092_READ
    addLog(LOG_LEVEL_ERROR, F("## StartReceiving: Error: Nothing received! No DL bus connected!"));
  }
}

/****************\
   DLBus get data
\****************/
boolean P092_data_struct::P092_GetData(int OptionIdx, int CurIdx, sP092_ReadData *ReadData) {
  String  log;
  boolean result = false;

  switch (OptionIdx) {
    case 1: // F("Sensor")
      log  = F("Get Sensor");
      log += CurIdx;

      if (CurIdx > P092_DataSettings.MaxSensors) {
        result = false;
        break;
      }
      ReadData->Idx = P092_DataSettings.IdxSensor;
      result        = P092_fetch_sensor(CurIdx, ReadData);
      break;
    case 2: // F("Sensor")
      log  = F("Get ExtSensor");
      log += CurIdx;

      if (CurIdx > P092_DataSettings.MaxExtSensors) {
        result = false;
        break;
      }
      ReadData->Idx = P092_DataSettings.IdxExtSensor;
      result        = P092_fetch_sensor(CurIdx, ReadData);
      break;
    case 3: // F("Digital output")
      log  = F("Get DigitalOutput");
      log += CurIdx;

      if (CurIdx > (8 * P092_DataSettings.OutputBytes)) {
        result = false;
        break;
      }
      result = P092_fetch_output(CurIdx, ReadData);
      break;
    case 4: // F("Speed step")
      log  = F("Get SpeedStep");
      log += CurIdx;

      if (CurIdx > P092_DataSettings.SpeedBytes) {
        result = false;
        break;
      }
      result = P092_fetch_speed(CurIdx, ReadData);
      break;
    case 5: // F("Analog output")
      log  = F("Get AnalogOutput");
      log += CurIdx;

      if (CurIdx > P092_DataSettings.AnalogBytes) {
        result = false;
        break;
      }
      result = P092_fetch_analog(CurIdx, ReadData);
      break;
    case 6: // F("Heat power (kW)")
      log  = F("Get HeatPower");
      log += CurIdx;

      if (CurIdx > P092_DataSettings.MaxHeatMeters) {
        result = false;
        break;
      }
      result = P092_fetch_heatpower(CurIdx, ReadData);
      break;
    case 7: // F("Heat meter (MWh)"
      log  = F("Get HeatMeter");
      log += CurIdx;

      if (CurIdx > P092_DataSettings.MaxHeatMeters) {
        result = false;
        break;
      }
      result = P092_fetch_heatmeter(CurIdx, ReadData);
      break;
  }

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    log += F(": ");

    if (result) {
      log += String(ReadData->value, 1);
    }
    else {
      log += F("nan");
    }
    addLog(LOG_LEVEL_INFO, log);
  }
  return result;
}

boolean P092_data_struct::P092_fetch_sensor(int number, sP092_ReadData *ReadData) {
  float value;

  ReadData->mode = -1;
  number         = ReadData->Idx + (number - 1) * 2;
  int32_t sensorvalue = (DLbus_Data->ByteStream[number + 1] << 8) | DLbus_Data->ByteStream[number];

  if (sensorvalue == 0) {
    return false;
  }
  uint8_t sensortype = (sensorvalue & 0x7000) >> 12;

  if (!(sensorvalue & 0x8000)) { // sign positive
    sensorvalue &= 0xfff;

    // calculations for different sensor types
    switch (sensortype) {
      case DLbus_Sensor_DIGITAL:
        value = false;
        break;
      case DLbus_Sensor_TEMP:
        value = sensorvalue * 0.1;
        break;
      case DLbus_Sensor_RAYS:
        value = sensorvalue;
        break;
      case DLbus_Sensor_VOLUME_FLOW:
        value = sensorvalue * 4;
        break;
      case DLbus_Sensor_ROOM:
        ReadData->mode = (sensorvalue & 0x600) >> 9;
        value          = (sensorvalue & 0x1ff) * 0.1;
        break;
      default:
        return false;
    }
  }
  else { // sign negative
    sensorvalue |= 0xf000;

    // calculations for different sensor types
    switch (sensortype) {
      case DLbus_Sensor_DIGITAL:
        value = true;
        break;
      case DLbus_Sensor_TEMP:
        value = (sensorvalue - 0x10000) * 0.1;
        break;
      case DLbus_Sensor_RAYS:
        value = sensorvalue - 0x10000;
        break;
      case DLbus_Sensor_VOLUME_FLOW:
        value = (sensorvalue - 0x10000) * 4;
        break;
      case DLbus_Sensor_ROOM:
        ReadData->mode = (sensorvalue & 0x600) >> 9;
        value          = ((sensorvalue & 0x1ff) - 0x10000) * 0.1;
        break;
      default:
        return false;
    }
  }
  ReadData->value = value;
  return true;
}

boolean P092_data_struct::P092_fetch_output(int number, sP092_ReadData *ReadData) {
  int32_t outputs;

  if (P092_DataSettings.OutputBytes > 1) {
    outputs = (DLbus_Data->ByteStream[P092_DataSettings.IdxOutput + 1] << 8) | DLbus_Data->ByteStream[P092_DataSettings.IdxOutput];
  }
  else {
    outputs = DLbus_Data->ByteStream[P092_DataSettings.IdxOutput];
  }

  if (outputs & (1 << (number - 1))) {
    ReadData->value = 1;
  }
  else {
    ReadData->value = 0;
  }
  return true;
}

boolean P092_data_struct::P092_fetch_speed(int number, sP092_ReadData *ReadData) {
  uint8_t speedbyte;

  if ((P092_DataSettings.IdxDrehzahl + (number - 1)) >= P092_DataSettings.IdxAnalog) {
    // wrong index for speed, overlapping next index (IdxAnalog)
    return false;
  }
  speedbyte = DLbus_Data->ByteStream[P092_DataSettings.IdxDrehzahl + (number - 1)];

  if (speedbyte & 0x80) {
    return false;
  }
  ReadData->value = (speedbyte & 0x1f);
  return true;
}

boolean P092_data_struct::P092_fetch_analog(int number, sP092_ReadData *ReadData) {
  uint8_t analogbyte;

  if ((P092_DataSettings.IdxAnalog + (number - 1)) >= P092_DataSettings.IdxHmRegister) {
    // wrong index for analog, overlapping next index (IdxHmRegister)
    return false;
  }
  analogbyte = DLbus_Data->ByteStream[P092_DataSettings.IdxAnalog + (number - 1)];

  if (analogbyte & 0x80) {
    return false;
  }
  ReadData->value = (analogbyte * 0.1);
  return true;
}

P092_data_struct::sDLbus_HMindex P092_data_struct::P092_CheckHmRegister(int number) {
  sDLbus_HMindex result;

  result.IndexIsValid = 0;

  switch (number) {
    case 1:

      if ((DLbus_Data->ByteStream[P092_DataSettings.IdxHmRegister] & 0x1) == 0) {
        return result;
      }
      result.power_index = P092_DataSettings.IdxHeatMeter1;
      result.kwh_index   = P092_DataSettings.IdxkWh1;
      result.mwh_index   = P092_DataSettings.IdxMWh1;
      break;
    case 2:

      if ((DLbus_Data->ByteStream[P092_DataSettings.IdxHmRegister] & 0x2) == 0) {
        return result;
      }
      result.power_index = P092_DataSettings.IdxHeatMeter2;
      result.kwh_index   = P092_DataSettings.IdxkWh2;
      result.mwh_index   = P092_DataSettings.IdxMWh2;
      break;
    case 3:

      if ((DLbus_Data->ByteStream[P092_DataSettings.IdxHmRegister] & 0x4) == 0) {
        return result;
      }
      result.power_index = P092_DataSettings.IdxHeatMeter3;
      result.kwh_index   = P092_DataSettings.IdxkWh3;
      result.mwh_index   = P092_DataSettings.IdxMWh3;
      break;
    default:
      return result;
  }
  result.IndexIsValid = 1;
  return result;
}

boolean P092_data_struct::P092_fetch_heatpower(int number, sP092_ReadData *ReadData) {
  // current power
  int32_t high;
  sDLbus_HMindex HMindex = P092_CheckHmRegister(number);

  if (HMindex.IndexIsValid == 0) {
    return false;
  }
  uint8_t b1 = DLbus_Data->ByteStream[HMindex.power_index];
  uint8_t b2 = DLbus_Data->ByteStream[HMindex.power_index + 1];

  if (P092_DataSettings.CurrentHmBytes > 2) {
    uint8_t b3 = DLbus_Data->ByteStream[HMindex.power_index + 2];
    uint8_t b4 = DLbus_Data->ByteStream[HMindex.power_index + 3];
    high = 0x10000 * b4 + 0x100 * b3 + b2;
    int low = (b1 * 10) / 0x100;

    if (!(b4 & 0x80)) { // sign positive
      ReadData->value = (10 * high + low) / 100;
    }
    else {              // sign negative
      ReadData->value = (10 * (high - 0x10000) - low) / 100;
    }
  }
  else {
    high = (b2 << 8) | b1;

    if ((b2 & 0x80) == 0) { // sign positive
      ReadData->value = high / 10;
    }
    else {                  // sign negative
      ReadData->value = (high - 0x10000) / 10;
    }
  }
  return true;
}

boolean P092_data_struct::P092_fetch_heatmeter(int number, sP092_ReadData *ReadData) {
  // heat meter
  int32_t heat_meter;
  float   heat_meter_mwh;

  sDLbus_HMindex HMindex = P092_CheckHmRegister(number);

  if (HMindex.IndexIsValid == 0) {
    return false;
  }
  heat_meter     = (DLbus_Data->ByteStream[HMindex.kwh_index + 1] << 8) | DLbus_Data->ByteStream[HMindex.kwh_index];
  heat_meter_mwh = (heat_meter * 0.1f) / 1000.0f; // in MWh

  if (heat_meter_mwh > 1.0f) {
    // in kWh
    heat_meter      = heat_meter_mwh;
    heat_meter_mwh -= heat_meter;
  }

  // MWh
  heat_meter      = (DLbus_Data->ByteStream[HMindex.mwh_index + 1] << 8) | DLbus_Data->ByteStream[HMindex.mwh_index];
  ReadData->value = heat_meter_mwh + heat_meter;
  return true;
}

#endif // ifdef USES_P092