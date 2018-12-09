#ifdef USES_P004
// #######################################################################################################
// #################################### Plugin 004: TempSensor Dallas DS18B20  ###########################
// #######################################################################################################

// Maxim Integrated (ex Dallas) DS18B20 datasheet : https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf

#if defined(ESP32)
#define ESP32noInterrupts() {portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; portENTER_CRITICAL(&mux)
#define ESP32interrupts() portEXIT_CRITICAL(&mux);}
#endif

#define PLUGIN_004
#define PLUGIN_ID_004         4
#define PLUGIN_NAME_004       "Environment - DS18b20"
#define PLUGIN_VALUENAME1_004 "Temperature"

// OneWire commands
#define STARTCONVO      0x44
#define COPYSCRATCH     0x48
#define WRITESCRATCH    0x4E
#define CHOOSEROM       0x55
#define READSCRATCH     0xBE
#define READPOWERSUPPLY 0xB4
#define RECALLSCRATCH   0xB8
#define SKIPROM         0xCC
#define ALARMSEARCH     0xEC
#define BUSSEARCH       0xF0


// Scratchpad locations
#define TEMP_LSB        0
#define TEMP_MSB        1
#define HIGH_ALARM_TEMP 2
#define LOW_ALARM_TEMP  3
#define CONFIGURATION   4
#define INTERNAL_BYTE   5
#define COUNT_REMAIN    6
#define COUNT_PER_C     7
#define SCRATCHPAD_CRC  8

// Device resolution
#define TEMP_9_BIT  0x1F
#define TEMP_10_BIT 0x3F
#define TEMP_11_BIT 0x5F
#define TEMP_12_BIT 0x7F

// Device ROM
#define ROM_DS1820_DS18S20  0x10
#define ROM_DS1822          0x22
#define ROM_DS18B20         0x28
#define ROM_DS1825          0x3B

// Arrays
#define SP_SIZE 9
#define ADDR_SIZE 8

static boolean Plugin_004_parasite[TASKS_MAX];
static boolean Plugin_004_newValue[TASKS_MAX];
static unsigned long Plugin_004_timeoutGPIO[MAX_GPIO];

boolean Plugin_004(byte function, struct EventStruct * event, String& string) {
    boolean success = false;

    switch (function) {
        case PLUGIN_DEVICE_ADD:
        {
            Device[++deviceCount].Number = PLUGIN_ID_004;
            Device[deviceCount].Type = DEVICE_TYPE_SINGLE;
            Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
            Device[deviceCount].Ports = 0;
            Device[deviceCount].PullUpOption = true;
            Device[deviceCount].InverseLogicOption = false;
            Device[deviceCount].FormulaOption = true;
            Device[deviceCount].ValueCount = 1;
            Device[deviceCount].SendDataOption = true;
            Device[deviceCount].TimerOption = true;
            Device[deviceCount].GlobalSyncOption = true;
            break;
        }

        case PLUGIN_GET_DEVICENAME:
        {
            string = F(PLUGIN_NAME_004);
            break;
        }

        case PLUGIN_GET_DEVICEVALUENAMES:
        {
            strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_004));
            break;
        }

        case PLUGIN_GET_DEVICEGPIONAMES:
        {
            event->String1 = formatGpioName_bidirectional(F("1-Wire"));
            break;
        }

        case PLUGIN_WEBFORM_LOAD:
        {
            uint8_t savedAddress[8];
            byte resolutionChoice = 12; //default to 12 bit
            uint8_t pin = Settings.TaskDevicePin1[event->TaskIndex];
            boolean pullup = Settings.TaskDevicePin1PullUp[event->TaskIndex];
            // Scan the onewire bus and fill dropdown list with devicecount on this GPIO.

            if (pin != -1) {
                if (!timeOutReached(Plugin_004_timeoutGPIO[pin])) {
                    //we cannot reschedule so wait
                    long time2wait = timeDiff(millis(), Plugin_004_timeoutGPIO[pin]);
                    if (time2wait > 0 && time2wait < 750) delay(time2wait);
                }
                // get currently saved address
                for (byte i = 0; i < ADDR_SIZE; i++)
                    savedAddress[i] = ExtraTaskSettings.TaskDevicePluginConfigLong[i];

                // find all suitable devices
                addRowLabel(F("Device Address"));
                addSelector_Head(F("p004_dev"), false);
                addSelector_Item("", -1, false, false, "");
                uint8_t tmpAddress[ADDR_SIZE];
                byte count = 0;
                Plugin_004_DS_reset(pin, pullup);
                Plugin_004_DS_reset_search();
                while (Plugin_004_DS_search(tmpAddress, pin, pullup)) {
                    String option = "";
                    for (byte j = 0; j < ADDR_SIZE; j++) {
                        option += String(tmpAddress[j], HEX);
                        if (j < 7) option += '-';
                    }
                    bool selected = (memcmp(tmpAddress, savedAddress, ADDR_SIZE) == 0) ? true : false;
                    addSelector_Item(option, count, selected, false, "");
                    count++;
                }
                addSelector_Foot();

                // Device Resolution select
                if (ExtraTaskSettings.TaskDevicePluginConfigLong[0] != 0)
                    resolutionChoice = ExtraTaskSettings.TaskDevicePluginConfig[0];

                String resultsOptions[4] = {F("9"), F("10"), F("11"), F("12")};
                int resultsOptionValues[4] = {9, 10, 11, 12};
                addFormSelector(F("Device Resolution"), F("p004_res"), 4, resultsOptions, resultsOptionValues, resolutionChoice);
                addHtml(F(" Bit"));
            }
            success = true;
            break;
        }

        case PLUGIN_WEBFORM_SAVE:
        {
            uint8_t pin = Settings.TaskDevicePin1[event->TaskIndex];
            boolean pullup = Settings.TaskDevicePin1PullUp[event->TaskIndex];
            uint8_t addr[ADDR_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0};

            // save the address for selected device and store into extra tasksettings
            if (pin != -1) {
                if (!timeOutReached(Plugin_004_timeoutGPIO[pin])) {
                    //we cannot reschedule so wait
                    long time2wait = timeDiff(millis(), Plugin_004_timeoutGPIO[pin]);
                    if (time2wait > 0 && time2wait < 750) delay(time2wait);
                }

                Plugin_004_DS_scan(getFormItemInt(F("p004_dev")), addr, pin, pullup);
                for (byte x = 0; x < ADDR_SIZE; x++)
                    ExtraTaskSettings.TaskDevicePluginConfigLong[x] = addr[x];

                //save resolution in espeasy config
                ExtraTaskSettings.TaskDevicePluginConfig[0] = getFormItemInt(F("p004_res"));
                //write resolution to sp only
                Plugin_004_DS_setResolution(addr, ExtraTaskSettings.TaskDevicePluginConfig[0], pin, pullup);

            }
            success = true;
            break;
        }

        case PLUGIN_WEBFORM_SHOW_CONFIG:
        {
            for (byte x = 0; x < ADDR_SIZE; x++) {
                if (x != 0)
                    string += '-';
                string += String(ExtraTaskSettings.TaskDevicePluginConfigLong[x], HEX);
            }
            string += '(';
            string += String(ExtraTaskSettings.TaskDevicePluginConfig[0]);
            if (Plugin_004_parasite[event->TaskIndex]) string += F("P");
            string += ')';
            success = true;
            break;
        }

        case PLUGIN_INIT:
        {
            uint8_t pin = Settings.TaskDevicePin1[event->TaskIndex];
            boolean pullup = Settings.TaskDevicePin1PullUp[event->TaskIndex];

            if (pin != -1) {
                uint8_t addr[ADDR_SIZE];
                Plugin_004_get_addr(addr, event->TaskIndex);
                Plugin_004_parasite[event->TaskIndex] = Plugin_004_isParasite(addr, pin, pullup);
                Plugin_004_DS_setResolution(addr, ExtraTaskSettings.TaskDevicePluginConfig[0], pin, pullup);
                Plugin_004_newValue[event->TaskIndex] = false;
                Plugin_004_timeoutGPIO[pin] = millis();
                String log = F("DS   : Init: ");
                for (byte x = 0; x < ADDR_SIZE; x++) {
                    if (x != 0)
                        log += '-';
                    log += String(ExtraTaskSettings.TaskDevicePluginConfigLong[x], HEX);
                }
                if (Plugin_004_parasite[event->TaskIndex]) log += F(" P");
                addLog(LOG_LEVEL_INFO, log);
            }
            success = true;
            break;
        }

        case PLUGIN_READ:
        {
            if (ExtraTaskSettings.TaskDevicePluginConfigLong[0] != 0) {
                uint8_t pin = Settings.TaskDevicePin1[event->TaskIndex];
                boolean pullup = Settings.TaskDevicePin1PullUp[event->TaskIndex];
                String log = F("DS   : ");
                uint8_t addr[ADDR_SIZE];
                Plugin_004_get_addr(addr, event->TaskIndex);
                if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
                    log += (" (");
                    for (byte x = 0; x < ADDR_SIZE; x++) {
                        if (x != 0)
                            log += '-';
                        log += String(ExtraTaskSettings.TaskDevicePluginConfigLong[x], HEX);
                    }
                    log += (") ");
                }
                if (timeOutReached(Plugin_004_timeoutGPIO[pin])) {
                    log += F("->gpio ");
                    log += String(pin);
                    log += F(" ready ");
                    if (Plugin_004_newValue[event->TaskIndex]) {
                        log += F("->new value task ");
                        log += String(event->TaskIndex);
                        log += F(" ->Temp: ");
                        Plugin_004_newValue[event->TaskIndex] = false;
                        float value = 0;
                        if (Plugin_004_DS_readTemp(addr, &value, pin, pullup)) {
                            UserVar[event->BaseVarIndex] = value;
                            log += UserVar[event->BaseVarIndex];
                            success = true;
                        } else {
                            UserVar[event->BaseVarIndex] = NAN;
                            log += F("Error!");
                        }


                    } else {
                        log += F("->startConversion ");
                        if (Plugin_004_DS_startConversion(addr, pin, pullup, Plugin_004_parasite[event->TaskIndex], ExtraTaskSettings.TaskDevicePluginConfig[0])) {
                            Plugin_004_newValue[event->TaskIndex] = true;
                            log += F("->reschedule read task ");
                            log += String(event->TaskIndex);
                            //GPIO in use -> reschedule after timeOutReached
                            schedule_task_device_timer(event->TaskIndex, Plugin_004_timeoutGPIO[pin]);
                        }
                    }
                } else {
                    log += F("->gpio ");
                    log += String(pin);
                    log += (" busy ->reschedule task ");
                    log += String(event->TaskIndex);
                    //GPIO in use -> reschedule after timeOutReached
                    schedule_task_device_timer(event->TaskIndex, Plugin_004_timeoutGPIO[pin]);
                }
                addLog(LOG_LEVEL_INFO, log);
            }
            break;
        }
    }
    return success;
}

void Plugin_004_toString(const uint8_t * ROM, String & string) {
    for (uint8_t i = 0; i < ADDR_SIZE; i++) {
        if (i != 0)
            string += '-';
        string += String(ROM[i], HEX);
    }
}

void Plugin_004_get_addr(uint8_t addr[], byte taskIndex) {
    // Load ROM address from tasksettings
    LoadTaskSettings(taskIndex);
    for (byte x = 0; x < ADDR_SIZE; x++)
        addr[x] = ExtraTaskSettings.TaskDevicePluginConfigLong[x];
}

//
// Do a ROM select
//

void Plugin_004_ROM_select(const uint8_t * ROM, uint8_t pin) {
    Plugin_004_DS_write(CHOOSEROM, pin);
    for (uint8_t i = 0; i < ADDR_SIZE; i++) Plugin_004_DS_write(ROM[i], pin);
}

// check for parasite power

bool Plugin_004_isParasite(const uint8_t * ROM, uint8_t pin, boolean pullup) {
    if (Plugin_004_DS_reset(pin, pullup)) {
        Plugin_004_ROM_select(ROM, pin);
        Plugin_004_DS_write(READPOWERSUPPLY, pin);
    } else {
        String log = F("DS   : isParasite: reset fail ");
        Plugin_004_toString(ROM, log);
        addLog(LOG_LEVEL_ERROR, log);
    }
    return !Plugin_004_DS_read_bit(pin, pullup);
}

/*********************************************************************************************\
   Dallas Scan bus
\*********************************************************************************************/
byte Plugin_004_DS_scan(byte getDeviceROM, uint8_t* ROM, uint8_t pin, boolean pullup) {
    byte tmpaddr[ADDR_SIZE];
    byte devCount = 0;
    if (Plugin_004_DS_reset(pin, pullup)) {
        Plugin_004_DS_reset_search();
        while (Plugin_004_DS_search(tmpaddr, pin, pullup)) {
            if (getDeviceROM == devCount)
                for (byte i = 0; i < ADDR_SIZE; i++)
                    ROM[i] = tmpaddr[i];
            devCount++;
        }
    } else {
        String log = F("DS   : scan: reset fail ");
        Plugin_004_toString(ROM, log);
        addLog(LOG_LEVEL_ERROR, log);
    }
    return devCount;
}

/*********************************************************************************************\
 *  Dallas Start Temperature Conversion, expected duration:
 *    9 bits resolution ->  93.75 ms
 *   10 bits resolution -> 187.5 ms
 *   11 bits resolution -> 375 ms
 *   12 bits resolution -> 750 ms
\*********************************************************************************************/
boolean Plugin_004_DS_startConversion(uint8_t * ROM, uint8_t pin, boolean pullup, boolean parasite, uint8_t res) {
    if (Plugin_004_DS_reset(pin, pullup)) {
        Plugin_004_ROM_select(ROM, pin);
        Plugin_004_DS_write(STARTCONVO, pin);
        if (parasite) {
            digitalWrite(pin, HIGH);
            pinMode(pin, OUTPUT);
        }
        switch (res) {
            case 9:
                Plugin_004_timeoutGPIO[pin] = millis() + 94;
                break;
            case 10:
                Plugin_004_timeoutGPIO[pin] = millis() + 188;
                break;
            case 11:
                Plugin_004_timeoutGPIO[pin] = millis() + 376;
                break;
            default:
                Plugin_004_timeoutGPIO[pin] = millis() + 751;
        }
        return true;
    } else {
        String log = F("DS   : startConversion: reset fail ");
        Plugin_004_toString(ROM, log);
        addLog(LOG_LEVEL_ERROR, log);
        return false;
    }
}

boolean Plugin_004_DS_readSP(uint8_t * ROM, byte * ScratchPad, uint8_t pin, boolean pullup) {
    boolean crc_ok = false;
    uint8_t retries = 3;
    do {
        if (retries-- == 0) return false;
        if (Plugin_004_DS_reset(pin, pullup)) {
            Plugin_004_ROM_select(ROM, pin);
            Plugin_004_DS_write(READSCRATCH, pin);

            for (byte i = 0; i < SP_SIZE; i++)
                ScratchPad[i] = Plugin_004_DS_read(pin, pullup);

            crc_ok = Plugin_004_DS_crc8(ScratchPad);
            //SP of all zeros makes crc ok, fix it
            if (ScratchPad[CONFIGURATION] == 0) crc_ok = false;

            if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
                String log = F("DS: SP: ");

                for (byte x = 0; x < SP_SIZE; x++) {
                    if (x != 0)
                        log += ',';
                    log += String(ScratchPad[x], HEX);
                }

                if (crc_ok)
                    log += F(",CRC OK");
                else
                    log += F(",CRC FAIL");

                addLog(LOG_LEVEL_DEBUG, log);
            }
        } else {
            String log = F("DS   : readSP: reset fail ");
            Plugin_004_toString(ROM, log);
            addLog(LOG_LEVEL_ERROR, log);
        }
    } while (!crc_ok);

    return true;
}

/*********************************************************************************************\
 *  Dallas Read temperature from scratchpad
\*********************************************************************************************/
boolean Plugin_004_DS_readTemp(uint8_t * ROM, float * value, uint8_t pin, boolean pullup) {
    int16_t DSTemp;
    byte ScratchPad[SP_SIZE];

    bool crc_ok = Plugin_004_DS_readSP(ROM, ScratchPad, pin, pullup);

    if (!crc_ok) {
        *value = 0;
        return false;
    }

    if ((ROM[0] == ROM_DS18B20) || (ROM[0] == ROM_DS1825) || (ROM[0] == ROM_DS1822)) {
        DSTemp = (ScratchPad[TEMP_MSB] << 8) + ScratchPad[TEMP_LSB];
        *value = (float(DSTemp) * 0.0625);
    } else if (ROM[0] == ROM_DS1820_DS18S20) {
        DSTemp = (ScratchPad[TEMP_MSB] << 11) | ScratchPad[TEMP_LSB] << 3;
        DSTemp = ((DSTemp & 0xfff0) << 3) - 16 +
                (((ScratchPad[COUNT_PER_C] - ScratchPad[COUNT_REMAIN]) << 7) / ScratchPad[COUNT_PER_C]);
        *value = float(DSTemp) * 0.0078125;
    }
    return true;
}

/*********************************************************************************************\
 * Dallas Get Resolution
\*********************************************************************************************/
int Plugin_004_DS_getResolution(uint8_t * ROM, uint8_t pin, boolean pullup) {
    // DS1820 and DS18S20 have no resolution configuration register
    if (ROM[0] == ROM_DS1820_DS18S20) return 12;

    byte ScratchPad[9];

    if (Plugin_004_DS_readSP(ROM, ScratchPad, pin, pullup)) {
        switch (ScratchPad[CONFIGURATION]) {
            case TEMP_9_BIT:
                return 9;
            case TEMP_10_BIT:
                return 10;
            case TEMP_11_BIT:
                return 11;
            default:
                return 12; //default to 12 bit
        }
    } else {
        return 12;
    }
}

/*********************************************************************************************\
 * Dallas set Resolution
\*********************************************************************************************/
boolean Plugin_004_DS_setResolution(uint8_t ROM[8], byte res, uint8_t pin, boolean pullup) {
    // DS1820 and DS18S20 have no resolution configuration register
    if (ROM[0] == ROM_DS1820_DS18S20) return true;

    byte ScratchPad[SP_SIZE];

    if (Plugin_004_DS_readSP(ROM, ScratchPad, pin, pullup)) {
        switch (res) {
            case 9:
                ScratchPad[CONFIGURATION] = TEMP_9_BIT;
                break;
            case 10:
                ScratchPad[CONFIGURATION] = TEMP_10_BIT;
                break;
            case 11:
                ScratchPad[CONFIGURATION] = TEMP_11_BIT;
                break;
            default:
                ScratchPad[CONFIGURATION] = TEMP_12_BIT;
        }

        Plugin_004_DS_reset(pin, pullup);
        Plugin_004_ROM_select(ROM, pin);

        Plugin_004_DS_write(WRITESCRATCH, pin);
        Plugin_004_DS_write(ScratchPad[HIGH_ALARM_TEMP], pin);
        Plugin_004_DS_write(ScratchPad[LOW_ALARM_TEMP], pin);
        Plugin_004_DS_write(ScratchPad[CONFIGURATION], pin);

        //for some reason this has never worked :-(
        /*
           Plugin_004_ROM_select(ROM);

           // save the newly written values to eeprom
           Plugin_004_DS_write(COPYSCRATCH);
           delay(100); // <--- added 20ms delay to allow 10ms long EEPROM write operation (as specified by datasheet)
         */
        return true;
    } else {
        return false;
    }

}

/*********************************************************************************************\
 *  Dallas Reset
\*********************************************************************************************/
uint8_t Plugin_004_DS_reset(uint8_t pin, boolean pullup) {
    uint8_t r;
    uint8_t retries = 10;
#if defined(ESP32)
    ESP32noInterrupts();
#endif
    if (pullup)
        pinMode(pin, INPUT_PULLUP);
    else
        pinMode(pin, INPUT);

    while (!digitalRead(pin)) // wait until the wire is high... just in case
    {
        if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
            String log = F("DS: RESET: bus low, wait ...");
            addLog(LOG_LEVEL_DEBUG, log);
        }
        delayMicroseconds(5);
        if (retries-- == 0)
            return 0;
    }
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
    delayMicroseconds(480); // Dallas spec. = Min. 480uSec. Arduino 500uSec.
    if (pullup)
        pinMode(pin, INPUT_PULLUP);
    else
        pinMode(pin, INPUT);

    delayMicroseconds(70);
    r = !digitalRead(pin);
    delayMicroseconds(410);
#if defined(ESP32)
    ESP32interrupts();
#endif
    return r;
}

#define FALSE 0
#define TRUE  1

unsigned char ROM_NO[8];
uint8_t LastDiscrepancy;
uint8_t LastFamilyDiscrepancy;
uint8_t LastDeviceFlag;

/*********************************************************************************************\
 *  Dallas Reset Search
\*********************************************************************************************/
void Plugin_004_DS_reset_search() {
    // reset the search state
    LastDiscrepancy = 0;
    LastDeviceFlag = FALSE;
    LastFamilyDiscrepancy = 0;
    for (byte i = 0; i < 8; i++)
        ROM_NO[i] = 0;
}

/*********************************************************************************************\
 *  Dallas Search bus
\*********************************************************************************************/
uint8_t Plugin_004_DS_search(uint8_t * newAddr, uint8_t pin, boolean pullup) {
    uint8_t id_bit_number;
    uint8_t last_zero, rom_byte_number, search_result;
    uint8_t id_bit, cmp_id_bit;
    unsigned char rom_byte_mask, search_direction;

    // initialize for search
    id_bit_number = 1;
    last_zero = 0;
    rom_byte_number = 0;
    rom_byte_mask = 1;
    search_result = 0;

    // if the last call was not the last one
    if (!LastDeviceFlag) {
        // 1-Wire reset
        if (!Plugin_004_DS_reset(pin, pullup)) {
            // reset the search
            LastDiscrepancy = 0;
            LastDeviceFlag = FALSE;
            LastFamilyDiscrepancy = 0;
            return FALSE;
        }

        Plugin_004_DS_write(BUSSEARCH, pin);

        // loop to do the search
        do {
            // read a bit and its complement
            id_bit = Plugin_004_DS_read_bit(pin, pullup);
            cmp_id_bit = Plugin_004_DS_read_bit(pin, pullup);

            // check for no devices on 1-wire
            if ((id_bit == 1) && (cmp_id_bit == 1))
                break;
            else {
                // all devices coupled have 0 or 1
                if (id_bit != cmp_id_bit)
                    search_direction = id_bit; // bit write value for search
                else {
                    // if this discrepancy if before the Last Discrepancy
                    // on a previous next then pick the same as last time
                    if (id_bit_number < LastDiscrepancy)
                        search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
                    else
                        // if equal to last pick 1, if not then pick 0
                        search_direction = (id_bit_number == LastDiscrepancy);

                    // if 0 was picked then record its position in LastZero
                    if (search_direction == 0) {
                        last_zero = id_bit_number;

                        // check for Last discrepancy in family
                        if (last_zero < 9)
                            LastFamilyDiscrepancy = last_zero;
                    }
                }

                // set or clear the bit in the ROM byte rom_byte_number
                // with mask rom_byte_mask
                if (search_direction == 1)
                    ROM_NO[rom_byte_number] |= rom_byte_mask;
                else
                    ROM_NO[rom_byte_number] &= ~rom_byte_mask;

                // serial number search direction write bit
                Plugin_004_DS_write_bit(search_direction, pin);

                // increment the byte counter id_bit_number
                // and shift the mask rom_byte_mask
                id_bit_number++;
                rom_byte_mask <<= 1;

                // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
                if (rom_byte_mask == 0) {
                    rom_byte_number++;
                    rom_byte_mask = 1;
                }
            }
        } while (rom_byte_number < 8); // loop until through all ROM bytes 0-7

        // if the search was successful then
        if (!(id_bit_number < 65)) {
            // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
            LastDiscrepancy = last_zero;

            // check for last device
            if (LastDiscrepancy == 0)
                LastDeviceFlag = TRUE;

            search_result = TRUE;
        }
    }

    // if no device found then reset counters so next 'search' will be like a first
    if (!search_result || !ROM_NO[0]) {
        LastDiscrepancy = 0;
        LastDeviceFlag = FALSE;
        LastFamilyDiscrepancy = 0;
        search_result = FALSE;
    }

    for (int i = 0; i < 8; i++)
        newAddr[i] = ROM_NO[i];

    return search_result;
}

/*********************************************************************************************\
 *  Dallas Read byte
\*********************************************************************************************/
uint8_t Plugin_004_DS_read(uint8_t pin, boolean pullup) {

    uint8_t bitMask;
    uint8_t r = 0;

    for (bitMask = 0x01; bitMask; bitMask <<= 1)
        if (Plugin_004_DS_read_bit(pin, pullup))
            r |= bitMask;

    return r;
}

/*********************************************************************************************\
 *  Dallas Write byte
\*********************************************************************************************/
void Plugin_004_DS_write(uint8_t ByteToWrite, uint8_t pin) {
    uint8_t bitMask;
    for (bitMask = 0x01; bitMask; bitMask <<= 1)
        Plugin_004_DS_write_bit((bitMask & ByteToWrite) ? 1 : 0, pin);
}

/*********************************************************************************************\
 *  Dallas Read bit
\*********************************************************************************************/
uint8_t Plugin_004_DS_read_bit(uint8_t pin, boolean pullup) {
    uint8_t r;

#if defined(ESP32)
    ESP32noInterrupts();
#endif
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
    delayMicroseconds(3);
    if (pullup)
        pinMode(pin, INPUT_PULLUP);
    else
        pinMode(pin, INPUT);
    delayMicroseconds(10);
    r = digitalRead(pin);
#if defined(ESP32)
    ESP32interrupts();
#endif
    delayMicroseconds(53);
    return r;
}

/*********************************************************************************************\
 *  Dallas Write bit
\*********************************************************************************************/
void Plugin_004_DS_write_bit(uint8_t v, uint8_t pin) {
    if (v & 1) {
#if defined(ESP32)
        ESP32noInterrupts();
#endif
        digitalWrite(pin, LOW);
        pinMode(pin, OUTPUT);
        delayMicroseconds(10);
        digitalWrite(pin, HIGH);
#if defined(ESP32)
        ESP32interrupts();
#endif
        delayMicroseconds(55);
    } else {
#if defined(ESP32)
        ESP32noInterrupts();
#endif
        digitalWrite(pin, LOW);
        pinMode(pin, OUTPUT);
        delayMicroseconds(65);
        digitalWrite(pin, HIGH);
#if defined(ESP32)
        ESP32interrupts();
#endif
        delayMicroseconds(5);
    }
}

/*********************************************************************************************\
 *  Dallas Calculate CRC8 and compare it of addr[0-7] and compares it to addr[8]
\*********************************************************************************************/
boolean Plugin_004_DS_crc8(uint8_t * addr) {
    uint8_t crc = 0;
    uint8_t len = 8;

    while (len--) {
        uint8_t inbyte = *addr++; // from 0 to 7
        for (uint8_t i = 8; i; i--) {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix) crc ^= 0x8C;
            inbyte >>= 1;
        }
    }
    return crc == *addr; // addr 8
}
#endif // USES_P004
