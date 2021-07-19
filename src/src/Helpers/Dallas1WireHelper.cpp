#include "Dallas1WireHelper.h"

#include "../../_Plugin_Helper.h"
#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Misc.h"

#include "../WebServer/JSON.h"

#include "../../ESPEasy_common.h"

#if defined(ESP32)
  # define ESP32noInterrupts() { portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; portENTER_CRITICAL(&mux)
  # define ESP32interrupts() portEXIT_CRITICAL(&mux); }
#endif // if defined(ESP32)


unsigned char ROM_NO[8];
uint8_t LastDiscrepancy;
uint8_t LastFamilyDiscrepancy;
uint8_t LastDeviceFlag;

long usec_release   = 0;
long presence_start = 0;
long presence_end   = 0;


// References to 1-wire family codes:
// http://owfs.sourceforge.net/simple_family.html
// https://github.com/owfs/owfs-doc/wiki/1Wire-Device-List
String Dallas_getModel(uint8_t family) {
  String model;

  switch (family) {
    case 0x28: model = F("DS18B20"); break;
    case 0x3b: model = F("DS1825");  break;
    case 0x22: model = F("DS1822");  break;
    case 0x10: model = F("DS1820 / DS18S20");  break;
    case 0x1D: model = F("DS2423");  break; // 4k RAM with counter
    case 0x01: model = F("DS1990A"); break; // Serial Number iButton
  }
  return model;
}

String Dallas_format_address(const uint8_t addr[]) {
  String result;

  result.reserve(40);

  for (uint8_t j = 0; j < 8; j++)
  {
    if (addr[j] < 0x10) {
      result += '0';
    }
    result += String(addr[j], HEX);

    if (j < 7) { result += '-'; }
  }
  result += F(" [");
  result += Dallas_getModel(addr[0]);
  result += ']';

  return result;
}

uint64_t Dallas_addr_to_uint64(const uint8_t addr[]) {
  uint64_t tmpAddr_64 = 0;

  for (uint8_t i = 0; i < 8; ++i) {
    tmpAddr_64 *= 256;
    tmpAddr_64 += addr[i];
  }
  return tmpAddr_64;
}

void Dallas_uint64_to_addr(uint64_t value, uint8_t addr[]) {
  uint8_t i = 8;

  while (i > 0) {
    --i;
    addr[i] = static_cast<uint8_t>(value & 0xFF);
    value  /= 256;
  }
}

void Dallas_addr_selector_webform_load(taskIndex_t TaskIndex, int8_t gpio_pin_rx, int8_t gpio_pin_tx, uint8_t nrVariables) {
  if (gpio_pin_rx == -1 || gpio_pin_tx == -1) {
    return;
  }

  if (nrVariables >= VARS_PER_TASK) {
    nrVariables = VARS_PER_TASK;
  }

  if (!validTaskIndex(TaskIndex)) {
    return;
  }

  std::map<uint64_t, String> addr_task_map;

  for (taskIndex_t task = 0; validTaskIndex(task); ++task) {
    if (Dallas_plugin(Settings.TaskDeviceNumber[task])) {
      uint8_t tmpAddress[8] = { 0 };

      for (uint8_t var_index = 0; var_index < VARS_PER_TASK; ++var_index) {
        Dallas_plugin_get_addr(tmpAddress, task, var_index);
        uint64_t tmpAddr_64 = Dallas_addr_to_uint64(tmpAddress);

        if (tmpAddr_64 != 0) {
          String label;
          label.reserve(32);
          label  = F(" (task ");
          label += String(task + 1);
          label += F(" [");
          label += getTaskDeviceName(task);
          label += '#';
          label += ExtraTaskSettings.TaskDeviceValueNames[var_index];
          label += F("])");

          addr_task_map[tmpAddr_64] = label;
        }
      }
    }
  }

  // The Shelly 1 temp. addon uses separate
  // input and output pins, and therefor
  // doesn't switch between input and output
  // when running.
  if(gpio_pin_rx != gpio_pin_tx) {
    pinMode(gpio_pin_rx, INPUT);
    pinMode(gpio_pin_tx, OUTPUT);
  }

  // find all suitable devices
  std::vector<uint64_t> scan_res;

  Dallas_reset(gpio_pin_rx, gpio_pin_tx);
  Dallas_reset_search();
  uint8_t tmpAddress[8];

  while (Dallas_search(tmpAddress, gpio_pin_rx, gpio_pin_tx))
  {
    scan_res.push_back(Dallas_addr_to_uint64(tmpAddress));
  }

  for (uint8_t var_index = 0; var_index < nrVariables; ++var_index) {
    String id = F("dallas_addr");
    id += String(var_index);
    String rowLabel = F("Device Address");

    if (nrVariables > 1) {
      rowLabel += ' ';
      rowLabel += String(var_index + 1);
    }
    addRowLabel(rowLabel);
    addSelector_Head(id);
    addSelector_Item(F("- None -"), -1, false); // Empty choice
    uint8_t tmpAddress[8];

    // get currently saved address
    uint8_t savedAddress[8];

    for (uint8_t index = 0; index < scan_res.size(); ++index) {
      Dallas_plugin_get_addr(savedAddress, TaskIndex, var_index);
      Dallas_uint64_to_addr(scan_res[index], tmpAddress);
      String option = Dallas_format_address(tmpAddress);
      auto   it     = addr_task_map.find(Dallas_addr_to_uint64(tmpAddress));

      if (it != addr_task_map.end()) {
        option += it->second;
      }

      bool selected = (memcmp(tmpAddress, savedAddress, 8) == 0) ? true : false;
      addSelector_Item(option, index, selected);
    }
    addSelector_Foot();
  }
}

void Dallas_show_sensor_stats_webform_load(const Dallas_SensorData& sensor_data)
{
  if (sensor_data.addr == 0) {
    return;
  }
  addRowLabel(F("Address"));
  addHtml(sensor_data.get_formatted_address());

  addRowLabel(F("Resolution"));
  addHtmlInt(sensor_data.actual_res);

  addRowLabel(F("Parasite Powered"));
  addHtml(jsonBool(sensor_data.parasitePowered));

  addRowLabel(F("Samples Read Success"));
  addHtmlInt(sensor_data.read_success);

  addRowLabel(F("Samples Read Failed"));
  addHtmlInt(sensor_data.read_failed);
}

void Dallas_addr_selector_webform_save(taskIndex_t TaskIndex, int8_t gpio_pin_rx, int8_t gpio_pin_tx, uint8_t nrVariables)
{
  if (gpio_pin_rx == -1) {
    return;
  }
  if (gpio_pin_tx == -1) {
    return;
  }

  if (nrVariables >= VARS_PER_TASK) {
    nrVariables = VARS_PER_TASK;
  }

  if (!validTaskIndex(TaskIndex)) {
    return;
  }
  uint8_t addr[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  for (uint8_t var_index = 0; var_index < nrVariables; ++var_index) {
    String id = F("dallas_addr");
    id += String(var_index);
    int selection = getFormItemInt(id, -1);

    if (selection != -1) {
      Dallas_scan(getFormItemInt(id), addr, gpio_pin_rx, gpio_pin_tx);
      Dallas_plugin_set_addr(addr, TaskIndex, var_index);
    }
  }
}

bool Dallas_plugin(pluginID_t pluginID)
{
  switch (pluginID) {
    case 4:
    case 80:
    case 100:
      return true;
  }
  return false;
}

void Dallas_plugin_get_addr(uint8_t addr[], taskIndex_t TaskIndex, uint8_t var_index)
{
  if (var_index >= 4) {
    return;
  }

  // Load ROM address from tasksettings
  LoadTaskSettings(TaskIndex);

  for (uint8_t x = 0; x < 8; x++) {
    uint32_t value = (uint32_t)ExtraTaskSettings.TaskDevicePluginConfigLong[x];
    addr[x] = static_cast<uint8_t>((value >> (var_index * 8)) & 0xFF);
  }
}

void Dallas_plugin_set_addr(uint8_t addr[], taskIndex_t TaskIndex, uint8_t var_index)
{
  if (var_index >= 4) {
    return;
  }
  LoadTaskSettings(TaskIndex);
  const uint32_t mask = ~(0xFF << (var_index * 8));

  for (uint8_t x = 0; x < 8; x++) {
    uint32_t value = (uint32_t)ExtraTaskSettings.TaskDevicePluginConfigLong[x];
    value                                          &= mask;
    value                                          += (static_cast<uint32_t>(addr[x]) << (var_index * 8));
    ExtraTaskSettings.TaskDevicePluginConfigLong[x] = (long)value;
  }
}

/*********************************************************************************************\
   Dallas Scan bus
\*********************************************************************************************/
uint8_t Dallas_scan(uint8_t getDeviceROM, uint8_t *ROM, int8_t gpio_pin_rx, int8_t gpio_pin_tx)
{
  uint8_t tmpaddr[8];
  uint8_t devCount = 0;

  Dallas_reset(gpio_pin_rx, gpio_pin_tx);

  Dallas_reset_search();

  while (Dallas_search(tmpaddr, gpio_pin_rx, gpio_pin_tx))
  {
    if (getDeviceROM == devCount) {
      for (uint8_t  i = 0; i < 8; i++) {
        ROM[i] = tmpaddr[i];
      }
    }
    devCount++;
  }
  return devCount;
}

// read power supply
bool Dallas_is_parasite(const uint8_t ROM[8], int8_t gpio_pin_rx, int8_t gpio_pin_tx)
{
  if (!Dallas_address_ROM(ROM, gpio_pin_rx, gpio_pin_tx)) {
    return false;
  }
  Dallas_write(0xB4, gpio_pin_rx, gpio_pin_tx); // read power supply
  return !Dallas_read_bit(gpio_pin_rx, gpio_pin_tx);
}

void Dallas_startConversion(const uint8_t ROM[8], int8_t gpio_pin_rx, int8_t gpio_pin_tx)
{
  Dallas_reset(gpio_pin_rx, gpio_pin_tx);
  Dallas_write(0x55, gpio_pin_rx, gpio_pin_tx); // Choose ROM

  for (uint8_t i = 0; i < 8; i++) {
    Dallas_write(ROM[i], gpio_pin_rx, gpio_pin_tx);
  }
  Dallas_write(0x44, gpio_pin_rx, gpio_pin_tx);
}

/*********************************************************************************************\
*  Dallas Read temperature from scratchpad
\*********************************************************************************************/
bool Dallas_readTemp(const uint8_t ROM[8], float *value, int8_t gpio_pin_rx, int8_t gpio_pin_tx)
{
  int16_t DSTemp;
  uint8_t    ScratchPad[12];

  if (!Dallas_address_ROM(ROM, gpio_pin_rx, gpio_pin_tx)) {
    return false;
  }
  Dallas_write(0xBE, gpio_pin_rx, gpio_pin_tx);  // Read scratchpad

  for (uint8_t i = 0; i < 9; i++) { // read 9 bytes
    ScratchPad[i] = Dallas_read(gpio_pin_rx, gpio_pin_tx);
  }

  bool crc_ok = Dallas_crc8(ScratchPad);

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("DS: SP: ");

    for (uint8_t x = 0; x < 9; x++)
    {
      if (x != 0) {
        log += ',';
      }
      log += String(ScratchPad[x], HEX);
    }

    if (crc_ok) {
      log += F(",OK");
    }

    if (Dallas_is_parasite(ROM, gpio_pin_rx, gpio_pin_tx)) {
      log += F(",P");
    }
    log += ',';
    log += String(usec_release, DEC);
    log += ',';
    log += String(presence_start, DEC);
    log += ',';
    log += String(presence_end, DEC);
    addLog(LOG_LEVEL_DEBUG, log);
  }

  if (!crc_ok)
  {
    *value = 0;
    return false;
  }

  if ((ROM[0] == 0x28) || (ROM[0] == 0x3b) || (ROM[0] == 0x22)) // DS18B20 or DS1825 or DS1822
  {
    DSTemp = (ScratchPad[1] << 8) + ScratchPad[0];

    if (DSTemp == 0x550) { // power-on reset value
      return false;
    }
    *value = (float(DSTemp) * 0.0625);
  }
  else if (ROM[0] == 0x10)       // DS1820 DS18S20
  {
    if (ScratchPad[0] == 0xaa) { // power-on reset value
      return false;
    }
    DSTemp = (ScratchPad[1] << 11) | ScratchPad[0] << 3;
    DSTemp = ((DSTemp & 0xfff0) << 3) - 16 +
             (((ScratchPad[7] - ScratchPad[6]) << 7) / ScratchPad[7]);
    *value = float(DSTemp) * 0.0078125;
  }
  return true;
}

bool Dallas_readiButton(const uint8_t addr[8], int8_t gpio_pin_rx, int8_t gpio_pin_tx)
{
  // maybe this is needed to trigger the reading
  //    uint8_t ScratchPad[12];

  Dallas_reset(gpio_pin_rx, gpio_pin_tx);
  Dallas_write(0x55, gpio_pin_rx, gpio_pin_tx); // Choose ROM

  for (uint8_t i = 0; i < 8; i++) {
    Dallas_write(addr[i], gpio_pin_rx, gpio_pin_tx);
  }

  Dallas_write(0xBE, gpio_pin_rx, gpio_pin_tx); // Read scratchpad

  //    for (uint8_t i = 0; i < 9; i++) // read 9 bytes
  //        ScratchPad[i] = Dallas_read();
  // end maybe this is needed to trigger the reading

  uint8_t tmpaddr[8];
  bool found = false;

  Dallas_reset(gpio_pin_rx, gpio_pin_tx);
  String log;

  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    log  = F("DS   : iButton searching for address: ");
    log += Dallas_format_address(addr);
    log += F(" found: ");
  }
  Dallas_reset_search();

  while (Dallas_search(tmpaddr, gpio_pin_rx, gpio_pin_tx))
  {
    if (loglevelActiveFor(LOG_LEVEL_INFO)) {
      log += Dallas_format_address(tmpaddr);
      log += ',';
    }

    if (memcmp(addr, tmpaddr, 8) == 0)
    {
      if (loglevelActiveFor(LOG_LEVEL_INFO)) {
        log += F("Success. Button was found");
      }
      found = true;
    }
  }
  addLog(LOG_LEVEL_INFO, log);
  return found;
}

/*********************************************************************************************\
   Dallas read DS2423 counter
   Taken from https://github.com/jbechter/arduino-onewire-DS2423
\*********************************************************************************************/
#define DS2423_READ_MEMORY_COMMAND 0xa5
#define DS2423_PAGE_ONE 0xc0
#define DS2423_PAGE_TWO 0xe0

bool Dallas_readCounter(const uint8_t ROM[8], float *value, int8_t gpio_pin_rx, int8_t gpio_pin_tx, uint8_t counter)
{
  uint8_t data[45];

  data[0] = DS2423_READ_MEMORY_COMMAND;
  data[1] = (counter == 0 ? DS2423_PAGE_ONE : DS2423_PAGE_TWO);
  data[2] = 0x01;

  if (!Dallas_address_ROM(ROM, gpio_pin_rx, gpio_pin_tx)) {
    return false;
  }

  Dallas_write(data[0], gpio_pin_rx, gpio_pin_tx);
  Dallas_write(data[1], gpio_pin_rx, gpio_pin_tx);
  Dallas_write(data[2], gpio_pin_rx, gpio_pin_tx);

  for (int j = 3; j < 45; j++) {
    data[j] = Dallas_read(gpio_pin_rx, gpio_pin_tx);
  }

  Dallas_reset(gpio_pin_rx, gpio_pin_tx);

  uint32_t count = (uint32_t)data[38];

  for (int j = 37; j >= 35; j--) {
    count = (count << 8) + (uint32_t)data[j];
  }

  uint16_t crc      = Dallas_crc16(data, 43, 0);
  uint8_t *crcBytes = (uint8_t *)&crc;
  uint8_t  crcLo    = ~data[43];
  uint8_t  crcHi    = ~data[44];
  bool     error    = (crcLo != crcBytes[0]) || (crcHi != crcBytes[1]);

  if (!error)
  {
    *value = count;
    return true;
  }
  else
  {
    *value = 0;
    return false;
  }
}

/*********************************************************************************************\
* Dallas Get Resolution
\*********************************************************************************************/
uint8_t Dallas_getResolution(const uint8_t ROM[8], int8_t gpio_pin_rx, int8_t gpio_pin_tx)
{
  // DS1820 and DS18S20 have no resolution configuration register
  if (ROM[0] == 0x10) { return 12; }

  uint8_t ScratchPad[12];

  if (!Dallas_address_ROM(ROM, gpio_pin_rx, gpio_pin_tx)) {
    return 0;
  }
  Dallas_write(0xBE, gpio_pin_rx, gpio_pin_tx);  // Read scratchpad

  for (uint8_t i = 0; i < 9; i++) { // read 9 bytes
    ScratchPad[i] = Dallas_read(gpio_pin_rx, gpio_pin_tx);
  }

  if (!Dallas_crc8(ScratchPad)) {
    return 0;
  }
  else
  {
    switch (ScratchPad[4])
    {
      case 0x7F: // 12 bit
        return 12;

      case 0x5F: // 11 bit
        return 11;

      case 0x3F: // 10 bit
        return 10;

      case 0x1F: //  9 bit
      default:
        return 9;
    }
  }
  return 0;
}

/*********************************************************************************************\
* Dallas Get Resolution
\*********************************************************************************************/
bool Dallas_setResolution(const uint8_t ROM[8], uint8_t res, int8_t gpio_pin_rx, int8_t gpio_pin_tx)
{
  // DS1820 and DS18S20 have no resolution configuration register
  if (ROM[0] == 0x10) { return true; }

  uint8_t ScratchPad[12];

  if (!Dallas_address_ROM(ROM, gpio_pin_rx, gpio_pin_tx)) {
    return false;
  }
  Dallas_write(0xBE, gpio_pin_rx, gpio_pin_tx);  // Read scratchpad

  for (uint8_t i = 0; i < 9; i++) { // read 9 bytes
    ScratchPad[i] = Dallas_read(gpio_pin_rx, gpio_pin_tx);
  }

  if (!Dallas_crc8(ScratchPad)) {
    addLog(LOG_LEVEL_ERROR, F("DS   : Cannot set resolution"));
    return false;
  }
  else
  {
    uint8_t old_configuration = ScratchPad[4];

    switch (res)
    {
      case 12:
        ScratchPad[4] = 0x7F; // 12 bits
        break;
      case 11:
        ScratchPad[4] = 0x5F; // 11 bits
        break;
      case 10:
        ScratchPad[4] = 0x3F; // 10 bits
        break;
      case 9:
      default:
        ScratchPad[4] = 0x1F; //  9 bits
        break;
    }

    if (ScratchPad[4] == old_configuration) {
      return true;
    }

    Dallas_address_ROM(ROM, gpio_pin_rx, gpio_pin_tx);
    Dallas_write(0x4E,          gpio_pin_rx, gpio_pin_tx); // Write to EEPROM
    Dallas_write(ScratchPad[2], gpio_pin_rx, gpio_pin_tx); // high alarm temp
    Dallas_write(ScratchPad[3], gpio_pin_rx, gpio_pin_tx); // low alarm temp
    Dallas_write(ScratchPad[4], gpio_pin_rx, gpio_pin_tx); // configuration register

    Dallas_address_ROM(ROM, gpio_pin_rx, gpio_pin_tx);

    // save the newly written values to eeprom
    Dallas_write(0x48, gpio_pin_rx, gpio_pin_tx);
    delay(100);  // <--- added 20ms delay to allow 10ms long EEPROM write operation (as specified by datasheet)
    Dallas_reset(gpio_pin_rx, gpio_pin_tx);

    return true; // new value set
  }
}

/*********************************************************************************************\
*  Dallas Reset
\*********************************************************************************************/
uint8_t Dallas_reset(int8_t gpio_pin_rx, int8_t gpio_pin_tx)
{
  uint8_t retries = 125;

    #if defined(ESP32)
  ESP32noInterrupts();
    #endif // if defined(ESP32)
  if(gpio_pin_rx == gpio_pin_tx) {
    pinMode(gpio_pin_rx, INPUT);
  } else {
    digitalWrite(gpio_pin_tx, HIGH);
  }
  bool success = true;

  do // wait until the wire is high... just in case
  {
    if (--retries == 0) {
      success = false;
    }
    delayMicroseconds(2);
  }
  while (!digitalRead(gpio_pin_rx) && success);

  usec_release   = 0;
  presence_start = 0;
  presence_end   = 0;

  if (success) {
    // The master starts a transmission with a reset pulse,
    // which pulls the wire to 0 volts for at least 480 µs.
    // This resets every slave device on the bus.
    digitalWrite(gpio_pin_tx, LOW);
    if(gpio_pin_rx == gpio_pin_tx) {
      pinMode(gpio_pin_rx, OUTPUT);
    }
    delayMicroseconds(480);
    if(gpio_pin_rx == gpio_pin_tx) {
      pinMode(gpio_pin_rx, INPUT);
    } else {
      digitalWrite(gpio_pin_tx, HIGH);
    }

    // After that, any slave device, if present, shows that it exists with a "presence" pulse:
    // it holds the bus low for at least 60 µs after the master releases the bus.
    // This may take about 30 usec after release for present sensors to pull the line low.
    // Sequence:
    // - Release => pin high
    // - Presemce condition start (typ: 30 usec after release)
    // - Presence condition end   (minimal duration 60 usec, typ: 100 usec)
    // - Wait till 480 usec after release.
    const unsigned long start = micros();
    long usec_passed          = 0;

    while (usec_passed < 480) {
      usec_passed = usecPassedSince(start);

      const bool pin_state = !!digitalRead(gpio_pin_rx);

      if (usec_release == 0) {
        if (pin_state) {
          // Pin has been released
          usec_release = usec_passed;
        }
      } else if (presence_start == 0) {
        if (!pin_state) {
          // Presence condition started
          presence_start = usec_passed;
        }
      } else if (presence_end == 0) {
        if (pin_state) {
          // Presence condition ended
          presence_end = usec_passed;
        }
      }
      delayMicroseconds(2);
    }
  }
    #if defined(ESP32)
  ESP32interrupts();
    #endif // if defined(ESP32)

  if (presence_end != 0) {
    const long presence_duration = presence_end - presence_start;

    if (presence_duration > 60) { return 1; }
  }
  return 0;
}

#define FALSE 0
#define TRUE  1

/*********************************************************************************************\
*  Dallas Reset Search
\*********************************************************************************************/
void Dallas_reset_search()
{
  // reset the search state
  LastDiscrepancy       = 0;
  LastDeviceFlag        = FALSE;
  LastFamilyDiscrepancy = 0;

  for (uint8_t i = 0; i < 8; i++) {
    ROM_NO[i] = 0;
  }
}

/*********************************************************************************************\
*  Dallas Search bus
\*********************************************************************************************/
uint8_t Dallas_search(uint8_t *newAddr, int8_t gpio_pin_rx, int8_t gpio_pin_tx)
{
  uint8_t id_bit_number;
  uint8_t last_zero, rom_byte_number, search_result;
  uint8_t id_bit, cmp_id_bit;
  unsigned char rom_byte_mask, search_direction;

  // initialize for search
  id_bit_number   = 1;
  last_zero       = 0;
  rom_byte_number = 0;
  rom_byte_mask   = 1;
  search_result   = 0;

  // if the last call was not the last one
  if (!LastDeviceFlag)
  {
    // 1-Wire reset
    if (!Dallas_reset(gpio_pin_rx, gpio_pin_tx))
    {
      // reset the search
      LastDiscrepancy       = 0;
      LastDeviceFlag        = FALSE;
      LastFamilyDiscrepancy = 0;
      return FALSE;
    }

    // issue the search command
    Dallas_write(0xF0, gpio_pin_rx, gpio_pin_tx);

    // loop to do the search
    do
    {
      // read a bit and its complement
      id_bit     = Dallas_read_bit(gpio_pin_rx, gpio_pin_tx);
      cmp_id_bit = Dallas_read_bit(gpio_pin_rx, gpio_pin_tx);

      // check for no devices on 1-wire
      if ((id_bit == 1) && (cmp_id_bit == 1)) {
        break;
      }
      else
      {
        // all devices coupled have 0 or 1
        if (id_bit != cmp_id_bit) {
          search_direction = id_bit; // bit write value for search
        }
        else
        {
          // if this discrepancy if before the Last Discrepancy
          // on a previous next then pick the same as last time
          if (id_bit_number < LastDiscrepancy) {
            search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
          }
          else {
            // if equal to last pick 1, if not then pick 0
            search_direction = (id_bit_number == LastDiscrepancy);
          }

          // if 0 was picked then record its position in LastZero
          if (search_direction == 0)
          {
            last_zero = id_bit_number;

            // check for Last discrepancy in family
            if (last_zero < 9) {
              LastFamilyDiscrepancy = last_zero;
            }
          }
        }

        // set or clear the bit in the ROM byte rom_byte_number
        // with mask rom_byte_mask
        if (search_direction == 1) {
          ROM_NO[rom_byte_number] |= rom_byte_mask;
        }
        else {
          ROM_NO[rom_byte_number] &= ~rom_byte_mask;
        }

        // serial number search direction write bit
        Dallas_write_bit(search_direction, gpio_pin_rx, gpio_pin_tx);

        // increment the byte counter id_bit_number
        // and shift the mask rom_byte_mask
        id_bit_number++;
        rom_byte_mask <<= 1;

        // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
        if (rom_byte_mask == 0)
        {
          rom_byte_number++;
          rom_byte_mask = 1;
        }
      }
    }
    while (rom_byte_number < 8); // loop until through all ROM bytes 0-7

    // if the search was successful then
    if (!(id_bit_number < 65))
    {
      // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
      LastDiscrepancy = last_zero;

      // check for last device
      if (LastDiscrepancy == 0) {
        LastDeviceFlag = TRUE;
      }

      search_result = TRUE;
    }
  }

  // if no device found then reset counters so next 'search' will be like a first
  if (!search_result || !ROM_NO[0])
  {
    LastDiscrepancy       = 0;
    LastDeviceFlag        = FALSE;
    LastFamilyDiscrepancy = 0;
    search_result         = FALSE;
  }

  for (int i = 0; i < 8; i++) {
    newAddr[i] = ROM_NO[i];
  }

  return search_result;
}

#undef FALSE
#undef TRUE

/*********************************************************************************************\
*  Dallas Read byte
\*********************************************************************************************/
uint8_t Dallas_read(int8_t gpio_pin_rx, int8_t gpio_pin_tx)
{
  uint8_t bitMask;
  uint8_t r = 0;

  for (bitMask = 0x01; bitMask; bitMask <<= 1) {
    if (Dallas_read_bit(gpio_pin_rx, gpio_pin_tx)) {
      r |= bitMask;
    }
  }

  return r;
}

/*********************************************************************************************\
*  Dallas Write byte
\*********************************************************************************************/
void Dallas_write(uint8_t ByteToWrite, int8_t gpio_pin_rx, int8_t gpio_pin_tx)
{
  uint8_t bitMask;

  for (bitMask = 0x01; bitMask; bitMask <<= 1) {
    Dallas_write_bit((bitMask & ByteToWrite) ? 1 : 0, gpio_pin_rx, gpio_pin_tx);
  }
}

/*********************************************************************************************\
*  Dallas Read bit
\*********************************************************************************************/
uint8_t Dallas_read_bit(int8_t gpio_pin_rx, int8_t gpio_pin_tx)
{
  if (gpio_pin_rx == -1) { return 0; }
  if (gpio_pin_tx == -1) { return 0; }
  unsigned long start = micros();
  uint8_t r = Dallas_read_bit_ISR(gpio_pin_rx, gpio_pin_tx, start);

  while (usecPassedSince(start) < 70) {
    // Wait for another 55 usec
    // Complete read cycle:
    // LOW: 6 usec
    // Float: 9 msec
    // Read value. Typically the sensor keeps the level low for 27 usec.
    // Wait for 55 usec => complete cycle = 6 + 9 + 55 = 70 usec.
  }
  return r;
}

uint8_t IRAM_ATTR Dallas_read_bit_ISR(int8_t gpio_pin_rx, int8_t gpio_pin_tx, unsigned long start)
{
  uint8_t r;
  {
      #if defined(ESP32)
    ESP32noInterrupts();
      #endif // if defined(ESP32)
    digitalWrite(gpio_pin_tx, LOW);
    if(gpio_pin_rx == gpio_pin_tx) {
      pinMode(gpio_pin_rx, OUTPUT);
    }

    while (usecPassedSince(start) < 6) {
      // Wait for 6 usec
    }
    unsigned long startwait = micros();
    if(gpio_pin_rx == gpio_pin_tx) {
      pinMode(gpio_pin_rx, INPUT); // let pin float, pull up will raise
    } else {
      digitalWrite(gpio_pin_tx, HIGH);
    }

    while (usecPassedSince(startwait) < 9) {
      // Wait for another 9 usec
    }
    r = digitalRead(gpio_pin_rx);

      #if defined(ESP32)
    ESP32interrupts();
      #endif // if defined(ESP32)
  }
  return r;
}


/*********************************************************************************************\
*  Dallas Write bit
\*********************************************************************************************/
void Dallas_write_bit(uint8_t v, int8_t gpio_pin_rx, int8_t gpio_pin_tx)
{
  if (gpio_pin_tx == -1) { return; }

  // Determine times in usec for high and low
  // write 1: low 6 usec, high 64 usec
  // write 0: low 60 usec, high 10 usec
  const long low_time  = (v & 1) ? 6 : 60;
  const long high_time = (v & 1) ? 64 : 10;
  unsigned long start  = micros();

  Dallas_write_bit_ISR(v, gpio_pin_rx, gpio_pin_tx, low_time, high_time, start);

  while (usecPassedSince(start) < high_time) {
    // output remains high
  }
}

void IRAM_ATTR Dallas_write_bit_ISR(uint8_t v,
                      int8_t  gpio_pin_rx,
                      int8_t  gpio_pin_tx,
                      long low_time,
                      long high_time,
                      unsigned long &start)
{
  #if defined(ESP32)
  ESP32noInterrupts();
  #endif // if defined(ESP32)
  digitalWrite(gpio_pin_tx, LOW);
  if(gpio_pin_rx == gpio_pin_tx) {
    pinMode(gpio_pin_rx, OUTPUT);
  }

  while (usecPassedSince(start) < low_time) {
    // output remains low
  }
  start = micros();
  digitalWrite(gpio_pin_tx, HIGH);
  #if defined(ESP32)
  ESP32interrupts();
  #endif // if defined(ESP32)
}

/*********************************************************************************************\
*  Standard function to initiate addressing a sensor.
\*********************************************************************************************/
bool Dallas_address_ROM(const uint8_t ROM[8], int8_t gpio_pin_rx, int8_t gpio_pin_tx)
{
  if (!Dallas_reset(gpio_pin_rx, gpio_pin_tx)) { return false; }
  Dallas_write(0x55, gpio_pin_rx, gpio_pin_tx); // Choose ROM

  for (uint8_t i = 0; i < 8; i++) {
    Dallas_write(ROM[i], gpio_pin_rx, gpio_pin_tx);
  }
  return true;
}

/*********************************************************************************************\
*  Dallas Calculate CRC8 and compare it of addr[0-7] and compares it to addr[8]
\*********************************************************************************************/
bool Dallas_crc8(const uint8_t *addr)
{
  uint8_t crc = 0;
  uint8_t len = 8;

  while (len--)
  {
    uint8_t inbyte = *addr++; // from 0 to 7

    for (uint8_t i = 8; i; i--)
    {
      uint8_t mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;

      if (mix) { crc ^= 0x8C; }
      inbyte >>= 1;
    }
  }
  return crc == *addr; // addr 8
}

/*********************************************************************************************\
*  Dallas Calculate CRC16
\*********************************************************************************************/
uint16_t Dallas_crc16(const uint8_t *input, uint16_t len, uint16_t crc)
{
  static const uint8_t oddparity[16] =
  { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };

  for (uint16_t i = 0; i < len; i++) {
    // Even though we're just copying a byte from the input,
    // we'll be doing 16-bit computation with it.
    uint16_t cdata = input[i];
    cdata = (cdata ^ crc) & 0xff;
    crc >>= 8;

    if (oddparity[cdata & 0x0F] ^ oddparity[cdata >> 4]) {
      crc ^= 0xC001;
    }

    cdata <<= 6;
    crc    ^= cdata;
    cdata <<= 1;
    crc    ^= cdata;
  }

  return crc;
}

#if defined(ESP32)
  # undef ESP32noInterrupts
  # undef ESP32interrupts
#endif // if defined(ESP32)


void Dallas_SensorData::set_measurement_inactive() {
  measurementActive = false;
  value             = 0.0f;
  valueRead         = false;
}

bool Dallas_SensorData::initiate_read(int8_t gpio_rx, int8_t gpio_tx, int8_t res) {
  if (addr == 0) { return false; }
  uint8_t tmpaddr[8];

  Dallas_uint64_to_addr(addr, tmpaddr);

  if (lastReadError) {
    if (!check_sensor(gpio_rx, gpio_tx, res)) {
      return false;
    }
    lastReadError = false;
  }

  if (!Dallas_address_ROM(tmpaddr, gpio_rx, gpio_tx)) {
    ++read_failed;
    lastReadError = true;
    return false;
  }
  Dallas_write(0x44, gpio_rx, gpio_tx); // Take temperature measurement
  return true;
}

bool Dallas_SensorData::collect_value(int8_t gpio_rx, int8_t gpio_tx) {
  if ((addr != 0) && measurementActive) {
    uint8_t tmpaddr[8];
    Dallas_uint64_to_addr(addr, tmpaddr);

    if (Dallas_readTemp(tmpaddr, &value, gpio_rx, gpio_tx)) {
      ++read_success;
      lastReadError = false;
      valueRead     = true;
      return true;
    }
    ++read_failed;
    lastReadError = true;
  }
  return false;
}

String Dallas_SensorData::get_formatted_address() const {
  if (addr == 0) { return ""; }

  uint8_t tmpaddr[8];

  Dallas_uint64_to_addr(addr, tmpaddr);
  return Dallas_format_address(tmpaddr);
}

bool Dallas_SensorData::check_sensor(int8_t gpio_rx, int8_t gpio_tx, int8_t res) {
  if (addr == 0) { return false; }
  uint8_t tmpaddr[8];

  Dallas_uint64_to_addr(addr, tmpaddr);

  actual_res = Dallas_getResolution(tmpaddr, gpio_rx, gpio_tx);

  if (actual_res == 0) {
    ++read_failed;
    lastReadError = true;
    return false;
  }

  if (res != actual_res) {
    if (!Dallas_setResolution(tmpaddr, res, gpio_rx, gpio_tx)) {
      return false;
    }
  }

  parasitePowered = Dallas_is_parasite(tmpaddr, gpio_rx, gpio_tx);
  return true;
}
