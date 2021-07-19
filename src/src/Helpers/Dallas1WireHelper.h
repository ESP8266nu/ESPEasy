#ifndef HELPERS_DALLAS1WIREHELPER_H
#define HELPERS_DALLAS1WIREHELPER_H

#include <Arduino.h>

#include "../DataTypes/TaskIndex.h"
#include "../DataTypes/PluginID.h"


// Used timings based on Maxim documentation.
// See https://www.maximintegrated.com/en/design/technical-documents/app-notes/1/126.html
// We use the "standard speed" timings, not the "Overdrive speed"


struct Dallas_SensorData {
  bool check_sensor(int8_t gpio_rx,
                    int8_t gpio_tx,
                    int8_t res);

  void set_measurement_inactive();

  bool initiate_read(int8_t gpio_rx,
                     int8_t gpio_tx,
                     int8_t res);

  bool collect_value(int8_t gpio_rx, int8_t gpio_tx);

  String get_formatted_address() const;

  uint64_t addr              = 0;
  float    value             = 0.0f;
  uint32_t read_success      = 0;
  uint32_t read_failed       = 0;  
  uint8_t  actual_res        = 0;
  bool     measurementActive = false;
  bool     valueRead         = false;
  bool     parasitePowered   = false;
  bool     lastReadError     = false;
};




/*********************************************************************************************\
   Variables used to keep track of scanning the bus
   N.B. these should not be shared for simultaneous scans on different pins
\*********************************************************************************************/
extern unsigned char ROM_NO[8];
extern uint8_t LastDiscrepancy;
extern uint8_t LastFamilyDiscrepancy;
extern uint8_t LastDeviceFlag;


/*********************************************************************************************\
   Timings for diagnostics regarding the reset + presence detection
\*********************************************************************************************/
extern long usec_release;   // Time needed for the line to rise (typ: < 1 usec)
extern long presence_start; // Start presence condition after release by master (typ: 30 usec)
extern long presence_end;   // End presence condition (minimal 60 usec, typ: 100 usec)


/*********************************************************************************************\
   Format 1-wire address
\*********************************************************************************************/
String Dallas_getModel(uint8_t family);

String Dallas_format_address(const uint8_t addr[]);

uint64_t Dallas_addr_to_uint64(const uint8_t addr[]);

void Dallas_uint64_to_addr(uint64_t value, uint8_t addr[]);

void Dallas_addr_selector_webform_load(taskIndex_t TaskIndex, int8_t gpio_pin_rx, int8_t gpio_pin_tx, uint8_t nrVariables = 1);

void Dallas_show_sensor_stats_webform_load(const Dallas_SensorData& sensor_data);

void Dallas_addr_selector_webform_save(taskIndex_t TaskIndex, int8_t gpio_pin_rx, int8_t gpio_pin_tx, uint8_t nrVariables = 1);

bool Dallas_plugin(pluginID_t pluginID);

// Load ROM address from tasksettings
void Dallas_plugin_get_addr(uint8_t addr[], taskIndex_t TaskIndex, uint8_t var_index = 0);

void Dallas_plugin_set_addr(uint8_t addr[], taskIndex_t TaskIndex, uint8_t var_index = 0);


/*********************************************************************************************\
   Dallas Scan bus
\*********************************************************************************************/
uint8_t   Dallas_scan(uint8_t     getDeviceROM,
                   uint8_t *ROM,
                   int8_t   gpio_pin_rx,
                   int8_t   gpio_pin_tx);

// read power supply
bool Dallas_is_parasite(const uint8_t ROM[8],
                        int8_t        gpio_pin_rx,
                        int8_t        gpio_pin_tx);

void Dallas_startConversion(const uint8_t ROM[8],
                            int8_t        gpio_pin_rx,
                            int8_t        gpio_pin_tx);

/*********************************************************************************************\
*  Dallas data from scratchpad
\*********************************************************************************************/
bool Dallas_readTemp(const uint8_t ROM[8],
                     float        *value,
                     int8_t        gpio_pin_rx,
                     int8_t        gpio_pin_tx);

bool Dallas_readiButton(const uint8_t addr[8],
                        int8_t     gpio_pin_rx,
                        int8_t     gpio_pin_tx);

bool Dallas_readCounter(const uint8_t ROM[8],
                        float        *value,
                        int8_t        gpio_pin_rx,
                        int8_t        gpio_pin_tx,
                        uint8_t       counter);

/*********************************************************************************************\
* Dallas Get Resolution
\*********************************************************************************************/
uint8_t Dallas_getResolution(const uint8_t ROM[8],
                          int8_t        gpio_pin_rx,
                          int8_t        gpio_pin_tx);

/*********************************************************************************************\
* Dallas Set Resolution
\*********************************************************************************************/
bool Dallas_setResolution(const uint8_t ROM[8],
                          uint8_t          res,
                          int8_t        gpio_pin_rx,
                          int8_t        gpio_pin_tx);

/*********************************************************************************************\
*  Dallas Reset
\*********************************************************************************************/
uint8_t Dallas_reset(int8_t gpio_pin_rx, int8_t gpio_pin_tx);


/*********************************************************************************************\
*  Dallas Reset Search
\*********************************************************************************************/
void    Dallas_reset_search();

/*********************************************************************************************\
*  Dallas Search bus
\*********************************************************************************************/
uint8_t Dallas_search(uint8_t *newAddr,
                      int8_t   gpio_pin_rx,
                      int8_t   gpio_pin_tx);

/*********************************************************************************************\
*  Dallas Read byte
\*********************************************************************************************/
uint8_t Dallas_read(int8_t gpio_pin_rx, int8_t gpio_pin_tx);

/*********************************************************************************************\
*  Dallas Write byte
\*********************************************************************************************/
void    Dallas_write(uint8_t ByteToWrite,
                     int8_t  gpio_pin_rx,
                     int8_t  gpio_pin_tx);

/*********************************************************************************************\
*  Dallas Read bit
*  See https://github.com/espressif/arduino-esp32/issues/1335
\*********************************************************************************************/
uint8_t Dallas_read_bit(int8_t gpio_pin_rx, int8_t gpio_pin_tx);
uint8_t Dallas_read_bit_ISR(int8_t gpio_pin_rx, int8_t gpio_pin_tx, unsigned long start);

/*********************************************************************************************\
*  Dallas Write bit
*  See https://github.com/espressif/arduino-esp32/issues/1335
\*********************************************************************************************/
void Dallas_write_bit(uint8_t v,
                      int8_t  gpio_pin_rx,
                      int8_t  gpio_pin_tx);

void Dallas_write_bit_ISR(uint8_t v,
                      int8_t  gpio_pin_rx,
                      int8_t  gpio_pin_tx,
                      long low_time,
                      long high_time,
                      unsigned long &start);

/*********************************************************************************************\
*  Standard function to initiate addressing a sensor.
\*********************************************************************************************/
bool Dallas_address_ROM(const uint8_t ROM[8],
                        int8_t        gpio_pin_rx,
                        int8_t        gpio_pin_tx);

/*********************************************************************************************\
*  Dallas Calculate CRC8 and compare it of addr[0-7] and compares it to addr[8]
\*********************************************************************************************/
bool     Dallas_crc8(const uint8_t *addr);

/*********************************************************************************************\
*  Dallas Calculate CRC16
\*********************************************************************************************/
uint16_t Dallas_crc16(const uint8_t *input,
                      uint16_t       len,
                      uint16_t       crc);



#endif // ifndef HELPERS_DALLAS1WIREHELPER_H
